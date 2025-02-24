//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GPUTestConfig_mac.h:
//   Helper functions for GPUTestConfig that have to be compiled in ObjectiveC++
//

#ifndef TEST_EXPECTATIONS_GPU_TEST_CONFIG_MAC_H_
#define TEST_EXPECTATIONS_GPU_TEST_CONFIG_MAC_H_

#include <stdint.h>

namespace angle
{

void GetOperatingSystemVersionNumbers(int32_t *majorVersion, int32_t *minorVersion);

}  // namespace angle

#endif  // TEST_EXPECTATIONS_GPU_TEST_CONFIG_MAC_H_
