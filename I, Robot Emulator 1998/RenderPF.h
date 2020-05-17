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
// void RenderPlayfield( void );

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Local constants
#define TileSizeX     128
#define TileSizeY     64
#define TileSizeZ     128

// Local variables

static WORD PlayfieldObjectList, RenderingMethod;
static int NumRows, LeftOfCamera, RightOfCamera;

static int Xvector[3], Yvector[3], Zvector[3];

// Here's the layout of the tiles (TileA is in the center)
//
//          TileC   TileD
//  TileF   TileA   TileB
//          TileE
static WORD TileA, TileB, TileC, TileD, TileE, TileF;

#define Project(ThisX,ThisY,ThisZ) \
	if(ThisZ)\
	{\
		ThisX = ((ThisX << SCALEFACTORX) / ThisZ) + xmid;\
		ThisY = ((ThisY << SCALEFACTORY) / ThisZ) + ymid;\
		ThisZ = 0;\
	}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Functions

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static _inline void RenderPlayfieldTile( POINT * pPts, int color )
{
	if (RenderingMethod == 0x0000)
		DrawPolygon( pPts, 4, color);
	else if (RenderingMethod == 0x0100)
	{
		DrawVector( pPts[0].x, pPts[0].y, pPts[1].x, pPts[1].y, color );
		DrawVector( pPts[1].x, pPts[1].y, pPts[2].x, pPts[2].y, color );
		DrawVector( pPts[2].x, pPts[2].y, pPts[3].x, pPts[3].y, color );
		DrawVector( pPts[3].x, pPts[3].y, pPts[0].x, pPts[0].y, color );
	}
	else
	{
		DrawDot( pPts[0].x, pPts[0].y, color );
		DrawDot( pPts[1].x, pPts[1].y, color );
		DrawDot( pPts[2].x, pPts[2].y, color );
		DrawDot( pPts[3].x, pPts[3].y, color );
	}

}


