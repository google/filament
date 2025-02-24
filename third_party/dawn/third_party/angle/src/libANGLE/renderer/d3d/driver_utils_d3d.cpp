//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// driver_utils_d3d.cpp: Information specific to the D3D driver

#include "libANGLE/renderer/d3d/driver_utils_d3d.h"

namespace rx
{

std::string GetDriverVersionString(LARGE_INTEGER driverVersion)
{
    std::stringstream versionString;
    uint64_t intVersion        = driverVersion.QuadPart;
    constexpr uint64_t kMask16 = std::numeric_limits<uint16_t>::max();
    versionString << ((intVersion >> 48) & kMask16) << ".";
    versionString << ((intVersion >> 32) & kMask16) << ".";
    versionString << ((intVersion >> 16) & kMask16) << ".";
    versionString << (intVersion & kMask16);
    return versionString.str();
}

}  // namespace rx
