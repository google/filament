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

#include "src/tint/lang/core/ir/binary.h"

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/spirv/ir/binary.h"
#include "src/tint/lang/spirv/writer/common/helper_test.h"

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

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* lhs = MakeScalarValue(params.lhs_type);
        auto* rhs = MakeScalarValue(params.rhs_type);
        auto* result = b.Binary(params.op, MakeScalarType(params.res_type), lhs, rhs);
        b.Return(func);
        mod.SetName(result, "result");
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %" + params.spirv_type_name);
}
TEST_P(Arithmetic_Bitwise, Vector) {
    auto params = GetParam();

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* lhs = MakeVectorValue(params.lhs_type);
        auto* rhs = MakeVectorValue(params.rhs_type);
        auto* result = b.Binary(params.op, MakeVectorType(params.res_type), lhs, rhs);
        b.Return(func);
        mod.SetName(result, "result");
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
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
    auto* vector = b.FunctionParam("vector", ty.vec4f());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({scalar, vector});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(scalar, vector);
        b.Return(func);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Call(func, b.Zero(ty.f32()), b.Zero(ty.vec4f()));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%result = OpVectorTimesScalar %v4float %vector %scalar");
}

TEST_F(SpirvWriterTest, Binary_VectorTimesScalar_F32) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4f());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({scalar, vector});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(vector, scalar);
        b.Return(func);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Call(func, b.Zero(ty.f32()), b.Zero(ty.vec4f()));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%result = OpVectorTimesScalar %v4float %vector %scalar");
}

TEST_F(SpirvWriterTest, Binary_ScalarTimesMatrix_F32) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* matrix = b.FunctionParam("matrix", ty.mat3x4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({scalar, matrix});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(scalar, matrix);
        b.Return(func);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Call(func, b.Zero(ty.f32()), b.Zero(ty.mat3x4<f32>()));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%result = OpMatrixTimesScalar %mat3v4float %matrix %scalar");
}

TEST_F(SpirvWriterTest, Binary_MatrixTimesScalar_F32) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* matrix = b.FunctionParam("matrix", ty.mat3x4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({scalar, matrix});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(matrix, scalar);
        b.Return(func);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Call(func, b.Zero(ty.f32()), b.Zero(ty.mat3x4<f32>()));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%result = OpMatrixTimesScalar %mat3v4float %matrix %scalar");
}

