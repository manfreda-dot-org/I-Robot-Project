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

#ifndef LogH
#define LogH
//---------------------------------------------------------------------------

#include <system.hpp>

class TLog
{
public:
        void __fastcall Clear(void);
        void __fastcall Add(AnsiString string, COLORREF color = RGB(255,255,255), int delay = -1);
        void __fastcall Render(void);
};

extern TLog Log;

//---------------------------------------------------------------------------
#endif
