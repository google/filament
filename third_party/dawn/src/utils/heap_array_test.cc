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

#include "src/utils/heap_array.h"

#include <array>
#include <limits>
#include <ranges>

#include "src/utils/gtest.h"
#include "src/utils/typed_integer.h"

namespace dawn {
namespace {

class HeapArrayTest : public ::testing::Test {
  protected:
    using Index = TypedInteger<struct KeyT, size_t>;
    using Val = TypedInteger<struct ValT, uint32_t>;
    using HeapArrayVal = ityp::HeapArray<Index, Val>;

    // Type for use with Uninit() which requires a POD value type.
    using HeapArrayPOD = ityp::HeapArray<Index, int>;

    // Read the pointer of a HeapArray's allocation to prevent the allocation attempt from
    // being optimized away, forcing any OOM or leak to actually happen at runtime.
    template <typename Arr>
    void PreventElidingAllocation(const Arr& arr) {
        // Write the pointer value to volatile memory so that a pointer address must be created.
        using ConstVoidPtr = const void*;
        [[maybe_unused]] static volatile ConstVoidPtr data;
        data = arr.data();
    }
};

// Name "*DeathTest" per https://google.github.io/googletest/advanced.html#death-test-naming
using HeapArrayDeathTest = HeapArrayTest;

TEST_F(HeapArrayTest, KeyTypes) {
    (void)ityp::HeapArray<TypedInteger<struct KU8, uint8_t>, int>();
    (void)ityp::HeapArray<TypedInteger<struct KU32, uint32_t>, int>();
    (void)ityp::HeapArray<TypedInteger<struct KSizeT, size_t>, int>();

    enum class E : uint32_t {};
    (void)ityp::HeapArray<E, int>();

    (void)ityp::HeapArray<uint16_t, int>();
}

// Test that values can be set at an index and retrieved from the same index.
TEST_F(HeapArrayTest, Indexing) {
    HeapArrayVal arr{Index{10u}};
    {
        arr[Index{2u}] = Val{5u};
        arr[Index{1u}] = Val{9u};
        arr[Index{9u}] = Val{2u};
        arr.front() = Val{8u};

        EXPECT_EQ(arr[Index{2u}], Val{5u});
        EXPECT_EQ(arr[Index{1u}], Val{9u});
        EXPECT_EQ(arr[Index{9u}], Val{2u});
        EXPECT_EQ(arr.front(), Val{8u});
    }
}

// Test that the HeapArray can be iterated in order with a range-based for loop
TEST_F(HeapArrayTest, RangeBasedIteration) {
    // Also check that it satisfies various range concepts.
    static_assert(std::ranges::common_range<HeapArrayVal>);
    static_assert(std::ranges::contiguous_range<HeapArrayVal>);
    static_assert(std::ranges::input_range<HeapArrayVal>);
    static_assert(std::ranges::output_range<HeapArrayVal, Val>);
    static_assert(std::ranges::sized_range<HeapArrayVal>);
    static_assert(std::ranges::viewable_range<HeapArrayVal>);
    static_assert(!std::ranges::borrowed_range<HeapArrayVal>);
    static_assert(!std::ranges::view<HeapArrayVal>);

    HeapArrayVal arr{Index{10u}};
    EXPECT_TRUE(bool{arr});

    // Assign in a non-const range-based for loop
    for (uint32_t i = 0; Val& val : arr) {
        val = Val(i);
    }

    // Check values in a const range-based for loop
    for (uint32_t i = 0; Val val : static_cast<const HeapArrayVal&>(arr)) {
        EXPECT_EQ(val, arr[Index(i++)]);
    }
}

// Test that begin/end/front/back/data return pointers/references to the correct elements.
TEST_F(HeapArrayTest, BeginEndFrontBackData) {
    HeapArrayVal arr{Index{10u}};

    // non-const versions
    EXPECT_EQ(&*arr.begin(), &arr[Index{0u}]);
    EXPECT_EQ(&*arr.end(),
              // SAFETY: Comparing safe implementation with unsafe re-implementation.
              DAWN_UNSAFE_BUFFERS(&arr[Index{0u}] + static_cast<size_t>(arr.size())));
    EXPECT_EQ(&arr.front(), &arr[Index{0u}]);
    EXPECT_EQ(&arr.back(), &arr[Index{9u}]);
    EXPECT_EQ(arr.data(), &arr[Index{0u}]);

    // const versions
    const HeapArrayVal& constArr = arr;
    EXPECT_EQ(&*constArr.begin(), &constArr[Index{0u}]);
    EXPECT_EQ(&*constArr.end(),
              // SAFETY: Comparing safe implementation with unsafe re-implementation.
              DAWN_UNSAFE_BUFFERS(&constArr[Index{0u}] + static_cast<size_t>(constArr.size())));
    EXPECT_EQ(&constArr.front(), &constArr[Index{0u}]);
    EXPECT_EQ(&constArr.back(), &constArr[Index{9u}]);
    EXPECT_EQ(constArr.data(), &constArr[Index{0u}]);
}

// Empty HeapArray.
TEST_F(HeapArrayTest, Empty) {
    {
        HeapArrayVal arr{Index{0u}};
        EXPECT_TRUE(bool{arr});
        EXPECT_NE(nullptr, arr.data());
        EXPECT_EQ(Index{0u}, arr.size());
    }
    {
        HeapArrayVal arr{Index{0u}, std::nothrow};
        EXPECT_TRUE(bool{arr});
        EXPECT_NE(nullptr, arr.data());
        EXPECT_EQ(Index{0u}, arr.size());
    }
}

// `Uninit`ialized HeapArray.
TEST_F(HeapArrayTest, Uninit) {
    // SAFETY: Testing unsafe API.
    auto arr = DAWN_UNSAFE_BUFFERS(HeapArrayPOD::Uninit(Index{1u}));
    EXPECT_TRUE(bool{arr});

    arr[Index{0u}] = 5;
    EXPECT_EQ(arr[Index{0u}], 5);
}

// Tests for MoveToRawPointer.
TEST_F(HeapArrayTest, MoveToRawPointer) {
    // Check with an untyped HeapArray.
    {
        auto arr = HeapArray<int>(5);
        int* originalData = arr.data();
        size_t originalSize = arr.size();

        auto [size, data] = std::move(arr).MoveToRawPointer();

        ASSERT_TRUE(arr.empty());
        static_assert(std::is_same_v<decltype(data), int*>);
        static_assert(std::is_same_v<decltype(size), size_t>);
        ASSERT_EQ(data, originalData);
        ASSERT_EQ(size, originalSize);

        delete[] data;
    }
    // Check with a typed HeapArray.
    {
        auto arr = ityp::HeapArray<Index, int>(Index{5u});
        int* originalData = arr.data();
        Index originalSize = arr.size();

        auto [size, data] = std::move(arr).MoveToRawPointer();

        ASSERT_TRUE(arr.empty());
        static_assert(std::is_same_v<decltype(data), int*>);
        static_assert(std::is_same_v<decltype(size), size_t>);  // Not an Index!
        ASSERT_EQ(data, originalData);
        ASSERT_EQ(size, size_t{originalSize});

        delete[] data;
    }
}

// Tests for MoveToSpan.
TEST_F(HeapArrayTest, MoveToSpan) {
    // Check with an untyped HeapArray.
    {
        auto arr = HeapArray<int>(5);
        int* originalData = arr.data();
        size_t originalSize = arr.size();

        auto sp = std::move(arr).MoveToSpan();

        ASSERT_TRUE(arr.empty());
        static_assert(std::is_same_v<decltype(sp), Span<int>>);
        ASSERT_EQ(sp.data(), originalData);
        ASSERT_EQ(sp.size(), originalSize);

        delete[] sp.data();
    }
    // Check with a typed HeapArray.
    {
        auto arr = ityp::HeapArray<Index, int>(Index{5u});
        int* originalData = arr.data();
        Index originalSize = arr.size();

        auto sp = std::move(arr).MoveToSpan();

        ASSERT_TRUE(arr.empty());
        static_assert(std::is_same_v<decltype(sp), ityp::span<Index, int>>);
        ASSERT_EQ(sp.data(), originalData);
        ASSERT_EQ(sp.size(), originalSize);

        delete[] sp.data();
    }
}

// Tests for the HeapArrayFrom helper.
TEST_F(HeapArrayTest, HeapArrayFrom) {
    constexpr std::array<int, 5> kSrc = {1, 2, 3, 4, 5};
    {
        auto arr = ityp::HeapArrayFrom<Index>(kSrc);
        EXPECT_EQ(arr[Index{0u}], 1);
        EXPECT_EQ(arr[Index{4u}], 5);
    }
    {
        auto arr = HeapArrayFrom(kSrc);
        EXPECT_EQ(arr[0], 1);
        EXPECT_EQ(arr[4], 5);
    }
}

// Test for HeapArray's assignment operator.
TEST_F(HeapArrayTest, Assignment) {
    HeapArray<int> arr(100);
    PreventElidingAllocation(arr);
    arr[0] = 10;
    EXPECT_EQ(arr[0], 10);

    // operator=() should replace the old allocation. If the test runs with LeakSanitizer
    // (is_asan + ASAN_OPTIONS=detect_leaks=1), this will also check it's not leaked, but
    // the FreedBy* tests are the primary tests of this.
    arr = HeapArray<int>(200);
    PreventElidingAllocation(arr);
    EXPECT_EQ(arr.size(), 200u);
    EXPECT_EQ(arr[0], 0);
    arr[199] = 20;
    EXPECT_EQ(arr[199], 20);
}

// HeapArray is freed when it goes out of scope. Requires 512MiB of memory if working.
// Should OOM (at least on systems without ridiculous amounts of memory) if broken.
TEST_F(HeapArrayTest, FreedByScope) {
    constexpr size_t k512MiB = static_cast<size_t>(512) * 1024 * 1024;
    constexpr uint64_t kAllocateTotal = sizeof(size_t) > sizeof(uint32_t)
                                            ? 0x100'0000'0000  // 1 TiB
                                            : 0x1'0000'0000;   // 4 GiB

    for (uint64_t allocated = 0; allocated < kAllocateTotal; ++allocated) {
        // SAFETY: Data not used. Initialization would take a long time.
        auto x = DAWN_UNSAFE_BUFFERS(HeapArray<uint8_t>::Uninit(k512MiB, std::nothrow));
        allocated += k512MiB;

        // Use nothrow and then check the pointer, to prevent the allocation from being optimized
        // away by the compiler assuming it will succeed.
        ASSERT_NE(x.data(), nullptr);
    }
}

// HeapArray is freed when assigned over. Requires 512MiB of memory if working.
// Should OOM (at least on systems without ridiculous amounts of memory) if broken.
TEST_F(HeapArrayTest, FreedByAssignment) {
    constexpr size_t k256MiB = static_cast<size_t>(256) * 1024 * 1024;
    constexpr uint64_t kAllocateTotal = sizeof(size_t) > sizeof(uint32_t)
                                            ? 0x100'0000'0000  // 1 TiB
                                            : 0x1'0000'0000;   // 4 GiB

    HeapArray<uint8_t> x;
    for (uint64_t allocated = 0; allocated < kAllocateTotal; ++allocated) {
        // SAFETY: Data not used. Initialization would take a long time.
        x = DAWN_UNSAFE_BUFFERS(HeapArray<uint8_t>::Uninit(k256MiB, std::nothrow));
        allocated += k256MiB;

        // Use nothrow and then check the pointer, to prevent the allocation from being optimized
        // away by the compiler assuming it will succeed.
        ASSERT_NE(x.data(), nullptr);
    }
}

// Test HeapArray can be used as a range (by converting it to span).
TEST_F(HeapArrayTest, Range) {
    HeapArrayVal arr{Index{10u}};

    // Explicit construction.
    (void)std::span<Val>(arr);
    // Dawn's span requires the index type to match.
    (void)ityp::span<Index, Val>(arr);

    // Spans also allow implicit construction from a range.
    [[maybe_unused]] std::span<Val> s1 = arr;
    [[maybe_unused]] ityp::span<Index, Val> s2 = arr;
}

// Test HeapArrayFrom handles when the source range is larger than its index type.
// (There are no other HeapArray constructors that need to check this.)
TEST_F(HeapArrayDeathTest, SmallIndex) {
    using IndexU8 = TypedInteger<struct IndexU8T, uint8_t>;
    {
        (void)ityp::HeapArrayFrom<IndexU8>(std::array<int, 254>{});

        // 255 is invalid because it's reserved as a sentinel value by dawn::SpanBase.
        EXPECT_DEATH_IF_SUPPORTED((void)ityp::HeapArrayFrom<IndexU8>(std::array<int, 255>{}), "");
    }
}

// Checks that various byte types are always aligned to alignof(std::max_align_t).
template <typename HA>
void CheckByteAllocation(const HA& ha) {
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ha.data()) % alignof(std::max_align_t), 0u);
}
TEST_F(HeapArrayTest, ByteTypeAlignment) {
    CheckByteAllocation(HeapArray<std::byte>(16));
    CheckByteAllocation(HeapArray<char>(16));
    // uint8_t is not technically guaranteed to be max-aligned, but in practice it will be.
    // As of this writing, some Dawn code relies on it.
    CheckByteAllocation(HeapArray<uint8_t>(16));
}

