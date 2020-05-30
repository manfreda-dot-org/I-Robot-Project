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
using System.Runtime.CompilerServices;
using System.Windows.Documents;

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

        public Cheats(Machine machine)
        {
            Machine = machine;
        }

        public void Update()
        {
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