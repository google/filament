//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// platform_helpers.cpp: implementations for platform identification functions
// which require runtime detections.

#include "common/platform_helpers.h"
#include "common/debug.h"

#include <tuple>

#ifdef ANGLE_PLATFORM_WINDOWS
#    include <windows.h>
#endif

namespace angle
{

namespace
{

// Windows version constants, for range check functions
constexpr VersionTriple kVersionWindowsXP    = VersionTriple(5, 1, 0);
constexpr VersionTriple kVersionWindowsVista = VersionTriple(6, 0, 0);
constexpr VersionTriple kVersionWindows7     = VersionTriple(6, 1, 0);
constexpr VersionTriple kVersionWindows8     = VersionTriple(6, 2, 0);
constexpr VersionTriple kVersionWindows10    = VersionTriple(10, 0, 0);
constexpr VersionTriple kVersionWindows11    = VersionTriple(10, 0, 22000);

bool IsWindowsVersionOrLater(VersionTriple greaterOrEqual)
{
#if defined(ANGLE_PLATFORM_WINDOWS)
#    if defined(ANGLE_ENABLE_WINDOWS_UWP)
    // Windows UWP does not provide access to the VerifyVersionInfo API, nor
    // the versionhelpers.h functions. To work around this, always treat UWP
    // applications as running on Windows 10 (which is when UWP was
    // introduced).
    return greaterOrEqual >= kVersionWindows10;
#    else
    OSVERSIONINFOEXW osvi;
    DWORDLONG dwlConditionMask = 0;

    dwlConditionMask = VerSetConditionMask(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    dwlConditionMask = VerSetConditionMask(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
    dwlConditionMask = VerSetConditionMask(dwlConditionMask, VER_BUILDNUMBER, VER_GREATER_EQUAL);

    osvi                     = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion      = greaterOrEqual.majorVersion;
    osvi.dwMinorVersion      = greaterOrEqual.minorVersion;
    osvi.dwBuildNumber       = greaterOrEqual.patchVersion;

    return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER,
                              dwlConditionMask) != FALSE;
#    endif  // ANGLE_ENABLE_WINDOWS_UWP
#else
    return false;
#endif  // ANGLE_PLATFORM_WINDOWS
}

bool IsWindowsVersionInRange(VersionTriple greaterOrEqual, VersionTriple lessThan)
{
    // This function is checking (greaterOrEqual <= WindowsVersion < lessThan),
    // for when you need to match a specific Windows release.
    ASSERT(greaterOrEqual < lessThan);
    return IsWindowsVersionOrLater(greaterOrEqual) && !IsWindowsVersionOrLater(lessThan);
}

}  // namespace

bool operator==(const VersionTriple &a, const VersionTriple &b)
{
    return std::tie(a.majorVersion, a.minorVersion, a.patchVersion) ==
           std::tie(b.majorVersion, b.minorVersion, b.patchVersion);
}
bool operator!=(const VersionTriple &a, const VersionTriple &b)
{
    return std::tie(a.majorVersion, a.minorVersion, a.patchVersion) !=
           std::tie(b.majorVersion, b.minorVersion, b.patchVersion);
}
bool operator<(const VersionTriple &a, const VersionTriple &b)
{
    return std::tie(a.majorVersion, a.minorVersion, a.patchVersion) <
           std::tie(b.majorVersion, b.minorVersion, b.patchVersion);
}
bool operator>=(const VersionTriple &a, const VersionTriple &b)
{
    return std::tie(a.majorVersion, a.minorVersion, a.patchVersion) >=
           std::tie(b.majorVersion, b.minorVersion, b.patchVersion);
}

//
// Exact Windows version checks
//

bool IsWindowsXP()
{
    return IsWindowsVersionInRange(kVersionWindowsXP, kVersionWindowsVista);
}

bool IsWindowsVista()
{
    return IsWindowsVersionInRange(kVersionWindowsVista, kVersionWindows7);
}

bool IsWindows7()
{
    return IsWindowsVersionInRange(kVersionWindows7, kVersionWindows8);
}

bool IsWindows8()
{
    return IsWindowsVersionInRange(kVersionWindows8, kVersionWindows10);
}

bool IsWindows10()
{
    return IsWindowsVersionInRange(kVersionWindows10, kVersionWindows11);
}

bool IsWindows11()
{
    // There's no post-Windows 11 release yet, so this is functionally a
    // "Windows 11 or later" check right now.
    return IsWindowsVersionOrLater(kVersionWindows11);
}

//
// Windows version or later helpers
//

bool IsWindowsXPOrLater()
{
    return IsWindowsVersionOrLater(kVersionWindowsXP);
}

bool IsWindowsVistaOrLater()
{
    return IsWindowsVersionOrLater(kVersionWindowsVista);
}

bool IsWindows7OrLater()
{
    return IsWindowsVersionOrLater(kVersionWindows7);
}

bool IsWindows8OrLater()
{
    return IsWindowsVersionOrLater(kVersionWindows8);
}

bool IsWindows10OrLater()
{
    return IsWindowsVersionOrLater(kVersionWindows10);
}

bool IsWindows11OrLater()
{
    return IsWindowsVersionOrLater(kVersionWindows11);
}

bool Is64Bit()
{
#if defined(ANGLE_IS_64_BIT_CPU)
    return true;
#else
    return false;
#endif  // defined(ANGLE_IS_64_BIT_CPU)
}

}  // namespace angle
