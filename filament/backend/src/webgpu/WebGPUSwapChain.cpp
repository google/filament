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

#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_cpp.h>

#include <utils/ostream.h>

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

void initConfig(wgpu::SurfaceConfiguration& config, wgpu::Device& device,
        wgpu::SurfaceCapabilities const& capabilities) {
    config.device = device;
    config.usage = wgpu::TextureUsage::RenderAttachment;
    // TODO logic for selecting appropriate color format
    config.format = capabilities.formats[0];
    // TODO logic for selecting appropriate present mode
    config.presentMode = capabilities.presentModes[0];
    // TODO logic for selecting appropriate alpha mode
    config.alphaMode = capabilities.alphaModes[0];
}

}// namespace

namespace filament::backend {

WebGPUSwapChain::WebGPUSwapChain(wgpu::Surface&& surface, wgpu::Adapter& adapter, wgpu::Device& device)
    : mSurface(surface) {
    wgpu::SurfaceCapabilities capabilities = {};
    if (!mSurface.GetCapabilities(adapter, &capabilities)) {
        FWGPU_LOGW << "Failed to get WebGPU surface capabilities" << utils::io::endl;
    } else {
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
        printSurfaceCapabilitiesDetails(capabilities);
#endif
    }
    initConfig(mConfig, device, capabilities);
}

WebGPUSwapChain::~WebGPUSwapChain() {
    if (mConfigured) {
        mSurface.Unconfigure();
        mConfigured = false;
    }
}

void WebGPUSwapChain::resize(uint32_t width, uint32_t height) {
    FWGPU_LOGD << "Called WebGPUSwapChain::resize(width=" << width << ", height=" << height << ")"
               << utils::io::endl;
    if (width < 1 || height < 1) {
        // should we panic or do nothing if we get 0s? expected?
        FWGPU_LOGW << "Non-zero width (" << width << ") and/or height (" << height
                   << ") requested, which is invalid. Ignoring request to resize."
                   << utils::io::endl;
        return;
    }
    if (mConfig.width == width && mConfig.height == height && mConfigured) {
        // nothing to do (already configured with this extent)
        FWGPU_LOGW << "WebGPUSwapChain::resize(...) called with the same width and height as "
                      "currently configured. Ignoring request to resize."
                   << utils::io::endl;
        return;
    }
    mConfig.width = width;
    mConfig.height = height;
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    printSurfaceConfiguration(mConfig);
#endif
    mSurface.Configure(&mConfig);
    mConfigured = true;
}

//void WebGPUSwapChain::setFormat(wgpu::TextureFormat format) {
//    if (mConfig.format == format && mConfigured) {
//        // nothing to do (already configured with this extent)
//        FWGPU_LOGW << "WebGPUSwapChain::setFormat(...) called with the same format as "
//                      "currently configured. Ignoring request to reformat."
//                   << utils::io::endl;
//        return;
//    }
//    mConfig.format = format;
//    // is this ok for the headless mode?
//    if (mConfig.width > 0 && mConfig.height > 0) {
//#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
//        printSurfaceConfiguration(mConfig);
//#endif
//        mSurface.Configure(&mConfig);
//        mConfigured = true;
//    }
//}

void WebGPUSwapChain::GetCurrentTexture(wgpu::SurfaceTexture* texture) {
    mSurface.GetCurrentTexture(texture);
}

}// namespace filament::backend
