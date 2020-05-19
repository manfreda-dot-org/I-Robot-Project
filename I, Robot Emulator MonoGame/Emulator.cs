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
using SharpDX;
using System;
using System.IO;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using Color = Microsoft.Xna.Framework.Color;
using Rectangle = Microsoft.Xna.Framework.Rectangle;

namespace I_Robot
{
    /// <summary>
    /// This is the main type for your game.
    /// </summary>
    public class Emulator : Game
    {

        Hardware? Hardware;
        AlphanumericsRenderer? AlphanumericsOverlay;

        GraphicsDeviceManager graphics;
        SpriteBatch? spriteBatch;

        public Emulator()
        {
            graphics = new GraphicsDeviceManager(this);

            // make the game window an integer multiple of the default resolution
            int scaleX = GraphicsAdapter.DefaultAdapter.CurrentDisplayMode.Width / Hardware.NATIVE_RESOLUTION.Width;
            int scaleY = GraphicsAdapter.DefaultAdapter.CurrentDisplayMode.Height / Hardware.NATIVE_RESOLUTION.Height;
            int scale = Math.Min(scaleX, scaleY);
            graphics.PreferredBackBufferWidth = scale * Hardware.NATIVE_RESOLUTION.Width;
            graphics.PreferredBackBufferHeight = scale * Hardware.NATIVE_RESOLUTION.Height;
            //graphics.IsFullScreen = true; // set true to default later
            graphics.ApplyChanges();

            Content.RootDirectory = "Content";

            Window.Title = "I, Emulator";
        }

        /// <summary>
        /// Allows the game to perform any initialization it needs to before starting to run.
        /// This is where it can query for any required services and load any non-graphic
        /// related content.  Calling base.Initialize will enumerate through any components
        /// and initialize them as well.
        /// </summary>
        protected override void Initialize()
        {
            // TODO: Add your initialization logic here

            base.Initialize();
        }

        /// <summary>
        /// LoadContent will be called once per game and is the place to load all of your content.
        /// </summary>
        protected override void LoadContent()
        {
            // Create a new SpriteBatch, which can be used to draw textures.
            spriteBatch = new SpriteBatch(GraphicsDevice);

            // read the ROMs
            if (RomSet.ReadRomSetFromZipArchive("irobot.zip", out RomSet? roms, out string errMessage) && roms != null)
            {
                // create hardware that uses the ROMs
                Hardware = new Hardware(roms);
                //                AlphanumericsOverlay.Hardware = Hardware;

                // start our timer
                //                DispatcherTimer timer = new DispatcherTimer();
                //                timer.Interval = TimeSpan.FromMilliseconds(33);
                //                timer.Tick += timer_Tick;
                //                timer.Start();

                AlphanumericsOverlay = new AlphanumericsRenderer(Hardware, GraphicsDevice);
            }
            else
            {               
                System.Windows.MessageBox.Show(errMessage, "ROM loader");
            }
        }

        /// <summary>
        /// UnloadContent will be called once per game and is the place to unload game-specific content.
        /// </summary>
        protected override void UnloadContent()
        {
            // TODO: Unload any non ContentManager content here
            AlphanumericsOverlay?.Dispose();
        }

        /// <summary>
        /// Allows the game to run logic such as updating the world,
        /// checking for collisions, gathering input, and playing audio.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Update(GameTime gameTime)
        {
            Keyboard.GetState();

            if (GamePad.GetState(PlayerIndex.One).Buttons.Back == ButtonState.Pressed || Keyboard.IsPressed(Keys.Escape))
                Exit();
            if (Keyboard.HasBeenPressed(Keys.Tab))
                Settings.TestSwitch = !Settings.TestSwitch;
            if (Keyboard.HasBeenPressed(Keys.F3)) 
                Hardware?.Reset(Hardware.RESET_TYPE.USER);
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


            // TODO: Add your update logic here

            base.Update(gameTime);
        }

        /// <summary>
        /// This is called when the game should draw itself.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Draw(GameTime gameTime)
        {
            GraphicsDevice.Clear(Color.CornflowerBlue);

            // TODO: Add your drawing code here
            AlphanumericsOverlay?.Draw(GraphicsDevice);

            base.Draw(gameTime);
        }
    }
}
