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
#include "src/tint/utils/text/string.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using SpvModuleScopeVarParserTest = SpirvASTParserTest;

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Not;

std::string Preamble() {
    return R"(
   OpCapability Shader
   OpMemoryModel Logical Simple
)";
}

std::string FragMain() {
    return R"(
   OpEntryPoint Fragment %main "main"
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

std::string CommonCapabilities() {
    return R"(
    OpCapability Shader
    OpCapability SampleRateShading
    OpMemoryModel Logical Simple
)";
}

std::string CommonTypes() {
    return R"(
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %bool = OpTypeBool
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1

    %ptr_bool = OpTypePointer Private %bool
    %ptr_float = OpTypePointer Private %float
    %ptr_uint = OpTypePointer Private %uint
    %ptr_int = OpTypePointer Private %int

    %true = OpConstantTrue %bool
    %false = OpConstantFalse %bool
    %float_0 = OpConstant %float 0.0
    %float_1p5 = OpConstant %float 1.5
    %uint_1 = OpConstant %uint 1
    %int_m1 = OpConstant %int -1
    %int_14 = OpConstant %int 14
    %uint_2 = OpConstant %uint 2

    %v2bool = OpTypeVector %bool 2
    %v2uint = OpTypeVector %uint 2
    %v2int = OpTypeVector %int 2
    %v2float = OpTypeVector %float 2
    %v4float = OpTypeVector %float 4
    %m3v2float = OpTypeMatrix %v2float 3

    %arr2uint = OpTypeArray %uint %uint_2
  )";
}

std::string StructTypes() {
    return R"(
    %strct = OpTypeStruct %uint %float %arr2uint
)";
}

// Returns layout annotations for types in StructTypes()
std::string CommonLayout() {
    return R"(
    OpMemberDecorate %strct 0 Offset 0
    OpMemberDecorate %strct 1 Offset 4
    OpMemberDecorate %strct 2 Offset 8
    OpDecorate %arr2uint ArrayStride 4
)";
}

TEST_F(SpvModuleScopeVarParserTest, NoVar) {
    auto assembly = Preamble() + FragMain() + CommonTypes() + MainBody();
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule()) << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_ast = test::ToString(p->program());
    EXPECT_THAT(module_ast, Not(HasSubstr("Variable"))) << module_ast;
}

TEST_F(SpvModuleScopeVarParserTest, BadAddressSpace_NotAWebGPUAddressSpace) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
    %float = OpTypeFloat 32
    %ptr = OpTypePointer CrossWorkgroup %float
    %52 = OpVariable %ptr CrossWorkgroup
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());
    // Normally we should run ASTParser::RegisterTypes before emitting
    // variables. But defensive coding in EmitModuleScopeVariables lets
    // us catch this error.
    EXPECT_FALSE(p->EmitModuleScopeVariables()) << p->error();
    EXPECT_THAT(p->error(), HasSubstr("unknown SPIR-V storage class: 5"));
}

TEST_F(SpvModuleScopeVarParserTest, BadAddressSpace_Function) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
    %float = OpTypeFloat 32
    %ptr = OpTypePointer Function %float
    %52 = OpVariable %ptr Function
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());
    // Normally we should run ASTParser::RegisterTypes before emitting
    // variables. But defensive coding in EmitModuleScopeVariables lets
    // us catch this error.
    EXPECT_FALSE(p->EmitModuleScopeVariables()) << p->error();
    EXPECT_THAT(p->error(), HasSubstr("invalid SPIR-V storage class 7 for module scope "
                                      "variable: %52 = OpVariable %3 Function"));
}

TEST_F(SpvModuleScopeVarParserTest, BadPointerType) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
    %float = OpTypeFloat 32
    %fn_ty = OpTypeFunction %float
    %3 = OpTypePointer Private %fn_ty
    %52 = OpVariable %3 Private
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());
    // Normally we should run ASTParser::RegisterTypes before emitting
    // variables. But defensive coding in EmitModuleScopeVariables lets
    // us catch this error.
    EXPECT_FALSE(p->EmitModuleScopeVariables());
    EXPECT_THAT(p->error(), HasSubstr("internal error: failed to register Tint "
                                      "AST type for SPIR-V type with ID: 3"));
}

TEST_F(SpvModuleScopeVarParserTest, NonPointerType) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
    %float = OpTypeFloat 32
    %5 = OpTypeFunction %float
    %3 = OpTypePointer Private %5
    %52 = OpVariable %float Private
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());
    EXPECT_FALSE(p->RegisterTypes());
    EXPECT_THAT(p->error(), HasSubstr("SPIR-V pointer type with ID 3 has invalid pointee type 5"));
}

TEST_F(SpvModuleScopeVarParserTest, AnonWorkgroupVar) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
    %float = OpTypeFloat 32
    %ptr = OpTypePointer Workgroup %float
    %52 = OpVariable %ptr Workgroup
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
  )" + MainBody()));

    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<workgroup> x_52 : f32;"));
}

TEST_F(SpvModuleScopeVarParserTest, NamedWorkgroupVar) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
    OpName %52 "the_counter"
    %float = OpTypeFloat 32
    %ptr = OpTypePointer Workgroup %float
    %52 = OpVariable %ptr Workgroup
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
  )" + MainBody()));

    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<workgroup> the_counter : f32;"));
}

TEST_F(SpvModuleScopeVarParserTest, PrivateVar) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
    OpName %52 "my_own_private_idaho"
    %float = OpTypeFloat 32
    %ptr = OpTypePointer Private %float
    %52 = OpVariable %ptr Private
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
  )" + MainBody()));

    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> my_own_private_idaho : f32;"));
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinVertexIndex) {
    // This is the simple case for the vertex_index builtin,
    // where the SPIR-V uses the same store type as in WGSL.
    // See later for tests where the SPIR-V store type is signed
    // integer, as in GLSL.
    auto p = parser(test::Assemble(Preamble() + R"(
    OpEntryPoint Vertex %main "main" %52 %position
    OpName %position "position"
    OpDecorate %position BuiltIn Position
    OpDecorate %52 BuiltIn VertexIndex
    %uint = OpTypeInt 32 0
    %ptr = OpTypePointer Input %uint
    %52 = OpVariable %ptr Input
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %posty = OpTypePointer Output %v4float
    %position = OpVariable %posty Output
  )" + MainBody()));

    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_52 : u32;"));
}

std::string PerVertexPreamble() {
    return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1

    OpDecorate %10 Block
    OpMemberDecorate %10 0 BuiltIn Position
    OpMemberDecorate %10 1 BuiltIn PointSize
    OpMemberDecorate %10 2 BuiltIn ClipDistance
    OpMemberDecorate %10 3 BuiltIn CullDistance
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %12 = OpTypeVector %float 4
    %uint = OpTypeInt 32 0
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %arr = OpTypeArray %float %uint_1
    %10 = OpTypeStruct %12 %float %arr %arr
    %11 = OpTypePointer Output %10
    %1 = OpVariable %11 Output
)";
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPosition_StoreWholeStruct_NotSupported) {
    // Glslang does not generate this code pattern.
    const std::string assembly = PerVertexPreamble() + R"(
  %nil = OpConstantNull %10 ; the whole struct

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  OpStore %1 %nil  ; store the whole struct
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->BuildAndParseInternalModule()) << assembly;
    EXPECT_THAT(p->error(), Eq("storing to the whole per-vertex structure is not "
                               "supported: OpStore %1 %13"))
        << p->error();
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPosition_IntermediateWholeStruct_NotSupported) {
    const std::string assembly = PerVertexPreamble() + R"(
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %1000 = OpUndef %10
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->BuildAndParseInternalModule()) << assembly;
    EXPECT_THAT(p->error(), Eq("operations producing a per-vertex structure are "
                               "not supported: %1000 = OpUndef %10"))
        << p->error();
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPosition_IntermediatePtrWholeStruct_NotSupported) {
    const std::string assembly = PerVertexPreamble() + R"(
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %1000 = OpCopyObject %11 %1
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->BuildAndParseInternalModule());
    EXPECT_THAT(p->error(), Eq("operations producing a pointer to a per-vertex structure are "
                               "not supported: %1000 = OpCopyObject %11 %1"))
        << p->error();
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPosition_StorePosition) {
    const std::string assembly = PerVertexPreamble() + R"(
  %ptr_v4float = OpTypePointer Output %12
  %nil = OpConstantNull %12

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr_v4float %1 %uint_0 ; address of the Position member
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("gl_Position = vec4f();")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPosition_StorePosition_PerVertexStructOutOfOrderDecl) {
    const std::string assembly = R"(
  OpCapability Shader
  OpMemoryModel Logical Simple
  OpEntryPoint Vertex %main "main" %1

 ;  scramble the member indices
  OpDecorate %10 Block
  OpMemberDecorate %10 0 BuiltIn ClipDistance
  OpMemberDecorate %10 1 BuiltIn CullDistance
  OpMemberDecorate %10 2 BuiltIn Position
  OpMemberDecorate %10 3 BuiltIn PointSize
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  %float = OpTypeFloat 32
  %12 = OpTypeVector %float 4
  %uint = OpTypeInt 32 0
  %uint_0 = OpConstant %uint 0
  %uint_1 = OpConstant %uint 1
  %uint_2 = OpConstant %uint 2
  %arr = OpTypeArray %float %uint_1
  %10 = OpTypeStruct %arr %arr %12 %float
  %11 = OpTypePointer Output %10
  %1 = OpVariable %11 Output

  %ptr_v4float = OpTypePointer Output %12
  %nil = OpConstantNull %12

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr_v4float %1 %uint_2 ; address of the Position member
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("gl_Position = vec4f();")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPosition_StorePositionMember_OneAccessChain) {
    const std::string assembly = PerVertexPreamble() + R"(
  %ptr_float = OpTypePointer Output %float
  %nil = OpConstantNull %float

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr_float %1 %uint_0 %uint_1 ; address of the Position.y member
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("gl_Position.y = 0.0f;")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPosition_StorePositionMember_TwoAccessChain) {
    // The algorithm is smart enough to collapse it down.
    const std::string assembly = PerVertexPreamble() + R"(
  %ptr = OpTypePointer Output %12
  %ptr_float = OpTypePointer Output %float
  %nil = OpConstantNull %float

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr %1 %uint_0 ; address of the Position member
  %101 = OpAccessChain %ptr_float %100 %uint_1 ; address of the Position.y member
  OpStore %101 %nil
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("gl_Position.y = 0.0f;")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPointSize_Write1_IsErased) {
    const std::string assembly = PerVertexPreamble() + R"(
  %ptr = OpTypePointer Output %float
  %one = OpConstant %float 1.0

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr %1 %uint_1 ; address of the PointSize member
  OpStore %100 %one
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_EQ(module_str, R"(var<private> gl_Position : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @builtin(position)
  gl_Position : vec4f,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(gl_Position);
}
)") << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPointSize_WriteNon1_IsError) {
    const std::string assembly = PerVertexPreamble() + R"(
  %ptr = OpTypePointer Output %float
  %999 = OpConstant %float 2.0

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr %1 %uint_1 ; address of the PointSize member
  OpStore %100 %999
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->BuildAndParseInternalModule());
    EXPECT_THAT(p->error(), HasSubstr("cannot store a value other than constant 1.0 to "
                                      "PointSize builtin: OpStore %100 %999"));
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPointSize_ReadReplaced) {
    const std::string assembly = PerVertexPreamble() + R"(
  %ptr = OpTypePointer Output %float
  %nil = OpConstantNull %12
  %private_ptr = OpTypePointer Private %float
  %900 = OpVariable %private_ptr Private

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr %1 %uint_1 ; address of the PointSize member
  %99 = OpLoad %float %100
  OpStore %900 %99
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_EQ(module_str, R"(var<private> x_900 : f32;

var<private> gl_Position : vec4f;

fn main_1() {
  x_900 = 1.0f;
  return;
}

struct main_out {
  @builtin(position)
  gl_Position : vec4f,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(gl_Position);
}
)") << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPointSize_WriteViaCopyObjectPriorAccess_Unsupported) {
    const std::string assembly = PerVertexPreamble() + R"(
  %ptr = OpTypePointer Output %float
  %nil = OpConstantNull %12

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %20 = OpCopyObject %11 %1
  %100 = OpAccessChain %20 %1 %uint_1 ; address of the PointSize member
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->BuildAndParseInternalModule()) << p->error();
    EXPECT_THAT(p->error(),
                HasSubstr("operations producing a pointer to a per-vertex structure are "
                          "not supported: %20 = OpCopyObject %11 %1"));
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPointSize_WriteViaCopyObjectPostAccessChainErased) {
    const std::string assembly = PerVertexPreamble() + R"(
  %ptr = OpTypePointer Output %float
  %one = OpConstant %float 1.0

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr %1 %uint_1 ; address of the PointSize member
  %101 = OpCopyObject %ptr %100
  OpStore %101 %one
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_EQ(module_str, R"(var<private> gl_Position : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @builtin(position)
  gl_Position : vec4f,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(gl_Position);
}
)") << module_str;
}

