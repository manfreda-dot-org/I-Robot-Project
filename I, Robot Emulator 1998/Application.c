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
#include "resource.h"

BOOL StartApplication( void )
{
	WNDCLASS WndClass;
	UINT delta;

	srand( GetTickCount() );

	// Register the window class for the main window
	ZeroMemory( &WndClass, sizeof(WNDCLASS) );
	WndClass.style         = CS_NOCLOSE;
	WndClass.lpfnWndProc   = (WNDPROC) WndProc;
	WndClass.hInstance     = ghInstance;
	WndClass.hIcon         = LoadIcon( ghInstance, MAKEINTRESOURCE(IDI_IROBOT) );
	WndClass.hCursor       = LoadCursor( NULL, IDC_ARROW );
	WndClass.hbrBackground = GetStockObject( BLACK_BRUSH );
	WndClass.lpszMenuName  = NULL;
	WndClass.lpszClassName = "I, Robot emulator";
	if (!RegisterClass( &WndClass ))
		return ErrorBox( "Can't register the program window class" );

	// Create the program window
	ghwndMain = CreateWindow( "I, Robot emulator", "I, Robot", WS_DLGFRAME | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, ghInstance, NULL);
	if (!ghwndMain)
		return ErrorBox( "Problem creating the program window" );
	ShowWindow( ghwndMain, SW_MAXIMIZE );

	// controls default to keyboard
	GameControls = USE_KEYBOARD;

	// capture joystick and determine if there is joystick support
	if (joySetCapture( ghwndMain, JOYSTICKID1, 20, TRUE ) == JOYERR_NOERROR )
	{
		JoystickPresent = TRUE;
		GameControls = USE_DIGITAL_JOYSTICK;
		joyGetPos( JOYSTICKID1, &JoyInfo );
		joyGetDevCaps( JOYSTICKID1, &JoyCaps, sizeof(JoyCaps) );
	
		// calculate scale factors for joystick -> hall effect
		JoyScaleX = (2 * MAX_HALL_DELTA + 1) / ((float)(JoyCaps.wXmax - JoyCaps.wXmin + 1));
		JoyScaleY =	(2 * MAX_HALL_DELTA + 1) / ((float)(JoyCaps.wYmax - JoyCaps.wYmin + 1));

		// calculate joystick thresholds
		delta = (JoyCaps.wYmax - JoyCaps.wYmin + 1) / 3;
		JoyUpThreshold = JoyCaps.wYmin + delta;
		JoyDownThreshold = JoyCaps.wYmax - delta;
		delta = (JoyCaps.wXmax - JoyCaps.wXmin + 1) / 3;
		JoyLeftThreshold = JoyCaps.wXmin + delta;
		JoyRightThreshold = JoyCaps.wXmax - delta;
	}

	return TRUE;
}

void StopApplication( void )
{
}
