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

#ifndef SRC_DAWN_NATIVE_SERIALIZABLE_H_
#define SRC_DAWN_NATIVE_SERIALIZABLE_H_

#include <utility>

#include "dawn/native/VisitableMembers.h"
#include "dawn/native/stream/BlobSource.h"
#include "dawn/native/stream/ByteVectorSink.h"
#include "dawn/native/stream/Stream.h"

namespace dawn::native {

// Base CRTP for implementing StreamIn/StreamOut/FromBlob/ToBlob for Derived,
// assuming Derived has VisitAll methods provided by DAWN_VISITABLE_MEMBERS.
template <typename Derived>
class Serializable {
  public:
    friend void StreamIn(stream::Sink* s, const Derived& in) {
        in.VisitAll([&](const auto&... members) { StreamIn(s, members...); });
    }

    friend MaybeError StreamOut(stream::Source* s, Derived* out) {
        return out->VisitAll([&](auto&... members) { return StreamOut(s, &members...); });
    }

    static ResultOrError<Derived> FromBlob(Blob blob) {
        stream::BlobSource source(std::move(blob));
        Derived out;
        DAWN_TRY(StreamOut(&source, &out));
        return out;
    }

    Blob ToBlob() const {
        stream::ByteVectorSink sink;
        StreamIn(&sink, static_cast<const Derived&>(*this));
        return CreateBlob(std::move(sink));
    }
};
}  // namespace dawn::native

// Helper macro to define a struct or class along with VisitAll methods to call
// a functor on all members. Derives from Visitable which provides
// implementations of StreamIn/StreamOut/FromBlob/ToBlob.
// Example usage:
//   #define MEMBERS(X) \
//       X(int, a)              \
//       X(float, b)            \
//       X(Foo, foo)            \
//       X(Bar, bar)
//   DAWN_SERIALIZABLE(struct, MyStruct, MEMBERS) {
//      void SomeAdditionalMethod();
//   };
//   #undef MEMBERS
#define DAWN_SERIALIZABLE(qualifier, Name, MEMBERS) \
    struct Name##__Contents {                       \
        DAWN_VISITABLE_MEMBERS(MEMBERS)             \
    };                                              \
    qualifier Name : Name##__Contents, public ::dawn::native::Serializable<Name>

#endif  // SRC_DAWN_NATIVE_SERIALIZABLE_H_
