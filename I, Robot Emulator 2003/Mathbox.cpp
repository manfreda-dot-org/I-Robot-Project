// Copyright 2003 by John Manfreda. All Rights Reserved.
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

#include <vcl.h>
#include "Mathbox.h"
#include "Emulator.h"
#include "ROM.h"

//---------------------------------------------------------------------------

#pragma check_stack(off)

#define COMMAND_START_PLAYFIELD       0x8400
#define COMMAND_UNKNOWN_MATH_FUNCTION 0x8600
#define COMMAND_ROLL                  0x8800
#define COMMAND_YAW                   0x9000
#define COMMAND_PITCH                 0xA000
#define COMMAND_CAMERA                0xC000

static unsigned int Bank;
static BYTE ComRAM[2][0x2000]; // 2 banks
static BYTE CPUROM[6][0x2000]; // 6 obj ROM banks, accessible by CPU

// mathbox memory
// 0000-0FFF    shared mathobox / CPU ram
// 1000-1FFF    mathbox scratch RAM
// 2000-7FFF    3D object ROMs
static WORD Memory[8*0x1000];

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static BYTE MathboxRead(WORD address)
{
        // direct RAM access
	// flip x86 word values, so 6809 can read them!
        return ((BYTE*)Memory)[(address&0x1FFF)^1];
}

static DWORD MathboxWrite(BYTE data, WORD address)
{
        // this is either a write to mathbox RAM
        // or a write to mathbox ROM!
        if (!Bank)
                ((BYTE*)Memory)[(address&0x1FFF)^1] = data; // flip word values, so x86 processor can read them
        else if (Bank >= 3)
                Terminate("Undefined mathbox write: " + IntToHex((int) Bank,2) + IntToHex(address,4) + "=" + IntToHex((int) data, 2));
        else
                Terminate("Mathbox I/O internal consistency failure");

	return (DWORD) Hardware.Program.ROM;
}

//---------------------------------------------------------------------------

__fastcall TMathbox::TMathbox()
{
        // load actual program ROMs
        TROM _101, _102, _103, _104;
        if (!_101.Load("136029.101", 0x4000, 0x150247)) return;
        if (!_102.Load("136029.102", 0x4000, 0xF557F)) return;
        if (!_103.Load("136029.103", 0x2000, 0x6A797)) return;
        if (!_104.Load("136029.104", 0x2000, 0x43382)) return;

        // rearrange ROMs into proper banks
        // arrangment is high endian for CPU
        BYTE * dst = CPUROM[0];
        for (int s=0; s<0x2000; s++)
        {
                *dst++ = _104.Data[s];
                *dst++ = _103.Data[s];
        }
        for (int s=0; s<0x4000; s++)
        {
                *dst++ = _102.Data[s];
                *dst++ = _101.Data[s];
        }

        // create low endian object banks for our use
        BYTE * src = CPUROM[0];
        dst = (BYTE *) &Memory[0x2000];
        for (int s=0; s<0x6000; s++)
        {
                dst[1] = *src++;
                dst[0] = *src++;
                dst += 2;
        }
}

//---------------------------------------------------------------------------

void __fastcall TMathbox::InitIO(void)
{
        // setup 6809 page vector pointers

	// $20xx-$3Fxx banked math RAM
	for (int n=0x20; n<=0x3F; n++)
	{
		M6809_PageReadFunction[n] = (DWORD) MathboxRead;
		M6809_PageWriteFunction[n] = (DWORD) MathboxWrite;
	}

        // setup bank switched areas
        Bank = -1;
        BankSwitch();
}


