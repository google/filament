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

[[nodiscard]] constexpr bool hasStencil(const wgpu::TextureFormat textureFormat) {
    return textureFormat == wgpu::TextureFormat::Depth24PlusStencil8 ||
           textureFormat == wgpu::TextureFormat::Depth32FloatStencil8 ||
           textureFormat == wgpu::TextureFormat::Stencil8;
}

[[nodiscard]] constexpr bool hasDepth(const wgpu::TextureFormat textureFormat) {
    return textureFormat == wgpu::TextureFormat::Depth16Unorm ||
           textureFormat == wgpu::TextureFormat::Depth32Float ||
           textureFormat == wgpu::TextureFormat::Depth24Plus ||
           textureFormat == wgpu::TextureFormat::Depth24PlusStencil8 ||
           textureFormat == wgpu::TextureFormat::Depth32FloatStencil8;
}

class WebGPUTexture : public HwTexture {
public:
    enum class MipmapGenerationStrategy : uint8_t {
        RENDER_PASS,
        SPD_COMPUTE_PASS,
        NONE,
    };

    WebGPUTexture(SamplerType, uint8_t levels, TextureFormat, uint8_t samples, uint32_t width,
            uint32_t height, uint32_t depth, TextureUsage, wgpu::Device const&) noexcept;

    WebGPUTexture(WebGPUTexture const* src, uint8_t baseLevel, uint8_t levelCount) noexcept;

    [[nodiscard]] wgpu::TextureAspect getAspect() const { return mAspect; }

    [[nodiscard]] size_t getBlockWidth() const { return mBlockWidth; }

    [[nodiscard]] size_t getBlockHeight() const { return mBlockHeight; }

    [[nodiscard]] wgpu::Texture const& getTexture() const { return mTexture; }

    [[nodiscard]] wgpu::Texture const& getMsaaSidecarTexture(uint8_t sampleCount) const;

    [[nodiscard]] wgpu::TextureView const& getDefaultTextureView() const {
        return mDefaultTextureView;
    }

    [[nodiscard]] wgpu::TextureView getOrMakeTextureView(uint8_t mipLevel, uint32_t arrayLayer);

    [[nodiscard]] wgpu::TextureViewDimension getViewDimension() const { return mDimension; }
    [[nodiscard]] wgpu::TextureFormat getViewFormat() const { return mViewFormat; }

    [[nodiscard]] uint32_t getArrayLayerCount() const { return mArrayLayerCount; }

    [[nodiscard]] MipmapGenerationStrategy getMipmapGenerationStrategy() const {
        return mMipmapGenerationStrategy;
    }

    /**
     * Creates the MSAA sidecar texture if it has not already been created.
     * If the sidecar already exists with a different number of samples this will panic
     * (at the time of writing this WebGPU only supports MSAA with 4 samples. If this changes, then
     * multiple sidecars with different number of samples could be supported if needed, but that
     * level of complexity is not warranted at this time).
     * Additionally, if samples is <= 1 this will panic as well, as that is not an MSAA texture.
     * @param samples The number of samples the texture will have
     */
    void createMsaaSidecarTextureIfNotAlreadyCreated(uint8_t samples, wgpu::Device const&);

    /**
     * @param samples The number of samples the underlying texture supports
     * @param mipLevel The mip level into the underyling texture for which this view will reference
     * (this view will only have one mip level)
     * @param arrayLayer The layer into the underyling texture for which this view will reference
     * (this view will only have one layer)
     * @return A texture view for the MSAA sidecar texture
     */
    wgpu::TextureView makeMsaaSidecarTextureViewIfTextureSidecarExists(uint8_t samples,
            uint8_t mipLevel, uint32_t arrayLayer) const;

    /**
     * @return nullptr if a MSAA sidecar texture is not appliable, otherwise a view to one
     */
    [[nodiscard]] wgpu::TextureView makeMsaaSidecarTextureView(wgpu::Texture const&, uint8_t mipLevel, uint32_t arrayLayer) const;

    [[nodiscard]] static wgpu::TextureFormat fToWGPUTextureFormat(
            filament::backend::TextureFormat const& fFormat);

    /**
     * @param format a required texture format (can be a view to a different underlying texture
     * format)
     * @return true if multiple mip levels can be generated via a compute shader + storage binding
     * on the texture, false otherwise.
     */
    [[nodiscard]] static bool supportsMultipleMipLevelsViaStorageBinding(
            wgpu::TextureFormat format);

private:
    // the texture view's format, which may differ from the underlying texture's format itself,
    // an example of when this is needed is when certain underlying shaders don't support
    // certain formats (e.g. compute shaders), but we need a view to that format elsewhere
    // (e.g. another shader or render target)
    wgpu::TextureFormat mViewFormat = wgpu::TextureFormat::Undefined;
    MipmapGenerationStrategy mMipmapGenerationStrategy = MipmapGenerationStrategy::NONE;
    // format is inherited from HwTexture. This naming is to distinguish it from Filament's format
    // this is the underlying texture's format, not necessarily the "view" of it... see mViewFormat
    wgpu::TextureFormat mWebGPUFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureAspect mAspect = wgpu::TextureAspect::Undefined;
    // usage is inherited from HwTexture. This naming is to distinguish it from Filament's usage
    wgpu::TextureUsage mWebGPUUsage = wgpu::TextureUsage::None;
    wgpu::TextureUsage mViewUsage = wgpu::TextureUsage::None;
    wgpu::TextureViewDimension mDimension = wgpu::TextureViewDimension::Undefined;
    size_t mBlockWidth = 0;
    size_t mBlockHeight = 0;
    uint32_t mArrayLayerCount = 1;
    wgpu::Texture mTexture = nullptr;
    uint32_t mDefaultMipLevel = 0;
    uint32_t mDefaultBaseArrayLayer = 0;
    wgpu::TextureView mDefaultTextureView = nullptr;
    // At the time of writing this, WebGPU only supported 4 samples in a multi-sampled texture.
    // If that has changed, then consider updating the implementation to have a map of msaa textures
    // by sampleCount or something like that.
    // For now that complexity and cost is not warranted due to WebGPU's restrictions.
    wgpu::Texture mMsaaSidecarTexture = nullptr;

    [[nodiscard]] wgpu::TextureView makeTextureView(const uint8_t& baseLevel,
            const uint8_t& levelCount, const uint32_t& baseArrayLayer,
            const uint32_t& arrayLayerCount, SamplerType samplerType) const noexcept;
};

}// namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUTEXTURE_H
