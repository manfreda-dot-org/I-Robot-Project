// Copyright 1997, 1998, 2020 by John Manfreda. All Rights Reserved.
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

#include "IRobot.h"

BOOL EnsureExecutableIntegrity( void )
{
	int m = 0, n;
	WORD CRC = 0, vc, ve;
	BYTE value;

	// calculate the CRC of the entire text block
	value = **GlobalStrings;
	while (value)
	{
		n = 0;
		while (value)
		{
			ve = (CRC>>8) ^ value;
			vc = ((ve<<5) ^ (ve<<1)) & 0x1FE0;
			CRC = (CRC<<8) ^ (vc<<7) ^ vc ^ ve ^ (ve>>4);
			value = GlobalStrings[m][++n];
		}
		value = *GlobalStrings[++m];
	}

	if (CRC == 0x4ED3)
		return TRUE;

//	wsprintf( GlobalExitMessage, "CRC = $%0.4X", (int) CRC);

	return FALSE;
}