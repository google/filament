// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_WIRE_BUFFERCONSUMER_H_
#define SRC_DAWN_WIRE_BUFFERCONSUMER_H_

#include <cstddef>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/wire/WireResult.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::wire {

// Wire specific alignment helpers.
template <typename T>
constexpr size_t WireAlignSizeof() {
    return AlignSizeof<T, kWireBufferAlignment>();
}
template <typename T>
std::optional<size_t> WireAlignSizeofN(size_t n) {
    return AlignSizeofN<T, kWireBufferAlignment>(n);
}

// BufferConsumer is a utility class that allows reading bytes from a buffer
// while simultaneously decrementing the amount of remaining space by exactly
// the amount read. It helps prevent bugs where incrementing a pointer and
// decrementing a size value are not kept in sync.
// BufferConsumer also contains bounds checks to prevent reading out-of-bounds.
template <typename BufferT>
class BufferConsumer {
    static_assert(sizeof(BufferT) == 1,
                  "BufferT must be 1-byte, but may have const/volatile qualifiers.");

  public:
    BufferConsumer(BufferT* buffer, size_t size) : mBuffer(buffer), mSize(size) {}

    BufferT* Buffer() const { return mBuffer; }
    size_t AvailableSize() const { return mSize; }

  protected:
    template <typename T, typename N>
    WireResult NextN(N count, T** data);

    template <typename T>
    WireResult Next(T** data);

    template <typename T>
    WireResult Peek(T** data);

  private:
    raw_ptr<BufferT, AllowPtrArithmetic> mBuffer;
    size_t mSize;
};

class SerializeBuffer : public BufferConsumer<char> {
  public:
    using BufferConsumer::BufferConsumer;
    using BufferConsumer::Next;
    using BufferConsumer::NextN;
};

class DeserializeBuffer : public BufferConsumer<const volatile char> {
  public:
    using BufferConsumer::BufferConsumer;
    using BufferConsumer::Peek;

    template <typename T, typename N>
    WireResult ReadN(N count, const volatile T** data) {
        return NextN(count, data);
    }

    template <typename T>
    WireResult Read(const volatile T** data) {
        return Next(data);
    }
};

}  // namespace dawn::wire

#endif  // SRC_DAWN_WIRE_BUFFERCONSUMER_H_
