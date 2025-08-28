/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ZstdHelper.h"

#include <zstd.h>

#include <cstddef>
#include <cstdint>

namespace filament {

bool ZstdHelper::isCompressed(const void* src, size_t src_size) noexcept {
    if (src_size < 4) {
        return false;
    }

    // `src` may not be aligned to 4 bytes, which is violating the alignment requirement of
    // `UndefinedBehaviorSanitizer: misaligned-pointer-use`. So reconstruct the 32-bit integer from
    // bytes in little-endian order as the expected byte sequence is 28 B5 2F FD and
    // ZSTD_MAGICNUMBER refers to 0xFD2FB528. This should work correctly on both little-endian and
    // big-endian systems.
    const auto* p = static_cast<const uint8_t*>(src);
    const uint32_t magic =
            (uint32_t)p[0] |
            (uint32_t)(p[1] << 8) |
            (uint32_t)(p[2] << 16) |
            (uint32_t)(p[3] << 24);

    return magic == ZSTD_MAGICNUMBER;
}

size_t ZstdHelper::getDecodedSize(const void* src, size_t src_size) noexcept {
    return ZSTD_getFrameContentSize(src, src_size);
}

size_t ZstdHelper::decompress(void* dst, size_t dst_size, const void* src, size_t src_size) noexcept {
    return ZSTD_decompress(dst, dst_size, src, src_size);
}

} // namespace filament
