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

#include "fg/FrameGraphTexture.h"

#include "ResourceAllocator.h"

#include <algorithm>

namespace filament {

void FrameGraphTexture::create(ResourceAllocatorInterface& resourceAllocator, const char* name,
        Descriptor const& descriptor, Usage usage,
        bool useProtectedMemory) noexcept {
    if (useProtectedMemory) {
        // FIXME: I think we should restrict this to attachments and blit destinations only
        usage |= Usage::PROTECTED;
    }
    std::array<backend::TextureSwizzle, 4> swizzle = {
            descriptor.swizzle.r,
            descriptor.swizzle.g,
            descriptor.swizzle.b,
            descriptor.swizzle.a };
    handle = resourceAllocator.createTexture(name,
            descriptor.type, descriptor.levels, descriptor.format, descriptor.samples,
            descriptor.width, descriptor.height, descriptor.depth,
            swizzle, usage);
}

void FrameGraphTexture::destroy(ResourceAllocatorInterface& resourceAllocator) noexcept {
    if (handle) {
        resourceAllocator.destroyTexture(handle);
        handle.clear();
    }
}

FrameGraphTexture::Descriptor FrameGraphTexture::generateSubResourceDescriptor(
        Descriptor descriptor,
        SubResourceDescriptor const& srd) noexcept {
    descriptor.levels = 1;
    descriptor.width  = std::max(1u, descriptor.width >> srd.level);
    descriptor.height = std::max(1u, descriptor.height >> srd.level);
    return descriptor;
}

} // namespace filament
