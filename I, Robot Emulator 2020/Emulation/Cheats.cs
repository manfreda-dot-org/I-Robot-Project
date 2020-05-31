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

namespace I_Robot.Emulation
{
    /// <summary>
    /// Interface to a rasterizer that can interpert I Robot Mathbox commands and build a rasterizable display list
    /// </summary>
    public class Cheats
    {
        readonly Machine Machine;

        public bool JumpsCreateBridges = true;
        public bool NoRedTilesRemain = false;
        public bool UnlimitedTransporters = false;
        public bool UnlimitedLives = false;
        public bool UnlimitedDoodleCity = false;
        public byte PlayfieldRenderMode = 0;
        public bool RapidFire = false;
        bool mBulletStorm = false;
        bool mOneShotKillsMeteors = false;
        bool mNoSawsInsidePyramid = false;
        bool mRomDebugMode = false;

        public Cheats(Machine machine)
        {
            Machine = machine;
        }

        public bool BulletStorm
        {
            get => mBulletStorm;
            set
            {
                if (mBulletStorm != value)
                {
                    mBulletStorm = value;
                    if (value)
                    {
                        Machine.ProgROM.ROM_6000[0xD1CE - 0x6000] = 10;
                        Machine.ProgROM.ROM_6000[0xE473 - 0x6000] = 10;
                    }
                    else
                    {
                        Machine.ProgROM.ROM_6000[0xD1CE - 0x6000] = 3;
                        Machine.ProgROM.ROM_6000[0xE473 - 0x6000] = 3;
                    }
                }
            }
        }

        public bool OneShotKillsMeteors
        {
            get => mOneShotKillsMeteors;
            set
            {
                if (mOneShotKillsMeteors != value)
                {
                    mOneShotKillsMeteors = value;
                    if (value)
                        Machine.ProgROM.ROM_6000[0xDB24 - 0x6000] = 0x00;
                    else
                        Machine.ProgROM.ROM_6000[0xDB24 - 0x6000] = 0x1A;
                }
            }
        }

        public bool NoSawsInsidePyramid
        {
            get => mNoSawsInsidePyramid;
            set
            {
                if (mNoSawsInsidePyramid != value)
                {
                    mNoSawsInsidePyramid = value;
                    if (value)
                        Machine.ProgROM.ROM_6000[0x70BB - 0x6000] = 0x39;
                    else
                        Machine.ProgROM.ROM_6000[0x70BB - 0x6000] = 0xBD;
                }
            }
        }

        public bool RomDebugMode
        {
            get => mRomDebugMode;
            set
            {
                if (mRomDebugMode != value)
                {
                    mRomDebugMode = value;
                    if (value)
                        Machine.ProgROM.ROM_6000[0x65C5 - 0x6000] = 0xFF;
                    else
                        Machine.ProgROM.ROM_6000[0x65C5 - 0x6000] = 0x00;
                }
            }
        }

        public void Update()
        {
#if false
            Machine.ProgROM.ROM_6000[0xD1BF - 0x6000] = 11;
            Machine.ProgROM.ROM_6000[0xD206 - 0x6000] = 0x80;
            Machine.ProgROM.ROM_6000[0xE4A5 - 0x6000] = 0x03;
            
            Machine.ProgROM.ROM_6000[0xb264 - 0x6000] = 0x90;
            Machine.ProgROM.ROM_6000[0xb266 - 0x6000] = 0x70;
            Machine.ProgROM.ROM_6000[0xb268 - 0x6000] = 0x90;
            Machine.ProgROM.ROM_6000[0xb26A - 0x6000] = 0x70;
            Machine.ProgROM.ROM_6000[0xb26E - 0x6000] = 0x90;
            Machine.ProgROM.ROM_6000[0xb270 - 0x6000] = 0x70;

            Machine.ProgROM.ROM_6000[0xb288 - 0x6000] = 0xA0;
            Machine.ProgROM.ROM_6000[0xb28A - 0x6000] = 0x70;
            Machine.ProgROM.ROM_6000[0xb28C - 0x6000] = 0x90;
            Machine.ProgROM.ROM_6000[0xb28E - 0x6000] = 0x70;
            Machine.ProgROM.ROM_6000[0xb292 - 0x6000] = 0x90;
            Machine.ProgROM.ROM_6000[0xb294 - 0x6000] = 0x70;

            // allow space out of bounds
            Machine.ProgROM.ROM_6000[0xDF7E - 0x6000] = 0x20; // BRA
            Machine.ProgROM.ROM_6000[0xDF88 - 0x6000] = 0x20; // BRA

            // always safe landing
            Machine.ProgROM.ROM_6000[0xE027 - 0x6000] = 0x00;
            Machine.ProgROM.ROM_6000[0xE02D - 0x6000] = 0x00;
            Machine.ProgROM.ROM_6000[0xE03D - 0x6000] = 0x20; // BRA



            // no saucers
            Machine.ProgROM.ROM_6000[0xDCF1 - 0x6000] = 0x0;
#endif


            if (!JumpsCreateBridges)
                Machine.RAM_0000[0x288] |= 0x10;

            if (NoRedTilesRemain)
                Machine.RAM_0000[0x2FF] = 1;

            if (UnlimitedTransporters)
                Machine.RAM_0000[0x302] = 0;

            if (UnlimitedLives)
                Machine.RAM_0000[0x303] = 4;

            if (UnlimitedDoodleCity)
                Machine.RAM_0000[0x371] = 61;

            if (PlayfieldRenderMode > 0)
                Machine.RAM_0000[0x3D5] = PlayfieldRenderMode;

            if (RapidFire)
                Machine.RAM_0000[0x625] = 0;
        }
    }
}