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

#ifndef InputH
#define InputH

#include <Controls.hpp>
#include "DirectX.h"

class TGameInput
{
private:
        int __fastcall GetKeyboardDirection(BYTE pos, BYTE neg);
public:

        __fastcall TGameInput();
        __fastcall ~TGameInput();

        bool __fastcall Create(TForm *form);
        void __fastcall Destroy(void);

        void __fastcall Update(void);

        BYTE __fastcall GetJoystickX(void);
        BYTE __fastcall GetJoystickY(void);

        TPoint __fastcall MousePos(void);
        BYTE __fastcall GetMouseClickState(void);
        BYTE __fastcall GetMouseClickEvent(void);
        BYTE __fastcall GetMouseClickPress(void);
        BYTE __fastcall GetMouseClickRelease(void);

        BYTE __fastcall GetKeyState(BYTE key);
        BYTE __fastcall GetKeyEvent(BYTE key);
        BYTE __fastcall GetKeyPress(BYTE key);
        BYTE __fastcall GetKeyRelease(BYTE key);
};

extern TGameInput GameInput;

//---------------------------------------------------------------------------
#endif
