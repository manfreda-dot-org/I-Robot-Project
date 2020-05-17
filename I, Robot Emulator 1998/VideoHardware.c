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

static RECT rc640 = { 0, 0, 640, 480 };
static RECT rc512 = { 0, 0, 512, 464 };
static RECT rc64_8 = { 64, 8, 512+64, 464+8 };

// All alphanumerics related code is here
#include "Alphanumerics.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void FASTCALL EraseScreenBuffer( LPDIRECTDRAWSURFACE lpVIDRAM )
{
    DDBLTFX ddbltfx;
    ddbltfx.dwSize = sizeof( DDBLTFX );
    ddbltfx.dwFillColor = 0;
	if (lpVIDRAM->lpVtbl->Blt( lpVIDRAM, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx ) == DDERR_SURFACELOST)
		RestorePrimaryBuffer();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ChangeScreenBuffer( void )
{
	LPDIRECTDRAWSURFACE lpOld = (REG1140&BUFSEL) ? VIDRAM1 : VIDRAM0;
	LPDIRECTDRAWSURFACE lpNew = (REG1140&BUFSEL) ? VIDRAM0 : VIDRAM1;
	static BOOL OldLocked = FALSE;
	static DDSURFACEDESC ddsd;

	// unlock previous surface
	if (OldLocked)
		lpOld->lpVtbl->Unlock( lpOld, &ddsd );

	// lock the new surface
	ddsd.dwSize = sizeof(DDSURFACEDESC);
	if (lpNew->lpVtbl->Lock( lpNew, NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL ) == DD_OK)
	{
		ScreenBuffer = ddsd.lpSurface;
		ScreenPitch = ddsd.lPitch;
		EndOfScreenBuffer = ScreenBuffer + 480 * ScreenPitch;
		OldLocked = TRUE;

		if (PerformingClipDump)
			SendMessage( ghwndMain, WM_USER, 0, 0 );
		else if (PerformClipDumpNextFrame)
		{
			PerformClipDumpNextFrame = FALSE;
			PerformingClipDump = TRUE;
		}
	}
	else
		OldLocked = FALSE;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static void DisplayFrameRate( void )
{
	static BOOL flag = FALSE;
	static int FPS = 0;
	static int tempFPS = 0;
	static DWORD LastTime;
	DWORD ThisTime, Delta;
	int temp;

	// always add one frame to the count
	tempFPS++;

	// get starting clock time
	if (!flag)
	{
		LastTime = timeGetTime();
		flag = TRUE;
	}
	
	// determine if a second has elapsed
	ThisTime = timeGetTime();
	Delta = ThisTime - LastTime;
	if (Delta >= 1000)
	{
		FPS = min( 9999, tempFPS * 1000 / Delta);
		tempFPS = 0;
		LastTime = ThisTime;
	}

	// display the current FPS
	temp = FPS;
	if (temp > 999)
	{
		DisplayChar( (temp/1000) + '0', 576, 464, 0 );
		temp %= 1000;
	}
	if (temp > 99)
	{
		DisplayChar( (temp/100) + '0', 592, 464, 0 );
		temp %= 100;
	}
	if (temp > 9)
		DisplayChar( (temp/10) + '0', 608, 464, 0 );
	DisplayChar( (temp%10) + '0', 624, 464, 0 );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void DisplayScreenBuffer( void )
{
	// blit the playfield onto the back screen buffer
	if (ExtendedScreen)
	{
		if(lpBackBuffer->lpVtbl->BltFast( lpBackBuffer, 0, 0, (REG1140&BUFSEL) ? VIDRAM1 : VIDRAM0, &rc640, DDBLTFAST_WAIT | DDBLTFAST_NOCOLORKEY ) == DDERR_SURFACELOST)
			RestorePrimaryBuffer();
	}
	else
	{
		if (ClipScreen)
		{
			ClipScreen--;
			EraseScreenBuffer( lpBackBuffer );
		}
		if (lpBackBuffer->lpVtbl->BltFast( lpBackBuffer, 64, 8, (REG1140&BUFSEL) ? VIDRAM1 : VIDRAM0, &rc64_8, DDBLTFAST_WAIT | DDBLTFAST_NOCOLORKEY ) == DDERR_SURFACELOST)
			RestorePrimaryBuffer();
	}

	// transparently render alphanumerics onto back buffer
	if (ShowAlphanumerics)
		RenderAllAlphanumerics();

	if (Paused)
		DisplayString( "PAUSED", 272, 232, 0 );
	else if (DisplayFPS)
		DisplayFrameRate();

	DisplayWelcomeMessage();

	// change the palette
	lpGamePalette->lpVtbl->SetEntries( lpGamePalette, 0, 0, 70, ColorRAM );

	// page flip the PC video buffers
	if (lpFrontBuffer->lpVtbl->Flip( lpFrontBuffer, NULL, DDFLIP_WAIT ) == DDERR_SURFACELOST)
		RestorePrimaryBuffer();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void FASTCALL SetColorRAM( WORD address, BYTE data )
{
	PALETTEENTRY * ptr = &ColorRAM[((address - 0x1800) >> 1) & 0x3F];
	BYTE i;
 
	data = ~data;
	i = (((data & 0x03) << 1) + (~address&1) + 1) * 8;
	ptr->peRed = ((data >> 6) & 3) * i;
	ptr->peGreen = ((data >> 4) & 3) * i;
	ptr->peBlue = ((data >> 2) & 3) * i;
}
