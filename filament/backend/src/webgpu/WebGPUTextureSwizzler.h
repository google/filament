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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUTEXTURESWIZZLER_H
#define TNT_FILAMENT_BACKEND_WEBGPUTEXTURESWIZZLER_H

#include "WebGPUTextureHelpers.h"

#include <backend/DriverEnums.h>

#include <tsl/robin_map.h>
#include <webgpu/webgpu_cpp.h>

#include <cstdint>

namespace filament::backend {

class WebGPUTextureSwizzler final {
public:
    explicit WebGPUTextureSwizzler(wgpu::Device const&);

    void swizzle(wgpu::Queue const&, wgpu::Texture const& source, wgpu::Texture const& destination,
            TextureSwizzle red, TextureSwizzle green, TextureSwizzle blue, TextureSwizzle alpha);

private:
    void swizzle(wgpu::CommandEncoder const&, wgpu::RenderPipeline const&,
            wgpu::BindGroupLayout const&, wgpu::Texture const& source,
            wgpu::Texture const& destination, uint32_t layer, uint32_t mipLevel,
            wgpu::Buffer const& uniformBuffer);

    [[nodiscard]] wgpu::RenderPipeline const& getOrCreateRenderPipelineFor(wgpu::TextureFormat,
            uint32_t sampleCount);

    [[nodiscard]] wgpu::RenderPipeline createRenderPipeline(wgpu::ShaderModule const&,
            wgpu::PipelineLayout const&, wgpu::TextureFormat, uint32_t sampleCount);

    [[nodiscard]] wgpu::PipelineLayout const& getOrCreatePipelineLayoutFor(ScalarSampleType,
            bool multiSampled);

    [[nodiscard]] wgpu::PipelineLayout createPipelineLayout(ScalarSampleType, bool multiSampled);

    [[nodiscard]] wgpu::BindGroupLayout const& getOrCreateBindGroupLayout(
            ScalarSampleType scalarTextureFormat, bool multiSampled);

    [[nodiscard]] wgpu::BindGroupLayout createBindGroupLayout(ScalarSampleType scalarTextureFormat,
            bool multiSampled);

    [[nodiscard]] wgpu::ShaderModule const& getOrCreateShaderModuleFor(ScalarSampleType,
            bool multiSampled);

    [[nodiscard]] wgpu::ShaderModule createShaderModule(ScalarSampleType, bool multiSampled);

    [[nodiscard]] static size_t hashBindGroupLayoutKey(ScalarSampleType textureScalarFormat,
            bool multiSampled);

    [[nodiscard]] static size_t hashPipelineLayoutKey(ScalarSampleType textureScalarFormat,
            bool multiSampled);

    [[nodiscard]] static size_t hashShaderModuleKey(ScalarSampleType textureScalarFormat,
            bool multiSampled);

    [[nodiscard]] static size_t hashPipelineKey(wgpu::TextureFormat, uint32_t sampleCount);

    wgpu::Device const& mDevice;
    tsl::robin_map<size_t, wgpu::BindGroupLayout> mBindGroupLayouts{};
    tsl::robin_map<size_t, wgpu::PipelineLayout> mPipelineLayouts{};
    tsl::robin_map<size_t, wgpu::ShaderModule> mShaderModules{};
    tsl::robin_map<size_t, wgpu::RenderPipeline> mPipelines{};
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUTEXTURESWIZZLER_H
