/* ========================================================================
   $File: EntryPoint.h$
   $Date: 3/22/2021$
   $Revision: 2$
   $Creator: Brian Montgomery $
   $Notice: (C) Copyright 2021 by Monty, Inc. All Rights Reserved. $
   ======================================================================== */

#pragma once

#include "HandmadePCH.h"
#include "Windows.h"
#include "XInput.h"
#include "mmeapi.h"
#include "DSound.h"


//#define static "local_persist"
#define internal static
#define global_variable static
#define local_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int32 bool32;

struct Win32OffscreenBuffer {
	BITMAPINFO info;
	void *memory;
	int width;
	int height;
	int pitch;
	int bytesPerPixel;
};

struct Win32WindowDimension {
	int width;
	int height;
};

Win32WindowDimension win32GetWindowDimension(HWND window)
{
	Win32WindowDimension result;

	RECT clientRect;
	GetClientRect(window, &clientRect);
	result.width = clientRect.right - clientRect.left;
	result.height = clientRect.bottom - clientRect.top;

	return result;
}

//TODO: Take out of global
global_variable bool globalrunning;
global_variable Win32OffscreenBuffer globalBackBuffer;

//XInputGetState Loading
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(xInputGetStateStub)
{
	return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *xInputGetState_ = xInputGetStateStub;
#define XInputGetState xInputGetState_

//XInputSetState Loading
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(xInputSetStateStub)
{
	return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *xInputSetState_ = xInputSetStateStub;
#define XInputSetState xInputSetState_

//DSoundCreate Loading
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void win32LoadXInput(void)
{
	HMODULE xInputLibrary = LoadLibraryA("xinput1_4.dll");
	if (!xInputLibrary) {
		xInputLibrary = LoadLibraryA("xinput1_3.dll");
	}
	if (xInputLibrary)
	{
		XInputGetState = (x_input_get_state *)GetProcAddress(xInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state *)GetProcAddress(xInputLibrary, "XInputSetState");
	}
}

internal void win32InitDSound(HWND window, int32 samplesPerSecond, int32 bufferSize)
{
	//load the library
	HMODULE dSoundLibrary = LoadLibraryA("dsound.dll");

	if (dSoundLibrary)
	{
		//get a directsound object! - cooperative
		direct_sound_create *directSoundCreate = (direct_sound_create *)GetProcAddress(dSoundLibrary, "DirectSoundCreate");
		LPDIRECTSOUND directSound;
		if (directSoundCreate && SUCCEEDED(directSoundCreate(0, &directSound, 0)))
		{
			WAVEFORMATEX waveFormat = {};
			waveFormat.wFormatTag = WAVE_FORMAT_PCM;
			waveFormat.nChannels = 2;
			waveFormat.nSamplesPerSec = samplesPerSecond;
			waveFormat.wBitsPerSample = 16;
			waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
			waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
			waveFormat.cbSize = 0;

			if (SUCCEEDED(directSound->SetCooperativeLevel(window, DSSCL_PRIORITY)))
			{
				//create a primary buffer
				DSBUFFERDESC bufferDescription = {};
				bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
				bufferDescription.dwSize = sizeof(bufferDescription);

				LPDIRECTSOUNDBUFFER primaryBuffer;
				if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0)))
				{
					if (SUCCEEDED(primaryBuffer->SetFormat(&waveFormat))) 
					{

					}
					else {

					}
				}
			}

			//create a secondary buffer
			DSBUFFERDESC bufferDescription = {};
			bufferDescription.dwFlags = 0;
			bufferDescription.dwSize = sizeof(bufferDescription);
			bufferDescription.dwSize = bufferSize;
			bufferDescription.lpwfxFormat = &waveFormat;

			LPDIRECTSOUNDBUFFER secondaryBuffer;
			if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &secondaryBuffer, 0)))
			{

				if (SUCCEEDED(secondaryBuffer->SetFormat(&waveFormat)))
				{

				}
				else {

				}
			}
			//start playing
			else {

			}
		}
		else {

		}
	}
	else {

	}
}

internal void renderWeirdGradient(Win32OffscreenBuffer *buffer, int xOffset, int yOffset)
{
	uint8 *row = (uint8 *)buffer->memory;
	for (int y = 0; y < buffer->height; ++y)
	{
		uint32 *pixel = (uint32 *)row;
		for (int x = 0; x < buffer->width; ++x)
		{
			uint8 blue = (uint8)(x + xOffset);
			uint8 green = (uint8)(y + yOffset);

			*pixel++ = ((green << 8) | blue);
		}

		row += buffer->pitch;
	}
}

internal void win32ResizeDIBSection(Win32OffscreenBuffer *buffer, int width, int height)
{	
	if (buffer->memory)
	{
		VirtualFree(buffer->memory, 0, MEM_RELEASE);
	}

	buffer->width = width;
	buffer->height = height;
	buffer->bytesPerPixel = 4;

	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = buffer->width;
	buffer->info.bmiHeader.biHeight = -buffer->height;
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;

	int bitmapMemorySize = buffer->bytesPerPixel * (buffer->width * buffer->height);
	buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

	buffer->pitch = buffer->width * buffer->bytesPerPixel;
}

