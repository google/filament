// Copyright 2022 The Dawn & Tint Authors
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

#include <cstring>
#include <iomanip>
#include <limits>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "dawn/common/TypedInteger.h"
#include "dawn/native/Blob.h"
#include "dawn/native/Serializable.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/stream/BlobSource.h"
#include "dawn/native/stream/ByteVectorSink.h"
#include "dawn/native/stream/Stream.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "tint/tint.h"

namespace dawn::native::stream {
namespace {

// Testing classes with mock serializing implemented for testing.
class A {
  public:
    MOCK_METHOD(void, WriteMock, (Sink*, const A&), (const));
};

struct Nested {
    A a1;
    A a2;
};

}  // anonymous namespace

template <>
void Stream<A>::Write(Sink* s, const A& t) {
    t.WriteMock(s, t);
}

template <>
void Stream<Nested>::Write(Sink* s, const Nested& t) {
    StreamIn(s, t.a1);
    StreamIn(s, t.a2);
}

// Custom printer for ByteVectorSink for clearer debug testing messages.
void PrintTo(const ByteVectorSink& key, std::ostream* stream) {
    *stream << std::hex;
    for (const int b : key) {
        *stream << std::setfill('0') << std::setw(2) << b << " ";
    }
    *stream << std::dec;
}

namespace {

using ::testing::InSequence;
using ::testing::NotNull;
using ::testing::PrintToString;
using ::testing::Ref;

using TypedIntegerForTest = TypedInteger<struct TypedIntegerForTestTag, uint32_t>;

// Matcher to compare ByteVectorSinks for easier testing.
MATCHER_P(VectorEq, key, PrintToString(key)) {
    return arg.size() == key.size() && memcmp(arg.data(), key.data(), key.size()) == 0;
}

#define EXPECT_CACHE_KEY_EQ(lhs, rhs)       \
    do {                                    \
        ByteVectorSink actual;              \
        StreamIn(&actual, lhs);             \
        EXPECT_THAT(actual, VectorEq(rhs)); \
    } while (0)

// Test that ByteVectorSink calls Write on a value.
TEST(SerializeTests, CallsWrite) {
    ByteVectorSink sink;

    A a;
    EXPECT_CALL(a, WriteMock(NotNull(), Ref(a))).Times(1);
    StreamIn(&sink, a);
}

// Test that ByteVectorSink calls Write on all elements of an iterable.
TEST(SerializeTests, StreamInIterable) {
    constexpr size_t kIterableSize = 100;

    std::vector<A> vec(kIterableSize);
    auto iterable = Iterable(vec.data(), kIterableSize);

    // Expect write to be called for each element
    for (const auto& a : vec) {
        EXPECT_CALL(a, WriteMock(NotNull(), Ref(a))).Times(1);
    }

    ByteVectorSink sink;
    StreamIn(&sink, iterable);

    // Expecting the size of the container.
    ByteVectorSink expected;
    StreamIn(&expected, kIterableSize);
    EXPECT_THAT(sink, VectorEq(expected));
}

// Test that ByteVectorSink calls Write on all nested members of a struct.
TEST(SerializeTests, StreamInNested) {
    ByteVectorSink sink;

    Nested n;
    EXPECT_CALL(n.a1, WriteMock(NotNull(), Ref(n.a1))).Times(1);
    EXPECT_CALL(n.a2, WriteMock(NotNull(), Ref(n.a2))).Times(1);
    StreamIn(&sink, n);
}

// Test that ByteVectorSink serializes integral data as expected.
TEST(SerializeTests, IntegralTypes) {
    // Only testing explicitly sized types for simplicity, and using 0s for larger types to
    // avoid dealing with endianess.
    EXPECT_CACHE_KEY_EQ('c', ByteVectorSink({'c'}));
    EXPECT_CACHE_KEY_EQ(uint8_t(255), ByteVectorSink({255}));
    EXPECT_CACHE_KEY_EQ(uint16_t(0), ByteVectorSink({0, 0}));
    EXPECT_CACHE_KEY_EQ(uint32_t(0), ByteVectorSink({0, 0, 0, 0}));
}

// Test that ByteVectorSink serializes floating-point data as expected.
TEST(SerializeTests, FloatingTypes) {
    // Using 0s to avoid dealing with implementation specific float details.
    ByteVectorSink k1, k2;
    EXPECT_CACHE_KEY_EQ(float{0}, ByteVectorSink(sizeof(float), 0));
    EXPECT_CACHE_KEY_EQ(double{0}, ByteVectorSink(sizeof(double), 0));
}

// Test that ByteVectorSink serializes literal strings as expected.
TEST(SerializeTests, LiteralStrings) {
    // Using a std::string here to help with creating the expected result.
    std::string str = "string";

    ByteVectorSink expected;
    expected.insert(expected.end(), str.begin(), str.end());
    expected.push_back('\0');

    EXPECT_CACHE_KEY_EQ("string", expected);
}

// Test that ByteVectorSink serializes std::strings as expected.
TEST(SerializeTests, StdStrings) {
    std::string str = "string";

    ByteVectorSink expected;
    StreamIn(&expected, size_t(6));
    expected.insert(expected.end(), str.begin(), str.end());

    EXPECT_CACHE_KEY_EQ(str, expected);
}

// Test that ByteVectorSink serializes std::string_views as expected.
TEST(SerializeTests, StdStringViews) {
    static constexpr std::string_view str("string");

    ByteVectorSink expected;
    StreamIn(&expected, size_t(6));
    expected.insert(expected.end(), str.begin(), str.end());

    EXPECT_CACHE_KEY_EQ(str, expected);
}

// Test that ByteVectorSink serializes std::wstring_views as expected.
TEST(SerializeTests, StdWStringViews) {
    static constexpr std::wstring_view str(L"Hello world!");

    ByteVectorSink expected;
    StreamIn(&expected, size_t(str.length()));
    size_t bytes = str.length() * sizeof(wchar_t);
    memcpy(expected.GetSpace(bytes), str.data(), bytes);

    EXPECT_CACHE_KEY_EQ(str, expected);
}

// Test that ByteVectorSink serializes Blobs as expected.
TEST(SerializeTests, Blob) {
    uint8_t data[] = "dawn native Blob";
    Blob blob = Blob::UnsafeCreateWithDeleter(data, sizeof(data), [] {});

    ByteVectorSink expected;
    StreamIn(&expected, sizeof(data));
    expected.insert(expected.end(), data, data + sizeof(data));

    EXPECT_CACHE_KEY_EQ(blob, expected);
}

// Test that ByteVectorSink serializes other ByteVectorSinks as expected.
TEST(SerializeTests, ByteVectorSinks) {
    ByteVectorSink data = {'d', 'a', 't', 'a'};

    ByteVectorSink expected;
    expected.insert(expected.end(), data.begin(), data.end());

    EXPECT_CACHE_KEY_EQ(data, expected);
}

// Test that serializing a value, then deserializing it with unexpected size, an error is raised.
TEST(SerializeTests, SerializeDeserializeVectorSizeOutOfBounds) {
    size_t value = std::numeric_limits<size_t>::max();
    ByteVectorSink sink;
    StreamIn(&sink, value);

    BlobSource source(CreateBlob(std::move(sink)));
    std::vector<uint8_t> deserialized;
    auto err = StreamOut(&source, &deserialized);
    EXPECT_TRUE(err.IsError());
    err.AcquireError();
}

// Test that serializing a value, then deserializing it without vector elements, an error is raised.
TEST(SerializeTests, SerializeDeserializeNoElementsInVector) {
    size_t value = 1;
    ByteVectorSink sink;
    StreamIn(&sink, value);

    BlobSource source(CreateBlob(std::move(sink)));
    std::vector<uint8_t> deserialized;
    auto err = StreamOut(&source, &deserialized);
    EXPECT_TRUE(err.IsError());
    err.AcquireError();
}

// Test that ByteVectorSink serializes std::pair as expected.
TEST(SerializeTests, StdPair) {
    std::string_view s = "hi!";

    ByteVectorSink expected;
    StreamIn(&expected, s, uint32_t(42));

    EXPECT_CACHE_KEY_EQ(std::make_pair(s, uint32_t(42)), expected);
}

// Test that ByteVectorSink serializes std::optional as expected.
TEST(SerializeTests, StdOptional) {
    std::string_view s = "webgpu";
    {
        ByteVectorSink expected;
        StreamIn(&expected, true, s);

        EXPECT_CACHE_KEY_EQ(std::optional(s), expected);
    }
    {
        ByteVectorSink expected;
        StreamIn(&expected, false);

        EXPECT_CACHE_KEY_EQ(std::optional<std::string_view>(), expected);
    }
}

// Test that ByteVectorSink serializes std::unordered_map as expected.
TEST(SerializeTests, StdUnorderedMap) {
    std::unordered_map<uint32_t, std::string_view> m;

    m[4] = "hello";
    m[1] = "world";
    m[7] = "test";
    m[3] = "data";

    // Expect the number of entries, followed by (K, V) pairs sorted in order of key.
    ByteVectorSink expected;
    StreamIn(&expected, size_t(4), std::make_pair(uint32_t(1), m[1]),
             std::make_pair(uint32_t(3), m[3]), std::make_pair(uint32_t(4), m[4]),
             std::make_pair(uint32_t(7), m[7]));

    EXPECT_CACHE_KEY_EQ(m, expected);
}

// Test that ByteVectorSink serializes std::unordered_set as expected.
TEST(SerializeTests, StdUnorderedSet) {
    const std::unordered_set<int> input = {99, 4, 6, 1};

    // Expect the number of entries, followed by values sorted in order of key.
    ByteVectorSink expected;
    StreamIn(&expected, size_t(4), 1, 4, 6, 99);

    EXPECT_CACHE_KEY_EQ(input, expected);
}

// Test that ByteVectorSink serializes tint::BindingPoint as expected.
TEST(SerializeTests, TintSemBindingPoint) {
    tint::BindingPoint bp{3, 6};

    ByteVectorSink expected;
    StreamIn(&expected, uint32_t(3), uint32_t(6));

    EXPECT_CACHE_KEY_EQ(bp, expected);
}

// Test that serializing then deserializing a param pack yields the same values.
TEST(StreamTests, SerializeDeserializeParamPack) {
    int a = 1;
    float b = 2.0;
    std::pair<std::string, double> c = std::make_pair("dawn", 3.4);

    ByteVectorSink sink;
    StreamIn(&sink, a, b, c);

    BlobSource source(CreateBlob(std::move(sink)));
    int aOut;
    float bOut;
    std::pair<std::string, double> cOut;
    auto err = StreamOut(&source, &aOut, &bOut, &cOut);
    if (err.IsError()) {
        FAIL() << err.AcquireError()->GetFormattedMessage();
    }
    EXPECT_EQ(a, aOut);
    EXPECT_EQ(b, bOut);
    EXPECT_EQ(c, cOut);
}

#define FOO_MEMBERS(X) \
    X(int, a)          \
    X(float, b)        \
    X(std::string, c)
DAWN_SERIALIZABLE(struct, Foo, FOO_MEMBERS){};
#undef FOO_MEMBERS

// Test that serializing then deserializing a struct made with DAWN_SERIALIZABLE works as
// expected.
TEST(StreamTests, SerializeDeserializeVisitable) {
    Foo foo{{1, 2, "3"}};
    ByteVectorSink sink;
    StreamIn(&sink, foo);

    // Test that the serialization is correct.
    {
        ByteVectorSink expected;
        StreamIn(&expected, foo.a, foo.b, foo.c);
        EXPECT_THAT(sink, VectorEq(expected));
    }

    // Test that deserialization works for StructMembers, passed inline.
    {
        BlobSource src(CreateBlob(sink));
        Foo out;
        auto err = StreamOut(&src, &out);
        EXPECT_FALSE(err.IsError());
        EXPECT_EQ(foo.a, out.a);
        EXPECT_EQ(foo.b, out.b);
        EXPECT_EQ(foo.c, out.c);
    }
}

// Test that serializing then deserializing a Blob yields the same data.
// Tested here instead of in the type-parameterized tests since Blobs are not copyable.
TEST(StreamTests, SerializeDeserializeBlobs) {
    // Test an empty blob
    {
        Blob blob;
        EXPECT_EQ(blob.Size(), 0u);

        ByteVectorSink sink;
        StreamIn(&sink, blob);

        BlobSource src(CreateBlob(sink));
        Blob out;
        auto err = StreamOut(&src, &out);
        EXPECT_FALSE(err.IsError());
        EXPECT_EQ(blob.Size(), out.Size());
        EXPECT_EQ(memcmp(blob.Data(), out.Data(), blob.Size()), 0);
    }

    // Test a blob with some data
    {
        Blob blob = CreateBlob(std::vector<double>{6.24, 3.12222});

        ByteVectorSink sink;
        StreamIn(&sink, blob);

        BlobSource src(CreateBlob(sink));
        Blob out;
        auto err = StreamOut(&src, &out);
        EXPECT_FALSE(err.IsError());
        EXPECT_EQ(blob.Size(), out.Size());
        EXPECT_EQ(memcmp(blob.Data(), out.Data(), blob.Size()), 0);
    }
}

template <size_t N>
std::bitset<N - 1> BitsetFromBitString(const char (&str)[N]) {
    // N - 1 because the last character is the null terminator.
    return std::bitset<N - 1>(str, N - 1);
}

static auto kStreamValueVectorParams = std::make_tuple(
    // Test primitives.
    std::vector<int>{4, 5, 6, 2},
    std::vector<float>{6.50, 78.28, 92., 8.28},
    // Test various types of strings.
    std::vector<std::string>{"abcdefg", "9461849495", ""},
    // Test pairs.
    std::vector<std::pair<int, float>>{{1, 3.}, {6, 4.}},
    // Test TypedIntegers
    std::vector<TypedIntegerForTest>{TypedIntegerForTest(42), TypedIntegerForTest(13)},
    // Test enums
    std::vector<wgpu::TextureUsage>{wgpu::TextureUsage::CopyDst,
                                    wgpu::TextureUsage::RenderAttachment},
    // Test bitsets of various sizes.
    std::vector<std::bitset<7>>{0b1001011, 0b0011010, 0b0000000, 0b1111111},
    std::vector<std::bitset<17>>{0x0000, 0xFFFF1},
    std::vector<std::bitset<32>>{0x0C0FFEE0, 0xDEADC0DE, 0x00000000, 0xFFFFFFFF},
    std::vector<std::bitset<57>>{
        BitsetFromBitString("100110010101011001100110101011001100101010110011001011011"),
        BitsetFromBitString("000110010101011000100110101011001100101010010011001010100"),
        BitsetFromBitString("111111111111111111111111111111111111111111111111111111111"), 0},
    // Test unordered_sets.
    std::vector<std::unordered_set<int>>{{}, {4, 6, 99, 0}, {100, 300, 300}},
    // Test vectors.
    std::vector<std::vector<int>>{{}, {1, 5, 2, 7, 4}, {3, 3, 3, 3, 3, 3, 3}});

static auto kStreamValueInitListParams = std::make_tuple(
    std::initializer_list<char[12]>{"test string", "string test"},
    std::initializer_list<double[3]>{{5.435, 32.3, 1.23}, {8.2345, 0.234532, 4.435}});

template <typename, typename>
struct StreamValueTestTypesImpl;

template <typename... T, typename... T2>
struct StreamValueTestTypesImpl<std::tuple<std::vector<T>...>,
                                std::tuple<std::initializer_list<T2>...>> {
    using type = ::testing::Types<T..., T2...>;
};

using StreamValueTestTypes =
    typename StreamValueTestTypesImpl<decltype(kStreamValueVectorParams),
                                      decltype(kStreamValueInitListParams)>::type;

template <typename T>
class StreamParameterizedTests : public ::testing::Test {
  protected:
    static std::vector<T> GetParams() { return std::get<std::vector<T>>(kStreamValueVectorParams); }

    void ExpectEq(const T& lhs, const T& rhs) { EXPECT_EQ(lhs, rhs); }
};

template <typename T, size_t N>
class StreamParameterizedTests<T[N]> : public ::testing::Test {
  protected:
    static std::initializer_list<T[N]> GetParams() {
        return std::get<std::initializer_list<T[N]>>(kStreamValueInitListParams);
    }

    void ExpectEq(const T lhs[N], const T rhs[N]) { EXPECT_EQ(memcmp(lhs, rhs, sizeof(T[N])), 0); }
};

TYPED_TEST_SUITE_P(StreamParameterizedTests);

// Test that serializing a value, then deserializing it yields the same value.
TYPED_TEST_P(StreamParameterizedTests, SerializeDeserialize) {
    for (const auto& value : this->GetParams()) {
        ByteVectorSink sink;
        StreamIn(&sink, value);

        BlobSource source(CreateBlob(std::move(sink)));
        TypeParam deserialized;
        auto err = StreamOut(&source, &deserialized);
        if (err.IsError()) {
            FAIL() << err.AcquireError()->GetFormattedMessage();
        }
        this->ExpectEq(deserialized, value);
    }
}

// Test that serializing a value, then deserializing it with insufficient space, an error is raised.
TYPED_TEST_P(StreamParameterizedTests, SerializeDeserializeOutOfBounds) {
    for (const auto& value : this->GetParams()) {
        ByteVectorSink sink;
        StreamIn(&sink, value);

        // Make the vector 1 byte too small.
        std::vector<uint8_t> src = sink;
        src.pop_back();

        BlobSource source(CreateBlob(std::move(src)));
        TypeParam deserialized;
        auto err = StreamOut(&source, &deserialized);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }
}

// Test that deserializing from an empty source raises an error.
TYPED_TEST_P(StreamParameterizedTests, DeserializeEmpty) {
    BlobSource source(CreateBlob(0));
    TypeParam deserialized;
    auto err = StreamOut(&source, &deserialized);
    EXPECT_TRUE(err.IsError());
    err.AcquireError();
}

REGISTER_TYPED_TEST_SUITE_P(StreamParameterizedTests,
                            SerializeDeserialize,
                            SerializeDeserializeOutOfBounds,
                            DeserializeEmpty);
INSTANTIATE_TYPED_TEST_SUITE_P(DawnUnittests, StreamParameterizedTests, StreamValueTestTypes, );

}  // anonymous namespace
}  // namespace dawn::native::stream
