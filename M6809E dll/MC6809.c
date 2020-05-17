/********************************************
*
* Copyright 2012 by Sean Conner.  All Rights Reserved.
*
* This library is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 3 of the License, or (cpu, at your
* option) any later version.
*
* This library is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
* License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, see <http://www.gnu.org/licenses/>.
*
* Comments, questions and criticisms can be sent to: sean@conman.org
*
*********************************************/

#include <stddef.h>
#include <assert.h>

#include "mc6809.h"

/************************************************************************/

#define MC6809_VECTOR_RSVP      0xFFF0
#define MC6809_VECTOR_SWI3      0xFFF2
#define MC6809_VECTOR_SWI2      0xFFF4
#define MC6809_VECTOR_FIRQ      0xFFF6
#define MC6809_VECTOR_IRQ       0xFFF8
#define MC6809_VECTOR_SWI       0xFFFA
#define MC6809_VECTOR_NMI       0xFFFC
#define MC6809_VECTOR_RESET     0xFFFE

/************************************************************************/

static uint8 READ(mc6809__t* cpu, uint16 address)
{
    uint8 page = address >> 8;
    if (cpu->ReadPointer[page])
        return cpu->ReadPointer[page][address & 0xFF];
    if (cpu->ReadFunction[page])
        return cpu->ReadFunction[page](address);
    cpu->Fault = MC6809_FAULT_UNDEFINED_READ;
    return 0;
}

static void WRITE(mc6809__t* cpu, uint16 address, uint8 data)
{
    uint8 page = address >> 8;
    if (cpu->WritePointer[page])
        cpu->WritePointer[page][address & 0xFF] = data;
    else if (cpu->WriteFunction[page])
        cpu->WriteFunction[page](address, data);
    else
        cpu->Fault = MC6809_FAULT_UNDEFINED_WRITE;
}

/************************************************************************/

#define bhs(cpu) (!cpu->CC_C )
#define blo(cpu) (cpu->CC_C )
#define bhi(cpu) (!cpu->CC_C && !cpu->CC_Z )
#define bls(cpu) (cpu->CC_C || cpu->CC_Z )
#define bne(cpu) (!cpu->CC_Z )
#define beq(cpu) (cpu->CC_Z )
#define bge(cpu) (cpu->CC_N == cpu->CC_V )
#define blt(cpu) (cpu->CC_N != cpu->CC_V )
#define bgt(cpu) ((cpu->CC_N == cpu->CC_V) && !cpu->CC_Z )
#define ble(cpu) (cpu->CC_Z || (cpu->CC_N != cpu->CC_V) )
#define bpl(cpu) (!cpu->CC_N )
#define bmi(cpu) (cpu->CC_N )
#define bvc(cpu) (!cpu->CC_V )
#define bvs(cpu) (cpu->CC_V )

/**************************************************************************/

static uint16 mc6809_direct(mc6809__t* cpu)
{
    WORD ea;

    ea.b[MSB] = cpu->DP;
    ea.b[LSB] = READ(cpu, cpu->PC.w++);
    return ea.w;
}

/***********************************************************************/

static uint16 mc6809_relative(mc6809__t* cpu)
{
    WORD ea;

    ea.b[LSB] = READ(cpu, cpu->PC.w++);
    ea.b[MSB] = (ea.b[LSB] > 0x7F) ? 0xFF : 0x00;
    ea.w += cpu->PC.w;
    return ea.w;
}

/***********************************************************************/

static uint16 mc6809_lrelative(mc6809__t* cpu)
{
    WORD ea;

    ea.b[MSB] = READ(cpu, cpu->PC.w++);
    ea.b[LSB] = READ(cpu, cpu->PC.w++);
    ea.w += cpu->PC.w;
    return ea.w;
}

/*********************************************************************/

static uint16 mc6809_extended(mc6809__t* cpu)
{
    WORD ea;

    ea.b[MSB] = READ(cpu, cpu->PC.w++);
    ea.b[LSB] = READ(cpu, cpu->PC.w++);
    return ea.w;
}

/***********************************************************************/

static uint16 mc6809_indexed(mc6809__t* cpu)
{
    WORD ea;
    WORD d16;

    uint8 mode = READ(cpu, cpu->PC.w++);
    int reg = (mode >> 5) & 3;
    int off = (mode & 0x1F);

    if (mode < 0x80)
    {
        cpu->Clock++;
        ea.w = (uint16)off;
        if (ea.w > 15) ea.w |= 0xFFE0;
        ea.w += cpu->Index[reg].w;
        return ea.w;
    }

    switch (off)
    {
    case 0x00:
        cpu->Clock += 2;
        ea.w = cpu->Index[reg].w++;
        return ea.w;

    case 0x01:
        cpu->Clock += 3;
        ea.w = cpu->Index[reg].w;
        cpu->Index[reg].w += 2;
        return ea.w;

    case 0x02:
        cpu->Clock += 2;
        ea.w = --cpu->Index[reg].w;
        return ea.w;

    case 0x03:
        cpu->Clock += 3;
        cpu->Index[reg].w -= 2;
        ea.w = cpu->Index[reg].w;
        return ea.w;

    case 0x04:
        ea.w = cpu->Index[reg].w;
        return ea.w;

    case 0x05:
        cpu->Clock++;
        ea.b[LSB] = cpu->B;
        ea.b[MSB] = (cpu->B < 0x80) ? 0x00 : 0xFF;
        ea.w += cpu->Index[reg].w;
        return ea.w;

    case 0x06:
        cpu->Clock++;
        ea.b[LSB] = cpu->A;
        ea.b[MSB] = (cpu->A < 0x80) ? 0x00 : 0xFF;
        ea.w += cpu->Index[reg].w;
        return ea.w;

    case 0x07:
        cpu->Fault = MC6809_FAULT_ADDRESS_MODE;
        return 0xFFFF;

    case 0x08:
        cpu->Clock++;
        ea.b[LSB] = READ(cpu, cpu->PC.w++);
        ea.b[MSB] = (ea.b[LSB] < 0x80) ? 0x00 : 0xFF;
        ea.w += cpu->Index[reg].w;
        return ea.w;

    case 0x09:
        cpu->Clock += 4;
        ea.b[MSB] = READ(cpu, cpu->PC.w++);
        ea.b[LSB] = READ(cpu, cpu->PC.w++);
        ea.w += cpu->Index[reg].w;
        return ea.w;

    case 0x0A:
        cpu->Fault = MC6809_FAULT_ADDRESS_MODE;
        return 0xFFFF;

    case 0x0B:
        cpu->Clock += 4;
        ea.w = cpu->Index[reg].w + cpu->D.w;
        return ea.w;

    case 0x0C:
        cpu->Clock++;
        ea.b[LSB] = READ(cpu, cpu->PC.w++);
        ea.b[MSB] = (ea.b[LSB] < 0x80) ? 0x00 : 0xFF;
        ea.w += cpu->PC.w;
        return ea.w;

    case 0x0D:
        cpu->Clock += 5;
        ea.b[MSB] = READ(cpu, cpu->PC.w++);
        ea.b[LSB] = READ(cpu, cpu->PC.w++);
        ea.w += cpu->PC.w;
        return ea.w;

    case 0x0E:
        cpu->Fault = MC6809_FAULT_ADDRESS_MODE;
        return 0xFFFF;

    case 0x0F:
        cpu->Fault = MC6809_FAULT_ADDRESS_MODE;
        return 0xFFFF;

    case 0x10:
        cpu->Fault = MC6809_FAULT_ADDRESS_MODE;
        return 0xFFFF;

    case 0x11:
        cpu->Clock += 6;
        ea.w = cpu->Index[reg].w;
        cpu->Index[reg].w += 2;
        d16.b[MSB] = READ(cpu, ea.w++);
        d16.b[LSB] = READ(cpu, ea.w);
        ea.w = d16.w;
        return ea.w;

    case 0x12:
        cpu->Fault = MC6809_FAULT_ADDRESS_MODE;
        return 0xFFFF;

    case 0x13:
        cpu->Clock += 6;
        cpu->Index[reg].w -= 2;
        ea.b[MSB] = READ(cpu, cpu->Index[reg].w);
        ea.b[LSB] = READ(cpu, (uint16)(cpu->Index[reg].w + 1));
        return ea.w;

    case 0x14:
        cpu->Clock += 3;
        ea.b[MSB] = READ(cpu, cpu->Index[reg].w);
        ea.b[LSB] = READ(cpu, (uint16)(cpu->Index[reg].w + 1));
        return ea.w;

    case 0x15:
        cpu->Clock += 4;
        ea.b[LSB] = cpu->B;
        ea.b[MSB] = (cpu->B < 0x80) ? 0x00 : 0xFF;
        ea.w += cpu->Index[reg].w;
        d16.b[MSB] = READ(cpu, ea.w++);
        d16.b[LSB] = READ(cpu, ea.w);
        ea.w = d16.w;
        return ea.w;

    case 0x16:
        cpu->Clock += 4;
        ea.b[LSB] = cpu->A;
        ea.b[MSB] = (cpu->A < 0x80) ? 0x00 : 0xFF;
        ea.w += cpu->Index[reg].w;
        d16.b[MSB] = READ(cpu, ea.w++);
        d16.b[LSB] = READ(cpu, ea.w);
        ea.w = d16.w;
        return ea.w;

    case 0x17:
        cpu->Fault = MC6809_FAULT_ADDRESS_MODE;
        return 0xFFFF;

    case 0x18:
        cpu->Clock += 4;
        ea.b[LSB] = READ(cpu, cpu->PC.w++);
        ea.b[MSB] = (ea.b[LSB] < 0x80) ? 0x00 : 0xFF;
        ea.w += cpu->Index[reg].w;
        d16.b[MSB] = READ(cpu, ea.w++);
        d16.b[LSB] = READ(cpu, ea.w);
        ea.w = d16.w;
        return ea.w;

    case 0x19:
        cpu->Clock += 7;
        ea.b[MSB] = READ(cpu, cpu->PC.w++);
        ea.b[LSB] = READ(cpu, cpu->PC.w++);
        ea.w += cpu->Index[reg].w;
        d16.b[MSB] = READ(cpu, ea.w++);
        d16.b[LSB] = READ(cpu, ea.w);
        ea.w = d16.w;
        return ea.w;

    case 0x1A:
        cpu->Fault = MC6809_FAULT_ADDRESS_MODE;
        return 0xFFFF;

    case 0x1B:
        cpu->Clock += 7;
        ea.w = cpu->D.w;
        ea.w += cpu->Index[reg].w;
        d16.b[MSB] = READ(cpu, ea.w++);
        d16.b[LSB] = READ(cpu, ea.w);
        ea.w = d16.w;
        return ea.w;

    case 0x1C:
        cpu->Clock += 4;
        ea.b[LSB] = READ(cpu, cpu->PC.w++);
        ea.b[MSB] = (ea.b[LSB] < 0x80) ? 0x00 : 0xFF;
        ea.w += cpu->PC.w;
        d16.b[MSB] = READ(cpu, ea.w++);
        d16.b[LSB] = READ(cpu, ea.w);
        ea.w = d16.w;
        return ea.w;

    case 0x1D:
        cpu->Clock += 8;
        ea.b[MSB] = READ(cpu, cpu->PC.w++);
        ea.b[LSB] = READ(cpu, cpu->PC.w++);
        ea.w += cpu->PC.w;
        d16.b[MSB] = READ(cpu, ea.w++);
        d16.b[LSB] = READ(cpu, ea.w);
        ea.w = d16.w;
        return ea.w;

    case 0x1E:
        cpu->Fault = MC6809_FAULT_ADDRESS_MODE;
        return 0xFFFF;

    case 0x1F:
        if (reg == 0)
        {
            cpu->Clock += 5;
            ea.b[MSB] = READ(cpu, cpu->PC.w++);
            ea.b[LSB] = READ(cpu, cpu->PC.w++);
            d16.b[MSB] = READ(cpu, ea.w++);
            d16.b[LSB] = READ(cpu, ea.w);
            ea.w = d16.w;
            return ea.w;
        }
        cpu->Fault = MC6809_FAULT_ADDRESS_MODE;
        return 0xFFFF;

    default:
        cpu->Fault = MC6809_FAULT_INTERNAL_ERROR;
        return 0xFFFF;
    }
}

