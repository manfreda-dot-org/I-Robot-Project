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
using System.Diagnostics;
using System.Drawing;
using System.Runtime.Serialization;
using System.Threading;

namespace I_Robot.Emulation
{
    [Serializable]
    unsafe public class Machine : ISerializable, IDisposable
    {
        ///////////////////////////////////////////////////////////////////////////////

        // GAME CPU INFORMATION

        // Game is controlled by a Motorola B6809-E clocked at 1.512 MHz.

        // VIDEO CPU INFORMATION

        // VIDEO CLOCK
        // video clock runs at 5 MHz
        // horizontal dot clock counts 0-319
        // vertical dot clock counts 0-255

        // VIDEO INTERRUPTS
        // video hardware generates an INQ interrupt on the 6809.  These
        // interrupts occur 4 times per frame, on scan lines 48, 112,
        // 176 and 240.  These interrupts requests are latched, and are
        // typically cleared by the 6809 program in the IQR interrupt
        // service routine.  However, the video hardware will automatically
        // clear the IRQ assertion if it is not processed within 32 scan
        // lines.  Thus, the IRQ is deasserted on scan lines  80, 144, 208
        // and 16 (240 + 32 - 256).

        // VBLANK
        // the VBLANK signal is active low, and is asserted from
        // scan lines 240 - 255.

        ////////////////////////////////////////////////////////////////////////////////

        public static readonly Size NATIVE_RESOLUTION = new Size(Alphanumerics.COLUMNS * Alphanumerics.CHAR_WIDTH, Alphanumerics.VISIBLE_ROWS * Alphanumerics.CHAR_HEIGHT);

        // Video clocks 320 x 256 pixels per frame, at 5 MHz
        public const double DOT_CLOCK = 5000000;
        public const int HORIZONTAL_DOTS = 320;
        public const int VERTICAL_DOTS = 256;

        /// <summary>
        /// Approximately 61 FPS
        /// </summary>
        public const double VIDEO_REFRESH_HZ = (DOT_CLOCK / HORIZONTAL_DOTS / VERTICAL_DOTS);

        /// <summary>
        /// CPU crystal is 8 MHz
        /// </summary>
        public const double CPU_XTAL_HZ = 12069000;

        /// <summary>
        /// M6809E clock runs at 1.508625 MHz
        /// </summary>
        public const double CPU_CLOCK_HZ = CPU_XTAL_HZ / 8;

        /// <summary>
        /// Approx 24717 CPU cycles per video frame
        /// </summary>
        public const double CPU_CYCLES_PER_VIDEO_FRAME = (CPU_CLOCK_HZ / VIDEO_REFRESH_HZ);

        /// <summary>
        /// Approx 97 CPU cycles per scanline
        /// </summary>
        public const double CPU_CYCLES_PER_SCANLINE = CPU_CYCLES_PER_VIDEO_FRAME / VERTICAL_DOTS;

        /// <summary>
        /// this is the scanline that VBLANK is toggled on
        /// </summary>
        public const int VBLANK_SCANLINE = 240;

        public enum RESET_TYPE
        {
            COLD,
            WARM,
            WATCHDOG,
            USER,
        }

        /// <summary>
        /// Represents a hardware subsystem
        /// All subsystems must derive from this class
        /// </summary>
        [Serializable]
        public abstract class Subsystem : IDisposable, ISerializable
        {
            public readonly String Name;
            public readonly Machine Machine;

            public Subsystem(Machine machine, String name)
            {
                Name = name;
                Machine = machine;
                Machine.Subsystems.Add(this);
            }

            /// <summary>
            /// Disposes of the subsystem and it's resourcess
            /// </summary>
            abstract public void Dispose();

            /// <summary>
            /// Resets the subsystem
            /// </summary>
            abstract public void Reset();

