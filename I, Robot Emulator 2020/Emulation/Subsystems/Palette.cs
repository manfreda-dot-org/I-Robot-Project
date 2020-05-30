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
using System;
using System.Runtime.Serialization;

namespace I_Robot.Emulation
{
    /// <summary>
    /// Represents the 64 x 9-bit color palette on the I, Robot video PCB
    /// </summary>
    [Serializable]
    unsafe public class Palette : Machine.Subsystem
    {
        public const int NUM_COLORS = 64;

        public readonly Color[] Color = new Color[NUM_COLORS];
        readonly M6809E.WriteDelegate Write18xx;

        public Palette(Machine machine) : base(machine, "Palette")
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
                //
                // NOTE: data and address are written inverted

                data = (byte)(~data);

                // color = 255 * (rgb / 3) * (i / 7)
                float i = (((data & 0x03) << 1) + ((~address) & 1));
                byte r = (byte)(255 * ((data >> 6) & 3) * i / 21);
                byte g = (byte)(255 * ((data >> 4) & 3) * i / 21);
                byte b = (byte)(255 * ((data >> 2) & 3) * i / 21);

                Color[(address >> 1) & 0x3F] = new Color(r, g, b);
            });
        }

        public override void Dispose()
        {
        }

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            uint[] array = new uint[Color.Length];
            for (int n = 0; n < array.Length; n++)
                array[n] = Color[n].PackedValue;
            info.AddValue("PALETTE", array);
        }

        public override void Reset()
        {
            Machine.M6809E.SetPageIO(0x18, M6809E.pNullPage, Write18xx);
        }
    }
}