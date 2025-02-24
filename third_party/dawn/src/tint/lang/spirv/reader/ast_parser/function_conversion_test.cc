// Copyright 2020 The Dawn & Tint Authors
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
#include "src/tint/lang/spirv/reader/ast_parser/function.h"
#include "src/tint/lang/spirv/reader/ast_parser/helper_test.h"
#include "src/tint/lang/spirv/reader/ast_parser/spirv_tools_helpers_test.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ::testing::Eq;
using ::testing::HasSubstr;

std::string Preamble() {
    return R"(
  OpCapability Shader
  OpCapability Float16
  OpMemoryModel Logical Simple
  OpEntryPoint Fragment %100 "main"
  OpExecutionMode %100 OriginUpperLeft

  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

  %bool = OpTypeBool
  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %float = OpTypeFloat 32
  %half = OpTypeFloat 16

  %true = OpConstantTrue %bool
  %false = OpConstantFalse %bool
  %v2bool = OpTypeVector %bool 2
  %v2bool_t_f = OpConstantComposite %v2bool %true %false

  %uint_10 = OpConstant %uint 10
  %uint_20 = OpConstant %uint 20
  %int_30 = OpConstant %int 30
  %int_40 = OpConstant %int 40
  %float_50 = OpConstant %float 50
  %float_60 = OpConstant %float 60
  %half_50 = OpConstant %half 50
  %half_60 = OpConstant %half 60

  %ptr_uint = OpTypePointer Function %uint
  %ptr_int = OpTypePointer Function %int
  %ptr_float = OpTypePointer Function %float
  %ptr_half = OpTypePointer Function %half

  %v2uint = OpTypeVector %uint 2
  %v2int = OpTypeVector %int 2
  %v2float = OpTypeVector %float 2
  %v2half = OpTypeVector %half 2

  %v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
  %v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
  %v2int_30_40 = OpConstantComposite %v2int %int_30 %int_40
  %v2int_40_30 = OpConstantComposite %v2int %int_40 %int_30
  %v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
  %v2float_60_50 = OpConstantComposite %v2float %float_60 %float_50
  %v2half_50_60 = OpConstantComposite %v2half %half_50 %half_60
  %v2half_60_50 = OpConstantComposite %v2half %half_60 %half_50
)";
}

using SpvUnaryConversionTest = SpirvASTParserTestBase<::testing::Test>;

TEST_F(SpvUnaryConversionTest, Bitcast_Scalar) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpBitcast %uint %float_50
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = bitcast<u32>(50.0f);"));
}

