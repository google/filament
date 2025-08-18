// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/metal/QueueMTL.h"

#include "dawn/common/Math.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/MetalBackend.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/metal/CommandBufferMTL.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::metal {

ResultOrError<Ref<Queue>> Queue::Create(Device* device, const QueueDescriptor* descriptor) {
    Ref<Queue> queue = AcquireRef(new Queue(device, descriptor));
    DAWN_TRY(queue->Initialize());
    return queue;
}

Queue::Queue(Device* device, const QueueDescriptor* descriptor) : QueueBase(device, descriptor) {}

Queue::~Queue() = default;

void Queue::DestroyImpl() {
    // Forget all pending commands.
    mCommandContext.AcquireCommands();
    UpdateWaitingEvents(kMaxExecutionSerial);
    mCommandQueue = nullptr;
    mLastSubmittedCommands = nullptr;

    mSharedFence = nullptr;
    // Don't free `mMtlSharedEvent` because it can be queried after device destruction for
    // synchronization needs. However, we destroy the `mSharedFence` to release its device ref.
}

MaybeError Queue::Initialize() {
    id<MTLDevice> mtlDevice = ToBackend(GetDevice())->GetMTLDevice();

#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 150000
    if (@available(macOS 15.0, *)) {
        if (gpu_info::IsApple(GetDevice()->GetPhysicalDevice()->GetVendorId()) &&
            GetDevice()->IsToggleEnabled(Toggle::EnableShaderPrint)) {
            // Add a logging callback to the command queue that forwards print output to the Dawn
            // logging callback.
            NSError* error = nil;
            MTLLogStateDescriptor* logStateDesc = [MTLLogStateDescriptor new];
            logStateDesc.bufferSize = 4096;
            logStateDesc.level = MTLLogLevelInfo;
            id<MTLLogState> mtlLogState = [mtlDevice newLogStateWithDescriptor:logStateDesc
                                                                         error:&error];
            if (error != nil) {
                return DAWN_INTERNAL_ERROR("Error creating MTLLogState:" +
                                           std::string([error.localizedDescription UTF8String]));
            }
            [mtlLogState addLogHandler:^(NSString* substring, NSString* category, MTLLogLevel level,
                                         NSString* message) {
                GetDevice()->EmitLog([message UTF8String]);
            }];

            MTLCommandQueueDescriptor* mtlQueueDescriptor = [MTLCommandQueueDescriptor new];
            mtlQueueDescriptor.logState = mtlLogState;
            mCommandQueue.Acquire([mtlDevice newCommandQueueWithDescriptor:mtlQueueDescriptor]);
        } else {
            mCommandQueue.Acquire([mtlDevice newCommandQueue]);
        }
    } else
#endif
    {
        mCommandQueue.Acquire([mtlDevice newCommandQueue]);
    }

    if (mCommandQueue == nil) {
        return DAWN_INTERNAL_ERROR("Failed to allocate MTLCommandQueue.");
    }

    mMtlSharedEvent.Acquire([mtlDevice newSharedEvent]);
    if (mMtlSharedEvent == nil) {
        return DAWN_INTERNAL_ERROR("Failed to create MTLSharedEvent.");
    }
    DAWN_TRY_ASSIGN(mSharedFence, GetOrCreateSharedFence());

    return mCommandContext.PrepareNextCommandBuffer(*mCommandQueue);
}

void Queue::UpdateWaitingEvents(ExecutionSerial completedSerial) {
    mWaitingEvents.Use([&](auto events) {
        for (auto& s : events->IterateUpTo(completedSerial)) {
            std::move(s)->Signal();
        }
        events->ClearUpTo(completedSerial);
    });
}

MaybeError Queue::WaitForIdleForDestruction() {
    // Forget all pending commands.
    mCommandContext.AcquireCommands();
    DAWN_TRY(CheckPassedSerials());

    // Wait for all commands to be finished so we can free resources
    while (GetCompletedCommandSerial() != GetLastSubmittedCommandSerial()) {
        usleep(100);
        DAWN_TRY(CheckPassedSerials());
    }

    return {};
}

void Queue::WaitForCommandsToBeScheduled() {
    if (!IsAlive()) {
        return;
    }
    if (GetDevice()->ConsumedError(SubmitPendingCommandBuffer())) {
        return;
    }

    // Only lock the object while we take a reference to it, otherwise we could block further
    // progress if the driver calls the scheduled handler (which also acquires the lock) before
    // finishing the waitUntilScheduled.
    NSPRef<id<MTLCommandBuffer>> lastSubmittedCommands;
    {
        std::lock_guard<std::mutex> lock(mLastSubmittedCommandsMutex);
        lastSubmittedCommands = mLastSubmittedCommands;
    }
    [*lastSubmittedCommands waitUntilScheduled];
}

CommandRecordingContext* Queue::GetPendingCommandContext(SubmitMode submitMode) {
    if (submitMode == SubmitMode::Normal) {
        mCommandContext.SetNeedsSubmit();
    }
    mCommandContext.MarkUsed();
    return &mCommandContext;
}

