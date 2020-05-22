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
using Vortice.Multimedia;
using Vortice.XAudio2;

namespace I_Robot.Emulation
{
    /// <summary>
    /// Emulates the Quad-Pokey chip on the I, Robot PCB
    /// Heavily based on Ron Fries POKEY Chip Emulator V2.3
    /// </summary>
    [Serializable]
    unsafe public class Quad_POKEY : Machine.Subsystem
    {
        // As an alternative to using the exact frequencies, selecting a playback
        // frequency that is an exact division of the main clock provides a higher
        // quality output due to less aliasing.  For best results, a value of
        // 1787520 MHz is used for the main clock.  With this value, both the
        // 64 kHz and 15 kHz clocks are evenly divisible.  Selecting a playback
        // frequency that is also a division of the clock provides the best
        // results.  The best options are FREQ_64 divided by either 2, 3, or 4.
        // The best selection is based on a trade off between performance and
        // sound quality.
        //
        // Of course, using a main clock frequency that is not exact will affect
        // the pitch of the output.  With these numbers, the pitch will be low
        // by 0.127%.  (More than likely, an actual unit will vary by this much!)

        const int POKEY_FREQ = POKEY.FREQ_17_APPROX;
        const int PLAYBACK_FREQ_HZ = POKEY_FREQ / 40;

        /// <summary>
        /// POKEY emulation
        /// Based on Ron Fries POKEY Chip Emulator V2.3
        /// </summary>
        [Serializable]
        unsafe class POKEY : ISerializable
        {
            // POKEY WRITE LOGICALS
            // Note: only writes from 0x00 - 0x08 are emulated here
            public const byte AUDF1_C = 0x00;
            public const byte AUDC1_C = 0x01;
            public const byte AUDF2_C = 0x02;
            public const byte AUDC2_C = 0x03;
            public const byte AUDF3_C = 0x04;
            public const byte AUDC3_C = 0x05;
            public const byte AUDF4_C = 0x06;
            public const byte AUDC4_C = 0x07;
            public const byte AUDCTL_C = 0x08;

            public const int FREQ_17_EXACT = 1789790;  // exact 1.79 MHz clock freq
            public const int FREQ_17_APPROX = 1787520;  // approximate 1.79 MHz clock freq

            // for accuracy, the 64kHz and 15kHz clocks are exact divisions of the 1.79MHz clock
            const uint DIV_64 = 28;  // divisor for 1.79MHz clock to 64 kHz
            const uint DIV_15 = 114; // divisor for 1.79MHz clock to 15 kHz

            // the size (in entries) of the 4 polynomial tables
            const int POLY4_SIZE = 0x000f;
            const int POLY5_SIZE = 0x001f;
            const int POLY9_SIZE = 0x01ff;
            const int POLY17_SIZE = 0x0001ffff;

            // channel/chip definitions
            const byte CHAN1 = 0;
            const byte CHAN2 = 1;
            const byte CHAN3 = 2;
            const byte CHAN4 = 3;

            /// <summary>
            /// Polynomial 4 bit pattern, identical to that used in chip
            /// </summary>
            static readonly bool[] bit4 = new bool[POLY4_SIZE] { true, true, false, true, true, true, false, false, false, false, true, false, true, false, false };

            /// <summary>
            /// Polynomial 5 bit pattern, identical to that used in chip
            /// </summary>
            static readonly bool[] bit5 = new bool[POLY5_SIZE] { false, false, true, true, false, false, false, true, true, true, true, false, false, true, false, true, false, true, true, false, true, true, true, false, true, false, false, false, false, false, true };

            /// <summary>
            /// Polynomial 17 bit pattern, identical to that used in chip
            /// </summary>
            static readonly bool[] bit17 = new bool[POLY17_SIZE];

            /// <summary>
            /// One time initialization of POKEY at startup
            /// Sets the bit pattern for polynomial 17
            /// </summary>
            static POKEY()
            {
                // Initialze the bit patterns for the polynomial 17
                int poly17 = 0;
                for (int i = 0; i < POLY17_SIZE; ++i)
                {
                    poly17 = (poly17 >> 1) + (~((poly17 << 16) ^ (poly17 << 11)) & 0x10000);
                    bit17[i] = ((poly17 & 0x100) != 0);
                }
            }

