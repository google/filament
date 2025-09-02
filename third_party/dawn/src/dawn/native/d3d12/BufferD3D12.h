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

#ifndef SRC_DAWN_NATIVE_D3D12_BUFFERD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_BUFFERD3D12_H_

#include <limits>
#include <memory>
#include <vector>

#include "dawn/native/Buffer.h"
#include "partition_alloc/pointers/raw_ptr.h"

#include "dawn/native/d3d12/ResourceHeapAllocationD3D12.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class CommandRecordingContext;
class Device;
class SharedBufferMemory;

class Buffer final : public BufferBase {
  public:
    static ResultOrError<Ref<Buffer>> Create(Device* device,
                                             const UnpackedPtr<BufferDescriptor>& descriptor);
    static ResultOrError<Ref<Buffer>> CreateFromSharedBufferMemory(
        SharedBufferMemory* memory,
        const UnpackedPtr<BufferDescriptor>& descriptor);

    ID3D12Resource* GetD3D12Resource() const;
    D3D12_GPU_VIRTUAL_ADDRESS GetVA() const;

    bool TrackUsageAndGetResourceBarrier(CommandRecordingContext* commandContext,
                                         D3D12_RESOURCE_BARRIER* barrier,
                                         wgpu::BufferUsage newUsage);
    void TrackUsageAndTransitionNow(CommandRecordingContext* commandContext,
                                    wgpu::BufferUsage newUsage);

    bool CheckAllocationMethodForTesting(AllocationMethod allocationMethod) const;
    bool CheckIsResidentForTesting() const;

    MaybeError EnsureDataInitialized(CommandRecordingContext* commandContext);
    ResultOrError<bool> EnsureDataInitializedAsDestination(CommandRecordingContext* commandContext,
                                                           uint64_t offset,
                                                           uint64_t size);
    MaybeError EnsureDataInitializedAsDestination(CommandRecordingContext* commandContext,
                                                  const CopyTextureToBufferCmd* copy);

    MaybeError SynchronizeBufferBeforeMapping();
    MaybeError SynchronizeBufferBeforeUseOnGPU();

    // Dawn API
    void SetLabelImpl() override;

  private:
    Buffer(Device* device, const UnpackedPtr<BufferDescriptor>& descriptor);
    ~Buffer() override;

    MaybeError Initialize(bool mappedAtCreation);
    MaybeError InitializeHostMapped(const BufferHostMappedPointer* hostMappedDesc);
    MaybeError InitializeAsExternalBuffer(ComPtr<ID3D12Resource> d3dBuffer,
                                          const UnpackedPtr<BufferDescriptor>& descriptor);
    MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) override;
    void UnmapImpl() override;
    void DestroyImpl() override;
    bool IsCPUWritableAtCreation() const override;
    MaybeError MapAtCreationImpl() override;
    void* GetMappedPointerImpl() override;

    MaybeError MapInternal(bool isWrite, size_t start, size_t end, const char* contextInfo);

    MaybeError InitializeToZero(CommandRecordingContext* commandContext);
    MaybeError ClearBuffer(CommandRecordingContext* commandContext,
                           uint8_t clearValue,
                           uint64_t offset = 0,
                           uint64_t size = 0);

    ResourceHeapAllocation mResourceAllocation;
    bool mFixedResourceState = false;
    D3D12_RESOURCE_STATES mLastState = D3D12_RESOURCE_STATE_COMMON;
    ExecutionSerial mLastUsedSerial = std::numeric_limits<ExecutionSerial>::max();

    D3D12_RANGE mWrittenMappedRange = {0, 0};
    void* mMappedData = nullptr;

    std::unique_ptr<Heap> mHostMappedHeap;
    wgpu::Callback mHostMappedDisposeCallback = nullptr;
    raw_ptr<void, DisableDanglingPtrDetection> mHostMappedDisposeUserdata = nullptr;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_BUFFERD3D12_H_
