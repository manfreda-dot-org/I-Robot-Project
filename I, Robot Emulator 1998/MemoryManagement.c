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

static BOOL LoadFile( char * Filename, unsigned int size, BYTE * buffer, int offset, unsigned int correctsize )
{
	HANDLE hFile;
	BY_HANDLE_FILE_INFORMATION fi;
	char string[255];
	BOOL error = FALSE;
	int numread;

	// Open the file
	hFile = CreateFile( Filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if (hFile == INVALID_HANDLE_VALUE)
	{
		wsprintf( string, "Can't find ROM %s", Filename );
		error = TRUE;
		goto done;
	}
	if (GetFileInformationByHandle( hFile, &fi ) == 0)
	{
		wsprintf( string, "Strange problem with file %s", Filename );
		error = TRUE;
		goto done;
	}

	if (fi.nFileSizeHigh!=0 || fi.nFileSizeLow!=correctsize)
	{
		wsprintf( string, "%s is corrupt", Filename );
		error = TRUE;
		goto done;
	}


	if (offset && SetFilePointer( hFile, offset, NULL, FILE_BEGIN )==0xFFFFFFFF)
	{
		wsprintf( string, "Problem scanning %s", Filename );
		error = TRUE;
		goto done;
	}

	if (!ReadFile( hFile, buffer, size, &numread, NULL ))
	{
		wsprintf( string, "Problem reading %s", Filename );
		error = TRUE;
	}


done:
	CloseHandle(hFile);

	if (error)
		return ErrorBox( string );

	return TRUE;
}


static void RearrangeStep( BYTE * temp, BYTE * ptr )
{
	int n;
	static BYTE *ptr1, *ptr2;

	ptr1 = ptr;
	ptr2 = temp;
	CopyMemory( ptr2, ptr1, 0x4000 );
	// low bytes
	ptr1++;
	for (n=0; n<0x2000; n++, ptr1+=2)
		*ptr1 = *(ptr2++);
	// high bytes
	ptr1 = ptr;
	ptr2 = temp+0x2000;
	for (n=0; n<0x2000; n++, ptr1+=2)
		*ptr1 = *(ptr2++);

}

static BOOL RearrangeMathROMs( void )
{
	static BYTE * temp;
	
	temp = (BYTE *) LocalAlloc( LMEM_FIXED, 0x4000 );
	if (!temp)
		return FALSE;

	RearrangeStep( temp, RAM2000t3FFF[3] );
	RearrangeStep( temp, RAM2000t3FFF[5] );
	RearrangeStep( temp, RAM2000t3FFF[7] );

	LocalFree( (HLOCAL) temp );

	return TRUE;
}

static void CreateMathRAMROM( void )
{
	BYTE * src, * dest;
	int n;

	src = RAM2000t3FFF[3];
	dest = (BYTE *) &MathRAMROM[0x2000];

	for (n=0; n<0xc000; n+=2, src+=2)
	{
		*(dest++) = *(src+1);
		*(dest++) = *src;
	}
}

BOOL LoadROMs( void )
{
	int n;

	// Load the program ROM files
	if (!LoadFile( "136029.405", 0x2000, ROM[0]+0x4000, 0, 0x4000 ))
		return FALSE;
	if (!LoadFile( "136029.405", 0x2000, ROM[1]+0x4000, 0x2000, 0x4000 ))
		return FALSE;
	if (!LoadFile( "136029.206", 0x2000, ROM[2]+0x4000, 0, 0x4000 ))
		return FALSE;
	if (!LoadFile( "136029.206", 0x2000, ROM[3]+0x4000, 0x2000, 0x4000 ))
		return FALSE;
	if (!LoadFile( "136029.207", 0x2000, ROM[4]+0x4000, 0, 0x4000 ))
		return FALSE;
	if (!LoadFile( "136029.207", 0x2000, ROM[5]+0x4000, 0x2000, 0x4000 ))
		return FALSE;
	if (!LoadFile( "136029.208", 0x2000, ROM[0]+0x6000, 0, 0x2000 ))
		return FALSE;
	if (!LoadFile( "136029.209", 0x4000, ROM[0]+0x8000, 0, 0x4000 ))
		return FALSE;
	if (!LoadFile( "136029.210", 0x4000, ROM[0]+0xC000, 0, 0x4000 ))
		return FALSE;
	for (n=1; n<6; n++)
		CopyMemory( ROM[n] + 0x6000, ROM[0] + 0x6000, 0xA000 );
	// Do a mathbox self-test hack
/*	{
		BYTE hack[] = { 0x40,0x00,0x00,0x00,0x01,0x21,
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
		CopyMemory( ROM[4]+0x5CAD, hack, 72 );
	}*/


	// Load Mathbox ROM files
	if (!LoadFile( "136029.103", 0x2000, RAM2000t3FFF[3], 0, 0x2000 ))
		return FALSE;
	if (!LoadFile( "136029.104", 0x2000, RAM2000t3FFF[4], 0, 0x2000 ))
		return FALSE;
	if (!LoadFile( "136029.101", 0x2000, RAM2000t3FFF[5], 0, 0x4000 ))
		return FALSE;
	if (!LoadFile( "136029.102", 0x2000, RAM2000t3FFF[6], 0, 0x4000 ))
		return FALSE;
	if (!LoadFile( "136029.101", 0x2000, RAM2000t3FFF[7], 0x2000, 0x4000 ))
		return FALSE;
	if (!LoadFile( "136029.102", 0x2000, RAM2000t3FFF[8], 0x2000, 0x4000 ))
		return FALSE;
	if (!RearrangeMathROMs())
		return ErrorBox( "Not enough memory to construct MathROM" );
	// Copy the MathROMs to MathRAMROM, and rearrange
	CreateMathRAMROM();


	// Load video ROM files
	if (!LoadFile( "136029.124", 0x800, AlphaROM, 0, 0x800 ))
		return FALSE;
	if (!LoadFile( "136029.125", 0x20, ColorROM, 0, 0x20 ))
		return FALSE;
	// Expand ColorROM into RRGGBBII format
	{
		BYTE b[8];
		b[0] = ColorROM[4];
		b[1] = ColorROM[5];
		b[2] = ColorROM[6];
		b[3] = ColorROM[7];
		b[4] = ColorROM[20];
		b[5] = ColorROM[21];
		b[6] = ColorROM[22];
		b[7] = ColorROM[23];
		for (n=0; n<8; n++)
		{
			BYTE i = (((b[n] & 0x03) << 1) + 1 + 1) * 8;
			ColorROM[n<<2] = ((b[n] >> 6) & 3) * i;
			ColorROM[(n<<2)+1] = ((b[n] >> 4) & 3) * i;
			ColorROM[(n<<2)+2] = ((b[n] >> 2) & 3) * i;
			ColorROM[(n<<2)+3] = 0;
		}
	}
	// Initialize Alphanumerics palette
	SetAlphaPalette();

	return TRUE;
}

BOOL AllocateMemory( void )
{
	int n;

	// Allocate working RAM from $0000-$07FF
	RAM0000t07FF = (BYTE *) LocalAlloc( LPTR, 0x800 );

	// Allocate 3 pages of working RAM from $07FF-$0FFF
	RAM0800t0FFF[0] = (BYTE *) LocalAlloc( LPTR, 3 * 0x800 );
	RAM0800t0FFF[1] = RAM0800t0FFF[0] + 0x800;
	RAM0800t0FFF[2] = RAM0800t0FFF[1] + 0x800;

	// Allocate storage EERAM from $1200-$12FF
	EERAM1200t12FF = (BYTE *) LocalAlloc( LPTR, 0x100 );

	// Allocate alphanumerics RAM from $1C00t1FFF
	RAM1C00t1FFF = (BYTE *) LocalAlloc( LPTR, 0x400 );

	// Allocate math RAM/ROM space for 3d object rasterization
	// 8 pages of 0x2000 length are necessary
	MathRAMROM = (WORD *) LocalAlloc( LPTR, 8 * 0x2000 );

	// 9 pages of RAM/ROM from $2000-$3FFF
	// Only 8 pages need to be allocated, since first page is shared
	// with MathRAMROM
	RAM2000t3FFF[0] = (BYTE *) MathRAMROM;
	RAM2000t3FFF[1] = (BYTE *) LocalAlloc( LPTR, 8 * 0x2000 );
	for (n=2; n<=8; n++)
		RAM2000t3FFF[n] = RAM2000t3FFF[n-1] + 0x2000;

	// Allocate 6 64K address spaces for program CODE
	ROM[0] = (BYTE *) LocalAlloc( LPTR, 6 * 0x10000 );
	for (n=1; n<6; n++)
		ROM[n] = ROM[n-1] + 0x10000;

	// Allocate 32 bytes for color ROM
	ColorROM = (BYTE *) LocalAlloc( LPTR, 0x20 );

	// Allocate 2048 bytes for Alphanumerics
	AlphaROM = (BYTE *) LocalAlloc( LPTR, 0x800 );

	if (!RAM0000t07FF ||
		!RAM0800t0FFF[0] ||
		!EERAM1200t12FF ||
		!RAM1C00t1FFF ||
		!MathRAMROM || 
		!RAM2000t3FFF[1] ||
		!ROM[0] ||
		!ColorROM ||
		!AlphaROM)
		return ErrorBox( "Problem allocating resources for emulator" );

	return TRUE;
}


void FreeMemory( void )
{
	if (RAM0000t07FF)
		LocalFree( (HLOCAL) RAM0000t07FF );
	if (RAM0800t0FFF[0])
		LocalFree( (HLOCAL) RAM0800t0FFF[0] );
	if (EERAM1200t12FF)
		LocalFree( (HLOCAL) EERAM1200t12FF );
	if (RAM1C00t1FFF)
		LocalFree( (HLOCAL) RAM1C00t1FFF );
	if (MathRAMROM)
		LocalFree( (HLOCAL) MathRAMROM );
	if (RAM2000t3FFF[1])
		LocalFree( (HLOCAL) RAM2000t3FFF[1] );
	if (ROM[0])
		LocalFree( (HLOCAL) ROM[0] );
	if (ColorROM)
		LocalFree( (HLOCAL) ColorROM );
	if (AlphaROM)
		LocalFree( (HLOCAL) AlphaROM );
}


void LoadEEPROM( void )
{
	DWORD size = 0x100;
	DWORD type;
	HKEY key;

	// attempt to locate the key
	if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, RegistrySubkey, 0, KEY_ALL_ACCESS, &key )
		!= ERROR_SUCCESS )
		return;

	// load EEPROM
	if (RegQueryValueEx( key, "", NULL, &type, EERAM1200t12FF, &size ) != ERROR_SUCCESS
		|| type != REG_BINARY
		|| size != 0x100 )
		// data is invalid, delete the bad key
		RegDeleteKey( key, "" );

	RegCloseKey( key );
}

void SaveEEPROM( void )
{
	HKEY key;
	DWORD temp;

	// open/create the key
	if (RegCreateKeyEx( HKEY_LOCAL_MACHINE, RegistrySubkey, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &temp )
		!= ERROR_SUCCESS )
		return;

	// save EEPROM
	RegSetValueEx( key, "", (DWORD) NULL, REG_BINARY, EERAM1200t12FF, 0x100 );

	RegCloseKey( key );
}