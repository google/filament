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

#include "dawn/native/d3d12/RenderPassBuilderD3D12.h"

#include <algorithm>

#include "dawn/native/Format.h"
#include "dawn/native/d3d12/CommandBufferD3D12.h"
#include "dawn/native/d3d12/Forward.h"
#include "dawn/native/d3d12/TextureD3D12.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native::d3d12 {

namespace {
D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE D3D12BeginningAccessType(wgpu::LoadOp loadOp) {
    switch (loadOp) {
        case wgpu::LoadOp::Clear:
            return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
        case wgpu::LoadOp::Load:
        case wgpu::LoadOp::ExpandResolveTexture:
            // TODO(402810062): consider not loading the MSAA texture on tile based GPUs.
            return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;

        case wgpu::LoadOp::Undefined:
            DAWN_UNREACHABLE();
            break;
    }
}

D3D12_RENDER_PASS_ENDING_ACCESS_TYPE D3D12EndingAccessType(wgpu::StoreOp storeOp) {
    switch (storeOp) {
        case wgpu::StoreOp::Discard:
            return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
        case wgpu::StoreOp::Store:
            return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
        case wgpu::StoreOp::Undefined:
            DAWN_UNREACHABLE();
            break;
    }
}

D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_PARAMETERS D3D12EndingAccessResolveParameters(
    wgpu::StoreOp storeOp,
    TextureView* resolveSource,
    TextureView* resolveDestination) {
    D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_PARAMETERS resolveParameters;

    resolveParameters.Format = resolveDestination->GetD3D12Format();
    resolveParameters.pSrcResource = ToBackend(resolveSource->GetTexture())->GetD3D12Resource();
    resolveParameters.pDstResource =
        ToBackend(resolveDestination->GetTexture())->GetD3D12Resource();

    // Clear or preserve the resolve source.
    if (storeOp == wgpu::StoreOp::Discard) {
        resolveParameters.PreserveResolveSource = false;
    } else if (storeOp == wgpu::StoreOp::Store) {
        resolveParameters.PreserveResolveSource = true;
    }

    // RESOLVE_MODE_AVERAGE is only valid for non-integer formats.
    DAWN_ASSERT(resolveDestination->GetFormat().GetAspectInfo(Aspect::Color).baseType ==
                TextureComponentType::Float);
    resolveParameters.ResolveMode = D3D12_RESOLVE_MODE_AVERAGE;

    resolveParameters.SubresourceCount = 1;

    return resolveParameters;
}

D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS
D3D12EndingAccessResolveSubresourceParameters(TextureView* resolveDestination,
                                              const ResolveRect& resolveRect) {
    D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS subresourceParameters;
    Texture* resolveDestinationTexture = ToBackend(resolveDestination->GetTexture());
    DAWN_ASSERT(resolveDestinationTexture->GetFormat().aspects == Aspect::Color);

    subresourceParameters.SrcSubresource = 0;
    subresourceParameters.DstSubresource = resolveDestinationTexture->GetSubresourceIndex(
        resolveDestination->GetBaseMipLevel(), resolveDestination->GetBaseArrayLayer(),
        Aspect::Color);
    subresourceParameters.DstX = resolveRect.resolveOffsetX;
    subresourceParameters.DstY = resolveRect.resolveOffsetY;
    subresourceParameters.SrcRect = {
        static_cast<int32_t>(resolveRect.colorOffsetX),
        static_cast<int32_t>(resolveRect.colorOffsetY),
        static_cast<int32_t>(resolveRect.colorOffsetX + resolveRect.updateWidth),
        static_cast<int32_t>(resolveRect.colorOffsetY + resolveRect.updateHeight)};

    return subresourceParameters;
}
}  // anonymous namespace

RenderPassBuilder::RenderPassBuilder(bool hasUAV) {
    if (hasUAV) {
        mRenderPassFlags = D3D12_RENDER_PASS_FLAG_ALLOW_UAV_WRITES;
    }
}

void RenderPassBuilder::SetRenderTargetView(ColorAttachmentIndex attachmentIndex,
                                            D3D12_CPU_DESCRIPTOR_HANDLE baseDescriptor,
                                            bool isNullRTV) {
    mRenderTargetViews[attachmentIndex] = baseDescriptor;
    mRenderPassRenderTargetDescriptors[attachmentIndex].cpuDescriptor = baseDescriptor;
    if (!isNullRTV) {
        mHighestColorAttachmentIndexPlusOne = std::max(
            mHighestColorAttachmentIndexPlusOne,
            ColorAttachmentIndex{static_cast<uint8_t>(static_cast<uint8_t>(attachmentIndex) + 1u)});
    } else {
        // Null views must be marked as preserved so that they keep using attachment slots.
        // Otherwise the holes in the attachments would get compacted by D3D12.
        // See the chapter "surfaces-that-beginrenderpass-binds-for-raster" in
        // https://microsoft.github.io/DirectX-Specs/d3d/RenderPasses.html for more information.
        mRenderPassRenderTargetDescriptors[attachmentIndex].BeginningAccess.Type =
            D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
        mRenderPassRenderTargetDescriptors[attachmentIndex].EndingAccess.Type =
            D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
    }
}

void RenderPassBuilder::SetDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE baseDescriptor,
                                            bool isDepthReadOnly,
                                            bool isStencilReadOnly) {
    mRenderPassDepthStencilDesc.cpuDescriptor = baseDescriptor;
#if D3D12_SDK_VERSION >= 612
    if (isDepthReadOnly) {
        mRenderPassFlags |= D3D12_RENDER_PASS_FLAG_BIND_READ_ONLY_DEPTH;
    }
    if (isStencilReadOnly) {
        mRenderPassFlags |= D3D12_RENDER_PASS_FLAG_BIND_READ_ONLY_STENCIL;
    }
#endif
}

