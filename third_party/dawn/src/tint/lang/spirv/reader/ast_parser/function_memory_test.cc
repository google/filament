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

using SpvParserMemoryTest = SpirvASTParserTest;

std::string Preamble() {
    return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Fragment %100 "main"
    OpExecutionMode %100 OriginUpperLeft
)";
}

TEST_F(SpvParserMemoryTest, EmitStatement_StoreBoolConst) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeBool
     %true = OpConstantTrue %ty
     %false = OpConstantFalse %ty
     %null = OpConstantNull %ty
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function
     OpStore %1 %true
     OpStore %1 %false
     OpStore %1 %null
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(x_1 = true;
x_1 = false;
x_1 = false;
)"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_StoreUintConst) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 0
     %val = OpConstant %ty 42
     %null = OpConstantNull %ty
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function
     OpStore %1 %val
     OpStore %1 %null
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody());
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(x_1 = 42u;
x_1 = 0u;
)"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_StoreIntConst) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 1
     %val = OpConstant %ty 42
     %null = OpConstantNull %ty
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function
     OpStore %1 %val
     OpStore %1 %null
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody());
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(x_1 = 42i;
x_1 = 0i;
return;
)"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_StoreFloatConst) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeFloat 32
     %val = OpConstant %ty 42
     %null = OpConstantNull %ty
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function
     OpStore %1 %val
     OpStore %1 %null
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody());
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(x_1 = 42.0f;
x_1 = 0.0f;
return;
)"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_LoadBool) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeBool
     %true = OpConstantTrue %ty
     %false = OpConstantFalse %ty
     %null = OpConstantNull %ty
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function %true
     %2 = OpLoad %ty %1
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_2 = x_1;"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_LoadScalar) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 0
     %ty_42 = OpConstant %ty 42
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function %ty_42
     %2 = OpLoad %ty %1
     %3 = OpLoad %ty %1
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"( x_1 = 42u;
let x_2 = x_1;
let x_3 = x_1;
)"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_UseLoadedScalarTwice) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 0
     %ty_42 = OpConstant %ty 42
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function %ty_42
     %2 = OpLoad %ty %1
     OpStore %1 %2
     OpStore %1 %2
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"( x_1 = 42u;
let x_2 = x_1;
x_1 = x_2;
x_1 = x_2;
)"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_StoreToModuleScopeVar) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 0
     %val = OpConstant %ty 42
     %ptr_ty = OpTypePointer Private %ty
     %1 = OpVariable %ptr_ty Private
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     OpStore %1 %val
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody());
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("x_1 = 42u;"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_CopyMemory_Scalar_Function_To_Private) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 0
     %val = OpConstant %ty 42
     %ptr_fn_ty = OpTypePointer Function %ty
     %ptr_priv_ty = OpTypePointer Private %ty
     %2 = OpVariable %ptr_priv_ty Private
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_fn_ty Function
     OpCopyMemory %2 %1
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody());
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    const auto* expected = "x_2 = x_1;";
    EXPECT_THAT(got, HasSubstr(expected));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_BaseIsNotPointer) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %10 = OpTypeInt 32 0
     %val = OpConstant %10 42
     %ptr_ty = OpTypePointer Private %10
     %20 = OpVariable %10 Private ; bad pointer type
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpAccessChain %ptr_ty %20
     OpStore %1 %val
     OpReturn
  )"));
    EXPECT_FALSE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_THAT(p->error(), Eq("variable with ID 20 has non-pointer type 10"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_VectorSwizzle) {
    const std::string assembly = Preamble() + R"(
     OpName %1 "myvar"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %store_ty = OpTypeVector %uint 4
     %uint_2 = OpConstant %uint 2
     %uint_42 = OpConstant %uint 42
     %elem_ty = OpTypePointer Private %uint
     %var_ty = OpTypePointer Private %store_ty
     %1 = OpVariable %var_ty Private
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_2
     OpStore %2 %uint_42
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody());
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("myvar.z = 42u;"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_VectorConstOutOfBounds) {
    const std::string assembly = Preamble() + R"(
     OpName %1 "myvar"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %store_ty = OpTypeVector %uint 4
     %42 = OpConstant %uint 42
     %uint_99 = OpConstant %uint 99
     %elem_ty = OpTypePointer Private %uint
     %var_ty = OpTypePointer Private %store_ty
     %1 = OpVariable %var_ty Private
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %42
     OpStore %2 %uint_99
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), Eq("Access chain %2 index %42 value 42 is out of "
                               "bounds for vector of 4 elements"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_VectorNonConstIndex) {
    const std::string assembly = Preamble() + R"(
     OpName %1 "myvar"
     OpName %13 "a_dynamic_index"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %store_ty = OpTypeVector %uint 4
     %uint_2 = OpConstant %uint 2
     %uint_42 = OpConstant %uint 42
     %elem_ty = OpTypePointer Private %uint
     %var_ty = OpTypePointer Private %store_ty
     %1 = OpVariable %var_ty Private
     %10 = OpVariable %var_ty Private
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %11 = OpLoad %store_ty %10
     %12 = OpCompositeExtract %uint %11 2
     %13 = OpCopyObject %uint %12
     %2 = OpAccessChain %elem_ty %1 %13
     OpStore %2 %uint_42
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody());
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("myvar[a_dynamic_index] = 42u;"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_VectorComponent_MultiUse) {
    // WGSL does not support pointer-to-vector-component, so test that we sink
    // these pointers into the point of use.
    const std::string assembly = Preamble() + R"(
     OpName %1 "myvar"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %store_ty = OpTypeVector %uint 4
     %uint_2 = OpConstant %uint 2
     %uint_42 = OpConstant %uint 42
     %elem_ty = OpTypePointer Private %uint
     %var_ty = OpTypePointer Private %store_ty
     %1 = OpVariable %var_ty Private
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %ptr = OpAccessChain %elem_ty %1 %uint_2
     %load = OpLoad %uint %ptr
     %result = OpIAdd %uint %load %uint_2
     OpStore %ptr %result
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto wgsl = test::ToString(p->program(), ast_body);
    EXPECT_THAT(wgsl, Not(HasSubstr("&")));
    EXPECT_THAT(wgsl, HasSubstr(" = myvar.z;"));
    EXPECT_THAT(wgsl, HasSubstr("myvar.z = "));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_VectorComponent_MultiUse_NonConstIndex) {
    // WGSL does not support pointer-to-vector-component, so test that we sink
    // these pointers into the point of use.
    const std::string assembly = Preamble() + R"(
     OpName %1 "myvar"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %store_ty = OpTypeVector %uint 4
     %uint_2 = OpConstant %uint 2
     %uint_42 = OpConstant %uint 42
     %elem_ty = OpTypePointer Private %uint
     %var_ty = OpTypePointer Private %store_ty
     %1 = OpVariable %var_ty Private
     %2 = OpVariable %elem_ty Private
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %idx = OpLoad %uint %2
     %ptr = OpAccessChain %elem_ty %1 %idx
     %load = OpLoad %uint %ptr
     %result = OpIAdd %uint %load %uint_2
     OpStore %ptr %result
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto wgsl = test::ToString(p->program(), ast_body);
    EXPECT_THAT(wgsl, Not(HasSubstr("&")));
    EXPECT_THAT(wgsl, HasSubstr(" = myvar[x_12];"));
    EXPECT_THAT(wgsl, HasSubstr("myvar[x_12] = "));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_VectorComponent_SinkThroughChain) {
    // Test that we can sink a pointer-to-vector-component through a chain of
    // instructions that propagate it.
    const std::string assembly = Preamble() + R"(
     OpName %1 "myvar"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %store_ty = OpTypeVector %uint 4
     %uint_2 = OpConstant %uint 2
     %uint_42 = OpConstant %uint 42
     %elem_ty = OpTypePointer Private %uint
     %var_ty = OpTypePointer Private %store_ty
     %1 = OpVariable %var_ty Private
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %ptr = OpAccessChain %elem_ty %1 %uint_2
     %ptr2 = OpCopyObject %elem_ty %ptr
     %ptr3 = OpInBoundsAccessChain %elem_ty %ptr2
     %ptr4 = OpAccessChain %elem_ty %ptr3
     %load = OpLoad %uint %ptr3
     %result = OpIAdd %uint %load %uint_2
     OpStore %ptr4 %result
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto wgsl = test::ToString(p->program(), ast_body);
    EXPECT_THAT(wgsl, Not(HasSubstr("&")));
    EXPECT_THAT(wgsl, HasSubstr(" = myvar.z;"));
    EXPECT_THAT(wgsl, HasSubstr("myvar.z = "));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_Matrix) {
    const std::string assembly = Preamble() + R"(
     OpName %1 "myvar"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %v4float = OpTypeVector %float 4
     %m3v4float = OpTypeMatrix %v4float 3
     %elem_ty = OpTypePointer Private %v4float
     %var_ty = OpTypePointer Private %m3v4float
     %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %float_42 = OpConstant %float 42
     %v4float_42 = OpConstantComposite %v4float %float_42 %float_42 %float_42 %float_42

     %1 = OpVariable %var_ty Private
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_2
     OpStore %2 %v4float_42
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody());
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("myvar[2u] = vec4f(42.0f);"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_Array) {
    const std::string assembly = Preamble() + R"(
     OpName %1 "myvar"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %v4float = OpTypeVector %float 4
     %m3v4float = OpTypeMatrix %v4float 3
     %elem_ty = OpTypePointer Private %v4float
     %var_ty = OpTypePointer Private %m3v4float
     %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %float_42 = OpConstant %float 42
     %v4float_42 = OpConstantComposite %v4float %float_42 %float_42 %float_42 %float_42

     %1 = OpVariable %var_ty Private
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_2
     OpStore %2 %v4float_42
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody());
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("myvar[2u] = vec4f(42.0f);"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_Struct) {
    const std::string assembly = Preamble() + R"(
     OpName %1 "myvar"
     OpMemberName %strct 1 "age"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %float_42 = OpConstant %float 42
     %strct = OpTypeStruct %float %float
     %elem_ty = OpTypePointer Private %float
     %var_ty = OpTypePointer Private %strct
     %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1

     %1 = OpVariable %var_ty Private
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_1
     OpStore %2 %float_42
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody());
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("myvar.age = 42.0f;"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_Struct_DifferOnlyMemberName) {
    // The spirv-opt internal representation will map both structs to the
    // same canonicalized type, because it doesn't care about member names.
    // But we care about member names when producing a member-access expression.
    // crbug.com/tint/213
    const std::string assembly = Preamble() + R"(
     OpName %1 "myvar"
     OpName %10 "myvar2"
     OpMemberName %strct 1 "age"
     OpMemberName %strct2 1 "ancientness"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %float_42 = OpConstant %float 42
     %float_420 = OpConstant %float 420
     %strct = OpTypeStruct %float %float
     %strct2 = OpTypeStruct %float %float
     %elem_ty = OpTypePointer Private %float
     %var_ty = OpTypePointer Private %strct
     %var2_ty = OpTypePointer Private %strct2
     %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1

     %1 = OpVariable %var_ty Private
     %10 = OpVariable %var2_ty Private
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_1
     OpStore %2 %float_42
     %20 = OpAccessChain %elem_ty %10 %uint_1
     OpStore %20 %float_420
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody());
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(myvar.age = 42.0f;
myvar2.ancientness = 420.0f;
return;
)"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_StructNonConstIndex) {
    const std::string assembly = Preamble() + R"(
     OpName %1 "myvar"
     OpMemberName %55 1 "age"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %float_42 = OpConstant %float 42
     %55 = OpTypeStruct %float %float
     %elem_ty = OpTypePointer Private %float
     %var_ty = OpTypePointer Private %55
     %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_ptr = OpTypePointer Private %uint
     %uintvar = OpVariable %uint_ptr Private

     %1 = OpVariable %var_ty Private
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %10 = OpLoad %uint %uintvar
     %2 = OpAccessChain %elem_ty %1 %10
     OpStore %2 %float_42
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), Eq("Access chain %2 index %10 is a non-constant "
                               "index into a structure %55"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_StructConstOutOfBounds) {
    const std::string assembly = Preamble() + R"(
     OpName %1 "myvar"
     OpMemberName %55 1 "age"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %float_42 = OpConstant %float 42
     %55 = OpTypeStruct %float %float
     %elem_ty = OpTypePointer Private %float
     %var_ty = OpTypePointer Private %55
     %uint = OpTypeInt 32 0
     %uint_99 = OpConstant %uint 99

     %1 = OpVariable %var_ty Private
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_99
     OpStore %2 %float_42
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), Eq("Access chain %2 index value 99 is out of bounds "
                               "for structure %55 having 2 members"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_Struct_RuntimeArray) {
    const std::string assembly = Preamble() + R"(
     OpName %1 "myvar"
     OpMemberName %strct 1 "age"

     OpDecorate %1 DescriptorSet 0
     OpDecorate %1 Binding 0
     OpDecorate %strct BufferBlock
     OpMemberDecorate %strct 0 Offset 0
     OpMemberDecorate %strct 1 Offset 4
     OpDecorate %rtarr ArrayStride 4

     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %float_42 = OpConstant %float 42
     %rtarr = OpTypeRuntimeArray %float
     %strct = OpTypeStruct %float %rtarr
     %elem_ty = OpTypePointer Uniform %float
     %var_ty = OpTypePointer Uniform %strct
     %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2

     %1 = OpVariable %var_ty Uniform
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_1 %uint_2
     OpStore %2 %float_42
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody());
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("myvar.age[2u] = 42.0f;"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_Compound_Matrix_Vector) {
    const std::string assembly = Preamble() + R"(
     OpName %1 "myvar"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %v4float = OpTypeVector %float 4
     %m3v4float = OpTypeMatrix %v4float 3
     %elem_ty = OpTypePointer Private %float
     %var_ty = OpTypePointer Private %m3v4float
     %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
     %float_42 = OpConstant %float 42

     %1 = OpVariable %var_ty Private
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_2 %uint_3
     OpStore %2 %float_42
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody());
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("myvar[2u].w = 42.0f;"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_InvalidPointeeType) {
    const std::string assembly = Preamble() + R"(
     OpName %1 "myvar"
     %55 = OpTypeVoid
     %voidfn = OpTypeFunction %55
     %float = OpTypeFloat 32
     %60 = OpTypePointer Private %55
     %var_ty = OpTypePointer Private %60
     %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2

     %1 = OpVariable %var_ty Private
     %100 = OpFunction %55 None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %60 %1 %uint_2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("Access chain with unknown or invalid pointee type "
                                      "%60: %60 = OpTypePointer Private %55"));
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_DereferenceBase) {
    // The base operand to OpAccessChain may have to be dereferenced first.
    // crbug.com/tint/737
    const std::string assembly = Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void

     %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
     %elem_ty = OpTypePointer Private %uint
     %vec_ty = OpTypePointer Private %v2uint

     %ptrfn = OpTypeFunction %void %vec_ty

     %uint_0 = OpConstant %uint 0

     ; The shortest way to make a pointer example is as a function parameter.
     %200 = OpFunction %void None %ptrfn
     %1 = OpFunctionParameter %vec_ty
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_0
     %3 = OpLoad %uint %2
     OpReturn
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %main_entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    const auto got = test::ToString(p->program());
    const std::string expected = R"(fn x_200(x_1 : ptr<private, vec2u>) {
  let x_3 = (*(x_1)).x;
  return;
}

