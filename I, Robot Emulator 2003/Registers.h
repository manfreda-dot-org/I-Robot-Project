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

#ifndef RegistersH
#define RegistersH
//---------------------------------------------------------------------------

#include <system.hpp>

class TRegisters
{
public:
        __fastcall TRegisters();

        BYTE _1000;
        BYTE _1040;
        BYTE _1080;
        BYTE _1140;
        BYTE _1180;
        BYTE _11C0;
        BYTE _1300;

        void __fastcall InitIO(void);
};

//---------------------------------------------------------------------------
#endif
