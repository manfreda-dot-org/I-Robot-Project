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

// Determine bank of working RAM to use
void FASTCALL SelectBank0800t0FFF( void )
{
	unsigned int n, bank = (REG1180 & (_2PLAYRAM | _800FLIP)) >> 5;
	BYTE * ptr;

	if (bank < 4)
	{
		if (bank != Bank0800)
		{
			Bank0800 = bank;
			ptr = RAM0800t0FFF[bank];
			for (n=0x08; n<=0x0F; n++, ptr+=0x100)
				ReadPointer[n] =
				WritePointer[n] = (DWORD) ptr;
			return;
		}
	}
	else
	{
		wsprintf( GlobalExitMessage, "Illegal bank selected $1180=%0.2X", (int) REG1180 );
		PostMessage( ghwndMain, WM_CLOSE, 0, 0 );
	}
}

// Determine bank of MATH RAM/ROM or COM RAM to use
void FASTCALL SelectBank2000t3FFF( void )
{
	unsigned int n, bank;
	BOOL isRAM, isMRAM = FALSE;
	BYTE * ptr;
	
	if(REG1180 & OUT04)
	{
		isRAM = TRUE;
		if (REG1180 & OUT03)				// Mathbox RAM
			isMRAM = TRUE, bank = 0;
		else					// C1 / C2 RAM
			bank = (REG1140 & EXTCOMSWAP) ? 1 : 2;

	}
	else
	{
		isRAM = FALSE;
		bank = ((REG1180 & (OUT03 | MPAGE2 | MPAGE1)) >> 1) + 1;
		if (bank > 8)
		{
			wsprintf( GlobalExitMessage, "Illegal bank selected $1180=%0.2X", (int) REG1180 );
			PostMessage( ghwndMain, WM_CLOSE, 0, 0 );
			return;
		}
	}

	if (bank != Bank2000)
	{
		Bank2000 = bank;
		ptr = RAM2000t3FFF[bank];
		if(isRAM)
		{
			if (isMRAM)
				for (n=0x20; n<=0x3F; n++)
					ReadPointer[n] =
					WritePointer[n] = 0;
			else
				for (n=0x20; n<=0x3F; n++, ptr+=0x100)
					ReadPointer[n] =
					WritePointer[n] = (DWORD) ptr;
		}
		else for (n=0x20; n<=0x3F; n++, ptr+=0x100)
		{
			ReadPointer[n] = (DWORD) ptr;
			WritePointer[n] = 0;
		}
	}
}

// Determine bank of program ROM to use
void FASTCALL SelectBank4000t5FFF( void )
{
	unsigned int n, bank = (REG11C0 & (BANK2 | BANK1 | BANK0)) >> 1;
	BYTE * ptr;

	if (bank < 6)
	{
		if (bank != BankROM)
		{
			BankROM = bank;
			CurrentROM = ROM[bank];
			ptr = CurrentROM + 0x4000;
			for (n=0x40; n<=0x5F; n++, ptr+=0x100)
//				WritePointer[n] =		// No writes to ROM
				ReadPointer[n] = (DWORD) ptr;
		}
	}
	else
	{
		wsprintf( GlobalExitMessage, "Illegal bank selected $11C0=%0.2X", (int) REG11C0 );
		PostMessage( ghwndMain, WM_CLOSE, 0, 0 );
	}
}
