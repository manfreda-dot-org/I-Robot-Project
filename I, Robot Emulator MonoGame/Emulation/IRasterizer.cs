﻿// Copyright 2020 by John Manfreda. All Rights Reserved.
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

namespace I_Robot.Emulation
{
    /// <summary>
    /// Interface to a rasterizer that interprets the I Robot display list
    /// </summary>
    public interface IRasterizer
    {
        /// <summary>
        /// Sets pointer to mathbox memory
        /// </summary>
        public Hardware Hardware { set; }

        /// <summary>
        /// Sets the current video buffer being renderd to
        /// </summary>
        /// <param name="index"></param>
        public void SetVideoBuffer(int index);

        /// <summary>
        /// Erases the selected video buffer
        /// </summary>
        public void EraseVideoBuffer();

        /// <summary>
        /// This command kicks off playfield rasterization
        /// </summary>
        void RasterizePlayfield();

        /// <summary>
        /// This command performas an unknown function, likely related to rasterization
        /// </summary>
        void UnknownCommand();

        /// <summary>
        /// Rasterizes the object at the given address, using the current lighting vector, camera vector, etc
        /// </summary>
        /// <param name="address"></param>
        void RasterizeObject(UInt16 address);
    }


}