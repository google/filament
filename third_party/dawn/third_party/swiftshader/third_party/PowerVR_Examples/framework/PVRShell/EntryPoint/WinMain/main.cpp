/*!
\brief Application entry point for Microsoft Windows systems.
\file PVRShell/EntryPoint/WinMain/main.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#if defined(_WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "PVRShell/OS/ShellOS.h"
#include "PVRShell/StateMachine.h"
#include "PVRShell/OS/Windows/WindowsOSData.h"
#include <windows.h>
#include <io.h>

/// <summary>The entry point for Microsoft Windows (Windowed application). See the Win32 spec for
/// a detailed explanation of the function parameters.</summary>
/// <param name="hInstance">The Win32 Application Instance</param>
/// <param name="HINSTANCE">Unused</param>
/// <param name="lpCmdLine">The command line for the application, excluding program name</param>
/// <param name="nCmdShow">Controls how the window is to be shown. See the Win32 spec.</param>
/// <returns>0 on no error, otherwise 1</returns>
int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
#if defined(_WIN32) && defined(_DEBUG)
	// Enable memory-leak reports
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_EVERY_128_DF);

	/*
		To break on a specific block use

			_CrtSetBreakAlloc(block);

		Where block is 30145 in the below example

		{30145} normal block at 0x0000019947F28DD0, 56 bytes long.
		Data: <   G       G    > D0 8D F2 47 99 01 00 00 D0 8D F2 47 99 01 00 00
	*/
	//_CrtSetBreakAlloc(30145);
#endif
	int retval;
	{
		pvr::platform::WindowsOSData data;
		data.cmdShow = nCmdShow;

		pvr::platform::CommandLineParser commandLine;
		commandLine.set(lpCmdLine);

		pvr::platform::StateMachine stateMachine(hInstance, commandLine, &data);

		stateMachine.init();

		// Enter our loop
		retval = (stateMachine.execute() == pvr::Result::Success) ? 0 : 1;
	}
	return retval;
}
