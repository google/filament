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

#include "src/dawn/utils/TerribleCommandBuffer.h"

#include "src/utils/assert.h"
#include "src/utils/compiler.h"

namespace dawn::utils {

TerribleCommandBuffer::TerribleCommandBuffer() : mBuffer(mBackingBuffer) {}

TerribleCommandBuffer::TerribleCommandBuffer(dawn::wire::CommandHandler* handler)
    : mHandler(handler), mBuffer(mBackingBuffer) {}

void TerribleCommandBuffer::SetHandler(dawn::wire::CommandHandler* handler) {
    mHandler = handler;
}

size_t TerribleCommandBuffer::GetMaximumAllocationSize() const {
    return mBackingBuffer.size();
}

std::optional<std::span<volatile std::byte>> TerribleCommandBuffer::GetCommandSpace(size_t size) {
    if (size > mBackingBuffer.size()) {
        return std::nullopt;
    }

    if (mBuffer.size() - size < mOffset) {
        if (!Flush()) {
            return std::nullopt;
        }
        return GetCommandSpace(size);
    }

    std::span<volatile std::byte> result = mBuffer.subspan(mOffset, size);
    mOffset += size;
    return result;
}

bool TerribleCommandBuffer::Flush() {
    Span<std::byte> flushRange = mBuffer.subspan(mLastFlushedOffset, mOffset - mLastFlushedOffset);
    mLastFlushedOffset = mOffset;
    bool success = mHandler->HandleCommands(flushRange);

    // After a flush, we can only reset |mOffset| to 0 if both offsets are equal. Otherwise, there
    // are unflushed commands, likely queued as a part of the last flush, so defer resetting
    // |mOffset| until those have been processed as well.
    if (mLastFlushedOffset == mOffset) {
        mOffset = 0;
        mLastFlushedOffset = 0;
    }
    return success;
}

bool TerribleCommandBuffer::Empty() {
    return mOffset == 0;
}

size_t TerribleCommandBuffer::GetOffsetForTesting() const {
    return mOffset;
}

void TerribleCommandBuffer::SetOffsetForTesting(size_t offset) {
    mOffset = offset;
}

Span<const std::byte> TerribleCommandBuffer::GetContentSubrange(size_t startOffset,
                                                                size_t endOffset) {
    DAWN_ASSERT(endOffset >= startOffset);
    return mBuffer.subspan(startOffset, endOffset - startOffset);
}

}  // namespace dawn::utils
