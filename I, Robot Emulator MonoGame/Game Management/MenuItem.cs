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

namespace GameManagement
{
    /// <summary>
    /// Helper class represents a single entry in a MenuScreen. By default this
    /// just draws the entry text string, but it can be customized to display menu
    /// entries in different ways. This also provides an event that will be raised
    /// when the menu entry is selected.
    /// </summary>
    class MenuItem
    {
        /// <summary>
        /// Text entry of this menu
        /// </summary>
        public string Text;

        /// <summary>
        /// Tracks a fading selection effect on the entry.
        /// </summary>
        /// <remarks>
        /// The entries transition out of the selection effect when they are deselected.
        /// </remarks>
        float Fade;

        /// <summary>
        /// The position at which the entry is drawn. This is set by the MenuScreen each frame in Update.
        /// </summary>
        public Vector2 Position;

        public readonly float SpacingBefore;
        public readonly float SpacingAfter;
        public bool IsSelectable => (Selected != null);

        /// <summary>
        /// Event raised when the menu entry is selected.
        /// </summary>
        public event EventHandler<PlayerIndexEventArgs>? Selected;

        protected MenuItem(string text, float before = 0, float after = 0)
        {
            Text = text;
            SpacingBefore = before;
            SpacingAfter = 1 + after;
        }

        public MenuItem(string text, EventHandler<PlayerIndexEventArgs> callback, float before = 0, float after = 0 )
            : this (text, before, after)
        {
            Selected += callback;
        }

        /// <summary>
        /// Method for raising the Selected event.
        /// </summary>
        protected internal virtual void OnSelectEntry(PlayerIndex playerIndex)
        {
            Selected?.Invoke(this, new PlayerIndexEventArgs(playerIndex));
        }

        /// <summary>
        /// Updates the menu entry.
        /// </summary>
        public virtual void Update(MenuScreen screen, bool isSelected, GameTime gameTime)
        {
            // When the menu selection changes, entries gradually fade between
            // their selected and deselected appearance, rather than instantly
            // popping to the new state.
            float fadeSpeed = (float)gameTime.ElapsedGameTime.TotalSeconds * 4;

            if (isSelected)
                Fade = Math.Min(Fade + fadeSpeed, 1);
            else
                Fade = Math.Max(Fade - fadeSpeed, 0);
        }


        /// <summary>
        /// Draws the menu entry. This can be overridden to customize the appearance.
        /// </summary>
        public virtual void Draw(MenuScreen screen, bool isSelected, GameTime gameTime)
        {
            // Draw the selected entry in yellow, otherwise white.
            Color color = Color.White;
            if (!IsSelectable)
                color = Color.Gray;
            if (isSelected)
                color = Color.Yellow;

            // Pulsate the size of the selected menu entry.
            double time = gameTime.TotalGameTime.TotalSeconds;
            
            float pulsate = (float)Math.Sin(time * 6) + 1;

            float scale = 1 + pulsate * 0.05f * Fade;

            // Modify the alpha to fade text out during transitions.
            color *= screen.TransitionAlpha;

            // Draw text, centered on the middle of each line.
            if (screen.ScreenManager?.Font is SpriteFont font)
            {
                Vector2 size = font.MeasureString(Text);
                Vector2 origin = new Vector2(size.X / 2, size.Y / 2);

                var p = Position;
                p.X += size.X / 2;
                screen.ScreenManager?.SpriteBatch?.DrawString(font, Text, p, color, 0, origin, scale, SpriteEffects.None, 0);
            }
        }


        /// <summary>
        /// Queries how much space this menu entry requires.
        /// </summary>
        public virtual int GetHeight(MenuScreen screen)
        {
            return screen.ScreenManager?.Font?.LineSpacing ?? 0;
        }


        /// <summary>
        /// Queries how wide the entry is, used for centering on the screen.
        /// </summary>
        public virtual int GetWidth(MenuScreen screen)
        {
            return (int)(screen.ScreenManager?.Font?.MeasureString(Text).X ?? 0);
        }
    }
}
