// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_REFBASE_H_
#define SRC_DAWN_COMMON_REFBASE_H_

#include <concepts>
#include <cstddef>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Compiler.h"

namespace dawn {

// A common class for various smart-pointers acting on referenceable/releasable pointer-like
// objects. Logic for each specialization can be customized using a Traits type that looks
// like the following:
//
//   struct {
//      static constexpr T kNullValue = ...;
//      static void AddRef(T value) { ... }
//      static void Release(T value) { ... }
//   };
//
// RefBase supports
template <typename T, typename Traits>
class RefBase {
  public:
    // Default constructor and destructor.
    RefBase() : mValue(Traits::kNullValue) {}

    ~RefBase() { Release(mValue); }

    // Constructors from nullptr.
    // NOLINTNEXTLINE(runtime/explicit)
    constexpr RefBase(std::nullptr_t) : RefBase() {}

    RefBase<T, Traits>& operator=(std::nullptr_t) {
        Set(Traits::kNullValue);
        return *this;
    }

    // Constructors from a value T.
    // NOLINTNEXTLINE(runtime/explicit)
    RefBase(T value) : mValue(value) { AddRef(value); }

    RefBase<T, Traits>& operator=(const T& value) {
        Set(value);
        return *this;
    }

    // Constructors from a RefBase<T>
    RefBase(const RefBase<T, Traits>& other) : mValue(other.mValue) { AddRef(other.mValue); }

    RefBase<T, Traits>& operator=(const RefBase<T, Traits>& other) {
        Set(other.mValue);
        return *this;
    }

    RefBase(RefBase<T, Traits>&& other) { mValue = other.Detach(); }

    RefBase<T, Traits>& operator=(RefBase<T, Traits>&& other) {
        if (&other != this) {
            Release(mValue);
            mValue = other.Detach();
        }
        return *this;
    }

    // Constructors from a RefBase<U>. Note that in the *-assignment operators this cannot be the
    // same as `other` because overload resolution rules would have chosen the *-assignement
    // operators defined with `other` == RefBase<T, Traits>.
    template <typename U, typename UTraits>
        requires std::convertible_to<U, T>
    RefBase(const RefBase<U, UTraits>& other) : mValue(other.mValue) {
        AddRef(other.mValue);
    }

    template <typename U, typename UTraits>
        requires std::convertible_to<U, T>
    RefBase<T, Traits>& operator=(const RefBase<U, UTraits>& other) {
        Set(other.mValue);
        return *this;
    }

    template <typename U, typename UTraits>
        requires std::convertible_to<U, T>
    RefBase(RefBase<U, UTraits>&& other) {
        mValue = other.Detach();
    }

    template <typename U, typename UTraits>
        requires std::convertible_to<U, T>
    RefBase<T, Traits>& operator=(RefBase<U, UTraits>&& other) {
        Release(mValue);
        mValue = other.Detach();
        return *this;
    }

    explicit operator bool() const { return !!mValue; }

    // Comparison operators.
    bool operator==(const T& other) const { return mValue == other; }
    bool operator!=(const T& other) const { return mValue != other; }

    bool operator==(const RefBase<T, Traits>& other) const = default;

    const T operator->() const { return mValue; }
    T operator->() { return mValue; }

    bool operator<(const RefBase<T, Traits>& other) const { return mValue < other.mValue; }

    // Smart pointer methods.
    const T& Get() const { return mValue; }
    T& Get() { return mValue; }

    [[nodiscard]] T Detach() {
        T value{std::move(mValue)};
        mValue = Traits::kNullValue;
        return value;
    }

    void Acquire(T value) {
        Release(mValue);
        mValue = value;
    }

    [[nodiscard]] T* InitializeInto() {
        DAWN_ASSERT(mValue == Traits::kNullValue);
        return &mValue;
    }

    // Cast operator.
    template <typename Other>
    Other Cast() && {
        Other other;
        CastImpl(this, &other);
        return other;
    }

  private:
    // Friend is needed so that instances of RefBase<U> can call AddRef and Release on
    // RefBase<T>.
    template <typename U, typename UTraits>
    friend class RefBase;

    template <typename U, typename UTraits>
        requires std::convertible_to<U, T>
    static void CastImpl(RefBase<T, Traits>* ref, RefBase<U, UTraits>* other) {
        other->Acquire(static_cast<U>(ref->Detach()));
    }

    static void AddRef(T value) {
        if (value != Traits::kNullValue) {
            Traits::AddRef(value);
        }
    }
    static void Release(T value) {
        if (value != Traits::kNullValue) {
            Traits::Release(value);
        }
    }

    void Set(T value) {
        if (mValue != value) {
            // Ensure that the new value is referenced before the old is released to prevent any
            // transitive frees that may affect the new value.
            AddRef(value);
            Release(mValue);
            mValue = value;
        }
    }

    T mValue;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_REFBASE_H_
