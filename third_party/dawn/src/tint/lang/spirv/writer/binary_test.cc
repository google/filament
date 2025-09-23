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

#include "src/tint/lang/spirv/writer/common/helper_test.h"

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/binary.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::spirv::writer {
namespace {

/// A parameterized test case.
struct BinaryTestCase {
    BinaryTestCase(TestElementType type_,
                   core::BinaryOp op_,
                   std::string spirv_inst_,
                   std::string spirv_type_name_)
        : res_type(type_),
          lhs_type(type_),
          rhs_type(type_),
          op(op_),
          spirv_inst(spirv_inst_),
          spirv_type_name(spirv_type_name_) {}

    BinaryTestCase(TestElementType res_type_,
                   TestElementType lhs_type_,
                   TestElementType rhs_type_,
                   core::BinaryOp op_,
                   std::string spirv_inst_,
                   std::string spirv_type_name_)
        : res_type(res_type_),
          lhs_type(lhs_type_),
          rhs_type(rhs_type_),
          op(op_),
          spirv_inst(spirv_inst_),
          spirv_type_name(spirv_type_name_) {}

    /// The result type of the binary op.
    TestElementType res_type;
    /// The LHS type of the binary op.
    TestElementType lhs_type;
    /// The RHS type of the binary op.
    TestElementType rhs_type;
    /// The binary operation.
    core::BinaryOp op;
    /// The expected SPIR-V instruction.
    std::string spirv_inst;
    /// The expected SPIR-V result type name.
    std::string spirv_type_name;
};

using Arithmetic_Bitwise = SpirvWriterTestWithParam<BinaryTestCase>;
TEST_P(Arithmetic_Bitwise, Scalar) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* lhs = MakeScalarValue(params.lhs_type);
        auto* rhs = MakeScalarValue(params.rhs_type);
        auto* result = b.Binary(params.op, MakeScalarType(params.res_type), lhs, rhs);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %" + params.spirv_type_name);
}
TEST_P(Arithmetic_Bitwise, Vector) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* lhs = MakeVectorValue(params.lhs_type);
        auto* rhs = MakeVectorValue(params.rhs_type);
        auto* result = b.Binary(params.op, MakeVectorType(params.res_type), lhs, rhs);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %v2" + params.spirv_type_name);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_I32,
    Arithmetic_Bitwise,
    testing::Values(BinaryTestCase{kI32, core::BinaryOp::kAnd, "OpBitwiseAnd", "int"},
                    BinaryTestCase{kI32, core::BinaryOp::kOr, "OpBitwiseOr", "int"},
                    BinaryTestCase{kI32, core::BinaryOp::kXor, "OpBitwiseXor", "int"},
                    BinaryTestCase{kI32, kI32, kU32, core::BinaryOp::kShiftRight,
                                   "OpShiftRightArithmetic", "int"}));
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_U32,
    Arithmetic_Bitwise,
    testing::Values(BinaryTestCase{kU32, core::BinaryOp::kAdd, "OpIAdd", "uint"},
                    BinaryTestCase{kU32, core::BinaryOp::kSubtract, "OpISub", "uint"},
                    BinaryTestCase{kU32, core::BinaryOp::kMultiply, "OpIMul", "uint"},
                    BinaryTestCase{kU32, core::BinaryOp::kAnd, "OpBitwiseAnd", "uint"},
                    BinaryTestCase{kU32, core::BinaryOp::kOr, "OpBitwiseOr", "uint"},
                    BinaryTestCase{kU32, core::BinaryOp::kXor, "OpBitwiseXor", "uint"},
                    BinaryTestCase{kU32, core::BinaryOp::kShiftLeft, "OpShiftLeftLogical", "uint"},
                    BinaryTestCase{kU32, core::BinaryOp::kShiftRight, "OpShiftRightLogical",
                                   "uint"}));
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_F32,
    Arithmetic_Bitwise,
    testing::Values(BinaryTestCase{kF32, core::BinaryOp::kAdd, "OpFAdd", "float"},
                    BinaryTestCase{kF32, core::BinaryOp::kSubtract, "OpFSub", "float"},
                    BinaryTestCase{kF32, core::BinaryOp::kMultiply, "OpFMul", "float"},
                    BinaryTestCase{kF32, core::BinaryOp::kDivide, "OpFDiv", "float"},
                    BinaryTestCase{kF32, core::BinaryOp::kModulo, "OpFRem", "float"}));
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_F16,
    Arithmetic_Bitwise,
    testing::Values(BinaryTestCase{kF16, core::BinaryOp::kAdd, "OpFAdd", "half"},
                    BinaryTestCase{kF16, core::BinaryOp::kSubtract, "OpFSub", "half"},
                    BinaryTestCase{kF16, core::BinaryOp::kMultiply, "OpFMul", "half"},
                    BinaryTestCase{kF16, core::BinaryOp::kDivide, "OpFDiv", "half"},
                    BinaryTestCase{kF16, core::BinaryOp::kModulo, "OpFRem", "half"}));
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_Bool,
    Arithmetic_Bitwise,
    testing::Values(BinaryTestCase{kBool, core::BinaryOp::kAnd, "OpLogicalAnd", "bool"},
                    BinaryTestCase{kBool, core::BinaryOp::kOr, "OpLogicalOr", "bool"}));