std::string LoosePointSizePreamble(std::string stage = "Vertex") {
    return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint )" +
           stage + R"( %500 "main" %1
)" + (stage == "Vertex" ? " %2 " : "") +
           +(stage == "Fragment" ? "OpExecutionMode %500 OriginUpperLeft" : "") +
           +(stage == "Vertex" ? " OpDecorate %2 BuiltIn Position " : "") +
           R"(
    OpDecorate %1 BuiltIn PointSize
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %uint = OpTypeInt 32 0
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %11 = OpTypePointer Output %float
    %1 = OpVariable %11 Output
    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output
)";
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPointSize_Loose_Write1_IsErased) {
    const std::string assembly = LoosePointSizePreamble() + R"(
  %ptr = OpTypePointer Output %float
  %one = OpConstant %float 1.0

  %500 = OpFunction %void None %voidfn
  %entry = OpLabel
  OpStore %1 %one
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_EQ(module_str, R"(var<private> x_2 : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @builtin(position)
  x_2_1 : vec4f,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(x_2);
}
)") << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPointSize_Loose_WriteNon1_IsError) {
    const std::string assembly = LoosePointSizePreamble() + R"(
  %ptr = OpTypePointer Output %float
  %999 = OpConstant %float 2.0

  %500 = OpFunction %void None %voidfn
  %entry = OpLabel
  OpStore %1 %999
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->BuildAndParseInternalModule());
    EXPECT_THAT(p->error(), HasSubstr("cannot store a value other than constant 1.0 to "
                                      "PointSize builtin: OpStore %1 %999"));
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPointSize_Loose_ReadReplaced_Vertex) {
    const std::string assembly = LoosePointSizePreamble() + R"(
  %ptr = OpTypePointer Private %float
  %900 = OpVariable %ptr Private

  %500 = OpFunction %void None %voidfn
  %entry = OpLabel
  %99 = OpLoad %float %1
  OpStore %900 %99
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_EQ(module_str, R"(var<private> x_2 : vec4f;

var<private> x_900 : f32;

fn main_1() {
  x_900 = 1.0f;
  return;
}

struct main_out {
  @builtin(position)
  x_2_1 : vec4f,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(x_2);
}
)") << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPointSize_Loose_ReadReplaced_Fragment) {
    const std::string assembly = LoosePointSizePreamble("Fragment") + R"(
  %ptr = OpTypePointer Private %float
  %900 = OpVariable %ptr Private

  %500 = OpFunction %void None %voidfn
  %entry = OpLabel
  %99 = OpLoad %float %1
  OpStore %900 %99
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    // This example is invalid because you PointSize is not valid in Vulkan
    // Fragment shaders.
    EXPECT_FALSE(p->Parse());
    EXPECT_FALSE(p->success());
    EXPECT_THAT(p->error(), HasSubstr("VUID-PointSize-PointSize-04314"));
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPointSize_Loose_WriteViaCopyObjectPriorAccess_Erased) {
    const std::string assembly = LoosePointSizePreamble() + R"(
  %one = OpConstant %float 1.0

  %500 = OpFunction %void None %voidfn
  %entry = OpLabel
  %20 = OpCopyObject %11 %1
  %100 = OpAccessChain %11 %20
  OpStore %100 %one
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_EQ(module_str, R"(var<private> x_2 : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @builtin(position)
  x_2_1 : vec4f,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(x_2);
}
)") << module_str;
}

TEST_F(SpvModuleScopeVarParserTest,
       BuiltinPointSize_Loose_WriteViaCopyObjectPostAccessChainErased) {
    const std::string assembly = LoosePointSizePreamble() + R"(
  %one = OpConstant %float 1.0

  %500 = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %11 %1
  %101 = OpCopyObject %11 %100
  OpStore %101 %one
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule()) << p->error();
    EXPECT_TRUE(p->error().empty()) << p->error();
    const auto module_str = test::ToString(p->program());
    EXPECT_EQ(module_str, R"(var<private> x_2 : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @builtin(position)
  x_2_1 : vec4f,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(x_2);
}
)") << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinClipDistance_NotSupported) {
    const std::string assembly = PerVertexPreamble() + R"(
  %ptr_float = OpTypePointer Output %float
  %nil = OpConstantNull %float
  %uint_2 = OpConstant %uint 2

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
; address of the first entry in ClipDistance
  %100 = OpAccessChain %ptr_float %1 %uint_2 %uint_0
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->BuildAndParseInternalModule());
    EXPECT_EQ(p->error(),
              "accessing per-vertex member 2 is not supported. Only Position is "
              "supported, and PointSize is ignored");
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinCullDistance_NotSupported) {
    const std::string assembly = PerVertexPreamble() + R"(
  %ptr_float = OpTypePointer Output %float
  %nil = OpConstantNull %float
  %uint_3 = OpConstant %uint 3

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
; address of the first entry in CullDistance
  %100 = OpAccessChain %ptr_float %1 %uint_3 %uint_0
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->BuildAndParseInternalModule());
    EXPECT_EQ(p->error(),
              "accessing per-vertex member 3 is not supported. Only Position is "
              "supported, and PointSize is ignored");
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPerVertex_MemberIndex_NotConstant) {
    const std::string assembly = PerVertexPreamble() + R"(
  %ptr_float = OpTypePointer Output %float
  %nil = OpConstantNull %float

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %sum = OpIAdd %uint %uint_0 %uint_0
  %100 = OpAccessChain %ptr_float %1 %sum
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->BuildAndParseInternalModule());
    EXPECT_THAT(p->error(), Eq("first index of access chain into per-vertex structure is not "
                               "a constant: %100 = OpAccessChain %13 %1 %16"));
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPerVertex_MemberIndex_NotConstantInteger) {
    const std::string assembly = PerVertexPreamble() + R"(
  %ptr_float = OpTypePointer Output %float
  %nil = OpConstantNull %float

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
; nil is bad here!
  %100 = OpAccessChain %ptr_float %1 %nil
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->BuildAndParseInternalModule());
    EXPECT_THAT(p->error(), Eq("first index of access chain into per-vertex structure is not "
                               "a constant integer: %100 = OpAccessChain %13 %1 %14"));
}

TEST_F(SpvModuleScopeVarParserTest, ScalarInitializers) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %1 = OpVariable %ptr_bool Private %true
     %2 = OpVariable %ptr_bool Private %false
     %3 = OpVariable %ptr_int Private %int_m1
     %4 = OpVariable %ptr_uint Private %uint_1
     %5 = OpVariable %ptr_float Private %float_1p5
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr(R"(var<private> x_1 = true;

var<private> x_2 = false;

var<private> x_3 = -1i;

var<private> x_4 = 1u;

var<private> x_5 = 1.5f;
)"));
}

TEST_F(SpvModuleScopeVarParserTest, ScalarNullInitializers) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %null_bool = OpConstantNull %bool
     %null_int = OpConstantNull %int
     %null_uint = OpConstantNull %uint
     %null_float = OpConstantNull %float

     %1 = OpVariable %ptr_bool Private %null_bool
     %2 = OpVariable %ptr_int Private %null_int
     %3 = OpVariable %ptr_uint Private %null_uint
     %4 = OpVariable %ptr_float Private %null_float
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr(R"(var<private> x_1 = false;

var<private> x_2 = 0i;

var<private> x_3 = 0u;

var<private> x_4 = 0.0f;
)"));
}

TEST_F(SpvModuleScopeVarParserTest, ScalarUndefInitializers) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %undef_bool = OpUndef %bool
     %undef_int = OpUndef %int
     %undef_uint = OpUndef %uint
     %undef_float = OpUndef %float

     %1 = OpVariable %ptr_bool Private %undef_bool
     %2 = OpVariable %ptr_int Private %undef_int
     %3 = OpVariable %ptr_uint Private %undef_uint
     %4 = OpVariable %ptr_float Private %undef_float
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr(R"(var<private> x_1 = false;

var<private> x_2 = 0i;

var<private> x_3 = 0u;

var<private> x_4 = 0.0f;
)"));

    // This example module emits ok, but is not valid SPIR-V in the first place.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvModuleScopeVarParserTest, VectorInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2float
     %two = OpConstant %float 2.0
     %const = OpConstantComposite %v2float %float_1p5 %two
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = vec2f(1.5f, 2.0f);"));
}

TEST_F(SpvModuleScopeVarParserTest, VectorBoolNullInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2bool
     %const = OpConstantNull %v2bool
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = vec2<bool>();"));
}

TEST_F(SpvModuleScopeVarParserTest, VectorBoolUndefInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2bool
     %const = OpUndef %v2bool
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = vec2<bool>();"));

    // This example module emits ok, but is not valid SPIR-V in the first place.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvModuleScopeVarParserTest, VectorUintNullInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2uint
     %const = OpConstantNull %v2uint
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = vec2u();"));
}

TEST_F(SpvModuleScopeVarParserTest, VectorUintUndefInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2uint
     %const = OpUndef %v2uint
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = vec2u();"));

    // This example module emits ok, but is not valid SPIR-V in the first place.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvModuleScopeVarParserTest, VectorIntNullInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2int
     %const = OpConstantNull %v2int
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = vec2i();"));
}

TEST_F(SpvModuleScopeVarParserTest, VectorIntUndefInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2int
     %const = OpUndef %v2int
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = vec2i();"));

    // This example module emits ok, but is not valid SPIR-V in the first place.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvModuleScopeVarParserTest, VectorFloatNullInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2float
     %const = OpConstantNull %v2float
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = vec2f();"));
}

TEST_F(SpvModuleScopeVarParserTest, VectorFloatUndefInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2float
     %const = OpUndef %v2float
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = vec2f();"));

    // This example module emits ok, but is not valid SPIR-V in the first place.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvModuleScopeVarParserTest, MatrixInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %m3v2float
     %two = OpConstant %float 2.0
     %three = OpConstant %float 3.0
     %four = OpConstant %float 4.0
     %v0 = OpConstantComposite %v2float %float_1p5 %two
     %v1 = OpConstantComposite %v2float %two %three
     %v2 = OpConstantComposite %v2float %three %four
     %const = OpConstantComposite %m3v2float %v0 %v1 %v2
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = mat3x2f("
                                      "vec2f(1.5f, 2.0f), "
                                      "vec2f(2.0f, 3.0f), "
                                      "vec2f(3.0f, 4.0f));"));
}

TEST_F(SpvModuleScopeVarParserTest, MatrixNullInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %m3v2float
     %const = OpConstantNull %m3v2float
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = mat3x2f();"));
}

TEST_F(SpvModuleScopeVarParserTest, MatrixUndefInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %m3v2float
     %const = OpUndef %m3v2float
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = mat3x2f();"));

    // This example module emits ok, but is not valid SPIR-V in the first place.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvModuleScopeVarParserTest, ArrayInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %arr2uint
     %two = OpConstant %uint 2
     %const = OpConstantComposite %arr2uint %uint_1 %two
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = array<u32, 2u>(1u, 2u);"));
}

TEST_F(SpvModuleScopeVarParserTest, ArrayNullInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %arr2uint
     %const = OpConstantNull %arr2uint
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = array<u32, 2u>();"));
}

TEST_F(SpvModuleScopeVarParserTest, ArrayUndefInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %arr2uint
     %const = OpUndef %arr2uint
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = array<u32, 2u>();"));

    // This example module emits ok, but is not valid SPIR-V in the first place.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvModuleScopeVarParserTest, StructInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + StructTypes() + R"(
     %ptr = OpTypePointer Private %strct
     %two = OpConstant %uint 2
     %arrconst = OpConstantComposite %arr2uint %uint_1 %two
     %const = OpConstantComposite %strct %uint_1 %float_1p5 %arrconst
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = S(1u, 1.5f, array<u32, 2u>(1u, 2u));"))
        << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, StructNullInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + StructTypes() + R"(
     %ptr = OpTypePointer Private %strct
     %const = OpConstantNull %strct
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = S(0u, 0.0f, array<u32, 2u>());"))
        << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, StructUndefInitializer) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + StructTypes() + R"(
     %ptr = OpTypePointer Private %strct
     %const = OpUndef %strct
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());

    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("var<private> x_200 = S(0u, 0.0f, array<u32, 2u>());"))
        << module_str;

    // This example module emits ok, but is not valid SPIR-V in the first place.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvModuleScopeVarParserTest, DescriptorGroupDecoration_Valid) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + CommonLayout() + R"(
     OpDecorate %1 DescriptorSet 3
     OpDecorate %1 Binding 9 ; Required to pass WGSL validation
     OpDecorate %strct Block
)" + CommonTypes() + StructTypes() +
                                   R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %1 = OpVariable %ptr_sb_strct StorageBuffer
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("@group(3) @binding(9) var<storage, read_write> x_1 : S;"))
        << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, BindingDecoration_Valid) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %1 DescriptorSet 0 ; WGSL validation requires this already
     OpDecorate %1 Binding 3
     OpDecorate %strct Block
)" + CommonLayout() + CommonTypes() +
                                   StructTypes() +
                                   R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %1 = OpVariable %ptr_sb_strct StorageBuffer
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("@group(0) @binding(3) var<storage, read_write> x_1 : S;"))
        << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, StructMember_NonReadableDecoration_Dropped) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %1 DescriptorSet 0
     OpDecorate %1 Binding 0
     OpDecorate %strct Block
     OpMemberDecorate %strct 0 NonReadable
)" + CommonLayout() + CommonTypes() +
                                   StructTypes() + R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %1 = OpVariable %ptr_sb_strct StorageBuffer
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr(R"(alias Arr = @stride(4) array<u32, 2u>;

struct S {
  /* @offset(0) */
  field0 : u32,
  /* @offset(4) */
  field1 : f32,
  /* @offset(8) */
  field2 : Arr,
}

@group(0) @binding(0) var<storage, read_write> x_1 : S;
)")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ColMajorDecoration_Dropped) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %myvar DescriptorSet 0
     OpDecorate %myvar Binding 0
     OpDecorate %s Block
     OpMemberDecorate %s 0 ColMajor
     OpMemberDecorate %s 0 Offset 0
     OpMemberDecorate %s 0 MatrixStride 8
     %float = OpTypeFloat 32
     %v2float = OpTypeVector %float 2
     %m3v2float = OpTypeMatrix %v2float 3

     %s = OpTypeStruct %m3v2float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %myvar = OpVariable %ptr_sb_s StorageBuffer
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr(R"(struct S {
  /* @offset(0) */
  @stride(8) @internal(disable_validation__ignore_stride)
  field0 : mat3x2f,
}

@group(0) @binding(0) var<storage, read_write> myvar : S;
)")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, MatrixStrideDecoration_Natural_ColMajor) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %myvar DescriptorSet 0
     OpDecorate %myvar Binding 0
     OpDecorate %s Block
     OpMemberDecorate %s 0 MatrixStride 8
     OpMemberDecorate %s 0 Offset 0
     OpMemberDecorate %s 0 ColMajor
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %v2float = OpTypeVector %float 2
     %m3v2float = OpTypeMatrix %v2float 3

     %s = OpTypeStruct %m3v2float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %myvar = OpVariable %ptr_sb_s StorageBuffer
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr(R"(struct S {
  /* @offset(0) */
  @stride(8) @internal(disable_validation__ignore_stride)
  field0 : mat3x2f,
}

