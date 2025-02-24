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

std::string Caps() {
    return R"(
  OpCapability Shader
  OpMemoryModel Logical Simple
  OpEntryPoint GLCompute %100 "main"
  OpExecutionMode %100 LocalSize 1 1 1
)";
}

std::string CommonTypes() {
    return R"(
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %float = OpTypeFloat 32

  %uint_10 = OpConstant %uint 10
  %uint_20 = OpConstant %uint 20
  %uint_1 = OpConstant %uint 1
  %uint_3 = OpConstant %uint 3
  %uint_4 = OpConstant %uint 4
  %uint_5 = OpConstant %uint 5
  %int_1 = OpConstant %int 1
  %int_30 = OpConstant %int 30
  %int_40 = OpConstant %int 40
  %float_50 = OpConstant %float 50
  %float_60 = OpConstant %float 60
  %float_70 = OpConstant %float 70

  %v2uint = OpTypeVector %uint 2
  %v3uint = OpTypeVector %uint 3
  %v4uint = OpTypeVector %uint 4
  %v2int = OpTypeVector %int 2
  %v2float = OpTypeVector %float 2

  %m3v2float = OpTypeMatrix %v2float 3
  %m3v2float_0 = OpConstantNull %m3v2float

  %s_v2f_u_i = OpTypeStruct %v2float %uint %int
  %a_u_5 = OpTypeArray %uint %uint_5

  %v2uint_3_4 = OpConstantComposite %v2uint %uint_3 %uint_4
  %v2uint_4_3 = OpConstantComposite %v2uint %uint_4 %uint_3
  %v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
  %v2float_60_50 = OpConstantComposite %v2float %float_60 %float_50
  %v2float_70_70 = OpConstantComposite %v2float %float_70 %float_70
)";
}

std::string Preamble() {
    return Caps() + CommonTypes();
}

using SpirvASTParserTest_Composite_Construct = SpirvASTParserTest;

TEST_F(SpirvASTParserTest_Composite_Construct, Vector) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCompositeConstruct %v2uint %uint_10 %uint_20
     %2 = OpCompositeConstruct %v2int %int_30 %int_40
     %3 = OpCompositeConstruct %v2float %float_50 %float_60
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(let x_1 = vec2u(10u, 20u);
let x_2 = vec2i(30i, 40i);
let x_3 = vec2f(50.0f, 60.0f);
)"));
}

TEST_F(SpirvASTParserTest_Composite_Construct, VectorSplat) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCompositeConstruct %v4uint %uint_10 %uint_10 %uint_10 %uint_10
     %2 = OpCompositeConstruct %v2int %int_30 %int_30
     %3 = OpCompositeConstruct %v2float %float_50 %float_50
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(let x_1 = vec4u(10u);
let x_2 = vec2i(30i);
let x_3 = vec2f(50.0f);
)"));
}

TEST_F(SpirvASTParserTest_Composite_Construct, Matrix) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCompositeConstruct %m3v2float %v2float_50_60 %v2float_60_50 %v2float_70_70
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = mat3x2f("
                                                                  "vec2f(50.0f, 60.0f), "
                                                                  "vec2f(60.0f, 50.0f), "
                                                                  "vec2f(70.0f));"));
}

TEST_F(SpirvASTParserTest_Composite_Construct, Array) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCompositeConstruct %a_u_5 %uint_10 %uint_20 %uint_3 %uint_4 %uint_5
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = array<u32, 5u>(10u, 20u, 3u, 4u, 5u);"));
}

TEST_F(SpirvASTParserTest_Composite_Construct, Struct) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCompositeConstruct %s_v2f_u_i %v2float_50_60 %uint_5 %int_30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = S(vec2f(50.0f, 60.0f), 5u, 30i);"));
}

TEST_F(SpirvASTParserTest_Composite_Construct, ConstantComposite_Struct_NoDeduplication) {
    const auto assembly = Preamble() + R"(
     %200 = OpTypeStruct %uint
     %300 = OpTypeStruct %uint ; isomorphic structures

     %201 = OpConstantComposite %200 %uint_10
     %301 = OpConstantComposite %300 %uint_10  ; isomorphic constants

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpCopyObject %200 %201
     %3 = OpCopyObject %300 %301
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    const auto expected = std::string(
        R"(let x_2 = S_1(10u);
let x_3 = S_2(10u);
return;
)");
    EXPECT_EQ(got, expected) << got;
}

