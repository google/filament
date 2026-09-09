// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_UTILS_SPAN_H_
#define SRC_UTILS_SPAN_H_

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <new>
#include <ranges>
#include <span>
#include <utility>

#include "src/utils/numeric.h"
#include "src/utils/underlying_type.h"

namespace dawn {

// A span is a contiguous view of elements that can be accessed like an array. They are preferred
// over pointers and sizes because they enforce safe usage (in addition to requiring less typing).
// Dawn has its own Span class for multiple reasons:
//
// - It needs a version of std::span that can work with TypedInteger to make it impossible to use a
//   span with the wrong kind of index.
// - It needs a version of std::span with a configurable type for the pointer, so that it can be
//   changed to a raw_ptr<T> when the span is stored as a member (this is important to keep the
//   MiraclePtr refcount exact).
// - It ensures that the layout of Span is exactly (size_t mSize, T* mData) to match webgpu.h
//   structures where the count of elements always comes just before the pointer to the elements.
//   This is important because conversions between webgpu.h and dawn_platform.h structures is done
//   by simply casting the pointer (and checking that the layout matches). dawn_platform.h
//   structures use Span so we need to know the exact layout.
//
// The deviations from std::span are marked with DIFF.

namespace detail {

// The equivalent of std::dynamic_extent for the given index type.
template <typename Index>
inline constexpr Index DynamicExtent = std::numeric_limits<Index>::max();

template <typename Index, Index Extent>
inline constexpr bool IsDynamicExtent = (Extent == DynamicExtent<Index>);

template <typename T,
          HasUnsignedUnderlyingType Index,
          typename PtrType,
          Index Extent = DynamicExtent<Index>>
class SpanBase;
}  // namespace detail

// A direct replacement for std::span<T>
template <typename T, size_t Extent = detail::DynamicExtent<size_t>>
using Span = detail::SpanBase<T, size_t, T*, Extent>;

namespace ityp {
// A replacement for std::span<T> but with a different index type (like a TypedInteger).
template <typename Index, typename T, Index Extent = dawn::detail::DynamicExtent<Index>>
using span = dawn::detail::SpanBase<T, Index, T*, Extent>;
}  // namespace ityp

}  // namespace dawn

// Mark `SpanBase` as satisfying the `view` and `borrowed_range` concepts.
template <typename T, typename Index, typename PtrType, Index Extent>
inline constexpr bool std::ranges::enable_view<dawn::detail::SpanBase<T, Index, PtrType, Extent>> =
    true;
template <typename T, typename Index, typename PtrType, Index Extent>
inline constexpr bool
    std::ranges::enable_borrowed_range<dawn::detail::SpanBase<T, Index, PtrType, Extent>> = true;

namespace dawn {
namespace detail {

template <typename From, typename To>
concept LegalDataConversion = std::is_convertible_v<From (*)[], To (*)[]>;

template <typename T, typename It>
concept CompatibleIter = std::contiguous_iterator<It> &&
                         LegalDataConversion<std::remove_reference_t<std::iter_reference_t<It>>, T>;

template <typename T, typename Index, typename R>
concept CompatibleRange =
    std::ranges::contiguous_range<R> &&  //
    (std::ranges::borrowed_range<R> || std::is_const_v<T>) &&
    LegalDataConversion<std::remove_reference_t<std::ranges::range_reference_t<R>>, T> &&
    requires(R r) {
        { r.size() } -> std::same_as<Index>;
    };

template <typename T, typename Index, typename R>
concept CompatibleCopySource =
    std::ranges::contiguous_range<R> &&  //
    LegalDataConversion<
        std::remove_volatile_t<std::remove_reference_t<std::ranges::range_reference_t<R>>>,
        T> &&
    requires(R r) {
        { r.size() } -> std::same_as<Index>;
    };

template <typename Index, typename PtrType, Index Extent = DynamicExtent<Index>>
struct SpanStorage;

template <typename Index, typename PtrType, Index Extent>
    requires IsDynamicExtent<Index, Extent>
struct SpanStorage<Index, PtrType, Extent> {
    constexpr SpanStorage() noexcept = default;
    constexpr SpanStorage(size_t size, PtrType data) noexcept : mSize(size), mData(data) {
        if constexpr (sizeof(size_t) >= sizeof(Index)) {
            DAWN_CHECK(size < size_t{UnderlyingType<Index>{DynamicExtent<Index>}});
        }
    }

