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

#include "Hardware.h"
#include "Emulator.h"
#pragma check_stack(off)

TIRobotHardware Hardware;

//---------------------------------------------------------------------------

static BYTE ReadUnknown(WORD address)
{
        Terminate("Undefined read at address $" + IntToHex(address, 4));
	return 0;
}

static DWORD WriteUnknown( BYTE data, WORD address )
{
        Terminate("Undefined write: " + IntToHex(address,4) + "=" + IntToHex((int) data, 2));
	return (DWORD) Hardware.Program.ROM;
}

static DWORD WriteNull( BYTE data, WORD address )
{
	return (DWORD) Hardware.Program.ROM;
}

//---------------------------------------------------------------------------

void __fastcall TIRobotHardware::Boot(void)
{
        // reset all 6809 I/O page vectors
	for (int n=0; n<256; n++)
	{
		M6809_PageReadPointer[n] =
		M6809_PageWritePointer[n] = (DWORD) 0;
		M6809_PageReadFunction[n] = (DWORD) ReadUnknown;
		M6809_PageWriteFunction[n] = (DWORD) WriteUnknown;
	}

        // initialize 6809 I/O
	M6809_PageReadPointer[0x12] = M6809_PageWritePointer[0x12] = (DWORD) EEPROM; // 12xx EEPROM
	M6809_PageWriteFunction[0x15] = (DWORD) WriteNull; // $15xx undefined
	M6809_PageWriteFunction[0x16] = (DWORD) WriteNull; // $16xx undefined
	M6809_PageWriteFunction[0x17] = (DWORD) WriteNull; // $17xx undefined
	M6809_PageWriteFunction[0x19] = (DWORD) WriteNull; // $19xx undefined

        // initialize 6809 I/O on subsystems
        Alphanumerics.InitIO();
        Mathbox.InitIO();
        RAM.InitIO();
        ColorRAM.InitIO();
        Registers.InitIO();
        Program.InitIO();

        // reset the 6809
        M6809_Reset(Program.ROM);

        Log.Add("Hardware reset");
}
