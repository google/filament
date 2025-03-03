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

#ifndef SRC_DAWN_NATIVE_CACHEKEY_H_
#define SRC_DAWN_NATIVE_CACHEKEY_H_

#include <utility>

#include "dawn/native/stream/ByteVectorSink.h"
#include "dawn/native/stream/Stream.h"

namespace dawn::native {

class CacheKey : public stream::ByteVectorSink {
  public:
    using stream::ByteVectorSink::ByteVectorSink;

    enum class Type { ComputePipeline, RenderPipeline, Shader };

    template <typename T>
    class UnsafeUnkeyedValue {
      public:
        UnsafeUnkeyedValue() = default;
        // NOLINTNEXTLINE(runtime/explicit) allow implicit construction to decrease verbosity
        UnsafeUnkeyedValue(T&& value) : mValue(std::forward<T>(value)) {}

        const T& UnsafeGetValue() const { return mValue; }

        // Friend definition of StreamIn which can be found by ADL to override
        // stream::StreamIn<T>.
        friend constexpr void StreamIn(stream::Sink*, const UnsafeUnkeyedValue&) {}

      private:
        T mValue;
    };
};

template <typename T>
CacheKey::UnsafeUnkeyedValue<T> UnsafeUnkeyedValue(T&& value) {
    return CacheKey::UnsafeUnkeyedValue<T>(std::forward<T>(value));
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_CACHEKEY_H_
