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

#include "FrameGraphHandle.h"

#include "FrameGraph.h"

#include "fg/ResourceAllocator.h"

namespace filament {

using namespace backend;

void FrameGraphTexture::create(FrameGraph& fg, const char* name,
        FrameGraphTexture::Descriptor const& desc) noexcept {

    // FIXME (workaround): a texture could end up with no usage if it was used as an attachment
    //  of a RenderTarget that itself was replaced by a moveResource(). In this case, the texture
    //  is simply unused.  A better fix would be to let the framegraph culling eliminate the
    //  this resource, but this is currently not working or set-up this way.
    //  Instead, we simply do nothing here.
    if (none(desc.usage)) {
        return;
    }

    assert(any(desc.usage));
    // (it means it's only used as an attachment for a rendertarget)
    uint8_t samples = desc.samples;
    if (any(desc.usage & TextureUsage::SAMPLEABLE)) {
        samples = 1; // sampleable textures can't be multi-sampled
    }
    texture = fg.getResourceAllocator().createTexture(name, desc.type, desc.levels,
            desc.format, samples, desc.width, desc.height, desc.depth, desc.usage);
}

void FrameGraphTexture::destroy(FrameGraph& fg) noexcept {
    if (texture) {
        fg.getResourceAllocator().destroyTexture(texture);
        //texture.clear(); // needed because of noop driver
    }
}

} // namespace filament
