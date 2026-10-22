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

#include "src/dawn/wire/client/Buffer.h"

#include <functional>
#include <limits>
#include <string>
#include <utility>

#include "dawn/wire/WireCmd_autogen.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/wire/client/Client.h"
#include "src/dawn/wire/client/Device.h"
#include "src/dawn/wire/client/EventManager.h"
#include "src/utils/compiler.h"
#include "src/utils/numeric.h"

namespace dawn::wire::client {
namespace {

// Returns either an error buffer or null, depending on mappedAtCreation.
[[nodiscard]] Buffer* ReturnOOMAtClient(Device* device, const BufferDescriptor* descriptor) {
    if (descriptor->mappedAtCreation) {
        return nullptr;
    }
    BufferDescriptor errorBufferDescriptor = *descriptor;
    DawnBufferDescriptorErrorInfoFromWireClient errorInfo = {};
    errorInfo.outOfMemory = true;
    errorBufferDescriptor.nextInChain = &errorInfo;
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

    WireResult ReadyHook(FutureID futureID,
                         WGPUMapAsyncStatus status,
                         WGPUStringView message,
                         Span<const std::byte> readDataUpdateInfo = {}) {
        auto FailRequest = [this](const char* message) -> WireResult {
            mStatus = static_cast<WGPUMapAsyncStatus>(0);
            mMessage = message;
            return WireResult::FatalError;
        };

        return mBuffer->mState.Use([&](auto state) {
            // If we got a failure status, we always override the last one. Note that this can lead
            // to some indeterministic results since the server and the user could race different
            // non-success results. That said, given that it's a non-success result, the race only
            // determines what the user sees as the error message and status.
            if (status != WGPUMapAsyncStatus_Success) {
                mStatus = status;
                mMessage = ToString(message);
                return WireResult::Success;
            }

            // If the request was already aborted via the client side, we don't need to do anything,
            // so just return success.
            if (!state->PendingRequestIs(futureID)) {
                return WireResult::Success;
            }

            const auto& pending = *state->pendingMapRequest;
            if (!pending.type) {
                return FailRequest("Invalid map call without a specified mapping type.");
            }
            switch (*pending.type) {
                case MapRequestType::Read: {
                    // Update user map data with server returned data.
                    if (!state->memoryHandle->DeserializeDataUpdate(readDataUpdateInfo,
                                                                    pending.offset, pending.size)) {
                        return FailRequest("Failed to deserialize data returned from the server.");
                    }
                    break;
                }
                case MapRequestType::Write: {
                    break;
                }
            }
            state->mappedData = state->memoryHandle->GetData();
            state->mappedOffset = pending.offset;
            state->mappedSize = pending.size;

            return WireResult::Success;
        });
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        auto Callback = [&]() {
            if (mCallback) {
                mCallback(mStatus, ToOutputStringView(mMessage), mUserdata1.ExtractAsDangling(),
                          mUserdata2.ExtractAsDangling());
            }
        };

        return mBuffer->mState.Use([&](auto state) {
            if (completionType == EventCompletionType::Shutdown) {
                mStatus = WGPUMapAsyncStatus_CallbackCancelled;
                mMessage = "A valid external Instance reference no longer exists.";
            }

            // The request has been cancelled before completion, return that result.
            if (!state->PendingRequestIs(futureID)) {
                DAWN_ASSERT(mStatus != WGPUMapAsyncStatus_Success);
                return Callback();
            }

            // Device destruction/loss implicitly makes the map requests aborted.
            if (mBuffer->mDevice->IsDestroyed()) {
                mStatus = WGPUMapAsyncStatus_Aborted;
                mMessage = "The Device was lost before mapping was resolved.";
            }

            if (mStatus == WGPUMapAsyncStatus_Success) {
                DAWN_ASSERT(state->pendingMapRequest && state->pendingMapRequest->type);
                switch (*state->pendingMapRequest->type) {
                    case MapRequestType::Read:
                        state->mappedState = MapState::MappedForRead;
                        break;
                    case MapRequestType::Write:
                        state->mappedState = MapState::MappedForWrite;
                        break;
                }
            }
            state->pendingMapRequest = std::nullopt;
            return Callback();
        });
    }

    WGPUBufferMapCallback mCallback;
    raw_ptr<void> mUserdata1;
    raw_ptr<void> mUserdata2;

    // The response for the map async callback are implicitly protected by the mutex protecting the
    // map state in the Buffer.
    WGPUMapAsyncStatus mStatus = WGPUMapAsyncStatus_Success;
    std::string mMessage;

    // Strong reference to the buffer for synchronization purposes.
    Ref<Buffer> mBuffer;
};

// static
Buffer* Buffer::Create(Device* device, const BufferDescriptor* descriptor) {
    Client* wireClient = device->GetClient();

    bool fakeOOMAtWireClientMap = false;
    for (const auto* chain = descriptor->nextInChain; chain != nullptr;
         chain = chain->nextInChain) {
        switch (chain->sType) {
            case wgpu::SType::DawnFakeBufferOOMForTesting: {
                auto oomForTesting =
                    reinterpret_cast<const wgpu::DawnFakeBufferOOMForTesting*>(chain);
                fakeOOMAtWireClientMap = oomForTesting->fakeOOMAtWireClientMap;
            } break;
            default:
                break;
        }
    }

    // Handle client-side error cases.
    bool mappableForWrite =
        (descriptor->usage & wgpu::BufferUsage::MapWrite) != wgpu::BufferUsage::None ||
        descriptor->mappedAtCreation;
    bool mappable = mappableForWrite ||
                    (descriptor->usage & wgpu::BufferUsage::MapRead) != wgpu::BufferUsage::None;
    if (mappable &&
        (descriptor->size >= std::numeric_limits<size_t>::max() || fakeOOMAtWireClientMap)) {
        return ReturnOOMAtClient(device, descriptor);
    }

    // Create the MemoryHandle for mappable buffers.
    std::shared_ptr<MemoryTransferService::MemoryHandle> memoryHandle = nullptr;
    size_t memoryHandleCreateInfoLength = 0;
    if (mappable) {
        memoryHandle = wireClient->GetMemoryTransferService()->CreateMemoryHandle(
            checked_cast<size_t>(descriptor->size));
        if (memoryHandle == nullptr) {
            return ReturnOOMAtClient(device, descriptor);
        }
        memoryHandleCreateInfoLength = memoryHandle->GetSerializeCreateSize();

        // Prevent uninitialized memory from being visible via GetMappedRange().
        if (mappableForWrite) {
            std::ranges::fill(memoryHandle->GetData(), std::byte(0u));
        }
    }

    // Create the buffer and send the creation command.
    // This must happen after any potential error buffer creation as the server expects allocating
    // ids to be monotonically increasing
    Ref<Buffer> buffer = wireClient->Make<Buffer>(device->GetInstance(), device, descriptor);

    DeviceCreateBufferCmd cmd;
    cmd.deviceId = device->GetWireHandle(wireClient).id;
    cmd.descriptor = ToAPI(descriptor);
    // SAFETY: This Span is NEVER supposed to be read/serialized, so nullptr is fine.
    // The member is not serialized because skip_serialize, but is a Span so that on
    // the deserialization side we have a well-formed member.
    // TODO(https://crbug.com/542275488): Clean these up if when we update command extension
    // serialization to serialize into this span directly.
    cmd.memoryHandleCreateInfo = DAWN_UNSAFE_BUFFERS(Span<const std::byte>(
        static_cast<const std::byte*>(nullptr), memoryHandleCreateInfoLength));
    cmd.result = buffer->GetWireHandle(wireClient);

    buffer->mState.Use([&](auto state) {
        state->memoryHandle = memoryHandle;

        if (descriptor->mappedAtCreation) {
            // If the buffer is mapped at creation, a memory handle is created and will be
            // destroyed on unmap if the buffer doesn't have Map* usage.
            // The buffer is mapped right now.
            state->mappedState = MapState::MappedAtCreation;
            state->mappedOffset = 0;
            state->mappedSize = checked_cast<size_t>(buffer->mSize);

            DAWN_ASSERT(state->memoryHandle != nullptr);
            state->mappedData = state->memoryHandle->GetData();
        }
    });

    wireClient->SerializeCommand(
        cmd,
        // Extensions to replace fields skipped by skip_serialize.
        CommandExtension{memoryHandleCreateInfoLength,
                         [&](Span<volatile std::byte> serializeBuffer) {
                             if (memoryHandle != nullptr) {
                                 // Serialize the MemoryHandle into the space after the command.
                                 memoryHandle->SerializeCreate(std::span(serializeBuffer));
                             }
                         }});

    return ReturnToAPI2(std::move(buffer));
}

// static
Buffer* Buffer::CreateError(Device* device, const BufferDescriptor* descriptor) {
    if (descriptor->mappedAtCreation) {
        // This codepath isn't used (at the time of this writing). Just return nullptr
        // (pretend there was a mapping OOM), so we don't have to bother mapping the ErrorBuffer
        // (would have to return nullptr anyway if there was actually an OOM).
        std::string error = "mappedAtCreation is not implemented for CreateErrorBuffer";
        device->HandleLogging(WGPULoggingType_Error, WGPUStringView{error.data(), error.size()});
        return nullptr;
    }

    Client* client = device->GetClient();
    Ref<Buffer> buffer = client->Make<Buffer>(device->GetInstance(), device, descriptor);

    DeviceCreateErrorBufferCmd cmd;
    cmd.self = ToAPI(device);
    cmd.descriptor = ToAPI(descriptor);
    cmd.result = buffer->GetWireHandle(client);
    client->SerializeCommand(cmd);

    return ReturnToAPI2(std::move(buffer));
}

Buffer::Buffer(const ObjectBaseParams& params,
               Ref<Instance> instance,
               Device* device,
               const BufferDescriptor* descriptor)
    : RefCountedWithExternalCount<ObjectWithEventsBase>(params, std::move(instance)),
      mSize(descriptor->size),
      mUsage(descriptor->usage),
      mDestructMemoryHandleOnUnmap(
          descriptor->mappedAtCreation &&
          ((descriptor->usage & (wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::MapRead)) ==
           wgpu::BufferUsage::None)),
      mDevice(device) {}

void Buffer::DeleteThis() {
    mState.Use([&](auto state) { FreeMappedData(state); });
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
    auto futureID = mState.Use([&](auto state) -> std::optional<FutureID> {
        if (!state->pendingMapRequest) {
            return std::nullopt;
        }

        FutureID result = state->pendingMapRequest->futureID;
        state->pendingMapRequest = std::nullopt;
        return result;
    });

    if (!futureID) {
        return;
    }
    auto wireStatus = GetEventManager().SetFutureReady<MapAsyncEvent>(*futureID, status,
                                                                      ToOutputStringView(message));
    DAWN_CHECK(wireStatus == WireResult::Success);
}

Future Buffer::APIMapAsync(wgpu::MapMode mode,
                           size_t offset,
                           size_t size,
                           const WGPUBufferMapCallbackInfo& callbackInfo) {
    Client* client = GetClient();
    auto [futureIDInternal, tracked] =
        GetEventManager().TrackEvent(AcquireRef(new MapAsyncEvent(callbackInfo, this)));
    if (!tracked) {
        return {futureIDInternal};
    }

    bool success = mState.Use([&](auto state) {
        if (state->pendingMapRequest) {
            return false;
        }

        // Handle the defaulting of size required by WebGPU.
        if (size == WGPU_WHOLE_MAP_SIZE) {
            if (offset <= mSize) {
                size = checked_cast<size_t>(mSize - offset);
            } else {
                // Send any valid size to the server as the mapping will be rejected anyway.
                size = 0;
            }
        }

        // Set up the request structure that will hold information while this mapping is in flight.
        std::optional<MapRequestType> mapMode;
        if (mode & wgpu::MapMode::Read) {
            mapMode = MapRequestType::Read;
        } else if (mode & wgpu::MapMode::Write) {
            mapMode = MapRequestType::Write;
        }
        state->pendingMapRequest = {futureIDInternal, offset, size, mapMode};
        return true;
    });
    if (!success) {
        [[maybe_unused]] auto id = GetEventManager().SetFutureReady<MapAsyncEvent>(
            futureIDInternal, WGPUMapAsyncStatus_Error,
            ToOutputStringView("Buffer already has an outstanding map pending."));
        return {futureIDInternal};
    }

    // Serialize the command to send to the server.
    BufferMapAsyncCmd cmd;
    cmd.bufferId = GetWireHandle(client).id;
    cmd.instanceId = GetInstance()->GetWireHandle(client).id;
    cmd.future = {futureIDInternal};
    cmd.mode = ToAPI(mode);
    cmd.offset = offset;
    cmd.size = size;

    client->SerializeCommand(cmd);
    return {futureIDInternal};
}

WireResult Client::DoBufferMapAsyncCallback(ObjectId instanceId,
                                            WGPUFuture future,
                                            WGPUMapAsyncStatus status,
                                            WGPUStringView message,
                                            Span<const std::byte> readDataUpdateInfo) {
    return SetFutureReady<Buffer::MapAsyncEvent>(instanceId, future.id, status, message,
                                                 readDataUpdateInfo);
}

void* Buffer::APIGetMappedRange(size_t offset, size_t size) {
    return mState.Use([&](auto state) -> void* {
        if (!state->IsMappedForWriting()) {
            if (state->IsMappedForReading()) {
                std::string error =
                    "GetMappedRange: Mapping is read-only. Use GetConstMappedRange instead.";
                mDevice->HandleLogging(WGPULoggingType_Error,
                                       WGPUStringView{error.data(), error.size()});
            }
            return nullptr;
        }
        return static_cast<void*>(state->GetMappedRange(offset, size).data());
    });
}

const void* Buffer::APIGetConstMappedRange(size_t offset, size_t size) {
    return mState.Use([&](auto state) -> const void* {
        if (!(state->IsMappedForWriting() || state->IsMappedForReading())) {
            return nullptr;
        }
        return static_cast<const void*>(state->GetMappedRange(offset, size).data());
    });
}

wgpu::Status Buffer::APIWriteMappedRange(size_t offset, Span<const std::byte> data) {
    return mState.Use([&](auto state) {
        auto dst = state->GetMappedRange(offset, data.size());
        if (dst.data() == nullptr) {
            return wgpu::Status::Error;
        }
        dst.CopyFrom(data);
        return wgpu::Status::Success;
    });
}

wgpu::Status Buffer::APIReadMappedRange(size_t offset, Span<std::byte> data) {
    return mState.Use([&](auto state) {
        auto src = state->GetMappedRange(offset, data.size());
        if (src.data() == nullptr) {
            return wgpu::Status::Error;
        }
        data.CopyFrom(src);
        return wgpu::Status::Success;
    });
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

    BufferUpdateMappedDataCmd cmd{};
    std::shared_ptr<MemoryTransferService::MemoryHandle> memoryHandle;

    mState.Use([&](auto state) {
        if (state->IsMappedForWriting()) {
            // Writes need to be flushed before Unmap is sent. Unmap calls all associated
            // in-flight callbacks which may read the updated data.
            DAWN_ASSERT(state->memoryHandle != nullptr);

            cmd.bufferId = GetWireHandle(client).id;
            cmd.offset = state->mappedOffset;
            cmd.size = state->mappedSize;

            memoryHandle = state->memoryHandle;

            // If mDestructMemoryHandleOnUnmap is true, that means the memory handle is merely
            // for mappedAtCreation usage. It is destroyed on unmap after flush to server
            // instead of at buffer destruction.
            if (mDestructMemoryHandleOnUnmap) {
                state->mappedData = {};
                state->memoryHandle = nullptr;
            }
        }

        // Free map access tokens
        state->mappedState = MapState::Unmapped;
        state->mappedOffset = 0;
        state->mappedSize = 0;
    });

    if (memoryHandle) {
        size_t memoryDataUpdateInfoLength =
            memoryHandle->GetSerializeDataUpdateSize(cmd.offset, cmd.size);
        // SAFETY: This Span is NEVER supposed to be read/serialized, so nullptr is fine.
        // The member is not serialized because skip_serialize, but is a Span so that on
        // the deserialization side we have a well-formed member.
        // TODO(https://crbug.com/542275488): Clean these up if when we update command extension
        // serialization to serialize into this span directly.
        cmd.dataUpdateInfo = DAWN_UNSAFE_BUFFERS(Span<const std::byte>(
            static_cast<const std::byte*>(nullptr), memoryDataUpdateInfoLength));

        client->SerializeCommand(cmd,
                                 // Extensions to replace fields skipped by skip_serialize.
                                 CommandExtension{memoryDataUpdateInfoLength,
                                                  [&](Span<volatile std::byte> serializeBuffer) {
                                                      memoryHandle->SerializeDataUpdate(
                                                          serializeBuffer, cmd.offset, cmd.size);
                                                  }});
    }

    SetFutureStatus(WGPUMapAsyncStatus_Aborted, "Buffer was unmapped before mapping was resolved.");

    BufferUnmapCmd unmapCmd{};
    unmapCmd.self = ToAPI(this);
    client->SerializeCommand(unmapCmd);
}

void Buffer::APIDestroy() {
    Client* client = GetClient();

    // Remove the current mapping and destroy MemoryHandle.
    mState.Use([&](auto state) {
        FreeMappedData(state);
    });
    SetFutureStatus(WGPUMapAsyncStatus_Aborted,
                    "Buffer was destroyed before mapping was resolved.");

    BufferDestroyCmd cmd{};
    cmd.self = ToAPI(this);
    client->SerializeCommand(cmd);
}

wgpu::BufferUsage Buffer::APIGetUsage() const {
    return mUsage;
}

uint64_t Buffer::APIGetSize() const {
    return mSize;
}

wgpu::BufferMapState Buffer::APIGetMapState() const {
    return mState.Use([](auto state) {
        switch (state->mappedState) {
            case MapState::MappedForRead:
            case MapState::MappedForWrite:
            case MapState::MappedAtCreation:
                return wgpu::BufferMapState::Mapped;
            case MapState::Unmapped:
                if (state->pendingMapRequest) {
                    return wgpu::BufferMapState::Pending;
                } else {
                    return wgpu::BufferMapState::Unmapped;
                }
        }
        DAWN_UNREACHABLE();
    });
}

void Buffer::FreeMappedData(GuardedState& state) {
    state->mappedOffset = 0;
    state->mappedSize = 0;
    state->mappedData = {};
    state->memoryHandle = nullptr;
    state->mappedState = MapState::Unmapped;
}

bool Buffer::State::IsMappedForReading() const {
    return mappedState == MapState::MappedForRead;
}

bool Buffer::State::IsMappedForWriting() const {
    return mappedState == MapState::MappedForWrite || mappedState == MapState::MappedAtCreation;
}

Span<std::byte> Buffer::State::GetMappedRange(size_t offset, size_t size) const {
    auto CanGetMappedRange = [&]() {
        if (!IsMappedForReading() && !IsMappedForWriting()) {
            return false;
        }

        size_t bufferSize = mappedData.size();
        if (offset % 8 != 0 || offset < mappedOffset || offset > bufferSize) {
            return false;
        }

        size_t rangeSize = size == WGPU_WHOLE_MAP_SIZE ? bufferSize - offset : size;

        if (rangeSize % 4 != 0 || rangeSize > mappedSize) {
            return false;
        }

        size_t offsetInMappedRange = offset - mappedOffset;
        return offsetInMappedRange <= mappedSize - rangeSize;
    };

    if (CanGetMappedRange()) {
        if (size == WGPU_WHOLE_MAP_SIZE) {
            return mappedData.subspan(offset);
        } else {
            return mappedData.subspan(offset, size);
        }
    }
    return {};
}

bool Buffer::State::PendingRequestIs(FutureID futureID) const {
    return pendingMapRequest && pendingMapRequest->futureID == futureID;
}

}  // namespace dawn::wire::client
