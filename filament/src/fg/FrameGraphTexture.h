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

#ifndef TNT_FILAMENT_FG_FRAMEGRAPHTEXTURE_H
#define TNT_FILAMENT_FG_FRAMEGRAPHTEXTURE_H

#include "fg/FrameGraphId.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

namespace filament {
class ResourceAllocatorInterface;
} // namespace::filament

namespace filament {

/**
 * A FrameGraph resource is a structure that declares at least:
 *      struct Descriptor;
 *      struct SubResourceDescriptor;
 *      a Usage bitmask
 * And declares and define:
 *      void create(ResourceAllocatorInterface&, const char* name, Descriptor const&, Usage,
 *              bool useProtectedMemory) noexcept;
 *      void destroy(ResourceAllocatorInterface&) noexcept;
 */
struct FrameGraphTexture {
    backend::Handle<backend::HwTexture> handle;

    /** describes a FrameGraphTexture resource */
    struct Descriptor {
        uint32_t width = 1;     // width of resource in pixel
        uint32_t height = 1;    // height of resource in pixel
        uint32_t depth = 1;     // # of images for 3D textures
        uint8_t levels = 1;     // # of levels for textures
        uint8_t samples = 0;    // 0=auto, 1=request not multisample, >1 only for NOT SAMPLEABLE
        backend::SamplerType type = backend::SamplerType::SAMPLER_2D;     // texture target type
        backend::TextureFormat format = backend::TextureFormat::RGBA8;    // resource internal format
        struct {
            using TS = backend::TextureSwizzle;
            union {
                backend::TextureSwizzle channels[4] = {
                        TS::CHANNEL_0, TS::CHANNEL_1, TS::CHANNEL_2, TS::CHANNEL_3 };
                struct {
                    backend::TextureSwizzle r, g, b, a;
                };
            };
        } swizzle;
    };

    /** Describes a FrameGraphTexture sub-resource */
    struct SubResourceDescriptor {
        uint8_t level = 0;      // resource's mip level
        uint8_t layer = 0;      // resource's layer or face
    };

    /** Usage for read and write */
    using Usage = backend::TextureUsage;
    static constexpr Usage DEFAULT_R_USAGE = Usage::SAMPLEABLE;
    static constexpr Usage DEFAULT_W_USAGE = Usage::COLOR_ATTACHMENT;

    /**
     * Create the concrete resource
     * @param resourceAllocator resource allocator for textures and such
     * @param descriptor Descriptor to the resource
     */
    void create(ResourceAllocatorInterface& resourceAllocator, const char* name,
            Descriptor const& descriptor, Usage usage, bool useProtectedMemory) noexcept;

    /**
     * Destroy the concrete resource
     * @param resourceAllocator
     */
    void destroy(ResourceAllocatorInterface& resourceAllocator) noexcept;

    /**
     * Generates the Descriptor for a subresource from its parent Descriptor and its
     * SubResourceDescriptor
     * @param descriptor the parent's descriptor
     * @param srd        this subresource's  SubResourceDescriptor
     * @return           a new Descriptor suitable for this subresource
     */
    static Descriptor generateSubResourceDescriptor(Descriptor descriptor,
            SubResourceDescriptor const& srd) noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_FG_FRAMEGRAPHTEXTURE_H
