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

#ifndef INCLUDE_DAWN_WIRE_WIRESERVER_H_
#define INCLUDE_DAWN_WIRE_WIRESERVER_H_

#include <cstdint>
#include <limits>
#include <memory>
#include <span>

#include "dawn/wire/Wire.h"

struct DawnProcTable;

namespace dawn::wire {

namespace server {
class Server;
class MemoryTransferService;
}  // namespace server

struct DAWN_WIRE_EXPORT WireServerDescriptor {
    const DawnProcTable* procs;
    CommandSerializer* serializer;
    server::MemoryTransferService* memoryTransferService = nullptr;
    bool useSpontaneousCallbacks = false;
};

class DAWN_WIRE_EXPORT WireServer : public CommandHandler {
  public:
    explicit WireServer(const WireServerDescriptor& descriptor);
    ~WireServer() override;

    bool HandleCommands(std::span<const volatile std::byte> commands) override;

    bool InjectBuffer(WGPUBuffer buffer, const Handle& handle, const Handle& deviceHandle);
    bool InjectTexture(WGPUTexture texture, const Handle& handle, const Handle& deviceHandle);
    bool InjectSurface(WGPUSurface surface, const Handle& handle, const Handle& instanceHandle);
    bool InjectInstance(WGPUInstance instance, const Handle& handle);

    // Look up a device by (id, generation) pair. Returns nullptr if the generation
    // has expired or the id is not found.
    // The Wire does not have destroy hooks to allow an embedder to observe when an object
    // has been destroyed, but in Chrome, we need to know the list of live devices so we
    // can call device.Tick() on all of them periodically to ensure progress on asynchronous
    // work is made. Getting this list can be done by tracking the (id, generation) of
    // previously injected devices, and observing if GetDevice(id, generation) returns non-null.
    WGPUDevice GetDevice(uint32_t id, uint32_t generation);

    // Check if a device handle is known by the wire.
    // In Chrome, we need to know the list of live devices so we can call device.Tick() on all of
    // them periodically to ensure progress on asynchronous work is made.
    bool IsDeviceKnown(WGPUDevice device) const;

    server::Server* GetImplForTesting();

  private:
    std::shared_ptr<server::Server> mImpl;
};

namespace server {
class DAWN_WIRE_EXPORT MemoryTransferService {
  public:
    MemoryTransferService();
    virtual ~MemoryTransferService();

    // Returns a MemoryHandle from the parameters in creationData. May return nullptr on failure.
    class MemoryHandle;
    virtual std::unique_ptr<MemoryHandle> DeserializeMemoryHandle(
        std::span<const std::byte> creationData) = 0;

    class DAWN_WIRE_EXPORT MemoryHandle {
      public:
        MemoryHandle();
        virtual ~MemoryHandle();

        // Return a direct view of the memory if this MemoryHandle supports it, nullptr when not
        // supported.
        virtual std::span<std::byte> GetSource() const { return {}; }

        // Get the required serialization size for SerializeDataUpdate for the range [offset, offset
        // + size)
        virtual size_t GetSerializeDataUpdateSize(size_t offset, size_t size) const = 0;

        // Serializes into |serializeData| the modification of the contents in the range [offset,
        // offset + size). The modified contents is passed in |data|.
        //
        // Parameters:
        //  - `serializeData`: The output buffer to write the serialized payload into.
        //  - `offset`: The byte offset of data.data() within the whole allocation..
        //  - `size`: The size of the range to update (must be <= data.size()).
        //  - `data`: The new contents for the range [offset, offset + data.size()).
        virtual void SerializeDataUpdate(std::span<volatile std::byte> serializeData,
                                         size_t offset,
                                         size_t size,
                                         std::span<const std::byte> data) const = 0;

        // Applies a data update for the range [offset, offset + size) that was produced by
        // `client::MemoryTransferService::MemoryHandle::SerializeDataUpdate`.
        //
        // For hardening, the implementation must return false if offset + size overflows the size
        // of the memory (or if offset + size overflows a size_t), or if size > target.size().
        //
        // Parameters:
        //  - `deserializeData`: The serialized payload from the client specifying the updated
        //    buffer contents.
        //  - `offset`: The byte offset for target.data() within the whole allocation.
        //  - `size`: The size of the range to update.
        //  - `target`: The range of data that is written by the update.
        //
        // Returns true on success, or false if the deserialization is invalid (e.g. OOB access).
        virtual bool DeserializeDataUpdate(std::span<const std::byte> deserializeData,
                                           size_t offset,
                                           size_t size,
                                           std::span<std::byte> target) = 0;

      private:
        MemoryHandle(const MemoryHandle&) = delete;
        MemoryHandle& operator=(const MemoryHandle&) = delete;
    };

  private:
    MemoryTransferService(const MemoryTransferService&) = delete;
    MemoryTransferService& operator=(const MemoryTransferService&) = delete;
};
}  // namespace server

}  // namespace dawn::wire

#endif  // INCLUDE_DAWN_WIRE_WIRESERVER_H_