@group(0) @binding(0) var<storage, read_write> myvar : S;
)")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, MatrixStrideDecoration_Natural_RowMajor) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %myvar DescriptorSet 0
     OpDecorate %myvar Binding 0
     OpDecorate %s Block
     OpMemberDecorate %s 0 MatrixStride 8
     OpMemberDecorate %s 0 Offset 0
     OpMemberDecorate %s 0 RowMajor
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %v3float = OpTypeVector %float 3
     %m2v3float = OpTypeMatrix %v3float 2

     %s = OpTypeStruct %m2v3float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %myvar = OpVariable %ptr_sb_s StorageBuffer
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr(R"(struct S {
  /* @offset(0) */
  @stride(8) @internal(disable_validation__ignore_stride) @row_major
  field0 : mat2x3f,
}

@group(0) @binding(0) var<storage, read_write> myvar : S;
)")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, MatrixStrideDecoration) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %myvar DescriptorSet 0
     OpDecorate %myvar Binding 0
     OpDecorate %s Block
     OpMemberDecorate %s 0 MatrixStride 64
     OpMemberDecorate %s 0 Offset 0
     OpMemberDecorate %s 0 ColMajor
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %v2float = OpTypeVector %float 2
     %m3v2float = OpTypeMatrix %v2float 3

     %s = OpTypeStruct %m3v2float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %myvar = OpVariable %ptr_sb_s StorageBuffer
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr(R"(struct S {
  /* @offset(0) */
  @stride(64) @internal(disable_validation__ignore_stride)
  field0 : mat3x2f,
}

@group(0) @binding(0) var<storage, read_write> myvar : S;
)")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, RowMajorDecoration) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %s Block
     OpMemberDecorate %s 0 RowMajor
     OpMemberDecorate %s 0 Offset 0
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %v2float = OpTypeVector %float 2
     %m3v2float = OpTypeMatrix %v2float 3

     %s = OpTypeStruct %m3v2float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %myvar = OpVariable %ptr_sb_s StorageBuffer
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr(R"(struct S {
  /* @offset(0) */
  @row_major
  field0 : mat3x2f,
}

var<storage, read_write> myvar : S;
)")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, StorageBuffer_NonWritable_Var) {
    // Variable should have access(read)
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %s Block
     OpDecorate %1 DescriptorSet 0
     OpDecorate %1 Binding 0
     OpDecorate %1 NonWritable
     OpMemberDecorate %s 0 Offset 0
     OpMemberDecorate %s 1 Offset 4
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32

     %s = OpTypeStruct %float %float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %1 = OpVariable %ptr_sb_s StorageBuffer
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr(R"(struct S {
  /* @offset(0) */
  field0 : f32,
  /* @offset(4) */
  field1 : f32,
}

@group(0) @binding(0) var<storage, read> x_1 : S;
)")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, StorageBuffer_NonWritable_AllMembers) {
    // Variable should have access(read)
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %s Block
     OpDecorate %1 DescriptorSet 0
     OpDecorate %1 Binding 0
     OpMemberDecorate %s 0 NonWritable
     OpMemberDecorate %s 1 NonWritable
     OpMemberDecorate %s 0 Offset 0
     OpMemberDecorate %s 1 Offset 4
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32

     %s = OpTypeStruct %float %float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %1 = OpVariable %ptr_sb_s StorageBuffer
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr(R"(struct S {
  /* @offset(0) */
  field0 : f32,
  /* @offset(4) */
  field1 : f32,
}

@group(0) @binding(0) var<storage, read> x_1 : S;
)")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, StorageBuffer_NonWritable_NotAllMembers) {
    // Variable should have access(read_write)
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %1 DescriptorSet 0
     OpDecorate %1 Binding 0
     OpDecorate %s Block
     OpMemberDecorate %s 0 NonWritable
     OpMemberDecorate %s 0 Offset 0
     OpMemberDecorate %s 1 Offset 4
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32

     %s = OpTypeStruct %float %float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %1 = OpVariable %ptr_sb_s StorageBuffer
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr(R"(struct S {
  /* @offset(0) */
  field0 : f32,
  /* @offset(4) */
  field1 : f32,
}

@group(0) @binding(0) var<storage, read_write> x_1 : S;
)")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest,
       StorageBuffer_NonWritable_NotAllMembers_DuplicatedOnSameMember) {  // NOLINT
    // Variable should have access(read_write)
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %s Block
     OpDecorate %1 DescriptorSet 0
     OpDecorate %1 Binding 0
     OpMemberDecorate %s 0 NonWritable
     OpMemberDecorate %s 0 NonWritable ; same member. Don't double-count it
     OpMemberDecorate %s 0 Offset 0
     OpMemberDecorate %s 1 Offset 4
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32

     %s = OpTypeStruct %float %float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %1 = OpVariable %ptr_sb_s StorageBuffer
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr(R"(struct S {
  /* @offset(0) */
  field0 : f32,
  /* @offset(4) */
  field1 : f32,
}

@group(0) @binding(0) var<storage, read_write> x_1 : S;
)")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ScalarSpecConstant_DeclareConst_Id_TooBig) {
    // Override IDs must be between 0 and 65535
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %1 SpecId 65536
     %bool = OpTypeBool
     %1 = OpSpecConstantTrue %bool
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
    EXPECT_FALSE(p->Parse());
    EXPECT_EQ(p->error(),
              "SpecId too large. WGSL override IDs must be between 0 and 65535: "
              "ID %1 has SpecId 65536");
}

TEST_F(SpvModuleScopeVarParserTest, ScalarSpecConstant_DeclareConst_Id_MaxValid) {
    // Override IDs must be between 0 and 65535
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %1 SpecId 65535
     %bool = OpTypeBool
     %1 = OpSpecConstantTrue %bool
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
    EXPECT_TRUE(p->Parse());
    EXPECT_EQ(p->error(), "");
}

TEST_F(SpvModuleScopeVarParserTest, ScalarSpecConstant_DeclareConst_True) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %c "myconst"
     OpDecorate %c SpecId 12
     %bool = OpTypeBool
     %c = OpSpecConstantTrue %bool
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("@id(12) override myconst : bool = true;")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ScalarSpecConstant_DeclareConst_False) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %c "myconst"
     OpDecorate %c SpecId 12
     %bool = OpTypeBool
     %c = OpSpecConstantFalse %bool
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("@id(12) override myconst : bool = false;")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ScalarSpecConstant_DeclareConst_U32) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %c "myconst"
     OpDecorate %c SpecId 12
     %uint = OpTypeInt 32 0
     %c = OpSpecConstant %uint 42
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("@id(12) override myconst : u32 = 42u;")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ScalarSpecConstant_DeclareConst_I32) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %c "myconst"
     OpDecorate %c SpecId 12
     %int = OpTypeInt 32 1
     %c = OpSpecConstant %int 42
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("@id(12) override myconst : i32 = 42i;")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ScalarSpecConstant_DeclareConst_F32) {
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %c "myconst"
     OpDecorate %c SpecId 12
     %float = OpTypeFloat 32
     %c = OpSpecConstant %float 2.5
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("@id(12) override myconst : f32 = 2.5f;")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ScalarSpecConstant_DeclareConst_F32_WithoutSpecId) {
    // When we don't have a spec ID, declare an undecorated module-scope constant.
    auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %c "myconst"
     %float = OpTypeFloat 32
     %c = OpSpecConstant %float 2.5
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    EXPECT_THAT(module_str, HasSubstr("override myconst : f32 = 2.5f;")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ScalarSpecConstant_UsedInFunction) {
    const auto assembly = Preamble() + FragMain() + R"(
     OpName %c "myconst"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %c = OpSpecConstant %float 2.5
     %floatfn = OpTypeFunction %float
     %100 = OpFunction %float None %floatfn
     %entry = OpLabel
     %1 = OpFAdd %float %c %c
     OpReturnValue %1
     OpFunctionEnd
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    EXPECT_TRUE(p->error().empty());

    Program program = p->program();
    const auto got = test::ToString(program, fe.ast_body());

    EXPECT_THAT(got, HasSubstr("return (myconst + myconst);")) << got;
}

// Returns the start of a shader for testing SampleId,
// parameterized by store type of %int or %uint
std::string SampleIdPreamble(std::string store_type) {
    return R"(
    OpCapability Shader
    OpCapability SampleRateShading
    OpMemoryModel Logical Simple
    OpEntryPoint Fragment %main "main" %1
    OpExecutionMode %main OriginUpperLeft
    OpDecorate %1 BuiltIn SampleId
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
    %ptr_ty = OpTypePointer Input )" +
           store_type + R"(
    %1 = OpVariable %ptr_ty Input
)";
}

TEST_F(SpvModuleScopeVarParserTest, SampleId_I32_Load_Direct) {
    const std::string assembly = SampleIdPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpLoad %int %1
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected =
        R"(var<private> x_1 : i32;

fn main_1() {
  let x_2 = x_1;
  return;
}

@fragment
fn main(@builtin(sample_index) x_1_param : u32) {
  x_1 = bitcast<i32>(x_1_param);
  main_1();
}
)";
    EXPECT_EQ(module_str, expected) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleId_I32_Load_CopyObject) {
    const std::string assembly = SampleIdPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpCopyObject %ptr_ty %1
    %2 = OpLoad %int %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected =
        R"(Module{
  Variable{
    x_1
    private
    undefined
    __i32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_11
        none
        undefined
        __ptr_private__i32
        {
          UnaryOp[not set]{
            address-of
            Identifier[not set]{x_1}
          }
        }
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        __i32
        {
          UnaryOp[not set]{
            indirection
            Identifier[not set]{x_14}
          }
        }
      }
    }
    Return{}
  }
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{sample_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Bitcast[not set]<__i32>{
        Identifier[not set]{x_1_param}
      }
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
}

TEST_F(SpvModuleScopeVarParserTest, SampleId_I32_Load_AccessChain) {
    const std::string assembly = SampleIdPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpAccessChain %ptr_ty %1
    %2 = OpLoad %int %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : i32;

fn main_1() {
  let x_2 = x_1;
  return;
}

@fragment
fn main(@builtin(sample_index) x_1_param : u32) {
  x_1 = bitcast<i32>(x_1_param);
  main_1();
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, SampleId_I32_FunctParam) {
    const std::string assembly = SampleIdPreamble("%int") + R"(
    %helper_ty = OpTypeFunction %int %ptr_ty
    %helper = OpFunction %int None %helper_ty
    %param = OpFunctionParameter %ptr_ty
    %helper_entry = OpLabel
    %3 = OpLoad %int %param
    OpReturnValue %3
    OpFunctionEnd

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %result = OpFunctionCall %int %helper %1
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));

    // This example is invalid because you can't pass pointer-to-Input
    // as a function parameter.
    EXPECT_FALSE(p->Parse());
    EXPECT_FALSE(p->success());
    EXPECT_THAT(p->error(), HasSubstr("Invalid storage class for pointer operand '1"));
}

TEST_F(SpvModuleScopeVarParserTest, SampleId_U32_Load_Direct) {
    const std::string assembly = SampleIdPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpLoad %uint %1
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : u32;

fn main_1() {
  let x_2 = x_1;
  return;
}

@fragment
fn main(@builtin(sample_index) x_1_param : u32) {
  x_1 = x_1_param;
  main_1();
}
)";
    EXPECT_EQ(module_str, expected) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleId_U32_Load_CopyObject) {
    const std::string assembly = SampleIdPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpCopyObject %ptr_ty %1
    %2 = OpLoad %uint %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : u32;

fn main_1() {
  let x_11 = &(x_1);
  let x_2 = *(x_11);
  return;
}

@fragment
fn main(@builtin(sample_index) x_1_param : u32) {
  x_1 = x_1_param;
  main_1();
}
)";
    EXPECT_EQ(module_str, expected) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleId_U32_Load_AccessChain) {
    const std::string assembly = SampleIdPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpAccessChain %ptr_ty %1
    %2 = OpLoad %uint %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : u32;

fn main_1() {
  let x_2 = x_1;
  return;
}

@fragment
fn main(@builtin(sample_index) x_1_param : u32) {
  x_1 = x_1_param;
  main_1();
}
)";
    EXPECT_EQ(module_str, expected) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleId_U32_FunctParam) {
    const std::string assembly = SampleIdPreamble("%uint") + R"(
    %helper_ty = OpTypeFunction %uint %ptr_ty
    %helper = OpFunction %uint None %helper_ty
    %param = OpFunctionParameter %ptr_ty
    %helper_entry = OpLabel
    %3 = OpLoad %uint %param
    OpReturnValue %3
    OpFunctionEnd

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %result = OpFunctionCall %uint %helper %1
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    // This example is invalid because you can't pass pointer-to-Input
    // as a function parameter.
    EXPECT_FALSE(p->Parse());
    EXPECT_THAT(p->error(), HasSubstr("Invalid storage class for pointer operand '1"));
}

