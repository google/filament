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
    // cache.

    struct RenderPipelineKey;
    struct PipelineLayoutKey;
    struct ShaderModuleKey;

    [[nodiscard]] wgpu::RenderPipeline const& getOrCreateRenderPipeline(RenderPipelineKey const&);
    [[nodiscard]] wgpu::RenderPipeline createRenderPipeline(RenderPipelineKey const&);

    [[nodiscard]] wgpu::PipelineLayout const& getOrCreatePipelineLayout(PipelineLayoutKey const&);
    [[nodiscard]] wgpu::PipelineLayout createPipelineLayout(PipelineLayoutKey const&);

    [[nodiscard]] wgpu::BindGroupLayout const& getOrCreateTextureBindGroupLayout(
            PipelineLayoutKey const&);
    [[nodiscard]] wgpu::BindGroupLayout createTextureBindGroupLayout(PipelineLayoutKey const&);

    [[nodiscard]] wgpu::ShaderModule const& getOrCreateShaderModule(ShaderModuleKey const&);
    [[nodiscard]] wgpu::ShaderModule createShaderModule(ShaderModuleKey const&);

    struct RenderPipelineKey {
        wgpu::TextureViewDimension sourceDimension;   // 4 bytes
        wgpu::TextureFormat sourceTextureFormat;      // 4
        wgpu::TextureSampleType sourceSampleType;     // 4
        wgpu::TextureFormat destinationTextureFormat; // 4
        uint8_t sourceSampleCount;                    // 1
        SamplerMagFilter filterType;                  // 1
        uint8_t padding[2] = {};                      // 2

        bool operator==(const RenderPipelineKey& other) const;
        using Hasher = utils::hash::MurmurHashFn<RenderPipelineKey>;
    };

    struct PipelineLayoutKey {
        wgpu::TextureViewDimension sourceDimension; // 4 bytes
        wgpu::TextureSampleType sourceSampleType;   // 4
        SamplerMagFilter filterType;                // 1
        bool multisampledSource;                    // 1
        uint8_t padding[2] = {};                    // 2

        bool operator==(const PipelineLayoutKey& other) const;
        using Hasher = utils::hash::MurmurHashFn<PipelineLayoutKey>;
    };

    struct ShaderModuleKey {
        wgpu::TextureViewDimension sourceDimension;   // 4 bytes
        wgpu::TextureFormat sourceTextureFormat;      // 4
        wgpu::TextureFormat destinationTextureFormat; // 4
        bool multisampledSource;                      // 1
        uint8_t padding0[3] = {};                     // 3

        bool operator==(const ShaderModuleKey& other) const;
        using Hasher = utils::hash::MurmurHashFn<ShaderModuleKey>;
    };

    static_assert(sizeof(RenderPipelineKey) == 20,
            "RenderPipelineKey must not have implicit padding.");
    static_assert(sizeof(PipelineLayoutKey) == 12,
            "PipelineLayoutKey must not have implicit padding.");
    static_assert(sizeof(ShaderModuleKey) == 16, "ShaderModuleKey must not have implicit padding.");

    wgpu::Device mDevice;
    wgpu::Sampler mNearestSampler{ nullptr };
    wgpu::Sampler mLinearSampler{ nullptr };
    tsl::robin_map<RenderPipelineKey, wgpu::RenderPipeline, RenderPipelineKey::Hasher>
            mRenderPipelines{};
    tsl::robin_map<PipelineLayoutKey, wgpu::PipelineLayout, PipelineLayoutKey::Hasher>
            mPipelineLayouts{};
    tsl::robin_map<PipelineLayoutKey, wgpu::BindGroupLayout, PipelineLayoutKey::Hasher>
            mTextureBindGroupLayouts{};
    tsl::robin_map<ShaderModuleKey, wgpu::ShaderModule, ShaderModuleKey::Hasher> mShaderModules{};
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUBLITTER_H
