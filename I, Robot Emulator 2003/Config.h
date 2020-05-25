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

#ifndef ConfigH
#define ConfigH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Menus.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include <ComCtrls.hpp>
#include <Registry.hpp>

//---------------------------------------------------------------------------

#define CONFIG_KEY "I, Robot emulator (mark 2)"

#define CONFIG_DEFINITIONS \
CONFIG(bool, SoundEnabled,       ReadBool,    WriteBool,    true) \
CONFIG(bool, ShowDots,           ReadBool,    WriteBool,    true) \
CONFIG(bool, ShowVectors,        ReadBool,    WriteBool,    true) \
CONFIG(bool, ShowPolygons,        ReadBool,    WriteBool,    true) \
CONFIG(bool, ShowFPS,             ReadBool,    WriteBool,    false) \
CONFIG(bool, Wireframe,           ReadBool,    WriteBool,    false) \
CONFIG(int,  RefreshRate,         ReadInteger, WriteInteger, 60) \
CONFIG(int,  AdapterMode,         ReadInteger, WriteInteger, 0) \
CONFIG(int,  NumAdapterModes,     ReadInteger, WriteInteger, 0) \
CONFIG(bool, Force1x1Scale,       ReadBool,    WriteBool,    true) \
CONFIG(BYTE, OverlayTransparency, ReadInteger, WriteInteger, 255) \
CONFIG(BYTE, Dipswitch3J,         ReadInteger, WriteInteger, 0x00) \
CONFIG(BYTE, Dipswitch5E,         ReadInteger, WriteInteger, 0xFF) \

class TConfig
{
public:
        __fastcall TConfig();
        __fastcall ~TConfig();

        // declare user config values
        #define CONFIG(size, name, read, write, default) size name;
        CONFIG_DEFINITIONS
        #undef CONFIG
};

extern TConfig Config;

//---------------------------------------------------------------------------
#endif

