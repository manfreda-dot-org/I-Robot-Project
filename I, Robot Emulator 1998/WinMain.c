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

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow )
{
	MSG	msg;
	BOOL StartupOK;
	HANDLE hMutex;

	// Create a mutex to ensure only one copy of application is active
	hMutex = CreateMutex( NULL, TRUE, "I, Robot mutex" );
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		ReleaseMutex( hMutex );
		return 0;
	}

	ghInstance = hInstance;

	StartupOK = StartEmulationThread();

	if (StartupOK)
		StartupOK = StartApplication();

	if (StartupOK)
		StartupOK = StartVideoHardware();

	// only continue with program if system is capable of
	// running it
	if (StartupOK)
	{
		// load default configuration settings (if available)
		LoadDefaultConfig();
		
		// initialize sound emulation (if available)
		SoundEnabled = SoundSupported = StartWinPokey( ghwndMain );

		// ensure the executable is not modified before moving on
		ApplicationCorrupt = !EnsureExecutableIntegrity();

		// activate the emulation thread
		ResumeThread( gh6809Thread );

		// Message loop
		while ( GetMessage( &msg, NULL, 0, 0) )
			DispatchMessage( &msg );

		StopWinPokey();

		SaveDefaultConfig();

	}

	StopEmulationThread();
	StopVideoHardware();
	StopApplication();

	if (GlobalExitMessage[0] != 0)
		ErrorBox( GlobalExitMessage );

	// release the mutex so other instances can own it
	ReleaseMutex( hMutex );

	return msg.wParam;
}