TEST_F(SpirvWriterTest, Binary_VectorTimesMatrix_F32) {
    auto* vector = b.FunctionParam("vector", ty.vec4f());
    auto* matrix = b.FunctionParam("matrix", ty.mat3x4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({vector, matrix});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(vector, matrix);
        b.Return(func);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Call(func, b.Zero(ty.vec4f()), b.Zero(ty.mat3x4<f32>()));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%result = OpVectorTimesMatrix %v3float %vector %matrix");
}

TEST_F(SpirvWriterTest, Binary_MatrixTimesVector_F32) {
    auto* vector = b.FunctionParam("vector", ty.vec3f());
    auto* matrix = b.FunctionParam("matrix", ty.mat3x4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({vector, matrix});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(matrix, vector);
        b.Return(func);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Call(func, b.Zero(ty.vec3f()), b.Zero(ty.mat3x4<f32>()));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%result = OpMatrixTimesVector %v4float %matrix %vector");
}

TEST_F(SpirvWriterTest, Binary_MatrixTimesMatrix_F32) {
    auto* mat1 = b.FunctionParam("mat1", ty.mat4x3<f32>());
    auto* mat2 = b.FunctionParam("mat2", ty.mat3x4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({mat1, mat2});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(mat1, mat2);
        b.Return(func);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Call(func, b.Zero(ty.mat4x3<f32>()), b.Zero(ty.mat3x4<f32>()));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%result = OpMatrixTimesMatrix %mat3v3float %mat1 %mat2");
}

using Comparison = SpirvWriterTestWithParam<BinaryTestCase>;
TEST_P(Comparison, Scalar) {
    auto params = GetParam();

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* lhs = MakeScalarValue(params.lhs_type);
        auto* rhs = MakeScalarValue(params.rhs_type);
        auto* result = b.Binary(params.op, ty.bool_(), lhs, rhs);
        b.Return(func);
        mod.SetName(result, "result");
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %bool");
}

TEST_P(Comparison, Vector) {
    auto params = GetParam();

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* lhs = MakeVectorValue(params.lhs_type);
        auto* rhs = MakeVectorValue(params.rhs_type);
        auto* result = b.Binary(params.op, ty.vec2<bool>(), lhs, rhs);
        b.Return(func);
        mod.SetName(result, "result");
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
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
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* sub = b.Subtract(1_i, 2_i);
        auto* add = b.Add(sub, sub);
        b.Return(func);
        mod.SetName(sub, "sub");
        mod.SetName(add, "add");
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
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

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.u32()), b.Zero(ty.u32())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %uint None %5
        %lhs = OpFunctionParameter %uint
        %rhs = OpFunctionParameter %uint
          %6 = OpLabel
     %result = OpFunctionCall %uint %tint_div_u32 %lhs %rhs
               OpReturnValue %result
               OpFunctionEnd

               ; Function main
       %main = OpFunction %void None %11
         %12 = OpLabel
          %x = OpFunctionCall %uint %foo %uint_0 %uint_0
               OpReturn
               OpFunctionEnd

               ; Function tint_div_u32
%tint_div_u32 = OpFunction %uint None %5
      %lhs_0 = OpFunctionParameter %uint
      %rhs_0 = OpFunctionParameter %uint
         %17 = OpLabel
         %18 = OpIEqual %bool %rhs_0 %uint_0
         %20 = OpSelect %uint %18 %uint_1 %rhs_0
         %22 = OpUDiv %uint %lhs_0 %20
               OpReturnValue %22
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

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.i32()), b.Zero(ty.i32())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %int None %5
        %lhs = OpFunctionParameter %int
        %rhs = OpFunctionParameter %int
          %6 = OpLabel
     %result = OpFunctionCall %int %tint_div_i32 %lhs %rhs
               OpReturnValue %result
               OpFunctionEnd

               ; Function main
       %main = OpFunction %void None %11
         %12 = OpLabel
          %x = OpFunctionCall %int %foo %int_0 %int_0
               OpReturn
               OpFunctionEnd

               ; Function tint_div_i32
%tint_div_i32 = OpFunction %int None %5
      %lhs_0 = OpFunctionParameter %int
      %rhs_0 = OpFunctionParameter %int
         %17 = OpLabel
         %18 = OpIEqual %bool %rhs_0 %int_0
         %20 = OpIEqual %bool %lhs_0 %int_n2147483648
         %22 = OpIEqual %bool %rhs_0 %int_n1
         %24 = OpLogicalAnd %bool %20 %22
         %25 = OpLogicalOr %bool %18 %24
         %26 = OpSelect %int %25 %int_1 %rhs_0
         %28 = OpSDiv %int %lhs_0 %26
               OpReturnValue %28
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Divide_i32_vec4i) {
    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("lhs", ty.i32()));
    args.Push(b.FunctionParam("rhs", ty.vec4i()));
    auto* func = b.Function("foo", ty.vec4i());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Binary(core::BinaryOp::kDivide, ty.vec4i(), args[0], args[1]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.i32()), b.Zero(ty.vec4i())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%17 = OpConstantNull %v4int");
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

               ; Function main
       %main = OpFunction %void None %13
         %14 = OpLabel
          %x = OpFunctionCall %v4int %foo %int_0 %17
               OpReturn
               OpFunctionEnd

               ; Function tint_div_v4i32
%tint_div_v4i32 = OpFunction %v4int None %20
      %lhs_0 = OpFunctionParameter %v4int
      %rhs_0 = OpFunctionParameter %v4int
         %21 = OpLabel
         %22 = OpIEqual %v4bool %rhs_0 %17
         %25 = OpIEqual %v4bool %lhs_0 %26
         %28 = OpIEqual %v4bool %rhs_0 %29
         %31 = OpLogicalAnd %v4bool %25 %28
         %32 = OpLogicalOr %v4bool %22 %31
         %33 = OpSelect %v4int %32 %34 %rhs_0
         %36 = OpSDiv %v4int %lhs_0 %33
               OpReturnValue %36
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Divide_vec4i_i32) {
    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("lhs", ty.vec4i()));
    args.Push(b.FunctionParam("rhs", ty.i32()));
    auto* func = b.Function("foo", ty.vec4i());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Binary(core::BinaryOp::kDivide, ty.vec4i(), args[0], args[1]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.vec4i()), b.Zero(ty.i32())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
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

               ; Function main
       %main = OpFunction %void None %13
         %14 = OpLabel
          %x = OpFunctionCall %v4int %foo %16 %int_0
               OpReturn
               OpFunctionEnd

               ; Function tint_div_v4i32
