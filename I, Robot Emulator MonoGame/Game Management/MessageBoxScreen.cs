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
using Microsoft.Xna.Framework.Input;
using System;

namespace GameManagement
{
    /// <summary>
    /// A popup message box screen, used to display "are you sure?"
    /// confirmation messages.
    /// </summary>
    class MessageBoxScreen : Screen
    {
        string Message;
        Texture2D? GradientTexture;

        readonly InputAction menuSelect;
        readonly InputAction menuCancel;

        public event EventHandler<PlayerIndexEventArgs>? Accepted;
        public event EventHandler<PlayerIndexEventArgs>? Cancelled;

        /// <summary>
        /// Constructor automatically includes the standard "A=ok, B=cancel"
        /// usage text prompt.
        /// </summary>
        public MessageBoxScreen(ScreenManager screenManager, string message)
            : this(screenManager, message, true)
        { }


        /// <summary>
        /// Constructor lets the caller specify whether to include the standard
        /// "A=ok, B=cancel" usage text prompt.
        /// </summary>
        public MessageBoxScreen(ScreenManager screenManager, string message, bool includeUsageText) : base(screenManager)
        {
            const string usageText = "\n\nA button, Space, Enter = ok" +
                                     "\nB button, Esc = cancel";

            if (includeUsageText)
                this.Message = message + usageText;
            else
                this.Message = message;

            IsPopup = true;

            TransitionOnTime = TimeSpan.FromSeconds(0.2);
            TransitionOffTime = TimeSpan.FromSeconds(0.2);

            menuSelect = new InputAction(
                new Buttons[] { Buttons.A, Buttons.Start },
                new Keys[] { Keys.Space, Keys.Enter },
                true);
            menuCancel = new InputAction(
                new Buttons[] { Buttons.B, Buttons.Back },
                new Keys[] { Keys.Escape, Keys.Back },
                true);
        }


        /// <summary>
        /// Loads graphics content for this screen. This uses the shared ContentManager
        /// provided by the Game class, so the content will remain loaded forever.
        /// Whenever a subsequent MessageBoxScreen tries to load this same content,
        /// it will just get back another reference to the already loaded data.
        /// </summary>
        public override void Activate(bool instancePreserved)
        {
            if (!instancePreserved)
                GradientTexture = Game.Content.Load<Texture2D>("gradient");
        }


        /// <summary>
        /// Responds to user input, accepting or cancelling the message box.
        /// </summary>
        public override void HandleInput(GameTime gameTime, InputState input)
        {
            PlayerIndex playerIndex = PlayerIndex.One;

            // We pass in our ControllingPlayer, which may either be null (to
            // accept input from any player) or a specific index. If we pass a null
            // controlling player, the InputState helper returns to us which player
            // actually provided the input. We pass that through to our Accepted and
            // Cancelled events, so they can tell which player triggered them.
            if (menuSelect?.Evaluate(input, ControllingPlayer, out playerIndex) ?? false)
            {
                // Raise the accepted event, then exit the message box.
                Accepted?.Invoke(this, new PlayerIndexEventArgs(playerIndex));

                ExitScreen();
            }
            else if (menuCancel?.Evaluate(input, ControllingPlayer, out playerIndex) ?? false)
            {
                // Raise the cancelled event, then exit the message box.
                Cancelled?.Invoke(this, new PlayerIndexEventArgs(playerIndex));

                ExitScreen();
            }
        }


        /// <summary>
        /// Draws the message box.
        /// </summary>
        public override void Draw(GameTime gameTime)
        {
            if ((ScreenManager.SpriteBatch is SpriteBatch spriteBatch)
                && (ScreenManager.Font is SpriteFont font))
            {
                // Darken down any other screens that were drawn beneath the popup.
                ScreenManager.FadeBackBufferToBlack(TransitionAlpha * 2 / 3);

                // Center the message text in the viewport.
                Viewport viewport = ScreenManager.GraphicsDevice.Viewport;
                Vector2 viewportSize = new Vector2(viewport.Width, viewport.Height);
                Vector2 textSize = font.MeasureString(Message);
                Vector2 textPosition = (viewportSize - textSize) / 2;

                // The background includes a border somewhat larger than the text itself.
                const int hPad = 32;
                const int vPad = 16;

                Rectangle backgroundRectangle = new Rectangle((int)textPosition.X - hPad,
                                                              (int)textPosition.Y - vPad,
                                                              (int)textSize.X + hPad * 2,
                                                              (int)textSize.Y + vPad * 2);

                // Fade the popup alpha during transitions.
                Color color = Color.White * TransitionAlpha;

                spriteBatch.Begin();

                // Draw the background rectangle.
                if (GradientTexture != null)
                    spriteBatch.Draw(GradientTexture, backgroundRectangle, color);

                // Draw the message box text.
                spriteBatch.DrawString(font, Message, textPosition, color);

                spriteBatch.End();
            }
        }
    }
}