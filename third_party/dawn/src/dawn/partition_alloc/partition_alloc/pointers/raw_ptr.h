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

#ifndef SRC_DAWN_PARTITION_ALLOC_PARTITION_ALLOC_POINTERS_RAW_PTR_H_
#define SRC_DAWN_PARTITION_ALLOC_PARTITION_ALLOC_POINTERS_RAW_PTR_H_

// `raw_ptr<T>` is a non-owning smart pointer that has improved memory-safety
// over raw pointers. See the documentation for details:
// https://source.chromium.org/chromium/chromium/src/+/main:base/memory/raw_ptr.md
//
// Here, dawn provides a "no-op" implementation, because partition_alloc
// dependendency is missing.

#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>
#include "dawn/common/Compiler.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"

namespace partition_alloc::internal {
using RawPtrTraits = int;

// This type trait verifies a type can be used as a pointer offset.
//
// We support pointer offsets in signed (ptrdiff_t) or unsigned (size_t) values.
// Smaller types are also allowed.
template <typename Z>
static constexpr bool is_offset_type = std::is_integral_v<Z> && sizeof(Z) <= sizeof(ptrdiff_t);

// `raw_ptr<T>` is a non-owning smart pointer that has improved memory-safety
// over raw pointers. See the documentation for details:
// https://source.chromium.org/chromium/chromium/src/+/main:base/memory/raw_ptr.md
//
// raw_ptr<T> is marked as [[gsl::Pointer]] which allows the compiler to catch
// some bugs where the raw_ptr holds a dangling pointer to a temporary object.
// However the [[gsl::Pointer]] analysis expects that such types do not have a
// non-default move constructor/assignment. Thus, it's possible to get an error
// where the pointer is not actually dangling, and have to work around the
// compiler. We have not managed to construct such an example in Chromium yet.
template <typename T, RawPtrTraits Traits = 0>
class DAWN_TRIVIAL_ABI DAWN_GSL_POINTER raw_ptr {
  public:
    DAWN_FORCE_INLINE constexpr raw_ptr() noexcept = default;

    // Deliberately implicit, because raw_ptr is supposed to resemble raw ptr.
    // NOLINTNEXTLINE
    DAWN_FORCE_INLINE constexpr raw_ptr(std::nullptr_t) noexcept {}

    // Deliberately implicit, because raw_ptr is supposed to resemble raw ptr.
    // NOLINTNEXTLINE
    DAWN_FORCE_INLINE constexpr raw_ptr(T* p) noexcept : wrapped_ptr_(p) {}

    // Deliberately implicit in order to support implicit upcast.
    template <typename U,
              typename Unused = std::enable_if_t<std::is_convertible_v<U*, T*> &&
                                                 !std::is_void_v<typename std::remove_cv<T>::type>>>
    // NOLINTNEXTLINE
    DAWN_FORCE_INLINE constexpr raw_ptr(const raw_ptr<U, Traits>& ptr) noexcept
        : wrapped_ptr_(ptr.get()) {}

    // Deliberately implicit in order to support implicit upcast.
    template <typename U,
              typename Unused = std::enable_if_t<std::is_convertible_v<U*, T*> &&
                                                 !std::is_void_v<typename std::remove_cv<T>::type>>>
    // NOLINTNEXTLINE
    DAWN_FORCE_INLINE constexpr raw_ptr(raw_ptr<U, Traits>&& ptr) noexcept
        : wrapped_ptr_(ptr.wrapped_ptr_) {
        // Contrary to T*, we do implement "zero on move". This avoids the behavior to diverge
        // depending on whether this implementation or PartitionAlloc's one is used.
        ptr.wrapped_ptr_ = nullptr;
    }

    DAWN_FORCE_INLINE constexpr raw_ptr& operator=(std::nullptr_t) noexcept {
        wrapped_ptr_ = nullptr;
        return *this;
    }

    DAWN_FORCE_INLINE constexpr raw_ptr& operator=(T* p) noexcept {
        wrapped_ptr_ = p;
        return *this;
    }

    // Upcast assignment
    template <typename U,
              typename Unused = std::enable_if_t<std::is_convertible_v<U*, T*> &&
                                                 !std::is_void_v<typename std::remove_cv<T>::type>>>
    DAWN_FORCE_INLINE constexpr raw_ptr& operator=(const raw_ptr<U, Traits>& ptr) noexcept {
        wrapped_ptr_ = ptr.wrapped_ptr_;
        return *this;
    }

    template <typename U,
              typename Unused = std::enable_if_t<std::is_convertible_v<U*, T*> &&
                                                 !std::is_void_v<typename std::remove_cv<T>::type>>>
    DAWN_FORCE_INLINE constexpr raw_ptr& operator=(raw_ptr<U, Traits>&& ptr) noexcept {
        wrapped_ptr_ = ptr.wrapped_ptr_;
        ptr.wrapped_ptr_ = nullptr;
        return *this;
    }