TEST_F(SpvUnaryConversionTest, Bitcast_Vector) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpBitcast %v2float %v2uint_10_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = bitcast<vec2f>(vec2u(10u, 20u));"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_BadArg) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertSToF %float %void
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("unhandled expression for ID 2\n%2 = OpTypeVoid"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_BadArg) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertUToF %float %void
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("unhandled expression for ID 2\n%2 = OpTypeVoid"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_BadArg) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToS %float %void
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("unhandled expression for ID 2\n%2 = OpTypeVoid"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_BadArg) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToU %float %void
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("unhandled expression for ID 2\n%2 = OpTypeVoid"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Scalar_BadArgType) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertSToF %float %false
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("operand for conversion to floating point must be "
                                      "integral scalar or vector"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Vector_BadArgType) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertSToF %v2float %v2bool_t_f
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("operand for conversion to floating point must be integral "
                                      "scalar or vector"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Scalar_FromSigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %int %int_30
     %1 = OpConvertSToF %float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = f32(x_30);"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Scalar_FromUnsigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %uint %uint_10
     %1 = OpConvertSToF %float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = f32(bitcast<i32>(x_30));"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Vector_FromSigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2int %v2int_30_40
     %1 = OpConvertSToF %v2float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = vec2f(x_30);"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Vector_FromUnsigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2uint %v2uint_10_20
     %1 = OpConvertSToF %v2float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = vec2f(bitcast<vec2i>(x_30));"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Scalar_BadArgType) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertUToF %float %false
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("operand for conversion to floating point must be "
                                      "integral scalar or vector"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Vector_BadArgType) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertUToF %v2float %v2bool_t_f
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("operand for conversion to floating point must be integral "
                                      "scalar or vector"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Scalar_FromSigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %int %int_30
     %1 = OpConvertUToF %float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = f32(bitcast<u32>(x_30));"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Scalar_FromUnsigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %uint %uint_10
     %1 = OpConvertUToF %float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = f32(x_30);"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Vector_FromSigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2int %v2int_30_40
     %1 = OpConvertUToF %v2float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = vec2f(bitcast<vec2u>(x_30));"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Vector_FromUnsigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2uint %v2uint_10_20
     %1 = OpConvertUToF %v2float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = vec2f(x_30);"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Scalar_BadArgType) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToS %int %uint_10
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("operand for conversion to signed integer must be floating "
                                      "point scalar or vector"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Vector_BadArgType) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToS %v2float %v2bool_t_f
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("operand for conversion to signed integer must be floating "
                                      "point scalar or vector"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Scalar_ToSigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %float %float_50
     %1 = OpConvertFToS %int %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = i32(x_30);"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Scalar_ToUnsigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %float %float_50
     %1 = OpConvertFToS %uint %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = bitcast<u32>(i32(x_30));"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Vector_ToSigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2float %v2float_50_60
     %1 = OpConvertFToS %v2int %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = vec2i(x_30);"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Vector_ToUnsigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2float %v2float_50_60
     %1 = OpConvertFToS %v2uint %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = bitcast<vec2u>(vec2i(x_30));"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Scalar_BadArgType) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToU %int %uint_10
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("operand for conversion to unsigned integer must be floating "
                                      "point scalar or vector"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Vector_BadArgType) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToU %v2float %v2bool_t_f
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("operand for conversion to unsigned integer must be floating "
                                      "point scalar or vector"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Scalar_ToSigned_IsError) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %float %float_50
     %1 = OpConvertFToU %int %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->Parse());
    EXPECT_FALSE(p->success());
    EXPECT_THAT(p->error(), HasSubstr("Expected unsigned int scalar or vector "
                                      "type as Result Type: ConvertFToU"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Scalar_ToUnsigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %float %float_50
     %1 = OpConvertFToU %uint %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = u32(x_30);"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Vector_ToSigned_IsError) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2float %v2float_50_60
     %1 = OpConvertFToU %v2int %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->Parse());
    EXPECT_FALSE(p->success());
    EXPECT_THAT(p->error(), HasSubstr("Expected unsigned int scalar or vector "
                                      "type as Result Type: ConvertFToU"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Vector_ToUnsigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2float %v2float_50_60
     %1 = OpConvertFToU %v2uint %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = vec2u(x_30);"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_HoistedValue) {
    // From crbug.com/tint/804
    const auto assembly = Preamble() + R"(

%100 = OpFunction %void None %voidfn
%10 = OpLabel
OpBranch %30

%30 = OpLabel
OpLoopMerge %90 %80 None
OpBranchConditional %true %90 %40

%40 = OpLabel
OpSelectionMerge %50 None
OpBranchConditional %true %45 %50

%45 = OpLabel
; This value is hoisted
%600 = OpCopyObject %float %float_50
OpBranch %50

%50 = OpLabel
OpBranch %90

%80 = OpLabel ; unreachable continue target
%82 = OpConvertFToU %uint %600
OpBranch %30 ; backedge

%90 = OpLabel
OpReturn
OpFunctionEnd

  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_82 = u32(x_600);"));
}

TEST_F(SpvUnaryConversionTest, FConvert_BadArg) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpFConvert %float %void
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("unhandled expression for ID 2\n%2 = OpTypeVoid"));
}

TEST_F(SpvUnaryConversionTest, FConvertFToH_Scalar) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %float %float_50
     %1 = OpFConvert %half %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = f16(x_30);"));
}

TEST_F(SpvUnaryConversionTest, FConvertHToF_Scalar) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %half %half_50
     %1 = OpFConvert %float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = f32(x_30);"));
}

TEST_F(SpvUnaryConversionTest, FConvertFToH_Vector) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2float %v2float_50_60
     %1 = OpFConvert %v2half %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = vec2h(x_30);"));
}

TEST_F(SpvUnaryConversionTest, FConvertHToF_Vector) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2half %v2half_50_60
     %1 = OpFConvert %v2float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = vec2f(x_30);"));
}

// TODO(dneto): OpSConvert // only if multiple widths
// TODO(dneto): OpUConvert // only if multiple widths
// TODO(dneto): OpSatConvertSToU // Kernel (OpenCL), not in WebGPU
// TODO(dneto): OpSatConvertUToS // Kernel (OpenCL), not in WebGPU

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
