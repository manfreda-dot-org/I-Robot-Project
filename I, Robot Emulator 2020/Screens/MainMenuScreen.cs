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
        public MainMenuScreen(ScreenManager screenManager)
            : base(screenManager, "MAIN    MENU")
        {
            // Create our menu entries.
            MenuItems.Add(new MenuItem("RETURN    TO    GAME", OnCancel));
            MenuItems.Add(new MenuItem(SoundString, SoundMenuSelected, 1f));
            MenuItems.Add(new MenuItem("GAME    OPTIONS", OptionsMenuSelected, 0.5f));
            MenuItems.Add(new MenuItem("RENDERING", RenderingMenuSelected, 0.5f));
            MenuItems.Add(new MenuItem("EMULATION", EmulationMenuSelected, 0.5f));
            MenuItems.Add(new MenuItem("CHEATS", CheatsMenuSelected, 0.5f));
            MenuItems.Add(new MenuItem("RESET    GAME", OnResetMachine, 1));
            MenuItems.Add(new MenuItem("QUIT", QuitMenuSelected, 1));
        }

        public override void Draw(GameTime gameTime)
        {
            base.Draw(gameTime);
            
            SpriteBatch.Begin();
            SpriteBatch.DrawString(
                MenuFont, 
                $"v1.02   Copyright 2020 John Manfreda    lordfrito@manfreda.org", 
                new Vector2(50, ScreenManager.GraphicsDevice.Viewport.Height - MenuFont.LineSpacing), 
                Color.CornflowerBlue, 
                0, 
                Vector2.Zero, 
                0.3F, 
                Microsoft.Xna.Framework.Graphics.SpriteEffects.None, 0);
            SpriteBatch.End();
        }

        string SoundString => $"SOUND:    {(Settings.SoundEnabled ? "ENABLED" : "DISABLED")}";

        void SoundMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.SoundEnabled = !Settings.SoundEnabled;
            if (sender is MenuItem item)
                item.Text = SoundString;
        }

        void OptionsMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            ScreenManager.AddScreen(new GameOptionsMenuScreen(ScreenManager), e.PlayerIndex);
        }

        void RenderingMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            ScreenManager.AddScreen(new RenderingMenuScreen(ScreenManager), e.PlayerIndex);
        }

        void EmulationMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            ScreenManager.AddScreen(new EmulationMenuScreen(ScreenManager), e.PlayerIndex);
        }

        void CheatsMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            ScreenManager.AddScreen(new CheatsScreen(ScreenManager), e.PlayerIndex);
        }


        void OnResetMachine(object? sender, PlayerIndexEventArgs e)
        {
            const string message = "Are you sure you want to quit?\n\n";
            MessageBoxScreen dialog = new MessageBoxScreen(ScreenManager, message);
            dialog.Accepted += new System.EventHandler<PlayerIndexEventArgs>((object? sender, PlayerIndexEventArgs e) =>
            {
                if (this.Game is I_Robot.Game game)
                    game.Machine.Reset(Emulation.Machine.RESET_TYPE.USER);
                OnCancel(sender, e);
            });
            ScreenManager.AddScreen(dialog, null);
        }

        void QuitMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            const string message = "Are you sure you want to quit?\n\n";
            MessageBoxScreen dialog = new MessageBoxScreen(ScreenManager, message);
            dialog.Accepted += new System.EventHandler<PlayerIndexEventArgs>((object? sender, PlayerIndexEventArgs e) => { Game.Exit(); });
            ScreenManager.AddScreen(dialog, null);
        }
    }
}