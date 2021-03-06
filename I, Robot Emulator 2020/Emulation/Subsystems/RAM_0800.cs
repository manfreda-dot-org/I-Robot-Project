﻿// Copyright 2020 by John Manfreda. All Rights Reserved.
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
    /// Represents the 3 x 2k banks of memory from address 0800 - 0FFF
    /// </summary>
    [Serializable]
    unsafe public class RAM_0800 : Machine.Subsystem
    {
        public const int NUM_BANKS = 3;

        public readonly PinnedBuffer<byte>[] Bank = new PinnedBuffer<byte>[NUM_BANKS] {
            new PinnedBuffer<byte>(0x800),
            new PinnedBuffer<byte>(0x800),
            new PinnedBuffer<byte>(0x800) };

        byte mBankSelect = 0xFF;

        public RAM_0800(Machine machine) : base(machine, "RAM 0800 - 0FFF")
        {
        }

        public override void Dispose()
        {
        }

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("BankSelect", BankSelect);
            info.AddValue("RAM_0_0800", Bank[0]);
            info.AddValue("RAM_1_0800", Bank[1]);
            info.AddValue("RAM_2_0800", Bank[2]);
        }

        public override void Reset()
        {
            Machine.M6809E.SetPageIO(0x08, 0x0F, M6809E.pNullPage, M6809E.pNullPage);

            mBankSelect = 0xFF;
            BankSelect = 0;
        }

        /// <summary>
        /// Gets/sets the currently selected bank number
        /// </summary>
        public byte BankSelect
        {
            get { return mBankSelect; }
            set
            {
                value &= 3;
                if (mBankSelect != value)
                {
                    mBankSelect = value;
                    //EmulatorTrace($"Bank 0800 = {value}");

                    // setup M6809 page read/write pointers
                    if (value < Bank.Length)
                        Machine.M6809E.SetPageIO(0x08, 0x0F, Bank[value], Bank[value]);
                    else
                        Machine.M6809E.SetPageIO(0x08, 0x0F, M6809E.pNullPage, M6809E.pNullPage);
                }
            }
        }
    }
}