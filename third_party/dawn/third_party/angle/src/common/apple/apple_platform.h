//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// apple_platform.h: Apple operating system specific includes and defines.
//

#ifndef COMMON_APPLE_PLATFORM_H_
#define COMMON_APPLE_PLATFORM_H_

#import "common/platform.h"

#if ((ANGLE_PLATFORM_MACOS && __MAC_OS_X_VERSION_MIN_REQUIRED >= 120000) ||   \
     (((ANGLE_PLATFORM_IOS_FAMILY && !ANGLE_PLATFORM_IOS_FAMILY_SIMULATOR) || \
       ANGLE_PLATFORM_MACCATALYST) &&                                         \
      __IPHONE_OS_VERSION_MIN_REQUIRED >= 150000) ||                          \
     (ANGLE_PLATFORM_WATCHOS && !ANGLE_PLATFORM_IOS_FAMILY_SIMULATOR &&       \
      __WATCH_OS_VERSION_MIN_REQUIRED >= 80000) ||                            \
     (TARGET_OS_TV && !ANGLE_PLATFORM_IOS_FAMILY_SIMULATOR &&                 \
      __TV_OS_VERSION_MIN_REQUIRED >= 150000)) &&                             \
    (defined(__has_include) && __has_include(<Metal/MTLResource_Private.h>))
#    define ANGLE_HAVE_MTLRESOURCE_SET_OWNERSHIP_IDENTITY 1
#else
#    define ANGLE_HAVE_MTLRESOURCE_SET_OWNERSHIP_IDENTITY 0
#endif

#if (ANGLE_HAVE_MTLRESOURCE_SET_OWNERSHIP_IDENTITY && \
     defined(ANGLE_ENABLE_METAL_OWNERSHIP_IDENTITY))
#    define ANGLE_USE_METAL_OWNERSHIP_IDENTITY 1
#else
#    define ANGLE_USE_METAL_OWNERSHIP_IDENTITY 0
#endif

#endif /* COMMON_APPLE_PLATFORM_H_ */
