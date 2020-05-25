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
#pragma hdrstop

#include "Video.h"
#include "Emulator.h"

TVideo Video;

//---------------------------------------------------------------------------

static struct {
        int NumModes;
        struct {
                UINT Width;
                UINT Height;
                UINT RefreshRate;
        } Mode[256];
        int NumRefreshRates;
        UINT RefreshRate[256];
} Display;


static TForm *Parent = NULL;
static LPDIRECT3D9 pD3D = NULL;          // used to create the D3DDevice
static LPDIRECT3DDEVICE9 pDevice = NULL;    // our rendering device
static const D3DFORMAT D3DFormat = D3DFMT_R5G6B5;

static struct { int Display, Count; DWORD Time; } FPS;

static bool ChangeAdapterMode = false;

static bool FullScreen = true;


//---------------------------------------------------------------------------

__fastcall TVideo::TVideo()
{
        Scanline = 0;
        MenuFont = NULL;
        Font = NULL;

        // enumerate all display modes
        LPDIRECT3D9 d3d = Direct3DCreate9(D3D_SDK_VERSION);
        Display.NumModes = d3d->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFormat);
        Display.NumRefreshRates = 0;
        for (int i=0; i<Display.NumModes; i++)
        {
                D3DDISPLAYMODE dm;
                d3d->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFormat, i, &dm);
                Display.Mode[i].Width = dm.Width;
                Display.Mode[i].Height = dm.Height;
                Display.Mode[i].RefreshRate = dm.RefreshRate;

                int j;
                for (j=0; j<Display.NumRefreshRates; j++)
                {
                        if (Display.RefreshRate[j] == dm.RefreshRate)
                                break;
                }
                if (j == Display.NumRefreshRates)
                        Display.RefreshRate[Display.NumRefreshRates++] = dm.RefreshRate;
        }
        d3d->Release();

        // sanity check configuration
        if (Config.NumAdapterModes != Display.NumModes
                || Config.AdapterMode >= Display.NumModes
                || Display.Mode[Config.AdapterMode].RefreshRate != (UINT) Config.RefreshRate)
        {
                Config.NumAdapterModes = Display.NumModes;

                // find first mode that supports 60 Hz
                Config.RefreshRate = 60; // default to 60 Hz;
                Config.AdapterMode = Config.NumAdapterModes/2; // pick a mode in the middle
                while (Config.AdapterMode && Display.Mode[Config.AdapterMode].RefreshRate != 60)
                        Config.AdapterMode--;
        }

        if (!Config.NumAdapterModes)
                Terminate("No supported video modes found.");
}

__fastcall TVideo::~TVideo()
{
        Destroy();
       _RELEASE_(pD3D);
}

//---------------------------------------------------------------------------

TPoint __fastcall TVideo::ScreenResolution(void)
{
        if (FullScreen)
        {
                D3DDISPLAYMODE dm;
                pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFormat, Config.AdapterMode, &dm);
                return TPoint(dm.Width, dm.Height);
        }
        return TPoint(MainForm->Width, MainForm->Height);
}

//---------------------------------------------------------------------------

D3DPRESENT_PARAMETERS __fastcall TVideo::GetPresentationParamters(void)
{
        // Most parameters are zeroed out
        // Set the SwapEffect to "discard", which is the most
        // efficient method of presenting the back buffer to the display.

        // display mode information
        D3DPRESENT_PARAMETERS d3dpp;
        ZeroMemory(&d3dpp, sizeof(d3dpp));
        D3DDISPLAYMODE dm;
	if (FullScreen)
	{
                // read information regarding requested adapter mode
                if (FAILED(pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFormat, Config.AdapterMode, &dm)))
                {
                        // we have a problem -- requested mode does not work!
                        Terminate("Requested fullscreen mode not available");
                }

		d3dpp.BackBufferWidth = dm.Width;
		d3dpp.BackBufferHeight = dm.Height;
		d3dpp.BackBufferCount = 1;
		d3dpp.FullScreen_RefreshRateInHz = dm.RefreshRate;
	}
	else
		pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dm);
	d3dpp.BackBufferFormat = dm.Format;

	d3dpp.Windowed = !FullScreen;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;

        return d3dpp;
}

