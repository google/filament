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

#ifndef TNT_WEBGPUSURFACECONFIGURATION_H
#define TNT_WEBGPUSURFACECONFIGURATION_H

#include <backend/platforms/WebGPUPlatform.h>
#include "webgpu/WebGPUDriver.h"


#include <webgpu/webgpu_cpp.h>

#include <cstdint>


namespace filament::backend {
class WebGPUSurfaceConfiguration {
public:
    explicit WebGPUSurfaceConfiguration(wgpu::Device device, wgpu::Surface surface, uint32_t width, uint32_t height, wgpu::TextureFormat);
    ~WebGPUSurfaceConfiguration();

    void ConfigureSwapChain();

    void Resize();

private:
    wgpu::Device mDevice;
    wgpu::Surface mSurface;
    wgpu::Adapter mAdapter;
    uint32_t mWidth;
    uint32_t mHeight;
    wgpu::TextureFormat mFormat;

};
} // namespace filament::backend


#endif //TNT_WEBGPUSURFACECONFIGURATION_H
