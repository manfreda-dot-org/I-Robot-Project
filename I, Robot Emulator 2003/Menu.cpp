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

#include "Emulator.h"

#include "main.h"

#define CHECKBOX " × "
#define SUBMENU " >"

TGameMenu Menu;

//---------------------------------------------------------------------------

__fastcall TGameMenuItem::TGameMenuItem(AnsiString Text, TGameMenuItem *Parent, CLICKFUNCTION OnClick, CHECKFUNCTION IsChecked, int Parameter, bool Enabled)
{
        mText = Text;
        mParameter = Parameter;

        ClickFunction = OnClick;
        CheckFunction = IsChecked;

        // link up to parent
        pParent = Parent;

        // link up to sibling
        pPrevSibling = NULL;
        pNextSibling = NULL;
        if (pParent)
        {
                // find previous sibling
                pPrevSibling = pParent->pChild;
                if (!pPrevSibling)
                        pParent->pChild = this; // we are the first child
                else
                {
                        // find previous sibling
                        while (pPrevSibling->pNextSibling)
                                pPrevSibling = pPrevSibling->pNextSibling;
                        pPrevSibling->pNextSibling = this;
                }
        }

        // no child assumed (child must be created later)
        pChild = NULL;

        mEnabled = Enabled;
}

__fastcall TGameMenuItem::~TGameMenuItem()
{
        // fixup parent
        if (pParent && pParent->pChild == this)
                pParent->pChild = pNextSibling;

        // fixup siblings
        if (pPrevSibling)
                pPrevSibling->pNextSibling = pNextSibling;
        if (pNextSibling)
                pNextSibling->pPrevSibling = pPrevSibling;

        // delete child menu(s), if any
        DeleteSubmenu();
}

bool TGameMenuItem::Click(void)
{
        if (!ClickFunction)
                return false;
        ClickFunction(mParameter);
        return true;
}

bool TGameMenuItem::IsValidParent(TGameMenuItem *Parent)
{
        TGameMenuItem *prev = pParent;
        while (prev)
        {
                if (prev == Parent)
                        return true;
                else prev = prev->pParent;
        }

        return false;
}

void __fastcall TGameMenuItem::DeleteSubmenu(void)
{
        while (pChild)
                delete pChild; // pChild will be modified by child destructor
}


//---------------------------------------------------------------------------

__fastcall TGameMenu::TGameMenu()
{
        Root = new TGameMenuItem("Root");
        ActiveMenu = NULL;
        MouseMenu = NULL;
        mVisible = true;
        Hide();
}

__fastcall TGameMenu::~TGameMenu()
{
        delete Root;
}

//---------------------------------------------------------------------------

void __fastcall TGameMenu::Create(LPDIRECT3DDEVICE9 device)
{
        Device = device;
        Border.x = Video.MenuFont->GetTextExtent(CHECKBOX).x;
        Border.y = Video.ScreenResolution().y/300;
}

//---------------------------------------------------------------------------

void __fastcall TGameMenu::Show()
{
        if (!mVisible && Root)
        {
                ShowCursor(true);
                mVisible = true;
                ActiveMenu = NULL;
                MouseMenu = NULL;
        }
}

void __fastcall TGameMenu::Hide()
{
        if (mVisible)
        {
                ShowCursor(false);
                mVisible = false;
        }
}


TGameMenuItem* __fastcall TGameMenu::AddMenu(AnsiString Text, TGameMenuItem *Parent, CLICKFUNCTION OnClick, CHECKFUNCTION IsChecked, int Parameter, bool Enabled)
{
        if (!Parent)
                Parent = Root;
        return new TGameMenuItem(Text, Parent, OnClick, IsChecked, Parameter, Enabled);
}

TGameMenuItem* __fastcall TGameMenu::AddSeparator(TGameMenuItem *Parent)
{
        if (!Parent)
                Parent = Root;
        return new TGameMenuItem("", Parent);
}

void __fastcall TGameMenu::Render()
{
        BYTE menukey = GameInput.GetKeyPress(DIK_ESCAPE) || GameInput.GetKeyPress(DIK_RETURN);
        BYTE click = GameInput.GetMouseClickPress();

        // enable/disable menu as appropriate
        if (mVisible)
        {
                // check to disable menu
                if (menukey)
                {
                        Hide();
                        return;
                }
        }
        else
        {
                if (click || menukey)
                        Show();
                return;
        }

        // if we get here, then we must show the menu
        MousePos = MainForm->ScreenToClient(GameInput.MousePos());

        MouseMenu = NULL; // recalculated each time
        DrawMenu(Root->pChild);

        // handle mouse click
        if (click)
        {
                if (!MouseMenu || MouseMenu->Click())
                        Hide();
        }
}