// Returns the start of a shader for testing SampleMask
// parameterized by store type.
std::string SampleMaskPreamble(std::string store_type, uint32_t stride = 0u) {
    return std::string(R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Fragment %main "main" %1
    OpExecutionMode %main OriginUpperLeft
    OpDecorate %1 BuiltIn SampleMask
)") +
           (stride > 0u ? R"(
    OpDecorate %uarr1 ArrayStride 4
    OpDecorate %uarr2 ArrayStride 4
    OpDecorate %iarr1 ArrayStride 4
    OpDecorate %iarr2 ArrayStride 4
)"
                        : "") +
           R"(
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
    %int_12 = OpConstant %int 12
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %uint_2 = OpConstant %uint 2
    %uarr1 = OpTypeArray %uint %uint_1
    %uarr2 = OpTypeArray %uint %uint_2
    %iarr1 = OpTypeArray %int %uint_1
    %iarr2 = OpTypeArray %int %uint_2
    %iptr_in_ty = OpTypePointer Input %int
    %uptr_in_ty = OpTypePointer Input %uint
    %iptr_out_ty = OpTypePointer Output %int
    %uptr_out_ty = OpTypePointer Output %uint
    %in_ty = OpTypePointer Input )" +
           store_type + R"(
    %out_ty = OpTypePointer Output )" +
           store_type + R"(
)";
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_In_ArraySize2_Error) {
    const std::string assembly = SampleMaskPreamble("%uarr2") + R"(
    %1 = OpVariable %in_ty Input

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_in_ty %1 %uint_0
    %3 = OpLoad %int %2
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_FALSE(p->BuildAndParseInternalModule());
    EXPECT_THAT(p->error(), HasSubstr("WGSL supports a sample mask of at most 32 bits. "
                                      "SampleMask must be an array of 1 element"))
        << p->error() << assembly;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_In_U32_Direct) {
    const std::string assembly = SampleMaskPreamble("%uarr1") + R"(
    %1 = OpVariable %in_ty Input

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_in_ty %1 %uint_0
    %3 = OpLoad %uint %2
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<u32, 1u>;

fn main_1() {
  let x_3 = x_1[0i];
  return;
}

@fragment
fn main(@builtin(sample_mask) x_1_param : u32) {
  x_1[0i] = x_1_param;
  main_1();
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_In_U32_CopyObject) {
    const std::string assembly = SampleMaskPreamble("%uarr1") + R"(
    %1 = OpVariable %in_ty Input

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_in_ty %1 %uint_0
    %3 = OpCopyObject %uptr_in_ty %2
    %4 = OpLoad %uint %3
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<u32, 1u>;

fn main_1() {
  let x_4 = x_1[0i];
  return;
}

@fragment
fn main(@builtin(sample_mask) x_1_param : u32) {
  x_1[0i] = x_1_param;
  main_1();
}
)";
    EXPECT_EQ(module_str, expected) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_In_U32_AccessChain) {
    const std::string assembly = SampleMaskPreamble("%uarr1") + R"(
    %1 = OpVariable %in_ty Input

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_in_ty %1 %uint_0
    %3 = OpAccessChain %uptr_in_ty %2
    %4 = OpLoad %uint %3
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<u32, 1u>;

fn main_1() {
  let x_4 = x_1[0i];
  return;
}

@fragment
fn main(@builtin(sample_mask) x_1_param : u32) {
  x_1[0i] = x_1_param;
  main_1();
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_In_I32_Direct) {
    const std::string assembly = SampleMaskPreamble("%iarr1") + R"(
    %1 = OpVariable %in_ty Input

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %iptr_in_ty %1 %uint_0
    %3 = OpLoad %int %2
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<i32, 1u>;

fn main_1() {
  let x_3 = x_1[0i];
  return;
}

@fragment
fn main(@builtin(sample_mask) x_1_param : u32) {
  x_1[0i] = bitcast<i32>(x_1_param);
  main_1();
}
)";
    EXPECT_EQ(module_str, expected) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_In_I32_CopyObject) {
    const std::string assembly = SampleMaskPreamble("%iarr1") + R"(
    %1 = OpVariable %in_ty Input

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %iptr_in_ty %1 %uint_0
    %3 = OpCopyObject %iptr_in_ty %2
    %4 = OpLoad %int %3
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<i32, 1u>;

fn main_1() {
  let x_4 = x_1[0i];
  return;
}

@fragment
fn main(@builtin(sample_mask) x_1_param : u32) {
  x_1[0i] = bitcast<i32>(x_1_param);
  main_1();
}
)";
    EXPECT_EQ(module_str, expected) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_In_I32_AccessChain) {
    const std::string assembly = SampleMaskPreamble("%iarr1") + R"(
    %1 = OpVariable %in_ty Input

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %iptr_in_ty %1 %uint_0
    %3 = OpAccessChain %iptr_in_ty %2
    %4 = OpLoad %int %3
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<i32, 1u>;

fn main_1() {
  let x_4 = x_1[0i];
  return;
}

@fragment
fn main(@builtin(sample_mask) x_1_param : u32) {
  x_1[0i] = bitcast<i32>(x_1_param);
  main_1();
}
)";
    EXPECT_EQ(module_str, expected) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_Out_ArraySize2_Error) {
    const std::string assembly = SampleMaskPreamble("%uarr2") + R"(
    %1 = OpVariable %out_ty Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_out_ty %1 %uint_0
    OpStore %2 %uint_0
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_FALSE(p->BuildAndParseInternalModule());
    EXPECT_THAT(p->error(), HasSubstr("WGSL supports a sample mask of at most 32 bits. "
                                      "SampleMask must be an array of 1 element"))
        << p->error() << assembly;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_Out_U32_Direct) {
    const std::string assembly = SampleMaskPreamble("%uarr1") + R"(
    %1 = OpVariable %out_ty Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_out_ty %1 %uint_0
    OpStore %2 %uint_0
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<u32, 1u>;

fn main_1() {
  x_1[0i] = 0u;
  return;
}

struct main_out {
  @builtin(sample_mask)
  x_1_1 : u32,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_1[0i]);
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_Out_U32_CopyObject) {
    const std::string assembly = SampleMaskPreamble("%uarr1") + R"(
    %1 = OpVariable %out_ty Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_out_ty %1 %uint_0
    %3 = OpCopyObject %uptr_out_ty %2
    OpStore %2 %uint_0
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<u32, 1u>;

fn main_1() {
  x_1[0i] = 0u;
  return;
}

struct main_out {
  @builtin(sample_mask)
  x_1_1 : u32,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_1[0i]);
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_Out_U32_AccessChain) {
    const std::string assembly = SampleMaskPreamble("%uarr1") + R"(
    %1 = OpVariable %out_ty Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_out_ty %1 %uint_0
    %3 = OpAccessChain %uptr_out_ty %2
    OpStore %2 %uint_0
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<u32, 1u>;

fn main_1() {
  x_1[0i] = 0u;
  return;
}

struct main_out {
  @builtin(sample_mask)
  x_1_1 : u32,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_1[0i]);
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_Out_I32_Direct) {
    const std::string assembly = SampleMaskPreamble("%iarr1") + R"(
    %1 = OpVariable %out_ty Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %iptr_out_ty %1 %uint_0
    OpStore %2 %int_12
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<i32, 1u>;

fn main_1() {
  x_1[0i] = 12i;
  return;
}

struct main_out {
  @builtin(sample_mask)
  x_1_1 : u32,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(bitcast<u32>(x_1[0i]));
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_Out_I32_CopyObject) {
    const std::string assembly = SampleMaskPreamble("%iarr1") + R"(
    %1 = OpVariable %out_ty Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %iptr_out_ty %1 %uint_0
    %3 = OpCopyObject %iptr_out_ty %2
    OpStore %2 %int_12
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<i32, 1u>;

fn main_1() {
  x_1[0i] = 12i;
  return;
}

struct main_out {
  @builtin(sample_mask)
  x_1_1 : u32,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(bitcast<u32>(x_1[0i]));
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_Out_I32_AccessChain) {
    const std::string assembly = SampleMaskPreamble("%iarr1") + R"(
    %1 = OpVariable %out_ty Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %iptr_out_ty %1 %uint_0
    %3 = OpAccessChain %iptr_out_ty %2
    OpStore %2 %int_12
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<i32, 1u>;

fn main_1() {
  x_1[0i] = 12i;
  return;
}

struct main_out {
  @builtin(sample_mask)
  x_1_1 : u32,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(bitcast<u32>(x_1[0i]));
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_In_WithStride) {
    const std::string assembly = SampleMaskPreamble("%uarr1", 4u) + R"(
    %1 = OpVariable %in_ty Input

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_in_ty %1 %uint_0
    %3 = OpLoad %uint %2
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(alias Arr = @stride(4) array<u32, 1u>;

alias Arr_1 = @stride(4) array<u32, 2u>;

alias Arr_2 = @stride(4) array<i32, 1u>;

alias Arr_3 = @stride(4) array<i32, 2u>;

var<private> x_1 : Arr;

fn main_1() {
  let x_3 = x_1[0i];
  return;
}

@fragment
fn main(@builtin(sample_mask) x_1_param : u32) {
  x_1[0i] = x_1_param;
  main_1();
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_Out_WithStride) {
    const std::string assembly = SampleMaskPreamble("%uarr1", 4u) + R"(
    %1 = OpVariable %out_ty Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_out_ty %1 %uint_0
    OpStore %2 %uint_0
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(alias Arr = @stride(4) array<u32, 1u>;

alias Arr_1 = @stride(4) array<u32, 2u>;

alias Arr_2 = @stride(4) array<i32, 1u>;

alias Arr_3 = @stride(4) array<i32, 2u>;

var<private> x_1 : Arr;

fn main_1() {
  x_1[0i] = 0u;
  return;
}

struct main_out {
  @builtin(sample_mask)
  x_1_1 : u32,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_1[0i]);
}
)";
    EXPECT_EQ(module_str, expected);
}

// Returns the start of a shader for testing VertexIndex,
// parameterized by store type of %int or %uint
std::string VertexIndexPreamble(std::string store_type) {
    return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %position %1
    OpDecorate %position BuiltIn Position
    OpDecorate %1 BuiltIn VertexIndex
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
    %ptr_ty = OpTypePointer Input )" +
           store_type + R"(
    %1 = OpVariable %ptr_ty Input
    %v4float = OpTypeVector %float 4
    %posty = OpTypePointer Output %v4float
    %position = OpVariable %posty Output
)";
}

TEST_F(SpvModuleScopeVarParserTest, VertexIndex_I32_Load_Direct) {
    const std::string assembly = VertexIndexPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpLoad %int %1
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : i32;

var<private> x_4 : vec4f;

fn main_1() {
  let x_2 = x_1;
  return;
}

struct main_out {
  @builtin(position)
  x_4_1 : vec4f,
}

@vertex
fn main(@builtin(vertex_index) x_1_param : u32) -> main_out {
  x_1 = bitcast<i32>(x_1_param);
  main_1();
  return main_out(x_4);
}
)";
    EXPECT_EQ(module_str, expected) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, VertexIndex_UsedTwice_DifferentConstructs) {
    // Test crbug.com/tint/1577
    // Builtin variables must not be hoisted. Before the fix, the reader
    // would see two uses of the variable in different constructs and try
    // to hoist it.  Only function-local definitions should be hoisted.
    const std::string assembly = VertexIndexPreamble("%uint") + R"(
    %bool = OpTypeBool
    %900 = OpConstantTrue %bool
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpLoad %uint %1   ; used in outer selection
    OpSelectionMerge %99 None
    OpBranchConditional %900 %30 %99

    %30 = OpLabel
    %3 = OpLoad %uint %1 ; used in inner selection
    OpSelectionMerge %40 None
    OpBranchConditional %900 %35 %40

    %35 = OpLabel
    OpBranch %40

    %40 = OpLabel
    OpBranch %99

    %99 = OpLabel
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : u32;

var<private> x_5 : vec4f;

fn main_1() {
  let x_2 = x_1;
  if (true) {
    let x_3 = x_1;
    if (true) {
    }
  }
  return;
}

struct main_out {
  @builtin(position)
  x_5_1 : vec4f,
}

@vertex
fn main(@builtin(vertex_index) x_1_param : u32) -> main_out {
  x_1 = x_1_param;
  main_1();
  return main_out(x_5);
}
)";
    EXPECT_EQ(module_str, expected) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, VertexIndex_I32_Load_CopyObject) {
    const std::string assembly = VertexIndexPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpCopyObject %ptr_ty %1
    %2 = OpLoad %int %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : i32;

var<private> x_4 : vec4f;

fn main_1() {
  let x_14 = &(x_1);
  let x_2 = *(x_14);
  return;
}

struct main_out {
  @builtin(position)
  x_4_1 : vec4f,
}

@vertex
fn main(@builtin(vertex_index) x_1_param : u32) -> main_out {
  x_1 = bitcast<i32>(x_1_param);
  main_1();
  return main_out(x_4);
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, VertexIndex_I32_Load_AccessChain) {
    const std::string assembly = VertexIndexPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpAccessChain %ptr_ty %1
    %2 = OpLoad %int %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : i32;

var<private> x_4 : vec4f;

fn main_1() {
  let x_2 = x_1;
  return;
}

struct main_out {
  @builtin(position)
  x_4_1 : vec4f,
}

@vertex
fn main(@builtin(vertex_index) x_1_param : u32) -> main_out {
  x_1 = bitcast<i32>(x_1_param);
  main_1();
  return main_out(x_4);
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, VertexIndex_U32_Load_Direct) {
    const std::string assembly = VertexIndexPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpLoad %uint %1
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : u32;

var<private> x_4 : vec4f;

fn main_1() {
  let x_2 = x_1;
  return;
}

struct main_out {
  @builtin(position)
  x_4_1 : vec4f,
}

@vertex
fn main(@builtin(vertex_index) x_1_param : u32) -> main_out {
  x_1 = x_1_param;
  main_1();
  return main_out(x_4);
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, VertexIndex_U32_Load_CopyObject) {
    const std::string assembly = VertexIndexPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpCopyObject %ptr_ty %1
    %2 = OpLoad %uint %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : u32;

var<private> x_4 : vec4f;

fn main_1() {
  let x_14 = &(x_1);
  let x_2 = *(x_14);
  return;
}

struct main_out {
  @builtin(position)
  x_4_1 : vec4f,
}

@vertex
fn main(@builtin(vertex_index) x_1_param : u32) -> main_out {
  x_1 = x_1_param;
  main_1();
  return main_out(x_4);
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, VertexIndex_U32_Load_AccessChain) {
    const std::string assembly = VertexIndexPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpAccessChain %ptr_ty %1
    %2 = OpLoad %uint %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : u32;

var<private> x_4 : vec4f;

fn main_1() {
  let x_2 = x_1;
  return;
}

struct main_out {
  @builtin(position)
  x_4_1 : vec4f,
}

@vertex
fn main(@builtin(vertex_index) x_1_param : u32) -> main_out {
  x_1 = x_1_param;
  main_1();
  return main_out(x_4);
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, VertexIndex_U32_FunctParam) {
    const std::string assembly = VertexIndexPreamble("%uint") + R"(
    %helper_ty = OpTypeFunction %uint %ptr_ty
    %helper = OpFunction %uint None %helper_ty
    %param = OpFunctionParameter %ptr_ty
    %helper_entry = OpLabel
    %3 = OpLoad %uint %param
    OpReturnValue %3
    OpFunctionEnd

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %result = OpFunctionCall %uint %helper %1
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));

    // This example is invalid because you can't pass pointer-to-Input
    // as a function parameter.
    EXPECT_FALSE(p->Parse());
    EXPECT_THAT(p->error(), HasSubstr("Invalid storage class for pointer operand '1"));
}

// Returns the start of a shader for testing InstanceIndex,
// parameterized by store type of %int or %uint
std::string InstanceIndexPreamble(std::string store_type) {
    return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %position %1
    OpName %position "position"
    OpDecorate %position BuiltIn Position
    OpDecorate %1 BuiltIn InstanceIndex
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
    %ptr_ty = OpTypePointer Input )" +
           store_type + R"(
    %1 = OpVariable %ptr_ty Input
    %v4float = OpTypeVector %float 4
    %posty = OpTypePointer Output %v4float
    %position = OpVariable %posty Output
)";
}

TEST_F(SpvModuleScopeVarParserTest, InstanceIndex_I32_Load_Direct) {
    const std::string assembly = InstanceIndexPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpLoad %int %1
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : i32;

var<private> position_1 : vec4f;

fn main_1() {
  let x_2 = x_1;
  return;
}

struct main_out {
  @builtin(position)
  position_1_1 : vec4f,
}

@vertex
fn main(@builtin(instance_index) x_1_param : u32) -> main_out {
  x_1 = bitcast<i32>(x_1_param);
  main_1();
  return main_out(position_1);
}
)";
    EXPECT_EQ(module_str, expected) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, InstanceIndex_I32_Load_CopyObject) {
    const std::string assembly = InstanceIndexPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpCopyObject %ptr_ty %1
    %2 = OpLoad %int %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : i32;

var<private> position_1 : vec4f;

fn main_1() {
  let x_14 = &(x_1);
  let x_2 = *(x_14);
  return;
}

struct main_out {
  @builtin(position)
  position_1_1 : vec4f,
}

@vertex
fn main(@builtin(instance_index) x_1_param : u32) -> main_out {
  x_1 = bitcast<i32>(x_1_param);
  main_1();
  return main_out(position_1);
}
)";
    EXPECT_EQ(module_str, expected) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, InstanceIndex_I32_Load_AccessChain) {
    const std::string assembly = InstanceIndexPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpAccessChain %ptr_ty %1
    %2 = OpLoad %int %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : i32;

var<private> position_1 : vec4f;

fn main_1() {
  let x_2 = x_1;
  return;
}

struct main_out {
  @builtin(position)
  position_1_1 : vec4f,
}

@vertex
fn main(@builtin(instance_index) x_1_param : u32) -> main_out {
  x_1 = bitcast<i32>(x_1_param);
  main_1();
  return main_out(position_1);
}
)";
    EXPECT_EQ(module_str, expected) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, InstanceIndex_I32_FunctParam) {
    const std::string assembly = InstanceIndexPreamble("%int") + R"(
    %helper_ty = OpTypeFunction %int %ptr_ty
    %helper = OpFunction %int None %helper_ty
    %param = OpFunctionParameter %ptr_ty
    %helper_entry = OpLabel
    %3 = OpLoad %int %param
    OpReturnValue %3
    OpFunctionEnd

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %result = OpFunctionCall %int %helper %1
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    // This example is invalid because you can't pass pointer-to-Input
    // as a function parameter.
    EXPECT_FALSE(p->Parse());
    EXPECT_THAT(p->error(), HasSubstr("Invalid storage class for pointer operand '1"));
}