fn main_1() {
  return;
}

@fragment
fn main() {
  main_1();
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvParserMemoryTest, EmitStatement_AccessChain_InferFunctionAddressSpace) {
    // An access chain can have no indices. When the base is a Function variable,
    // the reference type has no explicit address space in the AST representation.
    // But the pointer type for the let declaration must have an explicit
    // 'function' address space. From crbug.com/tint/807
    const std::string assembly = R"(
OpCapability Shader
OpMemoryModel Logical Simple
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft

%uint = OpTypeInt 32 0
%ptr_ty = OpTypePointer Function %uint

  %void = OpTypeVoid
%voidfn = OpTypeFunction %void
  %main = OpFunction %void None %voidfn
 %entry = OpLabel
     %1 = OpVariable %ptr_ty Function
     %2 = OpAccessChain %ptr_ty %1
          OpReturn
          OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly;
    const auto got = test::ToString(p->program());
    const std::string expected = R"(fn main_1() {
  var x_1 : u32;
  let x_2 = &(x_1);
  return;
}

@fragment
fn main() {
  main_1();
}
)";
    EXPECT_EQ(got, expected) << got;
}

std::string NewStorageBufferPreamble(bool nonwritable = false) {
    // Declare a buffer with
    //  StorageBuffer storage class
    //  Block struct decoration
    return std::string(R"(
     OpCapability Shader
     OpExtension "SPV_KHR_storage_buffer_storage_class"
     OpMemoryModel Logical Simple
     OpEntryPoint Fragment %100 "main"
     OpExecutionMode %100 OriginUpperLeft
     OpName %myvar "myvar"

     OpDecorate %myvar DescriptorSet 0
     OpDecorate %myvar Binding 0

     OpDecorate %struct Block
     OpMemberDecorate %struct 0 Offset 0
     OpMemberDecorate %struct 1 Offset 4
     OpDecorate %arr ArrayStride 4
     )") +
           (nonwritable ? R"(
     OpMemberDecorate %struct 0 NonWritable
     OpMemberDecorate %struct 1 NonWritable)"
                        : "") +
           R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0

     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1

     %arr = OpTypeRuntimeArray %uint
     %struct = OpTypeStruct %uint %arr
     %ptr_struct = OpTypePointer StorageBuffer %struct
     %ptr_uint = OpTypePointer StorageBuffer %uint

     %myvar = OpVariable %ptr_struct StorageBuffer
  )";
}

