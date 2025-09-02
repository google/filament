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

#include <cstring>
#include <memory>

#include "dawn/common/Assert.h"
#include "dawn/wire/WireServer.h"
#include "dawn/wire/server/Server.h"

namespace dawn::wire::server {

class InlineMemoryTransferService : public MemoryTransferService {
  public:
    class ReadHandleImpl : public ReadHandle {
      public:
        ReadHandleImpl() {}
        ~ReadHandleImpl() override = default;

        size_t SizeOfSerializeDataUpdate(size_t offset, size_t size) override { return size; }

        void SerializeDataUpdate(const void* data,
                                 size_t offset,
                                 size_t size,
                                 void* serializePointer) override {
            if (size > 0) {
                DAWN_ASSERT(data != nullptr);
                DAWN_ASSERT(serializePointer != nullptr);
                memcpy(serializePointer, data, size);
            }
        }
    };

    class WriteHandleImpl : public WriteHandle {
      public:
        WriteHandleImpl() {}
        ~WriteHandleImpl() override = default;

        bool DeserializeDataUpdate(const void* deserializePointer,
                                   size_t deserializeSize,
                                   size_t offset,
                                   size_t size) override {
            auto target = GetTarget();
            if (deserializeSize != size || target.data() == nullptr ||
                deserializePointer == nullptr) {
                return false;
            }
            if (offset > target.size() || size > target.size() - offset) {
                return false;
            }
            memcpy(target.data() + offset, deserializePointer, size);
            return true;
        }
    };

    InlineMemoryTransferService() {}
    ~InlineMemoryTransferService() override = default;

    bool DeserializeReadHandle(const void* deserializePointer,
                               size_t deserializeSize,
                               ReadHandle** readHandle) override {
        DAWN_ASSERT(readHandle != nullptr);
        *readHandle = new ReadHandleImpl();
        return true;
    }

    bool DeserializeWriteHandle(const void* deserializePointer,
                                size_t deserializeSize,
                                WriteHandle** writeHandle) override {
        DAWN_ASSERT(writeHandle != nullptr);
        *writeHandle = new WriteHandleImpl();
        return true;
    }
};

std::unique_ptr<MemoryTransferService> CreateInlineMemoryTransferService() {
    return std::make_unique<InlineMemoryTransferService>();
}

}  // namespace dawn::wire::server
