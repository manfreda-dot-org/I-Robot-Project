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
using System.Collections.Generic;
using System.IO;
using System.Runtime.Serialization;
using System.Text;
using System.Windows.Media;

namespace I_Robot
{
    /// <summary>
    /// Simulates the 32x32 alphanumerics video overlay
    /// 1K of ram is emulated, but only the first 29 lines are visible
    /// </summary>
    [Serializable]
    unsafe public class Alphanumerics : Hardware.Subsystem
    {
        public const int NUM_CHARS = 64;
        public const int NUM_PALETTES = 2;
        public const int NUM_COLORS = 4;

        public const int CHAR_WIDTH = 8;
        public const int CHAR_HEIGHT = 8;
        public const int COLUMNS = 32;
        public const int ROWS = 32;
        public const int VISIBLE_ROWS = 29;

        /// <summary>
        /// Character set: 64 characters, 8x8 pixels
        /// </summary>
        public readonly byte[][] CharacterSet = new byte[NUM_CHARS][];

        /// <summary>
        /// Holds the Alphanumerics color ROM palette table
        /// </summary>
        public readonly Color[][] PaletteTable = new Color[NUM_PALETTES][];

        /// <summary>
        /// 1K of character RAM
        /// </summary>
        public readonly PinnedBuffer<byte> RAM;

        bool mALPHA_MAP = false;

        private Alphanumerics(Hardware hardware, PinnedBuffer<byte> ram) : base(hardware, "Alphanumerics")
        {
            RAM = ram;

            // load the character ROM
            ROM? rom124 = Hardware.Roms["136029-124"];
            if (rom124 != null)
            {
                // unpack the ROM into character scanlines
                int index = 0;
                for (int alpha = 0; alpha < NUM_CHARS; alpha++)
                {
                    CharacterSet[alpha] = new byte[CHAR_HEIGHT];
                    for (int row = 0; row < CHAR_HEIGHT; row++)
                    {
                        byte bits = (byte)(rom124[index++] << 4);
                        bits |= (byte)(rom124[index++] & 0xF);
                        CharacterSet[alpha][row] = bits;
                    }
                }
            }

            // load the color ROM
            ROM? rom125 = Hardware.Roms["136029-125"];
            if (rom125 != null)
            {
                for (int p = 0; p < NUM_PALETTES; p++)
                {
                    PaletteTable[p] = new Color[NUM_COLORS];
                    for (int color = 0; color < NUM_COLORS; color++)
                    {
                        byte rrggbbii = rom125[p * 16 + color + 4];
                        int i = (rrggbbii & 3) + 1; // 1-4
                        int r = (rrggbbii >> 6) & 3; // 0-3
                        int g = (rrggbbii >> 4) & 3; // 0-3
                        int b = (rrggbbii >> 2) & 3; // 0-3

                        // color = 255 * (r / 3) * (i / 4)
                        r = Math.Min(255 * r * i / 12, 255);
                        g = Math.Min(255 * g * i / 12, 255);
                        b = Math.Min(255 * b * i / 12, 255);

                        PaletteTable[p][color] = System.Windows.Media.Color.FromRgb((byte)r, (byte)g, (byte)b);
                    }
                }
            }
        }

        public Alphanumerics(Hardware hardware) : this(hardware, new PinnedBuffer<byte>(0x400))
        {
        }

        public override void Dispose()
        {
        }

        public override void Reset()
        {
            Hardware.M6809E.SetPageIO(0x1C, 0x1F, RAM, RAM);

            ALPHA_MAP = false;
        }

        /// <summary>
        /// ALPHA_MAP hardware signal
        /// </summary>
        public bool ALPHA_MAP;

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("ALPHA_MAP", mALPHA_MAP);
            info.AddValue("ALPHA_RAM", RAM);
        }

        #region DEBUG

        // these variables are used to help speed up creation of debug text
        static readonly string DebugCharset = " ABCDEFGHIJKLMNOPQRSTUVWXYZ-{}.:░▒▓█v¾ ⅔ ?      0123456789©½¼⅓  ";
        readonly StringBuilder sb = new StringBuilder(ROWS * COLUMNS);
        byte[] TextBuf = new Byte[VISIBLE_ROWS * COLUMNS];
        string LastString = "";

        public override string ToString()
        {
            byte* p = RAM;

            sb.Clear();

            int different = 0;
            int index = 0;
            for (int r = 0; r < VISIBLE_ROWS; r++)
            {
                if (r > 0)
                    sb.Append("\n");

                for (int c = 0; c < COLUMNS; c++, index++)
                {
                    byte ch = (byte)((*p++) & 0x3F);
                    different += (ch ^ TextBuf[index]);
                    TextBuf[index] = ch;

                    sb.Append(DebugCharset[ch]);
                }
            }

            if (different != 0)
                LastString = sb.ToString();
            return LastString;
        }
        #endregion
    }
}