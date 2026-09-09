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

#ifndef SRC_UTILS_HEAP_ARRAY_H_
#define SRC_UTILS_HEAP_ARRAY_H_

#include <algorithm>
#include <cstddef>
#include <limits>
#include <ranges>
#include <type_traits>
#include <utility>

#include "src/utils/non_copyable.h"
#include "src/utils/numeric.h"
#include "src/utils/span.h"
#include "src/utils/underlying_type.h"

namespace dawn {

namespace ityp {

// "Lite", index-typed version of base::HeapArray from Chromium. It's like a safer, sized
// std::unique_ptr<T[]>, and it has a nicer (span-like / std::ranges-compatible) interface.
// To convert to an actual span, use `Span{heapArray}`, which constructs it via std::ranges.
//
// Note that just like new[], it's invalid to reinterpret to another type UNLESS the element type is
// char or std::byte AND the target type's alignment is <= alignof(std::max_align_t).
template <HasUnsignedUnderlyingType Index, typename Value>
class HeapArray :
    // Could be copyable, if needed, but we haven't needed it.
    public NonCopyable {
  private:
    using I = UnderlyingType<Index>;
    // TODO(https://crbug.com/526537224): This should probably be a RawSpan.
    using TSpan = ityp::span<Index, Value>;

    // HeapArrays never start with data in them, so a constant HeapArray wouldn't be usable.
    // If we really want that, we can implement a move-constructor from non-const to const.
    static_assert(!std::is_const_v<Value>,
                  "Array contents cannot be constant. Use `const HeapArray<I, V>` instead of "
                  "`HeapArray<I, const V>`.");

  public:
    // Constructs an empty HeapArray. (Note it cannot be resized.)
    constexpr HeapArray() = default;

    constexpr HeapArray(HeapArray<Index, Value>&& other) { *this = std::move(other); }
    constexpr HeapArray<Index, Value>& operator=(HeapArray<Index, Value>&& other) {
        std::swap(mOwnedData, other.mOwnedData);
        return *this;
    }

    constexpr ~HeapArray() {
        if (Value* ptr = mOwnedData.data()) {
            mOwnedData = {};
            delete[] ptr;
        }
    }

    // Constructs a zero-initialized HeapArray with count `count`.
    constexpr explicit HeapArray(Index count)
        // SAFETY: Allocation size matches container size.
        : DAWN_UNSAFE_BUFFERS(HeapArray{Alloc(count, InitType::Init), count}) {
        // Even if count is 0, the new[] shouldn't have returned nullptr.
        DAWN_ASSERT(mOwnedData.data() != nullptr);
    }
    // Constructs a zero-initialized HeapArray with count `count`.
    // If allocation fails, returns a falsy object, with .data() == nullptr and .size() == 0.
    constexpr HeapArray(Index count, std::nothrow_t)
        // SAFETY: Allocation size matches container size; private constructor handles if it fails.
        : DAWN_UNSAFE_BUFFERS(HeapArray{AllocNoThrow(count, InitType::Init), count}) {
        if (mOwnedData.data() == nullptr) {
            DAWN_ASSERT(mOwnedData.size() == Index{});
        }
    }

    // Constructs an uninitialized HeapArray with count `count`.
    // This can only be used with POD types, as other types are always initialized.
    [[nodiscard]] DAWN_UNSAFE_BUFFER_USAGE static constexpr HeapArray<Index, Value> Uninit(
        Index count)
        requires std::is_trivially_default_constructible_v<Value>
    {
        return HeapArray<Index, Value>{Alloc(count, InitType::Uninit), count};
    }
    // Constructs an uninitialized HeapArray with count `count`, or count 0 if allocation fails.
    // This can only be used with POD types, as other types are always initialized.
    [[nodiscard]] DAWN_UNSAFE_BUFFER_USAGE static constexpr HeapArray<Index, Value> Uninit(
        Index count,
        std::nothrow_t)
        requires std::is_trivially_default_constructible_v<Value>
    {
        return HeapArray<Index, Value>{AllocNoThrow(count, InitType::Uninit), count};
    }

    // Acquire the contents as a raw pointer and size (kind of like what you get from new[] - it's
    // valid to delete using `delete[]`).
    // Useful with std::tie when returning a size+ptr array (e.g. via the webgpu.h C API).
    constexpr std::pair<size_t, Value*> MoveToRawPointer() && {
        auto data = mOwnedData;
        mOwnedData = {};
        return {checked_cast<size_t>(data.size()), data.data()};
    }

    // Acquire the contents as a size, it's valid to delete using `delete[] span.data()`).
    // Useful when returning an array as a span (e.g. via the webgpu.h C++ API).
    constexpr TSpan MoveToSpan() && {
        TSpan result = mOwnedData;
        mOwnedData = {};
        return result;
    }

    // Returns true if the allocation succeeded. This can be used like `if (myHeapArray) {}` to
    // check if nothrow allocation succeeded. Note, even if the size is 0, this may return true.
    constexpr explicit operator bool() { return mOwnedData.data() != nullptr; }

    // Span-like interface
    constexpr auto empty() const { return mOwnedData.empty(); }
    constexpr auto data() { return mOwnedData.data(); }
    constexpr auto data() const { return mOwnedData.data(); }
    constexpr auto size() const { return mOwnedData.size(); }
    constexpr auto begin() { return mOwnedData.begin(); }
    constexpr auto begin() const { return mOwnedData.begin(); }
    constexpr auto end() { return mOwnedData.end(); }
    constexpr auto end() const { return mOwnedData.end(); }
    constexpr auto subspan(Index offset) { return mOwnedData.subspan(offset); }
    constexpr auto subspan(Index offset) const { return mOwnedData.subspan(offset); }
    constexpr auto subspan(Index offset, Index count) { return mOwnedData.subspan(offset, count); }
    constexpr auto subspan(Index offset, Index count) const {
        return mOwnedData.subspan(offset, count);
    }
    constexpr auto& front() { return mOwnedData.front(); }
    constexpr const auto& front() const { return mOwnedData.front(); }
    constexpr auto& back() { return mOwnedData.back(); }
    constexpr const auto& back() const { return mOwnedData.back(); }
    constexpr auto& operator[](Index i) { return mOwnedData[i]; }
    constexpr const auto& operator[](Index i) const { return mOwnedData[i]; }

  private:
    // Constructs a HeapArray by taking ownership of an existing allocation that was allocated with
    // new[] (we will free it with delete[]). If the allocation is nullptr, we treat it as a failed
    // allocation (e.g. from AllocNoThrow), and replace the count with 0.
    [[nodiscard]] DAWN_UNSAFE_BUFFER_USAGE constexpr HeapArray(Value* data, Index count)
        : mOwnedData{data ? TSpan{data, count} : TSpan{}} {}

    enum class InitType {
        // Value-initialized
        Init,
        // For trivially constructible default types this uses their default-initialization which
        // leaves the value as uninitialized memory.
        Uninit,
    };

    static constexpr Value* Alloc(Index count, InitType initType) {
        switch (initType) {
            case InitType::Init:
                return new Value[checked_cast<size_t>(count)]{};
            case InitType::Uninit:
                return new Value[checked_cast<size_t>(count)];
        }
        DAWN_UNREACHABLE();
        return nullptr;
    }

    static constexpr Value* AllocNoThrow(Index count, InitType initType) {
#if DAWN_ASAN_ENABLED() || DAWN_MSAN_ENABLED()
        // std::nothrow isn't implemented in sanitizers and they often have a 2GB allocation
        // limit. Catch large allocations and error out so fuzzers make progress.
        constexpr size_t kLargestAllowedAllocationAttemptBytes = 0x70000000;
#else
        constexpr size_t kLargestAllowedAllocationAttemptBytes =
            std::numeric_limits<size_t>::max() - 4095;
#endif

        // Early-fail to cover two cases:
        // - The checked_cast is going to fail.
        // - The total allocation size is going to be too close to the whole address space.
        //   PartitionAlloc in particular crashes instead of failing if (size >= SIZE_MAX - 23).
        if (I{count} > kLargestAllowedAllocationAttemptBytes / sizeof(Value)) {
            return nullptr;
        }

        switch (initType) {
            case InitType::Init:
                return new (std::nothrow) Value[checked_cast<size_t>(count)]{};
            case InitType::Uninit:
                return new (std::nothrow) Value[checked_cast<size_t>(count)];
        }
        DAWN_UNREACHABLE();
        return nullptr;
    }

    // We store this as a span, but we own its allocation.
    // {nullptr, 0} = failed to allocate. (operator bool() returns false)
    // {non-null, size} = succeeded in allocating, even if size==0. (operator bool() returns true)
    TSpan mOwnedData;
};

// Construct a HeapArray by copying its data from a range.
template <typename Index>
[[nodiscard]] static constexpr auto HeapArrayFrom(const std::ranges::sized_range auto& src) {
    // Infer the value type. (HeapArrayFrom is defined outside HeapArray so that we can do this.)
    using Value = std::ranges::range_value_t<decltype(src)>;

    Index size = checked_cast<Index>(std::ranges::size(src));

    auto result = HeapArray<Index, Value>(size);
    std::ranges::copy(src, result.begin());
    return result;
}

}  // namespace ityp

// Aliases in the dawn:: namespace with size_t for the index type.
template <typename Value>
using HeapArray = ityp::HeapArray<size_t, Value>;

[[nodiscard]] static constexpr auto HeapArrayFrom(const std::ranges::sized_range auto& src) {
    return ityp::HeapArrayFrom<size_t>(src);
}

}  // namespace dawn

#endif  // SRC_UTILS_HEAP_ARRAY_H_
