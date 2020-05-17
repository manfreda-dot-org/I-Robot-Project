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

void ScanEdge(int X1, int Y1, int X2, int Y2, int SetXStart, int SkipFirst, int **EdgePtr);
void DrawHorizontalLineList( int * LinePtr, int ListLength, int StartScanLine, int Color );

#define INDEX_FORWARD(Index) \
	if (++Index>=Length) Index=0;
#define INDEX_BACKWARD(Index) \
	if (--Index<0) Index=Length-1;
#define INDEX_MOVE(Index,Direction) \
	if (Direction > 0) {if (++Index>=Length) Index=0;}\
	else if (--Index<0) Index=Length-1;

static int LinePtr[480*10];

extern volatile int PerformingClipDump;
extern volatile int ShowDots;
extern volatile int ShowVectors;
extern volatile int ShowPolygons;
extern volatile HDC hdcMetafile;
extern PALETTEENTRY ColorRAM[256];

void _inline DrawDot( int x, int y, int color )
{
	if (!ShowDots)
		return;

	// align dots on 2 pixel boundaries
	x &= ~0x01;
	y &= ~0x01;
	
	if (x>=0 && x<640 && y>=0 && y<480)
//		*(ScreenBuffer + y * ScreenPitch + x) = (BYTE) color;
	{
		BYTE * pPixel = ScreenBuffer + y * ScreenPitch + x;

		*(pPixel++) = (BYTE) color;
		*(pPixel) = (BYTE) color;
		pPixel += ScreenPitch;
		*(pPixel--) = (BYTE) color;
		*(pPixel) = (BYTE) color;
	}

	if (PerformingClipDump)
	{
		HPEN hOldPen = SelectObject( hdcMetafile, GetStockObject( NULL_PEN ) );
		HBRUSH hBrush = CreateSolidBrush( RGB(ColorRAM[color].peRed,ColorRAM[color].peGreen,ColorRAM[color].peBlue) );
		HBRUSH hOldBrush = SelectObject( hdcMetafile, hBrush );
		Rectangle( hdcMetafile, x, y, x+1, y+1 );
		SelectObject( hdcMetafile, hOldPen );
		SelectObject( hdcMetafile, hOldBrush );
		DeleteObject( hBrush );
	}
}

void _inline DrawVector( int x1, int y1, int x2, int y2, int color )
{
	BYTE * PixelAddress;
	int flag = 0;
	int dx, dy;
	int m11, m12, m21, m22=ScreenPitch;
	int k1, k2, c, e;

	if (!ShowVectors)
		return;

	if (PerformingClipDump)
	{
		HPEN hPen = CreatePen( PS_SOLID, 1, RGB(ColorRAM[color].peRed,ColorRAM[color].peGreen,ColorRAM[color].peBlue) );
		HPEN hOldPen = SelectObject( hdcMetafile, hPen );
		MoveToEx( hdcMetafile, x1, y1, NULL );
		LineTo( hdcMetafile, x2, y2 );
		SelectObject( hdcMetafile, hOldPen );
		DeleteObject( hPen );
	}

	// make sure y1 is always highest point
	if (y1 > y2)
	{
		_asm mov	eax,y1
		_asm xchg	eax,y2
		_asm mov	y1,eax
		_asm mov	eax,x1
		_asm xchg	eax,x2
		_asm mov	x1,eax
	}

	// dont draw line if it goes off screen
	if (y2<0 ||
		y1>=480 ||
		(x1<0 && x2<0) ||
		(x1>=640 && x2>=640))
		return;

	if (x2 > x1)
	{
		dx = x2 - x1;
		m21 = 1;
		m22++;
	}
	else
	{
		dx = x1 - x2;
		m21 = -1;
		m22--;
	}

	dy = y2 - y1;

	if (dy > dx)
	{
		k1 = dx << 1;
		c = dy;
		m11 = 0;
		m12 = ScreenPitch;
	}
	else
	{
		k1 = dy << 1;
		c = dx;
		m12 = m11 = m21;
	}
	
	
	e = k1 - c;
	k2 = k1 - (c << 1);

	PixelAddress = ScreenBuffer + y1 * ScreenPitch + x1;

	if (x1>=0 && x1<640 && PixelAddress>=ScreenBuffer && PixelAddress<EndOfScreenBuffer)
		flag = 1, *PixelAddress = (BYTE) color;

	while ((--c)>=0)
	{
		if (e<0)
		{
			x1 += m11;
			PixelAddress += m12;
			e += k1;
		}
		else
		{
			x1 += m21;
			PixelAddress += m22;
			e += k2;
		}

		if (x1>=0 && x1<640 && PixelAddress>=ScreenBuffer && PixelAddress<EndOfScreenBuffer)
			flag = 1, *PixelAddress = (BYTE) color;
		else if (flag)
			return;
	}
}