            /// <summary>
            /// Audio channel control register
            /// </summary>
            struct REGISTER_AUDC
            {
                public byte Value;
                public REGISTER_AUDC(byte value) { Value = value; }

                /// <summary>
                /// selects POLY5 or direct CLOCK
                /// </summary>
                public bool NOTPOLY5
                {
                    get { return (Value & 0x80) != 0; }
                    set { if (value) Value |= 0x80; else Value &= 0x7F; }
                }

                /// <summary>
                /// selects POLY4 or POLY17
                /// </summary>
                public bool POLY4
                {
                    get { return (Value & 0x40) != 0; }
                    set { if (value) Value |= 0x40; else Value &= 0xBF; }
                }

                /// <summary>
                /// selects POLY4/17 or PURE tone
                /// </summary>
                public bool PURE
                {
                    get { return (Value & 0x20) != 0; }
                    set { if (value) Value |= 0x20; else Value &= 0xDF; }
                }

                /// <summary>
                /// selects VOLUME OUTPUT ONLY
                /// </summary>
                public bool VOL_ONLY
                {
                    get { return (Value & 0x10) != 0; }
                    set { if (value) Value |= 0x10; else Value &= 0xEF; }
                }

                /// <summary>
                /// channel volume 0 - F
                /// </summary>
                public byte VOL
                {
                    get { return (byte)(Value & 0xF); }
                    set { Value = (byte)((Value & 0xF0) | (value & 0xF)); }
                }

                public static implicit operator byte(REGISTER_AUDC r) => r.Value;
                public static implicit operator REGISTER_AUDC(byte b) => new REGISTER_AUDC(b);
                public override string ToString() { return Value.HexString(); }
            }

            /// <summary>
            /// Audio control register
            /// </summary>
            struct REGISTER_AUDCTL
            {
                public byte Value;
                public REGISTER_AUDCTL(byte value) { Value = value; }

                /// <summary>
                /// selects POLY9 or POLY17
                /// </summary>
                public bool POLY9
                {
                    get { return (Value & 0x80) != 0; }
                    set { if (value) Value |= 0x80; else Value &= 0x7F; }
                }

                /// <summary>
                /// sets channel 1 frequency (true = 1.78979 MHz NTSC, false = 64 kHz)
                /// </summary>
                public bool CH1_179
                {
                    get { return (Value & 0x40) != 0; }
                    set { if (value) Value |= 0x40; else Value &= 0xBF; }
                }

                /// <summary>
                /// sets channel 3 frequency (true = 1.78979 MHz NTSC, false = 64 kHz)
                /// </summary>
                public bool CH3_179
                {
                    get { return (Value & 0x20) != 0; }
                    set { if (value) Value |= 0x20; else Value &= 0xDF; }
                }

                /// <summary>
                /// connection of dividers 2+1 to obtain 16-bit accuracy
                /// </summary>
                public bool CH1_CH2
                {
                    get { return (Value & 0x10) != 0; }
                    set { if (value) Value |= 0x10; else Value &= 0xEF; }
                }

                /// <summary>
                /// connection of dividers 4+3 to obtain 16-bit accuracy
                /// </summary>
                public bool CH3_CH4
                {
                    get { return (Value & 0x08) != 0; }
                    set { if (value) Value |= 0x08; else Value &= 0xF7; }
                }

                /// <summary>
                /// high-pass filter for channel 1 rated by frequency of channel 3
                /// </summary>
                public bool CH1_FILTER
                {
                    get { return (Value & 0x04) != 0; }
                    set { if (value) Value |= 0x04; else Value &= 0xFB; }
                }

                /// <summary>
                /// high-pass filter for channel 2 rated by frequency of channel 4
                /// </summary>
                public bool CH2_FILTER
                {
                    get { return (Value & 0x02) != 0; }
                    set { if (value) Value |= 0x02; else Value &= 0xFD; }
                }

                /// <summary>
                /// choice of frequency divider rate (true = 15 kHz, false = 64 kHz)
                /// </summary>
                public bool CLOCK_15
                {
                    get { return (Value & 0x01) != 0; }
                    set { if (value) Value |= 0x01; else Value &= 0xFE; }
                }

                public static implicit operator byte(REGISTER_AUDCTL r) => r.Value;
                public static implicit operator REGISTER_AUDCTL(byte b) => new REGISTER_AUDCTL(b);
                public override string ToString() { return Value.HexString(); }
            }

