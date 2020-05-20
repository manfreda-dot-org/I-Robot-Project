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
using System.Collections.Generic;

namespace GameManagement
{
    /// <summary>
    /// Base class for screens that contain a menu of options. The user can
    /// move up and down to select an entry, or cancel to back out of the screen.
    /// </summary>
    abstract class MenuScreen : Screen
    {
        List<MenuItem> MenuItemList = new List<MenuItem>();
        int SelectedItem = 0;
        string Title;

        InputAction MenuUp;
        InputAction MenuDown;
        InputAction MenuSelect;
        InputAction MenuCancel;

        /// <summary>
        /// Gets the list of menu entries, so derived classes can add or change the menu contents.
        /// </summary>
        protected IList<MenuItem> MenuItems
        {
            get { return MenuItemList; }
        }

        public MenuScreen(string title)
        {
            this.Title = title;

            TransitionOnTime = TimeSpan.FromSeconds(0.5);
            TransitionOffTime = TimeSpan.FromSeconds(0.5);

            MenuUp = new InputAction(
                new Buttons[] { Buttons.DPadUp, Buttons.LeftThumbstickUp }, 
                new Keys[] { Keys.Up },
                true);
            MenuDown = new InputAction(
                new Buttons[] { Buttons.DPadDown, Buttons.LeftThumbstickDown },
                new Keys[] { Keys.Down },
                true);
            MenuSelect = new InputAction(
                new Buttons[] { Buttons.A, Buttons.Start },
                new Keys[] { Keys.Enter, Keys.Space },
                true);
            MenuCancel = new InputAction(
                new Buttons[] { Buttons.B, Buttons.Back },
                new Keys[] { Keys.Escape },
                true);
        }


        /// <summary>
        /// Responds to user input, changing the selected entry and accepting
        /// or cancelling the menu.
        /// </summary>
        public override void HandleInput(GameTime gameTime, InputState input)
        {
            // For input tests we pass in our ControllingPlayer, which may
            // either be null (to accept input from any player) or a specific index.
            // If we pass a null controlling player, the InputState helper returns to
            // us which player actually provided the input. We pass that through to
            // OnSelectEntry and OnCancel, so they can tell which player triggered them.
            PlayerIndex playerIndex;

            // Move to the previous menu entry?
            if (MenuUp.Evaluate(input, ControllingPlayer, out playerIndex))
            {
                do
                {
                    SelectedItem--;

                    if (SelectedItem < 0)
                        SelectedItem = MenuItemList.Count - 1;
                } while (!MenuItemList[SelectedItem].IsSelectable);
            }

            // Move to the next menu entry?
            if (MenuDown.Evaluate(input, ControllingPlayer, out playerIndex))
            {
                do
                {
                    SelectedItem++;

                    if (SelectedItem >= MenuItemList.Count)
                        SelectedItem = 0;
                } while (!MenuItemList[SelectedItem].IsSelectable);
            }

            if (MenuSelect.Evaluate(input, ControllingPlayer, out playerIndex))
            {
                OnSelectEntry(SelectedItem, playerIndex);
            }
            else if (MenuCancel.Evaluate(input, ControllingPlayer, out playerIndex))
            {
                OnCancel(playerIndex);
            }
        }


        /// <summary>
        /// Handler for when the user has chosen a menu entry.
        /// </summary>
        protected virtual void OnSelectEntry(int entryIndex, PlayerIndex playerIndex)
        {
            MenuItemList[entryIndex].OnSelectEntry(playerIndex);
        }


        /// <summary>
        /// Handler for when the user has cancelled the menu.
        /// </summary>
        protected virtual void OnCancel(PlayerIndex playerIndex)
        {
            ExitScreen();
        }


        /// <summary>
        /// Helper overload makes it easy to use OnCancel as a MenuEntry event handler.
        /// </summary>
        protected void OnCancel(object? sender, PlayerIndexEventArgs e)
        {
            OnCancel(e.PlayerIndex);
        }


        /// <summary>
        /// Allows the screen the chance to position the menu entries. By default
        /// all menu entries are lined up in a vertical list, centered on the screen.
        /// </summary>
        protected virtual void UpdateMenuEntryLocations()
        {
            // Make the menu slide into place during transitions, using a
            // power curve to make things look more interesting (this makes
            // the movement slow down as it nears the end).
            float transitionOffset = (float)Math.Pow(TransitionPosition, 2);

            // start at Y = 175; each X value is generated per entry
            Vector2 position = new Vector2(0f, 175f);

            // update each menu entry's location in turn
            for (int i = 0; i < MenuItemList.Count; i++)
            {
                MenuItem menuEntry = MenuItemList[i];
                
                // each entry is to be centered horizontally
                position.X = ScreenManager?.GraphicsDevice.Viewport.Width / 2 - menuEntry.GetWidth(this) / 2 ?? 0;

                if (ScreenState == State.TransitionOn)
                    position.X -= transitionOffset * 256;
                else
                    position.X += transitionOffset * 512;

                position.Y += menuEntry.SpacingBefore * menuEntry.GetHeight(this);

                // set the entry's position
                menuEntry.Position = position;

                // move down for the next entry the size of this entry
                position.Y += menuEntry.SpacingAfter * menuEntry.GetHeight(this);
            }
        }

        /// <summary>
        /// Updates the menu.
        /// </summary>
        public override void Update(GameTime gameTime, bool otherScreenHasFocus, bool coveredByOtherScreen)
        {
            base.Update(gameTime, otherScreenHasFocus, coveredByOtherScreen);

            // Update each nested MenuEntry object.
            for (int i = 0; i < MenuItemList.Count; i++)
            {
                bool isSelected = IsActive && (i == SelectedItem);

                MenuItemList[i].Update(this, isSelected, gameTime);
            }
        }


        /// <summary>
        /// Draws the menu.
        /// </summary>
        public override void Draw(GameTime gameTime)
        {
            // make sure our entries are in the right place before we draw them
            UpdateMenuEntryLocations();

            if ((ScreenManager is ScreenManager screenManager)
                && (ScreenManager?.SpriteBatch is SpriteBatch spriteBatch)
                && (ScreenManager?.GraphicsDevice is GraphicsDevice graphics))
            {
                SpriteFont? font = ScreenManager?.Font;

                spriteBatch.Begin();

                // Draw each menu entry in turn.
                for (int i = 0; i < MenuItemList.Count; i++)
                {
                    MenuItem menuEntry = MenuItemList[i];

                    bool isSelected = IsActive && (i == SelectedItem);

                    menuEntry.Draw(this, isSelected, gameTime);
                }

                // Make the menu slide into place during transitions, using a
                // power curve to make things look more interesting (this makes
                // the movement slow down as it nears the end).
                float transitionOffset = (float)Math.Pow(TransitionPosition, 2);

                // Draw the menu title centered on the screen
                Vector2 titlePosition = new Vector2(graphics.Viewport.Width / 2, 80);
                Vector2 titleOrigin = font?.MeasureString(Title) / 2 ?? Vector2.Zero;
                Color titleColor = new Color(255, 64, 64) * TransitionAlpha;
                float titleScale = 1.5f;

                titlePosition.Y -= transitionOffset * 100;

                spriteBatch.DrawString(font, Title, titlePosition, titleColor, 0, titleOrigin, titleScale, SpriteEffects.None, 0);

                spriteBatch.End();
            }
        }
    }
}
