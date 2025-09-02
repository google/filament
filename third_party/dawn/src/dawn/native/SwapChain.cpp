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

#include "dawn/native/SwapChain.h"

#include <utility>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/Surface.h"
#include "dawn/native/Texture.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

TextureDescriptor GetSwapChainBaseTextureDescriptor(SwapChainBase* swapChain) {
    TextureDescriptor desc;
    desc.usage = swapChain->GetUsage();
    desc.dimension = wgpu::TextureDimension::e2D;
    desc.size = {swapChain->GetWidth(), swapChain->GetHeight(), 1};
    desc.format = swapChain->GetFormat();
    desc.viewFormatCount = swapChain->GetViewFormats().size();
    desc.viewFormats = swapChain->GetViewFormats().data();
    desc.mipLevelCount = 1;
    desc.sampleCount = 1;

    return desc;
}

SwapChainBase::SwapChainBase(DeviceBase* device,
                             Surface* surface,
                             const SurfaceConfiguration* config)
    : mDevice(device),
      mWidth(config->width),
      mHeight(config->height),
      mFormat(config->format),
      mUsage(config->usage),
      mPresentMode(config->presentMode),
      mAlphaMode(config->alphaMode),
      mSurface(surface) {
    for (uint32_t i = 0; i < config->viewFormatCount; ++i) {
        if (config->viewFormats[i] == config->format) {
            // Skip our own format, like texture creations does.
            continue;
        }
        mViewFormats.push_back(config->viewFormats[i]);
    }
}

FormatSet SwapChainBase::ComputeViewFormatSet() const {
    FormatSet viewFormatSet;
    for (wgpu::TextureFormat format : mViewFormats) {
        viewFormatSet[GetDevice()->GetValidInternalFormat(format)] = true;
    }
    return viewFormatSet;
}

SwapChainBase::~SwapChainBase() {
    if (mCurrentTextureInfo.texture != nullptr) {
        DAWN_ASSERT(mCurrentTextureInfo.texture->IsDestroyed());
    }

    DAWN_ASSERT(!mAttached);
}

void SwapChainBase::DetachFromSurface() {
    if (mAttached) {
        DetachFromSurfaceImpl();
        mSurface = nullptr;
        mAttached = false;
    }
}

void SwapChainBase::SetIsAttached() {
    mAttached = true;
}

ResultOrError<SurfaceTexture> SwapChainBase::GetCurrentTexture() {
    if (mCurrentTextureInfo.texture == nullptr) {
        DAWN_TRY_ASSIGN(mCurrentTextureInfo, GetCurrentTextureImpl());
    }

    SurfaceTexture surfaceTexture;
    surfaceTexture.texture = nullptr;
    surfaceTexture.status = mCurrentTextureInfo.status;

    // Handle cases where the backend swapchain goes bad and can't return a texture.
    if (mCurrentTextureInfo.texture == nullptr) {
        return surfaceTexture;
    }

    SetChildLabel(mCurrentTextureInfo.texture.Get());

    // Check that the return texture matches exactly what was given for this descriptor.
    DAWN_ASSERT(mCurrentTextureInfo.texture->GetFormat().format == mFormat);
    DAWN_ASSERT(IsSubset(mUsage, mCurrentTextureInfo.texture->GetUsage()));
    DAWN_ASSERT(mCurrentTextureInfo.texture->GetDimension() == wgpu::TextureDimension::e2D);
    DAWN_ASSERT(mCurrentTextureInfo.texture->GetWidth(Aspect::Color) == mWidth);
    DAWN_ASSERT(mCurrentTextureInfo.texture->GetHeight(Aspect::Color) == mHeight);
    DAWN_ASSERT(mCurrentTextureInfo.texture->GetNumMipLevels() == 1);
    DAWN_ASSERT(mCurrentTextureInfo.texture->GetArrayLayers() == 1);
    DAWN_ASSERT(mCurrentTextureInfo.texture->GetViewFormats() == ComputeViewFormatSet());

    // Calling GetCurrentTexture always returns a new reference.
    auto texture = mCurrentTextureInfo.texture;
    surfaceTexture.texture = ReturnToAPI(std::move(texture));
    return surfaceTexture;
}

MaybeError SwapChainBase::Present() {
    DAWN_TRY(ValidatePresent());
    DAWN_TRY(PresentImpl());

    DAWN_ASSERT(mCurrentTextureInfo.texture->IsDestroyed());
    mCurrentTextureInfo.texture = nullptr;
    return {};
}

DeviceBase* SwapChainBase::GetDevice() const {
    return mDevice.Get();
}
uint32_t SwapChainBase::GetWidth() const {
    return mWidth;
}

uint32_t SwapChainBase::GetHeight() const {
    return mHeight;
}

wgpu::TextureFormat SwapChainBase::GetFormat() const {
    return mFormat;
}

const std::vector<wgpu::TextureFormat>& SwapChainBase::GetViewFormats() const {
    return mViewFormats;
}

wgpu::TextureUsage SwapChainBase::GetUsage() const {
    return mUsage;
}

wgpu::PresentMode SwapChainBase::GetPresentMode() const {
    return mPresentMode;
}

wgpu::CompositeAlphaMode SwapChainBase::GetAlphaMode() const {
    return mAlphaMode;
}

Surface* SwapChainBase::GetSurface() const {
    return mSurface;
}

bool SwapChainBase::IsAttached() const {
    return mAttached;
}

wgpu::BackendType SwapChainBase::GetBackendType() const {
    return GetDevice()->GetPhysicalDevice()->GetBackendType();
}

MaybeError SwapChainBase::ValidatePresent() const {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_ASSERT(mAttached);

    DAWN_INVALID_IF(mCurrentTextureInfo.texture == nullptr,
                    "GetCurrentTexture was not called on %s this frame prior to calling Present.",
                    this->GetSurface());

    return {};
}

MaybeError SwapChainBase::ValidateGetCurrentTexture() const {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_ASSERT(mAttached);

    return {};
}

void SwapChainBase::SetChildLabel(ApiObjectBase* child) const {
    child->SetLabel(absl::StrFormat("of %s", this->GetSurface()));
}

}  // namespace dawn::native
