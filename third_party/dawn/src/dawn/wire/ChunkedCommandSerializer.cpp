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

#include "dawn/wire/ChunkedCommandSerializer.h"

#include "dawn/common/Assert.h"

namespace dawn::wire {

ChunkedCommandSerializer::ChunkedCommandSerializer(CommandSerializer* serializer)
    : mSerializer(serializer), mMaxAllocationSize(serializer->GetMaximumAllocationSize()) {
    DAWN_ASSERT(mMaxAllocationSize > 0);
}

void ChunkedCommandSerializer::SetCommandSerializerForDisconnect(CommandSerializer* serializer) {
    mSerializer = serializer;
    mMaxAllocationSize = serializer->GetMaximumAllocationSize();
    DAWN_ASSERT(mMaxAllocationSize > 0);
}

void ChunkedCommandSerializer::Flush() {
    mSerializer->Flush();
}

void ChunkedCommandSerializer::SerializeChunkedCommand(const char* allocatedBuffer,
                                                       size_t totalSize) {
    // Constant regarding the size of the WireChunkedCommandCmd that can be computed once.
    static size_t kWireChunkedCmdPrefixSize = ChunkedCommandCmd{0, 0, nullptr, 0}.GetRequiredSize();

    ChunkedCommandCmd cmd;
    cmd.id = mNextChunkedCommandId++;
    cmd.size = totalSize;

    size_t remainingSize = totalSize;
    while (remainingSize > 0) {
        cmd.chunkData = allocatedBuffer;
        cmd.chunkSize = std::min(remainingSize, mMaxAllocationSize - kWireChunkedCmdPrefixSize);
        DAWN_ASSERT(cmd.GetRequiredSize() <= mMaxAllocationSize);

        SerializeCommand(cmd);

        allocatedBuffer += cmd.chunkSize;
        remainingSize -= cmd.chunkSize;
    }
}

}  // namespace dawn::wire
