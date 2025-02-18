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

#ifndef SRC_TINT_UTILS_CONTAINERS_SLICE_H_
#define SRC_TINT_UTILS_CONTAINERS_SLICE_H_

#include <array>
#include <cstdint>
#include <iterator>

#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/memory/bitcast.h"
#include "src/tint/utils/rtti/castable.h"
#include "src/tint/utils/rtti/traits.h"

TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

namespace tint {

/// A type used to indicate an empty array.
struct EmptyType {};

/// An instance of the EmptyType.
static constexpr EmptyType Empty;

/// Mode enumerator for ReinterpretSlice
enum class ReinterpretMode {
    /// Only upcasts of pointers are permitted
    kSafe,
    /// Potentially unsafe downcasts of pointers are also permitted
    kUnsafe,
};

namespace detail {

template <typename TO, typename FROM>
static constexpr bool ConstRemoved = std::is_const_v<FROM> && !std::is_const_v<TO>;

/// Private implementation of tint::CanReinterpretSlice.
/// Specialized for the case of TO equal to FROM, which is the common case, and avoids inspection of
/// the base classes, which can be troublesome if the slice is of an incomplete type.
template <ReinterpretMode MODE, typename TO, typename FROM>
struct CanReinterpretSlice {
  private:
    using TO_EL = std::remove_pointer_t<std::decay_t<TO>>;
    using FROM_EL = std::remove_pointer_t<std::decay_t<FROM>>;

  public:
    /// @see tint::CanReinterpretSlice
    static constexpr bool value =
        // const can only be applied, not removed
        !ConstRemoved<TO, FROM> &&

        // Both TO and FROM are the same type (ignoring const)
        (std::is_same_v<std::remove_const_t<TO>, std::remove_const_t<FROM>> ||

         // Both TO and FROM are pointers...
         ((std::is_pointer_v<TO> && std::is_pointer_v<FROM>)&&

          // const can only be applied to element type, not removed
          !ConstRemoved<TO_EL, FROM_EL> &&

          // Either:
          // * Both the pointer elements are of the same type (ignoring const)
          // * Both the pointer elements are both Castable, and MODE is kUnsafe, or FROM is of,
          // or
          //   derives from TO
          (std::is_same_v<std::remove_const_t<FROM_EL>, std::remove_const_t<TO_EL>> ||
           (IsCastable<FROM_EL, TO_EL> &&
            (MODE == ReinterpretMode::kUnsafe || tint::traits::IsTypeOrDerived<FROM_EL, TO_EL>)))));
};

/// Specialization of 'CanReinterpretSlice' for when TO and FROM are equal types.
template <typename T, ReinterpretMode MODE>
struct CanReinterpretSlice<MODE, T, T> {
    /// Always `true` as TO and FROM are the same type.
    static constexpr bool value = true;
};

}  // namespace detail

/// Evaluates whether a `Slice<FROM>` and be reinterpreted as a `Slice<TO>`.
/// Slices can be reinterpreted if:
///  * TO has the same or more 'constness' than FROM.
///  * And either:
///  * `FROM` and `TO` are pointers to the same type
///  * `FROM` and `TO` are pointers to CastableBase (or derived), and the pointee type of `TO` is of
///     the same type as, or is an ancestor of the pointee type of `FROM`.
template <ReinterpretMode MODE, typename TO, typename FROM>
static constexpr bool CanReinterpretSlice =
    tint::detail::CanReinterpretSlice<MODE, TO, FROM>::value;

/// A slice represents a contigious array of elements of type T.
template <typename T>
struct Slice {
    /// Type of `T`.
    using value_type = T;

    /// The pointer to the first element in the slice
    T* data = nullptr;

    /// The total number of elements in the slice
    size_t len = 0;

    /// The total capacity of the backing store for the slice
    size_t cap = 0;

    /// Constructor
    constexpr Slice() = default;

    /// Constructor
    constexpr Slice(EmptyType) {}  // NOLINT

    /// Copy constructor with covariance / const conversion
    /// @param other the vector to copy
    /// @see CanReinterpretSlice for rules about conversion
    template <typename U,
              typename = std::enable_if_t<CanReinterpretSlice<ReinterpretMode::kSafe, T, U>>>
    Slice(const Slice<U>& other) {  // NOLINT(runtime/explicit)
        *this = other.template Reinterpret<T, ReinterpretMode::kSafe>();
    }

    /// Constructor
    /// @param d pointer to the first element in the slice
    /// @param l total number of elements in the slice
    /// @param c total capacity of the backing store for the slice
    constexpr Slice(T* d, size_t l, size_t c) : data(d), len(l), cap(c) {}

