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

#include <algorithm>
#include <array>
#include <vector>

#include "gmock/gmock.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/utils/gtest.h"
#include "src/utils/span.h"
#include "src/utils/typed_integer.h"

namespace dawn {
namespace {

using testing::ElementsAreArray;

static constexpr std::array<int, 5> kSpanData = {1, 2, 3, 4, 5};

using Index = TypedInteger<struct IndexT, uint32_t>;
using Index8 = TypedInteger<struct IndexT, uint8_t>;
using Index64 = TypedInteger<struct IndexT, uint64_t>;

struct FakeRange {
    size_t size() const { return kSpanData.size(); }
    const int* data() const { return kSpanData.data(); }
    auto begin() { return kSpanData.begin(); }
    auto end() { return kSpanData.end(); }
};

struct FakeTypedRange {
    Index size() const {
        return Index{uint32_t{kSpanData.size()}};
    }
    const int* data() const { return kSpanData.data(); }
    auto begin() { return kSpanData.begin(); }
    auto end() { return kSpanData.end(); }
};

struct FakeTyped64Range {
    Index64 size() const {
        return Index64{uint64_t{kSpanData.size()}};
    }
    const int* data() const { return kSpanData.data(); }
    auto begin() { return kSpanData.begin(); }
    auto end() { return kSpanData.end(); }
};

TEST(SpanTest, Constructor_Default) {
    {
        Span<int> sp;
        EXPECT_EQ(sp.size(), 0u);
        EXPECT_EQ(sp.data(), nullptr);
        EXPECT_TRUE(sp.empty());
        EXPECT_EQ(sp.size_bytes(), 0u);
    }
    {
        Span<int, 0> sp;
        EXPECT_EQ(sp.size(), 0u);
        EXPECT_EQ(sp.data(), nullptr);
        EXPECT_TRUE(sp.empty());
        EXPECT_EQ(sp.size_bytes(), 0u);
    }
    {
        Span<int, 5> sp;
        EXPECT_EQ(sp.size(), 0u);
        EXPECT_EQ(sp.data(), nullptr);
        EXPECT_TRUE(sp.empty());
        EXPECT_EQ(sp.size_bytes(), 0u);

        Span<const int, 5> sp_const = sp;
        EXPECT_EQ(sp_const.size(), 0u);
        EXPECT_EQ(sp_const.data(), nullptr);
        EXPECT_TRUE(sp_const.empty());
        EXPECT_EQ(sp_const.size_bytes(), 0u);
    }
    {
        ityp::span<Index, int> sp;
        EXPECT_EQ(sp.size(), Index{0u});
        EXPECT_EQ(sp.data(), nullptr);
        EXPECT_TRUE(sp.empty());
        EXPECT_EQ(sp.size_bytes(), 0u);
    }
    {
        ityp::span<Index, int, Index{0u}> sp;
        EXPECT_EQ(sp.size(), Index{0u});
        EXPECT_EQ(sp.data(), nullptr);
        EXPECT_TRUE(sp.empty());
        EXPECT_EQ(sp.size_bytes(), 0u);
    }
    {
        ityp::span<Index, int, Index{5u}> sp;
        EXPECT_EQ(sp.size(), Index{0u});
        EXPECT_EQ(sp.data(), nullptr);
        EXPECT_TRUE(sp.empty());
        EXPECT_EQ(sp.size_bytes(), 0u);

        ityp::span<Index, const int, Index{5u}> sp_const = sp;
        EXPECT_EQ(sp_const.size(), Index{0u});
        EXPECT_EQ(sp_const.data(), nullptr);
        EXPECT_TRUE(sp_const.empty());
        EXPECT_EQ(sp_const.size_bytes(), 0u);
    }
}

TEST(SpanDeathTest, Constructor_DefaultFixedSpan) {
    {
        Span<int, 5> sp;
        EXPECT_DEATH_IF_SUPPORTED(sp.front(), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.back(), "");
        EXPECT_DEATH_IF_SUPPORTED(sp[0], "");
        EXPECT_DEATH_IF_SUPPORTED(sp.at(0), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.begin(), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.first(1), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.last(1), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.subspan(1), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.subspan(0, 1), "");
    }
    {
        ityp::span<Index, int, Index{5u}> sp;
        EXPECT_DEATH_IF_SUPPORTED(sp.front(), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.back(), "");
        EXPECT_DEATH_IF_SUPPORTED(sp[Index{0u}], "");
        EXPECT_DEATH_IF_SUPPORTED(sp.at(Index{0u}), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.begin(), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.first(Index{1u}), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.last(Index{1u}), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.subspan(Index{1u}), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.subspan(Index{0u}, Index{1u}), "");
    }
}

TEST(SpanTest, Constructor_PointerAndSize) {
    int data[] = {1, 2, 3};
    const int constData[] = {1, 2, 3};
    raw_ptr<int> rptr = data;

    // T* + size for Span<T>
    {
        // SAFETY: Test for the unsafe constructor.
        Span<int> DAWN_UNSAFE_BUFFERS(sp{&data[0], 3});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data);
    }
    // const T* + size for Span<const T>
    {
        // SAFETY: Test for the unsafe constructor.
        Span<const int> DAWN_UNSAFE_BUFFERS(sp{&constData[0], 3});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), constData);
    }
    // T* + size for Span<const T>
    {
        // SAFETY: Test for the unsafe constructor.
        Span<const int> DAWN_UNSAFE_BUFFERS(sp{&data[0], 3});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data);
    }

    // T* for Span<T, N>
    {
        // SAFETY: Test for the unsafe constructor.
        Span<int, 3> DAWN_UNSAFE_BUFFERS(sp{&data[0]});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data);
    }
    // const T* for Span<const T, N>
    {
        // SAFETY: Test for the unsafe constructor.
        Span<const int, 3> DAWN_UNSAFE_BUFFERS(sp{&constData[0]});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), constData);
    }
    // T* for Span<const T, N>
    {
        // SAFETY: Test for the unsafe constructor.
        Span<const int, 3> DAWN_UNSAFE_BUFFERS(sp{&data[0]});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data);
    }

    // T* + size for ityp::span<T, ...>
    {
        // SAFETY: Test for the unsafe constructor.
        ityp::span<Index, int> DAWN_UNSAFE_BUFFERS(sp{&data[0], Index{3u}});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), data);
    }
    // const T* + size for ityp::span<const T, ...>
    {
        // SAFETY: Test for the unsafe constructor.
        ityp::span<Index, const int> DAWN_UNSAFE_BUFFERS(sp{&constData[0], Index{3u}});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), constData);
    }
    // T* + size for ityp::span<const T, ...>
    {
        // SAFETY: Test for the unsafe constructor.
        ityp::span<Index, const int> DAWN_UNSAFE_BUFFERS(sp{&data[0], Index{3u}});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), data);
    }

    // T* for ityp::span<T, ..., N>
    {
        // SAFETY: Test for the unsafe constructor.
        ityp::span<Index, int, Index{3u}> DAWN_UNSAFE_BUFFERS(sp{&data[0]});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), data);
    }
    // const T* for ityp::span<const T, ..., N>
    {
        // SAFETY: Test for the unsafe constructor.
        ityp::span<Index, const int, Index{3u}> DAWN_UNSAFE_BUFFERS(sp{&constData[0]});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), constData);
    }
    // T* for ityp::span<const T, ..., N>
    {
        // SAFETY: Test for the unsafe constructor.
        ityp::span<Index, const int, Index{3u}> DAWN_UNSAFE_BUFFERS(sp{&data[0]});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), data);
    }
}

TEST(SpanDeathTest, Constructor_PointerAndSizeOversizedIndex) {
    // These tests are only relevant on 32-bit builds.
    if constexpr (sizeof(size_t) > sizeof(uint32_t)) {
        GTEST_SKIP();
    }

    constexpr Index64 kHugeSize{0x1'0000'0000LLU};
    // SAFETY: Test for the unsafe constructor.
    DAWN_UNSAFE_BUFFERS(EXPECT_DEATH_IF_SUPPORTED(
        (ityp::span<Index64, const int>(kSpanData.data(), kHugeSize)), ""));
}

TEST(SpanDeathTest, Constructor_PointerAndSizeDynamicExtent) {
    // DynamicExtent is invalid because it's reserved as a sentinel value.
    // SAFETY: Test for the unsafe constructor.
    DAWN_UNSAFE_BUFFERS(EXPECT_DEATH_IF_SUPPORTED(
        (ityp::span<Index8, const int>(kSpanData.data(), detail::DynamicExtent<Index8>)), ""));
}

