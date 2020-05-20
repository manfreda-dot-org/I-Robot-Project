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
using GameManagement;
using Microsoft.Xna.Framework;

namespace I_Robot
{
    /// <summary>
    /// Main game class, manages different game states, with transitions
    /// between menu screens, a loading screen, the game itself, and a pause
    /// menu. This main game class is extremely simple: all the interesting
    /// stuff happens in the ScreenManager component.
    /// </summary>
    public class Game : Microsoft.Xna.Framework.Game
    {
        static public Hardware? Hardware;
        static public AlphanumericsRenderer? AlphanumericsOverlay;

        GraphicsDeviceManager Graphics;
        ScreenManager ScreenManager;
        ScreenFactory ScreenFactory;

        /// <summary>
        /// The main game constructor.
        /// </summary>
        public Game()
        {
            Content.RootDirectory = "Content";

            Graphics = new GraphicsDeviceManager(this);
            TargetElapsedTime = TimeSpan.FromTicks(333333);

            // Create the screen factory and add it to the Services
            ScreenFactory = new ScreenFactory();
            Services.AddService(typeof(IScreenFactory), ScreenFactory);

            // Create the screen manager component.
            ScreenManager = new ScreenManager(this);
            Components.Add(ScreenManager);
        }

        protected override void Initialize()
        {
            base.Initialize();

            // read the ROMs
            if (RomSet.ReadRomSetFromZipArchive("irobot.zip", out RomSet? roms, out string errMsg) && roms != null)
            {
                // create hardware that uses the ROMs
                Hardware = new Hardware(roms);
                AlphanumericsOverlay = new AlphanumericsRenderer(Hardware, GraphicsDevice);
            }
            else
            {
                string message = $"UNABLE    TO    CREATE\n\n{errMsg}";
                MessageBoxScreen dialog = new MessageBoxScreen(message);
                ScreenManager?.AddScreen(dialog, null);
            }
        }

        protected override void LoadContent()
        {
            base.LoadContent();

            AddInitialScreens();
        }

        protected override void UnloadContent()
        {
            base.UnloadContent();
            AlphanumericsOverlay?.Dispose();
        }

        private void AddInitialScreens()
        {
#if false
            // Activate the first screens.
            ScreenManager.AddScreen(new BackgroundScreen(), null);

            // We have different menus for Windows Phone to take advantage of the touch interface
            ScreenManager.AddScreen(new MainMenuScreen(), null);
#else
            if (ScreenManager is ScreenManager screenManager)
                LoadingScreen.Load(screenManager, true, null, new GameScreen());
#endif
        }

        /// <summary>
        /// This is called when the game should draw itself.
        /// </summary>
        protected override void Draw(GameTime gameTime)
        {
            Graphics.GraphicsDevice.Clear(Color.Black);

            // The real drawing happens inside the screen manager component.
            base.Draw(gameTime);
        }
    }
}
