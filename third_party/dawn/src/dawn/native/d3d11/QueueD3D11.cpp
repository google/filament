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

#include "dawn/native/d3d11/QueueD3D11.h"

#include <algorithm>
#include <chrono>
#include <deque>
#include <iterator>
#include <limits>
#include <thread>
#include <utility>
#include <vector>

#include "absl/container/inlined_vector.h"
#include "dawn/common/Log.h"
#include "dawn/native/WaitAnySystemEvent.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/BufferD3D11.h"
#include "dawn/native/d3d11/CommandBufferD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/DeviceInfoD3D11.h"
#include "dawn/native/d3d11/PhysicalDeviceD3D11.h"
#include "dawn/native/d3d11/SharedFenceD3D11.h"
#include "dawn/native/d3d11/TextureD3D11.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d11 {
namespace {

// Queue's subclass that uses monitored fences exclusively to signal/track work done.
class MonitoredFenceQueue final : public Queue {
  public:
    using Queue::Queue;
    MaybeError Initialize();
    MaybeError NextSerial() override;
    ResultOrError<ExecutionSerial> CheckCompletedSerialsImpl() override;
    void SetEventOnCompletion(ExecutionSerial serial, HANDLE event) override;

  private:
    ~MonitoredFenceQueue() override = default;
};

// Queue's subclass that uses SystemEvent + Flush1 to track work done.
class SystemEventQueue final : public Queue {
  public:
    using Queue::Queue;
    MaybeError Initialize();
    MaybeError NextSerial() override;
    ResultOrError<ExecutionSerial> CheckCompletedSerialsImpl() override;
    ResultOrError<ExecutionSerial> WaitForQueueSerialImpl(ExecutionSerial serial,
                                                          Nanoseconds timeout) override;
    void SetEventOnCompletion(ExecutionSerial serial, HANDLE event) override;

  private:
    ~SystemEventQueue() override = default;

    struct SerialEventReceiverPair {
        ExecutionSerial serial;
        SystemEventReceiver receiver;
    };
    // Events associated with submitted commands. They are in old to recent order.
    MutexProtected<std::deque<SerialEventReceiverPair>> mPendingEvents;

    // List of completed events to be recycled in CheckCompletedSerialsImpl().
    MutexProtected<std::vector<SerialEventReceiverPair>> mCompletedEvents;
};

// Queue's subclass that doesn't flush the commands to GPU immediately in Submit().
// This class uses ID3D11Query to track a serial's work completion. ID3D11Query::GetData() can be
// used to check whether the serial has passed or not.
// Note that if the commands were never sent to the GPU, ID3D11Query::GetData() might never return
// true and the application might wait indefinitely for a serial to complete. Thus, a flush must be
// triggered implicitly at some points. For the current implementation, a flush will be triggered
// when one of the following conditions is met:
// - Application calls IDXGISwapChain::Present() either via SwapChain class or externally.
// - Application calls WaitAny() to wait for a GPU operation/buffer mapping to finish.
// - ID3D11Query::GetData() has been called N times. This can be triggered indirectly via:
//   - Calling Device::Tick() or Queue::Submit() N times.
//   - Both the above methods indirectly trigger CheckAndUpdateCompletedSerials() which in turn
//   calls ID3D11Query::GetData() N times.
class DelayFlushQueue final : public Queue {
  public:
    using Queue::Queue;
    MaybeError Initialize();
    MaybeError NextSerial() override;
    ResultOrError<ExecutionSerial> CheckCompletedSerialsImpl() override;
    ResultOrError<ExecutionSerial> WaitForQueueSerialImpl(ExecutionSerial serial,
                                                          Nanoseconds timeout) override;
    void SetEventOnCompletion(ExecutionSerial serial, HANDLE event) override;

  private:
    ~DelayFlushQueue() override = default;

    // Check for completed serials in the pending list.
    MaybeError CheckPendingQueries(const ScopedCommandRecordingContext* commandContext);

    void MarkPendingQueriesAsComplete(size_t numCompletedQueries);

    MaybeError BlockWaitForLastSubmittedSerial(const ScopedCommandRecordingContext* commandContext);

    struct EventQuery {
      public:
        EventQuery(ExecutionSerial serial, ComPtr<ID3D11Query> query)
            : mSerial(serial), mQuery(std::move(query)) {}

