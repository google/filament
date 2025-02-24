//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// png_utils: Wrapper around libpng.
//

#ifndef UTIL_PNG_UTILS_H_
#define UTIL_PNG_UTILS_H_

#include <cstdint>
#include <vector>

namespace angle
{
bool SavePNGRGB(const char *fileName,
                const char *title,
                uint32_t width,
                uint32_t height,
                const std::vector<uint8_t> &rgbData);
}  // namespace angle

#endif  // UTIL_PNG_UTILS_H_
