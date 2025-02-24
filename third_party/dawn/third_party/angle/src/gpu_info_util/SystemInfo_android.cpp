//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SystemInfo_android.cpp: implementation of the Android-specific parts of SystemInfo.h

#include "gpu_info_util/SystemInfo_internal.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

#include "common/android_util.h"
#include "common/angleutils.h"
#include "common/debug.h"

namespace angle
{

bool GetSystemInfo(SystemInfo *info)
{
    bool isFullyPopulated = true;

    isFullyPopulated = android::GetSystemProperty(android::kManufacturerSystemPropertyName,
                                                  &info->machineManufacturer) &&
                       isFullyPopulated;
    isFullyPopulated =
        android::GetSystemProperty(android::kModelSystemPropertyName, &info->machineModelName) &&
        isFullyPopulated;

    std::string androidSdkLevel;
    if (android::GetSystemProperty(android::kSDKSystemPropertyName, &androidSdkLevel))
    {
        info->androidSdkLevel = std::atoi(androidSdkLevel.c_str());
    }
    else
    {
        isFullyPopulated = false;
    }

    return GetSystemInfoVulkan(info) && isFullyPopulated;
}

}  // namespace angle
