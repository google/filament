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

#include "src/dawn/wire/ChunkedCommandHandler.h"

#include <algorithm>
#include <cstring>
#include <span>
#include <utility>

#include "src/utils/compiler.h"
#include "src/utils/heap_array.h"
#include "src/utils/numeric.h"

namespace dawn::wire {

ChunkedCommandHandler::~ChunkedCommandHandler() = default;

WireResult ChunkedCommandHandler::HandleChunkedCommand(DeserializeBuffer* deserializeBuffer) {
    ChunkedCommandCmd cmd;
    WIRE_TRY(cmd.Deserialize(deserializeBuffer, &mAllocator));

    ChunkedCommand* chunkedCommand = nullptr;
    if (auto it = mChunkedCommands.find(cmd.id); it != mChunkedCommands.end()) {
        // We already started handling this chunked command. Continue writing into that allocation.
        chunkedCommand = &(it->second);
    } else {
        // This is the first block of this chunked command. Make a new allocation to write into.
        ChunkedCommand newChunkedCommand = {};
        newChunkedCommand.data =
            // SAFETY: This will be incrementally initialized by each chunk of the command.
            DAWN_UNSAFE_BUFFERS(HeapArray<std::byte>::Uninit(cmd.size, std::nothrow));
        if (!newChunkedCommand.data) {
            return WireResult::FatalError;
        }
        newChunkedCommand.current = newChunkedCommand.data;

        const auto& [newIt, inserted] =
            mChunkedCommands.insert({cmd.id, std::move(newChunkedCommand)});
        DAWN_ASSERT(inserted);
        chunkedCommand = &(newIt->second);
    }
    DAWN_ASSERT(chunkedCommand);

    if (cmd.chunkData.size() > chunkedCommand->current.size()) {
        // If the chunk size is greater than the remaining size, something is wrong and we can no
        // longer handle it, so just return a FatalError.
        return WireResult::FatalError;
    }
    chunkedCommand->current.TakeFirst(cmd.chunkData.size()).CopyFrom(cmd.chunkData);

    if (chunkedCommand->current.empty()) {
        ChunkedCommand fullCommand = std::move(*chunkedCommand);
        mChunkedCommands.erase(cmd.id);
        if (!HandleCommands(fullCommand.data)) {
            return WireResult::FatalError;
        }
    }

    return WireResult::Success;
}

}  // namespace dawn::wire