using SpirvASTParserTest_CompositeExtract = SpirvASTParserTest;

TEST_F(SpirvASTParserTest_CompositeExtract, Vector) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCompositeExtract %float %v2float_50_60 1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = vec2f(50.0f, 60.0f).y;"));
}

TEST_F(SpirvASTParserTest_CompositeExtract, Vector_IndexTooBigError) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCompositeExtract %float %v2float_50_60 900
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_EQ(p->error(),
              "OpCompositeExtract %1 index value 900 is out of bounds for vector "
              "of 2 elements");
}

TEST_F(SpirvASTParserTest_CompositeExtract, Matrix) {
    const auto assembly = Preamble() + R"(
     %ptr = OpTypePointer Function %m3v2float

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var = OpVariable %ptr Function
     %1 = OpLoad %m3v2float %var
     %2 = OpCompositeExtract %v2float %1 2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_2 = x_1[2u];"));
}

TEST_F(SpirvASTParserTest_CompositeExtract, Matrix_IndexTooBigError) {
    const auto assembly = Preamble() + R"(
     %ptr = OpTypePointer Function %m3v2float

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var = OpVariable %ptr Function
     %1 = OpLoad %m3v2float %var
     %2 = OpCompositeExtract %v2float %1 3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody()) << p->error();
    EXPECT_EQ(p->error(),
              "OpCompositeExtract %2 index value 3 is out of bounds for matrix "
              "of 3 elements");
}

TEST_F(SpirvASTParserTest_CompositeExtract, Matrix_Vector) {
    const auto assembly = Preamble() + R"(
     %ptr = OpTypePointer Function %m3v2float

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var = OpVariable %ptr Function
     %1 = OpLoad %m3v2float %var
     %2 = OpCompositeExtract %float %1 2 1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_2 = x_1[2u].y;"));
}

TEST_F(SpirvASTParserTest_CompositeExtract, Array) {
    const auto assembly = Preamble() + R"(
     %ptr = OpTypePointer Function %a_u_5

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var = OpVariable %ptr Function
     %1 = OpLoad %a_u_5 %var
     %2 = OpCompositeExtract %uint %1 3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_2 = x_1[3u];"));
}

TEST_F(SpirvASTParserTest_CompositeExtract, RuntimeArray_IsError) {
    const auto assembly = Preamble() + R"(
     %rtarr = OpTypeRuntimeArray %uint
     %ptr = OpTypePointer Function %rtarr

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var = OpVariable %ptr Function
     %1 = OpLoad %rtarr %var
     %2 = OpCompositeExtract %uint %1 3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody()) << p->error();
    EXPECT_THAT(p->error(), HasSubstr("can't do OpCompositeExtract on a runtime array: "));
}

TEST_F(SpirvASTParserTest_CompositeExtract, Struct) {
    const auto assembly = Preamble() + R"(
     %ptr = OpTypePointer Function %s_v2f_u_i

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var = OpVariable %ptr Function
     %1 = OpLoad %s_v2f_u_i %var
     %2 = OpCompositeExtract %int %1 2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_2 = x_1.field2;"));
}