        ExecutionSerial Serial() const { return mSerial; }
        ComPtr<ID3D11Query> AcquireQuery() { return std::move(mQuery); }
        ID3D11Query* GetQuery() { return mQuery.Get(); }

        // Number of GetData() calls on this query.
        size_t checkCount = 0;

      private:
        ExecutionSerial mSerial;
        ComPtr<ID3D11Query> mQuery;
    };

    ResultOrError<bool> IsQueryCompleted(const ScopedCommandRecordingContext* commandContext,
                                         bool requireFlush,
                                         EventQuery* eventQuery);

    // Events associated with submitted commands. They are in old to recent order.
    std::deque<EventQuery> mPendingQueries;
    // List of completed queries to be recycled in CheckCompletedSerialsImpl().
    std::vector<EventQuery> mCompletedQueries;
    // List of recycled queries.
    std::vector<ComPtr<ID3D11Query>> mFreeQueries;
};

}  // namespace

ResultOrError<Ref<Queue>> Queue::Create(Device* device, const QueueDescriptor* descriptor) {
    const auto& deviceInfo = ToBackend(device->GetPhysicalDevice())->GetDeviceInfo();
    if (device->IsToggleEnabled(Toggle::D3D11DelayFlushToGPU)) {
        auto queue = AcquireRef(new DelayFlushQueue(device, descriptor));
        DAWN_TRY(queue->Initialize());
        return queue;
    }

    if (device->IsToggleEnabled(Toggle::D3D11UseUnmonitoredFence) ||
        device->IsToggleEnabled(Toggle::D3D11DisableFence)) {
        auto queue = AcquireRef(new SystemEventQueue(device, descriptor));
        DAWN_TRY(queue->Initialize());
        return queue;
    }

    DAWN_ASSERT(deviceInfo.supportsMonitoredFence);
    auto queue = AcquireRef(new MonitoredFenceQueue(device, descriptor));
    DAWN_TRY(queue->Initialize());
    return queue;
}

MaybeError Queue::Initialize(bool useNonMonitoredFence) {
    if (!GetDevice()->IsToggleEnabled(Toggle::D3D11DisableFence)) {
        DAWN_TRY(InitializeD3DFence(useNonMonitoredFence));
    }

    return {};
}

MaybeError Queue::InitializeD3DFence(bool useNonMonitoredFence) {
    const auto& deviceInfo = ToBackend(GetDevice()->GetPhysicalDevice())->GetDeviceInfo();

    // Create the fence.
    D3D11_FENCE_FLAG flags = D3D11_FENCE_FLAG_SHARED;
    if (useNonMonitoredFence) {
        if (deviceInfo.supportsNonMonitoredFence) {
            flags |= D3D11_FENCE_FLAG_NON_MONITORED;
            // For adapters that support both monitored and non-monitored fences, non-monitored
            // fences are only supported when created with the D3D12_FENCE_FLAG_SHARED and
            // D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER flags
            // https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_6/ne-dxgi1_6-dxgi_adapter_flag3
            if (deviceInfo.supportsMonitoredFence) {
                flags |= D3D11_FENCE_FLAG_SHARED_CROSS_ADAPTER;
            }
        } else {
            WarningLog()
                << "D3D11: non-monitored fence is not supported, fallback to monitored fence";
        }
    } else {
        DAWN_ASSERT(deviceInfo.supportsMonitoredFence);
    }
    DAWN_TRY(CheckHRESULT(
        ToBackend(GetDevice())->GetD3D11Device5()->CreateFence(0, flags, IID_PPV_ARGS(&mFence)),
        useNonMonitoredFence ? "D3D11: creating non-monitored fence"
                             : "D3D11: creating monitored fence"));

    DAWN_TRY_ASSIGN(mSharedFence, SharedFence::Create(ToBackend(GetDevice()),
                                                      "Internal shared DXGI fence", mFence));

    return {};
}

