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
    /// Represents the bank switch hardware for addresses 2000 - 3FFF on the I, Robot PCB
    /// </summary>
    [Serializable]
    unsafe public class Bank_2000 : Hardware.Subsystem
    {
        /// <summary>
        /// Enumeration of the possible banks of memory that can be switched in from 2000 - 3FFF
        /// </summary>
        public enum BANK : byte
        {
            MB_ROM_0 = 0,
            MB_ROM_1 = 1,
            MB_ROM_2 = 2,
            MB_ROM_3 = 3,
            MB_ROM_4 = 4,
            MB_ROM_5 = 5,
            MB_RAM,
            COMRAM_0,
            COMRAM_1,
        }

        /// <summary>
        /// Gets the currently selected bank of memory
        /// </summary>
        public BANK BankSelect { get; private set; }

        public Bank_2000(Hardware hardware) : base(hardware, "Bank 2000 - 3FFF")
        {
        }

        public override void Dispose()
        {
        }

        public override void Reset()
        {
            // $20xx-$3Fxx banked COMRAM / Mathbox
            Hardware.M6809E.SetPageIO(0x20, 0x3F, M6809E.pNullPage, M6809E.pNullPage);

            BankSelect = (BANK)0xFF;
            BankSwitch();
        }

        BANK CurrentBank
        { 
            get
            {
                if (!Hardware.Registers.OUT0.OUT4)
                {
                    switch ((Hardware.Registers.OUT0.Value >> 1) & 7)
                    {
                        case 0: return BANK.MB_ROM_0;
                        case 1: return BANK.MB_ROM_1;
                        case 2: return BANK.MB_ROM_0;
                        case 3: return BANK.MB_ROM_1;
                        case 4: return BANK.MB_ROM_2;
                        case 5: return BANK.MB_ROM_3;
                        case 6: return BANK.MB_ROM_4;
                        default: return BANK.MB_ROM_5;
                    }
                }
                else if (Hardware.Registers.OUT0.OUT3)
                    return BANK.MB_RAM;
                else
                    return Hardware.Registers.STATWR.EXT_COM_SWAP ? BANK.COMRAM_1 : BANK.COMRAM_0;
            }
        }

        /// <summary>
        /// Performs a bank switch, according to latest values in OUT0 and STATWR
        /// </summary>
        public void BankSwitch()
        {
            BANK bank = CurrentBank;

            if (BankSelect != bank)
            {
                BankSelect = bank;

                // System.Diagnostics.Debug.WriteLine($"Bank 2000: OUT0 = {out0.HexString()}  EXT_COM_SWAP = {Hardware.Registers.STATWR.EXT_COM_SWAP}");

                switch (bank)
                {
                    case BANK.COMRAM_0: Hardware.M6809E.SetPageIO(0x20, 0x3F, Hardware.VideoProcessor.COMRAM[0]); break;
                    case BANK.COMRAM_1: Hardware.M6809E.SetPageIO(0x20, 0x3F, Hardware.VideoProcessor.COMRAM[1]); break;
                    case BANK.MB_RAM: Hardware.M6809E.SetPageIO(0x20, 0x3F, Hardware.Mathbox.ReadRamFunction, Hardware.Mathbox.WriteRamFunction); break;
                    default: Hardware.M6809E.SetPageIO(0x20, 0x3F, Hardware.Mathbox.ROM[(byte)bank], M6809E.pNullPage); break;
                }
            }
        }

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("BankSelect", (byte)BankSelect);
        }

    }
}