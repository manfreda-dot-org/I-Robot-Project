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

#include "ColorROM.h"

__fastcall TColorROM::TColorROM()
{
        // load the ROM
        if (!Load("136029.125", 0x20, 0x42C))
                return;

        // unpack the ROM into r/g/b structure
        for (int palette=0; palette<2; palette++)
                for (int color=0; color<4; color++)
                {
                        BYTE byte = Data[palette*16 + color + 4];
                        BYTE i = ((byte & 3) << 1) + 1; // 0-7
                        int r = (byte >> 6) & 3; // 0-3
                        int g = (byte >> 4) & 3; // 0-3
                        int b = (byte >> 2) & 3; // 0-3

                        r = min(255*r*i/3/7,255);
                        g = min(255*g*i/3/7,255);
                        b = min(255*b*i/3/7,255);

                        Color[palette][color] = D3DCOLOR_XRGB(r,g,b);
                }
}

