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
    // texture to be used as render target
    Handle<HwTexture> handle;

    // starting layer index for multiview. This value is only used when the `layerCount` for the
    // render target is greater than 1.
    uint8_t baseViewIndex = 0;

    // level to be used
    uint8_t level = 0;

    // for cubemaps and 3D textures. See TextureCubemapFace for the face->layer mapping
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

    MRT(TargetBufferInfo const& color) noexcept // NOLINT(hicpp-explicit-conversions)
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
            : mInfos{{ handle, 0, level, layer }} {
    }
};

} // namespace filament::backend

#if !defined(NDEBUG)
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::TargetBufferInfo& tbi);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::MRT& mrt);
#endif

#endif //TNT_FILAMENT_BACKEND_TARGETBUFFERINFO_H
