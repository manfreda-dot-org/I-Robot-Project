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

#include "Registers.h"
#include "Emulator.h"

//---------------------------------------------------------------------------

static BYTE Read10xx(WORD address)
{
	if (address == 0x1000)
		return Hardware.Registers._1000;

	if (address == 0x1040)
                return Hardware.Registers._1040;

	if (address == 0x1080)
		return Hardware.Registers._1080;

//	if (address == 0x10C0)
		return Config.Dipswitch3J;
}

//---------------------------------------------------------------------------

static DWORD Write11xx(BYTE data, WORD address)
{
	// clear IRQ
	if (address == 0x1100)
		M6809_IRQ_Pending &= ~IRQ;

	else if (address == 0x1140)
	{
		BYTE toggled = Hardware.Registers._1140 ^ data;
		Hardware.Registers._1140 = data;

		// check if EXTCOMSWAP was toggled
		if (EXTCOMSWAP & toggled)
                        Hardware.Mathbox.BankSwitch();

		// change screen buffer if BUFSEL was toggled
		if (BUFSEL & toggled)
                        Hardware.Mathbox.ChangeScreenBuffer();

                // check if ERASE goes from 0->1
		if (ERASE & toggled & Hardware.Registers._1140)
                        Hardware.Mathbox.EraseScreenBuffer();

		// check if MATHSTART goes from 0->1
		if (MATHSTART & toggled & Hardware.Registers._1140)
			Hardware.Mathbox.Execute();
	}

	else if (address == 0x1180)
	{
		Hardware.Registers._1180 = data;
                Hardware.RAM.BankSwitch();
                Hardware.Mathbox.BankSwitch();
	}

	else if (address == 0x11C0)
	{
		Hardware.Registers._11C0 = data;
                Hardware.Program.BankSwitch();
	}

	return (DWORD) Hardware.Program.ROM;
}


static BYTE Read13xx(WORD address)
{
	return Hardware.Registers._1300;
}

//---------------------------------------------------------------------------

static BYTE Read14xx(WORD address)
{
	if (address == 0x1420)
		return Config.Dipswitch5E;

	return LOBYTE(rand());
}

static DWORD Write14xx(BYTE data, WORD address)
{
	// Quad-Pokey area
//	Update_pokey_sound(
//		(WORD) ((address&0x07)+((address&0x20)?8:0)),
//		data,
//		(BYTE)((address>>3)&0x03) );

	return (DWORD) Hardware.Program.ROM;
}

//---------------------------------------------------------------------------

static DWORD Write1Axx(BYTE data, WORD address)
{
	// clear FIRQ
	if (address == 0x1A00)
		M6809_IRQ_Pending &= ~FIRQ;

	return (DWORD) Hardware.Program.ROM;
}

//---------------------------------------------------------------------------

static DWORD Write1Bxx(BYTE data, WORD address)
{
	// get joystick Y axis
	if (address == 0x1B00)
		Hardware.Registers._1300 = GameInput.GetJoystickY();

	// get joystick X axis;
	else if (address == 0x1B01)
		Hardware.Registers._1300 = GameInput.GetJoystickX();

	return (DWORD) Hardware.Program.ROM;
}

//---------------------------------------------------------------------------

__fastcall TRegisters::TRegisters(void)
{
        // init registers
        _1000 = 0xFF;
        _1040 = 0xFF;
        _1080 = 0xBF;
        _1140 = 0x00;
        _1180 = 0x10;
        _11C0 = 0x00;
        _1300 = 0x80;
}

void __fastcall TRegisters::InitIO(void)
{
        M6809_PageReadFunction[0x10] = (DWORD) Read10xx; // $10xx
	M6809_PageWriteFunction[0x11] = (DWORD) Write11xx; // $11xx
	M6809_PageReadFunction[0x13] = (DWORD) Read13xx; // $13xx ADC result register
	M6809_PageReadFunction[0x14] = (DWORD) Read14xx; // $14xx quad pokey read
	M6809_PageWriteFunction[0x14] = (DWORD) Write14xx; // $14xx quad pokey write
	M6809_PageWriteFunction[0x1A] = (DWORD) Write1Axx; // $1Axx clear FIRQ
	M6809_PageWriteFunction[0x1B] = (DWORD) Write1Bxx; // $1Bxx ADC control
}

