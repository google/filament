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

#include <type_traits>

#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

struct Type {
    template <typename T, std::enable_if_t<IsVector<T>, bool> = true>
    static constexpr bool UsedF16() {
        return std::is_same_v<typename T::type, f16>;
    }

    template <typename T, std::enable_if_t<!IsVector<T>, bool> = true>
    static constexpr bool UsedF16() {
        return std::is_same_v<T, f16>;
    }

    template <typename T>
    static constexpr Type Create() {
        return Type{builder::DataType<T>::AST, builder::DataType<T>::Sem,
                    builder::DataType<T>::ExprFromDouble, UsedF16<T>()};
    }

    builder::ast_type_func_ptr ast;
    builder::sem_type_func_ptr sem;
    builder::ast_expr_from_double_func_ptr expr;
    bool used_f16;
};

// Valids numeric scalar and vector types of all bit width
static constexpr Type k16BitsNumericTypes[] = {
    Type::Create<f16>(),
};
static constexpr Type k32BitsNumericTypes[] = {
    Type::Create<f32>(),
    Type::Create<i32>(),
    Type::Create<u32>(),
    Type::Create<vec2<f16>>(),
};
static constexpr Type k48BitsNumericTypes[] = {
    Type::Create<vec3<f16>>(),
};
static constexpr Type k64BitsNumericTypes[] = {
    Type::Create<vec2<f32>>(),
    Type::Create<vec2<i32>>(),
    Type::Create<vec2<u32>>(),
    Type::Create<vec4<f16>>(),
};
static constexpr Type k96BitsNumericTypes[] = {
    Type::Create<vec3<f32>>(),
    Type::Create<vec3<i32>>(),
    Type::Create<vec3<u32>>(),
};
static constexpr Type k128BitsNumericTypes[] = {
    Type::Create<vec4<f32>>(),
    Type::Create<vec4<i32>>(),
    Type::Create<vec4<u32>>(),
};

static constexpr Type kInvalid[] = {
    // A non-exhaustive selection of uncastable types
    Type::Create<bool>(),
    Type::Create<vec2<bool>>(),
    Type::Create<vec3<bool>>(),
    Type::Create<vec4<bool>>(),
    Type::Create<array<i32, 2>>(),
    Type::Create<array<u32, 3>>(),
    Type::Create<array<f32, 4>>(),
    Type::Create<array<bool, 5>>(),
    Type::Create<mat2x2<f32>>(),
    Type::Create<mat3x3<f32>>(),
    Type::Create<mat4x4<f32>>(),
    Type::Create<ptr<private_, i32>>(),
    Type::Create<ptr<private_, array<i32, 2>>>(),
    Type::Create<ptr<private_, mat2x2<f32>>>(),
};

using ResolverBitcastValidationTest = ResolverTestWithParam<std::tuple<Type, Type>>;

