/*!
\brief Contains implementation details required by the Microsoft Windows version of PVRShell.
\file PVRShell/OS/Windows/WindowsOSData.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
namespace pvr {
namespace platform {
/// <summary>OS specific data for windows</summary>
struct WindowsOSData
{
	/// <summary>The mode that this window should be shown with (minimized, maximized etc.)</summary>
	int cmdShow;

	WindowsOSData() : cmdShow(SW_SHOW) {}
};
} // namespace platform
} // namespace pvr
