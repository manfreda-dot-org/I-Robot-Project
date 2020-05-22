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
    /// Represents the video processor circuit on the I, Robot PCB
    /// </summary>
    [Serializable]
    unsafe public class VideoProcessor : Machine.Subsystem
    {
        // two banks of COMRAM
        public readonly PinnedBuffer<byte>[] COMRAM = new PinnedBuffer<byte>[2] { new PinnedBuffer<byte>(0x2000), new PinnedBuffer<byte>(0x2000) };

        readonly IRasterizer Rasterizer;
        bool mEXT_START;

        /// <summary>
        /// Video processor finished signal
        /// </summary>
        public bool EXT_DONE { get; private set; } = true;

        public VideoProcessor(Machine machine, IRasterizer rasterizer) : base(machine, "Video Processor")
        {
            Rasterizer = rasterizer;
        }

        public override void Dispose()
        {
        }

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("EXT_DONE", EXT_DONE);
            info.AddValue("COMRAM_0", COMRAM[0]);
            info.AddValue("COMRAM_1", COMRAM[1]);
        }

        public override void Reset()
        {

        }

        /// <summary>
        /// Hardware signal that tells the video processor to execute the display list in COMRAM
        /// </summary>
        public bool EXT_START
        {
            set
            {
                if (mEXT_START != value)
                {
                    mEXT_START = value;
                    if (!value)
                        Rasterizer.EXT_START();
                }
            }
        }
    }
}