// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/replay/ReadHead.h"

#include <algorithm>

namespace dawn::replay {

ReadHead::ReadHead(std::span<const uint8_t> data) : mData(data), mReadHead(data.begin()) {}

MaybeError ReadHead::ReadBytes(std::span<uint8_t> dest) {
    auto newHead = mReadHead + dest.size();
    if (newHead <= mData.end()) {
        std::copy_n(mReadHead, dest.size(), dest.begin());
        mReadHead = newHead;
        return {};
    }
    mBad = true;
    return DAWN_INTERNAL_ERROR("Read past end of data");
}

ResultOrError<const uint32_t*> ReadHead::GetData(size_t size) {
    if (size % 4 != 0) {
        mBad = true;
        return DAWN_INTERNAL_ERROR("GetData size not multiple of 4");
    }
    size_t alignedSize = (size + 3) & ~3;
    auto newHead = mReadHead + alignedSize;
    if (newHead <= mData.end()) {
        const uint32_t* data = reinterpret_cast<const uint32_t*>(&*mReadHead);
        mReadHead = newHead;
        return {data};
    }

    mBad = true;
    return DAWN_INTERNAL_ERROR("GetData past end of data");
}

bool ReadHead::IsBad() const {
    return mBad;
}

bool ReadHead::IsDone() const {
    return mBad || mReadHead == mData.end();
}

}  // namespace dawn::replay
