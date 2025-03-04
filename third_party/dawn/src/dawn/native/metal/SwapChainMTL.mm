// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/metal/SwapChainMTL.h"

#include "dawn/native/ChainUtils.h"
#include "dawn/native/Surface.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/TextureMTL.h"

#import <QuartzCore/CAMetalLayer.h>

namespace dawn::native::metal {

// static
ResultOrError<Ref<SwapChain>> SwapChain::Create(Device* device,
                                                Surface* surface,
                                                SwapChainBase* previousSwapChain,
                                                const SurfaceConfiguration* config) {
    Ref<SwapChain> swapchain = AcquireRef(new SwapChain(device, surface, config));
    DAWN_TRY(swapchain->Initialize(previousSwapChain));
    return swapchain;
}

SwapChain::SwapChain(DeviceBase* dev, Surface* sur, const SurfaceConfiguration* config)
    : SwapChainBase(dev, sur, config) {}

SwapChain::~SwapChain() = default;

MaybeError SwapChain::Initialize(SwapChainBase* previousSwapChain) {
    DAWN_ASSERT(GetSurface()->GetType() == Surface::Type::MetalLayer);

    if (previousSwapChain != nullptr) {
        // TODO(crbug.com/dawn/269): figure out what should happen when surfaces are used by
        // multiple backends one after the other. It probably needs to block until the backend
        // and GPU are completely finished with the previous swapchain.
        DAWN_INVALID_IF(previousSwapChain->GetBackendType() != wgpu::BackendType::Metal,
                        "Metal SwapChain cannot switch backend types from %s to %s.",
                        previousSwapChain->GetBackendType(), wgpu::BackendType::Metal);

        previousSwapChain->DetachFromSurface();
    }

    mLayer = static_cast<CAMetalLayer*>(GetSurface()->GetMetalLayer());
    DAWN_ASSERT(mLayer != nullptr);

    CGSize size = {};
    size.width = GetWidth();
    size.height = GetHeight();
    [*mLayer setDrawableSize:size];

    [*mLayer setFramebufferOnly:(GetUsage() == wgpu::TextureUsage::RenderAttachment)];
    [*mLayer setDevice:ToBackend(GetDevice())->GetMTLDevice()];
    [*mLayer setPixelFormat:MetalPixelFormat(GetDevice(), GetFormat())];

    // TODO(dawn:2320): Check that this behaves as expected by the spec
    [*mLayer setOpaque:(GetAlphaMode() != wgpu::CompositeAlphaMode::Premultiplied)];

#if DAWN_PLATFORM_IS(MACOS)
    [*mLayer setDisplaySyncEnabled:(GetPresentMode() != wgpu::PresentMode::Immediate)];
#endif  // DAWN_PLATFORM_IS(MACOS)

    // There is no way to control Fifo vs. Mailbox in Metal.

    return {};
}

MaybeError SwapChain::PresentImpl() {
    DAWN_ASSERT(mCurrentDrawable != nullptr);
    [*mCurrentDrawable present];

    mTexture->APIDestroy();
    mTexture = nullptr;

    mCurrentDrawable = nullptr;

    return {};
}

ResultOrError<SwapChainTextureInfo> SwapChain::GetCurrentTextureImpl() {
    @autoreleasepool {
        DAWN_ASSERT(mCurrentDrawable == nullptr);
        mCurrentDrawable = [*mLayer nextDrawable];

        TextureDescriptor textureDesc = GetSwapChainBaseTextureDescriptor(this);

        mTexture = Texture::CreateWrapping(ToBackend(GetDevice()), Unpack(&textureDesc),
                                           NSPRef<id<MTLTexture>>([*mCurrentDrawable texture]));

        SwapChainTextureInfo info;
        info.texture = mTexture;
        info.status = wgpu::SurfaceGetCurrentTextureStatus::Success;
        // TODO(dawn:2320): Check for optimality
        info.suboptimal = false;
        return info;
    }
}

void SwapChain::DetachFromSurfaceImpl() {
    DAWN_ASSERT((mTexture == nullptr) == (mCurrentDrawable == nullptr));

    if (mTexture != nullptr) {
        mTexture->APIDestroy();
        mTexture = nullptr;

        mCurrentDrawable = nullptr;
    }
}

}  // namespace dawn::native::metal
