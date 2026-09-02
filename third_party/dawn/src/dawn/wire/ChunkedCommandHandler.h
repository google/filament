// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_WIRE_CHUNKEDCOMMANDHANDLER_H_
#define SRC_DAWN_WIRE_CHUNKEDCOMMANDHANDLER_H_

#include <cstdint>
#include <limits>
#include <memory>

#include "absl/container/flat_hash_map.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "src/dawn/wire/WireDeserializeAllocator.h"
#include "src/utils/assert.h"
#include "src/utils/heap_array.h"
#include "src/utils/span.h"

namespace dawn::wire {

class ChunkedCommandHandler {
  public:
    virtual ~ChunkedCommandHandler();

    virtual bool HandleCommands(Span<const volatile std::byte> commands) { return false; }

    // TODO(https://crbug.com/528027992): Remove in favor of overload above, once both the server
    // and client implement it.
    virtual const volatile char* HandleCommands(const volatile char* commands, size_t size) {
        return nullptr;
    }

  protected:
    WireResult HandleChunkedCommand(DeserializeBuffer* deserializeBuffer);

    WireDeserializeAllocator mAllocator;

  private:
    // This map keeps track of all in-flight chunked commands. Note that because |HandleCommands|
    // must be called in a thread-safe manner, we do not need to explicitly synchronize access to
    // this map.
    struct ChunkedCommand {
        HeapArray<std::byte> data;
        Span<std::byte> current;
    };
    absl::flat_hash_map<uint64_t, ChunkedCommand> mChunkedCommands;
};

}  // namespace dawn::wire

#endif  // SRC_DAWN_WIRE_CHUNKEDCOMMANDHANDLER_H_
