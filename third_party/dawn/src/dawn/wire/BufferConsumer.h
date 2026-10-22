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
#include <new>

#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/Constants.h"
#include "src/dawn/common/Math.h"
#include "src/dawn/wire/WireResult.h"
#include "src/utils/compiler.h"
#include "src/utils/span.h"

namespace dawn::wire {

// Wire specific alignment helpers.
template <typename T>
constexpr size_t WireAlignSizeof() {
    static_assert(alignof(T) <= kWireBufferAlignment, "T must be 8-byte alignable for the wire");
    return AlignSizeof<T, kWireBufferAlignment>();
}
template <typename T>
std::optional<size_t> WireAlignSizeofN(size_t n) {
    static_assert(alignof(T) <= kWireBufferAlignment, "T must be 8-byte alignable for the wire");
    return AlignSizeofN<T, kWireBufferAlignment>(n);
}

// BufferConsumer is a utility class that allows reading bytes from a buffer
// while simultaneously decrementing the amount of remaining space by exactly
// the amount read. It helps prevent bugs where incrementing a pointer and
// decrementing a size value are not kept in sync.
// BufferConsumer also contains bounds checks to prevent reading out-of-bounds.
template <typename BufferT>
class BufferConsumer {
    static_assert(std::is_same_v<std::remove_cv_t<BufferT>, std::byte>,
                  "BufferT must be std::byte, but may have const/volatile qualifiers.");

  public:
    explicit BufferConsumer(Span<BufferT> data) : mData(data) {}

    bool Empty() const { return mData.empty(); }

  protected:
    template <typename T>
    WireResult NextN(size_t count, Span<T>* out) {
        size_t byteCount = 0;
        WIRE_TRY(PeekN(count, out, &byteCount));
        mData = mData.subspan(byteCount);
        return WireResult::Success;
    }

    template <typename T>
    WireResult Next(T** data) {
        Span<T> out;
        WIRE_TRY(NextN(1u, &out));
        *data = out.data();
        return WireResult::Success;
    }

    // Reads `count` objects of type T from the buffer and returns a span to them. The number of
    // bytes that would be read is also optionally returned in |byteCount|.
    template <typename T>
    WireResult PeekN(size_t count, Span<T>* out, size_t* byteCount = nullptr) const {
        // If size is nullopt then it indicates an overflow.
        auto size = WireAlignSizeofN<T>(count);
        if (!size || *size > mData.size()) {
            return WireResult::FatalError;
        }
        DAWN_CHECK(*size >= sizeof(T) * count);
        if (byteCount) {
            *byteCount = *size;
        }

        // TODO(https://crbug.com/528027992): Use ReinterpretSpan once we fix alignment check that
        // currently fails on Android AMD. This should be solvable by ensuring that
        // dawn::wire::CommandSerializer::GetCmdSpace returns std::max_align_t aligned memory
        // address.
        *out = DAWN_UNSAFE_TODO(
            Span<T>(reinterpret_cast<T*>(mData.first(sizeof(T) * count).data()), count));
        return WireResult::Success;
    }

    // TODO(https://crbug.com/526537224): Use RawSpan instead of Span.
    Span<BufferT> mData = {};
};

class SerializeBuffer : public BufferConsumer<volatile std::byte> {
  public:
    using BufferConsumer::BufferConsumer;

    using BufferConsumer::Next;
    using BufferConsumer::NextN;
};

class DeserializeBuffer : public BufferConsumer<const volatile std::byte> {
  public:
    using BufferConsumer::BufferConsumer;

    template <typename T>
    WireResult Peek(const volatile T** data) const {
        Span<const volatile T> out;
        WIRE_TRY(PeekN(1u, &out));
        *data = out.data();
        return WireResult::Success;
    }

    template <typename T>
    WireResult ReadN(size_t count, Span<const volatile T>* out) {
        return NextN(count, out);
    }

    template <typename T>
    WireResult Read(const volatile T** data) {
        return Next(data);
    }
};

}  // namespace dawn::wire

#endif  // SRC_DAWN_WIRE_BUFFERCONSUMER_H_
