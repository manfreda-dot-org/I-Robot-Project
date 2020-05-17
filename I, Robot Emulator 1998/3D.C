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

// user definitions

// screen resolution
#define xres 640
#define yres 480

// PC screen buffser scale
// = 0 for 256x232 screen
// = 1 for 512x464 screen
// = 2 for 1024x928 screen
#define SCREENSCALE 1

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// typedefs and definitions

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <windows.h>

/*#ifndef POINT
typedef struct tagPOINT {
	long x, y;
} POINT;
#endif

#ifndef BOOL
typedef char BOOL;
#endif

#ifndef WORD
typedef unsigned short WORD;
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
*/
#ifndef max
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b)) 
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// private variables

static int CameraX, CameraY, CameraZ;
static int LightX, LightY, LightZ;
static int CameraMatrix[9];
static int RotationMatrix[9];
static int x, y, z;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Externally needed functions and variables

extern WORD* MathRAMROM;
extern BYTE* ScreenBuffer;
extern BYTE* EndOfScreenBuffer;
extern int ScreenPitch;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// constant definitions - DO NOT MODIFY

#define xmid xres/2
#define ymid yres/2+(((256-232)<<SCREENSCALE)>>2)

#define SCALEFACTORX 8+SCREENSCALE
#define SCALEFACTORY 8+SCREENSCALE

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// MACROS

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define DoRotation \
	{\
		int ox = x, oy = y;\
		x = (x*RotationMatrix[0] + y*RotationMatrix[1] + z*RotationMatrix[2])>>14;\
		y = (ox*RotationMatrix[3] + y*RotationMatrix[4] + z*RotationMatrix[5])>>14;\
		z = (ox*RotationMatrix[6] + oy*RotationMatrix[7] + z*RotationMatrix[8])>>14;\
	}


// Translate
// - translates an x/y/z value
#define Translate \
	x += dx;\
	y += dy;\
	z += dz;

// RotateCamera
// - rotates x/y/z value relative to camera
#define RotateCamera \
	{\
		int ox = x, oy = y;\
		x = (x*CameraMatrix[0] + y*CameraMatrix[1] + z*CameraMatrix[2])>>14;\
		y = (ox*CameraMatrix[3] + y*CameraMatrix[4] + z*CameraMatrix[5])>>14;\
		z = (ox*CameraMatrix[6] + oy*CameraMatrix[7] + z*CameraMatrix[8])>>14;\
	}


// ProjectCamera
// - projects x/y/z value relative to viewport  
#define ProjectCamera \
	if (z)\
	{\
		x = (x << SCALEFACTORX) / z;\
		y = (y << SCALEFACTORY) / z;\
	}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#pragma check_stack(off)

// Function to load the camera matrix
static _inline void LoadCameraMatrix(WORD addr)
{
	short* ptr;

	if (addr == 0x787C)
		addr = 0x15;

	ptr = &MathRAMROM[addr];

	CameraMatrix[0] = (int)(*ptr++);
	CameraMatrix[1] = (int)(*ptr++);
	CameraMatrix[2] = (int)(*ptr++);
	CameraMatrix[3] = (int)(*ptr++);
	CameraMatrix[4] = (int)(*ptr++);
	CameraMatrix[5] = (int)(*ptr++);
	CameraMatrix[6] = (int)(*ptr++);
	CameraMatrix[7] = (int)(*ptr++);
	CameraMatrix[8] = (int)(*ptr);
}

// Function to load the rotation matrix
static _inline void LoadRotationMatrix(WORD addr)
{
	short* ptr;

	if (addr == 0x787C)
		addr = 0x15;

	ptr = &MathRAMROM[addr];

	RotationMatrix[0] = (int)(*ptr++);
	RotationMatrix[1] = (int)(*ptr++);
	RotationMatrix[2] = (int)(*ptr++);
	RotationMatrix[3] = (int)(*ptr++);
	RotationMatrix[4] = (int)(*ptr++);
	RotationMatrix[5] = (int)(*ptr++);
	RotationMatrix[6] = (int)(*ptr++);
	RotationMatrix[7] = (int)(*ptr++);
	RotationMatrix[8] = (int)(*ptr);
}

// Function to retrieve the current camera position from MathRAM
void GetCamera(void)
{
	// Get light source
	LightX = (int)((short)MathRAMROM[0x88 / 2]);
	LightY = (int)((short)MathRAMROM[0x8A / 2]);
	LightZ = (int)((short)MathRAMROM[0x8C / 2]);

	// Get camera position
	CameraX = (int)((short)MathRAMROM[0x12]);
	CameraY = (int)((short)MathRAMROM[0x13]);
	CameraZ = (int)((short)MathRAMROM[0x14]);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "DotVectorPolygon.h"
#include "RenderOB.h"
#include "RenderPF.h"