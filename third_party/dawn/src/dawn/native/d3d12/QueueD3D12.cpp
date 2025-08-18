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

#include "dawn/native/d3d12/QueueD3D12.h"

#include <limits>
#include <utility>

#include "dawn/common/Math.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/CommandBufferD3D12.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/SharedFenceD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d12 {

// static
ResultOrError<Ref<Queue>> Queue::Create(Device* device, const QueueDescriptor* descriptor) {
    Ref<Queue> queue = AcquireRef(new Queue(device, descriptor));
    DAWN_TRY(queue->Initialize());
    return queue;
}

Queue::~Queue() {}

MaybeError Queue::Initialize() {
    mFreeAllocators.set();

    SetLabelImpl();

    ID3D12Device* d3d12Device = ToBackend(GetDevice())->GetD3D12Device();

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    DAWN_TRY(CheckHRESULT(d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)),
                          "D3D12 create command queue"));

    // If PIX is not attached, the QueryInterface fails. Hence, no need to check the return
    // value.
    mCommandQueue.As(&mD3d12SharingContract);

    DAWN_TRY(CheckHRESULT(d3d12Device->CreateFence(uint64_t(kBeginningOfGPUTime),
                                                   D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&mFence)),
                          "D3D12 create fence"));

    DAWN_TRY_ASSIGN(mSharedFence, SharedFence::Create(ToBackend(GetDevice()),
                                                      "Internal shared DXGI fence", mFence));

    return OpenPendingCommands();
}

void Queue::DestroyImpl() {
    // Immediately forget about all pending commands for the case where device is lost on its
    // own and WaitForIdleForDestruction isn't called.
    mPendingCommands.Release();
    mCommandQueue.Reset();

    // Release the shared fence here to prevent a ref-cycle with the device, but do not destroy the
    // underlying native fence so that we can return a SharedFence on EndAccess after destruction.
    mSharedFence = nullptr;
}

ResultOrError<Ref<d3d::SharedFence>> Queue::GetOrCreateSharedFence() {
    if (mSharedFence == nullptr) {
        DAWN_ASSERT(!IsAlive());
        return SharedFence::Create(ToBackend(GetDevice()), "Internal shared DXGI fence", mFence);
    }
    return mSharedFence;
}

ID3D12CommandQueue* Queue::GetCommandQueue() const {
    return mCommandQueue.Get();
}

ID3D12SharingContract* Queue::GetSharingContract() const {
    return mD3d12SharingContract.Get();
}

MaybeError Queue::SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) {
    CommandRecordingContext* commandContext = GetPendingCommandContext();
    ExecutionSerial pendingSerial = GetPendingCommandSerial();

    TRACE_EVENT_BEGIN1(GetDevice()->GetPlatform(), Recording, "CommandBufferD3D12::RecordCommands",
                       "serial", uint64_t(pendingSerial));
    for (uint32_t i = 0; i < commandCount; ++i) {
        DAWN_TRY(ToBackend(commands[i])->RecordCommands(commandContext));
    }
    TRACE_EVENT_END1(GetDevice()->GetPlatform(), Recording, "CommandBufferD3D12::RecordCommands",
                     "serial", uint64_t(pendingSerial));

    return SubmitPendingCommandsImpl();
}

MaybeError Queue::SubmitPendingCommandsImpl() {
    Device* device = ToBackend(GetDevice());
    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());

    if (!mPendingCommands.NeedsSubmit()) {
        return {};
    }

    DAWN_TRY(mPendingCommands.ExecuteCommandList(device, mCommandQueue.Get()));
    RecycleLastCommandListAfter(GetPendingCommandSerial());

    // Immediately reopen the command recording context so it is always available.
    DAWN_TRY(NextSerial());
    DAWN_TRY(RecycleUnusedCommandLists());
    return OpenPendingCommands();
}

MaybeError Queue::NextSerial() {
    Device* device = ToBackend(GetDevice());
    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());
    // NextSerial should not ever be called with a command list that needs submission since the
    // underlying command allocator could be recycled after the serial completes on the GPU.
    DAWN_ASSERT(!mPendingCommands.NeedsSubmit());

    IncrementLastSubmittedCommandSerial();

    TRACE_EVENT1(device->GetPlatform(), General, "D3D12Device::SignalFence", "serial",
                 uint64_t(GetLastSubmittedCommandSerial()));

    return CheckHRESULT(
        mCommandQueue->Signal(mFence.Get(), uint64_t(GetLastSubmittedCommandSerial())),
        "D3D12 command queue signal fence");
}

MaybeError Queue::WaitForSerial(ExecutionSerial serial) {
    if (GetCompletedCommandSerial() >= serial) {
        return {};
    }
    DAWN_TRY_ASSIGN(std::ignore,
                    WaitForQueueSerialImpl(serial, std::numeric_limits<Nanoseconds>::max()));
    return CheckPassedSerials();
}

bool Queue::HasPendingCommands() const {
    return mPendingCommands.NeedsSubmit();
}

