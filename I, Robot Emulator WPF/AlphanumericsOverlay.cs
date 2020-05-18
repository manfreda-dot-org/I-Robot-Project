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

using SharpDX;
using SharpDX.Direct2D1;
using SharpDX.Direct3D11;
using SharpDX.Direct3D9;
using SharpDX.DirectWrite;
using SharpDX.DXGI;
using SharpDX.Mathematics.Interop;
using System;
using System.CodeDom;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace I_Robot
{
    class AlphanumericsOverlay : Direct2D.WpfControl
    {
        static readonly int TotalColors = Alphanumerics.NUM_COLORS * Alphanumerics.NUM_PALETTES;
        static readonly Size2 BitmapSize = new Size2(Alphanumerics.CHAR_WIDTH * TotalColors, Alphanumerics.NUM_CHARS * Alphanumerics.CHAR_HEIGHT);
        readonly uint[] PixelData;
        Hardware? mHardware;

        public AlphanumericsOverlay()
        {
            PixelData = new uint[BitmapSize.Width * BitmapSize.Height];   
        }

        Bitmap CreateCharacterBitmap(RenderTarget renderTarget)
        {
            return Bitmap.New<uint>(renderTarget, BitmapSize, PixelData, new BitmapProperties(renderTarget.PixelFormat));
        }

        public Hardware? Hardware
        {
            get => mHardware;
            set
            {
                if (mHardware != value)
                {
                    mHardware = value;
                    if (value == null)
                        return;

                    UInt32 index = 0;
                    foreach (var character in value.Alphanumerics.CharacterSet)
                    {
                        foreach (BYTE row in character)
                        {
                            foreach (var palette in value.Alphanumerics.PaletteTable)
                                foreach (var color in palette)
                                {
                                    uint pixel = color.ToUint();
                                    PixelData[index++] = (row.BIT_7 ? pixel : 0);
                                    PixelData[index++] = (row.BIT_6 ? pixel : 0);
                                    PixelData[index++] = (row.BIT_5 ? pixel : 0);
                                    PixelData[index++] = (row.BIT_4 ? pixel : 0);
                                    PixelData[index++] = (row.BIT_3 ? pixel : 0);
                                    PixelData[index++] = (row.BIT_2 ? pixel : 0);
                                    PixelData[index++] = (row.BIT_1 ? pixel : 0);
                                    PixelData[index++] = (row.BIT_0 ? pixel : 0);
                                }
                        }
                    }

                    ResourceCache.Add("CharacterBitmap", t => CreateCharacterBitmap(t));
                }
            }
        }

        protected override void Render(RenderTarget target)
        {
            // fill target with transparent color
            target.Clear(new RawColor4(0.0f, 0.0f, 0.0f, 0.0f));

            if (Hardware == null)
                return;

            if (ResourceCache["CharacterBitmap"] is Bitmap bitmap)
            {
                // scale overlay to fit screen
                double scale_x = this.ActualWidth / Alphanumerics.COLUMNS;
                double scale_y = this.ActualHeight / Alphanumerics.VISIBLE_ROWS;
                float scale = (float)Math.Min(scale_x, scale_y);
                scale = 8;

                int index = 0;
                RawRectangleF src = new RawRectangleF(0, 0, Alphanumerics.CHAR_WIDTH, Alphanumerics.CHAR_HEIGHT);
                RawRectangleF dst = new RawRectangleF(0, 0, Alphanumerics.CHAR_WIDTH, Alphanumerics.CHAR_HEIGHT);
                float left = !Hardware.Alphanumerics.ALPHA_MAP ? 0 : 4 * Alphanumerics.CHAR_WIDTH;
                for (int y = 0; y < Alphanumerics.VISIBLE_ROWS; y++)
                {
                    dst.Top = y * scale;
                    dst.Bottom = (y + 1) * scale;
                    for (int x = 0; x < Alphanumerics.COLUMNS; x++)
                    {
                        byte c = Hardware.Alphanumerics.RAM[index++];
                        int character = c & 63;
                        if (character != 0)
                        {
                            dst.Left = x * scale;
                            dst.Right = (x + 1) * scale;

                            src.Top = (c & 63) * Alphanumerics.CHAR_HEIGHT;
                            src.Bottom = src.Top + Alphanumerics.CHAR_HEIGHT;
                            src.Left = left + (c >> 6) * Alphanumerics.CHAR_WIDTH;
                            src.Right = src.Left + Alphanumerics.CHAR_WIDTH;
                            target.DrawBitmap(bitmap, dst, 1.0f, BitmapInterpolationMode.Linear, src);
                        }
                    }
                }
            }
        }
    }

}
