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
using System.Windows.Input;

namespace I_Robot
{
    /// <summary>
    /// Class that represents all of the special function registers on the I, Robot hardware
    /// </summary>
    [Serializable]
    unsafe public class Registers : Hardware.Subsystem
    {
        #region STRUCTURES
        public struct REGISTER_1000
        {
            public byte Value;
            public REGISTER_1000(byte value) { Value = value; }

            // active low
            public bool COIN_R
            {
                get { return (Value & 0x80) == 0; }
                set { if (!value) Value |= 0x80; else Value &= 0x7F; }
            }

            // active low
            public bool COIN_L
            {
                get { return (Value & 0x40) == 0; }
                set { if (!value) Value |= 0x40; else Value &= 0xBF; }
            }

            // active low
            public bool COIN_AUX
            {
                get { return (Value & 0x20) == 0; }
                set { if (!value) Value |= 0x20; else Value &= 0xDF; }
            }

            // active low
            public bool TEST
            {
                get { return (Value & 0x10) == 0; }
                set { if (!value) Value |= 0x10; else Value &= 0xEF; }
            }

            // active low
            public bool COIN_TAMPER
            {
                get { return (Value & 0x08) == 0; }
                set { if (!value) Value |= 0x08; else Value &= 0xF7; }
            }

            public static implicit operator byte(REGISTER_1000 r) => r.Value;
            public static implicit operator REGISTER_1000(byte b) => new REGISTER_1000(b);
            public override string ToString() { return Value.HexString(); }
        }

        public struct REGISTER_1040
        {
            public byte Value;
            public REGISTER_1040(byte value) { Value = value; }

            // active low
            public bool START_1
            {
                get { return (Value & 0x80) == 0; }
                set { if (!value) Value |= 0x80; else Value &= 0x7F; }
            }

            // active low
            public bool START_2
            {
                get { return (Value & 0x40) == 0; }
                set { if (!value) Value |= 0x40; else Value &= 0xBF; }
            }

            // active low
            public bool FIRE
            {
                get { return (Value & 0x10) == 0; }
                set { if (!value) Value |= 0x10; else Value &= 0xEF; }
            }

            public static implicit operator byte(REGISTER_1040 r) => r.Value;
            public static implicit operator REGISTER_1040(byte b) => new REGISTER_1040(b);
            public override string ToString() { return Value.HexString(); }

        }

        public struct REGISTER_1080
        {
            public byte Value;
            public REGISTER_1080(byte value) { Value = value; }

            public bool VBLANK
            {
                get { return (Value & 0x80) != 0; }
                set { if (value) Value |= 0x80; else Value &= 0x7F; }
            }

            // active low
            public bool EXT_DONE
            {
                get { return (Value & 0x40) == 0; }
                set { if (!value) Value |= 0x40; else Value &= 0xBF; }
            }

            // active high
            public bool MB_DONE
            {
                get { return (Value & 0x20) != 0; }
                set { if (value) Value |= 0x20; else Value &= 0xDF; }
            }

            public static implicit operator byte(REGISTER_1080 r) => r.Value;
            public static implicit operator REGISTER_1080(byte b) => new REGISTER_1080(b);
            public override string ToString() { return Value.HexString(); }

        }

        public struct REGISER_1140
        {
            public byte Value;
            public REGISER_1140(byte value) { Value = value; }

            public bool EXT_COM_SWAP { get { return (Value & 0x80) != 0; } }
            public bool RECALL { get { return (Value & 0x40) != 0; } }
            public bool COCKTAIL { get { return (Value & 0x20) != 0; } }
            public bool MATH_START { get { return (Value & 0x10) != 0; } }
            public bool ADDCON { get { return (Value & 0x08) != 0; } }
            public bool EXT_START { get { return (Value & 0x04) != 0; } }
            public bool COM_RAM_SEL { get { return (Value & 0x02) != 0; } }
            public bool ERASE { get { return (Value & 0x01) != 0; } }

            public static implicit operator byte(REGISER_1140 r) => r.Value;
            public static implicit operator REGISER_1140(byte b) => new REGISER_1140(b);
            public override string ToString() { return Value.HexString(); }

        }

        public struct REGISTER_1180
        {
            public byte Value;
            public REGISTER_1180(byte value) { Value = value; }

            public bool ALPHA_MAP { get { return (Value & 0x80) != 0; } }
            public byte RAM_800_BANK { get { return (byte)((Value >> 5) & 3); } }
            public bool OUT4 { get { return (Value & 0x10) != 0; } }
            public bool OUT3 { get { return (Value & 0x08) != 0; } }
            public bool MPAGE2 { get { return (Value & 0x04) != 0; } }
            public bool MPAGE1 { get { return (Value & 0x02) != 0; } }

            public static implicit operator byte(REGISTER_1180 r) => r.Value;
            public static implicit operator REGISTER_1180(byte b) => new REGISTER_1180(b);
            public override string ToString() { return Value.HexString(); }

        }

        public struct REGISTER_11C0
        {
            public byte Value;
            public REGISTER_11C0(byte value) { Value = value; }

            public bool LEFT_COIN_COUNTER { get { return (Value & 0x80) != 0; } }
            public bool RIGHT_COIN_COUNTER { get { return (Value & 0x40) != 0; } }
            public bool LED2 { get { return (Value & 0x20) != 0; } }
            public bool LED1 { get { return (Value & 0x10) != 0; } }
            public byte ROM_4000_BANK { get { return (byte)((Value >> 1) & 7); } }