void __fastcall TMathbox::BankSwitch(void)
{
	unsigned int n, bank;
	bool isRAM, isMRAM = false;
	BYTE * ptr;

	if (Hardware.Registers._1180 & OUT04)
	{
		isRAM = true;
		if (Hardware.Registers._1180 & OUT03)
                        // Mathbox RAM
			isMRAM = true, bank = 0;
		else
                        // C1 / C2 RAM
			bank = (Hardware.Registers._1140 & EXTCOMSWAP) ? 1 : 2;

	}
	else
	{
		isRAM = false;
		bank = ((Hardware.Registers._1180 & (OUT03 | MPAGE2 | MPAGE1)) >> 1) + 1;
		if (bank > 8)
		{
                        Terminate("Illegal mathbox bank selection: 1180=" + IntToHex((int) Hardware.Registers._1180, 2));
			return;
		}
	}

	if (bank != Bank)
	{
		Bank = bank;
                switch (bank)
                {
                case 1: ptr = ComRAM[0]; break;
                case 2: ptr = ComRAM[1]; break;
                case 3: ptr = CPUROM[0]; break;
                case 4: ptr = CPUROM[1]; break;
                case 5: ptr = CPUROM[2]; break;
                case 6: ptr = CPUROM[3]; break;
                case 7: ptr = CPUROM[4]; break;
                case 8: ptr = CPUROM[5]; break;
                }
		if(isRAM)
		{
			if (isMRAM)
				for (n=0x20; n<=0x3F; n++)
					M6809_PageReadPointer[n] =
					M6809_PageWritePointer[n] = 0;
			else
				for (n=0x20; n<=0x3F; n++, ptr+=0x100)
					M6809_PageReadPointer[n] =
					M6809_PageWritePointer[n] = (DWORD) ptr;
		}
		else for (n=0x20; n<=0x3F; n++, ptr+=0x100)
		{
			M6809_PageReadPointer[n] = (DWORD) ptr;
			M6809_PageWritePointer[n] = 0;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Pitch function  (X)
// Multiplies source matrix by
//	[ 1   0   0  ]
//	[ 0  cos sin ]
//	[ 0 -sin cos ]
static void __inline Pitch(__int16 * matrix, __int16 SIN, __int16 COS )
{
	__int16 B = matrix[1];
	__int16 C = matrix[2];
	__int16 E = matrix[4];
	__int16 F = matrix[5];
	__int16 H = matrix[7];
	__int16 I = matrix[8];

	matrix[1] = (__int16)((B * COS - C * SIN) >> 14);
	matrix[2] = (__int16)((C * COS + B * SIN) >> 14);
	matrix[4] = (__int16)((E * COS - F * SIN) >> 14);
	matrix[5] = (__int16)((F * COS + E * SIN) >> 14);
	matrix[7] = (__int16)((H * COS - I * SIN) >> 14);
	matrix[8] = (__int16)((I * COS + H * SIN) >> 14);
}

// Yaw function  (Y)
// Multiplies source matrix by
//	[ cos 0 -sin ]
//	[  0  1   0  ]
//	[ sin 0  cos ]
static void __inline Yaw(__int16 * matrix, __int16 SIN, __int16 COS )
{
	__int16 A = matrix[0];
	__int16 C = matrix[2];
	__int16 D = matrix[3];
	__int16 F = matrix[5];
	__int16 G = matrix[6];
	__int16 I = matrix[8];

	matrix[0] = (__int16)((A * COS + C * SIN) >> 14);
	matrix[2] = (__int16)((C * COS - A * SIN) >> 14);
	matrix[3] = (__int16)((D * COS + F * SIN) >> 14);
	matrix[5] = (__int16)((F * COS - D * SIN) >> 14);
	matrix[6] = (__int16)((G * COS + I * SIN) >> 14);
	matrix[8] = (__int16)((I * COS - G * SIN) >> 14);
}

// Roll function  (Z)
// Multiplies source matrix by
//	[  cos sin 0 ]
//	[ -sin cos 0 ]
//	[   0   0  1 ]
static void __inline Roll(__int16 * matrix, __int16 SIN, __int16 COS )
{
	__int16 A = matrix[0];
	__int16 B = matrix[1];
	__int16 D = matrix[3];
	__int16 E = matrix[4];
	__int16 G = matrix[6];
	__int16 H = matrix[7];

	matrix[0] = (__int16 )((A * COS - B * SIN) >> 14);
	matrix[1] = (__int16 )((B * COS + A * SIN) >> 14);
	matrix[3] = (__int16 )((D * COS - E * SIN) >> 14);
	matrix[4] = (__int16 )((E * COS + D * SIN) >> 14);
	matrix[6] = (__int16 )((G * COS - H * SIN) >> 14);
	matrix[7] = (__int16 )((H * COS + G * SIN) >> 14);
}


// Camera function
// performs A x B = C
// result is C transpose
static void __inline MatrixMultiply(__int16 * matrixA, __int16 * matrixB, __int16 * matrixC )
{
	matrixC[0] = (__int16)((matrixA[0]*matrixB[0] + matrixA[1]*matrixB[1] + matrixA[2]*matrixB[2]) >> 14);
	matrixC[1] = (__int16)((matrixA[0]*matrixB[3] + matrixA[1]*matrixB[4] + matrixA[2]*matrixB[5]) >> 14);
	matrixC[2] = (__int16)((matrixA[0]*matrixB[6] + matrixA[1]*matrixB[7] + matrixA[2]*matrixB[8]) >> 14);
	matrixC[3] = (__int16)((matrixA[3]*matrixB[0] + matrixA[4]*matrixB[1] + matrixA[5]*matrixB[2]) >> 14);
	matrixC[4] = (__int16)((matrixA[3]*matrixB[3] + matrixA[4]*matrixB[4] + matrixA[5]*matrixB[5]) >> 14);
	matrixC[5] = (__int16)((matrixA[3]*matrixB[6] + matrixA[4]*matrixB[7] + matrixA[5]*matrixB[8]) >> 14);
	matrixC[6] = (__int16)((matrixA[6]*matrixB[0] + matrixA[7]*matrixB[1] + matrixA[8]*matrixB[2]) >> 14);
	matrixC[7] = (__int16)((matrixA[6]*matrixB[3] + matrixA[7]*matrixB[4] + matrixA[8]*matrixB[5]) >> 14);
	matrixC[8] = (__int16)((matrixA[6]*matrixB[6] + matrixA[7]*matrixB[7] + matrixA[8]*matrixB[8]) >> 14);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static struct {
        VECTOR Position;
        MATRIX Rotation;
} View, World;
static VECTOR Light;

void __fastcall TMathbox::LoadLightVector(void)
{
        Light.x = (__int16) -Memory[0x44];
        Light.y = (__int16) -Memory[0x45];
        Light.z = (__int16) -Memory[0x46];
}

void __fastcall TMathbox::LoadViewPosition(void)
{
        View.Position.x = (__int16) Memory[0x12];
        View.Position.y = (__int16) Memory[0x13];
        View.Position.z = (__int16) Memory[0x14];
}

void __fastcall TMathbox::LoadViewMatrix(int address)
{
	if (address == 0x787C)
		address = 0x15;

	__int16 * ptr = (__int16 *) &Memory[address];

        MATRIX rotation(
                ptr[0] / 16384.0, -ptr[3] / 16384.0, ptr[6] / 16384.0, 0,
                ptr[1] / 16384.0, -ptr[4] / 16384.0, ptr[7] / 16384.0, 0,
                ptr[2] / 16384.0, -ptr[5] / 16384.0, ptr[8] / 16384.0, 0,
                0,                0,                 0,                1);
        View.Rotation = rotation;
}

void __fastcall TMathbox::LoadRotationMatrix(int address)
{
	if (address == 0x787C)
		address = 0x15;

	__int16 * ptr = (__int16 *) &Memory[address];

        MATRIX rotation(
                ptr[0] / 16384.0, -ptr[3] / 16384.0, ptr[6] / 16384.0, 0,
                ptr[1] / 16384.0, -ptr[4] / 16384.0, ptr[7] / 16384.0, 0,
                ptr[2] / 16384.0, -ptr[5] / 16384.0, ptr[8] / 16384.0, 0,
                0,                0,                 0,                1);

        World.Rotation = rotation;
}

void __fastcall TMathbox::SetColor(int index)
{
        index &= 0x3F;
        TRasterizer::SetColor(
                Hardware.ColorRAM.Palette[index].r / 255.0,
                Hardware.ColorRAM.Palette[index].g / 255.0,
                Hardware.ColorRAM.Palette[index].b / 255.0);
}

void __fastcall TMathbox::SetColor(int index, float shade)
{
        index += shade;
        index &= 0x3F;
        if (shade < 7)
        {
                shade -= (int) shade; // fractional portion
                TRasterizer::SetColor(
                        (Hardware.ColorRAM.Palette[index].r + (Hardware.ColorRAM.Palette[index+1].r - Hardware.ColorRAM.Palette[index].r) * shade) / 255.0,
                        (Hardware.ColorRAM.Palette[index].g + (Hardware.ColorRAM.Palette[index+1].g - Hardware.ColorRAM.Palette[index].g) * shade) / 255.0,
                        (Hardware.ColorRAM.Palette[index].b + (Hardware.ColorRAM.Palette[index+1].b - Hardware.ColorRAM.Palette[index].b) * shade) / 255.0);
        }
        else
                TRasterizer::SetColor(
                        Hardware.ColorRAM.Palette[index].r / 255.0f,
                        Hardware.ColorRAM.Palette[index].g / 255.0f,
                        Hardware.ColorRAM.Palette[index].b / 255.0f);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static struct {
        __int16 *VertexSource;
} Object;

void __fastcall TMathbox::RasterizeObject(WORD address)
{
        StartRender();
        LoadLightVector();
        LoadViewPosition();
        ParseObjectList(address);
        EndRender();
}

void __fastcall TMathbox::ParseObjectList(WORD address)
{
	static WORD SurfaceList; // must be static
	int index;

	for( ; ; )
	{
                // check for end of list
		if (address >= 0x8000 || !address)
			return;

		// Control word encoding
		// 0x8000 = stop flag
		// 0x4000 = don't load camera matrix
		// 0x1000 = ?
		// 0x0800 = don't load base address or rotation matrix
		// 0x0400 = x/y/z value is relative offset, not absolute position
		WORD control = Memory[address + 3];

		// load camera matrix
		if (!(control & 0x4000))
			LoadViewMatrix(Memory[address + 4]);

		// Get new base address and rotation matrix if they exist
		if (control & 0x0800)
			index = 5;
		else
		{
			SurfaceList = Memory[address + 6];
			if (SurfaceList >= 0x8000)
				return;
			LoadRotationMatrix(Memory[address + 5]);
			index = 7;
		}

		// Don't render invalid objects
		if (SurfaceList >= 0x8000)
			return;

		// Determine position of object
                VECTOR pt(
                        (__int16) Memory[address],
                        (__int16) Memory[address + 1],
                        (__int16) Memory[address + 2]);
		if (control & 0x0400)
                {
                        // relative position
                        World.Position += pt * View.Rotation;
                }
		else
                {
                        // absolute position
                        pt -= View.Position;
                        World.Position = pt * View.Rotation;
                }
                SetWorldMatrix(World.Position, World.Rotation);

		// parese the surfaces in this object
                ParseSurfaceList(SurfaceList);

		// parse all child objects
		for (;;)
		{
			WORD child = Memory[address + (index++)];
			if (child >= 0x8000 || !child)
				return;
			if (child == 0x0002)
			{
				address += 8;
				break;
			}

			ParseObjectList(child);
		}
	}
}

void __fastcall TMathbox::ParseSurfaceList(WORD address)
{
        Object.VertexSource = (__int16 *) &Memory[Memory[address++]];

        for (;;)
        {
                // get face pointer
        	WORD pface = Memory[address++];

                // exit when end of surface list is encountered
                if (pface >= 0x8000)
                        return;

                // get control flags
                WORD flags = Memory[address++];

                // fill vertex buffer
                bool visible = RenderFace(pface, flags);

        	// keep/remove hidden surface 'groups'/'chunks'
        	// 8000 = jump always
        	// 9000 = jump if surface is visible
        	// A000 = jump if this surface is invisible
        	if (flags & 0x8000)
        	{
        		if ((flags & 0x2000) && visible)
	        		address++;
        		else if ((flags & 0x1000) && !visible)
	        		address++;
		        else
			        address += (__int16) Memory[address];
        	}
        }
}

bool __fastcall TMathbox::RenderFace(WORD address, int flags)
{
        float shade = 0;
	if (!(Memory[address] & 0x4000))
	{
        	int index;

                index = Memory[address] & 0x3FFF;
                VECTOR normal(
                        Object.VertexSource[index+0],
                        Object.VertexSource[index+1],
                        Object.VertexSource[index+2]);
                normal = normal * World.Rotation;

                index = Memory[address+1] & 0x3FFF;
                VECTOR pt(
                        Object.VertexSource[index+0],
                        Object.VertexSource[index+1],
                        Object.VertexSource[index+2]);
                pt = pt * World.Rotation + World.Position;

                // check if surface is visible
                if (normal * pt <= 0)
                        return false;

                // if shading enabled
                if (flags & 0x0040)
                {
                        shade = (normal * -Light) / (1 << 25);
                        if (shade < 0)
                                shade = 0;
                        else if (shade > 7)
                                shade = 7;
                }
	}
	if (flags & 0x3000)
		return true; // don't render

        // prepare color
        SetColor(flags, shade);

        // prepare the vertex buffer
        PrepareVertexBuffer(address);

        // render surface using appropriate method
        switch(flags & 0x0300)
        {
        case 0x0000: Polygon(); break;
        case 0x0100: Vector(); break;
        case 0x0200: Dot(); break;
        }

        return true;
}

void __fastcall TMathbox::PrepareVertexBuffer(WORD address)
{
        VECTOR * ptr = LockVertexBuffer();

        WORD * pface = &Memory[address+1];
        WORD flags;

	// add points to buffer
        int length = 0;
	do
	{
                flags = *pface++;
        	int index = flags & 0x3FFF;
                ptr->x = Object.VertexSource[index+0];
        	ptr->y = Object.VertexSource[index+1];
        	ptr->z = Object.VertexSource[index+2];
                ptr++;
                length++;
	} while (!(flags & 0x8000));

        UnlockVertexBuffer(length);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define TILE_SIZE_X     128
#define TILE_SIZE_Y     256
#define TILE_SIZE_Z     128

static struct {
        void __fastcall (*RenderFunction)(void);
        MATRIX Rotation;
        WORD ObjectList;
        struct {
                int Count;
                int Objects;
                WORD *Prev;
                WORD *This;
                WORD *Next;
        } Row;
} Playfield;

void __fastcall TMathbox::RasterizePlayfield(void)
{
        StartRender();

        LoadLightVector();
        LoadViewPosition();
	LoadViewMatrix(0x15);
        Playfield.Rotation = View.Rotation;
        SetWorldMatrix(Playfield.Rotation);

	// determine rendering method (dot/vector/polygon)
        switch (Memory[0x72])
        {
        case 0x0000: Playfield.RenderFunction = Polygon; break;
        case 0x0100: Playfield.RenderFunction = Vector; break;
        default:     Playfield.RenderFunction = Dot; break;
        }

	// get address of playfield object buffer
	Playfield.ObjectList = Memory[0x6B];

        __int16 z_max    = Memory[0x6C];
        __int16 z_min    = Memory[0x6D];
        __int16 x_       = Memory[0x74];
        __int16 z_frac   = Memory[0x75];
        __int16 x_offset = Memory[0x76];
        __int16 z_offset = Memory[0x77];

        // locate left front tile corner (x1,y1,z1)
        //         +-------+
        //        /       /|
        //       /       / |           x2 = x1 + TILE_SIZE_X
        // y1-- +-------+  |           y2 = y1 + TILE_SIZE_Y
        //      |       |  + --z2      z2 = z1 + TILE_SIZE_Z
        //      |       | /
        //      |       |/
        // y2-- +-------+ --z1
        //      |       |
        //      x1      x2
        VECTOR corner(
                TILE_SIZE_X - x_offset * 128 - x_,
                -View.Position.y,
                z_max * 128 - z_frac );

	// render each row in the playfield
	int row = z_offset / 16 + z_max; // first absolute row to be rendered
        Playfield.Row.Count = z_max - z_min + 1; // number of rows to display
	while (Playfield.Row.Count--)
	{
		DrawPlayfieldRow(row-- & 31, corner);
                corner.z -= TILE_SIZE_Z; // move to next row along the Z axis
        }

        EndRender();
}


void __fastcall TMathbox::DrawPlayfieldRow(int row, VECTOR corner)
{
	// Get address of the three rows that determine how tiles
	// in the current row are drawn
        int base = 0xE00; // Memory[0x70]
	Playfield.Row.Prev = &Memory[base + 16 * ((row - 1) & 31)];
        Playfield.Row.This = &Memory[base + 16 * row];
	Playfield.Row.Next = &Memory[base + 16 * ((row + 1) & 31)];

        // there are 15 tiles per row
        // some are to the left of the camera, some to the right
        // we render from outside -> in, stopping once camera is reached

        int TilesLeft = 15;

        // reset row object counter
        Playfield.Row.Objects = 0;

        // left side (not including center)
	if (corner.x < -TILE_SIZE_X)
	{
		int n = 1;
	 	while (corner.x < -TILE_SIZE_X && TilesLeft)
		{
			DrawPlayfieldTile(n++, corner);
                        corner.x += TILE_SIZE_X;
                        TilesLeft--;
		}
	}

        // right side (includes center)
	if (TilesLeft)
	{
                corner.x += (TilesLeft - 1) * TILE_SIZE_X;

		int n = 15;
		while (TilesLeft--)
		{
			DrawPlayfieldTile(n--, corner);
                        corner.x -= TILE_SIZE_X;
		}
	}

        // any object lists to deal with?
        if (Playfield.Row.Objects)
        {
                // render the objects above this row
                do
                {
                	int count = Memory[Playfield.ObjectList++] + 1;
       		        while (count--)
                		ParseObjectList(Memory[Playfield.ObjectList++]);
                } while (--Playfield.Row.Objects);

                // reset the world matrix to what it was before
                SetWorldMatrix(Playfield.Rotation);
        }
}

typedef struct { float a, b, c, d; } TILE_HEIGHT;
static TILE_HEIGHT __fastcall GetTileHeight(WORD a, WORD b, WORD c, WORD d)
{
        #define TileHeight(tile) (((__int8)((tile) >> 8)) << 2)
        TILE_HEIGHT height;
        if ((a & 0xFF00) == 0x8000)
        {
                // no tile
                height.a =
                height.b =
                height.c =
                height.d = -View.Position.y + TILE_SIZE_Y;
        }
	else if (a & 0x0040)
        {
                // surface is flat
                height.a =
                height.b =
                height.c =
                height.d = -View.Position.y + TileHeight(a);
        }
        else
        {
                // surface is sloped
                height.a = -View.Position.y + TileHeight(a);
                height.b = -View.Position.y + TileHeight(b);
                height.c = -View.Position.y + TileHeight(c);
                height.d = -View.Position.y + TileHeight(d);
        }
        return height;
}


void __fastcall TMathbox::DrawPlayfieldTile(int index, VECTOR corner)
{
        // local tile map (we are rendering tile A)
        //  TileF   TileD   TileC
        //  TileE   TileA   TileB
        //          TileG   TileH

        // get this tile
        WORD TileA = Playfield.Row.This[(index+0) & 15];

        // check if object must be rendered with this tile
        if (TileA & 0x0080)
                Playfield.Row.Objects++;

	// check if tile is empty
	if ((TileA & 0xFF00) == 0x8000)
                return; // nothing to draw

        // get base tile color
	int color = TileA & 0x003F;

        // locate tile corners
        //       d +-------+ c
        //        /       /|
        //       /       / |          x2 = x1 + TILE_SIZE_X
        // y1-- +-------+  |          y2 = y1 + TILE_SIZE_Y
        //      |a     b| g+ --z2     z2 = z1 + TILE_SIZE_Z
        //      |       | /
        //      |e     f|/
        // y2-- +-------+ --z1
        //      |       |
        //      x1      x2
        float x1 = corner.x;
        float x2 = x1 + TILE_SIZE_X;
        float z1 = corner.z;
        float z2 = corner.z + TILE_SIZE_Z;

	// determine tile height offset
        WORD TileB = Playfield.Row.This[(index+1) & 15];
        WORD TileC = Playfield.Row.Next[(index+1) & 15];
        WORD TileD = Playfield.Row.Next[(index+0) & 15];
        TILE_HEIGHT height = GetTileHeight(TileA, TileB, TileC, TileD);

        // draw left or right side of cube
        if (x1 > 0)
        {
                // left side of the cube
                TILE_HEIGHT side;

                if (index == 1)
                        // leftmost tile
                        side.b = side.c = -View.Position.y + TILE_SIZE_Y;
                else
                {
                        WORD TileE = Playfield.Row.This[(index-1) & 15];
                        WORD TileF = Playfield.Row.Next[(index-1) & 15];
                        side = GetTileHeight(TileE, TileA, TileD, TileF);
                }

               	// draw if tile to the left is lower than current tile
                if (height.a < side.b || height.d < side.c)
                {
                        SetColor(color - 1);
                        VECTOR * ptr = LockVertexBuffer();
                        ptr[0] = VECTOR(x1, height.a, z1);
                        ptr[1] = VECTOR(x1, height.d, z2);
                        ptr[2] = VECTOR(x1, side.c,   z2);
                        ptr[3] = VECTOR(x1, side.b,   z1);
                        UnlockVertexBuffer(4);
                        Playfield.RenderFunction();
                }
        }
        else if (x2 < 0)
        {
                // right side of the cube
                TILE_HEIGHT side;

                if (index == 15)
                        // rightmost tile
                        side.a = side.d = -View.Position.y + TILE_SIZE_Y;
                else
                        side = GetTileHeight(TileB, TileB, TileC, TileC);

               	// draw if tile to the right is lower than current tile
                if (height.b < side.a || height.c < side.d)
                {
                        SetColor(color - 1);
                        VECTOR * ptr = LockVertexBuffer();
                        ptr[0] = VECTOR(x2, height.b, z1);
                        ptr[1] = VECTOR(x2, side.a,   z1);
                        ptr[2] = VECTOR(x2, side.d,   z2);
                        ptr[3] = VECTOR(x2, height.c, z2);
                        UnlockVertexBuffer(4);
                        Playfield.RenderFunction();
                }
        }

        // if tile is flat
        if (TileA & 0x0040)
        {
               	// Draw the front of the cube if:
                //   - This is the last row to render
               	// OR
                //   - Tile in front is lower than current tile (or empty)
                TILE_HEIGHT side;
                if (!Playfield.Row.Count)
                        side.c = side.d = -View.Position.y + TILE_SIZE_Y;
                else
                {
                        WORD TileG = Playfield.Row.Prev[(index+0) & 15];
                        WORD TileH = Playfield.Row.Prev[(index+1) & 15];
                        side = GetTileHeight(TileG, TileH, TileB, TileA);
                }
                if (height.a < side.d || height.b < side.c)
                {
                        SetColor(color - 2);
                        VECTOR * ptr = LockVertexBuffer();
                        ptr[0] = VECTOR(x1, height.a, z1);
                        ptr[1] = VECTOR(x1, side.d,   z1);
                        ptr[2] = VECTOR(x2, side.c,   z1);
                        ptr[3] = VECTOR(x2, height.b, z1);
                        UnlockVertexBuffer(4);
                        Playfield.RenderFunction();
                }
        }

       	// draw the top of the cube
        SetColor(color);
        VECTOR * ptr = LockVertexBuffer();
        ptr[0] = VECTOR(x1, height.a, z1);
        ptr[1] = VECTOR(x2, height.b, z1);
        ptr[2] = VECTOR(x2, height.c, z2);
        ptr[3] = VECTOR(x1, height.d, z2);
        UnlockVertexBuffer(4);
        Playfield.RenderFunction();

        // special case for rendering solid sloped tiles
        // this is done for maximum compatibility with real machine
        if (!(TileA & 0x0040) && Playfield.RenderFunction == Polygon)
                Vector();

#if 0
       	// draw the bottom of the cube
        SetColor(color);
        ptr = LockVertexBuffer();
        ptr[0] = VECTOR(x1, -View.Position.y + TILE_SIZE_Y, z1);
        ptr[1] = VECTOR(x1, -View.Position.y + TILE_SIZE_Y, z2);
        ptr[2] = VECTOR(x2, -View.Position.y + TILE_SIZE_Y, z2);
        ptr[3] = VECTOR(x2, -View.Position.y + TILE_SIZE_Y, z1);
        UnlockVertexBuffer(4);
        Playfield.RenderFunction();
#endif
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void __fastcall TMathbox::Execute(void)
{
	// command is in first mathbox address
	switch (Memory[0])
	{
	case COMMAND_START_PLAYFIELD:
		RasterizePlayfield();
		break;
	case COMMAND_UNKNOWN_MATH_FUNCTION:
		break;
	case COMMAND_ROLL:
		Roll((__int16*) &Memory[Memory[6]], Memory[7], Memory[8]);
		break;
	case COMMAND_YAW:
		Yaw((__int16*) &Memory[Memory[6]], Memory[7], Memory[8]);
		break;
	case COMMAND_PITCH:
		Pitch((__int16*) &Memory[Memory[6]], Memory[7], Memory[8]);
		break;
	case COMMAND_CAMERA:
		MatrixMultiply((__int16*) &Memory[Memory[0xF6/2]], (__int16*) &Memory[Memory[0xF8/2]], (__int16*) &Memory[Memory[0xFA/2]] );
		break;
	default:
		RasterizeObject(Memory[0]);
		break;
	}

        // signal mathbox completion to CPU
        M6809_IRQ_Pending |= FIRQ;
}

