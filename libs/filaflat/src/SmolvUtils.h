/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_FILAFLAT_SMOLV_UTILS_H
#define TNT_FILAFLAT_SMOLV_UTILS_H

#if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)

#include <smolv.h>

#include <stddef.h>

namespace filaflat {

// GetDecodedBufferSize() returns the blob's declared decoded size without knowing the input size.
// smol-v's largest possible expansion is 8x (a bunched OpMemberDecorate run where each member
// costs two input bytes and emits four output words). Dividing keeps the check free of overflow.
inline size_t getBoundedSmolvDecodedSize(void const* data, size_t size) noexcept {
    constexpr size_t MAX_SMOLV_EXPANSION = 8;
    size_t const spirvSize = smolv::GetDecodedBufferSize(data, size);
    if (spirvSize == 0 || spirvSize / MAX_SMOLV_EXPANSION > size) {
        return 0;
    }
    return spirvSize;
}

} // namespace filaflat

#endif // defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)

#endif // TNT_FILAFLAT_SMOLV_UTILS_H