std::string OldStorageBufferPreamble(bool nonwritable = false) {
    // Declare a buffer with
    //  Unifrom storage class
    //  BufferBlock struct decoration
    // This is the deprecated way to declare a storage buffer.
    return Preamble() + R"(
     OpName %myvar "myvar"

     OpDecorate %myvar DescriptorSet 0
     OpDecorate %myvar Binding 0

     OpDecorate %struct BufferBlock
     OpMemberDecorate %struct 0 Offset 0
     OpMemberDecorate %struct 1 Offset 4
     OpDecorate %arr ArrayStride 4
     )" +
           (nonwritable ? R"(
     OpMemberDecorate %struct 0 NonWritable
     OpMemberDecorate %struct 1 NonWritable)"
                        : "") +
           R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0

     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1

     %arr = OpTypeRuntimeArray %uint
     %struct = OpTypeStruct %uint %arr
     %ptr_struct = OpTypePointer Uniform %struct
     %ptr_uint = OpTypePointer Uniform %uint

     %myvar = OpVariable %ptr_struct Uniform
  )";
}

TEST_F(SpvParserMemoryTest, RemapStorageBuffer_TypesAndVarDeclarations) {
    // Enusure we get the right module-scope declaration.  This tests translation
    // of the structure type, arrays of the structure, pointers to them, and
    // OpVariable of these.
    const auto assembly = OldStorageBufferPreamble() + R"(
  ; The preamble declared %100 to be an entry point, so supply it.
  %100 = OpFunction %void None %voidfn
  %entry = OpLabel
  OpReturn
  OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr(R"(alias RTArr = @stride(4) array<u32>;

struct S {
  /* @offset(0) */
  field0 : u32,
  /* @offset(4) */
  field1 : RTArr,
}

@group(0) @binding(0) var<storage, read_write> myvar : S;
)"));
}

