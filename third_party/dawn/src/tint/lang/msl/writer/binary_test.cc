
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

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/msl/writer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::msl::writer {
namespace {

struct BinaryData {
    const char* result;
    core::BinaryOp op;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
    StringStream str;
    str << data.op;
    out << str.str();
    return out;
}

using MslWriterBinaryTest = MslWriterTestWithParam<BinaryData>;
TEST_P(MslWriterBinaryTest, Emit) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(params.op, ty.u32(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  uint const left = 1u;
  uint const right = 2u;
  uint const val = )" + params.result +
                               R"(;
}
)");
}
INSTANTIATE_TEST_SUITE_P(MslWriterTest,
                         MslWriterBinaryTest,
                         testing::Values(BinaryData{"(left + right)", core::BinaryOp::kAdd},
                                         BinaryData{"(left - right)", core::BinaryOp::kSubtract},
                                         BinaryData{"(left * right)", core::BinaryOp::kMultiply},
                                         BinaryData{"(left & right)", core::BinaryOp::kAnd},
                                         BinaryData{"(left | right)", core::BinaryOp::kOr},
                                         BinaryData{"(left ^ right)", core::BinaryOp::kXor}));

TEST_F(MslWriterTest, BinaryDivU32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(core::BinaryOp::kDivide, ty.u32(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / select(rhs, 1u, (rhs == 0u)));
}

void foo() {
  uint const left = 1u;
  uint const right = 2u;
  uint const val = tint_div_u32(left, right);
}
)");
}

TEST_F(MslWriterTest, BinaryModU32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(core::BinaryOp::kModulo, ty.u32(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
uint tint_mod_u32(uint lhs, uint rhs) {
  return (lhs - ((lhs / select(rhs, 1u, (rhs == 0u))) * select(rhs, 1u, (rhs == 0u))));
}

void foo() {
  uint const left = 1u;
  uint const right = 2u;
  uint const val = tint_mod_u32(left, right);
}
)");
}

TEST_F(MslWriterTest, BinaryShiftLeft) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(core::BinaryOp::kShiftLeft, ty.u32(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  uint const left = 1u;
  uint const right = 2u;
  uint const val = (left << (right & 31u));
}
)");
}

TEST_F(MslWriterTest, BinaryShiftRight) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(core::BinaryOp::kShiftRight, ty.u32(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  uint const left = 1u;
  uint const right = 2u;
  uint const val = (left >> (right & 31u));
}
)");
}

using MslWriterBinaryBoolTest = MslWriterTestWithParam<BinaryData>;
TEST_P(MslWriterBinaryBoolTest, Emit) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(params.op, ty.bool_(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  uint const left = 1u;
  uint const right = 2u;
  bool const val = )" + params.result +
                               R"(;
}
)");
}
INSTANTIATE_TEST_SUITE_P(
    MslWriterTest,
    MslWriterBinaryBoolTest,
    testing::Values(BinaryData{"(left == right)", core::BinaryOp::kEqual},
                    BinaryData{"(left != right)", core::BinaryOp::kNotEqual},
                    BinaryData{"(left < right)", core::BinaryOp::kLessThan},
                    BinaryData{"(left > right)", core::BinaryOp::kGreaterThan},
                    BinaryData{"(left <= right)", core::BinaryOp::kLessThanEqual},
                    BinaryData{"(left >= right)", core::BinaryOp::kGreaterThanEqual}));

using MslWriterBinaryTest_SignedOverflowDefinedBehaviour = MslWriterTestWithParam<BinaryData>;
TEST_P(MslWriterBinaryTest_SignedOverflowDefinedBehaviour, Emit_Scalar) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_i));
        auto* r = b.Let("right", b.Constant(3_i));

        auto* bin = b.Binary(params.op, ty.i32(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  int const left = 1;
  int const right = 3;
  int const val = as_type<int>((as_type<uint>(left) )" +
                               params.result + R"( as_type<uint>(right)));
}
)");
}

TEST_P(MslWriterBinaryTest_SignedOverflowDefinedBehaviour, Emit_Vector) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Splat<vec4<i32>>(1_i));
        auto* r = b.Let("right", b.Splat<vec4<i32>>(3_i));

        auto* bin = b.Binary(params.op, ty.vec4<i32>(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  int4 const left = int4(1);
  int4 const right = int4(3);
  int4 const val = as_type<int4>((as_type<uint4>(left) )" +
                               params.result + R"( as_type<uint4>(right)));
}
)");
}

constexpr BinaryData signed_overflow_defined_behaviour_cases[] = {
    {"+", core::BinaryOp::kAdd},
    {"-", core::BinaryOp::kSubtract},
    {"*", core::BinaryOp::kMultiply},
};
INSTANTIATE_TEST_SUITE_P(MslWriterTest,
                         MslWriterBinaryTest_SignedOverflowDefinedBehaviour,
                         testing::ValuesIn(signed_overflow_defined_behaviour_cases));

using MslWriterBinaryTest_ShiftSignedOverflowDefinedBehaviour = MslWriterTestWithParam<BinaryData>;
TEST_P(MslWriterBinaryTest_ShiftSignedOverflowDefinedBehaviour, Emit) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_i));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(params.op, ty.i32(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  int const left = 1;
  uint const right = 2u;
  int const val = )" + params.result +
                               R"(;
}
)");
}

