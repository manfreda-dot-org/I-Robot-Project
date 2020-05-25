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

#ifndef AlphanumericsH
#define AlphanumericsH
//---------------------------------------------------------------------------

#include <vcl.h>

#include "DirectX.h"
#include "AlphaROM.h"
#include "ColorROM.h"

class TAlphanumerics
{
private:
        TAlphaROM AlphaROM;
        TColorROM ColorROM;

        void __fastcall Destroy(void);
        bool __fastcall CreateAlphaSurfaces(void);
        bool __fastcall CreateOverlayTexture(void);
public:
        __fastcall TAlphanumerics();
        __fastcall ~TAlphanumerics();

        BYTE RAM[32*32]; // 1C00-1FFF

        bool __fastcall Create(LPDIRECT3DDEVICE9 device);
        void __fastcall InitIO(void);
        void __fastcall Render();
};

//---------------------------------------------------------------------------
#endif
