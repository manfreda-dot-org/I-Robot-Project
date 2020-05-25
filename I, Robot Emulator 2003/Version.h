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

#ifndef VersionH
#define VersionH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Menus.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include <ComCtrls.hpp>
#include <Registry.hpp>

//---------------------------------------------------------------------------

class TVersion
{
private:	// User declarations
        BYTE * fileinfo;
        int major;
        int minor;
        int release;
        int build;

public:		// User declarations
        __fastcall TVersion();
        __fastcall ~TVersion();

        int __fastcall GetMajorVersion() { return major; }
        int __fastcall GetMinorVersion() { return minor; }
        int __fastcall GetReleaseVersion() { return release; }
        int __fastcall GetBuildVersion() { return build; }

        AnsiString __fastcall GetDateTime();
        AnsiString __fastcall GetVersionString();
};

#endif

