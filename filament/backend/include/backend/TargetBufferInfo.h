/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_TARGETBUFFERINFO_H
#define TNT_FILAMENT_BACKEND_TARGETBUFFERINFO_H

#include <backend/Handle.h>

#include <utils/ostream.h>

#include <stddef.h>
#include <stdint.h>

namespace filament::backend {

//! \privatesection

struct TargetBufferInfo {
    // note: the parameters of this constructor are not in the order of this structure's fields
    TargetBufferInfo(Handle<HwTexture> handle, uint8_t level, uint16_t layer) noexcept
            : handle(handle), level(level), layer(layer) {
    }

    TargetBufferInfo(Handle<HwTexture> handle, uint8_t level) noexcept
            : handle(handle), level(level) {
    }

    TargetBufferInfo(Handle<HwTexture> handle) noexcept // NOLINT(*-explicit-constructor)
            : handle(handle) {
    }

    TargetBufferInfo() noexcept = default;

    // texture to be used as render target
    Handle<HwTexture> handle;

    // level to be used
    uint8_t level = 0;

    // - For cubemap textures, this indicates the face of the cubemap. See TextureCubemapFace for
    //   the face->layer mapping)
    // - For 2d array, cubemap array, and 3d textures, this indicates an index of a single layer of
    //   them.
    // - For multiview textures (i.e., layerCount for the RenderTarget is greater than 1), this
    //   indicates a starting layer index of the current 2d array texture for multiview.
    uint16_t layer = 0;
};

class MRT {
public:
    static constexpr uint8_t MIN_SUPPORTED_RENDER_TARGET_COUNT = 4u;

    // When updating this, make sure to also take care of RenderTarget.java
    static constexpr uint8_t MAX_SUPPORTED_RENDER_TARGET_COUNT = 8u;

private:
    TargetBufferInfo mInfos[MAX_SUPPORTED_RENDER_TARGET_COUNT];

public:
    TargetBufferInfo const& operator[](size_t i) const noexcept {
        return mInfos[i];
    }

    TargetBufferInfo& operator[](size_t i) noexcept {
        return mInfos[i];
    }

    MRT() noexcept = default;

    MRT(TargetBufferInfo const& color) noexcept // NOLINT(hicpp-explicit-conversions, *-explicit-constructor)
            : mInfos{ color } {
    }

    MRT(TargetBufferInfo const& color0, TargetBufferInfo const& color1) noexcept
            : mInfos{ color0, color1 } {
    }

    MRT(TargetBufferInfo const& color0, TargetBufferInfo const& color1,
        TargetBufferInfo const& color2) noexcept
            : mInfos{ color0, color1, color2 } {
    }

    MRT(TargetBufferInfo const& color0, TargetBufferInfo const& color1,
        TargetBufferInfo const& color2, TargetBufferInfo const& color3) noexcept
            : mInfos{ color0, color1, color2, color3 } {
    }

    // this is here for backward compatibility
    MRT(Handle<HwTexture> handle, uint8_t level, uint16_t layer) noexcept
            : mInfos{{ handle, level, layer }} {
    }
};

} // namespace filament::backend

#if !defined(NDEBUG)
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::TargetBufferInfo& tbi);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::MRT& mrt);
#endif

#endif //TNT_FILAMENT_BACKEND_TARGETBUFFERINFO_H
