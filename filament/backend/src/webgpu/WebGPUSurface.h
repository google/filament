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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUSURFACE_H
#define TNT_FILAMENT_BACKEND_WEBGPUSURFACE_H

#include <webgpu/webgpu_cpp.h>

#include <cstdint>

namespace filament::backend {

class WebGPUSurface {
public:
    WebGPUSurface(wgpu::Surface&& surface, wgpu::Adapter& adapter, wgpu::Device& device);
    ~WebGPUSurface();

    void resize(uint32_t width, uint32_t height);
    void GetCurrentTexture(wgpu::SurfaceTexture*);

private:
    wgpu::Surface mSurface = {};
    wgpu::SurfaceConfiguration mConfig = {};
    bool mConfigured = false;
};

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_WEBGPUSURFACE_H
