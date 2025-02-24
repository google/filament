//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils_ios.mm: Implementation of iOS-specific functions for OSX

#include "system_utils.h"

#include <unistd.h>

#include <CoreServices/CoreServices.h>
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <array>
#include <cstdlib>
#include <vector>

#import <Foundation/Foundation.h>

namespace angle
{
std::string GetExecutablePath()
{

    NSString *executableString = [[NSBundle mainBundle] executablePath];
    std::string result([executableString UTF8String]);
    return result;
}

std::string GetExecutableDirectory()
{
    std::string executablePath = GetExecutablePath();
    size_t lastPathSepLoc      = executablePath.find_last_of("/");
    return (lastPathSepLoc != std::string::npos) ? executablePath.substr(0, lastPathSepLoc) : "";
}

const char *GetSharedLibraryExtension()
{
    return "dylib";
}

double GetCurrentTime()
{
    mach_timebase_info_data_t timebaseInfo;
    mach_timebase_info(&timebaseInfo);

    double secondCoeff = timebaseInfo.numer * 1e-9 / timebaseInfo.denom;
    return secondCoeff * mach_absolute_time();
}
}  // namespace angle
