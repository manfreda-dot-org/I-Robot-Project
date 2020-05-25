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

#ifndef RasterizerH
#define RasterizerH
//---------------------------------------------------------------------------

#include <system.hpp>
#include "DirectX.h"

typedef D3DXMATRIXA16 MATRIX;
typedef D3DXVECTOR3 VECTOR;

inline VECTOR __fastcall operator *(const VECTOR &vector, const MATRIX &matrix)
{
        VECTOR result;
        D3DXVec3TransformCoord(&result, &vector, &matrix);
        return result;
}

inline float __fastcall operator *(const VECTOR &vector1, const VECTOR &vector2)
{
        return D3DXVec3Dot(&vector1, &vector2);
}

class TRasterizer
{
private:
        static void __fastcall Destroy(void);

public:
        __fastcall TRasterizer();

        // video interface
        static bool __fastcall Create(LPDIRECT3DDEVICE9 device);
        static void __fastcall ChangeScreenBuffer(void);
        static void __fastcall EraseScreenBuffer(void);
        static void __fastcall Render(void);

        // render interface
        static void __fastcall StartRender(void);
        static void __fastcall EndRender(void);
        static void __fastcall SetWorldMatrix(const MATRIX &rotation);
        static void __fastcall SetWorldMatrix(const VECTOR &position, const MATRIX &rotation);
        static void __fastcall SetColor(float r, float g, float b);
        static VECTOR * __fastcall LockVertexBuffer(void);
        static void __fastcall UnlockVertexBuffer(int numvertices);
        static void __fastcall Dot(void);
        static void __fastcall Vector(void);
        static void __fastcall Polygon(void);
};

//---------------------------------------------------------------------------
#endif