TEST_F(SpirvWriterTest, Binary_ScalarTimesVector_F32) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({scalar, vector});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.vec4<f32>(), scalar, vector);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpVectorTimesScalar %v4float %vector %scalar");
}

TEST_F(SpirvWriterTest, Binary_VectorTimesScalar_F32) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({scalar, vector});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.vec4<f32>(), vector, scalar);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpVectorTimesScalar %v4float %vector %scalar");
}

TEST_F(SpirvWriterTest, Binary_ScalarTimesMatrix_F32) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* matrix = b.FunctionParam("matrix", ty.mat3x4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({scalar, matrix});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.mat3x4<f32>(), scalar, matrix);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpMatrixTimesScalar %mat3v4float %matrix %scalar");
}

TEST_F(SpirvWriterTest, Binary_MatrixTimesScalar_F32) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* matrix = b.FunctionParam("matrix", ty.mat3x4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({scalar, matrix});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.mat3x4<f32>(), matrix, scalar);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpMatrixTimesScalar %mat3v4float %matrix %scalar");
}

TEST_F(SpirvWriterTest, Binary_VectorTimesMatrix_F32) {
    auto* vector = b.FunctionParam("vector", ty.vec4<f32>());
    auto* matrix = b.FunctionParam("matrix", ty.mat3x4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({vector, matrix});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.vec3<f32>(), vector, matrix);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpVectorTimesMatrix %v3float %vector %matrix");
}

TEST_F(SpirvWriterTest, Binary_MatrixTimesVector_F32) {
    auto* vector = b.FunctionParam("vector", ty.vec3<f32>());
    auto* matrix = b.FunctionParam("matrix", ty.mat3x4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({vector, matrix});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.vec4<f32>(), matrix, vector);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpMatrixTimesVector %v4float %matrix %vector");
}

TEST_F(SpirvWriterTest, Binary_MatrixTimesMatrix_F32) {
    auto* mat1 = b.FunctionParam("mat1", ty.mat4x3<f32>());
    auto* mat2 = b.FunctionParam("mat2", ty.mat3x4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({mat1, mat2});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.mat3x3<f32>(), mat1, mat2);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpMatrixTimesMatrix %mat3v3float %mat1 %mat2");
}

using Comparison = SpirvWriterTestWithParam<BinaryTestCase>;
TEST_P(Comparison, Scalar) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* lhs = MakeScalarValue(params.lhs_type);
        auto* rhs = MakeScalarValue(params.rhs_type);
        auto* result = b.Binary(params.op, ty.bool_(), lhs, rhs);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %bool");
}