            /// <summary>
            /// Represents one of the 4 channels in a POKEY
            /// </summary>
            [Serializable]
            class ChannelRegisters : ISerializable
            {
                public readonly POKEY POKEY;
                public readonly int Number;

                // last output volume
                public bool OutputOn = false;

                byte mAUDF = 0;
                public byte AUDF
                {
                    get => mAUDF;
                    set
                    {
                        mAUDF = value;
                        Update();

                        switch (Number)
                        {
                            case CHAN1: if (POKEY.AUDCTL.CH1_CH2) POKEY.Channel[CHAN2].Update(); break;
                            case CHAN3: if (POKEY.AUDCTL.CH3_CH4) POKEY.Channel[CHAN4].Update(); break;
                        }
                    }
                }

                REGISTER_AUDC mAUDC = 0;
                public REGISTER_AUDC AUDC
                {
                    get => mAUDC;

                    set
                    {
                        mAUDC = value;

                        // hack the channel to have 2x volume
                        // this allows us to retain the fractional LSBs when AUDV is divided
                        AUDV = (byte)(value.VOL << 1);

                        Update();
                    }
                }

                public byte AUDV { get; private set; }

                // Divide by n counter
                public UInt32 Div_n_cnt = 0;

                // Divide by n maximum
                UInt32 mDiv_n_max = 0x7fffffff;

                public UInt32 Div_n_max
                {
                    get => mDiv_n_max;
                    set
                    {
                        if (mDiv_n_max != value)
                        {
                            mDiv_n_max = value;

                            if (Div_n_cnt > value)
                                Div_n_cnt = value;
                        }

                    }
                }

                public ChannelRegisters(POKEY pokey, int channel)
                {
                    POKEY = pokey;
                    Number = channel;
                }

                public void GetObjectData(SerializationInfo info, StreamingContext context)
                {
                    info.AddValue("Number", Number);
                    info.AddValue("OutputOn", OutputOn);
                    info.AddValue("AUDF", AUDF);
                    info.AddValue("AUDC", AUDC);
                    info.AddValue("Div_n_cnt", Div_n_cnt);
                    info.AddValue("Div_n_max", Div_n_max);
                }

                public void Reset()
                {
                    AUDF = 0;
                    AUDC = 0;
                    Div_n_cnt = 0;
                    mDiv_n_max = 0x7fffffff;
                    OutputOn = false;
                }

                void Update()
                {
                    // As defined in the manual, the exact Div_n_cnt values are
                    // different depending on the frequency and resolution:    
                    //    64 kHz or 15 kHz - AUDF + 1                          
                    //    1 MHz, 8-bit -     AUDF + 4                          
                    //    1 MHz, 16-bit -    AUDF[CHAN1]+256*AUDF[CHAN2] + 7   

                    switch (Number)
                    {
                        default: throw new IndexOutOfRangeException();
                        case CHAN1:
                            // process channel 1 frequency
                            if (POKEY.AUDCTL.CH1_179)
                                Div_n_max = (UInt32)(AUDF + 4);
                            else
                                Div_n_max = (UInt32)((AUDF + 1) * POKEY.Base_mult);
                            break;
                        case CHAN2:
                            // process channel 2 frequency
                            if (!POKEY.AUDCTL.CH1_CH2)
                                Div_n_max = (UInt32)((AUDF + 1) * POKEY.Base_mult);
                            else if (POKEY.AUDCTL.CH1_179)
                                Div_n_max = (UInt32)(AUDF * 256 + POKEY.Channel[CHAN1].AUDF + 7);
                            else
                                Div_n_max = (UInt32)((AUDF * 256 + POKEY.Channel[CHAN1].AUDF + 1) * POKEY.Base_mult);
                            break;
                        case CHAN3:
                            // process channel 3 frequency
                            if (POKEY.AUDCTL.CH3_179)
                                Div_n_max = (UInt32)(AUDF + 4);
                            else
                                Div_n_max = (UInt32)((AUDF + 1) * POKEY.Base_mult);
                            break;
                        case CHAN4:
                            // process channel 4 frequency
                            if (!POKEY.AUDCTL.CH3_CH4)
                                Div_n_max = (UInt32)((AUDF + 1) * POKEY.Base_mult);
                            else if (POKEY.AUDCTL.CH3_179)
                                Div_n_max = (UInt32)(AUDF * 256 + POKEY.Channel[CHAN3].AUDF + 7);
                            else
                                Div_n_max = (UInt32)((AUDF * 256 + POKEY.Channel[CHAN3].AUDF + 1) * POKEY.Base_mult);
                            break;
                    }
                    SetOutput();
                }

