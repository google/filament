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

#include "src/tint/utils/bytes/decoder.h"

#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "gmock/gmock.h"
#include "src/tint/utils/bytes/buffer_reader.h"
#include "src/tint/utils/result/result.h"

namespace tint {
namespace {
/// An enum used for decoder tests below.
enum class TestEnum : uint8_t { A = 2, B = 3, C = 4 };
}  // namespace

/// Reflect valid value ranges for the TestEnum enum.
TINT_REFLECT_ENUM_RANGE(TestEnum, A, C);

}  // namespace tint

namespace tint::bytes {
namespace {
template <typename... ARGS>
auto Data(ARGS&&... args) {
    return std::array{std::byte{static_cast<uint8_t>(args)}...};
}

TEST(BytesDecoderTest, Uint8) {
    auto data = Data(0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80);
    auto reader = BufferReader{Slice{data}};
    EXPECT_EQ(Decode<uint8_t>(reader).Get(), 0x10u);
    EXPECT_EQ(Decode<uint8_t>(reader).Get(), 0x20u);
    EXPECT_EQ(Decode<uint8_t>(reader).Get(), 0x30u);
    EXPECT_EQ(Decode<uint8_t>(reader).Get(), 0x40u);
    EXPECT_EQ(Decode<uint8_t>(reader).Get(), 0x50u);
    EXPECT_EQ(Decode<uint8_t>(reader).Get(), 0x60u);
    EXPECT_EQ(Decode<uint8_t>(reader).Get(), 0x70u);
    EXPECT_EQ(Decode<uint8_t>(reader).Get(), 0x80u);
    EXPECT_NE(Decode<uint8_t>(reader), Success);
}

TEST(BytesDecoderTest, Uint16) {
    auto data = Data(0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80);
    auto reader = BufferReader{Slice{data}};
    EXPECT_EQ(Decode<uint16_t>(reader).Get(), 0x2010u);
    EXPECT_EQ(Decode<uint16_t>(reader).Get(), 0x4030u);
    EXPECT_EQ(Decode<uint16_t>(reader).Get(), 0x6050u);
    EXPECT_EQ(Decode<uint16_t>(reader).Get(), 0x8070u);
    EXPECT_NE(Decode<uint16_t>(reader), Success);
}

TEST(BytesDecoderTest, Uint32) {
    auto data = Data(0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80);
    auto reader = BufferReader{Slice{data}};
    EXPECT_EQ(Decode<uint32_t>(reader, Endianness::kBig).Get(), 0x10203040u);
    EXPECT_EQ(Decode<uint32_t>(reader, Endianness::kBig).Get(), 0x50607080u);
    EXPECT_NE(Decode<uint32_t>(reader), Success);
}

TEST(BytesDecoderTest, Float) {
    auto data = Data(0x00, 0x00, 0x08, 0x41);
    auto reader = BufferReader{Slice{data}};
    EXPECT_EQ(Decode<float>(reader).Get(), 8.5f);
    EXPECT_NE(Decode<float>(reader), Success);
}

TEST(BytesDecoderTest, Bool) {
    auto data = Data(0x0, 0x1, 0x2, 0x1, 0x0);
    auto reader = BufferReader{Slice{data}};
    EXPECT_EQ(Decode<bool>(reader).Get(), false);
    EXPECT_EQ(Decode<bool>(reader).Get(), true);
    EXPECT_EQ(Decode<bool>(reader).Get(), true);
    EXPECT_EQ(Decode<bool>(reader).Get(), true);
    EXPECT_EQ(Decode<bool>(reader).Get(), false);
    EXPECT_NE(Decode<bool>(reader), Success);
}

TEST(BytesDecoderTest, String) {
    auto data = Data(0x5, 0x0, 'h', 'e', 'l', 'l', 'o', 0x5, 0x0, 'w', 'o', 'r', 'l', 'd');
    auto reader = BufferReader{Slice{data}};
    EXPECT_EQ(Decode<std::string>(reader).Get(), "hello");
    EXPECT_EQ(Decode<std::string>(reader).Get(), "world");
    EXPECT_NE(Decode<std::string>(reader), Success);
}

struct S {
    uint8_t a;
    uint16_t b;
    uint32_t c;
    TINT_REFLECT(S, a, b, c);
};

TEST(BytesDecoderTest, ReflectedObject) {
    auto data = Data(0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80);
    auto reader = BufferReader{Slice{data}};
    auto got = Decode<S>(reader);
    EXPECT_EQ(got->a, 0x10u);
    EXPECT_EQ(got->b, 0x3020u);
    EXPECT_EQ(got->c, 0x70605040u);
    EXPECT_NE(Decode<S>(reader), Success);
}

TEST(BytesDecoderTest, ReflectedEnum) {
    auto data = Data(0x03, 0x01, 0x05);
    auto reader = BufferReader{Slice{data}};
    auto got = Decode<TestEnum>(reader);
    EXPECT_EQ(got, TestEnum::B);
    EXPECT_NE(Decode<S>(reader), Success);  // Out of range
    EXPECT_NE(Decode<S>(reader), Success);  // Out of range
}

TEST(BytesDecoderTest, UnorderedMap) {
    using M = std::unordered_map<uint8_t, uint16_t>;
    auto data = Data(0x00, 0x10, 0x02, 0x20,  //
                     0x00, 0x30, 0x04, 0x40,  //
                     0x00, 0x50, 0x06, 0x60,  //
                     0x00, 0x70, 0x08, 0x80,  //
                     0x01);
    auto reader = BufferReader{Slice{data}};
    {
        auto got = Decode<M>(reader);
        ASSERT_EQ(got, Success);
        EXPECT_THAT(got.Get(), testing::ContainerEq(M{
                                   std::pair<uint8_t, uint16_t>(0x10u, 0x2002u),
                                   std::pair<uint8_t, uint16_t>(0x30u, 0x4004u),
                                   std::pair<uint8_t, uint16_t>(0x50u, 0x6006u),
                                   std::pair<uint8_t, uint16_t>(0x70u, 0x8008u),
                               }));
    }
    {
        auto got = Decode<M>(reader);
        ASSERT_EQ(got, Success);
        EXPECT_THAT(got.Get(), testing::IsEmpty());
    }
}

TEST(BytesDecoderTest, UnorderedSet) {
    using S = std::unordered_set<uint16_t>;
    auto data = Data(0x00, 0x02, 0x20,  //
                     0x00, 0x04, 0x40,  //
                     0x00, 0x06, 0x60,  //
                     0x00, 0x08, 0x80,  //
                     0x01);
    auto reader = BufferReader{Slice{data}};
    {
        auto got = Decode<S>(reader);
        ASSERT_EQ(got, Success);
        EXPECT_THAT(got.Get(), testing::ContainerEq(S{
                                   0x2002u,
                                   0x4004u,
                                   0x6006u,
                                   0x8008u,
                               }));
    }
    {
        auto got = Decode<S>(reader);
        ASSERT_EQ(got, Success);
        EXPECT_THAT(got.Get(), testing::IsEmpty());
    }
}

TEST(BytesDecoderTest, Vector) {
    using M = std::vector<uint8_t>;
    auto data = Data(0x00, 0x10,  //
                     0x00, 0x30,  //
                     0x00, 0x50,  //
                     0x00, 0x70,  //
                     0x01);
    auto reader = BufferReader{Slice{data}};
    {
        auto got = Decode<M>(reader);
        ASSERT_EQ(got, Success);
        EXPECT_THAT(got.Get(), testing::ContainerEq(M{
                                   0x10u,
                                   0x30u,
                                   0x50u,
                                   0x70u,
                               }));
    }
    {
        auto got = Decode<M>(reader);
        ASSERT_EQ(got, Success);
        EXPECT_THAT(got.Get(), testing::IsEmpty());
    }
}

TEST(BytesDecoderTest, Optional) {
    auto data = Data(0x00,  //
                     0x01, 0x42);
    auto reader = BufferReader{Slice{data}};
    auto first = Decode<std::optional<uint8_t>>(reader);
    auto second = Decode<std::optional<uint8_t>>(reader);
    auto third = Decode<std::optional<uint8_t>>(reader);
    ASSERT_EQ(first, Success);
    ASSERT_EQ(second, Success);
    EXPECT_NE(third, Success);
    EXPECT_EQ(first.Get(), std::nullopt);
    EXPECT_EQ(second.Get(), std::optional<uint8_t>{0x42});
}

TEST(BytesDecoderTest, Bitset) {
    auto data = Data(0x2D, 0x6D);
    auto reader = BufferReader{Slice{data}};
    auto got = Decode<std::bitset<14>>(reader);
    EXPECT_EQ(got, std::bitset<14>(0x6D2D));
}

TEST(BytesDecoderTest, Tuple) {
    using T = std::tuple<uint8_t, uint16_t, uint32_t>;
    auto data = Data(0x10,                    //
                     0x20, 0x30,              //
                     0x40, 0x50, 0x60, 0x70,  //
                     0x80);
    auto reader = BufferReader{Slice{data}};
    EXPECT_EQ(Decode<T>(reader), (T{0x10u, 0x3020u, 0x70605040u}));
    EXPECT_NE(Decode<T>(reader), Success);
}

}  // namespace
}  // namespace tint::bytes
