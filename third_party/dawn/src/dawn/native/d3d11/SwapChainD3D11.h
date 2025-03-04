// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D11_SWAPCHAIND3D11_H_
#define SRC_DAWN_NATIVE_D3D11_SWAPCHAIND3D11_H_

#include <vector>

#include "dawn/native/d3d/SwapChainD3D.h"

#include "dawn/native/IntegerTypes.h"

namespace dawn::native::d3d11 {

class Device;
class Texture;

class SwapChain final : public d3d::SwapChain {
  public:
    static ResultOrError<Ref<SwapChain>> Create(Device* device,
                                                Surface* surface,
                                                SwapChainBase* previousSwapChain,
                                                const SurfaceConfiguration* config);

  private:
    using Base = d3d::SwapChain;
    using Base::Base;
    ~SwapChain() override;

    // SwapChainBase implementation
    MaybeError PresentImpl() override;
    ResultOrError<SwapChainTextureInfo> GetCurrentTextureImpl() override;
    void DetachFromSurfaceImpl() override;

    // d3d::SwapChain implementation
    IUnknown* GetD3DDeviceForCreatingSwapChain() override;
    void ReuseBuffers(SwapChainBase* previousSwapChain) override;
    // Does the swapchain initialization step of gathering the buffers.
    MaybeError CollectSwapChainBuffers() override;
    // Calls DetachFromSurface but also synchronously waits until all references to the
    // swapchain and buffers are removed, as that's a constraint for some DXGI operations.
    MaybeError DetachAndWaitForDeallocation() override;

    ComPtr<ID3D11Texture2D> mBuffer;
    Ref<Texture> mApiTexture;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D12_SWAPCHAIND3D11_H_
