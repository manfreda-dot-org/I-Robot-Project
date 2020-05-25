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

#ifndef EmulatorH
#define EmulatorH
//---------------------------------------------------------------------------

#include <Forms.hpp>
#include "main.h"

#include "Config.h"
#include "Hardware.h"
#include "Input.h"
#include "Log.h"
#include "Menu.h"
#include "Video.h"

class TEmulator
{
private:

public:
        __fastcall TEmulator();
        __fastcall ~TEmulator();

        TForm *Owner;

        bool Paused;

        bool __fastcall Create(TForm* owner);
        bool __fastcall Run();
        void __fastcall Repaint();
        void __fastcall Resize();

        void __fastcall TogglePause();
};

extern TEmulator Emulator;

//---------------------------------------------------------------------------
#endif

