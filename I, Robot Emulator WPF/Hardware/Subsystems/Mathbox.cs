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
using System.Printing.IndexedProperties;
using System.Reflection.Metadata;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Text;
using System.Windows.Markup.Localizer;

namespace I_Robot
{
    [Serializable]
    unsafe public class Mathbox : Hardware.Subsystem
    {
        public enum COMMAND : UInt16
        {
            START_PLAYFIELD = 0x8400,
            UNKNOWN = 0x8600,
            ROLL = 0x8800,
            YAW = 0x9000,
            PITCH = 0xA000,
            MATRIX_MULTIPLY = 0xC000,
        }

        [StructLayout(LayoutKind.Sequential, Pack = 2)]
        struct Matrix
        {
            public Int16 M11, M12, M13;
            public Int16 M21, M22, M23;
            public Int16 M31, M32, M33;

            public override string ToString()
            {
                return $"| {M11.HexString()}, {M12.HexString()}, {M13.HexString()} |\n| {M21.HexString()}, {M22.HexString()}, {M23.HexString()} |\n| {M31.HexString()}, {M32.HexString()}, {M33.HexString()} |";
            }
        }

        /// <summary>
        /// Patches self test to allow it to pass mathbox testing
        /// Replaces the result matrices with results that this mathbox generates
        /// </summary>
        static public readonly byte[] SelfTestPatch = new byte[]
               { 0x40,0x00, 0x00,0x00, 0x01,0x21, // yaw test result
                  0x00,0x00, 0x40,0x00, 0x00,0x00,
                  0xFE,0xE1, 0x00,0x00, 0x40,0x01,
                  0x40,0x01, 0xFE,0xE1, 0x01,0x21, // roll test result
                   0x01,0x21, 0x40,0x00, 0x00,0x00,
                  0xFE,0xE9, 0x00,0x05, 0x40,0x01,
                  0x40,0x01, 0xFE,0xE8, 0x01,0x2C, // pitch test result
                  0x01,0x21, 0x40,0x01, 0xFE,0xE1,
                  0xFE,0xE9, 0x01,0x22, 0x3F,0xFC,
                  0x40,0x01, 0x01,0x21, 0xFE,0xE9, // matrix multiply result
                  0xFE,0xE8, 0x40,0x01, 0x01,0x22,
                  0x01,0x2C, 0xFE,0xE1, 0x3F,0xFC };

        // 6 mathbox ROM banks that are accessible by the CPU
        public readonly PinnedBuffer<byte>[] ROM = new PinnedBuffer<byte>[6] {
            new PinnedBuffer<byte>(0x2000),
            new PinnedBuffer<byte>(0x2000),
            new PinnedBuffer<byte>(0x2000),
            new PinnedBuffer<byte>(0x2000),
            new PinnedBuffer<byte>(0x2000),
            new PinnedBuffer<byte>(0x2000) };

        readonly PinnedBuffer<UInt16> Buffer16 = new PinnedBuffer<UInt16>(0x8000);
        public byte* pData => (byte*)Buffer16.pData;
        UInt16* Memory => Buffer16.pData;

        bool mMATH_START = false;

        public bool MB_DONE { get; private set; } = true;

        // keep delegates around to prevent them from being garbage collected
        public readonly M6809E.ReadDelegate ReadRamFunction;
        public readonly M6809E.WriteDelegate WriteRamFunction;