            /// <summary>
            /// Populates a System.Runtime.Serialization.SerializationInfo with the data needed
            /// to serialize the target object
            /// </summary>
            /// <param name="info">The System.Runtime.Serialization.SerializationInfo to populate with data.</param>
            /// <param name="context">The destination (see System.Runtime.Serialization.StreamingContext) for this serialization.</param>
            abstract public void GetObjectData(SerializationInfo info, StreamingContext context);
        }

        class SpeedThrottler
        {
            // 32.32 fixed point
            static Int64 TicksPerM6809Clock = (Int64)(0x100000000 * Stopwatch.Frequency / CPU_CLOCK_HZ);

            Stopwatch Clock = new Stopwatch();
            UInt32 Ticks = 0;
            Int64 AccumulatedTicks = 0;

            public void Reset()
            {
                Clock.Restart();
                Ticks = 0;
                AccumulatedTicks = 0;
            }

            public long M6809CyclesElapsed
            {
                get
                {
                    // determine how many ticks have elapsed
                    UInt32 prev = Ticks;
                    Ticks = (UInt32)Clock.ElapsedTicks;
                    UInt32 elapsed_ticks = Ticks - prev;

                    // accumulate running ticks
                    AccumulatedTicks += ((Int64)elapsed_ticks) << 32;

                    UInt32 clocks = (UInt32)Math.DivRem(AccumulatedTicks, TicksPerM6809Clock, out Int64 remainder);
                    if (clocks != 0)
                    {
                        //                        System.Diagnostics.Debug.WriteLine($"clocks {clocks}");
                        AccumulatedTicks = remainder;
                    }

                    return clocks;
                }
            }
        }

        public readonly LEDs LEDs;
        public readonly CoinCounters CoinCounters;

        public readonly M6809E M6809E = new M6809E();

        public readonly RAM_0000 RAM_0000;
        public readonly RAM_0800 RAM_0800;
        public readonly EEPROM EEPROM;
        public readonly Bank_2000 Bank_2000;
        public readonly ProgROM ProgROM;

        public readonly ADC ADC;
        public readonly Alphanumerics Alphanumerics;
        public readonly Mathbox Mathbox;
        public readonly Palette Palette;
        public readonly Quad_POKEY Quad_POKEY;
        public readonly Registers Registers;
        public readonly VideoProcessor VideoProcessor;

        readonly List<Subsystem> Subsystems = new List<Subsystem>();

        readonly SpeedThrottler Throttle = new SpeedThrottler();

        byte mScanline = 0;
        public byte Scanline
        {
            get => mScanline;
            private set
            {
                mScanline = value;
                //  IRQ asserted on lines:   48, 112, 176, 240
                //  IRQ deasserted on lines: 16,  80, 144, 208
                if ((Scanline & 0x10) == 0x10)
                    M6809E.IRQ = (((Scanline - 16) & 0x20) != 0);
            }
        }

        public bool VBLANK { get { return Scanline >= VBLANK_SCANLINE; } }

        public bool Paused = false;

        Int32 CyclesToRun = 0;

        const byte CALLBACK_SCANLINES = 16;

        readonly M6809E.PeriodicDelegate ScanlineCallback;

        readonly Thread Thread;
        readonly object Lock = new object();
        bool mDisposed = false;

        public double FPS { get; private set; }
        readonly TimeAccumulator FPS_Time = new TimeAccumulator();
        int FrameCount = 0;


        class TimeAccumulator
        {
            Stopwatch Stopwatch = new Stopwatch();

            long Base = 0;

            public TimeAccumulator()
            {
                Restart();
            }

            public void Restart()
            {
                Stopwatch.Restart();
                Base = 0;
            }

            public long ElapsedMilliseconds
            {
                get { return Stopwatch.ElapsedMilliseconds - Base; }
            }

            public void RemoveTime(int time)
            {
                Base += time;
            }
        }

        public readonly RomSet Roms;

