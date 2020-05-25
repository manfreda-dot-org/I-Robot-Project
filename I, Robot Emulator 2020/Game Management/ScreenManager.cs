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

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using System.Diagnostics;
using System.Collections.Generic;

namespace GameManagement
{
    /// <summary>
    /// The screen manager is a component which manages one or more GameScreen instances.
    /// It maintains a stack of screens, calls their Update and Draw methods at the 
    /// appropriate times, and automatically routes input to the topmost active screen.
    /// </summary>
    public class ScreenManager : DrawableGameComponent
    {
        private const string StateFilename = "ScreenManagerState.xml";

        readonly List<Screen> Screens = new List<Screen>();
        readonly List<Screen> TempScreensList = new List<Screen>();

        readonly InputState Input = new InputState();

        /// <summary>
        /// A default SpriteBatch shared by all the screens. This saves
        /// each screen having to bother creating their own local instance.
        /// </summary>
        public readonly SpriteBatch SpriteBatch;

        /// <summary>
        /// Default in-game font shared by all screens
        /// </summary>
        public readonly SpriteFont GameFont;

        /// <summary>
        /// Default menu font shared by all the screens
        /// </summary>
        public readonly SpriteFont MenuFont;

        /// <summary>
        /// Gets a blank texture that can be used by the screens.
        /// </summary>
        public readonly Texture2D BlankTexture;

        /// <summary>
        /// If true, the manager prints out a list of all the screens
        /// each time it is updated. This can be useful for making sure
        /// everything is being added and removed at the right times.
        /// </summary>
        public bool TraceEnabled { get; set; }

        /// <summary>
        /// Constructs a new screen manager component.
        /// </summary>
        public ScreenManager(Game game)
            : base(game)
        {
            System.Diagnostics.Debug.Assert(GraphicsDevice != null, "ScreenManager cannot be created prior to Game.LoadContent()");
            SpriteBatch = new SpriteBatch(GraphicsDevice);

            Initialize();

            // Load content belonging to the screen manager.
            GameFont = Game.Content.Load<SpriteFont>("gamefont");
            MenuFont = Game.Content.Load<SpriteFont>("menufont");
            BlankTexture = Game.Content.Load<Texture2D>("blank");

            LoadContent();

            // Tell each of the screens to load their content.
            foreach (Screen screen in Screens)
            {
                screen.Activate(false);
            }
        }


        /// <summary>
        /// Unload your graphics content.
        /// </summary>
        protected override void UnloadContent()
        {
            // Tell each of the screens to unload their content.
            foreach (Screen screen in Screens)
            {
                screen.Unload();
            }
        }


        /// <summary>
        /// Allows each screen to run logic.
        /// </summary>
        public override void Update(GameTime gameTime)
        {
            // Read the keyboard and gamepad.
            Input.Update();

            // Make a copy of the master screen list, to avoid confusion if
            // the process of updating one screen adds or removes others.
            TempScreensList.Clear();

            foreach (Screen screen in Screens)
                TempScreensList.Add(screen);

            bool otherScreenHasFocus = !Game.IsActive;
            bool coveredByOtherScreen = false;

            // Loop as long as there are screens waiting to be updated.
            while (TempScreensList.Count > 0)
            {
                // Pop the topmost screen off the waiting list.
                Screen screen = TempScreensList[TempScreensList.Count - 1];

                TempScreensList.RemoveAt(TempScreensList.Count - 1);

                // Update the screen.
                screen.Update(gameTime, otherScreenHasFocus, coveredByOtherScreen);

                if (screen.ScreenState == Screen.State.TransitionOn ||
                    screen.ScreenState == Screen.State.Active)
                {
                    // If this is the first active screen we came across,
                    // give it a chance to handle input.
                    if (!otherScreenHasFocus)
                    {
                        screen.HandleInput(gameTime, Input);

                        otherScreenHasFocus = true;
                    }

                    // If this is an active non-popup, inform any subsequent
                    // screens that they are covered by it.
                    if (!screen.IsPopup)
                        coveredByOtherScreen = true;
                }
            }

            // Print debug trace?
            if (TraceEnabled)
                TraceScreens();
        }


        /// <summary>
        /// Prints a list of all the screens, for debugging.
        /// </summary>
        void TraceScreens()
        {
            List<string> screenNames = new List<string>();

            foreach (Screen screen in Screens)
                screenNames.Add(screen.GetType().Name);

            Debug.WriteLine(string.Join(", ", screenNames.ToArray()));
        }


        /// <summary>
        /// Tells each screen to draw itself.
        /// </summary>
        public override void Draw(GameTime gameTime)
        {
            foreach (Screen screen in Screens)
            {
                if (screen.ScreenState == Screen.State.Hidden)
                    continue;

                screen.Draw(gameTime);
            }
        }

        /// <summary>
        /// Adds a new screen to the screen manager.
        /// </summary>
        public void AddScreen(Screen screen, PlayerIndex? controllingPlayer)
        {
            screen.ControllingPlayer = controllingPlayer;
            screen.IsExiting = false;

            screen.Activate(false);

            Screens.Add(screen);
        }


        /// <summary>
        /// Removes a screen from the screen manager. You should normally
        /// use GameScreen.ExitScreen instead of calling this directly, so
        /// the screen can gradually transition off rather than just being
        /// instantly removed.
        /// </summary>
        public void RemoveScreen(Screen screen)
        {
            // If we have a graphics device, tell the screen to unload content.
            screen.Unload();

            Screens.Remove(screen);
            TempScreensList.Remove(screen);
        }


        /// <summary>
        /// Expose an array holding all the screens. We return a copy rather
        /// than the real master list, because screens should only ever be added
        /// or removed using the AddScreen and RemoveScreen methods.
        /// </summary>
        public Screen[] GetScreens()
        {
            return Screens.ToArray();
        }


        /// <summary>
        /// Helper draws a translucent black fullscreen sprite, used for fading
        /// screens in and out, and for darkening the background behind popups.
        /// </summary>
        public void FadeBackBufferToBlack(float alpha)
        {
            SpriteBatch.Begin();
            SpriteBatch.Draw(BlankTexture, GraphicsDevice.Viewport.Bounds, Color.Black * alpha);
            SpriteBatch.End();
        }

        /// <summary>
        /// Informs the screen manager to serialize its state to disk.
        /// </summary>
        public void Deactivate()
        {
        }

        public bool Activate(bool instancePreserved)
        {
            return false;
        }
    }
}