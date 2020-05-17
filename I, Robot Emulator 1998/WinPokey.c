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

#include <windows.h>
#include <mmsystem.h>
#include "WinPokey.h"

// sampling rate is an even divison of FREQ_17_APPROX,
// in order to ensure good quality sound
#define SAMPLING_RATE (FREQ_17_APPROX/28/3)

// buffer is 100 milliseconds long
#define BUFFER_SIZE (SAMPLING_RATE * 100 / 1000)

static LPDIRECTSOUND lpDS = NULL;
static LPDIRECTSOUNDBUFFER lpStreamBuffer = NULL;
static LPDIRECTSOUNDBUFFER lpPrimaryBuffer = NULL;

static BOOL Paused = TRUE;
static BOOL Enabled = FALSE;

void ProcessSound( void )
{
	static DWORD WriteCursor = 0;
	DWORD PlayCursor;
	DWORD FreeBytes;
	DWORD size1, size2;
	BYTE *buf1, *buf2;

	if (Paused || !Enabled)
		return;

	// locate current audio play position
	if (lpStreamBuffer->lpVtbl->GetCurrentPosition( lpStreamBuffer, &PlayCursor, NULL ) != DS_OK)
		return;

	// determine number of free bytes in the buffer
	if (WriteCursor <= PlayCursor)
		FreeBytes = PlayCursor - WriteCursor;
	else
		FreeBytes = BUFFER_SIZE - WriteCursor + PlayCursor;

	if (!FreeBytes)
		return;

	// attempt to lock the buffer
	switch (lpStreamBuffer->lpVtbl->Lock( lpStreamBuffer, WriteCursor, FreeBytes, &buf1, &size1, &buf2, &size2, 0 ))
	{
	case DS_OK:
		// update sound information in the locked buffer(s)
		if (buf1)
			Pokey_process( buf1, size1 );
		if (buf2)
			Pokey_process( buf2, size2 );

		// update write cursor
		WriteCursor = (WriteCursor + size1 + size2) % BUFFER_SIZE;

		// unlock the buffer now that we're done
		lpStreamBuffer->lpVtbl->Unlock( lpStreamBuffer, buf1, size1, buf2, size2 );

		break;
	case DSERR_BUFFERLOST:
		// attempt to restore the buffer
		lpStreamBuffer->lpVtbl->Restore( lpStreamBuffer );
		break;
	}
}

BOOL StartWinPokey( HWND hWndOwner )
{
	DSBUFFERDESC dsbuf;
	PCMWAVEFORMAT pcmwf;

	// initialize Ron Fries' engine
	Pokey_sound_init( FREQ_17_APPROX, SAMPLING_RATE );

	// create direct sound object
	if(DirectSoundCreate(NULL, &lpDS, NULL) != DS_OK)
		return FALSE;

	// set cooperative level for direct sound
	if(lpDS->lpVtbl->SetCooperativeLevel( lpDS, hWndOwner, DSSCL_EXCLUSIVE ) != DS_OK)
	{
		StopWinPokey();
		return FALSE;
	}

	// set up wave format structure we'll use here
	memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT)); 
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels = 1; 
	pcmwf.wBitsPerSample = 8;
	pcmwf.wf.nBlockAlign = 1;
	pcmwf.wf.nSamplesPerSec = SAMPLING_RATE;
	pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;

	// locate the primary buffer
	memset(&dsbuf, 0, sizeof(DSBUFFERDESC));
	dsbuf.dwSize = sizeof(DSBUFFERDESC); 
	dsbuf.dwFlags = DSBCAPS_PRIMARYBUFFER;
    if (lpDS->lpVtbl->CreateSoundBuffer(lpDS, &dsbuf, &lpPrimaryBuffer, NULL) != DS_OK)
	{
		StopWinPokey();
		return FALSE;
	}

	// set the format of the primary buffer to match our streaming buffer
	lpPrimaryBuffer->lpVtbl->SetFormat(lpPrimaryBuffer, (LPWAVEFORMATEX) &pcmwf.wf);

	// create the secondary streaming buffer
	memset(&dsbuf, 0, sizeof(DSBUFFERDESC));
	dsbuf.dwSize = sizeof(DSBUFFERDESC); 
	dsbuf.dwBufferBytes = BUFFER_SIZE; 
    dsbuf.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;
    if (lpDS->lpVtbl->CreateSoundBuffer(lpDS, &dsbuf, &lpStreamBuffer, NULL) != DS_OK)
	{
		StopWinPokey();
		return FALSE;
	}

	// if we get here sound is ready to be played
	Enabled = TRUE;

	return TRUE;
}

void StopWinPokey( void )
{
	Paused = TRUE;
	Enabled = FALSE;

	// destroy any direct sound objects that may exist
	if (lpStreamBuffer)
	{
		lpStreamBuffer->lpVtbl->Stop( lpStreamBuffer );
		lpStreamBuffer->lpVtbl->Release( lpStreamBuffer );
		lpStreamBuffer = NULL;
	}
	if (lpPrimaryBuffer)
	{
		lpPrimaryBuffer->lpVtbl->Release( lpPrimaryBuffer );
		lpPrimaryBuffer = NULL;
	}
	if (lpDS)
	{
		lpDS->lpVtbl->Release( lpDS );
		lpDS = NULL;
	}
}

void PlayWinPokey( void )
{
	if (Enabled)
	{
		Paused = FALSE;
		lpStreamBuffer->lpVtbl->Play( lpStreamBuffer, 0, 0, DSBPLAY_LOOPING );
	}
}

void PauseWinPokey( void )
{
	if (Enabled)
	{
		lpStreamBuffer->lpVtbl->Stop( lpStreamBuffer );
		Paused = TRUE;
	}
}