    // Keep members in this order as it matches the layout of webgpu.h, see toplevel comment.
    size_t mSize = 0;
    PtrType mData = {};
};

template <typename Index, typename PtrType, Index Extent>
    requires(!IsDynamicExtent<Index, Extent>)
struct SpanStorage<Index, PtrType, Extent> {
    constexpr SpanStorage() noexcept = default;
    constexpr explicit SpanStorage(PtrType data) noexcept : mData(data) {}
    constexpr SpanStorage(size_t size, PtrType data) noexcept : mData(data) {
        if constexpr (Extent != Index{}) {
            if (data == nullptr && size == 0) {
                return;
            }
        }
        DAWN_CHECK(size == size_t{UnderlyingType<Index>{Extent}});
    }

    PtrType mData = {};
};

// DIFF: size_t in function signatures is replaced by Index to support typed integers as the index
// type. The only exception is size_bytes().
template <typename T, HasUnsignedUnderlyingType Index, typename PtrType, Index Extent>
class SpanBase : private SpanStorage<Index, PtrType, Extent> {
    using Storage = SpanStorage<Index, PtrType, Extent>;
    static constexpr bool kIsDynamicExtent = IsDynamicExtent<Index, Extent>;
    static constexpr size_t kStdExtent =
        kIsDynamicExtent ? std::dynamic_extent : checked_cast<size_t>(Extent);

  public:
    // Type aliases and static members.

    // DIFF: size_type / difference_type missing because it's not clear what they should be, Index
    // or size_t?
    // DIFF: const_iterator missing to match LLVM's libc++
    // DIFF: [const_]reverse_iterator missing to avoid including <iterator> until it is needed.
    using element_type = T;
    using value_type = std::remove_cv<T>::type;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = std::span<T, kStdExtent>::iterator;

    static constexpr Index extent = Extent;

    static constexpr size_t GetOffsetOfSize()
        requires(kIsDynamicExtent)
    {
        return offsetof(SpanBase, mSize);
    }
    static constexpr size_t GetOffsetOfData() { return offsetof(SpanBase, mData); }

    // Constructors
    // DIFF: Many constructors are UNSAFE_BUFFER_USAGE.
    // DIFF: The constructor from a range is less strict than the spec requires as it would be
    // challenging to duplicate the exact logic while keeping support for custom index types.

    constexpr SpanBase() noexcept = default;

    // Constructor from a C-style array.
    template <typename ElementType, size_t N>
        requires LegalDataConversion<ElementType, T> && std::same_as<Index, size_t> &&
                 (kIsDynamicExtent || Extent == N)
    explicit constexpr SpanBase(ElementType (&arr)[N]) noexcept
        : Storage(checked_cast<size_t>(N), arr) {}

    // Constructor from an initializer list.
    // This is needed because until C++26's P3016R6, std::initializer_list does not have a .data()
    // member so it cannot use the constructor from a range.
    template <typename ElementType>
        requires LegalDataConversion<ElementType, T> && std::is_const_v<T> &&
                 std::same_as<Index, size_t> && kIsDynamicExtent
    constexpr SpanBase(std::initializer_list<ElementType> l) : Storage(l.size(), l.begin()) {}

    // Constructor from a pointer + size. This is UNSAFE_BUFFER_USAGE as other methods should be
    // preferred to create spans directly from ranges. Will DAWN_CHECK() if the size doesn't fit in
    // a size_t.
    template <typename It>
        requires CompatibleIter<element_type, It>
    DAWN_UNSAFE_BUFFER_USAGE constexpr SpanBase(It first, Index count)
        : Storage(checked_cast<size_t>(count), std::to_address(first)) {}

