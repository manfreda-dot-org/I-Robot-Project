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
    /// Represents the ADC chip on the I, Robot PCB
    /// This chip is responsible performing ADC conversions on the hall effect joystick
    /// </summary>
    [Serializable]
    unsafe public class ADC : Machine.Subsystem
    {
        readonly M6809E.ReadDelegate Read13xx;
        readonly M6809E.WriteDelegate Write1Bxx;

        /// <summary>
        /// Current ADC result from last conversion
        /// </summary>
        byte ADC_RESULT = 0x80;

        public ADC(Machine machine) : base(machine, "ADC")
        {
            Read13xx = new M6809E.ReadDelegate((UInt16 address) =>
                {
                    // return the last ADC result
                    return ADC_RESULT;
                });

            Write1Bxx = new M6809E.WriteDelegate((UInt16 address, byte data) =>
                {
                    // writes to this range kickff ADC conversions of joystick values
                    if ((address & 0x1) == 0)
                        ADC_RESULT = Joystick.ADC_Y;
                    else
                        ADC_RESULT = Joystick.ADC_X;
                });
        }
        public override void Dispose()
        {
        }

        public override void Reset()
        {
            ADC_RESULT = 0x80;

            Machine.M6809E.SetPageIO(0x13, Read13xx, M6809E.pNullPage);
            Machine.M6809E.SetPageIO(0x1B, M6809E.pNullPage, Write1Bxx);
        }

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("ADC_RESULT", ADC_RESULT);
        }
    }
}