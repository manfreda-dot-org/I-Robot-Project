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
using System;

namespace I_Robot
{
    public static class Extensions
    {
        public static Vector3 ToNormal(this Mathbox.Vector3 v)
        {
            Vector3 n = ToVector(v);
            n.Normalize();
            return n;
        }

        public static Vector3 ToVector(this Mathbox.Vector3 v)
        {
            return new Vector3(v.X, v.Y, v.Z);
        }

        public static SharpDX.Matrix3x3 ToMatrix3(this Mathbox.Matrix m)
        {
            const float scale = 1.0f / 0x4000;
            return new SharpDX.Matrix3x3(
                m.M11 * scale, m.M12 * scale, m.M13 * scale,
                m.M21 * scale, m.M22 * scale, m.M23 * scale,
                m.M31 * scale, m.M32 * scale, m.M33 * scale);
        }

        public static Matrix ToMatrix4(this Mathbox.Matrix m)
        {
            const float scale = 1.0f / 0x4000;
            return new Matrix(
                m.M11 * scale, m.M12 * scale, m.M13 * scale, 0,
                m.M21 * scale, m.M22 * scale, m.M23 * scale, 0,
                m.M31 * scale, m.M32 * scale, m.M33 * scale, 0,
                0, 0, 0, 1);
        }

        public static string HexString(this byte b) { return b.ToString("X2"); }
        public static string HexString(this sbyte b) { return b.ToString("X2"); }
        public static string HexString(this UInt16 w) { return w.ToString("X4"); }
        public static string HexString(this Int16 w) { return w.ToString("X4"); }
        public static string HexString(this UInt32 d) { return d.ToString("X8"); }
        public static string HexString(this Int32 d) { return d.ToString("X8"); }
    }
}
