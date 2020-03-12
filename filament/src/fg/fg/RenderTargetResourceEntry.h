/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef TNT_RENDERTARGETRESOURCEENTRY_H
#define TNT_RENDERTARGETRESOURCEENTRY_H

#include "fg/fg/ResourceEntry.h"

#include <fg/FrameGraphHandle.h>

#include <backend/TargetBufferInfo.h>

namespace filament {

class FrameGraph;

namespace fg {

class RenderTargetResourceEntry : public ResourceEntry<FrameGraphRenderTarget> {
public:
    using ResourceEntry<FrameGraphRenderTarget>::ResourceEntry;

    void update(FrameGraph& fg, PassNode const& pass) noexcept;

private:
    void resolve(FrameGraph& fg) noexcept override;
    void preExecuteDevirtualize(FrameGraph& fg) noexcept override;
    void postExecuteDestroy(FrameGraph& fg) noexcept override;
    void preExecuteDestroy(FrameGraph& fg) noexcept override;
    void postExecuteDevirtualize(FrameGraph& fg) noexcept override;
    RenderTargetResourceEntry* asRenderTargetResourceEntry() noexcept override { return this; }

    // render target creation info
    uint32_t width;
    uint32_t height;
    backend::TargetBufferFlags attachments;
};

} // namespace fg
} // namespace filament


#endif //TNT_FILAMENT_FG_RENDERTARGETRESOURCEENTRY_H
