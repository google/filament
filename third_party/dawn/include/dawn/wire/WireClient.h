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

#ifndef INCLUDE_DAWN_WIRE_WIRECLIENT_H_
#define INCLUDE_DAWN_WIRE_WIRECLIENT_H_

#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include "dawn/dawn_proc_table.h"
#include "dawn/wire/Wire.h"

namespace dawn::wire {

namespace client {
class Client;
class MemoryTransferService;

DAWN_WIRE_EXPORT const DawnProcTable& GetProcs();
}  // namespace client

struct ReservedBuffer {
    WGPUBuffer buffer;
    Handle handle;
    Handle deviceHandle;
};

struct ReservedTexture {
    WGPUTexture texture;
    Handle handle;
    Handle deviceHandle;
};

struct ReservedSurface {
    WGPUSurface surface;
    Handle instanceHandle;
    Handle handle;
};

struct ReservedInstance {
    WGPUInstance instance;
    Handle handle;
};

struct DAWN_WIRE_EXPORT WireClientDescriptor {
    CommandSerializer* serializer;
    client::MemoryTransferService* memoryTransferService = nullptr;
};

class DAWN_WIRE_EXPORT WireClient : public CommandHandler {
  public:
    explicit WireClient(const WireClientDescriptor& descriptor);
    ~WireClient() override;

    bool HandleCommands(std::span<const volatile std::byte> commands) override;

    ReservedBuffer ReserveBuffer(WGPUDevice device, const WGPUBufferDescriptor* descriptor);
    ReservedTexture ReserveTexture(WGPUDevice device, const WGPUTextureDescriptor* descriptor);
    ReservedSurface ReserveSurface(WGPUInstance instance,
                                   const WGPUSurfaceCapabilities* capabilities);
    ReservedInstance ReserveInstance(const WGPUInstanceDescriptor* descriptor = nullptr);

    void ReclaimBufferReservation(const ReservedBuffer& reservation);
    void ReclaimTextureReservation(const ReservedTexture& reservation);
    void ReclaimSurfaceReservation(const ReservedSurface& reservation);
    void ReclaimInstanceReservation(const ReservedInstance& reservation);

    Handle GetWireHandle(WGPUDevice device) const;

    // Disconnects the client.
    // Commands allocated after this point will not be sent.
    void Disconnect();

    client::Client* GetImplForTesting();

  private:
    std::unique_ptr<client::Client> mImpl;
};

namespace client {
class DAWN_WIRE_EXPORT MemoryTransferService {
  public:
    MemoryTransferService();
    virtual ~MemoryTransferService();

    // Create a handle for sharing memory with the server.
    // This may fail and return nullptr.
    class MemoryHandle;
    virtual std::unique_ptr<MemoryHandle> CreateMemoryHandle(size_t size) = 0;

    class DAWN_WIRE_EXPORT MemoryHandle {
      public:
        MemoryHandle();
        virtual ~MemoryHandle();

        // Get the required serialization size for SerializeCreate
        virtual size_t GetSerializeCreateSize() const = 0;

        // Serialize the handle into |serializeSpace| so it can be received by the server.
        virtual void SerializeCreate(std::span<volatile std::byte> serializeSpace) const = 0;

        // Returns a const view of the memory.
        // dawn::wire::client ensures that the memory is initialized by a data update before it is
        // made visible via wgpu::Buffer::GetConstMappedRange().
        virtual std::span<const std::byte> GetConstData() const { return GetData(); }

        // Returns a mutable view of the memory.
        // dawn::wire::client ensures that the memory is zeroed out before it is made visible via
        // wgpu::Buffer::GetMappedRange().
        virtual std::span<std::byte> GetData() const = 0;

        // Get the required serialization size for SerializeDataUpdate for the range [offset, offset
        // + size)
        virtual size_t GetSerializeDataUpdateSize(size_t offset, size_t size) const = 0;

        // Serializes into |serializeData| the modification of the contents in the range [offset,
        // offset + size).
        virtual void SerializeDataUpdate(std::span<volatile std::byte> serializeData,
                                         size_t offset,
                                         size_t size) const = 0;

        // Applies a data update for the range [offset, offset + size) that was produced by
        // `server::MemoryTransferService::MemoryHandle::SerializeDataUpdate`.
        //
        // For hardening, the implementation must return false if offset + size overflows the size
        // of the memory (or if offset + size overflows a size_t).
        //
        // Parameters:
        //  - `deserializeData`: The serialized payload from the server specifying the updated
        //    buffer contents.
        //  - `offset`: The byte offset of the range to update within the whole allocation.
        //  - `size`: The size of the range to update.
        //
        // Returns true on success, or false if the deserialization is invalid (e.g. OOB access).
        virtual bool DeserializeDataUpdate(std::span<const std::byte> deserializeData,
                                           size_t offset,
                                           size_t size) = 0;

      private:
        MemoryHandle(const MemoryHandle&) = delete;
        MemoryHandle& operator=(const MemoryHandle&) = delete;
    };

  private:
    MemoryTransferService(const MemoryTransferService&) = delete;
    MemoryTransferService& operator=(const MemoryTransferService&) = delete;
};

// Backdoor to get the order of the ProcMap for testing
DAWN_WIRE_EXPORT std::vector<std::string_view> GetProcMapNamesForTesting();
}  // namespace client
}  // namespace dawn::wire

#endif  // INCLUDE_DAWN_WIRE_WIRECLIENT_H_
