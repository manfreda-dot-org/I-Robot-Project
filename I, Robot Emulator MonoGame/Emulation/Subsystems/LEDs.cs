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
using System.Runtime.InteropServices;
using System.Runtime.Serialization;

namespace I_Robot.Emulation
{
    /// <summary>
    /// Represents the two LEDs on the I, Robot control panel
    /// </summary>
    [Serializable]
    public class LEDs : Hardware.Subsystem
    {
        static byte[] keys = new byte[256];
        static bool? mLED1 = null;
        static bool? mLED2 = null;

        [DllImport("user32.dll", SetLastError = true)] static extern bool GetKeyboardState(byte[] lpKeyState);
        [DllImport("user32.dll")] static extern void keybd_event(byte bVk, byte bScan, uint dwFlags, UIntPtr dwExtraInfo);

        const int KEYEVENTF_EXTENDEDKEY = 0x1;
        const int KEYEVENTF_KEYUP = 0x2;

        public LEDs(Hardware hardware) : base(hardware, "LEDs")
        {
            GetKeyboardState(keys);
        }

        enum VK : byte
        {
            NUMLOCK = 0x90,
            SCROLL = 0x91,
            CAPSLOCK = 0x14,
        }

        void Set(VK type, bool state)
        {
            GetKeyboardState(keys);
            bool k = ((keys[(byte)type] & 1) != 0);
            if (state != k)
            {
                // key press
                keybd_event((byte)type, 0x45, KEYEVENTF_EXTENDEDKEY, (UIntPtr)0);
                // key release
                keybd_event((byte)type, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, (UIntPtr)0);
            }
        }

        public bool LED1
        {
            get => mLED1 ?? false;
            set
            {
                if (mLED1 != value)
                {
                    mLED1 = value;
                    Set(VK.NUMLOCK, value);
                }
            }
        }

        public bool LED2
        {
            get => mLED2 ?? false;
            set
            {
                if (mLED2 != value)
                {
                    mLED2 = value;
                    Set(VK.SCROLL, value);
                }
            }
        }

        public override void Dispose()
        {
        }

        public override void Reset()
        {
            LED1 = false;
            LED2 = false;
        }

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("LED1", LED1);
            info.AddValue("LED2", LED2);
        }
    }
}