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
#include <backend/Platform.h>
#include <backend/platforms/WebGPUPlatform.h>
#if defined(__EMSCRIPTEN__)
#include <backend/platforms/WebGPUWasmPolyfill.h>
#endif

#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <memory>

namespace filament::backend {

/**
 * Owns the WebGPU objects backing an imported external image. It is shared (via std::shared_ptr)
 * between the WebGPUTexture created by the import and any views derived from it (e.g. swizzle
 * views), so that the image, and its SharedTextureMemory access scope, outlive the destruction of
 * the original texture handle.
 *
 * The destructor ends the SharedTextureMemory access and releases the retained external image,
 * which hands the buffer back to its producer. It must therefore only run once the GPU has
 * finished every submission that may reference the image. WebGPUDriver::destroyTexture ensures
 * this by deferring the release of the last reference until that submission has completed.
 */
class WebGPUExternalImageState {
public:
    /**
     * How the SharedTextureMemory access scope is ended, which depends on the device.
     */
    struct EndAccessConfig {
        // Whether EndAccess can be called. Once the texture was used, Dawn's EndAccess exports a
        // fence, which requires a shared fence feature (SharedFenceMTLSharedEvent on Metal,
        // SharedFenceSyncFD or SharedFenceVkSemaphoreOpaqueFD on Vulkan). Without it, EndAccess
        // is a validation error, so the access scope is left open and the objects are released.
        bool supported = false;
        // Whether EndAccess must chain a wgpu::SharedTextureMemoryVkImageLayoutEndState, which is
        // only valid (and required) when the device runs on Dawn's Vulkan backend.
        bool chainVkImageLayoutEndState = false;
    };

    WebGPUExternalImageState(WebGPUPlatform::ExternalImageImportResult&& imported,
            EndAccessConfig endAccessConfig) noexcept;
    ~WebGPUExternalImageState();

    WebGPUExternalImageState(WebGPUExternalImageState const&) = delete;
    WebGPUExternalImageState& operator=(WebGPUExternalImageState const&) = delete;

    [[nodiscard]] wgpu::ExternalTexture const& getExternalTexture() const noexcept {
        return mExternalTexture;
    }

private:
    wgpu::ExternalTexture mExternalTexture = nullptr;
#if !defined(__EMSCRIPTEN__)
    wgpu::SharedTextureMemory mSharedTextureMemory = nullptr;
    EndAccessConfig mEndAccessConfig{};
#endif
    // The texture created from mSharedTextureMemory, which is needed to end the access scope.
    wgpu::Texture mTexture = nullptr;
    wgpu::TextureView mPlane0View = nullptr;
    wgpu::TextureView mPlane1View = nullptr;
    // Released last, after the WebGPU objects that alias the external buffer.
    Platform::ExternalImageHandle mRetainedExternalImage{};
};

class WebGPUTexture : public HwTexture {
public:
    enum class MipmapGenerationStrategy : uint8_t {
        RENDER_PASS,
        SPD_COMPUTE_PASS,
        NONE,
    };

    /**
     * Creates a Filament texture and a texture view
     */
    WebGPUTexture(SamplerType, uint8_t levels, TextureFormat, uint8_t samples, uint32_t width,
            uint32_t height, uint32_t depth, TextureUsage, wgpu::Device const&,
            bool asynchronous = false) noexcept;

    /**
     * Creates a Filament texture wrapping an imported external image (e.g., wgpu::ExternalTexture
     * or wgpu::SharedTextureMemory).
     * When the import provides no wgpu::ExternalTexture and no default view, a 2D default view is
     * created from the imported texture if its format can be bound to an external texture slot
     * (RGBA8Unorm, BGRA8Unorm or RGBA16Float).
     * @param endAccessConfig see WebGPUExternalImageState::EndAccessConfig
     */
    WebGPUTexture(SamplerType, TextureFormat, uint32_t width, uint32_t height, TextureUsage,
            WebGPUPlatform::ExternalImageImportResult&& imported,
            WebGPUExternalImageState::EndAccessConfig endAccessConfig) noexcept;

    /**
     * Creates a "Filament Texture View", where the underlying texture is the same as the source,
     * but the view elements differ
     */
    WebGPUTexture(WebGPUTexture const* src, uint8_t baseLevel, uint8_t levelCount) noexcept;

    /**
     * @param nextSwizzle the swizzle to apply to the source texture.
     * Creates a new texture view by composing the nextSwizzle with the source which itself it
     * could be a swizzled view.
     * If the source wraps a wgpu::ExternalTexture, the view shares the source's external image
     * state and the swizzle is ignored, because external textures cannot be swizzled.
     */
    WebGPUTexture(WebGPUTexture const* source, wgpu::TextureComponentSwizzle nextSwizzle) noexcept;

    [[nodiscard]] wgpu::TextureAspect getAspect() const { return mAspect; }

    [[nodiscard]] size_t getBlockWidth() const { return mBlockWidth; }

    [[nodiscard]] size_t getBlockHeight() const { return mBlockHeight; }

    [[nodiscard]] wgpu::Texture const& getTexture() const { return mTexture; }

    [[nodiscard]] wgpu::TextureView const& getDefaultTextureView() const {
        return mDefaultTextureView;
    }

    [[nodiscard]] wgpu::ExternalTexture getExternalTexture() const noexcept {
        return mExternalImage ? mExternalImage->getExternalTexture() : nullptr;
    }

    // true if this texture wraps an imported external image (or is a view derived from one)
    [[nodiscard]] bool isExternalImage() const noexcept { return mExternalImage != nullptr; }

    // the external image state shared with views of this texture, or null
    [[nodiscard]] std::shared_ptr<WebGPUExternalImageState> const&
    getExternalImageState() const noexcept {
        return mExternalImage;
    }

    [[nodiscard]] wgpu::TextureView makeAttachmentTextureView(uint8_t mipLevel, uint32_t arrayLayer,
            uint32_t layerCount);

    [[nodiscard]] wgpu::TextureViewDimension getViewDimension() const { return mDimension; }
    [[nodiscard]] wgpu::TextureFormat getViewFormat() const { return mViewFormat; }
    [[nodiscard]] wgpu::TextureUsage getUsage() const { return mWebGPUUsage; }

    [[nodiscard]] wgpu::TextureComponentSwizzle getSwizzle() const { return mSwizzle; }

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
     * @param mipLevel The mip level into the underlying texture for which this view will reference
     * (this view will only have one mip level)
     * @param arrayLayer The layer into the underlying texture for which this view will reference
     * (this view will only have one layer)
     * @return A texture view for the MSAA sidecar texture
     */
    wgpu::TextureView makeMsaaSidecarTextureViewIfTextureSidecarExists(uint8_t samples,
            uint8_t mipLevel, uint32_t arrayLayer) const;

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
    // an undefined mSwizzle means that is not a swizzle view
    wgpu::TextureComponentSwizzle mSwizzle{};
    // non-null only for textures wrapping an imported external image (and views derived from them)
    std::shared_ptr<WebGPUExternalImageState> mExternalImage;

    [[nodiscard]] wgpu::TextureView makeTextureView(const uint8_t& baseLevel,
            const uint8_t& levelCount, const uint32_t& baseArrayLayer,
            const uint32_t& arrayLayerCount, wgpu::TextureViewDimension dimension) const noexcept;
};

}// namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUTEXTURE_H
