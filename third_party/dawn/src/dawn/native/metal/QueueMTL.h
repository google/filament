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

#ifndef SRC_DAWN_NATIVE_METAL_QUEUEMTL_H_
#define SRC_DAWN_NATIVE_METAL_QUEUEMTL_H_

#import <Metal/Metal.h>
#include <map>

#include "dawn/common/MutexProtected.h"
#include "dawn/common/SerialMap.h"
#include "dawn/native/EventManager.h"
#include "dawn/native/Queue.h"
#include "dawn/native/WaitListEvent.h"
#include "dawn/native/metal/CommandRecordingContext.h"
#include "dawn/native/metal/SharedFenceMTL.h"

namespace dawn::native::metal {

class Device;

class Queue final : public QueueBase {
  public:
    static ResultOrError<Ref<Queue>> Create(Device* device, const QueueDescriptor* descriptor);

    CommandRecordingContext* GetPendingCommandContext(SubmitMode submitMode = SubmitMode::Normal);
    MaybeError SubmitPendingCommandBuffer();
    void WaitForCommandsToBeScheduled();
    id<MTLSharedEvent> GetMTLSharedEvent() const;
    ResultOrError<Ref<SharedFence>> GetOrCreateSharedFence();

    Ref<WaitListEvent> CreateWorkDoneEvent(ExecutionSerial serial);
    ResultOrError<ExecutionSerial> WaitForQueueSerialImpl(ExecutionSerial waitSerial,
                                                          Nanoseconds timeout) override;

  private:
    Queue(Device* device, const QueueDescriptor* descriptor);
    ~Queue() override;

    MaybeError Initialize();
    void UpdateWaitingEvents(ExecutionSerial completedSerial);

    MaybeError SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) override;
    bool HasPendingCommands() const override;
    MaybeError SubmitPendingCommandsImpl() override;
    ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() override;
    void ForceEventualFlushOfCommands() override;
    MaybeError WaitForIdleForDestruction() override;
    void DestroyImpl() override;

    NSPRef<id<MTLCommandQueue>> mCommandQueue;
    CommandRecordingContext mCommandContext{this};

    // mLastSubmittedCommands will be accessed in a Metal schedule handler that can be fired on
    // a different thread so we guard access to it with a mutex.
    std::mutex mLastSubmittedCommandsMutex;
    NSPRef<id<MTLCommandBuffer>> mLastSubmittedCommands;

    // This mutex must be held to access mWaitingEvents (which may happen in a Metal driver
    // thread).
    // TODO(crbug.com/dawn/2065): If we atomically knew a conservative lower bound on the
    // mWaitingEvents serials, we could avoid taking this lock sometimes. Optimize if needed.
    // See old draft code: https://dawn-review.googlesource.com/c/dawn/+/137502/29
    MutexProtected<SerialMap<ExecutionSerial, Ref<WaitListEvent>>> mWaitingEvents;

    // A shared event that can be exported for synchronization with other users of Metal.
    // MTLSharedEvent is not available until macOS 10.14+ so use just `id`.
    NSPRef<id> mMtlSharedEvent;
    // The shared event wrapped as a SharedFence object.
    Ref<SharedFence> mSharedFence;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_QUEUEMTL_H_
