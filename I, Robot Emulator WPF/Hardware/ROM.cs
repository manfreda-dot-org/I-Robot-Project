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
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Security.Cryptography;
using System.Text;

namespace I_Robot
{
    /// <summary>
    /// interface for a byte addressable ROM
    /// </summary>
    public interface IRom8 : IReadOnlyList<byte>
    {
        string SHA1 { get; }
        UInt32 Checksum { get; }
    }
    
    public static class RomExtensions
    {
        public static UInt16 Word(this IRom8 rom, int index)
        {
            return (UInt16)((rom[index] << 8) + rom[index + 1]);
        }
    }

    public class ROM : IRom8
    {
        #region STATIC

        static public bool FromStream(Stream stream, out ROM? rom)
        {
            try
            {

                using (MemoryStream memory = new MemoryStream())
                {
                    stream.CopyTo(memory);
                    rom = new ROM(memory);
                    return true;
                }
            }
            catch
            {
                rom = null;
                return false;
            }
        }

        static public bool FromFile(String filename, out ROM? rom)
        {
            try
            {
                ROM r = new ROM(filename);
                rom = r;
                return true;
            }
            catch
            {
                Log.LogMessage($"Failed to load ROM file: {filename}");
                rom = null;
                return false;
            }
        }

        static public bool FromFile(String filename, int size, out ROM? rom)
        {
            if (FromFile(filename, out rom) && rom != null)
            {
                if (rom.Count != size)
                {
                    Log.LogMessage($"ROM {filename} has incorrect size: expected = {size} bytes, actual = {rom.Count} bytes");
                    rom = null;
                }
            }

            return rom != null;
        }

        static public bool FromFile(String filename, int size, UInt32 checksum, out ROM? rom)
        {
            if (FromFile(filename, size, out rom) && rom != null)
            {
                if (rom.Checksum != checksum)
                {
                    Log.LogMessage($"ROM {filename} has incorrect checksum: expected = {ChecksumString(checksum)}, actual = {ChecksumString(rom.Checksum)}");
                    rom = null;
                }
            }
            return rom != null;
        }
        #endregion

        public UInt32 Checksum { get; private set; }
        public string SHA1 { get; private set; } = "";
        public readonly byte[] Data;
        public int Count => Data.Length;

        public ROM(MemoryStream stream) : this(stream.ToArray()) { }

        public ROM(byte[] data)
        {
            Data = data;

            // calculate checksum
            for (int n = 0; n < Data.Length; n++)
                Checksum += Data[n];

            SHA1 = BitConverter.ToString(new SHA1CryptoServiceProvider().ComputeHash(Data)).Replace("-", "");
        }

        // hide the constructor - force user to call TryLoad() method
        ROM(String filename)
        {
            Data = System.IO.File.ReadAllBytes(filename);

            // calculate checksum
            for (int n = 0; n < Data.Length; n++)
                Checksum += Data[n];

            System.Diagnostics.Debug.WriteLine($"ROM {filename} loaded, checksum = {ChecksumString(Checksum)}");
        }

        public byte this[int index]
        {
            get { return Data[index]; }
        }

        static String ChecksumString(UInt32 checksum)
        {
            return "$" + checksum.ToString("X").PadLeft(8, '0');
        }

        IEnumerator<byte> IEnumerable<byte>.GetEnumerator() { return ((IReadOnlyList<byte>)Data).GetEnumerator(); }
        IEnumerator IEnumerable.GetEnumerator() { return Data.GetEnumerator(); }

        public static implicit operator byte[](ROM r) => r.Data;
    }
}