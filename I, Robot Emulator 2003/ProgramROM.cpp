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

#include "ProgramROM.h"
#include "Emulator.h"
#include "ROM.h"

__fastcall TProgramROM::TProgramROM()
{
        // load actual program ROMs
        TROM _405, _206, _207, _208, _209, _210;
        if (!_405.Load("136029.405", 0x4000, 0x150A97)) return;
        if (!_206.Load("136029.206", 0x4000, 0x174942)) return;
        if (!_207.Load("136029.207", 0x4000, 0x17384C)) return;
        if (!_208.Load("136029.208", 0x2000, 0xD5E26)) return;
        if (!_209.Load("136029.209", 0x4000, 0x1A1B59)) return;
        if (!_210.Load("136029.210", 0x4000, 0x179092)) return;

        // rearrange ROMs into proper banks
        memcpy(&mROM[0][0x4000], &_405.Data[0x0000], 0x2000); // 4000-5FFF bank 0
        memcpy(&mROM[1][0x4000], &_405.Data[0x2000], 0x2000); // 4000-5FFF bank 1
        memcpy(&mROM[2][0x4000], &_206.Data[0x0000], 0x2000); // 4000-5FFF bank 2
        memcpy(&mROM[3][0x4000], &_206.Data[0x2000], 0x2000); // 4000-5FFF bank 3
        memcpy(&mROM[4][0x4000], &_207.Data[0x0000], 0x2000); // 4000-5FFF bank 4
        memcpy(&mROM[5][0x4000], &_207.Data[0x2000], 0x2000); // 4000-5FFF bank 5
        memcpy(&mROM[0][0x6000], &_208.Data[0x0000], 0x2000); // 6000-7FFF
        memcpy(&mROM[0][0x8000], &_209.Data[0x0000], 0x4000); // 8000-BFFF
        memcpy(&mROM[0][0xC000], &_210.Data[0x0000], 0x4000); // C000-FFFF

        // ensure 6000-FFFF is mirrored in all banks
	for (int n=1; n<6; n++)
                memcpy(&mROM[n][0x6000], &mROM[0][0x6000], 0xA000);

        // mathbox self-test hack
/*        BYTE hack[] = { 0x40,0x00,0x00,0x00,0x01,0x21,
                0x00,0x00,0x40,0x00,0x00,0x00,
                0xFE,0xE1,0x00,0x00,0x40,0x01,
                0x40,0x01,0xFE,0xE1,0x01,0x21,
                0x01,0x21,0x40,0x00,0x00,0x00,
                0xFE,0xE9,0x00,0x05,0x40,0x01,
                0x40,0x01,0xFE,0xE8,0x01,0x2C,
                0x01,0x21,0x40,0x01,0xFE,0xE1,
                0xFE,0xE9,0x01,0x22,0x3F,0xFC,
                0x40,0x01,0x01,0x21,0xFE,0xE9,
                0xFE,0xE8,0x40,0x01,0x01,0x22,
                0x01,0x2C,0xFE,0xE1,0x3F,0xFC };
        memcpy(&ROM[4][0x5CAD], hack, sizeof(hack));
*/
}

void __fastcall TProgramROM::InitIO(void)
{
        // setup 6809 page vector pointers

	// $60xx-$FFxx  fixed program ROM
        BYTE * ptr = &mROM[0][0x6000];
	for (int n=0x60; n<=0xFF; n++, ptr+=0x100)
		M6809_PageReadPointer[n] = (DWORD) ptr;

        // setup bank switched areas
        Bank = -1;
        BankSwitch();
}

void __fastcall TProgramROM::BankSwitch(void)
{
        int bank = (Hardware.Registers._11C0 >> 1) & 7;

        if (bank > 5)
        {
                Terminate("Illegal program bank selection: 11C0=" + IntToHex(Hardware.Registers._11C0, 2));
                return;
        }

	if (bank != Bank)
        {
                Bank = bank;
                ROM = mROM[bank];
                BYTE * ptr = ROM + 0x4000;
                for (int n=0x40; n<=0x5F; n++, ptr+=0x100)
                        M6809_PageReadPointer[n] = (DWORD) ptr;
	}
}