                public void SetOutput()
                {
                    // I've disabled any frequencies that exceed the sampling
                    // frequency.  There isn't much point in processing frequencies
                    // that the hardware can't reproduce.  I've also disabled
                    // processing if the volume is zero.

                    // if the channel is volume only
                    // or the channel is off (volume == 0)
                    // or the channel freq is greater than the playback freq
                    if (AUDC.VOL_ONLY || (AUDC.VOL == 0) || (Div_n_max < (POKEY.Samp_n_max >> 32)))
                    {
                        // indicate the channel is 'on'
                        OutputOn = true;
                        // and set channel freq to max to reduce processing
                        Div_n_max = 0x7fffffff;
                        Div_n_cnt = 0x7fffffff;
                    }
                }
            }

            // pokey registers
            readonly ChannelRegisters[] Channel = new ChannelRegisters[4];
            REGISTER_AUDCTL mAUDCTL = 0;

            // Sample max.  For accuracy, it is 32.32 fixed point
            readonly UInt64 Samp_n_max;
            UInt64 Samp_n_cnt;

            // selects either 64Khz or 15Khz clock mult
            UInt32 Base_mult = DIV_64;

            // polynomial counters
            UInt32 Poly_adjust = 0;                 // the amount that the polynomial will need to be adjusted to process the next bit
            UInt32 P4 = 0;                          // Global position pointer for the 4-bit  POLY array
            UInt32 P5 = 0;                          // Global position pointer for the 5-bit  POLY array
            UInt32 P9 = 0;							// Global position pointer for the 9-bit  POLY array
            UInt32 P17 = 0;                         // Global position pointer for the 17-bit POLY array

            /// <summary>
            /// to handle the power-up initialization functions
            /// these functions should only be executed on a cold-restart
            /// </summary>
            /// <param name="freq17">the value for the '1.79MHz' Pokey audio clock</param>
            /// <param name="playback_freq">the playback frequency in samples per second</param>
            public POKEY(uint freq17, uint playback_freq)
            {
                // calculate the sample 'divide by N' value based on the playback freq.
                Samp_n_max = ((UInt64)freq17 << 32) / playback_freq;

                // create the 4 channels
                for (int chan = 0; chan < Channel.Length; chan++)
                    Channel[chan] = new ChannelRegisters(this, chan);

                Reset();
            }

            public void GetObjectData(SerializationInfo info, StreamingContext context)
            {
                info.AddValue("Samp_n_max", Samp_n_max);
                info.AddValue("Samp_n_cnt", Samp_n_cnt);
                info.AddValue("Base_mult", Base_mult);
                info.AddValue("Poly_adjust", Poly_adjust);
                info.AddValue("P4", P4);
                info.AddValue("P5", P5);
                info.AddValue("P9", P9);
                info.AddValue("P17", P17);
                info.AddValue("AUDCTL", (byte)AUDCTL);
                info.AddValue("Channel[0]", Channel[0]);
                info.AddValue("Channel[1]", Channel[1]);
                info.AddValue("Channel[2]", Channel[2]);
                info.AddValue("Channel[3]", Channel[3]);
            }

            /// <summary>
            /// Resets the POKEY chip
            /// </summary>
            public void Reset()
            {
                foreach (var ch in Channel)
                    ch.Reset();

                AUDCTL = 0;
                Poly_adjust = 0;
                P4 = 0;
                P5 = 0;
                P9 = 0;
                P17 = 0;
                Samp_n_cnt = 0;
                Base_mult = DIV_64;
            }

            REGISTER_AUDCTL AUDCTL
            {
                get => mAUDCTL;
                set
                {
                    mAUDCTL = value;

                    // determine the base multiplier for the 'div by n' calculations
                    Base_mult = (value.CLOCK_15 ? DIV_15 : DIV_64);

                    foreach (var ch in Channel)
                        ch.SetOutput();
                }
            }

