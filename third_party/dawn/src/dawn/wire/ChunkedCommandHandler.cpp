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

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/439062058): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

#include "dawn/wire/ChunkedCommandHandler.h"

#include <algorithm>
#include <cstring>
#include <utility>

#include "dawn/common/Alloc.h"

namespace dawn::wire {

ChunkedCommandHandler::ChunkedCommandHandler() = default;

ChunkedCommandHandler::~ChunkedCommandHandler() = default;

WireResult ChunkedCommandHandler::HandleChunkedCommand(DeserializeBuffer* deserializeBuffer) {
    ChunkedCommandCmd cmd;
    WIRE_TRY(cmd.Deserialize(deserializeBuffer, &mAllocator));

    ChunkedCommand* chunkedCommand = nullptr;
    if (auto it = mChunkedCommands.find(cmd.id); it != mChunkedCommands.end()) {
        chunkedCommand = &(it->second);
    } else {
        ChunkedCommand newChunkedCommand = {};
        newChunkedCommand.remainingSize = cmd.size;
        newChunkedCommand.data.reset(AllocNoThrow<char>(cmd.size));
        if (newChunkedCommand.data.get() == nullptr) {
            return WireResult::FatalError;
        }

        const auto& [newIt, inserted] =
            mChunkedCommands.insert({cmd.id, std::move(newChunkedCommand)});
        DAWN_ASSERT(inserted);
        chunkedCommand = &(newIt->second);
    }
    DAWN_ASSERT(chunkedCommand);

    if (cmd.chunkSize > chunkedCommand->remainingSize) {
        // If the chunk size is greater than the remaining size, something is wrong and we can no
        // longer handle it, so just return a FatalError.
        return WireResult::FatalError;
    }
    memcpy(chunkedCommand->data.get() + chunkedCommand->putOffset,
           const_cast<const char*>(cmd.chunkData), cmd.chunkSize);
    chunkedCommand->putOffset += cmd.chunkSize;
    chunkedCommand->remainingSize -= cmd.chunkSize;

    if (chunkedCommand->remainingSize == 0) {
        ChunkedCommand fullCommand = std::move(*chunkedCommand);
        mChunkedCommands.erase(cmd.id);
        if (HandleCommands(fullCommand.data.get(), fullCommand.putOffset) == nullptr) {
            return WireResult::FatalError;
        }
    }

    return WireResult::Success;
}

}  // namespace dawn::wire
