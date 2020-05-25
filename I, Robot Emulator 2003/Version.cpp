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

#include <vcl.h>
#pragma hdrstop

#include "Version.h"



//---------------------------------------------------------------------------
__fastcall TVersion::TVersion()
{
        major = minor = 0;
        fileinfo = NULL;

        // get file version info block size
        DWORD handle;
        DWORD size = GetFileVersionInfoSize(Application->ExeName.c_str(), &handle);
        if (size == 0)
                return;

        // allocate memory and retrieve file version info block
        BYTE *fileinfo = new BYTE[size];
        if (!GetFileVersionInfo(Application->ExeName.c_str(), handle, size, fileinfo))
                return;

        // get pointer to fixed file info structure
        VS_FIXEDFILEINFO * fixedfileinfo;
        unsigned int len;
        if (VerQueryValue(fileinfo, "\\", &(LPVOID)fixedfileinfo, &len))
        {
                // unload data from fixed file info
                major = HIWORD(fixedfileinfo->dwFileVersionMS);
                minor = LOWORD(fixedfileinfo->dwFileVersionMS);
                release = HIWORD(fixedfileinfo->dwFileVersionLS);
                build = LOWORD(fixedfileinfo->dwFileVersionLS);
        }

        delete[] fileinfo;
}

__fastcall TVersion::~TVersion()
{
        delete fileinfo;
}


AnsiString __fastcall TVersion::GetDateTime()
{
        return __DATE__ "  " __TIME__;
}

AnsiString __fastcall TVersion::GetVersionString()
{
        return IntToStr(major) + "." + IntToStr(minor) + "." + IntToStr(release) + "." + IntToStr(build);
}