internal void win32CopyBufferToWindow(Win32OffscreenBuffer *buffer, HDC deviceContext, int width, int height)
{
	StretchDIBits(deviceContext, 0, 0, width, height, 0, 0, buffer->width, buffer->height, buffer->memory, &buffer->info,
						DIB_RGB_COLORS, SRCCOPY);
}

internal LRESULT CALLBACK
win32MainWindowCallback(HWND window,
				   UINT message,
				   WPARAM wParam,
				   LPARAM lParam)
{
	LRESULT result = 0; 

	switch(message)
	{
		case WM_SIZE:
		{
			OutputDebugStringA("WM_SIZE\n");
		} break;		
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint32 VKCode = (uint32)wParam;
			bool wasDown = ((lParam & (1 << 30)) != 0);
			bool isDown = ((lParam & (1 << 31)) == 0);

			if (isDown != wasDown)
			{
				if (VKCode == 'W') {

				}
				else if (VKCode == 'A') {

				}
				else if (VKCode == 'S') {

				}
				else if (VKCode == 'D') {

				}
				else if (VKCode == 'Q') {

				}
				else if (VKCode == 'E') {

				}
				else if (VKCode == VK_UP) {

				}
				else if (VKCode == VK_DOWN) {

				}
				else if (VKCode == VK_LEFT) {

				}
				else if (VKCode == VK_RIGHT) {

				}
				else if (VKCode == VK_SPACE) {

				}
				else if (VKCode == VK_ESCAPE) {

				}
			}

			bool32 altKeyWasDown = (lParam & (1 << 29));
			if ((VKCode == VK_F4) && altKeyWasDown) 
			{
				globalrunning = false;
			}

		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC deviceContext = BeginPaint(window, &paint);
			Win32WindowDimension dimension = win32GetWindowDimension(window);
			win32CopyBufferToWindow(&globalBackBuffer, deviceContext, dimension.width, dimension.height);
			EndPaint(window, &paint);

			OutputDebugStringA("WM_PAINT\n");
		} break;
		case WM_CLOSE:
		{
			globalrunning = false;
			OutputDebugStringA("WM_CLOSE\n");
		} break;
		case WM_DESTROY:
		{
			//TODO: Handle as error
			globalrunning = false;
			OutputDebugStringA("WM_DESTROY\n");
		} break;
		default:
		{
			// OutputDebugString("default\n");
			result = DefWindowProc(window, message, wParam, lParam);
		} break;
		
	}

	return result;
}

int CALLBACK
WinMain(HINSTANCE instance,
		HINSTANCE prevInstance,
		LPSTR cmdLine,
		int cmdShow)
{
	win32LoadXInput();

	WNDCLASSA windowClass = {};

	win32ResizeDIBSection(&globalBackBuffer, 1280, 720);

	windowClass.style = CS_HREDRAW|CS_VREDRAW;
	windowClass.lpfnWndProc = win32MainWindowCallback;
	windowClass.hInstance = instance; //or GetModuleHandle(0) in WINAPI
	//  WindowClass.hIcon;
	windowClass.lpszClassName = "HandmadeWindowClass";

	if (RegisterClassA(&windowClass))
	{
		HWND window = CreateWindowExA(0, windowClass.lpszClassName, "Handmade", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);

		if (window)
		{
			globalrunning = true;

			int xOffset = 0;
			int yOffset = 0;

			win32InitDSound(window, 48000, 48000 * sizeof(int16) * 2);

			while(globalrunning)
			{
				MSG message;
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
				{
					if (message.message == WM_QUIT)
						globalrunning = false;
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				
				for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex)
				{
					XINPUT_STATE controllerState;
					if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) 
					{
						XINPUT_GAMEPAD *pad = &controllerState.Gamepad;

						bool up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool start = (pad->wButtons & XINPUT_GAMEPAD_START);
						bool back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
						bool leftShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool rightShoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool aButton = (pad->wButtons & XINPUT_GAMEPAD_A);
						bool bButton = (pad->wButtons & XINPUT_GAMEPAD_B);
						bool xButton = (pad->wButtons & XINPUT_GAMEPAD_X);
						bool yButton = (pad->wButtons & XINPUT_GAMEPAD_Y);

						int16 stickX = pad->sThumbLX;
						int16 stickY = pad->sThumbLY;
					}
					else {
						// TODO controller not available
					}
				}

				XINPUT_VIBRATION vibration;
				vibration.wLeftMotorSpeed = 60000;
				vibration.wRightMotorSpeed = 60000;
				XInputSetState(0, &vibration);

				HDC deviceContext = GetDC(window);
				Win32WindowDimension dimension = win32GetWindowDimension(window);

				renderWeirdGradient(&globalBackBuffer, xOffset, yOffset);
				win32CopyBufferToWindow(&globalBackBuffer, deviceContext, dimension.width, dimension.height);
				ReleaseDC(window, deviceContext);
				++xOffset;
			}
		}
		else {
			//TODO Logging "Failed to create window handle!"
		}
	}
	else {
		// TODO Logging "Failed to register window class!"
	}

	return 0;
}
		
