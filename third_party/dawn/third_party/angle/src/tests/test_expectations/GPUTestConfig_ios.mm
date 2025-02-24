//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GPUTestConfig_iOS.mm:
//   Helper functions for GPUTestConfig that have to be compiled in ObjectiveC++

#include "GPUTestConfig_ios.h"

#include "common/apple_platform_utils.h"

#import <Foundation/Foundation.h>

namespace angle
{

void GetOperatingSystemVersionNumbers(int32_t *majorVersion, int32_t *minorVersion)
{
    NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
    *majorVersion                    = static_cast<int32_t>(version.majorVersion);
    *minorVersion                    = static_cast<int32_t>(version.minorVersion);
}

}  // namespace angle
