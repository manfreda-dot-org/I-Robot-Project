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

#ifndef MenuH
#define MenuH
//---------------------------------------------------------------------------

#include "Font.h"

typedef bool (*CHECKFUNCTION)(int);
typedef void (*CLICKFUNCTION)(int);

class TGameMenu;

class TGameMenuItem
{
        friend class TGameMenu;
private:
        AnsiString mText;
        TGameMenuItem *pParent;
        TGameMenuItem *pPrevSibling;
        TGameMenuItem *pNextSibling;
        TGameMenuItem *pChild;
        bool mEnabled;
        CHECKFUNCTION CheckFunction;
        CLICKFUNCTION ClickFunction;
        int mParameter;
public:
        __fastcall TGameMenuItem(AnsiString Text, TGameMenuItem *Parent=NULL, CLICKFUNCTION OnClick=NULL, CHECKFUNCTION IsChecked=NULL, int Parameter = 0, bool Enabled = true);
        __fastcall ~TGameMenuItem();

        bool Click(void);
        bool IsValidParent(TGameMenuItem *Parent);
        void __fastcall DeleteSubmenu(void);
};

class TGameMenu
{
private:
        LPDIRECT3DDEVICE9 Device;
        TGameMenuItem *Root;
        TGameMenuItem *ActiveMenu;
        TGameMenuItem *MouseMenu;
        bool mVisible;

        TPoint MousePos;
        TPoint Border;

        void __fastcall DrawMenu(TGameMenuItem * menu, TPoint pt = TPoint(0,0), TPoint size = TPoint(0,0));

public:
        __fastcall TGameMenu();
        __fastcall ~TGameMenu();

        void __fastcall Create(LPDIRECT3DDEVICE9 device);

        void __fastcall Show();
        void __fastcall Hide();
        bool __fastcall Visible() { return mVisible; }

        TGameMenuItem* __fastcall AddMenu(AnsiString Text, TGameMenuItem *Parent=NULL, CLICKFUNCTION OnClick=NULL, CHECKFUNCTION IsChecked=NULL, int Parameter = 0, bool Enabled = true);
        TGameMenuItem* __fastcall AddSeparator(TGameMenuItem *Parent);

        void __fastcall Render();
};

extern TGameMenu Menu;

//---------------------------------------------------------------------------
#endif