TEST_F(SpirvASTParserTest_CompositeExtract, Struct_DifferOnlyInMemberName) {
    const std::string assembly = R"(
     OpCapability Shader
     OpMemoryModel Logical Simple
     OpEntryPoint Fragment %100 "main"
     OpExecutionMode %100 OriginUpperLeft

     OpMemberName %s0 0 "algo"
     OpMemberName %s1 0 "rithm"

     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void

     %uint = OpTypeInt 32 0

     %s0 = OpTypeStruct %uint
     %s1 = OpTypeStruct %uint
     %ptr0 = OpTypePointer Function %s0
     %ptr1 = OpTypePointer Function %s1

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var0 = OpVariable %ptr0 Function
     %var1 = OpVariable %ptr1 Function
     %1 = OpLoad %s0 %var0
     %2 = OpCompositeExtract %uint %1 0
     %3 = OpLoad %s1 %var1
     %4 = OpCompositeExtract %uint %3 0
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto got = fe.ast_body();
    auto program = p->program();
    EXPECT_THAT(test::ToString(program, got), HasSubstr("let x_2 = x_1.algo;"))
        << test::ToString(program, got);
    EXPECT_THAT(test::ToString(program, got), HasSubstr("let x_4 = x_3.rithm;"))
        << test::ToString(program, got);
    p->SkipDumpingPending("crbug.com/tint/863");
}

TEST_F(SpirvASTParserTest_CompositeExtract, Struct_IndexTooBigError) {
    const auto assembly = Preamble() + R"(
     %ptr = OpTypePointer Function %s_v2f_u_i

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var = OpVariable %ptr Function
     %1 = OpLoad %s_v2f_u_i %var
     %2 = OpCompositeExtract %int %1 40
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_EQ(p->error(),
              "OpCompositeExtract %2 index value 40 is out of bounds for "
              "structure %27 having 3 members");
}

TEST_F(SpirvASTParserTest_CompositeExtract, Struct_Array_Matrix_Vector) {
    const auto assembly = Preamble() + R"(
     %a_mat = OpTypeArray %m3v2float %uint_3
     %s = OpTypeStruct %uint %a_mat
     %ptr = OpTypePointer Function %s

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var = OpVariable %ptr Function
     %1 = OpLoad %s %var
     %2 = OpCompositeExtract %float %1 1 2 0 1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_2 = x_1.field1[2u][0u].y;"));
}

using SpirvASTParserTest_CompositeInsert = SpirvASTParserTest;

TEST_F(SpirvASTParserTest_CompositeInsert, Vector) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCompositeInsert %v2float %float_70 %v2float_50_60 1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    const auto* expected =
        R"(var x_1_1 = vec2f(50.0f, 60.0f);
x_1_1.y = 70.0f;
let x_1 = x_1_1;
return;
)";
    EXPECT_EQ(got, expected);
}

TEST_F(SpirvASTParserTest_CompositeInsert, Vector_IndexTooBigError) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCompositeInsert %v2float %float_70 %v2float_50_60 900
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_EQ(p->error(),
              "OpCompositeInsert %1 index value 900 is out of bounds for vector "
              "of 2 elements");
}

TEST_F(SpirvASTParserTest_CompositeInsert, Matrix) {
    const auto assembly = Preamble() + R"(
     %ptr = OpTypePointer Function %m3v2float

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var = OpVariable %ptr Function
     %1 = OpLoad %m3v2float %var
     %2 = OpCompositeInsert %m3v2float %v2float_50_60 %1 2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body_str = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body_str, HasSubstr(R"(var x_2_1 = x_1;
x_2_1[2u] = vec2f(50.0f, 60.0f);
let x_2 = x_2_1;
)")) << body_str;
}

TEST_F(SpirvASTParserTest_CompositeInsert, Matrix_IndexTooBigError) {
    const auto assembly = Preamble() + R"(
     %ptr = OpTypePointer Function %m3v2float

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var = OpVariable %ptr Function
     %1 = OpLoad %m3v2float %var
     %2 = OpCompositeInsert %m3v2float %v2float_50_60 %1 3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody()) << p->error();
    EXPECT_EQ(p->error(),
              "OpCompositeInsert %2 index value 3 is out of bounds for matrix of "
              "3 elements");
}

TEST_F(SpirvASTParserTest_CompositeInsert, Matrix_Vector) {
    const auto assembly = Preamble() + R"(
     %ptr = OpTypePointer Function %m3v2float

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var = OpVariable %ptr Function
     %1 = OpLoad %m3v2float %var
     %2 = OpCompositeInsert %m3v2float %v2float_50_60 %1 2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body_str = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body_str, HasSubstr(R"(var x_2_1 = x_1;
x_2_1[2u] = vec2f(50.0f, 60.0f);
let x_2 = x_2_1;
return;
)")) << body_str;
}