%tint_div_v4i32 = OpFunction %v4int None %20
      %lhs_0 = OpFunctionParameter %v4int
      %rhs_0 = OpFunctionParameter %v4int
         %21 = OpLabel
         %22 = OpIEqual %v4bool %rhs_0 %16
         %25 = OpIEqual %v4bool %lhs_0 %26
         %28 = OpIEqual %v4bool %rhs_0 %29
         %31 = OpLogicalAnd %v4bool %25 %28
         %32 = OpLogicalOr %v4bool %22 %31
         %33 = OpSelect %v4int %32 %34 %rhs_0
         %36 = OpSDiv %v4int %lhs_0 %33
               OpReturnValue %36
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

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.u32()), b.Zero(ty.u32())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %uint None %5
        %lhs = OpFunctionParameter %uint
        %rhs = OpFunctionParameter %uint
          %6 = OpLabel
     %result = OpFunctionCall %uint %tint_mod_u32 %lhs %rhs
               OpReturnValue %result
               OpFunctionEnd

               ; Function main
       %main = OpFunction %void None %11
         %12 = OpLabel
          %x = OpFunctionCall %uint %foo %uint_0 %uint_0
               OpReturn
               OpFunctionEnd

               ; Function tint_mod_u32
%tint_mod_u32 = OpFunction %uint None %5
      %lhs_0 = OpFunctionParameter %uint
      %rhs_0 = OpFunctionParameter %uint
         %17 = OpLabel
         %18 = OpIEqual %bool %rhs_0 %uint_0
         %20 = OpSelect %uint %18 %uint_1 %rhs_0
         %22 = OpUDiv %uint %lhs_0 %20
         %23 = OpIMul %uint %22 %20
         %24 = OpISub %uint %lhs_0 %23
               OpReturnValue %24
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

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.i32()), b.Zero(ty.i32())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %int None %5
        %lhs = OpFunctionParameter %int
        %rhs = OpFunctionParameter %int
          %6 = OpLabel
     %result = OpFunctionCall %int %tint_mod_i32 %lhs %rhs
               OpReturnValue %result
               OpFunctionEnd

               ; Function main
       %main = OpFunction %void None %11
         %12 = OpLabel
          %x = OpFunctionCall %int %foo %int_0 %int_0
               OpReturn
               OpFunctionEnd

               ; Function tint_mod_i32
