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

/**
 * A utility class for generating mipmaps for a texture using a series of render passes.
 */
class WebGPURenderPassMipmapGenerator final {
public:
    struct FormatCompatibility final {
        bool compatible{ false };
        std::string_view reason;
    };

    /**
     * Checks if a given texture format is compatible with render pass-based mipmap generation.
     * @return A FormatCompatibility struct indicating whether the format is compatible
     *         and a reason if not.
     */
    [[nodiscard]] static FormatCompatibility getCompatibilityFor(wgpu::TextureFormat,
            wgpu::TextureDimension, uint32_t sampleCount);

    explicit WebGPURenderPassMipmapGenerator(wgpu::Device const&);

    /**
     * IMPORTANT NOTE: when reusing a command encoder and/or textures make sure to flush/submit
     * pending commands (draws, etc.) to the GPU prior to calling this call, because texture updates
     * may otherwise (unintentionally) happen after draw commands encoded in the encoder.
     * Submitting any commands up to this point ensures the calls happen in the expected
     * sequence.
     */
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

    wgpu::Device mDevice;
    const wgpu::Sampler mPreviousMipLevelSampler{ nullptr };
    const wgpu::ShaderModule mShaderModule{ nullptr };
    const wgpu::BindGroupLayout mTextureBindGroupLayout{ nullptr };
    const wgpu::PipelineLayout mPipelineLayout{ nullptr };
    tsl::robin_map<uint32_t, wgpu::RenderPipeline> mPipelineByTextureFormat;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPURENDERPASSMIPMAPGENERATOR_H
