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

#include "webgpu/WebGPUSwapChain.h"

#include "webgpu/WebGPUConstants.h"
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
#include "webgpu/WebGPUStrings.h"
#endif

#include "backend/DriverEnums.h"

#include <utils/Panic.h>

#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <cstdint>

namespace filament::backend {

namespace {

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
void printSurfaceCapabilitiesDetails(wgpu::SurfaceCapabilities const& capabilities) {
    FWGPU_LOGI << "WebGPU surface capabilities:";
    FWGPU_LOGI << "  surface usages: " << webGPUPrintableToString(capabilities.usages);
    FWGPU_LOGI << "  surface formats (" << capabilities.formatCount << "):";
    if (capabilities.formatCount > 0 && capabilities.formats != nullptr) {
        std::for_each(capabilities.formats, capabilities.formats + capabilities.formatCount,
                [](wgpu::TextureFormat const format) {
                    FWGPU_LOGI << "    " << webGPUPrintableToString(format);
                });
    }
    FWGPU_LOGI << "  surface present modes (" << capabilities.presentModeCount << "):";
    if (capabilities.presentModeCount > 0 && capabilities.presentModes != nullptr) {
        std::for_each(capabilities.presentModes,
                capabilities.presentModes + capabilities.presentModeCount,
                [](wgpu::PresentMode const presentMode) {
                    FWGPU_LOGI << "    " << webGPUPrintableToString(presentMode);
                });
    }
    FWGPU_LOGI << "  surface alpha modes (" << capabilities.alphaModeCount << "):";
    if (capabilities.alphaModeCount > 0 && capabilities.alphaModes != nullptr) {
        std::for_each(capabilities.alphaModes,
                capabilities.alphaModes + capabilities.alphaModeCount,
                [](wgpu::CompositeAlphaMode const alphaMode) {
                    FWGPU_LOGI << "    " << webGPUPrintableToString(alphaMode);
                });
    }
}

void printSurfaceConfiguration(wgpu::SurfaceConfiguration const& config,
        wgpu::TextureFormat depthFormat) {
    FWGPU_LOGI << "WebGPU surface configuration:";
    FWGPU_LOGI << "  surface format: " << webGPUPrintableToString(config.format);
    FWGPU_LOGI << "  surface usage: " << webGPUPrintableToString(config.usage);
    FWGPU_LOGI << "  surface view formats (" << config.viewFormatCount << "):";
    if (config.viewFormatCount > 0 && config.viewFormats != nullptr) {
        std::for_each(config.viewFormats, config.viewFormats + config.viewFormatCount,
                [](wgpu::TextureFormat const viewFormat) {
                    FWGPU_LOGI << "    " << webGPUPrintableToString(viewFormat);
                });
    }
    FWGPU_LOGI << "  surface alpha mode: " << webGPUPrintableToString(config.alphaMode);
    FWGPU_LOGI << "  surface width: " << config.width;
    FWGPU_LOGI << "  surface height: " << config.height;
    FWGPU_LOGI << "  surface present mode: " << webGPUPrintableToString(config.presentMode);
    FWGPU_LOGI << "WebGPU selected depth format: " << webGPUPrintableToString(depthFormat);
}
#endif// FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)

[[nodiscard]] constexpr wgpu::TextureFormat selectColorFormat(size_t availableFormatsCount,
        wgpu::TextureFormat const* availableFormats, bool useSRGBColorSpace) {
    const std::array expectedColorFormats =
            useSRGBColorSpace ?
                              std::array{
                                  wgpu::TextureFormat::RGBA8UnormSrgb,
                                  wgpu::TextureFormat::BGRA8UnormSrgb }
                              : std::array{
                                    wgpu::TextureFormat::RGBA8Unorm,
                                    wgpu::TextureFormat::BGRA8Unorm };
    auto const firstFoundColorFormat = std::find_first_of(expectedColorFormats.begin(),
            expectedColorFormats.end(), availableFormats, availableFormats + availableFormatsCount);
    FILAMENT_CHECK_POSTCONDITION(firstFoundColorFormat != expectedColorFormats.end())
            << "Cannot find a suitable WebGPU swapchain "
            << (useSRGBColorSpace ? "sRGB" : "non-standard (e.g. linear) RGB") << " color format";
    return *firstFoundColorFormat;
}

[[nodiscard]] constexpr wgpu::TextureFormat selectDepthFormat(bool depth32FloatStencil8Enabled,
        bool needStencil) {
    if (needStencil) {
        if (depth32FloatStencil8Enabled) {
            return wgpu::TextureFormat::Depth32FloatStencil8;
        } else {
            return wgpu::TextureFormat::Depth24PlusStencil8;
        }
    } else {
        return wgpu::TextureFormat::Depth32Float;
    }
}

[[nodiscard]] constexpr wgpu::PresentMode selectPresentMode(size_t availablePresentModesCount,
        wgpu::PresentMode const* availablePresentModes) {
    // Verify that our chosen present mode is supported. In practice all devices support the FIFO
    // mode, but we check for it anyway for completeness.  (and to avoid validation warnings)
    const wgpu::PresentMode desiredPresentMode = wgpu::PresentMode::Fifo;
    FILAMENT_CHECK_POSTCONDITION(
            std::any_of(availablePresentModes, availablePresentModes + availablePresentModesCount,
                    [](const wgpu::PresentMode availablePresentMode) {
                        return availablePresentMode == desiredPresentMode;
                    }))
            << "Cannot find a suitable WebGPU swapchain present mode";
    return desiredPresentMode;
}

[[nodiscard]] constexpr wgpu::CompositeAlphaMode selectAlphaMode(size_t availableAlphaModesCount,
        wgpu::CompositeAlphaMode const* availableAlphaModes) {
    bool autoAvailable = false;
    bool inheritAvailable = false;
    bool opaqueAvailable = false;
    bool premultipliedAvailable = false;
    bool unpremultipliedAvailable = false;
    std::for_each(availableAlphaModes, availableAlphaModes + availableAlphaModesCount,
            [&](const wgpu::CompositeAlphaMode alphMode) {
                switch (alphMode) {
                    // in practice, the surface capabilities would not list Auto,
                    // but for completeness and defensive programming we can leverage it
                    // if it explicitly comes back as available/supported
                    case wgpu::CompositeAlphaMode::Auto:
                        autoAvailable = true;
                        break;
                    case wgpu::CompositeAlphaMode::Opaque:
                        opaqueAvailable = true;
                        break;
                    case wgpu::CompositeAlphaMode::Premultiplied:
                        premultipliedAvailable = true;
                        break;
                    case wgpu::CompositeAlphaMode::Unpremultiplied:
                        unpremultipliedAvailable = true;
                        break;
                    case wgpu::CompositeAlphaMode::Inherit:
                        inheritAvailable = true;
                        break;
                }
            });
    if (autoAvailable || inheritAvailable || opaqueAvailable) {
        return wgpu::CompositeAlphaMode::Auto;
    } else if (premultipliedAvailable) {
        // In practice, we do not expect this to possibly happen, as opaque should be supported,
        // which we select first. However, if the underlying system actually does not support
        // opaque we allow it to be tested, but warn that this may not likely work as they expect
        // (untested territory).
        // We prefer premultiplied to unpremuliplied until that assumption should be adjusted.
        FWGPU_LOGW << "Auto, Inherit, & Opaque composite alpha modes not supported. Filament has "
                      "historically used these. Premultiplied alpha composite mode for "
                      "transparency is being selected as a fallback, but may not work as expected.";
        return wgpu::CompositeAlphaMode::Premultiplied;
    } else {
        FILAMENT_CHECK_POSTCONDITION(unpremultipliedAvailable)
                << "No available composite alpha modes? Unknown/unhandled composite alpha mode?";
        // Again, we don't expect this in practice, but allow if for the same reason as premultiplied.
        // We prefer premultiplied to unpremuliplied until that assumption should be adjusted.
        FWGPU_LOGW << "Auto, Inherit, & Opaque composite alpha modes not supported. Filament has "
                      "historically used these. Unpremultiplied alpha composite mode for "
                      "transparency is being selected as a fallback "
                      "(premulitipled is not available either), but may not work as expected.";
        return wgpu::CompositeAlphaMode::Unpremultiplied;
    }
}

void initConfig(wgpu::SurfaceConfiguration& config, wgpu::Device const& device,
        wgpu::SurfaceCapabilities const& capabilities, wgpu::Extent2D const& surfaceSize,
        bool useSRGBColorSpace) {
    config.device = device;
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.width = surfaceSize.width;
    config.height = surfaceSize.height;
    config.format =
            selectColorFormat(capabilities.formatCount, capabilities.formats, useSRGBColorSpace);
    config.presentMode =
            selectPresentMode(capabilities.presentModeCount, capabilities.presentModes);
    config.alphaMode = selectAlphaMode(capabilities.alphaModeCount, capabilities.alphaModes);
}

[[nodiscard]] wgpu::Texture createDepthTexture(wgpu::Device const& device,
        wgpu::Extent2D const& extent, wgpu::TextureFormat depthFormat) {
    wgpu::TextureDescriptor descriptor{ .label = "depth_texture",
        .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment,
        .dimension = wgpu::TextureDimension::e2D,
        .size = { .width = extent.width, .height = extent.height, .depthOrArrayLayers = 1 },
        .format = depthFormat,
        .mipLevelCount = 1,
        .sampleCount = 1,
        .viewFormatCount = 1,
        .viewFormats = &depthFormat
    };
    wgpu::Texture depthTexture = device.CreateTexture(&descriptor);
    FILAMENT_CHECK_POSTCONDITION(depthTexture) << "Failed to create depth texture with width "
                                               << extent.width << " and height " << extent.height;
    return depthTexture;
}

[[nodiscard]] wgpu::TextureView createDepthTextureView(wgpu::Texture const& depthTexture,
        wgpu::TextureFormat const& depthFormat, bool const needStencil) {
    wgpu::TextureViewDescriptor descriptor{
        .label = "depth_texture_view",
        .dimension = wgpu::TextureViewDimension::e2D,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .aspect = wgpu::TextureAspect::DepthOnly,
        .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment
    };
    if (needStencil) {
        descriptor.aspect = wgpu::TextureAspect::All;
        descriptor.format = depthFormat;
    } else {
        descriptor.aspect = wgpu::TextureAspect::DepthOnly;
        if (depthFormat == wgpu::TextureFormat::Depth32FloatStencil8) {
            descriptor.format = wgpu::TextureFormat::Depth32Float;
        } else if (depthFormat == wgpu::TextureFormat::Depth24PlusStencil8) {
            descriptor.format = wgpu::TextureFormat::Depth24Plus;
        } else {
            descriptor.format = depthFormat;
        }
    }
    wgpu::TextureView depthTextureView = depthTexture.CreateView(&descriptor);
    FILAMENT_CHECK_POSTCONDITION(depthTextureView) << "Failed to create depth texture view";
    return depthTextureView;
}

}// namespace

WebGPUSwapChain::WebGPUSwapChain(wgpu::Surface&& surface, wgpu::Extent2D const& surfaceSize,
        wgpu::Adapter const& adapter, wgpu::Device const& device, uint64_t flags)
    : mDevice(device),
      mSurface(surface),
      mNeedStencil((flags & SWAP_CHAIN_HAS_STENCIL_BUFFER) != 0),
      mDepthFormat(selectDepthFormat(device.HasFeature(wgpu::FeatureName::Depth32FloatStencil8),
              mNeedStencil)),
      mDepthTexture(createDepthTexture(device, surfaceSize, mDepthFormat)),
      mDepthTextureView(createDepthTextureView(mDepthTexture, mDepthFormat, mNeedStencil)) {
    wgpu::SurfaceCapabilities capabilities = {};
    if (!mSurface.GetCapabilities(adapter, &capabilities)) {
        FWGPU_LOGW << "Failed to get WebGPU surface capabilities";
    } else {
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
        printSurfaceCapabilitiesDetails(capabilities);
#endif
    }
    const bool useSRGBColorSpace = (flags & SWAP_CHAIN_CONFIG_SRGB_COLORSPACE) != 0;
    initConfig(mConfig, device, capabilities, surfaceSize, useSRGBColorSpace);
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    printSurfaceConfiguration(mConfig, mDepthFormat);
#endif
    mSurface.Configure(&mConfig);
}

WebGPUSwapChain::~WebGPUSwapChain() { mSurface.Unconfigure(); }

void WebGPUSwapChain::setExtent(wgpu::Extent2D const& currentSurfaceSize) {
    FILAMENT_CHECK_POSTCONDITION(currentSurfaceSize.width > 0 || currentSurfaceSize.height > 0)
            << "WebGPUSwapChain::setExtent: Invalid width " << currentSurfaceSize.width
            << " and/or height " << currentSurfaceSize.height << " requested.";
    if (mConfig.width != currentSurfaceSize.width || mConfig.height != currentSurfaceSize.height) {
        mConfig.width = currentSurfaceSize.width;
        mConfig.height = currentSurfaceSize.height;
        FWGPU_LOGD << "Resizing to width " << mConfig.width << " height " << mConfig.height;
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
        printSurfaceConfiguration(mConfig, mDepthFormat);
#endif
        // TODO we may need to ensure no surface texture is in flight when we do this. some
        //      synchronization may be necessary
        mSurface.Configure(&mConfig);
        mDepthTexture = createDepthTexture(mDevice, currentSurfaceSize, mDepthFormat);
        mDepthTextureView = createDepthTextureView(mDepthTexture, mDepthFormat, mNeedStencil);
    }
}

wgpu::TextureView WebGPUSwapChain::getCurrentSurfaceTextureView(
        wgpu::Extent2D const& currentSurfaceSize) {
    setExtent(currentSurfaceSize);
    wgpu::SurfaceTexture surfaceTexture;
    mSurface.GetCurrentTexture(&surfaceTexture);
    if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal) {
        return nullptr;
    }
    // Create a view for this surface texture
    // TODO: review these initiliazations as webgpu pipeline gets mature
    wgpu::TextureViewDescriptor textureViewDescriptor = {
        .label = "surface_texture_view",
        .format = surfaceTexture.texture.GetFormat(),
        .dimension = wgpu::TextureViewDimension::e2D,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1
    };
    return surfaceTexture.texture.CreateView(&textureViewDescriptor);
}

void WebGPUSwapChain::present() {
    assert_invariant(mSurface);
    mSurface.Present();
}

}// namespace filament::backend
