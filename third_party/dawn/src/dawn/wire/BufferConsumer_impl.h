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

#ifndef SRC_DAWN_WIRE_BUFFERCONSUMER_IMPL_H_
#define SRC_DAWN_WIRE_BUFFERCONSUMER_IMPL_H_

#include <limits>
#include <type_traits>

#include "dawn/wire/BufferConsumer.h"

namespace dawn::wire {

template <typename BufferT>
template <typename T>
WireResult BufferConsumer<BufferT>::Peek(T** data) {
    if (sizeof(T) > mSize) {
        return WireResult::FatalError;
    }

    *data = reinterpret_cast<T*>(mBuffer.get());
    return WireResult::Success;
}

template <typename BufferT>
template <typename T>
WireResult BufferConsumer<BufferT>::Next(T** data) {
    constexpr size_t kSize = WireAlignSizeof<T>();
    if (kSize > mSize) {
        return WireResult::FatalError;
    }

    *data = reinterpret_cast<T*>(mBuffer.get());
    mBuffer += kSize;
    mSize -= kSize;
    return WireResult::Success;
}

template <typename BufferT>
template <typename T, typename N>
WireResult BufferConsumer<BufferT>::NextN(N count, T** data) {
    static_assert(std::is_unsigned<N>::value, "|count| argument of NextN must be unsigned.");

    // If size is zero then it indicates an overflow.
    auto size = WireAlignSizeofN<T>(count);
    if (!size || *size > mSize) {
        return WireResult::FatalError;
    }

    *data = reinterpret_cast<T*>(mBuffer.get());
    mBuffer += *size;
    mSize -= *size;
    return WireResult::Success;
}

}  // namespace dawn::wire

#endif  // SRC_DAWN_WIRE_BUFFERCONSUMER_IMPL_H_
