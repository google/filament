//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SystemInfo_apple.cpp: implementation of the Apple-specific parts of SystemInfo.h

#import "gpu_info_util/SystemInfo.h"

#import "common/platform.h"
#import "gpu_info_util/SystemInfo_internal.h"

namespace angle
{

bool GetSystemInfo(SystemInfo *info)
{
#if defined(ANGLE_PLATFORM_MACOS) || defined(ANGLE_PLATFORM_MACCATALYST)
    return GetSystemInfo_mac(info);
#else
    return GetSystemInfo_ios(info);
#endif
}

}  // namespace angle
