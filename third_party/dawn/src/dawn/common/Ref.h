// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_REF_H_
#define SRC_DAWN_COMMON_REF_H_

#include <mutex>
#include <utility>

#include "dawn/common/RefBase.h"
#include "dawn/common/RefCounted.h"

namespace dawn {

template <typename T>
class WeakRef;

namespace detail {

class WeakRefSupportBase;

template <typename T>
struct RefCountedTraits {
    static constexpr T* kNullValue = nullptr;
    static void AddRef(T* value) { value->AddRef(); }
    static void Release(T* value) { value->Release(); }
};

}  // namespace detail

template <typename T>
class Ref : public RefBase<T*, detail::RefCountedTraits<T>> {
  public:
    using RefBase<T*, detail::RefCountedTraits<T>>::RefBase;
};

template <typename T>
Ref<T> AcquireRef(T* pointee) {
    Ref<T> ref;
    ref.Acquire(pointee);
    return ref;
}

template <typename T>
struct UnwrapRef {
    using type = T;
};
template <typename T>
struct UnwrapRef<Ref<T>> {
    using type = T;
};

template <typename T>
struct IsRef {
    static constexpr bool value = false;
};
template <typename T>
struct IsRef<Ref<T>> {
    static constexpr bool value = true;
};

template <typename T, typename H>
H AbslHashValue(H h, const Ref<T>& v) {
    return H::combine(std::move(h), v.Get());
}

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_REF_H_