constexpr BinaryData shift_signed_overflow_defined_behaviour_cases[] = {
    {"as_type<int>((as_type<uint>(left) << (right & 31u)))", core::BinaryOp::kShiftLeft},
    {"(left >> (right & 31u))", core::BinaryOp::kShiftRight}};
INSTANTIATE_TEST_SUITE_P(MslWriterTest,
                         MslWriterBinaryTest_ShiftSignedOverflowDefinedBehaviour,
                         testing::ValuesIn(shift_signed_overflow_defined_behaviour_cases));

using MslWriterBinaryTest_SignedOverflowDefinedBehaviour_Chained =
    MslWriterTestWithParam<BinaryData>;
TEST_P(MslWriterBinaryTest_SignedOverflowDefinedBehaviour_Chained, Emit) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* left = b.Let("left", 1_i);
        auto* right = b.Let("right", 2_i);
        auto* expr1 = b.Binary(params.op, ty.i32(), left, right);
        auto* expr2 = b.Binary(params.op, ty.i32(), expr1, right);

        b.Let("val", expr2);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  int const left = 1;
  int const right = 2;
  int const val = )" + params.result +
                               R"(;
}
)");
}
constexpr BinaryData signed_overflow_defined_behaviour_chained_cases[] = {
    {R"(as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(left) + as_type<uint>(right)))) + as_type<uint>(right))))",
     core::BinaryOp::kAdd},
    {R"(as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(left) - as_type<uint>(right)))) - as_type<uint>(right))))",
     core::BinaryOp::kSubtract},
    {R"(as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(left) * as_type<uint>(right)))) * as_type<uint>(right))))",
     core::BinaryOp::kMultiply}};
INSTANTIATE_TEST_SUITE_P(MslWriterTest,
                         MslWriterBinaryTest_SignedOverflowDefinedBehaviour_Chained,
                         testing::ValuesIn(signed_overflow_defined_behaviour_chained_cases));

using MslWriterBinaryTest_ShiftSignedOverflowDefinedBehaviour_Chained =
    MslWriterTestWithParam<BinaryData>;
TEST_P(MslWriterBinaryTest_ShiftSignedOverflowDefinedBehaviour_Chained, Emit) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* left = b.Let("left", b.Constant(1_i));
        auto* right = b.Let("right", b.Constant(2_u));
        auto* expr1 = b.Binary(params.op, ty.i32(), left, right);
        auto* expr2 = b.Binary(params.op, ty.i32(), expr1, right);

        b.Let("val", expr2);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  int const left = 1;
  uint const right = 2u;
  int const val = )" + params.result +
                               R"(;
}
)");
}
constexpr BinaryData shift_signed_overflow_defined_behaviour_chained_cases[] = {
    {R"(as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(left) << (right & 31u)))) << (right & 31u))))",
     core::BinaryOp::kShiftLeft},
    {R"(((left >> (right & 31u)) >> (right & 31u)))", core::BinaryOp::kShiftRight},
};
INSTANTIATE_TEST_SUITE_P(MslWriterTest,
                         MslWriterBinaryTest_ShiftSignedOverflowDefinedBehaviour_Chained,
                         testing::ValuesIn(shift_signed_overflow_defined_behaviour_chained_cases));

TEST_F(MslWriterTest, BinaryModF32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr<core::AddressSpace::kFunction, f32>());
        auto* right = b.Var("right", ty.ptr<core::AddressSpace::kFunction, f32>());

        auto* l = b.Load(left);
        auto* r = b.Load(right);
        auto* expr1 = b.Binary(core::BinaryOp::kModulo, ty.f32(), l, r);

        b.Let("val", expr1);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float left = 0.0f;
  float right = 0.0f;
  float const val = fmod(left, right);
}
)");
}

TEST_F(MslWriterTest, BinaryModF16) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr<core::AddressSpace::kFunction, f16>());
        auto* right = b.Var("right", ty.ptr<core::AddressSpace::kFunction, f16>());

        auto* l = b.Load(left);
        auto* r = b.Load(right);
        auto* expr1 = b.Binary(core::BinaryOp::kModulo, ty.f16(), l, r);

        b.Let("val", expr1);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  half left = 0.0h;
  half right = 0.0h;
  half const val = fmod(left, right);
}
)");
}

TEST_F(MslWriterTest, BinaryModVec3F32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr(core::AddressSpace::kFunction, ty.vec3<f32>()));
        auto* right = b.Var("right", ty.ptr(core::AddressSpace::kFunction, ty.vec3<f32>()));

        auto* l = b.Load(left);
        auto* r = b.Load(right);
        auto* expr1 = b.Binary(core::BinaryOp::kModulo, ty.vec3<f32>(), l, r);

        b.Let("val", expr1);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float3 left = 0.0f;
  float3 right = 0.0f;
  float3 const val = fmod(left, right);
}
)");
}

TEST_F(MslWriterTest, BinaryModVec3F16) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr(core::AddressSpace::kFunction, ty.vec3<f16>()));
        auto* right = b.Var("right", ty.ptr(core::AddressSpace::kFunction, ty.vec3<f16>()));

        auto* l = b.Load(left);
        auto* r = b.Load(right);
        auto* expr1 = b.Binary(core::BinaryOp::kModulo, ty.vec3<f16>(), l, r);

        b.Let("val", expr1);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  half3 left = 0.0h;
  half3 right = 0.0h;
  half3 const val = fmod(left, right);
}
)");
}

}  // namespace
}  // namespace tint::msl::writer