    /// Constructor
    /// @param d pointer to the first element in the slice
    /// @param l total number of elements in the slice
    constexpr Slice(T* d, size_t l) : data(d), len(l), cap(l) {}

    /// Constructor
    /// @param elements c-array of elements
    template <size_t N>
    constexpr Slice(T (&elements)[N])  // NOLINT
        : data(elements), len(N), cap(N) {}

    /// Constructor
    /// @param array std::array of elements
    template <size_t N>
    constexpr Slice(std::array<T, N>& array)  // NOLINT
        : data(array.data()), len(N), cap(N) {}

    /// Reinterprets this slice as `const Slice<TO>&`
    /// @returns the reinterpreted slice
    /// @see CanReinterpretSlice
    template <typename TO, ReinterpretMode MODE = ReinterpretMode::kSafe>
    const Slice<TO>& Reinterpret() const {
        static_assert(CanReinterpretSlice<MODE, TO, T>);
        return *Bitcast<const Slice<TO>*>(this);
    }

    /// Reinterprets this slice as `Slice<TO>&`
    /// @returns the reinterpreted slice
    /// @see CanReinterpretSlice
    template <typename TO, ReinterpretMode MODE = ReinterpretMode::kSafe>
    Slice<TO>& Reinterpret() {
        static_assert(CanReinterpretSlice<MODE, TO, T>);
        return *Bitcast<Slice<TO>*>(this);
    }

    /// @return true if the slice length is zero
    bool IsEmpty() const { return len == 0; }

    /// @return the length of the slice
    size_t Length() const { return len; }

    /// Create a new slice that represents an offset into this slice
    /// @param offset the number of elements to offset
    /// @return the new slice
    Slice<T> Offset(size_t offset) const {
        if (offset > len) {
            offset = len;
        }
        return Slice(data + offset, len - offset, cap - offset);
    }

    /// Create a new slice that represents a truncated version of this slice
    /// @param length the new length
    /// @return a new slice that is truncated to `length` elements
    Slice<T> Truncate(size_t length) const {
        if (length > len) {
            length = len;
        }
        return Slice(data, length, length);
    }

    /// Index operator
    /// @param i the element index. Must be less than `len`.
    /// @returns a reference to the i'th element.
    T& operator[](size_t i) {
        TINT_ASSERT(i < Length());
        return data[i];
    }

    /// Index operator
    /// @param i the element index. Must be less than `len`.
    /// @returns a reference to the i'th element.
    const T& operator[](size_t i) const {
        TINT_ASSERT(i < Length());
        return data[i];
    }

    /// @returns a reference to the first element in the vector
    T& Front() {
        TINT_ASSERT(!IsEmpty());
        return data[0];
    }

    /// @returns a reference to the first element in the vector
    const T& Front() const {
        TINT_ASSERT(!IsEmpty());
        return data[0];
    }

    /// @returns a reference to the last element in the vector
    T& Back() {
        TINT_ASSERT(!IsEmpty());
        return data[len - 1];
    }

    /// @returns a reference to the last element in the vector
    const T& Back() const {
        TINT_ASSERT(!IsEmpty());
        return data[len - 1];
    }

    /// @returns a pointer to the first element in the vector
    T* begin() { return data; }

    /// @returns a pointer to the first element in the vector
    const T* begin() const { return data; }

    /// @returns a pointer to one past the last element in the vector
    T* end() { return data + len; }

    /// @returns a pointer to one past the last element in the vector
    const T* end() const { return data + len; }

    /// @returns a reverse iterator starting with the last element in the vector
    auto rbegin() { return std::reverse_iterator<T*>(end()); }

    /// @returns a reverse iterator starting with the last element in the vector
    auto rbegin() const { return std::reverse_iterator<const T*>(end()); }

    /// @returns the end for a reverse iterator
    auto rend() { return std::reverse_iterator<T*>(begin()); }

    /// @returns the end for a reverse iterator
    auto rend() const { return std::reverse_iterator<const T*>(begin()); }

    /// Equality operator.
    /// @param other the other slice to compare against
    /// @returns true if all fields of this slice are equal to the fields of @p other
    bool operator==(const Slice& other) const {
        return data == other.data && len == other.len && cap == other.cap;
    }

    /// Inequality operator.
    /// @param other the other slice to compare against
    /// @returns false if any fields of this slice are not equal to the fields of @p other
    bool operator!=(const Slice& other) const { return !(*this == other); }
};

/// Deduction guide for Slice from c-array
/// @param elements the input elements
template <typename T, size_t N>
Slice(T (&elements)[N]) -> Slice<T>;

}  // namespace tint

TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

#endif  // SRC_TINT_UTILS_CONTAINERS_SLICE_H_
