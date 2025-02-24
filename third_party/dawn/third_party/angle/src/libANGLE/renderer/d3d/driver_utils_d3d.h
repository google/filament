//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// driver_utils_d3d.h: Information specific to the D3D driver

#ifndef LIBANGLE_RENDERER_D3D_DRIVER_UTILS_D3D_H_
#define LIBANGLE_RENDERER_D3D_DRIVER_UTILS_D3D_H_

#include "libANGLE/angletypes.h"

namespace rx
{

std::string GetDriverVersionString(LARGE_INTEGER driverVersion);

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_DRIVER_UTILS_D3D_H_