TEST_F(SpirvASTParserTest_CompositeInsert, Array) {
    const auto assembly = Preamble() + R"(
     %ptr = OpTypePointer Function %a_u_5

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var = OpVariable %ptr Function
     %1 = OpLoad %a_u_5 %var
     %2 = OpCompositeInsert %a_u_5 %uint_20 %1 3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body_str = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body_str, HasSubstr(R"(var x_2_1 = x_1;
x_2_1[3u] = 20u;
let x_2 = x_2_1;
)")) << body_str;
}

TEST_F(SpirvASTParserTest_CompositeInsert, RuntimeArray_IsError) {
    const auto assembly = Preamble() + R"(
     %rtarr = OpTypeRuntimeArray %uint
     %ptr = OpTypePointer Function %rtarr

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var = OpVariable %ptr Function
     %1 = OpLoad %rtarr %var
     %2 = OpCompositeInsert %rtarr %uint_20 %1 3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody()) << p->error();
    EXPECT_THAT(p->error(), HasSubstr("can't do OpCompositeInsert on a runtime array: "));
}

TEST_F(SpirvASTParserTest_CompositeInsert, Struct) {
    const auto assembly = Preamble() + R"(
     %ptr = OpTypePointer Function %s_v2f_u_i

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var = OpVariable %ptr Function
     %1 = OpLoad %s_v2f_u_i %var
     %2 = OpCompositeInsert %s_v2f_u_i %int_30 %1 2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body_str = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body_str, HasSubstr(R"(var x_36 : S;
let x_1 = x_36;
var x_2_1 = x_1;
x_2_1.field2 = 30i;
let x_2 = x_2_1;
)")) << body_str;
}

TEST_F(SpirvASTParserTest_CompositeInsert, Struct_DifferOnlyInMemberName) {
    const std::string assembly = R"(
     OpCapability Shader
     OpMemoryModel Logical Simple
     OpEntryPoint Fragment %100 "main"
     OpExecutionMode %100 OriginUpperLeft

     OpName %var0 "var0"
     OpName %var1 "var1"
     OpMemberName %s0 0 "algo"
     OpMemberName %s1 0 "rithm"

     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void

     %uint = OpTypeInt 32 0
     %uint_10 = OpConstant %uint 10
     %uint_11 = OpConstant %uint 11

     %s0 = OpTypeStruct %uint
     %s1 = OpTypeStruct %uint
     %ptr0 = OpTypePointer Function %s0
     %ptr1 = OpTypePointer Function %s1

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var0 = OpVariable %ptr0 Function
     %var1 = OpVariable %ptr1 Function
     %1 = OpLoad %s0 %var0
     %2 = OpCompositeInsert %s0 %uint_10 %1 0
     %3 = OpLoad %s1 %var1
     %4 = OpCompositeInsert %s1 %uint_11 %3 0
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    const std::string expected = R"(var var0 : S;
var var1 : S_1;
let x_1 = var0;
var x_2_1 = x_1;
x_2_1.algo = 10u;
let x_2 = x_2_1;
let x_3 = var1;
var x_4_1 = x_3;
x_4_1.rithm = 11u;
let x_4 = x_4_1;
return;
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpirvASTParserTest_CompositeInsert, Struct_IndexTooBigError) {
    const auto assembly = Preamble() + R"(
     %ptr = OpTypePointer Function %s_v2f_u_i

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var = OpVariable %ptr Function
     %1 = OpLoad %s_v2f_u_i %var
     %2 = OpCompositeInsert %s_v2f_u_i %uint_10 %1 40
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_EQ(p->error(),
              "OpCompositeInsert %2 index value 40 is out of bounds for "
              "structure %27 having 3 members");
}

