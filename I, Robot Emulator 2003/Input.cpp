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

#include "Input.h"
#include "Emulator.h"

//---------------------------------------------------------------------------
TGameInput GameInput;

static LPDIRECTINPUT8       pDI       = NULL; // the DirectInput object
static LPDIRECTINPUTDEVICE8 pMouse    = NULL; // the mouse device
static LPDIRECTINPUTDEVICE8 pKeyboard = NULL; // the keyboard device

static BYTE Keys[2][256];
static DIMOUSESTATE MouseState[2];
static bool BufNum;

//---------------------------------------------------------------------------
__fastcall TGameInput::TGameInput()
{
        ZeroMemory(Keys, sizeof(Keys));
        ZeroMemory(MouseState, sizeof(MouseState));
}

__fastcall TGameInput::~TGameInput()
{
        Destroy();
}

//---------------------------------------------------------------------------

void __fastcall TGameInput::Destroy(void)
{
        // Unacquire the devices one last time just in case
        // the app tried to exit while the device is still acquired.
        if (pMouse)
                pMouse->Unacquire();
        _RELEASE_(pMouse);
        if (pKeyboard)
                pKeyboard->Unacquire();
        _RELEASE_(pKeyboard);
        _RELEASE_(pDI);
}

bool __fastcall TGameInput::Create(TForm *form)
{
        // Create a DInput object
        if (FAILED(DirectInput8Create(
                GetModuleHandle(NULL),
                DIRECTINPUT_VERSION,
                IID_IDirectInput8,
                (VOID**)&pDI,
                NULL)))
                return false;

        // get mouse interface
        if (FAILED(pDI->CreateDevice(GUID_SysMouse, &pMouse, NULL)))
                return false;
        if (FAILED(pMouse->SetDataFormat(&c_dfDIMouse))) // predfined format
                return false;
        if (FAILED(pMouse->SetCooperativeLevel(form->Handle, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
                return false;

        // get keyboard interface
        if (FAILED(pDI->CreateDevice(GUID_SysKeyboard, &pKeyboard, NULL)))
                return false;

        // Set the data format to "keyboard format" - a predefined data format
        //
        // A data format specifies which controls on a device we
        // are interested in, and how they should be reported.
        //
        // This tells DirectInput that we will be passing an array
        // of 256 bytes to IDirectInputDevice::GetDeviceState.
        if (FAILED(pKeyboard->SetDataFormat(&c_dfDIKeyboard)))
                return false;

        // Set the cooperativity level to let DirectInput know how
        // this device should interact with the system and with other
        // DirectInput applications.
        if (FAILED(pKeyboard->SetCooperativeLevel(form->Handle, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY)))
                return false;

        // Acquire the newly created devices
        pMouse->Acquire();
        pKeyboard->Acquire();

        Log.Add("Input subsystem: OK");

        return true;
}

void __fastcall TGameInput::Update(void)
{
        // toggle buffer
        BufNum = !BufNum;

        // read mouse state
        if (FAILED(pMouse->GetDeviceState(sizeof(MouseState[0]), &MouseState[BufNum])))
        {
                pMouse->Acquire();
                ZeroMemory(MouseState, sizeof(MouseState));
                pMouse->GetDeviceState(sizeof(MouseState[0]),&MouseState[0]);
                pMouse->GetDeviceState(sizeof(MouseState[0]),&MouseState[1]);
        }

        // read keys
        if (FAILED(pKeyboard->GetDeviceState(sizeof(Keys[0]),&Keys[BufNum])))
        {
                pKeyboard->Acquire();
                ZeroMemory(Keys, sizeof(Keys));
                pKeyboard->GetDeviceState(sizeof(Keys[0]),&Keys[0]);
                pKeyboard->GetDeviceState(sizeof(Keys[0]),&Keys[1]);
        }

        // update hardware registers

        // update game I/O
        Hardware.Registers._1040 |= START1 | START2 | FIRE1;
        if (GetKeyState(DIK_1))
                Hardware.Registers._1040 &= ~START1;
        if (GetKeyState(DIK_2))
                Hardware.Registers._1040 &= ~START2;
        if (GetKeyState(DIK_SPACE))
                Hardware.Registers._1040 &= ~FIRE1;

        // update coin inputs
        Hardware.Registers._1000 |= LEFTCOIN | RIGHTCOIN | AUXCOIN;
        if (GetKeyState(DIK_F1))
                Hardware.Registers._1000 &= ~LEFTCOIN;
        if (GetKeyState(DIK_F2))
                Hardware.Registers._1000 &= ~RIGHTCOIN;
        if (GetKeyState(DIK_F3))
                Hardware.Registers._1000 &= ~AUXCOIN;

        // tab key toggles test mode
        if (GetKeyPress(DIK_TAB))
                Hardware.Registers._1000 ^= TESTSWITCH;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define HALL_JOY_MID 128
#define MAX_HALL_DELTA 48

int __fastcall TGameInput::GetKeyboardDirection(BYTE pos, BYTE neg)
{
        if (GetKeyState(pos))
        {
                if (!GetKeyState(neg))
                        return 1;
        }
        else if (GetKeyState(neg))
                return -1;
        return 0;
}

BYTE __fastcall TGameInput::GetJoystickX(void)
{
	static float delta = 0;

        switch (GetKeyboardDirection(DIK_LEFT, DIK_RIGHT))
        {
        default:
                delta = 0;
                break;
        case 1:
                if (delta < 0)
                        delta = 0;
                delta += 2.0/3.0;
                if (delta > MAX_HALL_DELTA)
                        delta = MAX_HALL_DELTA;
                break;
        case -1:
                if (delta > 0)
                        delta = 0;
                delta -= 2.0/3.0;
                if (delta < -MAX_HALL_DELTA)
                        delta = -MAX_HALL_DELTA;
                break;
        }

	return HALL_JOY_MID + delta;
}

BYTE __fastcall TGameInput::GetJoystickY(void)
{
        static float delta = 0;

        switch (GetKeyboardDirection(DIK_DOWN, DIK_UP))
        {
        default:
                delta = 0;
                break;
        case 1:
                if (delta < 0)
                        delta = 0;
                delta += 2.0/3.0;
                if (delta > MAX_HALL_DELTA)
                        delta = MAX_HALL_DELTA;
                break;
        case -1:
                if (delta > 0)
                        delta = 0;
                delta -= 2.0/3.0;
                if (delta < -MAX_HALL_DELTA)
                        delta = -MAX_HALL_DELTA;
                break;
        }

        return HALL_JOY_MID + delta;
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TPoint __fastcall TGameInput::MousePos(void)           { return Mouse->CursorPos; }
BYTE __fastcall TGameInput::GetMouseClickState(void)   { return MouseState[BufNum].rgbButtons[0] & 0x80; }
BYTE __fastcall TGameInput::GetMouseClickEvent(void)   { return (MouseState[0].rgbButtons[0] ^ MouseState[1].rgbButtons[0]) & 0x80; }
BYTE __fastcall TGameInput::GetMouseClickPress(void)   { return (MouseState[0].rgbButtons[0] ^ MouseState[1].rgbButtons[0]) & MouseState[BufNum].rgbButtons[0] & 0x80; }
BYTE __fastcall TGameInput::GetMouseClickRelease(void) { return (MouseState[0].rgbButtons[0] ^ MouseState[1].rgbButtons[0]) & MouseState[BufNum].rgbButtons[0] & 0x80; }

BYTE __fastcall TGameInput::GetKeyState(BYTE key)   { return Keys[BufNum][key] & 0x80; }
BYTE __fastcall TGameInput::GetKeyEvent(BYTE key)   { return (Keys[0][key] ^ Keys[1][key]) & 0x80; }
BYTE __fastcall TGameInput::GetKeyPress(BYTE key)   { return (Keys[0][key] ^ Keys[1][key]) & Keys[BufNum][key] & 0x80; }
BYTE __fastcall TGameInput::GetKeyRelease(BYTE key) { return (Keys[0][key] ^ Keys[1][key]) & ~Keys[BufNum][key] & 0x80; }

//---------------------------------------------------------------------------


