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

// Externally visible function
// void DoObject( int ObjAddr );


static WORD tempword, addr;
static int nx, ny, nz;
static short* CoordinateAddress;
static int dx, dy, dz;

// GetPoint
// - fetches an x/y/z coordinate from the coordinate list
#define GetPoint \
	addr = tempword & 0x3FFF;\
	x = CoordinateAddress[addr++];\
	y = CoordinateAddress[addr++];\
	z = CoordinateAddress[addr];

// DoShading
// - Determines palette shading offset (0-7) based on 
//   angle between normal vector nx/ny/nz and light source vector
#define DoShading \
	if (ControlWord & 0x40)\
		color += min( 7, max( 0, (nx*LightX + ny*LightY + nz*LightZ) >> 25 ));

// CheckNormalVector
// - Determines if surface is visible
// - returns FALSE if surface is not visible
#define CheckNormalVector \
	CheckNormalVectorBegin\
	CheckNormalVectorEnd

// CheckNormalVectorDoShading
// - Determines if surface is visible
// - returns FALSE if surface is not visible
// - calculates shading offset if surface is visible
#define CheckNormalVectorDoShading \
	CheckNormalVectorBegin\
	DoShading\
	CheckNormalVectorEnd

#define CheckNormalVectorBegin \
	tempword = *(address++);\
	if (tempword & 0x4000)\
		{ DoCoordinate }\
	else\
	{\
		GetPoint\
		DoRotation\
		nx = x;\
		ny = y;\
		nz = z;\
		tempword = *(address++);\
		GetPoint\
		DoRotation\
		Translate\
		if (nx*x+ny*y+nz*z<=0)\
			return FALSE;\
		if (ControlWord & 0x3000)\
			return TRUE;\
		ProjectCamera

#define CheckNormalVectorEnd \
	}\
	if (ControlWord & 0x3000)\
		return TRUE;

