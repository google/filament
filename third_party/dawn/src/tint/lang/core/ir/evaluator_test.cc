// Copyright 2024 The Dawn & Tint Authors
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

#include "gmock/gmock.h"

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/evaluator.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/type/u32.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::core::ir::eval::compile_time {
namespace {

using IR_EvaluatorTest = ir::IRTestHelper;

TEST_F(IR_EvaluatorTest, InvalidExpression) {
    auto* inst = b.Negation(mod.Types().u32(), 4_u);
    b.ir.SetSource(inst, Source{{2, 3}});
    auto res = Eval(b, inst);
    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(2:3 error: no matching overload for 'operator - (u32)'

2 candidate operators:
 • 'operator - (T  ✗ ) -> T' where:
      ✗  'T' is 'f32', 'i32' or 'f16'
 • 'operator - (vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32' or 'f16'
)");
}

TEST_F(IR_EvaluatorTest, Bitcast) {
    auto* inst = b.Bitcast(mod.Types().i32(), 4_u);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_TRUE(c->Type()->Is<core::type::I32>());
    EXPECT_EQ(4, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, Unary) {
    auto* inst = b.Complement(mod.Types().i32(), 4_i);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(~4, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, Binary) {
    auto* inst = b.Add(mod.Types().i32(), 1_i, 2_i);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(3, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, ConstructScalar) {
    auto* inst = b.Construct(ty.i32(), 1_i);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(1, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, ConstructArray_Access) {
    auto* obj = b.Construct(ty.array<i32, 3>(), 1_i, 2_i, 3_i);
    auto* inst = b.Access(mod.Types().i32(), obj, 1_u);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(2, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, NestedStruct_AccessSuccess) {
    auto* S = ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("data"), ty.array<u32, 4>()}});

    auto* arr = b.Var("struct", ty.ptr(storage, S));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 0_i, 3_i);
    auto res = Eval(b, inst);

    ASSERT_EQ(res, Success);
}

TEST_F(IR_EvaluatorTest, NestedStruct_AccessFailArray) {
    auto* S = ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("data"), ty.array<u32, 4>()}});

    auto* arr = b.Var("struct", ty.ptr(storage, S));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 0_i, 4_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 4 out of bounds [0..3])");
}

TEST_F(IR_EvaluatorTest, NestedStruct_AccessFailMember) {
    auto* S = ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("data"), ty.array<u32, 4>()}});

    auto* arr = b.Var("struct", ty.ptr(storage, S));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 1_i, 3_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 1 out of bounds [0..0])");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_OutOfBoundsAccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<u32>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, -1_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index -1 out of bounds)");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_OverflowBoundsAccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<u32, 3>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 3_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 3 out of bounds [0..2])");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_NestedOverflowBoundsAccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<array<u32, 3>, 5>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 3_i, 3_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 3 out of bounds [0..2])");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_NestedoundsAccessSuccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<array<u32, 3>, 5>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 4_i, 2_i);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_EvaluatorTest, ArrayBounds_DoubleNestedOverflowBoundsAccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<array<array<u32, 3>, 5>, 7>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 3_i, 3_i, 3_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 3 out of bounds [0..2])");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_DoubleNestedBoundsAccessSuccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<array<array<u32, 3>, 5>, 7>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 6_i, 4_i, 2_i);
    auto res = Eval(b, inst);

    ASSERT_EQ(res, Success);
}

TEST_F(IR_EvaluatorTest, ArrayBounds_DoubleNestedEarlyOverflowBoundsAccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<array<array<u32, 3>, 5>, 7>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 7_i, 3_i, 3_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 7 out of bounds [0..6])");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_DoubleNestedMidOverflowBoundsAccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<array<array<u32, 3>, 5>, 7>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 3_i, 5_i, 3_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 5 out of bounds [0..4])");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_NestedVecOverflowBoundsAccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<vec3<u32>, 7>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 3_i, 3_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 3 out of bounds [0..2])");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_NestedVecBoundsAccessSuccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<vec3<u32>, 7>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 3_i, 2_i);
    auto res = Eval(b, inst);

    ASSERT_EQ(res, Success);
}

