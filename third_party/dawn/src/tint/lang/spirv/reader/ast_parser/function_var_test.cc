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
#include "src/tint/utils/text/string_stream.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ::testing::Eq;
using ::testing::HasSubstr;

/// @returns a SPIR-V assembly segment which assigns debug names
/// to particular IDs.
std::string Names(std::vector<std::string> ids) {
    StringStream outs;
    for (auto& id : ids) {
        outs << "    OpName %" << id << " \"" << id << "\"\n";
    }
    return outs.str();
}

std::string CommonTypes() {
    return
        R"(

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %bool = OpTypeBool
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1

    %ptr_bool = OpTypePointer Function %bool
    %ptr_float = OpTypePointer Function %float
    %ptr_uint = OpTypePointer Function %uint
    %ptr_int = OpTypePointer Function %int

    %true = OpConstantTrue %bool
    %false = OpConstantFalse %bool
    %float_0 = OpConstant %float 0.0
    %float_1p5 = OpConstant %float 1.5
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %int_m1 = OpConstant %int -1
    %int_0 = OpConstant %int 0
    %int_1 = OpConstant %int 1
    %int_3 = OpConstant %int 3
    %uint_2 = OpConstant %uint 2
    %uint_3 = OpConstant %uint 3
    %uint_4 = OpConstant %uint 4
    %uint_5 = OpConstant %uint 5

    %v2int = OpTypeVector %int 2
    %v2float = OpTypeVector %float 2
    %m3v2float = OpTypeMatrix %v2float 3

    %v2int_null = OpConstantNull %v2int

    %arr2uint = OpTypeArray %uint %uint_2
    %strct = OpTypeStruct %uint %float %arr2uint
  )";
}

// Returns the SPIR-V assembly for capabilities, the memory model,
// a vertex shader entry point declaration, and name declarations
// for specified IDs.
std::string Caps(std::vector<std::string> ids = {}) {
    return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Fragment %100 "main"
    OpExecutionMode %100 OriginUpperLeft
)" + Names(ids);
}

// Returns the SPIR-V assembly for a vertex shader, optionally
// with OpName decorations for certain SPIR-V IDs
std::string PreambleNames(std::vector<std::string> ids) {
    return Caps(ids) + CommonTypes();
}

std::string Preamble() {
    return PreambleNames({});
}

