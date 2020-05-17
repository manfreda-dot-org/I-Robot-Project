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

typedef unsigned short WORD;
typedef unsigned char BYTE;

// external variable needed
extern WORD * MathRAMROM;

// external function prototypes
void GetCamera( void );
void DoObject( int ObjAddr );
void RenderPlayfield( void );

#define StartPlayfield 0x8400
#define UnknownMathFunction 0x8600
#define Roll 0x8800
#define Yaw 0x9000
#define Pitch 0xA000
#define Camera 0xC000

#pragma check_stack(off)

// Pitch function  (X)
// Multiplies source matrix by
//	[ 1   0   0  ]
//	[ 0  cos sin ]
//	[ 0 -sin cos ]
static void _inline Pfunction( short * matrix, short SIN, short COS )
{
	short B, C, E, F, H, I;

	B = matrix[1];
	C = matrix[2];
	E = matrix[4];
	F = matrix[5];
	H = matrix[7];
	I = matrix[8];

	matrix[1] = (short)((B * COS - C * SIN) >> 14);
	matrix[2] = (short)((C * COS + B * SIN) >> 14);
	matrix[4] = (short)((E * COS - F * SIN) >> 14);
	matrix[5] = (short)((F * COS + E * SIN) >> 14);
	matrix[7] = (short)((H * COS - I * SIN) >> 14);
	matrix[8] = (short)((I * COS + H * SIN) >> 14);
}

// Yaw function  (Y)
// Multiplies source matrix by
//	[ cos 0 -sin ]
//	[  0  1   0  ]
//	[ sin 0  cos ]
static void _inline Yfunction( short * matrix, short SIN, short COS )
{
	short A, C, D, F, G, I;

	A = matrix[0];
	C = matrix[2];
	D = matrix[3];
	F = matrix[5];
	G = matrix[6];
	I = matrix[8];
	
	matrix[0] = (short)((A * COS + C * SIN) >> 14);
	matrix[2] = (short)((C * COS - A * SIN) >> 14);
	matrix[3] = (short)((D * COS + F * SIN) >> 14);
	matrix[5] = (short)((F * COS - D * SIN) >> 14);
	matrix[6] = (short)((G * COS + I * SIN) >> 14);
	matrix[8] = (short)((I * COS - G * SIN) >> 14);
}

// Roll function  (Z)
// Multiplies source matrix by
//	[  cos sin 0 ]
//	[ -sin cos 0 ]
//	[   0   0  1 ]
static void _inline Rfunction( short * matrix, short SIN, short COS )
{
	short A, B, D, E, G, H;

	A = matrix[0];
	B = matrix[1];
	D = matrix[3];
	E = matrix[4];
	G = matrix[6];
	H = matrix[7];
	
	matrix[0] = (short)((A * COS - B * SIN) >> 14);
	matrix[1] = (short)((B * COS + A * SIN) >> 14);
	matrix[3] = (short)((D * COS - E * SIN) >> 14);
	matrix[4] = (short)((E * COS + D * SIN) >> 14);
	matrix[6] = (short)((G * COS - H * SIN) >> 14);
	matrix[7] = (short)((H * COS + G * SIN) >> 14);
}


// Camera function
// performs A x B = C
// result is C transpose
static void _inline Cfunction( short * matrixA, short * matrixB, short * matrixC )
{
	matrixC[0] = (short)((matrixA[0]*matrixB[0] + matrixA[1]*matrixB[1] + matrixA[2]*matrixB[2]) >> 14);
	matrixC[1] = (short)((matrixA[0]*matrixB[3] + matrixA[1]*matrixB[4] + matrixA[2]*matrixB[5]) >> 14);
	matrixC[2] = (short)((matrixA[0]*matrixB[6] + matrixA[1]*matrixB[7] + matrixA[2]*matrixB[8]) >> 14);
	matrixC[3] = (short)((matrixA[3]*matrixB[0] + matrixA[4]*matrixB[1] + matrixA[5]*matrixB[2]) >> 14);
	matrixC[4] = (short)((matrixA[3]*matrixB[3] + matrixA[4]*matrixB[4] + matrixA[5]*matrixB[5]) >> 14);
	matrixC[5] = (short)((matrixA[3]*matrixB[6] + matrixA[4]*matrixB[7] + matrixA[5]*matrixB[8]) >> 14);
	matrixC[6] = (short)((matrixA[6]*matrixB[0] + matrixA[7]*matrixB[1] + matrixA[8]*matrixB[2]) >> 14);
	matrixC[7] = (short)((matrixA[6]*matrixB[3] + matrixA[7]*matrixB[4] + matrixA[8]*matrixB[5]) >> 14);
	matrixC[8] = (short)((matrixA[6]*matrixB[6] + matrixA[7]*matrixB[7] + matrixA[8]*matrixB[8]) >> 14);
}

void DoMathbox( void )
{
	WORD command = *MathRAMROM;

	switch( command )
	{
	case StartPlayfield:
		GetCamera();
		RenderPlayfield( );
		break;
	case UnknownMathFunction:
		break;
	case Roll:
		Rfunction( &MathRAMROM[MathRAMROM[6]], MathRAMROM[7], MathRAMROM[8] );
		break;
	case Yaw:
		Yfunction( &MathRAMROM[MathRAMROM[6]], MathRAMROM[7], MathRAMROM[8] );
		break;
	case Pitch:
		Pfunction( &MathRAMROM[MathRAMROM[6]], MathRAMROM[7], MathRAMROM[8] );
		break;
	case Camera:
		Cfunction( &MathRAMROM[MathRAMROM[0xF6/2]], &MathRAMROM[MathRAMROM[0xF8/2]], &MathRAMROM[MathRAMROM[0xFA/2]] );
		break;
	default:
		GetCamera();
		DoObject( command );
		break;
	}
}