bool __fastcall TVideo::InitDevice(HWND hwnd)
{
        Destroy();

        // create Direct3D interface
        pD3D = Direct3DCreate9(D3D_SDK_VERSION);
        if (!pD3D)
        {
                Terminate("Failed to get Direct3D interface");
                return false;
        }

        // get the presentation parameter structure used to create the device
        D3DPRESENT_PARAMETERS d3dpp = GetPresentationParamters();

        // Create the Direct3D device. Here we are using the default adapter (most
        // systems only have one, unless they have multiple graphics hardware cards
        // installed) and requesting the HAL (which is saying we want the hardware
        // device rather than a software one). Software vertex processing is
        // specified since we know it will work on all cards. On cards that support
        // hardware vertex processing, though, we would see a big performance gain
        // by specifying hardware vertex processing.
        if( FAILED( pD3D->CreateDevice(
                D3DADAPTER_DEFAULT,
                D3DDEVTYPE_HAL,
                hwnd,
                D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                &d3dpp,
                &pDevice ) ) )
        {
                Terminate("Failed to create video device");
                return false;
        }

        // cull back facing polygons
        pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

        // clear display
        pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB (0, 0, 0), 1.0f, 0);
        pDevice->BeginScene();
        pDevice->EndScene();
        pDevice->Present(NULL, NULL, NULL, NULL);

        // create fonts
        int size = 35 * ScreenResolution().y / 768;
        MenuFont = new TDirectXFont(pDevice, "Arial", size, true, false, false);
        size /= 2;
        Font = new TDirectXFont(pDevice, "Arial", max(10,size), false, false, false);

        return true;
}

//---------------------------------------------------------------------------

bool __fastcall TVideo::Create(TForm *form)
{
        Parent = form;

        FPS.Count = 0;
        FPS.Time = timeGetTime();

        // bring window to the front
        form->Show();

        // initialize the device
        if (!InitDevice(form->Handle))
                return false;

        // rebuild the menu
        Menu.Create(pDevice);
        RebuildVideoMenu();

        // initialize hardware surfaces
        if (!Hardware.Alphanumerics.Create(pDevice))
                return false;
        if (!Hardware.Mathbox.Create(pDevice))
                return false;

        TPoint size = ScreenResolution();
        Log.Add("Video subsystem (" + IntToStr(size.x) + "x" + IntToStr(size.y) + " " + IntToStr(Config.RefreshRate) + "Hz): OK");

        return true;
}

void __fastcall TVideo::Destroy()
{
        _DELETE_(MenuFont);
        _DELETE_(Font);
        _RELEASE_(pDevice);
        _RELEASE_(pD3D);
}

//---------------------------------------------------------------------------