    // Constructor from a pointer (fixed extent only).
    template <typename It>
        requires CompatibleIter<element_type, It> && (!kIsDynamicExtent)
    DAWN_UNSAFE_BUFFER_USAGE explicit constexpr SpanBase(It first)
        : Storage(std::to_address(first)) {}

    // Constructor from two iterators, this is UNSAFE_BUFFER_USAGE as other method should be
    // preferred to create spans directly from ranges. Will DAWN_CHECK() if the size doesn't fit in
    // a size_t.
    template <typename It, typename End>
        requires(CompatibleIter<element_type, It> && std::sized_sentinel_for<End, It> &&
                 !std::is_convertible_v<End, size_t>)
    DAWN_UNSAFE_BUFFER_USAGE constexpr SpanBase(It first, End last)
        : Storage(checked_cast<size_t>(last - first), std::to_address(first)) {
        DAWN_CHECK(first <= last);
    }

    // Constructor from a range that provides `T* data()` and `Index size()`. Note that this
    // is quite different from Chromium's base::span constructor from ranges because adapting that
    // constructor to support TypedInteger for Index would be extremely challenging. Will
    // DAWN_CHECK() if the size doesn't fit in a size_t or match Extent.
    template <typename R>
        requires CompatibleRange<T, Index, R>
    explicit(false) constexpr SpanBase(R&& range)
        : Storage(checked_cast<size_t>(range.size()), range.data()) {}
    template <typename R>
        requires CompatibleRange<T, Index, R> && std::is_const_v<T>
    explicit(false) constexpr SpanBase(const R& range)
        : Storage(checked_cast<size_t>(range.size()), range.data()) {}

    // Constructor converting a dynamic extent Span to a fixed one.
    explicit constexpr SpanBase(SpanBase<T, Index, PtrType, DynamicExtent<Index>>& other)
        requires(!kIsDynamicExtent)
        : Storage(other.data()) {
        DAWN_CHECK(other.size() == Extent);
    }
    explicit constexpr SpanBase(const SpanBase<T, Index, PtrType, DynamicExtent<Index>>& other)
        requires(!kIsDynamicExtent)
        : Storage(other.data()) {
        DAWN_CHECK(other.size() == Extent);
    }

    // Move / copy constructor / assignment operator.
    constexpr SpanBase(const SpanBase& other) noexcept = default;
    constexpr SpanBase(SpanBase&& other) noexcept = default;
    constexpr SpanBase& operator=(const SpanBase& other) noexcept = default;
    constexpr SpanBase& operator=(SpanBase&& other) noexcept = default;

    // Iterators
    //
    // Note that iterators use the std::span iterators to take advantage of the hardened stdlib.

    // DIFF: cbegin/cend missing to match LLVM's libc++
    // DIFF: r[c]begin/end missing to avoid including <iterator> until it is needed.

    constexpr iterator begin() const noexcept { return as_std_span().begin(); }
    constexpr iterator end() const noexcept { return as_std_span().end(); }

    // Element access

    // Returns a reference to the first element of this. When empty, will DAWN_CHECK().
    constexpr reference front() const noexcept { return at(Index{}); }
    // Returns a reference to the last element of this. When empty, will DAWN_CHECK().
    constexpr reference back() const noexcept {
        // Note that an underflow happens in empty spans, which will make the argument to `at` be
        // the maximum value and cause a DAWN_CHECK.
        return at(size() - Index{UnderlyingType<Index>(1u)});
    }

    // Returns a reference to the element of this at `index`. Will DAWN_CHECK() if OOB.
    constexpr reference at(Index index) const {
        DAWN_CHECK(index < size());
        // SAFETY: The argument to unchecked_at is already checked in range in the DAWN_CHECK above.
        return DAWN_UNSAFE_BUFFERS(unchecked_at(index));
    }
    // Returns a reference to the element of this at `index`. Will DAWN_CHECK() if OOB.
    constexpr reference operator[](Index index) const noexcept { return at(index); }

    // Returns a pointer at the contents of this.
    constexpr pointer data() const noexcept { return this->mData; }

    // Observers

