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

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/439062058): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

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

// Returns either an error buffer or null, depending on mappedAtCreation.
[[nodiscard]] WGPUBuffer ReturnOOMAtClient(Device* device, const WGPUBufferDescriptor* descriptor) {
    if (descriptor->mappedAtCreation) {
        return nullptr;
    }
    WGPUBufferDescriptor errorBufferDescriptor = *descriptor;
    WGPUDawnBufferDescriptorErrorInfoFromWireClient errorInfo = {};
    errorInfo.chain.sType = WGPUSType_DawnBufferDescriptorErrorInfoFromWireClient;
    errorInfo.outOfMemory = static_cast<WGPUBool>(true);
    errorBufferDescriptor.nextInChain = &errorInfo.chain;
    return device->APICreateErrorBuffer(&errorBufferDescriptor);
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
            mResponse.Use([&](auto response) {
                response->status = status;
                response->message = ToString(message);
            });
            return WireResult::Success;
        }

        // If the request was already aborted via the client side, we don't need to actually do
        // anything, so just return success.
        if (!IsPendingRequest(futureID)) {
            return WireResult::Success;
        }

        auto FailRequest = [this](const char* message) -> WireResult {
            mResponse.Use([&](auto response) {
                response->status = static_cast<WGPUMapAsyncStatus>(0);
                response->message = message;
            });
            return WireResult::FatalError;
        };

        mResponse->status = status;
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
        // Move the response while holding the lock so that we avoid racing against the callback
        // firing and the server replying with a response.
        Response response = {};
        mResponse.Use([&](auto res) { response = std::move(*res); });

        if (completionType == EventCompletionType::Shutdown) {
            response.status = WGPUMapAsyncStatus_CallbackCancelled;
            response.message = "A valid external Instance reference no longer exists.";
        }

        auto Callback = [&]() {
            if (mCallback) {
                mCallback(response.status, ToOutputStringView(response.message),
                          mUserdata1.ExtractAsDangling(), mUserdata2.ExtractAsDangling());
            }
        };

        // The request has been cancelled before completion, return that result.
        if (!IsPendingRequest(futureID)) {
            DAWN_ASSERT(response.status != WGPUMapAsyncStatus_Success);
            return Callback();
        }

        // Device destruction/loss implicitly makes the map requests aborted.
        if (!mBuffer->mDevice->IsAlive()) {
            response.status = WGPUMapAsyncStatus_Aborted;
            response.message = "The Device was lost before mapping was resolved.";
        }

        if (response.status == WGPUMapAsyncStatus_Success) {
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

    // The response for the map async callback needs to be protected with a lock since the response
    // can be updated from the server (via a response) or from the client (via an unmap/destroy
    // call).
    struct Response {
        WGPUMapAsyncStatus status;
        std::string message;
    };
    MutexProtected<Response> mResponse;

    // Strong reference to the buffer so that when we call the callback we can pass the buffer.
    Ref<Buffer> mBuffer;
};