MaybeError Queue::InitializePendingContext() {
    // Initialize mPendingCommands. After this, calls to the use the command context
    // are thread safe.
    CommandRecordingContext commandContext;
    DAWN_TRY(commandContext.Initialize(ToBackend(GetDevice())));

    mPendingCommands.Use(
        [&](auto pendingCommandContext) { *pendingCommandContext = std::move(commandContext); });

    // Configure the command context's uniform buffer. This is used to emulate builtins.
    // Creating the buffer is done outside of Initialize because it requires mPendingCommands
    // to already be initialized.
    Ref<BufferBase> uniformBuffer;
    DAWN_TRY_ASSIGN(uniformBuffer,
                    CommandRecordingContext::CreateInternalUniformBuffer(GetDevice()));

    {
        auto persistentCommandContext =
            GetScopedSwapStatePendingCommandContext(SubmitMode::Passive);
        DAWN_TRY(persistentCommandContext.SetInternalUniformBuffer(std::move(uniformBuffer)));
    }

    return {};
}

void Queue::DestroyImpl() {
    // Release the shared fence here to prevent a ref-cycle with the device, but do not destroy the
    // underlying native fence so that we can return a SharedFence on EndAccess after destruction.
    mSharedFence = nullptr;

    mPendingCommands.Use([&](auto pendingCommands) {
        pendingCommands->Destroy();
        mPendingCommandsNeedSubmit.store(false, std::memory_order_release);
    });
}

ResultOrError<Ref<d3d::SharedFence>> Queue::GetOrCreateSharedFence() {
    if (mSharedFence == nullptr) {
        if (!mFence) {
            DAWN_ASSERT(GetDevice()->IsToggleEnabled(Toggle::D3D11DisableFence));
            return Ref<d3d::SharedFence>{};
        }
        DAWN_ASSERT(!IsAlive());
        return SharedFence::Create(ToBackend(GetDevice()), "Internal shared DXGI fence", mFence);
    }
    return mSharedFence;
}

ScopedCommandRecordingContext Queue::GetScopedPendingCommandContext(SubmitMode submitMode) {
    return mPendingCommands.Use([&](auto commands) {
        if (submitMode == SubmitMode::Normal) {
            mPendingCommandsNeedSubmit.store(true, std::memory_order_release);
        }
        return ScopedCommandRecordingContext(std::move(commands));
    });
}

ScopedSwapStateCommandRecordingContext Queue::GetScopedSwapStatePendingCommandContext(
    SubmitMode submitMode) {
    return mPendingCommands.Use([&](auto commands) {
        if (submitMode == SubmitMode::Normal) {
            mPendingCommandsNeedSubmit.store(true, std::memory_order_release);
        }
        return ScopedSwapStateCommandRecordingContext(std::move(commands));
    });
}

MaybeError Queue::SubmitPendingCommandsImpl() {
    bool needsSubmit = mPendingCommands.Use([&](auto pendingCommands) {
        pendingCommands->ReleaseKeyedMutexes();
        return mPendingCommandsNeedSubmit.exchange(false, std::memory_order_acq_rel);
    });
    if (needsSubmit) {
        return NextSerial();
    }
    return {};
}

MaybeError Queue::SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) {
    // CommandBuffer::Execute() will modify the state of the global immediate device context, it may
    // affect following usage of it.
    // TODO(dawn:1770): figure how if we need to track and restore the state of the immediate device
    // context.
    TRACE_EVENT_BEGIN0(GetDevice()->GetPlatform(), Recording, "CommandBufferD3D11::Execute");
    {
        auto commandContext =
            GetScopedSwapStatePendingCommandContext(QueueBase::SubmitMode::Normal);
        for (uint32_t i = 0; i < commandCount; ++i) {
            DAWN_TRY(ToBackend(commands[i])->Execute(&commandContext));
        }
    }
    DAWN_TRY(SubmitPendingCommandsImpl());
    TRACE_EVENT_END0(GetDevice()->GetPlatform(), Recording, "CommandBufferD3D11::Execute");

    return {};
}

ResultOrError<ExecutionSerial> Queue::CheckAndUpdateCompletedSerials() {
    // TODO(crbug.com/40643114): Revisit whether this lock is needed for this backend.
    auto deviceGuard = GetDevice()->GetGuard();

    if (!mPendingCommands->IsValid()) {
        // Device might already be destroyed. Skip checking further.
        return GetCompletedCommandSerial();
    }

    ExecutionSerial completedSerial;

    DAWN_TRY_ASSIGN(completedSerial, CheckCompletedSerialsImpl());

    // Finalize Mapping on ready buffers.
    DAWN_TRY(CheckAndMapReadyBuffers(completedSerial));

    return completedSerial;
}

