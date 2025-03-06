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
#include "src/tint/lang/spirv/reader/ast_parser/helper_test.h"
#include "src/tint/lang/spirv/reader/ast_parser/spirv_tools_helpers_test.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ::testing::HasSubstr;

std::string Caps() {
    return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
  )";
}

std::string Preamble() {
    return Caps() + R"(
    OpEntryPoint Fragment %main "x_100"
    OpExecutionMode %main OriginUpperLeft
  )";
}

std::string MainBody() {
    return R"(
    %main = OpFunction %void None %voidfn
    %main_entry = OpLabel
    OpReturn
    OpFunctionEnd
  )";
}

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
    return R"(
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
  )";
}

std::string BuiltinPosition() {
    return R"(OpDecorate %position BuiltIn Position
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %ptr = OpTypePointer Output %v4float
    %position = OpVariable %ptr Output
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
)";
}

TEST_F(SpirvASTParserTest, EmitFunctions_NoFunctions) {
    auto p = parser(test::Assemble(
        R"(
     OpCapability Shader
     OpMemoryModel Logical Simple
)" + CommonTypes()));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    Program program = p->program();
    const auto program_ast = test::ToString(program);
    EXPECT_THAT(program_ast, Not(HasSubstr("Function{")));
    p->SkipDumpingPending("Not valid for Vulkan: needs an entry point");
}

TEST_F(SpirvASTParserTest, EmitFunctions_FunctionWithoutBody) {
    auto p = parser(test::Assemble(Preamble() + Names({"main"}) + CommonTypes() + R"(
     %main = OpFunction %void None %voidfn
     OpFunctionEnd
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    Program program = p->program();
    const auto program_ast = test::ToString(program);
    EXPECT_THAT(program_ast, Not(HasSubstr("Function{")));
    p->SkipDumpingPending("Missing an entry point body requires Linkage");
}

TEST_F(SpirvASTParserTest, EmitFunctions_Function_EntryPoint_Vertex) {
    std::string input = Caps() + R"(OpEntryPoint Vertex %main "main" %position )" +
                        Names({"main"}) + BuiltinPosition() + R"(

%main = OpFunction %void None %voidfn
%entry = OpLabel
OpReturn
OpFunctionEnd)";

    auto p = parser(test::Assemble(input));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    ASSERT_TRUE(p->error().empty()) << p->error();
    Program program = p->program();
    const auto program_ast = test::ToString(program);
    EXPECT_THAT(program_ast, HasSubstr(R"(
struct main_out {
  @builtin(position)
  x_2_1 : vec4f,
}
)")) << program_ast;

    EXPECT_THAT(program_ast, HasSubstr(R"(
@vertex
fn main() -> main_out {
)"));
}

TEST_F(SpirvASTParserTest, EmitFunctions_Function_EntryPoint_Fragment) {
    std::string input = Caps() + R"(
     OpEntryPoint Fragment %main "main"
     OpExecutionMode %main OriginUpperLeft
)" + Names({"main"}) + CommonTypes() +
                        MainBody();

    auto p = parser(test::Assemble(input));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    ASSERT_TRUE(p->error().empty()) << p->error();
    Program program = p->program();
    const auto program_ast = test::ToString(program);
    EXPECT_THAT(program_ast, HasSubstr(R"(
@fragment
fn main() {
)"));
}

TEST_F(SpirvASTParserTest, EmitFunctions_Function_EntryPoint_GLCompute) {
    std::string input = Caps() + R"(
      OpEntryPoint GLCompute %main "main"
      OpExecutionMode %main LocalSize 1 1 1
)" + Names({"main"}) + CommonTypes() +
                        MainBody();

    auto p = parser(test::Assemble(input));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    ASSERT_TRUE(p->error().empty()) << p->error();
    Program program = p->program();
    const auto program_ast = test::ToString(program);
    EXPECT_THAT(program_ast, HasSubstr(R"(
@compute @workgroup_size(1i, 1i, 1i)
fn main() {
)"));
}

TEST_F(SpirvASTParserTest, EmitFunctions_Function_EntryPoint_MultipleEntryPoints) {
    std::string input = Caps() +
                        R"(
OpEntryPoint Fragment %main "first_shader"
OpEntryPoint Fragment %main "second_shader"
OpExecutionMode %main OriginUpperLeft
)" + Names({"main"}) + CommonTypes() +
                        MainBody();

    auto p = parser(test::Assemble(input));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    ASSERT_TRUE(p->error().empty()) << p->error();
    Program program = p->program();
    const auto program_ast = test::ToString(program);
    EXPECT_THAT(program_ast, HasSubstr(R"(
@fragment
fn first_shader() {
)"));
    EXPECT_THAT(program_ast, HasSubstr(R"(
@fragment
fn second_shader() {
)"));
}

TEST_F(SpirvASTParserTest, EmitFunctions_Function_EntryPoint_GLCompute_LocalSize_Only) {
    std::string input = Caps() + R"(
OpEntryPoint GLCompute %main "comp_main"
OpExecutionMode %main LocalSize 2 4 8
)" + Names({"main"}) + CommonTypes() +
                        R"(
%main = OpFunction %void None %voidfn
%entry = OpLabel
OpReturn
OpFunctionEnd)";

    auto p = parser(test::Assemble(input));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    ASSERT_TRUE(p->error().empty()) << p->error();
    Program program = p->program();
    const auto program_ast = test::ToString(program);
    EXPECT_THAT(program_ast, HasSubstr(R"(
@compute @workgroup_size(2i, 4i, 8i)
fn comp_main() {
)")) << program_ast;
}

TEST_F(SpirvASTParserTest, EmitFunctions_Function_EntryPoint_WorkgroupSizeBuiltin_Constant_Only) {
    std::string input = Caps() + R"(OpEntryPoint GLCompute %main "comp_main"
OpDecorate %wgsize BuiltIn WorkgroupSize
)" + CommonTypes() + R"(
%uvec3 = OpTypeVector %uint 3
%uint_3 = OpConstant %uint 3
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%wgsize = OpConstantComposite %uvec3 %uint_3 %uint_5 %uint_7
%main = OpFunction %void None %voidfn
%entry = OpLabel
OpReturn
OpFunctionEnd)";

    auto p = parser(test::Assemble(input));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    ASSERT_TRUE(p->error().empty()) << p->error();
    Program program = p->program();
    const auto program_ast = test::ToString(program);
    EXPECT_THAT(program_ast, HasSubstr(R"(
@compute @workgroup_size(3i, 5i, 7i)
fn comp_main() {
)")) << program_ast;
}

TEST_F(SpirvASTParserTest,
       EmitFunctions_Function_EntryPoint_WorkgroupSizeBuiltin_SpecConstant_Only) {
    std::string input = Caps() +
                        R"(OpEntryPoint GLCompute %main "comp_main"
OpDecorate %wgsize BuiltIn WorkgroupSize
OpDecorate %uint_3 SpecId 0
OpDecorate %uint_5 SpecId 1
OpDecorate %uint_7 SpecId 2
)" + CommonTypes() + R"(
%uvec3 = OpTypeVector %uint 3
%uint_3 = OpSpecConstant %uint 3
%uint_5 = OpSpecConstant %uint 5
%uint_7 = OpSpecConstant %uint 7
%wgsize = OpSpecConstantComposite %uvec3 %uint_3 %uint_5 %uint_7
%main = OpFunction %void None %voidfn
%entry = OpLabel
OpReturn
OpFunctionEnd)";

    auto p = parser(test::Assemble(input));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    ASSERT_TRUE(p->error().empty()) << p->error();
    Program program = p->program();
    const auto program_ast = test::ToString(program);
    EXPECT_THAT(program_ast, HasSubstr(R"(
@compute @workgroup_size(3i, 5i, 7i)
fn comp_main() {
)")) << program_ast;
}

TEST_F(SpirvASTParserTest,
       EmitFunctions_Function_EntryPoint_WorkgroupSize_MixedConstantSpecConstant) {
    std::string input = Caps() +
                        R"(OpEntryPoint GLCompute %main "comp_main"
OpDecorate %wgsize BuiltIn WorkgroupSize
OpDecorate %uint_3 SpecId 0
OpDecorate %uint_7 SpecId 2
)" + CommonTypes() + R"(
%uvec3 = OpTypeVector %uint 3
%uint_3 = OpSpecConstant %uint 3
%uint_5 = OpConstant %uint 5
%uint_7 = OpSpecConstant %uint 7
%wgsize = OpSpecConstantComposite %uvec3 %uint_3 %uint_5 %uint_7
%main = OpFunction %void None %voidfn
%entry = OpLabel
OpReturn
OpFunctionEnd)";

    auto p = parser(test::Assemble(input));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    ASSERT_TRUE(p->error().empty()) << p->error();
    Program program = p->program();
    const auto program_ast = test::ToString(program);
    EXPECT_THAT(program_ast, HasSubstr(R"(
@compute @workgroup_size(3i, 5i, 7i)
fn comp_main() {
)")) << program_ast;
}

TEST_F(SpirvASTParserTest,
       // I had to shorten the name to pass the linter.
       EmitFunctions_Function_EntryPoint_LocalSize_And_WGSBuiltin_SpecConstant) {
    // WorkgroupSize builtin wins.
    std::string input = Caps() +
                        R"(OpEntryPoint GLCompute %main "comp_main"
OpExecutionMode %main LocalSize 2 4 8
OpDecorate %wgsize BuiltIn WorkgroupSize
OpDecorate %uint_3 SpecId 0
OpDecorate %uint_5 SpecId 1
OpDecorate %uint_7 SpecId 2
)" + CommonTypes() + R"(
%uvec3 = OpTypeVector %uint 3
%uint_3 = OpSpecConstant %uint 3
%uint_5 = OpSpecConstant %uint 5
%uint_7 = OpSpecConstant %uint 7
%wgsize = OpSpecConstantComposite %uvec3 %uint_3 %uint_5 %uint_7
%main = OpFunction %void None %voidfn
%entry = OpLabel
OpReturn
OpFunctionEnd)";

    auto p = parser(test::Assemble(input));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    ASSERT_TRUE(p->error().empty()) << p->error();
    Program program = p->program();
    const auto program_ast = test::ToString(program);
    EXPECT_THAT(program_ast, HasSubstr(R"(
@compute @workgroup_size(3i, 5i, 7i)
fn comp_main() {
)")) << program_ast;
}

TEST_F(SpirvASTParserTest, EmitFunctions_VoidFunctionWithoutParams) {
    auto p = parser(test::Assemble(Preamble() + Names({"another_function"}) + CommonTypes() + R"(
    %another_function = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)" + MainBody()));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    Program program = p->program();
    const auto program_ast = test::ToString(program);
    EXPECT_THAT(program_ast, HasSubstr(R"(fn another_function() {
)"));
}

TEST_F(SpirvASTParserTest, EmitFunctions_CalleePrecedesCaller) {
    auto p = parser(test::Assemble(
        Preamble() + Names({"root", "branch", "leaf", "leaf_result", "branch_result"}) +
        CommonTypes() + R"(
     %uintfn = OpTypeFunction %uint
     %uint_0 = OpConstant %uint 0

     %root = OpFunction %void None %voidfn
     %root_entry = OpLabel
     %branch_result = OpFunctionCall %uint %branch
     OpReturn
     OpFunctionEnd

     %branch = OpFunction %uint None %uintfn
     %branch_entry = OpLabel
     %leaf_result = OpFunctionCall %uint %leaf
     OpReturnValue %leaf_result
     OpFunctionEnd

     %leaf = OpFunction %uint None %uintfn
     %leaf_entry = OpLabel
     OpReturnValue %uint_0
     OpFunctionEnd
  )" + MainBody()));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    Program program = p->program();
    const auto program_ast = test::ToString(program);
    EXPECT_THAT(program_ast, HasSubstr(R"(fn leaf() -> u32 {
  return 0u;
}

fn branch() -> u32 {
  let leaf_result = leaf();
  return leaf_result;
}

fn root() {
  let branch_result = branch();
  return;
}
)")) << program_ast;
}

TEST_F(SpirvASTParserTest, EmitFunctions_NonVoidResultType) {
    auto p = parser(test::Assemble(Preamble() + Names({"ret_float"}) + CommonTypes() + R"(
     %float_0 = OpConstant %float 0.0
     %fn_ret_float = OpTypeFunction %float

     %ret_float = OpFunction %float None %fn_ret_float
     %ret_float_entry = OpLabel
     OpReturnValue %float_0
     OpFunctionEnd
)" + MainBody()));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    Program program = p->program();
    const auto program_ast = test::ToString(program);
    EXPECT_THAT(program_ast, HasSubstr(R"(fn ret_float() -> f32 {
  return 0.0f;
}

fn x_100_1() {
  return;
}

@fragment
fn x_100() {
  x_100_1();
}
)")) << program_ast;
}

TEST_F(SpirvASTParserTest, EmitFunctions_MixedParamTypes) {
    auto p = parser(
        test::Assemble(Preamble() + Names({"mixed_params", "a", "b", "c"}) + CommonTypes() + R"(
     %fn_mixed_params = OpTypeFunction %void %uint %float %int

     %mixed_params = OpFunction %void None %fn_mixed_params
     %a = OpFunctionParameter %uint
     %b = OpFunctionParameter %float
     %c = OpFunctionParameter %int
     %mixed_entry = OpLabel
     OpReturn
     OpFunctionEnd
  )" + MainBody()));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    Program program = p->program();
    const auto program_ast = test::ToString(program);
    EXPECT_THAT(program_ast, HasSubstr(R"(fn mixed_params(a : u32, b : f32, c : i32) {
  return;
}
)"));
}

TEST_F(SpirvASTParserTest, EmitFunctions_GenerateParamNames) {
    auto p = parser(test::Assemble(Preamble() + Names({"mixed_params"}) + CommonTypes() + R"(
     %fn_mixed_params = OpTypeFunction %void %uint %float %int

     %mixed_params = OpFunction %void None %fn_mixed_params
     %14 = OpFunctionParameter %uint
     %15 = OpFunctionParameter %float
     %16 = OpFunctionParameter %int
     %mixed_entry = OpLabel
     OpReturn
     OpFunctionEnd
  )" + MainBody()));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    Program program = p->program();
    const auto program_ast = test::ToString(program);
    EXPECT_THAT(program_ast, HasSubstr(R"(fn mixed_params(x_14 : u32, x_15 : f32, x_16 : i32) {
  return;
}
)")) << program_ast;
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
