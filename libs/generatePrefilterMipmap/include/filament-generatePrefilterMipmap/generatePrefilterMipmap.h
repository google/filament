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

#pragma once

#include <utils/compiler.h>

#include <stddef.h>
#include <stdint.h>

namespace filament {

namespace backend {
class PixelBufferDescriptor;
}

class Engine;
class Texture;

struct UTILS_PUBLIC FaceOffsets {
    using size_type = size_t;
    union {
        struct {
            size_type px;   //!< +x face offset in bytes
            size_type nx;   //!< -x face offset in bytes
            size_type py;   //!< +y face offset in bytes
            size_type ny;   //!< -y face offset in bytes
            size_type pz;   //!< +z face offset in bytes
            size_type nz;   //!< -z face offset in bytes
        };
        size_type offsets[6];
    };
    size_type  operator[](size_t const n) const noexcept { return offsets[n]; }
    size_type& operator[](size_t const n) { return offsets[n]; }
    FaceOffsets() noexcept = default;
    explicit FaceOffsets(size_type const faceSize) noexcept {
        px = faceSize * 0;
        nx = faceSize * 1;
        py = faceSize * 2;
        ny = faceSize * 3;
        pz = faceSize * 4;
        nz = faceSize * 5;
    }
    FaceOffsets(const FaceOffsets& rhs) noexcept {
        px = rhs.px;
        nx = rhs.nx;
        py = rhs.py;
        ny = rhs.ny;
        pz = rhs.pz;
        nz = rhs.nz;
    }
    FaceOffsets& operator=(const FaceOffsets& rhs) noexcept {
        px = rhs.px;
        nx = rhs.nx;
        py = rhs.py;
        ny = rhs.ny;
        pz = rhs.pz;
        nz = rhs.nz;
        return *this;
    }
};

/**
 * Options for environment prefiltering into reflection map
 *
 * @see generatePrefilterMipmap()
 */
struct UTILS_PUBLIC PrefilterOptions {
    uint16_t sampleCount = 8;   //!< sample count used for filtering
    bool mirror = true;         //!< whether the environment must be mirrored
private:
    UTILS_UNUSED uintptr_t reserved[3] = {};
};

UTILS_PUBLIC
void generatePrefilterMipmap(Texture* UTILS_NONNULL texture, Engine& engine,
        backend::PixelBufferDescriptor&& buffer, const FaceOffsets& faceOffsets,
        PrefilterOptions const* UTILS_NULLABLE options = nullptr);


} // namespace filament
