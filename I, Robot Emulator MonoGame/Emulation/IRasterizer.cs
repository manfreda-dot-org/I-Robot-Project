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
using System.Windows.Documents;

namespace I_Robot.Emulation
{
    /// <summary>
    /// Interface to a rasterizer that can interpert I Robot Mathbox commands and build a rasterizable display list
    /// </summary>
    public interface IRasterizer
    {
        public interface Factory
        {
            /// <summary>
            /// Creates a rasterizer for the given machine
            /// </summary>
            /// <param name="machine">The machine the rasterizer belongs to.</param>
            /// <returns>The newly created rasterizer.</returns>
            IRasterizer CreateRasterizer(Machine machine);
        }
        
        /// <summary>
        /// Called when game program asserts ERASE
        /// </summary>
        /// <param name="BUFSEL">buffer to erase</param>
        void ERASE(bool BUFSEL);

        /// <summary>
        /// Called when game program asserts EXT_START
        /// </summary>
        /// <param name="BUFSEL">buffer to render to</param>
        void EXT_START(bool BUFSEL);

        /// <summary>
        /// Gets the state of the video processor
        /// </summary>
        bool EXT_DONE { get; }

        /// <summary>
        /// Rasterizes the object at the given address, using the current lighting vector, camera vector, etc
        /// </summary>
        /// <param name="address"></param>
        void RasterizeObject(UInt16 address);

        /// <summary>
        /// This command kicks off playfield rasterization
        /// </summary>
        void RasterizePlayfield();

        /// <summary>
        /// This command performas an unknown function, likely related to rasterization of playfield
        /// </summary>
        void UnknownCommand();
    }
}