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

#include "ColorRAM.h"
#include "Emulator.h"

static DWORD Write18xx(BYTE data, WORD address)
{
	// color RAM area
	if (address <= 0x187F)
        {
                // I Robot palette
                // address --------:-aaaaaai
                // data    rrggbbii

                // NOTE: intensity is 3 bits, LSB of which is in address

	        data = ~data;

                int index = (address >> 1) & 0x3F;

	        BYTE i = (((data & 0x03) << 1) + (~address&1) + 1) * 8;
                Hardware.ColorRAM.Palette[index].r = ((data >> 6) & 3) * i;
	        Hardware.ColorRAM.Palette[index].g = ((data >> 4) & 3) * i;
        	Hardware.ColorRAM.Palette[index].b = ((data >> 2) & 3) * i;
        }

	return (DWORD) Hardware.Program.ROM;
}

void __fastcall TColorRAM::InitIO()
{
        // setup 6809 page vector pointers
	M6809_PageWriteFunction[0x18] = (DWORD) Write18xx;
}

