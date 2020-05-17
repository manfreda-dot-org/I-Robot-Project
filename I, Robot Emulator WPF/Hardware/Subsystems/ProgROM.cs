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

namespace I_Robot
{
    [Serializable]
    unsafe public class ProgROM : Hardware.Subsystem
    {
        // there are 6 ROM banks from 4000 to 5FFF
        public readonly PinnedBuffer<byte>[] Bank_4000 = new PinnedBuffer<byte>[6] {
            new PinnedBuffer<byte>(0x2000),
            new PinnedBuffer<byte>(0x2000),
            new PinnedBuffer<byte>(0x2000),
            new PinnedBuffer<byte>(0x2000),
            new PinnedBuffer<byte>(0x2000),
            new PinnedBuffer<byte>(0x2000) };

        // 6000 - FFFF
        readonly PinnedBuffer<byte> ROM_6000 = new PinnedBuffer<byte>(0x10000 - 0x6000);

        byte mBankSelect = 0xFF;

        /// <summary>
        /// Indicates whether or not the program ROMs loaded successfully
        /// </summary>
        public bool ProgramLoaded { get; private set; }

        public ProgROM(Hardware hardware) : base(hardware, "Program ROM")
        {
            int count = 0;
            if (ROM.TryLoad("136029-405.bin", 0x4000, 0x150A97, out ROM? r405) && r405 != null)
            {
                count++;
                Array.Copy(r405.Data, 0x0000, Bank_4000[0].ManagedBuffer, 0x0000, 0x2000);
                Array.Copy(r405.Data, 0x2000, Bank_4000[1].ManagedBuffer, 0x0000, 0x2000);
            }
            if (ROM.TryLoad("136029-206.bin", 0x4000, 0x174942, out ROM? r206) && r206 != null)
            {
                count++;
                Array.Copy(r206.Data, 0x0000, Bank_4000[2].ManagedBuffer, 0x0000, 0x2000);
                Array.Copy(r206.Data, 0x2000, Bank_4000[3].ManagedBuffer, 0x0000, 0x2000);
            }
            if (ROM.TryLoad("136029-207.bin", 0x4000, 0x17384C, out ROM? r207) && r207 != null)
            {
                count++;
                Array.Copy(r207.Data, 0x0000, Bank_4000[4].ManagedBuffer, 0x0000, 0x2000);
                Array.Copy(r207.Data, 0x2000, Bank_4000[5].ManagedBuffer, 0x0000, 0x2000);
            }
            if (ROM.TryLoad("136029-208.bin", 0x2000, 0x0D5E26, out ROM? r208) && r208 != null)
            {
                count++;
                Array.Copy(r208.Data, 0, ROM_6000.ManagedBuffer, 0x0000, r208.Data.Length);
            }
            if (ROM.TryLoad("136029-209.bin", 0x4000, 0x1A1B59, out ROM? r209) && r209 != null)
            {
                count++;
                Array.Copy(r209.Data, 0, ROM_6000.ManagedBuffer, 0x2000, r209.Data.Length);
            }
            if (ROM.TryLoad("136029-210.bin", 0x4000, 0x179092, out ROM? r210) && r210 != null)
            {
                count++;
                Array.Copy(r210.Data, 0, ROM_6000.ManagedBuffer, 0x6000, r210.Data.Length);
            }
            ProgramLoaded = (count == 6);
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
            Hardware.M6809E.SetPageIO(0x40, 0x5F, M6809E.pNullPage, M6809E.pNullPage);

            // setup fixed ROM reads 6000-FFFF
            Hardware.M6809E.SetPageIO(0x60, 0xFF, ROM_6000.pData, M6809E.pNullPage);

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
                        Hardware.M6809E.SetPageIO(0x40, 0x5F, Bank_4000[value].pData, M6809E.pNullPage);
                    else
                        Hardware.M6809E.SetPageIO(0x40, 0x5F, M6809E.pNullPage, M6809E.pNullPage);
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