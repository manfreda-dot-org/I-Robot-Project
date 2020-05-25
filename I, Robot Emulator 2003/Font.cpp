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

#include "Font.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------

__fastcall TDirectXFont::TDirectXFont(LPDIRECT3DDEVICE9 Device, LPSTR FontFace, int nHeight, bool fBold, bool fItalic, bool fUnderlined)
{
        pDevice = Device;

        // create the font
        int nWeight = fBold ? FW_BOLD : FW_NORMAL;
        DWORD dwItalic = fItalic ? 1 : 0;
        DWORD dwUnderlined = fUnderlined ? 1 : 0;
        HFONT hFont = CreateFont(nHeight, 0, 0, 0, nWeight, dwItalic, dwUnderlined, 0, ANSI_CHARSET, 0, 0, 0, 0, FontFace);
        D3DXCreateFont(Device, hFont, &pFont);
        DeleteObject(hFont);
}

__fastcall TDirectXFont::~TDirectXFont()
{
       _RELEASE_(pFont);
}


TPoint __fastcall TDirectXFont::GetTextExtent(AnsiString string)
{
        RECT rect;
        pFont->Begin();
        pFont->DrawTextA(string.c_str(), -1, &rect, DT_CALCRECT, 0);
        pFont->End();
        return TPoint(rect.right-rect.left, rect.bottom-rect.top);
}

void __fastcall TDirectXFont::DrawText(AnsiString string, TPoint pt, D3DCOLOR rgbFontColour)
{
        RECT rect = { pt.x, pt.y, 0, 0 };
        pFont->Begin();
        pFont->DrawTextA(string.c_str(), -1, &rect, DT_CALCRECT, 0);           //Calculate the size of the rect needed
        pFont->DrawTextA(string.c_str(), -1, &rect, DT_LEFT, rgbFontColour);   //Draw the text
        pFont->End();
}

void __fastcall TDirectXFont::DrawText(AnsiString string, RECT rect, D3DCOLOR rgbFontColour)
{
        pFont->Begin();
        pFont->DrawTextA(string.c_str(), -1, &rect, DT_LEFT, rgbFontColour);   //Draw the text
        pFont->End();
}
