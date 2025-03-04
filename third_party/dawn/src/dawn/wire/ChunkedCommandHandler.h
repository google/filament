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

#include "dawn/common/Assert.h"
#include "dawn/wire/Wire.h"
#include "dawn/wire/WireCmd_autogen.h"

namespace dawn::wire {

class ChunkedCommandHandler : public CommandHandler {
  public:
    ChunkedCommandHandler();
    ~ChunkedCommandHandler() override;

    const volatile char* HandleCommands(const volatile char* commands, size_t size) override;

  protected:
    enum class ChunkedCommandsResult {
        Passthrough,
        Consumed,
        Error,
    };

    // Returns |true| if the commands were entirely consumed into the chunked command vector
    // and should be handled later once we receive all the command data.
    // Returns |false| if commands should be handled now immediately.
    ChunkedCommandsResult HandleChunkedCommands(const volatile char* commands, size_t size) {
        uint64_t commandSize64 = reinterpret_cast<const volatile CmdHeader*>(commands)->commandSize;

        if (commandSize64 > std::numeric_limits<size_t>::max()) {
            return ChunkedCommandsResult::Error;
        }
        size_t commandSize = static_cast<size_t>(commandSize64);
        if (size < commandSize) {
            return BeginChunkedCommandData(commands, commandSize, size);
        }
        return ChunkedCommandsResult::Passthrough;
    }

  private:
    virtual const volatile char* HandleCommandsImpl(const volatile char* commands, size_t size) = 0;

    ChunkedCommandsResult BeginChunkedCommandData(const volatile char* commands,
                                                  size_t commandSize,
                                                  size_t initialSize);

    size_t mChunkedCommandRemainingSize = 0;
    size_t mChunkedCommandPutOffset = 0;
    std::unique_ptr<char[]> mChunkedCommandData;
};

}  // namespace dawn::wire

#endif  // SRC_DAWN_WIRE_CHUNKEDCOMMANDHANDLER_H_
