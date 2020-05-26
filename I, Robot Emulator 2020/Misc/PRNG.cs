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
using System.Text;

namespace I_Robot
{
    /// <summary>
    /// Pseudo-Random Number Generator
    /// Nothing special, other than it ensures the same playback sequence every time
    /// Useful for procedural generation stuff
    /// </summary>
    public class PRNG
    {
        UInt16 Seed = 0xB00B;

        public PRNG(UInt16 seed)
        {
            Seed = seed;
        }

        public UInt16 Next16 => (UInt16)((Next8 << 8) + Next8);

        public byte Next8
        {
            get
            {
                Seed *= 64013;
                Seed += 5639;
                return (byte)(Seed >> 8);
            }
        }

        public double NextDouble => Next16 / 65535d;
    }
}