/*************************************************************************/

__declspec(dllexport) uint8 __stdcall Get_6809_CC(mc6809__t* cpu)
{
    uint8 r = 0;

    if (cpu->CC_C) r |= 0x01;
    if (cpu->CC_V) r |= 0x02;
    if (cpu->CC_Z) r |= 0x04;
    if (cpu->CC_N) r |= 0x08;
    if (cpu->CC_I) r |= 0x10;
    if (cpu->CC_H) r |= 0x20;
    if (cpu->CC_F) r |= 0x40;
    if (cpu->CC_E) r |= 0x80;

    return r;
}

__declspec(dllexport) void __stdcall Set_6809_CC(mc6809__t* cpu, uint8 const r)
{
    cpu->CC_C = ((r & 0x01) != 0);
    cpu->CC_V = ((r & 0x02) != 0);
    cpu->CC_Z = ((r & 0x04) != 0);
    cpu->CC_N = ((r & 0x08) != 0);
    cpu->CC_I = ((r & 0x10) != 0);
    cpu->CC_H = ((r & 0x20) != 0);
    cpu->CC_F = ((r & 0x40) != 0);
    cpu->CC_E = ((r & 0x80) != 0);
}

/*************************************************************************/

static uint8 op_neg(mc6809__t* cpu, uint8 const src)
{
    uint8 res;

    res = -src;
    cpu->CC_N = (res > 0x7F);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_V = (src == 0x80);
    cpu->CC_C = (src > 0x7F);
    return res;
}

/*************************************************************************/

static uint8 op_com(mc6809__t* cpu, uint8 const src)
{
    uint8 res;

    res = ~src;
    cpu->CC_N = (res > 0x7F);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_V = false;
    cpu->CC_C = true;
    return res;
}

/*************************************************************************/

static uint8 op_lsr(mc6809__t* cpu, uint8 const src)
{
    uint8 res;

    res = src >> 1;
    cpu->CC_N = false;
    cpu->CC_Z = (res == 0x00);
    cpu->CC_C = (src & 0x01) == 0x01;
    return res;
}

/*************************************************************************/

static uint8 op_ror(mc6809__t* cpu, uint8 const src)
{
    uint8 res;

    res = src >> 1;
    res |= cpu->CC_C ? 0x80 : 0x00;
    cpu->CC_N = (res > 0x7F);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_C = (src & 0x01) == 0x01;
    return res;
}

/*************************************************************************/

static uint8 op_asr(mc6809__t* cpu, uint8 const src)
{
    uint8 res;

    res = src >> 1;
    res |= (src > 0x7F) ? 0x80 : 0x00;
    cpu->CC_N = (res > 0x7F);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_C = (src & 0x01) == 0x01;
    return res;
}

/*************************************************************************/

static uint8 op_lsl(mc6809__t* cpu, uint8 const src)
{
    uint8 res;

    res = src << 1;
    cpu->CC_N = (res > 0x7F);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_V = ((src ^ res) & 0x80) == 0x80;
    cpu->CC_C = (src > 0x7F);
    return res;
}

/************************************************************************/

static uint8 op_rol(mc6809__t* cpu, uint8 const src)
{
    uint8 res;

    res = src << 1;
    res |= cpu->CC_C ? 0x01 : 0x00;
    cpu->CC_N = (res > 0x7F);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_V = ((src ^ res) & 0x80) == 0x80;
    cpu->CC_C = (src > 0x7F);
    return res;
}

/************************************************************************/

static uint8 op_dec(mc6809__t* cpu, uint8 const src)
{
    uint8 res;

    res = src - 1;
    cpu->CC_N = (res > 0x7F);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_V = (src == 0x80);
    return res;
}

/************************************************************************/

static uint8 op_inc(mc6809__t* cpu, uint8 const src)
{
    uint8 res;

    res = src + 1;
    cpu->CC_N = (res > 0x7F);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_V = (src == 0x7F);
    return res;
}

/************************************************************************/

static void op_tst(mc6809__t* cpu, uint8 const src)
{
    cpu->CC_N = (src > 0x7F);
    cpu->CC_Z = (src == 0x00);
    cpu->CC_V = false;
}

/************************************************************************/

static uint8 op_clr(mc6809__t* cpu)
{
    cpu->CC_N = false;
    cpu->CC_Z = true;
    cpu->CC_V = false;
    cpu->CC_C = false;
    return 0;
}

/************************************************************************/

static uint8 op_sub(mc6809__t* cpu, uint8 const  dest, uint8 const  src)
{
    uint8 res;
    uint8 ci;

    res = dest - src;
    ci = res ^ dest ^ src;
    cpu->CC_N = (res > 0x7F);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_C = src > dest;
    cpu->CC_V = ((ci & 0x80) != 0) ^ cpu->CC_C;
    return res;
}

/************************************************************************/

static void op_cmp(mc6809__t* cpu, uint8 const  dest, uint8 const  src)
{
    uint8 res;
    uint8 ci;

    res = dest - src;
    ci = res ^ dest ^ src;
    cpu->CC_N = (res > 0x7F);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_C = src > dest;
    cpu->CC_V = ((ci & 0x80) != 0) ^ cpu->CC_C;
}

/************************************************************************/

static uint8 op_sbc(mc6809__t* cpu, uint8 const  dest, uint8 const  src)
{
    uint8 res;
    uint8 ci;

    assert(cpu->CC_C <= 1);

    res = dest - src - cpu->CC_C;
    ci = res ^ dest ^ src;
    cpu->CC_N = (res > 0x7F);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_C = src >= dest;
    cpu->CC_V = ((ci & 0x80) != 0) ^ cpu->CC_C;
    return res;
}

/************************************************************************/

static uint8 op_and(mc6809__t* cpu, uint8 const  dest, uint8 const  src)
{
    uint8 res;

    res = dest & src;
    cpu->CC_N = (res > 0x7F);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_V = false;
    return res;
}

/************************************************************************/

static void op_bit(mc6809__t* cpu, uint8 const  dest, uint8 const  src)
{
    uint8 res;

    res = dest & src;
    cpu->CC_N = (res > 0x7F);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_V = false;
}

/************************************************************************/

static void op_ldst(mc6809__t* cpu, uint8 const  dest)
{
    cpu->CC_N = (dest > 0x7F);
    cpu->CC_Z = (dest == 0x00);
    cpu->CC_V = false;
}

/************************************************************************/

static uint8 op_eor(mc6809__t* cpu, uint8 const  dest, uint8 const  src)
{
    uint8 res;

    res = dest ^ src;
    cpu->CC_N = (res > 0x7F);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_V = false;
    return res;
}

/************************************************************************/

static uint8 op_adc(mc6809__t* cpu, uint8 const  dest, uint8 const  src)
{
    uint8 res;
    uint8 ci;

    assert(cpu->CC_C <= 1);

    res = dest + src + cpu->CC_C;
    ci = res ^ dest ^ src;
    cpu->CC_H = (ci & 0x10);
    cpu->CC_N = (res > 0x7F);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_C = (res < dest) || (res < src);
    cpu->CC_V = ((ci & 0x80) != 0) ^ cpu->CC_C;
    return res;
}

/************************************************************************/

static uint8 op_or(mc6809__t* cpu, uint8 const  dest, uint8 const  src)
{
    uint8 res = dest | src;
    cpu->CC_N = (res > 0x7f);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_V = false;
    return res;
}

/************************************************************************/

static uint8 op_add(mc6809__t* cpu, uint8 const  dest, uint8 const  src)
{
    uint8 res = dest + src;
    uint8 ci = res ^ dest ^ src;
    cpu->CC_H = (ci & 0x10);
    cpu->CC_N = (res > 0x7F);
    cpu->CC_Z = (res == 0x00);
    cpu->CC_C = (res < dest) || (res < src);
    cpu->CC_V = ((ci & 0x80) != 0) ^ cpu->CC_C;
    return res;
}

/************************************************************************/

static uint16 op_sub16(mc6809__t* cpu, uint16 const  dest, uint16 const  src)
{
    uint16 res = dest - src;
    uint16 ci = res ^ dest ^ src;
    cpu->CC_N = (res > 0x7FFF);
    cpu->CC_Z = (res == 0x0000);
    cpu->CC_C = src > dest;
    cpu->CC_V = ((ci & 0x8000) != 0) ^ cpu->CC_C;
    return res;
}

/************************************************************************/

static void op_cmp16(mc6809__t* cpu, uint16 const  dest, uint16 const  src)
{
    uint16 res = dest - src;
    uint16 ci = res ^ dest ^ src;
    cpu->CC_N = (res > 0x7FFF);
    cpu->CC_Z = (res == 0x0000);
    cpu->CC_C = src > dest;
    cpu->CC_V = ((ci & 0x8000) != 0) ^ cpu->CC_C;
}

/************************************************************************/

static void op_ldst16(mc6809__t* cpu, uint16 const  data)
{
    cpu->CC_N = (data > 0x7FFF);
    cpu->CC_Z = (data == 0x0000);
    cpu->CC_V = false;
}

/************************************************************************/

static uint16 op_add16(mc6809__t* cpu, uint16 const  dest, uint16 const  src)
{
    uint16 res = dest + src;
    uint16 ci = res ^ dest ^ src;
    cpu->CC_N = (res > 0x7FFF);
    cpu->CC_Z = (res == 0x0000);
    cpu->CC_C = (res < dest) || (res < src);
    cpu->CC_V = ((ci & 0x8000) != 0) ^ cpu->CC_C;
    return res;
}

/**************************************************************************/

void mc6809_reset(mc6809__t* cpu)
{
    cpu->Clock = 0;
    cpu->Fault = MC6809_RESULT_OK;
    cpu->NMI_Armed = false;
    cpu->NMI = false;
    cpu->FIRQ = false;
    cpu->IRQ = false;
    cpu->CWAI = false;
    cpu->SYNC = false;
    cpu->PC.b[MSB] = READ(cpu, MC6809_VECTOR_RESET);
    cpu->PC.b[LSB] = READ(cpu, MC6809_VECTOR_RESET + 1);
    cpu->DP = 0;
    cpu->CC_E = false;
    cpu->CC_F = true;
    cpu->CC_H = false;
    cpu->CC_I = true;
    cpu->CC_N = false;
    cpu->CC_Z = false;
    cpu->CC_V = false;
    cpu->CC_C = false;
}

/************************************************************************/