MaybeError Queue::CheckAndMapReadyBuffers(ExecutionSerial completedSerial) {
    auto commandContext = GetScopedPendingCommandContext(QueueBase::SubmitMode::Passive);
    for (const auto& bufferEntry : mPendingMapBuffers.IterateUpTo(completedSerial)) {
        DAWN_TRY(
            bufferEntry.buffer->FinalizeMap(&commandContext, completedSerial, bufferEntry.mode));
    }
    mPendingMapBuffers.ClearUpTo(completedSerial);
    return {};
}

void Queue::TrackPendingMapBuffer(Ref<Buffer>&& buffer,
                                  wgpu::MapMode mode,
                                  ExecutionSerial readySerial) {
    mPendingMapBuffers.Enqueue({buffer, mode}, readySerial);
}

MaybeError Queue::WriteBufferImpl(BufferBase* buffer,
                                  uint64_t bufferOffset,
                                  const void* data,
                                  size_t size) {
    if (size == 0) {
        // skip the empty write
        return {};
    }

    auto commandContext = GetScopedPendingCommandContext(QueueBase::SubmitMode::Normal);
    return ToBackend(buffer)->Write(&commandContext, bufferOffset, data, size);
}

MaybeError Queue::WriteTextureImpl(const TexelCopyTextureInfo& destination,
                                   const void* data,
                                   size_t dataSize,
                                   const TexelCopyBufferLayout& dataLayout,
                                   const Extent3D& writeSizePixel) {
    if (writeSizePixel.width == 0 || writeSizePixel.height == 0 ||
        writeSizePixel.depthOrArrayLayers == 0) {
        return {};
    }

    auto commandContext = GetScopedPendingCommandContext(QueueBase::SubmitMode::Normal);
    TextureCopy textureCopy;
    textureCopy.texture = destination.texture;
    textureCopy.mipLevel = destination.mipLevel;
    textureCopy.origin = destination.origin;
    textureCopy.aspect = SelectFormatAspects(destination.texture->GetFormat(), destination.aspect);

    SubresourceRange subresources = GetSubresourcesAffectedByCopy(textureCopy, writeSizePixel);

    Texture* texture = ToBackend(destination.texture);
    DAWN_TRY(texture->SynchronizeTextureBeforeUse(&commandContext));
    return texture->Write(&commandContext, subresources, destination.origin, writeSizePixel,
                          static_cast<const uint8_t*>(data) + dataLayout.offset,
                          dataLayout.bytesPerRow, dataLayout.rowsPerImage);
}

bool Queue::HasPendingCommands() const {
    return mPendingCommandsNeedSubmit.load(std::memory_order_acquire);
}

void Queue::ForceEventualFlushOfCommands() {}

MaybeError Queue::WaitForIdleForDestruction() {
    if (!mPendingCommands->IsValid()) {
        return {};
    }

    DAWN_TRY(NextSerial());
    // Wait for all in-flight commands to finish executing
    DAWN_TRY_ASSIGN(std::ignore, WaitForQueueSerialImpl(GetLastSubmittedCommandSerial(),
                                                        std::numeric_limits<Nanoseconds>::max()));
    return CheckPassedSerials();
}

// MonitoredFenceQueue:
MaybeError MonitoredFenceQueue::Initialize() {
    return Queue::Initialize(/*useNonMonitoredFence=*/false);
}

MaybeError MonitoredFenceQueue::NextSerial() {
    auto commandContext = GetScopedPendingCommandContext(SubmitMode::Passive);

    DAWN_TRY(commandContext.FlushBuffersForSyncingWithCPU());

    const uint64_t submitSerial = uint64_t(GetPendingCommandSerial());

    {
        TRACE_EVENT1(GetDevice()->GetPlatform(), General, "D3D11Device::SignalFence", "serial",
                     submitSerial);
        DAWN_TRY(CheckHRESULT(commandContext.Signal(mFence.Get(), submitSerial),
                              "D3D11 command queue signal fence"));
    }

    IncrementLastSubmittedCommandSerial();

    return {};
}

