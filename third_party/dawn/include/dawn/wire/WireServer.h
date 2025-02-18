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

#include <memory>

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
};

class DAWN_WIRE_EXPORT WireServer : public CommandHandler {
  public:
    explicit WireServer(const WireServerDescriptor& descriptor);
    ~WireServer() override;

    const volatile char* HandleCommands(const volatile char* commands, size_t size) override;

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

  private:
    std::shared_ptr<server::Server> mImpl;
};

namespace server {
class DAWN_WIRE_EXPORT MemoryTransferService {
  public:
    MemoryTransferService();
    virtual ~MemoryTransferService();

    class ReadHandle;
    class WriteHandle;

    // Deserialize data to create Read/Write handles. These handles are for the client
    // to Read/Write data.
    virtual bool DeserializeReadHandle(const void* deserializePointer,
                                       size_t deserializeSize,
                                       ReadHandle** readHandle) = 0;
    virtual bool DeserializeWriteHandle(const void* deserializePointer,
                                        size_t deserializeSize,
                                        WriteHandle** writeHandle) = 0;

    class DAWN_WIRE_EXPORT ReadHandle {
      public:
        ReadHandle();
        virtual ~ReadHandle();

        // Return the size of the command serialized if
        // SerializeDataUpdate is called with the same offset/size args
        virtual size_t SizeOfSerializeDataUpdate(size_t offset, size_t size) = 0;

        // Gets called when a MapReadCallback resolves.
        // Serialize the data update for the range (offset, offset + size) into
        // |serializePointer| to the client There could be nothing to be serialized (if
        // using shared memory)
        virtual void SerializeDataUpdate(const void* data,
                                         size_t offset,
                                         size_t size,
                                         void* serializePointer) = 0;

      private:
        ReadHandle(const ReadHandle&) = delete;
        ReadHandle& operator=(const ReadHandle&) = delete;
    };

    class DAWN_WIRE_EXPORT WriteHandle {
      public:
        WriteHandle();
        virtual ~WriteHandle();

        // Set the target for writes from the client. DeserializeFlush should copy data
        // into the target.
        void SetTarget(void* data);
        // Set Staging data length for OOB check
        void SetDataLength(size_t dataLength);

        // This function takes in the serialized result of
        // client::MemoryTransferService::WriteHandle::SerializeDataUpdate.
        // Needs to check potential offset/size OOB and overflow
        virtual bool DeserializeDataUpdate(const void* deserializePointer,
                                           size_t deserializeSize,
                                           size_t offset,
                                           size_t size) = 0;

      protected:
        void* mTargetData = nullptr;
        size_t mDataLength = 0;

      private:
        WriteHandle(const WriteHandle&) = delete;
        WriteHandle& operator=(const WriteHandle&) = delete;
    };

  private:
    MemoryTransferService(const MemoryTransferService&) = delete;
    MemoryTransferService& operator=(const MemoryTransferService&) = delete;
};
}  // namespace server

}  // namespace dawn::wire

#endif  // INCLUDE_DAWN_WIRE_WIRESERVER_H_
