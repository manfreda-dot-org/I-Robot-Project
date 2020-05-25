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

#ifndef HardwareH
#define HardwareH
//---------------------------------------------------------------------------

// REG1000 bit definitions
#define TESTSWITCH  0x10
#define AUXCOIN     0x20
#define LEFTCOIN    0x40
#define RIGHTCOIN   0x80

// REG1040 bit definitions
#define FIRE1       0x10
#define START2      0x40
#define START1      0x80

// REG1080 bit definitions
#define MBDONE      0x20
#define EXTDONE     0x40
#define VBLANK      0x80

// REG1140 bit definitions
#define ERASE		0x01
#define BUFSEL		0x02
#define EXTSTART	0x04
#define ADDCON      0x08
#define MATHSTART	0x10
#define COCKTAIL    0x20
#define RECALL      0x40
#define EXTCOMSWAP	0x80

// REG1180 bit definitions
#define MPAGE1      0x02
#define MPAGE2      0x04
#define OUT03       0x08
#define OUT04       0x10
#define _2PLAYRAM   0x20
#define _800FLIP    0x40
#define ALPHAMAP1   0x80

// REG11C0 bit definitions
#define BANK0 		0x02
#define BANK1   	0x04
#define BANK2       0x08
#define LED1     	0x10
#define LED2        0x20
#define COUNTERL    0x40
#define COUNTERR  	0x80

#include "Alphanumerics.h"
#include "CPU.h"
#include "Mathbox.h"
#include "ProgramROM.h"
#include "RAM.h"
#include "ColorRAM.h"
#include "Registers.h"

class TIRobotHardware
{
public:
        // 6809 processor
        TCPU CPU;

        // memory mapped I/O
        TAlphanumerics Alphanumerics;
        TMathbox Mathbox;
        TRAM RAM;
        TColorRAM ColorRAM;
        TRegisters Registers;
        TProgramROM Program;

        BYTE EEPROM[0x100]; // EEPROM

        void __fastcall Boot(void);
};

extern TIRobotHardware Hardware;

//---------------------------------------------------------------------------
#endif