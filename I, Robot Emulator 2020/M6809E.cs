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
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;

namespace I_Robot
{
    [Serializable]
    unsafe public class M6809E : ISerializable, IDisposable
    {
        // unused page of data to route null writes to
        static readonly PinnedBuffer<byte> Null64k = new(0x10000);
        static public readonly byte* pNullPage = Null64k;

        [DllImport("kernel32.dll")] static extern void RtlZeroMemory(IntPtr dst, int length);

        #region M6809E.dll

        public enum Result : int
        {
            OK = 0,
            ERROR_INTERNAL,
            ERROR_INSTRUCTION,
            ERROR_ADDRESS_MODE,
            ERROR_EXG,
            ERROR_TFR,

            ERROR_UNKNOWN_READ,
            ERROR_UNKNOWN_WRITE,
        }

        // delegates for the fault callback function
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        delegate void FaultDelegate(Result fault);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void PeriodicDelegate();

        /// <summary>
        /// Delegate for 6809 memory read
        /// </summary>
        /// <param name="address">address to read from</param>
        /// <returns>value at address</returns>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate byte ReadDelegate(UInt16 address);

        /// <summary>
        /// Delegate for 6809 memory write
        /// </summary>
        /// <param name="address">address to write to</param>
        /// <param name="value">value to write</param>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void WriteDelegate(UInt16 address, byte value);

        [DllImport("M6809E.dll")] static extern int Get_6809_StructSize();

        [DllImport("M6809E.dll")] static extern UInt64 Get_6809_Clock(IntPtr state);
        [DllImport("M6809E.dll")] static extern byte Get_6809_A(IntPtr state);
        [DllImport("M6809E.dll")] static extern byte Get_6809_B(IntPtr state);
        [DllImport("M6809E.dll")] static extern byte Get_6809_DP(IntPtr state);
        [DllImport("M6809E.dll")] static extern byte Get_6809_CC(IntPtr state);
        [DllImport("M6809E.dll")] static extern UInt16 Get_6809_D(IntPtr state);
        [DllImport("M6809E.dll")] static extern UInt16 Get_6809_X(IntPtr state);
        [DllImport("M6809E.dll")] static extern UInt16 Get_6809_Y(IntPtr state);
        [DllImport("M6809E.dll")] static extern UInt16 Get_6809_U(IntPtr state);
        [DllImport("M6809E.dll")] static extern UInt16 Get_6809_SP(IntPtr state);
        [DllImport("M6809E.dll")] static extern UInt16 Get_6809_PC(IntPtr state);
        [DllImport("M6809E.dll")] static extern bool Get_6809_NMI(IntPtr state);
        [DllImport("M6809E.dll")] static extern bool Get_6809_FIRQ(IntPtr state);
        [DllImport("M6809E.dll")] static extern bool Get_6809_IRQ(IntPtr state);

        [DllImport("M6809E.dll")] static extern void Set_6809_Clock(IntPtr state, UInt64 val);
        [DllImport("M6809E.dll")] static extern void Set_6809_A(IntPtr state, byte val);
        [DllImport("M6809E.dll")] static extern void Set_6809_B(IntPtr state, byte val);
        [DllImport("M6809E.dll")] static extern void Set_6809_DP(IntPtr state, byte val);
        [DllImport("M6809E.dll")] static extern void Set_6809_CC(IntPtr state, byte val);
        [DllImport("M6809E.dll")] static extern void Set_6809_D(IntPtr state, UInt16 val);
        [DllImport("M6809E.dll")] static extern void Set_6809_X(IntPtr state, UInt16 val);
        [DllImport("M6809E.dll")] static extern void Set_6809_Y(IntPtr state, UInt16 val);
        [DllImport("M6809E.dll")] static extern void Set_6809_U(IntPtr state, UInt16 val);
        [DllImport("M6809E.dll")] static extern void Set_6809_SP(IntPtr state, UInt16 val);
        [DllImport("M6809E.dll")] static extern void Set_6809_PC(IntPtr state, UInt16 val);
        [DllImport("M6809E.dll")] static extern void Set_6809_NMI(IntPtr state, bool val);
        [DllImport("M6809E.dll")] static extern void Set_6809_FIRQ(IntPtr state, bool val);
        [DllImport("M6809E.dll")] static extern void Set_6809_IRQ(IntPtr state, bool val);


