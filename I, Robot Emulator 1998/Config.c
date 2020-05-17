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

typedef enum TAG_MENU_ITEM {
	MI_TEST = 0,
	MI_COIN,
	MI_LANGUAGE,
	MI_GAMETIME,
	MI_BONUS,
	MI_LIVES,
	MI_SKILL,
	MI_DEMO,
	MI_INPUT,
	MI_SOUND,
	MI_SYNC_RENDER,
	MI_SCREEN,
	MI_DOTS,
	MI_VECTORS,
	MI_POLYGONS,
	MI_ALPHAS,
	MI_FPS,
	MI_SPEED,
	MI_END } MENU_ITEM;

typedef struct _CONFIG_BLOCK {
	BYTE TestSwitch;
	BYTE Dipswitch3J;
	BYTE Dipswitch5E;
	BYTE ExtendedScreen;
	BYTE DisplayFPS;
	BYTE SpeedThrottling;
	BYTE ShowDots;
	BYTE ShowVectors;
	BYTE ShowPolygons;
	BYTE ShowAlphanumerics;
	BYTE RenderEveryFrame;
	BYTE SoundEnabled;
	BYTE GameControls; } CONFIG_BLOCK;

static MENU_ITEM CurrentMenuItem = 0;
static int FlashyColor = 0;

#define GetColor(item) ((CurrentMenuItem==item)?FlashyColor:GetBaseColor(item))
#define GetBaseColor(item) ((item<=MI_DEMO)?GREEN:BLUE)

static void DisplayExtras(void)
{
	
}

static void DrawRotatingBorder( int x, int y )
{
	static int PrevBlock = 32;
	static int PrevColor = 0;
	static int block = 32;
	static int color = 0;

	if (!x && !y)
	{
		block = PrevBlock;
		color = PrevColor;
	}

	if (block != 36)
		DisplayChar( block, x<<4, y<<4, color );

	if (++block > 36)
	{
		block = 32;
		color++;
		color &= 3;
	}

	if (!x && !y)
	{
		if (--PrevBlock < 32)
		{
			PrevBlock = 36;
			PrevColor += 3;
			PrevColor &= 3;
		}
	}
}

static void ModifyCurrentParameter( int step )
{
	switch(CurrentMenuItem)
	{
	case MI_SCREEN:		ExtendedScreen = !ExtendedScreen; ClipScreen = (ExtendedScreen) ? 0 : 5; break;
	case MI_FPS:		DisplayFPS = !DisplayFPS; break;
	case MI_SPEED:		SpeedThrottling = !SpeedThrottling; break;
	case MI_DOTS:		ShowDots = !ShowDots; break;
	case MI_VECTORS:	ShowVectors = !ShowVectors; break;
	case MI_POLYGONS:	ShowPolygons = !ShowPolygons; break;
	case MI_ALPHAS:		ShowAlphanumerics = !ShowAlphanumerics; break;
	case MI_TEST:		REG1000 ^= TESTSWITCH; break;
	case MI_INPUT:		if (JoystickPresent)
						{
							GameControls += 3 + step;
							GameControls %= 3;
						}
						break;
	case MI_SOUND:		if (SoundSupported)
							SoundEnabled = !SoundEnabled;
						break;
	case MI_COIN:		DIPSWITCH3J = (DIPSWITCH3J) ? 0 : 0xE0; break;
	case MI_LANGUAGE:	DIPSWITCH5E ^= 0x01; break;
	case MI_GAMETIME:	DIPSWITCH5E ^= 0x02; break;
	case MI_BONUS:		DIPSWITCH5E = (DIPSWITCH5E & 0xF3) | ((DIPSWITCH5E + (0x04 * step)) & 0x0C); break;
	case MI_LIVES:		DIPSWITCH5E = (DIPSWITCH5E & 0xCF) | ((DIPSWITCH5E + (0x10 * step)) & 0x30); break;
	case MI_SKILL:		DIPSWITCH5E ^= 0x40; break;
	case MI_DEMO:		DIPSWITCH5E ^= 0x80; break;
	case MI_SYNC_RENDER: RenderEveryFrame = !RenderEveryFrame; break;
	}
}