TEST_F(SpvParserMemoryTest, RemapStorageBuffer_ThroughAccessChain_NonCascaded) {
    const auto assembly = OldStorageBufferPreamble() + R"(
  %100 = OpFunction %void None %voidfn
  %entry = OpLabel

  ; the scalar element
  %1 = OpAccessChain %ptr_uint %myvar %uint_0
  OpStore %1 %uint_0

  ; element in the runtime array
  %2 = OpAccessChain %ptr_uint %myvar %uint_1 %uint_1
  OpStore %2 %uint_0

  OpReturn
  OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr(R"(myvar.field0 = 0u;
myvar.field1[1u] = 0u;
)"));
}

TEST_F(SpvParserMemoryTest, RemapStorageBuffer_ThroughAccessChain_NonCascaded_UsedTwice_ReadWrite) {
    // Use the pointer value twice, which provokes the spirv-reader
    // to make a let declaration for the pointer.  The storage class
    // must be 'storage', not 'uniform'.
    const auto assembly = OldStorageBufferPreamble() + R"(
  %100 = OpFunction %void None %voidfn
  %entry = OpLabel

  ; the scalar element
  %1 = OpAccessChain %ptr_uint %myvar %uint_0
  OpStore %1 %uint_0
  OpStore %1 %uint_0

  ; element in the runtime array
  %2 = OpAccessChain %ptr_uint %myvar %uint_1 %uint_1
  ; Use the pointer twice
  %3 = OpLoad %uint %2
  OpStore %2 %uint_0

  OpReturn
  OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr(R"(let x_1 = &(myvar.field0);
*(x_1) = 0u;
*(x_1) = 0u;
let x_2 = &(myvar.field1[1u]);
let x_3 = *(x_2);
*(x_2) = 0u;
)"));
}

