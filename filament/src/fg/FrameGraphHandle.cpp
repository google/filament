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

#ifndef NDEBUG
#include "details/Texture.h"    // only needed for assert()
#endif

#include <assert.h>

namespace filament {

using namespace backend;

void FrameGraphTexture::create(FrameGraph& fg, const char* name,
        FrameGraphTexture::Descriptor const& desc) noexcept {

    // FIXME (workaround): a texture could end up with no usage if it was used as an attachment
    //  of a RenderTarget that itself was replaced by a moveResource(). In this case, the texture
    //  is simply unused.  A better fix would be to let the framegraph culling eliminate
    //  this resource, but this is currently not working or set-up this way.
    //  Instead, we simply do nothing here.
    if (none(desc.usage)) {
        return;
    }

    assert(any(desc.usage));

    // texture that can't be sampled can't have LOD -- they obviously can't be accessed
    // note: this could happen if a texture was created with LODs, but a later pass didn't
    // actually sample from it.
    uint8_t levels = desc.levels;
    if (!(desc.usage & TextureUsage::SAMPLEABLE)) {
        levels = 1;
    }
    assert(levels <= FTexture::maxLevelCount(desc.width, desc.height));

    uint8_t samples = desc.samples;
    assert(samples <= 1 || none(desc.usage & TextureUsage::SAMPLEABLE));
    if (any(desc.usage & TextureUsage::SAMPLEABLE)) {
        // Sampleable textures can't be multi-sampled
        // This should never happen (and will be caught by the assert above), but just to be safe,
        // we reset the sample count to 1 in that case.
        samples = 1;
    }

    texture = fg.getResourceAllocator().createTexture(name, desc.type, levels,
            desc.format, samples, desc.width, desc.height, desc.depth, desc.usage);

    assert(texture);
}

void FrameGraphTexture::destroy(FrameGraph& fg) noexcept {
    if (texture) {
        fg.getResourceAllocator().destroyTexture(texture);
    }
}

} // namespace filament