    DAWN_FORCE_INLINE constexpr explicit operator bool() const {
        return static_cast<bool>(wrapped_ptr_);
    }

    template <typename U = T,
              typename Unused = std::enable_if_t<!std::is_void_v<typename std::remove_cv<U>::type>>>
    DAWN_FORCE_INLINE constexpr U& operator*() const {
        return *wrapped_ptr_;
    }
    DAWN_FORCE_INLINE constexpr T* operator->() const { return wrapped_ptr_; }

    // Deliberately implicit, because raw_ptr is supposed to resemble raw ptr.
    // NOLINTNEXTLINE
    DAWN_FORCE_INLINE constexpr operator T*() const { return wrapped_ptr_; }

    template <typename U>
    DAWN_FORCE_INLINE constexpr explicit operator U*() const {
        // This operator may be invoked from static_cast, meaning the types may not be implicitly
        // convertible, hence the need for static_cast here.
        return static_cast<U*>(wrapped_ptr_);
    }

    DAWN_FORCE_INLINE constexpr raw_ptr& operator++() {
        wrapped_ptr_++;
        return *this;
    }
    DAWN_FORCE_INLINE constexpr raw_ptr& operator--() {
        wrapped_ptr_--;
        return *this;
    }
    DAWN_FORCE_INLINE constexpr raw_ptr operator++(int /* post_increment */) {
        return ++wrapped_ptr_;
    }
    DAWN_FORCE_INLINE constexpr raw_ptr operator--(int /* post_decrement */) {
        return --wrapped_ptr_;
    }
    template <typename Z, typename = std::enable_if_t<partition_alloc::internal::is_offset_type<Z>>>
    DAWN_FORCE_INLINE constexpr raw_ptr& operator+=(Z delta) {
        wrapped_ptr_ += delta;
        return *this;
    }
    template <typename Z, typename = std::enable_if_t<partition_alloc::internal::is_offset_type<Z>>>
    DAWN_FORCE_INLINE constexpr raw_ptr& operator-=(Z delta) {
        wrapped_ptr_ -= delta;
        return *this;
    }

    template <
        typename Z,
        typename U = T,
        typename Unused = std::enable_if_t<!std::is_void_v<typename std::remove_cv<U>::type> &&
                                           partition_alloc::internal::is_offset_type<Z>>>
    U& operator[](Z delta) const {
        return wrapped_ptr_[delta];
    }

    // Stop referencing the underlying pointer and free its memory. Compared to raw delete calls,
    // this avoids the raw_ptr to be temporarily dangling during the free operation, which will lead
    // to taking the slower path that involves quarantine.
    DAWN_FORCE_INLINE constexpr void ClearAndDelete() noexcept { delete ExtractAsDangling(); }
    DAWN_FORCE_INLINE constexpr void ClearAndDeleteArray() noexcept {
        delete[] ExtractAsDangling();
    }

    // Clear the underlying pointer and return a temporary raw_ptr instance allowed to dangle.
    // This can be useful in cases such as:
    // ```
    //  ptr.ExtractAsDangling()->SelfDestroy();
    // ```
    // ```
    //  c_style_api_do_something_and_destroy(ptr.ExtractAsDangling());
    // ```
    // NOTE, avoid using this method as it indicates an error-prone memory ownership pattern. If
    // possible, use smart pointers like std::unique_ptr<> instead of raw_ptr<>. If you have to use
    // it, avoid saving the return value in a long-lived variable (or worse, a field)! It's meant to
    // be used as a temporary, to be passed into a cleanup & freeing function, and destructed at the
    // end of the statement.
    DAWN_FORCE_INLINE constexpr raw_ptr<T, Traits> ExtractAsDangling() noexcept {
        T* ptr = wrapped_ptr_;
        wrapped_ptr_ = nullptr;
        return raw_ptr(ptr);
    }

    // Comparison operators between raw_ptr and raw_ptr<U>/U*/std::nullptr_t.  Strictly speaking,
    // it is not necessary to provide these: the compiler can use the conversion operator implicitly
    // to allow comparisons to fall back to comparisons between raw pointers. However, `operator
    // T*`/`operator U*` may perform safety checks with a higher runtime cost, so to avoid this,
    // provide explicit comparison operators for all combinations of parameters.