void _inline DrawPolygon( POINT * Vertex, int Length, int Color )
{
	int i, MinIndexL, MaxIndex, MinIndexR, SkipFirst, Temp;
	int MinPoint_Y, MaxPoint_Y, TopIsFlat, LeftEdgeDir;
	int NextIndex, CurrentIndex, PreviousIndex;
	int DeltaXN, DeltaYN, DeltaXP, DeltaYP;
	int ListLength, YStart;
	int xPrev, yPrev, xNew, yNew;
	int * EdgePointPtr;

	if (!ShowPolygons)
		return;

	if (Length <= 0)
		return;

	if (PerformingClipDump)
	{
		HPEN hOldPen = SelectObject( hdcMetafile, GetStockObject( NULL_PEN ) );
		HBRUSH hBrush = CreateSolidBrush( RGB(ColorRAM[Color].peRed,ColorRAM[Color].peGreen,ColorRAM[Color].peBlue) );
		HBRUSH hOldBrush = SelectObject( hdcMetafile, hBrush );
		Polygon( hdcMetafile, Vertex, Length );
		SelectObject( hdcMetafile, hOldPen );
		SelectObject( hdcMetafile, hOldBrush );
		DeleteObject( hBrush );
	}

	// Scan the list to find the top and bottom of the polygon
	MinIndexL = MaxIndex = 0;
	MaxPoint_Y = MinPoint_Y = Vertex[0].y;
	for (i = 1; i < Length; i++)
	{
		if (Vertex[i].y < MinPoint_Y)
			MinPoint_Y = Vertex[MinIndexL = i].y;
		else if (Vertex[i].y > MaxPoint_Y)
			MaxPoint_Y = Vertex[MaxIndex = i].y;
	}
	if (MinPoint_Y == MaxPoint_Y)
		return;

	// Scan in ascending order to find the last top-edge point
	MinIndexR = MinIndexL;
	while (Vertex[MinIndexR].y == MinPoint_Y)
		{INDEX_FORWARD(MinIndexR)}
	INDEX_BACKWARD(MinIndexR)

	// Now scan in descending order to find the first top-edge point
	while (Vertex[MinIndexL].y == MinPoint_Y)
		{INDEX_BACKWARD(MinIndexL)}
	INDEX_FORWARD(MinIndexL)

	// Figure out which direction through the vertex list from the top
	// vertex is the left edge and which is the right
	LeftEdgeDir = -1;
	TopIsFlat = (Vertex[MinIndexL].x != Vertex[MinIndexR].x) ? 1 : 0;
	if (TopIsFlat == 1)
	{
		// If the top is flat, just see which of the ends is leftmost
		if (Vertex[MinIndexL].x > Vertex[MinIndexR].x)
		{
			LeftEdgeDir = 1;
			Temp = MinIndexL;
			MinIndexL = MinIndexR;
			MinIndexR = Temp;
		}
	}
	else
	{
		// Point to the downward end of the first line of each of the
		// two edges down from the top
		NextIndex = MinIndexR;
		INDEX_FORWARD(NextIndex)
		PreviousIndex = MinIndexL;
		INDEX_BACKWARD(PreviousIndex)
		// Calculate X and Y lengths from the top vertex to the end of
		// the first line down each edge; use those to compare slopes
		// and see which line is leftmost
		DeltaXN = Vertex[NextIndex].x - Vertex[MinIndexL].x;
		DeltaYN = Vertex[NextIndex].y - Vertex[MinIndexL].y;
		DeltaXP = Vertex[PreviousIndex].x - Vertex[MinIndexL].x;
		DeltaYP = Vertex[PreviousIndex].y - Vertex[MinIndexL].y;
		if (((long)DeltaXN * DeltaYP - (long)DeltaYN * DeltaXP) < 0L)
		{
			LeftEdgeDir = 1;
			Temp = MinIndexL;
			MinIndexL = MinIndexR;
			MinIndexR = Temp;
		}
	}

	// Set the # of scan lines in the polygon, skipping the bottom edge
	// and also skipping the top vertex if the top isn't flat because
	// in that case the top vertex has a right edge component, and set
	// the top scan line to draw, which is likewise the second line of
	// the polygon unless the top is flat
	YStart = max( 0, MinPoint_Y + 1 - TopIsFlat);
	ListLength = min(480, MaxPoint_Y) - YStart;
	if (ListLength <= 0)
		return;

	// Scan the left edge and store the boundary points in the list
	EdgePointPtr = LinePtr;
	CurrentIndex = MinIndexL;
	xPrev = Vertex[CurrentIndex].x;
	yPrev = Vertex[CurrentIndex].y;
	SkipFirst = TopIsFlat ? 0 : 1;
	// Scan convert each line in the left edge from top to bottom
	do {
		INDEX_MOVE(CurrentIndex,LeftEdgeDir)
		xNew = Vertex[CurrentIndex].x;
		yNew = Vertex[CurrentIndex].y;
		ScanEdge( xPrev, yPrev, xNew, yNew, 1, SkipFirst, &EdgePointPtr );
		xPrev = xNew;
		yPrev = yNew;
		SkipFirst = 0; // scan convert the first point from now on
	} while (CurrentIndex != MaxIndex);

	// Scan the right edge and store the boundary points in the list
	EdgePointPtr = LinePtr;
	CurrentIndex = MinIndexR;
	xPrev = Vertex[CurrentIndex].x;
	yPrev = Vertex[CurrentIndex].y;
	SkipFirst = TopIsFlat ? 0 : 1;
	// Scan convert the right edge, top to bottom. X coordinates are
	// adjusted 1 to the left, effectively causing scan conversion of
	// the nearest points to the left of but not exactly on the edge
	do {
		INDEX_MOVE(CurrentIndex,-LeftEdgeDir)
		xNew = Vertex[CurrentIndex].x;
		yNew = Vertex[CurrentIndex].y;
		ScanEdge( xPrev, yPrev, xNew, yNew, 0, SkipFirst, &EdgePointPtr );
		xPrev = xNew;
		yPrev = yNew;
		SkipFirst = 0; // scan convert the first point from now on
	} while (CurrentIndex != MaxIndex);

	DrawHorizontalLineList( LinePtr, ListLength, YStart, Color );
}