            /// <summary>
            /// To process the latest control values stored in the AUDF, AUDC, and AUDCTL registers.
            /// It pre-calculates as much information as possible for better performance.
            /// This routine has not been optimized.
            /// </summary>
            /// <param name="addr">the address of the parameter to be changed</param>
            /// <param name="val">the new value to be placed in the specified address</param>
            public void Write(UInt16 addr, byte val)
            {
                // determine which address was changed
                switch (addr & 0x0f)
                {
                    case AUDF1_C: Channel[CHAN1].AUDF = val; return;
                    case AUDC1_C: Channel[CHAN1].AUDC = val; return;
                    case AUDF2_C: Channel[CHAN2].AUDF = val; return;
                    case AUDC2_C: Channel[CHAN2].AUDC = val; return;
                    case AUDF3_C: Channel[CHAN3].AUDF = val; return;
                    case AUDC3_C: Channel[CHAN3].AUDC = val; return;
                    case AUDF4_C: Channel[CHAN4].AUDF = val; return;
                    case AUDC4_C: Channel[CHAN4].AUDC = val; return;
                    case AUDCTL_C: AUDCTL = val; return;
                }
            }


            /// <summary>
            /// To fill the output buffer with the sound output based on the pokey chip parameters.
            /// </summary>
            /// <param name="buffer">pointer to the buffer where the audio output will be placed</param>
            /// <param name="count">number of samples to write to the buffer</param>
            public void FillAudioBuffer(byte* buffer, int count)
            {
                // The current output is pre-determined and then adjusted based on each
                // output change for increased performance (less over-all math).

                // add the output values of all 4 channels
                byte sample = 128;
                foreach (var ch in Channel)
                {
                    sample -= (byte)(ch.AUDV >> 1);
                    if (ch.OutputOn) sample += ch.AUDV;
                }

                // loop until the buffer is filled
                while (count > 0)
                {
                    // Normally the routine would simply decrement the 'div by N'
                    // counters and react when they reach zero.  Since we normally
                    // won't be processing except once every 80 or so counts,
                    // I've optimized by finding the smallest count and then
                    // 'accelerated' time by adjusting all pointers by that amount.

                    // find next smallest event (either sample or chan 1-4)
                    ChannelRegisters? chEvent = null;
                    UInt32 event_min = (UInt32)(Samp_n_cnt >> 32);

                    // find the smallest event count
                    if (Channel[0].Div_n_cnt <= event_min) { event_min = Channel[0].Div_n_cnt; chEvent = Channel[0]; }
                    if (Channel[1].Div_n_cnt <= event_min) { event_min = Channel[1].Div_n_cnt; chEvent = Channel[1]; }
                    if (Channel[2].Div_n_cnt <= event_min) { event_min = Channel[2].Div_n_cnt; chEvent = Channel[2]; }
                    if (Channel[3].Div_n_cnt <= event_min) { event_min = Channel[3].Div_n_cnt; chEvent = Channel[3]; }

                    // decrement all counters by the smallest count found
                    // again, no loop for efficiency
                    Channel[0].Div_n_cnt -= event_min;
                    Channel[1].Div_n_cnt -= event_min;
                    Channel[2].Div_n_cnt -= event_min;
                    Channel[3].Div_n_cnt -= event_min;

                    Samp_n_cnt -= (UInt64)event_min << 32;

                    // since the polynomials require a mod (%) function which is division,
                    // I don't adjust the polynomials on the SAMPLE events, only the CHAN
                    // events.  I have to keep track of the change, though.
                    Poly_adjust += event_min;

                    // if the next event is a channel change
                    if (chEvent != null)
                    {
                        // shift the polynomial counters
                        P4 = (P4 + Poly_adjust) % POLY4_SIZE;
                        P5 = (P5 + Poly_adjust) % POLY5_SIZE;
                        P9 = (P9 + Poly_adjust) % POLY9_SIZE;
                        P17 = (P17 + Poly_adjust) % POLY17_SIZE;

                        // reset the polynomial adjust counter to zero
                        Poly_adjust = 0;

                        // adjust channel counter
                        chEvent.Div_n_cnt += chEvent.Div_n_max;

                        // assume no changes to the output
                        bool toggle = false;

                        // From here, a good understanding of the hardware is required
                        // to understand what is happening.  I won't be able to provide
                        // much description to explain it here.

                        // if the output is pure or the output is poly5 and the poly5 bit is set
                        if (chEvent.AUDC.NOTPOLY5 || bit5[P5])
                        {
                            // if the PURE bit is set
                            if (chEvent.AUDC.PURE)
                                toggle = true; // then simply toggle the output
                            // otherwise if POLY4 is selected
                            else if (chEvent.AUDC.POLY4)
                                toggle = (bit4[P4] != chEvent.OutputOn); // then compare to the poly4 bit
                            // if 9-bit poly is selected on this chip
                            else if (AUDCTL.POLY9)
                                toggle = (bit17[P9] != chEvent.OutputOn); // compare to the poly9 bit
                            else
                                toggle = (bit17[P17] != chEvent.OutputOn); // otherwise compare to the poly17 bit
                        }

                        // At this point I haven't emulated the filters.  Though I don't
                        // expect it to be complicated, I don't believe this feature is
                        // used much anyway.  I'll work on it later.
                        // if ((chEvent == CHAN1) || (chEvent == CHAN3))
                        // {                                                  
                        //    INSERT FILTER HERE                              
                        // }                                                  

                        // if the current output bit has changed
                        if (toggle)
                        {
                            if (chEvent.OutputOn)
                            {
                                // remove this channel from the signal
                                sample -= chEvent.AUDV;

                                // and turn the output off
                                chEvent.OutputOn = false;
                            }
                            else
                            {
                                // turn the output on
                                chEvent.OutputOn = true;

                                // and add it to the output signal
                                sample += chEvent.AUDV;
                            }
                        }
                        continue;
                    }

                    // no more events, so process samples

                    // adjust the sample counter - note we're using the 32.32 fixed point
                    Samp_n_cnt += Samp_n_max;

                    *buffer++ = sample;

                    // and indicate one less sample to make
                    count--;
                }
            }
        }

