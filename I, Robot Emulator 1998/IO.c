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

#pragma check_stack(off)

BYTE Read10xx( WORD address )
{
	if (address == 0x1000)
		return REG1000;

	if (address == 0x1040)
	{
		REG1040 = 0xFF;
		if (IsStart1ButtonPressed())
			REG1040 &= ~START1;
		if (IsStart2ButtonPressed())
			REG1040 &= ~START2;
		if (IsFireButtonPressed())
			REG1040 &= ~FIRE1;
		return REG1040;
	}

	if (address == 0x1080)
		return REG1080;

//	if (address == 0x10C0)
		return DIPSWITCH3J;
}


BYTE Read13xx( WORD address )
{
	return REG1300;
}

BYTE Read14xx( WORD address )
{
	if (address == 0x1420)
		return DIPSWITCH5E;

	return LOBYTE(rand());
}

BYTE Read20xxto3Fxx( WORD address )
{
	// Bank-switched MathRAM
	// flip x86 word values, so 6809 can read them!
	return RAM2000t3FFF[Bank2000][(address-0x2000)^1];
}

BYTE ReadUnknown( WORD address )
{
	wsprintf( GlobalExitMessage, "Undefined read at address $%0.4X", address );
	PostMessage( ghwndMain, WM_CLOSE, 0, 0 );
	return 0;
}



DWORD Write11xx( BYTE data, WORD address )
{
	// remove any active IRQ
	if (address == 0x1100)
		IRQnotify &= ~IRQline;

	else if (address == 0x1140)
	{
		BYTE toggled = REG1140 ^ data;
		REG1140 = data;

		// check if EXTCOMSWAP was toggled
		if (toggled & EXTCOMSWAP)
			SelectBank2000t3FFF();

		// check if ERASE goes from 0->1
		if ((toggled & ERASE) && (REG1140 & ERASE))
			EraseScreenBuffer( (REG1140 & BUFSEL) ? VIDRAM0 : VIDRAM1 );

		// change screen buffer if BUFSEL was toggled
		if (toggled & BUFSEL)
		{
			ChangeScreenBuffer();
			if (!RenderEveryFrame)
				DisplayScreenBuffer();
		}

		// check if MATHSTART goes from 0->1
		if ((toggled & MATHSTART) && (REG1140 & MATHSTART))
		{
			DoMathbox();

			// if the application has been modified
			// then randombly fuck with the user
			if (!ApplicationCorrupt)
				IRQnotify |= FIRQline;
			else if (rand()%1000)
			{
				IRQnotify |= FIRQline;
				REG1080 |= 0x20;
			}	
			else
				REG1080 &= 0xDF;
		}
	}

	else if (address == 0x1180)
	{
		REG1180 = data;
		SetAlphaPalette();
		SelectBank0800t0FFF();
		SelectBank2000t3FFF();
	}

	else if (address == 0x11C0)
	{
		REG11C0 = data;
		SelectBank4000t5FFF();
	}

	return (DWORD) CurrentROM;
}


DWORD Write14xx( BYTE data, WORD address )
{
	// Quad-Pokey area
	Update_pokey_sound(
		(WORD) ((address&0x07)+((address&0x20)?8:0)),
		data,
		(BYTE)((address>>3)&0x03) );

	return (DWORD) CurrentROM;
}

DWORD Write18xx( BYTE data, WORD address )
{
	// color RAM area
	if (address <= 0x187F)
		SetColorRAM( address, data );

	return (DWORD) CurrentROM;
}

DWORD Write1Axx( BYTE data, WORD address )
{
	// remove any active FIRQ
	if (address == 0x1A00)
		IRQnotify &= ~FIRQline;

	return (DWORD) CurrentROM;
}

DWORD Write1Bxx( BYTE data, WORD address )
{
	// get joystick Y axis
	if (address == 0x1B00)
		REG1300 = GetJoystickYAxis();

	// get joystick X axis;
	else if (address == 0x1B01)
		REG1300 = GetJoystickXAxis();

	return (DWORD) CurrentROM;
}

DWORD Write20xxto3Fxx( BYTE data, WORD address )
{
	if (Bank2000 >= 3)
		return WriteUnknown( 0, 0 );

	// flip word values, so x86 processor can read them
	RAM2000t3FFF[Bank2000][(address-0x2000)^1] = data;

	// The following is for test purposes
//	if(Bank2000==0)
//		RAM2000t3FFF[Bank2000][address] = data;

	return (DWORD) CurrentROM;
}


DWORD WriteNull( BYTE data, WORD address )
{
	return (DWORD) CurrentROM;
}

DWORD WriteUnknown( BYTE data, WORD address )
{
	wsprintf( GlobalExitMessage, "Undefined write: $%0.4X=$%0.2X", address, (int) data );
	PostMessage( ghwndMain, WM_CLOSE, 0, 0 );

	return (DWORD) CurrentROM;
}