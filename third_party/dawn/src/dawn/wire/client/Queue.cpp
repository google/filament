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

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/439062058): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

#include "dawn/wire/client/Queue.h"

#include <memory>
#include <string>
#include <utility>

#include "dawn/common/Atomic.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/wire/BufferConsumer_impl.h"
#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/EventManager.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::wire::client {
namespace {

// Buffer and Texture uploads larger than 4Mb use a different path optimized for larger transfers.
const uint64_t kWriteXLThreshold = 1024 * 1024 * 4;

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

void Queue::APISubmit(size_t commandCount, const WGPUCommandBuffer* commands) {
    mLastSubmitIndex++;

    // Send the submit command
    QueueSubmitCmd cmd;
    cmd.self = ToAPI(this);
    cmd.commandCount = commandCount;
    cmd.commands = commands;
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

WireResult Client::DoQueueWorkDoneCallback(ObjectHandle eventManager,
                                           WGPUFuture future,
                                           WGPUQueueWorkDoneStatus status,
                                           WGPUStringView message) {
    return SetFutureReady<WorkDoneEvent>(eventManager, future.id, status, message);
}

WGPUFuture Queue::APIOnSubmittedWorkDone(const WGPUQueueWorkDoneCallbackInfo& callbackInfo) {
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
    cmd.eventManagerHandle = GetEventManagerHandle();
    cmd.future = {futureIDInternal};

    client->SerializeCommand(cmd);
    return {futureIDInternal};
}

void Queue::APIWriteBuffer(WGPUBuffer cBuffer,
                           uint64_t bufferOffset,
                           const void* data,
                           size_t size) {
    if (size >= kWriteXLThreshold) {
        WriteBufferXL(cBuffer, bufferOffset, data, size);
        return;
    }

    Buffer* buffer = FromAPI(cBuffer);

    QueueWriteBufferCmd cmd;
    cmd.queueId = GetWireHandle(GetClient()).id;
    cmd.bufferId = buffer->GetWireHandle(GetClient()).id;
    cmd.bufferOffset = bufferOffset;
    cmd.data = static_cast<const uint8_t*>(data);
    cmd.size = size;

    GetClient()->SerializeCommand(cmd);
}

void Queue::WriteBufferXL(WGPUBuffer cBuffer,
                          uint64_t bufferOffset,
                          const void* data,
                          size_t size) {
    Buffer* buffer = FromAPI(cBuffer);
    Client* client = GetClient();

    // Create write handle and prepare to serialize command.
    size_t writeHandleCreateInfoLength = 0;
    std::unique_ptr<MemoryTransferService::WriteHandle> writeHandle(
        client->GetMemoryTransferService()->CreateWriteHandle(size));
    if (writeHandle == nullptr) {
        // Trigger a device loss.
        client->Disconnect();
        return;
    }
    writeHandleCreateInfoLength = writeHandle->SerializeCreateSize();

    // Write the data to the allocated memory.
    memcpy(writeHandle->GetData(), data, size);

    // Prepare to serialize data update command.
    size_t writeDataUpdateInfoLength = writeHandle->SizeOfSerializeDataUpdate(0u, size);

    QueueWriteBufferXlCmd cmd;
    cmd.queueId = GetWireHandle(client).id;
    cmd.bufferId = buffer->GetWireHandle(client).id;
    cmd.bufferOffset = bufferOffset;
    cmd.size = size;
    // Set the pointer lengths, but the pointed-to data itself won't be serialized as usual (due
    // to skip_serialize). Instead, the custom CommandExtensions below fill that memory. [*]
    cmd.writeHandleCreateInfoLength = writeHandleCreateInfoLength;
    cmd.writeHandleCreateInfo = nullptr;  // Skipped by skip_serialize.
    cmd.writeDataUpdateInfoLength = writeDataUpdateInfoLength;
    cmd.writeDataUpdateInfo = nullptr;  // Skipped by skip_serialize.

    client->SerializeCommand(
        cmd,
        // Extensions to replace fields skipped by skip_serialize.
        CommandExtension{
            writeHandleCreateInfoLength,
            [&](char* writeHandleBuffer) { writeHandle->SerializeCreate(writeHandleBuffer); }},
        CommandExtension{writeDataUpdateInfoLength, [&](char* writeHandleBuffer) {
                             writeHandle->SerializeDataUpdate(writeHandleBuffer, 0u, cmd.size);
                         }});
}

void Queue::APIWriteTexture(const WGPUTexelCopyTextureInfo* destination,
                            const void* data,
                            size_t dataSize,
                            const WGPUTexelCopyBufferLayout* dataLayout,
                            const WGPUExtent3D* writeSize) {
    if (dataSize >= kWriteXLThreshold) {
        WriteTextureXL(destination, data, dataSize, dataLayout, writeSize);
        return;
    }

    QueueWriteTextureCmd cmd;
    cmd.queueId = GetWireHandle(GetClient()).id;
    cmd.destination = destination;
    cmd.data = static_cast<const uint8_t*>(data);
    cmd.dataSize = dataSize;
    cmd.dataLayout = dataLayout;
    cmd.writeSize = writeSize;

    GetClient()->SerializeCommand(cmd);
}

void Queue::WriteTextureXL(const WGPUTexelCopyTextureInfo* destination,
                           const void* data,
                           size_t dataSize,
                           const WGPUTexelCopyBufferLayout* dataLayout,
                           const WGPUExtent3D* writeSize) {
    Client* client = GetClient();

    // Create write handle and prepare to serialize command.
    size_t writeHandleCreateInfoLength = 0;
    std::unique_ptr<MemoryTransferService::WriteHandle> writeHandle(
        client->GetMemoryTransferService()->CreateWriteHandle(dataSize));
    if (writeHandle == nullptr) {
        // Trigger a device loss.
        client->Disconnect();
        return;
    }
    writeHandleCreateInfoLength = writeHandle->SerializeCreateSize();

    // Write the data to the allocated memory.
    memcpy(writeHandle->GetData(), data, dataSize);

    // Prepare to serialize data update command.
    size_t writeDataUpdateInfoLength = writeHandle->SizeOfSerializeDataUpdate(0u, dataSize);

    QueueWriteTextureXlCmd cmd;
    cmd.queueId = GetWireHandle(GetClient()).id;
    cmd.destination = destination;
    cmd.dataSize = dataSize;
    cmd.dataLayout = dataLayout;
    cmd.writeSize = writeSize;
    // Set the pointer lengths, but the pointed-to data itself won't be serialized as usual (due
    // to skip_serialize). Instead, the custom CommandExtensions below fill that memory. [*]
    cmd.writeHandleCreateInfoLength = writeHandleCreateInfoLength;
    cmd.writeHandleCreateInfo = nullptr;  // Skipped by skip_serialize.
    cmd.writeDataUpdateInfoLength = writeDataUpdateInfoLength;
    cmd.writeDataUpdateInfo = nullptr;  // Skipped by skip_serialize.

    client->SerializeCommand(
        cmd,
        // Extensions to replace fields skipped by skip_serialize.
        CommandExtension{
            writeHandleCreateInfoLength,
            [&](char* writeHandleBuffer) { writeHandle->SerializeCreate(writeHandleBuffer); }},
        CommandExtension{writeDataUpdateInfoLength, [&](char* writeHandleBuffer) {
                             writeHandle->SerializeDataUpdate(writeHandleBuffer, 0u, cmd.dataSize);
                         }});
}

}  // namespace dawn::wire::client
