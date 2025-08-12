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

#include "dawn/native/d3d11/CommandRecordingContextD3D11.h"

#include <string>
#include <utility>

#include "dawn/native/D3DBackend.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/BufferD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/Forward.h"
#include "dawn/native/d3d11/PhysicalDeviceD3D11.h"
#include "dawn/native/d3d11/PipelineLayoutD3D11.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d11 {

ScopedCommandRecordingContext::ScopedCommandRecordingContext(CommandRecordingContext::Guard&& guard)
    : CommandRecordingContext::Guard(std::move(guard)) {
    DAWN_ASSERT(Get()->mIsOpen);
}

Device* ScopedCommandRecordingContext::GetDevice() const {
    return Get()->mDevice.Get();
}

void ScopedCommandRecordingContext::UpdateSubresource1(ID3D11Resource* pDstResource,
                                                       UINT DstSubresource,
                                                       const D3D11_BOX* pDstBox,
                                                       const void* pSrcData,
                                                       UINT SrcRowPitch,
                                                       UINT SrcDepthPitch,
                                                       UINT CopyFlags) const {
    Get()->mD3D11DeviceContext3->UpdateSubresource1(pDstResource, DstSubresource, pDstBox, pSrcData,
                                                    SrcRowPitch, SrcDepthPitch, CopyFlags);
}

void ScopedCommandRecordingContext::CopyResource(ID3D11Resource* pDstResource,
                                                 ID3D11Resource* pSrcResource) const {
    Get()->mD3D11DeviceContext3->CopyResource(pDstResource, pSrcResource);
}

void ScopedCommandRecordingContext::CopySubresourceRegion(ID3D11Resource* pDstResource,
                                                          UINT DstSubresource,
                                                          UINT DstX,
                                                          UINT DstY,
                                                          UINT DstZ,
                                                          ID3D11Resource* pSrcResource,
                                                          UINT SrcSubresource,
                                                          const D3D11_BOX* pSrcBox) const {
    Get()->mD3D11DeviceContext3->CopySubresourceRegion(pDstResource, DstSubresource, DstX, DstY,
                                                       DstZ, pSrcResource, SrcSubresource, pSrcBox);
}

void ScopedCommandRecordingContext::ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView,
                                                          const FLOAT ColorRGBA[4]) const {
    Get()->mD3D11DeviceContext3->ClearRenderTargetView(pRenderTargetView, ColorRGBA);
}

void ScopedCommandRecordingContext::ClearDepthStencilView(ID3D11DepthStencilView* pDepthStencilView,
                                                          UINT ClearFlags,
                                                          FLOAT Depth,
                                                          UINT8 Stencil) const {
    Get()->mD3D11DeviceContext3->ClearDepthStencilView(pDepthStencilView, ClearFlags, Depth,
                                                       Stencil);
}

HRESULT ScopedCommandRecordingContext::Map(ID3D11Resource* pResource,
                                           UINT Subresource,
                                           D3D11_MAP MapType,
                                           UINT MapFlags,
                                           D3D11_MAPPED_SUBRESOURCE* pMappedResource) const {
    return Get()->mD3D11DeviceContext3->Map(pResource, Subresource, MapType, MapFlags,
                                            pMappedResource);
}

void ScopedCommandRecordingContext::Unmap(ID3D11Resource* pResource, UINT Subresource) const {
    Get()->mD3D11DeviceContext3->Unmap(pResource, Subresource);
}

HRESULT ScopedCommandRecordingContext::Signal(ID3D11Fence* pFence, UINT64 Value) const {
    DAWN_ASSERT(Get()->mD3D11DeviceContext4);
    return Get()->mD3D11DeviceContext4->Signal(pFence, Value);
}

HRESULT ScopedCommandRecordingContext::Wait(ID3D11Fence* pFence, UINT64 Value) const {
    DAWN_ASSERT(Get()->mD3D11DeviceContext4);
    return Get()->mD3D11DeviceContext4->Wait(pFence, Value);
}

HRESULT ScopedCommandRecordingContext::GetData(ID3D11Query* pQuery,
                                               void* pResult,
                                               UINT size,
                                               UINT flags) const {
    return Get()->mD3D11DeviceContext3->GetData(pQuery, pResult, size, flags);
}

void ScopedCommandRecordingContext::End(ID3D11Query* pQuery) const {
    Get()->mD3D11DeviceContext3->End(pQuery);
}

void ScopedCommandRecordingContext::Flush() const {
    return Get()->mD3D11DeviceContext3->Flush();
}

void ScopedCommandRecordingContext::Flush1(D3D11_CONTEXT_TYPE ContextType, HANDLE hEvent) const {
    return Get()->mD3D11DeviceContext3->Flush1(ContextType, hEvent);
}

