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

#ifndef SRC_DAWN_WIRE_SERVER_SERVERMEMORYTRANSFERSERVICE_MOCK_H_
#define SRC_DAWN_WIRE_SERVER_SERVERMEMORYTRANSFERSERVICE_MOCK_H_

#include <gmock/gmock.h>

#include <memory>
#include <span>

#include "dawn/wire/WireServer.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/wire/server/Server.h"
#include "src/utils/compiler.h"

namespace dawn::wire::server {

class MockMemoryTransferService : public MemoryTransferService {
  public:
    class MockMemoryHandle : public MemoryHandle {
      public:
        ~MockMemoryHandle() override;
        MOCK_METHOD(void, Destroy, ());

        MOCK_METHOD(std::span<std::byte>, GetSource, (), (const, override));
        MOCK_METHOD(size_t, GetSerializeDataUpdateSize, (size_t, size_t), (const, override));
        MOCK_METHOD(void,
                    SerializeDataUpdate,
                    (std::span<std::byte>, size_t, size_t, std::span<const std::byte>),
                    (const));
        // GMock does not natively support printing/handling volatile types in mock argument tuples
        // without custom printers, so we implement the volatile overload directly to cast away
        // volatile and forward to the non-volatile MOCK_METHOD.
        void SerializeDataUpdate(std::span<volatile std::byte> serializeData,
                                 size_t offset,
                                 size_t size,
                                 std::span<const std::byte> data) const override {
            SerializeDataUpdate(
                DAWN_UNSAFE_TODO(std::span<std::byte>(const_cast<std::byte*>(serializeData.data()),
                                                      serializeData.size())),
                offset, size, data);
        }
        MOCK_METHOD(bool,
                    DeserializeDataUpdate,
                    (std::span<const std::byte>, size_t, size_t, std::span<std::byte>),
                    (override));
    };

    MOCK_METHOD(std::unique_ptr<MemoryHandle>,
                DeserializeMemoryHandle,
                (std::span<const std::byte>),
                (override));
};

}  // namespace dawn::wire::server

#endif  // SRC_DAWN_WIRE_SERVER_SERVERMEMORYTRANSFERSERVICE_MOCK_H_
