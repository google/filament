//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SystemInfo_fuchsia.cpp: implementation of the Fuchsia-specific parts of SystemInfo.h

#include "gpu_info_util/SystemInfo_vulkan.h"

namespace angle
{

bool GetSystemInfo(SystemInfo *info)
{
    return GetSystemInfoVulkan(info);
}

}  // namespace angle
