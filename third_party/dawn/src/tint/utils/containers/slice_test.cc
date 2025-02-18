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

#include "src/tint/utils/containers/slice.h"

#include "gmock/gmock.h"

TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

namespace tint {
namespace {

class C0 : public Castable<C0> {};
class C1 : public Castable<C1, C0> {};
class C2a : public Castable<C2a, C1> {};
class C2b : public Castable<C2b, C1> {};

////////////////////////////////////////////////////////////////////////////////
// Static asserts
////////////////////////////////////////////////////////////////////////////////
// Non-pointer
static_assert(CanReinterpretSlice<ReinterpretMode::kSafe, int, int>, "same type");
static_assert(CanReinterpretSlice<ReinterpretMode::kSafe, int const, int>, "apply const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, int, int const>, "remove const");

// Non-castable pointers
static_assert(CanReinterpretSlice<ReinterpretMode::kSafe, int* const, int*>, "apply ptr const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, int*, int* const>, "remove ptr const");
static_assert(CanReinterpretSlice<ReinterpretMode::kSafe, int const*, int*>, "apply el const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, int*, int const*>, "remove el const");

// Castable
static_assert(CanReinterpretSlice<ReinterpretMode::kSafe, const C0*, C0*>, "apply const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, C0*, const C0*>, "remove const");
static_assert(CanReinterpretSlice<ReinterpretMode::kSafe, C0*, C1*>, "up cast");
static_assert(CanReinterpretSlice<ReinterpretMode::kSafe, const C0*, const C1*>, "up cast");
static_assert(CanReinterpretSlice<ReinterpretMode::kSafe, const C0*, C1*>, "up cast, apply const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, C0*, const C1*>,
              "up cast, remove const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, C1*, C0*>, "down cast");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, const C1*, const C0*>, "down cast");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, const C1*, C0*>,
              "down cast, apply const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, C1*, const C0*>,
              "down cast, remove const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, const C1*, C0*>,
              "down cast, apply const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, C1*, const C0*>,
              "down cast, remove const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, C2a*, C2b*>, "sideways cast");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, const C2a*, const C2b*>,
              "sideways cast");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, const C2a*, C2b*>,
              "sideways cast, apply const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, C2a*, const C2b*>,
              "sideways cast, remove const");

TEST(TintSliceTest, Ctor) {
    Slice<int> slice;
    EXPECT_EQ(slice.data, nullptr);
    EXPECT_EQ(slice.len, 0u);
    EXPECT_EQ(slice.cap, 0u);
    EXPECT_TRUE(slice.IsEmpty());
}

TEST(TintSliceTest, CtorCast) {
    C1* elements[3];

    Slice<C1*> slice_a;
    slice_a.data = &elements[0];
    slice_a.len = 3;
    slice_a.cap = 3;

    Slice<const C0*> slice_b(slice_a);
    EXPECT_EQ(slice_b.data, Bitcast<const C0**>(&elements[0]));
    EXPECT_EQ(slice_b.len, 3u);
    EXPECT_EQ(slice_b.cap, 3u);
    EXPECT_FALSE(slice_b.IsEmpty());
}

TEST(TintSliceTest, CtorEmpty) {
    Slice<int> slice{Empty};
    EXPECT_EQ(slice.data, nullptr);
    EXPECT_EQ(slice.len, 0u);
    EXPECT_EQ(slice.cap, 0u);
    EXPECT_TRUE(slice.IsEmpty());
}

TEST(TintSliceTest, CtorCArray) {
    int elements[] = {1, 2, 3};

    auto slice = Slice{elements};
    EXPECT_EQ(slice.data, elements);
    EXPECT_EQ(slice.len, 3u);
    EXPECT_EQ(slice.cap, 3u);
    EXPECT_FALSE(slice.IsEmpty());
}

TEST(TintSliceTest, CtorStdArray) {
    std::array elements{1, 2, 3};

    auto slice = Slice{elements};
    EXPECT_EQ(slice.data, &elements[0]);
    EXPECT_EQ(slice.len, 3u);
    EXPECT_EQ(slice.cap, 3u);
    EXPECT_FALSE(slice.IsEmpty());
}

TEST(TintSliceTest, Index) {
    int elements[] = {1, 2, 3};

    auto slice = Slice{elements};
    EXPECT_EQ(slice[0], 1);
    EXPECT_EQ(slice[1], 2);
    EXPECT_EQ(slice[2], 3);
}

TEST(TintSliceTest, Front) {
    int elements[] = {1, 2, 3};
    auto slice = Slice{elements};
    EXPECT_EQ(slice.Front(), 1);
}

TEST(TintSliceTest, Back) {
    int elements[] = {1, 2, 3};
    auto slice = Slice{elements};
    EXPECT_EQ(slice.Back(), 3);
}

TEST(TintSliceTest, BeginEnd) {
    int elements[] = {1, 2, 3};
    auto slice = Slice{elements};
    EXPECT_THAT(slice, testing::ElementsAre(1, 2, 3));
}

TEST(TintSliceTest, ReverseBeginEnd) {
    int elements[] = {1, 2, 3};
    auto slice = Slice{elements};
    size_t i = 0;
    for (auto it = slice.rbegin(); it != slice.rend(); it++) {
        EXPECT_EQ(*it, elements[2 - i]);
        i++;
    }
}

TEST(TintSliceTest, Offset) {
    int elements[] = {1, 2, 3};

    auto slice = Slice{elements};
    auto offset = slice.Offset(1);
    EXPECT_EQ(offset.Length(), 2u);
    EXPECT_EQ(offset[0], 2);
    EXPECT_EQ(offset[1], 3);
}

TEST(TintSliceTest, Offset_PastEnd) {
    int elements[] = {1, 2, 3};

    auto slice = Slice{elements};
    auto offset = slice.Offset(4);
    EXPECT_EQ(offset.Length(), 0u);
}

TEST(TintSliceTest, Truncate) {
    int elements[] = {1, 2, 3};

    auto slice = Slice{elements};
    auto truncated = slice.Truncate(2);
    EXPECT_EQ(truncated.Length(), 2u);
    EXPECT_EQ(truncated[0], 1);
    EXPECT_EQ(truncated[1], 2);
}

TEST(TintSliceTest, Truncate_PastEnd) {
    int elements[] = {1, 2, 3};

    auto slice = Slice{elements};
    auto truncated = slice.Truncate(4);
    EXPECT_EQ(truncated.Length(), 3u);
    EXPECT_EQ(truncated[0], 1);
    EXPECT_EQ(truncated[1], 2);
    EXPECT_EQ(truncated[2], 3);
}

TEST(TintSliceTest, Equality) {
    int elements[] = {1, 2, 3};
    const auto a = Slice{elements};
    {
        auto b = a;
        EXPECT_TRUE(a == b);
        EXPECT_FALSE(a != b);
    }
    {
        auto b = a;
        b.data++;
        EXPECT_FALSE(a == b);
        EXPECT_TRUE(a != b);
    }
    {
        auto b = a;
        b.len++;
        EXPECT_FALSE(a == b);
        EXPECT_TRUE(a != b);
    }
    {
        auto b = a;
        b.cap++;
        EXPECT_FALSE(a == b);
        EXPECT_TRUE(a != b);
    }
}

}  // namespace
}  // namespace tint

TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