    // Returns the number of elements of this.
    // DIFF: For fixed extent Spans, returns 0 if mData is nullptr.
    constexpr Index size() const noexcept {
        if constexpr (kIsDynamicExtent) {
            return Index(static_cast<UnderlyingType<Index>>(this->mSize));
        } else {
            if constexpr (Extent != Index{}) {
                if (this->mData == nullptr) {
                    return Index{};
                }
            }
            return Extent;
        }
    }

    // Returns the footprint in bytes of the elements of this.
    constexpr size_t size_bytes() const noexcept {
        return static_cast<size_t>(static_cast<UnderlyingType<Index>>(size())) *
               sizeof(element_type);
    }

    // Returns true if this contains no elements.
    [[nodiscard]] constexpr bool empty() const noexcept { return size() == Index{}; }

    // Subviews
    // DIFF: subviews with template Offset / Count missing.
    // DIFF: dynamic_extent is not supported in the 2-arg version of subspan(), the single argument
    // overload must be used instead.

    // Returns a span of the first `count` elements of this. Will DAWN_CHECK() if OOB.
    constexpr auto first(Index count) const {
        DAWN_CHECK(count <= size());
        // SAFETY: data() points at at least size() elements, so the DAWN_CHECK ensure that the
        // result span is a subset of this
        return DAWN_UNSAFE_BUFFERS(SpanBase<T, Index, PtrType>(data(), count));
    }

    // Returns a span of the last `count` elements of this. Will DAWN_CHECK() if OOB.
    constexpr auto last(Index count) const {
        DAWN_CHECK(count <= size());
        // SAFETY: data() points at at least size() elements, so the DAWN_CHECK ensure that the
        // result span is a subset of this. The argument to unchecked_at is already checked in range
        // in the DAWN_CHECK above.
        return DAWN_UNSAFE_BUFFERS(
            SpanBase<T, Index, PtrType>(&unchecked_at(size() - count), count));
    }

    // Returns a span starting at `offset` and until the end of this. Will DAWN_CHECK() if OOB.
    constexpr auto subspan(Index offset) const {
        DAWN_CHECK(offset <= size());
        Index remainingSize = size() - offset;
        // SAFETY: data() points at at least size() elements, so the DAWN_CHECK ensure that the
        // result span is a subset of this. The argument to unchecked_at is already checked in range
        // in the first DAWN_CHECK above.
        return DAWN_UNSAFE_BUFFERS(
            SpanBase<T, Index, PtrType>(&unchecked_at(offset), remainingSize));
    }
    // Returns a span starting at `offset` and of `count` elements inside of this. Will DAWN_CHECK()
    // if OOB.
    constexpr auto subspan(Index offset, Index count) const {
        DAWN_CHECK(offset <= size());
        DAWN_CHECK(count <= size() - offset);
        // SAFETY: data() points at at least size() elements, so the DAWN_CHECKs ensure that the
        // result span is a subset of this. The argument to unchecked_at is already checked in range
        // in the first DAWN_CHECK above.
        return DAWN_UNSAFE_BUFFERS(SpanBase<T, Index, PtrType>(&unchecked_at(offset), count));
    }

    // Additions not part of std::span.

    // Consider adding the following like in Chromium's base::span:
    //
    //  - operator ==, operator <=>
    //  - get_at
    //  - to_fixed_extent
    //  - variants of methods that take offset / indices as template params to return sized spans.

    // Splits a span a given offset, returning a pair of spans that cover the ranges strictly before
    // the offset and starting at the offset, respectively. Will DAWN_CHECK when the offset is OOB.
    constexpr auto SplitAt(Index offset) const { return std::pair(first(offset), subspan(offset)); }

    // Returns a span of the first N elements, removing them. Will DAWN_CHECK when the offset is
    // OOB.
    constexpr auto TakeFirst(Index offset)
        requires(kIsDynamicExtent)
    {
        const auto [first, rest] = SplitAt(offset);
        *this = rest;
        return first;
    }

