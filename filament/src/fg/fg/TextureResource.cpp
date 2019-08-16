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

#include "TextureResource.h"

#include "fg/FrameGraph.h"
#include "fg/ResourceAllocator.h"

namespace filament {

using namespace backend;

namespace fg {

VirtualResource::~VirtualResource() = default;

TextureResource::TextureResource(const char* name, uint16_t id,
        Type type, FrameGraphResource::Descriptor desc, bool imported) noexcept
        : name(name), id(id), type(type), imported(imported), desc(desc) {
}

TextureResource::~TextureResource() noexcept {
    if (!imported) {
        assert(!texture);
    }
}

void TextureResource::create(FrameGraph& fg) noexcept {
    // some sanity check
    if (!imported) {
        assert(usage);
        // (it means it's only used as an attachment for a rendertarget)
        uint8_t samples = desc.samples;
        if (usage & TextureUsage::SAMPLEABLE) {
            samples = 1; // sampleable textures can't be multi-sampled
        }
        texture = fg.getResourceAllocator().createTexture(name, desc.type, desc.levels,
                desc.format, samples, desc.width, desc.height, desc.depth, usage);
    }
}

void TextureResource::destroy(FrameGraph& fg) noexcept {
    // we don't own the handles of imported resources
    if (!imported) {
        if (texture) {
            fg.getResourceAllocator().destroyTexture(texture);
            texture.clear(); // needed because of noop driver
        }
    }
}

} // namespace fg
} // namespace filament