TEST_F(SpvParserMemoryTest, RemapStorageBuffer_ThroughAccessChain_NonCascaded_UsedTwice_ReadOnly) {
    // Like the previous test, but make the storage buffer read_only.
    // The pointer type must also be read-only.
    const auto assembly = OldStorageBufferPreamble(true) + R"(
  %100 = OpFunction %void None %voidfn
  %entry = OpLabel

  ; the scalar element
  %1 = OpAccessChain %ptr_uint %myvar %uint_0
  OpStore %1 %uint_0
  OpStore %1 %uint_0

  ; element in the runtime array
  %2 = OpAccessChain %ptr_uint %myvar %uint_1 %uint_1
  ; Use the pointer twice
  %3 = OpLoad %uint %2
  OpStore %2 %uint_0

  OpReturn
  OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr(R"(let x_1 = &(myvar.field0);
*(x_1) = 0u;
*(x_1) = 0u;
let x_2 = &(myvar.field1[1u]);
let x_3 = *(x_2);
*(x_2) = 0u;
)")) << got
     << assembly;
}

TEST_F(SpvParserMemoryTest, StorageBuffer_ThroughAccessChain_NonCascaded_UsedTwice_ReadWrite) {
    // Use new style storage buffer declaration:
    //   StorageBuffer storage class,
    //   Block decoration
    // The pointer type must use 'storage' address space, and should use defaulted access
    const auto assembly = NewStorageBufferPreamble() + R"(
  %100 = OpFunction %void None %voidfn
  %entry = OpLabel

  ; the scalar element
  %1 = OpAccessChain %ptr_uint %myvar %uint_0
  OpStore %1 %uint_0
  OpStore %1 %uint_0

  ; element in the runtime array
  %2 = OpAccessChain %ptr_uint %myvar %uint_1 %uint_1
  ; Use the pointer twice
  %3 = OpLoad %uint %2
  OpStore %2 %uint_0

  OpReturn
  OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr(R"(let x_1 = &(myvar.field0);
*(x_1) = 0u;
*(x_1) = 0u;
let x_2 = &(myvar.field1[1u]);
let x_3 = *(x_2);
*(x_2) = 0u;
)")) << got;
}

