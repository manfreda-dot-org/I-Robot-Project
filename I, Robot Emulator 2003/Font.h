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

#ifndef FontH
#define FontH

#include <system.hpp>
#include "DirectX.h"

class TDirectXFont
{
private:
        LPDIRECT3DDEVICE9 pDevice;
        LPD3DXFONT pFont;
public:
        __fastcall TDirectXFont(LPDIRECT3DDEVICE9 Device, LPSTR pFontFace, int nHeight, bool fBold, bool fItalic, bool fUnderlined);
        __fastcall ~TDirectXFont();

        TPoint __fastcall GetTextExtent(AnsiString string);
        void __fastcall DrawText(AnsiString string, TPoint pt, D3DCOLOR rgbFontColour);
        void __fastcall DrawText(AnsiString string, RECT rect, D3DCOLOR rgbFontColour);
};

//---------------------------------------------------------------------------
#endif
