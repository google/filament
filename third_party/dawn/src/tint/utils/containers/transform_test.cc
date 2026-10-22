// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/utils/containers/transform.h"

#include <string>
#include <type_traits>

#include "gmock/gmock.h"

#define CHECK_ELEMENT_TYPE(vector, expected)                                   \
    static_assert(std::is_same<decltype(vector)::value_type, expected>::value, \
                  "unexpected result vector element type")

namespace tint {
namespace {

TEST(TransformTest, StdVectorEmpty) {
    const std::vector<int> empty{};
    {
        auto transformed = Transform(empty, [](int) -> int {
            [] { FAIL() << "Callback should not be called for empty vector"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_EQ(transformed.size(), 0u);
    }
    {
        auto transformed = Transform(empty, [](int, size_t) -> int {
            [] { FAIL() << "Callback should not be called for empty vector"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_EQ(transformed.size(), 0u);
    }
}

TEST(TransformTest, StdVectorIdentity) {
    const std::vector<int> input{1, 2, 3, 4};
    auto transformed = Transform(input, [](int i) { return i; });
    CHECK_ELEMENT_TYPE(transformed, int);
    EXPECT_THAT(transformed, testing::ElementsAre(1, 2, 3, 4));
}

TEST(TransformTest, StdVectorIdentityWithIndex) {
    const std::vector<int> input{1, 2, 3, 4};
    auto transformed = Transform(input, [](int i, size_t) { return i; });
    CHECK_ELEMENT_TYPE(transformed, int);
    EXPECT_THAT(transformed, testing::ElementsAre(1, 2, 3, 4));
}

TEST(TransformTest, StdVectorIndex) {
    const std::vector<int> input{10, 20, 30, 40};
    {
        auto transformed = Transform(input, [](int, size_t idx) { return idx; });
        CHECK_ELEMENT_TYPE(transformed, size_t);
        EXPECT_THAT(transformed, testing::ElementsAre(0u, 1u, 2u, 3u));
    }
}

TEST(TransformTest, TransformStdVectorSameType) {
    const std::vector<int> input{1, 2, 3, 4};
    {
        auto transformed = Transform(input, [](int i) { return i * 10; });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_THAT(transformed, testing::ElementsAre(10, 20, 30, 40));
    }
}

TEST(TransformTest, TransformStdVectorDifferentType) {
    const std::vector<int> input{1, 2, 3, 4};
    {
        auto transformed = Transform(input, [](int i) { return std::to_string(i); });
        CHECK_ELEMENT_TYPE(transformed, std::string);
        EXPECT_THAT(transformed, testing::ElementsAre("1", "2", "3", "4"));
    }
}

TEST(TransformNTest, StdVectorEmpty) {
    const std::vector<int> empty{};
    {
        auto transformed = TransformN(empty, 4u, [](int) -> int {
            [] { FAIL() << "Callback should not be called for empty vector"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_EQ(transformed.size(), 0u);
    }
    {
        auto transformed = TransformN(empty, 4u, [](int, size_t) -> int {
            [] { FAIL() << "Callback should not be called for empty vector"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_EQ(transformed.size(), 0u);
    }
}

TEST(TransformNTest, StdVectorIdentity) {
    const std::vector<int> input{1, 2, 3, 4};
    {
        auto transformed = TransformN(input, 0u, [](int) {
            [] { FAIL() << "Callback should not call the transform when n == 0"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_TRUE(transformed.empty());
    }
    {
        auto transformed = TransformN(input, 2u, [](int i) { return i; });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_THAT(transformed, testing::ElementsAre(1, 2));
    }
    {
        auto transformed = TransformN(input, 6u, [](int i) { return i; });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_THAT(transformed, testing::ElementsAre(1, 2, 3, 4));
    }
}

TEST(TransformNTest, StdVectorIdentityWithIndex) {
    const std::vector<int> input{1, 2, 3, 4};
    {
        auto transformed = TransformN(input, 0u, [](int, size_t) {
            [] { FAIL() << "Callback should not call the transform when n == 0"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_TRUE(transformed.empty());
    }
    {
        auto transformed = TransformN(input, 3u, [](int i, size_t) { return i; });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_THAT(transformed, testing::ElementsAre(1, 2, 3));
    }
    {
        auto transformed = TransformN(input, 9u, [](int i, size_t) { return i; });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_THAT(transformed, testing::ElementsAre(1, 2, 3, 4));
    }
}

TEST(TransformNTest, StdVectorIndex) {
    const std::vector<int> input{10, 20, 30, 40};
    {
        auto transformed = TransformN(input, 0u, [](int, size_t) {
            [] { FAIL() << "Callback should not call the transform when n == 0"; }();
            return static_cast<size_t>(0);
        });
        CHECK_ELEMENT_TYPE(transformed, size_t);
        EXPECT_TRUE(transformed.empty());
    }
    {
        auto transformed = TransformN(input, 2u, [](int, size_t idx) { return idx; });
        CHECK_ELEMENT_TYPE(transformed, size_t);
        EXPECT_THAT(transformed, testing::ElementsAre(0u, 1u));
    }
    {
        auto transformed = TransformN(input, 9u, [](int, size_t idx) { return idx; });
        CHECK_ELEMENT_TYPE(transformed, size_t);
        EXPECT_THAT(transformed, testing::ElementsAre(0u, 1u, 2u, 3u));
    }
}

TEST(TransformNTest, StdVectorTransformSameType) {
    const std::vector<int> input{1, 2, 3, 4};
    {
        auto transformed = TransformN(input, 0u, [](int, size_t) {
            [] { FAIL() << "Callback should not call the transform when n == 0"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_TRUE(transformed.empty());
    }
    {
        auto transformed = TransformN(input, 2u, [](int i) { return i * 10; });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_THAT(transformed, testing::ElementsAre(10, 20));
    }
    {
        auto transformed = TransformN(input, 9u, [](int i) { return i * 10; });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_THAT(transformed, testing::ElementsAre(10, 20, 30, 40));
    }
}

TEST(TransformNTest, StdVectorTransformDifferentType) {
    const std::vector<int> input{1, 2, 3, 4};
    {
        auto transformed = TransformN(input, 0u, [](int) {
            [] { FAIL() << "Callback should not call the transform when n == 0"; }();
            return std::string();
        });
        CHECK_ELEMENT_TYPE(transformed, std::string);
        EXPECT_TRUE(transformed.empty());
    }
    {
        auto transformed = TransformN(input, 2u, [](int i) { return std::to_string(i); });
        CHECK_ELEMENT_TYPE(transformed, std::string);
        EXPECT_THAT(transformed, testing::ElementsAre("1", "2"));
    }
    {
        auto transformed = TransformN(input, 9u, [](int i) { return std::to_string(i); });
        CHECK_ELEMENT_TYPE(transformed, std::string);
        EXPECT_THAT(transformed, testing::ElementsAre("1", "2", "3", "4"));
    }
}

TEST(TransformTest, TintVectorEmpty) {
    const Vector<int, 4> empty{};
    {
        auto transformed = Transform(empty, [](int) -> int {
            [] { FAIL() << "Callback should not be called for empty vector"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_EQ(transformed.Length(), 0u);
    }
    {
        auto transformed = Transform(empty, [](int, size_t) -> int {
            [] { FAIL() << "Callback should not be called for empty vector"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_EQ(transformed.Length(), 0u);
    }
}

TEST(TransformTest, TintVectorIdentity) {
    const Vector<int, 4> input{1, 2, 3, 4};
    auto transformed = Transform(input, [](int i) { return i; });
    CHECK_ELEMENT_TYPE(transformed, int);
    EXPECT_THAT(transformed, testing::ElementsAre(1, 2, 3, 4));
}

TEST(TransformTest, TintVectorIdentityWithIndex) {
    const Vector<int, 4> input{1, 2, 3, 4};
    auto transformed = Transform(input, [](int i, size_t) { return i; });
    CHECK_ELEMENT_TYPE(transformed, int);
    EXPECT_THAT(transformed, testing::ElementsAre(1, 2, 3, 4));
}

TEST(TransformTest, TintVectorIndex) {
    const Vector<int, 4> input{10, 20, 30, 40};
    {
        auto transformed = Transform(input, [](int, size_t idx) { return idx; });
        CHECK_ELEMENT_TYPE(transformed, size_t);
        EXPECT_THAT(transformed, testing::ElementsAre(0u, 1u, 2u, 3u));
    }
}

TEST(TransformTest, TransformTintVectorSameType) {
    const Vector<int, 4> input{1, 2, 3, 4};
    {
        auto transformed = Transform(input, [](int i) { return i * 10; });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_THAT(transformed, testing::ElementsAre(10, 20, 30, 40));
    }
}

TEST(TransformTest, TransformTintVectorDifferentType) {
    const Vector<int, 4> input{1, 2, 3, 4};
    {
        auto transformed = Transform(input, [](int i) { return std::to_string(i); });
        CHECK_ELEMENT_TYPE(transformed, std::string);
        EXPECT_THAT(transformed, testing::ElementsAre("1", "2", "3", "4"));
    }
}

TEST(TransformTest, VectorRefEmpty) {
    Vector<int, 4> empty{};
    VectorRef<int> ref(empty);
    {
        auto transformed = Transform<4>(ref, [](int) -> int {
            [] { FAIL() << "Callback should not be called for empty vector"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_EQ(transformed.Length(), 0u);
    }
    {
        auto transformed = Transform<4>(ref, [](int, size_t) -> int {
            [] { FAIL() << "Callback should not be called for empty vector"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_EQ(transformed.Length(), 0u);
    }
}

TEST(TransformTest, VectorRefIdentity) {
    Vector<int, 4> input{1, 2, 3, 4};
    VectorRef<int> ref(input);
    auto transformed = Transform<8>(ref, [](int i) { return i; });
    CHECK_ELEMENT_TYPE(transformed, int);
    EXPECT_THAT(transformed, testing::ElementsAre(1, 2, 3, 4));
}

TEST(TransformTest, VectorRefIdentityWithIndex) {
    Vector<int, 4> input{1, 2, 3, 4};
    VectorRef<int> ref(input);
    auto transformed = Transform<2>(ref, [](int i, size_t) { return i; });
    CHECK_ELEMENT_TYPE(transformed, int);
    EXPECT_THAT(transformed, testing::ElementsAre(1, 2, 3, 4));
}

TEST(TransformTest, VectorRefIndex) {
    Vector<int, 4> input{10, 20, 30, 40};
    VectorRef<int> ref(input);
    {
        auto transformed = Transform<4>(ref, [](int, size_t idx) { return idx; });
        CHECK_ELEMENT_TYPE(transformed, size_t);
        EXPECT_THAT(transformed, testing::ElementsAre(0u, 1u, 2u, 3u));
    }
}

TEST(TransformTest, TransformVectorRefSameType) {
    Vector<int, 4> input{1, 2, 3, 4};
    VectorRef<int> ref(input);
    {
        auto transformed = Transform<4>(ref, [](int i) { return i * 10; });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_THAT(transformed, testing::ElementsAre(10, 20, 30, 40));
    }
}

TEST(TransformTest, TransformVectorRefDifferentType) {
    Vector<int, 4> input{1, 2, 3, 4};
    VectorRef<int> ref(input);
    {
        auto transformed = Transform<4>(ref, [](int i) { return std::to_string(i); });
        CHECK_ELEMENT_TYPE(transformed, std::string);
        EXPECT_THAT(transformed, testing::ElementsAre("1", "2", "3", "4"));
    }
}

}  // namespace
}  // namespace tint