TEST_F(SpvParserMemoryTest, StorageBuffer_ThroughAccessChain_NonCascaded_UsedTwice_ReadOnly) {
    // Like the previous test, but make the storage buffer read_only.
    // Use new style storage buffer declaration:
    //   StorageBuffer storage class,
    //   Block decoration
    // The pointer type must use 'storage' address space, and must use read_only
    // access
    const auto assembly = NewStorageBufferPreamble(true) + R"(
  %100 = OpFunction %void None %voidfn
  %entry = OpLabel

  ; the scalar element
  %1 = OpAccessChain %ptr_uint %myvar %uint_0
  OpStore %1 %uint_0
  OpStore %1 %uint_0

  ; element in the runtime array
  %2 = OpAccessChain %ptr_uint %myvar %uint_1 %uint_1
  ; Use the pointer twice
  %3 = OpLoad %uint %2
  OpStore %2 %uint_0

  OpReturn
  OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr(R"(let x_1 = &(myvar.field0);
*(x_1) = 0u;
*(x_1) = 0u;
let x_2 = &(myvar.field1[1u]);
let x_3 = *(x_2);
*(x_2) = 0u;
)")) << got;
}

TEST_F(SpvParserMemoryTest, RemapStorageBuffer_ThroughAccessChain_NonCascaded_InBoundsAccessChain) {
    // Like the previous test, but using OpInBoundsAccessChain.
    const auto assembly = OldStorageBufferPreamble() + R"(
  %100 = OpFunction %void None %voidfn
  %entry = OpLabel

  ; the scalar element
  %1 = OpInBoundsAccessChain %ptr_uint %myvar %uint_0
  OpStore %1 %uint_0

  ; element in the runtime array
  %2 = OpInBoundsAccessChain %ptr_uint %myvar %uint_1 %uint_1
  OpStore %2 %uint_0

  OpReturn
  OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr(R"(myvar.field0 = 0u;
myvar.field1[1u] = 0u;
)")) << got
     << p->error();
}

