//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PrintSystemInfoTest.cpp:
//     prints the information gathered about the system so that it appears in the test logs

#include <gtest/gtest.h>

#include <iostream>

#include "common/platform.h"
#include "common/system_utils.h"
#include "gpu_info_util/SystemInfo.h"

using namespace angle;

namespace
{

#if defined(ANGLE_PLATFORM_WINDOWS) || defined(ANGLE_PLATFORM_LINUX) || \
    defined(ANGLE_PLATFORM_APPLE)
#    define SYSTEM_INFO_IMPLEMENTED
#endif

// Prints the information gathered about the system
TEST(PrintSystemInfoTest, Print)
{
#if defined(SYSTEM_INFO_IMPLEMENTED)
    SystemInfo info;

    ASSERT_TRUE(GetSystemInfo(&info));
    ASSERT_GT(info.gpus.size(), 0u);

    PrintSystemInfo(info);
#else
    std::cerr << "GetSystemInfo not implemented, skipping" << std::endl;
#endif
}

TEST(PrintSystemInfoTest, GetSystemInfoNoCrashOnInvalidDisplay)
{
#if defined(SYSTEM_INFO_IMPLEMENTED) && defined(ANGLE_USE_X11)
    const char kX11DisplayEnvVar[] = "DISPLAY";
    const char kInvalidDisplay[]   = "124:";
    std::string previous_display   = GetEnvironmentVar(kX11DisplayEnvVar);
    SetEnvironmentVar(kX11DisplayEnvVar, kInvalidDisplay);
    SystemInfo info;

    // This should not crash.
    GetSystemInfo(&info);

    if (previous_display.empty())
    {
        UnsetEnvironmentVar(kX11DisplayEnvVar);
    }
    else
    {
        SetEnvironmentVar(kX11DisplayEnvVar, previous_display.c_str());
    }
#elif defined(SYSTEM_INFO_IMPLEMENTED)
    std::cerr << "GetSystemInfo not implemented, skipping" << std::endl;
#else
    std::cerr << "GetSystemInfo X11 test not applicable, skipping" << std::endl;
#endif
}

}  // anonymous namespace
