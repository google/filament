//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils_win.cpp: Implementation of OS-specific functions for Windows

#include "system_utils.h"

#include <fileapi.h>
#include <stdarg.h>
#include <windows.h>
#include <algorithm>
#include <array>
#include <vector>

namespace angle
{

namespace
{

std::string GetPath(HMODULE module)
{
    std::array<wchar_t, MAX_PATH> executableFileBuf;
    DWORD executablePathLen = GetModuleFileNameW(module, executableFileBuf.data(),
                                                 static_cast<DWORD>(executableFileBuf.size()));
    return Narrow(executablePathLen > 0 ? executableFileBuf.data() : L"");
}

std::string GetDirectory(HMODULE module)
{
    std::string executablePath = GetPath(module);
    return StripFilenameFromPath(executablePath);
}

}  // anonymous namespace

std::string GetExecutablePath()
{
    return GetPath(nullptr);
}

std::string GetExecutableDirectory()
{
    return GetDirectory(nullptr);
}

const char *GetSharedLibraryExtension()
{
    return "dll";
}

Optional<std::string> GetCWD()
{
    std::array<wchar_t, MAX_PATH> pathBuf;
    DWORD result = GetCurrentDirectoryW(static_cast<DWORD>(pathBuf.size()), pathBuf.data());
    if (result == 0)
    {
        return Optional<std::string>::Invalid();
    }
    return Narrow(pathBuf.data());
}

bool SetCWD(const char *dirName)
{
    return (SetCurrentDirectoryW(Widen(dirName).c_str()) == TRUE);
}

const char *GetPathSeparatorForEnvironmentVar()
{
    return ";";
}

double GetCurrentSystemTime()
{
    LARGE_INTEGER frequency = {};
    QueryPerformanceFrequency(&frequency);

    LARGE_INTEGER curTime;
    QueryPerformanceCounter(&curTime);

    return static_cast<double>(curTime.QuadPart) / frequency.QuadPart;
}

double GetCurrentProcessCpuTime()
{
    FILETIME creationTime = {};
    FILETIME exitTime     = {};
    FILETIME kernelTime   = {};
    FILETIME userTime     = {};

    // Note this will not give accurate results if the current thread is
    // scheduled for less than the tick rate, which is often 15 ms. In that
    // case, GetProcessTimes will not return different values, making it
    // possible to end up with 0 ms for a process that takes 93% of a core
    // (14/15 ms)!  An alternative is QueryProcessCycleTime but there is no
    // simple way to convert cycles back to seconds, and on top of that, it's
    // not supported pre-Windows Vista.

    // Returns 100-ns intervals, so we want to divide by 1e7 to get seconds
    GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime);

    ULARGE_INTEGER kernelInt64;
    kernelInt64.LowPart      = kernelTime.dwLowDateTime;
    kernelInt64.HighPart     = kernelTime.dwHighDateTime;
    double systemTimeSeconds = static_cast<double>(kernelInt64.QuadPart) * 1e-7;

    ULARGE_INTEGER userInt64;
    userInt64.LowPart      = userTime.dwLowDateTime;
    userInt64.HighPart     = userTime.dwHighDateTime;
    double userTimeSeconds = static_cast<double>(userInt64.QuadPart) * 1e-7;

    return systemTimeSeconds + userTimeSeconds;
}

bool IsDirectory(const char *filename)
{
    WIN32_FILE_ATTRIBUTE_DATA fileInformation;

    BOOL result =
        GetFileAttributesExW(Widen(filename).c_str(), GetFileExInfoStandard, &fileInformation);
    if (result)
    {
        DWORD attribs = fileInformation.dwFileAttributes;
        return (attribs != INVALID_FILE_ATTRIBUTES) && ((attribs & FILE_ATTRIBUTE_DIRECTORY) > 0);
    }

    return false;
}

bool IsDebuggerAttached()
{
    return !!::IsDebuggerPresent();
}

void BreakDebugger()
{
    __debugbreak();
}

const char *GetExecutableExtension()
{
    return ".exe";
}

