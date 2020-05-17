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

////////////////////////////////////////////////////////////////////////////////

// GAME CPU INFORMATION

// Game is controlled by a Motorola B6809-E clocked at 1.5 MHz.

// VIDEO CPU INFORMATION

// VIDEO CLOCK
// video clock runs at 5 MHz
// horizontal dot clock counts 0-319
// vertical dot clock counts 0-255

// VIDEO INTERRUPTS
// video hardware generates an INQ interrupt on the 6809.  These
// interrupts occur 4 times per frame, on scan lines 48, 112,
// 176 and 240.  These interrupts requests are latched, and are
// typically cleared by the 6809 program in the IQR interrupt
// service routine.  However, the video hardware will automatically
// clear the IRQ assertion if it is not processed within 32 scan
// lines.  Thus, the IRQ is deasserted on scan lines  80, 144, 208
// and 16 (240 + 32 - 256).

// VBLANK
// the VBLANK signal is active low, and is asserted from
// scan lines 240 - 255.

////////////////////////////////////////////////////////////////////////////////

// Video clocks 320 x 256 pixels per frame, at 5 MHz
#define FRAMES_PER_SECOND (5000000.0/320/256)

// 6809 E clock runs at 1.5MHz
#define CYCLES_PER_FRAME (1500000/FRAMES_PER_SECOND)

// we have to stop emulation every 16 scan lines to handle
// vidoe IRQs and VBLANK toggling
#define SCANLINES_EMULATED 16

// this is the scanline that VBLANK is toggled on
#define VBLANK_SCANLINE 240

#define CYCLES_TO_EMULATE (DWORD) (CYCLES_PER_FRAME/(256/SCANLINES_EMULATED)+0.5)

#pragma check_stack(off)

static void Initialize6809( void )
{
	int n;
	BYTE * ptr;

	// Initialize all 6809 engine page vectors to default values
	for (n=0; n<256; n++)
	{
		ReadPointer[n] =
		WritePointer[n] = (DWORD) 0;
		ReadFunction[n] = (DWORD) ReadUnknown;
		WriteFunction[n] = (DWORD) WriteUnknown;
	}

	// $00xx-$07xx  fixed working RAM
	for (n=0x00, ptr=RAM0000t07FF; n<=0x07; n++, ptr+=0x100)
		ReadPointer[n] =
		WritePointer[n] = (DWORD) ptr;

	// $08xx-$0Fxx  banked working RAM
	SelectBank0800t0FFF();

	// $10xx        input registers
	ReadFunction[0x10] = (DWORD) Read10xx;

	// $11xx        output registers
	WriteFunction[0x11] = (DWORD) Write11xx;

	// $12xx        EERAM
	ReadPointer[0x12] =
	WritePointer[0x12] = (DWORD) EERAM1200t12FF;

	// $13xx        ADC result register
	ReadFunction[0x13] = (DWORD) Read13xx;

	// $14xx        quad-pokey
	ReadFunction[0x14] = (DWORD) Read14xx;
	WriteFunction[0x14] = (DWORD) Write14xx;

	// $15xx-$17xx
	WriteFunction[0x15] =
	WriteFunction[0x16] =
	WriteFunction[0x17] = (DWORD) WriteNull;

	// $18xx        color RAM
	WriteFunction[0x18] = (DWORD) Write18xx;

	// $19xx
	WriteFunction[0x19] = (DWORD) WriteNull;

	// $1Axx        de-assert FIRQ
	WriteFunction[0x1A] = (DWORD) Write1Axx;

	// $1Bxx        ADC control
	WriteFunction[0x1B] = (DWORD) Write1Bxx;

	// $1Cxx-$1Fxx  alphanumerics RAM
	for (n=0x1C, ptr=RAM1C00t1FFF; n<=0x1F; n++, ptr+=0x100)
		ReadPointer[n] = 
		WritePointer[n] = (DWORD) ptr;

	// $20xx-$3Fxx banked math RAM
	for (n=0x20; n<=0x3F; n++)
	{
		ReadFunction[n] = (DWORD) Read20xxto3Fxx;
		WriteFunction[n] = (DWORD) Write20xxto3Fxx;
	}	
	SelectBank2000t3FFF();

	// $40xx-$5Fxx  banked program ROM
	SelectBank4000t5FFF();

	// $60xx-$FFxx  fixed program ROM
	for (n=0x60, ptr=ROM[0]+0x6000; n<=0xFF; n++, ptr+=0x100)
		ReadPointer[n] = (DWORD) ptr;
	
	// Reset the 6809
	Reset6809( CurrentROM );
}