// static
WGPUBuffer Buffer::Create(Device* device, const WGPUBufferDescriptor* descriptor) {
    Client* wireClient = device->GetClient();

    bool fakeOOMAtWireClientMap = false;
    for (const auto* chain = descriptor->nextInChain; chain != nullptr; chain = chain->next) {
        switch (chain->sType) {
            case WGPUSType_DawnFakeBufferOOMForTesting: {
                auto oomForTesting =
                    reinterpret_cast<const WGPUDawnFakeBufferOOMForTesting*>(chain);
                fakeOOMAtWireClientMap = (oomForTesting->fakeOOMAtWireClientMap != 0u);
            } break;
            default:
                break;
        }
    }

    bool mappable =
        (descriptor->usage & (WGPUBufferUsage_MapRead | WGPUBufferUsage_MapWrite)) != 0 ||
        wgpu::Bool(descriptor->mappedAtCreation);
    if (mappable &&
        (descriptor->size >= std::numeric_limits<size_t>::max() || fakeOOMAtWireClientMap)) {
        return ReturnOOMAtClient(device, descriptor);
    }

    std::unique_ptr<MemoryTransferService::ReadHandle> readHandle = nullptr;
    std::unique_ptr<MemoryTransferService::WriteHandle> writeHandle = nullptr;

    DeviceCreateBufferCmd cmd;
    cmd.deviceId = device->GetWireHandle(wireClient).id;
    cmd.descriptor = descriptor;
    // Set the pointer lengths, but the pointed-to data itself won't be serialized as usual (due
    // to skip_serialize). Instead, the custom CommandExtensions below fill that memory.
    cmd.readHandleCreateInfoLength = 0;
    cmd.readHandleCreateInfo = nullptr;  // Skipped by skip_serialize.
    cmd.writeHandleCreateInfoLength = 0;
    cmd.writeHandleCreateInfo = nullptr;  // Skipped by skip_serialize.

    size_t readHandleCreateInfoLength = 0;
    size_t writeHandleCreateInfoLength = 0;
    if (mappable) {
        if ((descriptor->usage & WGPUBufferUsage_MapRead) != 0) {
            // Create the read handle on buffer creation.
            readHandle.reset(
                wireClient->GetMemoryTransferService()->CreateReadHandle(descriptor->size));
            if (readHandle == nullptr) {
                return ReturnOOMAtClient(device, descriptor);
            }
            readHandleCreateInfoLength = readHandle->SerializeCreateSize();
            cmd.readHandleCreateInfoLength = readHandleCreateInfoLength;
        }

        if ((descriptor->usage & WGPUBufferUsage_MapWrite) != 0 || descriptor->mappedAtCreation) {
            // Create the write handle on buffer creation.
            writeHandle.reset(
                wireClient->GetMemoryTransferService()->CreateWriteHandle(descriptor->size));
            if (writeHandle == nullptr) {
                return ReturnOOMAtClient(device, descriptor);
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
    buffer->mReadHandle = std::move(readHandle);
    buffer->mWriteHandle = std::move(writeHandle);

    if (descriptor->mappedAtCreation) {
        // If the buffer is mapped at creation, a write handle is created and will be
        // destructed on unmap if the buffer doesn't have MapWrite usage
        // The buffer is mapped right now.
        buffer->mMappedState = MapState::MappedAtCreation;
        buffer->mMappedOffset = 0;
        buffer->mMappedSize = buffer->mSize;
        DAWN_ASSERT(buffer->mWriteHandle != nullptr);
        buffer->mMappedData = buffer->mWriteHandle->GetData();
    }

    cmd.result = buffer->GetWireHandle(wireClient);

    // clang-format off
    // Turning off clang format here because for some reason it does not format the
    // CommandExtensions consistently, making it harder to read.
    wireClient->SerializeCommand(
        cmd,
        // Extensions to replace fields skipped by skip_serialize.
        CommandExtension{readHandleCreateInfoLength,
                         [&](char* readHandleBuffer) {
                             if (buffer->mReadHandle != nullptr) {
                                 // Serialize the ReadHandle into the space after the command.
                                 buffer->mReadHandle->SerializeCreate(readHandleBuffer);
                             }
                         }},
        CommandExtension{writeHandleCreateInfoLength,
                         [&](char* writeHandleBuffer) {
                             if (buffer->mWriteHandle != nullptr) {
                                 // Serialize the WriteHandle into the space after the command.
                                 buffer->mWriteHandle->SerializeCreate(writeHandleBuffer);
                             }
                         }});
    // clang-format on
    return ReturnToAPI(std::move(buffer));
}

// static
WGPUBuffer Buffer::CreateError(Device* device, const WGPUBufferDescriptor* descriptor) {
    if (descriptor->mappedAtCreation) {
        // This codepath isn't used (at the time of this writing). Just return nullptr
        // (pretend there was a mapping OOM), so we don't have to bother mapping the ErrorBuffer
        // (would have to return nullptr anyway if there was actually an OOM).
        std::string error = "mappedAtCreation is not implemented for CreateErrorBuffer";
        device->HandleLogging(WGPULoggingType_Error, WGPUStringView{error.data(), error.size()});
        return nullptr;
    }

    Client* client = device->GetClient();
    Ref<Buffer> buffer = client->Make<Buffer>(device->GetEventManagerHandle(), device, descriptor);

    DeviceCreateErrorBufferCmd cmd;
    cmd.self = ToAPI(device);
    cmd.descriptor = descriptor;
    cmd.result = buffer->GetWireHandle(client);
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
      mDestructWriteHandleOnUnmap(wgpu::Bool(descriptor->mappedAtCreation) &&
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

WGPUFuture Buffer::APIMapAsync(WGPUMapMode mode,
                               size_t offset,
                               size_t size,
                               const WGPUBufferMapCallbackInfo& callbackInfo) {
    Client* client = GetClient();
    auto [futureIDInternal, tracked] =
        GetEventManager().TrackEvent(AcquireRef(new MapAsyncEvent(callbackInfo, this)));
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
    cmd.bufferId = GetWireHandle(client).id;
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
    return SetFutureReady<Buffer::MapAsyncEvent>(eventManager, future.id, status, message,
                                                 readDataUpdateInfoLength, readDataUpdateInfo);
}

void* Buffer::APIGetMappedRange(size_t offset, size_t size) {
    if (!IsMappedForWriting()) {
        if (IsMappedForReading()) {
            std::string error =
                "GetMappedRange: Mapping is read-only. Use GetConstMappedRange instead.";
            mDevice->HandleLogging(WGPULoggingType_Error,
                                   WGPUStringView{error.data(), error.size()});
        }
        return nullptr;
    }
    if (!CheckGetMappedRangeOffsetSize(offset, size)) {
        return nullptr;
    }
    return static_cast<uint8_t*>(mMappedData) + offset;
}

const void* Buffer::APIGetConstMappedRange(size_t offset, size_t size) {
    if (!(IsMappedForWriting() || IsMappedForReading()) ||
        !CheckGetMappedRangeOffsetSize(offset, size)) {
        return nullptr;
    }
    return static_cast<uint8_t*>(mMappedData) + offset;
}

WGPUStatus Buffer::APIWriteMappedRange(size_t offset, void const* data, size_t size) {
    void* range = APIGetMappedRange(offset, size);
    if (range == nullptr) {
        return WGPUStatus_Error;
    }

    memcpy(range, data, size);
    return WGPUStatus_Success;
}

WGPUStatus Buffer::APIReadMappedRange(size_t offset, void* data, size_t size) {
    const void* range = APIGetConstMappedRange(offset, size);
    if (range == nullptr) {
        return WGPUStatus_Error;
    }

    memcpy(data, range, size);
    return WGPUStatus_Success;
}

void Buffer::APIUnmap() {
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
        DAWN_ASSERT(mWriteHandle != nullptr);

        // Get the serialization size of data update writes.
        size_t writeDataUpdateInfoLength =
            mWriteHandle->SizeOfSerializeDataUpdate(mMappedOffset, mMappedSize);

        BufferUpdateMappedDataCmd cmd;
        cmd.bufferId = GetWireHandle(client).id;
        // Set the pointer length, but the pointed-to data itself won't be serialized as usual (due
        // to skip_serialize). Instead, the custom CommandExtension below fills that memory.
        cmd.writeDataUpdateInfoLength = writeDataUpdateInfoLength;
        cmd.writeDataUpdateInfo = nullptr;  // Skipped by skip_serialize.
        cmd.offset = mMappedOffset;
        cmd.size = mMappedSize;

        client->SerializeCommand(
            cmd,
            // Extensions to replace fields skipped by skip_serialize.
            CommandExtension{writeDataUpdateInfoLength, [&](char* writeHandleBuffer) {
                                 // Serialize flush metadata into the space after the command.
                                 // This closes the handle for writing.
                                 mWriteHandle->SerializeDataUpdate(writeHandleBuffer, cmd.offset,
                                                                   cmd.size);
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

void Buffer::APIDestroy() {
    Client* client = GetClient();

    // Remove the current mapping and destroy Read/WriteHandles.
    FreeMappedData();

    BufferDestroyCmd cmd;
    cmd.self = ToAPI(this);
    client->SerializeCommand(cmd);

    SetFutureStatus(WGPUMapAsyncStatus_Aborted,
                    "Buffer was destroyed before mapping was resolved.");
}

WGPUBufferUsage Buffer::APIGetUsage() const {
    return mUsage;
}

uint64_t Buffer::APIGetSize() const {
    return mSize;
}

WGPUBufferMapState Buffer::APIGetMapState() const {
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
