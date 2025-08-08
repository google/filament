/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUBLITTER_H
#define TNT_FILAMENT_BACKEND_WEBGPUBLITTER_H

#include <backend/DriverEnums.h>

#include <tsl/robin_map.h>
#include <webgpu/webgpu_cpp.h>

#include <cstdint>

namespace filament::backend {

/**
 * A helper class for blitting textures.
 * This class provides a `blit` method that can copy a region from a source texture to a destination
 * texture. It handles different texture formats, and can perform filtering (nearest or linear).
 * It can also be used to resolve MSAA textures.
 *
 * The blitter maintains a cache of pipelines, pipeline layouts, and shader modules to avoid
 * creating them on every blit operation.
 */
class WebGPUBlitter final {
public:
    /**
     * A struct that contains the arguments for a blit operation.
     */
    struct BlitArgs final {
        /**
         * A struct that describes a texture attachment for a blit operation.
         */
        struct Attachment final {
            // The WebGPU texture to use for the attachment.
            wgpu::Texture const& texture;
            // The texture aspect to use (e.g., color, depth, stencil).
            wgpu::TextureAspect aspect{ wgpu::TextureAspect::Undefined };
            // The origin of the region to blit, in pixels.
            wgpu::Origin2D origin{};
            // The extent of the region to blit, in pixels.
            wgpu::Extent2D extent{};
            // The mip level to use.
            uint32_t mipLevel{ 0 };
            // The array layer or depth slice to use.
            uint32_t layerOrDepth{ 0 };
        };

        // The source attachment for the blit operation.
        Attachment source;
        // The destination attachment for the blit operation.
        Attachment destination;
        // The magnification filter to use when blitting.
        SamplerMagFilter filter;
    };

    explicit WebGPUBlitter(wgpu::Device const& device);

    /**
     * Blits a region from a source texture to a destination texture.
     * @param queue The WebGPU queue to submit the blit command to.
     * @param encoder The command encoder to record the blit command into.
     * @param args The arguments for the blit operation.
     */
    void blit(wgpu::Queue const&, wgpu::CommandEncoder const&, BlitArgs const&);

private:
    /**
     * Performs a direct texture-to-texture copy.
     * ONLY should be called if canDoDirectCopy(...) is true (see it in WebGPUBlitter.cpp)
     */
    void copy(wgpu::CommandEncoder const&, BlitArgs const&);

    void createSampler(SamplerMagFilter);

    // The following methods are used to create and cache WebGPU objects.
    // The pattern is to have a `getOrCreate...` method that looks up the object in a cache,
    // and if it's not found, it calls a `create...` method to create it and then stores it in the
    // cache. A `hash...` method is used to generate a key for the cache.

    [[nodiscard]] wgpu::RenderPipeline const& getOrCreateRenderPipeline(SamplerMagFilter,
            wgpu::TextureViewDimension sourceDimension, uint32_t sourceSampleCount,
            bool depthSource, wgpu::TextureFormat destinationTextureFormat);

    [[nodiscard]] wgpu::RenderPipeline createRenderPipeline(SamplerMagFilter,
            wgpu::TextureViewDimension sourceDimension, uint32_t sourceSampleCount,
            bool depthSource, wgpu::TextureFormat destinationTextureFormat);

    [[nodiscard]] static size_t hashRenderPipelineKey(SamplerMagFilter,
            wgpu::TextureViewDimension sourceDimension, uint32_t sourceSampleCount,
            bool depthSource, wgpu::TextureFormat destinationTextureFormat);

    [[nodiscard]] wgpu::PipelineLayout const& getOrCreatePipelineLayout(SamplerMagFilter,
            wgpu::TextureViewDimension, bool multisampledSource, bool depthSource);

    [[nodiscard]] wgpu::PipelineLayout createPipelineLayout(SamplerMagFilter,
            wgpu::TextureViewDimension sourceDimension, bool multisampledSource, bool depthSource);

    [[nodiscard]] static size_t hashPipelineLayoutKey(SamplerMagFilter,
            wgpu::TextureViewDimension sourceDimension, bool multisampledSource, bool depthSource);

    [[nodiscard]] wgpu::BindGroupLayout const& getOrCreateTextureBindGroupLayout(SamplerMagFilter,
            wgpu::TextureViewDimension sourceDimension, bool multisampledSource, bool depthSource);

    [[nodiscard]] wgpu::BindGroupLayout createTextureBindGroupLayout(SamplerMagFilter,
            wgpu::TextureViewDimension sourceDimension, bool multisampledSource, bool depthSource);

    [[nodiscard]] static size_t hashTextureBindGroupLayoutKey(SamplerMagFilter,
            wgpu::TextureViewDimension sourceDimension, bool multisampledSource, bool depthSource);

    [[nodiscard]] wgpu::ShaderModule const& getOrCreateShaderModule(
            wgpu::TextureViewDimension sourceDimension, bool multisampledSource, bool depthSource,
            bool depthDestination);

    [[nodiscard]] wgpu::ShaderModule createShaderModule(wgpu::TextureViewDimension sourceDimension,
            bool multisampledSource, bool depthSource, bool depthDestination);

    [[nodiscard]] static size_t hashShaderModuleKey(wgpu::TextureViewDimension sourceDimension,
            bool multisampledSource, bool depthSource, bool depthDestination);

    // The WebGPU device.
    wgpu::Device mDevice;
    // A sampler with nearest-neighbor filtering.
    wgpu::Sampler mNearestSampler{ nullptr };
    // A sampler with linear filtering.
    wgpu::Sampler mLinearSampler{ nullptr };
    // A cache of render pipelines.
    tsl::robin_map<size_t, wgpu::RenderPipeline> mRenderPipelines{};
    // A cache of pipeline layouts.
    tsl::robin_map<size_t, wgpu::PipelineLayout> mPipelineLayouts{};
    // A cache of texture bind group layouts.
    tsl::robin_map<size_t, wgpu::BindGroupLayout> mTextureBindGroupLayouts{};
    // A cache of shader modules.
    tsl::robin_map<size_t, wgpu::ShaderModule> mShaderModules{};
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUBLITTER_H
