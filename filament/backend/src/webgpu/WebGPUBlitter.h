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

class WebGPUBlitter final {
public:
    struct BlitArgs final {
        struct Attachment final {
            wgpu::Texture const& texture;
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

    void blit(wgpu::Queue const&, wgpu::CommandEncoder const&, BlitArgs const&);

private:
    void createSampler(SamplerMagFilter);

    [[nodiscard]] wgpu::RenderPipeline const& getOrCreateRenderPipeline(SamplerMagFilter,
            wgpu::TextureViewDimension sourceDimension, uint32_t sourceSampleCount,
            wgpu::TextureFormat destinationTextureFormat);

    [[nodiscard]] wgpu::RenderPipeline createRenderPipeline(SamplerMagFilter,
            wgpu::TextureViewDimension sourceDimension, uint32_t sourceSampleCount,
            wgpu::TextureFormat destinationTextureFormat);

    [[nodiscard]] static size_t hashRenderPipelineKey(SamplerMagFilter,
            wgpu::TextureViewDimension sourceDimension, uint32_t sourceSampleCount);

    [[nodiscard]] wgpu::PipelineLayout const& getOrCreatePipelineLayout(SamplerMagFilter,
            wgpu::TextureViewDimension, bool multisampledSource);

    [[nodiscard]] wgpu::PipelineLayout createPipelineLayout(SamplerMagFilter,
            wgpu::TextureViewDimension sourceDimension, bool multisampledSource);

    [[nodiscard]] static size_t hashPipelineLayoutKey(SamplerMagFilter,
            wgpu::TextureViewDimension sourceDimension, bool multisampledSource);

    [[nodiscard]] wgpu::BindGroupLayout const& getOrCreateTextureBindGroupLayout(SamplerMagFilter,
            wgpu::TextureViewDimension sourceDimension, bool multisampledSource);

    [[nodiscard]] wgpu::BindGroupLayout createTextureBindGroupLayout(SamplerMagFilter,
            wgpu::TextureViewDimension sourceDimension, bool multisampledSource);

    [[nodiscard]] static size_t hashTextureBindGroupLayoutKey(SamplerMagFilter,
            wgpu::TextureViewDimension sourceDimension, bool multisampledSource);

    [[nodiscard]] wgpu::ShaderModule const& getOrCreateShaderModule(
            wgpu::TextureViewDimension sourceDimension, bool multisampledSource);

    [[nodiscard]] wgpu::ShaderModule createShaderModule(wgpu::TextureViewDimension sourceDimension,
            bool multisampledSource);

    [[nodiscard]] static size_t hashShaderModuleKey(wgpu::TextureViewDimension sourceDimension,
            bool multisampledSource);

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