static void DisplayMenuItem( MENU_ITEM item, int x, int y )
{
	char * s1, *s2;

	switch (item)
	{
	case MI_SOUND:
		s1 = "     SOUND SUPPORT: ";
		if (SoundSupported)
			s2 = (SoundEnabled) ? "ENABLED" : "DISABLED";
		else
			s2 = "N-A";
		break;
	case MI_INPUT:
		s1 = "      INPUT DEVICE: ";
		switch(GameControls)
		{
		case USE_KEYBOARD:         s2 = "KEYBOARD"; break;
		case USE_DIGITAL_JOYSTICK: s2 = "DIGITAL JOYSTICK"; break;
		case USE_ANALOG_JOYSTICK:  s2 = "ANALOG JOYSTICK"; break;
		}
		break;
	case MI_TEST:
		s1 = "       TEST SWITCH: ";
		s2 = (REG1000 & TESTSWITCH) ? "OFF" : "ON";
		break;
	case MI_COIN:
		s1 = "      COIN OPTIONS: ";
		s2 = DIPSWITCH3J ? "FREE PLAY" : "1 COIN  1 PLAY";
		break;
	case MI_LANGUAGE:
		s1 = "          LANGUAGE: ";
		s2 = (DIPSWITCH5E & 0x01) ? "ENGLISH" : "GERMAN";
		break;
	case MI_GAMETIME:
		s1 = "  MINIMUM GAMETIME: ";
		s2 = (DIPSWITCH5E & 0x02) ? "OFF" : "ON";
		break;
	case MI_BONUS:
		s1 = "        EXTRA LIFE: ";
		switch( DIPSWITCH5E & 0x0C )
		{
		case 0x00: s2 = "EVERY 30000"; break;
		case 0x04: s2 = "EVERY 50000"; break;
		case 0x08: s2 = "NONE"; break;
		case 0x0C: s2 = "EVERY 20000"; break;
		}
		break;
	case MI_LIVES:
		s1 = "             LIVES: ";
		switch( DIPSWITCH5E & 0x30 )
		{
		case 0x00: s2 = "4"; break;
		case 0x10: s2 = "5"; break;
		case 0x20: s2 = "2"; break;
		case 0x30: s2 = "3"; break;
		}
		break;
	case MI_SKILL:
		s1 = "        DIFFICULTY: ";
		s2 = (DIPSWITCH5E & 0x40) ? "MEDIUM" : "EASY";
		break;
	case MI_DEMO:
	    s1 = "         DEMO MODE: ";
		s2 = (DIPSWITCH5E & 0x80) ? "OFF" : "ON";
		break;
	case MI_SCREEN:
		s1 = "            SCREEN: ";
		s2 = (ExtendedScreen) ? "EXTENDED" : "ORIGINAL";
		break;
	case MI_DOTS:
		s1 = "         SHOW DOTS: ";
		s2 = (ShowDots) ? "YES" : "NO";
		break;
	case MI_VECTORS:
		s1 = "      SHOW VECTORS: ";
		s2 = (ShowVectors) ? "YES" : "NO";
		break;
	case MI_POLYGONS:
		s1 = "     SHOW POLYGONS: ";
		s2 = (ShowPolygons) ? "YES" : "NO";
		break;
	case MI_ALPHAS:
		s1 = "SHOW ALPHANUMERICS: ";
		s2 = (ShowAlphanumerics) ? "YES" : "NO";
		break;
	case MI_FPS:
		s1 = "        FRAME RATE: ";
		s2 = (DisplayFPS) ? "SHOW" : "HIDE";
		break;
	case MI_SPEED:
		s1 = "  SPEED THROTTLING: ";
		s2 = (SpeedThrottling) ? "ON" : "OFF";
		break;	
	case MI_SYNC_RENDER:
		s1 = " VIDEO SYNC METHOD: ";
		s2 = (RenderEveryFrame) ? "61 FPS" : "MATHBOX";
		break;
	}

	x = DisplayString( s1, x, y, GetBaseColor(item) );
	DisplayString( s2, x, y, GetColor(item) );
}

