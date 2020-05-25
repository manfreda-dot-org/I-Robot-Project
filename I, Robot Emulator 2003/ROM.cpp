// Copyright 2003 by John Manfreda. All Rights Reserved.
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

//---------------------------------------------------------------------------

#include <vcl.h>
#include "ROM.h"
#include "Emulator.h"

__fastcall TROM::TROM()
{
        Size = 0;
        Checksum = 0;
        Data = NULL;
}

__fastcall TROM::~TROM()
{
        Size = 0;
        Checksum = 0;
        Delete();
}

void __fastcall TROM::Delete(void)
{
        if (Data)
        {
                delete[] Data;
                Data = NULL;
        }
}

bool __fastcall TROM::Error(AnsiString error)
{
        Delete();
        Size = Checksum = 0;
        Terminate(error);
        return false;
}

bool __fastcall TROM::Load(AnsiString filename)
{
        Delete();

        int handle = FileOpen(filename, fmOpenRead);
        if (handle == -1)
                return Error("Can't load ROM " + filename);

        // load the file
        Size = FileSeek(handle, 0, 2);
        Data = new BYTE[Size+1];
        FileSeek(handle, 0, 0);
        int bytesread = FileRead(handle, Data, Size);
        FileClose(handle);
        if (bytesread != Size)
                return Error("Problem reading " + filename);

        // calculate checksum
        DWORD n = Size;
        Checksum = 0;
        while (n--)
                Checksum += Data[n];

        return true;
}

bool __fastcall TROM::Load(AnsiString filename, int size)
{
        if (!Load(filename))
                return false;
        if (size != Size)
                return Error("ROM " + filename + " has incorrect size.\nExpected size = " + IntToStr(size) + " bytes\nActual size = " + IntToStr(Size) + " bytes");
        return true;
}

bool __fastcall TROM::Load(AnsiString filename, int size, DWORD checksum)
{
        if (!Load(filename, size))
                return false;
        if (checksum != Checksum)
        {
                AnsiString error = "ROM " + filename + " has incorrect checksum.\nExpected checksum = $" + IntToHex((__int64)checksum,8) + "\nActual checksum = $" + IntToHex((__int64)Checksum,8) + "\n\nDo you want to use this file?";
                if (Application->MessageBox(error.c_str(), "File Error", MB_YESNO | MB_ICONQUESTION) != IDYES)
                {
                        Delete();
                        Size = Checksum = 0;
                        Application->Terminate();
                        return false;
                }
        }
        return true;
}

int __fastcall TROM::GetSize()
{
        return Size;
}

DWORD __fastcall TROM::GetChecksum()
{
        return Checksum;
}