static _inline void DrawPlayfieldTile( int ax, int ay, int az, int bx, int by, int bz )
{
	int color;
	int cx, cy, cz, dx, dy, dz, ex, ey, ez, fx, fy, fz, gx, gy, gz, hx, hy, hz;
	int h, HeightX, HeightY, HeightZ, HeightA=0;
	POINT points[4];

	// Don't draw empty tiles
	if ((TileA & 0xFF00) == 0x8000)
		goto TileDrawn;

	// Don't draw tiles that are behind the camera
	if (az<=0 || bz<=0)
		goto TileDrawn;

	// get all other 'top' points
	// bottom points are only retrieved if necessary
	cx = ax + Xvector[0];
	cy = ay + Xvector[1];
	cz = az + Xvector[2];
	ex = ax + Zvector[0];
	ey = ay + Zvector[1];
	ez = az + Zvector[2];
	gx = cx + Zvector[0];
	gy = cy + Zvector[1];
	gz = cz + Zvector[2];

	// surface is 'flat'
	if (TileA & 0x40)
	{
		// get tile base color
		color = TileA & 0x3F;

		// determine if height should be offset
		if (TileA&0x7F00)
		{
			HeightA = ((short)TileA) >> 8;
			HeightX = (HeightA * Yvector[0]) >> 4;
			HeightY = (HeightA * Yvector[1]) >> 4;
			HeightZ = (HeightA * Yvector[2]) >> 4;

			// offset top points by height
			ax += HeightX;
			cx += HeightX;
			ex += HeightX;
			gx += HeightX;
			ay += HeightY;
			cy += HeightY;
			ey += HeightY;
			gy += HeightY;
			az += HeightZ;
			cz += HeightZ;
			ez += HeightZ;
			gz += HeightZ;
		}

		// get all other 'bottom' points
		dx = bx + Xvector[0];
		dy = by + Xvector[1];
		dz = bz + Xvector[2];
		fx = bx + Zvector[0];
		hx = dx + Zvector[0];
		hy = dy + Zvector[1];
		fy = by + Zvector[1];
		hz = dz + Zvector[2];
		fz = bz + Zvector[2];

		// These 2 points must necessarily be projected
		// All other points are projected as needed
		Project( ax, ay, az )
		Project( ex, ey, ez )

		// draw only right or left side of cube, never both sides
		if (ax>ex)
		{
			// Draw the left side of the cube if:
			//       -- tile to the left is lower than current tile
			//     OR
			//       -- tile to the left is empty
			h = (TileF & 0x7F00) ? ((short)TileF) >> 8 : 0;
			if (h > HeightA ||
				(TileF>>8)==0x80)
			{
				Project( bx, by, bz )
				Project( fx, fy, fz )
				points[0].x = ex;
				points[0].y = ey;
				points[1].x = fx;
				points[1].y = fy;
				points[2].x = bx;
				points[2].y = by;
				points[3].x = ax;
				points[3].y = ay;
				RenderPlayfieldTile( points, color-2 );
			}
		}
		else
		{
			Project( cx, cy, cz )
			Project( gx, gy, gz )
			if (cx<gx)
			{
				// Draw the right side of the cube if:
				//       -- tile to the right is lower than current tile
				//     OR
				//       -- tile to the right is empty
				h = (TileB & 0x7F00) ? ((short)TileB) >> 8 : 0;
				if (h > HeightA ||
					(TileB>>8)==0x80)
				{
					Project( dx, dy, dz )
					Project( hx, hy, hz )
					points[0].x = cx;
					points[0].y = cy;
					points[1].x = dx;
					points[1].y = dy;
					points[2].x = hx;
					points[2].y = hy;
					points[3].x = gx;
					points[3].y = gy;
					RenderPlayfieldTile( points, color-2 );
				}
			}
		}
		
		// Draw the front of the cube if:
		//   - This is the last row to render
		// OR
		//   - Tile in front is lower than current tile
		// OR
		//   - Tile in front is empty
		h = (TileE & 0x7F00) ? ((short)TileE) >> 8 : 0;
		if (!NumRows ||
			h > HeightA ||
			(TileE>>8)==0x80)
		{
			Project( bx, by, bz )
			Project( cx, cy, cz )
			Project( dx, dy, dz )
			points[0].x = ax;
			points[0].y = ay;
			points[1].x = bx;
			points[1].y = by;
			points[2].x = dx;
			points[2].y = dy;
			points[3].x = cx;
			points[3].y = cy;
			RenderPlayfieldTile( points, color-4 );
		}

		// Draw the top of the cube if visible
		if(ay > ey)
		{
			Project( cx, cy, cz )
			Project( gx, gy, gz )
			points[0].x = ex;
			points[0].y = ey;
			points[1].x = ax;
			points[1].y = ay;
			points[2].x = cx;
			points[2].y = cy;
			points[3].x = gx;
			points[3].y = gy;
			RenderPlayfieldTile( points, color );
		}
/*		// else draw the bottom of the cube if visible
		else
		{
			Project( bx, by, bz )
			Project( fx, fy, fz )
			if(by < fy)
			{
				Project( dx, dy, dz )
				Project( hx, hy, hz )
				points[0].x = bx;
				points[0].y = by;
				points[1].x = dx;
				points[1].y = dy;
				points[2].x = hx;
				points[2].y = hy;
				points[3].x = fx;
				points[3].y = fy;
				RenderPlayfieldTile( points, color );
			}
		}
*/
	}
	// surface is sloped
	else
	{
		// offset each corner
		if (TileA & 0x7F00)
		{
			h = ((short)TileA) >> 8;
			ax += (h * Yvector[0]) >> 4;
			ay += (h * Yvector[1]) >> 4;
			az += (h * Yvector[2]) >> 4;
		}
		if (TileB & 0x7F00)
		{
			h = ((short)TileB) >> 8;
			cx += (h * Yvector[0]) >> 4;
			cy += (h * Yvector[1]) >> 4;
			cz += (h * Yvector[2]) >> 4;
		}
		if (TileC & 0x7F00)
		{
			h = ((short)TileC) >> 8;
			ex += (h * Yvector[0]) >> 4;
			ey += (h * Yvector[1]) >> 4;
			ez += (h * Yvector[2]) >> 4;
		}
		if (TileD & 0x7F00)
		{
			h = ((short)TileD) >> 8;
			gx += (h * Yvector[0]) >> 4;
			gy += (h * Yvector[1]) >> 4;
			gz += (h * Yvector[2]) >> 4;
		}

		// Project all points
		Project( ax, ay, az )
		Project( cx, cy, cz )
		Project( ex, ey, ez )
		Project( gx, gy, gz )

		// Get the sloped surface
		points[0].x = ex;
		points[0].y = ey;
		points[1].x = ax;
		points[1].y = ay;
		points[2].x = cx;
		points[2].y = cy;
		points[3].x = gx;
		points[3].y = gy;

		if (RenderingMethod == 0x0000)
		{
			int old = RenderingMethod;
			if (gx < 320)
				RenderingMethod = (gy > cy) ? 0x0100 : 0x0000;
			else if (ex > 320)
				RenderingMethod = (ey > ay) ? 0x0100 : 0x0000;				
			else
				RenderingMethod = (ey > ay || gy > cy) ? 0x0100 : 0x0000;
			RenderPlayfieldTile( points, TileA & 0x3F );
			RenderingMethod = old;
		}
		else
			RenderPlayfieldTile( points, TileA & 0x3F );

	}

TileDrawn:

	// draw any objects above this tile
	if (TileA & 0x80)
	{
		int count = MathRAMROM[PlayfieldObjectList++] + 1;
		while (count--)
			DoObject( MathRAMROM[PlayfieldObjectList++] );
	}
}


