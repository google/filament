// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/wire/client/Buffer.h"

#include <functional>
#include <limits>
#include <string>
#include <utility>

#include "dawn/common/StringViewUtils.h"
#include "dawn/wire/BufferConsumer_impl.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/Device.h"
#include "dawn/wire/client/EventManager.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::wire::client {
namespace {
WGPUBuffer CreateErrorBufferOOMAtClient(Device* device, const WGPUBufferDescriptor* descriptor) {
    if (descriptor->mappedAtCreation) {
        return nullptr;
    }
    WGPUBufferDescriptor errorBufferDescriptor = *descriptor;
    WGPUDawnBufferDescriptorErrorInfoFromWireClient errorInfo = {};
    errorInfo.chain.sType = WGPUSType_DawnBufferDescriptorErrorInfoFromWireClient;
    errorInfo.outOfMemory = true;
    errorBufferDescriptor.nextInChain = &errorInfo.chain;
    return device->CreateErrorBuffer(&errorBufferDescriptor);
}
}  // anonymous namespace

class Buffer::MapAsyncEvent : public TrackedEvent {
  public:
    static constexpr EventType kType = EventType::MapAsync;

    MapAsyncEvent(const WGPUBufferMapCallbackInfo& callbackInfo, Ref<Buffer> buffer)
        : TrackedEvent(callbackInfo.mode),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2),
          mBuffer(buffer) {
        DAWN_ASSERT(mBuffer != nullptr);
    }

    EventType GetType() override { return kType; }

    bool IsPendingRequest(FutureID futureID) {
        return mBuffer->mPendingMapRequest && mBuffer->mPendingMapRequest->futureID == futureID;
    }

    WireResult ReadyHook(FutureID futureID,
                         WGPUMapAsyncStatus status,
                         WGPUStringView message,
                         uint64_t readDataUpdateInfoLength = 0,
                         const uint8_t* readDataUpdateInfo = nullptr) {
        if (status != WGPUMapAsyncStatus_Success) {
            mStatus = status;
            mMessage = ToString(message);
            return WireResult::Success;
        }

        // If the request was already aborted via the client side, we don't need to actually do
        // anything, so just return success.
        if (!IsPendingRequest(futureID)) {
            return WireResult::Success;
        }

        auto FailRequest = [this](const char* message) -> WireResult {
            mStatus = static_cast<WGPUMapAsyncStatus>(0);
            mMessage = message;
            return WireResult::FatalError;
        };

        mStatus = status;
        const auto& pending = mBuffer->mPendingMapRequest.value();
        if (!pending.type) {
            return FailRequest("Invalid map call without a specified mapping type.");
        }
        switch (*pending.type) {
            case MapRequestType::Read: {
                if (readDataUpdateInfoLength > std::numeric_limits<size_t>::max()) {
                    // This is the size of data deserialized from the command stream, which must be
                    // CPU-addressable.
                    return FailRequest("Invalid data size returned from the server.");
                }

                // Update user map data with server returned data
                if (!mBuffer->mReadHandle->DeserializeDataUpdate(
                        readDataUpdateInfo, static_cast<size_t>(readDataUpdateInfoLength),
                        pending.offset, pending.size)) {
                    return FailRequest("Failed to deserialize data returned from the server.");
                }
                mBuffer->mMappedData = const_cast<void*>(mBuffer->mReadHandle->GetData());
                break;
            }
            case MapRequestType::Write: {
                mBuffer->mMappedData = mBuffer->mWriteHandle->GetData();
                break;
            }
        }
        mBuffer->mMappedOffset = pending.offset;
        mBuffer->mMappedSize = pending.size;

        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        if (completionType == EventCompletionType::Shutdown) {
            mStatus = WGPUMapAsyncStatus_InstanceDropped;
            mMessage = "A valid external Instance reference no longer exists.";
        }

        auto Callback = [this]() {
            if (mCallback) {
                mCallback(mStatus, ToOutputStringView(mMessage), mUserdata1.ExtractAsDangling(),
                          mUserdata2.ExtractAsDangling());
            }
        };

        // The request has been cancelled before completion, return that result.
        if (!IsPendingRequest(futureID)) {
            DAWN_ASSERT(mStatus != WGPUMapAsyncStatus_Success);
            return Callback();
        }

        // Device destruction/loss implicitly makes the map requests aborted.
        if (!mBuffer->mDevice->IsAlive()) {
            mStatus = WGPUMapAsyncStatus_Aborted;
            mMessage = "The Device was lost before mapping was resolved.";
        }

        if (mStatus == WGPUMapAsyncStatus_Success) {
            DAWN_ASSERT(mBuffer->mPendingMapRequest && mBuffer->mPendingMapRequest->type);
            switch (*mBuffer->mPendingMapRequest->type) {
                case MapRequestType::Read:
                    mBuffer->mMappedState = MapState::MappedForRead;
                    break;
                case MapRequestType::Write:
                    mBuffer->mMappedState = MapState::MappedForWrite;
                    break;
            }
        }
        mBuffer->mPendingMapRequest = std::nullopt;
        return Callback();
    }

    WGPUBufferMapCallback mCallback;
    raw_ptr<void> mUserdata1;
    raw_ptr<void> mUserdata2;

    WGPUMapAsyncStatus mStatus;
    std::string mMessage;

    // Strong reference to the buffer so that when we call the callback we can pass the buffer.
    Ref<Buffer> mBuffer;
};