    // Performs a deep copy of the elements referenced by `range` to those referenced by `this`.
    // The spans must be the same size.
    // Mirrors Chromium's base::span::copy_from but does not implement the constexpr version since
    // it is not currently needed.
    template <typename R>
        requires(CompatibleCopySource<const T, Index, R> && !std::is_const_v<T>)
    void CopyFrom(R&& range) {
        using RElement =
            std::add_const_t<std::remove_reference_t<std::ranges::range_reference_t<R>>>;
        SpanBase<RElement, Index, const RElement*> other(std::forward<R>(range));
        DAWN_CHECK(other.size() == size());
        // Using `<=` to compare pointers to different allocations is UB;
        // delegating to the STL is the legal workaround:
        // https://eel.is/c++draft/defns.order.ptr
        std::less_equal{}(data(), other.data()) ? std::ranges::copy(other, begin())
                                                : std::ranges::copy_backward(other, end());
    }

    // Like `CopyFrom()`, but allows the source to be smaller than this span, and will only copy as
    // far as the source size, leaving the remaining elements of this span unwritten. Mirrors
    // Chromium's base::span::copy_prefix_from but does not implement the constexpr version since it
    // is not currently needed.
    template <typename R>
        requires(CompatibleCopySource<const T, Index, R> && !std::is_const_v<T>)
    void CopyPrefixFrom(R&& range) {
        using RElement =
            std::add_const_t<std::remove_reference_t<std::ranges::range_reference_t<R>>>;
        SpanBase<RElement, Index, const RElement*> other(std::forward<R>(range));
        first(other.size()).CopyFrom(other);
    }

  private:
    // Helper function used to avoid doing the DAWN_CHECK inside at() redundantly when the
    // precondition is already checked by some other mean.
    DAWN_UNSAFE_BUFFER_USAGE constexpr reference unchecked_at(Index typedIndex) const {
        size_t index = checked_cast<size_t>(typedIndex);
        // SAFETY: The caller must guarantee that the index is in range already.
        return DAWN_UNSAFE_BUFFERS(*(data() + index));
    }