        /// <summary>
        /// An XAudio2.AudioBuffer linked to pinned memory
        /// This enables fast writing to the buffer
        /// </summary>
        unsafe class PinnedAudioBuffer : AudioBuffer
        {
            /// <summary>
            /// Provides an unsafe pointer which can be used to access the raw data
            /// </summary>
            public byte* pBuffer => (byte*)base.AudioDataPointer;

            /// <summary>
            /// The number of audio samples this buffer can hold
            /// </summary>
            public int NumSamples => SizeInBytes;

            /// <summary>
            /// The size, in bytes, of this audio buffer
            /// </summary>
            public int SizeInBytes => base.AudioBytes;

            /// <summary>
            /// Creates a PinnedAudioBuffer containing the specified number of samples
            /// </summary>
            /// <param name="num_samples"></param>
            public PinnedAudioBuffer(int num_samples) : base(num_samples, BufferFlags.None)
            {
                Util.Memset(AudioDataPointer, 0, SizeInBytes);
            }
        }

        /// <summary>
        /// Manages a pool of PinnedAudioBuffers
        /// Buffers can be taken from the pool, filled, then enqueued for playback in real time
        /// </summary>
        class BufferPool : IDisposable
        {
            readonly PinnedAudioBuffer[] Buffers;
            public readonly int Count;
            int Index;

            /// <summary>
            /// Creates a pool of PinnedAudioBuffers
            /// </summary>
            /// <param name="count"></param>
            /// <param name="size"></param>
            public BufferPool(int count, int size)
            {
                System.Diagnostics.Debug.Assert(count > 1);
                System.Diagnostics.Debug.Assert(size > 0);

                Count = count;
                Buffers = new PinnedAudioBuffer[count];
                for (int n = 0; n < count; n++)
                    Buffers[n] = new PinnedAudioBuffer(size);
            }

            ~BufferPool()
            {
                Dispose();
            }

            public void Dispose()
            {
                foreach (var buffer in Buffers)
                    buffer.Dispose();
            }

            /// <summary>
            /// Gets the next PinnedAudioBuffer from the pool
            /// </summary>
            /// <returns></returns>
            public PinnedAudioBuffer GetNextBuffer()
            {
                PinnedAudioBuffer next = Buffers[Index++ % Buffers.Length];
                next.PlayBegin = 0;
                return next;
            }
        }

        /// <summary>
        /// An XAudio2.SourceVoice wrapped around a POKEY
        /// </summary>
        class PokeySourceVoice : IDisposable
        {
            public static implicit operator IXAudio2SourceVoice(PokeySourceVoice v) => v.SourceVoice;

