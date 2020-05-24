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
using Microsoft.Xna.Framework.Graphics;

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
        Emulation.Machine? mMachine;

        GraphicsDeviceManager Graphics;
        public ScreenManager? ScreenManager;
        ScreenFactory ScreenFactory;

        /// <summary>
        /// The main game constructor.
        /// </summary>
        public Game()
        {
            TargetElapsedTime = TimeSpan.FromTicks(333333);
            Content.RootDirectory = "Content";
            Graphics = new GraphicsDeviceManager(this)
            {
                GraphicsProfile = GraphicsProfile.HiDef,
                PreferMultiSampling = true,
            };

            // Create the screen factory and add it to the Services
            ScreenFactory = new ScreenFactory();
            Services.AddService(typeof(IScreenFactory), ScreenFactory);
        }

        public Emulation.Machine Machine
        {
            get
            {
                if (mMachine == null)
                    throw new Exception();
                return mMachine;
            }
        }

        protected override void Initialize()
        {
            GraphicsDevice.PresentationParameters.MultiSampleCount = 8;
            Graphics.PreferredBackBufferWidth = GraphicsDevice.DisplayMode.Width;
            Graphics.PreferredBackBufferHeight = GraphicsDevice.DisplayMode.Height - 90;
            //Graphics.PreferredBackBufferWidth = Emulation.Machine.NATIVE_RESOLUTION.Width * 3; 
            //Graphics.PreferredBackBufferHeight = (int)(Graphics.PreferredBackBufferWidth / Emulation.Machine.MonitorAspectRatio);
            //   Graphics.IsFullScreen = true;
            Graphics.ApplyChanges();

            base.Initialize();
        }

        protected override void LoadContent()
        {
            base.LoadContent();

            // Create the screen manager component.
            ScreenManager = new ScreenManager(this);
            Components.Add(ScreenManager);

            // read the ROMs
            if (RomSet.ReadRomSetFromZipArchive("irobot.zip", out RomSet? roms, out string? errMsg) && roms != null)
            {
                // now create the hardware
                mMachine = new Emulation.Machine(roms, new MathboxRenderer.Factory(ScreenManager));
            }

            LoadingScreen.Load(ScreenManager, true, null, new GameScreen(ScreenManager));

            if (errMsg != null)
            {
                string message = $"ROMSET    MISSING\n\n{errMsg}";
                MessageBoxScreen dialog = new MessageBoxScreen(ScreenManager, message, false);
                ScreenManager.AddScreen(dialog, null);
            }
        }

        protected override void UnloadContent()
        {
            Machine.Dispose();
            base.UnloadContent();
        }

        /// <summary>
        /// This is called when the game should draw itself.
        /// </summary>
        protected override void Draw(GameTime gameTime)
        {
            // Graphics.GraphicsDevice.Clear(Color.Black);

            // The real drawing happens inside the screen manager component.
            base.Draw(gameTime);
        }

        protected override void OnExiting(object sender, EventArgs args)
        {
            Machine.Paused = true;
            base.OnExiting(sender, args);
        }
    }
}