        [DllImport("M6809E.dll")] static extern void M6809_Reset(IntPtr state, PeriodicDelegate? callback, double cycles_till_callback);
        [DllImport("M6809E.dll")] static extern Result M6809_Run(IntPtr state, UInt32 cyclesToRun);

        #endregion

        class PageReadPointerManager(IntPtr state)
        {
            [DllImport("M6809E.dll")] static extern byte* Get_6809_PageReadPointer(IntPtr state, byte page);
            [DllImport("M6809E.dll")] static extern void Set_6809_PageReadPointer(IntPtr state, byte page, byte* ptr);
            readonly IntPtr State = state;

            public byte* this[byte page]
            {
                get { return Get_6809_PageReadPointer(State, page); }
                set { Set_6809_PageReadPointer(State, page, value); System.Diagnostics.Debug.Assert(this[page] == value); }
            }
        }

        class PageWritePointerManager
        {
            [DllImport("M6809E.dll")] static extern byte* Get_6809_PageWritePointer(IntPtr state, byte page);
            [DllImport("M6809E.dll")] static extern void Set_6809_PageWritePointer(IntPtr state, byte page, byte* ptr);
            readonly IntPtr State;
            public PageWritePointerManager(IntPtr state) { State = state; }
            public byte* this[byte page]
            {
                get { return Get_6809_PageWritePointer(State, page); }
                set { Set_6809_PageWritePointer(State, page, value); System.Diagnostics.Debug.Assert(this[page] == value); }
            }
        }

        class PageReadFunctionManager(IntPtr State)
        {
            [DllImport("M6809E.dll")] static extern ReadDelegate? Get_6809_PageReadFunction(IntPtr state, byte page);
            [DllImport("M6809E.dll")] static extern void Set_6809_PageReadFunction(IntPtr state, byte page, ReadDelegate? fptr);

            public ReadDelegate? this[byte page]
            {
                get { return Get_6809_PageReadFunction(State, page); }
                set { Set_6809_PageReadFunction(State, page, value); System.Diagnostics.Debug.Assert(this[page] == value); }
            }
        }

        class PageWriteFunctionManager(IntPtr State)
        {
            [DllImport("M6809E.dll")] static extern WriteDelegate? Get_6809_PageWriteFunction(IntPtr state, byte page);
            [DllImport("M6809E.dll")] static extern void Set_6809_PageWriteFunction(IntPtr state, byte page, WriteDelegate? fptr);

            public WriteDelegate? this[byte page]
            {
                get { return Get_6809_PageWriteFunction(State, page); }
                set { Set_6809_PageWriteFunction(State, page, value); System.Diagnostics.Debug.Assert(this[page] == value); }
            }
        }

        readonly PageReadPointerManager PageReadPointer;
        readonly PageWritePointerManager PageWritePointer;
        readonly PageReadFunctionManager PageReadFunction;
        readonly PageWriteFunctionManager PageWriteFunction;

        /// <summary>
        /// Current clock cycle
        /// </summary>
        public UInt64 Clock { get => Get_6809_Clock(State); set => Set_6809_Clock(State, value); }

        /// <summary>
        /// Gets/sets A register
        /// </summary>
        public byte A { get => Get_6809_A(State); set => Set_6809_A(State, value); }

        /// <summary>
        /// Gets/sets B register
        /// </summary>
        public byte B { get => Get_6809_B(State); set => Set_6809_B(State, value); }

        /// <summary>
        /// Gets/sets DP register
        /// </summary>
        public byte DP { get => Get_6809_DP(State); set => Set_6809_DP(State, value); }

        /// <summary>
        /// Gets/sets CC register
        /// </summary>
        public byte CC { get => Get_6809_B(State); set => Set_6809_CC(State, value); }

        /// <summary>
        /// Gets/sets D register
        /// </summary>
        public UInt16 D { get => Get_6809_D(State); set => Set_6809_D(State, value); }

        /// <summary>
        /// Gets/sets X register
        /// </summary>
        public UInt16 X { get => Get_6809_X(State); set => Set_6809_X(State, value); }

