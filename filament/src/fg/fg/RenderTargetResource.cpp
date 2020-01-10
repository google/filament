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

#include "RenderTargetResource.h"

namespace filament {

using namespace backend;

namespace fg {

void RenderTargetResource::create(FrameGraph& fg) noexcept {
    if (!imported) {
        if (any(attachments)) {
            // devirtualize our texture handles. By this point these handles have been
            // remapped to their alias if any.
            backend::TargetBufferInfo infos[FrameGraphRenderTarget::Attachments::COUNT];
            for (size_t i = 0, c = desc.attachments.textures.size(); i < c; i++) {

                static constexpr TargetBufferFlags flags[] = {
                        TargetBufferFlags::COLOR,
                        TargetBufferFlags::DEPTH,
                        TargetBufferFlags::STENCIL };

                auto const& attachmentInfo = desc.attachments.textures[i];
                assert(bool(attachments & flags[i]) == attachmentInfo.isValid());

                if (attachmentInfo.isValid()) {
                    fg::ResourceEntry<FrameGraphTexture> const& entry =
                            fg.getResourceEntryUnchecked(attachmentInfo.getHandle());
                    infos[i].handle = entry.getResource().texture;
                    infos[i].level = attachmentInfo.getLevel();
                    assert(infos[i].handle);
                }
            }
            targetInfo.target = fg.getResourceAllocator().createRenderTarget(name,
                    attachments, width, height, desc.samples,
                    infos[0], infos[1], {});
        }
    }
}

void RenderTargetResource::destroy(FrameGraph& fg) noexcept {
    if (!imported) {
        if (targetInfo.target) {
            fg.getResourceAllocator().destroyRenderTarget(targetInfo.target);
            targetInfo.target.clear();
        }
    }
}

} // namespace fg
} // namespace filament