TEST_F(IR_EvaluatorTest, ConstructStruct_Access) {
    auto* S = ty.Struct(mod.symbols.New("S"),
                        {{mod.symbols.New("a"), ty.i32()}, {mod.symbols.New("b"), ty.f32()}});

    auto* obj = b.Construct(S, 1_i, 2_f);
    auto* inst = b.Access(mod.Types().i32(), obj, 1_u);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_FLOAT_EQ(2, c->Value()->ValueAs<float>());
}

TEST_F(IR_EvaluatorTest, ConstructVector_Swizzle) {
    auto* obj = b.Construct(ty.vec3<i32>(), 1_i, 2_i, 3_i);
    auto* inst = b.Swizzle(mod.Types().i32(), obj, {1});
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(2, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, ConstructVector_Access) {
    auto* obj = b.Construct(ty.vec3<i32>(), 1_i, 2_i, 3_i);
    auto* inst = b.Access(mod.Types().i32(), obj, 1_u);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(2, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, Convert) {
    auto* obj = b.Construct(ty.i32(), 1_i);
    auto* inst = b.Convert(ty.u32(), obj);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    ASSERT_EQ(ty.u32(), c->Type());
    EXPECT_EQ(1u, c->Value()->ValueAs<uint32_t>());
}

TEST_F(IR_EvaluatorTest, ConstExprIfSimple) {
    auto* constexpr_if = b.ConstExprIf(true);
    constexpr_if->SetResult(b.InstructionResult(ty.bool_()));
    b.Append(constexpr_if->True(), [&] { b.ExitIf(constexpr_if, true); });
    b.Append(constexpr_if->False(), [&] { b.ExitIf(constexpr_if, false); });
    auto res = Eval(b, constexpr_if);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    ASSERT_EQ(ty.bool_(), c->Type());
    EXPECT_EQ(true, c->Value()->ValueAs<bool>());
}

TEST_F(IR_EvaluatorTest, BuiltinCall) {
    auto* inst = b.Call(ty.i32(), core::BuiltinFn::kAbs, -1_i);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(1, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, MultiExpression) {
    auto* abs = b.Call(ty.i32(), core::BuiltinFn::kAbs, -1_i);
    auto* mul = b.Multiply(ty.i32(), abs, 5_i);
    auto* cons = b.Construct(ty.vec2<i32>(), mul, mul);
    auto* inst = b.Swizzle(ty.i32(), cons, {1});

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(5, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, NonConstBuiltinCall) {
    auto* inst = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstBuiltinCallArg) {
    auto* dpdx = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);
    auto* inst = b.Call(ty.f32(), core::BuiltinFn::kAbs, dpdx);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstCallInsideUnary) {
    auto* dpdx = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);
    auto* inst = b.Unary(core::UnaryOp::kNegation, ty.f32(), dpdx);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstCallInsideBinaryRHS) {
    auto* dpdx = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);
    auto* inst = b.Add(ty.f32(), 1.0_f, dpdx);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstCallInsideBinaryLHS) {
    auto* dpdx = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);
    auto* inst = b.Add(ty.f32(), dpdx, 1.0_f);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstCallInsideSwizzle) {
    auto* dpdx =
        b.Call(ty.vec2<f32>(), core::BuiltinFn::kDpdx, b.Construct(ty.vec2<f32>(), 2.0_f, 2.0_f));
    auto* inst = b.Swizzle(ty.f32(), dpdx, {1});

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstCallInsideConstruct) {
    auto* dpdx = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);
    auto* inst = b.Construct(ty.vec2<f32>(), dpdx, 2_f);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstCallInsideConvert) {
    auto* dpdx = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);
    auto* inst = b.Convert(ty.u32(), dpdx);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstCallInsideBitcast) {
    auto* dpdx = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);
    auto* inst = b.Bitcast(ty.u32(), dpdx);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstCallInsideAccessObject) {
    auto* dpdx =
        b.Call(ty.vec2<f32>(), core::BuiltinFn::kDpdx, b.Construct(ty.vec2<f32>(), 2.0_f, 2.0_f));
    auto* inst = b.Access(ty.f32(), dpdx, 0_u);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

}  // namespace
}  // namespace tint::core::ir::eval::compile_time