ColorAttachmentIndex RenderPassBuilder::GetHighestColorAttachmentIndexPlusOne() const {
    return mHighestColorAttachmentIndexPlusOne;
}

bool RenderPassBuilder::HasDepthOrStencil() const {
    return mHasDepthOrStencil;
}

ityp::span<ColorAttachmentIndex, const D3D12_RENDER_PASS_RENDER_TARGET_DESC>
RenderPassBuilder::GetRenderPassRenderTargetDescriptors() const {
    return {mRenderPassRenderTargetDescriptors.data(), mHighestColorAttachmentIndexPlusOne};
}

const D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* RenderPassBuilder::GetRenderPassDepthStencilDescriptor()
    const {
    return &mRenderPassDepthStencilDesc;
}

D3D12_RENDER_PASS_FLAGS RenderPassBuilder::GetRenderPassFlags() const {
    return mRenderPassFlags;
}

const D3D12_CPU_DESCRIPTOR_HANDLE* RenderPassBuilder::GetRenderTargetViews() const {
    return mRenderTargetViews.data();
}

void RenderPassBuilder::SetRenderTargetBeginningAccess(ColorAttachmentIndex attachment,
                                                       wgpu::LoadOp loadOp,
                                                       dawn::native::Color clearColor,
                                                       DXGI_FORMAT format) {
    mRenderPassRenderTargetDescriptors[attachment].BeginningAccess.Type =
        D3D12BeginningAccessType(loadOp);
    if (loadOp == wgpu::LoadOp::Clear) {
        mRenderPassRenderTargetDescriptors[attachment].BeginningAccess.Clear.ClearValue.Color[0] =
            static_cast<float>(clearColor.r);
        mRenderPassRenderTargetDescriptors[attachment].BeginningAccess.Clear.ClearValue.Color[1] =
            static_cast<float>(clearColor.g);
        mRenderPassRenderTargetDescriptors[attachment].BeginningAccess.Clear.ClearValue.Color[2] =
            static_cast<float>(clearColor.b);
        mRenderPassRenderTargetDescriptors[attachment].BeginningAccess.Clear.ClearValue.Color[3] =
            static_cast<float>(clearColor.a);
        mRenderPassRenderTargetDescriptors[attachment].BeginningAccess.Clear.ClearValue.Format =
            format;
    }
}

void RenderPassBuilder::SetRenderTargetEndingAccess(ColorAttachmentIndex attachment,
                                                    wgpu::StoreOp storeOp) {
    mRenderPassRenderTargetDescriptors[attachment].EndingAccess.Type =
        D3D12EndingAccessType(storeOp);
}

void RenderPassBuilder::SetRenderTargetEndingAccessResolve(ColorAttachmentIndex attachment,
                                                           wgpu::StoreOp storeOp,
                                                           TextureView* resolveSource,
                                                           TextureView* resolveDestination,
                                                           const ResolveRect& resolveRect) {
    mRenderPassRenderTargetDescriptors[attachment].EndingAccess.Type =
        D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE;
    mRenderPassRenderTargetDescriptors[attachment].EndingAccess.Resolve =
        D3D12EndingAccessResolveParameters(storeOp, resolveSource, resolveDestination);

    mSubresourceParams[attachment] =
        D3D12EndingAccessResolveSubresourceParameters(resolveDestination, resolveRect);

    mRenderPassRenderTargetDescriptors[attachment].EndingAccess.Resolve.pSubresourceParameters =
        &mSubresourceParams[attachment];
}

void RenderPassBuilder::SetDepthAccess(wgpu::LoadOp loadOp,
                                       wgpu::StoreOp storeOp,
                                       float clearDepth,
                                       DXGI_FORMAT format) {
    mHasDepthOrStencil = true;
    mRenderPassDepthStencilDesc.DepthBeginningAccess.Type = D3D12BeginningAccessType(loadOp);
    if (loadOp == wgpu::LoadOp::Clear) {
        mRenderPassDepthStencilDesc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth =
            clearDepth;
        mRenderPassDepthStencilDesc.DepthBeginningAccess.Clear.ClearValue.Format = format;
    }
    mRenderPassDepthStencilDesc.DepthEndingAccess.Type = D3D12EndingAccessType(storeOp);
}

void RenderPassBuilder::SetStencilAccess(wgpu::LoadOp loadOp,
                                         wgpu::StoreOp storeOp,
                                         uint8_t clearStencil,
                                         DXGI_FORMAT format) {
    mHasDepthOrStencil = true;
    mRenderPassDepthStencilDesc.StencilBeginningAccess.Type = D3D12BeginningAccessType(loadOp);
    if (loadOp == wgpu::LoadOp::Clear) {
        mRenderPassDepthStencilDesc.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil =
            clearStencil;
        mRenderPassDepthStencilDesc.StencilBeginningAccess.Clear.ClearValue.Format = format;
    }
    mRenderPassDepthStencilDesc.StencilEndingAccess.Type = D3D12EndingAccessType(storeOp);
}

void RenderPassBuilder::SetDepthNoAccess() {
    mRenderPassDepthStencilDesc.DepthBeginningAccess.Type =
        D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
    mRenderPassDepthStencilDesc.DepthEndingAccess.Type =
        D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
}

void RenderPassBuilder::SetDepthStencilNoAccess() {
    SetDepthNoAccess();
    SetStencilNoAccess();
}

void RenderPassBuilder::SetStencilNoAccess() {
    mRenderPassDepthStencilDesc.StencilBeginningAccess.Type =
        D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
    mRenderPassDepthStencilDesc.StencilEndingAccess.Type =
        D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
}

}  // namespace dawn::native::d3d12