%tint_mod_i32 = OpFunction %int None %5
      %lhs_0 = OpFunctionParameter %int
      %rhs_0 = OpFunctionParameter %int
         %17 = OpLabel
         %18 = OpIEqual %bool %rhs_0 %int_0
         %20 = OpIEqual %bool %lhs_0 %int_n2147483648
         %22 = OpIEqual %bool %rhs_0 %int_n1
         %24 = OpLogicalAnd %bool %20 %22
         %25 = OpLogicalOr %bool %18 %24
         %26 = OpSelect %int %25 %int_1 %rhs_0
         %28 = OpSDiv %int %lhs_0 %26
         %30 = OpBitcast %uint %28
         %31 = OpBitcast %uint %26
         %32 = OpIMul %uint %30 %31
         %33 = OpBitcast %int %32
         %34 = OpBitcast %uint %lhs_0
         %35 = OpBitcast %uint %33
         %36 = OpISub %uint %34 %35
         %37 = OpBitcast %int %36
               OpReturnValue %37
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

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.i32()), b.Zero(ty.i32())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %lhs "lhs"                    ; id %3
               OpName %rhs "rhs"                    ; id %4
               OpName %main "main"                  ; id %12
               OpName %x "x"                        ; id %16

               ; Types, variables and constants
        %int = OpTypeInt 32 1
          %5 = OpTypeFunction %int %int %int
       %uint = OpTypeInt 32 0
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
      %int_0 = OpConstant %int 0

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

               ; Function main
       %main = OpFunction %void None %14
         %15 = OpLabel
          %x = OpFunctionCall %int %foo %int_0 %int_0
               OpReturn
               OpFunctionEnd
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

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.i32()), b.Zero(ty.i32())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %lhs "lhs"                    ; id %3
               OpName %rhs "rhs"                    ; id %4
               OpName %main "main"                  ; id %12
               OpName %x "x"                        ; id %16

               ; Types, variables and constants
        %int = OpTypeInt 32 1
          %5 = OpTypeFunction %int %int %int
       %uint = OpTypeInt 32 0
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
      %int_0 = OpConstant %int 0

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

               ; Function main
       %main = OpFunction %void None %14
         %15 = OpLabel
          %x = OpFunctionCall %int %foo %int_0 %int_0
               OpReturn
               OpFunctionEnd
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

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.i32()), b.Zero(ty.i32())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %lhs "lhs"                    ; id %3
               OpName %rhs "rhs"                    ; id %4
               OpName %main "main"                  ; id %12
               OpName %x "x"                        ; id %16

               ; Types, variables and constants
        %int = OpTypeInt 32 1
          %5 = OpTypeFunction %int %int %int
       %uint = OpTypeInt 32 0
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
      %int_0 = OpConstant %int 0

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

               ; Function main
       %main = OpFunction %void None %14
         %15 = OpLabel
          %x = OpFunctionCall %int %foo %int_0 %int_0
               OpReturn
               OpFunctionEnd
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

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.i32()), b.Zero(ty.u32())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %lhs "lhs"                    ; id %3
               OpName %rhs "rhs"                    ; id %5
               OpName %main "main"                  ; id %13
               OpName %x "x"                        ; id %17

               ; Types, variables and constants
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
          %6 = OpTypeFunction %int %int %uint
    %uint_31 = OpConstant %uint 31
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
     %uint_0 = OpConstant %uint 0

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

               ; Function main
       %main = OpFunction %void None %15
         %16 = OpLabel
          %x = OpFunctionCall %int %foo %int_0 %uint_0
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Modulo_i32_vec4i) {
    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("lhs", ty.i32()));
    args.Push(b.FunctionParam("rhs", ty.vec4i()));
    auto* func = b.Function("foo", ty.vec4i());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Binary(core::BinaryOp::kModulo, ty.vec4i(), args[0], args[1]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.i32()), b.Zero(ty.vec4i())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%17 = OpConstantNull %v4int");
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

               ; Function main
       %main = OpFunction %void None %13
         %14 = OpLabel
          %x = OpFunctionCall %v4int %foo %int_0 %17
               OpReturn
               OpFunctionEnd

               ; Function tint_mod_v4i32
%tint_mod_v4i32 = OpFunction %v4int None %20
      %lhs_0 = OpFunctionParameter %v4int
      %rhs_0 = OpFunctionParameter %v4int
         %21 = OpLabel
         %22 = OpIEqual %v4bool %rhs_0 %17
         %25 = OpIEqual %v4bool %lhs_0 %26
         %28 = OpIEqual %v4bool %rhs_0 %29
         %31 = OpLogicalAnd %v4bool %25 %28
         %32 = OpLogicalOr %v4bool %22 %31
         %33 = OpSelect %v4int %32 %34 %rhs_0
         %36 = OpSDiv %v4int %lhs_0 %33
         %39 = OpBitcast %v4uint %36
         %40 = OpBitcast %v4uint %33
         %41 = OpIMul %v4uint %39 %40
         %42 = OpBitcast %v4int %41
         %43 = OpBitcast %v4uint %lhs_0
         %44 = OpBitcast %v4uint %42
         %45 = OpISub %v4uint %43 %44
         %46 = OpBitcast %v4int %45
               OpReturnValue %46
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Modulo_vec4i_i32) {
    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("lhs", ty.vec4i()));
    args.Push(b.FunctionParam("rhs", ty.i32()));
    auto* func = b.Function("foo", ty.vec4i());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Binary(core::BinaryOp::kModulo, ty.vec4i(), args[0], args[1]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.vec4i()), b.Zero(ty.i32())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
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

               ; Function main
       %main = OpFunction %void None %13
         %14 = OpLabel
          %x = OpFunctionCall %v4int %foo %16 %int_0
               OpReturn
               OpFunctionEnd

               ; Function tint_mod_v4i32
