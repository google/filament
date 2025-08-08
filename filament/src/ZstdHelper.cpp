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
    const uint32_t magic = *static_cast<const uint32_t*>(src);
    return magic == ZSTD_MAGICNUMBER;
}

size_t ZstdHelper::getDecodedSize(const void* src, size_t src_size) noexcept {
    return ZSTD_getFrameContentSize(src, src_size);
}

size_t ZstdHelper::decompress(void* dst, size_t dst_size, const void* src, size_t src_size) noexcept {
    return ZSTD_decompress(dst, dst_size, src, src_size);
}

} // namespace filament
