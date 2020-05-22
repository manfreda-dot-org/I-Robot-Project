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

using GameManagement;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using System;
using System.IO;
using System.Runtime.Serialization.Formatters.Binary;
using System.Runtime.Serialization;
using System.Threading;

namespace I_Robot
{
    /// <summary>
    /// This screen implements the actual game logic. It is just a
    /// placeholder to get the idea across: you'll probably want to
    /// put some more interesting gameplay in here!
    /// </summary>
    class GameScreen : Screen
    {
        readonly AlphanumericsRenderer Alphanumerics;
        readonly MathboxRenderer MathboxRenderer;

        ContentManager? Content;

        float pauseAlpha;

        InputAction PauseAction;

        /// <summary>
        /// Constructor.
        /// </summary>
        public GameScreen(ScreenManager screenManager) : base(screenManager)
        {
            TransitionOnTime = TimeSpan.FromSeconds(1.5);
            TransitionOffTime = TimeSpan.FromSeconds(0.5);

            PauseAction = new InputAction(
                new Buttons[] { Buttons.Start, Buttons.Back },
                new Keys[] { Keys.Escape },
                true);

            MathboxRenderer = Game.MathboxRenderer;
            Alphanumerics = new AlphanumericsRenderer(this);
        }

        /// <summary>
        /// Load graphics content for the game.
        /// </summary>
        public override void Activate(bool instancePreserved)
        {
            Hardware.Paused = false;

            if (!instancePreserved)
            {
                if (Content == null)
                    Content = new ContentManager(Game.Services, "Content");

                // A real game would probably have more content than this sample, so
                // it would take longer to load. We simulate that by delaying for a
                // while, giving you a chance to admire the beautiful loading screen.
                Thread.Sleep(1000);

                // once the load has finished, we use ResetElapsedTime to tell the game's
                // timing mechanism that we have just finished a very long frame, and that
                // it should not try to catch up.
                Game.ResetElapsedTime();
            }
        }

        /// <summary>
        /// Unload graphics content used by the game.
        /// </summary>
        public override void Unload()
        {
            Hardware.Paused = true;
            Alphanumerics.Dispose();
            MathboxRenderer.Dispose();
            Content?.Unload();
        }


        /// <summary>
        /// Updates the state of the game. This method checks the GameScreen.IsActive
        /// property, so the game will stop updating when the pause menu is active,
        /// or if you tab away to a different application.
        /// </summary>
        public override void Update(GameTime gameTime, bool otherScreenHasFocus, bool coveredByOtherScreen)
        {
            base.Update(gameTime, otherScreenHasFocus, false);

            // Gradually fade in or out depending on whether we are covered by the pause screen.
            if (coveredByOtherScreen)
                pauseAlpha = Math.Min(pauseAlpha + 1f / 16, 1);
            else
                pauseAlpha = Math.Max(pauseAlpha - 1f / 16, 0);

            Hardware.Paused = !IsActive;
        }

        /// <summary>
        /// Lets the game respond to player input. Unlike the Update method,
        /// this will only be called when the gameplay screen is active.
        /// </summary>
        public override void HandleInput(GameTime gameTime, InputState input)
        {
            if (input == null)
                throw new ArgumentNullException("input");

            // Look up inputs for the active player profile.
            int playerIndex = (int)(ControllingPlayer ?? 0);

            KeyboardState keyboardState = input.CurrentKeyboardStates;
            GamePadState gamePadState = input.CurrentGamePadStates[playerIndex];

            // The game pauses either if the user presses the pause button, or if
            // they unplug the active gamepad. This requires us to keep track of
            // whether a gamepad was ever plugged in, because we don't want to pause
            // on PC if they are playing with a keyboard and have no gamepad at all!
            bool gamePadDisconnected = !gamePadState.IsConnected &&
                                       input.GamePadWasConnected[playerIndex];

            PlayerIndex player;
            if (PauseAction.Evaluate(input, ControllingPlayer, out player) || gamePadDisconnected)
            {
                Hardware.Paused = true;
                ScreenManager.AddScreen(new MainMenuScreen(ScreenManager), ControllingPlayer);
            }


            Keyboard.GetState();
            if (Keyboard.HasBeenPressed(Keys.Tab))
                Settings.TestSwitch = !Settings.TestSwitch;
            if (Keyboard.HasBeenPressed(Keys.F3))
                Hardware.Reset(Emulation.Hardware.RESET_TYPE.USER);
            if (Keyboard.HasBeenPressed(Keys.F7))
            {
                if (Keyboard.IsPressed(Keys.LeftShift) || Keyboard.IsPressed(Keys.RightShift))
                {
                    // save state
                    using (FileStream stream = new FileStream("irobot.sav", FileMode.Create))
                    {
                        IFormatter formatter = new BinaryFormatter();
                        formatter.Serialize(stream, Hardware);
                        stream.Close();
                    }
                }
                else
                {
                    // load state
                    using (FileStream stream = new FileStream("irobot.sav", FileMode.Open))
                    {
                        //                        IFormatter formatter = new BinaryFormatter();
                        //                        Hardware = (Hardware)formatter.Deserialize(stream);
                        //                        stream.Close();
                    }
                }
            }
            if (Keyboard.HasBeenPressed(Keys.F10))
                Settings.SpeedThrottle = !Settings.SpeedThrottle;
        }


        /// <summary>
        /// Draws the gameplay screen.
        /// </summary>
        public override void Draw(GameTime gameTime)
        {
            // perform any texture rendering first
            // all of this needs to be done before SetRenderTarget(null) is called
            MathboxRenderer.Render(GraphicsDevice);
            Alphanumerics.Render(GraphicsDevice);

            // now assemble the screen
            GraphicsDevice.SetRenderTarget(null);
            MathboxRenderer.Draw(GraphicsDevice);
            Alphanumerics.Draw(GraphicsDevice);

            // If the game is transitioning on or off, fade it out to black.
            if (TransitionPosition > 0 || pauseAlpha > 0)
            {
                float alpha = MathHelper.Lerp(1f - TransitionAlpha, 1f, pauseAlpha);

                ScreenManager.FadeBackBufferToBlack(alpha * 0.75f);
            }
        }
    }
}