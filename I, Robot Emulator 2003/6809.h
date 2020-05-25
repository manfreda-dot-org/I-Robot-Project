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

#ifndef _6809H
#define _6809H
//---------------------------------------------------------------------------

#define IRQ 0x02
#define FIRQ 0x20

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllimport) BYTE M6809_REG_A;
__declspec(dllimport) BYTE M6809_REG_B;
__declspec(dllimport) BYTE M6809_REG_DP;
__declspec(dllimport) BYTE M6809_REG_CC;
__declspec(dllimport) WORD M6809_REG_X;
__declspec(dllimport) WORD M6809_REG_Y;
__declspec(dllimport) WORD M6809_REG_U;
__declspec(dllimport) WORD M6809_REG_S;
__declspec(dllimport) WORD M6809_REG_PC;
__declspec(dllimport) DWORD M6809_PageReadPointer[256];
__declspec(dllimport) DWORD M6809_PageReadFunction[256];
__declspec(dllimport) DWORD M6809_PageWritePointer[256];
__declspec(dllimport) DWORD M6809_PageWriteFunction[256];
__declspec(dllimport) DWORD M6809_IRQ_Pending;

__declspec(dllimport) void M6809_Reset(BYTE * Ptr64K);
__declspec(dllimport) int M6809_Run(DWORD CyclesToRun);

#ifdef __cplusplus
}
#endif

//---------------------------------------------------------------------------
#endif