static void page2(mc6809__t* cpu)
{
    uint16 ea;
    WORD d16;

    uint8 inst = READ(cpu, cpu->PC.w++);

    /*-----------------------------------------------------------------------
    ; While the cycle counts may appear to be one less than stated in the
    ; manual, the addtional cycle has already been calculated via the
    ; mc6809_step() function (cpu, the same applies for the page3() routine).
    ;-----------------------------------------------------------------------*/

    switch (inst)
    {
    case 0x21: // LBRN
        cpu->Clock += 4;
        mc6809_lrelative(cpu);
        break;

    case 0x22: // LBHI
        cpu->Clock += 4;
        ea = mc6809_lrelative(cpu);
        if (bhi(cpu))
        {
            cpu->Clock++;
            cpu->PC.w = ea;
        }
        break;

    case 0x23: // LBLS
        cpu->Clock += 4;
        ea = mc6809_lrelative(cpu);
        if (bls(cpu))
        {
            cpu->Clock++;
            cpu->PC.w = ea;
        }
        break;

    case 0x24: // LBHS, LBCC
        cpu->Clock += 4;
        ea = mc6809_lrelative(cpu);
        if (bhs(cpu))
        {
            cpu->Clock++;
            cpu->PC.w = ea;
        }
        break;

    case 0x25: // LBLO, LBCS
        cpu->Clock += 4;
        ea = mc6809_lrelative(cpu);
        if (blo(cpu))
        {
            cpu->Clock++;
            cpu->PC.w = ea;
        }
        break;

    case 0x26: // LBNE
        cpu->Clock += 4;
        ea = mc6809_lrelative(cpu);
        if (bne(cpu))
        {
            cpu->Clock++;
            cpu->PC.w = ea;
        }
        break;

    case 0x27: // LBEQ
        cpu->Clock += 4;
        ea = mc6809_lrelative(cpu);
        if (beq(cpu))
        {
            cpu->Clock++;
            cpu->PC.w = ea;
        }
        break;

    case 0x28: // LBVC
        cpu->Clock += 4;
        ea = mc6809_lrelative(cpu);
        if (bvc(cpu))
        {
            cpu->Clock++;
            cpu->PC.w = ea;
        }
        break;

    case 0x29: // LBVS
        cpu->Clock += 4;
        ea = mc6809_lrelative(cpu);
        if (bvs(cpu))
        {
            cpu->Clock++;
            cpu->PC.w = ea;
        }
        break;

    case 0x2A: // LBPL
        cpu->Clock += 4;
        ea = mc6809_lrelative(cpu);
        if (bpl(cpu))
        {
            cpu->Clock++;
            cpu->PC.w = ea;
        }
        break;

    case 0x2B: // LBMI
        cpu->Clock += 4;
        ea = mc6809_lrelative(cpu);
        if (bmi(cpu))
        {
            cpu->Clock++;
            cpu->PC.w = ea;
        }
        break;

    case 0x2C: // LBGE
        cpu->Clock += 4;
        ea = mc6809_lrelative(cpu);
        if (bge(cpu))
        {
            cpu->Clock++;
            cpu->PC.w = ea;
        }
        break;

    case 0x2D: // LBLT
        cpu->Clock += 4;
        ea = mc6809_lrelative(cpu);
        if (blt(cpu))
        {
            cpu->Clock++;
            cpu->PC.w = ea;
        }
        break;

    case 0x2E: // LBGT
        cpu->Clock += 4;
        ea = mc6809_lrelative(cpu);
        if (bgt(cpu))
        {
            cpu->Clock++;
            cpu->PC.w = ea;
        }
        break;

    case 0x2F: // LBLE
        cpu->Clock += 4;
        ea = mc6809_lrelative(cpu);
        if (ble(cpu))
        {
            cpu->Clock++;
            cpu->PC.w = ea;
        }
        break;

    case 0x3F: // SWI2
        cpu->Clock += 19;
        cpu->CC_E = true;
        WRITE(cpu, --cpu->S.w, cpu->PC.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->PC.b[MSB]);
        WRITE(cpu, --cpu->S.w, cpu->U.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->U.b[MSB]);
        WRITE(cpu, --cpu->S.w, cpu->Y.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->Y.b[MSB]);
        WRITE(cpu, --cpu->S.w, cpu->X.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->X.b[MSB]);
        WRITE(cpu, --cpu->S.w, cpu->DP);
        WRITE(cpu, --cpu->S.w, cpu->B);
        WRITE(cpu, --cpu->S.w, cpu->A);
        WRITE(cpu, --cpu->S.w, Get_6809_CC(cpu));
        cpu->PC.b[MSB] = READ(cpu, MC6809_VECTOR_SWI2);
        cpu->PC.b[LSB] = READ(cpu, MC6809_VECTOR_SWI2 + 1);
        break;

    case 0x83: // CMPD
        cpu->Clock += 4;
        d16.b[MSB] = READ(cpu, cpu->PC.w++);
        d16.b[LSB] = READ(cpu, cpu->PC.w++);
        op_cmp16(cpu, cpu->D.w, d16.w);
        break;

    case 0x8C: // CMPY
        cpu->Clock += 4;
        d16.b[MSB] = READ(cpu, cpu->PC.w++);
        d16.b[LSB] = READ(cpu, cpu->PC.w++);
        op_cmp16(cpu, cpu->Y.w, d16.w);
        break;

    case 0x8E: // LDY
        cpu->Clock += 3;
        cpu->Y.b[MSB] = READ(cpu, cpu->PC.w++);
        cpu->Y.b[LSB] = READ(cpu, cpu->PC.w++);
        op_ldst16(cpu, cpu->Y.w);
        break;

    case 0x93: // CMPD
        cpu->Clock += 6;
        ea = mc6809_direct(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        op_cmp16(cpu, cpu->D.w, d16.w);
        break;

    case 0x9C: // CMPY
        cpu->Clock += 6;
        ea = mc6809_direct(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        op_cmp16(cpu, cpu->Y.w, d16.w);
        break;

    case 0x9E: // LDY
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        cpu->Y.b[MSB] = READ(cpu, ea++);
        cpu->Y.b[LSB] = READ(cpu, ea);
        op_ldst16(cpu, cpu->Y.w);
        break;

    case 0x9F: // STY
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea++, cpu->Y.b[MSB]);
        WRITE(cpu, ea, cpu->Y.b[LSB]);
        op_ldst16(cpu, cpu->Y.w);
        break;

    case 0xA3: // CMPD
        cpu->Clock += 6;
        ea = mc6809_indexed(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        op_cmp16(cpu, cpu->D.w, d16.w);
        break;

    case 0xAC: // CMPY
        cpu->Clock += 6;
        ea = mc6809_indexed(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        op_cmp16(cpu, cpu->Y.w, d16.w);
        break;

    case 0xAE: // LDY
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        cpu->Y.b[MSB] = READ(cpu, ea++);
        cpu->Y.b[LSB] = READ(cpu, ea);
        op_ldst16(cpu, cpu->Y.w);
        break;

    case 0xAF: // STY
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea++, cpu->Y.b[MSB]);
        WRITE(cpu, ea, cpu->Y.b[LSB]);
        op_ldst16(cpu, cpu->Y.w);
        break;

    case 0xB3: // CMPD
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        op_cmp16(cpu, cpu->D.w, d16.w);
        break;

    case 0xBC: // CMPY
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        op_cmp16(cpu, cpu->Y.w, d16.w);
        break;

    case 0xBE: // LDY
        cpu->Clock += 5;
        ea = mc6809_extended(cpu);
        cpu->Y.b[MSB] = READ(cpu, ea++);
        cpu->Y.b[LSB] = READ(cpu, ea);
        op_ldst16(cpu, cpu->Y.w);
        break;

    case 0xBF: // STY
        cpu->Clock += 5;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea++, cpu->Y.b[MSB]);
        WRITE(cpu, ea, cpu->Y.b[LSB]);
        op_ldst16(cpu, cpu->Y.w);
        break;

    case 0xCE: // LDS
        cpu->Clock += 3;
        cpu->S.b[MSB] = READ(cpu, cpu->PC.w++);
        cpu->S.b[LSB] = READ(cpu, cpu->PC.w++);
        op_ldst16(cpu, cpu->S.w);
        cpu->NMI_Armed = true;
        break;

    case 0xDE: // LDS
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        cpu->S.b[MSB] = READ(cpu, ea++);
        cpu->S.b[LSB] = READ(cpu, ea);
        op_ldst16(cpu, cpu->S.w);
        cpu->NMI_Armed = true;
        break;

    case 0xDF: // STS
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea++, cpu->S.b[MSB]);
        WRITE(cpu, ea, cpu->S.b[LSB]);
        op_ldst16(cpu, cpu->S.w);
        break;

    case 0xEE: // LDS
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        cpu->S.b[MSB] = READ(cpu, ea++);
        cpu->S.b[LSB] = READ(cpu, ea);
        op_ldst16(cpu, cpu->S.w);
        cpu->NMI_Armed = true;
        break;

    case 0xEF: // STS
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea++, cpu->S.b[MSB]);
        WRITE(cpu, ea, cpu->S.b[LSB]);
        op_ldst16(cpu, cpu->S.w);
        break;

    case 0xFE: // LDS
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        cpu->S.b[MSB] = READ(cpu, ea++);
        cpu->S.b[LSB] = READ(cpu, ea);
        op_ldst16(cpu, cpu->S.w);
        cpu->NMI_Armed = true;
        break;

    case 0xFF: // STS
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea++, cpu->S.b[MSB]);
        WRITE(cpu, ea, cpu->S.b[LSB]);
        op_ldst16(cpu, cpu->S.w);
        break;

    default:
        cpu->Clock++;
        cpu->Fault = MC6809_FAULT_INSTRUCTION;
    }
}

/************************************************************************/