            public static implicit operator byte(REGISTER_11C0 r) => r.Value;
            public static implicit operator REGISTER_11C0(byte b) => new REGISTER_11C0(b);
            public override string ToString() { return Value.HexString(); }
        }
        #endregion

        readonly M6809E.ReadDelegate Read10xx;
        readonly M6809E.WriteDelegate Write11xx;
        readonly M6809E.WriteDelegate Write19xx;
        readonly M6809E.WriteDelegate Write1Axx;

        UInt64 Watchdog = 0;
        public UInt64 WatchdogCounter => Hardware.M6809E.Clock - Watchdog;

        REGISER_1140 mSTATWR = 0x00;
        REGISTER_1180 mOUT0 = 0x10;
        REGISTER_11C0 mOUT1 = 0x00;

        public Registers(Hardware hardware) : base(hardware, "Regsisters")
        {
            Read10xx = new M6809E.ReadDelegate((UInt16 address) =>
            {
                if (address <= 0x103F)
                    return INRD1; // 1000

                if (address <= 0x107F)
                    return INRD2; // 1040

                if (address <= 0x10BF)
                    return STATRD; // 1080

                //	if (address <= 0x10FF)
                return Settings.DipSwitch3J; // 10C0
            });

            Write11xx = new M6809E.WriteDelegate((UInt16 address, byte data) =>
            {
                if (address <= 0x113F)
                    CLEAR_IRQ = data; // 1100
                else if (address <= 0x117F)
                    STATWR = data; // 1140
                else if (address <= 0x11BF)
                    OUT0 = data; // 1180
                else// if (address <= 0x11FF)
                    OUT1 = data; // 11C0
            });

            Write19xx = new M6809E.WriteDelegate((UInt16 address, byte data) => { CLEAR_WATCHDOG = data; });

            Write1Axx = new M6809E.WriteDelegate((UInt16 address, byte data) => { CLEAR_FIRQ = data; });
        }

        public override void Dispose()
        {
        }

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("Watchdog", Watchdog);
            info.AddValue("STATWR", (byte)STATWR);
            info.AddValue("OUT0", (byte)OUT0);
            info.AddValue("OUT1", (byte)OUT1);
        }

        public override void Reset()
        {
            Hardware.M6809E.SetPageIO(0x10, Read10xx, M6809E.pNullPage);
            Hardware.M6809E.SetPageIO(0x11, M6809E.pNullPage, Write11xx);
            Hardware.M6809E.SetPageIO(0x19, M6809E.pNullPage, Write19xx);
            Hardware.M6809E.SetPageIO(0x1A, M6809E.pNullPage, Write1Axx);

            Watchdog = 0;
            STATWR = 0x00;
            OUT0 = 0x10;
            OUT1 = 0x00;
        }

        public REGISTER_1000 INRD1
        {
            get
            {
                REGISTER_1000 reg = 0xFF;
                reg.COIN_L = Keyboard.IsKeyDown(Key.D5);
                reg.COIN_R = Keyboard.IsKeyDown(Key.D6);
                reg.COIN_AUX = Keyboard.IsKeyDown(Key.D7);
                reg.TEST = Settings.TestSwitch;
                return reg;
            }
        }

        public REGISTER_1040 INRD2
        {
            get
            {
                REGISTER_1040 reg = 0xFF;
                reg.START_1 = Keyboard.IsKeyDown(Key.D1);
                reg.START_2 = Keyboard.IsKeyDown(Key.D2);
                reg.FIRE = Keyboard.IsKeyDown(Key.LeftCtrl);
                return reg;
            }
        }

        public REGISTER_1080 STATRD
        {
            get
            {
                REGISTER_1080 reg = 0;
                reg.VBLANK = Hardware.VBLANK;
                reg.EXT_DONE = Hardware.VideoProcessor.EXT_DONE;
                reg.MB_DONE = Hardware.Mathbox.MB_DONE;
                return reg;
            }
        }

        public byte DIP_SWITCH_3J { get => Settings.DipSwitch3J; }
        public byte CLEAR_IRQ { set { Hardware.M6809E.IRQ = false; } }
        public byte CLEAR_WATCHDOG { set { Watchdog = Hardware.M6809E.Clock; } }
        public byte CLEAR_FIRQ { set { Hardware.M6809E.FIRQ = false; } }

        public REGISER_1140 STATWR
        {
            get => mSTATWR;
            set
            {
                mSTATWR = value;

                Hardware.Bank_2000.BankSwitch();
                Hardware.Mathbox.MATH_START = mSTATWR.MATH_START;
                Hardware.VideoProcessor.EXT_START = mSTATWR.EXT_START;
            }
        }

        public REGISTER_1180 OUT0
        {
            get => mOUT0;
            set
            {
                mOUT0 = value;

                Hardware.Alphanumerics.ALPHA_MAP = mOUT0.ALPHA_MAP;

                Hardware.RAM_0800.BankSelect = mOUT0.RAM_800_BANK;

                Hardware.Bank_2000.BankSwitch();
            }
        }

        public REGISTER_11C0 OUT1
        {
            get => mOUT1;
            set
            {
                mOUT1 = value;
                Hardware.ProgROM.BankSelect = value.ROM_4000_BANK;
            }
        }
    }
}