// static
WGPUBuffer Buffer::Create(Device* device, const WGPUBufferDescriptor* descriptor) {
    Client* wireClient = device->GetClient();

    bool mappable =
        (descriptor->usage & (WGPUBufferUsage_MapRead | WGPUBufferUsage_MapWrite)) != 0 ||
        descriptor->mappedAtCreation;
    if (mappable && descriptor->size >= std::numeric_limits<size_t>::max()) {
        return CreateErrorBufferOOMAtClient(device, descriptor);
    }

    std::unique_ptr<MemoryTransferService::ReadHandle> readHandle = nullptr;
    std::unique_ptr<MemoryTransferService::WriteHandle> writeHandle = nullptr;

    DeviceCreateBufferCmd cmd;
    cmd.deviceId = device->GetWireId();
    cmd.descriptor = descriptor;
    cmd.readHandleCreateInfoLength = 0;
    cmd.readHandleCreateInfo = nullptr;
    cmd.writeHandleCreateInfoLength = 0;
    cmd.writeHandleCreateInfo = nullptr;

    size_t readHandleCreateInfoLength = 0;
    size_t writeHandleCreateInfoLength = 0;
    if (mappable) {
        if ((descriptor->usage & WGPUBufferUsage_MapRead) != 0) {
            // Create the read handle on buffer creation.
            readHandle.reset(
                wireClient->GetMemoryTransferService()->CreateReadHandle(descriptor->size));
            if (readHandle == nullptr) {
                return CreateErrorBufferOOMAtClient(device, descriptor);
            }
            readHandleCreateInfoLength = readHandle->SerializeCreateSize();
            cmd.readHandleCreateInfoLength = readHandleCreateInfoLength;
        }

        if ((descriptor->usage & WGPUBufferUsage_MapWrite) != 0 || descriptor->mappedAtCreation) {
            // Create the write handle on buffer creation.
            writeHandle.reset(
                wireClient->GetMemoryTransferService()->CreateWriteHandle(descriptor->size));
            if (writeHandle == nullptr) {
                return CreateErrorBufferOOMAtClient(device, descriptor);
            }
            writeHandleCreateInfoLength = writeHandle->SerializeCreateSize();
            cmd.writeHandleCreateInfoLength = writeHandleCreateInfoLength;
        }
    }

    // Create the buffer and send the creation command.
    // This must happen after any potential error buffer creation
    // as server expects allocating ids to be monotonically increasing
    Ref<Buffer> buffer =
        wireClient->Make<Buffer>(device->GetEventManagerHandle(), device, descriptor);

    if (descriptor->mappedAtCreation) {
        // If the buffer is mapped at creation, a write handle is created and will be
        // destructed on unmap if the buffer doesn't have MapWrite usage
        // The buffer is mapped right now.
        buffer->mMappedState = MapState::MappedAtCreation;
        buffer->mMappedOffset = 0;
        buffer->mMappedSize = buffer->mSize;
        DAWN_ASSERT(writeHandle != nullptr);
        buffer->mMappedData = writeHandle->GetData();
    }

    cmd.result = buffer->GetWireHandle();

    // clang-format off
    // Turning off clang format here because for some reason it does not format the
    // CommandExtensions consistently, making it harder to read.
    wireClient->SerializeCommand(
        cmd,
        CommandExtension{readHandleCreateInfoLength,
                         [&](char* readHandleBuffer) {
                             if (readHandle != nullptr) {
                                 // Serialize the ReadHandle into the space after the command.
                                 readHandle->SerializeCreate(readHandleBuffer);
                                 buffer->mReadHandle = std::move(readHandle);
                             }
                         }},
        CommandExtension{writeHandleCreateInfoLength,
                         [&](char* writeHandleBuffer) {
                             if (writeHandle != nullptr) {
                                 // Serialize the WriteHandle into the space after the command.
                                 writeHandle->SerializeCreate(writeHandleBuffer);
                                 buffer->mWriteHandle = std::move(writeHandle);
                             }
                         }});
    // clang-format on
    return ReturnToAPI(std::move(buffer));
}

