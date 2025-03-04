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
#include <utility>

#include "dawn/common/Alloc.h"
#include "dawn/common/Assert.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/client/Client.h"

namespace dawn::wire::client {

class InlineMemoryTransferService : public MemoryTransferService {
    class ReadHandleImpl : public ReadHandle {
      public:
        explicit ReadHandleImpl(std::unique_ptr<uint8_t[]> stagingData, size_t size)
            : mStagingData(std::move(stagingData)), mSize(size) {}

        ~ReadHandleImpl() override = default;

        size_t SerializeCreateSize() override { return 0; }

        void SerializeCreate(void*) override {}

        const void* GetData() override { return mStagingData.get(); }

        bool DeserializeDataUpdate(const void* deserializePointer,
                                   size_t deserializeSize,
                                   size_t offset,
                                   size_t size) override {
            if (deserializeSize != size || deserializePointer == nullptr) {
                return false;
            }

            if (offset > mSize || size > mSize - offset) {
                return false;
            }

            void* start = static_cast<uint8_t*>(mStagingData.get()) + offset;
            memcpy(start, deserializePointer, size);
            return true;
        }

      private:
        std::unique_ptr<uint8_t[]> mStagingData;
        size_t mSize;
    };

    class WriteHandleImpl : public WriteHandle {
      public:
        explicit WriteHandleImpl(std::unique_ptr<uint8_t[]> stagingData, size_t size)
            : mStagingData(std::move(stagingData)), mSize(size) {}

        ~WriteHandleImpl() override = default;

        size_t SerializeCreateSize() override { return 0; }

        void SerializeCreate(void*) override {}

        void* GetData() override { return mStagingData.get(); }

        size_t SizeOfSerializeDataUpdate(size_t offset, size_t size) override {
            DAWN_ASSERT(offset <= mSize);
            DAWN_ASSERT(size <= mSize - offset);
            return size;
        }

        void SerializeDataUpdate(void* serializePointer, size_t offset, size_t size) override {
            DAWN_ASSERT(mStagingData != nullptr);
            DAWN_ASSERT(serializePointer != nullptr);
            DAWN_ASSERT(offset <= mSize);
            DAWN_ASSERT(size <= mSize - offset);
            memcpy(serializePointer, static_cast<uint8_t*>(mStagingData.get()) + offset, size);
        }

      private:
        std::unique_ptr<uint8_t[]> mStagingData;
        size_t mSize;
    };

  public:
    InlineMemoryTransferService() {}
    ~InlineMemoryTransferService() override = default;

    ReadHandle* CreateReadHandle(size_t size) override {
        auto stagingData = std::unique_ptr<uint8_t[]>(AllocNoThrow<uint8_t>(size));
        if (stagingData) {
            return new ReadHandleImpl(std::move(stagingData), size);
        }
        return nullptr;
    }

    WriteHandle* CreateWriteHandle(size_t size) override {
        auto stagingData = std::unique_ptr<uint8_t[]>(AllocNoThrow<uint8_t>(size));
        if (stagingData) {
            memset(stagingData.get(), 0, size);
            return new WriteHandleImpl(std::move(stagingData), size);
        }
        return nullptr;
    }
};

std::unique_ptr<MemoryTransferService> CreateInlineMemoryTransferService() {
    return std::make_unique<InlineMemoryTransferService>();
}

}  // namespace dawn::wire::client
