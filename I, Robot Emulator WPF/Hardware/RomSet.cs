using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Net;
using System.Runtime.Serialization;
using System.Text;
using System.Windows.Navigation;

namespace I_Robot
{
    public class RomSet : IReadOnlyCollection<ROM>
    {
        static IReadOnlyList<RomInfo> RomList;

        class RomInfo
        {
            public readonly string Key;
            public readonly string SearchTerm;
            public readonly int Size;
            public readonly uint Checksum;

            public RomInfo(string name, string searchTerm, int size, uint checksum)
            {
                Key = name;
                SearchTerm = searchTerm;
                Size = size;
                Checksum = checksum;
            }
        }

        static RomSet()
        {
            List<RomInfo> list = new List<RomInfo>();
            list.Add(new RomInfo("136029-101", "101", 0x4000, 0x150247));
            list.Add(new RomInfo("136029-102", "102", 0x4000, 0xF557F));
            list.Add(new RomInfo("136029-103", "103", 0x2000, 0x6A797));
            list.Add(new RomInfo("136029-104", "104", 0x2000, 0x43382));
            list.Add(new RomInfo("136029-405", "405", 0x4000, 0x150A97));
            list.Add(new RomInfo("136029-206", "206", 0x4000, 0x174942));
            list.Add(new RomInfo("136029-207", "207", 0x4000, 0x17384C));
            list.Add(new RomInfo("136029-208", "208", 0x2000, 0x0D5E26));
            list.Add(new RomInfo("136029-209", "209", 0x4000, 0x1A1B59));
            list.Add(new RomInfo("136029-210", "210", 0x4000, 0x179092));
            list.Add(new RomInfo("136029-124", "124", 0x800, 0x2069D));
            list.Add(new RomInfo("136029-125", "125", 0x20, 0x42C));
            RomList = list;
        }

        static bool FindRomInfo(ZipArchiveEntry entry, out RomInfo? info)
        {
            foreach (RomInfo rom in RomList)
            {
                if (entry.Name.Contains(rom.SearchTerm))
                {
                    info = rom;
                    return true;
                }
            }

            info = null;
            return false;
        }

        public readonly string Filename;
        readonly Dictionary<string, ROM> Dict = new Dictionary<string, ROM>();
        public int Count => Dict.Count;

        static public bool TryGetRomSet(string filename, out RomSet? set, out string errMessage)
        {
            set = new RomSet(filename, out errMessage);
            if (set.Count == RomList.Count)
                return true;
            set = null;
            return false;
        }

        private RomSet(string filename, out string errMessage)
        {
            Filename = filename;

            if (!File.Exists(filename))
            {
                errMessage = $"Unable to locate {filename}";
                return;
            }

            try
            {
                using (ZipArchive archive = ZipFile.OpenRead(filename))
                {
                    foreach (ZipArchiveEntry entry in archive.Entries)
                    {
                        if (FindRomInfo(entry, out RomInfo? info) && info != null)
                        {
                            if (entry.Length != info.Size)
                            {
                                errMessage = $"{filename}: {info.Key} is wrong size, filesize = {entry.Length}, expected = {info.Size}";
                                return;
                            }

                            System.Diagnostics.Debug.WriteLine($"{filename}: reading {entry.FullName}");
                            using (Stream stream = entry.Open())
                            {
                                if (ROM.FromStream(stream, out ROM? rom) && rom != null)
                                {
                                    if (rom.Checksum != info.Checksum)
                                    {
                                        errMessage = $"{filename}: {info.Key} has bad checksum, checksum = {rom.Checksum.ToString("X8")}, expected = {info.Checksum.ToString("X8")}";
                                        return;
                                    }

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

            foreach (RomInfo info in RomList)
            {
                if (!TryGetRom(info.Key, out ROM? rom))
                {
                    errMessage = $"Unable to locate rom {info.Key} in archive {filename}";
                    return;
                }
            }

            errMessage = "success";
        }

        public bool TryGetRom(string index, out ROM? rom) { return Dict.TryGetValue(index, out rom); }

        public IEnumerator<ROM> GetEnumerator() { return (IEnumerator<ROM>)RomList.GetEnumerator(); }
        IEnumerator IEnumerable.GetEnumerator() { return RomList.GetEnumerator(); }
    }
}