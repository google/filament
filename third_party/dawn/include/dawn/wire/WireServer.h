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
    // TODO(https://issues.chromium.org/492456046): Pass as a `span<uint8_t> deseriazlizeData`.
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
        // TODO(https://issues.chromium.org/492456046): Replace data+size with a `span<uint8_t>
        // update` and pass the deserializePointer as a span with the size from
        // SizeOfSerializeDataUpdate.
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
        // TODO(https://issues.chromium.org/492456046): Remove the setters / getters for data and
        // instead pass them directly as a span in DeserializeDataUpdate.
        void SetTarget(void* data);
        // Set Staging data length for OOB check
        void SetDataLength(size_t dataLength);

        // TODO(492456046): Remove this overload once it has been removed in Chromium. Currently we
        // need to declare it as a non-pure virtual function so that we can safely remove the old
        // 4-parameter `DeserializeDataUpdate` from Chromium instead of having to provide an
        // implementation for it.
        virtual bool DeserializeDataUpdate(const void* deserializePointer,
                                           size_t deserializeSize,
                                           size_t offset,
                                           size_t size) {
            return false;
        }
        std::span<uint8_t> GetTarget() const;

        std::span<uint8_t> GetSource() const {
            return std::span<uint8_t>(GetSourceData(), GetSourceSize());
        }

        // Deserialize a data update produced by
        // `client::MemoryTransferService::WriteHandle::SerializeDataUpdate` and apply it to
        // the mapped buffer memory.
        //
        // Parameters:
        //  - `deserializeData`: The serialized payload from the client specifying the updated
        //    buffer contents.
        //  - `target`: The range of data that is written by the update.
        //  - `offset`: The byte offset for target.data() in the GPU buffer, used by Chromium's
        //    implementation to offset into the shmem.
        //
        // Returns true on success, or false if the deserialization is invalid (e.g. OOB access).
        //
        // The default implementation calls the old 4-parameter overload by calling
        // SetTarget/SetDataLength just for the current Chromium implementation to work with
        // the current Dawn implementation.
        // TODO(492456046): Make this pure-virtual once the old overload is removed from
        // Chromium.
        virtual bool DeserializeDataUpdate(std::span<const uint8_t> deserializeData,
                                           std::span<uint8_t> target,
                                           size_t offset) {
            if (offset > std::numeric_limits<size_t>::max() - target.size()) {
                return false;
            }

            if (target.data() != nullptr && reinterpret_cast<uintptr_t>(target.data()) < offset) {
                return false;
            }

            // In the new `DeserializeDataUpdate` (with 3 parameters) `target` should already be the
            // correct subspan of the buffer to write into, so we just need to check for OOB and
            // then write into it.
            // However, in the old `DeserializeDataUpdate` (with 4 parameters) implementation in
            // Chromium, `target` is always the full buffer and `offset` is used to offset into both
            // the target data and the shmem pointer, so we need to do the same offsetting here to
            // be compatible with both implementations.
            uint8_t* bufferStart = target.data() - offset;
            size_t lengthFromStart = target.size() + offset;
            SetTarget(bufferStart);
            SetDataLength(lengthFromStart);
            return DeserializeDataUpdate(deserializeData.data(), deserializeData.size(), offset,
                                         target.size());
        }

      private:
        WriteHandle(const WriteHandle&) = delete;
        WriteHandle& operator=(const WriteHandle&) = delete;

        // Returns a direct pointer to the source data that will
        // be copied into Target in DeserializeDataUpdate if accessible, nullptr
        // otherwise.
        // TODO(https://issues.chromium.org/492456046): Remove in favor of making GetSourceData
        // virtual.
        virtual uint8_t* GetSourceData() const { return nullptr; }
        virtual size_t GetSourceSize() const { return 0; }

        uint8_t* mTargetData = nullptr;
        size_t mDataLength = 0;
    };

  private:
    MemoryTransferService(const MemoryTransferService&) = delete;
    MemoryTransferService& operator=(const MemoryTransferService&) = delete;
};
}  // namespace server

}  // namespace dawn::wire

#endif  // INCLUDE_DAWN_WIRE_WIRESERVER_H_
