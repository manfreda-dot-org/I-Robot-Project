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
    public static class Extensions
    {
        public static string HexString(this byte b) { return b.ToString("X2"); }
        public static string HexString(this sbyte b) { return b.ToString("X2"); }
        public static string HexString(this UInt16 w) { return w.ToString("X4"); }
        public static string HexString(this Int16 w) { return w.ToString("X4"); }
        public static string HexString(this UInt32 d) { return d.ToString("X8"); }
        public static string HexString(this Int32 d) { return d.ToString("X8"); }


        public static uint ToUint(this System.Windows.Media.Color c)
        {
            return (0xFF000000 | (uint)((c.R << 16) | (c.G << 8) | (c.B << 0)));
        }

        public static System.Windows.Media.Color ToColor(uint value)
        {
            return System.Windows.Media.Color.FromArgb((byte)(value >> 24), (byte)(value >> 16), (byte)(value >> 8), (byte)(value >> 0));
        }
    }
}