TEST(SpanTest, Constructor_TwoIterators) {
    std::array<int, 3> data = {1, 2, 3};
    const std::array<int, 3> constData = {1, 2, 3};

    // 2 x iterator for Span<T>
    {
        // SAFETY: Test for the unsafe constructor.
        Span<int> DAWN_UNSAFE_BUFFERS(sp{data.begin(), data.end()});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data.data());
    }
    // 2 x const_iterator for Span<const T>
    {
        // SAFETY: Test for the unsafe constructor.
        Span<const int> DAWN_UNSAFE_BUFFERS(sp{constData.begin(), constData.end()});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), constData.data());
    }
    // 2 x iterator for Span<const T>
    {
        // SAFETY: Test for the unsafe constructor.
        Span<const int> DAWN_UNSAFE_BUFFERS(sp{data.begin(), data.end()});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data.data());
    }

    // 2 x iterator for Span<T, N>
    {
        // SAFETY: Test for the unsafe constructor.
        Span<int, 3> DAWN_UNSAFE_BUFFERS(sp{data.begin(), data.end()});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data.data());
    }
    // 2 x const_iterator for Span<const T, N>
    {
        // SAFETY: Test for the unsafe constructor.
        Span<const int, 3> DAWN_UNSAFE_BUFFERS(sp{constData.begin(), constData.end()});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), constData.data());
    }
    // 2 x iterator for Span<const T, N>
    {
        // SAFETY: Test for the unsafe constructor.
        Span<const int, 3> DAWN_UNSAFE_BUFFERS(sp{data.begin(), data.end()});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data.data());
    }

    // 2 x iterator for ityp::span<T, ...>
    {
        // SAFETY: Test for the unsafe constructor.
        ityp::span<Index, int> DAWN_UNSAFE_BUFFERS(sp{data.begin(), data.end()});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), data.data());
    }
    // 2 x const_iterator for ityp::span<const T, ...>
    {
        // SAFETY: Test for the unsafe constructor.
        ityp::span<Index, const int> DAWN_UNSAFE_BUFFERS(sp{constData.begin(), constData.end()});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), constData.data());
    }
    // 2 x iterator for ityp::span<const T, ...>
    {
        // SAFETY: Test for the unsafe constructor.
        ityp::span<Index, const int> DAWN_UNSAFE_BUFFERS(sp{data.begin(), data.end()});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), data.data());
    }

    // 2 x iterator for ityp::span<T, ..., N>
    {
        // SAFETY: Test for the unsafe constructor.
        ityp::span<Index, int, Index{3u}> DAWN_UNSAFE_BUFFERS(sp{data.begin(), data.end()});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), data.data());
    }
    // 2 x const_iterator for ityp::span<const T, ..., N>
    {
        // SAFETY: Test for the unsafe constructor.
        ityp::span<Index, const int, Index{3u}> DAWN_UNSAFE_BUFFERS(
            sp{constData.begin(), constData.end()});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), constData.data());
    }
    // 2 x iterator for ityp::span<const T, ..., N>
    {
        // SAFETY: Test for the unsafe constructor.
        ityp::span<Index, const int, Index{3u}> DAWN_UNSAFE_BUFFERS(sp{data.begin(), data.end()});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), data.data());
    }
}

TEST(SpanDeathTest, Constructor_TwoIteratorsInverted) {
    std::array<int, 3> data = {1, 2, 3};
    // SAFETY: Test for the unsafe constructor.
    DAWN_UNSAFE_BUFFERS(EXPECT_DEATH_IF_SUPPORTED((Span<int>{data.end(), data.begin()}), ""));
    // SAFETY: Test for the unsafe constructor.
    DAWN_UNSAFE_BUFFERS(EXPECT_DEATH_IF_SUPPORTED((Span<int, 3>{data.end(), data.begin()}), ""));
}

TEST(SpanDeathTest, Constructor_TwoIteratorsLargerThanIndexType) {
    std::vector<int> data;
    data.resize(256);  // Larger than indices that can be store in a uint8_t.

    // SAFETY: Test for the unsafe constructor.
    DAWN_UNSAFE_BUFFERS(
        EXPECT_DEATH_IF_SUPPORTED((ityp::span<Index8, int>(data.begin(), data.end())), ""));

    // 255 is invalid because it's reserved as a sentinel value.
    data.resize(255);
    // SAFETY: Test for the unsafe constructor.
    DAWN_UNSAFE_BUFFERS(
        EXPECT_DEATH_IF_SUPPORTED((ityp::span<Index8, int>(data.begin(), data.end())), ""));

    // Fits in uint8_t for indexing without matching DynamicExtent.
    data.resize(254);

    // SAFETY: Test for the unsafe constructor.
    DAWN_UNSAFE_BUFFERS((ityp::span<Index8, int>(data.begin(), data.end())));
}

std::span<std::byte> GetByteSpan() {
    return {};
}