TEST_P(Comparison, Vector) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* lhs = MakeVectorValue(params.lhs_type);
        auto* rhs = MakeVectorValue(params.rhs_type);
        auto* result = b.Binary(params.op, ty.vec2<bool>(), lhs, rhs);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %v2bool");
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_I32,
    Comparison,
    testing::Values(
        BinaryTestCase{kI32, core::BinaryOp::kEqual, "OpIEqual", "bool"},
        BinaryTestCase{kI32, core::BinaryOp::kNotEqual, "OpINotEqual", "bool"},
        BinaryTestCase{kI32, core::BinaryOp::kGreaterThan, "OpSGreaterThan", "bool"},
        BinaryTestCase{kI32, core::BinaryOp::kGreaterThanEqual, "OpSGreaterThanEqual", "bool"},
        BinaryTestCase{kI32, core::BinaryOp::kLessThan, "OpSLessThan", "bool"},
        BinaryTestCase{kI32, core::BinaryOp::kLessThanEqual, "OpSLessThanEqual", "bool"}));
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_U32,
    Comparison,
    testing::Values(
        BinaryTestCase{kU32, core::BinaryOp::kEqual, "OpIEqual", "bool"},
        BinaryTestCase{kU32, core::BinaryOp::kNotEqual, "OpINotEqual", "bool"},
        BinaryTestCase{kU32, core::BinaryOp::kGreaterThan, "OpUGreaterThan", "bool"},
        BinaryTestCase{kU32, core::BinaryOp::kGreaterThanEqual, "OpUGreaterThanEqual", "bool"},
        BinaryTestCase{kU32, core::BinaryOp::kLessThan, "OpULessThan", "bool"},
        BinaryTestCase{kU32, core::BinaryOp::kLessThanEqual, "OpULessThanEqual", "bool"}));
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_F32,
    Comparison,
    testing::Values(
        BinaryTestCase{kF32, core::BinaryOp::kEqual, "OpFOrdEqual", "bool"},
        BinaryTestCase{kF32, core::BinaryOp::kNotEqual, "OpFOrdNotEqual", "bool"},
        BinaryTestCase{kF32, core::BinaryOp::kGreaterThan, "OpFOrdGreaterThan", "bool"},
        BinaryTestCase{kF32, core::BinaryOp::kGreaterThanEqual, "OpFOrdGreaterThanEqual", "bool"},
        BinaryTestCase{kF32, core::BinaryOp::kLessThan, "OpFOrdLessThan", "bool"},
        BinaryTestCase{kF32, core::BinaryOp::kLessThanEqual, "OpFOrdLessThanEqual", "bool"}));
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_F16,
    Comparison,
    testing::Values(
        BinaryTestCase{kF16, core::BinaryOp::kEqual, "OpFOrdEqual", "bool"},
        BinaryTestCase{kF16, core::BinaryOp::kNotEqual, "OpFOrdNotEqual", "bool"},
        BinaryTestCase{kF16, core::BinaryOp::kGreaterThan, "OpFOrdGreaterThan", "bool"},
        BinaryTestCase{kF16, core::BinaryOp::kGreaterThanEqual, "OpFOrdGreaterThanEqual", "bool"},
        BinaryTestCase{kF16, core::BinaryOp::kLessThan, "OpFOrdLessThan", "bool"},
        BinaryTestCase{kF16, core::BinaryOp::kLessThanEqual, "OpFOrdLessThanEqual", "bool"}));
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_Bool,
    Comparison,
    testing::Values(BinaryTestCase{kBool, core::BinaryOp::kEqual, "OpLogicalEqual", "bool"},
                    BinaryTestCase{kBool, core::BinaryOp::kNotEqual, "OpLogicalNotEqual", "bool"}));

TEST_F(SpirvWriterTest, Binary_Chain) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* sub = b.Subtract(ty.i32(), 1_i, 2_i);
        auto* add = b.Add(ty.i32(), sub, sub);
        b.Return(func);
        mod.SetName(sub, "sub");
        mod.SetName(add, "add");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpBitcast %uint %int_1");
    EXPECT_INST("OpBitcast %uint %int_2");
    EXPECT_INST("OpISub %uint %6 %9");
    EXPECT_INST("OpIAdd %uint %13 %14");
}

