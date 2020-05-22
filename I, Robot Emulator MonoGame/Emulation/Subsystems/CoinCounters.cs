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
    /// Represents the two LEDs on the I, Robot control panel
    /// </summary>
    [Serializable]
    public class CoinCounters : Machine.Subsystem
    {
        bool mLeftCoinCounter;
        bool mRightCoinCounter;

        public CoinCounters(Machine machine) : base(machine, "CoinCounters")
        {
        }

        public override void Dispose()
        {
        }

        public override void Reset()
        {
        }

        public bool LEFT_COIN_COUNTER
        {
            get => mLeftCoinCounter;
            set
            {
                if (mLeftCoinCounter != value)
                {
                    mLeftCoinCounter = value;
                    if (value)
                    {
                        Settings.LeftCoinCounter++;
                        System.Diagnostics.Debug.WriteLine($"Left coin counter = {Settings.LeftCoinCounter}");
                    }
                }
            }
        }

        public bool RIGHT_COIN_COUNTER
        {
            get => mRightCoinCounter;
            set
            {
                if (mRightCoinCounter != value)
                {
                    mRightCoinCounter = value;
                    if (value)
                    {
                        Settings.RightCoinCounter++;
                        System.Diagnostics.Debug.WriteLine($"Right coin counter = {Settings.RightCoinCounter}");
                    }
                }
            }
        }

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
        }
    }
}