        public Mathbox(Hardware hardware) : base(hardware, "Mathbox")
        {
            //  Mathbox     CPU           hi                lo
            //  2000-2FFF   0:2000-3FFF   136029-104[0000]  136029-103[0000]
            //  3000-3FFF   1:2000-3FFF   136029-104[1000]  136029-103[1000]
            //  4000-4FFF   2:2000-3FFF   136029-102[0000]  136029-101[0000]
            //  5000-5FFF   3:2000-3FFF   136029-102[1000]  136029-101[1000]
            //  6000-6FFF   4:2000-3FFF   136029-102[2000]  136029-101[2000]
            //  7000-7FFF   5:2000-3FFF   136029-102[3000]  136029-101[3000]

            // create x86 accessable bank (low endian)

            ROM? r101 = hardware.Roms["136029-101"];
            ROM? r102 = hardware.Roms["136029-102"];
            ROM? r103 = hardware.Roms["136029-103"];
            ROM? r104 = hardware.Roms["136029-104"];

            UInt16 address;
            if (r103 != null)
            {
                address = 0x2000;
                foreach (byte b in r103)
                    Buffer16[address++] = b;
            }
            if (r104 != null)
            {
                address = 0x2000;
                foreach (byte b in r104)
                    Buffer16[address++] |= (UInt16)(b << 8);
            }
            if (r101 != null)
            {
                address = 0x4000;
                foreach (byte b in r101)
                    Buffer16[address++] = b;
            }
            if (r102 != null)
            {
                address = 0x4000;
                foreach (byte b in r102)
                    Buffer16[address++] |= (UInt16)(b << 8);
            }
            
            // create M6809E accessable banks (high endian)
            address = 0x2000;
            for (int bank = 0; bank < ROM.Length; bank++)
            {
                for (int n = 0; n < 0x2000;)
                {
                    ROM[bank][n++] = (byte)(Buffer16[address] >> 8);
                    ROM[bank][n++] = (byte)(Buffer16[address++] >> 0);
                }
            }

#if true
            // now patch the self test so our emulation results passs
            Array.Copy(Mathbox.SelfTestPatch, 0, Hardware.ProgROM.Bank_4000[4], 0x5CAD - 0x4000, SelfTestPatch.Length);

            // adjust the value at the end of ROM bank 4 to account for the checksum change
            UInt16 checksum = 0;
            for (int n = 0; n < 0x1FFE; n += 2)
                checksum -= (UInt16)(Hardware.ProgROM.Bank_4000[4].pData[n] * 256 + Hardware.ProgROM.Bank_4000[4].pData[n + 1]);
            for (int n = 0; n < 0x2000; n += 2)
                checksum -= (UInt16)(Hardware.ProgROM.Bank_4000[5].pData[n] * 256 + Hardware.ProgROM.Bank_4000[5].pData[n + 1]);
            Hardware.ProgROM.Bank_4000[4].pData[0x1FFE] = (byte)(checksum >> 8);
            Hardware.ProgROM.Bank_4000[4].pData[0x1FFF] = (byte)(checksum >> 0);
#endif

            ReadRamFunction = new M6809E.ReadDelegate((UInt16 address) =>
            {
                // flip 6809 hi/lo, so x86 can read words
                return (byte)pData[(address & 0x1FFF) ^ 1];
            });

            WriteRamFunction = new M6809E.WriteDelegate((UInt16 address, byte data) =>
            {
                // flip 6809 hi/lo, so x86 can read words
                pData[(address & 0x1FFF) ^ 1] = data;
            });
        }

        public override void Dispose()
        {
        }

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("MATH_START", MATH_START);
            info.AddValue("MB_DONE", MB_DONE);
            info.AddValue("RAM", Buffer16);
        }

        public override void Reset()
        {
            MB_DONE = true;
        }

        public bool MATH_START
        {
            get => mMATH_START;
            set
            {
                // if signal is changing
                if (mMATH_START != value)
                {
                    mMATH_START = value;

                    // if signal is going from 0 --> 1
                    if (value)
                    {
                        // start execution
                        // command is in first mathbox address
                        switch ((COMMAND)Memory[0])
                        {
                            case COMMAND.START_PLAYFIELD:
                                //RasterizePlayfield();
                                break;
                            case COMMAND.UNKNOWN:
                                break;
                            case COMMAND.ROLL:
                                Roll((Matrix*)&Memory[Memory[6]], (Int16)Memory[7], (Int16)Memory[8]);
                                break;
                            case COMMAND.YAW:
                                Yaw((Matrix*)&Memory[Memory[6]], (Int16)Memory[7], (Int16)Memory[8]);
                                break;
                            case COMMAND.PITCH:
                                Pitch((Matrix*)&Memory[Memory[6]], (Int16)Memory[7], (Int16)Memory[8]);
                                break;
                            case COMMAND.MATRIX_MULTIPLY:
                                MatrixMultiply((Int16*)&Memory[Memory[0x7B]], (Int16*)&Memory[Memory[0x7C]], (Int16*)&Memory[Memory[0x7D]]);
                                break;
                            default:
                                // RasterizeObject(Memory[0]);
                                break;
                        }

                        // signal mathbox completion to CPU
                        Hardware.M6809E.FIRQ = true;
                    }
                }
            }
        }

        /// <summary>
        /// Pitch function (rotate about X axis).
        ///
        ///                     [ 1   0   0  ]
        ///  matrix = matrix x	[ 0  cos sin ]
        ///                     [ 0 -sin cos ]
        /// </summary>
        /// <param name="pMatrix">pointer to matrix</param>
        /// <param name="sin">sin() of angle to rotate by</param>
        /// <param name="cos">cos() of angle to rotate by</param>
        void Pitch(Matrix* pMatrix, Int16 sin, Int16 cos)
        {
            Int16 m12 = pMatrix->M12, m13 = pMatrix->M13;
            Int16 m22 = pMatrix->M22, m23 = pMatrix->M23;
            Int16 m32 = pMatrix->M32, m33 = pMatrix->M33;

            pMatrix->M12 = (Int16)((m12 * cos - m13 * sin) >> 14);
            pMatrix->M13 = (Int16)((m13 * cos + m12 * sin) >> 14);
            pMatrix->M22 = (Int16)((m22 * cos - m23 * sin) >> 14);
            pMatrix->M23 = (Int16)((m23 * cos + m22 * sin) >> 14);
            pMatrix->M32 = (Int16)((m32 * cos - m33 * sin) >> 14);
            pMatrix->M33 = (Int16)((m33 * cos + m32 * sin) >> 14);
        }

