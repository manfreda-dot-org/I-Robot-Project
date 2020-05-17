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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void SetAlphaPalette( void )
{
	static BYTE CurrentPalette = 0xFF;
	int n = REG1180 & ALPHAMAP1;
	BYTE * base;

	if (n != CurrentPalette)
	{
		CurrentPalette = n;

		// Update the palette
		base = (n) ? &ColorROM[16] : ColorROM;
		for (n=65; n<69; n++, base++)
		{
			ColorRAM[n].peRed = *(base++);
			ColorRAM[n].peGreen = *(base++);
			ColorRAM[n].peBlue = *(base++);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static int ConvertCharacter( int character )
{
	if (character>='A' && character<='Z')
		return character - 'A' + 1;
	if (character>='0' && character<='9')
		return character;
	switch(character)
	{
	case ' ':
		return 0;
	case '-':
		return 27;
	case '{':
		return 28;
	case '}':
		return 29;
	case '.':
		return 30;
	case ':':
		return 31;
	case 'c':
		return '9'+1;
	case 'w':
		return 32;
	case 'x':
		return 33;
	case 'y':
		return 34;
	case 'z':
		return 35;
	}

	return character;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void DisplayChar( int character, int x, int y, int color )
{
	RECT rc;

	rc.top = (character & 0x3F) << 4;
	rc.left = ((character >> 6) + (color & 3)) << 4;
	rc.right = rc.left + 16;
	rc.bottom = rc.top + 16;

	if (lpBackBuffer->lpVtbl->BltFast( lpBackBuffer, x, y, lpAlphaBuffer, &rc, DDBLTFAST_WAIT | DDBLTFAST_NOCOLORKEY ) == DDERR_SURFACELOST)
		RestoreAllSurfaces();
}

int DisplayString( char * string, int x, int y, int color )
{
	int thischar = *(string++);

	if (lpBackBuffer && thischar)
		do
		{
			DisplayChar( ConvertCharacter( thischar ), x, y, color );
			x += 16;
			thischar = *(string++);
		} while (thischar);

	return x;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static _inline void RenderAlphanumericAtAddress( WORD address, BYTE data )
{
	RECT rc;

	rc.top = (data & 0x3F) << 4;
	rc.left = (data >> 6) << 4;
	rc.right = rc.left + 16;
	rc.bottom = rc.top + 16;

	if (lpBackBuffer->lpVtbl->BltFast( lpBackBuffer, ((address & 0x1F) << 4) + 64, ((address & 0x3E0) >> 1) + 8, lpAlphaBuffer, &rc, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY ) == DDERR_SURFACELOST)
		RestoreAllSurfaces();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static void RenderAllAlphanumerics()
{
	BYTE * AlphaRAM = RAM1C00t1FFF;
	WORD n;

	for (n=0; n<0x3A0; n++, AlphaRAM++)
		if (*AlphaRAM & 63)
			RenderAlphanumericAtAddress( n, *AlphaRAM );
}

static _inline void DisplayWelcomeMessage( void )
{
	static DWORD LastTime;
	static BOOL flag = FALSE;
	static int state = 3;
	static int FlashyColor = 0;
	DWORD Delta;

	if (!state)
		return;

	// rotate color
	FlashyColor = (FlashyColor + 1) & 3;

	// get initial baseline for timing
	if (!flag)
	{
		LastTime = timeGetTime();
		flag = TRUE;
	}

	Delta = timeGetTime() - LastTime;

	if (Delta < 2000)
		switch(state)
		{
		case 3: DisplayString( GlobalStrings[STR_WELCOME1], 0, 0, FlashyColor ); break;
		case 2: DisplayString( GlobalStrings[STR_WELCOME2], 0, 0, FlashyColor ); break;
		case 1: DisplayString( GlobalStrings[STR_WELCOME3], 0, 0, FlashyColor ); break;
		}
	else
	{
		DisplayString( "                                        ", 0, 0, 0 );
		if (Delta > 2100)
		{
			state--;
			LastTime = timeGetTime();
		}
	}
}