void ScopedCommandRecordingContext::WriteUniformBufferRange(uint32_t offset,
                                                            const void* data,
                                                            size_t size) const {
    DAWN_ASSERT(offset < kMaxImmediateConstantsPerPipeline);
    DAWN_ASSERT(size <= sizeof(uint32_t) * (kMaxImmediateConstantsPerPipeline - offset));
    std::memcpy(&Get()->mUniformBufferData[offset], data, size);
    Get()->mUniformBufferDirty = true;
}

MaybeError ScopedCommandRecordingContext::FlushUniformBuffer() const {
    if (Get()->mUniformBufferDirty) {
        DAWN_TRY(Get()->mUniformBuffer->Write(this, 0, Get()->mUniformBufferData.data(),
                                              Get()->mUniformBufferData.size() * sizeof(uint32_t)));
        Get()->mUniformBufferDirty = false;
    }
    return {};
}

MaybeError ScopedCommandRecordingContext::AcquireKeyedMutex(Ref<d3d::KeyedMutex> keyedMutex) const {
    if (!Get()->mAcquiredKeyedMutexes.contains(keyedMutex)) {
        DAWN_TRY(keyedMutex->AcquireKeyedMutex());
        Get()->mAcquiredKeyedMutexes.emplace(std::move(keyedMutex));
    }
    return {};
}

void ScopedCommandRecordingContext::SetNeedsFence() const {
    Get()->mNeedsFence = true;
}

void ScopedCommandRecordingContext::AddBufferForSyncingWithCPU(GPUUsableBuffer* buffer) const {
    Get()->mBuffersToSyncWithCPU.push_back(buffer);
}

MaybeError ScopedCommandRecordingContext::FlushBuffersForSyncingWithCPU() const {
    for (auto buffer : Get()->mBuffersToSyncWithCPU) {
        DAWN_TRY(buffer->SyncGPUWritesToStaging(this));
    }
    Get()->mBuffersToSyncWithCPU.clear();
    return {};
}

ScopedSwapStateCommandRecordingContext::ScopedSwapStateCommandRecordingContext(
    CommandRecordingContextGuard&& guard)
    : ScopedCommandRecordingContext(std::move(guard)) {
    Get()->mD3D11DeviceContext3->SwapDeviceContextState(Get()->mD3D11DeviceContextState.Get(),
                                                        &mPreviousState);
}

ScopedSwapStateCommandRecordingContext::~ScopedSwapStateCommandRecordingContext() {
    Get()->mD3D11DeviceContext3->SwapDeviceContextState(mPreviousState.Get(), nullptr);
}

ID3D11Device* ScopedSwapStateCommandRecordingContext::GetD3D11Device() const {
    return Get()->mD3D11Device.Get();
}

ID3D11DeviceContext3* ScopedSwapStateCommandRecordingContext::GetD3D11DeviceContext3() const {
    return Get()->mD3D11DeviceContext3.Get();
}

ID3DUserDefinedAnnotation* ScopedSwapStateCommandRecordingContext::GetD3DUserDefinedAnnotation()
    const {
    return Get()->mD3DUserDefinedAnnotation.Get();
}

Buffer* ScopedSwapStateCommandRecordingContext::GetInternalUniformBuffer() const {
    return Get()->mUniformBuffer.Get();
}

MaybeError ScopedSwapStateCommandRecordingContext::SetInternalUniformBuffer(
    Ref<BufferBase> uniformBuffer) {
    Get()->mUniformBuffer = ToGPUUsableBuffer(std::move(uniformBuffer));

    // Always bind the uniform buffer to the reserved slot for all pipelines.
    // This buffer will be updated with the correct values before each draw or dispatch call.
    ID3D11Buffer* bufferPtr;
    DAWN_TRY_ASSIGN(bufferPtr, Get()->mUniformBuffer->GetD3D11ConstantBuffer(nullptr));
    Get()->mD3D11DeviceContext3->VSSetConstantBuffers(PipelineLayout::kReservedConstantBufferSlot,
                                                      1, &bufferPtr);
    Get()->mD3D11DeviceContext3->CSSetConstantBuffers(PipelineLayout::kReservedConstantBufferSlot,
                                                      1, &bufferPtr);
    Get()->mD3D11DeviceContext3->PSSetConstantBuffers(PipelineLayout::kReservedConstantBufferSlot,
                                                      1, &bufferPtr);

    return {};
}

