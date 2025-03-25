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

#include "WebGPUSurface.h"
#include <backend/platforms/WebGPUPlatform.h>
#include "WebGPUDriver.h"
#include "WebGPUConstants.h"
#include "WebGPUSurface.h"
#include <backend/platforms/WebGPUPlatform.h>

#include <webgpu/webgpu_cpp.h>

#include <utils/CString.h>
#include <utils/ostream.h>
#include <iostream>

namespace filament::backend {
WebGPUSurface::WebGPUSurface(wgpu::Surface surface, wgpu::Device device, wgpu::Adapter adapter, uint32_t width, uint32_t height)
: mSurface(surface), mDevice(device), mAdapter(adapter) /* mWidth(width) mHeight(height)*/ {
    ConfigureSurface(mSurface, mDevice, mAdapter, {});
}

WebGPUSurface::~WebGPUSurface() {
}

wgpu::Surface WebGPUSurface::ConfigureSurface(wgpu::Surface surface, wgpu::Device device,  wgpu::Adapter adapter,  wgpu::SurfaceConfiguration config ) {
FWGPU_LOGW << "Called ConfigureSurface" << utils::io::endl;

    if(config.device){
        surface.Configure(&config);
    }
    else {
        wgpu::SurfaceCapabilities surfaceCapabilities{};
        surface.GetCapabilities(adapter, &surfaceCapabilities);
        if (!surface.GetCapabilities(adapter, &surfaceCapabilities)) {
            FWGPU_LOGW << "Failed to get WebGPU surface capabilities" << utils::io::endl;
        } else {
            //printSurfaceCapabilitiesDetails(surfaceCapabilities);
        }
        wgpu::SurfaceConfiguration surfaceConfig = {};
        surfaceConfig.device = device;
        surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment;
        //TODO: get size from nativeWindow ,or hook into queue to get it from a texture
        surfaceConfig.width = 2048;
        surfaceConfig.height = 1280;
        //Should Probably make sure these formats and modes are ideal?
        surfaceConfig.format = surfaceCapabilities.formats[0];
        surfaceConfig.presentMode = surfaceCapabilities.presentModes[0];
        surfaceConfig.alphaMode = surfaceCapabilities.alphaModes[0];
        mConfig = surfaceConfig;
        surface.Configure(&surfaceConfig);
    }
    mSurface=surface;
    return surface;
}

wgpu::Surface WebGPUSurface::Resize(uint32_t width, uint32_t height) {
    if (width > 0 && height > 0) {
        wgpu::SurfaceConfiguration newConfig = {};
        newConfig.device = mConfig.device;
        newConfig.usage = mConfig.usage;
        newConfig.format = mConfig.format;
        newConfig.presentMode = mConfig.presentMode;
        newConfig.alphaMode = mConfig.alphaMode;
        newConfig.width = width;
        newConfig.height = height;
        mSurface.Configure(&newConfig);
    }
    return mSurface;
}

wgpu::Surface WebGPUSurface::ChangeFormat(wgpu::TextureFormat format) {
    //if (format) {
        wgpu::SurfaceConfiguration newConfig = {};
        newConfig.device = mConfig.device;
        newConfig.usage = mConfig.usage;
        newConfig.presentMode = mConfig.presentMode;
        newConfig.alphaMode = mConfig.alphaMode;
        newConfig.width = mConfig.width;
        newConfig.height = mConfig.height;
        newConfig.format = format;
        mSurface.Configure(&newConfig);
    //}
    return mSurface;
}

wgpu::Surface WebGPUSurface::ChangePresentMode(wgpu::PresentMode presentMode) {
    //if (mode) {
        wgpu::SurfaceConfiguration newConfig = {};
        newConfig.device = mConfig.device;
        newConfig.format = mConfig.format;
        newConfig.usage = mConfig.usage;
        newConfig.alphaMode = mConfig.alphaMode;
        newConfig.width = mConfig.width;
        newConfig.height = mConfig.height;
        newConfig.presentMode = presentMode;
        mSurface.Configure(&newConfig);
    //}
    return mSurface;
}

wgpu::Surface WebGPUSurface::ChangeAlphaMode(wgpu::CompositeAlphaMode alphaMode) {
    //if (mode) {
        wgpu::SurfaceConfiguration newConfig = {};
        newConfig.device = mConfig.device;
        newConfig.format = mConfig.format;
        newConfig.usage = mConfig.usage;
        newConfig.width = mConfig.width;
        newConfig.height = mConfig.height;
        newConfig.presentMode = mConfig.presentMode;
        newConfig.alphaMode = alphaMode;
        mSurface.Configure(&newConfig);
    //}
    return mSurface;
}



}// namespace filament::backend