TEST_F(SpvModuleScopeVarParserTest, InstanceIndex_U32_Load_Direct) {
    const std::string assembly = InstanceIndexPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpLoad %uint %1
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : u32;

var<private> position_1 : vec4f;

fn main_1() {
  let x_2 = x_1;
  return;
}

struct main_out {
  @builtin(position)
  position_1_1 : vec4f,
}

@vertex
fn main(@builtin(instance_index) x_1_param : u32) -> main_out {
  x_1 = x_1_param;
  main_1();
  return main_out(position_1);
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, InstanceIndex_U32_Load_CopyObject) {
    const std::string assembly = InstanceIndexPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpCopyObject %ptr_ty %1
    %2 = OpLoad %uint %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : u32;

var<private> position_1 : vec4f;

fn main_1() {
  let x_14 = &(x_1);
  let x_2 = *(x_14);
  return;
}

struct main_out {
  @builtin(position)
  position_1_1 : vec4f,
}

@vertex
fn main(@builtin(instance_index) x_1_param : u32) -> main_out {
  x_1 = x_1_param;
  main_1();
  return main_out(position_1);
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, InstanceIndex_U32_Load_AccessChain) {
    const std::string assembly = InstanceIndexPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpAccessChain %ptr_ty %1
    %2 = OpLoad %uint %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : u32;

var<private> position_1 : vec4f;

fn main_1() {
  let x_2 = x_1;
  return;
}

struct main_out {
  @builtin(position)
  position_1_1 : vec4f,
}

@vertex
fn main(@builtin(instance_index) x_1_param : u32) -> main_out {
  x_1 = x_1_param;
  main_1();
  return main_out(position_1);
}
)";
    EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, InstanceIndex_U32_FunctParam) {
    const std::string assembly = InstanceIndexPreamble("%uint") + R"(
    %helper_ty = OpTypeFunction %uint %ptr_ty
    %helper = OpFunction %uint None %helper_ty
    %param = OpFunctionParameter %ptr_ty
    %helper_entry = OpLabel
    %3 = OpLoad %uint %param
    OpReturnValue %3
    OpFunctionEnd

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %result = OpFunctionCall %uint %helper %1
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    // This example is invalid because you can't pass pointer-to-Input
    // as a function parameter.
    EXPECT_FALSE(p->Parse());
    EXPECT_THAT(p->error(), HasSubstr("Invalid storage class for pointer operand '1"));
}

// Returns the start of a shader for testing LocalInvocationIndex,
// parameterized by store type of %int or %uint
std::string ComputeBuiltinInputPreamble(std::string builtin, std::string store_type) {
    std::string ptr_component_type;
    if (store_type == "%v3int") {
        ptr_component_type = " %ptr_comp_ty = OpTypePointer Input %int\n";
    }
    if (store_type == "%v3uint") {
        ptr_component_type = " %ptr_comp_ty = OpTypePointer Input %uint\n";
    }

    return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint GLCompute %main "main" %1
    OpExecutionMode %main LocalSize 1 1 1
    OpDecorate %1 BuiltIn )" +
           builtin + R"(
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
    %int_1 = OpConstant %int 1
    %v3uint = OpTypeVector %uint 3
    %v3int = OpTypeVector %int 3
    %ptr_ty = OpTypePointer Input )" +
           store_type + ptr_component_type + R"(
    %1 = OpVariable %ptr_ty Input
)";
}

struct ComputeBuiltinInputCase {
    std::string spirv_builtin;
    std::string spirv_store_type;
    std::string wgsl_builtin;
};
inline std::ostream& operator<<(std::ostream& o, ComputeBuiltinInputCase c) {
    return o << "ComputeBuiltinInputCase(" << c.spirv_builtin << " " << c.spirv_store_type << " "
             << c.wgsl_builtin << ")";
}

std::string WgslType(std::string spirv_type) {
    if (spirv_type == "%uint") {
        return "u32";
    }
    if (spirv_type == "%int") {
        return "i32";
    }
    if (spirv_type == "%v3uint") {
        return "vec3u";
    }
    if (spirv_type == "%v3int") {
        return "vec3i";
    }
    return "error";
}

std::string UnsignedWgslType(std::string wgsl_type) {
    if (wgsl_type == "u32") {
        return "u32";
    }
    if (wgsl_type == "i32") {
        return "u32";
    }
    if (wgsl_type == "vec3u") {
        return "vec3u";
    }
    if (wgsl_type == "vec3i") {
        return "vec3u";
    }
    return "error";
}

std::string SignedWgslType(std::string wgsl_type) {
    if (wgsl_type == "u32") {
        return "i32";
    }
    if (wgsl_type == "i32") {
        return "i32";
    }
    if (wgsl_type == "vec3u") {
        return "vec3i";
    }
    if (wgsl_type == "vec3i") {
        return "vec3i";
    }
    return "error";
}

using SpvModuleScopeVarParserTest_ComputeBuiltin =
    SpirvASTParserTestBase<::testing::TestWithParam<ComputeBuiltinInputCase>>;

TEST_P(SpvModuleScopeVarParserTest_ComputeBuiltin, Load_Direct) {
    const auto wgsl_type = WgslType(GetParam().spirv_store_type);
    const auto wgsl_builtin = GetParam().wgsl_builtin;
    const auto unsigned_wgsl_type = UnsignedWgslType(wgsl_type);
    const auto signed_wgsl_type = SignedWgslType(wgsl_type);
    const std::string assembly =
        ComputeBuiltinInputPreamble(GetParam().spirv_builtin, GetParam().spirv_store_type) +
        R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpLoad )" +
        GetParam().spirv_store_type + R"( %1
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    std::string expected = R"(var<private> x_1 : ${wgsl_type};

fn main_1() {
  let x_2 = x_1;
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main(@builtin(${wgsl_builtin}) x_1_param : ${unsigned_wgsl_type}) {
  x_1 = ${assignment_value};
  main_1();
}
)";

    expected = tint::ReplaceAll(expected, "${wgsl_type}", wgsl_type);
    expected = tint::ReplaceAll(expected, "${unsigned_wgsl_type}", unsigned_wgsl_type);
    expected = tint::ReplaceAll(expected, "${wgsl_builtin}", wgsl_builtin);
    expected = tint::ReplaceAll(expected, "${assignment_value}",
                                (wgsl_type == unsigned_wgsl_type)
                                    ? "x_1_param"
                                    : "bitcast<" + signed_wgsl_type + ">(x_1_param)");

    EXPECT_EQ(module_str, expected) << module_str;
}

TEST_P(SpvModuleScopeVarParserTest_ComputeBuiltin, Load_CopyObject) {
    const auto wgsl_type = WgslType(GetParam().spirv_store_type);
    const auto wgsl_builtin = GetParam().wgsl_builtin;
    const auto unsigned_wgsl_type = UnsignedWgslType(wgsl_type);
    const auto signed_wgsl_type = SignedWgslType(wgsl_type);
    const std::string assembly =
        ComputeBuiltinInputPreamble(GetParam().spirv_builtin, GetParam().spirv_store_type) +
        R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %13 = OpCopyObject %ptr_ty %1
    %2 = OpLoad )" +
        GetParam().spirv_store_type + R"( %13
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    std::string expected = R"(var<private> x_1 : ${wgsl_type};

fn main_1() {
  let x_13 = &(x_1);
  let x_2 = *(x_13);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main(@builtin(${wgsl_builtin}) x_1_param : ${unsigned_wgsl_type}) {
  x_1 = ${assignment_value};
  main_1();
}
)";

    expected = tint::ReplaceAll(expected, "${wgsl_type}", wgsl_type);
    expected = tint::ReplaceAll(expected, "${unsigned_wgsl_type}", unsigned_wgsl_type);
    expected = tint::ReplaceAll(expected, "${wgsl_builtin}", wgsl_builtin);
    expected = tint::ReplaceAll(expected, "${assignment_value}",
                                (wgsl_type == unsigned_wgsl_type)
                                    ? "x_1_param"
                                    : "bitcast<" + signed_wgsl_type + ">(x_1_param)");

    EXPECT_EQ(module_str, expected) << module_str;
}

TEST_P(SpvModuleScopeVarParserTest_ComputeBuiltin, Load_AccessChain) {
    const auto wgsl_type = WgslType(GetParam().spirv_store_type);
    const auto wgsl_builtin = GetParam().wgsl_builtin;
    const auto unsigned_wgsl_type = UnsignedWgslType(wgsl_type);
    const auto signed_wgsl_type = SignedWgslType(wgsl_type);
    const std::string assembly =
        ComputeBuiltinInputPreamble(GetParam().spirv_builtin, GetParam().spirv_store_type) +
        R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %13 = OpAccessChain %ptr_ty %1
    %2 = OpLoad )" +
        GetParam().spirv_store_type + R"( %13
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    std::string expected = R"(var<private> x_1 : ${wgsl_type};

fn main_1() {
  let x_2 = x_1;
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main(@builtin(${wgsl_builtin}) x_1_param : ${unsigned_wgsl_type}) {
  x_1 = ${assignment_value};
  main_1();
}
)";

    expected = tint::ReplaceAll(expected, "${wgsl_type}", wgsl_type);
    expected = tint::ReplaceAll(expected, "${unsigned_wgsl_type}", unsigned_wgsl_type);
    expected = tint::ReplaceAll(expected, "${wgsl_builtin}", wgsl_builtin);
    expected = tint::ReplaceAll(expected, "${assignment_value}",
                                (wgsl_type == unsigned_wgsl_type)
                                    ? "x_1_param"
                                    : "bitcast<" + signed_wgsl_type + ">(x_1_param)");

    EXPECT_EQ(module_str, expected) << module_str;
}

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvModuleScopeVarParserTest_ComputeBuiltin,
                         ::testing::ValuesIn(std::vector<ComputeBuiltinInputCase>{
                             {"LocalInvocationIndex", "%uint", "local_invocation_index"},
                             {"LocalInvocationIndex", "%int", "local_invocation_index"},
                             {"LocalInvocationId", "%v3uint", "local_invocation_id"},
                             {"LocalInvocationId", "%v3int", "local_invocation_id"},
                             {"GlobalInvocationId", "%v3uint", "global_invocation_id"},
                             {"GlobalInvocationId", "%v3int", "global_invocation_id"},
                             {"NumWorkgroups", "%v3uint", "num_workgroups"},
                             {"NumWorkgroups", "%v3int", "num_workgroups"},
                             {"WorkgroupId", "%v3uint", "workgroup_id"},
                             {"WorkgroupId", "%v3int", "workgroup_id"}}));

// For compute shader builtins that are vectors, test loading one component.
struct ComputeBuiltinInputVectorCase {
    std::string spirv_builtin;
    std::string spirv_store_type;
    std::string spirv_component_store_type;
    std::string wgsl_builtin;
};
inline std::ostream& operator<<(std::ostream& o, ComputeBuiltinInputVectorCase c) {
    return o << "ComputeBuiltinInputVectorCase(" << c.spirv_builtin << " " << c.spirv_store_type
             << " " << c.spirv_component_store_type << " " << c.wgsl_builtin << ")";
}

using SpvModuleScopeVarParserTest_ComputeBuiltinVector =
    SpirvASTParserTestBase<::testing::TestWithParam<ComputeBuiltinInputVectorCase>>;