MaybeError CommandRecordingContext::Initialize(Device* device) {
    DAWN_ASSERT(!mIsOpen);
    DAWN_ASSERT(device);
    mDevice = device;

    ID3D11Device3* d3d11Device = device->GetD3D11Device3();

    {
        const D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

        HRESULT hr = S_OK;
        // Try all possible ID3D11Device* interfaces from highest to lowest.
        for (auto devUUID : {__uuidof(ID3D11Device5), __uuidof(ID3D11Device3)}) {
            hr = d3d11Device->CreateDeviceContextState(
                /*Flags=*/0, featureLevels, std::size(featureLevels), D3D11_SDK_VERSION, devUUID,
                nullptr, &mD3D11DeviceContextState);
            if (SUCCEEDED(hr)) {
                break;
            }
        }
        DAWN_TRY(CheckHRESULT(hr, "D3D11: create device context state"));
    }

    ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
    device->GetD3D11Device()->GetImmediateContext(&d3d11DeviceContext);

    ComPtr<ID3D11DeviceContext3> d3d11DeviceContext3;
    DAWN_TRY(CheckHRESULT(d3d11DeviceContext.As(&d3d11DeviceContext3),
                          "D3D11 querying immediate context for ID3D11DeviceContext3 interface"));

    ComPtr<ID3D11DeviceContext4> d3d11DeviceContext4;
    if (!device->IsToggleEnabled(Toggle::D3D11DisableFence)) {
        // This interface only adds methods related to fences. We only need it for Signal()/Wait().
        DAWN_TRY(
            CheckHRESULT(d3d11DeviceContext.As(&d3d11DeviceContext4),
                         "D3D11 querying immediate context for ID3D11DeviceContext4 interface"));
    }

    DAWN_TRY(
        CheckHRESULT(d3d11DeviceContext3.As(&mD3DUserDefinedAnnotation),
                     "D3D11 querying immediate context for ID3DUserDefinedAnnotation interface"));

    if (device->HasFeature(Feature::D3D11MultithreadProtected)) {
        DAWN_TRY(CheckHRESULT(d3d11DeviceContext.As(&mD3D11Multithread),
                              "D3D11 querying immediate context for ID3D11Multithread interface"));
        mD3D11Multithread->SetMultithreadProtected(TRUE);
    }

    mD3D11Device = d3d11Device;
    mD3D11DeviceContext3 = std::move(d3d11DeviceContext3);
    mD3D11DeviceContext4 = std::move(d3d11DeviceContext4);
    mIsOpen = true;
    return {};
}

bool CommandRecordingContext::IsValid() const {
    return mIsOpen;
}

void CommandRecordingContext::Destroy() {
    // mDevice could be null due to failure of initialization.
    if (!mDevice) {
        return;
    }

    DAWN_ASSERT(mDevice->IsLockedByCurrentThreadIfNeeded());
    mIsOpen = false;
    mUniformBuffer = nullptr;
    mDevice = nullptr;

    if (mD3D11DeviceContext3) {
        ID3D11Buffer* nullBuffer = nullptr;
        mD3D11DeviceContext3->VSSetConstantBuffers(PipelineLayout::kReservedConstantBufferSlot, 1,
                                                   &nullBuffer);
        mD3D11DeviceContext3->CSSetConstantBuffers(PipelineLayout::kReservedConstantBufferSlot, 1,
                                                   &nullBuffer);
    }

    ReleaseKeyedMutexes();

    mD3D11DeviceContextState = nullptr;
    mD3D11DeviceContext3 = nullptr;
    mD3D11DeviceContext4 = nullptr;
    mD3D11Device = nullptr;
}

// static
ResultOrError<Ref<BufferBase>> CommandRecordingContext::CreateInternalUniformBuffer(
    DeviceBase* device) {
    // Create a uniform buffer for user and internal ImmediateConstants.
    BufferDescriptor descriptor;
    descriptor.size = sizeof(uint32_t) * kMaxImmediateConstantsPerPipeline;
    descriptor.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    descriptor.mappedAtCreation = false;
    descriptor.label = "ImmediateConstantsInternalBuffer";

    Ref<BufferBase> uniformBuffer;
    // Lock the device to protect the clearing of the built-in uniform buffer.
    auto deviceGuard = device->GetGuard();
    return device->CreateBuffer(&descriptor);
}

void CommandRecordingContext::ReleaseKeyedMutexes() {
    for (auto& keyedMutex : mAcquiredKeyedMutexes) {
        keyedMutex->ReleaseKeyedMutex();
    }
    mAcquiredKeyedMutexes.clear();
}

bool CommandRecordingContext::AcquireNeedsFence() {
    bool needsFence = mNeedsFence;
    mNeedsFence = false;
    return needsFence;
}

}  // namespace dawn::native::d3d11
