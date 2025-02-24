//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// system_info_util.h:
//   Implementation of common test utilities for operating with SystemInfo.
//

#ifndef ANGLE_TESTS_SYSTEM_INFO_UTIL_H_
#define ANGLE_TESTS_SYSTEM_INFO_UTIL_H_

#include <stddef.h>

namespace angle
{
struct SystemInfo;
}  // namespace angle

// Returns the index of the low power GPU in SystemInfo.
size_t FindLowPowerGPU(const angle::SystemInfo &);

// Returns the index of the high power GPU in SystemInfo.
size_t FindHighPowerGPU(const angle::SystemInfo &);

// Returns the index of the GPU in SystemInfo based on the OpenGL renderer string.
size_t FindActiveOpenGLGPU(const angle::SystemInfo &);

#endif  // ANGLE_TESTS_SYSTEM_INFO_UTIL_H_
