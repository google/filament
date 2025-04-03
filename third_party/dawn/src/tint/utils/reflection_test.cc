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

#include "src/tint/utils/reflection.h"

#include "gmock/gmock.h"

#include "src/tint/utils/rtti/castable.h"

namespace tint {
namespace {

struct S {
    int i;
    unsigned u;
    bool b;
    TINT_REFLECT(S, i, u, b);
};

static_assert(!HasReflection<int>);
static_assert(HasReflection<S>);

TEST(ReflectionTest, ForeachFieldConst) {
    const S s{1, 2, true};
    size_t field_idx = 0;
    ForeachField(s, [&](auto& field) {
        using T = std::decay_t<decltype(field)>;
        switch (field_idx) {
            case 0:
                EXPECT_TRUE((std::is_same_v<T, int>));
                EXPECT_EQ(field, static_cast<T>(1));
                break;
            case 1:
                EXPECT_TRUE((std::is_same_v<T, unsigned>));
                EXPECT_EQ(field, static_cast<T>(2));
                break;
            case 2:
                EXPECT_TRUE((std::is_same_v<T, bool>));
                EXPECT_EQ(field, static_cast<T>(true));
                break;
            default:
                FAIL() << "unexpected field";
                break;
        }
        field_idx++;
    });
}

TEST(ReflectionTest, ForeachFieldNonConst) {
    S s{1, 2, true};
    size_t field_idx = 0;
    ForeachField(s, [&](auto& field) {
        using T = std::decay_t<decltype(field)>;
        switch (field_idx) {
            case 0:
                EXPECT_TRUE((std::is_same_v<T, int>));
                EXPECT_EQ(field, static_cast<T>(1));
                field = static_cast<T>(10);
                break;
            case 1:
                EXPECT_TRUE((std::is_same_v<T, unsigned>));
                EXPECT_EQ(field, static_cast<T>(2));
                field = static_cast<T>(20);
                break;
            case 2:
                EXPECT_TRUE((std::is_same_v<T, bool>));
                EXPECT_EQ(field, static_cast<T>(true));
                field = static_cast<T>(false);
                break;
            default:
                FAIL() << "unexpected field";
                break;
        }
        field_idx++;
    });

    EXPECT_EQ(s.i, 10);
    EXPECT_EQ(s.u, 20u);
    EXPECT_EQ(s.b, false);
}

////////////////////////////////////////////////////////////////////////////////
// TINT_ASSERT_ALL_FIELDS_REFLECTED tests
////////////////////////////////////////////////////////////////////////////////
struct VirtualNonCastable {
    virtual ~VirtualNonCastable() = default;
};

struct VirtualCastable : Castable<VirtualCastable, CastableBase> {
    ~VirtualCastable() override = default;
    int a, b, c;
    TINT_REFLECT(VirtualCastable, a, b, c);
};

struct MissingFirst {
    int a, b, c;
    TINT_REFLECT(MissingFirst, b, c);
};

struct MissingMid {
    int a, b, c;
    TINT_REFLECT(MissingMid, a, c);
};

struct MissingLast {
    int a, b, c;
    TINT_REFLECT(MissingLast, a, b);
};

TEST(TintCheckAllFieldsReflected, Tests) {
    TINT_ASSERT_ALL_FIELDS_REFLECTED(S);
    TINT_ASSERT_ALL_FIELDS_REFLECTED(VirtualCastable);

    auto missing_first = reflection::detail::CheckAllFieldsReflected<MissingFirst>();
    ASSERT_NE(missing_first, Success);
    EXPECT_THAT(missing_first.Failure().reason,
                testing::HasSubstr(R"(TINT_REFLECT(MissingFirst, ...) field mismatch at 'b'.
Expected field offset of 0 bytes, but field was at 4 bytes)"));

    auto missing_mid = reflection::detail::CheckAllFieldsReflected<MissingMid>();
    ASSERT_NE(missing_mid, Success);
    EXPECT_THAT(missing_mid.Failure().reason,
                testing::HasSubstr(R"(TINT_REFLECT(MissingMid, ...) field mismatch at 'c'.
Expected field offset of 4 bytes, but field was at 8 bytes)"));

    auto missing_last = reflection::detail::CheckAllFieldsReflected<MissingLast>();
    ASSERT_NE(missing_last, Success);
    EXPECT_THAT(missing_last.Failure().reason,
                testing::HasSubstr(R"(TINT_REFLECT(MissingLast, ...) missing fields at end of class
Expected class size of 8 bytes, but class is 12 bytes)"));
}

}  // namespace
}  // namespace tint