        /// <summary>
        /// Gets/sets Y register
        /// </summary>
        public UInt16 Y { get => Get_6809_Y(State); set => Set_6809_Y(State, value); }

        /// <summary>
        /// Gets/sets U register
        /// </summary>
        public UInt16 U { get => Get_6809_U(State); set => Set_6809_U(State, value); }

        /// <summary>
        /// Gets/sets SP register
        /// </summary>
        public UInt16 SP { get => Get_6809_SP(State); set => Set_6809_SP(State, value); }

        /// <summary>
        /// Gets/sets PC register
        /// </summary>
        public UInt16 PC { get => Get_6809_PC(State); set => Set_6809_PC(State, value); }


        /// <summary>
        /// Gets/sets NMI assert
        /// </summary>
        public bool NMI { get => Get_6809_NMI(State); set => Set_6809_NMI(State, value); }

        /// <summary>
        /// Gets/sets FIRQ assert
        /// </summary>
        public bool FIRQ { get => Get_6809_FIRQ(State); set => Set_6809_FIRQ(State, value); }

        /// <summary>
        /// Gets/sets IRQ assert
        /// </summary>
        public bool IRQ { get => Get_6809_IRQ(State); set => Set_6809_IRQ(State, value); }

        readonly IntPtr State;
        int mDisposed = 0;

        // prevent delegates from being garbage collected
        public readonly ReadDelegate UndefinedRead;
        public readonly WriteDelegate UndefinedWrite;

        public M6809E()
        {
            int size = Get_6809_StructSize();
            State = Marshal.AllocHGlobal(size);
            RtlZeroMemory(State, size);

            UndefinedRead = new M6809E.ReadDelegate((UInt16 address) =>
            {
                Debug.WriteLine($"Undefined read @ {address.HexString()}");
                return 0;
            });

            UndefinedWrite = new M6809E.WriteDelegate((UInt16 address, byte data) =>
            {
                Debug.WriteLine($"Undefined write @ {address.HexString()} = {data.HexString()}");
            });

            PageReadPointer = new PageReadPointerManager(State);
            PageWritePointer = new PageWritePointerManager(State);
            PageReadFunction = new PageReadFunctionManager(State);
            PageWriteFunction = new PageWriteFunctionManager(State);

            // initialize system to trap all reads and writes
            for (int page = 0; page < 256; page++)
            {
                PageReadPointer[(byte)page] = null;
                PageWritePointer[(byte)page] = null;
                PageReadFunction[(byte)page] = UndefinedRead;
                PageWriteFunction[(byte)page] = UndefinedWrite;
            }
        }

        ~M6809E()
        {
            Dispose();
        }

        public void Dispose()
        {
            int disposed = System.Threading.Interlocked.Exchange(ref mDisposed, 1);
            if (disposed == 0)
                Marshal.FreeHGlobal(State);
        }



        #region IO

        public void SetPageIO(byte page, byte* ptr)
        {
            System.Diagnostics.Debug.Assert(ptr != null);
            PageReadPointer[page] = ptr;
            PageWritePointer[page] = ptr;
            PageReadFunction[page] = null;
            PageWriteFunction[page] = null;
        }

        public void SetPageIO(byte page, byte* read, byte* write)
        {
            System.Diagnostics.Debug.Assert(read != null);
            System.Diagnostics.Debug.Assert(write != null);
            PageReadPointer[page] = read;
            PageWritePointer[page] = write;
            PageReadFunction[page] = null;
            PageWriteFunction[page] = null;
        }

        public void SetPageIO(byte startPage, byte endPage, byte* ptr)
        {
            System.Diagnostics.Debug.Assert(ptr != null);
            System.Diagnostics.Debug.Assert(startPage <= endPage);
            for (; ; startPage++, ptr += 0x100)
            {
                PageReadPointer[startPage] = ptr;
                PageWritePointer[startPage] = ptr;
                PageReadFunction[startPage] = null;
                PageWriteFunction[startPage] = null;
                if (startPage == endPage)
                    return;
            }
        }