TEST(SpanTest, Constructor_CompatibleRange) {
    {
        std::array<int, 3> data = {1, 2, 3};
        Span<int> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
    {
        const std::array<int, 3> data = {1, 2, 3};
        Span<const int> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
    {
        std::array<int, 3> data = {1, 2, 3};
        Span<int, 3> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
    {
        const std::array<int, 3> data = {1, 2, 3};
        Span<const int, 3> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
    {
        int data[] = {1, 2, 3};
        Span<int, 3> sp{data};
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data);
        static_assert(sizeof(sp) == sizeof(int*));
    }
    {
        const int data[] = {1, 2, 3};
        Span<const int, 3> sp{data};
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data);
        static_assert(sizeof(sp) == sizeof(const int*));
    }
    {
        std::vector<int> data{{1, 2, 3}};
        Span<const int> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
    {
        std::string data = "foo";
        Span<char> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
    {
        std::string_view data = "foo";
        Span<const char> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
    {
        FakeRange data;
        Span<const int> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
    {
        // Fails to compile if the constructor from a range is written to take a reference
        // (R& range) and not an rvalue reference (R&& range).
        Span<std::byte> sp;
        sp = GetByteSpan();
    }

    {
        FakeTypedRange data;
        ityp::span<Index, const int> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
}

TEST(SpanTest, Constructor_DynamicToFixed) {
    // Span<T> -> Span<T, N>
    {
        std::array<int, 3> data = {1, 2, 3};
        Span<int> sp(data);
        Span<int, 3> fixed(sp);
        EXPECT_EQ(fixed.size(), 3u);
        EXPECT_EQ(fixed.data(), data.data());
        EXPECT_EQ(fixed[0], 1);
        EXPECT_EQ(fixed[1], 2);
        EXPECT_EQ(fixed[2], 3);
    }
    // Span<const T> -> Span<const T, N>
    {
        FakeRange data;
        Span<const int> sp(data);
        Span<const int, 5> fixed(sp);
        EXPECT_EQ(fixed.size(), 5u);
        EXPECT_EQ(fixed.data(), data.data());
        EXPECT_EQ(fixed[0], 1);
        EXPECT_EQ(fixed[1], 2);
        EXPECT_EQ(fixed[2], 3);
    }
    // ityp::span<Index, const T> -> ityp::span<Index, const T, N>
    {
        FakeTypedRange data;
        ityp::span<Index, const int> sp(data);
        ityp::span<Index, const int, Index{5u}> fixed(sp);
        EXPECT_EQ(fixed.size(), Index{5u});
        EXPECT_EQ(fixed.data(), data.data());
        EXPECT_EQ(fixed[Index{0u}], 1);
        EXPECT_EQ(fixed[Index{1u}], 2);
        EXPECT_EQ(fixed[Index{2u}], 3);
    }
    // Empty dynamic span -> Span<T, 0>
    {
        Span<int> sp;
        Span<int, 0> fixed(sp);
        EXPECT_EQ(fixed.size(), 0u);
        EXPECT_EQ(fixed.data(), nullptr);
        EXPECT_TRUE(fixed.empty());
    }
    // Empty dynamic const span -> Span<const T, 0>
    {
        Span<const int> sp;
        Span<const int, 0> fixed(sp);
        EXPECT_EQ(fixed.size(), 0u);
        EXPECT_EQ(fixed.data(), nullptr);
        EXPECT_TRUE(fixed.empty());
    }
    // Empty dynamic typed span -> ityp::span<Index, T, 0>
    {
        ityp::span<Index, int> sp;
        ityp::span<Index, int, Index{0u}> fixed(sp);
        EXPECT_EQ(fixed.size(), Index{0u});
        EXPECT_EQ(fixed.data(), nullptr);
        EXPECT_TRUE(fixed.empty());
    }
    // Empty dynamic const typed span -> ityp::span<Index, const T, 0>
    {
        ityp::span<Index, const int> sp;
        ityp::span<Index, const int, Index{0u}> fixed(sp);
        EXPECT_EQ(fixed.size(), Index{0u});
        EXPECT_EQ(fixed.data(), nullptr);
        EXPECT_TRUE(fixed.empty());
    }
}

TEST(SpanDeathTest, Constructor_DynamicToFixedSizeMismatch) {
    {
        std::array<int, 5> data = {1, 2, 3, 4, 5};
        FakeRange constData;
        Span<int> sp(data);
        Span<const int> constSp(constData);

        // Dynamic span larger than fixed extent.
        EXPECT_DEATH_IF_SUPPORTED((Span<int, 2>{sp}), "");
        EXPECT_DEATH_IF_SUPPORTED((Span<const int, 2>{constSp}), "");

        // Dynamic span smaller than fixed extent.
        EXPECT_DEATH_IF_SUPPORTED((Span<int, 10>{sp}), "");
        EXPECT_DEATH_IF_SUPPORTED((Span<const int, 10>{constSp}), "");

        // Empty dynamic span to non-zero fixed extent.
        Span<int> emptySp;
        Span<const int> emptyConstSp;
        EXPECT_DEATH_IF_SUPPORTED((Span<int, 3>{emptySp}), "");
        EXPECT_DEATH_IF_SUPPORTED((Span<const int, 3>{emptyConstSp}), "");
    }

    // Typed integer index tests.
    {
        FakeTypedRange data;
        ityp::span<Index, const int> constSp(data);

        // Dynamic span larger than fixed extent.
        EXPECT_DEATH_IF_SUPPORTED((ityp::span<Index, const int, Index{2u}>{constSp}), "");

        // Dynamic span smaller than fixed extent.
        EXPECT_DEATH_IF_SUPPORTED((ityp::span<Index, const int, Index{10u}>{constSp}), "");

        // Empty dynamic span to non-zero fixed extent.
        ityp::span<Index, int> emptySp;
        ityp::span<Index, const int> emptyConstSp;
        EXPECT_DEATH_IF_SUPPORTED((ityp::span<Index, int, Index{3u}>{emptySp}), "");
        EXPECT_DEATH_IF_SUPPORTED((ityp::span<Index, const int, Index{3u}>{emptyConstSp}), "");
    }
}

TEST(SpanTest, CopyConstructor) {
    std::array<int, 3> data = {1, 2, 3};
    Span<int, 3> sp{data};

    Span<int> sp2{sp};
    ASSERT_EQ(sp.data(), sp2.data());
    ASSERT_EQ(sp.size(), sp2.size());

    // Actually calls the constructor from a range.
    Span<const int> sp3{sp2};
    ASSERT_EQ(sp.data(), sp3.data());
    ASSERT_EQ(sp.size(), sp3.size());

    // Actually calls the constructor from a range.
    Span<const int, 3> sp4{sp};
    ASSERT_EQ(sp.data(), sp4.data());
    ASSERT_EQ(sp.size(), sp4.size());
}

TEST(SpanTest, CopyAssignment) {
    std::array<int, 3> data = {1, 2, 3};
    Span<int, 3> sp{data};

    Span<int> sp2;
    sp2 = sp;
    ASSERT_EQ(sp.data(), sp2.data());
    ASSERT_EQ(sp.size(), sp2.size());

    // Actually calls the constructor from a range and then assigns.
    Span<const int> sp3;
    sp3 = sp2;
    ASSERT_EQ(sp.data(), sp3.data());
    ASSERT_EQ(sp.size(), sp3.size());
}

TEST(SpanTest, MoveConstructor) {
    std::array<int, 3> data = {1, 2, 3};
    Span<int> sp{data};

    Span<int> sp2{std::move(sp)};
    ASSERT_EQ(data.data(), sp2.data());
    ASSERT_EQ(data.size(), sp2.size());

    // "Default move constructor" copies so sp stays the same.
    ASSERT_EQ(data.data(), sp.data());
    ASSERT_EQ(data.size(), sp.size());
}

TEST(SpanTest, MoveAssignment) {
    std::array<int, 3> data = {1, 2, 3};
    Span<int> sp{data};

    Span<int> sp2;
    sp2 = std::move(sp);
    ASSERT_EQ(data.data(), sp2.data());
    ASSERT_EQ(data.size(), sp2.size());

    // "Default move assignment" copies so sp stays the same.
    ASSERT_EQ(data.data(), sp.data());
    ASSERT_EQ(data.size(), sp.size());
}

TEST(SpanTest, BeginEnd) {
    {
        Span<const int> sp(FakeRange{});
        ASSERT_EQ(&*sp.begin(), &*kSpanData.begin());
        ASSERT_EQ(&*sp.end(), &*kSpanData.end());
    }
    {
        Span<const int, 5> sp(FakeRange{});
        ASSERT_EQ(&*sp.begin(), &*kSpanData.begin());
        ASSERT_EQ(&*sp.end(), &*kSpanData.end());
    }
    {
        ityp::span<Index, const int> sp(FakeTypedRange{});
        ASSERT_EQ(&*sp.begin(), &*kSpanData.begin());
        ASSERT_EQ(&*sp.end(), &*kSpanData.end());
    }
    {
        ityp::span<Index, const int, Index{5u}> sp(FakeTypedRange{});
        ASSERT_EQ(&*sp.begin(), &*kSpanData.begin());
        ASSERT_EQ(&*sp.end(), &*kSpanData.end());
    }
}

TEST(SpanTest, BeginEndForIteration) {
    // Uses begin/end
    {
        Span<const int> sp(FakeRange{});

        int expected = 1;
        for (const int& i : sp) {
            EXPECT_EQ(i, expected);
            expected++;
        }
    }
    {
        Span<const int, 5> sp(FakeRange{});

        int expected = 1;
        for (const int& i : sp) {
            EXPECT_EQ(i, expected);
            expected++;
        }
    }

    // Uses cbegin/cend
    {
        const Span<const int> sp(FakeRange{});

        int expected = 1;
        for (const int& i : sp) {
            EXPECT_EQ(i, expected);
            expected++;
        }
    }
    {
        const Span<const int, 5> sp(FakeRange{});

        int expected = 1;
        for (const int& i : sp) {
            EXPECT_EQ(i, expected);
            expected++;
        }
    }

    // ityp, uses begin/end
    {
        ityp::span<Index, const int> sp(FakeTypedRange{});

        int expected = 1;
        for (const int& i : sp) {
            EXPECT_EQ(i, expected);
            expected++;
        }
    }
    {
        ityp::span<Index, const int, Index{5u}> sp(FakeTypedRange{});

        int expected = 1;
        for (const int& i : sp) {
            EXPECT_EQ(i, expected);
            expected++;
        }
    }

    // ityp, uses cbegin/cend
    {
        const ityp::span<Index, const int> sp(FakeTypedRange{});

        int expected = 1;
        for (const int& i : sp) {
            EXPECT_EQ(i, expected);
            expected++;
        }
    }
    {
        const ityp::span<Index, const int, Index{5u}> sp(FakeTypedRange{});

        int expected = 1;
        for (const int& i : sp) {
            EXPECT_EQ(i, expected);
            expected++;
        }
    }
}

TEST(SpanDeathTest, IteratorsAreHardened) {
    std::array<int, 4> src4{};
    std::array<int, 3> src3{};

    std::array<int, 4> dst{};
    Span<int> dstSpan = Span<int>(dst).first(3u);

    // Control case, copying 3 elements to dst is valid.
    std::ranges::copy(src3, dstSpan.begin());

    // We check specifically that libc++'s bounded iterators are used as iterators for dawn::Span.
#if defined(_LIBCPP_ABI_BOUNDED_ITERATORS)
    // Error case, the hardened iterator will fail when we try to write a non-existent 4th element.
    EXPECT_DEATH_IF_SUPPORTED(std::ranges::copy(src4, dstSpan.begin()), "");
#else
    // Still a success case because we don't have bounded iterators.
    std::ranges::copy(src4, dstSpan.begin());
#endif
}

TEST(SpanTest, FrontBack) {
    {
        Span<const int> sp(FakeRange{});
        EXPECT_EQ(&sp.front(), &kSpanData.front());
        EXPECT_EQ(&sp.back(), &kSpanData.back());
    }
    {
        Span<const int, 5> sp(FakeRange{});
        EXPECT_EQ(&sp.front(), &kSpanData.front());
        EXPECT_EQ(&sp.back(), &kSpanData.back());
    }
    {
        ityp::span<Index, const int> sp(FakeTypedRange{});
        EXPECT_EQ(&sp.front(), &kSpanData.front());
        EXPECT_EQ(&sp.back(), &kSpanData.back());
    }
    {
        ityp::span<Index, const int, Index{5u}> sp(FakeTypedRange{});
        EXPECT_EQ(&sp.front(), &kSpanData.front());
        EXPECT_EQ(&sp.back(), &kSpanData.back());
    }
}

TEST(SpanDeathTest, FrontBackOfEmpty) {
    {
        Span<const int> sp;
        EXPECT_DEATH_IF_SUPPORTED(sp.front(), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.back(), "");
    }
    {
        Span<const int, 0> sp;
        EXPECT_DEATH_IF_SUPPORTED(sp.front(), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.back(), "");
    }
}

TEST(SpanTest, Indexing) {
    {
        Span<const int> sp(FakeRange{});
        for (size_t i = 0; i < kSpanData.size(); i++) {
            EXPECT_EQ(&sp.at(i), &kSpanData[i]);
            EXPECT_EQ(&sp[i], &kSpanData[i]);
        }
    }
    {
        Span<const int, 5> sp(FakeRange{});
        for (size_t i = 0; i < kSpanData.size(); i++) {
            EXPECT_EQ(&sp.at(i), &kSpanData[i]);
            EXPECT_EQ(&sp[i], &kSpanData[i]);
        }
    }
    {
        ityp::span<Index, const int> sp(FakeTypedRange{});
        for (size_t i = 0; i < kSpanData.size(); i++) {
            Index id{static_cast<uint32_t>(i)};
            EXPECT_EQ(&sp.at(id), &kSpanData[i]);
            EXPECT_EQ(&sp[id], &kSpanData[i]);
        }
    }
    {
        ityp::span<Index, const int, Index{5u}> sp(FakeTypedRange{});
        for (size_t i = 0; i < kSpanData.size(); i++) {
            Index id{static_cast<uint32_t>(i)};
            EXPECT_EQ(&sp.at(id), &kSpanData[i]);
            EXPECT_EQ(&sp[id], &kSpanData[i]);
        }
    }
}

TEST(SpanDeathTest, IndexingOOB) {
    Span<const int> sp(FakeRange{});
    EXPECT_DEATH_IF_SUPPORTED(sp.at(sp.size()), "");
    EXPECT_DEATH_IF_SUPPORTED(sp[sp.size()], "");

    Span<const int> spEmpty;
    EXPECT_DEATH_IF_SUPPORTED(spEmpty.at(0u), "");
    EXPECT_DEATH_IF_SUPPORTED(spEmpty[0u], "");

    Span<const int, 0> spFixedEmpty;
    EXPECT_DEATH_IF_SUPPORTED(spFixedEmpty.at(0u), "");
    EXPECT_DEATH_IF_SUPPORTED(spFixedEmpty[0u], "");
}

TEST(SpanDeathTest, IndexingOversizedIndex) {
    // These tests are only relevant on 32-bit builds.
    if constexpr (sizeof(size_t) > sizeof(uint32_t)) {
        GTEST_SKIP();
    }

    auto sp = ityp::span<Index64, const int>(FakeTyped64Range());

    // The narrowing to size_t would give 0 which is in bounds, so this checks that the cast to
    // size_t itself causes a crash.
    constexpr Index64 kHugeIndex{0x1'0000'0000LLU};
    EXPECT_DEATH_IF_SUPPORTED(sp[kHugeIndex], "");
}

// .data() and .size() are tested in every test essentially.

TEST(SpanTest, Empty) {
    ASSERT_FALSE(Span<const int>{FakeRange{}}.empty());
    // SAFETY: Test for the unsafe constructor.
    ASSERT_FALSE(DAWN_UNSAFE_BUFFERS((Span<const int>{static_cast<int*>(nullptr), 1u})).empty());
    ASSERT_TRUE(Span<const int>{}.empty());
    // SAFETY: Test for the unsafe constructor.
    ASSERT_TRUE(DAWN_UNSAFE_BUFFERS((Span<const int>{kSpanData.data(), 0u})).empty());

    ASSERT_TRUE((Span<const int, 0>{}.empty()));
    // SAFETY: Test for the unsafe constructor.
    ASSERT_TRUE(DAWN_UNSAFE_BUFFERS((Span<const int, 0>{kSpanData.data()})).empty());

    ASSERT_FALSE((ityp::span<Index, const int>{FakeTypedRange{}}.empty()));
    ASSERT_FALSE(
        // SAFETY: Test for the unsafe constructor.
        DAWN_UNSAFE_BUFFERS((ityp::span<Index, const int>{static_cast<int*>(nullptr), Index{1u}}))
            .empty());
    ASSERT_TRUE((ityp::span<Index, const int>{}.empty()));
    ASSERT_TRUE(
        // SAFETY: Test for the unsafe constructor.
        DAWN_UNSAFE_BUFFERS((ityp::span<Index, const int>{kSpanData.data(), Index{0u}})).empty());

    ASSERT_TRUE((ityp::span<Index, const int, Index{0u}>{}.empty()));
    ASSERT_TRUE(
        // SAFETY: Test for the unsafe constructor.
        DAWN_UNSAFE_BUFFERS((ityp::span<Index, const int, Index{0u}>{kSpanData.data(), Index{0u}}))
            .empty());
}

TEST(SpanTest, SizeBytes) {
    ASSERT_EQ(Span<int>{}.size_bytes(), 0u);
    ASSERT_EQ((ityp::span<Index, int>{}.size_bytes()), 0u);
    ASSERT_EQ((ityp::span<Index, int, Index{0u}>{}.size_bytes()), 0u);

    std::array<int, 3> ints{};
    ASSERT_EQ(Span<int>{ints}.size_bytes(), 3 * sizeof(int));
    ASSERT_EQ((Span<int, 3>{ints}.size_bytes()), 3 * sizeof(int));

    std::array<double, 10> doubles{};
    ASSERT_EQ(Span<double>{doubles}.size_bytes(), 10 * sizeof(double));
    ASSERT_EQ((Span<double, 10>{doubles}.size_bytes()), 10 * sizeof(double));
}

TEST(SpanTest, FirstLast) {
    {
        Span<const int> sp{FakeRange()};
        Span<const int> first0 = sp.first(0);
        Span<const int> first2 = sp.first(2);
        Span<const int> last0 = sp.last(0);
        Span<const int> last2 = sp.last(2);

        EXPECT_EQ(first0.data(), sp.data());
        EXPECT_EQ(first0.size(), 0u);
        EXPECT_EQ(first2.data(), sp.data());
        EXPECT_EQ(first2.size(), 2u);

        EXPECT_EQ(last0.data(), &*sp.end());
        EXPECT_EQ(last0.size(), 0u);
        EXPECT_EQ(last2.data(), &sp.at(sp.size() - 2));
        EXPECT_EQ(last2.size(), 2u);
    }
    {
        Span<const int, 5> sp{FakeRange()};
        Span<const int> first0 = sp.first(0);
        Span<const int> first2 = sp.first(2);
        Span<const int> last0 = sp.last(0);
        Span<const int> last2 = sp.last(2);

        EXPECT_EQ(first0.data(), sp.data());
        EXPECT_EQ(first0.size(), 0u);
        EXPECT_EQ(first2.data(), sp.data());
        EXPECT_EQ(first2.size(), 2u);

        EXPECT_EQ(last0.data(), &*sp.end());
        EXPECT_EQ(last0.size(), 0u);
        EXPECT_EQ(last2.data(), &sp.at(sp.size() - 2));
        EXPECT_EQ(last2.size(), 2u);
    }

    {
        ityp::span<Index, const int> sp{FakeTypedRange()};
        ityp::span<Index, const int> first0 = sp.first(Index{0u});
        ityp::span<Index, const int> first2 = sp.first(Index{2u});
        ityp::span<Index, const int> last0 = sp.last(Index{0u});
        ityp::span<Index, const int> last2 = sp.last(Index{2u});

        EXPECT_EQ(first0.data(), sp.data());
        EXPECT_EQ(first0.size(), Index{0u});
        EXPECT_EQ(first2.data(), sp.data());
        EXPECT_EQ(first2.size(), Index{2u});

        EXPECT_EQ(last0.data(), &*sp.end());
        EXPECT_EQ(last0.size(), Index{0u});
        EXPECT_EQ(last2.data(), &sp.at(sp.size() - Index{2u}));
        EXPECT_EQ(last2.size(), Index{2u});
    }
    {
        ityp::span<Index, const int, Index{5u}> sp{FakeTypedRange()};
        ityp::span<Index, const int> first0 = sp.first(Index{0u});
        ityp::span<Index, const int> first2 = sp.first(Index{2u});
        ityp::span<Index, const int> last0 = sp.last(Index{0u});
        ityp::span<Index, const int> last2 = sp.last(Index{2u});

        EXPECT_EQ(first0.data(), sp.data());
        EXPECT_EQ(first0.size(), Index{0u});
        EXPECT_EQ(first2.data(), sp.data());
        EXPECT_EQ(first2.size(), Index{2u});

        EXPECT_EQ(last0.data(), &*sp.end());
        EXPECT_EQ(last0.size(), Index{0u});
        EXPECT_EQ(last2.data(), &sp.at(sp.size() - Index{2u}));
        EXPECT_EQ(last2.size(), Index{2u});
    }
}

TEST(SpanDeathTest, FirstLastOOB) {
    Span<const int> sp{FakeRange()};

    sp.first(sp.size());
    EXPECT_DEATH_IF_SUPPORTED(sp.first(sp.size() + 1), "");

    sp.last(sp.size());
    EXPECT_DEATH_IF_SUPPORTED(sp.last(sp.size() + 1), "");
}

TEST(SpanTest, Subspan1Arg) {
    {
        Span<const int> sp{FakeRange()};
        Span<const int> subspan0 = sp.subspan(0);
        Span<const int> subspan2 = sp.subspan(2);

        EXPECT_EQ(subspan0.data(), sp.data());
        EXPECT_EQ(subspan0.size(), sp.size());
        EXPECT_EQ(subspan2.data(), &sp.at(2));
        EXPECT_EQ(subspan2.size(), sp.size() - 2);
    }
    {
        Span<const int, 5> sp{FakeRange()};
        Span<const int> subspan0 = sp.subspan(0);
        Span<const int> subspan2 = sp.subspan(2);

        EXPECT_EQ(subspan0.data(), sp.data());
        EXPECT_EQ(subspan0.size(), sp.size());
        EXPECT_EQ(subspan2.data(), &sp.at(2));
        EXPECT_EQ(subspan2.size(), sp.size() - 2);
    }
    {
        ityp::span<Index, const int> sp{FakeTypedRange()};
        ityp::span<Index, const int> subspan0 = sp.subspan(Index{0u});
        ityp::span<Index, const int> subspan2 = sp.subspan(Index{2u});

        EXPECT_EQ(subspan0.data(), sp.data());
        EXPECT_EQ(subspan0.size(), sp.size());
        EXPECT_EQ(subspan2.data(), &sp.at(Index{2u}));
        EXPECT_EQ(subspan2.size(), sp.size() - Index{2u});
    }
    {
        ityp::span<Index, const int, Index{5u}> sp{FakeTypedRange()};
        ityp::span<Index, const int> subspan0 = sp.subspan(Index{0u});
        ityp::span<Index, const int> subspan2 = sp.subspan(Index{2u});

        EXPECT_EQ(subspan0.data(), sp.data());
        EXPECT_EQ(subspan0.size(), sp.size());
        EXPECT_EQ(subspan2.data(), &sp.at(Index{2u}));
        EXPECT_EQ(subspan2.size(), sp.size() - Index{2u});
    }
}

TEST(SpanDeathTest, Subspan1ArgOOB) {
    {
        Span<const int> sp{FakeRange()};
        sp.subspan(sp.size());
        EXPECT_DEATH_IF_SUPPORTED(sp.subspan(sp.size() + 1), "");
    }
    {
        Span<const int, 5> sp{FakeRange()};
        sp.subspan(sp.size());
        EXPECT_DEATH_IF_SUPPORTED(sp.subspan(sp.size() + 1), "");
    }
}

TEST(SpanTest, Subspan2Args) {
    {
        Span<const int> sp{FakeRange()};
        Span<const int> subspan0_2 = sp.subspan(0, 2);
        Span<const int> subspan3_2 = sp.subspan(3, 2);

        EXPECT_EQ(subspan0_2.data(), sp.data());
        EXPECT_EQ(subspan0_2.size(), 2u);
        EXPECT_EQ(subspan3_2.data(), &sp.at(3));
        EXPECT_EQ(subspan3_2.size(), 2u);
    }
    {
        Span<const int, 5> sp{FakeRange()};
        Span<const int> subspan0_2 = sp.subspan(0, 2);
        Span<const int> subspan3_2 = sp.subspan(3, 2);

        EXPECT_EQ(subspan0_2.data(), sp.data());
        EXPECT_EQ(subspan0_2.size(), 2u);
        EXPECT_EQ(subspan3_2.data(), &sp.at(3));
        EXPECT_EQ(subspan3_2.size(), 2u);
    }

    {
        ityp::span<Index, const int> sp{FakeTypedRange()};
        ityp::span<Index, const int> subspan0_2 = sp.subspan(Index{0u}, Index{2u});
        ityp::span<Index, const int> subspan3_2 = sp.subspan(Index{3u}, Index{2u});

        EXPECT_EQ(subspan0_2.data(), sp.data());
        EXPECT_EQ(subspan0_2.size(), Index{2u});
        EXPECT_EQ(subspan3_2.data(), &sp.at(Index{3u}));
        EXPECT_EQ(subspan3_2.size(), Index{2u});
    }
    {
        ityp::span<Index, const int, Index{5u}> sp{FakeTypedRange()};
        ityp::span<Index, const int> subspan0_2 = sp.subspan(Index{0u}, Index{2u});
        ityp::span<Index, const int> subspan3_2 = sp.subspan(Index{3u}, Index{2u});

        EXPECT_EQ(subspan0_2.data(), sp.data());
        EXPECT_EQ(subspan0_2.size(), Index{2u});
        EXPECT_EQ(subspan3_2.data(), &sp.at(Index{3u}));
        EXPECT_EQ(subspan3_2.size(), Index{2u});
    }
}

TEST(SpanDeathTest, Subspan2ArgOOB) {
    Span<const int> sp{FakeRange()};

    sp.subspan(2, sp.size() - 2);
    EXPECT_DEATH_IF_SUPPORTED(sp.subspan(2, sp.size() - 1), "");

    // Check that overflows of offset + count is handled.
    EXPECT_DEATH_IF_SUPPORTED(sp.subspan(std::numeric_limits<size_t>::max(), 1), "");
    EXPECT_DEATH_IF_SUPPORTED(sp.subspan(1, std::numeric_limits<size_t>::max()), "");

    // SAFETY: This is the same range as kSpanData, just viewed with a uint8_t index (which fits the
    // size of kSpanData since it is 5).
    auto sp8 = DAWN_UNSAFE_BUFFERS(
        ityp::span<Index8, const int>(kSpanData.data(), Index8{uint8_t{kSpanData.size()}}));

    Index8 kOne = Index8{uint8_t{1}};
    Index8 kTwo = Index8{uint8_t{2}};
    sp8.subspan(kTwo, sp8.size() - kTwo);
    EXPECT_DEATH_IF_SUPPORTED(sp8.subspan(kTwo, sp8.size() - kOne), "");

    // Check that overflows of offset + count is handled.
    EXPECT_DEATH_IF_SUPPORTED(sp8.subspan(std::numeric_limits<Index8>::max(), kOne), "");
    EXPECT_DEATH_IF_SUPPORTED(sp8.subspan(kOne, std::numeric_limits<Index8>::max()), "");
}

TEST(SpanTest, SpanAsBytes) {
    // Empty spans.
    {
        Span<int> sp;
        auto bsp = SpanAsBytes(sp);
        static_assert(std::is_same_v<Span<const std::byte>, decltype(bsp)>);
        EXPECT_TRUE(bsp.empty());
        auto wbsp = SpanAsWritableBytes(sp);
        static_assert(std::is_same_v<Span<std::byte>, decltype(wbsp)>);
        EXPECT_TRUE(wbsp.empty());

        Span<volatile int> vsp;
        auto vbsp = SpanAsBytes(vsp);
        static_assert(std::is_same_v<Span<const volatile std::byte>, decltype(vbsp)>);
        EXPECT_TRUE(vbsp.empty());
        auto vwbsp = SpanAsWritableBytes(vsp);
        static_assert(std::is_same_v<Span<volatile std::byte>, decltype(vwbsp)>);
        EXPECT_TRUE(vwbsp.empty());
    }

    // Non-empty span.
    {
        std::array<int, 3> ints{};

        Span<int> sp{ints};
        auto bsp = SpanAsBytes(sp);
        static_assert(std::is_same_v<Span<const std::byte>, decltype(bsp)>);
        EXPECT_EQ(bsp.size(), sp.size_bytes());
        EXPECT_EQ(bsp.data(), reinterpret_cast<const std::byte*>(sp.data()));
        auto wbsp = SpanAsWritableBytes(sp);
        static_assert(std::is_same_v<Span<std::byte>, decltype(wbsp)>);
        EXPECT_EQ(wbsp.size(), sp.size_bytes());
        EXPECT_EQ(wbsp.data(), reinterpret_cast<std::byte*>(sp.data()));

        Span<volatile int> vsp{ints};
        auto vbsp = SpanAsBytes(vsp);
        static_assert(std::is_same_v<Span<const volatile std::byte>, decltype(vbsp)>);
        EXPECT_EQ(vbsp.size(), vsp.size_bytes());
        EXPECT_EQ(vbsp.data(), reinterpret_cast<const volatile std::byte*>(vsp.data()));
        auto vwbsp = SpanAsWritableBytes(vsp);
        static_assert(std::is_same_v<Span<volatile std::byte>, decltype(vwbsp)>);
        EXPECT_EQ(vwbsp.size(), vsp.size_bytes());
        EXPECT_EQ(vwbsp.data(), reinterpret_cast<volatile std::byte*>(vsp.data()));
    }

    // Fixed-extent span.
    {
        std::array<int, 3> ints{};

        Span<int, 3> sp{ints};
        auto bsp = SpanAsBytes(sp);
        static_assert(std::is_same_v<Span<const std::byte, sizeof(int) * 3u>, decltype(bsp)>);
        EXPECT_EQ(bsp.size(), sp.size_bytes());
        EXPECT_EQ(bsp.data(), reinterpret_cast<const std::byte*>(sp.data()));
        auto wbsp = SpanAsWritableBytes(sp);
        static_assert(std::is_same_v<Span<std::byte, sizeof(int) * 3u>, decltype(wbsp)>);
        EXPECT_EQ(wbsp.size(), sp.size_bytes());
        EXPECT_EQ(wbsp.data(), reinterpret_cast<std::byte*>(sp.data()));

        Span<volatile int, 3> vsp{ints};
        auto vbsp = SpanAsBytes(vsp);
        static_assert(
            std::is_same_v<Span<const volatile std::byte, sizeof(int) * 3u>, decltype(vbsp)>);
        EXPECT_EQ(vbsp.size(), vsp.size_bytes());
        EXPECT_EQ(vbsp.data(), reinterpret_cast<const volatile std::byte*>(vsp.data()));
        auto vwbsp = SpanAsWritableBytes(vsp);
        static_assert(std::is_same_v<Span<volatile std::byte, sizeof(int) * 3u>, decltype(vwbsp)>);
        EXPECT_EQ(vwbsp.size(), vsp.size_bytes());
        EXPECT_EQ(vwbsp.data(), reinterpret_cast<volatile std::byte*>(vsp.data()));
    }

    // Span with an index.
    {
        std::array<int, 3> ints{};

        // SAFETY: This is viewing ints, just with typed indices.
        ityp::span<Index, int> DAWN_UNSAFE_BUFFERS(sp{ints.data(), Index(3u)});
        auto bsp = SpanAsBytes(sp);
        static_assert(std::is_same_v<Span<const std::byte>, decltype(bsp)>);
        EXPECT_EQ(bsp.size(), sp.size_bytes());
        EXPECT_EQ(bsp.data(), reinterpret_cast<const std::byte*>(sp.data()));
        auto wbsp = SpanAsWritableBytes(sp);
        static_assert(std::is_same_v<Span<std::byte>, decltype(wbsp)>);
        EXPECT_EQ(wbsp.size(), sp.size_bytes());
        EXPECT_EQ(wbsp.data(), reinterpret_cast<std::byte*>(sp.data()));

        // SAFETY: This is viewing ints, just with typed indices.
        ityp::span<Index, volatile int> DAWN_UNSAFE_BUFFERS(vsp{ints.data(), Index(3u)});
        auto vbsp = SpanAsBytes(vsp);
        static_assert(std::is_same_v<Span<const volatile std::byte>, decltype(vbsp)>);
        EXPECT_EQ(vbsp.size(), vsp.size_bytes());
        EXPECT_EQ(vbsp.data(), reinterpret_cast<const volatile std::byte*>(vsp.data()));
        auto vwbsp = SpanAsWritableBytes(vsp);
        static_assert(std::is_same_v<Span<volatile std::byte>, decltype(vwbsp)>);
        EXPECT_EQ(vwbsp.size(), vsp.size_bytes());
        EXPECT_EQ(vwbsp.data(), reinterpret_cast<volatile std::byte*>(vsp.data()));
    }
}

TEST(SpanTest, ReinterpretEmptySpans) {
    // Non-const, non-volatile span.
    {
        Span<std::byte> bsp;
        auto sp = ReinterpretSpan<int>(bsp);
        static_assert(std::is_same_v<Span<int>, decltype(sp)>);
        EXPECT_TRUE(sp.empty());

        auto sp_const = ReinterpretSpan<const int>(bsp);
        static_assert(std::is_same_v<Span<const int>, decltype(sp_const)>);
        EXPECT_TRUE(sp_const.empty());

        auto sp_volatile = ReinterpretSpan<volatile int>(bsp);
        static_assert(std::is_same_v<Span<volatile int>, decltype(sp_volatile)>);
        EXPECT_TRUE(sp_volatile.empty());

        auto sp_const_volatile = ReinterpretSpan<const volatile int>(bsp);
        static_assert(std::is_same_v<Span<const volatile int>, decltype(sp_const_volatile)>);
        EXPECT_TRUE(sp_const_volatile.empty());
    }
    // Const span.
    {
        Span<const std::byte> bsp;
        auto sp_const = ReinterpretSpan<const int>(bsp);
        static_assert(std::is_same_v<Span<const int>, decltype(sp_const)>);
        EXPECT_TRUE(sp_const.empty());

        auto sp_const_volatile = ReinterpretSpan<const volatile int>(bsp);
        static_assert(std::is_same_v<Span<const volatile int>, decltype(sp_const_volatile)>);
        EXPECT_TRUE(sp_const_volatile.empty());
    }
    // Volatile span.
    {
        Span<volatile std::byte> bsp;
        auto sp_volatile = ReinterpretSpan<volatile int>(bsp);
        static_assert(std::is_same_v<Span<volatile int>, decltype(sp_volatile)>);
        EXPECT_TRUE(sp_volatile.empty());

        auto sp_const_volatile = ReinterpretSpan<const volatile int>(bsp);
        static_assert(std::is_same_v<Span<const volatile int>, decltype(sp_const_volatile)>);
        EXPECT_TRUE(sp_const_volatile.empty());
    }
    // Const and volatile span.
    {
        Span<const volatile std::byte> bsp;
        auto sp_const_volatile = ReinterpretSpan<const volatile int>(bsp);
        static_assert(std::is_same_v<Span<const volatile int>, decltype(sp_const_volatile)>);
        EXPECT_TRUE(sp_const_volatile.empty());
    }
}

TEST(SpanTest, ReintepretSpans) {
    // Basic usages with varying const/volatile and type-ness.
    {
        alignas(uint32_t) std::array<std::byte, 8> bytes{};
        Span<std::byte> bsp{bytes};

        auto sp16 = ReinterpretSpan<uint16_t>(bsp);
        static_assert(std::is_same_v<Span<uint16_t>, decltype(sp16)>);
        EXPECT_EQ(sp16.size(), 4u);
        EXPECT_EQ(sp16.data(), reinterpret_cast<uint16_t*>(bsp.data()));

        auto sp32 = ReinterpretSpan<const uint32_t>(bsp);
        static_assert(std::is_same_v<Span<const uint32_t>, decltype(sp32)>);
        EXPECT_EQ(sp32.size(), 2u);
        EXPECT_EQ(sp32.data(), reinterpret_cast<const uint32_t*>(bsp.data()));

        auto sp16_typed = ReinterpretSpan<volatile uint16_t, Index>(bsp);
        static_assert(std::is_same_v<ityp::span<Index, volatile uint16_t>, decltype(sp16_typed)>);
        EXPECT_EQ(sp16_typed.size(), Index{4u});
        EXPECT_EQ(sp16_typed.data(), reinterpret_cast<volatile uint16_t*>(bsp.data()));

        auto sp32_typed = ReinterpretSpan<const volatile uint32_t, Index>(bsp);
        static_assert(
            std::is_same_v<ityp::span<Index, const volatile uint32_t>, decltype(sp32_typed)>);
        EXPECT_EQ(sp32_typed.size(), Index{2u});
        EXPECT_EQ(sp32_typed.data(), reinterpret_cast<const volatile uint32_t*>(bsp.data()));
    }
    // Round-trip data integrity with SpanAs*Bytes.
    {
        std::array<int, 3> ints{1, 2, 3};
        Span<int> sp{ints};
        {
            Span<const std::byte> bsp = SpanAsBytes(sp);
            Span<const int> sp2 = ReinterpretSpan<const int>(bsp);
            EXPECT_EQ(sp2.size(), sp.size());
            EXPECT_EQ(sp2.data(), sp.data());
            EXPECT_TRUE(std::ranges::equal(sp2, sp));
        }
        {
            Span<std::byte> wbsp = SpanAsWritableBytes(sp);
            Span<int> sp2 = ReinterpretSpan<int>(wbsp);
            EXPECT_EQ(sp2.size(), sp.size());
            EXPECT_EQ(sp2.data(), sp.data());
            EXPECT_TRUE(std::ranges::equal(sp2, sp));
        }
    }
}

TEST(SpanTest, ReintepretFixedExtentSpans) {
    // Basic usages with varying const/volatile and type-ness.
    {
        alignas(uint32_t) std::array<std::byte, 8> bytes{};
        Span<std::byte, 8> bsp{bytes};

        auto sp16 = ReinterpretSpan<uint16_t>(bsp);
        static_assert(std::is_same_v<Span<uint16_t, 4>, decltype(sp16)>);
        EXPECT_EQ(sp16.data(), reinterpret_cast<uint16_t*>(bsp.data()));

        auto sp32 = ReinterpretSpan<const uint32_t>(bsp);
        static_assert(std::is_same_v<Span<const uint32_t, 2>, decltype(sp32)>);
        EXPECT_EQ(sp32.data(), reinterpret_cast<const uint32_t*>(bsp.data()));

        auto sp16_typed = ReinterpretSpan<volatile uint16_t, Index>(bsp);
        static_assert(
            std::is_same_v<ityp::span<Index, volatile uint16_t, Index{4u}>, decltype(sp16_typed)>);
        EXPECT_EQ(sp16_typed.data(), reinterpret_cast<volatile uint16_t*>(bsp.data()));

        auto sp32_typed = ReinterpretSpan<const volatile uint32_t, Index>(bsp);
        static_assert(std::is_same_v<ityp::span<Index, const volatile uint32_t, Index{2u}>,
                                     decltype(sp32_typed)>);
        EXPECT_EQ(sp32_typed.data(), reinterpret_cast<const volatile uint32_t*>(bsp.data()));
    }
    // Round-trip data integrity with SpanAs*Bytes.
    {
        std::array<int, 3> ints{1, 2, 3};
        Span<int, 3> sp{ints};
        {
            Span<const std::byte, sizeof(int) * 3u> bsp = SpanAsBytes(sp);
            Span<const int, 3> sp2 = ReinterpretSpan<const int>(bsp);
            EXPECT_EQ(sp2.data(), sp.data());
            EXPECT_TRUE(std::ranges::equal(sp2, sp));
        }
        {
            Span<std::byte, sizeof(int) * 3u> wbsp = SpanAsWritableBytes(sp);
            Span<int, 3> sp2 = ReinterpretSpan<int>(wbsp);
            EXPECT_EQ(sp2.data(), sp.data());
            EXPECT_TRUE(std::ranges::equal(sp2, sp));
        }
    }
}

TEST(SpanDeathTest, ReinterpretSpan) {
    // Check unaligned empty span.
    // Empty slice (e.g. data() != nullptr, but size() == 0).
    {
        alignas(uint32_t) std::array<std::byte, 4> bytes;
        auto bsp = Span<std::byte>(bytes).subspan(1u, 0);
        EXPECT_EQ(bsp.size(), 0u);
        EXPECT_NE(bsp.data(), nullptr);
        EXPECT_DEATH_IF_SUPPORTED(ReinterpretSpan<uint32_t>(bsp), "");
        EXPECT_DEATH_IF_SUPPORTED((ReinterpretSpan<uint32_t, Index>(bsp)), "");
    }
    // Alignment check fails.
    {
        alignas(uint32_t) std::array<std::byte, 9> bytes;
        auto bsp = Span<std::byte>(bytes).subspan(1u, 4u);
        if (alignof(uint32_t) > 1) {
            EXPECT_DEATH_IF_SUPPORTED(ReinterpretSpan<uint32_t>(bsp), "");
            EXPECT_DEATH_IF_SUPPORTED((ReinterpretSpan<uint32_t, Index>(bsp)), "");
        }
    }
    // Size check fails.
    {
        alignas(uint32_t) std::array<std::byte, 8> bytes;
        auto bsp = Span<std::byte>(bytes).first(7u);
        EXPECT_DEATH_IF_SUPPORTED(ReinterpretSpan<uint32_t>(bsp), "");
        EXPECT_DEATH_IF_SUPPORTED((ReinterpretSpan<uint32_t, Index>(bsp)), "");
    }
}

TEST(SpanTest, SpanFromRef) {
    {
        uint32_t i = 0;

        auto sp = SpanFromRef(i);
        static_assert(std::is_same_v<Span<uint32_t, 1>, decltype(sp)>);
        EXPECT_EQ(sp.size(), 1u);
        EXPECT_EQ(sp.data(), &i);

        auto bsp = ByteSpanFromRef(i);
        static_assert(std::is_same_v<Span<std::byte, sizeof(uint32_t)>, decltype(bsp)>);
        EXPECT_EQ(bsp.size(), sizeof(uint32_t));
        EXPECT_EQ(bsp.data(), reinterpret_cast<std::byte*>(&i));
    }
    {
        const uint32_t i = 0;

        auto sp = SpanFromRef(i);
        static_assert(std::is_same_v<Span<const uint32_t, 1>, decltype(sp)>);
        EXPECT_EQ(sp.size(), 1u);
        EXPECT_EQ(sp.data(), &i);

        auto bsp = ByteSpanFromRef(i);
        static_assert(std::is_same_v<Span<const std::byte, sizeof(uint32_t)>, decltype(bsp)>);
        EXPECT_EQ(bsp.size(), sizeof(uint32_t));
        EXPECT_EQ(bsp.data(), reinterpret_cast<const std::byte*>(&i));
    }
}

TEST(SpanTest, SpanFromRefTyped) {
    {
        uint32_t i = 0;

        auto sp = SpanFromRef<Index>(i);
        static_assert(std::is_same_v<ityp::span<Index, uint32_t, Index{1u}>, decltype(sp)>);
        EXPECT_EQ(sp.size(), Index{1u});
        EXPECT_EQ(sp.data(), &i);
    }
    {
        const uint32_t i = 0;

        auto sp = SpanFromRef<Index>(i);
        static_assert(std::is_same_v<ityp::span<Index, const uint32_t, Index{1u}>, decltype(sp)>);
        EXPECT_EQ(sp.size(), Index{1u});
        EXPECT_EQ(sp.data(), &i);
    }
}

TEST(SpanTest, TakeFirst) {
    std::array<int, 3> ints{1, 2, 3};

    // Take only a part.
    {
        Span<int> sp{ints};
        auto taken = sp.TakeFirst(1);

        EXPECT_EQ(sp.data(), &ints[1]);
        EXPECT_EQ(sp.size(), 2u);

        static_assert(std::is_same_v<decltype(taken), Span<int>>);
        EXPECT_EQ(taken.data(), ints.data());
        EXPECT_EQ(taken.size(), 1u);
    }

    // Take none.
    {
        Span<int> sp{ints};
        auto taken = sp.TakeFirst(0);

        EXPECT_EQ(sp.data(), ints.data());
        EXPECT_EQ(sp.size(), 3u);

        static_assert(std::is_same_v<decltype(taken), Span<int>>);
        EXPECT_TRUE(taken.empty());
    }

    // Take all.
    {
        Span<int> sp{ints};
        auto taken = sp.TakeFirst(3);

        EXPECT_TRUE(sp.empty());

        static_assert(std::is_same_v<decltype(taken), Span<int>>);
        EXPECT_EQ(taken.data(), ints.data());
        EXPECT_EQ(taken.size(), 3u);
    }
}

TEST(SpanDeathTest, TakeFirstOOB) {
    Span<const int> sp{FakeRange()};

    sp.TakeFirst(sp.size());
    EXPECT_DEATH_IF_SUPPORTED(sp.TakeFirst(sp.size() + 1), "");
}

TEST(SpanTest, CopyFrom) {
    // Copy from implicitly constructed span (std::array)
    {
        std::array<int, 3> src = {1, 2, 3};
        std::array<int, 3> dst = {0, 0, 0};
        Span<int> dst_sp{dst};

        dst_sp.CopyFrom(src);
        EXPECT_THAT(dst, ElementsAreArray(src));
    }

    // Copy from implicitly constructed span (std::array) with fixed extents
    {
        std::array<int, 3> src = {1, 2, 3};
        std::array<int, 3> dst = {0, 0, 0};
        Span<int, 3> dst_sp{dst};

        dst_sp.CopyFrom(src);
        EXPECT_THAT(dst, ElementsAreArray(src));
    }

    // Copy from heap array (std::vector)
    {
        std::vector<int> src = {4, 5, 6};
        std::array<int, 3> dst = {0, 0, 0};
        Span<int> dst_sp{dst};

        dst_sp.CopyFrom(src);
        EXPECT_THAT(dst, ElementsAreArray(src));
    }

    // Test different index types
    {
        std::array<int, 3> src = {1, 2, 3};
        std::array<int, 3> dst = {0, 0, 0};
        // SAFETY: This is viewing dst, just with typed indices.
        ityp::span<Index, int> DAWN_UNSAFE_BUFFERS(dst_sp(dst.data(), Index{3u}));
        // SAFETY: This is viewing src, just with typed indices.
        ityp::span<Index, const int> DAWN_UNSAFE_BUFFERS(src_sp(src.data(), Index{3u}));

        dst_sp.CopyFrom(src_sp);
        EXPECT_THAT(dst, ElementsAreArray(src));
    }
}

TEST(SpanTest, CopyFromOverlapping) {
    // Forward copy (src before dst)
    {
        std::array<int, 5> data = {1, 2, 3, 4, 5};
        Span<int> sp{data};
        sp.subspan(1, 3).CopyFrom(sp.subspan(0, 3));
        EXPECT_THAT(data, testing::ElementsAre(1, 1, 2, 3, 5));
    }

    // Backward copy (dst before src)
    {
        std::array<int, 5> data = {1, 2, 3, 4, 5};
        Span<int> sp{data};
        sp.subspan(0, 3).CopyFrom(sp.subspan(1, 3));
        EXPECT_THAT(data, testing::ElementsAre(2, 3, 4, 4, 5));
    }
}

TEST(SpanDeathTest, CopyFromSizeMismatch) {
    std::array<int, 3> src = {1, 2, 3};
    std::array<int, 2> dst = {0, 0};
    Span<int> dst_sp{dst};
    Span<const int> src_sp{src};

    EXPECT_DEATH_IF_SUPPORTED(dst_sp.CopyFrom(src_sp), "");
}

TEST(SpanTest, CopyPrefixFrom) {
    // Copy from implicitly constructed span (std::array)
    {
        std::array<int, 2> src = {1, 2};
        std::array<int, 3> dst = {0, 0, 0};
        Span<int> dst_sp{dst};

        dst_sp.CopyPrefixFrom(src);
        EXPECT_THAT(dst, testing::ElementsAre(1, 2, 0));
    }

    // Copy from implicitly constructed span (std::array) with fixed extents
    {
        std::array<int, 2> src = {1, 2};
        std::array<int, 3> dst = {0, 0, 0};
        Span<int, 3> dst_sp{dst};

        dst_sp.CopyPrefixFrom(src);
        EXPECT_THAT(dst, testing::ElementsAre(1, 2, 0));
    }

    // Copy from heap array (std::vector)
    {
        std::vector<int> src = {4};
        std::array<int, 3> dst = {0, 0, 0};
        Span<int> dst_sp{dst};

        dst_sp.CopyPrefixFrom(src);
        EXPECT_THAT(dst, testing::ElementsAre(4, 0, 0));
    }

    // Test different index types
    {
        std::array<int, 2> src = {1, 2};
        std::array<int, 3> dst = {0, 0, 0};
        // SAFETY: This is viewing dst, just with typed indices.
        ityp::span<Index, int> DAWN_UNSAFE_BUFFERS(dst_sp(dst.data(), Index{3u}));
        // SAFETY: This is viewing src, just with typed indices.
        ityp::span<Index, const int> DAWN_UNSAFE_BUFFERS(src_sp(src.data(), Index{2u}));

        dst_sp.CopyPrefixFrom(src_sp);
        EXPECT_THAT(dst, testing::ElementsAre(1, 2, 0));
    }
}

TEST(SpanTest, CopyPrefixFromOverlapping) {
    // Forward copy
    {
        std::array<int, 5> data = {1, 2, 3, 4, 5};
        Span<int> sp{data};
        sp.subspan(1, 4).CopyPrefixFrom(sp.subspan(0, 3));
        EXPECT_THAT(data, testing::ElementsAre(1, 1, 2, 3, 5));
    }

    // Backward copy
    {
        std::array<int, 5> data = {1, 2, 3, 4, 5};
        Span<int> sp{data};
        sp.subspan(0, 4).CopyPrefixFrom(sp.subspan(1, 3));
        EXPECT_THAT(data, testing::ElementsAre(2, 3, 4, 4, 5));
    }
}

TEST(SpanDeathTest, CopyPrefixFromSizeMismatch) {
    std::array<int, 3> src = {1, 2, 3};
    std::array<int, 2> dst = {0, 0};
    Span<int> dst_sp{dst};
    Span<const int> src_sp{src};

    EXPECT_DEATH_IF_SUPPORTED(dst_sp.CopyPrefixFrom(src_sp), "");
}

TEST(SpanTest, Constructor_CArray) {
    int arr[] = {1, 2, 3, 4};
    const int constArr[] = {5, 6, 7};

    {
        Span<int> sp(arr);
        EXPECT_EQ(sp.size(), 4u);
        EXPECT_EQ(sp.data(), arr);
        EXPECT_EQ(sp[0], 1);
    }
    {
        Span<int, 4> sp(arr);
        EXPECT_EQ(sp.data(), arr);
        EXPECT_EQ(sp[0], 1);
    }
    {
        Span<const int> sp(arr);
        EXPECT_EQ(sp.size(), 4u);
        EXPECT_EQ(sp.data(), arr);
    }
    {
        Span<const int> sp(constArr);
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), constArr);
    }
    {
        Span<const int, 3> sp(constArr);
        EXPECT_EQ(sp.data(), constArr);
    }
}

TEST(SpanTest, Constructor_InitializeList) {
    std::initializer_list<int> list = {1, 2, 3, 4};
    std::initializer_list<const int> constList = {5, 6, 7};

    {
        Span<const int> sp(list);
        EXPECT_EQ(sp.size(), 4u);
        EXPECT_EQ(sp.data(), list.begin());
    }
    {
        Span<const int> sp(constList);
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), constList.begin());
    }
}

}  // anonymous namespace
}  // namespace dawn
