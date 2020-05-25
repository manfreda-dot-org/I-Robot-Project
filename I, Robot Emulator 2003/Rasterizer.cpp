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

#include "Rasterizer.h"
#include "Emulator.h"

static LPDIRECT3DDEVICE9 Device = NULL;
static LPDIRECT3DTEXTURE9 ScreenBuffer[2] = { NULL, NULL };
static D3DMATERIAL9 Material;
float PointSize = 1;

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ)

static struct {
        LPDIRECT3DVERTEXBUFFER9 Object;
        VECTOR * pVertices;
        int Length;
} VertexBuffer = { NULL, NULL, 0};

static int ActiveSurface = 0;

static TPoint ScreenResolution;

// structure that saves the old rendering surface
static struct {
        LPDIRECT3DSURFACE9 ColorBuffer;
        LPDIRECT3DSURFACE9 DepthBuffer;
} Old;

// vertices for blitting the frame buffer into the video card back buffer
static struct {
        float    x, y, z; // Position
        float    rhw;     // Reciprocal of homogeneous w
        float    tu, tv;  // Texture coordinates
} BlitVertices[4] =
{
	// x, y, z, rhw, colour, tu, tv
	{ 0, 0, 0, 1, 0, 0 },
	{ 0, 0, 0, 1, 0, 1 },
	{ 0, 0, 0, 1, 1, 1 },
	{ 0, 0, 0, 1, 1, 0 }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

__fastcall TRasterizer::TRasterizer()
{
}

void __fastcall TRasterizer::Destroy(void)
{
        _RELEASE_(VertexBuffer.Object);
        _RELEASE_(ScreenBuffer[0]);
        _RELEASE_(ScreenBuffer[1]);
}

bool __fastcall TRasterizer::Create(LPDIRECT3DDEVICE9 device)
{
        Device = device;

        Destroy();

        // get target render size
        ScreenResolution = Video.ScreenResolution();

        // setup for quick blits
        BlitVertices[2].x = BlitVertices[3].x = ScreenResolution.x;
        BlitVertices[1].y = BlitVertices[2].y = ScreenResolution.y;

        // reset material
        ZeroMemory(&Material, sizeof(Material));

        // determine point size
        int x = ScreenResolution.x / 256.0f + 0.5;
        int y = ScreenResolution.y / 232.0f + 0.5;
        PointSize = min(x,y);

        // create the two screen buffers
        for (int n=0; n<2; n++)
        {
               if (FAILED(Device->CreateTexture(
                        ScreenResolution.x, ScreenResolution.y,
                        1,
                        D3DUSAGE_RENDERTARGET,
                        D3DFMT_R5G6B5,
                        D3DPOOL_DEFAULT,
                        &ScreenBuffer[n],
                        NULL)))
                        return false;
        }

        // clear the active screen buffer
        ActiveSurface = 1; EraseScreenBuffer();
        ActiveSurface = 0; EraseScreenBuffer();

        // create the vertex buffers
        if (Device->CreateVertexBuffer(
                256 * sizeof(VECTOR),
                D3DUSAGE_DYNAMIC | D3DUSAGE_POINTS | D3DUSAGE_WRITEONLY,
                D3DFVF_CUSTOMVERTEX,
                D3DPOOL_DEFAULT,
                &VertexBuffer.Object,
                NULL) != D3D_OK)
                return false;

        ChangeScreenBuffer();

        return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void __fastcall TRasterizer::ChangeScreenBuffer(void)
{
        ActiveSurface = (Hardware.Registers._1140 & BUFSEL) ? 1 : 0;
}

void __fastcall TRasterizer::EraseScreenBuffer(void)
{
        StartRender();
        Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 1.0f, 0);
        EndRender();
}

void __fastcall TRasterizer::Render(void)
{
        Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID );
        Device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
        Device->SetTexture(0, ScreenBuffer[!ActiveSurface]);
        Device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, BlitVertices, sizeof(BlitVertices[0]));
        Device->SetTexture(0, NULL);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void __fastcall TRasterizer::StartRender(void)
{
        Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &Old.ColorBuffer);
        Device->GetDepthStencilSurface(&Old.DepthBuffer);

        LPDIRECT3DSURFACE9 surface;
        ScreenBuffer[ActiveSurface]->GetSurfaceLevel(0, &surface);
        Device->SetRenderTarget(0, surface);
        Device->SetDepthStencilSurface(NULL);

        Device->SetStreamSource(0, VertexBuffer.Object, 0, sizeof(VECTOR));
        Device->SetFVF(D3DFVF_CUSTOMVERTEX);

        Device->SetRenderState(D3DRS_AMBIENT, 0x00FFFFFF);
        Device->SetRenderState(D3DRS_FILLMODE, Config.Wireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
        Device->SetRenderState(D3DRS_ZENABLE, FALSE);
        Device->SetRenderState(D3DRS_POINTSIZE, *((DWORD*) &PointSize));

        // For the projection matrix, we set up a perspective transform (which
        // transforms geometry from 3D view space to 2D viewport space, with
        // a perspective divide making objects smaller in the distance). To build
        // a perpsective transform, we need the field of view (1/4 pi is common),
        // the aspect ratio, and the near and far clipping planes (which define at
        // what distances geometry should be no longer be rendered).
        float AspectRatio = 256.0 / 232.0;
        if (Config.Force1x1Scale)
                AspectRatio = (float) ScreenResolution.x / ScreenResolution.y;
        MATRIX matrix;
        D3DXMatrixPerspectiveFovLH(&matrix, D3DX_PI/3.7, AspectRatio, .01, 32768);

        // adjust the screen downwards, not sure why but I Robot does this
        MATRIX translate;
        D3DXMatrixTranslation(&translate, 0, -26.0 / 256, 0);
        matrix *= translate;

        Device->SetTransform(D3DTS_PROJECTION, &matrix);
}

