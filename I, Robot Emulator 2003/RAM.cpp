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

#include "RAM.h"
#include "Emulator.h"

__fastcall TRAM::TRAM()
{
}

void __fastcall TRAM::InitIO()
{
        // setup 6809 page vector pointers

	// $00xx-$07xx  fixed working RAM
        BYTE * ptr = _0000_07FF;
	for (int n=0x00; n<=0x07; n++, ptr+=0x100)
		M6809_PageReadPointer[n] = M6809_PageWritePointer[n] = (DWORD) ptr;

        // handle bank switched areas
        Bank = -1;
        BankSwitch();
}

void __fastcall TRAM::BankSwitch(void)
{
        int bank = (Hardware.Registers._1180 >> 5) & 3;

        if (bank > 2)
        {
                Terminate("Illegal RAM bank selection: 1180=" + IntToHex(Hardware.Registers._1180, 2));
                return;
        }

        if (bank != Bank)
        {
                Bank = bank;
                BYTE *ptr = _0800_0FFF[bank];
                for (int n=0x08; n<=0x0F; n++, ptr+=0x100)
                        M6809_PageReadPointer[n] =
                        M6809_PageWritePointer[n] = (DWORD) ptr;
	}
}

