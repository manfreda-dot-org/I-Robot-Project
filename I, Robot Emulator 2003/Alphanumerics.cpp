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

#include "Alphanumerics.h"
#include "Emulator.h"

#define NUM_CHARS  64

static const struct {
        int Width, Height;
        int NumChars;
} Font = {8, 8, NUM_CHARS};

static LPDIRECT3DDEVICE9 Device = NULL;
static LPDIRECT3DSURFACE9 Character[NUM_CHARS][4][2]; // 4 colors, 2 palettes
static LPDIRECT3DTEXTURE9 Overlay;

static TPoint ScreenResolution;
static D3DXVECTOR2 Scale;

//---------------------------------------------------------------------------

// custom vertex structure for rendering alphanumerics as "textures"
// note: color key is used to enable changing the color of the alphanumeric
struct D3DTLVERTEX {
        float    x, y, z; // Position
       	float    rhw;     // Reciprocal of homogeneous w
        D3DCOLOR colour;  // Vertex colour
       	float    tu, tv;  // Texture coordinates
};
static const DWORD D3DFVF_TLVERTEX = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

//-----------------------------------------------------------------------------
// Builds a transformation matrix from the arguments.
//
// pOut     - The result of this operation.
// pCentre  - The centre of scaling and rotation.
// pScaling - The scaling vector.
// angle    - The angle of rotation in radians.
//-----------------------------------------------------------------------------
static D3DMATRIX* BuildMatrix(D3DMATRIX* pOut, D3DXVECTOR2* centre, D3DXVECTOR2* scaling, float angle)
{
	D3DXMATRIX matrices[5];

	D3DXMatrixTranslation(&matrices[0], -centre->x, -centre->y, 0);
	D3DXMatrixScaling(&matrices[1], scaling->x, scaling->y, 1);
	D3DXMatrixIdentity(&matrices[2]);
	D3DXMatrixIdentity(&matrices[3]);
        if (angle)
        	D3DXMatrixRotationZ(&matrices[2], angle);
	D3DXMatrixTranslation(&matrices[3], centre->x, centre->y, 0);

	matrices[4] = matrices[0] * matrices[1] * matrices[2] * matrices[3];
	*pOut = matrices[4];

	return pOut;
}

//-----------------------------------------------------------------------------
// Transform the points in vertices[] according to the arguments
//
// pCentre  - The centre of scaling and rotation.
// pScaling - Scaling vector.
// angle    - Angle of rotation in radians.
//-----------------------------------------------------------------------------
static void __fastcall TransformVertices(D3DTLVERTEX vertices[], D3DXVECTOR2* pCentre, D3DXVECTOR2* pScaling, float angle)
{
	D3DXMATRIX matTransf, matVerts, matNew;

	BuildMatrix(&matTransf, pCentre, pScaling, angle);

	for (int cr = 0; cr < 4; cr++)
	{
		// 4 vertices fit nicely into a 4x4 matrix --
		// Put each vertex point into a matrix row.
		matVerts(cr, 0) = vertices[cr].x;
		matVerts(cr, 1) = vertices[cr].y;
		matVerts(cr, 2) = vertices[cr].z;
		matVerts(cr, 3) = 1.0f;  // the scaling factor, w
	}

	// Reuse D3D matrix multiplication code to transform our vertices.
	matNew = matVerts * matTransf;

	for (int cr = 0; cr < 4; cr++)
	{
		// Retrieve the newly transformed points.
		vertices[cr].x = matNew(cr, 0);
		vertices[cr].y = matNew(cr, 1);
		vertices[cr].z = matNew(cr, 2);
	}
}