TEST_F(SpvParserMemoryTest, RemapStorageBuffer_ThroughAccessChain_Cascaded) {
    const auto assembly = OldStorageBufferPreamble() + R"(
  %ptr_rtarr = OpTypePointer Uniform %arr
  %100 = OpFunction %void None %voidfn
  %entry = OpLabel

  ; get the runtime array
  %1 = OpAccessChain %ptr_rtarr %myvar %uint_1
  ; now an element in it
  %2 = OpAccessChain %ptr_uint %1 %uint_1
  OpStore %2 %uint_0

  OpReturn
  OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("myvar.field1[1u] = 0u;"))
        << p->error();
}

TEST_F(SpvParserMemoryTest, RemapStorageBuffer_ThroughCopyObject_WithoutHoisting) {
    // Generates a const declaration directly.
    // We have to do a bunch of address space tracking for locally
    // defined values in order to get the right pointer-to-storage-buffer
    // value type for the const declration.
    const auto assembly = OldStorageBufferPreamble() + R"(
  %100 = OpFunction %void None %voidfn
  %entry = OpLabel

  %1 = OpAccessChain %ptr_uint %myvar %uint_1 %uint_1
  %2 = OpCopyObject %ptr_uint %1
  OpStore %2 %uint_0

  OpReturn
  OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(let x_2 = &(myvar.field1[1u]);
*(x_2) = 0u;
)")) << p->error();

    p->SkipDumpingPending("crbug.com/tint/1041 track access mode in spirv-reader parser type");
}

std::string RuntimeArrayPreamble() {
    return R"(
     OpCapability Shader
     OpMemoryModel Logical Simple
     OpEntryPoint Fragment %100 "main"
     OpExecutionMode %100 OriginUpperLeft

     OpName %myvar "myvar"
     OpMemberName %struct 0 "first"
     OpMemberName %struct 1 "rtarr"

     OpDecorate %struct Block
     OpMemberDecorate %struct 0 Offset 0
     OpMemberDecorate %struct 1 Offset 4
     OpDecorate %arr ArrayStride 4

     OpDecorate %myvar DescriptorSet 0
     OpDecorate %myvar Binding 0

     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0

     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1

     %arr = OpTypeRuntimeArray %uint
     %struct = OpTypeStruct %uint %arr
     %ptr_struct = OpTypePointer StorageBuffer %struct
     %ptr_uint = OpTypePointer StorageBuffer %uint

     %myvar = OpVariable %ptr_struct StorageBuffer
  )";
}

