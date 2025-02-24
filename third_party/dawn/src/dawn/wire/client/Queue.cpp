// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/wire/client/Queue.h"

#include <memory>
#include <utility>

#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/EventManager.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::wire::client {
namespace {

class WorkDoneEvent : public TrackedEvent {
  public:
    static constexpr EventType kType = EventType::WorkDone;

    explicit WorkDoneEvent(const WGPUQueueWorkDoneCallbackInfo& callbackInfo)
        : TrackedEvent(callbackInfo.mode),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2) {}

    EventType GetType() override { return kType; }

    WireResult ReadyHook(FutureID futureID, WGPUQueueWorkDoneStatus status) {
        mStatus = status;
        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        if (completionType == EventCompletionType::Shutdown) {
            mStatus = WGPUQueueWorkDoneStatus_InstanceDropped;
        }
        void* userdata1 = mUserdata1.ExtractAsDangling();
        void* userdata2 = mUserdata2.ExtractAsDangling();
        if (mCallback) {
            mCallback(mStatus, userdata1, userdata2);
        }
    }

    WGPUQueueWorkDoneCallback mCallback;
    raw_ptr<void> mUserdata1;
    raw_ptr<void> mUserdata2;

    WGPUQueueWorkDoneStatus mStatus = WGPUQueueWorkDoneStatus_Success;
};

}  // anonymous namespace

Queue::~Queue() = default;

ObjectType Queue::GetObjectType() const {
    return ObjectType::Queue;
}

WireResult Client::DoQueueWorkDoneCallback(ObjectHandle eventManager,
                                           WGPUFuture future,
                                           WGPUQueueWorkDoneStatus status) {
    return GetEventManager(eventManager).SetFutureReady<WorkDoneEvent>(future.id, status);
}

WGPUFuture Queue::OnSubmittedWorkDone(const WGPUQueueWorkDoneCallbackInfo& callbackInfo) {
    // TODO(crbug.com/dawn/2052): Once we always return a future, change this to log to the instance
    // (note, not raise a validation error to the device) and return the null future.
    DAWN_ASSERT(callbackInfo.nextInChain == nullptr);

    Client* client = GetClient();
    auto [futureIDInternal, tracked] =
        GetEventManager().TrackEvent(std::make_unique<WorkDoneEvent>(callbackInfo));
    if (!tracked) {
        return {futureIDInternal};
    }

    QueueOnSubmittedWorkDoneCmd cmd;
    cmd.queueId = GetWireId();
    cmd.eventManagerHandle = GetEventManagerHandle();
    cmd.future = {futureIDInternal};

    client->SerializeCommand(cmd);
    return {futureIDInternal};
}

void Queue::WriteBuffer(WGPUBuffer cBuffer, uint64_t bufferOffset, const void* data, size_t size) {
    Buffer* buffer = FromAPI(cBuffer);

    QueueWriteBufferCmd cmd;
    cmd.queueId = GetWireId();
    cmd.bufferId = buffer->GetWireId();
    cmd.bufferOffset = bufferOffset;
    cmd.data = static_cast<const uint8_t*>(data);
    cmd.size = size;

    GetClient()->SerializeCommand(cmd);
}

void Queue::WriteTexture(const WGPUTexelCopyTextureInfo* destination,
                         const void* data,
                         size_t dataSize,
                         const WGPUTexelCopyBufferLayout* dataLayout,
                         const WGPUExtent3D* writeSize) {
    QueueWriteTextureCmd cmd;
    cmd.queueId = GetWireId();
    cmd.destination = destination;
    cmd.data = static_cast<const uint8_t*>(data);
    cmd.dataSize = dataSize;
    cmd.dataLayout = dataLayout;
    cmd.writeSize = writeSize;

    GetClient()->SerializeCommand(cmd);
}

}  // namespace dawn::wire::client
