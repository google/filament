// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/utils/TerribleCommandBuffer.h"

#include "dawn/common/Assert.h"

namespace dawn::utils {

TerribleCommandBuffer::TerribleCommandBuffer() {}

TerribleCommandBuffer::TerribleCommandBuffer(dawn::wire::CommandHandler* handler)
    : mHandler(handler) {}

void TerribleCommandBuffer::SetHandler(dawn::wire::CommandHandler* handler) {
    mHandler = handler;
}

size_t TerribleCommandBuffer::GetMaximumAllocationSize() const {
    return sizeof(mBuffer);
}

void* TerribleCommandBuffer::GetCmdSpace(size_t size) {
    // Note: This returns non-null even if size is zero.
    if (size > sizeof(mBuffer)) {
        return nullptr;
    }
    char* result = &mBuffer[mOffset];
    if (sizeof(mBuffer) - size < mOffset) {
        if (!Flush()) {
            return nullptr;
        }
        return GetCmdSpace(size);
    }

    mOffset += size;
    return result;
}

bool TerribleCommandBuffer::Flush() {
    bool success = mHandler->HandleCommands(mBuffer, mOffset) != nullptr;
    mOffset = 0;
    return success;
}

bool TerribleCommandBuffer::Empty() {
    return mOffset == 0;
}

}  // namespace dawn::utils