ResultOrError<ExecutionSerial> MonitoredFenceQueue::CheckCompletedSerialsImpl() {
    ExecutionSerial completedSerial = ExecutionSerial(mFence->GetCompletedValue());
    if (completedSerial == ExecutionSerial(UINT64_MAX)) [[unlikely]] {
        // GetCompletedValue returns UINT64_MAX if the device was removed.
        // Try to query the failure reason.
        ID3D11Device* d3d11Device = ToBackend(GetDevice())->GetD3D11Device();
        DAWN_TRY(CheckHRESULT(d3d11Device->GetDeviceRemovedReason(),
                              "ID3D11Device::GetDeviceRemovedReason"));
        // Otherwise, return a generic device lost error.
        return DAWN_DEVICE_LOST_ERROR("Device lost");
    }

    DAWN_TRY(RecycleSystemEventReceivers(completedSerial));

    return completedSerial;
}

void MonitoredFenceQueue::SetEventOnCompletion(ExecutionSerial serial, HANDLE event) {
    mFence->SetEventOnCompletion(static_cast<uint64_t>(serial), event);
}

// SystemEventQueue:
MaybeError SystemEventQueue::Initialize() {
    return Queue::Initialize(
        /*useNonMonitoredFence=*/GetDevice()->IsToggleEnabled(Toggle::D3D11UseUnmonitoredFence));
}

MaybeError SystemEventQueue::NextSerial() {
    auto commandContext = GetScopedPendingCommandContext(SubmitMode::Passive);

    DAWN_TRY(commandContext.FlushBuffersForSyncingWithCPU());

    const ExecutionSerial submitSerial = GetPendingCommandSerial();
    if (commandContext->AcquireNeedsFence()) {
        DAWN_ASSERT(mFence);

        TRACE_EVENT1(GetDevice()->GetPlatform(), General, "D3D11Device::SignalFence", "serial",
                     uint64_t(submitSerial));
        DAWN_TRY(CheckHRESULT(commandContext.Signal(mFence.Get(), uint64_t(submitSerial)),
                              "D3D11 command queue signal fence"));
    }

    SystemEventReceiver receiver;
    DAWN_TRY_ASSIGN(receiver, GetSystemEventReceiver());
    commandContext.Flush1(D3D11_CONTEXT_TYPE_ALL, receiver.GetPrimitive().Get());
    mPendingEvents->push_back({submitSerial, std::move(receiver)});

    IncrementLastSubmittedCommandSerial();

    return {};
}

ResultOrError<ExecutionSerial> SystemEventQueue::CheckCompletedSerialsImpl() {
    ExecutionSerial completedSerial;
    std::vector<SystemEventReceiver> returnedReceivers;
    // Check for completed events in the pending list.
    DAWN_TRY_ASSIGN(
        completedSerial,
        mPendingEvents.Use([&](auto pendingEvents) -> ResultOrError<ExecutionSerial> {
            if (pendingEvents->empty()) {
                return GetLastSubmittedCommandSerial();
            }

            absl::InlinedVector<HANDLE, 8> handles;
            const size_t numberOfHandles =
                std::min(pendingEvents->size(), static_cast<size_t>(MAXIMUM_WAIT_OBJECTS));
            handles.reserve(numberOfHandles);
            // Gather events in reversed order (from the most recent to the oldest events).
            std::for_each_n(pendingEvents->rbegin(), numberOfHandles, [&handles](const auto& e) {
                handles.push_back(e.receiver.GetPrimitive().Get());
            });
            DWORD result =
                WaitForMultipleObjects(handles.size(), handles.data(), /*bWaitAll=*/false,
                                       /*dwMilliseconds=*/0);
            DAWN_INTERNAL_ERROR_IF(result == WAIT_FAILED, "WaitForMultipleObjects() failed");

            DAWN_INTERNAL_ERROR_IF(
                result >= WAIT_ABANDONED_0 && result < WAIT_ABANDONED_0 + handles.size(),
                "WaitForMultipleObjects() get abandoned event");

            if (result == WAIT_TIMEOUT) {
                return GetCompletedCommandSerial();
            }

            DAWN_CHECK(result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + pendingEvents->size());
            const size_t completedEventIndex = result - WAIT_OBJECT_0;
            // |WaitForMultipleObjects()| returns the smallest index, if more than one
            // events are signalled. So the number of completed events are
            // |mPendingEvents.size() - index|.
            const size_t completedEvents = pendingEvents->size() - completedEventIndex;
            auto completedSerial = pendingEvents->at(completedEvents - 1).serial;
            returnedReceivers.reserve(completedEvents);
            std::for_each_n(pendingEvents->begin(), completedEvents, [&returnedReceivers](auto& e) {
                returnedReceivers.emplace_back(std::move(e.receiver));
            });
            pendingEvents->erase(pendingEvents->begin(), pendingEvents->begin() + completedEvents);

            return completedSerial;
        }));

    // Also check for completed events processed by WaitForQueueSerialImpl()
    mCompletedEvents.Use([&](auto completedEvents) {
        returnedReceivers.reserve(returnedReceivers.size() + completedEvents->size());
        for (auto& event : *completedEvents) {
            completedSerial = std::max(completedSerial, event.serial);
            returnedReceivers.emplace_back(std::move(event.receiver));
        }
        completedEvents->clear();
    });

    if (!returnedReceivers.empty()) {
        DAWN_TRY(ReturnSystemEventReceivers(
            std::span(returnedReceivers.data(), returnedReceivers.size())));
    }

    return completedSerial;
}