// static
WGPUBuffer Buffer::CreateError(Device* device, const WGPUBufferDescriptor* descriptor) {
    Client* client = device->GetClient();
    Ref<Buffer> buffer = client->Make<Buffer>(device->GetEventManagerHandle(), device, descriptor);

    DeviceCreateErrorBufferCmd cmd;
    cmd.self = ToAPI(device);
    cmd.descriptor = descriptor;
    cmd.result = buffer->GetWireHandle();
    client->SerializeCommand(cmd);

    return ReturnToAPI(std::move(buffer));
}

Buffer::Buffer(const ObjectBaseParams& params,
               const ObjectHandle& eventManagerHandle,
               Device* device,
               const WGPUBufferDescriptor* descriptor)
    : RefCountedWithExternalCount<ObjectWithEventsBase>(params, eventManagerHandle),
      mSize(descriptor->size),
      mUsage(static_cast<WGPUBufferUsage>(descriptor->usage)),
      // This flag is for the write handle created by mappedAtCreation
      // instead of MapWrite usage. We don't have such a case for read handle.
      mDestructWriteHandleOnUnmap(descriptor->mappedAtCreation &&
                                  ((descriptor->usage & WGPUBufferUsage_MapWrite) == 0)),
      mDevice(device) {}

void Buffer::DeleteThis() {
    FreeMappedData();
    ObjectWithEventsBase::DeleteThis();
}

void Buffer::WillDropLastExternalRef() {
    SetFutureStatus(WGPUMapAsyncStatus_Aborted,
                    "Buffer was destroyed before mapping was resolved.");
}

ObjectType Buffer::GetObjectType() const {
    return ObjectType::Buffer;
}

void Buffer::SetFutureStatus(WGPUMapAsyncStatus status, std::string_view message) {
    if (!mPendingMapRequest) {
        return;
    }

    FutureID futureID = mPendingMapRequest->futureID;
    mPendingMapRequest = std::nullopt;

    DAWN_CHECK(GetEventManager().SetFutureReady<MapAsyncEvent>(
                   futureID, status, ToOutputStringView(message)) == WireResult::Success);
}

