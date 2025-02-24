//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils_linux.cpp: Implementation of OS-specific functions for Linux

#include "common/debug.h"
#include "system_utils.h"

#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>

namespace angle
{
std::string GetExecutablePath()
{
    // We cannot use lstat to get the size of /proc/self/exe as it always returns 0
    // so we just use a big buffer and hope the path fits in it.
    char path[4096];

    ssize_t result = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (result < 0 || static_cast<size_t>(result) >= sizeof(path) - 1)
    {
        return "";
    }

    path[result] = '\0';
    return path;
}

std::string GetExecutableDirectory()
{
    std::string executablePath = GetExecutablePath();
    size_t lastPathSepLoc      = executablePath.find_last_of("/");
    return (lastPathSepLoc != std::string::npos) ? executablePath.substr(0, lastPathSepLoc) : "";
}

const char *GetSharedLibraryExtension()
{
    return "so";
}

double GetCurrentSystemTime()
{
    struct timespec currentTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    return currentTime.tv_sec + currentTime.tv_nsec * 1e-9;
}

void SetCurrentThreadName(const char *name)
{
    // There's a 15-character (16 including '\0') limit.  If the name is too big (and ERANGE is
    // returned), name will be ignored.
    ASSERT(strlen(name) < 16);
    pthread_setname_np(pthread_self(), name);
}
}  // namespace angle