char GetPathSeparator()
{
    return '\\';
}

std::string GetModuleDirectory()
{
// GetModuleHandleEx is unavailable on UWP
#if !defined(ANGLE_IS_WINUWP)
    static int placeholderSymbol = 0;
    HMODULE module               = nullptr;
    if (GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&placeholderSymbol), &module))
    {
        return GetDirectory(module);
    }
#endif
    return GetDirectory(nullptr);
}

std::string GetRootDirectory()
{
    return "C:\\";
}

bool CreateDirectories(const std::string &path)
{
    // First sanitize path so we can use "/" as universal path separator
    std::string sanitizedPath(path);
    MakeForwardSlashThePathSeparator(sanitizedPath);

    size_t pos = 0;
    do
    {
        pos = sanitizedPath.find("/", pos);
        std::string checkPath(sanitizedPath.substr(0, pos));
        if (!checkPath.empty() && !IsDirectory(checkPath.c_str()))
        {
            if (!CreateDirectoryW(Widen(checkPath).c_str(), nullptr))
            {
                return false;
            }
        }
        if (pos == std::string::npos)
        {
            break;
        }
        ++pos;
    } while (true);
    return true;
}

void MakeForwardSlashThePathSeparator(std::string &path)
{
    std::replace(path.begin(), path.end(), '\\', '/');
    return;
}

Optional<std::string> GetTempDirectory()
{
    char tempDirOut[MAX_PATH + 1];
    GetTempPathA(MAX_PATH + 1, tempDirOut);
    std::string tempDir = std::string(tempDirOut);

    if (tempDir.length() < 0 || tempDir.length() > MAX_PATH)
    {
        return Optional<std::string>::Invalid();
    }

    if (tempDir.length() > 0 && tempDir.back() == '\\')
    {
        tempDir.pop_back();
    }

    return tempDir;
}

Optional<std::string> CreateTemporaryFileInDirectory(const std::string &directory)
{
    char fileName[MAX_PATH + 1];
    if (GetTempFileNameA(directory.c_str(), "ANGLE", 0, fileName) == 0)
        return Optional<std::string>::Invalid();

    return std::string(fileName);
}

std::string GetLibraryPath(void *libraryHandle)
{
    if (!libraryHandle)
    {
        return "";
    }

    std::array<wchar_t, MAX_PATH> buffer;
    if (GetModuleFileNameW(reinterpret_cast<HMODULE>(libraryHandle), buffer.data(),
                           buffer.size()) == 0)
    {
        return "";
    }

    return Narrow(buffer.data());
}

void *GetLibrarySymbol(void *libraryHandle, const char *symbolName)
{
    if (!libraryHandle)
    {
        fprintf(stderr, "Module was not loaded\n");
        return nullptr;
    }

    return reinterpret_cast<void *>(
        GetProcAddress(reinterpret_cast<HMODULE>(libraryHandle), symbolName));
}

void CloseSystemLibrary(void *libraryHandle)
{
    if (libraryHandle)
    {
        FreeLibrary(reinterpret_cast<HMODULE>(libraryHandle));
    }
}
std::string Narrow(const std::wstring_view &utf16)
{
    if (utf16.empty())
    {
        return {};
    }
    int requiredSize = WideCharToMultiByte(CP_UTF8, 0, utf16.data(), static_cast<int>(utf16.size()),
                                           nullptr, 0, nullptr, nullptr);
    std::string utf8(requiredSize, '\0');
    WideCharToMultiByte(CP_UTF8, 0, utf16.data(), static_cast<int>(utf16.size()), &utf8[0],
                        requiredSize, nullptr, nullptr);
    return utf8;
}

std::wstring Widen(const std::string_view &utf8)
{
    if (utf8.empty())
    {
        return {};
    }
    int requiredSize =
        MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring utf16(requiredSize, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), &utf16[0],
                        requiredSize);
    return utf16;
}

void SetCurrentThreadName(const char *name)
{
    // Not implemented
}
}  // namespace angle
