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

#ifndef TNT_FILAMENT_FG_RENDERTARGETRESOURCE_H
#define TNT_FILAMENT_FG_RENDERTARGETRESOURCE_H

#include <backend/DriverEnums.h>
#include <backend/TargetBufferInfo.h>

#include "fg/FrameGraph.h"

#include "fg/ResourceAllocator.h"
#include "ResourceNode.h"
#include "VirtualResource.h"

namespace filament {

class FrameGraph;

namespace fg {

struct RenderTargetResource final : public VirtualResource {  // 104

    RenderTargetResource(
            FrameGraphRenderTarget::Descriptor const& desc, bool imported,
            backend::TargetBufferFlags targets, uint32_t width, uint32_t height, backend::TextureFormat format)
            : desc(desc), imported(imported),
              attachments(targets), format(format), width(width), height(height) {
        targetInfo.params.viewport = desc.viewport;
        // if Descriptor was initialized with default values, set the viewport to width/height
        if (targetInfo.params.viewport.width == 0 && targetInfo.params.viewport.height == 0) {
            targetInfo.params.viewport.width = width;
            targetInfo.params.viewport.height = height;
        }
    }

    RenderTargetResource(RenderTargetResource const&) = delete;
    RenderTargetResource(RenderTargetResource&&) noexcept = default;
    RenderTargetResource& operator=(RenderTargetResource const&) = delete;
    ~RenderTargetResource() override;

    // cache key
    const FrameGraphRenderTarget::Descriptor desc;
    const bool imported;

    // render target creation info
    backend::TargetBufferFlags attachments;
    backend::TextureFormat format;
    uint32_t width;
    uint32_t height;
    backend::TargetBufferFlags discardStart = backend::TargetBufferFlags::NONE;
    backend::TargetBufferFlags discardEnd = backend::TargetBufferFlags::NONE;

    // updated during execute with the current pass' discard flags
    FrameGraphPassResources::RenderTargetInfo targetInfo;

    void create(FrameGraph& fg) noexcept override {
        if (!imported) {
            if (attachments) {
                // devirtualize our texture handles. By this point these handles have been
                // remapped to their alias if any.
                backend::TargetBufferInfo infos[FrameGraphRenderTarget::Attachments::COUNT];
                for (size_t i = 0, c = desc.attachments.textures.size(); i < c; i++) {
                    auto const& attachmentInfo = desc.attachments.textures[i];
                    if (attachmentInfo.isValid()) {
                        fg::ResourceEntry<FrameGraphTexture> const& entry =
                                fg.getResourceEntryUnchecked(attachmentInfo.getHandle());
                        infos[i].handle = entry.getResource().texture;
                        infos[i].level = attachmentInfo.getLevel();
                    }
                }

                // create the concrete rendertarget
                targetInfo.target = fg.getResourceAllocator().createRenderTarget(attachments,
                        width, height, desc.samples, infos[0], infos[1], {});
            }
        }
    }

    void destroy(FrameGraph& fg) noexcept override {
        if (!imported) {
            if (targetInfo.target) {
                fg.getResourceAllocator().destroyRenderTarget(targetInfo.target);
                targetInfo.target.clear();
            }
        }
    }
};

} // namespace fg
} // namespace filament

#endif //TNT_FILAMENT_FG_RENDERTARGETRESOURCE_H
