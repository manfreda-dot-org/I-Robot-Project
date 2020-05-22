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
    /// Represents the program ROMs on the I, Robot PCB
    /// There are 40K of fixed ROM and 6 x 8k banks of ROM
    /// </summary>
    [Serializable]
    unsafe public class ProgROM : Machine.Subsystem
    {
        public const int NUM_BANKS = 6;

        // there are 6 ROM banks from 4000 to 5FFF
        public readonly PinnedBuffer<byte>[] Bank_4000 = new PinnedBuffer<byte>[NUM_BANKS] {
            new PinnedBuffer<byte>(0x2000),
            new PinnedBuffer<byte>(0x2000),
            new PinnedBuffer<byte>(0x2000),
            new PinnedBuffer<byte>(0x2000),
            new PinnedBuffer<byte>(0x2000),
            new PinnedBuffer<byte>(0x2000) };

        // 6000 - FFFF
        readonly PinnedBuffer<byte> ROM_6000 = new PinnedBuffer<byte>(0x10000 - 0x6000);

        byte mBankSelect = 0xFF;

        public ProgROM(Machine machine) : base(machine, "Program ROM")
        {
            ROM? r405 = machine.Roms["136029-405"];
            ROM? r206 = machine.Roms["136029-206"];
            ROM? r207 = machine.Roms["136029-207"];
            ROM? r208 = machine.Roms["136029-208"];
            ROM? r209 = machine.Roms["136029-209"];
            ROM? r210 = machine.Roms["136029-210"];

            if (r405 != null)
            {
                Array.Copy(r405, 0x0000, Bank_4000[0], 0x0000, 0x2000);
                Array.Copy(r405, 0x2000, Bank_4000[1], 0x0000, 0x2000);
            }
            if (r206 != null)
            {
                Array.Copy(r206, 0x0000, Bank_4000[2], 0x0000, 0x2000);
                Array.Copy(r206, 0x2000, Bank_4000[3], 0x0000, 0x2000);
            }
            if (r207 != null)
            {
                Array.Copy(r207, 0x0000, Bank_4000[4], 0x0000, 0x2000);
                Array.Copy(r207, 0x2000, Bank_4000[5], 0x0000, 0x2000);
            }
            if (r208 != null)
                Array.Copy(r208, 0, ROM_6000, 0x0000, r208.Data.Length);
            if (r209 != null)
                Array.Copy(r209, 0, ROM_6000, 0x2000, r209.Data.Length);
            if (r210 != null)
                Array.Copy(r210, 0, ROM_6000, 0x6000, r210.Data.Length);
        }

        public override void Dispose()
        {
        }

        /// <summary>
        /// Resets the program ROM
        /// </summary>
        public override void Reset()
        {
            // banked reads go nowhere for now
            Machine.M6809E.SetPageIO(0x40, 0x5F, M6809E.pNullPage, M6809E.pNullPage);

            // setup fixed ROM reads 6000-FFFF
            Machine.M6809E.SetPageIO(0x60, 0xFF, ROM_6000, M6809E.pNullPage);

            // setup the proper bank at 4000-5FFF
            mBankSelect = 0xFF;
            BankSelect = 0;
        }

        /// <summary>
        /// Selects one of the 6 ROM banks from 4000 - 5FFF
        /// </summary>
        public byte BankSelect
        {
            get { return mBankSelect; }
            set
            {
                value &= 7;
                if (mBankSelect != value)
                {
                    mBankSelect = value;

                    // setup M6809 page read pointers
                    if (value < Bank_4000.Length)
                        Machine.M6809E.SetPageIO(0x40, 0x5F, Bank_4000[value], M6809E.pNullPage);
                    else
                        Machine.M6809E.SetPageIO(0x40, 0x5F, M6809E.pNullPage, M6809E.pNullPage);
                }
            }
        }

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("BankSelect", BankSelect);
            info.AddValue("Bank_4000[0]", Bank_4000[0]);
            info.AddValue("Bank_4000[1]", Bank_4000[1]);
            info.AddValue("Bank_4000[2]", Bank_4000[2]);
            info.AddValue("Bank_4000[3]", Bank_4000[3]);
            info.AddValue("Bank_4000[4]", Bank_4000[4]);
            info.AddValue("Bank_4000[5]", Bank_4000[5]);
            info.AddValue("ROM_6000", ROM_6000);
        }
    }
}