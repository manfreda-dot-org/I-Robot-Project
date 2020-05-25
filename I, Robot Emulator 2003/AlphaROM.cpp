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

#include "AlphaROM.h"

__fastcall TAlphaROM::TAlphaROM()
{
        // load the ROM
        if (!Load("136029.124", 0x800, 0x2069D))
                return;

        // unpack the ROM into character scanlines
        BYTE const * src = Data;
	for (int alpha=0; alpha<64; alpha++)
        {
		for (int scanline=0; scanline<8; scanline++)
                {
                        BYTE bits = 0;
                        if (*src & 8) bits |= 0x80;
                        if (*src & 4) bits |= 0x40;
                        if (*src & 2) bits |= 0x20;
                        if (*src++ & 1) bits |= 0x10;
                        if (*src & 8) bits |= 0x08;
                        if (*src & 4) bits |= 0x04;
                        if (*src & 2) bits |= 0x02;
                        if (*src++ & 1) bits |= 0x01;
                        Character[alpha][scanline] = bits;
                }
        }
}

