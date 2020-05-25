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
using System.Runtime.Serialization;

namespace I_Robot.Emulation
{
    /// <summary>
    /// Represents the 2k of memory from address 0000 - 07FF
    /// </summary>
    [Serializable]
    unsafe public class RAM_0000 : Machine.Subsystem
    {
        public readonly PinnedBuffer<byte> RAM = new PinnedBuffer<byte>(0x800);

        public RAM_0000(Machine machine) : base(machine, "RAM 0000 - 07FF")
        {
        }

        public byte this[int index]
        {
            get => RAM[index];
            set => RAM[index] = value;
        }

        public override void Dispose()
        {
        }

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("RAM_0000", RAM);
        }

        public override void Reset()
        {
            // setup M6809 page read/write pointers
            Machine.M6809E.SetPageIO(0x00, 0x07, RAM, RAM);
        }
    }
}