WGPUFuture Buffer::MapAsync(WGPUMapMode mode,
                            size_t offset,
                            size_t size,
                            const WGPUBufferMapCallbackInfo& callbackInfo) {
    Client* client = GetClient();
    auto [futureIDInternal, tracked] =
        GetEventManager().TrackEvent(std::make_unique<MapAsyncEvent>(callbackInfo, this));
    if (!tracked) {
        return {futureIDInternal};
    }

    if (mPendingMapRequest) {
        [[maybe_unused]] auto id = GetEventManager().SetFutureReady<MapAsyncEvent>(
            futureIDInternal, WGPUMapAsyncStatus_Error,
            ToOutputStringView("Buffer already has an outstanding map pending."));
        return {futureIDInternal};
    }

    // Handle the defaulting of size required by WebGPU.
    if ((size == WGPU_WHOLE_MAP_SIZE) && (offset <= mSize)) {
        size = mSize - offset;
    }

    // Set up the request structure that will hold information while this mapping is in flight.
    std::optional<MapRequestType> mapMode;
    if (mode & WGPUMapMode_Read) {
        mapMode = MapRequestType::Read;
    } else if (mode & WGPUMapMode_Write) {
        mapMode = MapRequestType::Write;
    }

    mPendingMapRequest = {futureIDInternal, offset, size, mapMode};

    // Serialize the command to send to the server.
    BufferMapAsyncCmd cmd;
    cmd.bufferId = GetWireId();
    cmd.eventManagerHandle = GetEventManagerHandle();
    cmd.future = {futureIDInternal};
    cmd.mode = mode;
    cmd.offset = offset;
    cmd.size = size;

    client->SerializeCommand(cmd);
    return {futureIDInternal};
}

WireResult Client::DoBufferMapAsyncCallback(ObjectHandle eventManager,
                                            WGPUFuture future,
                                            WGPUMapAsyncStatus status,
                                            WGPUStringView message,
                                            uint64_t readDataUpdateInfoLength,
                                            const uint8_t* readDataUpdateInfo) {
    return GetEventManager(eventManager)
        .SetFutureReady<Buffer::MapAsyncEvent>(future.id, status, message, readDataUpdateInfoLength,
                                               readDataUpdateInfo);
}

void* Buffer::GetMappedRange(size_t offset, size_t size) {
    if (!IsMappedForWriting() || !CheckGetMappedRangeOffsetSize(offset, size)) {
        return nullptr;
    }
    return static_cast<uint8_t*>(mMappedData) + offset;
}

const void* Buffer::GetConstMappedRange(size_t offset, size_t size) {
    if (!(IsMappedForWriting() || IsMappedForReading()) ||
        !CheckGetMappedRangeOffsetSize(offset, size)) {
        return nullptr;
    }
    return static_cast<uint8_t*>(mMappedData) + offset;
}

