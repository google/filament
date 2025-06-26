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

#ifndef TNT_FILAMENT_BACKEND_WEBGPURENDERPASSMIPMAPGENERATOR_H
#define TNT_FILAMENT_BACKEND_WEBGPURENDERPASSMIPMAPGENERATOR_H

#include <utils/Panic.h>

#include <tsl/robin_map.h>
#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <string_view>

namespace filament::backend {

class WebGPURenderPassMipmapGenerator final {
public:
    struct FormatCompatibility final {
        bool compatible{ false };
        std::string_view reason;
    };

    [[nodiscard]] static FormatCompatibility getCompatibilityFor(wgpu::TextureFormat,
            wgpu::TextureDimension, uint32_t sampleCount);

    explicit WebGPURenderPassMipmapGenerator(wgpu::Device const&);

    void generateMipmaps(wgpu::Queue const&, wgpu::Texture const&);

private:
    enum class ScalarSampleType : uint8_t {
        F32,
        I32,
        U32,
    };

    [[nodiscard]] constexpr static ScalarSampleType getScalarSampleTypeFrom(
            wgpu::TextureFormat format);

    [[nodiscard]] wgpu::RenderPipeline& getOrCreatePipelineFor(wgpu::TextureFormat);

    void generateMipmap(wgpu::CommandEncoder const&, wgpu::Texture const&,
            wgpu::RenderPipeline const&, uint32_t layer, uint32_t mipLevel);

    wgpu::Device const& mDevice;
    const wgpu::Sampler mPreviousMipLevelSampler{ nullptr };
    const wgpu::ShaderModule mShaderModule{ nullptr };
    const wgpu::BindGroupLayout mTextureBindGroupLayout{ nullptr };
    const wgpu::PipelineLayout mPipelineLayout{ nullptr };
    tsl::robin_map<uint32_t, wgpu::RenderPipeline> mPipelineByTextureFormat;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPURENDERPASSMIPMAPGENERATOR_H
