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

#include "dawn/wire/WireClient.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/FutureUtils.h"
#include "src/dawn/common/MutexProtected.h"
#include "src/dawn/common/Ref.h"
#include "src/dawn/common/RefCountedWithExternalCount.h"
#include "src/dawn/wire/client/ObjectBase.h"
#include "src/utils/span.h"

namespace dawn::wire::client {

class Device;

class Buffer final : public RefCountedWithExternalCount<ObjectWithEventsBase> {
  public:
    static Buffer* Create(Device* device, const BufferDescriptor* descriptor);
    static Buffer* CreateError(Device* device, const BufferDescriptor* descriptor);

    Buffer(const ObjectBaseParams& params,
           Ref<Instance> instance,
           Device* device,
           const BufferDescriptor* descriptor);
    void DeleteThis() override;

    ObjectType GetObjectType() const override;

    Future APIMapAsync(wgpu::MapMode mode,
                       size_t offset,
                       size_t size,
                       const WGPUBufferMapCallbackInfo& callbackInfo);
    void* APIGetMappedRange(size_t offset, size_t size);
    const void* APIGetConstMappedRange(size_t offset, size_t size);
    wgpu::Status APIWriteMappedRange(size_t offset, Span<const std::byte> data);
    wgpu::Status APIReadMappedRange(size_t offset, Span<std::byte> data);
    void APIUnmap();
    void APIDestroy();

    // Note that these values can be arbitrary since they aren't validated in the wire client.
    wgpu::BufferUsage APIGetUsage() const;
    uint64_t APIGetSize() const;

    wgpu::BufferMapState APIGetMapState() const;

  private:
    friend class Client;
    class MapAsyncEvent;

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
    struct State {
        bool IsMappedForReading() const;
        bool IsMappedForWriting() const;
        Span<std::byte> GetMappedRange(size_t offset, size_t size) const;

        bool PendingRequestIs(FutureID futureID) const;

        MapState mappedState = MapState::Unmapped;
        std::optional<MapRequest> pendingMapRequest = std::nullopt;

        // TODO(https://crbug.com/526537224): Use RawSpan instead of Span.
        Span<std::byte> mappedData = {};
        size_t mappedOffset = 0;
        size_t mappedSize = 0;

        std::shared_ptr<MemoryTransferService::MemoryHandle> memoryHandle = nullptr;
    };
    using GuardedState = MutexRecursiveProtected<State>::Usage;

    void WillDropLastExternalRef() override;

    // Prepares the callbacks to be called and potentially calls them.
    void SetFutureStatus(WGPUMapAsyncStatus status, std::string_view message);

    void FreeMappedData(GuardedState& state);

    const uint64_t mSize = 0;
    const wgpu::BufferUsage mUsage;
    const bool mDestructMemoryHandleOnUnmap;
    Ref<Device> mDevice;

    MutexRecursiveProtected<State> mState;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_BUFFER_H_