// Check that various non-byte types are always aligned as expected.
template <typename T>
void CheckTypedAllocation(const HeapArray<T>& ha) {
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ha.data()) % alignof(T), 0u);
}
TEST_F(HeapArrayTest, NonByteTypeAlignment) {
    CheckTypedAllocation(HeapArray<uint32_t>(16));
    struct A {
        uint8_t x;
    };
    CheckTypedAllocation(HeapArray<A>{16});
    struct B {
        std::max_align_t x;
    };
    CheckTypedAllocation(HeapArray<B>{16});
    struct alignas(256) C {
        int x;
    };
    static_assert(alignof(C) == 256);
    CheckTypedAllocation(HeapArray<C>{16});
}

// Test allocations close to the size of the address space.
TEST_F(HeapArrayTest, OutOfMemoryAtLimit) {
    // Allocation of uint8_t values
    {
        // Test sizes that are blocked by the HeapArray implementation before trying to allocate,
        // and one element smaller so that HeapArray will actually try to allocate.
        for (size_t leaveSpace : {0u, 4094u, 4095u}) {
            HeapArray<uint8_t> arr{std::numeric_limits<size_t>::max() - leaveSpace, std::nothrow};
            PreventElidingAllocation(arr);
            EXPECT_FALSE(bool{arr});
        }
    }
    // Allocation of uint32_t values
    {
        static constexpr size_t kMaxSize = std::numeric_limits<size_t>::max() / sizeof(uint32_t);

        // Test sizes that are blocked by the HeapArray implementation before trying to allocate,
        // and one element smaller so that HeapArray will actually try to allocate.
        for (size_t leaveSpace : {0u, 1022u, 1023u}) {
            HeapArray<uint32_t> arr{kMaxSize - leaveSpace, std::nothrow};
            PreventElidingAllocation(arr);
            EXPECT_FALSE(bool{arr});
        }

        // Cases where size * sizeof(val) is larger than the address space and could overflow.
        {
            HeapArray<uint32_t> arr{kMaxSize + 1, std::nothrow};
            EXPECT_FALSE(bool{arr});
        }
        {
            HeapArray<uint32_t> arr{kMaxSize + 2, std::nothrow};
            EXPECT_FALSE(bool{arr});
        }
    }
}

