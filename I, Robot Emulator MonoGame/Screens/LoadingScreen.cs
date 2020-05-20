// Copyright 2020 by John Manfreda. All Rights Reserved.
// https://www.manfreda.org/
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.If not, see<https://www.gnu.org/licenses/>.

using System;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using GameManagement;

namespace I_Robot
{
    /// <summary>
    /// The loading screen coordinates transitions between the menu system and the
    /// game itself. Normally one screen will transition off at the same time as
    /// the next screen is transitioning on, but for larger transitions that can
    /// take a longer time to load their data, we want the menu system to be entirely
    /// gone before we start loading the game. This is done as follows:
    /// 
    /// - Tell all the existing screens to transition off.
    /// - Activate a loading screen, which will transition on at the same time.
    /// - The loading screen watches the state of the previous screens.
    /// - When it sees they have finished transitioning off, it activates the real
    ///   next screen, which may take a long time to load its data. The loading
    ///   screen will be the only thing displayed while this load is taking place.
    /// </summary>
    class LoadingScreen : Screen
    {
        bool LoadingIsSlow;
        bool OtherScreensAreGone;

        Screen[] ScreensToLoad;

        /// <summary>
        /// The constructor is private: loading screens should be activated via the static Load method instead.
        /// </summary>
        private LoadingScreen(ScreenManager screenManager, bool loadingIsSlow, Screen[] screensToLoad)
        {
            this.LoadingIsSlow = loadingIsSlow;
            this.ScreensToLoad = screensToLoad;

            TransitionOnTime = TimeSpan.FromSeconds(0.5);
        }


        /// <summary>
        /// Activates the loading screen.
        /// </summary>
        public static void Load(ScreenManager screenManager, bool loadingIsSlow,
                                PlayerIndex? controllingPlayer,
                                params Screen[] screensToLoad)
        {
            // Tell all the current screens to transition off.
            foreach (Screen screen in screenManager.GetScreens())
                screen.ExitScreen();

            // Create and activate the loading screen.
            LoadingScreen loadingScreen = new LoadingScreen(screenManager,
                                                            loadingIsSlow,
                                                            screensToLoad);

            screenManager.AddScreen(loadingScreen, controllingPlayer);
        }

        /// <summary>
        /// Updates the loading screen.
        /// </summary>
        public override void Update(GameTime gameTime, bool otherScreenHasFocus,
                                                       bool coveredByOtherScreen)
        {
            base.Update(gameTime, otherScreenHasFocus, coveredByOtherScreen);

            // If all the previous screens have finished transitioning
            // off, it is time to actually perform the load.
            if (OtherScreensAreGone)
            {
                ScreenManager?.RemoveScreen(this);

                foreach (Screen screen in ScreensToLoad)
                {
                    if (screen != null)
                        ScreenManager?.AddScreen(screen, ControllingPlayer);
                }

                // Once the load has finished, we use ResetElapsedTime to tell
                // the  game timing mechanism that we have just finished a very
                // long frame, and that it should not try to catch up.
                ScreenManager?.Game.ResetElapsedTime();
            }
        }


        /// <summary>
        /// Draws the loading screen.
        /// </summary>
        public override void Draw(GameTime gameTime)
        {
            // If we are the only active screen, that means all the previous screens
            // must have finished transitioning off. We check for this in the Draw
            // method, rather than in Update, because it isn't enough just for the
            // screens to be gone: in order for the transition to look good we must
            // have actually drawn a frame without them before we perform the load.
            if ((ScreenState == State.Active) && (ScreenManager?.GetScreens().Length == 1))
            {
                OtherScreensAreGone = true;
            }

            // The gameplay screen takes a while to load, so we display a loading
            // message while that is going on, but the menus load very quickly, and
            // it would look silly if we flashed this up for just a fraction of a
            // second while returning from the game to the menus. This parameter
            // tells us how long the loading is going to take, so we know whether
            // to bother drawing the message.
            if (LoadingIsSlow)
            {
                if ((ScreenManager is ScreenManager screenManager) &&
                    (ScreenManager?.SpriteBatch is SpriteBatch spriteBatch))
                {
                    SpriteFont? font = ScreenManager?.Font;

                    const string message = "Loading...";

                    // Center the text in the viewport.
                    Viewport viewport = screenManager.GraphicsDevice.Viewport;
                    Vector2 viewportSize = new Vector2(viewport.Width, viewport.Height);
                    Vector2 textSize = font?.MeasureString(message) ?? Vector2.Zero;
                    Vector2 textPosition = (viewportSize - textSize) / 2;

                    Color color = Color.White * TransitionAlpha;

                    // Draw the text.
                    spriteBatch.Begin();
                    spriteBatch.DrawString(font, message, textPosition, color);
                    spriteBatch.End();
                }
            }
        }
    }
}
