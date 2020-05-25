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

#ifndef MathboxH
#define MathboxH
//---------------------------------------------------------------------------

#include "DirectX.h"
#include "Rasterizer.h"

class TMathbox : public TRasterizer
{
private:
        static bool __fastcall InitGeometry(void);
        static void __fastcall SetupLights(void);
        static void __fastcall SetupMatrices(void);

        static void __fastcall LoadLightVector(void);
        static void __fastcall LoadViewPosition(void);
        static void __fastcall LoadViewMatrix(int address);
        static void __fastcall LoadRotationMatrix(int address);
        static void __fastcall SetColor(int index);
        static void __fastcall SetColor(int index, float shade);

        // object rendering
        static void __fastcall RasterizeObject(WORD address);
        static void __fastcall ParseObjectList(WORD address);
        static void __fastcall ParseSurfaceList(WORD address);
        static bool __fastcall RenderFace(WORD address, int flags);
        static void __fastcall PrepareVertexBuffer(WORD address);

        // playfield rendering
        static void __fastcall RasterizePlayfield(void);
        static void __fastcall DrawPlayfieldRow(int row, VECTOR corner);
        static void __fastcall DrawPlayfieldTile(int index, VECTOR corner);

public:
        __fastcall TMathbox();

        // memory interface
        static void __fastcall InitIO(void);
        static void __fastcall BankSwitch(void);
        static void __fastcall Execute(void);
};

//---------------------------------------------------------------------------
#endif
