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

using I_Robot.Emulation;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using System;

namespace I_Robot
{
    /// <summary>
    /// Renders the alphanumerics overlay
    /// </summary>
    class AlphanumericsRenderer : IDisposable
    {
        readonly Screen Screen;
        readonly Machine Machine;

        /// <summary>
        /// We render characters onto this overlay
        /// </summary>
        readonly RenderTarget2D Overlay;

        public Texture2D Texture => Overlay;

        /// <summary>
        /// Texture containing the alphanumerics characters
        /// </summary>
        readonly Texture2D CharacterSet;

        public AlphanumericsRenderer(Screen screen)
        {
            Screen = screen;
            Machine = screen.Machine;

            // create the character set texture, and fill with game font
            CharacterSet = new Texture2D(screen.Game.GraphicsDevice, Alphanumerics.CHAR_WIDTH, Alphanumerics.CHAR_HEIGHT * Alphanumerics.NUM_CHARS);
            CharacterSet.SetData(PixelMap);

            // create an overlay for us to render onto
            Overlay = new RenderTarget2D(
                screen.GraphicsDevice,
                Machine.NATIVE_RESOLUTION.Width,
                Machine.NATIVE_RESOLUTION.Height);
        }

        public void Dispose()
        {
            Overlay.Dispose();
            CharacterSet.Dispose();
        }

        /// <summary>
        /// Gets a raw pixel map of alphanumerics characters extracted from the Alphanumerics hardware
        /// </summary>
        Color[] PixelMap
        {
            get
            {
                // create a pixel buffer as the source for our character map texture
                Color[] pixels = new Color[CharacterSet.Width * CharacterSet.Height];
                UInt32 index = 0;
                foreach (var character in Machine.Alphanumerics.CharacterSet)
                {
                    foreach (BYTE row in character)
                    {
                        pixels[index++] = (row.BIT_7 ? Color.White : Color.Transparent);
                        pixels[index++] = (row.BIT_6 ? Color.White : Color.Transparent);
                        pixels[index++] = (row.BIT_5 ? Color.White : Color.Transparent);
                        pixels[index++] = (row.BIT_4 ? Color.White : Color.Transparent);
                        pixels[index++] = (row.BIT_3 ? Color.White : Color.Transparent);
                        pixels[index++] = (row.BIT_2 ? Color.White : Color.Transparent);
                        pixels[index++] = (row.BIT_1 ? Color.White : Color.Transparent);
                        pixels[index++] = (row.BIT_0 ? Color.White : Color.Transparent);
                    }
                }
                return pixels;
            }
        }

        /// <summary>
        /// Renders alphanumerics onto the overlay itself in native resolution
        /// </summary>
        /// <param name="graphicsDevice"></param>
        public void Render(GraphicsDevice graphicsDevice)
        {
            // Set the render target
            graphicsDevice.SetRenderTarget(Overlay);
            graphicsDevice.Clear(Color.Transparent);

            Screen.SpriteBatch.Begin();
            Color[] palette = Machine.Alphanumerics.Palette;
            int index = 0;
            Rectangle src = new Rectangle(0, 0, Alphanumerics.CHAR_WIDTH, Alphanumerics.CHAR_HEIGHT);
            Rectangle dst = new Rectangle(0, 0, Alphanumerics.CHAR_WIDTH, Alphanumerics.CHAR_HEIGHT);
            for (dst.Y = 0; dst.Y < Machine.NATIVE_RESOLUTION.Height; dst.Y += Alphanumerics.CHAR_HEIGHT)
            {
                for (dst.X = 0; dst.X < Machine.NATIVE_RESOLUTION.Width; dst.X += Alphanumerics.CHAR_WIDTH)
                {
                    // each byte of alpha ram
                    // xxxxxxxx
                    // ||||||||
                    // || \\\\\\___ character 0 - 63
                    //  \\_________ color index 0 - 3

                    byte _byte = Machine.Alphanumerics.RAM[index++];
                    int ch = _byte & 63;
                    if (ch != 0)
                    {
                        src.Y = ch * Alphanumerics.CHAR_HEIGHT;
                        Screen.SpriteBatch.Draw(CharacterSet, dst, src, palette[_byte >> 6]);
                    }
                }
            }
            Screen.SpriteBatch.End();
        }

        public void Draw(GraphicsDevice graphicsDevice)
        {
            // determine location of overlay
            int dstWidth = graphicsDevice.PresentationParameters.BackBufferWidth;
            int dstHeight = graphicsDevice.PresentationParameters.BackBufferHeight;
            float scale_x = dstWidth / (float)Emulation.Machine.MonitorAspectRatio;
            float scale_y = dstHeight / 1;
            float scale = Math.Min(scale_x, scale_y);
            int w = (int)Math.Round(Emulation.Machine.MonitorAspectRatio * scale);
            int h = (int)Math.Round(1 * scale);

            // draw overlay ontop of screen
            Screen.SpriteBatch.Begin();
            Screen.SpriteBatch.Draw(Texture, new Rectangle((dstWidth - w) / 2, (dstHeight - h) / 2, w, h), null, Color.White);
            Screen.SpriteBatch.End();
        }
    }
}