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

#define SCALEFACTOR 1

void IRGetClipboard( void )
{
	MessageBeep( 0xFFFFFFFF );

	if (!OpenClipboard( ghwndMain ))
		return;

	EmptyClipboard();

	hdcMetafile = CreateMetaFile( NULL );

	SetMapMode( hdcMetafile, MM_ANISOTROPIC );

	SetWindowExtEx( hdcMetafile, 640+640*SCALEFACTOR, 480+480*SCALEFACTOR, NULL );
	SetWindowOrgEx( hdcMetafile, -640*SCALEFACTOR/2, -480*SCALEFACTOR/2, NULL );

	{
		HPEN hOldPen = SelectObject( hdcMetafile, GetStockObject( NULL_PEN ) );
		HBRUSH hOldBrush = SelectObject( hdcMetafile, GetStockObject( BLACK_BRUSH ) );
		Rectangle( hdcMetafile, -640*SCALEFACTOR/2, -480*SCALEFACTOR/2, 640+640*SCALEFACTOR, 480+480*SCALEFACTOR );
		SelectObject( hdcMetafile, hOldPen );
		SelectObject( hdcMetafile, hOldBrush );
	}

	PerformClipDumpNextFrame = TRUE;
}

void IRCloseClipboard( void )
{
	HMETAFILE hMetafile = NULL;
	LPMETAFILEPICT lpMFP;
	HGLOBAL hGmem;

	hMetafile = CloseMetaFile( hdcMetafile );
	hdcMetafile = NULL;

	hGmem = GlobalAlloc( GHND, (DWORD) sizeof (METAFILEPICT) );
	lpMFP = (LPMETAFILEPICT) GlobalLock( hGmem ) ;
	lpMFP->mm   = MM_ANISOTROPIC;
	lpMFP->xExt = 640*2;
	lpMFP->yExt = 480*2;
	lpMFP->hMF  = hMetafile  ;
	GlobalUnlock( hGmem );
	
	SetClipboardData( CF_METAFILEPICT, hGmem );

	CloseClipboard();

	PerformingClipDump = FALSE;
}