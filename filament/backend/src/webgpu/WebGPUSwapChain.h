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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUSWAPCHAIN_H
#define TNT_FILAMENT_BACKEND_WEBGPUSWAPCHAIN_H

#include <webgpu/webgpu_cpp.h>

#include "DriverBase.h"
#include <backend/Platform.h>

#include <cstdint>

namespace filament::backend {

class WebGPUSwapChain final : public Platform::SwapChain, HwSwapChain {
public:
    WebGPUSwapChain(wgpu::Surface&& surface, wgpu::Extent2D const& extent,
            wgpu::Adapter& adapter, wgpu::Device& device, uint64_t flags);

    WebGPUSwapChain(wgpu::Extent2D const& extent,
            wgpu::Adapter& adapter, wgpu::Device& device, uint64_t flags);

    ~WebGPUSwapChain();

    wgpu::TextureView getCurrentSurfaceTextureView(wgpu::Extent2D const&);

    void present();

private:

    [[nodiscard]] bool isHeadless() const { return mType == SwapChainType::HEADLESS; }
    void setExtent(wgpu::Extent2D const&);


    enum class SwapChainType {
        HEADLESS,
        SURFACE
    };

    wgpu::Surface mSurface = {};
    wgpu::SurfaceConfiguration mConfig = {};
    wgpu::Device mDevice = {};
    const SwapChainType mType;
    const uint32_t mHeadlessWidth;
    const uint32_t mHeadlessHeight;


};

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_WEBGPUSWAPCHAIN_H