static DWORD WINAPI IRobot6809Thread( void )
{
	BYTE ScanLine = 0;
	int Running = TRUE;
	int CounterExists;
	LARGE_INTEGER Timer, Delta, Timeout, Frequency;

	Initialize6809();

	// Get program timing information from the high performance timer
	CounterExists = QueryPerformanceFrequency( &Frequency );
	if (CounterExists)
	{
		Timeout.QuadPart = Frequency.QuadPart / (__int64) (FRAMES_PER_SECOND+0.5);
		Frequency.QuadPart = Frequency.QuadPart / 1000;
		QueryPerformanceCounter( &Timer );
	}

	// Emulation is initially unpaused
	Paused = FALSE;

	// start the pokey audio
	PlayWinPokey();

	// continually run 6809 emulator until an error
	// occurs or the user exits the program
	while (Running)
	{
		// execute the 6809
		if (Run6809(CYCLES_TO_EMULATE) < 0)
		{
			Running = IsWindow(ghwndMain);

			if (Running)
			{
				ScanLine += SCANLINES_EMULATED;

				// Toggle IRQ line
				//  IRQ asserted on lines:   48, 112, 176, 240
				//  IRQ deasserted on lines: 16,  80, 144, 208
				if (ScanLine == 48 || ScanLine==112 || ScanLine == 176 || ScanLine==240 )
					IRQnotify |= IRQline;
				else if (ScanLine == 16 || ScanLine==80 || ScanLine == 144 || ScanLine==208 )
					IRQnotify &= ~IRQline;

				// Generate VBLANK
				if (ScanLine >= VBLANK_SCANLINE)
					REG1080 |= VBLANK;
				else
					REG1080 &= ~VBLANK;

				// process the current sound buffer
				ProcessSound();

				// display screen and perform speed throttling once per frame
				if (ScanLine == VBLANK_SCANLINE)
				{
					// display new screen
					if (RenderEveryFrame)
						DisplayScreenBuffer();

					// Perform speed throttling
					if (SpeedThrottling && CounterExists)
					{
						do
						{
							QueryPerformanceCounter( &Delta );
							Delta.QuadPart -= Timer.QuadPart;
						} while (Delta.QuadPart < Timeout.QuadPart);
						QueryPerformanceCounter( &Timer );
					}
				}
	
				Running = IsWindow(ghwndMain);
			}
		}
		else
		{
			// Display error message if return value is non-zero
			wsprintf( GlobalExitMessage, "Illegal instruction at address $%0.4X : %0.2X %0.2X %0.2X %0.2X %0.2X\nA=%0.2X  B=%0.2X  CC=%0.2X  DP=%02.X\nX=%0.4X  Y=%0.4X  S=%0.4X  U=%0.4X\n",
				(int) R6809PC,
				(int) CurrentROM[R6809PC], (int) CurrentROM[R6809PC+1], (int) CurrentROM[R6809PC+2], (int) CurrentROM[R6809PC+3], (int) CurrentROM[R6809PC+4],
				(int) R6809A,
				(int) R6809B,
				(int) R6809CC,
				(int) R6809DP,
				(int) R6809X,
				(int) R6809Y,
				(int) R6809S,
				(int) R6809U );
			Running = FALSE;
		}

		while ((Paused || !ApplicationActive || ShowConfigScreen) && Running)
		{
			if (ApplicationActive)
			{
				if (ShowConfigScreen)
					RunConfigScreen();
				else
					DisplayScreenBuffer();
			}
			Sleep(100);
			Running = IsWindow(ghwndMain);
		}
	}

	PostMessage( ghwndMain, WM_CLOSE, 0, 0 );
	
	return 0;
}

BOOL StartEmulationThread( void )
{
	// Allocate memory resources needed by program
	if (!AllocateMemory())
		return FALSE;

	// Load ROM images
	if(!LoadROMs())
		return FALSE;

	// Load EEPROM image
	LoadEEPROM();

	// Create an initially suspended thread for 6809 emulation
	gh6809Thread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) IRobot6809Thread, NULL, CREATE_SUSPENDED, &gdwThreadID );
	if (!gh6809Thread)
		return ErrorBox( "Couldn't create emulation thread" );

	return TRUE;
}

void StopEmulationThread( void )
{
	// Resume the thread if it is currently suspended
	ResumeThread( gh6809Thread );

	// Wait up to 1000 milliseconds for thread to terminate
	if (WaitForSingleObject( gh6809Thread, 1000 ) == WAIT_TIMEOUT)
		// Kill the emulation engine if it hasn't closed
		CloseHandle( gh6809Thread );

	// Save EEPROM image
	SaveEEPROM();

	// Free all memory resources allocated by the program
	FreeMemory();
}
