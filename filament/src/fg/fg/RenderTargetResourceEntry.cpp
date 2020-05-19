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

#include "RenderTargetResourceEntry.h"

#include "details/Texture.h"

#include "fg/fg/PassNode.h"
#include "fg/fg/ResourceNode.h"

#include <fg/FrameGraph.h>
#include <fg/FrameGraphHandle.h>
#include <fg/ResourceAllocator.h>

#include <backend/TargetBufferInfo.h>

#include <utils/Panic.h>

#include <vector>

namespace filament {

using namespace backend;

namespace fg {

void RenderTargetResourceEntry::resolve(FrameGraph& fg) noexcept {
    attachments = {};
    width = 0;
    height = 0;
    auto& resource = getResource();

    static constexpr TargetBufferFlags flags[] = {
            TargetBufferFlags::COLOR0,
            TargetBufferFlags::COLOR1,
            TargetBufferFlags::COLOR2,
            TargetBufferFlags::COLOR3,
            TargetBufferFlags::DEPTH,
            TargetBufferFlags::STENCIL };

    static_assert(sizeof(flags)/sizeof(*flags) == FrameGraphRenderTarget::Attachments::COUNT,
            "array sizes don't match");

    static constexpr TextureUsage usages[] = {
            TextureUsage::COLOR_ATTACHMENT,
            TextureUsage::COLOR_ATTACHMENT,
            TextureUsage::COLOR_ATTACHMENT,
            TextureUsage::COLOR_ATTACHMENT,
            TextureUsage::DEPTH_ATTACHMENT,
            TextureUsage::STENCIL_ATTACHMENT };

    static_assert(sizeof(flags)/sizeof(*flags) == sizeof(usages)/sizeof(*usages),
            "array sizes don't match");

    uint32_t minWidth = std::numeric_limits<uint32_t>::max();
    uint32_t minHeight = std::numeric_limits<uint32_t>::max();
    uint32_t maxWidth = 0;
    uint32_t maxHeight = 0;

    for (size_t i = 0; i < descriptor.attachments.textures.size(); i++) {
        auto attachment = descriptor.attachments.textures[i];
        if (attachment.isValid()) {
            fg::ResourceEntry<FrameGraphTexture>& entry =
                    fg.getResourceEntryUnchecked(attachment.getHandle());
            // update usage flags for referenced attachments
            entry.descriptor.usage |= usages[i];

            // update attachment sample count if not specified and usage permits it
            if (!entry.descriptor.samples &&
                none(entry.descriptor.usage & backend::TextureUsage::SAMPLEABLE)) {
                entry.descriptor.samples = descriptor.samples;
            }

            attachments |= flags[i];

            // figure out the min/max dimensions across all attachments
            const size_t level = attachment.getLevel();
            const uint32_t w = FTexture::valueForLevel(level, entry.descriptor.width);
            const uint32_t h = FTexture::valueForLevel(level, entry.descriptor.height);
            minWidth = std::min(minWidth, w);
            maxWidth = std::max(maxWidth, w);
            minHeight = std::min(minHeight, h);
            maxHeight = std::max(maxHeight, h);
        }
    }

    if (any(attachments)) {
        if (minWidth == maxWidth && minHeight == maxHeight) {
            // All attachments' size match, we're good to go.
            width = minWidth;
            height = minHeight;
        } else {
            // TODO: what should we do here? Is it a user-error?
            width = maxWidth;
            height = maxHeight;
        }

        if (resource.params.viewport.width == 0 && resource.params.viewport.height == 0) {
            resource.params.viewport.width = width;
            resource.params.viewport.height = height;
        }
    } else {
        // this can happen with imported targets
        resource.params.viewport = descriptor.viewport;
    }
    resource.params.clearColor = descriptor.clearColor;
    resource.params.flags.clear = descriptor.clearFlags;
}

void RenderTargetResourceEntry::update(FrameGraph& fg, PassNode const& pass) noexcept {
    auto& resource = getResource();

    // this update at every pass
    if (any(attachments)) {
        // overwrite discard flags with the per-rendertarget (per-pass) computed value
        resource.params.flags.discardStart = TargetBufferFlags::NONE;
        resource.params.flags.discardEnd   = TargetBufferFlags::NONE;

        static constexpr TargetBufferFlags flags[] = {
                TargetBufferFlags::COLOR0,
                TargetBufferFlags::COLOR1,
                TargetBufferFlags::COLOR2,
                TargetBufferFlags::COLOR3,
                TargetBufferFlags::DEPTH,
                TargetBufferFlags::STENCIL };

        static_assert(sizeof(flags)/sizeof(*flags) == FrameGraphRenderTarget::Attachments::COUNT,
                "array sizes don't match");

        auto& resourceNodes = fg.mResourceNodes;
        for (size_t i = 0; i < descriptor.attachments.textures.size(); i++) {
            FrameGraphHandle attachment = descriptor.attachments.textures[i];
            if (attachment.isValid()) {
                if (resourceNodes[attachment.index]->resource->discardStart) {
                    resource.params.flags.discardStart |= flags[i];
                }
                if (resourceNodes[attachment.index]->resource->discardEnd) {
                    resource.params.flags.discardEnd |= flags[i];
                }
            }
        }

        // check that this FrameGraphRenderTarget is indeed declared by this pass
        ASSERT_POSTCONDITION_NON_FATAL(resource.target,
                "Pass \"%s\" doesn't declare rendertarget \"%s\" -- expect graphic corruptions",
                pass.name, name);
    }

    // clear implies discarding the content of the buffer
    resource.params.flags.discardStart |= resource.params.flags.clear;
}

void RenderTargetResourceEntry::preExecuteDevirtualize(FrameGraph& fg) noexcept {
    if (!imported) {
        assert(any(attachments));

        // TODO: we could cache the result of this loop
        backend::TargetBufferInfo infos[FrameGraphRenderTarget::Attachments::COUNT];
        for (size_t i = 0, c = descriptor.attachments.textures.size(); i < c; i++) {
            auto const& attachmentInfo = descriptor.attachments.textures[i];
#ifndef NDEBUG
            static constexpr TargetBufferFlags flags[] = {
                    TargetBufferFlags::COLOR0,
                    TargetBufferFlags::COLOR1,
                    TargetBufferFlags::COLOR2,
                    TargetBufferFlags::COLOR3,
                    TargetBufferFlags::DEPTH,
                    TargetBufferFlags::STENCIL };
            static_assert(sizeof(flags)/sizeof(*flags) == FrameGraphRenderTarget::Attachments::COUNT,
                    "array sizes don't match");
            assert(bool(attachments & flags[i]) == attachmentInfo.isValid());
#endif
            if (attachmentInfo.isValid()) {
                fg::ResourceEntry<FrameGraphTexture> const& entry =
                        fg.getResourceEntryUnchecked(attachmentInfo.getHandle());
                infos[i].handle = entry.getResource().texture;
                infos[i].level = attachmentInfo.getLevel();
                // the attachment buffer (texture or renderbuffer) must be valid
                assert(infos[i].handle);
                // the attachment level must be within range
                assert(infos[i].level < entry.descriptor.levels);
                // if the attachment is multisampled, then the rendertarget must be too
                assert(entry.descriptor.samples <= 1 || entry.descriptor.samples == descriptor.samples);
            }
        }

        auto& resource = getResource();
        resource.target = fg.getResourceAllocator().createRenderTarget(name,
                attachments, width, height, descriptor.samples,
                { infos[0], infos[1], infos[2], infos[3] },
                infos[4], infos[5]);
    }
}

void RenderTargetResourceEntry::preExecuteDestroy(FrameGraph& fg) noexcept {
    // FIXME: (workaround: see postExecuteDevirtualize) technically we need
    //        to clear discardEnd here (in case the RT is reused), but save
    //        its original value, to be restored in postExecuteDevirtualize()
    //        we don't do that right now, because it's not needed in our current
    //        use case.
}

void RenderTargetResourceEntry::postExecuteDestroy(FrameGraph& fg) noexcept {
    if (!imported) {
        auto& resource = getResource();
        if (resource.target) {
            fg.getResourceAllocator().destroyRenderTarget(resource.target);
            resource.target.clear();
        }
    }
}

void RenderTargetResourceEntry::postExecuteDevirtualize(FrameGraph& fg) noexcept {
    // after a rendertarget has been used once, it's never cleared anymore
    // (otherwise it wouldn't be possible to meaningfully reuse it)
    auto& resource = getResource();
    resource.params.flags.clear = TargetBufferFlags::NONE;

    if (imported) {
        // FIXME: (workaround) imported targets currently don't have attachments associated to
        //        them, so the discard flags can't be calculated. So if the target is used
        //        several times, discardStart must be cleared after the first time.
        //        (technically, discardEnd should be reset to it's inital value -- we don't do
        //        this here, because we never modified it in the first place)
        resource.params.flags.discardStart = TargetBufferFlags::NONE;
    }
}

} // namespace fg
} // namespace filament