//-----------------------------------------------------------------------------
// Draws a textured quad (sprite).
//
// pTexture - The source texture used for the sprite.
// pDest    - Draw sprite at these screen coordinates.
// pCenter  - Centre of scaling and rotation, relative to pDest.
// pScaling - Scaling vector. If NULL, it is treated as identity.
// angle    - Angle of rotation in radians.
// colour   - The RGB and alpha channels are modulated by this value. Use
//            0xFFFFFFFF for a standard blit.
//-----------------------------------------------------------------------------
static void __fastcall Blit(LPDIRECT3DTEXTURE9 pTexture, POINT* pDest, D3DXVECTOR2* pCentre, D3DXVECTOR2* pScaling, float angle, D3DCOLOR colour)
{
	D3DSURFACE_DESC surfDesc;

	pTexture->GetLevelDesc(0, &surfDesc);

	float left   = (float)pDest->x;
	float top    = (float)pDest->y;
	float right  = left + surfDesc.Width; // - 1;
	float bottom = top + surfDesc.Height; // - 1;

	const float z = 0.0f, rhw = 1.0f;

	D3DTLVERTEX vertices[4] =
	{
		// x, y, z, rhw, colour, tu, tv
		{ left,  top,    z, rhw, colour, 0, 0 },
		{ right, top,    z, rhw, colour, 1, 0 },
		{ right, bottom, z, rhw, colour, 1, 1 },
		{ left,  bottom, z, rhw, colour, 0, 1 }
	};

	D3DXVECTOR2 centre, scaling;

	centre.x = (float)pDest->x + pCentre->x;
	centre.y = (float)pDest->y + pCentre->y;
	pCentre = &centre; // Don't want to modify the argument passed in.

	if (pScaling == NULL) // Use identity: no scaling performed.
	{
		scaling.x = scaling.y = 1;
		pScaling = &scaling;
	}

	TransformVertices(vertices, pCentre, pScaling, angle);

	// Draw the sprite
	Device->SetTexture(0, pTexture);
	Device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(D3DTLVERTEX));
}


static void __fastcall FastBlit(LPDIRECT3DTEXTURE9 pTexture, RECT *dest, D3DCOLOR colour)
{
	const float z = 0.0f, rhw = 1.0f;
	D3DTLVERTEX vertices[4] =
	{
		// x, y, z, rhw, colour, tu, tv
		{ dest->left,  dest->top,    z, rhw, colour, 0, 0 },
		{ dest->left,  dest->bottom, z, rhw, colour, 0, 1 },
		{ dest->right, dest->bottom, z, rhw, colour, 1, 1 },
		{ dest->right, dest->top,    z, rhw, colour, 1, 0 }
	};

	// Draw the sprite
	Device->SetTexture(0, pTexture);
	Device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(D3DTLVERTEX));
}

//---------------------------------------------------------------------------

__fastcall TAlphanumerics::TAlphanumerics()
{
}


__fastcall TAlphanumerics::~TAlphanumerics()
{
        Destroy();
}

//---------------------------------------------------------------------------

void __fastcall TAlphanumerics::InitIO(void)
{
	// $1Cxx-$1Fxx  alphanumerics RAM
        // setup 6809 read/write pointers
        BYTE * ptr = RAM;
	for (int n=0x1C; n<=0x1F; n++, ptr+=0x100)
		M6809_PageReadPointer[n] =
		M6809_PageWritePointer[n] = (DWORD) ptr;
}

//---------------------------------------------------------------------------

void __fastcall TAlphanumerics::Destroy(void)
{
//	Device->SetTexture(0, NULL);
	for (int i=0; i<Font.NumChars; i++)
                for (int p=0;p<2;p++)
                        for (int c=0;c<4;c++)
                        {
                		_RELEASE_(Character[i][c][p]);
                        }
        _RELEASE_(Overlay);
}

//---------------------------------------------------------------------------

bool __fastcall TAlphanumerics::CreateOverlayTexture(void)
{
        // create temp overlay texture
        LPDIRECT3DTEXTURE9 temp;
        if (FAILED(Device->CreateTexture(
                32*Font.Width, 29*Font.Height,
                1,
                0,
                D3DFMT_A8R8G8B8,
                D3DPOOL_SYSTEMMEM,
                &temp,
                NULL)))
                return false;

        // wipe the overlay
        D3DLOCKED_RECT rect;
        if (FAILED(temp->LockRect(0, &rect, NULL, D3DLOCK_DISCARD))) return false;
        rect.Pitch /= 4; // we access 4 bytes at a time!
        for (int y=0; y<29*Font.Height; y++)
        {
                DWORD *pixel = (DWORD*) rect.pBits;
                pixel = &pixel[y*rect.Pitch];
                for (int x=0; x<32*Font.Width; x++)
                        *pixel++ = 0;
        }
        temp->UnlockRect(0);

        // create the overlay
        if (FAILED(Device->CreateTexture(
                32*Font.Width, 29*Font.Height,
                1,
                0,
                D3DFMT_A8R8G8B8,
                D3DPOOL_DEFAULT,
                &Overlay,
                NULL)))
                return false;

        // copy 'empty' texture onto official overlay
        LPDIRECT3DSURFACE9 src, dst;
        temp->GetSurfaceLevel(0, &src);
        Overlay->GetSurfaceLevel(0, &dst);
        if (FAILED(Device->UpdateSurface(src, NULL, dst, NULL)))
                return false;

        // release temp texture
        _RELEASE_(temp);

        return true;
}

