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
using System.Text;

namespace I_Robot
{
    static class Settings
    {
        public enum Language : byte
        {
            German = 0,
            English = 1
        }

        public enum Difficulty : byte
        {
            Easy = 0,
            Medium = 1
        }

        public enum BonusLives : byte
        {
            Every_30000 = 0x00,
            Every_50000 = 0x04,
            None = 0x08,
            Every_20000 = 0x0C,
        }

        public enum Lives : byte
        {
            _4 = 0x00,
            _5 = 0x10,
            _2 = 0x20,
            _3 = 0x30
        }

        static public bool SpeedThrottle
        {
            get => Properties.Settings.Default.SpeedThrottle;
            set
            {
                Properties.Settings.Default.SpeedThrottle = value;
                Properties.Settings.Default.Save();
            }
        }

        static public bool TestSwitch
        {
            get => Properties.Settings.Default.TestSwitch;
            set
            {
                Properties.Settings.Default.TestSwitch = value;
                Properties.Settings.Default.Save();
            }
        }

        static public byte DipSwitch3J
        {
            get => Properties.Settings.Default.DipSwitch3J;
            set
            {
                Properties.Settings.Default.DipSwitch3J = value;
                Properties.Settings.Default.Save();
            }
        }

        static public byte DipSwitch5E
        {
            get => Properties.Settings.Default.DipSwitch5E;
            set
            {
                Properties.Settings.Default.DipSwitch5E = value;
                Properties.Settings.Default.Save();
            }
        }

        static public bool SoundEnabled
        {
            get => Properties.Settings.Default.SoundEnabled;
            set
            {
                Properties.Settings.Default.SoundEnabled = value;
                Properties.Settings.Default.Save();
            }
        }

        static public bool ShowDots
        {
            get => Properties.Settings.Default.ShowDots;
            set
            {
                Properties.Settings.Default.ShowDots = value;
                Properties.Settings.Default.Save();
            }
        }

        static public bool ShowVectors
        {
            get => Properties.Settings.Default.ShowVectors;
            set
            {
                Properties.Settings.Default.ShowVectors = value;
                Properties.Settings.Default.Save();
            }
        }

        static public bool ShowPolygons
        {
            get => Properties.Settings.Default.ShowPolygons;
            set
            {
                Properties.Settings.Default.ShowPolygons = value;
                Properties.Settings.Default.Save();
            }
        }

        static public bool ShowFPS
        {
            get => Properties.Settings.Default.ShowFPS;
            set
            {
                Properties.Settings.Default.ShowFPS = value;
                Properties.Settings.Default.Save();
            }
        } 

        static public bool Wireframe
        {
            get => Properties.Settings.Default.Wireframe;
            set
            {
                Properties.Settings.Default.Wireframe = value;
                Properties.Settings.Default.Save();
            }
        }

        static byte WriteFlag(bool value, byte b, byte flag)
        {
            if (value)
                return (byte)(b | flag);
            else
                return (byte)(b & (0xFF - flag));
        }

        static public bool FreePlay
        {
            get { return DipSwitch3J == 0xE0; }
            set { DipSwitch3J = (byte)(value ? 0xE0 : 0x00); }
        }

        static public Language GameLanguage
        {
            get { return ((DipSwitch5E & 0x01) == 0) ? Language.German : Language.English; }
            set { DipSwitch5E = WriteFlag(value == Language.English, DipSwitch5E, 0x01); }
        }

        static public bool MinimumGameTime
        {
            get { return (DipSwitch5E & 0x02) == 0; }
            set { DipSwitch5E = WriteFlag(!value, DipSwitch5E, 0x02); }
        }

        static public BonusLives BonusLifeInterval
        {
            get { return (BonusLives)(DipSwitch5E & 0x0C); }
            set { DipSwitch5E = (byte)((DipSwitch5E & 0xF3) | (byte)value); }
        }

        static public Lives LivesPerCredit
        {
            get { return (Lives)(DipSwitch5E & 0x30); }
            set { DipSwitch5E = (byte)((DipSwitch5E & 0xCF) | (byte)value); }
        }

        static public Difficulty GameDifficulty
        {
            get { return ((DipSwitch5E & 0x40) == 0) ? Difficulty.Easy : Difficulty.Medium; }
            set { DipSwitch5E = WriteFlag(value == Difficulty.Medium, DipSwitch5E, 0x40); }
        }

        static public bool DemoMode
        {
            get { return (DipSwitch5E & 0x80) == 0; }
            set { DipSwitch5E = WriteFlag(!value, DipSwitch5E, 0x80); }
        }
    }
}