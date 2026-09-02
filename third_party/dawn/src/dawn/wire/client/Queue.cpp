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

#include "src/dawn/wire/client/Queue.h"

#include <memory>
#include <string>
#include <utility>

#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/Atomic.h"
#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/wire/client/Client.h"
#include "src/dawn/wire/client/EventManager.h"
#include "src/utils/compiler.h"

namespace dawn::wire::client {
namespace {

// Buffer and Texture uploads larger than 4Mb use a different path optimized for larger transfers.
const uint64_t kWriteXLThreshold = 1024ULL * 1024 * 4;

class WorkDoneEvent : public TrackedEvent {
  public:
    static constexpr EventType kType = EventType::WorkDone;

    explicit WorkDoneEvent(const WGPUQueueWorkDoneCallbackInfo& callbackInfo)
        : TrackedEvent(callbackInfo.mode),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2) {}

    EventType GetType() override { return kType; }

    WireResult ReadyHook(FutureID futureID,
                         WGPUQueueWorkDoneStatus status,
                         WGPUStringView message) {
        mStatus = status;
        mMessage = ToString(message);
        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        if (completionType == EventCompletionType::Shutdown) {
            mStatus = WGPUQueueWorkDoneStatus_CallbackCancelled;
            mMessage = "A valid external Instance reference no longer exists.";
        }
        void* userdata1 = mUserdata1.ExtractAsDangling();
        void* userdata2 = mUserdata2.ExtractAsDangling();
        if (mCallback) {
            mCallback(mStatus, ToOutputStringView(mMessage), userdata1, userdata2);
        }
    }

    WGPUQueueWorkDoneCallback mCallback;
    raw_ptr<void> mUserdata1;
    raw_ptr<void> mUserdata2;

    WGPUQueueWorkDoneStatus mStatus = WGPUQueueWorkDoneStatus_Success;
    std::string mMessage;
};

}  // anonymous namespace

Queue::~Queue() = default;

ObjectType Queue::GetObjectType() const {
    return ObjectType::Queue;
}

uint64_t Queue::GetLastSubmitIndex() const {
    return mLastSubmitIndex;
}

uint64_t Queue::GetCompletedSubmitIndex() const {
    return mCompletedSubmitIndex;
}

void Queue::APISubmit(Span<CommandBuffer* const> commands) {
    mLastSubmitIndex++;

    // Send the submit command
    QueueSubmitCmd cmd;
    cmd.self = ToAPI(this);
    cmd.commands = ToAPI(commands);
    GetClient()->SerializeCommand(cmd);

    // Immediately request a callback for OnSubmittedWorkDone to update mCompletedSubmitIndex before
    // any OnSubmittedWorkDone callbacks from the application.
    struct CallbackData {
        Ref<Queue> self;
        uint64_t submitIndex;
    };
    WGPUQueueWorkDoneCallbackInfo callback = {
        .nextInChain = nullptr,
        .mode = WGPUCallbackMode_AllowSpontaneous,
        .callback =
            [](WGPUQueueWorkDoneStatus status, WGPUStringView, void* userdata1, void* userdata2) {
                if (status != WGPUQueueWorkDoneStatus_Success) {
                    return;
                }

                std::unique_ptr<CallbackData> data(reinterpret_cast<CallbackData*>(userdata1));
                FetchMax(data->self->mCompletedSubmitIndex, data->submitIndex);
            },
        .userdata1 = new CallbackData{this, mLastSubmitIndex},
        .userdata2 = nullptr,
    };

    APIOnSubmittedWorkDone(callback);
}

WireResult Client::DoQueueWorkDoneCallback(ObjectId instanceId,
                                           WGPUFuture future,
                                           WGPUQueueWorkDoneStatus status,
                                           WGPUStringView message) {
    return SetFutureReady<WorkDoneEvent>(instanceId, future.id, status, message);
}

Future Queue::APIOnSubmittedWorkDone(const WGPUQueueWorkDoneCallbackInfo& callbackInfo) {
    // TODO(crbug.com/dawn/2052): Once we always return a future, change this to log to the instance
    // (note, not raise a validation error to the device) and return the null future.
    DAWN_ASSERT(callbackInfo.nextInChain == nullptr);

    Client* client = GetClient();
    auto [futureIDInternal, tracked] =
        GetEventManager().TrackEvent(AcquireRef(new WorkDoneEvent(callbackInfo)));
    if (!tracked) {
        return {futureIDInternal};
    }

    QueueOnSubmittedWorkDoneCmd cmd;
    cmd.queueId = GetWireHandle(client).id;
    cmd.instanceId = GetInstance()->GetWireHandle(client).id;
    cmd.future = {futureIDInternal};

    client->SerializeCommand(cmd);
    return {futureIDInternal};
}

void Queue::APIWriteBuffer(Buffer* buffer, uint64_t bufferOffset, Span<const std::byte> data) {
    if (data.size() >= kWriteXLThreshold) {
        WriteBufferXL(buffer, bufferOffset, data);
        return;
    }

    QueueWriteBufferCmd cmd;
    cmd.queueId = GetWireHandle(GetClient()).id;
    cmd.bufferId = buffer->GetWireHandle(GetClient()).id;
    cmd.bufferOffset = bufferOffset;
    cmd.data = data;

    GetClient()->SerializeCommand(cmd);
}