TEST_F(SpirvASTParserTest_CompositeInsert, Struct_Array_Matrix_Vector) {
    const auto assembly = Preamble() + R"(
     %a_mat = OpTypeArray %m3v2float %uint_3
     %s = OpTypeStruct %uint %a_mat
     %ptr = OpTypePointer Function %s

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %var = OpVariable %ptr Function
     %1 = OpLoad %s %var
     %2 = OpCompositeInsert %s %float_70 %1 1 2 0 1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body_str = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body_str, HasSubstr(R"(var x_38 : S_1;
let x_1 = x_38;
var x_2_1 = x_1;
x_2_1.field1[2u][0u].y = 70.0f;
let x_2 = x_2_1;
)")) << body_str;
}

using SpirvASTParserTest_CopyObject = SpirvASTParserTest;

TEST_F(SpirvASTParserTest_CopyObject, Scalar) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %uint %uint_3
     %2 = OpCopyObject %uint %1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(let x_1 = 3u;
let x_2 = x_1;
)"));
}

TEST_F(SpirvASTParserTest_CopyObject, Pointer) {
    const auto assembly = Preamble() + R"(
     %ptr = OpTypePointer Function %uint

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %10 = OpVariable %ptr Function
     %1 = OpCopyObject %ptr %10
     %2 = OpCopyObject %ptr %1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"( x_10 : u32;
let x_1 = &(x_10);
let x_2 = x_1;
)"));
}

using SpirvASTParserTest_VectorShuffle = SpirvASTParserTest;

TEST_F(SpirvASTParserTest_VectorShuffle, FunctionScopeOperands_UseBoth) {
    // Note that variables are generated for the vector operands.
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %v2uint %v2uint_3_4
     %2 = OpIAdd %v2uint %v2uint_4_3 %v2uint_3_4
     %10 = OpVectorShuffle %v4uint %1 %2 3 2 1 0
     OpReturn
     OpFunctionEnd
)";

    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_10 = vec4u(x_2.yx, x_1.yx);"));
}

TEST_F(SpirvASTParserTest_VectorShuffle, FunctionScopeOperands_UseBoth_Swizzle) {
    // Use the same vector for both source operands.
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %v2uint %v2uint_3_4
     %10 = OpVectorShuffle %v4uint %1 %1 1 0 2 3
     OpReturn
     OpFunctionEnd
)";

    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_10 = x_1.yxxy;"));
}

TEST_F(SpirvASTParserTest_VectorShuffle, FunctionScopeOperands_UseOne_Swizzle) {
    // Only use the first vector operand.
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %v2uint %v2uint_3_4
     %2 = OpUndef %v2uint
     %10 = OpVectorShuffle %v4uint %1 %2 1 0 0 1
     OpReturn
     OpFunctionEnd
)";

    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_10 = x_1.yxxy;"));
}

TEST_F(SpirvASTParserTest_VectorShuffle, ConstantOperands_UseBoth) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %10 = OpVectorShuffle %v4uint %v2uint_3_4 %v2uint_4_3 3 2 1 0
     OpReturn
     OpFunctionEnd
)";

    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_10 = vec4u(vec2u(4u, 3u).yx, vec2u(3u, 4u).yx);"));
}

TEST_F(SpirvASTParserTest_VectorShuffle, ConstantOperands_AllOnesMapToNull) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %v2uint %v2uint_4_3
     %10 = OpVectorShuffle %v2uint %1 %1 0xFFFFFFFF 1
     OpReturn
     OpFunctionEnd
)";

    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_10 = x_1.xy;"));
}

TEST_F(SpirvASTParserTest_VectorShuffle, FunctionScopeOperands_MixedInputOperandSizes) {
    // Note that variables are generated for the vector operands.
    const auto assembly = Preamble() + R"(
     %v3uint_3_4_5 = OpConstantComposite %v3uint %uint_3 %uint_4 %uint_5
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %v2uint %v2uint_3_4
     %3 = OpCopyObject %v3uint %v3uint_3_4_5
     %10 = OpVectorShuffle %v2uint %1 %3 1 4
     OpReturn
     OpFunctionEnd
)";

    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_10 = vec2u(x_1.y, x_3.z);"));
}

