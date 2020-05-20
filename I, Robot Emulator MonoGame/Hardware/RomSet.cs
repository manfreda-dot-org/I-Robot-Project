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
using System.IO.Compression;

namespace I_Robot
{
    /// <summary>
    /// Represents the set of ROMs required to run the emulator
    /// </summary>
    public class RomSet : IReadOnlyCollection<ROM>
    {
        static IReadOnlyList<RomInfo> RomList;
        
        /// <summary>
        /// Represents information on a ROM that is part of the set
        /// </summary>
        class RomInfo
        {
            public readonly string Key;
            public readonly string SearchTerm;
            public readonly int Size;
            public readonly string SHA1;

            public RomInfo(string name, string searchTerm, int size, string sha1)
            {
                Key = name;
                SearchTerm = searchTerm;
                Size = size;
                SHA1 = sha1;
            }
        }

        static RomSet()
        {
            // create a list of the ROMs we are looking for
            List<RomInfo> list = new List<RomInfo>();
            list.Add(new RomInfo("136029-101", "101", 0x4000, "868BB3FE5657A4CE45C3DD04BA26A7FB5A5DED42"));
            list.Add(new RomInfo("136029-102", "102", 0x4000, "787EC3E642E1DC3417477348AFA88C764E1F2A88"));
            list.Add(new RomInfo("136029-103", "103", 0x2000, "C1F4041A58F395E24855254849604DFE3B8B0D71"));
            list.Add(new RomInfo("136029-104", "104", 0x2000, "B9FD76EAE8CA24FA3ABC30C46BBF30D89943D97D"));
            list.Add(new RomInfo("136029-405", "405", 0x4000, "5D71D8EC80C9BE4726189D48AD519B4638160D64"));
            list.Add(new RomInfo("136029-206", "206", 0x4000, "BD94AD4D536F681EFA81153050A12098A31D79CF"));
            list.Add(new RomInfo("136029-207", "207", 0x4000, "2E0C1E4C265E7D232CA86D5C8760E32FC49FE08D"));
            list.Add(new RomInfo("136029-208", "208", 0x2000, "5B476DBEE8B171A96301B2204420161333D4CA97"));
            list.Add(new RomInfo("136029-209", "209", 0x4000, "A88AE0CC9EE22AA5DD3DB0173F24313189F894F8"));
            list.Add(new RomInfo("136029-210", "210", 0x4000, "DAA77293678B7E822D0672B90789C53098C5451E"));
            list.Add(new RomInfo("136029-124", "124", 0x800, "743C6570C787BC9A2A14716ADC66B8E2FE57129F"));
            list.Add(new RomInfo("136029-125", "125", 0x20, "5B42CC065BFAC467028AE883844C8F94465C3666"));
            RomList = list;
        }

        // determins if the file in the zip archive is one of the ROM's we're looking for
        static bool FindRomInfo(ZipArchiveEntry file, out RomInfo? info)
        {
            foreach (RomInfo rom in RomList)
            {
                if (file.Name.Contains(rom.SearchTerm))
                {
                    info = rom;
                    return true;
                }
            }

            info = null;
            return false;
        }

        /// <summary>
        /// The filename of the rom archive
        /// </summary>
        public readonly string Filename;

        // dictionary that holds the ROMs that we've found
        readonly Dictionary<string, ROM> Dict = new Dictionary<string, ROM>();
        
        /// <summary>
        /// The number of ROMs in this RomSet
        /// </summary>
        public int Count => Dict.Count;

        /// <summary>
        /// Attempts to load a rom set from a zip archive file
        /// </summary>
        /// <param name="filename">filename of the zip archive containing the rom set</param>
        /// <param name="set">returns RomSet on success</param>
        /// <param name="errMessage">error message returned on failure</param>
        /// <returns>true if the rom set was sucessfully loaded, false if error</returns>
        static public bool ReadRomSetFromZipArchive(string filename, out RomSet? set, out string? errMessage)
        {
            set = new RomSet(filename, out errMessage);
            if (errMessage == null)
                return true;
            set = null;
            return false;
        }

        // private constructor -- users must call ReadRomSetFromZipArchive
        private RomSet(string filename, out string? errMessage)
        {
            Filename = filename;

            // make sure the file exists
            if (!File.Exists(filename))
            {
                errMessage = $"Unable to locate {filename}";
                return;
            }

            try
            {
                // get the zip archive
                using (ZipArchive archive = ZipFile.OpenRead(filename))
                {
                    // iterate through all files inside the archive
                    foreach (ZipArchiveEntry file in archive.Entries)
                    {
                        // see if the current file entry is one we are looking for
                        if (FindRomInfo(file, out RomInfo? info) && info != null)
                        {
                            // make sure the file length matches the expected size
                            if (file.Length != info.Size)
                            {
                                errMessage = $"{filename}: {info.Key} is wrong size\nSize = {file.Length}, expected = {info.Size}";
                                return;
                            }

                            // create a ROM from the archive
                            using (Stream stream = file.Open())
                            {
                                if (ROM.FromStream(stream, out ROM? rom) && rom != null)
                                {
                                    // verify the SHA1 signature matches
                                    if (rom.SHA1 != info.SHA1)
                                    {
                                        errMessage = $"{filename}: {info.Key} has bad hash\nSHA1     = {rom.SHA1}\nexpected = {info.SHA1}";
                                        return;
                                    }

                                    // this ROM looks good, add it to our list
                                    Dict.Add(info.Key, rom);
                                }
                            }
                        }
                    }
                }
            }
            catch (Exception e)
            {
                errMessage = e.ToString();
                return;
            }

            // make sure we've sucessfully read all roms in the set
            foreach (RomInfo info in RomList)
            {
                if (this[info.Key] == null)
                {
                    errMessage = $"{filename}: Unable to locate {info.Key}";
                    return;
                }
            }

            // success
            errMessage = null;
        }

        /// <summary>
        /// Gets the ROM from the RomSet that matches the name
        /// </summary>
        /// <param name="name">the name of the ROM to retrieve</param>
        /// <returns>The ROM that matches the given name, null if ROM does not exist in the set</returns>
        public ROM? this[string name]
        {
            get
            {
                Dict.TryGetValue(name, out ROM? rom);
                return rom;
            }
        }

        public IEnumerator<ROM> GetEnumerator() { return (IEnumerator<ROM>)RomList.GetEnumerator(); }
        IEnumerator IEnumerable.GetEnumerator() { return RomList.GetEnumerator(); }
    }
}