    // Comparisons between `raw_ptr`s. This unusual declaration and separate definition below is
    // because `GetForComparison()` is a private method. The more conventional approach of defining
    // a comparison operator between `raw_ptr` and `raw_ptr<U>` in the friend declaration itself
    // does not work, because a comparison operator defined inline would not be allowed to call
    // `raw_ptr<U>`'s private `GetForComparison()` method.
    template <typename U, typename V, RawPtrTraits R1, RawPtrTraits R2>
    friend bool operator==(const raw_ptr<U, R1>& lhs, const raw_ptr<V, R2>& rhs);
    template <typename U, typename V, RawPtrTraits R1, RawPtrTraits R2>
    friend bool operator!=(const raw_ptr<U, R1>& lhs, const raw_ptr<V, R2>& rhs);
    template <typename U, typename V, RawPtrTraits R1, RawPtrTraits R2>
    friend bool operator<(const raw_ptr<U, R1>& lhs, const raw_ptr<V, R2>& rhs);
    template <typename U, typename V, RawPtrTraits R1, RawPtrTraits R2>
    friend bool operator>(const raw_ptr<U, R1>& lhs, const raw_ptr<V, R2>& rhs);
    template <typename U, typename V, RawPtrTraits R1, RawPtrTraits R2>
    friend bool operator<=(const raw_ptr<U, R1>& lhs, const raw_ptr<V, R2>& rhs);
    template <typename U, typename V, RawPtrTraits R1, RawPtrTraits R2>
    friend bool operator>=(const raw_ptr<U, R1>& lhs, const raw_ptr<V, R2>& rhs);

    // Comparisons with U*. These operators also handle the case where the RHS is T*.
    template <typename U>
    DAWN_FORCE_INLINE friend bool operator==(const raw_ptr& lhs, U* rhs) {
        return lhs.wrapped_ptr_ == rhs;
    }
    template <typename U>
    DAWN_FORCE_INLINE friend bool operator!=(const raw_ptr& lhs, U* rhs) {
        return !(lhs == rhs);
    }
    template <typename U>
    DAWN_FORCE_INLINE friend bool operator==(U* lhs, const raw_ptr& rhs) {
        return rhs == lhs;  // Reverse order to call the operator above.
    }
    template <typename U>
    DAWN_FORCE_INLINE friend bool operator!=(U* lhs, const raw_ptr& rhs) {
        return rhs != lhs;  // Reverse order to call the operator above.
    }
    template <typename U>
    DAWN_FORCE_INLINE friend bool operator<(const raw_ptr& lhs, U* rhs) {
        return lhs.wrapped_ptr_ < rhs;
    }
    template <typename U>
    DAWN_FORCE_INLINE friend bool operator<=(const raw_ptr& lhs, U* rhs) {
        return lhs.wrapped_ptr_ <= rhs;
    }
    template <typename U>
    DAWN_FORCE_INLINE friend bool operator>(const raw_ptr& lhs, U* rhs) {
        return lhs.wrapped_ptr_ > rhs;
    }
    template <typename U>
    DAWN_FORCE_INLINE friend bool operator>=(const raw_ptr& lhs, U* rhs) {
        return lhs.wrapped_ptr_ >= rhs;
    }
    template <typename U>
    DAWN_FORCE_INLINE friend bool operator<(U* lhs, const raw_ptr& rhs) {
        return lhs < rhs.wrapped_ptr_;
    }
    template <typename U>
    DAWN_FORCE_INLINE friend bool operator<=(U* lhs, const raw_ptr& rhs) {
        return lhs <= rhs.wrapped_ptr_;
    }
    template <typename U>
    DAWN_FORCE_INLINE friend bool operator>(U* lhs, const raw_ptr& rhs) {
        return lhs > rhs.wrapped_ptr_;
    }
    template <typename U>
    DAWN_FORCE_INLINE friend bool operator>=(U* lhs, const raw_ptr& rhs) {
        return lhs >= rhs.wrapped_ptr_;
    }

    // Comparisons with `std::nullptr_t`.
    DAWN_FORCE_INLINE friend bool operator==(const raw_ptr& lhs, std::nullptr_t) { return !lhs; }
    DAWN_FORCE_INLINE friend bool operator!=(const raw_ptr& lhs, std::nullptr_t) {
        return !!lhs;  // Use !! otherwise the costly implicit cast will be used.
    }
    DAWN_FORCE_INLINE friend bool operator==(std::nullptr_t, const raw_ptr& rhs) { return !rhs; }
    DAWN_FORCE_INLINE friend bool operator!=(std::nullptr_t, const raw_ptr& rhs) {
        return !!rhs;  // Use !! otherwise the costly implicit cast will be used.
    }

    DAWN_FORCE_INLINE friend constexpr void swap(raw_ptr& lhs, raw_ptr& rhs) noexcept {
        std::swap(lhs.wrapped_ptr_, rhs.wrapped_ptr_);
    }

    // This getter is meant *only* for situations where the pointer is meant to be compared
    // (guaranteeing no dereference or extraction outside of this class). Any verifications can and
    // should be skipped for performance reasons.
    DAWN_FORCE_INLINE constexpr T* get() const { return wrapped_ptr_; }

