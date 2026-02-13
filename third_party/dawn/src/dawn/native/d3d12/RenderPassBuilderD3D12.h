// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D12_RENDERPASSBUILDERD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_RENDERPASSBUILDERD3D12_H_

#include <array>

#include "dawn/common/Constants.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_span.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/d3d12/d3d12_platform.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {
struct ResolveRect;
}  // namespace dawn::native

namespace dawn::native::d3d12 {

class TextureView;

// RenderPassBuilder stores parameters related to render pass load and store operations.
// When the D3D12 render pass API is available, the needed descriptors can be fetched
// directly from the RenderPassBuilder. When the D3D12 render pass API is not available, the
// descriptors are still fetched and any information necessary to emulate the load and store
// operations is extracted from the descriptors.
class RenderPassBuilder {
  public:
    explicit RenderPassBuilder(bool hasUAV);

    // Returns the highest color attachment index + 1. If there is no color attachment, returns
    // 0. Range: [0, kMaxColorAttachments + 1)
    ColorAttachmentIndex GetHighestColorAttachmentIndexPlusOne() const;

    // Returns descriptors that are fed directly to BeginRenderPass, or are used as parameter
    // storage if D3D12 render pass API is unavailable.
    ityp::span<ColorAttachmentIndex, const D3D12_RENDER_PASS_RENDER_TARGET_DESC>
    GetRenderPassRenderTargetDescriptors() const;
    const D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* GetRenderPassDepthStencilDescriptor() const;

    D3D12_RENDER_PASS_FLAGS GetRenderPassFlags() const;

    // Returns attachment RTVs to use with OMSetRenderTargets.
    const D3D12_CPU_DESCRIPTOR_HANDLE* GetRenderTargetViews() const;

    bool HasDepthOrStencil() const;

    // Functions that set the appropriate values in the render pass descriptors.
    void SetDepthAccess(wgpu::LoadOp loadOp,
                        wgpu::StoreOp storeOp,
                        float clearDepth,
                        DXGI_FORMAT format);
    void SetDepthNoAccess();
    void SetDepthStencilNoAccess();
    void SetRenderTargetBeginningAccess(ColorAttachmentIndex attachment,
                                        wgpu::LoadOp loadOp,
                                        dawn::native::Color clearColor,
                                        DXGI_FORMAT format);
    void SetRenderTargetEndingAccess(ColorAttachmentIndex attachment, wgpu::StoreOp storeOp);
    void SetRenderTargetEndingAccessResolve(ColorAttachmentIndex attachment,
                                            wgpu::StoreOp storeOp,
                                            TextureView* resolveSource,
                                            TextureView* resolveDestination,
                                            const ResolveRect& resolveRect);
    void SetStencilAccess(wgpu::LoadOp loadOp,
                          wgpu::StoreOp storeOp,
                          uint8_t clearStencil,
                          DXGI_FORMAT format);
    void SetStencilNoAccess();

    void SetRenderTargetView(ColorAttachmentIndex attachmentIndex,
                             D3D12_CPU_DESCRIPTOR_HANDLE baseDescriptor,
                             bool isNullRTV);
    void SetDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE baseDescriptor,
                             bool isDepthReadOnly,
                             bool isStencilReadOnly);

  private:
    ColorAttachmentIndex mHighestColorAttachmentIndexPlusOne{uint8_t(0)};
    bool mHasDepthOrStencil = false;
    D3D12_RENDER_PASS_FLAGS mRenderPassFlags = D3D12_RENDER_PASS_FLAG_NONE;
    D3D12_RENDER_PASS_DEPTH_STENCIL_DESC mRenderPassDepthStencilDesc;
    PerColorAttachment<D3D12_RENDER_PASS_RENDER_TARGET_DESC> mRenderPassRenderTargetDescriptors{};
    PerColorAttachment<D3D12_CPU_DESCRIPTOR_HANDLE> mRenderTargetViews{};
    PerColorAttachment<D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS>
        mSubresourceParams{};
};
}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_RENDERPASSBUILDERD3D12_H_
