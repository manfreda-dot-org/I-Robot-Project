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

#ifndef WIN32
#define WIN32
#endif

#include <windows.h>
#include <mmsystem.h>
#include "ddraw.h"
#include "d3d.h"
#include "WinPokey.h"

#define FASTCALL __fastcall

// Key mapping
#define KEY_START1          '1'
#define KEY_START2          '2'
#define KEY_FIRE1           VK_SPACE
#define KEY_LEFTCOIN        VK_F2
#define KEY_RIGHTCOIN       VK_F3
#define KEY_AUXCOIN         VK_F4

////////////////////////////////////////////////////////////////////////////////
// Global strings
////////////////////////////////////////////////////////////////////////////////

enum {	STR_WELCOME1,
		STR_WELCOME2,
		STR_WELCOME3,
		STR_MENU1,
		STR_MENU2,
		STR_MENU3,
		STR_MENU4 };

extern char *GlobalStrings[];
BOOL EnsureExecutableIntegrity( void );

////////////////////////////////////////////////////////////////////////////////
// Global section
////////////////////////////////////////////////////////////////////////////////

extern BOOL ApplicationCorrupt;
extern HINSTANCE ghInstance;
extern HWND ghwndMain;
char GlobalExitMessage[256];

extern volatile int Paused;
extern volatile int ApplicationActive;
extern volatile int ShowConfigScreen;

extern volatile BYTE ExtendedScreen;
extern volatile BYTE DisplayFPS;
extern volatile BYTE SpeedThrottling;
extern volatile BYTE ShowDots;
extern volatile BYTE ShowVectors;
extern volatile BYTE ShowPolygons;
extern volatile BYTE ShowAlphanumerics;
extern volatile BYTE RenderEveryFrame;

extern volatile int PerformClipDumpNextFrame;
extern volatile int PerformingClipDump;
extern volatile HDC hdcMetafile;

extern WPARAM LastKey;
extern const char RegistrySubkey[];

BOOL ErrorBox( char * Message );
void DisplayMessage( int x, int y, int color, char * string );

void IRGetClipboard( void );
void IRCloseClipboard( void );

////////////////////////////////////////////////////////////////////////////////
// Joystick section
////////////////////////////////////////////////////////////////////////////////

#define HALL_JOY_MID 128
#define MAX_HALL_DELTA 48

extern BOOL JoystickPresent;
extern JOYINFO JoyInfo;
extern JOYCAPS JoyCaps;
extern float JoyScaleX;
extern float JoyScaleY;
extern UINT JoyUpThreshold;
extern UINT JoyDownThreshold;
extern UINT JoyLeftThreshold;
extern UINT JoyRightThreshold;

enum { USE_ANALOG_JOYSTICK, USE_DIGITAL_JOYSTICK, USE_KEYBOARD };

extern BYTE GameControls;

BYTE GetJoystickXAxis( void );
BYTE GetJoystickYAxis( void );
BOOL IsFireButtonPressed( void );
BOOL IsStart1ButtonPressed( void );
BOOL IsStart2ButtonPressed( void );

////////////////////////////////////////////////////////////////////////////////
// Generic application section
////////////////////////////////////////////////////////////////////////////////

BOOL StartApplication( void );
void StopApplication( void );
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void RunConfigScreen( void );
void LoadDefaultConfig( void );
void SaveDefaultConfig( void );

////////////////////////////////////////////////////////////////////////////////
// DirectX section
////////////////////////////////////////////////////////////////////////////////

extern LPDIRECTDRAW lpDD;
extern LPDIRECTDRAWSURFACE lpFrontBuffer;
extern LPDIRECTDRAWSURFACE lpBackBuffer;
extern LPDIRECTDRAWSURFACE lpAlphaBuffer;
extern LPDIRECTDRAWSURFACE lpScreenBuffer1;
extern LPDIRECTDRAWSURFACE lpScreenBuffer2;
extern LPDIRECTDRAWPALETTE lpGamePalette;
extern PALETTEENTRY ColorRAM[256];

extern BYTE * ScreenBuffer;
extern BYTE * EndOfScreenBuffer;
extern int ScreenPitch;
extern unsigned int ClipScreen;

BOOL StartVideoHardware( void );
void StopVideoHardware( void );
BOOL RestorePrimaryBuffer( void );
BOOL RestoreScreenBuffer1( void );
BOOL RestoreScreenBuffer2( void );
BOOL RestoreAlphaBuffer( void );
BOOL RestoreAllSurfaces( void );