TEST_P(SpvModuleScopeVarParserTest_ComputeBuiltinVector, Load_Component_Direct) {
    const auto wgsl_type = WgslType(GetParam().spirv_store_type);
    const auto wgsl_component_type = WgslType(GetParam().spirv_component_store_type);
    const auto wgsl_builtin = GetParam().wgsl_builtin;
    const auto unsigned_wgsl_type = UnsignedWgslType(wgsl_type);
    const auto signed_wgsl_type = SignedWgslType(wgsl_type);
    const std::string assembly =
        ComputeBuiltinInputPreamble(GetParam().spirv_builtin, GetParam().spirv_store_type) +
        R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %3 = OpAccessChain %ptr_comp_ty %1 %int_1
    %2 = OpLoad )" +
        GetParam().spirv_component_store_type + R"( %3
    OpReturn
    OpFunctionEnd
 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto module_str = test::ToString(p->program());
    std::string expected = R"(var<private> x_1 : ${wgsl_type};

fn main_1() {
  let x_2 = x_1.y;
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main(@builtin(${wgsl_builtin}) x_1_param : ${unsigned_wgsl_type}) {
  x_1 = ${assignment_value};
  main_1();
}
)";

    expected = tint::ReplaceAll(expected, "${wgsl_type}", wgsl_type);
    expected = tint::ReplaceAll(expected, "${wgsl_component_type}", wgsl_component_type);
    expected = tint::ReplaceAll(expected, "${unsigned_wgsl_type}", unsigned_wgsl_type);
    expected = tint::ReplaceAll(expected, "${wgsl_builtin}", wgsl_builtin);
    expected = tint::ReplaceAll(expected, "${assignment_value}",
                                (wgsl_type == unsigned_wgsl_type)
                                    ? "x_1_param"
                                    : "bitcast<" + signed_wgsl_type + ">(x_1_param)");

    EXPECT_EQ(module_str, expected) << module_str;
}

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvModuleScopeVarParserTest_ComputeBuiltinVector,
                         ::testing::ValuesIn(std::vector<ComputeBuiltinInputVectorCase>{
                             {"LocalInvocationId", "%v3uint", "%uint", "local_invocation_id"},
                             {"LocalInvocationId", "%v3int", "%int", "local_invocation_id"},
                             {"GlobalInvocationId", "%v3uint", "%uint", "global_invocation_id"},
                             {"GlobalInvocationId", "%v3int", "%int", "global_invocation_id"},
                             {"NumWorkgroups", "%v3uint", "%uint", "num_workgroups"},
                             {"NumWorkgroups", "%v3int", "%int", "num_workgroups"},
                             {"WorkgroupId", "%v3uint", "%uint", "workgroup_id"},
                             {"WorkgroupId", "%v3int", "%int", "workgroup_id"}}));

TEST_F(SpvModuleScopeVarParserTest, RegisterInputOutputVars) {
    const std::string assembly =
        R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Fragment %1000 "w1000"
    OpEntryPoint Fragment %1100 "w1100" %1
    OpEntryPoint Fragment %1200 "w1200" %2 %15
    ; duplication is tolerated prior to SPIR-V 1.4
    OpEntryPoint Fragment %1300 "w1300" %1 %15 %2 %1
    OpExecutionMode %1000 OriginUpperLeft
    OpExecutionMode %1100 OriginUpperLeft
    OpExecutionMode %1200 OriginUpperLeft
    OpExecutionMode %1300 OriginUpperLeft

    OpDecorate %1 Location 1
    OpDecorate %2 Location 2
    OpDecorate %5 Location 5
    OpDecorate %11 Location 1
    OpDecorate %12 Location 2
    OpDecorate %15 Location 5

)" + CommonTypes() +
        R"(

    %ptr_in_uint = OpTypePointer Input %uint
    %ptr_out_uint = OpTypePointer Output %uint

    %1 = OpVariable %ptr_in_uint Input
    %2 = OpVariable %ptr_in_uint Input
    %5 = OpVariable %ptr_in_uint Input
    %11 = OpVariable %ptr_out_uint Output
    %12 = OpVariable %ptr_out_uint Output
    %15 = OpVariable %ptr_out_uint Output

    %100 = OpFunction %void None %voidfn
    %entry_100 = OpLabel
    %load_100 = OpLoad %uint %1
    OpReturn
    OpFunctionEnd

    %200 = OpFunction %void None %voidfn
    %entry_200 = OpLabel
    %load_200 = OpLoad %uint %2
    OpStore %15 %load_200
    OpStore %15 %load_200
    OpReturn
    OpFunctionEnd

    %300 = OpFunction %void None %voidfn
    %entry_300 = OpLabel
    %placeholder_300_1 = OpFunctionCall %void %100
    %placeholder_300_2 = OpFunctionCall %void %200
    OpReturn
    OpFunctionEnd

    ; Call nothing
    %1000 = OpFunction %void None %voidfn
    %entry_1000 = OpLabel
    OpReturn
    OpFunctionEnd

    ; Call %100
    %1100 = OpFunction %void None %voidfn
    %entry_1100 = OpLabel
    %placeholder_1100_1 = OpFunctionCall %void %100
    OpReturn
    OpFunctionEnd

    ; Call %200
    %1200 = OpFunction %void None %voidfn
    %entry_1200 = OpLabel
    %placeholder_1200_1 = OpFunctionCall %void %200
    OpReturn
    OpFunctionEnd

    ; Call %300
    %1300 = OpFunction %void None %voidfn
    %entry_1300 = OpLabel
    %placeholder_1300_1 = OpFunctionCall %void %300
    OpReturn
    OpFunctionEnd

 )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());

    const auto& info_1000 = p->GetEntryPointInfo(1000);
    EXPECT_EQ(1u, info_1000.size());
    EXPECT_TRUE(info_1000[0].inputs.IsEmpty());
    EXPECT_TRUE(info_1000[0].outputs.IsEmpty());

    const auto& info_1100 = p->GetEntryPointInfo(1100);
    EXPECT_EQ(1u, info_1100.size());
    EXPECT_THAT(info_1100[0].inputs, ElementsAre(1));
    EXPECT_TRUE(info_1100[0].outputs.IsEmpty());

    const auto& info_1200 = p->GetEntryPointInfo(1200);
    EXPECT_EQ(1u, info_1200.size());
    EXPECT_THAT(info_1200[0].inputs, ElementsAre(2));
    EXPECT_THAT(info_1200[0].outputs, ElementsAre(15));

    const auto& info_1300 = p->GetEntryPointInfo(1300);
    EXPECT_EQ(1u, info_1300.size());
    EXPECT_THAT(info_1300[0].inputs, ElementsAre(1, 2));
    EXPECT_THAT(info_1300[0].outputs, ElementsAre(15));

    // Validation incorrectly reports an overlap for the duplicated variable %1 on
    // shader %1300
    p->SkipDumpingPending("https://github.com/KhronosGroup/SPIRV-Tools/issues/4403");
}

TEST_F(SpvModuleScopeVarParserTest, InputVarsConvertedToPrivate) {
    const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %ptr_in_uint = OpTypePointer Input %uint
     %1 = OpVariable %ptr_in_uint Input
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected = "var<private> x_1 : u32;";
    EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvModuleScopeVarParserTest, OutputVarsConvertedToPrivate) {
    const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %ptr_out_uint = OpTypePointer Output %uint
     %1 = OpVariable %ptr_out_uint Output
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected = "var<private> x_1 : u32;";
    EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvModuleScopeVarParserTest, OutputVarsConvertedToPrivate_WithInitializer) {
    const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %ptr_out_uint = OpTypePointer Output %uint
     %1 = OpVariable %ptr_out_uint Output %uint_1
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected = "var<private> x_1 = 1u;";
    EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Builtin_Output_Initializer_SameSignednessAsWGSL) {
    // Only outputs can have initializers.
    // WGSL sample_mask store type is u32.
    const auto assembly = Preamble() + FragMain() + R"(
     OpDecorate %1 BuiltIn SampleMask
)" + CommonTypes() + R"(
     %arr_ty = OpTypeArray %uint %uint_1
     %ptr_ty = OpTypePointer Output %arr_ty
     %arr_init = OpConstantComposite %arr_ty %uint_2
     %1 = OpVariable %ptr_ty Output %arr_init
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected = "var<private> x_1 = array<u32, 1u>(2u);";
    EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Builtin_Output_Initializer_OppositeSignednessAsWGSL) {
    // Only outputs can have initializers.
    // WGSL sample_mask store type is u32.  Use i32 in SPIR-V
    const auto assembly = Preamble() + FragMain() + R"(
     OpDecorate %1 BuiltIn SampleMask
)" + CommonTypes() + R"(
     %arr_ty = OpTypeArray %int %uint_1
     %ptr_ty = OpTypePointer Output %arr_ty
     %arr_init = OpConstantComposite %arr_ty %int_14
     %1 = OpVariable %ptr_ty Output %arr_init
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected = "var<private> x_1 = array<i32, 1u>(14i);";
    EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Builtin_Input_SameSignednessAsWGSL) {
    // WGSL vertex_index store type is u32.
    const auto assembly = Preamble() + FragMain() + R"(
     OpDecorate %1 BuiltIn VertexIndex
)" + CommonTypes() + R"(
     %ptr_ty = OpTypePointer Input %uint
     %1 = OpVariable %ptr_ty Input
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected = "var<private> x_1 : u32;";
    EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Builtin_Input_OppositeSignednessAsWGSL) {
    // WGSL vertex_index store type is u32.  Use i32 in SPIR-V.
    const auto assembly = Preamble() + FragMain() + R"(
     OpDecorate %1 BuiltIn VertexIndex
)" + CommonTypes() + R"(
     %ptr_ty = OpTypePointer Input %int
     %1 = OpVariable %ptr_ty Input
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected = "var<private> x_1 : i32;";
    EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvModuleScopeVarParserTest, EntryPointWrapping_IOLocations) {
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1 %2 %3 %4
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 Location 0
     OpDecorate %2 Location 0
     OpDecorate %3 Location 30
     OpDecorate %4 Location 6
)" + CommonTypes() +
                          R"(
     %ptr_in_uint = OpTypePointer Input %uint
     %ptr_out_uint = OpTypePointer Output %uint
     %1 = OpVariable %ptr_in_uint Input
     %2 = OpVariable %ptr_out_uint Output
     %3 = OpVariable %ptr_in_uint Input
     %4 = OpVariable %ptr_out_uint Output

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected =
        R"(var<private> x_1 : u32;

var<private> x_2 : u32;

var<private> x_3 : u32;

var<private> x_4 : u32;

fn main_1() {
  return;
}

struct main_out {
  @location(0) @interpolate(flat)
  x_2_1 : u32,
  @location(6) @interpolate(flat)
  x_4_1 : u32,
}

@fragment
fn main(@location(0) @interpolate(flat) x_1_param : u32, @location(30) @interpolate(flat) x_3_param : u32) -> main_out {
  x_1 = x_1_param;
  x_3 = x_3_param;
  main_1();
  return main_out(x_2, x_4);
}
)";
    EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvModuleScopeVarParserTest, EntryPointWrapping_BuiltinVar_Input_SameSignedness) {
    // instance_index is u32 in WGSL. Use uint in SPIR-V.
    // No bitcasts are used for parameter formation or return value.
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Vertex %main "main" %1 %position
     OpDecorate %position BuiltIn Position
     OpDecorate %1 BuiltIn InstanceIndex
)" + CommonTypes() +
                          R"(
     %ptr_in_uint = OpTypePointer Input %uint
     %1 = OpVariable %ptr_in_uint Input
     %posty = OpTypePointer Output %v4float
     %position = OpVariable %posty Output

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpLoad %uint %1 ; load same signedness
     ;;;; %3 = OpLoad %int %1 ; loading different signedness is invalid.
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : u32;

var<private> x_4 : vec4f;

fn main_1() {
  let x_2 = x_1;
  return;
}

struct main_out {
  @builtin(position)
  x_4_1 : vec4f,
}

@vertex
fn main(@builtin(instance_index) x_1_param : u32) -> main_out {
  x_1 = x_1_param;
  main_1();
  return main_out(x_4);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, EntryPointWrapping_BuiltinVar_Input_OppositeSignedness) {
    // instance_index is u32 in WGSL. Use int in SPIR-V.
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Vertex %main "main" %position %1
     OpDecorate %position BuiltIn Position
     OpDecorate %1 BuiltIn InstanceIndex
)" + CommonTypes() +
                          R"(
     %ptr_in_int = OpTypePointer Input %int
     %1 = OpVariable %ptr_in_int Input
     %posty = OpTypePointer Output %v4float
     %position = OpVariable %posty Output

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpLoad %int %1 ; load same signedness
     ;;; %3 = OpLoad %uint %1 ; loading different signedness is invalid
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : i32;

var<private> x_4 : vec4f;

fn main_1() {
  let x_2 = x_1;
  return;
}

struct main_out {
  @builtin(position)
  x_4_1 : vec4f,
}

@vertex
fn main(@builtin(instance_index) x_1_param : u32) -> main_out {
  x_1 = bitcast<i32>(x_1_param);
  main_1();
  return main_out(x_4);
}
)";
    EXPECT_EQ(got, expected) << got;
}

// SampleMask is an array in Vulkan SPIR-V, but a scalar in WGSL.
TEST_F(SpvModuleScopeVarParserTest, EntryPointWrapping_BuiltinVar_SampleMask_In_Unsigned) {
    // SampleMask is u32 in WGSL.
    // Use unsigned array element in Vulkan.
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 BuiltIn SampleMask
)" + CommonTypes() +
                          R"(
     %arr = OpTypeArray %uint %uint_1
     %ptr_ty = OpTypePointer Input %arr
     %1 = OpVariable %ptr_ty Input

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<u32, 1u>;

fn main_1() {
  return;
}

@fragment
fn main(@builtin(sample_mask) x_1_param : u32) {
  x_1[0i] = x_1_param;
  main_1();
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, EntryPointWrapping_BuiltinVar_SampleMask_In_Signed) {
    // SampleMask is u32 in WGSL.
    // Use signed array element in Vulkan.
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 BuiltIn SampleMask
)" + CommonTypes() +
                          R"(
     %arr = OpTypeArray %int %uint_1
     %ptr_ty = OpTypePointer Input %arr
     %1 = OpVariable %ptr_ty Input

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<i32, 1u>;

fn main_1() {
  return;
}

