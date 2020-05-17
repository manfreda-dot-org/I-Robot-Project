// Copyright 1997, 1998, 2020 by John Manfreda. All Rights Reserved.
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

#include "IRobot.h"

enum { NO_MOVEMENT, LEFT, RIGHT, UP, DOWN };

static BYTE _inline GetKeyboardXaxis( void )
{
	short left, right;

	// left key
	left = GetAsyncKeyState( VK_LEFT )
		| GetAsyncKeyState( VK_NUMPAD1 )
		| GetAsyncKeyState( VK_NUMPAD4 )
		| GetAsyncKeyState( VK_NUMPAD7 );

	// right key
	right = GetAsyncKeyState( VK_RIGHT )
		| GetAsyncKeyState( VK_NUMPAD3 )
		| GetAsyncKeyState( VK_NUMPAD6 )
		| GetAsyncKeyState( VK_NUMPAD9 );

	if ((left ^ right) & 0x8000)
	{
		if (left & 0x8000)
			return LEFT;
		else
			return RIGHT;
	}

	return NO_MOVEMENT;
}

static BYTE _inline GetKeyboardYaxis( void )
{
	short up, down;

	// up key
	up = GetAsyncKeyState( VK_UP )
		| GetAsyncKeyState( VK_NUMPAD7 )
		| GetAsyncKeyState( VK_NUMPAD8 )
		| GetAsyncKeyState( VK_NUMPAD9 );

	// down key
	down = GetAsyncKeyState( VK_DOWN )
		| GetAsyncKeyState( VK_NUMPAD1 )
		| GetAsyncKeyState( VK_NUMPAD2 )
		| GetAsyncKeyState( VK_NUMPAD3 );

	if ((up ^ down) & 0x8000)
	{
		if (up & 0x8000)
			return UP;
		else
			return DOWN;
	}

	return NO_MOVEMENT;
}

static BYTE _inline GetAnalogJoystickXaxis( void )
{
	return HALL_JOY_MID + MAX_HALL_DELTA - (BYTE) ((JoyInfo.wXpos - JoyCaps.wXmin) * JoyScaleX);
}

static BYTE _inline GetAnalogJoystickYaxis( void )
{
	return (BYTE) ((JoyInfo.wYpos - JoyCaps.wYmin) * JoyScaleY) + HALL_JOY_MID - MAX_HALL_DELTA;
}

static BYTE _inline GetDigitalJoystickXaxis( void )
{
	if (JoyInfo.wXpos <= JoyLeftThreshold)
		return LEFT;

	if (JoyInfo.wXpos >= JoyRightThreshold)
		return RIGHT;

	return NO_MOVEMENT;
}

static BYTE _inline GetDigitalJoystickYaxis( void )
{
	if (JoyInfo.wYpos <= JoyUpThreshold)
		return UP;

	if (JoyInfo.wYpos >= JoyDownThreshold)
		return DOWN;

	return NO_MOVEMENT;
}

BYTE GetJoystickXAxis(void)
{
	static int LeftDelta = 0;
	static int RightDelta = 0;
	BYTE direction;
	
	if (GameControls == USE_ANALOG_JOYSTICK)
		return GetAnalogJoystickXaxis();
	
	if (GameControls == USE_DIGITAL_JOYSTICK)
		direction = GetDigitalJoystickXaxis();
	else
		direction = GetKeyboardXaxis();

	if (direction == LEFT)
	{
		RightDelta = 0;
//		RightDelta--;
		LeftDelta = min( MAX_HALL_DELTA*3, LeftDelta+2 );
	}
	else if (direction == RIGHT)
	{
		LeftDelta = 0;
//		LeftDelta--;
		RightDelta = min( MAX_HALL_DELTA*3, RightDelta+2 );
	}
	else
		LeftDelta = RightDelta = 0;

	return HALL_JOY_MID + (LeftDelta - RightDelta) / 3;
}

BYTE GetJoystickYAxis( void )
{
	static int UpDelta = 0;
	static int DownDelta = 0;
	BYTE direction;
	
	if (GameControls == USE_ANALOG_JOYSTICK)
		return GetAnalogJoystickYaxis();

	if (GameControls == USE_DIGITAL_JOYSTICK)
		direction = GetDigitalJoystickYaxis();
	else
		direction = GetKeyboardYaxis();
	
	if (direction == UP)
	{
		DownDelta = 0;
		UpDelta = min( MAX_HALL_DELTA*3, UpDelta+2 );
	}
	else if (direction == DOWN)
	{
		UpDelta = 0;
		DownDelta = min( MAX_HALL_DELTA*3, DownDelta+2 );
	}
	else
		UpDelta = DownDelta = 0;

	return HALL_JOY_MID + (DownDelta - UpDelta ) / 3;
}

BOOL IsFireButtonPressed( void )
{
	// joystick control
	if (GameControls != USE_KEYBOARD)
	{
		if (JoyInfo.wButtons & JOY_BUTTON1 )
			return TRUE;
	}
	// keyboard control
	else if (GetAsyncKeyState( KEY_FIRE1 ) & 0x8000)
		return TRUE;

	return FALSE;
}

BOOL IsStart1ButtonPressed( void )
{
	// joystick control
	if (GameControls != USE_KEYBOARD && JoyCaps.wMaxButtons >= 3)
	{
		if (JoyInfo.wButtons & JOY_BUTTON2 )
			return TRUE;
	}
	// keyboard control
	else if (GetAsyncKeyState( KEY_START1 ) & 0x8000)
		return TRUE;

	return FALSE;
}

BOOL IsStart2ButtonPressed( void )
{
	// joystick control
	if (GameControls != USE_KEYBOARD && JoyCaps.wMaxButtons >= 3)
	{
		if (JoyInfo.wButtons & JOY_BUTTON3 )
			return TRUE;
	}
	// keyboard control
	else if (GetAsyncKeyState( KEY_START2 ) & 0x8000)
		return TRUE;

	return FALSE;
}