        public Machine(RomSet roms, IRasterizer rasterizer)
        {
            Roms = roms;

            CoinCounters = new CoinCounters(this);
            LEDs = new LEDs(this);

            Quad_POKEY = new Quad_POKEY(this);

            ProgROM = new ProgROM(this);

            ADC = new ADC(this);
            Alphanumerics = new Alphanumerics(this);
            Mathbox = new Mathbox(this, rasterizer);
            Palette = new Palette(this);
            Registers = new Registers(this);
            VideoProcessor = new VideoProcessor(this);

            RAM_0000 = new RAM_0000(this);
            RAM_0800 = new RAM_0800(this);
            EEPROM = new EEPROM(this);
            Bank_2000 = new Bank_2000(this);

            // let the interpreter know about this hardware
            // must be done after subsystems are loaded
            rasterizer.Machine = this;

            // setup a periodic callback from the 6809 engine to update scanline counter
            ScanlineCallback = new M6809E.PeriodicDelegate(() =>
            {
                byte prev = Scanline;
                Scanline += CALLBACK_SCANLINES;
                if (Scanline < prev)
                {
                    FrameCount++;
                    long time_ms = FPS_Time.ElapsedMilliseconds;
                    if (time_ms >= 1000)
                    {
                        FPS = 1000.0 * FrameCount / time_ms;
                        FrameCount = 0;
                        //FPS_Time.RemoveTime(1000);
                        FPS_Time.Restart();
                    }
                }
                Quad_POKEY.Update();
            });

            Reset(Machine.RESET_TYPE.COLD);

            Thread = new Thread(EmulationThread);
            Thread.IsBackground = true;
            Thread.SetApartmentState(ApartmentState.STA);
            Thread.Start();
        }

        public void Dispose()
        {
            if (!mDisposed)
            {
                lock (Lock)
                {
                    if (!mDisposed)
                    {
                        mDisposed = true;

                        foreach (Subsystem subsystem in Subsystems)
                        {
                            try { subsystem.Dispose(); }
                            catch { }
                        }

                        try { M6809E.Dispose(); }
                        catch { }
                    }
                }
            }
        }

        public void Reset(RESET_TYPE type)
        {
            lock (Lock)
            {
                if (mDisposed)
                    return;

                System.Diagnostics.Debug.WriteLine($"Machine reset: {type}");

                foreach (Subsystem subsystem in Subsystems)
                {
                    System.Diagnostics.Debug.WriteLine($"\tReset: {subsystem.Name}");
                    subsystem.Reset();
                }

                M6809E.Reset(ScanlineCallback, CPU_CYCLES_PER_SCANLINE * CALLBACK_SCANLINES);

                Scanline = 0;
                CyclesToRun = (int)CPU_CYCLES_PER_VIDEO_FRAME;

                Throttle.Reset();
            }
        }

        void EmulationThread()
        {
            for (; ; )
            {
                Thread.Sleep(2);

                lock (Lock)
                {
                    if (mDisposed)
                        return;

                    if (Paused)
                    {
                        CyclesToRun = 0;
                        continue;
                    }

                    Quad_POKEY.Update();

                    if (Registers.WatchdogCounter > 500000)
                        Reset(Machine.RESET_TYPE.WATCHDOG);

                    // compute number of cycles to run
                    if (Settings.SpeedThrottle)
                    {
                        CyclesToRun += (Int32)Throttle.M6809CyclesElapsed;

                        // don't get too far ahead of ourselves
                        if (CyclesToRun > (Int32)CPU_CLOCK_HZ)
                            CyclesToRun = (Int32)CPU_CLOCK_HZ;
                    }
                    else
                        CyclesToRun = (Int32)CPU_CYCLES_PER_VIDEO_FRAME;

                    // execute 6809
                    while (CyclesToRun > 0)
                    {
                        // run and subtract off any consumed cycles
                        M6809E.Result result = M6809E.Run((UInt32)CyclesToRun, out UInt32 cycles);
                        CyclesToRun -= (int)cycles;
                    }
                }
            }
        }

        void ISerializable.GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("M6809E", M6809E);
            foreach (Subsystem subsystem in Subsystems)
            {
                info.AddValue(subsystem.Name, subsystem);
            }
        }
    }
}