// DoCoordinate
// - fetches an x/y/z value
// - rotates it
// - translates it
// - projects it
#define DoCoordinate \
	tempword = *(address++);\
	GetPoint\
	DoRotation\
	Translate\
	ProjectCamera

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static _inline BOOL RenderDot(WORD* address, int ControlWord)
{
	int color = ControlWord & 0x3F;

	// Check normal vector
	CheckNormalVector

		// Display all dots
		DrawDot(x + xmid, y + ymid, color);
	while (!(tempword & 0x8000))
	{
		DoCoordinate
			DrawDot(x + xmid, y + ymid, color);
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static _inline BOOL RenderVector(WORD* address, int ControlWord)
{
	int color = ControlWord & 0x3F;
	int xfirst, yfirst, xlast, ylast;
	int counter = 0;

	// Check normal vector
	CheckNormalVector

		// Save starting/ending coordinate
		xfirst = xlast = x + xmid;
	yfirst = ylast = y + ymid;

	// Display all vectors
	while (!(tempword & 0x8000))
	{
		DoCoordinate
			DrawVector(xlast, ylast, x + xmid, y + ymid, color);
		xlast = x + xmid;
		ylast = y + ymid;
		counter++;
	}

	// link last vector back to the first vector if necessary
	if (counter > 1)
		DrawVector(xlast, ylast, xfirst, yfirst, color);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static _inline BOOL RenderPolygon(WORD* address, int ControlWord)
{
	POINT points[100];
	int counter = 1;
	int color = ControlWord & 0x3F;

	// Check normal vector
	CheckNormalVectorDoShading

		// Get all polygon points
		points[0].x = x + xmid;
	points[0].y = y + ymid;
	while (!(tempword & 0x8000))
	{
		DoCoordinate
			points[counter].x = x + xmid;
		points[counter++].y = y + ymid;
	}

	// Draw the object
	if (counter > 2)
		DrawPolygon(points, counter, color);
	else if (counter == 2)
		DrawVector(points[0].x, points[0].y, points[1].x, points[1].y, color);
	else
		DrawDot(points[0].x, points[0].y, color);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Function returns address of next surface in object list

static _inline WORD* RenderSurface(WORD* CurrentSurface)
{
	WORD FaceWord = *(CurrentSurface++), ControlWord = *(CurrentSurface++);
	BOOL ret;

	switch (ControlWord & 0x0300)
	{
	case 0x0000:
		ret = RenderPolygon(MathRAMROM + FaceWord, ControlWord);
		break;
	case 0x0100:
		ret = RenderVector(MathRAMROM + FaceWord, ControlWord & 0x3F);
		break;
	case 0x0200:
		ret = RenderDot(MathRAMROM + FaceWord, ControlWord & 0x3F);
		break;
	}

	// keep/remove hidden surface 'groups'/'chunks'
	// 8000 = jump always
	// 9000 = jump if surface is visible
	// A000 = jump if this surface is invisible
	if (ControlWord & 0x8000)
	{
		if (ControlWord & 0x2000 && ret)
			CurrentSurface++;
		else if (ControlWord & 0x1000 && !ret)
			CurrentSurface++;
		else
			CurrentSurface += (short)*CurrentSurface;
	}

	return CurrentSurface;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static _inline void RenderObject(int StartAddress, int ObjectX, int ObjectY, int ObjectZ)
{
	WORD* CurrentSurface;

	// object position represents offset
	dx = ObjectX;
	dy = ObjectY;
	dz = ObjectZ;

	// Get address of coordinate/normal vector information
	CoordinateAddress = (short*)(MathRAMROM + MathRAMROM[StartAddress]);

	// Get address of first surface in object list
	CurrentSurface = MathRAMROM + StartAddress + 1;

	while (*CurrentSurface < 0x8000)
		CurrentSurface = RenderSurface(CurrentSurface);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Function to display the parent object (and all children)
// specified by ObjAddr
void DoObject(int ObjAddr)
{
	static WORD BaseAddress;
	static int ObjectX, ObjectY, ObjectZ;
	WORD Control, word;
	int index;

	for (; ; )
	{
		if (ObjAddr >= 0x8000 || !ObjAddr)
			return;

		// Control word encoding
		// 0x8000 = stop flag
		// 0x4000 = don't load camera matrix
		// 0x1000 = ?
		// 0x0800 = don't load base address or rotation matrix
		// 0x0400 = x/y/z value is relative offset, not absolute position
		Control = MathRAMROM[ObjAddr + 3];

		// Load camera matrix
		if (!(Control & 0x4000))
			LoadCameraMatrix(MathRAMROM[ObjAddr + 4]);

		// Get new base address and rotation matrix if they exist
		if (Control & 0x0800)
			index = 5;
		else
		{
			BaseAddress = MathRAMROM[ObjAddr + 6];
			if (BaseAddress >= 0x8000)
				return;
			LoadRotationMatrix(MathRAMROM[ObjAddr + 5]);
			index = 7;
		}

		// Don't render invalid objects
		if (BaseAddress >= 0x8000)
			return;

		// Determine x/y/z position of object
		x = (int)((short)MathRAMROM[ObjAddr]);
		y = (int)((short)MathRAMROM[ObjAddr + 1]);
		z = (int)((short)MathRAMROM[ObjAddr + 2]);
		if (Control & 0x0400)
		{
			// x/y/z is relative
			RotateCamera
				ObjectX += x;
			ObjectY += y;
			ObjectZ += z;
		}
		else
		{
			// x/y/z is absolute
			x -= CameraX;
			y -= CameraY;
			z -= CameraZ;
			RotateCamera
				ObjectX = x;
			ObjectY = y;
			ObjectZ = z;
		}

		// render this object
		RenderObject(BaseAddress, ObjectX, ObjectY, ObjectZ);

		// parse all child objects
		for (; ; )
		{
			word = MathRAMROM[ObjAddr + (index++)];
			if (word >= 0x8000 || !word)
				return;
			if (word == 0x0002)
			{
				ObjAddr += 8;
				break;
			}

			DoObject(word);
		}
	}
}
