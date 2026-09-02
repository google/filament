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

#include "src/dawn/wire/WireDeserializeAllocator.h"

#include <algorithm>
#include <utility>

#include "src/utils/compiler.h"

namespace dawn::wire {
WireDeserializeAllocator::WireDeserializeAllocator() {
    Reset();
}

WireDeserializeAllocator::~WireDeserializeAllocator() {
    Reset();
}

std::optional<Span<std::byte>> WireDeserializeAllocator::TryGetSpace(size_t size) {
    // Return space in the current buffer if possible first.
    if (mCurrentBuffer.size() >= size) {
        return mCurrentBuffer.TakeFirst(size);
    }

    // Otherwise allocate a new buffer and try again.
    size_t allocationSize = std::max(size, kDefaultBufferSize);
    auto allocation =
        // SAFETY: This is a pool allocation that will be initialized when it's suballocated.
        DAWN_UNSAFE_BUFFERS(HeapArray<std::byte>::Uninit(allocationSize, std::nothrow));
    if (!allocation) {
        return std::nullopt;
    }

    mCurrentBuffer = allocation;
    mAllocations.push_back(std::move(allocation));
    return TryGetSpace(size);
}

void WireDeserializeAllocator::Reset() {
    // The initial buffer is the inline buffer so that some allocations can be skipped
    mCurrentBuffer = mStaticBuffer;
    mAllocations.clear();
}
}  // namespace dawn::wire