void Queue::WriteBufferXL(Buffer* buffer, uint64_t bufferOffset, Span<const std::byte> data) {
    Client* client = GetClient();

    // Create the MemoryHandle.
    auto memoryHandle = client->GetMemoryTransferService()->CreateMemoryHandle(data.size());
    if (memoryHandle == nullptr) {
        // There was an OOM that we cannot handle in WriteBuffer: trigger a device loss.
        client->Disconnect();
        return;
    }

    // Write the data to the allocated memory.
    Span<std::byte>(memoryHandle->GetData()).CopyFrom(data);

    // Prepare to serialize the handle and the data update command.
    size_t memoryHandleCreateInfoLength = memoryHandle->GetSerializeCreateSize();
    size_t memoryDataUpdateInfoLength = memoryHandle->GetSerializeDataUpdateSize(0u, data.size());

    QueueWriteBufferXlCmd cmd;
    cmd.queueId = GetWireHandle(client).id;
    cmd.bufferId = buffer->GetWireHandle(client).id;
    cmd.bufferOffset = bufferOffset;
    cmd.size = data.size();

    // SAFETY: These Spans are NEVER supposed to be read/serialized, so nullptr is fine.
    // The members are not serialized because skip_serialize, but are Spans so that on
    // the deserialization side we have well-formed members.
    // TODO(https://crbug.com/542275488): Clean these up if when we update command extension
    // serialization to serialize into this span directly.
    cmd.memoryHandleCreateInfo = DAWN_UNSAFE_BUFFERS(Span<const std::byte>(
        static_cast<const std::byte*>(nullptr), memoryHandleCreateInfoLength));
    cmd.memoryDataUpdateInfo = DAWN_UNSAFE_BUFFERS(
        Span<const std::byte>(static_cast<const std::byte*>(nullptr), memoryDataUpdateInfoLength));

    client->SerializeCommand(
        cmd,
        // Extensions to replace fields skipped by skip_serialize.
        CommandExtension{memoryHandleCreateInfoLength,
                         [&](Span<volatile std::byte> serializeBuffer) {
                             memoryHandle->SerializeCreate(serializeBuffer);
                         }},
        CommandExtension{memoryDataUpdateInfoLength, [&](Span<volatile std::byte> serializeBuffer) {
                             memoryHandle->SerializeDataUpdate(serializeBuffer, 0u, data.size());
                         }});
}

void Queue::APIWriteTexture(const TexelCopyTextureInfo* destination,
                            Span<const std::byte> data,
                            const TexelCopyBufferLayout* dataLayout,
                            const Extent3D* writeSize) {
    if (data.size() >= kWriteXLThreshold) {
        WriteTextureXL(destination, data, dataLayout, writeSize);
        return;
    }

    QueueWriteTextureCmd cmd;
    cmd.queueId = GetWireHandle(GetClient()).id;
    cmd.destination = ToAPI(destination);
    cmd.data = data;
    cmd.dataLayout = ToAPI(dataLayout);
    cmd.writeSize = ToAPI(writeSize);

    GetClient()->SerializeCommand(cmd);
}

void Queue::WriteTextureXL(const TexelCopyTextureInfo* destination,
                           Span<const std::byte> data,
                           const TexelCopyBufferLayout* dataLayout,
                           const Extent3D* writeSize) {
    Client* client = GetClient();

    // Create the MemoryHandle.
    auto memoryHandle = client->GetMemoryTransferService()->CreateMemoryHandle(data.size());
    if (memoryHandle == nullptr) {
        // There was an OOM that we cannot handle in WriteBuffer: trigger a device loss.
        client->Disconnect();
        return;
    }

    // Write the data to the allocated memory.
    Span<std::byte>(memoryHandle->GetData()).CopyFrom(data);

    // Prepare to serialize the handle and the data update command.
    size_t memoryHandleCreateInfoLength = memoryHandle->GetSerializeCreateSize();
    size_t memoryDataUpdateInfoLength = memoryHandle->GetSerializeDataUpdateSize(0u, data.size());

    QueueWriteTextureXlCmd cmd;
    cmd.queueId = GetWireHandle(GetClient()).id;
    cmd.destination = ToAPI(destination);
    cmd.dataSize = data.size();
    cmd.dataLayout = ToAPI(dataLayout);
    cmd.writeSize = ToAPI(writeSize);
    // SAFETY: These Spans are NEVER supposed to be read/serialized, so nullptr is fine.
    // The members are not serialized because skip_serialize, but are Spans so that on
    // the deserialization side we have well-formed members.
    cmd.memoryHandleCreateInfo = DAWN_UNSAFE_BUFFERS(Span<const std::byte>(
        static_cast<const std::byte*>(nullptr), memoryHandleCreateInfoLength));
    cmd.memoryDataUpdateInfo = DAWN_UNSAFE_BUFFERS(
        Span<const std::byte>(static_cast<const std::byte*>(nullptr), memoryDataUpdateInfoLength));

    client->SerializeCommand(
        cmd,
        // Extensions to replace fields skipped by skip_serialize.
        CommandExtension{memoryHandleCreateInfoLength,
                         [&](Span<volatile std::byte> serializeBuffer) {
                             memoryHandle->SerializeCreate(serializeBuffer);
                         }},
        CommandExtension{memoryDataUpdateInfoLength, [&](Span<volatile std::byte> serializeBuffer) {
                             memoryHandle->SerializeDataUpdate(serializeBuffer, 0u, data.size());
                         }});
}

}  // namespace dawn::wire::client
