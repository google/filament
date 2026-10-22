// Copyright 2022 The Dawn & Tint Authors
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

#include "src/dawn/native/Blob.h"

#include <algorithm>
#include <span>
#include <utility>

#include "src/dawn/common/Math.h"
#include "src/dawn/native/stream/Stream.h"
#include "src/utils/assert.h"
#include "src/utils/compiler.h"

namespace dawn::native {

// static
Blob Blob::Create(size_t size) {
    if (size > 0) {
        uint8_t* ptr = new uint8_t[size];
        return Blob::UnsafeCreateWithDeleter(ptr, size, [=] { delete[] ptr; });
    } else {
        return Blob();
    }
}

// static
Blob Blob::Create(Blob&& original, size_t offset, size_t extent) {
    Blob result(original.mData.subspan(offset, extent), std::move(original.mDeleter));
    original.mData = {};
    original.mDeleter = nullptr;
    return result;
}

// static
Blob Blob::UnsafeCreateWithDeleter(uint8_t* data, size_t size, std::function<void()> deleter) {
    return Blob(DAWN_UNSAFE_TODO(std::span<std::byte>(reinterpret_cast<std::byte*>(data), size)),
                deleter);
}

Blob::Blob() : mData({}), mDeleter({}) {}

Blob::Blob(std::span<std::byte> data, std::function<void()> deleter)
    : mData(data), mDeleter(std::move(deleter)) {}

Blob::Blob(Blob&& rhs) : mData(rhs.mData) {
    mDeleter = std::move(rhs.mDeleter);
    rhs.mData = {};
    rhs.mDeleter = nullptr;
}

Blob& Blob::operator=(Blob&& rhs) {
    if (mDeleter) {
        mDeleter();
    }
    mData = rhs.mData;
    mDeleter = std::move(rhs.mDeleter);
    rhs.mData = {};
    rhs.mDeleter = nullptr;
    return *this;
}

Blob::~Blob() {
    if (mDeleter) {
        mDeleter();
    }
}

bool Blob::Empty() const {
    return mData.empty();
}

std::span<const std::byte> Blob::Data() const {
    return mData;
}

std::span<std::byte> Blob::Data() {
    return mData;
}

const std::byte* Blob::DataPtr() const {
    return mData.data();
}

std::byte* Blob::DataPtr() {
    return mData.data();
}

size_t Blob::Size() const {
    return mData.size();
}

bool Blob::operator==(const Blob& other) const {
    return std::ranges::equal(Data(), other.Data());
}

template <>
void stream::Stream<Blob>::Write(stream::Sink* s, const Blob& b) {
    size_t size = b.Size();
    StreamIn(s, size);
    if (size > 0) {
        std::ranges::copy(b.Data(), s->GetSpace(size).begin());
    }
}

template <>
MaybeError stream::Stream<Blob>::Read(stream::Source* s, Blob* b) {
    size_t size;
    DAWN_TRY(StreamOut(s, &size));
    if (size > 0) {
        std::span<const std::byte> src;
        DAWN_TRY_ASSIGN(src, s->Read(size));
        *b = Blob::Create(size);
        std::ranges::copy(src, b->Data().begin());
    } else {
        *b = Blob();
    }
    return {};
}

}  // namespace dawn::native
