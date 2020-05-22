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
    /// Represents the 256 nibbles of non-volatile storage on the I, Robot PCB
    /// The class persists the non-volatile data in the user properties file
    /// </summary>
    [Serializable]
    public class EEPROM : Hardware.Subsystem
    {
        // 4-bit x 256 NVRAM
        readonly byte[] NVRAM;

        readonly M6809E.ReadDelegate Read12xx;
        readonly M6809E.WriteDelegate Write12xx;

        public EEPROM(Hardware hardware) : base(hardware, "EEPROM")
        {
            try
            {
                NVRAM = Convert.FromBase64String(Properties.Settings.Default.EEPROM);
                if (NVRAM.Length != 256)
                    NVRAM = new byte[256];
            }
            catch
            {
                NVRAM = new byte[256];
            }

            Read12xx = new M6809E.ReadDelegate((UInt16 address) => { return (byte)(NVRAM[address & 0xFF] & 0x0F); });
            Write12xx = new M6809E.WriteDelegate((UInt16 address, byte data) => { NVRAM[address & 0xFF] = (byte)(data & 0x0F); });
        }

        public override void Dispose()
        {
            Properties.Settings.Default.EEPROM = Convert.ToBase64String(NVRAM);
            Properties.Settings.Default.Save();
        }

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("EEPROM", NVRAM);
        }

        public override void Reset()
        {
            Hardware.M6809E.SetPageIO(0x12, Read12xx, Write12xx);
        }
    }
}