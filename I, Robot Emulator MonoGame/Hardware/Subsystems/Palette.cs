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
using System.Runtime.Serialization;
using System.Text;
using System.Windows.Media;

namespace I_Robot
{
    /// <summary>
    /// Represents the 64 x 9-bit color palette on the I, Robot video PCB
    /// </summary>
    [Serializable]
    unsafe public class Palette : Hardware.Subsystem
    {
        public const int NUM_COLORS = 64;

        public readonly Color[] Color = new Color[NUM_COLORS];
        readonly M6809E.WriteDelegate Write18xx;

        public Palette(Hardware hardware) : base(hardware, "Palette")
        {
            Write18xx = new M6809E.WriteDelegate((UInt16 address, byte data) =>
            {
                // color RAM area
                // 64 9-bit entries
                //
                // address --------:-aaaaaai
                // data    rrggbbii
                // NOTE: 3 bits of intensity, 2 from data and 1 from address
                //
                // Address bit 0 = LSB of intensity
                // Address bit 1-6 = color table index
                // Address bit 7 = unused

                data = (byte)(~data);

                int index = (address >> 1) & 0x3F;

                byte i = (byte)((((data & 0x03) << 1) + ((byte)(~address) & 1) + 1) * 8);
                byte r = (byte)(((data >> 6) & 3) * i);
                byte g = (byte)(((data >> 4) & 3) * i);
                byte b = (byte)(((data >> 2) & 3) * i);
                Color[index] = System.Windows.Media.Color.FromRgb(r, g, b);
            });
        }

        public override void Dispose()
        {
        }

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            uint[] array = new uint[Color.Length];
            for (int n = 0; n < array.Length; n++)
                array[n] = Color[n].ToUint();
            info.AddValue("PALETTE", array);
        }

        public override void Reset()
        {
            Hardware.M6809E.SetPageIO(0x18, M6809E.pNullPage, Write18xx);
        }
    }
}