%tint_mod_v4i32 = OpFunction %v4int None %20
      %lhs_0 = OpFunctionParameter %v4int
      %rhs_0 = OpFunctionParameter %v4int
         %21 = OpLabel
         %22 = OpIEqual %v4bool %rhs_0 %16
         %25 = OpIEqual %v4bool %lhs_0 %26
         %28 = OpIEqual %v4bool %rhs_0 %29
         %31 = OpLogicalAnd %v4bool %25 %28
         %32 = OpLogicalOr %v4bool %22 %31
         %33 = OpSelect %v4int %32 %34 %rhs_0
         %36 = OpSDiv %v4int %lhs_0 %33
         %39 = OpBitcast %v4uint %36
         %40 = OpBitcast %v4uint %33
         %41 = OpIMul %v4uint %39 %40
         %42 = OpBitcast %v4int %41
         %43 = OpBitcast %v4uint %lhs_0
         %44 = OpBitcast %v4uint %42
         %45 = OpISub %v4uint %43 %44
         %46 = OpBitcast %v4int %45
               OpReturnValue %46
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Add_SubgroupMatrix) {
    auto* mat = ty.subgroup_matrix_result(ty.f32(), 8, 8);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr(function, mat, read_write));

        auto* scalar_mat = b.Construct(mat, 2_f);
        b.Binary<spirv::ir::Binary>(core::BinaryOp::kAdd, mat, b.Load(v), scalar_mat);
        b.Return(func);
    });

    Options options{
        .entry_point_name = "main",
        .extensions =
            {
                .use_vulkan_memory_model = true,
            },
    };

    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpCapability CooperativeMatrixKHR");
    EXPECT_INST("OpExtension \"SPV_KHR_cooperative_matrix\"");
    EXPECT_INST(R"(
          %7 = OpTypeCooperativeMatrixKHR %float %uint_3 %uint_8 %uint_8 %uint_2
%_ptr_Function_7 = OpTypePointer Function %7
         %13 = OpConstantNull %7
)");
    EXPECT_INST(R"(
          %v = OpVariable %_ptr_Function_7 Function %13
         %14 = OpCompositeConstruct %7 %float_2
         %16 = OpLoad %7 %v None
         %17 = OpFAdd %7 %16 %14
)");
}

TEST_F(SpirvWriterTest, Subtract_SubgroupMatrix) {
    auto* mat = ty.subgroup_matrix_result(ty.f32(), 8, 8);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr(function, mat, read_write));

        auto* scalar_mat = b.Construct(mat, 2_f);
        b.Binary<spirv::ir::Binary>(core::BinaryOp::kSubtract, mat, b.Load(v), scalar_mat);
        b.Return(func);
    });

    Options options{
        .entry_point_name = "main",
        .extensions =
            {
                .use_vulkan_memory_model = true,
            },
    };

    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpCapability CooperativeMatrixKHR");
    EXPECT_INST("OpExtension \"SPV_KHR_cooperative_matrix\"");
    EXPECT_INST(R"(
          %7 = OpTypeCooperativeMatrixKHR %float %uint_3 %uint_8 %uint_8 %uint_2
%_ptr_Function_7 = OpTypePointer Function %7
         %13 = OpConstantNull %7
)");
    EXPECT_INST(R"(
          %v = OpVariable %_ptr_Function_7 Function %13
         %14 = OpCompositeConstruct %7 %float_2
         %16 = OpLoad %7 %v None
         %17 = OpFSub %7 %16 %14
)");
}

TEST_F(SpirvWriterTest, Multiply_SubgroupMatrix) {
    auto* mat = ty.subgroup_matrix_result(ty.f32(), 8, 8);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr(function, mat, read_write));

        auto* scalar_mat = b.Construct(mat, 2_f);
        b.Binary<spirv::ir::Binary>(core::BinaryOp::kMultiply, mat, b.Load(v), scalar_mat);
        b.Return(func);
    });

    Options options{
        .entry_point_name = "main",
        .extensions =
            {
                .use_vulkan_memory_model = true,
            },
    };

    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpCapability CooperativeMatrixKHR");
    EXPECT_INST("OpExtension \"SPV_KHR_cooperative_matrix\"");
    EXPECT_INST(R"(
          %7 = OpTypeCooperativeMatrixKHR %float %uint_3 %uint_8 %uint_8 %uint_2
%_ptr_Function_7 = OpTypePointer Function %7
         %13 = OpConstantNull %7
)");
    EXPECT_INST(R"(
          %v = OpVariable %_ptr_Function_7 Function %13
         %14 = OpCompositeConstruct %7 %float_2
         %16 = OpLoad %7 %v None
         %17 = OpFMul %7 %16 %14
)");
}

}  // namespace
}  // namespace tint::spirv::writer