            public readonly POKEY POKEY = new POKEY(POKEY_FREQ, PLAYBACK_FREQ_HZ);
            readonly BufferPool Pool = new BufferPool(32, (int)(PLAYBACK_FREQ_HZ * 0.001));
            readonly IXAudio2SourceVoice SourceVoice;

            public PokeySourceVoice(IXAudio2 device)
            {
                SourceVoice = device.CreateSourceVoice(new WaveFormat(PLAYBACK_FREQ_HZ, 8, 1));
            }

            public VoiceState State => SourceVoice.State;
            public void Start() => SourceVoice.Start();
            public void Stop() => SourceVoice.Stop();

            /// <summary>
            /// Gets the next audio buffer and fills it with POKEY output
            /// </summary>
            public void FillNextBuffer()
            {
                PinnedAudioBuffer next = Pool.GetNextBuffer();
                POKEY.FillAudioBuffer(next.pBuffer, next.NumSamples);

                // and enqueue for playback
                SourceVoice.SubmitSourceBuffer(next);
            }

            public void Dispose()
            {
                SourceVoice.Dispose();
                Pool.Dispose();
            }
        }

        // random number generator used when reading from POKEY
        readonly Random RNG = new Random();

        readonly IXAudio2 Device;
        readonly IXAudio2MasteringVoice MasteringVoice;

        // driver has 4 POKEY voices
        readonly PokeySourceVoice[] Voice = new PokeySourceVoice[4];

        // M6809E delgates passed into the CPU emulation core
        // we have to hold on to these to prevent garbage collection
        readonly M6809E.ReadDelegate Read14xx;
        readonly M6809E.WriteDelegate Write14xx;

        /// <summary>
        /// Creates a new Quad POKEY subsystem for the hardware
        /// </summary>
        /// <param name="machine"></param>
        public Quad_POKEY(Machine machine) : base(machine, "Quad POKEY")
        {
            // setup I/O delegates

            Read14xx = new M6809E.ReadDelegate((UInt16 address) =>
            {
                // only address bits 0-5 are used
                address &= 0x3F;

                if (address == 0x20)
                    return Settings.DipSwitch5E;

                return (byte)RNG.Next();
            });

            Write14xx = new M6809E.WriteDelegate((UInt16 address, byte data) =>
            {
                // lower 6 bits of address bits dictate POKEY register and CHIP
                // --rccrrr
                byte chip = (byte)((address >> 3) & 0x03);
                Voice[chip].POKEY.Write((UInt16)((address & 0x07) + ((address & 0x20) >> 2)), data);
            });

            // startup the XAudio2 engine and pass it a few empty buffers to start playing
            Device = new IXAudio2();
            MasteringVoice = Device.CreateMasteringVoice(1, PLAYBACK_FREQ_HZ, AudioStreamCategory.GameEffects);

            // create the 4 pokey chips
            for (int chip = 0; chip < Voice.Length; chip++)
                Voice[chip] = new PokeySourceVoice(Device);
        }

        public override void Dispose()
        {
            foreach (PokeySourceVoice voice in Voice)
                voice.Stop();
            Device.Dispose();
            MasteringVoice.Dispose();
            foreach (PokeySourceVoice voice in Voice)
                voice.Dispose();
        }

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("POKEY[0]", Voice[0].POKEY);
            info.AddValue("POKEY[1]", Voice[1].POKEY);
            info.AddValue("POKEY[2]", Voice[2].POKEY);
            info.AddValue("POKEY[3]", Voice[3].POKEY);
        }

        /// <summary>
        /// Resets the Quad POKEY
        /// </summary>
        public override void Reset()
        {
            foreach (var voice in Voice)
                voice.Stop();

            Machine.M6809E.SetPageIO(0x14, Read14xx, Write14xx);

            foreach (PokeySourceVoice voice in Voice)
                voice.POKEY.Reset();

            Update();

            // start playing sounds
            foreach (var voice in Voice)
                voice.Start();
        }

        /// <summary>
        /// Fills sound buffers with POKEY output
        /// Must be called periodically to ensure sounds buffers stay ahead of playback
        /// </summary>
        public void Update()
        {
            if (Settings.SoundEnabled)
            {
                while (Voice[0].State.BuffersQueued < 20)
                {
                    foreach (PokeySourceVoice voice in Voice)
                        voice.FillNextBuffer();
                }
            }
        }
    }
}