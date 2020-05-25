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

#include "Log.h"
#include "Emulator.h"

TLog Log;

static struct {
        AnsiString String;
        DWORD Time;
        COLORREF Color;
} Entry[16];

static int Head = 0;
static int Tail = 0;

#define LOG_TIME 5000

//---------------------------------------------------------------------------

void __fastcall TLog::Clear(void)
{
        Head = 0;
        Tail = 0;
}

void __fastcall TLog::Add(AnsiString string, COLORREF color, int delay)
{
        Entry[Tail].String = string;
        Entry[Tail].Color = color;
        int time = timeGetTime();
        if (Head != Tail)
        {
                // check if previous entry is visible yet
                int prevtime = Entry[(Tail-1)&15].Time;
                if ((int)(time - prevtime) < 0)
                {
                        time = prevtime;
                        if (delay < 0)
                                delay = 300;
                }
        }
        if (delay < 0)
                delay = 0;
        Entry[Tail].Time = time + delay;
        Tail = (Tail + 1) & 15;
        if (Tail == Head)
                Head = (Head + 1) & 15;
}

void __fastcall TLog::Render(void)
{
        if (Head == Tail)
                return;

        // handle entry delay, determine the number of visible entries
        DWORD time = timeGetTime();
        int items = 0;
        for (int head = Head; head != Tail; head = (head+1)&15)
        {
                int delay = time - Entry[head].Time;
                if (delay > LOG_TIME+1000)
                        Head = (Head + 1) & 15;
                else if (delay >= 0)
                        items++;
        }
        if (!items)
                return;

        int y = Video.ScreenResolution().y;
        y *= 27.5 / 29.0;
        int dy = Video.Font->GetTextExtent("A").y;
        y -= dy * items;
        int head = Head;
        while (items--)
        {
                BYTE transparency = 0xFF;
                int drawtime = time - Entry[head].Time;
                if (drawtime > LOG_TIME)
                        transparency *= (1.0 - (drawtime-LOG_TIME)/1000.0);
                D3DCOLOR color = D3DCOLOR_RGBA(GetRValue(Entry[head].Color), GetGValue(Entry[head].Color), GetBValue(Entry[head].Color), transparency);

                // draw entry
                Video.Font->DrawText(Entry[head].String, TPoint(0,y), color);

                // goto next entry
                head = (head+1) & 15;

                // update y location
                y += dy;
        }
}



//---------------------------------------------------------------------------

