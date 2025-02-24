/*!
\brief A Stream implementation used to access Windows embedded resources.
\file PVRCore/Windows/WindowsResourceStream.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/stream/BufferStream.h"
#include <string>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace pvr {
/// <summary>A Stream implementation that is used to access resources built in a MS Windows executable.</summary>
/// <remarks>This Stream abstraction allows the user to easily access the Resources embedded in Microsoft Windows
/// .exe/.dll files created using Windows Resource Files. This is the default way resources are packed in the MS
/// Windows version of the PowerVR Examples.</remarks>
class WindowsResourceStream : public BufferStream
{
public:
	/// <summary>Constructor. Creates a new WindowsResourceStream from a Windows Embedded Resource.</summary>
	/// <param name="resourceName">The "filename". must be the same as the identifier of the Windows Embedded Resource</param>
	WindowsResourceStream(const std::string& resourceName, bool errorOnNotFound = true) : BufferStream(resourceName)
	{
		_isReadable = false;
		_isWritable = false;
		_isRandomAccess = false;

		// Find a handle to the resource
		HRSRC hR = FindResource(GetModuleHandle(NULL), resourceName.c_str(), RT_RCDATA);

		if (!hR)
		{
			if (errorOnNotFound) { throw FileNotFoundError(resourceName); }
			return;
		}
		// Get a global handle to the resource data, which allows us to query the data pointer
		HGLOBAL hG = LoadResource(NULL, hR);

		if (!hG)
		{
			if (errorOnNotFound) { throw FileNotFoundError(resourceName); }
			return;
		}
		// Get the data pointer itself. NB: Does not actually lock anything.
		_originalData = LockResource(hG);
		_currentPointer = _originalData;
		_bufferSize = SizeofResource(NULL, hR);
		_isReadable = true;
		_isWritable = false;
		_isRandomAccess = true;
		return;
	}
}; // namespace pvr
} // namespace pvr
