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
#pragma comment(lib, "dxguid.lib")

static DDCAPS HardwareCaps;
static DDCAPS EmulatedCaps;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

BOOL RestorePrimaryBuffer( void )
{
    DDBLTFX ddbltfx;
	DDCOLORKEY ddck;
	
	// restore the surface
    if(lpFrontBuffer->lpVtbl->Restore(lpFrontBuffer) != DD_OK)
		return FALSE;

	// set the source blitting color key
    ddck.dwColorSpaceLowValue =
    ddck.dwColorSpaceHighValue = 0;
	if (lpFrontBuffer->lpVtbl->SetColorKey( lpFrontBuffer, DDCKEY_SRCBLT, &ddck ) != DD_OK)
		return FALSE;
	if (lpBackBuffer->lpVtbl->SetColorKey( lpBackBuffer, DDCKEY_SRCBLT, &ddck ) != DD_OK)
		return FALSE;

	// restore the palette
	if (lpGamePalette->lpVtbl->SetEntries( lpGamePalette, 0, 0, 256, ColorRAM ) != DD_OK)
		return FALSE;

	// attach a palette to the primary buffer
	if (lpFrontBuffer->lpVtbl->SetPalette( lpFrontBuffer, lpGamePalette ) != DD_OK)
		return FALSE;

	// Erase the front and back buffers
    ddbltfx.dwSize = sizeof( ddbltfx );
    ddbltfx.dwFillColor = 0;
	lpFrontBuffer->lpVtbl->Blt( lpFrontBuffer, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx );
	lpBackBuffer->lpVtbl->Blt( lpBackBuffer, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx );

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

BOOL RestoreScreenBuffer1( void )
{
	// restore the surface
    if(lpScreenBuffer1->lpVtbl->Restore(lpScreenBuffer1) != DD_OK)
		return FALSE;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

BOOL RestoreScreenBuffer2( void )
{
	// restore the surface
    if(lpScreenBuffer2->lpVtbl->Restore(lpScreenBuffer2) != DD_OK)
		return FALSE;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

BOOL RestoreAlphaBuffer( void )
{
	DDSURFACEDESC ddsd;
	DDCOLORKEY ddck;
	int n, c, s;
	BYTE * addr, by;
	RECT rcs, rcd;

	// restore the surface
    if(lpAlphaBuffer->lpVtbl->Restore(lpAlphaBuffer) != DD_OK)
		return FALSE;

	// lock the surface in memory
	ddsd.dwSize = sizeof(DDSURFACEDESC);
	if (lpAlphaBuffer->lpVtbl->Lock( lpAlphaBuffer, NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL ) != DD_OK)
		return FALSE;

	// expand the alphanumerics ROM into the buffer	
	for (n=0; n<64; n++)
		for (c=65; c<=68; c++)
			for (s=0; s<8; s++)
			{
				addr = AlphaROM + (n<<4) + s*2;
				by = (*addr << 4) | (*(addr+1) & 0xF);
				addr = (BYTE *) ddsd.lpSurface + ((c-65)<<3);
				addr += ((n<<3)+s) * ddsd.lPitch;
				*(addr++) = (by&0x80) ? c : 0;
				*(addr++) = (by&0x40) ? c : 0;					
				*(addr++) = (by&0x20) ? c : 0;
				*(addr++) = (by&0x10) ? c : 0;
				*(addr++) = (by&0x08) ? c : 0;
				*(addr++) = (by&0x04) ? c : 0;
				*(addr++) = (by&0x02) ? c : 0;
				*(addr++) = (by&0x01) ? c : 0;
			}

	// release the surface
	lpAlphaBuffer->lpVtbl->Unlock( lpAlphaBuffer, &ddsd );

	// stretch the characters to 2x normal size
	rcd.left=0; rcd.right=64;
	rcs.left=0; rcs.right=32;
	for (n=63; n>0; n--)
	{
		rcs.top = n << 3; rcs.bottom = rcs.top + 8;
		rcd.top = n << 4; rcd.bottom = rcd.top + 16;
		if (lpAlphaBuffer->lpVtbl->Blt( lpAlphaBuffer, &rcd, lpAlphaBuffer, &rcs, DDBLT_WAIT, NULL ) != DD_OK)
			return FALSE;
	}
	rcs.top = rcs.left = 0;
	rcs.bottom = 8;
	rcs.right = 32;
	if (lpAlphaBuffer->lpVtbl->BltFast( lpAlphaBuffer,  0, 8, lpAlphaBuffer, &rcs, DDBLTFAST_WAIT | DDBLTFAST_NOCOLORKEY ) != DD_OK)
		return FALSE;
	if (lpAlphaBuffer->lpVtbl->BltFast( lpAlphaBuffer, 32, 0, lpAlphaBuffer, &rcs, DDBLTFAST_WAIT | DDBLTFAST_NOCOLORKEY ) != DD_OK)
		return FALSE;
	if (lpAlphaBuffer->lpVtbl->BltFast( lpAlphaBuffer, 32, 8, lpAlphaBuffer, &rcs, DDBLTFAST_WAIT | DDBLTFAST_NOCOLORKEY ) != DD_OK)
		return FALSE;

	// set the color key (black)
    ddck.dwColorSpaceLowValue =
    ddck.dwColorSpaceHighValue = 0;
	if (lpAlphaBuffer->lpVtbl->SetColorKey( lpAlphaBuffer, DDCKEY_SRCBLT, &ddck ) != DD_OK)
		return FALSE;

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

BOOL RestoreAllSurfaces( void )
{
	// Restore the surfaces
	if (!RestorePrimaryBuffer())
		return FALSE;
	if (!RestoreScreenBuffer1())
		return FALSE;
	if (!RestoreScreenBuffer2())
		return FALSE;
	if (!RestoreAlphaBuffer())
		return FALSE;

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

BOOL StartVideoHardware( void )
{
	DDSURFACEDESC ddsd;
	DDSCAPS ddscaps;

	// Create the direct draw object
	if( DirectDrawCreate( NULL, &lpDD, NULL ) != DD_OK )
		return ErrorBox( "Couldn't create a DirectDraw object" );

	// Query DirectDraw to find it's capabilities
	lpDD->lpVtbl->GetCaps( lpDD, &HardwareCaps, &EmulatedCaps );

	// Get exclusive access to entire video display
	if( lpDD->lpVtbl->SetCooperativeLevel( lpDD, ghwndMain, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX) != DD_OK )
		return ErrorBox( "DirectDraw SetCooperativeLevel failed" );

	// Set display resolution to 640x480x8
	if ( lpDD->lpVtbl->SetDisplayMode( lpDD, 640, 480, 8) != DD_OK )
		return ErrorBox( "Couldn't set the display mode to 640x480x8" );

	// Create primary page flipped surface
	ZeroMemory( &ddsd, sizeof( ddsd ) );
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddsd.dwBackBufferCount = 1;
	if (lpDD->lpVtbl->CreateSurface( lpDD, &ddsd, &lpFrontBuffer, NULL ) != DD_OK)
		return ErrorBox( "Couldn't create a page flipped surface" );

    // get a pointer to the back buffer of the primary surface
    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
    if (lpFrontBuffer->lpVtbl->GetAttachedSurface( lpFrontBuffer, &ddscaps, &lpBackBuffer ) != DD_OK )
		return ErrorBox( "Couldn't get second video page" );

	// Create alphanumerics character storage buffer
    ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	if (HardwareCaps.dwCKeyCaps & DDCKEYCAPS_SRCBLT)
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
	else
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    ddsd.dwWidth = 4*16;
    ddsd.dwHeight = 64*16;
    if (lpDD->lpVtbl->CreateSurface( lpDD, &ddsd, &lpAlphaBuffer, NULL ) != DD_OK )
		return ErrorBox( "Couldn't create Alphanumerics storage buffer" );

	// Create two screen buffers in PC mem, representing physics IR screen buffers
    ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    ddsd.dwWidth = 640;
	ddsd.dwHeight = 480;
    if (lpDD->lpVtbl->CreateSurface( lpDD, &ddsd, &lpScreenBuffer1, NULL ) != DD_OK )
		return ErrorBox( "Couldn't create screen buffer #1" );
    if (lpDD->lpVtbl->CreateSurface( lpDD, &ddsd, &lpScreenBuffer2, NULL ) != DD_OK )
		return ErrorBox( "Couldn't create screen buffer #2" );

	// Create the palette
	if (lpDD->lpVtbl->CreatePalette( lpDD, DDPCAPS_8BIT | DDPCAPS_ALLOW256, ColorRAM, &lpGamePalette, NULL ) != DD_OK)
		return ErrorBox( "Couldn't create the palette" );
	{
		LPDIRECT3D2 m_pD3D;
		if (lpDD->lpVtbl->QueryInterface( lpDD, &IID_IDirect3D2, (LPVOID *)&m_pD3D) != S_OK)
			return FALSE;
	}


	// Restore the surfaces
    if( !RestoreAllSurfaces() )
        return ErrorBox( "Problem restoring DirectDraw surfaces" );

	// Hide the cursor during the game
	SetCursor( NULL );

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void StopVideoHardware( void )
{
    // Make the cursor visible once again
    SetCursor(LoadCursor( NULL, IDC_ARROW ));

	if (lpAlphaBuffer)
		lpAlphaBuffer->lpVtbl->Release( lpAlphaBuffer );

	if (lpScreenBuffer1)
		lpScreenBuffer1->lpVtbl->Release( lpScreenBuffer1 );

	if (lpScreenBuffer2)
		lpScreenBuffer2->lpVtbl->Release( lpScreenBuffer2 );

	if (lpFrontBuffer)
		lpFrontBuffer->lpVtbl->Release( lpFrontBuffer );

	if (lpGamePalette)
		lpGamePalette->lpVtbl->Release( lpGamePalette );

	if (lpDD)
		lpDD->lpVtbl->Release( lpDD );
}