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
    public struct BYTE
    {
        public byte Value;
        public BYTE(byte value) { Value = value; }

        public bool BIT_7
        {
            get { return (Value & 0x80) != 0; }
            set { if (value) Value |= 0x80; else Value &= 0x7F; }
        }

        public bool BIT_6
        {
            get { return (Value & 0x40) != 0; }
            set { if (value) Value |= 0x40; else Value &= 0xBF; }
        }

        public bool BIT_5
        {
            get { return (Value & 0x20) != 0; }
            set { if (value) Value |= 0x20; else Value &= 0xDF; }
        }

        public bool BIT_4
        {
            get { return (Value & 0x10) != 0; }
            set { if (value) Value |= 0x10; else Value &= 0xEF; }
        }

        public bool BIT_3
        {
            get { return (Value & 0x08) != 0; }
            set { if (value) Value |= 0x08; else Value &= 0xF7; }
        }

        public bool BIT_2
        {
            get { return (Value & 0x04) != 0; }
            set { if (value) Value |= 0x04; else Value &= 0xFB; }
        }

        public bool BIT_1
        {
            get { return (Value & 0x02) != 0; }
            set { if (value) Value |= 0x02; else Value &= 0xFD; }
        }

        public bool BIT_0
        {
            get { return (Value & 0x01) != 0; }
            set { if (value) Value |= 0x01; else Value &= 0xFE; }
        }

        public static implicit operator byte(BYTE r) => r.Value;
        public static implicit operator BYTE(byte b) => new BYTE(b);
        public override string ToString() { return Value.HexString(); }
    }
}