static void page3(mc6809__t* cpu)
{
    uint16 ea;
    WORD d16;

    uint8 inst = READ(cpu, cpu->PC.w++);

    /*-----------------------------------------------------------------
    ; see the comment in mc6809_step(cpu) for an explanation on cycle counts
    ;------------------------------------------------------------------*/

    switch (inst)
    {
    case 0x3F: // SWI3
        cpu->Clock += 19;
        cpu->CC_E = true;
        WRITE(cpu, --cpu->S.w, cpu->PC.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->PC.b[MSB]);
        WRITE(cpu, --cpu->S.w, cpu->U.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->U.b[MSB]);
        WRITE(cpu, --cpu->S.w, cpu->Y.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->Y.b[MSB]);
        WRITE(cpu, --cpu->S.w, cpu->X.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->X.b[MSB]);
        WRITE(cpu, --cpu->S.w, cpu->DP);
        WRITE(cpu, --cpu->S.w, cpu->B);
        WRITE(cpu, --cpu->S.w, cpu->A);
        WRITE(cpu, --cpu->S.w, Get_6809_CC(cpu));
        cpu->PC.b[MSB] = READ(cpu, MC6809_VECTOR_SWI3);
        cpu->PC.b[LSB] = READ(cpu, MC6809_VECTOR_SWI3 + 1);
        break;

    case 0x83: // CMPU
        cpu->Clock += 4;
        d16.b[MSB] = READ(cpu, cpu->PC.w++);
        d16.b[LSB] = READ(cpu, cpu->PC.w++);
        op_cmp16(cpu, cpu->U.w, d16.w);
        break;

    case 0x8C: // CMPS
        cpu->Clock += 4;
        d16.b[MSB] = READ(cpu, cpu->PC.w++);
        d16.b[LSB] = READ(cpu, cpu->PC.w++);
        op_cmp16(cpu, cpu->S.w, d16.w);
        break;

    case 0x93: // CMPU
        cpu->Clock += 6;
        ea = mc6809_direct(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        op_cmp16(cpu, cpu->U.w, d16.w);
        break;

    case 0x9C: // CMPS
        cpu->Clock += 6;
        ea = mc6809_direct(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        op_cmp16(cpu, cpu->S.w, d16.w);
        break;

    case 0xA3: // CMPU
        cpu->Clock += 6;
        ea = mc6809_indexed(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        op_cmp16(cpu, cpu->U.w, d16.w);
        break;

    case 0xAC: // CMPS
        cpu->Clock += 6;
        ea = mc6809_indexed(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        op_cmp16(cpu, cpu->S.w, d16.w);
        break;

    case 0xB3: // CMPU
        cpu->Clock += 7;
        ea = mc6809_extended(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        op_cmp16(cpu, cpu->U.w, d16.w);
        break;

    case 0xBC: // CMPS
        cpu->Clock += 7;
        ea = mc6809_extended(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        op_cmp16(cpu, cpu->S.w, d16.w);
        break;

    default:
        cpu->Clock++;
        cpu->Fault = MC6809_FAULT_INSTRUCTION;
        break;
    }
}

/**************************************************************************/

MC6809_RESULT mc6809_step(mc6809__t* cpu)
{
    cpu->Fault = 0;

    // check for NMI
    if (cpu->NMI)
    {
        cpu->SYNC = false;
        if (cpu->NMI_Armed)
        {
            cpu->NMI = false;
            if (!cpu->CWAI)
            {
                cpu->Clock += 19;
                cpu->CC_E = true;
                WRITE(cpu, --cpu->S.w, cpu->PC.b[LSB]);
                WRITE(cpu, --cpu->S.w, cpu->PC.b[MSB]);
                WRITE(cpu, --cpu->S.w, cpu->U.b[LSB]);
                WRITE(cpu, --cpu->S.w, cpu->U.b[MSB]);
                WRITE(cpu, --cpu->S.w, cpu->Y.b[LSB]);
                WRITE(cpu, --cpu->S.w, cpu->Y.b[MSB]);
                WRITE(cpu, --cpu->S.w, cpu->X.b[LSB]);
                WRITE(cpu, --cpu->S.w, cpu->X.b[MSB]);
                WRITE(cpu, --cpu->S.w, cpu->DP);
                WRITE(cpu, --cpu->S.w, cpu->B);
                WRITE(cpu, --cpu->S.w, cpu->A);
                WRITE(cpu, --cpu->S.w, Get_6809_CC(cpu));
            }
            cpu->CC_F = true;
            cpu->CC_I = true;
            cpu->PC.b[MSB] = READ(cpu, MC6809_VECTOR_NMI);
            cpu->PC.b[LSB] = READ(cpu, MC6809_VECTOR_NMI + 1);
            cpu->CWAI = false;
            return cpu->Fault;
        }
    }
    
    // check for FIRQ
    if (cpu->FIRQ)
    {
        cpu->SYNC = false;
        if (!cpu->CC_F)
        {
            if (!cpu->CWAI)
            {
                cpu->Clock += 10;
                cpu->CC_E = false;
                WRITE(cpu, --cpu->S.w, cpu->PC.b[LSB]);
                WRITE(cpu, --cpu->S.w, cpu->PC.b[MSB]);
                WRITE(cpu, --cpu->S.w, Get_6809_CC(cpu));
            }
            cpu->CC_F = true;
            cpu->CC_I = true;
            cpu->PC.b[MSB] = READ(cpu, MC6809_VECTOR_FIRQ);
            cpu->PC.b[LSB] = READ(cpu, MC6809_VECTOR_FIRQ + 1);
            cpu->CWAI = false;
            return cpu->Fault;
        }
    }

    // check for IRQ
    if (cpu->IRQ)
    {
        cpu->SYNC = false;
        if (!cpu->CC_I)
        {
            if (!cpu->CWAI)
            {
                cpu->Clock += 19;
                cpu->CC_E = true;
                WRITE(cpu, --cpu->S.w, cpu->PC.b[LSB]);
                WRITE(cpu, --cpu->S.w, cpu->PC.b[MSB]);
                WRITE(cpu, --cpu->S.w, cpu->U.b[LSB]);
                WRITE(cpu, --cpu->S.w, cpu->U.b[MSB]);
                WRITE(cpu, --cpu->S.w, cpu->Y.b[LSB]);
                WRITE(cpu, --cpu->S.w, cpu->Y.b[MSB]);
                WRITE(cpu, --cpu->S.w, cpu->X.b[LSB]);
                WRITE(cpu, --cpu->S.w, cpu->X.b[MSB]);
                WRITE(cpu, --cpu->S.w, cpu->DP);
                WRITE(cpu, --cpu->S.w, cpu->B);
                WRITE(cpu, --cpu->S.w, cpu->A);
                WRITE(cpu, --cpu->S.w, Get_6809_CC(cpu));
            }
            cpu->CC_I = true;
            cpu->PC.b[MSB] = READ(cpu, MC6809_VECTOR_IRQ);
            cpu->PC.b[LSB] = READ(cpu, MC6809_VECTOR_IRQ + 1);
            cpu->CWAI = false;
            return cpu->Fault;
        }
    }

    cpu->Clock++;

    // nothing to do while in wait mode
    if (cpu->CWAI || cpu->SYNC)
        return cpu->Fault;

    /*------------------------------------------------------------------
    ; While the cycle counts may appear to be one less than stated in the
    ; manual, the additional cycle has already been calculated above.
    ;--------------------------------------------------------------------*/

    uint16 ea;
    WORD d16;
    uint8 data;

    switch (READ(cpu, cpu->PC.w++))
    {
    case 0x00: // NEG
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea, op_neg(cpu, READ(cpu, ea)));
        break;

    case 0x01: return MC6809_FAULT_INSTRUCTION;

    case 0x02: return MC6809_FAULT_INSTRUCTION;

    case 0x03: // COM
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea, op_com(cpu, READ(cpu, ea)));
        break;

    case 0x04: // LSR
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea, op_lsr(cpu, READ(cpu, ea)));
        break;

    case 0x05: return MC6809_FAULT_INSTRUCTION;

    case 0x06: // ROR
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea, op_ror(cpu, READ(cpu, ea)));
        break;

    case 0x07: // ASR
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea, op_asr(cpu, READ(cpu, ea)));
        break;

    case 0x08: // LSL
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea, op_lsl(cpu, READ(cpu, ea)));
        break;

    case 0x09: // ROL
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea, op_rol(cpu, READ(cpu, ea)));
        break;

    case 0x0A: // DEC
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea, op_dec(cpu, READ(cpu, ea)));
        break;

    case 0x0B: return MC6809_FAULT_INSTRUCTION;

    case 0x0C: // INC
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea, op_inc(cpu, READ(cpu, ea)));
        break;

    case 0x0D: // TST
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        op_tst(cpu, READ(cpu, ea));
        break;

    case 0x0E: // JMP
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->PC.w = ea;
        break;

    case 0x0F: // CLR
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea, op_clr(cpu));
        break;

    case 0x10: // PAGE2
        page2(cpu);
        break;

    case 0x11: // PAGE3
        page3(cpu);
        break;

    case 0x12: // NOP
        cpu->Clock++;
        break;

    case 0x13: // SYNC
        cpu->Clock++;
        cpu->SYNC = true;
        break;

    case 0x14: return MC6809_FAULT_INSTRUCTION;

    case 0x15: return MC6809_FAULT_INSTRUCTION;

    case 0x16: // LBRA
        cpu->Clock += 4;
        cpu->PC.w = mc6809_lrelative(cpu);
        break;

    case 0x17: // LBSR
        cpu->Clock += 8;
        ea = mc6809_lrelative(cpu);
        WRITE(cpu, --cpu->S.w, cpu->PC.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->PC.b[MSB]);
        cpu->PC.w = ea;
        break;

    case 0x18: return MC6809_FAULT_INSTRUCTION;

    case 0x19: // DAA
        cpu->Clock++;
        {
            uint8 msn = cpu->A >> 4;
            uint8 lsn = cpu->A & 0x0F;
            uint8 cf = 0;
            bool H = cpu->CC_H;
            bool C = cpu->CC_C;

            if (cpu->CC_C || (msn > 9) || ((msn > 8) && (lsn > 9)))
                cf |= 0x60;
            if (cpu->CC_H || (lsn > 9))
                cf |= 0x06;

            cpu->A = op_add(cpu, cpu->A, cf);
            cpu->CC_H = H;
            cpu->CC_C = cpu->CC_C | C;
            cpu->CC_V = false;
        }
        break;

    case 0x1A: // ORCC
        cpu->Clock += 2;
        data = READ(cpu, cpu->PC.w++);
        data |= Get_6809_CC(cpu);
        Set_6809_CC(cpu, data);
        break;

    case 0x1B: return MC6809_FAULT_INSTRUCTION;

    case 0x1C: // ANDCC
        cpu->Clock += 2;
        data = READ(cpu, cpu->PC.w++);
        data &= Get_6809_CC(cpu);
        Set_6809_CC(cpu, data);
        break;

    case 0x1D: // SEX
        cpu->Clock++;
        cpu->A = (cpu->B > 0x7F) ? 0xFF : 0x00;
        cpu->CC_N = (cpu->D.w > 0x7FFF);
        cpu->CC_Z = (cpu->D.w == 0x0000);
        cpu->CC_V = false;
        break;

    case 0x1E: // EXG
        cpu->Clock += 7;
        d16.b[LSB] = READ(cpu, cpu->PC.w++);

        if ((d16.b[LSB] & 0x88) == 0x00)
        {
            uint16* src;
            uint16* dest;
            uint16  tmp;

            switch (d16.b[LSB] & 0xF0)
            {
            case 0x00: src = &cpu->D.w;  break;
            case 0x10: src = &cpu->X.w;  break;
            case 0x20: src = &cpu->Y.w;  break;
            case 0x30: src = &cpu->U.w;  break;
            case 0x40: src = &cpu->S.w;  break;
            case 0x50: src = &cpu->PC.w; break;
            default: return MC6809_FAULT_EXG;
            }

            switch (d16.b[LSB] & 0x0F)
            {
            case 0x00: dest = &cpu->D.w;  break;
            case 0x01: dest = &cpu->X.w;  break;
            case 0x02: dest = &cpu->Y.w;  break;
            case 0x03: dest = &cpu->U.w;  break;
            case 0x04: dest = &cpu->S.w;  cpu->NMI_Armed = true; break;
            case 0x05: dest = &cpu->PC.w; break;
            default: return MC6809_FAULT_EXG;
            }

            tmp = *src;
            *src = *dest;
            *dest = tmp;
        }
        else if ((d16.b[LSB] & 0x88) == 0x88)
        {
            uint8* src;
            uint8* dest;
            uint8  tmp;
            uint8  ccs;
            uint8  ccd;

            switch (d16.b[LSB] & 0xF0)
            {
            case 0x80: src = &cpu->A; break;
            case 0x90: src = &cpu->B; break;
            case 0xA0: src = &ccs; ccs = Get_6809_CC(cpu); break;
            case 0xB0: src = &cpu->DP; break;
            default: return MC6809_FAULT_EXG;
            }

            switch (d16.b[LSB] & 0x0F)
            {
            case 0x08: dest = &cpu->A; break;
            case 0x09: dest = &cpu->B; break;
            case 0x0A: dest = &ccd; ccd = Get_6809_CC(cpu); break;
            case 0x0B: dest = &cpu->DP; break;
            default: return MC6809_FAULT_EXG;
            }

            tmp = *src;
            *src = *dest;
            *dest = tmp;

            if (src == &ccs) Set_6809_CC(cpu, ccs);
            if (dest == &ccd) Set_6809_CC(cpu, ccd);
        }
        else
            return MC6809_FAULT_EXG;
        break;

    case 0x1F: // TFR
        cpu->Clock += 6;
        d16.b[LSB] = READ(cpu, cpu->PC.w++);

        if ((d16.b[LSB] & 0x88) == 0x00)
        {
            switch (d16.b[LSB] & 0xF0)
            {
            case 0x00: ea = cpu->D.w;  break;
            case 0x10: ea = cpu->X.w;  break;
            case 0x20: ea = cpu->Y.w;  break;
            case 0x30: ea = cpu->U.w;  break;
            case 0x40: ea = cpu->S.w;  cpu->NMI_Armed = true; break;
            case 0x50: ea = cpu->PC.w; break;
            default: return MC6809_FAULT_TFR;
            }

            switch (d16.b[LSB] & 0x0F)
            {
            case 0x00: cpu->D.w = ea;  break;
            case 0x01: cpu->X.w = ea;  break;
            case 0x02: cpu->Y.w = ea;  break;
            case 0x03: cpu->U.w = ea;  break;
            case 0x04: cpu->S.w = ea;  cpu->NMI_Armed = true; break;
            case 0x05: cpu->PC.w = ea; break;
            default: return MC6809_FAULT_TFR;
            }
        }
        else if ((d16.b[LSB] & 0x88) == 0x88)
        {
            switch (d16.b[LSB] & 0xF0)
            {
            case 0x80: data = cpu->A; break;
            case 0x90: data = cpu->B; break;
            case 0xA0: data = Get_6809_CC(cpu); break;
            case 0xB0: data = cpu->DP; break;
            default: return MC6809_FAULT_TFR;
            }

            switch (d16.b[LSB] & 0x0F)
            {
            case 0x08: cpu->A = data; break;
            case 0x09: cpu->B = data; break;
            case 0x0A: Set_6809_CC(cpu, data); break;
            case 0x0B: cpu->DP = data; break;
            default: return MC6809_FAULT_TFR;
            }
        }
        else
            return MC6809_FAULT_TFR;
        break;

    case 0x20: // BRA
        cpu->Clock += 2;
        cpu->PC.w = mc6809_relative(cpu);
        break;

    case 0x21: // BRN
        cpu->Clock += 2;
        mc6809_relative(cpu);
        break;

    case 0x22: // BHI
        cpu->Clock += 2;
        ea = mc6809_relative(cpu);
        if (bhi(cpu))
            cpu->PC.w = ea;
        break;

    case 0x23: // BLS
        cpu->Clock += 2;
        ea = mc6809_relative(cpu);
        if (bls(cpu))
            cpu->PC.w = ea;
        break;

    case 0x24: // BHS, BCC
        cpu->Clock += 2;
        ea = mc6809_relative(cpu);
        if (bhs(cpu))
            cpu->PC.w = ea;
        break;

    case 0x25: // BLO
        cpu->Clock += 2;
        ea = mc6809_relative(cpu);
        if (blo(cpu))
            cpu->PC.w = ea;
        break;

    case 0x26: // BNE
        cpu->Clock += 2;
        ea = mc6809_relative(cpu);
        if (bne(cpu))
            cpu->PC.w = ea;
        break;

    case 0x27: // BEQ
        cpu->Clock += 2;
        ea = mc6809_relative(cpu);
        if (beq(cpu))
            cpu->PC.w = ea;
        break;

    case 0x28: // BVC
        cpu->Clock += 2;
        ea = mc6809_relative(cpu);
        if (bvc(cpu))
            cpu->PC.w = ea;
        break;

    case 0x29: // BVS
        cpu->Clock += 2;
        ea = mc6809_relative(cpu);
        if (bvs(cpu))
            cpu->PC.w = ea;
        break;

    case 0x2A: // BPL
        cpu->Clock += 2;
        ea = mc6809_relative(cpu);
        if (bpl(cpu))
            cpu->PC.w = ea;
        break;

    case 0x2B: // BMI
        cpu->Clock += 2;
        ea = mc6809_relative(cpu);
        if (bmi(cpu))
            cpu->PC.w = ea;
        break;

    case 0x2C: // BGE
        cpu->Clock += 2;
        ea = mc6809_relative(cpu);
        if (bge(cpu))
            cpu->PC.w = ea;
        break;

    case 0x2D: // BLT
        cpu->Clock += 2;
        ea = mc6809_relative(cpu);
        if (blt(cpu))
            cpu->PC.w = ea;
        break;

    case 0x2E: // BGT
        cpu->Clock += 2;
        ea = mc6809_relative(cpu);
        if (bgt(cpu))
            cpu->PC.w = ea;
        break;

    case 0x2F: // BLE
        cpu->Clock += 2;
        ea = mc6809_relative(cpu);
        if (ble(cpu))
            cpu->PC.w = ea;
        break;

    case 0x30: // LEAX
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->X.w = ea;
        cpu->CC_Z = (cpu->X.w == 0x0000);
        break;

    case 0x31: // LEAY
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->Y.w = ea;
        cpu->CC_Z = (cpu->Y.w == 0x0000);
        break;

    case 0x32: // LEAS
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->S.w = ea;
        cpu->NMI_Armed = true;
        break;

    case 0x33: // LEAU
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->U.w = ea;
        break;

    case 0x34: // PSHS
        cpu->Clock += 4;
        data = READ(cpu, cpu->PC.w++);
        if (data & 0x80)
        {
            cpu->Clock += 2;
            WRITE(cpu, --cpu->S.w, cpu->PC.b[LSB]);
            WRITE(cpu, --cpu->S.w, cpu->PC.b[MSB]);
        }
        if (data & 0x40)
        {
            cpu->Clock += 2;
            WRITE(cpu, --cpu->S.w, cpu->U.b[LSB]);
            WRITE(cpu, --cpu->S.w, cpu->U.b[MSB]);
        }
        if (data & 0x20)
        {
            cpu->Clock += 2;
            WRITE(cpu, --cpu->S.w, cpu->Y.b[LSB]);
            WRITE(cpu, --cpu->S.w, cpu->Y.b[MSB]);
        }
        if (data & 0x10)
        {
            cpu->Clock += 2;
            WRITE(cpu, --cpu->S.w, cpu->X.b[LSB]);
            WRITE(cpu, --cpu->S.w, cpu->X.b[MSB]);
        }
        if (data & 0x08)
        {
            cpu->Clock++;
            WRITE(cpu, --cpu->S.w, cpu->DP);
        }
        if (data & 0x04)
        {
            cpu->Clock++;
            WRITE(cpu, --cpu->S.w, cpu->B);
        }
        if (data & 0x02)
        {
            cpu->Clock++;
            WRITE(cpu, --cpu->S.w, cpu->A);
        }
        if (data & 0x01)
        {
            cpu->Clock++;
            WRITE(cpu, --cpu->S.w, Get_6809_CC(cpu));
        }
        break;

    case 0x35: // PULS
        cpu->Clock += 4;
        data = READ(cpu, cpu->PC.w++);
        if (data & 0x01)
        {
            cpu->Clock++;
            Set_6809_CC(cpu, READ(cpu, cpu->S.w++));
        }
        if (data & 0x02)
        {
            cpu->Clock++;
            cpu->A = READ(cpu, cpu->S.w++);
        }
        if (data & 0x04)
        {
            cpu->Clock++;
            cpu->B = READ(cpu, cpu->S.w++);
        }
        if (data & 0x08)
        {
            cpu->Clock++;
            cpu->DP = READ(cpu, cpu->S.w++);
        }
        if (data & 0x10)
        {
            cpu->Clock += 2;
            cpu->X.b[MSB] = READ(cpu, cpu->S.w++);
            cpu->X.b[LSB] = READ(cpu, cpu->S.w++);
        }
        if (data & 0x20)
        {
            cpu->Clock += 2;
            cpu->Y.b[MSB] = READ(cpu, cpu->S.w++);
            cpu->Y.b[LSB] = READ(cpu, cpu->S.w++);
        }
        if (data & 0x40)
        {
            cpu->Clock += 2;
            cpu->U.b[MSB] = READ(cpu, cpu->S.w++);
            cpu->U.b[LSB] = READ(cpu, cpu->S.w++);
        }
        if (data & 0x80)
        {
            cpu->Clock += 2;
            cpu->PC.b[MSB] = READ(cpu, cpu->S.w++);
            cpu->PC.b[LSB] = READ(cpu, cpu->S.w++);
        }
        break;

    case 0x36: // PSHU
        cpu->Clock += 4;
        data = READ(cpu, cpu->PC.w++);
        if (data & 0x80)
        {
            cpu->Clock += 2;
            WRITE(cpu, --cpu->U.w, cpu->PC.b[LSB]);
            WRITE(cpu, --cpu->U.w, cpu->PC.b[MSB]);
        }
        if (data & 0x40)
        {
            cpu->Clock += 2;
            WRITE(cpu, --cpu->U.w, cpu->S.b[LSB]);
            WRITE(cpu, --cpu->U.w, cpu->S.b[MSB]);
        }
        if (data & 0x20)
        {
            cpu->Clock += 2;
            WRITE(cpu, --cpu->U.w, cpu->Y.b[LSB]);
            WRITE(cpu, --cpu->U.w, cpu->Y.b[MSB]);
        }
        if (data & 0x10)
        {
            cpu->Clock += 2;
            WRITE(cpu, --cpu->U.w, cpu->X.b[LSB]);
            WRITE(cpu, --cpu->U.w, cpu->X.b[MSB]);
        }
        if (data & 0x08)
        {
            cpu->Clock++;
            WRITE(cpu, --cpu->U.w, cpu->DP);
        }
        if (data & 0x04)
        {
            cpu->Clock++;
            WRITE(cpu, --cpu->U.w, cpu->B);
        }
        if (data & 0x02)
        {
            cpu->Clock++;
            WRITE(cpu, --cpu->U.w, cpu->A);
        }
        if (data & 0x01)
        {
            cpu->Clock++;
            WRITE(cpu, --cpu->U.w, Get_6809_CC(cpu));
        }
        break;

    case 0x37: // PULU
        cpu->Clock += 4;
        data = READ(cpu, cpu->PC.w++);
        if (data & 0x01)
        {
            cpu->Clock++;
            Set_6809_CC(cpu, READ(cpu, cpu->U.w++));
        }
        if (data & 0x02)
        {
            cpu->Clock++;
            cpu->A = READ(cpu, cpu->U.w++);
        }
        if (data & 0x04)
        {
            cpu->Clock++;
            cpu->B = READ(cpu, cpu->U.w++);
        }
        if (data & 0x08)
        {
            cpu->Clock++;
            cpu->DP = READ(cpu, cpu->U.w++);
        }
        if (data & 0x10)
        {
            cpu->Clock += 2;
            cpu->X.b[MSB] = READ(cpu, cpu->U.w++);
            cpu->X.b[LSB] = READ(cpu, cpu->U.w++);
        }
        if (data & 0x20)
        {
            cpu->Clock += 2;
            cpu->Y.b[MSB] = READ(cpu, cpu->U.w++);
            cpu->Y.b[LSB] = READ(cpu, cpu->U.w++);
        }
        if (data & 0x40)
        {
            cpu->Clock += 2;
            cpu->S.b[MSB] = READ(cpu, cpu->U.w++);
            cpu->S.b[LSB] = READ(cpu, cpu->U.w++);
            cpu->NMI_Armed = true;
        }
        if (data & 0x80)
        {
            cpu->Clock += 2;
            cpu->PC.b[MSB] = READ(cpu, cpu->U.w++);
            cpu->PC.b[LSB] = READ(cpu, cpu->U.w++);
        }
        break;

    case 0x38: return MC6809_FAULT_INSTRUCTION;

    case 0x39: // RTS
        cpu->Clock += 4;
        cpu->PC.b[MSB] = READ(cpu, cpu->S.w++);
        cpu->PC.b[LSB] = READ(cpu, cpu->S.w++);
        break;

    case 0x3A: // ABX
        cpu->Clock += 2;
        d16.b[LSB] = cpu->B;
        d16.b[MSB] = 0;
        cpu->X.w += d16.w;
        break;

    case 0x3B: // RTI
        cpu->Clock += 5;
        Set_6809_CC(cpu, READ(cpu, cpu->S.w++));
        if (cpu->CC_E)
        {
            cpu->Clock += 9;
            cpu->A = READ(cpu, cpu->S.w++);
            cpu->B = READ(cpu, cpu->S.w++);
            cpu->DP = READ(cpu, cpu->S.w++);
            cpu->X.b[MSB] = READ(cpu, cpu->S.w++);
            cpu->X.b[LSB] = READ(cpu, cpu->S.w++);
            cpu->Y.b[MSB] = READ(cpu, cpu->S.w++);
            cpu->Y.b[LSB] = READ(cpu, cpu->S.w++);
            cpu->U.b[MSB] = READ(cpu, cpu->S.w++);
            cpu->U.b[LSB] = READ(cpu, cpu->S.w++);
        }
        cpu->PC.b[MSB] = READ(cpu, cpu->S.w++);
        cpu->PC.b[LSB] = READ(cpu, cpu->S.w++);
        break;

    case 0x3C: // CWAI
        cpu->Clock += 20;
        data = READ(cpu, cpu->PC.w++);
        Set_6809_CC(cpu, Get_6809_CC(cpu) & data);
        cpu->CC_E = true;
        WRITE(cpu, --cpu->S.w, cpu->PC.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->PC.b[MSB]);
        WRITE(cpu, --cpu->S.w, cpu->U.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->U.b[MSB]);
        WRITE(cpu, --cpu->S.w, cpu->Y.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->Y.b[MSB]);
        WRITE(cpu, --cpu->S.w, cpu->X.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->X.b[MSB]);
        WRITE(cpu, --cpu->S.w, cpu->DP);
        WRITE(cpu, --cpu->S.w, cpu->B);
        WRITE(cpu, --cpu->S.w, cpu->A);
        WRITE(cpu, --cpu->S.w, Get_6809_CC(cpu));
        cpu->CWAI = true;
        break;

    case 0x3D: // MUL
        cpu->Clock += 10;
        cpu->D.w = (uint16)cpu->A * (uint16)cpu->B;
        cpu->CC_C = (cpu->B & 0x80) == 0x80;
        cpu->CC_Z = (cpu->D.w == 0x0000);
        break;

    case 0x3E: return MC6809_FAULT_INSTRUCTION;

    case 0x3F: // SWI
        cpu->Clock += 18;
        cpu->CC_E = true;
        WRITE(cpu, --cpu->S.w, cpu->PC.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->PC.b[MSB]);
        WRITE(cpu, --cpu->S.w, cpu->U.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->U.b[MSB]);
        WRITE(cpu, --cpu->S.w, cpu->Y.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->Y.b[MSB]);
        WRITE(cpu, --cpu->S.w, cpu->X.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->X.b[MSB]);
        WRITE(cpu, --cpu->S.w, cpu->DP);
        WRITE(cpu, --cpu->S.w, cpu->B);
        WRITE(cpu, --cpu->S.w, cpu->A);
        WRITE(cpu, --cpu->S.w, Get_6809_CC(cpu));
        cpu->CC_F = true;
        cpu->CC_I = true;
        cpu->PC.b[MSB] = READ(cpu, MC6809_VECTOR_SWI);
        cpu->PC.b[LSB] = READ(cpu, MC6809_VECTOR_SWI + 1);
        break;

    case 0x40: // NEGA
        cpu->Clock++;
        cpu->A = op_neg(cpu, cpu->A);
        break;

    case 0x41: return MC6809_FAULT_INSTRUCTION;

    case 0x42: return MC6809_FAULT_INSTRUCTION;

    case 0x43: // COMA
        cpu->Clock++;
        cpu->A = op_com(cpu, cpu->A);
        break;

    case 0x44: // LSRA
        cpu->Clock++;
        cpu->A = op_lsr(cpu, cpu->A);
        break;

    case 0x45: return MC6809_FAULT_INSTRUCTION;

    case 0x46: // RORA
        cpu->Clock++;
        cpu->A = op_ror(cpu, cpu->A);
        break;

    case 0x47: // ASRA
        cpu->Clock++;
        cpu->A = op_asr(cpu, cpu->A);
        break;

    case 0x48: // LSLA
        cpu->Clock++;
        cpu->A = op_lsl(cpu, cpu->A);
        break;

    case 0x49: // ROLA
        cpu->Clock++;
        cpu->A = op_rol(cpu, cpu->A);
        break;

    case 0x4A: // DECA
        cpu->Clock++;
        cpu->A = op_dec(cpu, cpu->A);
        break;

    case 0x4B: return MC6809_FAULT_INSTRUCTION;

    case 0x4C: // INCA
        cpu->Clock++;
        cpu->A = op_inc(cpu, cpu->A);
        break;

    case 0x4D: // TSTA
        cpu->Clock++;
        op_tst(cpu, cpu->A);
        break;

    case 0x4E: return MC6809_FAULT_INSTRUCTION;

    case 0x4F: // CLRA
        cpu->Clock++;
        cpu->A = op_clr(cpu);
        break;

    case 0x50: // NEGB
        cpu->Clock++;
        cpu->B = op_neg(cpu, cpu->B);
        break;

    case 0x51: return MC6809_FAULT_INSTRUCTION;
        
    case 0x52: return MC6809_FAULT_INSTRUCTION;

    case 0x53: // COMB
        cpu->Clock++;
        cpu->B = op_com(cpu, cpu->B);
        break;

    case 0x54: // LSRB
        cpu->Clock++;
        cpu->B = op_lsr(cpu, cpu->B);
        break;

    case 0x55: return MC6809_FAULT_INSTRUCTION;

    case 0x56: // RORB
        cpu->Clock++;
        cpu->B = op_ror(cpu, cpu->B);
        break;

    case 0x57: // ASRB
        cpu->Clock++;
        cpu->B = op_asr(cpu, cpu->B);
        break;

    case 0x58:  // LSLB
        cpu->Clock++;
        cpu->B = op_lsl(cpu, cpu->B);
        break;

    case 0x59: // ROLB
        cpu->Clock++;
        cpu->B = op_rol(cpu, cpu->B);
        break;

    case 0x5A: // DECB
        cpu->Clock++;
        cpu->B = op_dec(cpu, cpu->B);
        break;

    case 0x5B: return MC6809_FAULT_INSTRUCTION;

    case 0x5C: // INCB
        cpu->Clock++;
        cpu->B = op_inc(cpu, cpu->B);
        break;

    case 0x5D: // TSTB
        cpu->Clock++;
        op_tst(cpu, cpu->B);
        break;

    case 0x5E: return MC6809_FAULT_INSTRUCTION;

    case 0x5F: // CLRB
        cpu->Clock++;
        cpu->B = op_clr(cpu);
        break;

    case 0x60: // NEG
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea, op_neg(cpu, READ(cpu, ea)));
        break;

    case 0x61: return MC6809_FAULT_INSTRUCTION;

    case 0x62: return MC6809_FAULT_INSTRUCTION;

    case 0x63: // COM
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea, op_com(cpu, READ(cpu, ea)));
        break;

    case 0x64: // LSR
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea, op_lsr(cpu, READ(cpu, ea)));
        break;

    case 0x65: return MC6809_FAULT_INSTRUCTION;

    case 0x66: // ROR
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea, op_ror(cpu, READ(cpu, ea)));
        break;

    case 0x67: // ASR
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea, op_asr(cpu, READ(cpu, ea)));
        break;

    case 0x68: // LSL
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea, op_lsl(cpu, READ(cpu, ea)));
        break;

    case 0x69: // ROL
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea, op_rol(cpu, READ(cpu, ea)));
        break;

    case 0x6A: // DEC
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea, op_dec(cpu, READ(cpu, ea)));
        break;

    case 0x6B: return MC6809_FAULT_INSTRUCTION;

    case 0x6C: // INC
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea, op_inc(cpu, READ(cpu, ea)));
        break;

    case 0x6D: // TST
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        op_tst(cpu, READ(cpu, ea));
        break;

    case 0x6E: // JMP
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->PC.w = ea;
        break;

    case 0x6F: // CLR
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea, op_clr(cpu));
        break;

    case 0x70: // NEG
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea, op_neg(cpu, READ(cpu, ea)));
        break;

    case 0x71: return MC6809_FAULT_INSTRUCTION;

    case 0x72: return MC6809_FAULT_INSTRUCTION;

    case 0x73: // COM
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea, op_com(cpu, READ(cpu, ea)));
        break;

    case 0x74: // LSR
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea, op_lsr(cpu, READ(cpu, ea)));
        break;

    case 0x75: return MC6809_FAULT_INSTRUCTION;

    case 0x76: // ROR
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea, op_ror(cpu, READ(cpu, ea)));
        break;

    case 0x77: // ASR
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea, op_asr(cpu, READ(cpu, ea)));
        break;

    case 0x78: // LSL
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea, op_lsl(cpu, READ(cpu, ea)));
        break;

    case 0x79: // ROL
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea, op_rol(cpu, READ(cpu, ea)));
        break;

    case 0x7A: // DEC
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea, op_dec(cpu, READ(cpu, ea)));
        break;

    case 0x7B: return MC6809_FAULT_INSTRUCTION;

    case 0x7C: // INC
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea, op_inc(cpu, READ(cpu, ea)));
        break;

    case 0x7D: // TST
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        op_tst(cpu, READ(cpu, ea));
        break;

    case 0x7E: // JMP
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->PC.w = ea;
        break;

    case 0x7F: // CLR
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea, op_clr(cpu));
        break;

    case 0x80: // SUBA
        cpu->Clock++;
        cpu->A = op_sub(cpu, cpu->A, READ(cpu, cpu->PC.w++));
        break;

    case 0x81: // CMPA
        cpu->Clock++;
        op_cmp(cpu, cpu->A, READ(cpu, cpu->PC.w++));
        break;

    case 0x82: // SBCA
        cpu->Clock++;
        cpu->A = op_sbc(cpu, cpu->A, READ(cpu, cpu->PC.w++));
        break;

    case 0x83: // SUBD
        cpu->Clock += 3;
        d16.b[MSB] = READ(cpu, cpu->PC.w++);
        d16.b[LSB] = READ(cpu, cpu->PC.w++);
        cpu->D.w = op_sub16(cpu, cpu->D.w, d16.w);
        break;

    case 0x84: // ANDA
        cpu->Clock++;
        cpu->A = op_and(cpu, cpu->A, READ(cpu, cpu->PC.w++));
        break;

    case 0x85: // BITA
        cpu->Clock++;
        op_bit(cpu, cpu->A, READ(cpu, cpu->PC.w++));
        break;

    case 0x86: // LDA
        cpu->Clock++;
        cpu->A = READ(cpu, cpu->PC.w++);
        op_ldst(cpu, cpu->A);
        break;

    case 0x87: return MC6809_FAULT_INSTRUCTION;

    case 0x88: // EORA
        cpu->Clock++;
        cpu->A = op_eor(cpu, cpu->A, READ(cpu, cpu->PC.w++));
        break;

    case 0x89: // ADCA
        cpu->Clock++;
        cpu->A = op_adc(cpu, cpu->A, READ(cpu, cpu->PC.w++));
        break;

    case 0x8A: // ORA
        cpu->Clock++;
        cpu->A = op_or(cpu, cpu->A, READ(cpu, cpu->PC.w++));
        break;

    case 0x8B: // ADDA
        cpu->Clock++;
        cpu->A = op_add(cpu, cpu->A, READ(cpu, cpu->PC.w++));
        break;

    case 0x8C: // CMPX
        cpu->Clock += 3;
        d16.b[MSB] = READ(cpu, cpu->PC.w++);
        d16.b[LSB] = READ(cpu, cpu->PC.w++);
        op_cmp16(cpu, cpu->X.w, d16.w);
        break;

    case 0x8D: // BSR
        cpu->Clock += 6;
        ea = mc6809_relative(cpu);
        WRITE(cpu, --cpu->S.w, cpu->PC.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->PC.b[MSB]);
        cpu->PC.w = ea;
        break;

    case 0x8E: // LDX
        cpu->Clock += 2;
        cpu->X.b[MSB] = READ(cpu, cpu->PC.w++);
        cpu->X.b[LSB] = READ(cpu, cpu->PC.w++);
        op_ldst16(cpu, cpu->X.w);
        break;

    case 0x8F: return MC6809_FAULT_INSTRUCTION;

    case 0x90: // SUBA
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->A = op_sub(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0x91: // CMPA
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        op_cmp(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0x92: // SBCA
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->A = op_sbc(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0x93: // SUBD
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        cpu->D.w = op_sub16(cpu, cpu->D.w, d16.w);
        break;

    case 0x94: // ANDA
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->A = op_and(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0x95: // BITA
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        op_bit(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0x96: // LDA
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->A = READ(cpu, ea);
        op_ldst(cpu, cpu->A);
        break;

    case 0x97: // STA
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea, cpu->A);
        op_ldst(cpu, cpu->A);
        break;

    case 0x98: // EORA
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->A = op_eor(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0x99: // ADCA
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->A = op_adc(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0x9A: // ORA
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->A = op_or(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0x9B: // ADDA
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->A = op_add(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0x9C: // CMPX
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        op_cmp16(cpu, cpu->X.w, d16.w);
        break;

    case 0x9D: // JSR
        cpu->Clock += 6;
        ea = mc6809_direct(cpu);
        WRITE(cpu, --cpu->S.w, cpu->PC.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->PC.b[MSB]);
        cpu->PC.w = ea;
        break;

    case 0x9E: // LDX
        cpu->Clock += 4;
        ea = mc6809_direct(cpu);
        cpu->X.b[MSB] = READ(cpu, ea++);
        cpu->X.b[LSB] = READ(cpu, ea);
        op_ldst16(cpu, cpu->X.w);
        break;

    case 0x9F: // STX
        cpu->Clock += 4;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea++, cpu->X.b[MSB]);
        WRITE(cpu, ea, cpu->X.b[LSB]);
        op_ldst16(cpu, cpu->X.w);
        break;

    case 0xA0: // SUBA
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->A = op_sub(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xA1: // CMPA
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        op_cmp(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xA2: // SBCA
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->A = op_sbc(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xA3: // SUBD
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        cpu->D.w = op_sub16(cpu, cpu->D.w, d16.w);
        break;

    case 0xA4: // ANDA
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->A = op_and(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xA5: // BITA
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        op_bit(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xA6: // LDA
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->A = READ(cpu, ea);
        op_ldst(cpu, cpu->A);
        break;

    case 0xA7: // STA
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea, cpu->A);
        op_ldst(cpu, cpu->A);
        break;

    case 0xA8: // EORA
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->A = op_eor(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xA9: // ADCA
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->A = op_adc(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xAA: // ORA
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->A = op_or(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xAB: // ADDA
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->A = op_add(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xAC: // CMPX
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        op_cmp16(cpu, cpu->X.w, d16.w);
        break;

    case 0xAD: // JSR
        cpu->Clock += 6;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, --cpu->S.w, cpu->PC.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->PC.b[MSB]);
        cpu->PC.w = ea;
        break;

    case 0xAE: // LDX
        cpu->Clock += 4;
        ea = mc6809_indexed(cpu);
        cpu->X.b[MSB] = READ(cpu, ea++);
        cpu->X.b[LSB] = READ(cpu, ea);
        op_ldst16(cpu, cpu->X.w);
        break;

    case 0xAF: // STX
        cpu->Clock += 4;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea++, cpu->X.b[MSB]);
        WRITE(cpu, ea, cpu->X.b[LSB]);
        op_ldst16(cpu, cpu->X.w);
        break;

    case 0xB0: // SUBA
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->A = op_sub(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xB1: // CMPA
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        op_cmp(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xB2: // SBCA
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->A = op_sbc(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xB3: // SUBD
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        cpu->D.w = op_sub16(cpu, cpu->D.w, d16.w);
        break;

    case 0xB4: // ANDA
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->A = op_and(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xB5: // BITA
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        op_bit(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xB6: // LDA
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->A = READ(cpu, ea);
        op_ldst(cpu, cpu->A);
        break;

    case 0xB7: // STA
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea, cpu->A);
        op_ldst(cpu, cpu->A);
        break;

    case 0xB8: // EORA
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->A = op_eor(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xB9: // ADCA
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->A = op_adc(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xBA: // ORA
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->A = op_or(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xBB: // ADDA
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->A = op_add(cpu, cpu->A, READ(cpu, ea));
        break;

    case 0xBC: // CMPX
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        op_cmp16(cpu, cpu->X.w, d16.w);
        break;

    case 0xBD: // JSR
        cpu->Clock += 7;
        ea = mc6809_extended(cpu);
        WRITE(cpu, --cpu->S.w, cpu->PC.b[LSB]);
        WRITE(cpu, --cpu->S.w, cpu->PC.b[MSB]);
        cpu->PC.w = ea;
        break;

    case 0xBE: // LDX
        cpu->Clock += 5;
        ea = mc6809_extended(cpu);
        cpu->X.b[MSB] = READ(cpu, ea++);
        cpu->X.b[LSB] = READ(cpu, ea);
        op_ldst16(cpu, cpu->X.w);
        break;

    case 0xBF: // STX
        cpu->Clock += 5;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea++, cpu->X.b[MSB]);
        WRITE(cpu, ea, cpu->X.b[LSB]);
        op_ldst16(cpu, cpu->X.w);
        break;

    case 0xC0: // SUBB
        cpu->Clock++;
        cpu->B = op_sub(cpu, cpu->B, READ(cpu, cpu->PC.w++));
        break;

    case 0xC1: // CMPB
        cpu->Clock++;
        op_cmp(cpu, cpu->B, READ(cpu, cpu->PC.w++));
        break;

    case 0xC2: // SBCB
        cpu->Clock++;
        cpu->B = op_sbc(cpu, cpu->B, READ(cpu, cpu->PC.w++));
        break;

    case 0xC3: // ADDD
        cpu->Clock += 3;
        d16.b[MSB] = READ(cpu, cpu->PC.w++);
        d16.b[LSB] = READ(cpu, cpu->PC.w++);
        cpu->D.w = op_add16(cpu, cpu->D.w, d16.w);
        break;

    case 0xC4: // ANDB
        cpu->Clock++;
        cpu->B = op_and(cpu, cpu->B, READ(cpu, cpu->PC.w++));
        break;

    case 0xC5: // BITB
        cpu->Clock++;
        op_bit(cpu, cpu->B, READ(cpu, cpu->PC.w++));
        break;

    case 0xC6: // LDB
        cpu->Clock++;
        cpu->B = READ(cpu, cpu->PC.w++);
        op_ldst(cpu, cpu->B);
        break;

    case 0xC7: return MC6809_FAULT_INSTRUCTION;

    case 0xC8: // EORB
        cpu->Clock++;
        cpu->B = op_eor(cpu, cpu->B, READ(cpu, cpu->PC.w++));
        break;

    case 0xC9: // ADCB
        cpu->Clock++;
        cpu->B = op_adc(cpu, cpu->B, READ(cpu, cpu->PC.w++));
        break;

    case 0xCA: // ORB
        cpu->Clock++;
        cpu->B = op_or(cpu, cpu->B, READ(cpu, cpu->PC.w++));
        break;

    case 0xCB: // ADDB
        cpu->Clock++;
        cpu->B = op_add(cpu, cpu->B, READ(cpu, cpu->PC.w++));
        break;

    case 0xCC: // LDD
        cpu->Clock += 2;
        cpu->A = READ(cpu, cpu->PC.w++);
        cpu->B = READ(cpu, cpu->PC.w++);
        op_ldst16(cpu, cpu->D.w);
        break;

    case 0xCD: return MC6809_FAULT_INSTRUCTION;

    case 0xCE: // LDU
        cpu->Clock += 2;
        cpu->U.b[MSB] = READ(cpu, cpu->PC.w++);
        cpu->U.b[LSB] = READ(cpu, cpu->PC.w++);
        op_ldst16(cpu, cpu->U.w);
        break;

    case 0xCF: return MC6809_FAULT_INSTRUCTION;

    case 0xD0: // SUBB
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->B = op_sub(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xD1: // CMPB
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        op_cmp(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xD2: // SBCB
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->B = op_sbc(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xD3: // ADDD
        cpu->Clock += 5;
        ea = mc6809_direct(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        cpu->D.w = op_add16(cpu, cpu->D.w, d16.w);
        break;

    case 0xD4: // ANDB
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->B = op_and(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xD5: // BITB
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        op_bit(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xD6: // LDB
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->B = READ(cpu, ea);
        op_ldst(cpu, cpu->B);
        break;

    case 0xD7: // STB
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea, cpu->B);
        op_ldst(cpu, cpu->B);
        break;

    case 0xD8: // EORB
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->B = op_eor(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xD9: // ADCB
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->B = op_adc(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xDA: // ORB
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->B = op_or(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xDB: // ADDB
        cpu->Clock += 3;
        ea = mc6809_direct(cpu);
        cpu->B = op_add(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xDC: // LDD
        cpu->Clock += 4;
        ea = mc6809_direct(cpu);
        cpu->A = READ(cpu, ea++);
        cpu->B = READ(cpu, ea);
        op_ldst16(cpu, cpu->D.w);
        break;

    case 0xDD: // STD
        cpu->Clock += 6;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea++, cpu->A);
        WRITE(cpu, ea, cpu->B);
        op_ldst16(cpu, cpu->D.w);
        break;

    case 0xDE: // LDU
        cpu->Clock += 4;
        ea = mc6809_direct(cpu);
        cpu->U.b[MSB] = READ(cpu, ea++);
        cpu->U.b[LSB] = READ(cpu, ea);
        op_ldst16(cpu, cpu->U.w);
        break;

    case 0xDF: // STU
        cpu->Clock += 4;
        ea = mc6809_direct(cpu);
        WRITE(cpu, ea++, cpu->U.b[MSB]);
        WRITE(cpu, ea, cpu->U.b[LSB]);
        op_ldst16(cpu, cpu->U.w);
        break;

    case 0xE0: // SUBB
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->B = op_sub(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xE1: // CMPB
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        op_cmp(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xE2: // SBCB
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->B = op_sbc(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xE3: // ADDD
        cpu->Clock += 5;
        ea = mc6809_indexed(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        cpu->D.w = op_add16(cpu, cpu->D.w, d16.w);
        break;

    case 0xE4: // ANDB
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->B = op_and(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xE5: // BITB
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        op_bit(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xE6: // LDB
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->B = READ(cpu, ea);
        op_ldst(cpu, cpu->B);
        break;

    case 0xE7: // STB
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea, cpu->B);
        op_ldst(cpu, cpu->B);
        break;

    case 0xE8: // EORB
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->B = op_eor(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xE9: // ADCB
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->B = op_adc(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xEA: // ORB
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->B = op_or(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xEB: // ADDB
        cpu->Clock += 3;
        ea = mc6809_indexed(cpu);
        cpu->B = op_add(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xEC: // LDD
        cpu->Clock += 4;
        ea = mc6809_indexed(cpu);
        cpu->A = READ(cpu, ea++);
        cpu->B = READ(cpu, ea);
        op_ldst16(cpu, cpu->D.w);
        break;

    case 0xED: // STD
        cpu->Clock += 6;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea++, cpu->A);
        WRITE(cpu, ea, cpu->B);
        op_ldst16(cpu, cpu->D.w);
        break;

    case 0xEE: // LDU
        cpu->Clock += 4;
        ea = mc6809_indexed(cpu);
        cpu->U.b[MSB] = READ(cpu, ea++);
        cpu->U.b[LSB] = READ(cpu, ea);
        op_ldst16(cpu, cpu->U.w);
        break;

    case 0xEF: // STU
        cpu->Clock += 4;
        ea = mc6809_indexed(cpu);
        WRITE(cpu, ea++, cpu->U.b[MSB]);
        WRITE(cpu, ea, cpu->U.b[LSB]);
        op_ldst16(cpu, cpu->U.w);
        break;

    case 0xF0: // SUBB
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->B = op_sub(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xF1: // CMPB
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        op_cmp(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xF2: // SBCB
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->B = op_sbc(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xF3: // ADDD
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        d16.b[MSB] = READ(cpu, ea++);
        d16.b[LSB] = READ(cpu, ea);
        cpu->D.w = op_add16(cpu, cpu->D.w, d16.w);
        break;

    case 0xF4: // ANDB
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->B = op_and(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xF5: // BITB
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        op_bit(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xF6: // LDB
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->B = READ(cpu, ea);
        op_ldst(cpu, cpu->B);
        break;

    case 0xF7: // STB
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea, cpu->B);
        op_ldst(cpu, cpu->B);
        break;

    case 0xF8: // EORB
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->B = op_eor(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xF9: // ADCB
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->B = op_adc(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xFA: // ORB
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->B = op_or(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xFB: // ADDB
        cpu->Clock += 4;
        ea = mc6809_extended(cpu);
        cpu->B = op_add(cpu, cpu->B, READ(cpu, ea));
        break;

    case 0xFC: // LDD
        cpu->Clock += 5;
        ea = mc6809_extended(cpu);
        cpu->A = READ(cpu, ea++);
        cpu->B = READ(cpu, ea);
        op_ldst16(cpu, cpu->D.w);
        break;

    case 0xFD: // STD
        cpu->Clock += 6;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea++, cpu->A);
        WRITE(cpu, ea, cpu->B);
        op_ldst16(cpu, cpu->D.w);
        break;

    case 0xFE: // LDU
        cpu->Clock += 5;
        ea = mc6809_extended(cpu);
        cpu->U.b[MSB] = READ(cpu, ea++);
        cpu->U.b[LSB] = READ(cpu, ea);
        op_ldst16(cpu, cpu->U.w);
        break;

    case 0xFF: // STU
        cpu->Clock += 5;
        ea = mc6809_extended(cpu);
        WRITE(cpu, ea++, cpu->U.b[MSB]);
        WRITE(cpu, ea, cpu->U.b[LSB]);
        op_ldst16(cpu, cpu->U.w);
        break;

    default: return MC6809_FAULT_INTERNAL_ERROR;
    }

    return cpu->Fault;
}

/************************************************************************/

__declspec(dllexport) int __stdcall Get_6809_StructSize(void) { return sizeof(mc6809__t); }

__declspec(dllexport) uint64 __stdcall Get_6809_Clock(mc6809__t* cpu) { return cpu->Clock; }
__declspec(dllexport) uint8 __stdcall Get_6809_A(mc6809__t* cpu) { return cpu->A; }
__declspec(dllexport) uint8 __stdcall Get_6809_B(mc6809__t* cpu) { return cpu->B; }
__declspec(dllexport) uint8 __stdcall Get_6809_DP(mc6809__t* cpu) { return cpu->DP; }
__declspec(dllexport) uint16 __stdcall Get_6809_D(mc6809__t* cpu) { return cpu->D.w; }
__declspec(dllexport) uint16 __stdcall Get_6809_X(mc6809__t* cpu) { return cpu->X.w; }
__declspec(dllexport) uint16 __stdcall Get_6809_Y(mc6809__t* cpu) { return cpu->Y.w; }
__declspec(dllexport) uint16 __stdcall Get_6809_U(mc6809__t* cpu) { return cpu->U.w; }
__declspec(dllexport) uint16 __stdcall Get_6809_SP(mc6809__t* cpu) { return cpu->S.w; }
__declspec(dllexport) uint16 __stdcall Get_6809_PC(mc6809__t* cpu) { return cpu->PC.w; }
__declspec(dllexport) bool __stdcall Get_6809_NMI(mc6809__t* cpu) { return cpu->NMI; }
__declspec(dllexport) bool __stdcall Get_6809_FIRQ(mc6809__t* cpu) { return cpu->FIRQ; }
__declspec(dllexport) bool __stdcall Get_6809_IRQ(mc6809__t* cpu) { return cpu->IRQ; }

__declspec(dllexport) void __stdcall Set_6809_Clock(mc6809__t* cpu, uint64 val) { cpu->Clock = val; }
__declspec(dllexport) void __stdcall Set_6809_A(mc6809__t* cpu, uint8 val) { cpu->A = val; }
__declspec(dllexport) void __stdcall Set_6809_B(mc6809__t* cpu, uint8 val) { cpu->B = val; }
__declspec(dllexport) void __stdcall Set_6809_DP(mc6809__t* cpu, uint8 val) { cpu->DP = val; }
__declspec(dllexport) void __stdcall Set_6809_D(mc6809__t* cpu, uint16 val) { cpu->D.w = val; }
__declspec(dllexport) void __stdcall Set_6809_X(mc6809__t* cpu, uint16 val) { cpu->X.w = val; }
__declspec(dllexport) void __stdcall Set_6809_Y(mc6809__t* cpu, uint16 val) { cpu->Y.w = val; }
__declspec(dllexport) void __stdcall Set_6809_U(mc6809__t* cpu, uint16 val) { cpu->U.w = val; }
__declspec(dllexport) void __stdcall Set_6809_SP(mc6809__t* cpu, uint16 val) { cpu->S.w = val; }
__declspec(dllexport) void __stdcall Set_6809_PC(mc6809__t* cpu, uint16 val) { cpu->PC.w = val; }
__declspec(dllexport) void __stdcall Set_6809_NMI(mc6809__t* cpu, bool val) { cpu->NMI = val; }
__declspec(dllexport) void __stdcall Set_6809_FIRQ(mc6809__t* cpu, bool val) { cpu->FIRQ = val; }
__declspec(dllexport) void __stdcall Set_6809_IRQ(mc6809__t* cpu, bool val) { cpu->IRQ = val; }

__declspec(dllexport) uint8* __stdcall Get_6809_PageReadPointer(mc6809__t* cpu, uint8 page) { return cpu->ReadPointer[page]; }
__declspec(dllexport) uint8* __stdcall Get_6809_PageWritePointer(mc6809__t* cpu, uint8 page) { return cpu->WritePointer[page]; }

__declspec(dllexport) void __stdcall Set_6809_PageReadPointer(mc6809__t* cpu, uint8 page, uint8* ptr) { cpu->ReadPointer[page] = ptr; }
__declspec(dllexport) void __stdcall Set_6809_PageWritePointer(mc6809__t* cpu, uint8 page, uint8* ptr) { cpu->WritePointer[page] = ptr; }

__declspec(dllexport) pReadFunction __stdcall Get_6809_PageReadFunction(mc6809__t* cpu, uint8 page) { return cpu->ReadFunction[page]; }
__declspec(dllexport) pWriteFunction __stdcall Get_6809_PageWriteFunction(mc6809__t* cpu, uint8 page) { return cpu->WriteFunction[page]; }

__declspec(dllexport) void __stdcall Set_6809_PageReadFunction(mc6809__t* cpu, uint8 page, pReadFunction fptr) { cpu->ReadFunction[page] = fptr; }
__declspec(dllexport) void __stdcall Set_6809_PageWriteFunction(mc6809__t* cpu, uint8 page, pWriteFunction fptr) { cpu->WriteFunction[page] = fptr; }

/************************************************************************/

// thunks for the 6809 routines
__declspec(dllexport) void __stdcall M6809_Reset(mc6809__t* cpu, pPeriodicCallback callback, double step)
{
    cpu->CallbackCycles = (uint32)(step * 0x10000);
    cpu->NextCallback = cpu->CallbackCycles;
    cpu->PeriodicCallback = callback;
    mc6809_reset(cpu);
}

__declspec(dllexport) int __stdcall M6809_Run(mc6809__t* cpu, uint32 cyclesToRun)
{
    uint32 start = (uint32)cpu->Clock;
    uint32 end = start + cyclesToRun;

    cpu->Fault = 0;

    if (cpu->PeriodicCallback)
    {
        do
        {
            uint32 last = (uint32)cpu->Clock;

            // run until we halt or encounter an error
            MC6809_RESULT result = mc6809_step(cpu);
            if (result != MC6809_RESULT_OK)
                return result;

            uint32 delta = (uint32)cpu->Clock - last;
            cpu->NextCallback -= (delta << 16);
            if (cpu->NextCallback < 0)
            {
                cpu->NextCallback += cpu->CallbackCycles;
                cpu->PeriodicCallback();
            }
        } while ((int32)((uint32)cpu->Clock - end) < 0);
    }
    else
    {
        do
        {
            // run until we halt or encounter an error
            MC6809_RESULT result = mc6809_step(cpu);
            if (result != MC6809_RESULT_OK)
                return result;
        } while ((int32)((uint32)cpu->Clock - end) < 0);
    }

    return MC6809_RESULT_OK;
}

/************************************************************************/
