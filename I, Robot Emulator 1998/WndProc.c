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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case MM_JOY1MOVE:
	case MM_JOY1BUTTONDOWN:
	case MM_JOY1BUTTONUP:
		JoyInfo.wButtons = wParam; 
		JoyInfo.wXpos = LOWORD(lParam); 
		JoyInfo.wYpos = HIWORD(lParam);
		return 0;
	case WM_USER:
		IRCloseClipboard();
		return 0;
/*		{
			static int flag = TRUE;
			FILE * fptr;
			if (flag)
			{
				fptr = fopen( "out.txt", "w" );
				fprintf(fptr, "Y CameraY DA EE\n");
			}
			else
				fptr = fopen( "out.txt", "a" );
			flag = FALSE;

			{
				int da = (short)MathRAMROM[0xDA/2];
				int ee = (short)MathRAMROM[0xEE/2];
				int Y = wParam;
				int CameraY = lParam;

				fprintf(fptr, "%d %d %d %d\n", Y, CameraY, da, ee );
			}
			fclose(fptr);
		}
		break;
*/
	
	case WM_NCPAINT:
		return 0;

	case WM_KEYDOWN:
		// save the last keypress if config screen is up
		if (ShowConfigScreen)
			LastKey = wParam;
		
		switch( wParam )
		{
		case KEY_LEFTCOIN:	// Left coin
			REG1000 &= ~LEFTCOIN;
			return 0;
		case KEY_RIGHTCOIN:	// Right coin
			REG1000 &= ~RIGHTCOIN;
			return 0;
		case KEY_AUXCOIN:	// Auxiliary coin
			REG1000 &= ~AUXCOIN;
			return 0;
	    case VK_ESCAPE:
            PostMessage( hWnd, WM_CLOSE, 0, 0 );
            return 0;
		}
		break;

	case WM_KEYUP:
		switch( wParam )
		{
		case VK_F1:
			ShowConfigScreen = !ShowConfigScreen;
			if (ShowConfigScreen)
				PauseWinPokey();
			else if (SoundEnabled && !Paused)
				PlayWinPokey();
			ClipScreen = 2;
			return 0;
		case KEY_LEFTCOIN:
			REG1000 |= LEFTCOIN;
			return 0;
		case KEY_RIGHTCOIN:
			REG1000 |= RIGHTCOIN;
			return 0;
		case KEY_AUXCOIN:
			REG1000 |= AUXCOIN;
			return 0;
		case VK_PAUSE:	// pause emulation
		case 'P':
			Paused = !Paused;
			if (Paused)
				PauseWinPokey();
			else if (SoundEnabled)
				PlayWinPokey();
			return 0;
		case 'C':		// copy screen to clipboard
			// CTRL+C was pressed
			if (GetAsyncKeyState( VK_CONTROL ) & 0x8000)
				IRGetClipboard();
			return 0;

/*		case 'D':
			{
				static int dumpnum = 0;
				char fname[256] = "dump0001.ir";
				FILE * fptr;
				wsprintf( fname, "dump%0.4d.ir", dumpnum );
				fptr = fopen( fname, "wb" );
				fwrite( &MathRAMROM[0x1000], 2, 0x1000, fptr );
//				fwrite( &MathRAMROM[0x0000], 2, 0x1000, fptr );
				fclose(fptr);
				dumpnum++;
			}
			break;
*/		}
		break;

	case WM_SYSCOMMAND:
		if (wParam == SC_SCREENSAVE)
			return 0;
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			ApplicationActive = FALSE;
			Paused = TRUE;
			PauseWinPokey();
		}
		else
			ApplicationActive = TRUE;
		break;;		

	case WM_SETCURSOR:
		SetCursor(NULL);
		return TRUE;

	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;

	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}