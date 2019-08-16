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

#include "details/Texture.h"

#include "fg/FrameGraph.h"
#include "fg/FrameGraphHandle.h"

#include "RenderTargetResource.h"

#include <stdint.h>

namespace filament {
namespace fg {

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
    backend::RenderPassFlags targetFlags{};
    RenderTargetResource* cache = nullptr;

    void resolve(FrameGraph& fg) noexcept {
        auto& renderTargetCache = fg.mRenderTargetCache;

        // find a matching rendertarget
        auto pos = std::find_if(renderTargetCache.begin(), renderTargetCache.end(),
                [this, &fg](auto const& rt) {
                    return fg.equals(rt->desc, desc);
                });

        if (pos != renderTargetCache.end()) {
            cache = pos->get();
            cache->targetInfo.params.flags.clear |= userClearFlags;
        } else {
            uint8_t attachments = 0;
            uint32_t width = 0;
            uint32_t height = 0;
            backend::TextureFormat colorFormat = {};

            static constexpr backend::TargetBufferFlags flags[] = {
                    backend::TargetBufferFlags::COLOR,
                    backend::TargetBufferFlags::DEPTH,
                    backend::TargetBufferFlags::STENCIL };

            uint32_t minWidth = std::numeric_limits<uint32_t>::max();
            uint32_t maxWidth = 0;
            uint32_t minHeight = std::numeric_limits<uint32_t>::max();
            uint32_t maxHeight = 0;

            for (size_t i = 0; i < desc.attachments.textures.size(); i++) {
                FrameGraphRenderTarget::Attachments::AttachmentInfo attachment = desc.attachments.textures[i];
                if (attachment.isValid()) {
                    fg::ResourceEntry<FrameGraphTexture>& entry =
                            fg.getResourceEntryUnchecked(attachment.getHandle());
                    attachments |= flags[i];

                    // figure out the min/max dimensions across all attachments
                    const size_t level = attachment.getLevel();
                    const uint32_t w = details::FTexture::valueForLevel(level, entry.descriptor.width);
                    const uint32_t h = details::FTexture::valueForLevel(level, entry.descriptor.height);
                    minWidth  = std::min(minWidth,  w);
                    maxWidth  = std::max(maxWidth,  w);
                    minHeight = std::min(minHeight, h);
                    maxHeight = std::max(maxHeight, h);

                    if (i == FrameGraphRenderTarget::Attachments::COLOR) {
                        colorFormat = entry.descriptor.format;
                    }
                }
            }

            if (attachments) {
                if (minWidth == maxWidth && minHeight == maxHeight) {
                    // All attachments' size match, we're good to go.
                    width = minWidth;
                    height = minHeight;
                } else {
                    // TODO: what should we do here? Is it a user-error?
                    width = maxWidth;
                    height = maxHeight;
                }

                // create the cache entry
                RenderTargetResource* pRenderTargetResource =
                        fg.mArena.make<RenderTargetResource>(desc, false,
                                backend::TargetBufferFlags(attachments), width, height, colorFormat);
                renderTargetCache.emplace_back(pRenderTargetResource, fg);
                cache = pRenderTargetResource;
                cache->targetInfo.params.flags.clear |= userClearFlags;
            }
        }
    }
};

} // namespace fg
} // namespace filament

#endif //TNT_FILAMENT_FG_RENDERTARGET_H
