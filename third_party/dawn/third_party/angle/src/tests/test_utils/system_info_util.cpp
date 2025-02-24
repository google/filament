//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// system_info_util.h:
//   Implementation of common test utilities for operating with SystemInfo.
//

#include "system_info_util.h"

#include "common/debug.h"
#include "common/string_utils.h"
#include "gpu_info_util/SystemInfo.h"
#include "util/util_gl.h"

using namespace angle;

namespace
{

size_t findGPU(const SystemInfo &systemInfo, bool lowPower)
{
    if (systemInfo.gpus.size() < 2)
    {
        return 0;
    }
    for (size_t i = 0; i < systemInfo.gpus.size(); ++i)
    {
        if (lowPower && IsIntel(systemInfo.gpus[i].vendorId))
        {
            return i;
        }
        // Return the high power GPU, i.e any non-intel GPU
        else if (!lowPower && !IsIntel(systemInfo.gpus[i].vendorId))
        {
            return i;
        }
    }
    // Can't find GPU
    ASSERT(false);
    return 0;
}

}  // namespace

size_t FindLowPowerGPU(const SystemInfo &systemInfo)
{
    return findGPU(systemInfo, true);
}

size_t FindHighPowerGPU(const SystemInfo &systemInfo)
{
    return findGPU(systemInfo, false);
}

size_t FindActiveOpenGLGPU(const SystemInfo &systemInfo)
{
    char *renderer = (char *)glGetString(GL_RENDERER);
    std::string rendererString(renderer);
    for (size_t i = 0; i < systemInfo.gpus.size(); ++i)
    {
        std::vector<std::string> vendorTokens;
        angle::SplitStringAlongWhitespace(VendorName(systemInfo.gpus[i].vendorId), &vendorTokens);
        for (std::string &token : vendorTokens)
        {
            if (rendererString.find(token) != std::string::npos)
            {
                return i;
            }
        }
    }
    // Can't find active GPU
    ASSERT(false);
    return 0;
}
