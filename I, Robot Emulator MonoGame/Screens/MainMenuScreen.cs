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

namespace I_Robot
{
    /// <summary>
    /// The main menu screen is the first thing displayed when the game starts up.
    /// </summary>
    class MainMenuScreen : MenuScreen
    {
        public MainMenuScreen()
            : base("MAIN    MENU")
        {
            // Create our menu entries.
            MenuItems.Add(new MenuItem("RETURN    TO    GAME", OnCancel));
            MenuItems.Add(new MenuItem(SoundString, SoundMenuSelected, 1f));
            MenuItems.Add(new MenuItem("GAME    OPTIONS", OptionsMenuSelected, 0.5f));
            MenuItems.Add(new MenuItem("RENDERING", RenderingMenuSelected, 0.5f));
            MenuItems.Add(new MenuItem("EMULATION", EmulationMenuSelected, 0.5f));
            MenuItems.Add(new MenuItem("QUIT", QuitMenuSelected, 1));
        }

        string SoundString => $"SOUND:    {(Settings.SoundEnabled ? "ENABLED" : "DISABLED")}";

        void PlayGameMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            if (ScreenManager is ScreenManager screenManager)
                LoadingScreen.Load(screenManager, true, e.PlayerIndex, new GameScreen());
        }

        void SoundMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.SoundEnabled = !Settings.SoundEnabled;
            if (sender is MenuItem item)
                item.Text = SoundString;
        }

        void OptionsMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            ScreenManager?.AddScreen(new GameOptionsMenuScreen(), e.PlayerIndex);
        }

        void RenderingMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            ScreenManager?.AddScreen(new RenderingMenuScreen(), e.PlayerIndex);
        }

        void EmulationMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            ScreenManager?.AddScreen(new EmulationMenuScreen(), e.PlayerIndex);
        }

        void QuitMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            const string message = "Are you sure you want to quit?\n\n";
            MessageBoxScreen confirmExitMessageBox = new MessageBoxScreen(message);
            confirmExitMessageBox.Accepted += ConfirmExitMessageBoxAccepted;
            ScreenManager?.AddScreen(confirmExitMessageBox, null);
        }

        /// <summary>
        /// Event handler for when the user selects ok on the "are you sure
        /// you want to exit" message box.
        /// </summary>
        void ConfirmExitMessageBoxAccepted(object? sender, PlayerIndexEventArgs e)
        {
            ScreenManager?.Game.Exit();
        }
    }
}