void _inline DrawDot( int x, int y, int color );
void _inline DrawVector( int x1, int y1, int x2, int y2, int color );
void _inline DrawPolygon( POINT * vertex, int length, int color );

////////////////////////////////////////////////////////////////////////////////
// Video emulation section
////////////////////////////////////////////////////////////////////////////////

#define VIDRAM0 lpScreenBuffer1
#define VIDRAM1 lpScreenBuffer2

#define BLUE 2
#define GREEN 1
#define YELLOW 0
#define RED 3

void FASTCALL EraseScreenBuffer( LPDIRECTDRAWSURFACE lpVIDRAM );
void ChangeScreenBuffer( void );
void FASTCALL RenderAlphanumeric( WORD address, BYTE data );
void DisplayScreenBuffer( void );
void SetAlphaPalette( void );
void FASTCALL SetColorRAM( WORD address, BYTE data );

void DisplayChar( int character, int x, int y, int color );
int DisplayString( char * string, int x, int y, int color );

////////////////////////////////////////////////////////////////////////////////
// Audio section
////////////////////////////////////////////////////////////////////////////////

extern BYTE SoundSupported;
extern BYTE SoundEnabled;

////////////////////////////////////////////////////////////////////////////////
// Mathbox emulation section
////////////////////////////////////////////////////////////////////////////////

void DoMathbox( void );

////////////////////////////////////////////////////////////////////////////////
// 6809 emulation section
////////////////////////////////////////////////////////////////////////////////

#define COM0RAM RAM2000t3FFF[1]
#define COM1RAM RAM2000t3FFF[2]

extern DWORD gdwThreadID;
extern HANDLE gh6809Thread;

extern BYTE * RAM0000t07FF;
extern BYTE * RAM0800t0FFF[3];
extern BYTE * EERAM1200t12FF;
extern BYTE * RAM1C00t1FFF;
extern BYTE * RAM2000t3FFF[9];
extern BYTE * ROM[6];
extern BYTE * CurrentROM;
extern BYTE * AlphaROM;
extern BYTE * ColorROM;
extern WORD * MathRAMROM;

extern BYTE REG1000;
extern BYTE REG1040;
extern BYTE REG1080;
extern BYTE REG1140;
extern BYTE REG1180;
extern BYTE REG11C0;
extern BYTE REG1300;

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

extern BYTE DIPSWITCH3J;
extern BYTE DIPSWITCH5E;

extern unsigned int Bank0800;
extern unsigned int Bank2000;
extern unsigned int BankROM;

BOOL StartEmulationThread( void );
void StopEmulationThread( void );

BOOL AllocateMemory( void );
void FreeMemory( void );
BOOL LoadROMs( void );
void LoadEEPROM( void );
void SaveEEPROM( void );
void FASTCALL SelectBank0800t0FFF( void );
void FASTCALL SelectBank2000t3FFF( void );
void FASTCALL SelectBank4000t5FFF( void );

BYTE Read10xx( WORD address );
BYTE Read13xx( WORD address );
BYTE Read14xx( WORD address );
BYTE Read20xxto3Fxx( WORD address );
BYTE ReadUnknown( WORD address );

DWORD Write11xx( BYTE data, WORD address );
DWORD Write14xx( BYTE data, WORD address );
DWORD Write18xx( BYTE data, WORD address );
DWORD Write1Axx( BYTE data, WORD address );
DWORD Write1Bxx( BYTE data, WORD address );
DWORD Write20xxto3Fxx( BYTE data, WORD address );
DWORD WriteNull( BYTE data, WORD address );
DWORD WriteUnknown( BYTE data, WORD address );

////////////////////////////////////////////////////////////////////////////////
// 6809 object definitions
////////////////////////////////////////////////////////////////////////////////

#define IRQline 0x02
#define FIRQline 0x20

extern BYTE R6809A, R6809B, R6809DP, R6809CC;
extern WORD R6809X, R6809Y, R6809U, R6809S, R6809PC;
extern DWORD ReadPointer[256], ReadFunction[256];
extern DWORD WritePointer[256], WriteFunction[256];
extern DWORD IRQnotify;

extern void Reset6809( BYTE * Ptr64K );
extern int Run6809( DWORD CyclesToRun );
