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

#ifndef SRC_DAWN_NATIVE_D3D11_COMMANDRECORDINGCONTEXT_D3D11_H_
#define SRC_DAWN_NATIVE_D3D11_COMMANDRECORDINGCONTEXT_D3D11_H_

#include "absl/container/flat_hash_set.h"
#include "absl/container/inlined_vector.h"
#include "dawn/common/Constants.h"
#include "dawn/common/MutexProtected.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/common/Ref.h"
#include "dawn/native/Error.h"
#include "dawn/native/d3d/KeyedMutex.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {

class CommandAllocatorManager;
class Buffer;
class GPUUsableBuffer;
class Device;

class CommandRecordingContext;

template <typename Ctx, typename Traits>
class CommandRecordingContextGuard;

// CommandRecordingContext::Guard is an implementation of Guard that uses two locks.
// It uses its own lock to synchronize access within Dawn.
// It also acquires a D3D11 lock if multithread protected mode is enabled.
// When enabled, it synchronizes access to the D3D11 context external to Dawn.
template <typename Ctx, typename Traits>
class CommandRecordingContextGuard : public ::dawn::detail::Guard<Ctx, Traits> {
  public:
    using Base = ::dawn::detail::Guard<Ctx, Traits>;

    CommandRecordingContextGuard(CommandRecordingContextGuard&& rhs) = default;
    CommandRecordingContextGuard(Ctx* ctx,
                                 typename Traits::MutexType& mutex,
                                 Defer* defer = nullptr)
        : Base(ctx, mutex, defer) {
        if (this->Get() && this->Get()->mD3D11Multithread) {
            this->Get()->mD3D11Multithread->Enter();
        }
    }
    ~CommandRecordingContextGuard() {
        if (this->Get() && this->Get()->mD3D11Multithread) {
            this->Get()->mD3D11Multithread->Leave();
        }
    }

    CommandRecordingContextGuard(const CommandRecordingContextGuard& other) = delete;
    CommandRecordingContextGuard& operator=(const CommandRecordingContextGuard& other) = delete;
    CommandRecordingContextGuard& operator=(CommandRecordingContextGuard&& other) = delete;
};

class CommandRecordingContext {
  public:
    using Guard =
        CommandRecordingContextGuard<CommandRecordingContext,
                                     ::dawn::detail::MutexProtectedTraits<CommandRecordingContext>>;

    MaybeError Initialize(Device* device);
    void Destroy();

    bool IsValid() const;

    static ResultOrError<Ref<BufferBase>> CreateInternalUniformBuffer(DeviceBase* device);

    void ReleaseKeyedMutexes();

    bool AcquireNeedsFence();

  private:
    template <typename Ctx, typename Traits>
    friend class CommandRecordingContextGuard;

    friend class ScopedCommandRecordingContext;
    friend class ScopedSwapStateCommandRecordingContext;

    bool mIsOpen = false;
    ComPtr<ID3D11Device> mD3D11Device;
    ComPtr<ID3DDeviceContextState> mD3D11DeviceContextState;
    ComPtr<ID3D11DeviceContext3> mD3D11DeviceContext3;
    ComPtr<ID3D11DeviceContext4> mD3D11DeviceContext4;
    ComPtr<ID3D11Multithread> mD3D11Multithread;
    ComPtr<ID3DUserDefinedAnnotation> mD3DUserDefinedAnnotation;

    // The uniform buffer for built-in variables.
    Ref<GPUUsableBuffer> mUniformBuffer;
    std::array<uint32_t, kMaxImmediateConstantsPerPipeline> mUniformBufferData{};
    bool mUniformBufferDirty = true;

    absl::flat_hash_set<Ref<d3d::KeyedMutex>> mAcquiredKeyedMutexes;

    bool mNeedsFence = false;

    // List of buffers to sync their CPU accessible storages.
    // Use inlined vector to avoid heap allocation when the vector is empty.
    absl::InlinedVector<GPUUsableBuffer*, 1> mBuffersToSyncWithCPU;

    Ref<Device> mDevice;
};

// For using ID3D11DeviceContext methods which don't change device context state.
class ScopedCommandRecordingContext : public CommandRecordingContext::Guard {
  public:
    explicit ScopedCommandRecordingContext(CommandRecordingContext::Guard&& guard);

    Device* GetDevice() const;

    // Wrapper method which don't depend on context state.
    void UpdateSubresource1(ID3D11Resource* pDstResource,
                            UINT DstSubresource,
                            const D3D11_BOX* pDstBox,
                            const void* pSrcData,
                            UINT SrcRowPitch,
                            UINT SrcDepthPitch,
                            UINT CopyFlags) const;
    void CopyResource(ID3D11Resource* pDstResource, ID3D11Resource* pSrcResource) const;
    void CopySubresourceRegion(ID3D11Resource* pDstResource,
                               UINT DstSubresource,
                               UINT DstX,
                               UINT DstY,
                               UINT DstZ,
                               ID3D11Resource* pSrcResource,
                               UINT SrcSubresource,
                               const D3D11_BOX* pSrcBox) const;
    void ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView,
                               const FLOAT ColorRGBA[4]) const;
    void ClearDepthStencilView(ID3D11DepthStencilView* pDepthStencilView,
                               UINT ClearFlags,
                               FLOAT Depth,
                               UINT8 Stencil) const;
    HRESULT Map(ID3D11Resource* pResource,
                UINT Subresource,
                D3D11_MAP MapType,
                UINT MapFlags,
                D3D11_MAPPED_SUBRESOURCE* pMappedResource) const;
    void Unmap(ID3D11Resource* pResource, UINT Subresource) const;
    HRESULT Signal(ID3D11Fence* pFence, UINT64 Value) const;
    HRESULT Wait(ID3D11Fence* pFence, UINT64 Value) const;
    HRESULT GetData(ID3D11Query* pQuery, void* pResult, UINT size, UINT flags) const;
    void End(ID3D11Query* pQuery) const;
    void Flush() const;
    void Flush1(D3D11_CONTEXT_TYPE ContextType, HANDLE hEvent) const;

    // Write immediate data to the uniform buffer.
    void WriteUniformBufferRange(uint32_t offset, const void* data, size_t size) const;
    MaybeError FlushUniformBuffer() const;

    MaybeError AcquireKeyedMutex(Ref<d3d::KeyedMutex> keyedMutex) const;

    void SetNeedsFence() const;

    // Add a buffer to a pending list for syncing CPU storages. The list is typically processed at
    // the end of a command buffer when it is about to be submitted.
    void AddBufferForSyncingWithCPU(GPUUsableBuffer* buffer) const;
    MaybeError FlushBuffersForSyncingWithCPU() const;
};

// For using ID3D11DeviceContext directly. It swaps and resets ID3DDeviceContextState of
// ID3D11DeviceContext for a scope. It is needed for sharing ID3D11Device between dawn and ANGLE.
class ScopedSwapStateCommandRecordingContext : public ScopedCommandRecordingContext {
  public:
    explicit ScopedSwapStateCommandRecordingContext(CommandRecordingContext::Guard&& guard);
    ~ScopedSwapStateCommandRecordingContext();

    ID3D11Device* GetD3D11Device() const;
    ID3D11DeviceContext3* GetD3D11DeviceContext3() const;
    ID3DUserDefinedAnnotation* GetD3DUserDefinedAnnotation() const;
    Buffer* GetInternalUniformBuffer() const;
    MaybeError SetInternalUniformBuffer(Ref<BufferBase> uniformBuffer);

  private:
    ComPtr<ID3DDeviceContextState> mPreviousState;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_COMMANDRECORDINGCONTEXT_D3D11_H_
