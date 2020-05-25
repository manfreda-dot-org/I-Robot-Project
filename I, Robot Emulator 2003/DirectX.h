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

#ifndef DirectXH
#define DirectXH
//---------------------------------------------------------------------------

#include <system.hpp>

#define INITGUID

// needed to make DirectX header files work in Borland
#define cosf  (float)cos
#define sinf  (float)sin
#define acosf (float)acos
#define asinf (float)asin
#define tanf  (float)tan
#define atanf (float)atan
#define sqrtf (float)sqrt

#include <stdlib.h>
#include <math.h>
#include <mmsystem.h>
//#include <dsound.h>
//#include <d3d9types.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9tex.h>
//#include <ddraw.h>
//#include <d3drm.h>
#include <DInput.h>
//#include <dsound.h>
//#include <ddutil.h>
//#include <dsutil.h>


// If you want to use Direct3D the following line must be added
// to your program, to override the floating point exception handler:
// int _matherr(struct _exception *e){e; return 1;}
// In addition, you will need to add the following line
// to your initialization routine prior to starting Direct3D:
// _control87(MCW_EM,MCW_EM);
// It has something to do with Borland error routines,
// don't ask why , just put it in somewhere

#define _RELEASE_(p) if (p) { (p)->Release(); (p)=NULL; }
#define _DELETE_(p)  if (p) { delete (p);     (p)=NULL; }

//---------------------------------------------------------------------------
#endif