void Buffer::Unmap() {
    // Invalidate the local pointer, and cancel all other in-flight requests that would
    // turn into errors anyway (you can't double map). This prevents race when the following
    // happens, where the application code would have unmapped a buffer but still receive a
    // callback:
    //   - Client -> Server: MapRequest1, Unmap, MapRequest2
    //   - Server -> Client: Result of MapRequest1
    //   - Unmap locally on the client
    //   - Server -> Client: Result of MapRequest2
    Client* client = GetClient();

    if (IsMappedForWriting()) {
        // Writes need to be flushed before Unmap is sent. Unmap calls all associated
        // in-flight callbacks which may read the updated data.

        // Get the serialization size of data update writes.
        size_t writeDataUpdateInfoLength =
            mWriteHandle->SizeOfSerializeDataUpdate(mMappedOffset, mMappedSize);

        BufferUpdateMappedDataCmd cmd;
        cmd.bufferId = GetWireId();
        cmd.writeDataUpdateInfoLength = writeDataUpdateInfoLength;
        cmd.writeDataUpdateInfo = nullptr;
        cmd.offset = mMappedOffset;
        cmd.size = mMappedSize;

        client->SerializeCommand(
            cmd, CommandExtension{writeDataUpdateInfoLength, [&](char* writeHandleBuffer) {
                                      // Serialize flush metadata into the space after the command.
                                      // This closes the handle for writing.
                                      mWriteHandle->SerializeDataUpdate(writeHandleBuffer,
                                                                        cmd.offset, cmd.size);
                                  }});

        // If mDestructWriteHandleOnUnmap is true, that means the write handle is merely
        // for mappedAtCreation usage. It is destroyed on unmap after flush to server
        // instead of at buffer destruction.
        if (mDestructWriteHandleOnUnmap) {
            mMappedData = nullptr;
            mWriteHandle = nullptr;
            if (mReadHandle) {
                // If it's both mappedAtCreation and MapRead we need to reset
                // mData to readHandle's GetData(). This could be changed to
                // merging read/write handle in future
                mMappedData = const_cast<void*>(mReadHandle->GetData());
            }
        }
    }

    // Free map access tokens
    mMappedState = MapState::Unmapped;
    mMappedOffset = 0;
    mMappedSize = 0;

    BufferUnmapCmd cmd;
    cmd.self = ToAPI(this);
    client->SerializeCommand(cmd);

    SetFutureStatus(WGPUMapAsyncStatus_Aborted, "Buffer was unmapped before mapping was resolved.");
}

void Buffer::Destroy() {
    Client* client = GetClient();

    // Remove the current mapping and destroy Read/WriteHandles.
    FreeMappedData();

    BufferDestroyCmd cmd;
    cmd.self = ToAPI(this);
    client->SerializeCommand(cmd);

    SetFutureStatus(WGPUMapAsyncStatus_Aborted,
                    "Buffer was destroyed before mapping was resolved.");
}

WGPUBufferUsage Buffer::GetUsage() const {
    return mUsage;
}

uint64_t Buffer::GetSize() const {
    return mSize;
}

WGPUBufferMapState Buffer::GetMapState() const {
    switch (mMappedState) {
        case MapState::MappedForRead:
        case MapState::MappedForWrite:
        case MapState::MappedAtCreation:
            return WGPUBufferMapState_Mapped;
        case MapState::Unmapped:
            if (mPendingMapRequest) {
                return WGPUBufferMapState_Pending;
            } else {
                return WGPUBufferMapState_Unmapped;
            }
    }
    DAWN_UNREACHABLE();
}

bool Buffer::IsMappedForReading() const {
    return mMappedState == MapState::MappedForRead;
}

bool Buffer::IsMappedForWriting() const {
    return mMappedState == MapState::MappedForWrite || mMappedState == MapState::MappedAtCreation;
}

bool Buffer::CheckGetMappedRangeOffsetSize(size_t offset, size_t size) const {
    if (offset % 8 != 0 || offset < mMappedOffset || offset > mSize) {
        return false;
    }

    size_t rangeSize = size == WGPU_WHOLE_MAP_SIZE ? mSize - offset : size;

    if (rangeSize % 4 != 0 || rangeSize > mMappedSize) {
        return false;
    }

    size_t offsetInMappedRange = offset - mMappedOffset;
    return offsetInMappedRange <= mMappedSize - rangeSize;
}

void Buffer::FreeMappedData() {
#if defined(DAWN_ENABLE_ASSERTS)
    // When in "debug" mode, 0xCA-out the mapped data when we free it so that in we can detect
    // use-after-free of the mapped data. This is particularly useful for WebGPU test about the
    // interaction of mapping and GC.
    if (mMappedData) {
        memset(static_cast<uint8_t*>(mMappedData) + mMappedOffset, 0xCA, mMappedSize);
    }
#endif  // defined(DAWN_ENABLE_ASSERTS)

    mMappedOffset = 0;
    mMappedSize = 0;
    mMappedData = nullptr;
    mReadHandle = nullptr;
    mWriteHandle = nullptr;
    mMappedState = MapState::Unmapped;
}

}  // namespace dawn::wire::client
