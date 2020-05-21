using System;
using System.Collections.Generic;
using System.Text;

namespace I_Robot
{
    unsafe class VideoInterpreter : Mathbox.IInterpreter
    {
        UInt16* Memory;

        public unsafe ushort* pMemory { set => Memory = value; }

        public void SetVideoBuffer(int index)
        {
        }

        public void EraseVideoBuffer()
        {
        }

        public void RasterizeObject(ushort address)
        {
        }

        public void RasterizePlayfield()
        {
        }

        public void UnknownCommand()
        {
        }
    }
}