void __fastcall TRasterizer::EndRender(void)
{
        Device->SetRenderTarget(0, Old.ColorBuffer);
        Device->SetDepthStencilSurface(Old.DepthBuffer);
}

void __fastcall TRasterizer::SetWorldMatrix(const MATRIX &rotation)
{
        Device->SetTransform(D3DTS_WORLD, &rotation);
}

void __fastcall TRasterizer::SetWorldMatrix(const VECTOR &position, const MATRIX &rotation)
{
        MATRIX world;
        D3DXMatrixTranslation(&world, position.x, position.y, position.z);
        world = rotation * world;
        Device->SetTransform(D3DTS_WORLD, &world);
}

void __fastcall TRasterizer::SetColor(float r, float g, float b)
{
        Material.Ambient.r = r;
        Material.Ambient.g = g;
        Material.Ambient.b = b;
        Material.Ambient.a = 0.0f;
        Device->SetMaterial(&Material);
}

VECTOR * __fastcall TRasterizer::LockVertexBuffer(void)
{
        VertexBuffer.Object->Lock(0, 256 * sizeof(VECTOR), (VOID**) &VertexBuffer.pVertices, D3DLOCK_DISCARD);
        return VertexBuffer.pVertices;
}

void __fastcall TRasterizer::UnlockVertexBuffer(int numvertices)
{
        VertexBuffer.Length = numvertices;

        // if object is to be rendered as a vector, we must close the object
        // by making the endpoint equal to the start point
        VertexBuffer.pVertices[numvertices] = VertexBuffer.pVertices[0];

        VertexBuffer.Object->Unlock();
}


void __fastcall TRasterizer::Dot(void)
{
        if (Config.ShowDots)
                Device->DrawPrimitive(D3DPT_POINTLIST, 0, VertexBuffer.Length);
}

void __fastcall TRasterizer::Vector(void)
{
        if (VertexBuffer.Length < 2)
                Dot();
        else if (Config.ShowVectors)
                Device->DrawPrimitive(D3DPT_LINESTRIP, 0, VertexBuffer.Length - 1 + (VertexBuffer.Length > 2));
}

void __fastcall TRasterizer::Polygon(void)
{
        if (VertexBuffer.Length < 3)
                Vector();
        else if (Config.ShowPolygons)
                Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, VertexBuffer.Length - 2);
}