void __fastcall TGameMenu::DrawMenu(TGameMenuItem * menu, TPoint pt, TPoint size)
{
        // end of menu
        if (!menu)
                return;

        // initialize when drawing top level menu
        if (menu == Root)
                ActiveMenu = NULL;

        // force each item in top level menu to have unique size
        if (menu->pParent == Root)
        {
                // find size of the sibling menu item
                size = Video.MenuFont->GetTextExtent(menu->mText);
                size.x += Border.x*2;
                size.y += Border.y*2;
        }

        // take menu measurement if top level
        else if (!menu->pPrevSibling)
        {
                // find maximum text size necessary for all siblings
                TGameMenuItem* sibling = menu;
                do
                {
                        // find size of the sibling menu item
                        POINT s = Video.MenuFont->GetTextExtent(sibling->mText);

                        // find maximum element size
                        size.x = max(size.x,s.x);
                        size.y = max(size.y,s.y);
                        sibling = sibling->pNextSibling;

                } while (sibling);

                // add space for menu border
                size.x += Border.x*2;
                size.y += Border.y*2;
        }

        bool separator = menu->mText == "";

        // locate this item
        RECT rect = {pt.x,pt.y,pt.x+size.x,pt.y+size.y};
        int dy = max((int) Border.y, 1);
        if (separator)
                rect.bottom = pt.y + dy*3;

        // assume this item is not selected
        bool selected = false;

        // determine if mouse is inside the item
        if (!MouseMenu && MousePos.x >= rect.left && MousePos.x <= rect.right && MousePos.y >= rect.top && MousePos.y <= rect.bottom)
        {
                MouseMenu = menu;
                ActiveMenu = (menu->pChild) ? menu->pChild : menu;
                selected = menu->mText != "";
        }

        // determine if this menu item is selected
        if (ActiveMenu && ActiveMenu->IsValidParent(menu))
                selected = true;

        // determine colors to use for this menu item
        #define DEFAULT_TEXT D3DCOLOR_XRGB(255,255,255)
        #define DEFAULT_BACKGROUND  D3DCOLOR_XRGB(0,0,128)
        #define HIGHLIGHT_TEXT D3DCOLOR_XRGB(0,0,128)
        #define HIGHLIGHT_BACKGROUND  D3DCOLOR_XRGB(255,255,255)
        D3DCOLOR textcolor = DEFAULT_TEXT;
        D3DCOLOR backgroundcolor = DEFAULT_BACKGROUND;
        if (selected)
        {
                textcolor = HIGHLIGHT_TEXT;
                backgroundcolor = HIGHLIGHT_BACKGROUND;
        }

        // draw item
        LPDIRECT3DSURFACE9 surface;
        Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface);
        Device->ColorFill(surface, &rect, backgroundcolor);
        if (separator)
        {
                int dx = Border.x/6; dx = max(dx, 1);
                rect.left += dx;
                rect.right -= dx;
                rect.top += dy;
                rect.bottom -= dy;
                Device->ColorFill(surface, &rect, DEFAULT_TEXT);
                pt.y += dy*3 - size.y;
        }
        else
        {
                rect.top += Border.y;
                if (menu->CheckFunction && menu->CheckFunction(menu->mParameter))
                        Video.MenuFont->DrawText(CHECKBOX, rect, textcolor);
                rect.left += Border.x;
                Video.MenuFont->DrawText(menu->mText, rect, textcolor);
                if (menu->pParent != Root && menu->pChild)
                {
                        rect.left += size.x - Border.x*2;
                        Video.MenuFont->DrawText(SUBMENU, rect, textcolor);
                }
        }

        // draw siblings
        TPoint sibling = pt;
        if (menu->pParent == Root)
                sibling.x += size.x; // sibling to the side
        else
                sibling.y += size.y; // sibling is below
        DrawMenu(menu->pNextSibling, sibling, size);

        // draw submenu (if selected)
        if (menu->pChild && selected)
        {
                TPoint child = pt;
                if (menu->pParent == Root)
                        child.y += size.y; // child below
                else
                        child.x += size.x; // child to the side
                DrawMenu(menu->pChild, child);
        }
}

