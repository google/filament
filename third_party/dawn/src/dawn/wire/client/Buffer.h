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

#ifndef SRC_DAWN_WIRE_CLIENT_BUFFER_H_
#define SRC_DAWN_WIRE_CLIENT_BUFFER_H_

#include <webgpu/webgpu.h>

#include <memory>
#include <optional>

#include "dawn/common/FutureUtils.h"
#include "dawn/common/Ref.h"
#include "dawn/common/RefCountedWithExternalCount.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/client/ObjectBase.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::wire::client {

class Device;

class Buffer final : public RefCountedWithExternalCount<ObjectWithEventsBase> {
  public:
    static WGPUBuffer Create(Device* device, const WGPUBufferDescriptor* descriptor);
    static WGPUBuffer CreateError(Device* device, const WGPUBufferDescriptor* descriptor);

    Buffer(const ObjectBaseParams& params,
           const ObjectHandle& eventManagerHandle,
           Device* device,
           const WGPUBufferDescriptor* descriptor);
    void DeleteThis() override;

    ObjectType GetObjectType() const override;

    WGPUFuture MapAsync(WGPUMapMode mode,
                        size_t offset,
                        size_t size,
                        const WGPUBufferMapCallbackInfo& callbackInfo);
    void* GetMappedRange(size_t offset, size_t size);
    const void* GetConstMappedRange(size_t offset, size_t size);
    void Unmap();

    void Destroy();

    // Note that these values can be arbitrary since they aren't validated in the wire client.
    WGPUBufferUsage GetUsage() const;
    uint64_t GetSize() const;

    WGPUBufferMapState GetMapState() const;

  private:
    friend class Client;
    class MapAsyncEvent;

    void WillDropLastExternalRef() override;

    // Prepares the callbacks to be called and potentially calls them
    void SetFutureStatus(WGPUMapAsyncStatus status, std::string_view message);

    bool IsMappedForReading() const;
    bool IsMappedForWriting() const;
    bool CheckGetMappedRangeOffsetSize(size_t offset, size_t size) const;

    void FreeMappedData();

    const uint64_t mSize = 0;
    const WGPUBufferUsage mUsage;
    const bool mDestructWriteHandleOnUnmap;
    Ref<Device> mDevice;

    // Mapping members are mutable depending on the current map state.
    enum class MapRequestType { Read, Write };
    struct MapRequest {
        FutureID futureID = kNullFutureID;
        size_t offset = 0;
        size_t size = 0;
        // Because validation for request type is validated via the backend, we use an optional type
        // here. This is nullopt when an invalid request type is passed to the wire.
        std::optional<MapRequestType> type;
    };
    enum class MapState {
        Unmapped,
        MappedForRead,
        MappedForWrite,
        MappedAtCreation,
    };
    std::optional<MapRequest> mPendingMapRequest = std::nullopt;
    MapState mMappedState = MapState::Unmapped;
    raw_ptr<void> mMappedData = nullptr;
    size_t mMappedOffset = 0;
    size_t mMappedSize = 0;

    // Only one mapped pointer can be active at a time
    // TODO(enga): Use a tagged pointer to save space.
    std::unique_ptr<MemoryTransferService::ReadHandle> mReadHandle = nullptr;
    std::unique_ptr<MemoryTransferService::WriteHandle> mWriteHandle = nullptr;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_BUFFER_H_
