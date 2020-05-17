// Checksum.c
// (c) 1998 John Manfreda

#include "IRobot.h"

BOOL EnsureExecutableIntegrity( void )
{
	int l, m, n;
	DWORD CRC;

	for (m=0; m<(sizeof(GlobalStrings)/sizeofGlobalStrings[0]); m++)
	{
		l = strlen(GlobalStrings[m])
		for (n=
	}

}