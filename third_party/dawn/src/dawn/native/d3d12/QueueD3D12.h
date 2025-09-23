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

#ifndef SRC_DAWN_NATIVE_D3D12_QUEUED3D12_H_
#define SRC_DAWN_NATIVE_D3D12_QUEUED3D12_H_

#include <array>
#include <memory>

#include "dawn/common/MutexProtected.h"
#include "dawn/common/SerialQueue.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/SystemEvent.h"
#include "dawn/native/d3d/QueueD3D.h"
#include "dawn/native/d3d12/CommandRecordingContext.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class Device;
class SharedFence;

class Queue final : public d3d::Queue {
  public:
    static ResultOrError<Ref<Queue>> Create(Device* device, const QueueDescriptor* descriptor);

    MaybeError NextSerial();
    MaybeError WaitForSerial(ExecutionSerial serial);
    CommandRecordingContext* GetPendingCommandContext(SubmitMode submitMode = SubmitMode::Normal);
    ID3D12CommandQueue* GetCommandQueue() const;
    ResultOrError<Ref<d3d::SharedFence>> GetOrCreateSharedFence() override;
    ID3D12SharingContract* GetSharingContract() const;

  private:
    using d3d::Queue::Queue;
    ~Queue() override;

    MaybeError Initialize();

    void DestroyImpl() override;
    MaybeError SubmitPendingCommandsImpl() override;
    MaybeError SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) override;
    bool HasPendingCommands() const override;
    ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() override;
    void ForceEventualFlushOfCommands() override;
    MaybeError WaitForIdleForDestruction() override;

    void SetEventOnCompletion(ExecutionSerial serial, HANDLE event) override;

    MaybeError OpenPendingCommands();
    void RecycleLastCommandListAfter(ExecutionSerial serial);
    MaybeError RecycleUnusedCommandLists();

    // Dawn API
    void SetLabelImpl() override;

    ComPtr<ID3D12Fence> mFence;
    Ref<SharedFence> mSharedFence;

    CommandRecordingContext mPendingCommands;
    ComPtr<ID3D12CommandQueue> mCommandQueue;
    ComPtr<ID3D12SharingContract> mD3d12SharingContract;

    // Use a maximum number of command allocators to try to mitigate the memory cost used in total
    // by allocators. Allocators are created lazily and then recycled when their commands are done
    // executing.
    static constexpr uint32_t kMaxCommandAllocators = 32;
    static constexpr uint32_t kNoCommandAllocator = kMaxCommandAllocators;
    uint32_t mAllocatorCount = 0;
    struct AllocatorAndList {
        ComPtr<ID3D12CommandAllocator> allocator;
        ComPtr<ID3D12GraphicsCommandList> list;
    };

    std::array<AllocatorAndList, kMaxCommandAllocators> mCommandAllocators;
    ityp::bitset<uint32_t, kMaxCommandAllocators> mFreeAllocators;
    uint32_t mLastAllocatorUsed = kNoCommandAllocator;
    SerialQueue<ExecutionSerial, uint32_t> mInFlightCommandAllocators;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_QUEUED3D12_H_