@fragment
fn main(@builtin(sample_mask) x_1_param : u32) {
  x_1[0i] = bitcast<i32>(x_1_param);
  main_1();
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_BuiltinVar_SampleMask_Out_Unsigned_Initializer) {
    // SampleMask is u32 in WGSL.
    // Use unsigned array element in Vulkan.
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 BuiltIn SampleMask
)" + CommonTypes() +
                          R"(
     %arr = OpTypeArray %uint %uint_1
     %ptr_ty = OpTypePointer Output %arr
     %zero = OpConstantNull %arr
     %1 = OpVariable %ptr_ty Output %zero

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected =
        R"(var<private> x_1 = array<u32, 1u>();

fn main_1() {
  return;
}

struct main_out {
  @builtin(sample_mask)
  x_1_1 : u32,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_1[0i]);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_BuiltinVar_SampleMask_Out_Signed_Initializer) {
    // SampleMask is u32 in WGSL.
    // Use signed array element in Vulkan.
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 BuiltIn SampleMask
)" + CommonTypes() +
                          R"(
     %arr = OpTypeArray %int %uint_1
     %ptr_ty = OpTypePointer Output %arr
     %zero = OpConstantNull %arr
     %1 = OpVariable %ptr_ty Output %zero

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected =
        R"(var<private> x_1 = array<i32, 1u>();

fn main_1() {
  return;
}

struct main_out {
  @builtin(sample_mask)
  x_1_1 : u32,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(bitcast<u32>(x_1[0i]));
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, EntryPointWrapping_BuiltinVar_FragDepth_Out_Initializer) {
    // FragDepth does not require conversion, because it's f32.
    // The member of the return type is just the identifier corresponding
    // to the module-scope private variable.
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 BuiltIn FragDepth
)" + CommonTypes() +
                          R"(
     %ptr_ty = OpTypePointer Output %float
     %1 = OpVariable %ptr_ty Output %float_0

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 = 0.0f;

fn main_1() {
  return;
}

struct main_out {
  @builtin(frag_depth)
  x_1_1 : f32,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_1);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPosition_BuiltIn_Position) {
    // In Vulkan SPIR-V, Position is the first member of gl_PerVertex
    const std::string assembly = PerVertexPreamble() + R"(
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  OpReturn
  OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());

    const auto got = test::ToString(p->program());
    const std::string expected = R"(var<private> gl_Position : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @builtin(position)
  gl_Position : vec4f,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(gl_Position);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPosition_BuiltIn_Position_Initializer) {
    const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1

    OpDecorate %10 Block
    OpMemberDecorate %10 0 BuiltIn Position
    OpMemberDecorate %10 1 BuiltIn PointSize
    OpMemberDecorate %10 2 BuiltIn ClipDistance
    OpMemberDecorate %10 3 BuiltIn CullDistance
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %uint = OpTypeInt 32 0
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %arr = OpTypeArray %float %uint_1
    %10 = OpTypeStruct %v4float %float %arr %arr
    %11 = OpTypePointer Output %10

    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
    %float_6 = OpConstant %float 6
    %float_7 = OpConstant %float 7

    %init_pos = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
    %init_clip = OpConstantComposite %arr %float_6
    %init_cull = OpConstantComposite %arr %float_7
    %init_per_vertex = OpConstantComposite %10 %init_pos %float_5 %init_clip %init_cull

    %1 = OpVariable %11 Output %init_per_vertex

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());

    const auto got = test::ToString(p->program());
    const std::string expected =
        R"(var<private> gl_Position = vec4f(1.0f, 2.0f, 3.0f, 4.0f);

fn main_1() {
  return;
}

struct main_out {
  @builtin(position)
  gl_Position : vec4f,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(gl_Position);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPosition_MultiplePerVertexVariables) {
    // This is not currently supported, so just make sure we produce a meaningful error instead of
    // crashing.
    const std::string assembly = R"(
  OpCapability Shader
  OpMemoryModel Logical Simple
  OpEntryPoint Vertex %main "main" %1
  OpDecorate %struct Block
  OpMemberDecorate %struct 0 BuiltIn Position
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  %f32 = OpTypeFloat 32
  %vec4f = OpTypeVector %f32 4
  %struct = OpTypeStruct %vec4f
  %struct_out_ptr = OpTypePointer Output %struct
  %1 = OpVariable %struct_out_ptr Output
  %2 = OpVariable %struct_out_ptr Output
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  OpReturn
  OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));

    EXPECT_FALSE(p->Parse());
    EXPECT_FALSE(p->success());
    EXPECT_EQ(p->error(), "unsupported: multiple Position built-in variables in the same module");
}

TEST_F(SpvModuleScopeVarParserTest, Input_FlattenArray_OneLevel) {
    const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2
    OpDecorate %1 Location 4
    OpDecorate %2 BuiltIn Position

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %uint = OpTypeInt 32 0
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %uint_3 = OpConstant %uint 3
    %arr = OpTypeArray %float %uint_3
    %11 = OpTypePointer Input %arr

    %1 = OpVariable %11 Input

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());

    const auto got = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<f32, 3u>;

var<private> x_2 : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @builtin(position)
  x_2_1 : vec4f,
}

@vertex
fn main(@location(4) x_1_param : f32, @location(5) x_1_param_1 : f32, @location(6) x_1_param_2 : f32) -> main_out {
  x_1[0i] = x_1_param;
  x_1[1i] = x_1_param_1;
  x_1[2i] = x_1_param_2;
  main_1();
  return main_out(x_2);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Input_FlattenMatrix) {
    const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2
    OpDecorate %1 Location 9
    OpDecorate %2 BuiltIn Position

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %m2v4float = OpTypeMatrix %v4float 2
    %uint = OpTypeInt 32 0

    %11 = OpTypePointer Input %m2v4float

    %1 = OpVariable %11 Input

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());

    const auto got = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : mat2x4f;

var<private> x_2 : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @builtin(position)
  x_2_1 : vec4f,
}

@vertex
fn main(@location(9) x_1_param : vec4f, @location(10) x_1_param_1 : vec4f) -> main_out {
  x_1[0i] = x_1_param;
  x_1[1i] = x_1_param_1;
  main_1();
  return main_out(x_2);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Input_FlattenStruct_LocOnVariable) {
    const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2

    OpName %strct "Communicators"
    OpMemberName %strct 0 "alice"
    OpMemberName %strct 1 "bob"

    OpDecorate %1 Location 9
    OpDecorate %2 BuiltIn Position


    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %strct = OpTypeStruct %float %v4float

    %11 = OpTypePointer Input %strct

    %1 = OpVariable %11 Input

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());

    const auto got = test::ToString(p->program());
    const std::string expected = R"(struct Communicators {
  alice : f32,
  bob : vec4f,
}

var<private> x_1 : Communicators;

var<private> x_2 : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @builtin(position)
  x_2_1 : vec4f,
}

@vertex
fn main(@location(9) x_1_param : f32, @location(10) x_1_param_1 : vec4f) -> main_out {
  x_1.alice = x_1_param;
  x_1.bob = x_1_param_1;
  main_1();
  return main_out(x_2);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Input_FlattenNested) {
    const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2
    OpDecorate %1 Location 7
    OpDecorate %2 BuiltIn Position

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %m2v4float = OpTypeMatrix %v4float 2
    %uint = OpTypeInt 32 0
    %uint_2 = OpConstant %uint 2

    %arr = OpTypeArray %m2v4float %uint_2

    %11 = OpTypePointer Input %arr
    %1 = OpVariable %11 Input

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());

    const auto got = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<mat2x4f, 2u>;

var<private> x_2 : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @builtin(position)
  x_2_1 : vec4f,
}

@vertex
fn main(@location(7) x_1_param : vec4f, @location(8) x_1_param_1 : vec4f, @location(9) x_1_param_2 : vec4f, @location(10) x_1_param_3 : vec4f) -> main_out {
  x_1[0i][0i] = x_1_param;
  x_1[0i][1i] = x_1_param_1;
  x_1[1i][0i] = x_1_param_2;
  x_1[1i][1i] = x_1_param_3;
  main_1();
  return main_out(x_2);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Output_FlattenArray_OneLevel) {
    const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2
    OpDecorate %1 Location 4
    OpDecorate %2 BuiltIn Position

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %uint = OpTypeInt 32 0
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %uint_3 = OpConstant %uint 3
    %arr = OpTypeArray %float %uint_3
    %11 = OpTypePointer Output %arr

    %1 = OpVariable %11 Output

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());

    const auto got = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : array<f32, 3u>;

var<private> x_2 : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @location(4)
  x_1_1 : f32,
  @location(5)
  x_1_2 : f32,
  @location(6)
  x_1_3 : f32,
  @builtin(position)
  x_2_1 : vec4f,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(x_1[0i], x_1[1i], x_1[2i], x_2);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Output_FlattenMatrix) {
    const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2
    OpDecorate %1 Location 9
    OpDecorate %2 BuiltIn Position

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %m2v4float = OpTypeMatrix %v4float 2
    %uint = OpTypeInt 32 0

    %11 = OpTypePointer Output %m2v4float

    %1 = OpVariable %11 Output

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());

    const auto got = test::ToString(p->program());
    const std::string expected = R"(var<private> x_1 : mat2x4f;

var<private> x_2 : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @location(9)
  x_1_1 : vec4f,
  @location(10)
  x_1_2 : vec4f,
  @builtin(position)
  x_2_1 : vec4f,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(x_1[0i], x_1[1i], x_2);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Output_FlattenStruct_LocOnVariable) {
    const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2

    OpName %strct "Communicators"
    OpMemberName %strct 0 "alice"
    OpMemberName %strct 1 "bob"

    OpDecorate %1 Location 9
    OpDecorate %2 BuiltIn Position


    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %strct = OpTypeStruct %float %v4float

    %11 = OpTypePointer Output %strct

    %1 = OpVariable %11 Output

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());

    const auto got = test::ToString(p->program());
    const std::string expected = R"(struct Communicators {
  alice : f32,
  bob : vec4f,
}

var<private> x_1 : Communicators;

var<private> x_2 : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @location(9)
  x_1_1 : f32,
  @location(10)
  x_1_2 : vec4f,
  @builtin(position)
  x_2_1 : vec4f,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(x_1.alice, x_1.bob, x_2);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, FlattenStruct_LocOnMembers) {
    // Block-decorated struct may have its members decorated with Location.
    const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2 %3

    OpName %strct "Communicators"
    OpMemberName %strct 0 "alice"
    OpMemberName %strct 1 "bob"

    OpMemberDecorate %strct 0 Location 9
    OpMemberDecorate %strct 1 Location 11
    OpDecorate %strct Block
    OpDecorate %2 BuiltIn Position

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %strct = OpTypeStruct %float %v4float

    %11 = OpTypePointer Input %strct
    %13 = OpTypePointer Output %strct

    %1 = OpVariable %11 Input
    %3 = OpVariable %13 Output

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->Parse()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty());

    const auto got = test::ToString(p->program());
    const std::string expected = R"(struct Communicators {
  alice : f32,
  bob : vec4f,
}

var<private> x_1 : Communicators;

var<private> x_3 : Communicators;

var<private> x_2 : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @builtin(position)
  x_2_1 : vec4f,
  @location(9)
  x_3_1 : f32,
  @location(11)
  x_3_2 : vec4f,
}

@vertex
fn main(@location(9) x_1_param : f32, @location(11) x_1_param_1 : vec4f) -> main_out {
  x_1.alice = x_1_param;
  x_1.bob = x_1_param_1;
  main_1();
  return main_out(x_2, x_3.alice, x_3.bob);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, EntryPointWrapping_Interpolation_Flat_Vertex_In) {
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Vertex %main "main" %1 %2 %3 %4 %5 %6 %10
     OpDecorate %1 Location 1
     OpDecorate %2 Location 2
     OpDecorate %3 Location 3
     OpDecorate %4 Location 4
     OpDecorate %5 Location 5
     OpDecorate %6 Location 6
     OpDecorate %1 Flat
     OpDecorate %2 Flat
     OpDecorate %3 Flat
     OpDecorate %4 Flat
     OpDecorate %5 Flat
     OpDecorate %6 Flat
     OpDecorate %10 BuiltIn Position
)" + CommonTypes() +
                          R"(
     %ptr_in_uint = OpTypePointer Input %uint
     %ptr_in_v2uint = OpTypePointer Input %v2uint
     %ptr_in_int = OpTypePointer Input %int
     %ptr_in_v2int = OpTypePointer Input %v2int
     %ptr_in_float = OpTypePointer Input %float
     %ptr_in_v2float = OpTypePointer Input %v2float
     %1 = OpVariable %ptr_in_uint Input
     %2 = OpVariable %ptr_in_v2uint Input
     %3 = OpVariable %ptr_in_int Input
     %4 = OpVariable %ptr_in_v2int Input
     %5 = OpVariable %ptr_in_float Input
     %6 = OpVariable %ptr_in_v2float Input

     %ptr_out_v4float = OpTypePointer Output %v4float
     %10 = OpVariable %ptr_out_v4float Output

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected =
        R"(var<private> x_1 : u32;

var<private> x_2 : vec2u;

var<private> x_3 : i32;

var<private> x_4 : vec2i;

var<private> x_5 : f32;

var<private> x_6 : vec2f;

var<private> x_10 : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @builtin(position)
  x_10_1 : vec4f,
}

@vertex
fn main(@location(1) @interpolate(flat) x_1_param : u32, @location(2) @interpolate(flat) x_2_param : vec2u, @location(3) @interpolate(flat) x_3_param : i32, @location(4) @interpolate(flat) x_4_param : vec2i, @location(5) @interpolate(flat) x_5_param : f32, @location(6) @interpolate(flat) x_6_param : vec2f) -> main_out {
  x_1 = x_1_param;
  x_2 = x_2_param;
  x_3 = x_3_param;
  x_4 = x_4_param;
  x_5 = x_5_param;
  x_6 = x_6_param;
  main_1();
  return main_out(x_10);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, EntryPointWrapping_Interpolation_Flat_Vertex_Output) {
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Vertex %main "main" %1 %2 %3 %4 %5 %6 %10
     OpDecorate %1 Location 1
     OpDecorate %2 Location 2
     OpDecorate %3 Location 3
     OpDecorate %4 Location 4
     OpDecorate %5 Location 5
     OpDecorate %6 Location 6
     OpDecorate %1 Flat
     OpDecorate %2 Flat
     OpDecorate %3 Flat
     OpDecorate %4 Flat
     OpDecorate %5 Flat
     OpDecorate %6 Flat
     OpDecorate %10 BuiltIn Position
)" + CommonTypes() +
                          R"(
     %ptr_out_uint = OpTypePointer Output %uint
     %ptr_out_v2uint = OpTypePointer Output %v2uint
     %ptr_out_int = OpTypePointer Output %int
     %ptr_out_v2int = OpTypePointer Output %v2int
     %ptr_out_float = OpTypePointer Output %float
     %ptr_out_v2float = OpTypePointer Output %v2float
     %1 = OpVariable %ptr_out_uint Output
     %2 = OpVariable %ptr_out_v2uint Output
     %3 = OpVariable %ptr_out_int Output
     %4 = OpVariable %ptr_out_v2int Output
     %5 = OpVariable %ptr_out_float Output
     %6 = OpVariable %ptr_out_v2float Output

     %ptr_out_v4float = OpTypePointer Output %v4float
     %10 = OpVariable %ptr_out_v4float Output

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected =
        R"(var<private> x_1 : u32;