void __fastcall TVideo::Render(void)
{
        if ((DWORD)(timeGetTime() - FPS.Time) >= 1000)
        {
                FPS.Display = FPS.Count;
                FPS.Count = 0;
                FPS.Time += 1000;
        }

        if (ChangeAdapterMode)
        {
                ChangeAdapterMode =  false;
                FullScreen = true;
                Create(Parent);
        }

        // does device exist?
        if (!pDevice)
        {
                Terminate("D3D device missing");
                return;
        }

        // is rendering possible?
        switch (pDevice->TestCooperativeLevel())
        {
        case D3D_OK: // we can render
                break;
        default:
        case D3DERR_DEVICELOST: // rendering not possible now
                return;
        case D3DERR_DEVICENOTRESET: // device lost but can be reset
                D3DPRESENT_PARAMETERS d3dpp = GetPresentationParamters();
                Create(Parent);
                return;
        }

        // Begin the scene
        if (SUCCEEDED(pDevice->BeginScene()))
        {
                // render the layers, in order
                Hardware.Mathbox.Render();
                Hardware.Alphanumerics.Render();
                Log.Render();
                Menu.Render();

                FPS.Count++;
                if (Config.ShowFPS)
                {
                        AnsiString fps = " FPS: " + IntToStr(FPS.Display) + " ";
                        TPoint size = Font->GetTextExtent(fps);
                        RECT rect = { 0, 0, 0, 0};
                        rect.right = ScreenResolution().x;
                        rect.left = rect.right - size.x;
                        rect.bottom = ScreenResolution().y;
                        rect.top = rect.bottom - size.y;
                        LPDIRECT3DSURFACE9 surface;
                        pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface);
                        pDevice->ColorFill(surface, &rect, D3DCOLOR_XRGB(255,0,0));
                        Font->DrawText(fps, rect, D3DCOLOR_XRGB(255,255,255));
                }

                // End the scene
                pDevice->EndScene();
        }

        // Present the backbuffer contents to the display
        pDevice->Present(NULL, NULL, NULL, NULL);
}

void __fastcall TVideo::Repaint(void)
{
        if (pDevice)
                pDevice->Present(NULL, NULL, NULL, NULL);
}

//---------------------------------------------------------------------------

static bool CheckVideoMode(int i) { return i == Config.AdapterMode; }
static void ClickVideoMode(int i)
{
        if (i == Config.AdapterMode)
                return;
        Config.AdapterMode = i;
        ChangeAdapterMode = true;
}

static bool CheckRefreshRate(int i) { return i == Config.RefreshRate; }
static void ClickRefreshRate(int i)
{
        if (i == Config.RefreshRate)
                return;
        Config.RefreshRate = i;
        ChangeAdapterMode = true;

        // save width and height of current mode
        UINT width = Display.Mode[Config.AdapterMode].Width;
        UINT height = Display.Mode[Config.AdapterMode].Height;

        // see if the same video mode exists at the new refresh rate
        for (i=0; i<Config.NumAdapterModes; i++)
        {
                if (Display.Mode[i].RefreshRate == (UINT) Config.RefreshRate)
                {
                        if (Display.Mode[i].Width == width && Display.Mode[i].Height == height)
                        {
                                Config.AdapterMode = i;
                                return;
                        }
                }
        }

        // hmmm... no compatible vidoe mode exists
        // find first video mode smaller than the current mode at the new refresh rate
        while (Config.AdapterMode)
        {
                if (Display.Mode[--Config.AdapterMode].RefreshRate == (UINT) Config.RefreshRate)
                        return;
        }

        // still no mode... this is strange
        // find ANY mode that has the new refresh rate
        while (Display.Mode[Config.AdapterMode].RefreshRate != (UINT) Config.RefreshRate)
                Config.AdapterMode++;
}

void __fastcall TVideo::RebuildVideoMenu(void)
{
        if (!ResolutionMenu || !RefreshRateMenu)
                return;

        // build the refresh rate menu
        RefreshRateMenu->DeleteSubmenu();
        for (int i=0; i<Display.NumRefreshRates; i++)
        {
                AnsiString s = IntToStr(Display.RefreshRate[i]) + " Hz";
                Menu.AddMenu(s, RefreshRateMenu, ClickRefreshRate, CheckRefreshRate, Display.RefreshRate[i]);
        }

        // rebuild the resolution menu
        ResolutionMenu->DeleteSubmenu();
        for (int i=0; i<Config.NumAdapterModes; i++)
        {
                if (Display.Mode[i].RefreshRate == (UINT) Config.RefreshRate)
                {
                        AnsiString s = IntToStr(Display.Mode[i].Width) + "x" + IntToStr(Display.Mode[i].Height);
                        Menu.AddMenu(s, ResolutionMenu, ClickVideoMode, CheckVideoMode, i);
                }
        }
}