////////////////////////////////////////////////////////////////////////////////


// Render the row at the current Z axis position
static _inline void RenderPlayfieldRow( int AbsoluteRow,
	int tlX, int tlY, int tlZ,
	int trX, int trY, int trZ,
	int blX, int blY, int blZ,
	int brX, int brY, int brZ )
{
	WORD * NextRow, * ThisRow, * PrevRow;
	int n, c;

	// Get address of the three rows that determine how tiles
	// in the current row are drawn
	NextRow =
	ThisRow =
	PrevRow = &MathRAMROM[0xE00];
//	PrevRow = &MathRAMROM[MathRAMROM[0xE0/2]];
	NextRow += ((AbsoluteRow + 1) & 31) << 4;
	ThisRow += AbsoluteRow << 4;
	PrevRow += ((AbsoluteRow + 31) & 31) << 4;

	// Draw from left to middle (not including the middle)
	if (LeftOfCamera > 0)
	{
		c = LeftOfCamera;
		n = 2;
		TileA = *(ThisRow+1);
		TileC = *(NextRow+1);
		TileE = *(PrevRow+1);
		TileF = *(ThisRow);
	 	while (n<16 && --c)
		{
			TileB = *(ThisRow+n);
			TileD = *(NextRow+n);
			DrawPlayfieldTile( tlX, tlY, tlZ, blX, blY, blZ );
			TileE = *(PrevRow+n);
			TileF = TileA;
			TileA = TileB;
			TileC = TileD;
			tlX += Xvector[0];
			blX += Xvector[0];
			tlY += Xvector[1];
			blY += Xvector[1];
			tlZ += Xvector[2];
			blZ += Xvector[2];
			n++;
		}
	}

	// Draw from right to middle (including the middle)
	if (RightOfCamera > 0)
	{
		c = RightOfCamera;
		n = 14;
		TileA = *(ThisRow+15);
		TileB = *(ThisRow);
		TileC = *(NextRow+15);
		TileD = *(NextRow);
		TileE = *(PrevRow+15);
		while (n>=0 && c--)
		{
			TileF = *(ThisRow+n);
			DrawPlayfieldTile( trX, trY, trZ, brX, brY, brZ );
			TileB = TileA;
			TileA = TileF;
			TileD = TileC;
			TileC = *(NextRow+n);
			TileE = *(PrevRow+n);
			trX -= Xvector[0];
			brX -= Xvector[0];
			trY -= Xvector[1];
			brY -= Xvector[1];
			trZ -= Xvector[2];
			brZ -= Xvector[2];
			n--;
		}
	}
}
	

////////////////////////////////////////////////////////////////////////////////