////////////////////////////////////////////////////////////////////////////////
// Valid bitcasts
////////////////////////////////////////////////////////////////////////////////
using ResolverBitcastValidationTestPass = ResolverBitcastValidationTest;
TEST_P(ResolverBitcastValidationTestPass, Test) {
    auto src = std::get<0>(GetParam());
    auto dst = std::get<1>(GetParam());

    if (src.used_f16 || dst.used_f16) {
        Enable(wgsl::Extension::kF16);
    }

    auto* cast = Bitcast(dst.ast(*this), src.expr(*this, 0));
    WrapInFunction(cast);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(TypeOf(cast), dst.sem(*this));
}
INSTANTIATE_TEST_SUITE_P(16Bits,
                         ResolverBitcastValidationTestPass,
                         testing::Combine(testing::ValuesIn(k16BitsNumericTypes),
                                          testing::ValuesIn(k16BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(32Bits,
                         ResolverBitcastValidationTestPass,
                         testing::Combine(testing::ValuesIn(k32BitsNumericTypes),
                                          testing::ValuesIn(k32BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(48Bits,
                         ResolverBitcastValidationTestPass,
                         testing::Combine(testing::ValuesIn(k48BitsNumericTypes),
                                          testing::ValuesIn(k48BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(64Bits,
                         ResolverBitcastValidationTestPass,
                         testing::Combine(testing::ValuesIn(k64BitsNumericTypes),
                                          testing::ValuesIn(k64BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(96Bits,
                         ResolverBitcastValidationTestPass,
                         testing::Combine(testing::ValuesIn(k96BitsNumericTypes),
                                          testing::ValuesIn(k96BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(128Bits,
                         ResolverBitcastValidationTestPass,
                         testing::Combine(testing::ValuesIn(k128BitsNumericTypes),
                                          testing::ValuesIn(k128BitsNumericTypes)));

////////////////////////////////////////////////////////////////////////////////
// Invalid bitcast tests
////////////////////////////////////////////////////////////////////////////////
using ResolverInvalidBitcastTest = ResolverBitcastValidationTest;
TEST_P(ResolverInvalidBitcastTest, Test) {
    auto src = std::get<0>(GetParam());
    auto dst = std::get<1>(GetParam());

    if (src.used_f16 || dst.used_f16) {
        Enable(wgsl::Extension::kF16);
    }

    WrapInFunction(Bitcast(Source{{12, 34}}, dst.ast(*this), src.expr(*this, 0)));

    std::string expected = "12:34 error: no matching call to 'bitcast<${TO}>(${FROM})'";
    expected = ReplaceAll(expected, "${FROM}", src.sem(*this)->FriendlyName());
    expected = ReplaceAll(expected, "${TO}", dst.sem(*this)->FriendlyName());

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), testing::StartsWith(expected));
}
INSTANTIATE_TEST_SUITE_P(InvalidSrcType_16Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(kInvalid),
                                          testing::ValuesIn(k16BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(InvalidSrcType_32Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(kInvalid),
                                          testing::ValuesIn(k32BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(InvalidSrcType_48Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(kInvalid),
                                          testing::ValuesIn(k48BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(InvalidSrcType_64Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(kInvalid),
                                          testing::ValuesIn(k64BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(InvalidSrcType_96Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(kInvalid),
                                          testing::ValuesIn(k96BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(InvalidSrcType_128Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(kInvalid),
                                          testing::ValuesIn(k128BitsNumericTypes)));

INSTANTIATE_TEST_SUITE_P(InvalidDstType_16Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k16BitsNumericTypes),
                                          testing::ValuesIn(kInvalid)));
INSTANTIATE_TEST_SUITE_P(InvalidDstType_32Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k32BitsNumericTypes),
                                          testing::ValuesIn(kInvalid)));
INSTANTIATE_TEST_SUITE_P(InvalidDstType_48Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k48BitsNumericTypes),
                                          testing::ValuesIn(kInvalid)));
INSTANTIATE_TEST_SUITE_P(InvalidDstType_64Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k64BitsNumericTypes),
                                          testing::ValuesIn(kInvalid)));
INSTANTIATE_TEST_SUITE_P(InvalidDstType_96Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k96BitsNumericTypes),
                                          testing::ValuesIn(kInvalid)));
INSTANTIATE_TEST_SUITE_P(InvalidDstType_128Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k128BitsNumericTypes),
                                          testing::ValuesIn(kInvalid)));

INSTANTIATE_TEST_SUITE_P(Incompatible_16BitsTo32Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k16BitsNumericTypes),
                                          testing::ValuesIn(k32BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_16BitsTo48Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k16BitsNumericTypes),
                                          testing::ValuesIn(k48BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_16BitsTo64Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k16BitsNumericTypes),
                                          testing::ValuesIn(k64BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_16BitsTo96Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k16BitsNumericTypes),
                                          testing::ValuesIn(k96BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_16BitsTo128Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k16BitsNumericTypes),
                                          testing::ValuesIn(k128BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_32BitsTo16Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k32BitsNumericTypes),
                                          testing::ValuesIn(k16BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_32BitsTo48Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k32BitsNumericTypes),
                                          testing::ValuesIn(k48BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_32BitsTo64Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k32BitsNumericTypes),
                                          testing::ValuesIn(k64BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_32BitsTo96Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k32BitsNumericTypes),
                                          testing::ValuesIn(k96BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_32BitsTo128Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k32BitsNumericTypes),
                                          testing::ValuesIn(k128BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_48BitsTo16Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k48BitsNumericTypes),
                                          testing::ValuesIn(k16BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_48BitsTo32Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k48BitsNumericTypes),
                                          testing::ValuesIn(k32BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_48BitsTo64Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k48BitsNumericTypes),
                                          testing::ValuesIn(k64BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_48BitsTo96Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k48BitsNumericTypes),
                                          testing::ValuesIn(k96BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_48BitsTo128Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k48BitsNumericTypes),
                                          testing::ValuesIn(k128BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_64BitsTo16Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k64BitsNumericTypes),
                                          testing::ValuesIn(k16BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_64BitsTo32Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k64BitsNumericTypes),
                                          testing::ValuesIn(k32BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_64BitsTo48Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k64BitsNumericTypes),
                                          testing::ValuesIn(k48BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_64BitsTo96Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k64BitsNumericTypes),
                                          testing::ValuesIn(k96BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_64BitsTo128Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k64BitsNumericTypes),
                                          testing::ValuesIn(k128BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_96BitsTo16Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k96BitsNumericTypes),
                                          testing::ValuesIn(k16BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_96BitsTo32Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k96BitsNumericTypes),
                                          testing::ValuesIn(k32BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_96BitsTo48Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k96BitsNumericTypes),
                                          testing::ValuesIn(k48BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_96BitsTo64Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k96BitsNumericTypes),
                                          testing::ValuesIn(k64BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_96BitsTo128Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k96BitsNumericTypes),
                                          testing::ValuesIn(k128BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_128BitsTo16Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k128BitsNumericTypes),
                                          testing::ValuesIn(k16BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_128BitsTo32Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k128BitsNumericTypes),
                                          testing::ValuesIn(k32BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_128BitsTo48Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k128BitsNumericTypes),
                                          testing::ValuesIn(k48BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_128BitsTo64Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k128BitsNumericTypes),
                                          testing::ValuesIn(k64BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(Incompatible_128BitsTo96Bits,
                         ResolverInvalidBitcastTest,
                         testing::Combine(testing::ValuesIn(k128BitsNumericTypes),
                                          testing::ValuesIn(k96BitsNumericTypes)));

////////////////////////////////////////////////////////////////////////////////
// Compile-time bitcasts to NaN or Inf are invalid
////////////////////////////////////////////////////////////////////////////////
using ResolverBitcastValidationTestInvalidConst = tint::resolver::ResolverTest;
TEST_F(ResolverBitcastValidationTestInvalidConst, ConstBitcastToF16NaN) {
    Enable(wgsl::Extension::kF16);

    // Lower 16 bits of const u32 0x7e10 is NaN in f16.
    auto* a = Const("a", Expr(u32(0x00007e10)));
    auto* b = Let("b", Bitcast(Source{{12, 34}}, ty.Of<vec2<f16>>(), Expr("a")));
    WrapInFunction(a, b);

    auto expected = "12:34 error: value nan cannot be represented as 'f16'";

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), expected);
}

TEST_F(ResolverBitcastValidationTestInvalidConst, ConstBitcastToF16Inf) {
    Enable(wgsl::Extension::kF16);

    // 0xfc00 is -Inf in f16.
    auto* a = Const("a", Call<vec2<u32>>(u32(0x00007010), u32(0xfc008000)));
    auto* b = Let("b", Bitcast(Source{{12, 34}}, ty.Of<vec4<f16>>(), Expr("a")));
    WrapInFunction(a, b);

    auto expected = "12:34 error: value -inf cannot be represented as 'f16'";

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), expected);
}

TEST_F(ResolverBitcastValidationTestInvalidConst, ConstBitcastToF32NaN) {
    // 0xffc00000 is NaN in f32.
    auto* a = Const("a", Expr(u32(0xffc00000)));
    auto* b = Let("b", Bitcast(Source{{12, 34}}, ty.Of<f32>(), Expr("a")));
    WrapInFunction(a, b);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), ::testing::HasSubstr("cannot be represented as 'f32'"));
}

TEST_F(ResolverBitcastValidationTestInvalidConst, ConstBitcastToF32Inf) {
    Enable(wgsl::Extension::kF16);

    // 0x7f800000 is Inf in f32.
    auto* a = Const("a", Call<vec3<u32>>(u32(0xA0008000), u32(0x7f800000), u32(0x40000000)));
    auto* b = Let("b", Bitcast(Source{{12, 34}}, ty.Of<vec3<f32>>(), Expr("a")));
    WrapInFunction(a, b);

    auto expected = "12:34 error: value inf cannot be represented as 'f32'";

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), expected);
}

}  // namespace
}  // namespace tint::resolver
