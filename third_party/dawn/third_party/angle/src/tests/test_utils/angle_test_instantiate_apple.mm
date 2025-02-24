//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file includes APIs to detect whether certain Apple renderer is available for testing.
//

#include "test_utils/angle_test_instantiate_apple.h"

#include "common/apple_platform_utils.h"
#include "test_utils/angle_test_instantiate.h"

namespace angle
{

bool IsMetalTextureSwizzleAvailable()
{
#if ANGLE_PLATFORM_IOS_FAMILY_SIMULATOR
    return false;
#else
    // All NVIDIA and older Intel don't support swizzle because they are GPU family 1.
    // We don't have a way to detect Metal family here, so skip all Intel for now.
    return !IsIntel() && !IsNVIDIA();
#endif
}

}  // namespace angle