ResultOrError<ExecutionSerial> SystemEventQueue::WaitForQueueSerialImpl(ExecutionSerial serial,
                                                                        Nanoseconds timeout) {
    ExecutionSerial completedSerial = GetCompletedCommandSerial();
    if (serial <= completedSerial) {
        return serial;
    }

    if (serial > GetLastSubmittedCommandSerial()) {
        return DAWN_FORMAT_INTERNAL_ERROR(
            "Wait a serial (%llu) which is greater than last submitted command serial (%llu).",
            uint64_t(serial), uint64_t(GetLastSubmittedCommandSerial()));
    }

    return mPendingEvents.Use([=, &completedEventsList = mCompletedEvents](
                                  auto pendingEvents) -> ResultOrError<ExecutionSerial> {
        DAWN_ASSERT(!pendingEvents->empty());
        DAWN_ASSERT(serial >= pendingEvents->front().serial);
        DAWN_ASSERT(serial <= pendingEvents->back().serial);
        auto it = std::lower_bound(
            pendingEvents->begin(), pendingEvents->end(), serial,
            [](const SerialEventReceiverPair& a, ExecutionSerial b) { return a.serial < b; });
        DAWN_ASSERT(it != pendingEvents->end());
        DAWN_ASSERT(it->serial == serial);

        // TODO(crbug.com/335553337): call WaitForSingleObject() without holding the mutex.
        DWORD result =
            WaitForSingleObject(it->receiver.GetPrimitive().Get(), ToMilliseconds(timeout));
        DAWN_INTERNAL_ERROR_IF(result == WAIT_FAILED, "WaitForSingleObject() failed");

        if (result != WAIT_OBJECT_0) {
            return kWaitSerialTimeout;
        }

        // Events before |it| should be signalled as well.
        completedEventsList.Use([&](auto completedEvList) {
            completedEvList->insert(completedEvList->end(),
                                    std::make_move_iterator(pendingEvents->begin()),
                                    std::make_move_iterator(it + 1));
        });
        pendingEvents->erase(pendingEvents->begin(), it + 1);

        return serial;
    });
}

void SystemEventQueue::SetEventOnCompletion(ExecutionSerial serial, HANDLE event) {
    DAWN_UNREACHABLE();
}

// DelayFlushQueue:
MaybeError DelayFlushQueue::Initialize() {
    return Queue::Initialize(
        /*useNonMonitoredFence=*/GetDevice()->IsToggleEnabled(Toggle::D3D11UseUnmonitoredFence));
}