MaybeError Queue::SubmitPendingCommandBuffer() {
    if (!mCommandContext.NeedsSubmit()) {
        return {};
    }

    auto platform = GetDevice()->GetPlatform();

    // Acquire the pending command buffer, which is retained. It must be released later.
    NSPRef<id<MTLCommandBuffer>> pendingCommands = mCommandContext.AcquireCommands();

    // Replace mLastSubmittedCommands with the mutex held so we avoid races between the
    // schedule handler and this code.
    {
        std::lock_guard<std::mutex> lock(mLastSubmittedCommandsMutex);
        mLastSubmittedCommands = pendingCommands;
    }

    // Make a local copy of the pointer to the commands because it's not clear how ObjC blocks
    // handle types with copy / move constructors being referenced in the block.
    id<MTLCommandBuffer> pendingCommandsPointer = pendingCommands.Get();
    [*pendingCommands addScheduledHandler:^(id<MTLCommandBuffer>) {
        // This is DRF because we hold the mutex for mLastSubmittedCommands and pendingCommands
        // is a local value (and not the member itself).
        std::lock_guard<std::mutex> lock(mLastSubmittedCommandsMutex);
        if (this->mLastSubmittedCommands.Get() == pendingCommandsPointer) {
            this->mLastSubmittedCommands = nullptr;
        }
    }];

    // Update the completed serial once the completed handler is fired. Make a local copy of
    // mLastSubmittedSerial so it is captured by value.
    ExecutionSerial pendingSerial = GetPendingCommandSerial();
    // This ObjC block runs on a different thread
    [*pendingCommands addCompletedHandler:^(id<MTLCommandBuffer>) {
        TRACE_EVENT_ASYNC_END0(platform, GPUWork, "DeviceMTL::SubmitPendingCommandBuffer",
                               uint64_t(pendingSerial));

        this->UpdateCompletedSerialTo(pendingSerial);
        this->UpdateWaitingEvents(pendingSerial);
    }];

    TRACE_EVENT_ASYNC_BEGIN0(platform, GPUWork, "DeviceMTL::SubmitPendingCommandBuffer",
                             uint64_t(pendingSerial));

    DAWN_ASSERT(mSharedFence);
    [*pendingCommands encodeSignalEvent:mSharedFence->GetMTLSharedEvent()
                                  value:static_cast<uint64_t>(pendingSerial)];

    [*pendingCommands commit];
    IncrementLastSubmittedCommandSerial();

    return mCommandContext.PrepareNextCommandBuffer(*mCommandQueue);
}

id<MTLSharedEvent> Queue::GetMTLSharedEvent() const {
    return mMtlSharedEvent.Get();
}

ResultOrError<Ref<SharedFence>> Queue::GetOrCreateSharedFence() {
    if (mSharedFence) {
        return mSharedFence;
    }
    SharedFenceMTLSharedEventDescriptor desc;
    desc.sharedEvent = mMtlSharedEvent.Get();
    return SharedFence::Create(ToBackend(GetDevice()), "Internal MTLSharedEvent", &desc);
}

MaybeError Queue::SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) {
    @autoreleasepool {
        CommandRecordingContext* commandContext = GetPendingCommandContext();

        TRACE_EVENT_BEGIN0(GetDevice()->GetPlatform(), Recording, "CommandBufferMTL::FillCommands");
        for (uint32_t i = 0; i < commandCount; ++i) {
            DAWN_TRY(ToBackend(commands[i])->FillCommands(commandContext));
        }
        TRACE_EVENT_END0(GetDevice()->GetPlatform(), Recording, "CommandBufferMTL::FillCommands");

        DAWN_TRY(SubmitPendingCommandBuffer());

        return {};
    }
}

bool Queue::HasPendingCommands() const {
    return mCommandContext.NeedsSubmit();
}

MaybeError Queue::SubmitPendingCommandsImpl() {
    return SubmitPendingCommandBuffer();
}

ResultOrError<ExecutionSerial> Queue::CheckAndUpdateCompletedSerials() {
    // Metal serials are updated via a thread owned by Metal so we don't actually need to do
    // anything, just return the latest completed serial.
    return GetCompletedCommandSerial();
}

void Queue::ForceEventualFlushOfCommands() {
    if (mCommandContext.WasUsed()) {
        mCommandContext.SetNeedsSubmit();
    }
}

Ref<WaitListEvent> Queue::CreateWorkDoneEvent(ExecutionSerial serial) {
    Ref<WaitListEvent> completionEvent = AcquireRef(new WaitListEvent());
    mWaitingEvents.Use([&](auto events) {
        // Now that we hold the lock, check against completed serial before inserting.
        // This serial may have just completed. If it did, mark the event complete.
        // Also check for device loss. Otherwise, we could enqueue the event
        // after mWaitingEvents has been flushed for device loss, and it'll never get cleaned up.
        if (GetDevice()->IsLost() || serial <= GetCompletedCommandSerial()) {
            completionEvent->Signal();
        } else {
            // Insert the event into the list which will be signaled inside Metal's queue
            // completion handler.
            events->Enqueue(completionEvent, serial);
        }
    });
    return completionEvent;
}

ResultOrError<ExecutionSerial> Queue::WaitForQueueSerialImpl(ExecutionSerial waitSerial,
                                                             Nanoseconds timeout) {
    return CreateWorkDoneEvent(waitSerial)->Wait(timeout) ? waitSerial : kWaitSerialTimeout;
}

}  // namespace dawn::native::metal
