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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUTEXTURE_H
#define TNT_FILAMENT_BACKEND_WEBGPUTEXTURE_H

#include "DriverBase.h"
#include <backend/DriverEnums.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>

namespace filament::backend {

class WebGPUTexture : public HwTexture {
public:
    WebGPUTexture(SamplerType, uint8_t levels, TextureFormat, uint8_t samples, uint32_t width,
            uint32_t height, uint32_t depth, TextureUsage, wgpu::Device const&) noexcept;

    WebGPUTexture(WebGPUTexture const* src, uint8_t baseLevel, uint8_t levelCount) noexcept;

    [[nodiscard]] wgpu::TextureAspect getAspect() const { return mAspect; }

    [[nodiscard]] size_t getBlockWidth() const { return mBlockWidth; }

    [[nodiscard]] size_t getBlockHeight() const { return mBlockHeight; }

    [[nodiscard]] wgpu::Texture const& getTexture() const { return mTexture; }

    [[nodiscard]] wgpu::TextureView const& getDefaultTextureView() const {
        return mDefaultTextureView;
    }

    [[nodiscard]] wgpu::TextureView getOrMakeTextureView(uint8_t mipLevel, uint32_t arrayLayer);

    [[nodiscard]] wgpu::TextureFormat getWebGPUFormat() const { return mWebGPUFormat; }

    [[nodiscard]] uint32_t getArrayLayerCount() const { return mArrayLayerCount; }

    [[nodiscard]] static wgpu::TextureFormat fToWGPUTextureFormat(
            filament::backend::TextureFormat const& fFormat);

private:
    // CreateTextureR has info for a texture and sampler. Texture Views are needed for binding,
    // along with a sampler Current plan: Inherit the sampler and Texture to always exist (It is a
    // ref counted pointer) when making views. View is optional
    wgpu::Texture mTexture = nullptr;
    // format is inherited from HwTexture. This naming is to distinguish it from Filament's format
    wgpu::TextureFormat mWebGPUFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureAspect mAspect = wgpu::TextureAspect::Undefined;
    // usage is inherited from HwTexture. This naming is to distinguish it from Filament's usage
    wgpu::TextureUsage mWebGPUUsage = wgpu::TextureUsage::None;
    uint32_t mArrayLayerCount = 1;
    size_t mBlockWidth = 0;
    size_t mBlockHeight = 0;
    wgpu::TextureView mDefaultTextureView = nullptr;
    uint32_t mDefaultMipLevel = 0;
    uint32_t mDefaultBaseArrayLayer = 0;

    [[nodiscard]] wgpu::TextureView makeTextureView(const uint8_t& baseLevel,
            const uint8_t& levelCount, const uint32_t& baseArrayLayer,
            const uint32_t& arrayLayerCount, SamplerType samplerType) const noexcept;
};

}// namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUTEXTURE_H
