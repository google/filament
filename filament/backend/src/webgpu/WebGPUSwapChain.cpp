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

#include "backend/DriverEnums.h"

#include <utils/Panic.h>
#include <utils/ostream.h>

#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <cstdint>
#include <sstream>

namespace {

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
void printSurfaceCapabilitiesDetails(wgpu::SurfaceCapabilities const& capabilities) {
    std::stringstream usagesStream{};
    usagesStream << capabilities.usages;
    FWGPU_LOGI << "WebGPU surface capabilities:" << utils::io::endl;
    FWGPU_LOGI << "  surface usages: " << usagesStream.str().data() << utils::io::endl;
    FWGPU_LOGI << "  surface formats (" << capabilities.formatCount << "):" << utils::io::endl;
    if (capabilities.formatCount > 0 && capabilities.formats != nullptr) {
        std::for_each(capabilities.formats, capabilities.formats + capabilities.formatCount,
                [](wgpu::TextureFormat const format) {
                    std::stringstream formatStream{};
                    formatStream << format;
                    FWGPU_LOGI << "    " << formatStream.str().data() << utils::io::endl;
                });
    }
    FWGPU_LOGI << "  surface present modes (" << capabilities.presentModeCount
               << "):" << utils::io::endl;
    if (capabilities.presentModeCount > 0 && capabilities.presentModes != nullptr) {
        std::for_each(capabilities.presentModes,
                capabilities.presentModes + capabilities.presentModeCount,
                [](wgpu::PresentMode const presentMode) {
                    std::stringstream presentModeStream{};
                    presentModeStream << presentMode;
                    FWGPU_LOGI << "    " << presentModeStream.str().data() << utils::io::endl;
                });
    }
    FWGPU_LOGI << "  surface alpha modes (" << capabilities.alphaModeCount
               << "):" << utils::io::endl;
    if (capabilities.alphaModeCount > 0 && capabilities.alphaModes != nullptr) {
        std::for_each(capabilities.alphaModes,
                capabilities.alphaModes + capabilities.alphaModeCount,
                [](wgpu::CompositeAlphaMode const alphaMode) {
                    std::stringstream alphaModeStream{};
                    alphaModeStream << alphaMode;
                    FWGPU_LOGI << "    " << alphaModeStream.str().data() << utils::io::endl;
                });
    }
}
#endif

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
void printSurfaceConfiguration(wgpu::SurfaceConfiguration const& config) {
    std::stringstream formatStream{};
    formatStream << config.format;
    std::stringstream usageStream{};
    usageStream << config.usage;
    std::stringstream alphaModeStream{};
    alphaModeStream << config.alphaMode;
    std::stringstream presentModeStream{};
    presentModeStream << config.presentMode;
    FWGPU_LOGI << "WebGPU surface configuration:" << utils::io::endl;
    FWGPU_LOGI << "  surface format: " << formatStream.str() << utils::io::endl;
    FWGPU_LOGI << "  surface usage: " << usageStream.str() << utils::io::endl;
    FWGPU_LOGI << "  surface view formats (" << config.viewFormatCount << "):" << utils::io::endl;
    if (config.viewFormatCount > 0 && config.viewFormats != nullptr) {
        std::for_each(config.viewFormats, config.viewFormats + config.viewFormatCount,
                [](wgpu::TextureFormat const viewFormat) {
                    std::stringstream viewFormatStream{};
                    viewFormatStream << viewFormat;
                    FWGPU_LOGI << "    " << viewFormatStream.str().data() << utils::io::endl;
                });
    }
    FWGPU_LOGI << "  surface alpha mode: " << alphaModeStream.str() << utils::io::endl;
    FWGPU_LOGI << "  surface width: " << config.width << utils::io::endl;
    FWGPU_LOGI << "  surface height: " << config.height << utils::io::endl;
    FWGPU_LOGI << "  surface present mode: " << presentModeStream.str() << utils::io::endl;
}
#endif

wgpu::TextureFormat selectColorFormat(size_t availableFormatsCount,
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

wgpu::PresentMode selectPresentMode(size_t availablePresentModesCount,
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

wgpu::CompositeAlphaMode selectAlphaMode(size_t availableAlphaModesCount,
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
                      "transparency is being selected as a fallback, but may not work as expected."
                   << utils::io::endl;
        return wgpu::CompositeAlphaMode::Premultiplied;
    } else {
        FILAMENT_CHECK_POSTCONDITION(unpremultipliedAvailable)
                << "No available composite alpha modes? Unknown/unhandled composite alpha mode?";
        // Again, we don't expect this in practice, but allow if for the same reason as premultiplied.
        // We prefer premultiplied to unpremuliplied until that assumption should be adjusted.
        FWGPU_LOGW << "Auto, Inherit, & Opaque composite alpha modes not supported. Filament has "
                      "historically used these. Unpremultiplied alpha composite mode for "
                      "transparency is being selected as a fallback "
                      "(premulitipled is not available either), but may not work as expected."
                   << utils::io::endl;
        return wgpu::CompositeAlphaMode::Unpremultiplied;
    }
}

void initConfig(wgpu::SurfaceConfiguration& config, wgpu::Device const& device,
        wgpu::SurfaceCapabilities const& capabilities, wgpu::Extent2D const& extent,
        bool useSRGBColorSpace) {
    config.device = device;
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.width = extent.width;
    config.height = extent.height;
    config.format =
            selectColorFormat(capabilities.formatCount, capabilities.formats, useSRGBColorSpace);
    config.presentMode =
            selectPresentMode(capabilities.presentModeCount, capabilities.presentModes);
    config.alphaMode = selectAlphaMode(capabilities.alphaModeCount, capabilities.alphaModes);
}

}// namespace

namespace filament::backend {

WebGPUSwapChain::WebGPUSwapChain(wgpu::Surface&& surface, wgpu::Extent2D const& extent,
        wgpu::Adapter& adapter, wgpu::Device& device, uint64_t flags)
    : mSurface(surface) {
    wgpu::SurfaceCapabilities capabilities = {};
    if(!surface){
        mType = SwapChainType::HEADLESS;
        mHeadlessWidth = extent.width;
        mHeadlessHeight = extent.height;
        const bool useSRGBColorSpace = (flags & SWAP_CHAIN_CONFIG_SRGB_COLORSPACE) != 0;
        initConfig(mConfig, device, capabilities, extent, useSRGBColorSpace);
    }
    mType = SwapChainType::SURFACE;
    if (!mSurface.GetCapabilities(adapter, &capabilities)) {
        FWGPU_LOGW << "Failed to get WebGPU surface capabilities" << utils::io::endl;
    } else {
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
        printSurfaceCapabilitiesDetails(capabilities);
#endif
    }
    const bool useSRGBColorSpace = (flags & SWAP_CHAIN_CONFIG_SRGB_COLORSPACE) != 0;
    initConfig(mConfig, device, capabilities, extent, useSRGBColorSpace);
    mSurface.Configure(&mConfig);
}


WebGPUSwapChain::~WebGPUSwapChain() {
    if(!isHeadless())
        mSurface.Unconfigure();
}

void WebGPUSwapChain::setExtent(wgpu::Extent2D const& currentSurfaceSize) {
    FILAMENT_CHECK_POSTCONDITION(currentSurfaceSize.width > 0 || currentSurfaceSize.height > 0)
            << "WebGPUSwapChain::setExtent: Invalid width " << currentSurfaceSize.width
            << " and/or height " << currentSurfaceSize.height << " requested.";
    if (mConfig.width != currentSurfaceSize.width || mConfig.height != currentSurfaceSize.height) {
        mConfig.width = currentSurfaceSize.width;
        mConfig.height = currentSurfaceSize.height;
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
        printSurfaceConfiguration(mConfig);
#endif
        FWGPU_LOGD << "Resizing to width " << mConfig.width << " height " << mConfig.height
                   << utils::io::endl;
        // TODO we may need to ensure no surface texture is flight when we do this. some
        //      synchronization may be necessary
        mSurface.Configure(&mConfig);
    }
}

wgpu::TextureView WebGPUSwapChain::getCurrentSurfaceTextureView( wgpu::Extent2D const& currentSurfaceSize) {

    wgpu::SurfaceTexture surfaceTexture;
    wgpu::Texture renderTarget;

    if(isHeadless()){
         wgpu::TextureViewDescriptor targetTextureViewDesc {
            .label = "headless Rendered texture view",
            .format = mConfig.format,
            .dimension = wgpu::TextureViewDimension::e2D,
            .baseMipLevel = 0,
            .mipLevelCount = 1,
            .baseArrayLayer = 0,
            .arrayLayerCount = 1,
            .aspect = wgpu::TextureAspect::All
         };
         return renderTarget.CreateView(&targetTextureViewDesc);
    }

    setExtent(currentSurfaceSize);
    mSurface.GetCurrentTexture(&surfaceTexture);
    if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal) {
        return nullptr;
    }
    // Create a view for this surface texture
    // TODO: review these initiliazations as webgpu pipeline gets mature
    wgpu::TextureViewDescriptor textureViewDescriptor = {
        .label = "texture_view",
        .format = mConfig.format,
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
