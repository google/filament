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


#include <webgpu/webgpu_cpp.h>

#include <utils/CString.h>
#include <utils/ostream.h>
#include <iostream>

namespace filament::backend {
WebGPUSurface::WebGPUSurface(wgpu::Surface surface, wgpu::Device device, wgpu::Adapter adapter, uint32_t width, uint32_t height)
: mSurface(surface), mDevice(device), mAdapter(adapter), mWidth(width), mHeight(height) {
    ConfigureSurface(mSurface, mDevice, mAdapter, mWidth, mHeight);
}

WebGPUSurface::~WebGPUSurface() {
}

wgpu::Surface WebGPUSurface::ConfigureSurface(wgpu::Surface surface, wgpu::Device device,  wgpu::Adapter adapter, uint32_t width, uint32_t height) {
FWGPU_LOGW << "Called ConfigureSurface" << utils::io::endl;
 wgpu::SurfaceCapabilities surfaceCapabilities{};
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
    surface.Configure(&surfaceConfig);

    return surface;
}

}// namespace filament::backend