TEST_F(SpvParserMemoryTest, ArrayLength_FromVar) {
    const auto assembly = RuntimeArrayPreamble() + R"(

  %100 = OpFunction %void None %voidfn

  %entry = OpLabel
  %1 = OpArrayLength %uint %myvar 1
  OpReturn
  OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body_str = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body_str, HasSubstr("let x_1 = arrayLength(&(myvar.rtarr));")) << body_str;
}

TEST_F(SpvParserMemoryTest, ArrayLength_FromCopyObject) {
    const auto assembly = RuntimeArrayPreamble() + R"(

  %100 = OpFunction %void None %voidfn

  %entry = OpLabel
  %2 = OpCopyObject %ptr_struct %myvar
  %1 = OpArrayLength %uint %2 1
  OpReturn
  OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body_str = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body_str, HasSubstr(R"(let x_2 = &(myvar);
let x_1 = arrayLength(&((*(x_2)).rtarr));
)")) << body_str;

    p->SkipDumpingPending("crbug.com/tint/1041 track access mode in spirv-reader parser type");
}

TEST_F(SpvParserMemoryTest, ArrayLength_FromAccessChain) {
    const auto assembly = RuntimeArrayPreamble() + R"(

  %100 = OpFunction %void None %voidfn

  %entry = OpLabel
  %2 = OpAccessChain %ptr_struct %myvar ; no indices
  %1 = OpArrayLength %uint %2 1
  OpReturn
  OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body_str = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body_str, HasSubstr("let x_1 = arrayLength(&(myvar.rtarr));")) << body_str;
}

std::string InvalidPointerPreamble() {
    return R"(
OpCapability Shader
OpMemoryModel Logical Simple
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft

%uint = OpTypeInt 32 0
%ptr_ty = OpTypePointer Function %uint

  %void = OpTypeVoid
%voidfn = OpTypeFunction %void
)";
}

TEST_F(SpvParserMemoryTest, InvalidPointer_Undef_ModuleScope_IsError) {
    const std::string assembly = InvalidPointerPreamble() + R"(
 %ptr = OpUndef %ptr_ty

  %main = OpFunction %void None %voidfn
 %entry = OpLabel
     %1 = OpCopyObject %ptr_ty %ptr
     %2 = OpAccessChain %ptr_ty %ptr
     %3 = OpInBoundsAccessChain %ptr_ty %ptr
; now show the invalid pointer propagates
     %10 = OpCopyObject %ptr_ty %1
     %20 = OpAccessChain %ptr_ty %2
     %30 = OpInBoundsAccessChain %ptr_ty %3
          OpReturn
          OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->BuildAndParseInternalModule()) << assembly;
    EXPECT_EQ(p->error(), "undef pointer is not valid: %9 = OpUndef %6");
}

TEST_F(SpvParserMemoryTest, InvalidPointer_Undef_FunctionScope_IsError) {
    const std::string assembly = InvalidPointerPreamble() + R"(

  %main = OpFunction %void None %voidfn
 %entry = OpLabel
   %ptr = OpUndef %ptr_ty
          OpReturn
          OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->BuildAndParseInternalModule()) << assembly;
    EXPECT_EQ(p->error(), "undef pointer is not valid: %7 = OpUndef %3");
}

TEST_F(SpvParserMemoryTest, InvalidPointer_ConstantNull_IsError) {
    // OpConstantNull on logical pointer requires variable-pointers, which
    // is not (yet) supported by WGSL features.
    const std::string assembly = InvalidPointerPreamble() + R"(
 %ptr = OpConstantNull %ptr_ty

  %main = OpFunction %void None %voidfn
 %entry = OpLabel
     %1 = OpCopyObject %ptr_ty %ptr
     %2 = OpAccessChain %ptr_ty %ptr
     %3 = OpInBoundsAccessChain %ptr_ty %ptr
; now show the invalid pointer propagates
     %10 = OpCopyObject %ptr_ty %1
     %20 = OpAccessChain %ptr_ty %2
     %30 = OpInBoundsAccessChain %ptr_ty %3
          OpReturn
          OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->BuildAndParseInternalModule());
    EXPECT_EQ(p->error(), "null pointer is not valid: %9 = OpConstantNull %6");
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