MaybeError DelayFlushQueue::NextSerial() {
    auto commandContext = GetScopedPendingCommandContext(SubmitMode::Passive);

    DAWN_TRY(commandContext.FlushBuffersForSyncingWithCPU());

    ExecutionSerial submitSerial = GetPendingCommandSerial();

    ComPtr<ID3D11Query> query;

    if (!mFreeQueries.empty()) {
        query = std::move(mFreeQueries.back());
        mFreeQueries.pop_back();
    } else {
        D3D11_QUERY_DESC queryDesc = {};
        queryDesc.Query = D3D11_QUERY_EVENT;
        DAWN_TRY(
            CheckHRESULT(ToBackend(GetDevice())->GetD3D11Device3()->CreateQuery(&queryDesc, &query),
                         "CreateQuery"));
    }

    commandContext.End(query.Get());

    if (commandContext->AcquireNeedsFence()) {
        DAWN_ASSERT(mFence);

        TRACE_EVENT1(GetDevice()->GetPlatform(), General, "D3D11Device::SignalFence", "serial",
                     uint64_t(submitSerial));
        DAWN_TRY(CheckHRESULT(commandContext.Signal(mFence.Get(), uint64_t(submitSerial)),
                              "D3D11 command queue signal fence"));
    }

    mPendingQueries.emplace_back(submitSerial, std::move(query));

    IncrementLastSubmittedCommandSerial();

    return {};
}

MaybeError DelayFlushQueue::CheckPendingQueries(
    const ScopedCommandRecordingContext* commandContext) {
    if (mPendingQueries.empty()) {
        return {};
    }

    // Check queries' status starting from oldest to most recent.
    // TODO(416736350): Consider using a binary search.
    size_t completedQueriesCount = 0;
    for (size_t i = 0; i < mPendingQueries.size(); ++i) {
        bool done;
        DAWN_TRY_ASSIGN(
            done, IsQueryCompleted(commandContext, /*requireFlush=*/false, &mPendingQueries[i]));
        if (!done) {
            // We stop at 1st incompleted query.
            break;
        }
        completedQueriesCount++;
    }

    if (completedQueriesCount == 0) {
        return {};
    }

    MarkPendingQueriesAsComplete(completedQueriesCount);

    return {};
}

ResultOrError<ExecutionSerial> DelayFlushQueue::CheckCompletedSerialsImpl() {
    ExecutionSerial completedSerial;
    {
        auto commandContext = GetScopedPendingCommandContext(SubmitMode::Passive);

        completedSerial = GetCompletedCommandSerial();

        DAWN_TRY(CheckPendingQueries(&commandContext));

        for (auto& e : mCompletedQueries) {
            completedSerial = std::max(completedSerial, e.Serial());
            mFreeQueries.emplace_back(e.AcquireQuery());
        }
        mCompletedQueries.clear();
    }

    return completedSerial;
}

ResultOrError<ExecutionSerial> DelayFlushQueue::WaitForQueueSerialImpl(ExecutionSerial waitSerial,
                                                                       Nanoseconds timeout) {
    ExecutionSerial completedSerial = GetCompletedCommandSerial();
    if (waitSerial <= completedSerial) {
        return waitSerial;
    }

    if (waitSerial > GetLastSubmittedCommandSerial()) {
        return DAWN_FORMAT_INTERNAL_ERROR(
            "Wait a serial (%llu) which is greater than last submitted command serial (%llu).",
            uint64_t(waitSerial), uint64_t(GetLastSubmittedCommandSerial()));
    }

    auto commandContext = GetScopedPendingCommandContext(SubmitMode::Passive);

    if (mPendingQueries.empty() || waitSerial < mPendingQueries.front().Serial()) {
        // Empty list must mean the serial have already completed.
        return waitSerial;
    }

    if (uint64_t(timeout) == std::numeric_limits<uint64_t>::max() &&
        waitSerial == GetLastSubmittedCommandSerial()) {
        // If user submits then waits immediately, we can do a small optimization here,
        // Flush + enqueue SetEvent then wait on the event. This can avoid spinning wait below,
        // wasting less CPU cycles.
        DAWN_TRY(BlockWaitForLastSubmittedSerial(&commandContext));
    }

    DAWN_ASSERT(waitSerial >= mPendingQueries.front().Serial());
    DAWN_ASSERT(waitSerial <= mPendingQueries.back().Serial());
    auto it =
        std::lower_bound(mPendingQueries.begin(), mPendingQueries.end(), waitSerial,
                         [](const EventQuery& a, ExecutionSerial b) { return a.Serial() < b; });
    DAWN_ASSERT(it != mPendingQueries.end());
    DAWN_ASSERT(it->Serial() == waitSerial);

    bool done;
    DAWN_TRY_ASSIGN(done, IsQueryCompleted(&commandContext, /*requireFlush=*/false, &(*it)));
    if (timeout == Nanoseconds(0)) {
        if (!done) {
            // Return timed-out immediately without using a timer.
            return kWaitSerialTimeout;
        }
    } else {
        auto startTime = std::chrono::steady_clock::now();

        while (!done) {
            DAWN_TRY_ASSIGN(done, IsQueryCompleted(&commandContext, /*requireFlush=*/true, &(*it)));

            if (!done) {
                auto curTime = std::chrono::steady_clock::now();
                auto elapsedNs =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(curTime - startTime);
                if (static_cast<uint64_t>(elapsedNs.count()) >= uint64_t(timeout)) {
                    return kWaitSerialTimeout;
                }
                std::this_thread::yield();
            }
        }
    }

    // Completed queries will be recycled in CheckCompletedSerialsImpl();
    auto numCompletedQueries = std::distance(mPendingQueries.begin(), it) + 1;
    MarkPendingQueriesAsComplete(numCompletedQueries);
    return done ? waitSerial : kWaitSerialTimeout;
}

