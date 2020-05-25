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

#include "Config.h"

TConfig Config;

__fastcall TConfig::TConfig()
{
        // load defaults
        #define CONFIG(size, name, read, write, default) name = default;
        CONFIG_DEFINITIONS
        #undef CONFIG

        // load values from registry (if exist)
        TRegistry *registry = new TRegistry;
        registry->OpenKey("Software", true);
        registry->OpenKey("FritoSoft", true);
        registry->OpenKey(CONFIG_KEY, true);
        #define CONFIG(size, name, read, write, default) \
        if (registry->ValueExists(#name)) \
                name = registry->read(#name);
        CONFIG_DEFINITIONS
        #undef CONFIG
        delete registry;
}

__fastcall TConfig::~TConfig()
{
        // save values to registry
        TRegistry *registry = new TRegistry;
        registry->OpenKey("Software", true);
        registry->OpenKey("FritoSoft", true);
        registry->OpenKey(CONFIG_KEY, true);
        #define CONFIG(size, name, read, write, default) registry->write(#name, name);
        CONFIG_DEFINITIONS
        #undef CONFIG
        delete registry;
}

