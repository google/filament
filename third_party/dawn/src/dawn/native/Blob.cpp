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

#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"
#include "dawn/native/Blob.h"
#include "dawn/native/stream/Stream.h"

namespace dawn::native {

Blob CreateBlob(size_t size) {
    if (size > 0) {
        uint8_t* ptr = new uint8_t[size];
        return Blob::UnsafeCreateWithDeleter(ptr, size, [=] { delete[] ptr; });
    } else {
        return Blob();
    }
}

// static
Blob Blob::UnsafeCreateWithDeleter(uint8_t* data, size_t size, std::function<void()> deleter) {
    return Blob(data, size, deleter);
}

Blob::Blob() : mData(nullptr), mSize(0), mDeleter({}) {}

Blob::Blob(uint8_t* data, size_t size, std::function<void()> deleter)
    : mData(data), mSize(size), mDeleter(std::move(deleter)) {
    // It is invalid to make a blob that has null data unless its size is also zero.
    DAWN_ASSERT(data != nullptr || size == 0);
}

Blob::Blob(Blob&& rhs) : mData(rhs.mData), mSize(rhs.mSize) {
    mDeleter = std::move(rhs.mDeleter);
    rhs.mData = nullptr;
    rhs.mDeleter = nullptr;
}

Blob& Blob::operator=(Blob&& rhs) {
    mData = rhs.mData;
    mSize = rhs.mSize;
    if (mDeleter) {
        mDeleter();
    }
    mDeleter = std::move(rhs.mDeleter);
    rhs.mData = nullptr;
    rhs.mDeleter = nullptr;
    return *this;
}

Blob::~Blob() {
    mData = nullptr;
    if (mDeleter) {
        mDeleter();
    }
}

bool Blob::Empty() const {
    return mSize == 0;
}

const uint8_t* Blob::Data() const {
    return mData;
}

uint8_t* Blob::Data() {
    return mData;
}

size_t Blob::Size() const {
    return mSize;
}

template <>
void stream::Stream<Blob>::Write(stream::Sink* s, const Blob& b) {
    size_t size = b.Size();
    StreamIn(s, size);
    if (size > 0) {
        void* ptr = s->GetSpace(size);
        memcpy(ptr, b.Data(), size);
    }
}

template <>
MaybeError stream::Stream<Blob>::Read(stream::Source* s, Blob* b) {
    size_t size;
    DAWN_TRY(StreamOut(s, &size));
    if (size > 0) {
        const void* ptr;
        DAWN_TRY(s->Read(&ptr, size));
        *b = CreateBlob(size);
        memcpy(b->Data(), ptr, size);
    } else {
        *b = Blob();
    }
    return {};
}

}  // namespace dawn::native
