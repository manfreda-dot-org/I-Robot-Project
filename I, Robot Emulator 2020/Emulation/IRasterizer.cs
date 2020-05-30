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

namespace I_Robot.Emulation
{
    /// <summary>
    /// Interface to a rasterizer that can interpert I Robot Mathbox commands and rasterize the display list
    /// Interface used to separate emulator from OS native graphics routines, etc
    /// </summary>
    public interface IRasterizer : IDisposable
    {
        /// <summary>
        /// Interface for a factory that can create a rasterizer for the emulator.
        /// </summary>
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
        /// Indicates that the selected bitmap must be zereoed out
        /// </summary>
        /// <param name="BUFSEL">bitmap buffer to erase</param>
        void ERASE(bool BUFSEL);

        /// <summary>
        /// Called when game program asserts EXT_START
        /// Indicates that the video process should start rasterizing to the selected bitmap
        /// </summary>
        /// <param name="BUFSEL">bitmap buffer to render into</param>
        void EXT_START(bool BUFSEL);

        /// <summary>
        /// Gets the state of the video processor
        /// </summary>
        bool EXT_DONE { get; }

        /// <summary>
        /// Rasterizes the object at the given address, using the current lighting vector, camera vector, etc
        /// </summary>
        /// <param name="address">linear address in mathbox memory map from 0000 - 7FFF</param>
        void RasterizeObject(UInt16 address);

        /// <summary>
        /// This command kicks off terrain generation
        /// </summary>
        void GenerateTerrain();

        /// <summary>
        /// This command re-generates current terrain (assumes that GenerateTerrain() has already been called)
        /// </summary>
        void GenerateTerrainReentrant();
    }
}