        public void SetPageIO(byte startPage, byte endPage, byte* read, byte* write)
        {
            System.Diagnostics.Debug.Assert(read != null);
            System.Diagnostics.Debug.Assert(write != null);
            System.Diagnostics.Debug.Assert(startPage <= endPage);
            for (; ; startPage++, read += 0x100, write += 0x100)
            {
                PageReadPointer[startPage] = read;
                PageWritePointer[startPage] = write;
                PageReadFunction[startPage] = null;
                PageWriteFunction[startPage] = null;
                if (startPage == endPage)
                    return;
            }
        }

        public void SetPageIO(byte page, byte* read, M6809E.WriteDelegate write)
        {
            System.Diagnostics.Debug.Assert(read != null);
            System.Diagnostics.Debug.Assert(write != null);
            PageReadPointer[page] = read;
            PageWritePointer[page] = null;
            PageReadFunction[page] = null;
            PageWriteFunction[page] = write;
        }

        public void SetPageIO(byte page, M6809E.ReadDelegate read, byte* write)
        {
            System.Diagnostics.Debug.Assert(read != null);
            System.Diagnostics.Debug.Assert(write != null);
            PageReadPointer[page] = null;
            PageWritePointer[page] = write;
            PageReadFunction[page] = read;
            PageWriteFunction[page] = null;
        }

        public void SetPageIO(byte page, M6809E.ReadDelegate read, M6809E.WriteDelegate write)
        {
            System.Diagnostics.Debug.Assert(read != null);
            System.Diagnostics.Debug.Assert(write != null);
            PageReadPointer[page] = null;
            PageWritePointer[page] = null;
            PageReadFunction[page] = read;
            PageWriteFunction[page] = write;
        }

        public void SetPageIO(byte startPage, byte endPage, M6809E.ReadDelegate read, M6809E.WriteDelegate write)
        {
            System.Diagnostics.Debug.Assert(read != null);
            System.Diagnostics.Debug.Assert(write != null);
            System.Diagnostics.Debug.Assert(startPage <= endPage);
            for (; ; startPage++)
            {
                PageReadPointer[startPage] = null;
                PageWritePointer[startPage] = null;
                PageReadFunction[startPage] = read;
                PageWriteFunction[startPage] = write;
                if (startPage == endPage)
                    return;
            }
        }

        #endregion

        /// <summary>
        /// Resets the 6809 and registers a periodic callback
        /// </summary>
        /// <param name="callback">callback delegate which will be called periodically</param>
        /// <param name="cycles">callback will be called this number of cycles</param>
        public void Reset(PeriodicDelegate? callback, double cycles)
        {
            M6809_Reset(State, callback, cycles);
        }

        /// <summary>
        /// Resets the 6809
        /// </summary>
        void Reset()
        {
            Reset(null, 0);
        }

        /// <summary>
        /// Executes the specified number of machine cycles
        /// </summary>
        /// <param name="cyclesToRun">number of machine cycles to execute</param>
        /// <param name="cyclesRun">actual number of cycleas that were executed</param>
        /// <returns>result of execution</returns>
        public Result Run(UInt32 cyclesToRun, out UInt32 cyclesRun)
        {
            UInt32 start = (UInt32)Clock;
            Result result = M6809_Run(State, cyclesToRun);
            cyclesRun = (UInt32)Clock - start;
            if (result != Result.OK)
                Debug.WriteLine($"M6809E error = {result}");
            System.Diagnostics.Debug.Assert(result == Result.OK);
            return result;
        }

        public override string ToString()
        {
            return $"A={A.HexString()}  B={B.HexString()}  X={X.HexString()}  Y={Y.HexString()}  U={U.HexString()}  DP={DP.HexString()}  CC={CC.HexString()}  SP={SP.HexString()}  PC={PC.HexString()}";
        }

        public void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("Clock", Clock);
            info.AddValue("D", D);
            info.AddValue("X", X);
            info.AddValue("Y", Y);
            info.AddValue("U", U);
            info.AddValue("SP", SP);
            info.AddValue("PC", PC);
            info.AddValue("DP", DP);
            info.AddValue("CC", CC);
            info.AddValue("NMI", NMI);
            info.AddValue("FIRQ", FIRQ);
            info.AddValue("IRQ", IRQ);
        }
    }
}