TEST_F(SpirvWriterTest, Divide_u32_u32) {
    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("lhs", ty.u32()));
    args.Push(b.FunctionParam("rhs", ty.u32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Binary(core::BinaryOp::kDivide, ty.u32(), args[0], args[1]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %uint None %5
        %lhs = OpFunctionParameter %uint
        %rhs = OpFunctionParameter %uint
          %6 = OpLabel
     %result = OpFunctionCall %uint %tint_div_u32 %lhs %rhs
               OpReturnValue %result
               OpFunctionEnd

               ; Function tint_div_u32
%tint_div_u32 = OpFunction %uint None %5
      %lhs_0 = OpFunctionParameter %uint
      %rhs_0 = OpFunctionParameter %uint
         %11 = OpLabel
         %12 = OpIEqual %bool %rhs_0 %uint_0
         %15 = OpSelect %uint %12 %uint_1 %rhs_0
         %17 = OpUDiv %uint %lhs_0 %15
               OpReturnValue %17
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Divide_i32_i32) {
    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("lhs", ty.i32()));
    args.Push(b.FunctionParam("rhs", ty.i32()));
    auto* func = b.Function("foo", ty.i32());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Binary(core::BinaryOp::kDivide, ty.i32(), args[0], args[1]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %int None %5
        %lhs = OpFunctionParameter %int
        %rhs = OpFunctionParameter %int
          %6 = OpLabel
     %result = OpFunctionCall %int %tint_div_i32 %lhs %rhs
               OpReturnValue %result
               OpFunctionEnd

               ; Function tint_div_i32
%tint_div_i32 = OpFunction %int None %5
      %lhs_0 = OpFunctionParameter %int
      %rhs_0 = OpFunctionParameter %int
         %11 = OpLabel
         %12 = OpIEqual %bool %rhs_0 %int_0
         %15 = OpIEqual %bool %lhs_0 %int_n2147483648
         %17 = OpIEqual %bool %rhs_0 %int_n1
         %19 = OpLogicalAnd %bool %15 %17
         %20 = OpLogicalOr %bool %12 %19
         %21 = OpSelect %int %20 %int_1 %rhs_0
         %23 = OpSDiv %int %lhs_0 %21
               OpReturnValue %23
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Divide_i32_vec4i) {
    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("lhs", ty.i32()));
    args.Push(b.FunctionParam("rhs", ty.vec4<i32>()));
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Binary(core::BinaryOp::kDivide, ty.vec4<i32>(), args[0], args[1]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%16 = OpConstantNull %v4int");
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %v4int None %6
        %lhs = OpFunctionParameter %int
        %rhs = OpFunctionParameter %v4int
          %7 = OpLabel
          %8 = OpCompositeConstruct %v4int %lhs %lhs %lhs %lhs
     %result = OpFunctionCall %v4int %tint_div_v4i32 %8 %rhs
               OpReturnValue %result
               OpFunctionEnd

               ; Function tint_div_v4i32
%tint_div_v4i32 = OpFunction %v4int None %13
      %lhs_0 = OpFunctionParameter %v4int
      %rhs_0 = OpFunctionParameter %v4int
         %14 = OpLabel
         %15 = OpIEqual %v4bool %rhs_0 %16
         %19 = OpIEqual %v4bool %lhs_0 %20
         %22 = OpIEqual %v4bool %rhs_0 %23
         %25 = OpLogicalAnd %v4bool %19 %22
         %26 = OpLogicalOr %v4bool %15 %25
         %27 = OpSelect %v4int %26 %28 %rhs_0
         %30 = OpSDiv %v4int %lhs_0 %27
               OpReturnValue %30
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Divide_vec4i_i32) {
    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("lhs", ty.vec4<i32>()));
    args.Push(b.FunctionParam("rhs", ty.i32()));
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Binary(core::BinaryOp::kDivide, ty.vec4<i32>(), args[0], args[1]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%16 = OpConstantNull %v4int");
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %v4int None %6
        %lhs = OpFunctionParameter %v4int
        %rhs = OpFunctionParameter %int
          %7 = OpLabel
          %8 = OpCompositeConstruct %v4int %rhs %rhs %rhs %rhs
     %result = OpFunctionCall %v4int %tint_div_v4i32 %lhs %8
               OpReturnValue %result
               OpFunctionEnd

               ; Function tint_div_v4i32
%tint_div_v4i32 = OpFunction %v4int None %13
      %lhs_0 = OpFunctionParameter %v4int
      %rhs_0 = OpFunctionParameter %v4int
         %14 = OpLabel
         %15 = OpIEqual %v4bool %rhs_0 %16
         %19 = OpIEqual %v4bool %lhs_0 %20
         %22 = OpIEqual %v4bool %rhs_0 %23
         %25 = OpLogicalAnd %v4bool %19 %22
         %26 = OpLogicalOr %v4bool %15 %25
         %27 = OpSelect %v4int %26 %28 %rhs_0
         %30 = OpSDiv %v4int %lhs_0 %27
               OpReturnValue %30
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Modulo_u32_u32) {
    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("lhs", ty.u32()));
    args.Push(b.FunctionParam("rhs", ty.u32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Binary(core::BinaryOp::kModulo, ty.u32(), args[0], args[1]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %uint None %5
        %lhs = OpFunctionParameter %uint
        %rhs = OpFunctionParameter %uint
          %6 = OpLabel
     %result = OpFunctionCall %uint %tint_mod_u32 %lhs %rhs
               OpReturnValue %result
               OpFunctionEnd

               ; Function tint_mod_u32
%tint_mod_u32 = OpFunction %uint None %5
      %lhs_0 = OpFunctionParameter %uint
      %rhs_0 = OpFunctionParameter %uint
         %11 = OpLabel
         %12 = OpIEqual %bool %rhs_0 %uint_0
         %15 = OpSelect %uint %12 %uint_1 %rhs_0
         %17 = OpUDiv %uint %lhs_0 %15
         %18 = OpIMul %uint %17 %15
         %19 = OpISub %uint %lhs_0 %18
               OpReturnValue %19
               OpFunctionEnd
)");
}
TEST_F(SpirvWriterTest, Modulo_i32_i32) {
    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("lhs", ty.i32()));
    args.Push(b.FunctionParam("rhs", ty.i32()));
    auto* func = b.Function("foo", ty.i32());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Binary(core::BinaryOp::kModulo, ty.i32(), args[0], args[1]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %int None %5
        %lhs = OpFunctionParameter %int
        %rhs = OpFunctionParameter %int
          %6 = OpLabel
     %result = OpFunctionCall %int %tint_mod_i32 %lhs %rhs
               OpReturnValue %result
               OpFunctionEnd

               ; Function tint_mod_i32
%tint_mod_i32 = OpFunction %int None %5
      %lhs_0 = OpFunctionParameter %int
      %rhs_0 = OpFunctionParameter %int
         %11 = OpLabel
         %12 = OpIEqual %bool %rhs_0 %int_0
         %15 = OpIEqual %bool %lhs_0 %int_n2147483648
         %17 = OpIEqual %bool %rhs_0 %int_n1
         %19 = OpLogicalAnd %bool %15 %17
         %20 = OpLogicalOr %bool %12 %19
         %21 = OpSelect %int %20 %int_1 %rhs_0
         %23 = OpSDiv %int %lhs_0 %21
         %25 = OpBitcast %uint %23
         %26 = OpBitcast %uint %21
         %27 = OpIMul %uint %25 %26
         %28 = OpBitcast %int %27
         %29 = OpBitcast %uint %lhs_0
         %30 = OpBitcast %uint %28
         %31 = OpISub %uint %29 %30
         %32 = OpBitcast %int %31
               OpReturnValue %32
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %35
         %36 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Add_i32_i32) {
    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("lhs", ty.i32()));
    args.Push(b.FunctionParam("rhs", ty.i32()));
    auto* func = b.Function("foo", ty.i32());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Binary(core::BinaryOp::kAdd, ty.i32(), args[0], args[1]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %unused_entry_point "unused_entry_point"
               OpExecutionMode %unused_entry_point LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %lhs "lhs"                    ; id %3
               OpName %rhs "rhs"                    ; id %4
               OpName %unused_entry_point "unused_entry_point"  ; id %12

               ; Types, variables and constants
        %int = OpTypeInt 32 1
          %5 = OpTypeFunction %int %int %int
       %uint = OpTypeInt 32 0
       %void = OpTypeVoid
         %14 = OpTypeFunction %void

               ; Function foo
        %foo = OpFunction %int None %5
        %lhs = OpFunctionParameter %int
        %rhs = OpFunctionParameter %int
          %6 = OpLabel
          %8 = OpBitcast %uint %lhs
          %9 = OpBitcast %uint %rhs
         %10 = OpIAdd %uint %8 %9
         %11 = OpBitcast %int %10
               OpReturnValue %11
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %14
         %15 = OpLabel
)");
}

TEST_F(SpirvWriterTest, Sub_i32_i32) {
    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("lhs", ty.i32()));
    args.Push(b.FunctionParam("rhs", ty.i32()));
    auto* func = b.Function("foo", ty.i32());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Binary(core::BinaryOp::kSubtract, ty.i32(), args[0], args[1]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %unused_entry_point "unused_entry_point"
               OpExecutionMode %unused_entry_point LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %lhs "lhs"                    ; id %3
               OpName %rhs "rhs"                    ; id %4
               OpName %unused_entry_point "unused_entry_point"  ; id %12

               ; Types, variables and constants
        %int = OpTypeInt 32 1
          %5 = OpTypeFunction %int %int %int
       %uint = OpTypeInt 32 0
       %void = OpTypeVoid
         %14 = OpTypeFunction %void

               ; Function foo
        %foo = OpFunction %int None %5
        %lhs = OpFunctionParameter %int
        %rhs = OpFunctionParameter %int
          %6 = OpLabel
          %8 = OpBitcast %uint %lhs
          %9 = OpBitcast %uint %rhs
         %10 = OpISub %uint %8 %9
         %11 = OpBitcast %int %10
               OpReturnValue %11
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %14
         %15 = OpLabel
)");
}

TEST_F(SpirvWriterTest, Mul_i32_i32) {
    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("lhs", ty.i32()));
    args.Push(b.FunctionParam("rhs", ty.i32()));
    auto* func = b.Function("foo", ty.i32());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Binary(core::BinaryOp::kMultiply, ty.i32(), args[0], args[1]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %unused_entry_point "unused_entry_point"
               OpExecutionMode %unused_entry_point LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %lhs "lhs"                    ; id %3
               OpName %rhs "rhs"                    ; id %4
               OpName %unused_entry_point "unused_entry_point"  ; id %12

               ; Types, variables and constants
        %int = OpTypeInt 32 1
          %5 = OpTypeFunction %int %int %int
       %uint = OpTypeInt 32 0
       %void = OpTypeVoid
         %14 = OpTypeFunction %void

               ; Function foo
        %foo = OpFunction %int None %5
        %lhs = OpFunctionParameter %int
        %rhs = OpFunctionParameter %int
          %6 = OpLabel
          %8 = OpBitcast %uint %lhs
          %9 = OpBitcast %uint %rhs
         %10 = OpIMul %uint %8 %9
         %11 = OpBitcast %int %10
               OpReturnValue %11
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %14
         %15 = OpLabel
)");
}