bool __fastcall TAlphanumerics::CreateAlphaSurfaces(void)
{
        // create alphanumeric textures
	for (int alpha=0; alpha<Font.NumChars; alpha++)
        {
                for (int pal=0;pal<2;pal++)
                {
                        for (int col=0;col<4;col++)
                        {
                                // create target surface
                        	if (FAILED(Device->CreateOffscreenPlainSurface(
                                        Font.Width, Font.Height,
                                        D3DFMT_A8R8G8B8,
                                        D3DPOOL_SYSTEMMEM,
                                        &Character[alpha][col][pal],
                                        NULL)))
                                        return false;

                                // lock surface
                                D3DLOCKED_RECT rect;
                                Character[alpha][col][pal]->LockRect(&rect, NULL, D3DLOCK_DISCARD);
                                rect.Pitch /= 4; // we access 4 bytes at a time!

                                // build surface from ROM
                		for (int scanline=0; scanline<Font.Height; scanline++)
                                {
                                        BYTE bits = AlphaROM.Character[alpha][scanline];
                                        BYTE mask = 0x80;
                                        DWORD *pixel = (DWORD*) rect.pBits;
                                        pixel = &pixel[scanline*rect.Pitch];
                                        do
                                        {
                                                if (bits & mask)
                                                        *pixel++ = ColorROM.Color[pal][col];
                                                else
                                                        *pixel++ = D3DCOLOR_ARGB(0,255,255,255);
                                                mask >>= 1;
                                        } while (mask);
                                }

                                Character[alpha][col][pal]->UnlockRect();
                        }
                }
        }

        return true;
}

bool __fastcall TAlphanumerics::Create(LPDIRECT3DDEVICE9 device)
{
        Device = device;

        Destroy(); // destroy any allocated surfaces

        if (!CreateAlphaSurfaces())
                return false;

        if (!CreateOverlayTexture())
                return false;

        // determine optimal scale factor for our resolution
        ScreenResolution = Video.ScreenResolution();
        Scale.x = ScreenResolution.x / (float) (Font.Width*32);
        Scale.y = ScreenResolution.y / (float) (Font.Height*29);

        // if scaling is close to an even division (/1, /2, /4)
        // then just use that value
        #define TRY(a,b) if ((b*a) - abs(b*a) < 0.1f) a = abs(b*a)/b
        TRY(Scale.x, 1.0);
        else TRY(Scale.x, 2.0);
        else TRY(Scale.x, 4.0);
        TRY(Scale.y, 1.0);
        else TRY(Scale.y, 2.0);
        else TRY(Scale.y, 4.0);

        return true;
}

void __fastcall TAlphanumerics::Render()
{
        // return if overlay is 100% transpareny
        if (Config.OverlayTransparency == 0)
                return;

        // copy alphanumerics onto overlay texture
        LPDIRECT3DSURFACE9 surface;
        Overlay->GetSurfaceLevel(0, &surface);
        POINT origin;
        BYTE *ram = RAM;
        int Palette = Hardware.Registers._1180 >> 7;
        for (origin.y=0; origin.y<29*8; origin.y+=8)
        {
                for (origin.x=0; origin.x<32*Font.Width; origin.x+=8)
                {
                        // read RAM byte, copy associated texture to overlay
                        BYTE byte = *ram++;
                        Device->UpdateSurface(Character[byte & 0x3F][byte>>6][Palette], NULL, surface, &origin);
                }
        }

	// Enable alpha blended transparency.
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// Allow modulation of the texture's and diffuse colour's alpha.
	// By default, the texture and diffuse colour's RGB are modulated.
	// This lets us create transparency and tinting effects by setting
	// the (diffuse) colour in Blit().
	Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        Device->SetFVF(D3DFVF_TLVERTEX);

        // show the overlay
        D3DXVECTOR2 scale = Scale;
        if (Config.Force1x1Scale)
        {
                if (scale.x > scale.y)
                        scale.x = scale.y;
                else
                        scale.y = scale.x;
        }
        TPoint size(scale.x * 32 * Font.Width, scale.y * 29 * Font.Height);
        RECT dest;
        dest.left = (ScreenResolution.x - size.x) / 2;
        dest.right = dest.left + size.x;
        dest.top = (ScreenResolution.y - size.y) / 2;
        dest.bottom = dest.top + size.y;
        FastBlit(Overlay, &dest, D3DCOLOR_RGBA(255,255,255,Config.OverlayTransparency));

        // restore default transparency render state
        Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
        Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
}

