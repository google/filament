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

#ifndef SRC_DAWN_WIRE_CHUNKEDCOMMANDSERIALIZER_H_
#define SRC_DAWN_WIRE_CHUNKEDCOMMANDSERIALIZER_H_

#include <algorithm>
#include <cstring>
#include <functional>
#include <memory>
#include <utility>

#include "dawn/common/Alloc.h"
#include "dawn/common/Compiler.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/wire/Wire.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::wire {

// Simple command extension struct used when a command needs to serialize additional information
// that is not baked directly into the command already.
struct CommandExtension {
    size_t size;
    std::function<void(char*)> serialize;
};

namespace detail {

inline WireResult SerializeCommandExtension(SerializeBuffer* serializeBuffer) {
    return WireResult::Success;
}

template <typename Extension, typename... Extensions>
WireResult SerializeCommandExtension(SerializeBuffer* serializeBuffer,
                                     Extension&& e,
                                     Extensions&&... es) {
    char* buffer;
    WIRE_TRY(serializeBuffer->NextN(e.size, &buffer));
    e.serialize(buffer);

    WIRE_TRY(SerializeCommandExtension(serializeBuffer, std::forward<Extensions>(es)...));
    return WireResult::Success;
}

}  // namespace detail

class ChunkedCommandSerializer {
  public:
    explicit ChunkedCommandSerializer(CommandSerializer* serializer);

    template <typename Cmd>
    void SerializeCommand(const Cmd& cmd) {
        SerializeCommandImpl(
            cmd, [](const Cmd& cmd, size_t requiredSize, SerializeBuffer* serializeBuffer) {
                return cmd.Serialize(requiredSize, serializeBuffer);
            });
    }

    template <typename Cmd, typename... Extensions>
    void SerializeCommand(const Cmd& cmd, CommandExtension&& e, Extensions&&... es) {
        SerializeCommandImpl(
            cmd,
            [](const Cmd& cmd, size_t requiredSize, SerializeBuffer* serializeBuffer) {
                return cmd.Serialize(requiredSize, serializeBuffer);
            },
            std::forward<CommandExtension>(e), std::forward<Extensions>(es)...);
    }

    template <typename Cmd, typename... Extensions>
    void SerializeCommand(const Cmd& cmd,
                          const ObjectIdProvider& objectIdProvider,
                          Extensions&&... extensions) {
        SerializeCommandImpl(
            cmd,
            [&objectIdProvider](const Cmd& cmd, size_t requiredSize,
                                SerializeBuffer* serializeBuffer) {
                return cmd.Serialize(requiredSize, serializeBuffer, objectIdProvider);
            },
            std::forward<Extensions>(extensions)...);
    }

  private:
    template <typename Cmd, typename SerializeCmdFn, typename... Extensions>
    void SerializeCommandImpl(const Cmd& cmd,
                              SerializeCmdFn&& SerializeCmd,
                              Extensions&&... extensions) {
        size_t commandSize = cmd.GetRequiredSize();
        size_t requiredSize = (Align(extensions.size, kWireBufferAlignment) + ... + commandSize);

        if (requiredSize <= mMaxAllocationSize) {
            char* allocatedBuffer = static_cast<char*>(mSerializer->GetCmdSpace(requiredSize));
            if (allocatedBuffer != nullptr) {
                SerializeBuffer serializeBuffer(allocatedBuffer, requiredSize);
                WireResult rCmd = SerializeCmd(cmd, requiredSize, &serializeBuffer);
                WireResult rExts =
                    detail::SerializeCommandExtension(&serializeBuffer, extensions...);
                if (DAWN_UNLIKELY(rCmd != WireResult::Success || rExts != WireResult::Success)) {
                    mSerializer->OnSerializeError();
                }
            }
            return;
        }

        auto cmdSpace = std::unique_ptr<char[]>(AllocNoThrow<char>(requiredSize));
        if (!cmdSpace) {
            return;
        }
        SerializeBuffer serializeBuffer(cmdSpace.get(), requiredSize);
        WireResult rCmd = SerializeCmd(cmd, requiredSize, &serializeBuffer);
        WireResult rExts = detail::SerializeCommandExtension(&serializeBuffer, extensions...);
        if (DAWN_UNLIKELY(rCmd != WireResult::Success || rExts != WireResult::Success)) {
            mSerializer->OnSerializeError();
            return;
        }
        SerializeChunkedCommand(cmdSpace.get(), requiredSize);
    }

    void SerializeChunkedCommand(const char* allocatedBuffer, size_t remainingSize);

    raw_ptr<CommandSerializer> mSerializer;
    size_t mMaxAllocationSize;
};

}  // namespace dawn::wire

#endif  // SRC_DAWN_WIRE_CHUNKEDCOMMANDSERIALIZER_H_