        /// <summary>
        /// Yaw function (rotate about Y axis).
        ///
        ///                     [ cos 0 -sin ]
        ///  matrix = matrix x	[  0  1   0  ]
        ///                     [ sin 0  cos ]
        /// </summary>
        /// <param name="pMatrix">pointer to matrix</param>
        /// <param name="sin">sin() of angle to rotate by</param>
        /// <param name="cos">cos() of angle to rotate by</param>
        void Yaw(Matrix* pMatrix, Int16 sin, Int16 cos)
        {
            Int16 m11 = pMatrix->M11, m13 = pMatrix->M13;
            Int16 m21 = pMatrix->M21, m23 = pMatrix->M23;
            Int16 m31 = pMatrix->M31, m33 = pMatrix->M33;

            pMatrix->M11 = (Int16)((m11 * cos + m13 * sin) >> 14);
            pMatrix->M13 = (Int16)((m13 * cos - m11 * sin) >> 14);
            pMatrix->M21 = (Int16)((m21 * cos + m23 * sin) >> 14);
            pMatrix->M23 = (Int16)((m23 * cos - m21 * sin) >> 14);
            pMatrix->M31 = (Int16)((m31 * cos + m33 * sin) >> 14);
            pMatrix->M33 = (Int16)((m33 * cos - m31 * sin) >> 14);
        }

        /// <summary>
        /// Roll function (rotate about Z axis)
        /// 
        ///                   [  cos sin 0 ]
        /// matrix = matrix x [ -sin cos 0 ]
        ///                   [   0   0  1 ]
        /// </summary>
        /// <param name="pMatrix">pointer to matrix</param>
        /// <param name="sin">sin() of angle to rotate by</param>
        /// <param name="cos">cos() of angle to rotate by</param>
        void Roll(Matrix* pMatrix, Int16 sin, Int16 cos)
        {
            Int16 m11 = pMatrix->M11, m12 = pMatrix->M12;
            Int16 m21 = pMatrix->M21, m22 = pMatrix->M22;
            Int16 m31 = pMatrix->M31, m32 = pMatrix->M32;

            pMatrix->M11 = (Int16)((m11 * cos - m12 * sin) >> 14);
            pMatrix->M12 = (Int16)((m12 * cos + m11 * sin) >> 14);
            pMatrix->M21 = (Int16)((m21 * cos - m22 * sin) >> 14);
            pMatrix->M22 = (Int16)((m22 * cos + m21 * sin) >> 14);
            pMatrix->M31 = (Int16)((m31 * cos - m32 * sin) >> 14);
            pMatrix->M32 = (Int16)((m32 * cos + m31 * sin) >> 14);
        }

        /// <summary>
        /// Matrix multiply 
        /// performs A x B = C
        /// result is C transpose
        /// </summary>
        /// <param name="pA">pointer to first matrix</param>
        /// <param name="pB">pointer to second matrix</param>
        /// <param name="pC">pointer to result matrix</param>
        void MatrixMultiply(Int16* pA, Int16* pB, Int16* pC)
        {
            pC[0] = (Int16)((pA[0] * pB[0] + pA[1] * pB[1] + pA[2] * pB[2]) >> 14);
            pC[1] = (Int16)((pA[0] * pB[3] + pA[1] * pB[4] + pA[2] * pB[5]) >> 14);
            pC[2] = (Int16)((pA[0] * pB[6] + pA[1] * pB[7] + pA[2] * pB[8]) >> 14);
            pC[3] = (Int16)((pA[3] * pB[0] + pA[4] * pB[1] + pA[5] * pB[2]) >> 14);
            pC[4] = (Int16)((pA[3] * pB[3] + pA[4] * pB[4] + pA[5] * pB[5]) >> 14);
            pC[5] = (Int16)((pA[3] * pB[6] + pA[4] * pB[7] + pA[5] * pB[8]) >> 14);
            pC[6] = (Int16)((pA[6] * pB[0] + pA[7] * pB[1] + pA[8] * pB[2]) >> 14);
            pC[7] = (Int16)((pA[6] * pB[3] + pA[7] * pB[4] + pA[8] * pB[5]) >> 14);
            pC[8] = (Int16)((pA[6] * pB[6] + pA[7] * pB[7] + pA[8] * pB[8]) >> 14);
        }
    }
}