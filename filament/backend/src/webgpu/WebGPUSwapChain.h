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
            wgpu::Adapter const& adapter, wgpu::Device const& device, uint64_t flags);

    WebGPUSwapChain( wgpu::Extent2D const& extent,
            wgpu::Adapter const& adapter, wgpu::Device const& device, uint64_t flags);

    ~WebGPUSwapChain();

    [[nodiscard]] wgpu::TextureFormat getColorFormat() const { return mConfig.format; }

    [[nodiscard]] wgpu::TextureFormat getDepthFormat() const { return mDepthFormat; }

    [[nodiscard]] wgpu::TextureView getCurrentTextureView(wgpu::Extent2D const& extent);
    [[nodiscard]] wgpu::TextureView getCurrentTextureView();

    [[nodiscard]] wgpu::TextureView getDepthTextureView() const { return mDepthTextureView; }

    [[nodiscard]] bool isHeadless() const { return mType == SwapChainType::HEADLESS; }

    void present();

private:

    void setExtent(wgpu::Extent2D const&);

    wgpu::Device mDevice = nullptr;
    enum class SwapChainType {
        HEADLESS,
        SURFACE
    };

    wgpu::Surface mSurface = {};
    wgpu::SurfaceConfiguration mConfig = {};
    bool mNeedStencil = false;
    wgpu::TextureFormat mDepthFormat = wgpu::TextureFormat::Undefined;
    wgpu::Texture mDepthTexture = nullptr;
    wgpu::TextureView mDepthTextureView = nullptr;
    const SwapChainType mType;
    const uint32_t mHeadlessWidth;
    const uint32_t mHeadlessHeight;

    ///TODO: eventually config for double or triple buffering
    static constexpr uint32_t mHeadlessBufferCount = 3;
    uint32_t mHeadlessBufferIndex = 0;
    std::array<wgpu::Texture, 3> mRenderTargetTextures;
    std::array<wgpu::TextureView, 3> mRenderTargetViews;
};

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_WEBGPUSWAPCHAIN_H