ResultOrError<ExecutionSerial> Queue::CheckAndUpdateCompletedSerials() {
    // TODO(crbug.com/40643114): Revisit whether this lock is needed for this backend.
    auto deviceGuard = GetDevice()->GetGuard();

    ExecutionSerial completedSerial = ExecutionSerial(mFence->GetCompletedValue());
    if (completedSerial == ExecutionSerial(UINT64_MAX)) [[unlikely]] {
        // GetCompletedValue returns UINT64_MAX if the device was removed.
        // Try to query the failure reason.
        ID3D12Device* d3d12Device = ToBackend(GetDevice())->GetD3D12Device();
        DAWN_TRY(CheckHRESULT(d3d12Device->GetDeviceRemovedReason(),
                              "ID3D12Device::GetDeviceRemovedReason"));
        // Otherwise, return a generic device lost error.
        return DAWN_DEVICE_LOST_ERROR("Device lost");
    }

    if (completedSerial <= GetCompletedCommandSerial()) {
        return ExecutionSerial(0);
    }

    DAWN_TRY(RecycleSystemEventReceivers(completedSerial));

    return completedSerial;
}

void Queue::ForceEventualFlushOfCommands() {
    mPendingCommands.SetNeedsSubmit();
}

MaybeError Queue::WaitForIdleForDestruction() {
    // Immediately forget about all pending commands
    mPendingCommands.Release();

    // Wait for all in-flight commands to finish executing and clean
    DAWN_TRY(NextSerial());
    DAWN_TRY(WaitForSerial(GetLastSubmittedCommandSerial()));

    // Clean up after waiting for the commands.
    return RecycleUnusedCommandLists();
}

CommandRecordingContext* Queue::GetPendingCommandContext(SubmitMode submitMode) {
    if (submitMode == SubmitMode::Normal) {
        mPendingCommands.SetNeedsSubmit();
    }
    return &mPendingCommands;
}

void Queue::SetLabelImpl() {
    Device* device = ToBackend(GetDevice());
    // TODO(crbug.com/dawn/1344): When we start using multiple queues this needs to be adjusted
    // so it doesn't always change the default queue's label.
    SetDebugName(device, mCommandQueue.Get(), "Dawn_Queue", GetLabel());
}

void Queue::SetEventOnCompletion(ExecutionSerial serial, HANDLE event) {
    mFence->SetEventOnCompletion(static_cast<uint64_t>(serial), event);
}

MaybeError Queue::OpenPendingCommands() {
    DAWN_ASSERT(mLastAllocatorUsed == kNoCommandAllocator);

    // If there are no free allocators, get the oldest serial in flight and wait on it
    if (mFreeAllocators.none()) {
        const ExecutionSerial firstSerial = mInFlightCommandAllocators.FirstSerial();
        DAWN_TRY(WaitForSerial(firstSerial));
        DAWN_TRY(RecycleUnusedCommandLists());
    }

    DAWN_ASSERT(mFreeAllocators.any());
    ID3D12Device* d3d12Device = ToBackend(GetDevice())->GetD3D12Device();

    // Get the index of the first free allocator from the bitset
    uint32_t freeIndex = *(mFreeAllocators).begin();
    mFreeAllocators.reset(freeIndex);
    auto& allocator = mCommandAllocators[freeIndex];

    // Lazily create an allocator or reset an existing one to start a new command list on it.
    if (freeIndex >= mAllocatorCount) {
        DAWN_ASSERT(freeIndex == mAllocatorCount);
        mAllocatorCount++;

        DAWN_TRY(
            CheckHRESULT(d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                             IID_PPV_ARGS(&allocator.allocator)),
                         "D3D12 create command allocator"));
        DAWN_TRY(CheckHRESULT(d3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                             allocator.allocator.Get(), nullptr,
                                                             IID_PPV_ARGS(&allocator.list)),
                              "D3D12 creating direct command list"));
    } else {
        DAWN_TRY(CheckHRESULT(allocator.list->Reset(allocator.allocator.Get(), nullptr),
                              "D3D12 resetting command list"));
    }

    mLastAllocatorUsed = freeIndex;
    mPendingCommands.Open(mCommandAllocators[mLastAllocatorUsed].list);
    return {};
}

void Queue::RecycleLastCommandListAfter(ExecutionSerial serial) {
    mInFlightCommandAllocators.Enqueue(mLastAllocatorUsed, serial);
    mLastAllocatorUsed = kNoCommandAllocator;
}

MaybeError Queue::RecycleUnusedCommandLists() {
    ExecutionSerial completedSerial = GetCompletedCommandSerial();
    for (auto index : mInFlightCommandAllocators.IterateUpTo(completedSerial)) {
        DAWN_TRY(CheckHRESULT(mCommandAllocators[index].allocator->Reset(),
                              "D3D12 reset command allocator"));
        mFreeAllocators.set(index);
    }
    mInFlightCommandAllocators.ClearUpTo(completedSerial);

    return {};
}

}  // namespace dawn::native::d3d12
