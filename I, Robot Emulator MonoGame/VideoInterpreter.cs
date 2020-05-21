using System;
using System.Collections.Generic;
using System.Text;

namespace I_Robot
{
    unsafe class VideoInterpreter : Mathbox.IInterpreter
    {
        #region MATHBOX INTERFACE

        /// <summary>
        /// Pointer to Mathbox memory
        /// </summary>
        UInt16* Memory;

        public unsafe ushort* pMemory { set => Memory = value; }

        void Mathbox.IInterpreter.SetVideoBuffer(int index)
        {
        }

        void Mathbox.IInterpreter.EraseVideoBuffer()
        {
        }

        void Mathbox.IInterpreter.RasterizeObject(ushort address)
        {
        }

        void Mathbox.IInterpreter.RasterizePlayfield()
        {
        }

        void Mathbox.IInterpreter.UnknownCommand()
        {
        }
        #endregion
    }
}