  private:
    // This field is not a raw_ptr<> because, because we are implementing it.
    //
    // Please note we do initialize the pointers on construction. It means we don't have
    // uninitialized raw_ptr<T>. This is important if we don't want Chrome and Dawn standalone to
    // behave differently than other Dawn embedders.
    RAW_PTR_EXCLUSION T* wrapped_ptr_ = nullptr;

    template <typename U, RawPtrTraits R>
    friend class raw_ptr;
};

template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
DAWN_FORCE_INLINE bool operator==(const raw_ptr<U, Traits1>& lhs, const raw_ptr<V, Traits2>& rhs) {
    return lhs.wrapped_ptr_ == rhs.wrapped_ptr_;
}

template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
DAWN_FORCE_INLINE bool operator!=(const raw_ptr<U, Traits1>& lhs, const raw_ptr<V, Traits2>& rhs) {
    return !(lhs == rhs);
}

template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
DAWN_FORCE_INLINE bool operator<(const raw_ptr<U, Traits1>& lhs, const raw_ptr<V, Traits2>& rhs) {
    return lhs.wrapped_ptr_ < rhs.wrapped_ptr_;
}

template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
DAWN_FORCE_INLINE bool operator>(const raw_ptr<U, Traits1>& lhs, const raw_ptr<V, Traits2>& rhs) {
    return lhs.wrapped_ptr_ > rhs.wrapped_ptr_;
}

template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
DAWN_FORCE_INLINE bool operator<=(const raw_ptr<U, Traits1>& lhs, const raw_ptr<V, Traits2>& rhs) {
    return lhs.wrapped_ptr_ <= rhs.wrapped_ptr_;
}

template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
DAWN_FORCE_INLINE bool operator>=(const raw_ptr<U, Traits1>& lhs, const raw_ptr<V, Traits2>& rhs) {
    return lhs.wrapped_ptr_ >= rhs.wrapped_ptr_;
}

}  // namespace partition_alloc::internal

using partition_alloc::internal::raw_ptr;
using partition_alloc::internal::RawPtrTraits;
constexpr RawPtrTraits DisableDanglingPtrDetection = 0;
constexpr RawPtrTraits DanglingUntriaged = 0;
constexpr RawPtrTraits LeakedDanglingUntriaged = 0;
constexpr RawPtrTraits AllowPtrArithmetic = 0;

namespace std {

// Override so set/map lookups do not create extra raw_ptr. This also allows dangling pointers to be
// used for lookup.
template <typename T, RawPtrTraits Traits>
struct less<raw_ptr<T, Traits>> {
    using Impl = typename raw_ptr<T, Traits>::Impl;
    using is_transparent = void;

    bool operator()(const raw_ptr<T, Traits>& lhs, const raw_ptr<T, Traits>& rhs) const {
        return lhs < rhs;
    }
    bool operator()(T* lhs, const raw_ptr<T, Traits>& rhs) const { return lhs < rhs; }
    bool operator()(const raw_ptr<T, Traits>& lhs, T* rhs) const { return lhs < rhs; }
};

template <typename T, RawPtrTraits Traits>
struct hash<raw_ptr<T, Traits>> {
    typedef raw_ptr<T, Traits> argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& ptr) const { return hash<T*>()(ptr.get()); }
};

// Define for cases where raw_ptr<T> holds a pointer to an array of type T. This is consistent with
// definition of std::iterator_traits<T*>. Algorithms like std::binary_search need that.
template <typename T, RawPtrTraits Traits>
struct iterator_traits<raw_ptr<T, Traits>> {
    using difference_type = ptrdiff_t;
    using value_type = std::remove_cv_t<T>;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::random_access_iterator_tag;
};

// Specialize std::pointer_traits. The latter is required to obtain the underlying raw pointer in
// the std::to_address(pointer) overload. Implementing the pointer_traits is the standard blessed
// way to customize `std::to_address(pointer)` in C++20 [1].
//
// [1] https://wg21.link/pointer.traits.optmem
template <typename T, RawPtrTraits Traits>
struct pointer_traits<raw_ptr<T, Traits>> {
    using pointer = raw_ptr<T, Traits>;
    using element_type = T;
    using difference_type = ptrdiff_t;

    template <typename U>
    using rebind = ::raw_ptr<U, Traits>;

    static constexpr pointer pointer_to(element_type& r) noexcept { return pointer(&r); }
    static constexpr element_type* to_address(pointer p) noexcept { return p.get(); }
};

}  // namespace std

#endif  // SRC_DAWN_PARTITION_ALLOC_PARTITION_ALLOC_POINTERS_RAW_PTR_H_