// 64-bit size is way too large to allocate (but well within size_t on 64-bit).
TEST_F(HeapArrayTest, OutOfMemory64) {
    using Key64 = TypedInteger<struct Key64T, uint64_t>;
    static constexpr Key64 kHugeSize{0x1000'0000'0000'0000u};

    {
        ityp::HeapArray<Key64, Val> arr{kHugeSize, std::nothrow};
        PreventElidingAllocation(arr);
        EXPECT_FALSE(bool{arr});
    }
    {
        auto arr =
            // SAFETY: Testing unsafe API.
            DAWN_UNSAFE_BUFFERS(ityp::HeapArray<Key64, int>::Uninit(kHugeSize, std::nothrow));
        PreventElidingAllocation(arr);
        EXPECT_FALSE(bool{arr});
    }
}

// Test allocations close to the size of the address space.
TEST_F(HeapArrayDeathTest, OutOfMemoryAtLimit) {
    // Allocation of uint8_t values
    {
        // Exact maximum amount that will fit in the address space
        EXPECT_DEATH_IF_SUPPORTED(
            PreventElidingAllocation(HeapArray<uint8_t>{std::numeric_limits<size_t>::max()}), "");
        // And a bit less.
        EXPECT_DEATH_IF_SUPPORTED(
            PreventElidingAllocation(HeapArray<uint8_t>{std::numeric_limits<size_t>::max() - 4095}),
            "");
    }
    // Allocation of uint32_t values
    {
        static constexpr size_t kMaxSize = std::numeric_limits<size_t>::max() / sizeof(uint32_t);
        // Maximum amount that will fit in the address space
        EXPECT_DEATH_IF_SUPPORTED(PreventElidingAllocation(HeapArray<uint32_t>{kMaxSize}), "");
        // And a bit less.
        EXPECT_DEATH_IF_SUPPORTED(PreventElidingAllocation(HeapArray<uint32_t>{kMaxSize - 1023}),
                                  "");

        // Cases where size * sizeof(val) is larger than the address space and could overflow.
        EXPECT_DEATH_IF_SUPPORTED(PreventElidingAllocation(HeapArray<uint32_t>{kMaxSize + 1}), "");
        EXPECT_DEATH_IF_SUPPORTED(PreventElidingAllocation(HeapArray<uint32_t>{kMaxSize + 2}), "");
    }
}

// 64-bit size is way too large to allocate (but well within size_t on 64-bit).
TEST_F(HeapArrayDeathTest, OutOfMemory64) {
    using Key64 = TypedInteger<struct Key64T, uint64_t>;
    static constexpr Key64 kHugeSize{0x1000'0000'0000'0000u};

    EXPECT_DEATH_IF_SUPPORTED(PreventElidingAllocation(ityp::HeapArray<Key64, Val>{kHugeSize}), "");
    EXPECT_DEATH_IF_SUPPORTED(
        PreventElidingAllocation(
            // SAFETY: Testing unsafe API.
            DAWN_UNSAFE_BUFFERS(ityp::HeapArray<Key64, int>::Uninit(kHugeSize))),
        "");
}

// Out of bounds accesses should crash even in release (the underlying container
// should have asserts enabled).
TEST_F(HeapArrayDeathTest, OutOfBounds) {
    // MSVC doesn't have asserts (without _MSVC_STL_HARDENING).
    if constexpr (DAWN_COMPILER_IS(MSVC)) {
        GTEST_SKIP();
    }

    HeapArrayVal arr{Index{10u}};
    EXPECT_DEATH_IF_SUPPORTED(arr[Index{10u}], "");

    const HeapArrayVal& constArr = arr;
    EXPECT_DEATH_IF_SUPPORTED(constArr[Index{10u}], "");
}

// If the index/size is 64-bit, it needs to be narrowed to size_t. Verify that's checked correctly.
TEST_F(HeapArrayDeathTest, OversizedIndex) {
    // These tests are only relevant on 32-bit builds.
    if constexpr (sizeof(size_t) > sizeof(uint32_t)) {
        GTEST_SKIP();
    }

    using Key64 = TypedInteger<struct Key64T, uint64_t>;
    static constexpr Key64 kHugeKey64{0x1000'0000'0000'0000u};

    // Crash either due to OOM (on 64-bit) or due to narrowing (on 32-bit).
    EXPECT_DEATH_IF_SUPPORTED((ityp::HeapArray<Key64, Val>(kHugeKey64)), "");

    ityp::HeapArray<Key64, Val> vec(Key64{10u});

    vec[Key64{9u}];
    // Regular out-of-bounds.
    EXPECT_DEATH_IF_SUPPORTED(vec[Key64{10u}], "");

    vec[Key64{0u}];
    // If this were cast to a 32-bit size_t without a check, it would be in-bounds.
    EXPECT_DEATH_IF_SUPPORTED(vec[kHugeKey64], "");
}

// Using uninitialized memory should crash on MSan builds.
TEST_F(HeapArrayDeathTest, ReadUninitWithPODType) {
    if constexpr (!DAWN_MSAN_ENABLED()) {
        GTEST_SKIP();
    }

    // SAFETY: Testing unsafe API.
    auto arr = DAWN_UNSAFE_BUFFERS(HeapArrayPOD::Uninit(Index{1u}));
    EXPECT_DEATH_IF_SUPPORTED([&]() { printf("%d\n", arr[Index{0u}]); }(), "");
}

}  // anonymous namespace
}  // namespace dawn