TEST_F(SpirvASTParserTest_VectorShuffle, IndexTooBig_IsError) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %10 = OpVectorShuffle %v4uint %v2uint_3_4 %v2uint_4_3 9 2 1 0
     OpReturn
     OpFunctionEnd
)";

    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody()) << p->error();
    EXPECT_THAT(p->error(), Eq("invalid vectorshuffle ID %10: index too large: 9"));
}

using SpirvASTParserTest_VectorExtractDynamic = SpirvASTParserTest;

TEST_F(SpirvASTParserTest_VectorExtractDynamic, SignedIndex) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %v2uint %v2uint_3_4
     %2 = OpCopyObject %int %int_1
     %10 = OpVectorExtractDynamic %uint %1 %2
     OpReturn
     OpFunctionEnd
)";

    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr("let x_10 = x_1[x_2];")) << got;
}

TEST_F(SpirvASTParserTest_VectorExtractDynamic, UnsignedIndex) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %v2uint %v2uint_3_4
     %2 = OpCopyObject %uint %uint_1
     %10 = OpVectorExtractDynamic %uint %1 %2
     OpReturn
     OpFunctionEnd
)";

    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr("let x_10 = x_1[x_2];")) << got;
}

using SpirvASTParserTest_VectorInsertDynamic = SpirvASTParserTest;

TEST_F(SpirvASTParserTest_VectorInsertDynamic, Sample) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %v2uint %v2uint_3_4
     %2 = OpCopyObject %uint %uint_3
     %3 = OpCopyObject %int %int_1
     %10 = OpVectorInsertDynamic %v2uint %1 %2 %3
     OpReturn
     OpFunctionEnd
)";

    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr(R"(var x_10_1 = x_1;
x_10_1[x_3] = x_2;
let x_10 = x_10_1;
)")) << got
     << assembly;
}

TEST_F(SpirvASTParserTest, DISABLED_WorkgroupSize_Overridable) {
    // TODO(dneto): Support specializable workgroup size. crbug.com/tint/504
    const auto* assembly = R"(
  OpCapability Shader
  OpMemoryModel Logical Simple
  OpEntryPoint GLCompute %100 "main"
  OpDecorate %1 BuiltIn WorkgroupSize
  OpDecorate %uint_2 SpecId 0
  OpDecorate %uint_4 SpecId 1
  OpDecorate %uint_8 SpecId 2

  %uint = OpTypeInt 32 0
  %uint_2 = OpSpecConstant %uint 2
  %uint_4 = OpSpecConstant %uint 4
  %uint_8 = OpSpecConstant %uint 8
  %v3uint = OpTypeVector %uint 3
  %1 = OpSpecConstantComposite %v3uint %uint_2 %uint_4 %uint_8
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %10 = OpCopyObject %v3uint %1
     %11 = OpCopyObject %uint %uint_2
     %12 = OpCopyObject %uint %uint_4
     %13 = OpCopyObject %uint %uint_8
     OpReturn
     OpFunctionEnd
)";

    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.Emit()) << p->error();
    const auto got = test::ToString(p->program());
    EXPECT_THAT(got, HasSubstr(R"(
  VariableConst{
    Decorations{
      OverrideDecoration{0}
    }
    x_2
    none
    __u32
    {
      ScalarInitializer[not set]{2}
    }
  }
  VariableConst{
    Decorations{
      OverrideDecoration{1}
    }
    x_3
    none
    __u32
    {
      ScalarInitializer[not set]{4}
    }
  }
  VariableConst{
    Decorations{
      OverrideDecoration{2}
    }
    x_4
    none
    __u32
    {
      ScalarInitializer[not set]{8}
    }
  }
)")) << got;
    EXPECT_THAT(got, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_10
        none
        __vec_3__u32
        {
          TypeInitializer[not set]{
            __vec_3__u32
            ScalarInitializer[not set]{2}
            ScalarInitializer[not set]{4}
            ScalarInitializer[not set]{8}
          }
        }
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_11
        none
        __u32
        {
          Identifier[not set]{x_2}
        }
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_12
        none
        __u32
        {
          Identifier[not set]{x_3}
        }
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_13
        none
        __u32
        {
          Identifier[not set]{x_4}
        }
      }
    })"))
        << got << assembly;
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
