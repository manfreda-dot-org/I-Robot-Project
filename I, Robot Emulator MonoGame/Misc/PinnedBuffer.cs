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

namespace I_Robot
{
    /// <summary>
    /// Class that allocates and pins a buffer in memory
    /// Memory is pinned so garbage collector can't move it
    /// An unsafe byte pointer to the data is available
    /// </summary>
    /// <typeparam name="T">type of data to be pinned, must be basic unmanaged type</typeparam>
    [Serializable]
    unsafe public sealed class PinnedBuffer<T> : ISerializable, IDisposable where T: unmanaged
    {
        /// <summary>
        /// The managed data buffer
        /// </summary>
        readonly T[] ManagedBuffer;

        readonly Memory<T> Memory;
        readonly System.Buffers.MemoryHandle Handle;

        /// <summary>
        /// Raw pointer to the data (unsafe)
        /// </summary>
        readonly T* pData;

        public int Length => ManagedBuffer.Length;

        /// <summary>
        /// Creates a new pinned buffer from an existing buffer
        /// </summary>
        /// <param name="buffer">existing buffer to pin</param>
        public PinnedBuffer(T[] buffer)
        {
            // allocate managed space
            ManagedBuffer = buffer;

            // pin the memory
            Memory = new Memory<T>(ManagedBuffer);
            Handle = Memory.Pin();

            // get a pointer to the pinned memory
            pData = (T*)Handle.Pointer;
        }

        /// <summary>
        /// Creates a new PinnedBuffer
        /// </summary>
        /// <param name="size">number of elements in the buffer</param>
        public PinnedBuffer(int size) 
            : this(new T[size])
        {
        }

        ~PinnedBuffer()
        {
            Dispose();
        }

        public void Dispose()
        {
            // unpin the memory
            Handle.Dispose();
        }

        public T this[int index]
        {
            get => pData[index];
            set => pData[index] = value;
        }

        public static implicit operator T[](PinnedBuffer<T> r) => r.ManagedBuffer;
        public static implicit operator T*(PinnedBuffer<T> r) => r.pData;

        public void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("Data", ManagedBuffer);
        }
    }
}
