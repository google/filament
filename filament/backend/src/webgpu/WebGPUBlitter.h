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
  * A blit essentially writes pixels to a region of one texture given a region of another texture.
  * Such a process can be a byte-by-byte copy when possible, while other scenarios involve various
  * transformations, such as pixel format conversion, image scaling, filtering/sampling
  * (such as down sampling), or even multi-sample resolves. Some blits can be faster than others,
  * depending on what is needed; for example, a byte-by-byte copy is generally faster than other
  * operations.
  *
  * This class is responsible for performing blits throughout the lifecycle of its owner/caller,
  * e.g. the WebGPU backend/driver, where it can leverage state to optimize such blit calls over time.
  */
class WebGPUBlitter final {
public:
    struct BlitArgs final {
        struct Attachment final {
            wgpu::Texture const& texture;
            wgpu::TextureAspect aspect{ wgpu::TextureAspect::Undefined };
            wgpu::Origin2D origin{};
            wgpu::Extent2D extent{};
            uint32_t mipLevel{ 0 };
            uint32_t layerOrDepth{ 0 };
        };

        Attachment source;
        Attachment destination;
        SamplerMagFilter filter;
    };

    explicit WebGPUBlitter(wgpu::Device const& device);

    /**
     * IMPORTANT NOTE: when reusing a command encoder and/or textures make sure to flush/submit
     * pending commands (draws, etc.) to the GPU prior to calling this blit, because texture updates
     * may otherwise (unintentionally) happen after draw commands encoded in the encoder.
     * Submitting any commands up to this point ensures the calls happen in the expected
     * sequence.
     */
    void blit(wgpu::Queue const&, wgpu::CommandEncoder const&, BlitArgs const&);

private:
    /**
     * ONLY should be called if canDoDirectCopy(...) is true (see it in WebGPUBlitter.cpp)
     */
    void copyByteByByte(wgpu::CommandEncoder const&, BlitArgs const&);

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

    wgpu::Device mDevice;
    wgpu::Sampler mNearestSampler{ nullptr };
    wgpu::Sampler mLinearSampler{ nullptr };
    tsl::robin_map<size_t, wgpu::RenderPipeline> mRenderPipelines{};
    tsl::robin_map<size_t, wgpu::PipelineLayout> mPipelineLayouts{};
    tsl::robin_map<size_t, wgpu::BindGroupLayout> mTextureBindGroupLayouts{};
    tsl::robin_map<size_t, wgpu::ShaderModule> mShaderModules{};
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUBLITTER_H
