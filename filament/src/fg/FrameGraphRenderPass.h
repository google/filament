/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_FILAMENT_FG_FRAMEGRAPHRENDERPASS_H
#define TNT_FILAMENT_FG_FRAMEGRAPHRENDERPASS_H

#include "fg/FrameGraphTexture.h"

#include <backend/DriverEnums.h>
#include <backend/TargetBufferInfo.h>

#include <filament/Viewport.h>

namespace filament {

/**
 * FrameGraphRenderPass is used to draw into a set of FrameGraphTexture resources.
 * These are transient objects that exist inside a pass only.
 */
struct FrameGraphRenderPass {
    static constexpr size_t ATTACHMENT_COUNT = backend::MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + 2;
    struct Attachments {
        struct Content {
            FrameGraphId<FrameGraphTexture> color[backend::MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT];
            FrameGraphId<FrameGraphTexture> depth;
            FrameGraphId<FrameGraphTexture> stencil;
        };
        union {
            FrameGraphId<FrameGraphTexture> array[ATTACHMENT_COUNT] = {};
            Content content;
        };
    };

    struct Descriptor {
        Attachments attachments{};
        Viewport viewport{};
        math::float4 clearColor{};
        uint8_t samples = 0; // # of samples (0 = unset, default)
        backend::TargetBufferFlags clearFlags{};
        backend::TargetBufferFlags discardStart{};
    };

    struct ImportDescriptor {
        backend::TargetBufferFlags attachments = backend::TargetBufferFlags::COLOR0;
        Viewport viewport{};
        math::float4 clearColor{};  // this overrides Descriptor::clearColor
        uint8_t samples = 0;        // # of samples (0 = unset, default)
        backend::TargetBufferFlags clearFlags{}; // this overrides Descriptor::clearFlags
        backend::TargetBufferFlags keepOverrideStart{};
        backend::TargetBufferFlags keepOverrideEnd{};
    };

    uint32_t id = 0;
};

} // namespace filament

#endif // TNT_FILAMENT_FG_FRAMEGRAPHRENDERPASS_H
