//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// apple_platform_utils.h: Common utilities for Apple platforms.

#ifndef COMMON_APPLE_PLATFORM_UTILS_H_
#define COMMON_APPLE_PLATFORM_UTILS_H_

#include "common/platform.h"

#include <string>

#if defined(__ARM_ARCH)
#    define ANGLE_APPLE_IS_ARM (__ARM_ARCH != 0)
#else
#    define ANGLE_APPLE_IS_ARM 0
#endif

#define ANGLE_APPLE_OBJC_SCOPE @autoreleasepool

#if !__has_feature(objc_arc)
#    define ANGLE_APPLE_AUTORELEASE autorelease
#    define ANGLE_APPLE_RETAIN retain
#    define ANGLE_APPLE_RELEASE release
#else
#    define ANGLE_APPLE_AUTORELEASE self
#    define ANGLE_APPLE_RETAIN self
#    define ANGLE_APPLE_RELEASE self
#endif

#define ANGLE_APPLE_UNUSED __attribute__((unused))

#if __has_warning("-Wdeprecated-declarations")
#    define ANGLE_APPLE_ALLOW_DEPRECATED_BEGIN \
        _Pragma("GCC diagnostic push")         \
            _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#else
#    define ANGLE_APPLE_ALLOW_DEPRECATED_BEGIN
#endif

#if __has_warning("-Wdeprecated-declarations")
#    define ANGLE_APPLE_ALLOW_DEPRECATED_END _Pragma("GCC diagnostic pop")
#else
#    define ANGLE_APPLE_ALLOW_DEPRECATED_END
#endif

namespace angle
{
bool IsMetalRendererAvailable();

#if defined(ANGLE_PLATFORM_MACOS) || defined(ANGLE_PLATFORM_MACCATALYST)
bool GetMacosMachineModel(std::string *outMachineModel);
bool ParseMacMachineModel(const std::string &identifier,
                          std::string *type,
                          int32_t *major,
                          int32_t *minor);
#endif

}  // namespace angle

#endif