MaybeError DelayFlushQueue::BlockWaitForLastSubmittedSerial(
    const ScopedCommandRecordingContext* commandContext) {
    TRACE_EVENT0(GetDevice()->GetPlatform(), General,
                 "DelayFlushQueue::BlockWaitForLastSubmittedSerial");

    SystemEventReceiver receiver;
    DAWN_TRY_ASSIGN(receiver, GetSystemEventReceiver());
    commandContext->Flush1(D3D11_CONTEXT_TYPE_ALL, receiver.GetPrimitive().Get());

    DWORD result = WaitForSingleObject(receiver.GetPrimitive().Get(), INFINITE);
    DAWN_INTERNAL_ERROR_IF(result != WAIT_OBJECT_0, "WaitForSingleObject() failed");

    SystemEventReceiver returnedReceivers[] = {std::move(receiver)};
    return ReturnSystemEventReceivers(returnedReceivers);
}

void DelayFlushQueue::SetEventOnCompletion(ExecutionSerial serial, HANDLE event) {
    DAWN_UNREACHABLE();
}

void DelayFlushQueue::MarkPendingQueriesAsComplete(size_t numCompletedQueries) {
    mCompletedQueries.insert(
        mCompletedQueries.end(), std::make_move_iterator(mPendingQueries.begin()),
        std::make_move_iterator(mPendingQueries.begin() + numCompletedQueries));

    mPendingQueries.erase(mPendingQueries.begin(), mPendingQueries.begin() + numCompletedQueries);
}

ResultOrError<bool> DelayFlushQueue::IsQueryCompleted(
    const ScopedCommandRecordingContext* commandContext,
    bool requireFlush,
    EventQuery* eventQuery) {
    BOOL done;

    // If the GetData() calls count is 100 we will flush the commands. This is to avoid infinity
    // loop when the application uses device.Tick() in a busy wait. The number 100 is arbitrarily
    // chosen. It could be changed in future. Notes:
    // - We shouldn't trigger flush eagerly when calls count < 100 because the the flush might be
    // triggered externally via SwapChain's Present().
    // - We don't need to trigger flush when calls count > 100 because any commands preceding this
    // query should already be flushed when calls count is 100.
    // - If the caller requires a flush, we only trigger flush if the calls count < 100.
    // - TODO(416736350): Consider tracking last flush's serial so that we can skip the flush here
    // if necessary.
    constexpr size_t kFlushCheckpoint = 100;
    if (requireFlush && eventQuery->checkCount < kFlushCheckpoint) {
        eventQuery->checkCount = kFlushCheckpoint;
    }

    // Pass flag=0 if we want GetData() to flush the commands.
    const UINT dontFlushFlag =
        (eventQuery->checkCount == kFlushCheckpoint) ? 0 : D3D11_ASYNC_GETDATA_DONOTFLUSH;
    HRESULT hr =
        commandContext->GetData(eventQuery->GetQuery(), &done, sizeof(done), dontFlushFlag);
    DAWN_TRY(CheckHRESULT(hr, "GetQueryData()"));

    eventQuery->checkCount++;

    // A return value of S_FALSE means the query is not completed.
    DAWN_ASSERT(hr == S_FALSE || done == TRUE);
    return hr == S_OK;
}

}  // namespace dawn::native::d3d11
