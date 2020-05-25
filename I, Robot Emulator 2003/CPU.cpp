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

#include "CPU.h"
#include "Emulator.h"


////////////////////////////////////////////////////////////////////////////////

// GAME CPU INFORMATION

// Game is controlled by a Motorola B6809-E clocked at 1.5 MHz.

// VIDEO CPU INFORMATION

// VIDEO CLOCK
// video clock runs at 5 MHz
// horizontal dot clock counts 0-319
// vertical dot clock counts 0-255

// VIDEO INTERRUPTS
// video hardware generates an INQ interrupt on the 6809.  These
// interrupts occur 4 times per frame, on scan lines 48, 112,
// 176 and 240.  These interrupts requests are latched, and are
// typically cleared by the 6809 program in the IQR interrupt
// service routine.  However, the video hardware will automatically
// clear the IRQ assertion if it is not processed within 32 scan
// lines.  Thus, the IRQ is deasserted on scan lines  80, 144, 208
// and 16 (240 + 32 - 256).

// VBLANK
// the VBLANK signal is active low, and is asserted from
// scan lines 240 - 255.

////////////////////////////////////////////////////////////////////////////////

// Video clocks 320 x 256 pixels per frame, at 5 MHz
#define FRAMES_PER_SECOND (5000000.0/320/256)

// 6809 E clock runs at 1.5MHz
#define CYCLES_PER_FRAME (1500000/FRAMES_PER_SECOND)

// we have to stop emulation every 16 scan lines to handle
// vidoe IRQs and VBLANK toggling
#define SCANLINES_EMULATED 16

// this is the scanline that VBLANK is toggled on
#define VBLANK_SCANLINE 240

#define CYCLES_TO_EMULATE (DWORD) (CYCLES_PER_FRAME/(256/SCANLINES_EMULATED)+0.5)

__fastcall TCPU::TCPU()
{

}

void __fastcall TCPU::Execute(void)
{
        // execute the 6809
        for (int n=0; n<16; n++)
        if (M6809_Run(CYCLES_TO_EMULATE) < 0)
        {
                Video.Scanline += SCANLINES_EMULATED;

                // Toggle IRQ line
                //  IRQ asserted on lines:   48, 112, 176, 240
                //  IRQ deasserted on lines: 16,  80, 144, 208
                switch (Video.Scanline)
                {
                case 48:
                case 112:
                case 176:
                case 240: M6809_IRQ_Pending |= IRQ; break;
                case 16:
                case 80:
                case 144:
                case 208: M6809_IRQ_Pending &= ~IRQ; break;
                }

                // Generate VBLANK
                if (Video.Scanline >= VBLANK_SCANLINE)
                        Hardware.Registers._1080 |= VBLANK;
                else
                        Hardware.Registers._1080 &= ~VBLANK;

        }
        else
        {
                // Display error message if return value is non-zero
                AnsiString s;
                s = "Illegal instruction at address $" + IntToHex(M6809_REG_PC, 4) + " :";
                s += " " + IntToHex(Hardware.Program.ROM[M6809_REG_PC++], 2);
                s += " " + IntToHex(Hardware.Program.ROM[M6809_REG_PC++], 2);
                s += " " + IntToHex(Hardware.Program.ROM[M6809_REG_PC++], 2);
                s += " " + IntToHex(Hardware.Program.ROM[M6809_REG_PC++], 2);
                s += " " + IntToHex(Hardware.Program.ROM[M6809_REG_PC++], 2);
                s += "\n A=" + IntToHex(M6809_REG_A, 2);
                s += " B=" + IntToHex(M6809_REG_B, 2);
                s += " CC=" + IntToHex(M6809_REG_CC, 2);
                s += " DP=" + IntToHex(M6809_REG_DP, 2);
                s += "\n X=" + IntToHex(M6809_REG_X, 4);
                s += " Y=" + IntToHex(M6809_REG_Y, 4);
                s += " S=" + IntToHex(M6809_REG_S, 4);
                s += " U=" + IntToHex(M6809_REG_U, 4);
                Terminate(s);
        }

}