// Playfield render function
void RenderPlayfield( void )
{
	int TopLeftPoint[3], TopRightPoint[3];
	int BottomLeftPoint[3], BottomRightPoint[3];
	int RowZ;

	// Get rendering method (dot/vector/polygon)
	RenderingMethod = MathRAMROM[0xE4/2];

	// get address of playfield object buffer
	PlayfieldObjectList = MathRAMROM[0xD6/2];

	// Determine number of rows to display
	NumRows = (int)((short)MathRAMROM[0xD8/2]) - (int)((short)MathRAMROM[0xDA/2]) + 1;

	// Determine number of cubes to the left and right of camera
	LeftOfCamera = CameraX >> 7;
	RightOfCamera = 16 - LeftOfCamera;

	// Determine first absolute row to be rendered
	RowZ = (MathRAMROM[0xEE/2] >> 4) + MathRAMROM[0xD8/2] + 1024;

	// load the camera matrix
	LoadCameraMatrix( 0x15 );

	// Create X Y and Z unit movement vectors
	Xvector[0] = (TileSizeX * CameraMatrix[0]) >> 14;
	Xvector[1] = (TileSizeX * CameraMatrix[3]) >> 14;
	Xvector[2] = (TileSizeX * CameraMatrix[6]) >> 14;
	Yvector[0] = (TileSizeY * CameraMatrix[1]) >> 14;
	Yvector[1] = (TileSizeY * CameraMatrix[4]) >> 14;
	Yvector[2] = (TileSizeY * CameraMatrix[7]) >> 14;
	Zvector[0] = (TileSizeZ * CameraMatrix[2]) >> 14;
	Zvector[1] = (TileSizeZ * CameraMatrix[5]) >> 14;
	Zvector[2] = (TileSizeZ * CameraMatrix[8]) >> 14;

	// Determine starting top left x/y/z position
	x = TileSizeX - ((short)MathRAMROM[0xEC/2] << 7) - (short)MathRAMROM[0xE8/2];
	y = -CameraY;
	z = ((short)MathRAMROM[0xD8/2] << 7) - (short)MathRAMROM[0xEA/2];
	TopLeftPoint[0] = (x*CameraMatrix[0] + y*CameraMatrix[1] + z*CameraMatrix[2]) >> 14;
	TopLeftPoint[1] = (x*CameraMatrix[3] + y*CameraMatrix[4] + z*CameraMatrix[5]) >> 14;
	TopLeftPoint[2] = (x*CameraMatrix[6] + y*CameraMatrix[7] + z*CameraMatrix[8]) >> 14;

	// Determine starting top right x/y/z position
	TopRightPoint[0] = TopLeftPoint[0] + Xvector[0] * 14;
	TopRightPoint[1] = TopLeftPoint[1] + Xvector[1] * 14;
	TopRightPoint[2] = TopLeftPoint[2] + Xvector[2] * 14;

	// Determine starting bottom left x/y/z position
	BottomLeftPoint[0] = TopLeftPoint[0] + (Yvector[0] << 2);
	BottomLeftPoint[1] = TopLeftPoint[1] + (Yvector[1] << 2);
	BottomLeftPoint[2] = TopLeftPoint[2] + (Yvector[2] << 2);

	// Determine starting bottom right x/y/z position
	BottomRightPoint[0] = TopRightPoint[0] + (Yvector[0] << 2);
	BottomRightPoint[1] = TopRightPoint[1] + (Yvector[1] << 2);
	BottomRightPoint[2] = TopRightPoint[2] + (Yvector[2] << 2);

	// Render each row in the playfield
	while (NumRows--)
	{
		RenderPlayfieldRow( RowZ-- & 31,
			    TopLeftPoint[0],     TopLeftPoint[1],     TopLeftPoint[2],
			   TopRightPoint[0],    TopRightPoint[1],    TopRightPoint[2],			
			 BottomLeftPoint[0],  BottomLeftPoint[1],  BottomLeftPoint[2],
			BottomRightPoint[0], BottomRightPoint[1], BottomRightPoint[2] );

		// Move all 4 points to next row along the Z axis
		TopLeftPoint[0] -= Zvector[0];
		TopRightPoint[0] -= Zvector[0];
		BottomLeftPoint[0] -= Zvector[0];
		BottomRightPoint[0] -= Zvector[0];
		TopLeftPoint[1] -= Zvector[1];
		TopRightPoint[1] -= Zvector[1];
		BottomLeftPoint[1] -= Zvector[1];
		BottomRightPoint[1] -= Zvector[1];
		TopLeftPoint[2] -= Zvector[2];
		TopRightPoint[2] -= Zvector[2];
		BottomLeftPoint[2] -= Zvector[2];
		BottomRightPoint[2] -= Zvector[2];
	}
}