static void DisplayMenu( void )
{
	int item, x, y;

	FlashyColor = (FlashyColor + 1) & 3;

	EraseScreenBuffer( lpBackBuffer );

	DisplayString( GlobalStrings[STR_MENU1], 32, 32, RED );
	DisplayString( GlobalStrings[STR_MENU2], 32, 48, RED );
	DisplayString( GlobalStrings[STR_MENU3], 32, 64, RED );
	DisplayString( GlobalStrings[STR_MENU4], 96, 88, YELLOW );

	y = 112;
	for (item = 0; item < MI_END; item++)
	{
		DisplayMenuItem( item, 48, y );
		y += 16 + 2;
		if (item == MI_DEMO)
			y += 8;
	}

	for (x=0; x<40; x++)
		DrawRotatingBorder( x, 0 );
	for (y=1; y<30; y++)
		DrawRotatingBorder( 39, y );
	for (x=38; x>=0; x--)
		DrawRotatingBorder( x, 29 );
	for (y=28; y>0; y--)
		DrawRotatingBorder( 0, y );
}

void RunConfigScreen( void )
{
	switch(LastKey)
	{
	case VK_UP:
		if (CurrentMenuItem == 0)
			CurrentMenuItem = MI_END;
		CurrentMenuItem--;
		break;
	case VK_DOWN:
		CurrentMenuItem++;
		if (CurrentMenuItem == MI_END)
			CurrentMenuItem = 0;
		break;
	case VK_LEFT:
	case VK_RIGHT:
		ModifyCurrentParameter( (LastKey==VK_LEFT) ? -1 : +1 );
		break;
	}
	LastKey = 0;

	DisplayExtras();

	DisplayMenu();

	if (lpFrontBuffer->lpVtbl->Flip( lpFrontBuffer, NULL, DDFLIP_WAIT ) == DDERR_SURFACELOST)
		RestorePrimaryBuffer();
}

void SaveDefaultConfig( void )
{
	CONFIG_BLOCK registry;
	HKEY key;
	DWORD temp;

	registry.TestSwitch = REG1000 & TESTSWITCH;
	registry.Dipswitch3J = DIPSWITCH3J;
	registry.Dipswitch5E = DIPSWITCH5E;
	registry.ExtendedScreen = ExtendedScreen;
	registry.DisplayFPS = DisplayFPS;
	registry.SpeedThrottling = SpeedThrottling;
	registry.ShowDots = ShowDots;
	registry.ShowVectors = ShowVectors;
	registry.ShowPolygons = ShowPolygons;
	registry.ShowAlphanumerics = ShowAlphanumerics;
	registry.RenderEveryFrame = RenderEveryFrame;
	registry.SoundEnabled = SoundEnabled;
	registry.GameControls = GameControls;

	// open/create the key
	RegCreateKeyEx(
		HKEY_CURRENT_USER,
		RegistrySubkey,
		0,
		REG_NONE,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL,
		&key,
		&temp );

	// write to the key
	RegSetValueEx(
		key,
		"",
		(DWORD) NULL,
		REG_BINARY,
		(BYTE *) &registry,
		sizeof( registry ) );

	// close the key
	RegCloseKey( key );
}

void LoadDefaultConfig( void )
{
	CONFIG_BLOCK registry;
	DWORD type;
	DWORD size = sizeof(registry);
	HKEY key;

	// attempt to locate the key
	if (RegOpenKeyEx( HKEY_CURRENT_USER, RegistrySubkey, 0, KEY_READ, &key )
		!= ERROR_SUCCESS )
		return;

	// try to locate configuration settings
	if (RegQueryValueEx(
		key,
		"",
		NULL,
		&type,
		(BYTE *) &registry,
		&size )
		== ERROR_SUCCESS )
	{
		// configuration exists, is it valid?
		if (type == REG_BINARY && size == sizeof(registry))
		{
			// configuration is valid, use it
			if (registry.TestSwitch)
				REG1000 |= TESTSWITCH;
			else
				REG1000 &= ~TESTSWITCH;
			DIPSWITCH3J = registry.Dipswitch3J;
			DIPSWITCH5E = registry.Dipswitch5E;
			ExtendedScreen = registry.ExtendedScreen;
			DisplayFPS = registry.DisplayFPS;
			SpeedThrottling = registry.SpeedThrottling;
			ShowDots = registry.ShowDots;
			ShowVectors = registry.ShowVectors;
			ShowPolygons = registry.ShowPolygons;
			ShowAlphanumerics = registry.ShowAlphanumerics;
			RenderEveryFrame = registry.RenderEveryFrame;
			if (SoundSupported)
				SoundEnabled = registry.SoundEnabled;
			if (JoystickPresent)
				GameControls = registry.GameControls;
		}
	}

	RegCloseKey( key );
}
