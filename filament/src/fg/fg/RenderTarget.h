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

#ifndef TNT_FILAMENT_FG_RENDERTARGET_H
#define TNT_FILAMENT_FG_RENDERTARGET_H

#include <backend/DriverEnums.h>

#include "fg/FrameGraphHandle.h"

#include <stdint.h>

namespace filament {

class FrameGraph;

namespace fg {

struct RenderTargetResource;

struct RenderTarget { // 32
    RenderTarget(const char* name,
            FrameGraphRenderTarget::Descriptor const& desc, uint16_t index) noexcept
            : name(name), index(index), desc(desc) {
    }
    RenderTarget(RenderTarget const&) = delete;
    RenderTarget(RenderTarget&&) noexcept = default;
    RenderTarget& operator=(RenderTarget const&) = delete;

    // constants
    const char* const name;         // for debugging
    uint16_t index;

    // set by builder
    FrameGraphRenderTarget::Descriptor desc;
    backend::TargetBufferFlags userClearFlags{};

    // set in compile
    backend::TargetBufferFlags clear = {}; // this is eventually set by the user
    RenderTargetResource* cache = nullptr;

    void resolve(FrameGraph& fg) noexcept;
};

} // namespace fg
} // namespace filament

#endif //TNT_FILAMENT_FG_RENDERTARGET_H