TEST_F(SpirvWriterTest, ShiftLeft_i32_u32) {
    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("lhs", ty.i32()));
    args.Push(b.FunctionParam("rhs", ty.u32()));
    auto* func = b.Function("foo", ty.i32());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Binary(core::BinaryOp::kShiftLeft, ty.i32(), args[0], args[1]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %unused_entry_point "unused_entry_point"
               OpExecutionMode %unused_entry_point LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %lhs "lhs"                    ; id %3
               OpName %rhs "rhs"                    ; id %5
               OpName %unused_entry_point "unused_entry_point"  ; id %13

               ; Types, variables and constants
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
          %6 = OpTypeFunction %int %int %uint
    %uint_31 = OpConstant %uint 31
       %void = OpTypeVoid
         %15 = OpTypeFunction %void

               ; Function foo
        %foo = OpFunction %int None %6
        %lhs = OpFunctionParameter %int
        %rhs = OpFunctionParameter %uint
          %7 = OpLabel
          %8 = OpBitwiseAnd %uint %rhs %uint_31
         %10 = OpBitcast %uint %lhs
         %11 = OpShiftLeftLogical %uint %10 %8
         %12 = OpBitcast %int %11
               OpReturnValue %12
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %15
         %16 = OpLabel
)");
}

TEST_F(SpirvWriterTest, Modulo_i32_vec4i) {
    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("lhs", ty.i32()));
    args.Push(b.FunctionParam("rhs", ty.vec4<i32>()));
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Binary(core::BinaryOp::kModulo, ty.vec4<i32>(), args[0], args[1]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%16 = OpConstantNull %v4int");
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %v4int None %6
        %lhs = OpFunctionParameter %int
        %rhs = OpFunctionParameter %v4int
          %7 = OpLabel
          %8 = OpCompositeConstruct %v4int %lhs %lhs %lhs %lhs
     %result = OpFunctionCall %v4int %tint_mod_v4i32 %8 %rhs
               OpReturnValue %result
               OpFunctionEnd

               ; Function tint_mod_v4i32
%tint_mod_v4i32 = OpFunction %v4int None %13
      %lhs_0 = OpFunctionParameter %v4int
      %rhs_0 = OpFunctionParameter %v4int
         %14 = OpLabel
         %15 = OpIEqual %v4bool %rhs_0 %16
         %19 = OpIEqual %v4bool %lhs_0 %20
         %22 = OpIEqual %v4bool %rhs_0 %23
         %25 = OpLogicalAnd %v4bool %19 %22
         %26 = OpLogicalOr %v4bool %15 %25
         %27 = OpSelect %v4int %26 %28 %rhs_0
         %30 = OpSDiv %v4int %lhs_0 %27
         %33 = OpBitcast %v4uint %30
         %34 = OpBitcast %v4uint %27
         %35 = OpIMul %v4uint %33 %34
         %36 = OpBitcast %v4int %35
         %37 = OpBitcast %v4uint %lhs_0
         %38 = OpBitcast %v4uint %36
         %39 = OpISub %v4uint %37 %38
         %40 = OpBitcast %v4int %39
               OpReturnValue %40
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %43
         %44 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Modulo_vec4i_i32) {
    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("lhs", ty.vec4<i32>()));
    args.Push(b.FunctionParam("rhs", ty.i32()));
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Binary(core::BinaryOp::kModulo, ty.vec4<i32>(), args[0], args[1]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%16 = OpConstantNull %v4int");
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %v4int None %6
        %lhs = OpFunctionParameter %v4int
        %rhs = OpFunctionParameter %int
          %7 = OpLabel
          %8 = OpCompositeConstruct %v4int %rhs %rhs %rhs %rhs
     %result = OpFunctionCall %v4int %tint_mod_v4i32 %lhs %8
               OpReturnValue %result
               OpFunctionEnd

               ; Function tint_mod_v4i32
%tint_mod_v4i32 = OpFunction %v4int None %13
      %lhs_0 = OpFunctionParameter %v4int
      %rhs_0 = OpFunctionParameter %v4int
         %14 = OpLabel
         %15 = OpIEqual %v4bool %rhs_0 %16
         %19 = OpIEqual %v4bool %lhs_0 %20
         %22 = OpIEqual %v4bool %rhs_0 %23
         %25 = OpLogicalAnd %v4bool %19 %22
         %26 = OpLogicalOr %v4bool %15 %25
         %27 = OpSelect %v4int %26 %28 %rhs_0
         %30 = OpSDiv %v4int %lhs_0 %27
         %33 = OpBitcast %v4uint %30
         %34 = OpBitcast %v4uint %27
         %35 = OpIMul %v4uint %33 %34
         %36 = OpBitcast %v4int %35
         %37 = OpBitcast %v4uint %lhs_0
         %38 = OpBitcast %v4uint %36
         %39 = OpISub %v4uint %37 %38
         %40 = OpBitcast %v4int %39
               OpReturnValue %40
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %43
         %44 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::spirv::writer