    // Helper function to return the equivalent std::span. This is necessary to be able to use
    // the hardened std::span iterators when the standard library enables them.
    constexpr std::span<T, kStdExtent> as_std_span() const {
        if constexpr (kIsDynamicExtent) {
            // SAFETY: This is the same allocation and size as this.
            return DAWN_UNSAFE_BUFFERS(std::span<T, kStdExtent>(this->mData, this->mSize));
        } else {
            if constexpr (Extent != Index{}) {
                DAWN_CHECK(this->mData != nullptr);
            }
            // SAFETY: This is the same allocation and size as this.
            return DAWN_UNSAFE_BUFFERS(std::span<T, kStdExtent>(this->mData, kStdExtent));
        }
    }
};

}  // namespace detail

// Converts a `Span<[const|volatile] T>` to a `Span<[const|volatile] std::byte>`.
// Mirrors Chromium's base::as_[writable_]bytes but with std::byte.
template <typename T, typename Index, typename PtrType, Index Extent>
constexpr auto SpanAsBytes(detail::SpanBase<T, Index, PtrType, Extent> s) {
    if constexpr (detail::IsDynamicExtent<Index, Extent>) {
        if constexpr (std::is_volatile_v<T>) {
            // SAFETY: `s.data()` points to at least `s.size_bytes()` bytes of data.
            return DAWN_UNSAFE_BUFFERS(Span<const volatile std::byte>{
                reinterpret_cast<const volatile std::byte*>(s.data()), s.size_bytes()});
        } else {
            // SAFETY: `s.data()` points to at least `s.size_bytes()` bytes of data.
            return DAWN_UNSAFE_BUFFERS(Span<const std::byte>{
                reinterpret_cast<const std::byte*>(s.data()), s.size_bytes()});
        }
    } else {
        constexpr size_t kByteExtent =
            static_cast<size_t>(static_cast<UnderlyingType<Index>>(Extent)) * sizeof(T);
        if constexpr (std::is_volatile_v<T>) {
            // SAFETY: `s.data()` points to at least `s.size_bytes()` bytes of data.
            return DAWN_UNSAFE_BUFFERS(Span<const volatile std::byte, kByteExtent>{
                reinterpret_cast<const volatile std::byte*>(s.data())});
        } else {
            // SAFETY: `s.data()` points to at least `s.size_bytes()` bytes of data.
            return DAWN_UNSAFE_BUFFERS(
                Span<const std::byte, kByteExtent>{reinterpret_cast<const std::byte*>(s.data())});
        }
    }
}

template <typename T, typename Index, typename PtrType, Index Extent>
    requires(!std::is_const_v<T>)
constexpr auto SpanAsWritableBytes(detail::SpanBase<T, Index, PtrType, Extent> s) {
    if constexpr (detail::IsDynamicExtent<Index, Extent>) {
        if constexpr (std::is_volatile_v<T>) {
            // SAFETY: `s.data()` points to at least `s.size_bytes()` bytes of data.
            return DAWN_UNSAFE_BUFFERS(Span<volatile std::byte>{
                reinterpret_cast<volatile std::byte*>(s.data()), s.size_bytes()});
        } else {
            // SAFETY: `s.data()` points to at least `s.size_bytes()` bytes of data.
            return DAWN_UNSAFE_BUFFERS(
                Span<std::byte>{reinterpret_cast<std::byte*>(s.data()), s.size_bytes()});
        }
    } else {
        constexpr size_t kByteExtent =
            static_cast<size_t>(static_cast<UnderlyingType<Index>>(Extent)) * sizeof(T);
        if constexpr (std::is_volatile_v<T>) {
            // SAFETY: `s.data()` points to at least `s.size_bytes()` bytes of data.
            return DAWN_UNSAFE_BUFFERS(Span<volatile std::byte, kByteExtent>{
                reinterpret_cast<volatile std::byte*>(s.data())});
        } else {
            // SAFETY: `s.data()` points to at least `s.size_bytes()` bytes of data.
            return DAWN_UNSAFE_BUFFERS(
                Span<std::byte, kByteExtent>{reinterpret_cast<std::byte*>(s.data())});
        }
    }
}

namespace detail {
template <typename T, typename Index, typename ByteType, size_t ByteExtent>
concept LegalByteReinterpretAs =
    // Only allow reintepreting spans of std::byte type.
    std::same_as<std::remove_cv_t<ByteType>, std::byte> &&
    // Ensure we are not casting away constness.
    (!std::is_const_v<ByteType> || std::is_const_v<T>) &&
    // Ensure we are not casting away volatility.
    (!std::is_volatile_v<ByteType> || std::is_volatile_v<T>) &&
    // For fixed-extent spans, ensure the byte size is a multiple of the target type.
    (IsDynamicExtent<size_t, ByteExtent> || ByteExtent % sizeof(T) == 0u);

template <typename T, typename Index, typename ByteType, size_t ByteExtent>
    requires(LegalByteReinterpretAs<T, Index, ByteType, ByteExtent>)
constexpr auto ReinterpretSpanImpl(Span<ByteType, ByteExtent> s) {
    // Check for proper alignment of the target type.
    DAWN_CHECK(reinterpret_cast<uintptr_t>(s.data()) % alignof(T) == 0u);

    // In C++, one cannot simply reinterpret a span of bytes as a span of `T` and dereference it,
    // unless an object of type `T` actually exists at that memory location.
    //
    // Per [intro.object], casting a pointer does not start the lifetime of an object. Even if the
    // bytes represent a valid object representation, strict adherence to the standard requires
    // explicit object creation. Dereferencing a pointer to an object that hasn't been created is
    // undefined behavior.
    //
    // The proper solution is C++23's `std::start_lifetime_as_array` (see [obj.lifetime]), which
    // explicitly blesses the memory as containing objects of type `ElementType`.
    //
    // [intro.object]: https://eel.is/c++draft/intro.object
    // [obj.lifetime] https://eel.is/c++draft/obj.lifetime
    //
    // std::start_lifetime_as_array is part of C++23, but not yet implemented by CLang as of January
    // 2026. `std::launder` helps with pointer provenance issues, but does not by itself start the
    // lifetime of the object. However, in practice, most compilers implicitly treat this pattern as
    // valid de facto, but it is technically not standard-compliant.
    auto* ptr = std::launder(reinterpret_cast<T*>(s.data()));

    if constexpr (IsDynamicExtent<size_t, ByteExtent>) {
        // Check that the size is a multiple of the target type.
        DAWN_CHECK(s.size_bytes() % sizeof(T) == 0u);

        // SAFETY: We checked for proper alignment, size, strict aliasing rules, and started the
        // lifetime of the array.
        return DAWN_UNSAFE_BUFFERS(
            ityp::span<Index, T>{ptr, Index{UnderlyingType<Index>(s.size_bytes() / sizeof(T))}});
    } else {
        constexpr Index kTargetExtent =
            Index{static_cast<UnderlyingType<Index>>(ByteExtent / sizeof(T))};
        // SAFETY: We checked for proper alignment, size, strict aliasing rules, and started the
        // lifetime of the array.
        return DAWN_UNSAFE_BUFFERS(ityp::span<Index, T, kTargetExtent>{ptr});
    }
}
}  // namespace detail

// Converts a `Span<[const|volatile] std::byte>` to a `Span<[const|volatile] T>`.
// Mirrors Chromium's base::subtle::reinterpret_span with some minor differences:
//   - Allows for volatile qualifiers as long as they are not cast away.
//   - Provides a less strict requirement for std::is_trivially_copyable<T> if using the
//     alternative overload along with the UNSAFE_BUFFER suppressions.
template <typename T,
          typename Index = size_t,
          typename ByteType,
          size_t ByteExtent = detail::DynamicExtent<size_t>>
    requires(detail::LegalByteReinterpretAs<T, Index, ByteType, ByteExtent>)
DAWN_UNSAFE_BUFFER_USAGE constexpr auto ReinterpretSpan(Span<ByteType, ByteExtent> s) {
    return detail::ReinterpretSpanImpl<T, Index, ByteType, ByteExtent>(s);
}
template <typename T,
          typename Index = size_t,
          typename ByteType,
          size_t ByteExtent = detail::DynamicExtent<size_t>>
    requires(
        // This function effectively "creates" objects by overlaying a type onto raw bytes,
        // bypassing constructors. Such an operation is only safe for objects that do not maintain
        // internal invariant. Therefore, we restrict this function to trivially copyable types.
        std::is_trivially_copyable_v<T> &&
        detail::LegalByteReinterpretAs<T, Index, ByteType, ByteExtent>)
constexpr auto ReinterpretSpan(Span<ByteType, ByteExtent> s) {
    // SAFETY: We don't need to worry about ctors/dtors since T is trivially copyable.
    return detail::ReinterpretSpanImpl<T, Index, ByteType, ByteExtent>(s);
}

// Converts a `[const] T&` to a `Span<[const] T, 1>`.
// Mirrors Chromium's base::span_from_ref.
template <typename Index = size_t, typename T>
constexpr auto SpanFromRef(const T& t) {
    // SAFETY: A reference always points at a valid allocation of one element.
    return DAWN_UNSAFE_BUFFERS(
        ityp::span<Index, const T, Index{UnderlyingType<Index>(1u)}>{std::addressof(t)});
}
template <typename Index = size_t, typename T>
constexpr auto SpanFromRef(T& t) {
    // SAFETY: A reference always points at a valid allocation of one element.
    return DAWN_UNSAFE_BUFFERS(
        ityp::span<Index, T, Index{UnderlyingType<Index>(1u)}>{std::addressof(t)});
}

// Converts a `[const] T&` to a `Span<[const] std::byte, sizeof(T)>`.
// Mirrors Chromium's base::byte_span_from_ref but with std::byte.
template <typename T>
constexpr auto ByteSpanFromRef(const T& t) {
    return SpanAsBytes(SpanFromRef(t));
}
template <typename T>
constexpr auto ByteSpanFromRef(T& t) {
    return SpanAsWritableBytes(SpanFromRef(t));
}

}  // namespace dawn

#endif  // SRC_UTILS_SPAN_H_