var<private> x_2 : vec2u;

var<private> x_3 : i32;

var<private> x_4 : vec2i;

var<private> x_5 : f32;

var<private> x_6 : vec2f;

var<private> x_10 : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @location(1) @interpolate(flat)
  x_1_1 : u32,
  @location(2) @interpolate(flat)
  x_2_1 : vec2u,
  @location(3) @interpolate(flat)
  x_3_1 : i32,
  @location(4) @interpolate(flat)
  x_4_1 : vec2i,
  @location(5) @interpolate(flat)
  x_5_1 : f32,
  @location(6) @interpolate(flat)
  x_6_1 : vec2f,
  @builtin(position)
  x_10_1 : vec4f,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(x_1, x_2, x_3, x_4, x_5, x_6, x_10);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, EntryPointWrapping_Flatten_Interpolation_Flat_Fragment_In) {
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1 %2
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 Location 1
     OpDecorate %2 Location 5
     OpDecorate %1 Flat
     OpDecorate %2 Flat
)" + CommonTypes() +
                          R"(
     %arr = OpTypeArray %float %uint_2
     %strct = OpTypeStruct %float %float
     %ptr_in_arr = OpTypePointer Input %arr
     %ptr_in_strct = OpTypePointer Input %strct
     %1 = OpVariable %ptr_in_arr Input
     %2 = OpVariable %ptr_in_strct Input

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected =
        R"(struct S {
  field0 : f32,
  field1 : f32,
}

var<private> x_1 : array<f32, 2u>;

var<private> x_2 : S;

fn main_1() {
  return;
}

@fragment
fn main(@location(1) @interpolate(flat) x_1_param : f32, @location(2) @interpolate(flat) x_1_param_1 : f32, @location(5) @interpolate(flat) x_2_param : f32, @location(6) @interpolate(flat) x_2_param_1 : f32) {
  x_1[0i] = x_1_param;
  x_1[1i] = x_1_param_1;
  x_2.field0 = x_2_param;
  x_2.field1 = x_2_param_1;
  main_1();
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, EntryPointWrapping_Interpolation_Floating_Fragment_In) {
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1 %2 %3 %4 %5 %6
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 Location 1
     OpDecorate %2 Location 2
     OpDecorate %3 Location 3
     OpDecorate %4 Location 4
     OpDecorate %5 Location 5
     OpDecorate %6 Location 6

     ; %1 perspective center

     OpDecorate %2 Centroid ; perspective centroid

     OpDecorate %3 Sample ; perspective sample

     OpDecorate %4 NoPerspective; linear center

     OpDecorate %5 NoPerspective ; linear centroid
     OpDecorate %5 Centroid

     OpDecorate %6 NoPerspective ; linear sample
     OpDecorate %6 Sample

)" + CommonTypes() +
                          R"(
     %ptr_in_float = OpTypePointer Input %float
     %1 = OpVariable %ptr_in_float Input
     %2 = OpVariable %ptr_in_float Input
     %3 = OpVariable %ptr_in_float Input
     %4 = OpVariable %ptr_in_float Input
     %5 = OpVariable %ptr_in_float Input
     %6 = OpVariable %ptr_in_float Input

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected =
        R"(var<private> x_1 : f32;

var<private> x_2 : f32;

var<private> x_3 : f32;

var<private> x_4 : f32;

var<private> x_5 : f32;

var<private> x_6 : f32;

fn main_1() {
  return;
}

@fragment
fn main(@location(1) x_1_param : f32, @location(2) @interpolate(perspective, centroid) x_2_param : f32, @location(3) @interpolate(perspective, sample) x_3_param : f32, @location(4) @interpolate(linear) x_4_param : f32, @location(5) @interpolate(linear, centroid) x_5_param : f32, @location(6) @interpolate(linear, sample) x_6_param : f32) {
  x_1 = x_1_param;
  x_2 = x_2_param;
  x_3 = x_3_param;
  x_4 = x_4_param;
  x_5 = x_5_param;
  x_6 = x_6_param;
  main_1();
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, EntryPointWrapping_Flatten_Interpolation_Floating_Fragment_In) {
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 Location 1

     ; member 0 perspective center

     OpMemberDecorate %10 1 Centroid ; perspective centroid

     OpMemberDecorate %10 2 Sample ; perspective sample

     OpMemberDecorate %10 3 NoPerspective; linear center

     OpMemberDecorate %10 4 NoPerspective ; linear centroid
     OpMemberDecorate %10 4 Centroid

     OpMemberDecorate %10 5 NoPerspective ; linear sample
     OpMemberDecorate %10 5 Sample

)" + CommonTypes() +
                          R"(

     %10 = OpTypeStruct %float %float %float %float %float %float
     %ptr_in_strct = OpTypePointer Input %10
     %1 = OpVariable %ptr_in_strct Input

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected =
        R"(struct S {
  field0 : f32,
  field1 : f32,
  field2 : f32,
  field3 : f32,
  field4 : f32,
  field5 : f32,
}

var<private> x_1 : S;

fn main_1() {
  return;
}

@fragment
fn main(@location(1) x_1_param : f32, @location(2) @interpolate(perspective, centroid) x_1_param_1 : f32, @location(3) @interpolate(perspective, sample) x_1_param_2 : f32, @location(4) @interpolate(linear) x_1_param_3 : f32, @location(5) @interpolate(linear, centroid) x_1_param_4 : f32, @location(6) @interpolate(linear, sample) x_1_param_5 : f32) {
  x_1.field0 = x_1_param;
  x_1.field1 = x_1_param_1;
  x_1.field2 = x_1_param_2;
  x_1.field3 = x_1_param_3;
  x_1.field4 = x_1_param_4;
  x_1.field5 = x_1_param_5;
  main_1();
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, EntryPointWrapping_Interpolation_Floating_Fragment_Out) {
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1 %2 %3 %4 %5 %6
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 Location 1
     OpDecorate %2 Location 2
     OpDecorate %3 Location 3
     OpDecorate %4 Location 4
     OpDecorate %5 Location 5
     OpDecorate %6 Location 6

     ; %1 perspective center

     OpDecorate %2 Centroid ; perspective centroid

     OpDecorate %3 Sample ; perspective sample

     OpDecorate %4 NoPerspective; linear center

     OpDecorate %5 NoPerspective ; linear centroid
     OpDecorate %5 Centroid

     OpDecorate %6 NoPerspective ; linear sample
     OpDecorate %6 Sample

)" + CommonTypes() +
                          R"(
     %ptr_out_float = OpTypePointer Output %float
     %1 = OpVariable %ptr_out_float Output
     %2 = OpVariable %ptr_out_float Output
     %3 = OpVariable %ptr_out_float Output
     %4 = OpVariable %ptr_out_float Output
     %5 = OpVariable %ptr_out_float Output
     %6 = OpVariable %ptr_out_float Output

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected =
        R"(var<private> x_1 : f32;

var<private> x_2 : f32;

var<private> x_3 : f32;

var<private> x_4 : f32;

var<private> x_5 : f32;

var<private> x_6 : f32;

fn main_1() {
  return;
}

struct main_out {
  @location(1)
  x_1_1 : f32,
  @location(2) @interpolate(perspective, centroid)
  x_2_1 : f32,
  @location(3) @interpolate(perspective, sample)
  x_3_1 : f32,
  @location(4) @interpolate(linear)
  x_4_1 : f32,
  @location(5) @interpolate(linear, centroid)
  x_5_1 : f32,
  @location(6) @interpolate(linear, sample)
  x_6_1 : f32,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_1, x_2, x_3, x_4, x_5, x_6);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_Flatten_Interpolation_Floating_Fragment_Out) {
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1
     OpExecutionMode %main OriginUpperLeft

     OpDecorate %1 Location 1

     ; member 0 perspective center

     OpMemberDecorate %10 1 Centroid ; perspective centroid

     OpMemberDecorate %10 2 Sample ; perspective sample

     OpMemberDecorate %10 3 NoPerspective; linear center

     OpMemberDecorate %10 4 NoPerspective ; linear centroid
     OpMemberDecorate %10 4 Centroid

     OpMemberDecorate %10 5 NoPerspective ; linear sample
     OpMemberDecorate %10 5 Sample

)" + CommonTypes() +
                          R"(

     %10 = OpTypeStruct %float %float %float %float %float %float
     %ptr_in_strct = OpTypePointer Output %10
     %1 = OpVariable %ptr_in_strct Output

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected =
        R"(struct S {
  field0 : f32,
  field1 : f32,
  field2 : f32,
  field3 : f32,
  field4 : f32,
  field5 : f32,
}

var<private> x_1 : S;

fn main_1() {
  return;
}

struct main_out {
  @location(1)
  x_1_1 : f32,
  @location(2) @interpolate(perspective, centroid)
  x_1_2 : f32,
  @location(3) @interpolate(perspective, sample)
  x_1_3 : f32,
  @location(4) @interpolate(linear)
  x_1_4 : f32,
  @location(5) @interpolate(linear, centroid)
  x_1_5 : f32,
  @location(6) @interpolate(linear, sample)
  x_1_6 : f32,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_1.field0, x_1.field1, x_1.field2, x_1.field3, x_1.field4, x_1.field5);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, EntryPointWrapping_Interpolation_Default_Vertex_Output) {
    // Integral types default to @interpolate(flat).
    // Floating types default to @interpolate(perspective, center), which is the
    // same as WGSL and therefore dropped.
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Vertex %main "main" %1 %2 %3 %4 %5 %6 %10
     OpDecorate %1 Location 1
     OpDecorate %2 Location 2
     OpDecorate %3 Location 3
     OpDecorate %4 Location 4
     OpDecorate %5 Location 5
     OpDecorate %6 Location 6
     OpDecorate %10 BuiltIn Position
)" + CommonTypes() +
                          R"(
     %ptr_out_uint = OpTypePointer Output %uint
     %ptr_out_v2uint = OpTypePointer Output %v2uint
     %ptr_out_int = OpTypePointer Output %int
     %ptr_out_v2int = OpTypePointer Output %v2int
     %ptr_out_float = OpTypePointer Output %float
     %ptr_out_v2float = OpTypePointer Output %v2float
     %1 = OpVariable %ptr_out_uint Output
     %2 = OpVariable %ptr_out_v2uint Output
     %3 = OpVariable %ptr_out_int Output
     %4 = OpVariable %ptr_out_v2int Output
     %5 = OpVariable %ptr_out_float Output
     %6 = OpVariable %ptr_out_v2float Output

     %ptr_out_v4float = OpTypePointer Output %v4float
     %10 = OpVariable %ptr_out_v4float Output

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected =
        R"(var<private> x_1 : u32;

var<private> x_2 : vec2u;

var<private> x_3 : i32;

var<private> x_4 : vec2i;

var<private> x_5 : f32;

var<private> x_6 : vec2f;

var<private> x_10 : vec4f;

fn main_1() {
  return;
}

struct main_out {
  @location(1) @interpolate(flat)
  x_1_1 : u32,
  @location(2) @interpolate(flat)
  x_2_1 : vec2u,
  @location(3) @interpolate(flat)
  x_3_1 : i32,
  @location(4) @interpolate(flat)
  x_4_1 : vec2i,
  @location(5)
  x_5_1 : f32,
  @location(6)
  x_6_1 : vec2f,
  @builtin(position)
  x_10_1 : vec4f,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(x_1, x_2, x_3, x_4, x_5, x_6, x_10);
}
)";
    EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, EntryPointWrapping_Interpolation_Default_Fragment_In) {
    // Integral types default to @interpolate(flat).
    // Floating types default to @interpolate(perspective, center), which is the
    // same as WGSL and therefore dropped.
    const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1 %2 %3 %4 %5 %6
     OpDecorate %1 Location 1
     OpDecorate %2 Location 2
     OpDecorate %3 Location 3
     OpDecorate %4 Location 4
     OpDecorate %5 Location 5
     OpDecorate %6 Location 6
)" + CommonTypes() +
                          R"(
     %ptr_in_uint = OpTypePointer Input %uint
     %ptr_in_v2uint = OpTypePointer Input %v2uint
     %ptr_in_int = OpTypePointer Input %int
     %ptr_in_v2int = OpTypePointer Input %v2int
     %ptr_in_float = OpTypePointer Input %float
     %ptr_in_v2float = OpTypePointer Input %v2float
     %1 = OpVariable %ptr_in_uint Input
     %2 = OpVariable %ptr_in_v2uint Input
     %3 = OpVariable %ptr_in_int Input
     %4 = OpVariable %ptr_in_v2int Input
     %5 = OpVariable %ptr_in_float Input
     %6 = OpVariable %ptr_in_v2float Input

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));

    ASSERT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program());
    const std::string expected =
        R"(var<private> x_1 : u32;

var<private> x_2 : vec2u;

var<private> x_3 : i32;

var<private> x_4 : vec2i;

var<private> x_5 : f32;

var<private> x_6 : vec2f;

fn main_1() {
  return;
}

@fragment
fn main(@location(1) @interpolate(flat) x_1_param : u32, @location(2) @interpolate(flat) x_2_param : vec2u, @location(3) @interpolate(flat) x_3_param : i32, @location(4) @interpolate(flat) x_4_param : vec2i, @location(5) x_5_param : f32, @location(6) x_6_param : vec2f) {
  x_1 = x_1_param;
  x_2 = x_2_param;
  x_3 = x_3_param;
  x_4 = x_4_param;
  x_5 = x_5_param;
  x_6 = x_6_param;
  main_1();
}
)";
    EXPECT_EQ(got, expected) << got;
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