using SpvParserFunctionVarTest = SpirvASTParserTest;

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_AnonymousVars) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_uint Function
     %2 = OpVariable %ptr_uint Function
     %3 = OpVariable %ptr_uint Function
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitFunctionVariables());

    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(var x_1 : u32;
var x_2 : u32;
var x_3 : u32;
)"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_NamedVars) {
    auto p = parser(test::Assemble(PreambleNames({"a", "b", "c"}) + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %a = OpVariable %ptr_uint Function
     %b = OpVariable %ptr_uint Function
     %c = OpVariable %ptr_uint Function
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitFunctionVariables());

    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(var a : u32;
var b : u32;
var c : u32;
)"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_MixedTypes) {
    auto p = parser(test::Assemble(PreambleNames({"a", "b", "c"}) + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %a = OpVariable %ptr_uint Function
     %b = OpVariable %ptr_int Function
     %c = OpVariable %ptr_float Function
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitFunctionVariables());

    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(var a : u32;
var b : i32;
var c : f32;
)"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_ScalarInitializers) {
    auto p = parser(test::Assemble(PreambleNames({"a", "b", "c", "d", "e"}) + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %a = OpVariable %ptr_bool Function %true
     %b = OpVariable %ptr_bool Function %false
     %c = OpVariable %ptr_int Function %int_m1
     %d = OpVariable %ptr_uint Function %uint_1
     %e = OpVariable %ptr_float Function %float_1p5
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitFunctionVariables());

    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(var a = true;
var b = false;
var c = -1i;
var d = 1u;
var e = 1.5f;
)"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_ScalarNullInitializers) {
    auto p = parser(test::Assemble(PreambleNames({"a", "b", "c", "d"}) + R"(
     %null_bool = OpConstantNull %bool
     %null_int = OpConstantNull %int
     %null_uint = OpConstantNull %uint
     %null_float = OpConstantNull %float

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %a = OpVariable %ptr_bool Function %null_bool
     %b = OpVariable %ptr_int Function %null_int
     %c = OpVariable %ptr_uint Function %null_uint
     %d = OpVariable %ptr_float Function %null_float
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitFunctionVariables());

    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(var a = false;
var b = 0i;
var c = 0u;
var d = 0.0f;
)"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_VectorInitializer) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %ptr = OpTypePointer Function %v2float
     %two = OpConstant %float 2.0
     %const = OpConstantComposite %v2float %float_1p5 %two

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitFunctionVariables());

    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("var x_200 = vec2f(1.5f, 2.0f);"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_MatrixInitializer) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %ptr = OpTypePointer Function %m3v2float
     %two = OpConstant %float 2.0
     %three = OpConstant %float 3.0
     %four = OpConstant %float 4.0
     %v0 = OpConstantComposite %v2float %float_1p5 %two
     %v1 = OpConstantComposite %v2float %two %three
     %v2 = OpConstantComposite %v2float %three %four
     %const = OpConstantComposite %m3v2float %v0 %v1 %v2

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitFunctionVariables());

    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("var x_200 = mat3x2f("
                                                                  "vec2f(1.5f, 2.0f), "
                                                                  "vec2f(2.0f, 3.0f), "
                                                                  "vec2f(3.0f, 4.0f));"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_ArrayInitializer) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %ptr = OpTypePointer Function %arr2uint
     %two = OpConstant %uint 2
     %const = OpConstantComposite %arr2uint %uint_1 %two

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitFunctionVariables());

    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("var x_200 = array<u32, 2u>(1u, 2u);"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_ArrayInitializer_Alias) {
    auto p = parser(test::Assemble(R"(
     OpCapability Shader
     OpMemoryModel Logical Simple
     OpEntryPoint Fragment %100 "main"
     OpExecutionMode %100 OriginUpperLeft
     OpDecorate %arr2uint ArrayStride 16
)" + CommonTypes() + R"(
     %ptr = OpTypePointer Function %arr2uint
     %two = OpConstant %uint 2
     %const = OpConstantComposite %arr2uint %uint_1 %two

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitFunctionVariables());

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    const char* expect = "var x_200 = Arr(1u, 2u);\n";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_ArrayInitializer_Null) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %ptr = OpTypePointer Function %arr2uint
     %two = OpConstant %uint 2
     %const = OpConstantNull %arr2uint

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitFunctionVariables());

    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("var x_200 = array<u32, 2u>();"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_ArrayInitializer_Alias_Null) {
    auto p = parser(test::Assemble(R"(
     OpCapability Shader
     OpMemoryModel Logical Simple
     OpEntryPoint Fragment %100 "main"
     OpExecutionMode %100 OriginUpperLeft
     OpDecorate %arr2uint ArrayStride 16
)" + CommonTypes() + R"(
     %ptr = OpTypePointer Function %arr2uint
     %two = OpConstant %uint 2
     %const = OpConstantNull %arr2uint

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitFunctionVariables());

    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("var x_200 = @stride(16) array<u32, 2u>();"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_StructInitializer) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %ptr = OpTypePointer Function %strct
     %two = OpConstant %uint 2
     %arrconst = OpConstantComposite %arr2uint %uint_1 %two
     %const = OpConstantComposite %strct %uint_1 %float_1p5 %arrconst

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitFunctionVariables());

    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("var x_200 = S(1u, 1.5f, array<u32, 2u>(1u, 2u));"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_StructInitializer_Null) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %ptr = OpTypePointer Function %strct
     %two = OpConstant %uint 2
     %arrconst = OpConstantComposite %arr2uint %uint_1 %two
     %const = OpConstantNull %strct

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitFunctionVariables());

    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("var x_200 = S(0u, 0.0f, array<u32, 2u>());"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_Decorate_RelaxedPrecision) {
    // RelaxedPrecisionis dropped
    const auto assembly = Caps({"myvar"}) + R"(
     OpDecorate %myvar RelaxedPrecision

     %float = OpTypeFloat 32
     %ptr = OpTypePointer Function %float

     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %myvar = OpVariable %ptr Function
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitFunctionVariables());

    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    EXPECT_EQ(got, "var myvar : f32;\n") << got;
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_MemberDecorate_RelaxedPrecision) {
    // RelaxedPrecisionis dropped
    const auto assembly = Caps({"myvar", "strct"}) + R"(
     OpMemberDecorate %strct 0 RelaxedPrecision

     %float = OpTypeFloat 32
     %strct = OpTypeStruct %float
     %ptr = OpTypePointer Function %strct

     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %myvar = OpVariable %ptr Function
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error() << "\n";
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitFunctionVariables());

    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    EXPECT_EQ(got, "var myvar : strct;\n") << got;
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_StructDifferOnlyInMemberName) {
    auto p = parser(test::Assemble(R"(
      OpCapability Shader
      OpMemoryModel Logical Simple
      OpEntryPoint Fragment %100 "main"
      OpExecutionMode %100 OriginUpperLeft
      OpName %_struct_5 "S"
      OpName %_struct_6 "S"
      OpMemberName %_struct_5 0 "algo"
      OpMemberName %_struct_6 0 "rithm"

      %void = OpTypeVoid
      %voidfn = OpTypeFunction %void
      %uint = OpTypeInt 32 0

      %_struct_5 = OpTypeStruct %uint
      %_struct_6 = OpTypeStruct %uint
      %_ptr_Function__struct_5 = OpTypePointer Function %_struct_5
      %_ptr_Function__struct_6 = OpTypePointer Function %_struct_6
      %100 = OpFunction %void None %voidfn
      %39 = OpLabel
      %40 = OpVariable %_ptr_Function__struct_5 Function
      %41 = OpVariable %_ptr_Function__struct_6 Function
      OpReturn
      OpFunctionEnd)"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitFunctionVariables());

    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr(R"(var x_40 : S;
var x_41 : S_1;
)"));
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_CombinatorialValue_Defer_UsedOnceSameConstruct) {
    auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     %25 = OpVariable %ptr_uint Function
     %2 = OpIAdd %uint %uint_1 %uint_1
     OpStore %25 %uint_1 ; Do initial store to mark source location
     OpBranch %20

     %20 = OpLabel
     OpStore %25 %2 ; defer emission of the addition until here.
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect =
        R"(var x_25 : u32;
x_25 = 1u;
x_25 = (1u + 1u);
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_CombinatorialValue_Immediate_UsedTwice) {
    auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     %25 = OpVariable %ptr_uint Function
     %2 = OpIAdd %uint %uint_1 %uint_1
     OpStore %25 %uint_1 ; Do initial store to mark source location
     OpBranch %20

     %20 = OpLabel
     OpStore %25 %2
     OpStore %25 %2
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var x_25 : u32;
let x_2 = (1u + 1u);
x_25 = 1u;
x_25 = x_2;
x_25 = x_2;
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest,
       EmitStatement_CombinatorialValue_Immediate_UsedOnceDifferentConstruct) {
    // Translation should not sink expensive operations into or out of control
    // flow. As a simple heuristic, don't move *any* combinatorial operation
    // across any control flow.
    auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     %25 = OpVariable %ptr_uint Function
     %2 = OpIAdd %uint %uint_1 %uint_1
     OpStore %25 %uint_1 ; Do initial store to mark source location
     OpBranch %20

     %20 = OpLabel  ; Introduce a new construct
     OpLoopMerge %99 %80 None
     OpBranch %80

     %80 = OpLabel
     OpStore %25 %2  ; store combinatorial value %2, inside the loop
     OpBranch %20

     %99 = OpLabel ; merge block
     OpStore %25 %uint_2
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var x_25 : u32;
let x_2 = (1u + 1u);
x_25 = 1u;
loop {

  continuing {
    x_25 = x_2;
  }
}
x_25 = 2u;
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest,
       EmitStatement_CombinatorialNonPointer_DefConstruct_DoesNotEncloseAllUses) {
    // Compensate for the difference between dominance and scoping.
    // Exercise hoisting of the constant definition to before its natural
    // location.
    //
    // The definition of %2 should be hoisted
    auto assembly = Preamble() + R"(
     %pty = OpTypePointer Private %uint
     %1 = OpVariable %pty Private

     %100 = OpFunction %void None %voidfn

     %3 = OpLabel
     OpStore %1 %uint_0
     OpBranch %5

     %5 = OpLabel
     OpStore %1 %uint_1
     OpLoopMerge  %99 %80 None
     OpBranchConditional %false %99 %20

     %20 = OpLabel
     OpStore %1 %uint_3
     OpSelectionMerge %50 None
     OpBranchConditional %true %30 %40

     %30 = OpLabel
     ; This combinatorial definition in nested control flow dominates
     ; the use in the merge block in %50
     %2 = OpIAdd %uint %uint_1 %uint_1
     OpBranch %50

     %40 = OpLabel
     OpReturn

     %50 = OpLabel ; merge block for if-selection
     OpStore %1 %2
     OpBranch %80

     %80 = OpLabel ; merge block
     OpStore %1 %uint_4
     OpBranchConditional %false %99 %5 ; loop backedge

     %99 = OpLabel
     OpStore %1 %uint_5
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(x_1 = 0u;
loop {
  var x_2 : u32;
  x_1 = 1u;
  if (false) {
    break;
  }
  x_1 = 3u;
  if (true) {
    x_2 = (1u + 1u);
  } else {
    return;
  }
  x_1 = x_2;

  continuing {
    x_1 = 4u;
    break if false;
  }
}
x_1 = 5u;
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest,
       EmitStatement_CombinatorialNonPointer_Hoisting_DefFirstBlockIf_InFunction) {
    // This is a hoisting case, where the definition is in the first block
    // of an if selection construct. In this case the definition should count
    // as being in the parent (enclosing) construct.
    //
    // The definition of %1 is in an IfSelection construct and also the enclosing
    // Function construct, both of which start at block %10. For the purpose of
    // determining the construct containing %10, go to the parent construct of
    // the IfSelection.
    auto assembly = Preamble() + R"(
     %pty = OpTypePointer Private %uint
     %200 = OpVariable %pty Private
     %cond = OpConstantTrue %bool

     %100 = OpFunction %void None %voidfn

     ; in IfSelection construct, nested in Function construct
     %10 = OpLabel
     %1 = OpCopyObject %uint %uint_1
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel  ; in IfSelection construct
     OpBranch %99

     %99 = OpLabel
     %3 = OpCopyObject %uint %1; in Function construct
     OpStore %200 %3
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    // We don't hoist x_1 into its own mutable variable. It is emitted as
    // a const definition.
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(let x_1 = 1u;
if (true) {
}
let x_3 = x_1;
x_200 = x_3;
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest,
       EmitStatement_CombinatorialNonPointer_Hoisting_DefFirstBlockIf_InIf) {
    // This is like the previous case, but the IfSelection is nested inside
    // another IfSelection.
    // This tests that the hoisting algorithm goes to only one parent of
    // the definining if-selection block, and doesn't jump all the way out
    // to the Function construct that encloses everything.
    //
    // We should not hoist %1 because its definition should count as being
    // in the outer IfSelection, not the inner IfSelection.
    auto assembly = Preamble() + R"(

     %pty = OpTypePointer Private %uint
     %200 = OpVariable %pty Private
     %cond = OpConstantTrue %bool

     %100 = OpFunction %void None %voidfn

     ; outer IfSelection
     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     ; inner IfSelection
     %20 = OpLabel
     %1 = OpCopyObject %uint %uint_1
     OpSelectionMerge %89 None
     OpBranchConditional %cond %30 %89

     %30 = OpLabel ; last block of inner IfSelection
     OpBranch %89

     ; in outer IfSelection
     %89 = OpLabel
     %3 = OpCopyObject %uint %1; Last use of %1, in outer IfSelection
     OpStore %200 %3
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(if (true) {
  let x_1 = 1u;
  if (true) {
  }
  let x_3 = x_1;
  x_200 = x_3;
}
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest,
       EmitStatement_CombinatorialNonPointer_Hoisting_DefFirstBlockSwitch_InIf) {
    // This is like the previous case, but the definition is in a SwitchSelection
    // inside another IfSelection.
    // Tests that definitions in the first block of a switch count as being
    // in the parent of the switch construct.
    auto assembly = Preamble() + R"(
     %pty = OpTypePointer Private %uint
     %200 = OpVariable %pty Private
     %cond = OpConstantTrue %bool

     %100 = OpFunction %void None %voidfn

     ; outer IfSelection
     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     ; inner SwitchSelection
     %20 = OpLabel
     %1 = OpCopyObject %uint %uint_1
     OpSelectionMerge %89 None
     OpSwitch %uint_1 %89 0 %30

     %30 = OpLabel ; last block of inner SwitchSelection
     OpBranch %89

     ; in outer IfSelection
     %89 = OpLabel
     %3 = OpCopyObject %uint %1; Last use of %1, in outer IfSelection
     OpStore %200 %3
     OpBranch %99

     %99 = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(if (true) {
  let x_1 = 1u;
  switch(1u) {
    case 0u: {
    }
    default: {
    }
  }
  let x_3 = x_1;
  x_200 = x_3;
}
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest,
       EmitStatement_CombinatorialNonPointer_Hoisting_DefAndUseFirstBlockIf) {
    // In this test, both the defintion and the use are in the first block
    // of an IfSelection.  No hoisting occurs because hoisting is triggered
    // on whether the defining construct contains the last use, rather than
    // whether the two constructs are the same.
    //
    // This example has two SSA IDs which are tempting to hoist but should not:
    //   %1 is defined and used in the first block of an IfSelection.
    //       Do not hoist it.
    auto assembly = Preamble() + R"(
     %cond = OpConstantTrue %bool

     %100 = OpFunction %void None %voidfn

     ; in IfSelection construct, nested in Function construct
     %10 = OpLabel
     %1 = OpCopyObject %uint %uint_1
     %2 = OpCopyObject %uint %1
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel  ; in IfSelection construct
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    // We don't hoist x_1 into its own mutable variable. It is emitted as
    // a const definition.
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(let x_1 = 1u;
let x_2 = x_1;
if (true) {
}
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Phi_SimultaneousAssignment) {
    // Phis must act as if they are simutaneously assigned.
    // %101 and %102 should exchange values on each iteration, and never have
    // the same value.
    auto assembly = Preamble() + R"(
%100 = OpFunction %void None %voidfn

%10 = OpLabel
OpBranch %20

%20 = OpLabel
%101 = OpPhi %bool %true %10 %102 %20
%102 = OpPhi %bool %false %10 %101 %20
OpLoopMerge %99 %20 None
OpBranchConditional %true %99 %20

%99 = OpLabel
OpReturn

OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var x_101 : bool;
var x_102 : bool;
x_101 = true;
x_102 = false;
loop {
  let x_101_c20 = x_101;
  let x_102_c20 = x_102;
  x_101 = x_102_c20;
  x_102 = x_101_c20;
  if (true) {
    break;
  }
}
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Phi_SingleBlockLoopIndex) {
    auto assembly = Preamble() + R"(
     %pty = OpTypePointer Private %uint
     %1 = OpVariable %pty Private
     %boolpty = OpTypePointer Private %bool
     %7 = OpVariable %boolpty Private
     %8 = OpVariable %boolpty Private

     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     OpBranch %10

     ; Use an outer loop to show we put the new variable in the
     ; smallest enclosing scope.
     %10 = OpLabel
     %101 = OpLoad %bool %7
     %102 = OpLoad %bool %8
     OpLoopMerge %99 %89 None
     OpBranchConditional %101 %99 %20

     %20 = OpLabel
     %2 = OpPhi %uint %uint_0 %10 %4 %20  ; gets computed value
     %3 = OpPhi %uint %uint_1 %10 %3 %20  ; gets itself
     %4 = OpIAdd %uint %2 %uint_1
     OpLoopMerge %79 %20 None
     OpBranchConditional %102 %79 %20

     %79 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %10

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  var x_2 : u32;
  var x_3 : u32;
  let x_101 = x_7;
  let x_102 = x_8;
  x_2 = 0u;
  x_3 = 1u;
  if (x_101) {
    break;
  }
  loop {
    let x_3_c20 = x_3;
    x_2 = (x_2 + 1u);
    x_3 = x_3_c20;
    if (x_102) {
      break;
    }
  }
}
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Phi_MultiBlockLoopIndex) {
    auto assembly = Preamble() + R"(
     %pty = OpTypePointer Private %uint
     %1 = OpVariable %pty Private
     %boolpty = OpTypePointer Private %bool
     %7 = OpVariable %boolpty Private
     %8 = OpVariable %boolpty Private

     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     OpBranch %10

     ; Use an outer loop to show we put the new variable in the
     ; smallest enclosing scope.
     %10 = OpLabel
     %101 = OpLoad %bool %7
     %102 = OpLoad %bool %8
     OpLoopMerge %99 %89 None
     OpBranchConditional %101 %99 %20

     %20 = OpLabel
     %2 = OpPhi %uint %uint_0 %10 %4 %30  ; gets computed value
     %3 = OpPhi %uint %uint_1 %10 %3 %30  ; gets itself
     OpLoopMerge %79 %30 None
     OpBranchConditional %102 %79 %30

     %30 = OpLabel
     %4 = OpIAdd %uint %2 %uint_1
     OpBranch %20

     %79 = OpLabel
     OpBranch %89

     %89 = OpLabel ; continue target for outer loop
     OpBranch %10

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  var x_2 : u32;
  var x_3 : u32;
  let x_101 = x_7;
  let x_102 = x_8;
  x_2 = 0u;
  x_3 = 1u;
  if (x_101) {
    break;
  }
  loop {
    var x_4 : u32;
    if (x_102) {
      break;
    }

    continuing {
      x_4 = (x_2 + 1u);
      let x_3_c30 = x_3;
      x_2 = x_4;
      x_3 = x_3_c30;
    }
  }
}
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Phi_ValueFromLoopBodyAndContinuing) {
    auto assembly = Preamble() + R"(
     %pty = OpTypePointer Private %uint
     %1 = OpVariable %pty Private
     %boolpty = OpTypePointer Private %bool
     %17 = OpVariable %boolpty Private

     %100 = OpFunction %void None %voidfn

     %9 = OpLabel
     %101 = OpLoad %bool %17
     OpBranch %10

     ; Use an outer loop to show we put the new variable in the
     ; smallest enclosing scope.
     %10 = OpLabel
     OpLoopMerge %99 %89 None
     OpBranch %20

     %20 = OpLabel
     %2 = OpPhi %uint %uint_0 %10 %4 %30  ; gets computed value
     %5 = OpPhi %uint %uint_1 %10 %7 %30
     %4 = OpIAdd %uint %2 %uint_1 ; define %4
     %6 = OpIAdd %uint %4 %uint_1 ; use %4
     OpLoopMerge %79 %30 None
     OpBranchConditional %101 %79 %30

     %30 = OpLabel
     %7 = OpIAdd %uint %4 %6 ; use %4 again
     %8 = OpCopyObject %uint %5 ; use %5
     OpBranch %20

     %79 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %10

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(let x_101 = x_17;
loop {
  var x_2 : u32;
  var x_5 : u32;
  x_2 = 0u;
  x_5 = 1u;
  loop {
    var x_4 : u32;
    var x_6 : u32;
    var x_7 : u32;
    x_4 = (x_2 + 1u);
    x_6 = (x_4 + 1u);
    if (x_101) {
      break;
    }

    continuing {
      x_7 = (x_4 + x_6);
      let x_8 = x_5;
      x_2 = x_4;
      x_5 = x_7;
    }
  }
}
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Phi_FromElseAndThen) {
    auto assembly = Preamble() + R"(
     %pty = OpTypePointer Private %uint
     %1 = OpVariable %pty Private
     %boolpty = OpTypePointer Private %bool
     %7 = OpVariable %boolpty Private
     %8 = OpVariable %boolpty Private

     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     %101 = OpLoad %bool %7
     %102 = OpLoad %bool %8
     OpBranch %10

     ; Use an outer loop to show we put the new variable in the
     ; smallest enclosing scope.
     %10 = OpLabel
     OpLoopMerge %99 %89 None
     OpBranchConditional %101 %99 %20

     %20 = OpLabel ; if seleciton
     OpSelectionMerge %79 None
     OpBranchConditional %102 %30 %40

     %30 = OpLabel
     OpBranch %89

     %40 = OpLabel
     OpBranch %89

     %79 = OpLabel ; disconnected selection merge node
     OpBranch %89

     %89 = OpLabel
     %2 = OpPhi %uint %uint_0 %30 %uint_1 %40 %uint_0 %79
     OpStore %1 %2
     OpBranch %10

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(let x_101 = x_7;
let x_102 = x_8;
loop {
  var x_2 : u32;
  if (x_101) {
    break;
  }
  if (x_102) {
    x_2 = 0u;
    continue;
  } else {
    x_2 = 1u;
    continue;
  }
  x_2 = 0u;

  continuing {
    x_1 = x_2;
  }
}
return;
)";
    EXPECT_EQ(expect, got) << got;
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Phi_FromHeaderAndThen) {
    auto assembly = Preamble() + R"(
     %pty = OpTypePointer Private %uint
     %1 = OpVariable %pty Private
     %boolpty = OpTypePointer Private %bool
     %7 = OpVariable %boolpty Private
     %8 = OpVariable %boolpty Private

     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     %101 = OpLoad %bool %7
     %102 = OpLoad %bool %8
     OpBranch %10

     ; Use an outer loop to show we put the new variable in the
     ; smallest enclosing scope.
     %10 = OpLabel
     OpLoopMerge %99 %89 None
     OpBranchConditional %101 %99 %20

     %20 = OpLabel ; if seleciton
     OpSelectionMerge %79 None
     OpBranchConditional %102 %30 %89

     %30 = OpLabel
     OpBranch %89

     %79 = OpLabel ; disconnected selection merge node
     OpUnreachable

     %89 = OpLabel
     %2 = OpPhi %uint %uint_0 %20 %uint_1 %30
     OpStore %1 %2
     OpBranch %10

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(let x_101 = x_7;
let x_102 = x_8;
loop {
  var x_2 : u32;
  if (x_101) {
    break;
  }
  x_2 = 0u;
  if (x_102) {
    x_2 = 1u;
    continue;
  } else {
    continue;
  }
  return;

  continuing {
    x_1 = x_2;
  }
}
return;
)";
    EXPECT_EQ(expect, got) << got;
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Phi_UseInPhiCountsAsUse) {
    // From crbug.com/215
    // If the only use of a combinatorially computed ID is as the value
    // in an OpPhi, then we still have to emit it.  The algorithm fix
    // is to always count uses in Phis.
    // This is the reduced case from the bug report.
    //
    // The only use of %12 is in the phi.
    // The only use of %11 is in %12.
    // Both definintions need to be emitted to the output.
    auto assembly = Preamble() + R"(
        %100 = OpFunction %void None %voidfn

         %10 = OpLabel
         %11 = OpLogicalAnd %bool %true %true
         %12 = OpLogicalNot %bool %11  ;
               OpSelectionMerge %99 None
               OpBranchConditional %true %20 %99

         %20 = OpLabel
               OpBranch %99

         %99 = OpLabel
        %101 = OpPhi %bool %11 %10 %12 %20
        %102 = OpCopyObject %bool %101  ;; ensure a use of %101
               OpReturn

               OpFunctionEnd

  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var x_101 : bool;
let x_11 = (true & true);
let x_12 = !(x_11);
x_101 = x_11;
if (true) {
  x_101 = x_12;
}
let x_102 = x_101;
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Phi_PhiInLoopHeader_FedByHoistedVar_PhiUnused) {
    // From investigation into crbug.com/1649
    //
    // Value %999 is defined deep in control flow, then we arrange for
    // it to dominate the backedge of the outer loop. The %999 value is then
    // fed back into the phi in the loop header.  So %999 needs to be hoisted
    // out of the loop.  The phi assignment needs to use the hoisted variable.
    // The hoisted variable needs to be placed such that its scope encloses
    // that phi in the header of the outer loop. The compiler needs
    // to "see" that there is an implicit use of %999 in the backedge block
    // of that outer loop.
    auto assembly = Preamble() + R"(
%100 = OpFunction %void None %voidfn

%10 = OpLabel
OpBranch %20

%20 = OpLabel
%101 = OpPhi %bool %true %10 %999 %80
OpLoopMerge %99 %80 None
OpBranchConditional %true %30 %99

  %30 = OpLabel
  OpSelectionMerge %50 None
  OpBranchConditional %true %40 %50

    %40 = OpLabel
    %999 = OpCopyObject %bool %true
    OpBranch %60

    %50 = OpLabel
    OpReturn

  %60 = OpLabel ; if merge
  OpBranch %80

  %80 = OpLabel ; continue target
  OpBranch %20

%99 = OpLabel
OpReturn

OpFunctionEnd

  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  var x_999 : bool;
  if (true) {
  } else {
    break;
  }
  if (true) {
    x_999 = true;
    continue;
  }
  return;
}
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Phi_PhiInLoopHeader_FedByHoistedVar_PhiUsed) {
    // From investigation into crbug.com/1649
    //
    // Value %999 is defined deep in control flow, then we arrange for
    // it to dominate the backedge of the outer loop. The %999 value is then
    // fed back into the phi in the loop header.  So %999 needs to be hoisted
    // out of the loop.  The phi assignment needs to use the hoisted variable.
    // The hoisted variable needs to be placed such that its scope encloses
    // that phi in the header of the outer loop. The compiler needs
    // to "see" that there is an implicit use of %999 in the backedge block
    // of that outer loop.
    auto assembly = Preamble() + R"(
%100 = OpFunction %void None %voidfn

%10 = OpLabel
OpBranch %20

%20 = OpLabel
%101 = OpPhi %bool %true %10 %999 %80
OpLoopMerge %99 %80 None
OpBranchConditional %true %30 %99

  %30 = OpLabel
  OpSelectionMerge %50 None
  OpBranchConditional %true %40 %50

    %40 = OpLabel
    %999 = OpCopyObject %bool %true
    OpBranch %60

    %50 = OpLabel
    OpReturn

  %60 = OpLabel ; if merge
  OpBranch %80

  %80 = OpLabel ; continue target
  OpBranch %20

%99 = OpLabel
%1000 = OpCopyObject %bool %101
OpReturn

OpFunctionEnd

  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var x_101 : bool;
x_101 = true;
loop {
  var x_999 : bool;
  if (true) {
  } else {
    break;
  }
  if (true) {
    x_999 = true;
    continue;
  }
  return;

  continuing {
    x_101 = x_999;
  }
}
let x_1000 = x_101;
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Phi_PhiInLoopHeader_FedByPhi_PhiUnused) {
    // From investigation into crbug.com/1649
    //
    // This is a reduction of one of the hard parts of test case
    // vk-gl-cts/graphicsfuzz/stable-binarysearch-tree-false-if-discard-loop/1.spvasm
    // In particular, see the data flow around %114 in that case.
    //
    // Here value %999 is is a *phi* defined deep in control flow, then we
    // arrange for it to dominate the backedge of the outer loop. The %999
    // value is then fed back into the phi in the loop header.  The variable
    // generated to hold the %999 value needs to be placed such that its scope
    // encloses that phi in the header of the outer loop. The compiler needs
    // to "see" that there is an implicit use of %999 in the backedge block
    // of that outer loop.
    auto assembly = Preamble() + R"(
%100 = OpFunction %void None %voidfn

%10 = OpLabel
OpBranch %20

%20 = OpLabel
%101 = OpPhi %bool %true %10 %999 %80
OpLoopMerge %99 %80 None
OpBranchConditional %true %99 %30

  %30 = OpLabel
  OpLoopMerge %70 %60 None
  OpBranch %40

    %40 = OpLabel
    OpBranchConditional %true %60 %50

      %50 = OpLabel
      OpBranch %60

    %60 = OpLabel ; inner continue
    %999 = OpPhi %bool %true %40 %false %50
    OpBranchConditional %true %70 %30

  %70 = OpLabel  ; inner merge
  OpBranch %80

  %80 = OpLabel ; outer continue target
  OpBranch %20

%99 = OpLabel
OpReturn

OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  var x_999 : bool;
  if (true) {
    break;
  }
  loop {
    x_999 = true;
    if (true) {
      continue;
    }
    x_999 = false;

    continuing {
      break if true;
    }
  }
}
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Phi_PhiInLoopHeader_FedByPhi_PhiUsed) {
    // From investigation into crbug.com/1649
    //
    // This is a reduction of one of the hard parts of test case
    // vk-gl-cts/graphicsfuzz/stable-binarysearch-tree-false-if-discard-loop/1.spvasm
    // In particular, see the data flow around %114 in that case.
    //
    // Here value %999 is is a *phi* defined deep in control flow, then we
    // arrange for it to dominate the backedge of the outer loop. The %999
    // value is then fed back into the phi in the loop header.  The variable
    // generated to hold the %999 value needs to be placed such that its scope
    // encloses that phi in the header of the outer loop. The compiler needs
    // to "see" that there is an implicit use of %999 in the backedge block
    // of that outer loop.
    auto assembly = Preamble() + R"(
%100 = OpFunction %void None %voidfn

%10 = OpLabel
OpBranch %20

%20 = OpLabel
%101 = OpPhi %bool %true %10 %999 %80
OpLoopMerge %99 %80 None
OpBranchConditional %true %99 %30

  %30 = OpLabel
  OpLoopMerge %70 %60 None
  OpBranch %40

    %40 = OpLabel
    OpBranchConditional %true %60 %50

      %50 = OpLabel
      OpBranch %60

    %60 = OpLabel ; inner continue
    %999 = OpPhi %bool %true %40 %false %50
    OpBranchConditional %true %70 %30

  %70 = OpLabel  ; inner merge
  OpBranch %80

  %80 = OpLabel ; outer continue target
  OpBranch %20

%99 = OpLabel
%1000 = OpCopyObject %bool %101
OpReturn

OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var x_101 : bool;
x_101 = true;
loop {
  var x_999 : bool;
  if (true) {
    break;
  }
  loop {
    x_999 = true;
    if (true) {
      continue;
    }
    x_999 = false;

    continuing {
      break if true;
    }
  }

  continuing {
    x_101 = x_999;
  }
}
let x_1000 = x_101;
return;
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Hoist_CompositeInsert) {
    // From crbug.com/tint/804
    const auto assembly = Preamble() + R"(
    %100 = OpFunction %void None %voidfn

    %10 = OpLabel
    OpSelectionMerge %50 None
    OpBranchConditional %true %20 %30

      %20 = OpLabel
      %200 = OpCompositeInsert %v2int %int_0 %v2int_null 0
      OpBranch %50

      %30 = OpLabel
      OpReturn

    %50 = OpLabel   ; dominated by %20, but %200 needs to be hoisted
    %201 = OpCopyObject %v2int %200
    OpReturn
    OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    const auto* expected = R"(var x_200 : vec2i;
if (true) {
  x_200 = vec2i();
  x_200.x = 0i;
} else {
  return;
}
let x_201 = x_200;
return;
)";
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    EXPECT_EQ(got, expected);
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Hoist_VectorInsertDynamic) {
    // Spawned from crbug.com/tint/804
    const auto assembly = Preamble() + R"(
    %100 = OpFunction %void None %voidfn

    %10 = OpLabel
    OpSelectionMerge %50 None
    OpBranchConditional %true %20 %30

      %20 = OpLabel
      %200 = OpVectorInsertDynamic %v2int %v2int_null %int_3 %int_1
      OpBranch %50

      %30 = OpLabel
      OpReturn

    %50 = OpLabel   ; dominated by %20, but %200 needs to be hoisted
    %201 = OpCopyObject %v2int %200
    OpReturn
    OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    const auto* expected = R"(var x_200 : vec2i;
if (true) {
  x_200 = vec2i();
  x_200[1i] = 3i;
} else {
  return;
}
let x_201 = x_200;
return;
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Hoist_UsedAsNonPtrArg) {
    // Spawned from crbug.com/tint/804
    const auto assembly = Preamble() + R"(
    %fn_int = OpTypeFunction %void %int

    %500 = OpFunction %void None %fn_int
    %501 = OpFunctionParameter %int
    %502 = OpLabel
    OpReturn
    OpFunctionEnd

    %100 = OpFunction %void None %voidfn

    %10 = OpLabel
    OpSelectionMerge %50 None
    OpBranchConditional %true %20 %30

      %20 = OpLabel
      %200 = OpCopyObject %int %int_1
      OpBranch %50

      %30 = OpLabel
      OpReturn

    %50 = OpLabel   ; dominated by %20, but %200 needs to be hoisted
    %201 = OpFunctionCall %void %500 %200
    OpReturn
    OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    const auto* expected = R"(var x_200 : i32;
if (true) {
  x_200 = 1i;
} else {
  return;
}
x_500(x_200);
return;
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvParserFunctionVarTest, DISABLED_EmitStatement_Hoist_UsedAsPtrArg) {
    // Spawned from crbug.com/tint/804
    // Blocked by crbug.com/tint/98: hoisting pointer types
    const auto assembly = Preamble() + R"(

    %fn_int = OpTypeFunction %void %ptr_int

    %500 = OpFunction %void None %fn_int
    %501 = OpFunctionParameter %ptr_int
    %502 = OpLabel
    OpReturn
    OpFunctionEnd

    %100 = OpFunction %void None %voidfn

    %10 = OpLabel
    %199 = OpVariable %ptr_int Function
    OpSelectionMerge %50 None
    OpBranchConditional %true %20 %30

      %20 = OpLabel
      %200 = OpCopyObject %ptr_int %199
      OpBranch %50

      %30 = OpLabel
      OpReturn

    %50 = OpLabel   ; dominated by %20, but %200 needs to be hoisted
    %201 = OpFunctionCall %void %500 %200
    OpReturn
    OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    const auto* expected = R"(xxxxxxxxxxxxxxxxxxxxx)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Phi_UnreachableLoopMerge) {
    // A phi in an unreachable block may have no operands.
    auto assembly = Preamble() + R"(
%100 = OpFunction %void None %voidfn

 %10 = OpLabel
       OpBranch %99

 %99 = OpLabel
       OpLoopMerge %101 %99 None
       OpBranch %99

%101 = OpLabel
%102 = OpPhi %uint
       OpUnreachable

OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
}
return;
)";
    EXPECT_EQ(expect, got);
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
