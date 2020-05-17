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

#include <windows.h>
#include <mmsystem.h>
#include <ddraw.h>

////////////////////////////////////////////////////////////////////////////////
// strings used by this program
////////////////////////////////////////////////////////////////////////////////

char *GlobalStrings[] = {
	"I ROBOT : THE EMULATOR     VERSION 0.98",
	"c1997 1998 JOHN MANFREDA",
	"PRESS F1 FOR OPTIONS",
	"I ROBOT:THE EMULATOR  V0.98 06-19-98",
	"c1997 1998 JOHN MANFREDA",
	"ALL RIGHTS RESERVED",
	"ORIGINAL PROGRAM c1983 ATARI",
	"" };

////////////////////////////////////////////////////////////////////////////////
// Global section
////////////////////////////////////////////////////////////////////////////////

// application management variables
BOOL ApplicationCorrupt = TRUE;
HINSTANCE ghInstance = NULL;
HWND ghwndMain = NULL;
char GlobalExitMessage[256] = "";

// generic emulation configuration variables
BYTE ExtendedScreen = FALSE;
BYTE DisplayFPS = FALSE;
BYTE SpeedThrottling = TRUE;
BYTE ShowDots = TRUE;
BYTE ShowVectors = TRUE;
BYTE ShowPolygons = TRUE;
BYTE ShowAlphanumerics = TRUE;
BYTE RenderEveryFrame = TRUE;

// generic state variables
int Paused = FALSE;
int ApplicationActive = FALSE;
int ShowConfigScreen = FALSE;

// clipboard variables
int PerformClipDumpNextFrame = FALSE;
int PerformingClipDump = FALSE;
HDC hdcMetafile = NULL;

WPARAM LastKey = 0;
const char RegistrySubkey[] = "Software\\FritoSoft\\I, Robot\\0.98";

////////////////////////////////////////////////////////////////////////////////
// Joystick section
////////////////////////////////////////////////////////////////////////////////

BOOL JoystickPresent = FALSE;
JOYINFO JoyInfo;
JOYCAPS JoyCaps;
float JoyScaleX;
float JoyScaleY;
UINT JoyUpThreshold;
UINT JoyDownThreshold;
UINT JoyLeftThreshold;
UINT JoyRightThreshold;

BYTE GameControls;

////////////////////////////////////////////////////////////////////////////////
// Video section
////////////////////////////////////////////////////////////////////////////////

LPDIRECTDRAW lpDD = NULL;
LPDIRECTDRAWSURFACE lpFrontBuffer = NULL;
LPDIRECTDRAWSURFACE lpBackBuffer = NULL;
LPDIRECTDRAWSURFACE lpAlphaBuffer = NULL;
LPDIRECTDRAWSURFACE lpScreenBuffer1 = NULL;
LPDIRECTDRAWSURFACE lpScreenBuffer2 = NULL;
LPDIRECTDRAWPALETTE lpGamePalette = NULL;
PALETTEENTRY ColorRAM[256];

BYTE * ScreenBuffer;
BYTE * EndOfScreenBuffer;
int ScreenPitch;
unsigned int ClipScreen = 2;

////////////////////////////////////////////////////////////////////////////////
// Audio section
////////////////////////////////////////////////////////////////////////////////

BYTE SoundSupported = FALSE;
BYTE SoundEnabled = FALSE;

////////////////////////////////////////////////////////////////////////////////
// Mathbox emulation section
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// 6809 emulation section
////////////////////////////////////////////////////////////////////////////////

DWORD gdwThreadID = 0;
HANDLE gh6809Thread = NULL;

BYTE * RAM0000t07FF = NULL;
BYTE * RAM0800t0FFF[3] = { NULL, NULL, NULL };
BYTE * EERAM1200t12FF = NULL;
BYTE * RAM1C00t1FFF = NULL;
BYTE * RAM2000t3FFF[9] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
BYTE * ROM[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
BYTE * CurrentROM = NULL;
BYTE * AlphaROM = NULL;
BYTE * ColorROM = NULL;
WORD * MathRAMROM = NULL;

BYTE REG1000 = 0xFF;
BYTE REG1040 = 0xFF;
BYTE REG1080 = 0xBF;
BYTE REG1140 = 0x00;
BYTE REG1180 = 0x00;
BYTE REG11C0 = 0x00;
BYTE REG1300 = 0x00;

// 3J = 11111111
BYTE DIPSWITCH3J = 0x00;
// 5E = 11111111;
BYTE DIPSWITCH5E = 0xFF;

unsigned int Bank0800 = 0xFF;
unsigned int Bank2000 = 0xFF;
unsigned int BankROM = 0xFF;
