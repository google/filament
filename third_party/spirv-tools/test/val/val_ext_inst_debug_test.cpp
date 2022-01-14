// Copyright (c) 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Tests validation rules of GLSL.450.std and OpenCL.std extended instructions.
// Doesn't test OpenCL.std vector size 2, 3, 4, 8 or 16 rules (not supported
// by standard SPIR-V).

#include <sstream>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Not;

using ValidateOldDebugInfo = spvtest::ValidateBase<std::string>;
using ValidateOpenCL100DebugInfo = spvtest::ValidateBase<std::string>;
using ValidateXDebugInfo = spvtest::ValidateBase<std::string>;
using ValidateLocalDebugInfoOutOfFunction = spvtest::ValidateBase<std::string>;
using ValidateOpenCL100DebugInfoDebugTypedef =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateVulkan100DebugInfoDebugTypedef =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugTypeEnum =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateVulkan100DebugInfoDebugTypeEnum =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugTypeComposite =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateVulkan100DebugInfoDebugTypeComposite =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugTypeMember =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateVulkan100DebugInfoDebugTypeMember =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugTypeInheritance =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugFunction =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateVulkan100DebugInfoDebugFunction =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugFunctionDeclaration =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateVulkan100DebugInfoDebugFunctionDeclaration =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugLexicalBlock =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateVulkan100DebugInfoDebugLexicalBlock =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugLocalVariable =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateVulkan100DebugInfoDebugLocalVariable =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugGlobalVariable =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateVulkan100DebugInfoDebugGlobalVariable =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugDeclare =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateVulkan100DebugInfoDebugDeclare =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugValue =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateVulkan100DebugInfoDebugValue =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateVulkan100DebugInfo = spvtest::ValidateBase<std::string>;

std::string GenerateShaderCodeForDebugInfo(
    const std::string& op_string_instructions,
    const std::string& op_const_instructions,
    const std::string& debug_instructions_before_main, const std::string& body,
    const std::string& capabilities_and_extensions = "",
    const std::string& execution_model = "Fragment") {
  std::ostringstream ss;
  ss << R"(
OpCapability Shader
OpCapability Float16
OpCapability Float64
OpCapability Int16
OpCapability Int64
)";

  ss << capabilities_and_extensions;
  ss << "%extinst = OpExtInstImport \"GLSL.std.450\"\n";
  ss << "OpMemoryModel Logical GLSL450\n";
  ss << "OpEntryPoint " << execution_model << " %main \"main\""
     << " %f32_output"
     << " %f32vec2_output"
     << " %u32_output"
     << " %u32vec2_output"
     << " %u64_output"
     << " %f32_input"
     << " %f32vec2_input"
     << " %u32_input"
     << " %u32vec2_input"
     << " %u64_input"
     << "\n";
  if (execution_model == "Fragment") {
    ss << "OpExecutionMode %main OriginUpperLeft\n";
  }

  ss << op_string_instructions;

  ss << R"(
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f16 = OpTypeFloat 16
%f32 = OpTypeFloat 32
%f64 = OpTypeFloat 64
%u32 = OpTypeInt 32 0
%s32 = OpTypeInt 32 1
%u64 = OpTypeInt 64 0
%s64 = OpTypeInt 64 1
%u16 = OpTypeInt 16 0
%s16 = OpTypeInt 16 1
%f32vec2 = OpTypeVector %f32 2
%f32vec3 = OpTypeVector %f32 3
%f32vec4 = OpTypeVector %f32 4
%f64vec2 = OpTypeVector %f64 2
%f64vec3 = OpTypeVector %f64 3
%f64vec4 = OpTypeVector %f64 4
%u32vec2 = OpTypeVector %u32 2
%u32vec3 = OpTypeVector %u32 3
%s32vec2 = OpTypeVector %s32 2
%u32vec4 = OpTypeVector %u32 4
%s32vec4 = OpTypeVector %s32 4
%u64vec2 = OpTypeVector %u64 2
%s64vec2 = OpTypeVector %s64 2
%f64mat22 = OpTypeMatrix %f64vec2 2
%f32mat22 = OpTypeMatrix %f32vec2 2
%f32mat23 = OpTypeMatrix %f32vec2 3
%f32mat32 = OpTypeMatrix %f32vec3 2
%f32mat33 = OpTypeMatrix %f32vec3 3

%f32_0 = OpConstant %f32 0
%f32_1 = OpConstant %f32 1
%f32_2 = OpConstant %f32 2
%f32_3 = OpConstant %f32 3
%f32_4 = OpConstant %f32 4
%f32_h = OpConstant %f32 0.5
%f32vec2_01 = OpConstantComposite %f32vec2 %f32_0 %f32_1
%f32vec2_12 = OpConstantComposite %f32vec2 %f32_1 %f32_2
%f32vec3_012 = OpConstantComposite %f32vec3 %f32_0 %f32_1 %f32_2
%f32vec3_123 = OpConstantComposite %f32vec3 %f32_1 %f32_2 %f32_3
%f32vec4_0123 = OpConstantComposite %f32vec4 %f32_0 %f32_1 %f32_2 %f32_3
%f32vec4_1234 = OpConstantComposite %f32vec4 %f32_1 %f32_2 %f32_3 %f32_4

%f64_0 = OpConstant %f64 0
%f64_1 = OpConstant %f64 1
%f64_2 = OpConstant %f64 2
%f64_3 = OpConstant %f64 3
%f64vec2_01 = OpConstantComposite %f64vec2 %f64_0 %f64_1
%f64vec3_012 = OpConstantComposite %f64vec3 %f64_0 %f64_1 %f64_2
%f64vec4_0123 = OpConstantComposite %f64vec4 %f64_0 %f64_1 %f64_2 %f64_3

%f16_0 = OpConstant %f16 0
%f16_1 = OpConstant %f16 1
%f16_h = OpConstant %f16 0.5

%u32_0 = OpConstant %u32 0
%u32_1 = OpConstant %u32 1
%u32_2 = OpConstant %u32 2
%u32_3 = OpConstant %u32 3

%s32_0 = OpConstant %s32 0
%s32_1 = OpConstant %s32 1
%s32_2 = OpConstant %s32 2
%s32_3 = OpConstant %s32 3

%u64_0 = OpConstant %u64 0
%u64_1 = OpConstant %u64 1
%u64_2 = OpConstant %u64 2
%u64_3 = OpConstant %u64 3

%s64_0 = OpConstant %s64 0
%s64_1 = OpConstant %s64 1
%s64_2 = OpConstant %s64 2
%s64_3 = OpConstant %s64 3
)";

  ss << op_const_instructions;

  ss << R"(
%s32vec2_01 = OpConstantComposite %s32vec2 %s32_0 %s32_1
%u32vec2_01 = OpConstantComposite %u32vec2 %u32_0 %u32_1

%s32vec2_12 = OpConstantComposite %s32vec2 %s32_1 %s32_2
%u32vec2_12 = OpConstantComposite %u32vec2 %u32_1 %u32_2

%s32vec4_0123 = OpConstantComposite %s32vec4 %s32_0 %s32_1 %s32_2 %s32_3
%u32vec4_0123 = OpConstantComposite %u32vec4 %u32_0 %u32_1 %u32_2 %u32_3

%s64vec2_01 = OpConstantComposite %s64vec2 %s64_0 %s64_1
%u64vec2_01 = OpConstantComposite %u64vec2 %u64_0 %u64_1

%f32mat22_1212 = OpConstantComposite %f32mat22 %f32vec2_12 %f32vec2_12
%f32mat23_121212 = OpConstantComposite %f32mat23 %f32vec2_12 %f32vec2_12 %f32vec2_12

%f32_ptr_output = OpTypePointer Output %f32
%f32vec2_ptr_output = OpTypePointer Output %f32vec2

%u32_ptr_output = OpTypePointer Output %u32
%u32vec2_ptr_output = OpTypePointer Output %u32vec2

%u64_ptr_output = OpTypePointer Output %u64

%f32_output = OpVariable %f32_ptr_output Output
%f32vec2_output = OpVariable %f32vec2_ptr_output Output

%u32_output = OpVariable %u32_ptr_output Output
%u32vec2_output = OpVariable %u32vec2_ptr_output Output

%u64_output = OpVariable %u64_ptr_output Output

%f32_ptr_input = OpTypePointer Input %f32
%f32vec2_ptr_input = OpTypePointer Input %f32vec2

%u32_ptr_input = OpTypePointer Input %u32
%u32vec2_ptr_input = OpTypePointer Input %u32vec2

%u64_ptr_input = OpTypePointer Input %u64

%f32_ptr_function = OpTypePointer Function %f32

%f32_input = OpVariable %f32_ptr_input Input
%f32vec2_input = OpVariable %f32vec2_ptr_input Input

%u32_input = OpVariable %u32_ptr_input Input
%u32vec2_input = OpVariable %u32vec2_ptr_input Input

%u64_input = OpVariable %u64_ptr_input Input

%u32_ptr_function = OpTypePointer Function %u32

%struct_f16_u16 = OpTypeStruct %f16 %u16
%struct_f32_f32 = OpTypeStruct %f32 %f32
%struct_f32_f32_f32 = OpTypeStruct %f32 %f32 %f32
%struct_f32_u32 = OpTypeStruct %f32 %u32
%struct_f32_u32_f32 = OpTypeStruct %f32 %u32 %f32
%struct_u32_f32 = OpTypeStruct %u32 %f32
%struct_u32_u32 = OpTypeStruct %u32 %u32
%struct_f32_f64 = OpTypeStruct %f32 %f64
%struct_f32vec2_f32vec2 = OpTypeStruct %f32vec2 %f32vec2
%struct_f32vec2_u32vec2 = OpTypeStruct %f32vec2 %u32vec2
)";

  ss << debug_instructions_before_main;

  ss << R"(
%main = OpFunction %void None %func
%main_entry = OpLabel
)";

  ss << body;

  ss << R"(
OpReturn
OpFunctionEnd)";

  return ss.str();
}

TEST_F(ValidateOldDebugInfo, UseDebugInstructionOutOfFunction) {
  const std::string src = R"(
%code = OpString "main() {}"
)";

  const std::string dbg_inst = R"(
%cu = OpExtInst %void %DbgExt DebugCompilationUnit %code 1 1
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "DebugInfo"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", dbg_inst, "",
                                                     extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, UseDebugInstructionOutOfFunction) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
)";

  const std::string dbg_inst = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", dbg_inst, "",
                                                     extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugSourceInFunction) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
)";

  const std::string dbg_inst = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", "", dbg_inst,
                                                     extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Debug info extension instructions other than DebugScope, "
                "DebugNoScope, DebugDeclare, DebugValue must appear between "
                "section 9 (types, constants, global variables) and section 10 "
                "(function declarations)"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugSourceInFunction) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
)";

  const std::string dbg_inst = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", "", dbg_inst,
                                                     extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Debug info extension instructions other than DebugScope, "
                "DebugNoScope, DebugDeclare, DebugValue must appear between "
                "section 9 (types, constants, global variables) and section 10 "
                "(function declarations)"));
}

TEST_P(ValidateLocalDebugInfoOutOfFunction, OpenCLDebugInfo100DebugScope) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() {}"
%void_name = OpString "void"
%main_name = OpString "main"
%main_linkage_name = OpString "v_main"
%int_name = OpString "int"
%foo_name = OpString "foo"
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%int_info = OpExtInst %void %DbgExt DebugTypeBasic %int_name %u32_0 Signed
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %void
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src 1 1 %comp_unit %main_linkage_name FlagIsPublic 1 %main
%foo_info = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %int_info %dbg_src 1 1 %main_info FlagIsLocal
%expr = OpExtInst %void %DbgExt DebugExpression
)";

  const std::string body = R"(
%foo = OpVariable %u32_ptr_function Function
%foo_val = OpLoad %u32 %foo
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, "", dbg_inst_header + GetParam(), body, extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("DebugScope, DebugNoScope, DebugDeclare, DebugValue "
                        "of debug info extension must appear in a function "
                        "body"));
}

TEST_P(ValidateLocalDebugInfoOutOfFunction, VulkanDebugInfo100DebugScope) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() {}"
%void_name = OpString "void"
%main_name = OpString "main"
%main_linkage_name = OpString "v_main"
%int_name = OpString "int"
%foo_name = OpString "foo"
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%int_info = OpExtInst %void %DbgExt DebugTypeBasic %int_name %u32_0 %u32_1 %u32_0
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction %u32_3 %void
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src %u32_1 %u32_1 %comp_unit %main_linkage_name %u32_3 %u32_1
%foo_info = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %int_info %dbg_src %u32_1 %u32_1 %main_info %u32_4
%expr = OpExtInst %void %DbgExt DebugExpression
)";

  const std::string body = R"(
%foo = OpVariable %u32_ptr_function Function
%main_def = OpExtInst %void %DbgExt DebugFunctionDefinition %main_info %main
%foo_val = OpLoad %u32 %foo
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header + GetParam(), body, extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("DebugScope, DebugNoScope, DebugDeclare, DebugValue "
                        "of debug info extension must appear in a function "
                        "body"));
}

INSTANTIATE_TEST_SUITE_P(
    AllLocalDebugInfo, ValidateLocalDebugInfoOutOfFunction,
    ::testing::ValuesIn(std::vector<std::string>{
        "%main_scope = OpExtInst %void %DbgExt DebugScope %main_info",
        "%no_scope = OpExtInst %void %DbgExt DebugNoScope",
    }));

TEST_F(ValidateOpenCL100DebugInfo, DebugFunctionForwardReference) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() {}"
%void_name = OpString "void"
%main_name = OpString "main"
%main_linkage_name = OpString "v_main"
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %void
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src 1 1 %comp_unit %main_linkage_name FlagIsPublic 1 %main
)";

  const std::string body = R"(
%main_scope = OpExtInst %void %DbgExt DebugScope %main_info
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, "", dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugFunctionMissingOpFunction) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() {}"
%void_name = OpString "void"
%main_name = OpString "main"
%main_linkage_name = OpString "v_main"
)";

  const std::string dbg_inst_header = R"(
%dbgNone = OpExtInst %void %DbgExt DebugInfoNone
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %void
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src 1 1 %comp_unit %main_linkage_name FlagIsPublic 1 %dbgNone
)";

  const std::string body = R"(
%main_scope = OpExtInst %void %DbgExt DebugScope %main_info
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, "", dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugScopeBeforeOpVariableInFunction) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float4 main(float arg) {
  float foo;
  return float4(0, 0, 0, 0);
}
"
%float_name = OpString "float"
%main_name = OpString "main"
%main_linkage_name = OpString "v4f_main_f"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%v4float_info = OpExtInst %void %DbgExt DebugTypeVector %float_info 4
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %v4float_info %float_info
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src 12 1 %comp_unit %main_linkage_name FlagIsPublic 13 %main
)";

  const std::string body = R"(
%main_scope = OpExtInst %void %DbgExt DebugScope %main_info
%foo = OpVariable %f32_ptr_function Function
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeCompositeSizeDebugInfoNone) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "OpaqueType foo;
main() {}
"
%ty_name = OpString "struct VS_OUTPUT"
)";

  const std::string dbg_inst_header = R"(
%dbg_none = OpExtInst %void %DbgExt DebugInfoNone
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%opaque = OpExtInst %void %DbgExt DebugTypeComposite %ty_name Class %dbg_src 1 1 %comp_unit %ty_name %dbg_none FlagIsPublic
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", dbg_inst_header,
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeCompositeForwardReference) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "struct VS_OUTPUT {
  float4 pos : SV_POSITION;
  float4 color : COLOR;
};
main() {}
"
%VS_OUTPUT_name = OpString "struct VS_OUTPUT"
%float_name = OpString "float"
%VS_OUTPUT_pos_name = OpString "pos : SV_POSITION"
%VS_OUTPUT_color_name = OpString "color : COLOR"
%VS_OUTPUT_linkage_name = OpString "VS_OUTPUT"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
%int_128 = OpConstant %u32 128
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%VS_OUTPUT_info = OpExtInst %void %DbgExt DebugTypeComposite %VS_OUTPUT_name Structure %dbg_src 1 1 %comp_unit %VS_OUTPUT_linkage_name %int_128 FlagIsPublic %VS_OUTPUT_pos_info %VS_OUTPUT_color_info
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%v4float_info = OpExtInst %void %DbgExt DebugTypeVector %float_info 4
%VS_OUTPUT_pos_info = OpExtInst %void %DbgExt DebugTypeMember %VS_OUTPUT_pos_name %v4float_info %dbg_src 2 3 %VS_OUTPUT_info %u32_0 %int_128 FlagIsPublic
%VS_OUTPUT_color_info = OpExtInst %void %DbgExt DebugTypeMember %VS_OUTPUT_color_name %v4float_info %dbg_src 3 3 %VS_OUTPUT_info %int_128 %int_128 FlagIsPublic
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeCompositeMissingReference) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "struct VS_OUTPUT {
  float4 pos : SV_POSITION;
  float4 color : COLOR;
};
main() {}
"
%VS_OUTPUT_name = OpString "struct VS_OUTPUT"
%float_name = OpString "float"
%VS_OUTPUT_pos_name = OpString "pos : SV_POSITION"
%VS_OUTPUT_color_name = OpString "color : COLOR"
%VS_OUTPUT_linkage_name = OpString "VS_OUTPUT"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
%int_128 = OpConstant %u32 128
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%VS_OUTPUT_info = OpExtInst %void %DbgExt DebugTypeComposite %VS_OUTPUT_name Structure %dbg_src 1 1 %comp_unit %VS_OUTPUT_linkage_name %int_128 FlagIsPublic %VS_OUTPUT_pos_info %VS_OUTPUT_color_info
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%v4float_info = OpExtInst %void %DbgExt DebugTypeVector %float_info 4
%VS_OUTPUT_pos_info = OpExtInst %void %DbgExt DebugTypeMember %VS_OUTPUT_pos_name %v4float_info %dbg_src 2 3 %VS_OUTPUT_info %u32_0 %int_128 FlagIsPublic
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("forward referenced IDs have not been defined"));
}

TEST_P(ValidateXDebugInfo, DebugSourceWrongResultType) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
)";

  const std::string dbg_inst = R"(
%dbg_src = OpExtInst %bool %DbgExt DebugSource %src %code
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", dbg_inst, "",
                                                     GetParam(), "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected result type must be a result id of "
                        "OpTypeVoid"));
}

TEST_P(ValidateXDebugInfo, DebugSourceFailFile) {
  const std::string src = R"(
%code = OpString "main() {}"
)";

  const std::string dbg_inst = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %DbgExt %code
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", dbg_inst, "",
                                                     GetParam(), "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand File must be a result id of "
                        "OpString"));
}

TEST_P(ValidateXDebugInfo, DebugSourceFailSource) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
)";

  const std::string dbg_inst = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %DbgExt
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", dbg_inst, "",
                                                     GetParam(), "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Text must be a result id of "
                        "OpString"));
}

TEST_P(ValidateXDebugInfo, DebugSourceNoText) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
)";

  const std::string dbg_inst = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", dbg_inst, "",
                                                     GetParam(), "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

INSTANTIATE_TEST_SUITE_P(OpenCLAndVkDebugInfo100, ValidateXDebugInfo,
                         ::testing::ValuesIn(std::vector<std::string>{
                             R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)",
                             R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)",
                         }));

TEST_F(ValidateOpenCL100DebugInfo, DebugCompilationUnit) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
)";

  const std::string dbg_inst = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", dbg_inst, "",
                                                     extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugCompilationUnitFail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
)";

  const std::string dbg_inst = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %src HLSL
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", dbg_inst, "",
                                                     extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Source must be a result id of "
                        "DebugSource"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugCompilationUnitFail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
)";

  const std::string dbg_inst = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %src %u32_5
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, constants, dbg_inst,
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Source must be a result id of "
                        "DebugSource"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeBasicFailName) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float4 main(float arg) {
  float foo;
  return float4(0, 0, 0, 0);
}
"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %int_32 %int_32 Float
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Name must be a result id of "
                        "OpString"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeBasicFailName) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float4 main(float arg) {
  float foo;
  return float4(0, 0, 0, 0);
}
"
%float_name = OpString "float"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %u32_32 %u32_32 %u32_3 %u32_0
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Name must be a result id of "
                        "OpString"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeBasicFailSize) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float4 main(float arg) {
  float foo;
  return float4(0, 0, 0, 0);
}
"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %float_name Float
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Size must be a result id of "
                        "OpConstant"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeBasicFailSize) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float4 main(float arg) {
  float foo;
  return float4(0, 0, 0, 0);
}
"
%float_name = OpString "float"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %float_name %u32_3 %u32_0
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Size must be a result id of "
                        "OpConstant"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypePointer) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float4 main(float arg) {
  float foo;
  return float4(0, 0, 0, 0);
}
"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%pfloat_info = OpExtInst %void %DbgExt DebugTypePointer %float_info Function FlagIsLocal
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypePointerFail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float4 main(float arg) {
  float foo;
  return float4(0, 0, 0, 0);
}
"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%pfloat_info = OpExtInst %void %DbgExt DebugTypePointer %dbg_src Function FlagIsLocal
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Base Type must be a result id of "
                        "DebugTypeBasic"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeQualifier) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float4 main(float arg) {
  float foo;
  return float4(0, 0, 0, 0);
}
"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%cfloat_info = OpExtInst %void %DbgExt DebugTypeQualifier %float_info ConstType
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeQualifierFail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float4 main(float arg) {
  float foo;
  return float4(0, 0, 0, 0);
}
"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%cfloat_info = OpExtInst %void %DbgExt DebugTypeQualifier %comp_unit ConstType
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Base Type must be a result id of "
                        "DebugTypeBasic"));
}
TEST_F(ValidateVulkan100DebugInfo, DebugTypeQualifier) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float4 main(float arg) {
  float foo;
  return float4(0, 0, 0, 0);
}
"
%float_name = OpString "float"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%cfloat_info = OpExtInst %void %DbgExt DebugTypeQualifier %float_info %u32_0
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeQualifierFail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float4 main(float arg) {
  float foo;
  return float4(0, 0, 0, 0);
}
"
%float_name = OpString "float"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%cfloat_info = OpExtInst %void %DbgExt DebugTypeQualifier %comp_unit %u32_0
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Base Type must be a result id of "
                        "DebugTypeBasic"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeArray) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%float_arr_info = OpExtInst %void %DbgExt DebugTypeArray %float_info %int_32
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeArrayWithVariableSize) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
%int_name = OpString "int"
%main_name = OpString "main"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%uint_info = OpExtInst %void %DbgExt DebugTypeBasic %int_name %int_32 Unsigned
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %void
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src 1 1 %comp_unit %main_name FlagIsPublic 1 %main
%foo_info = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %uint_info %dbg_src 1 1 %main_info FlagIsLocal
%float_arr_info = OpExtInst %void %DbgExt DebugTypeArray %float_info %foo_info
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeArrayFailBaseType) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%float_arr_info = OpExtInst %void %DbgExt DebugTypeArray %comp_unit %int_32
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Base Type is not a valid debug "
                        "type"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeArrayFailComponentCount) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%float_arr_info = OpExtInst %void %DbgExt DebugTypeArray %float_info %float_info
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Component Count must be OpConstant with a 32- or "
                        "64-bits integer scalar type or DebugGlobalVariable or "
                        "DebugLocalVariable with a 32- or 64-bits unsigned "
                        "integer scalar type"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeArrayFailComponentCountFloat) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%float_arr_info = OpExtInst %void %DbgExt DebugTypeArray %float_info %f32_4
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Component Count must be OpConstant with a 32- or "
                        "64-bits integer scalar type or DebugGlobalVariable or "
                        "DebugLocalVariable with a 32- or 64-bits unsigned "
                        "integer scalar type"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeArrayFailComponentCountZero) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%float_arr_info = OpExtInst %void %DbgExt DebugTypeArray %float_info %u32_0
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Component Count must be OpConstant with a 32- or "
                        "64-bits integer scalar type or DebugGlobalVariable or "
                        "DebugLocalVariable with a 32- or 64-bits unsigned "
                        "integer scalar type"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeArrayFailVariableSizeTypeFloat) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
%main_name = OpString "main"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %void
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src 1 1 %comp_unit %main_name FlagIsPublic 1 %main
%foo_info = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %float_info %dbg_src 1 1 %main_info FlagIsLocal
%float_arr_info = OpExtInst %void %DbgExt DebugTypeArray %float_info %foo_info
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Component Count must be OpConstant with a 32- or "
                        "64-bits integer scalar type or DebugGlobalVariable or "
                        "DebugLocalVariable with a 32- or 64-bits unsigned "
                        "integer scalar type"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeArray) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%float_arr_info = OpExtInst %void %DbgExt DebugTypeArray %float_info %u32_32
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeArrayWithVariableSize) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
%uint_name = OpString "uint"
%main_name = OpString "main"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_6 = OpConstant %u32 6
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%uint_info = OpExtInst %void %DbgExt DebugTypeBasic %uint_name %u32_32 %u32_6 %u32_0
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction %u32_3 %void
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src %u32_1 %u32_1 %comp_unit %main_name %u32_3 %u32_1
%foo_info = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %uint_info %dbg_src %u32_1 %u32_1 %main_info %u32_4
%float_arr_info = OpExtInst %void %DbgExt DebugTypeArray %float_info %foo_info
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeArrayFailBaseType) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%float_arr_info = OpExtInst %void %DbgExt DebugTypeArray %comp_unit %u32_32
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Base Type is not a valid debug "
                        "type"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeArrayFailComponentCount) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%float_arr_info = OpExtInst %void %DbgExt DebugTypeArray %float_info %float_info
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Component Count must be OpConstant with a 32- or "
                        "64-bits integer scalar type or DebugGlobalVariable or "
                        "DebugLocalVariable with a 32- or 64-bits unsigned "
                        "integer scalar type"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeArrayFailComponentCountFloat) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%float_arr_info = OpExtInst %void %DbgExt DebugTypeArray %float_info %f32_4
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Component Count must be OpConstant with a 32- or "
                        "64-bits integer scalar type or DebugGlobalVariable or "
                        "DebugLocalVariable with a 32- or 64-bits unsigned "
                        "integer scalar type"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeArrayFailComponentCountZero) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%float_arr_info = OpExtInst %void %DbgExt DebugTypeArray %float_info %u32_0
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Component Count must be OpConstant with a 32- or "
                        "64-bits integer scalar type or DebugGlobalVariable or "
                        "DebugLocalVariable with a 32- or 64-bits unsigned "
                        "integer scalar type"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeArrayFailVariableSizeTypeFloat) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
%main_name = OpString "main"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_6 = OpConstant %u32 6
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction %u32_3 %void
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src %u32_1 %u32_1 %comp_unit %main_name %u32_3 %u32_1
%foo_info = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %float_info %dbg_src %u32_1 %u32_1 %main_info %u32_4
%float_arr_info = OpExtInst %void %DbgExt DebugTypeArray %float_info %foo_info
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Component Count must be OpConstant with a 32- or "
                        "64-bits integer scalar type or DebugGlobalVariable or "
                        "DebugLocalVariable with a 32- or 64-bits unsigned "
                        "integer scalar type"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeVector) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%vfloat_info = OpExtInst %void %DbgExt DebugTypeVector %float_info 4
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeVectorFail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%vfloat_info = OpExtInst %void %DbgExt DebugTypeVector %dbg_src 4
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Base Type must be a result id of "
                        "DebugTypeBasic"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeVectorFailComponentZero) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%vfloat_info = OpExtInst %void %DbgExt DebugTypeVector %dbg_src 0
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Base Type must be a result id of "
                        "DebugTypeBasic"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeVectorFailComponentFive) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%vfloat_info = OpExtInst %void %DbgExt DebugTypeVector %dbg_src 5
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Base Type must be a result id of "
                        "DebugTypeBasic"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeVector) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%vfloat_info = OpExtInst %void %DbgExt DebugTypeVector %float_info %u32_4
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeVectorFail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%vfloat_info = OpExtInst %void %DbgExt DebugTypeVector %dbg_src %u32_4
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Base Type must be a result id of "
                        "DebugTypeBasic"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeVectorFailComponentZero) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%vfloat_info = OpExtInst %void %DbgExt DebugTypeVector %float_info %u32_0
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Component Count must be positive "
                        "integer less than or equal to 4"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeVectorFailComponentFive) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%vfloat_info = OpExtInst %void %DbgExt DebugTypeVector %float_info %u32_5
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Component Count must be positive "
                        "integer less than or equal to 4"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypedef) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%foo_info = OpExtInst %void %DbgExt DebugTypedef %foo_name %float_info %dbg_src 1 1 %comp_unit
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCL100DebugInfoDebugTypedef, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%foo_info = OpExtInst %void %DbgExt DebugTypedef )";
  ss << param.first;

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, size_const, ss.str(),
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second +
                        " must be a result id of "));
}

INSTANTIATE_TEST_SUITE_P(
    AllOpenCL100DebugInfoFail, ValidateOpenCL100DebugInfoDebugTypedef,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(R"(%dbg_src %float_info %dbg_src 1 1 %comp_unit)",
                       "Name"),
        std::make_pair(R"(%foo_name %dbg_src %dbg_src 1 1 %comp_unit)",
                       "Base Type"),
        std::make_pair(R"(%foo_name %float_info %comp_unit 1 1 %comp_unit)",
                       "Source"),
        std::make_pair(R"(%foo_name %float_info %dbg_src 1 1 %dbg_src)",
                       "Parent"),
    }));

TEST_F(ValidateVulkan100DebugInfo, DebugTypedef) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%foo_info = OpExtInst %void %DbgExt DebugTypedef %foo_name %float_info %dbg_src %u32_1 %u32_1 %comp_unit
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateVulkan100DebugInfoDebugTypedef, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%foo_info = OpExtInst %void %DbgExt DebugTypedef )";
  ss << param.first;

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, constants, ss.str(),
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second +
                        " must be a result id of "));
}

INSTANTIATE_TEST_SUITE_P(
    AllVulkan100DebugInfoFail, ValidateVulkan100DebugInfoDebugTypedef,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(
            R"(%dbg_src %float_info %dbg_src %u32_1 %u32_1 %comp_unit)",
            "Name"),
        std::make_pair(
            R"(%foo_name %dbg_src %dbg_src %u32_1 %u32_1 %comp_unit)",
            "Base Type"),
        std::make_pair(
            R"(%foo_name %float_info %comp_unit %u32_1 %u32_1 %comp_unit)",
            "Source"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src %u32_1 %u32_1 %dbg_src)",
            "Parent"),
    }));

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeFunction) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%main_name = OpString "main"
%main_linkage_name = OpString "v_main"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%main_type_info1 = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %void
%main_type_info2 = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %float_info
%main_type_info3 = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %float_info %float_info
%main_type_info4 = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %void %float_info %float_info
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeFunctionFailReturn) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%main_name = OpString "main"
%main_linkage_name = OpString "v_main"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %dbg_src %float_info
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("expected operand Return Type is not a valid debug type"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeFunctionFailParam) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%main_name = OpString "main"
%main_linkage_name = OpString "v_main"
%float_name = OpString "float"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %float_info %void
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("expected operand Parameter Types is not a valid debug type"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeFunctionAndParams) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%main_name = OpString "main"
%main_linkage_name = OpString "v_main"
%float_name = OpString "float"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%main_type_info1 = OpExtInst %void %DbgExt DebugTypeFunction %u32_3 %void
%main_type_info2 = OpExtInst %void %DbgExt DebugTypeFunction %u32_3 %float_info
%main_type_info3 = OpExtInst %void %DbgExt DebugTypeFunction %u32_3 %float_info %float_info
%main_type_info4 = OpExtInst %void %DbgExt DebugTypeFunction %u32_3 %void %float_info %float_info
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeFunctionFailReturn) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%main_name = OpString "main"
%main_linkage_name = OpString "v_main"
%float_name = OpString "float"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction %u32_3 %dbg_src %float_info
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("expected operand Return Type is not a valid debug type"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeFunctionFailParam) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%main_name = OpString "main"
%main_linkage_name = OpString "v_main"
%float_name = OpString "float"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction %u32_3 %float_info %void
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("expected operand Parameter Types is not a valid debug type"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeEnum) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%none = OpExtInst %void %DbgExt DebugInfoNone
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%foo_info1 = OpExtInst %void %DbgExt DebugTypeEnum %foo_name %float_info %dbg_src 1 1 %comp_unit %int_32 FlagIsPublic %u32_0 %foo_name %u32_1 %foo_name
%foo_info2 = OpExtInst %void %DbgExt DebugTypeEnum %foo_name %none %dbg_src 1 1 %comp_unit %int_32 FlagIsPublic %u32_0 %foo_name %u32_1 %foo_name
%foo_info3 = OpExtInst %void %DbgExt DebugTypeEnum %foo_name %none %dbg_src 1 1 %comp_unit %int_32 FlagIsPublic
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCL100DebugInfoDebugTypeEnum, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%foo_info = OpExtInst %void %DbgExt DebugTypeEnum )";
  ss << param.first;

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, size_const, ss.str(),
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllOpenCL100DebugInfoFail, ValidateOpenCL100DebugInfoDebugTypeEnum,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(
            R"(%dbg_src %float_info %dbg_src 1 1 %comp_unit %int_32 FlagIsPublic %u32_0 %foo_name)",
            "Name"),
        std::make_pair(
            R"(%foo_name %dbg_src %dbg_src 1 1 %comp_unit %int_32 FlagIsPublic %u32_0 %foo_name)",
            "Underlying Types"),
        std::make_pair(
            R"(%foo_name %float_info %comp_unit 1 1 %comp_unit %int_32 FlagIsPublic %u32_0 %foo_name)",
            "Source"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src 1 1 %dbg_src %int_32 FlagIsPublic %u32_0 %foo_name)",
            "Parent"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src 1 1 %comp_unit %void FlagIsPublic %u32_0 %foo_name)",
            "Size"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src 1 1 %comp_unit %u32_0 FlagIsPublic %u32_0 %foo_name)",
            "Size"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src 1 1 %comp_unit %int_32 FlagIsPublic %foo_name %foo_name)",
            "Value"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src 1 1 %comp_unit %int_32 FlagIsPublic %u32_0 %u32_1)",
            "Name"),
    }));

TEST_F(ValidateVulkan100DebugInfo, DebugTypeEnum) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%none = OpExtInst %void %DbgExt DebugInfoNone
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%foo_info1 = OpExtInst %void %DbgExt DebugTypeEnum %foo_name %float_info %dbg_src %u32_1 %u32_1 %comp_unit %u32_32 %u32_3 %u32_0 %foo_name %u32_1 %foo_name
%foo_info2 = OpExtInst %void %DbgExt DebugTypeEnum %foo_name %none %dbg_src %u32_1 %u32_1 %comp_unit %u32_32 %u32_3 %u32_0 %foo_name %u32_1 %foo_name
%foo_info3 = OpExtInst %void %DbgExt DebugTypeEnum %foo_name %none %dbg_src %u32_1 %u32_1 %comp_unit %u32_32 %u32_3
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateVulkan100DebugInfoDebugTypeEnum, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%foo_info = OpExtInst %void %DbgExt DebugTypeEnum )";
  ss << param.first;

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, constants, ss.str(),
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllVulkan100DebugInfoFail, ValidateVulkan100DebugInfoDebugTypeEnum,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(
            R"(%dbg_src %float_info %dbg_src %u32_1 %u32_1 %comp_unit %u32_32 %u32_3 %u32_0 %foo_name)",
            "Name"),
        std::make_pair(
            R"(%foo_name %dbg_src %dbg_src %u32_1 %u32_1 %comp_unit %u32_32 %u32_3 %u32_0 %foo_name)",
            "Underlying Types"),
        std::make_pair(
            R"(%foo_name %float_info %comp_unit %u32_1 %u32_1 %comp_unit %u32_32 %u32_3 %u32_0 %foo_name)",
            "Source"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src %u32_1 %u32_1 %dbg_src %u32_32 %u32_3 %u32_0 %foo_name)",
            "Parent"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src %u32_1 %u32_1 %comp_unit %void %u32_3 %u32_0 %foo_name)",
            "Size"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src %u32_1 %u32_1 %comp_unit %u32_0 %u32_3 %u32_0 %foo_name)",
            "Size"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src %u32_1 %u32_1 %comp_unit %u32_32 %u32_3 %foo_name %foo_name)",
            "Value"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src %u32_1 %u32_1 %comp_unit %u32_32 %u32_3 %u32_0 %u32_1)",
            "Name"),
    }));

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeCompositeFunctionAndInheritance) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "struct VS_OUTPUT {
  float4 pos : SV_POSITION;
};
struct foo : VS_OUTPUT {
};
main() {}
"
%VS_OUTPUT_name = OpString "struct VS_OUTPUT"
%float_name = OpString "float"
%foo_name = OpString "foo"
%VS_OUTPUT_pos_name = OpString "pos : SV_POSITION"
%VS_OUTPUT_linkage_name = OpString "VS_OUTPUT"
%main_name = OpString "main"
%main_linkage_name = OpString "v4f_main_f"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
%int_128 = OpConstant %u32 128
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%VS_OUTPUT_info = OpExtInst %void %DbgExt DebugTypeComposite %VS_OUTPUT_name Structure %dbg_src 1 1 %comp_unit %VS_OUTPUT_linkage_name %int_128 FlagIsPublic %VS_OUTPUT_pos_info %main_info %child
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%v4float_info = OpExtInst %void %DbgExt DebugTypeVector %float_info 4
%VS_OUTPUT_pos_info = OpExtInst %void %DbgExt DebugTypeMember %VS_OUTPUT_pos_name %v4float_info %dbg_src 2 3 %VS_OUTPUT_info %u32_0 %int_128 FlagIsPublic
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %v4float_info %float_info
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src 12 1 %comp_unit %main_linkage_name FlagIsPublic 13 %main
%foo_info = OpExtInst %void %DbgExt DebugTypeComposite %foo_name Structure %dbg_src 1 1 %comp_unit %foo_name %u32_0 FlagIsPublic
%child = OpExtInst %void %DbgExt DebugTypeInheritance %foo_info %VS_OUTPUT_info %int_128 %int_128 FlagIsPublic
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCL100DebugInfoDebugTypeComposite, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "struct VS_OUTPUT {
  float4 pos : SV_POSITION;
};
struct foo : VS_OUTPUT {
};
main() {}
"
%VS_OUTPUT_name = OpString "struct VS_OUTPUT"
%float_name = OpString "float"
%foo_name = OpString "foo"
%VS_OUTPUT_pos_name = OpString "pos : SV_POSITION"
%VS_OUTPUT_linkage_name = OpString "VS_OUTPUT"
%main_name = OpString "main"
%main_linkage_name = OpString "v4f_main_f"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
%int_128 = OpConstant %u32 128
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%VS_OUTPUT_info = OpExtInst %void %DbgExt DebugTypeComposite )";
  ss << param.first;
  ss << R"(
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%v4float_info = OpExtInst %void %DbgExt DebugTypeVector %float_info 4
%VS_OUTPUT_pos_info = OpExtInst %void %DbgExt DebugTypeMember %VS_OUTPUT_pos_name %v4float_info %dbg_src 2 3 %VS_OUTPUT_info %u32_0 %int_128 FlagIsPublic
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %v4float_info %float_info
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src 12 1 %comp_unit %main_linkage_name FlagIsPublic 13 %main
%foo_info = OpExtInst %void %DbgExt DebugTypeComposite %foo_name Structure %dbg_src 1 1 %comp_unit %foo_name %u32_0 FlagIsPublic
%child = OpExtInst %void %DbgExt DebugTypeInheritance %foo_info %VS_OUTPUT_info %int_128 %int_128 FlagIsPublic
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, size_const, ss.str(),
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second + " must be "));
}

INSTANTIATE_TEST_SUITE_P(
    AllOpenCL100DebugInfoFail, ValidateOpenCL100DebugInfoDebugTypeComposite,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(
            R"(%dbg_src Structure %dbg_src 1 1 %comp_unit %VS_OUTPUT_linkage_name %int_128 FlagIsPublic %VS_OUTPUT_pos_info %main_info %child)",
            "Name"),
        std::make_pair(
            R"(%VS_OUTPUT_name Structure %comp_unit 1 1 %comp_unit %VS_OUTPUT_linkage_name %int_128 FlagIsPublic %VS_OUTPUT_pos_info %main_info %child)",
            "Source"),
        std::make_pair(
            R"(%VS_OUTPUT_name Structure %dbg_src 1 1 %dbg_src %VS_OUTPUT_linkage_name %int_128 FlagIsPublic %VS_OUTPUT_pos_info %main_info %child)",
            "Parent"),
        std::make_pair(
            R"(%VS_OUTPUT_name Structure %dbg_src 1 1 %comp_unit %int_128 %int_128 FlagIsPublic %VS_OUTPUT_pos_info %main_info %child)",
            "Linkage Name"),
        std::make_pair(
            R"(%VS_OUTPUT_name Structure %dbg_src 1 1 %comp_unit %VS_OUTPUT_linkage_name %dbg_src FlagIsPublic %VS_OUTPUT_pos_info %main_info %child)",
            "Size"),
        std::make_pair(
            R"(%VS_OUTPUT_name Structure %dbg_src 1 1 %comp_unit %VS_OUTPUT_linkage_name %int_128 FlagIsPublic %dbg_src %main_info %child)",
            "Members"),
        std::make_pair(
            R"(%VS_OUTPUT_name Structure %dbg_src 1 1 %comp_unit %VS_OUTPUT_linkage_name %int_128 FlagIsPublic %VS_OUTPUT_pos_info %dbg_src %child)",
            "Members"),
        std::make_pair(
            R"(%VS_OUTPUT_name Structure %dbg_src 1 1 %comp_unit %VS_OUTPUT_linkage_name %int_128 FlagIsPublic %VS_OUTPUT_pos_info %main_info %dbg_src)",
            "Members"),
    }));

TEST_P(ValidateOpenCL100DebugInfoDebugTypeMember, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "struct VS_OUTPUT {
  float pos : SV_POSITION;
};
main() {}
"
%VS_OUTPUT_name = OpString "struct VS_OUTPUT"
%float_name = OpString "float"
%VS_OUTPUT_pos_name = OpString "pos : SV_POSITION"
%VS_OUTPUT_linkage_name = OpString "VS_OUTPUT"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%VS_OUTPUT_info = OpExtInst %void %DbgExt DebugTypeComposite %VS_OUTPUT_name Structure %dbg_src 1 1 %comp_unit %VS_OUTPUT_linkage_name %int_32 FlagIsPublic %VS_OUTPUT_pos_info
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%VS_OUTPUT_pos_info = OpExtInst %void %DbgExt DebugTypeMember )";
  ss << param.first;

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, size_const, ss.str(),
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  if (!param.second.empty()) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("expected operand " + param.second +
                          " must be a result id of "));
  }
}

INSTANTIATE_TEST_SUITE_P(
    AllOpenCL100DebugInfoFail, ValidateOpenCL100DebugInfoDebugTypeMember,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(
            R"(%dbg_src %float_info %dbg_src 2 3 %VS_OUTPUT_info %u32_0 %int_32 FlagIsPublic)",
            "Name"),
        std::make_pair(
            R"(%VS_OUTPUT_pos_name %dbg_src %dbg_src 2 3 %VS_OUTPUT_info %u32_0 %int_32 FlagIsPublic)",
            ""),
        std::make_pair(
            R"(%VS_OUTPUT_pos_name %float_info %float_info 2 3 %VS_OUTPUT_info %u32_0 %int_32 FlagIsPublic)",
            "Source"),
        std::make_pair(
            R"(%VS_OUTPUT_pos_name %float_info %dbg_src 2 3 %float_info %u32_0 %int_32 FlagIsPublic)",
            "Parent"),
        std::make_pair(
            R"(%VS_OUTPUT_pos_name %float_info %dbg_src 2 3 %VS_OUTPUT_info %void %int_32 FlagIsPublic)",
            "Offset"),
        std::make_pair(
            R"(%VS_OUTPUT_pos_name %float_info %dbg_src 2 3 %VS_OUTPUT_info %u32_0 %void FlagIsPublic)",
            "Size"),
    }));

TEST_P(ValidateOpenCL100DebugInfoDebugTypeInheritance, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "struct VS_OUTPUT {};
struct foo : VS_OUTPUT {};
"
%VS_OUTPUT_name = OpString "struct VS_OUTPUT"
%foo_name = OpString "foo"
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%VS_OUTPUT_info = OpExtInst %void %DbgExt DebugTypeComposite %VS_OUTPUT_name Structure %dbg_src 1 1 %comp_unit %VS_OUTPUT_name %u32_0 FlagIsPublic %child
%foo_info = OpExtInst %void %DbgExt DebugTypeComposite %foo_name Structure %dbg_src 1 1 %comp_unit %foo_name %u32_0 FlagIsPublic
%bar_info = OpExtInst %void %DbgExt DebugTypeComposite %foo_name Union %dbg_src 1 1 %comp_unit %foo_name %u32_0 FlagIsPublic
%child = OpExtInst %void %DbgExt DebugTypeInheritance )"
     << param.first;

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", ss.str(), "",
                                                     extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllOpenCL100DebugInfoFail, ValidateOpenCL100DebugInfoDebugTypeInheritance,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(R"(%dbg_src %VS_OUTPUT_info %u32_0 %u32_0 FlagIsPublic)",
                       "Child must be a result id of"),
        std::make_pair(R"(%foo_info %dbg_src %u32_0 %u32_0 FlagIsPublic)",
                       "Parent must be a result id of"),
        std::make_pair(
            R"(%bar_info %VS_OUTPUT_info %u32_0 %u32_0 FlagIsPublic)",
            "Child must be class or struct debug type"),
        std::make_pair(R"(%foo_info %bar_info %u32_0 %u32_0 FlagIsPublic)",
                       "Parent must be class or struct debug type"),
        std::make_pair(R"(%foo_info %VS_OUTPUT_info %void %u32_0 FlagIsPublic)",
                       "Offset"),
        std::make_pair(R"(%foo_info %VS_OUTPUT_info %u32_0 %void FlagIsPublic)",
                       "Size"),
    }));

TEST_F(ValidateVulkan100DebugInfo, DebugTypeComposite) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "struct VS_OUTPUT {
  float4 pos : SV_POSITION;
};
struct foo : VS_OUTPUT {
};
main() {}
"
%VS_OUTPUT_name = OpString "struct VS_OUTPUT"
%float_name = OpString "float"
%foo_name = OpString "foo"
%VS_OUTPUT_pos_name = OpString "pos : SV_POSITION"
%VS_OUTPUT_linkage_name = OpString "VS_OUTPUT"
%main_name = OpString "main"
%main_linkage_name = OpString "v4f_main_f"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
%u32_128 = OpConstant %u32 128
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%v4float_info = OpExtInst %void %DbgExt DebugTypeVector %float_info %u32_4
%VS_OUTPUT_pos_info = OpExtInst %void %DbgExt DebugTypeMember %VS_OUTPUT_pos_name %v4float_info %dbg_src %u32_2 %u32_3 %u32_0 %u32_128 %u32_3
%VS_OUTPUT_info = OpExtInst %void %DbgExt DebugTypeComposite %VS_OUTPUT_name %u32_1 %dbg_src %u32_1 %u32_1 %comp_unit %VS_OUTPUT_linkage_name %u32_128 %u32_3 %VS_OUTPUT_pos_info
%foo_info = OpExtInst %void %DbgExt DebugTypeComposite %foo_name %u32_1 %dbg_src %u32_1 %u32_1 %comp_unit %foo_name %u32_0 %u32_3
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateVulkan100DebugInfoDebugTypeComposite, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "struct VS_OUTPUT {
  float4 pos : SV_POSITION;
};
struct foo : VS_OUTPUT {
};
main() {}
"
%VS_OUTPUT_name = OpString "struct VS_OUTPUT"
%float_name = OpString "float"
%foo_name = OpString "foo"
%VS_OUTPUT_pos_name = OpString "pos : SV_POSITION"
%VS_OUTPUT_linkage_name = OpString "VS_OUTPUT"
%main_name = OpString "main"
%main_linkage_name = OpString "v4f_main_f"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
%u32_128 = OpConstant %u32 128
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%v4float_info = OpExtInst %void %DbgExt DebugTypeVector %float_info %u32_4
%VS_OUTPUT_pos_info = OpExtInst %void %DbgExt DebugTypeMember %VS_OUTPUT_pos_name %v4float_info %dbg_src %u32_2 %u32_3 %u32_0 %u32_128 %u32_3
%VS_OUTPUT_info = OpExtInst %void %DbgExt DebugTypeComposite )";
  ss << param.first;

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, constants, ss.str(),
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second + " must be "));
}

INSTANTIATE_TEST_SUITE_P(
    AllVulkan100DebugInfoFail, ValidateVulkan100DebugInfoDebugTypeComposite,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(
            R"(%dbg_src %u32_1 %dbg_src %u32_1 %u32_1 %comp_unit %VS_OUTPUT_linkage_name %u32_128 %u32_3 %VS_OUTPUT_pos_info)",
            "Name"),
        std::make_pair(
            R"(%VS_OUTPUT_name %u32_1 %comp_unit %u32_1 %u32_1 %comp_unit %VS_OUTPUT_linkage_name %u32_128 %u32_3 %VS_OUTPUT_pos_info)",
            "Source"),
        std::make_pair(
            R"(%VS_OUTPUT_name %u32_1 %dbg_src %u32_1 %u32_1 %dbg_src %VS_OUTPUT_linkage_name %u32_128 %u32_3 %VS_OUTPUT_pos_info)",
            "Parent"),
        std::make_pair(
            R"(%VS_OUTPUT_name %u32_1 %dbg_src %u32_1 %u32_1 %comp_unit %u32_128 %u32_128 %u32_3 %VS_OUTPUT_pos_info)",
            "Linkage Name"),
        std::make_pair(
            R"(%VS_OUTPUT_name %u32_1 %dbg_src %u32_1 %u32_1 %comp_unit %VS_OUTPUT_linkage_name %dbg_src %u32_3 %VS_OUTPUT_pos_info)",
            "Size"),
        std::make_pair(
            R"(%VS_OUTPUT_name %u32_1 %dbg_src %u32_1 %u32_1 %comp_unit %VS_OUTPUT_linkage_name %u32_128 %dbg_src %VS_OUTPUT_pos_info)",
            "Flags"),
        std::make_pair(
            R"(%VS_OUTPUT_name %u32_1 %dbg_src %u32_1 %u32_1 %comp_unit %VS_OUTPUT_linkage_name %u32_128 %u32_3 %dbg_src)",
            "Members"),
    }));

TEST_P(ValidateVulkan100DebugInfoDebugTypeMember, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "struct VS_OUTPUT {
  float pos : SV_POSITION;
};
main() {}
"
%VS_OUTPUT_name = OpString "struct VS_OUTPUT"
%float_name = OpString "float"
%VS_OUTPUT_pos_name = OpString "pos : SV_POSITION"
%VS_OUTPUT_linkage_name = OpString "VS_OUTPUT"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
%u32_128 = OpConstant %u32 128
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%VS_OUTPUT_pos_info = OpExtInst %void %DbgExt DebugTypeMember )";
  ss << param.first;

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, constants, ss.str(),
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  if (!param.second.empty()) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("expected operand " + param.second +
                          " must be a result id of "));
  }
}

INSTANTIATE_TEST_SUITE_P(
    AllVulkan100DebugInfoFail, ValidateVulkan100DebugInfoDebugTypeMember,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(
            R"(%dbg_src %float_info %dbg_src %u32_2 %u32_3 %u32_0 %u32_32 %u32_3)",
            "Name"),
        std::make_pair(
            R"(%VS_OUTPUT_pos_name %dbg_src %dbg_src %u32_2 %u32_3 %u32_0 %u32_32 %u32_3)",
            ""),
        std::make_pair(
            R"(%VS_OUTPUT_pos_name %float_info %float_info %u32_2 %u32_3 %u32_0 %u32_32 %u32_3)",
            "Source"),
        std::make_pair(
            R"(%VS_OUTPUT_pos_name %float_info %dbg_src %u32_2 %u32_3 %void %u32_32 %u32_3)",
            "Offset"),
        std::make_pair(
            R"(%VS_OUTPUT_pos_name %float_info %dbg_src %u32_2 %u32_3 %u32_0 %void %u32_3)",
            "Size"),
        std::make_pair(
            R"(%VS_OUTPUT_pos_name %float_info %dbg_src %u32_2 %u32_3 %u32_0 %u32_32 %void)",
            "Flags"),
    }));

TEST_F(ValidateOpenCL100DebugInfo, DebugFunctionDeclaration) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "struct VS_OUTPUT {
  float4 pos : SV_POSITION;
};
main() {}
"
%main_name = OpString "main"
%main_linkage_name = OpString "v4f_main_f"
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %void
%main_decl = OpExtInst %void %DbgExt DebugFunctionDeclaration %main_name %main_type_info %dbg_src 12 1 %comp_unit %main_linkage_name FlagIsPublic
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src 12 1 %comp_unit %main_linkage_name FlagIsPublic 13 %main)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", dbg_inst_header,
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCL100DebugInfoDebugFunction, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "struct VS_OUTPUT {
  float4 pos : SV_POSITION;
};
main() {}
"
%main_name = OpString "main"
%main_linkage_name = OpString "v4f_main_f"
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %void
%main_decl = OpExtInst %void %DbgExt DebugFunctionDeclaration %main_name %main_type_info %dbg_src 12 1 %comp_unit %main_linkage_name FlagIsPublic
%main_info = OpExtInst %void %DbgExt DebugFunction )"
     << param.first;

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", ss.str(), "",
                                                     extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllOpenCL100DebugInfoFail, ValidateOpenCL100DebugInfoDebugFunction,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(
            R"(%u32_0 %main_type_info %dbg_src 12 1 %comp_unit %main_linkage_name FlagIsPublic 13 %main)",
            "Name"),
        std::make_pair(
            R"(%main_name %dbg_src %dbg_src 12 1 %comp_unit %main_linkage_name FlagIsPublic 13 %main)",
            "Type"),
        std::make_pair(
            R"(%main_name %main_type_info %comp_unit 12 1 %comp_unit %main_linkage_name FlagIsPublic 13 %main)",
            "Source"),
        std::make_pair(
            R"(%main_name %main_type_info %dbg_src 12 1 %dbg_src %main_linkage_name FlagIsPublic 13 %main)",
            "Parent"),
        std::make_pair(
            R"(%main_name %main_type_info %dbg_src 12 1 %comp_unit %void FlagIsPublic 13 %main)",
            "Linkage Name"),
        std::make_pair(
            R"(%main_name %main_type_info %dbg_src 12 1 %comp_unit %main_linkage_name FlagIsPublic 13 %void)",
            "Function"),
        std::make_pair(
            R"(%main_name %main_type_info %dbg_src 12 1 %comp_unit %main_linkage_name FlagIsPublic 13 %main %dbg_src)",
            "Declaration"),
    }));

TEST_P(ValidateOpenCL100DebugInfoDebugFunctionDeclaration, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "struct VS_OUTPUT {
  float4 pos : SV_POSITION;
};
main() {}
"
%main_name = OpString "main"
%main_linkage_name = OpString "v4f_main_f"
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %void
%main_decl = OpExtInst %void %DbgExt DebugFunctionDeclaration )"
     << param.first;

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", ss.str(), "",
                                                     extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllOpenCL100DebugInfoFail,
    ValidateOpenCL100DebugInfoDebugFunctionDeclaration,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(
            R"(%u32_0 %main_type_info %dbg_src 12 1 %comp_unit %main_linkage_name FlagIsPublic)",
            "Name"),
        std::make_pair(
            R"(%main_name %dbg_src %dbg_src 12 1 %comp_unit %main_linkage_name FlagIsPublic)",
            "Type"),
        std::make_pair(
            R"(%main_name %main_type_info %comp_unit 12 1 %comp_unit %main_linkage_name FlagIsPublic)",
            "Source"),
        std::make_pair(
            R"(%main_name %main_type_info %dbg_src 12 1 %dbg_src %main_linkage_name FlagIsPublic)",
            "Parent"),
        std::make_pair(
            R"(%main_name %main_type_info %dbg_src 12 1 %comp_unit %void FlagIsPublic)",
            "Linkage Name"),
    }));

TEST_F(ValidateVulkan100DebugInfo, DebugFunctionDeclaration) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "struct VS_OUTPUT {
  float4 pos : SV_POSITION;
};
main() {}
"
%main_name = OpString "main"
%main_linkage_name = OpString "v4f_main_f"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_12 = OpConstant %u32 12
%u32_13 = OpConstant %u32 13
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction %u32_3 %void
%main_decl = OpExtInst %void %DbgExt DebugFunctionDeclaration %main_name %main_type_info %dbg_src %u32_12 %u32_1 %comp_unit %main_linkage_name %u32_3
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src %u32_12 %u32_1 %comp_unit %main_linkage_name %u32_3 %u32_13
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateVulkan100DebugInfoDebugFunction, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "struct VS_OUTPUT {
  float4 pos : SV_POSITION;
};
main() {}
"
%main_name = OpString "main"
%main_linkage_name = OpString "v4f_main_f"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_12 = OpConstant %u32 12
%u32_13 = OpConstant %u32 13
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction %u32_3 %void
%main_decl = OpExtInst %void %DbgExt DebugFunctionDeclaration %main_name %main_type_info %dbg_src %u32_12 %u32_1 %comp_unit %main_linkage_name %u32_3
%main_info = OpExtInst %void %DbgExt DebugFunction )"
     << param.first;

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, constants, ss.str(),
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllVulkan100DebugInfoFail, ValidateVulkan100DebugInfoDebugFunction,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(
            R"(%u32_0 %main_type_info %dbg_src %u32_12 %u32_1 %comp_unit %main_linkage_name %u32_3 %u32_13)",
            "Name"),
        std::make_pair(
            R"(%main_name %dbg_src %dbg_src %u32_12 %u32_1 %comp_unit %main_linkage_name %u32_3 %u32_13)",
            "Type"),
        std::make_pair(
            R"(%main_name %main_type_info %comp_unit %u32_12 %u32_1 %comp_unit %main_linkage_name %u32_3 %u32_13)",
            "Source"),
        std::make_pair(
            R"(%main_name %main_type_info %dbg_src %u32_12 %u32_1 %dbg_src %main_linkage_name %u32_3 %u32_13)",
            "Parent"),
        std::make_pair(
            R"(%main_name %main_type_info %dbg_src %u32_12 %u32_1 %comp_unit %void %u32_3 %u32_13)",
            "Linkage Name"),
        std::make_pair(
            R"(%main_name %main_type_info %dbg_src %u32_12 %u32_1 %comp_unit %main_linkage_name %u32_3 %u32_13 %dbg_src)",
            "Declaration"),
    }));

TEST_P(ValidateVulkan100DebugInfoDebugFunctionDeclaration, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "struct VS_OUTPUT {
  float4 pos : SV_POSITION;
};
main() {}
"
%main_name = OpString "main"
%main_linkage_name = OpString "v4f_main_f"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_12 = OpConstant %u32 12
%u32_13 = OpConstant %u32 13
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction %u32_3 %void
%main_decl = OpExtInst %void %DbgExt DebugFunctionDeclaration )"
     << param.first;

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, constants, ss.str(),
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllVulkan100DebugInfoFail,
    ValidateVulkan100DebugInfoDebugFunctionDeclaration,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(
            R"(%u32_0 %main_type_info %dbg_src %u32_12 %u32_1 %comp_unit %main_linkage_name %u32_3)",
            "Name"),
        std::make_pair(
            R"(%main_name %dbg_src %dbg_src %u32_12 %u32_1 %comp_unit %main_linkage_name %u32_3)",
            "Type"),
        std::make_pair(
            R"(%main_name %main_type_info %comp_unit %u32_12 %u32_1 %comp_unit %main_linkage_name %u32_3)",
            "Source"),
        std::make_pair(
            R"(%main_name %main_type_info %dbg_src %u32_12 %u32_1 %dbg_src %main_linkage_name %u32_3)",
            "Parent"),
        std::make_pair(
            R"(%main_name %main_type_info %dbg_src %u32_12 %u32_1 %comp_unit %void %u32_3)",
            "Linkage Name"),
    }));

TEST_F(ValidateOpenCL100DebugInfo, DebugLexicalBlock) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%main_name = OpString "main"
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%main_block = OpExtInst %void %DbgExt DebugLexicalBlock %dbg_src 1 1 %comp_unit %main_name)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", dbg_inst_header,
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCL100DebugInfoDebugLexicalBlock, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%main_name = OpString "main"
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%main_block = OpExtInst %void %DbgExt DebugLexicalBlock )"
     << param.first;

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", ss.str(), "",
                                                     extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllOpenCL100DebugInfoFail, ValidateOpenCL100DebugInfoDebugLexicalBlock,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(R"(%comp_unit 1 1 %comp_unit %main_name)", "Source"),
        std::make_pair(R"(%dbg_src 1 1 %dbg_src %main_name)", "Parent"),
        std::make_pair(R"(%dbg_src 1 1 %comp_unit %void)", "Name"),
    }));

TEST_F(ValidateOpenCL100DebugInfo, DebugScopeFailScope) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() {}"
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
)";

  const std::string body = R"(
%main_scope = OpExtInst %void %DbgExt DebugScope %dbg_src
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, "", dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("expected operand Scope"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugScopeFailInlinedAt) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() {}"
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
)";

  const std::string body = R"(
%main_scope = OpExtInst %void %DbgExt DebugScope %comp_unit %dbg_src
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, "", dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("expected operand Inlined At"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugLexicalBlock) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%main_name = OpString "main"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%main_block = OpExtInst %void %DbgExt DebugLexicalBlock %dbg_src %u32_1 %u32_1 %comp_unit %main_name
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateVulkan100DebugInfoDebugLexicalBlock, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
%main_name = OpString "main"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%main_block = OpExtInst %void %DbgExt DebugLexicalBlock )"
     << param.first;

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, constants, ss.str(),
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllVulkan100DebugInfoFail, ValidateVulkan100DebugInfoDebugLexicalBlock,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(R"(%comp_unit %u32_1 %u32_1 %comp_unit %main_name)",
                       "Source"),
        std::make_pair(R"(%dbg_src %u32_1 %u32_1 %dbg_src %main_name)",
                       "Parent"),
        std::make_pair(R"(%dbg_src %u32_1 %u32_1 %comp_unit %void)", "Name"),
    }));

TEST_F(ValidateVulkan100DebugInfo, DebugScopeFailScope) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() {}"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
)";

  const std::string body = R"(
%main_scope = OpExtInst %void %DbgExt DebugScope %dbg_src
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("expected operand Scope"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugScopeFailInlinedAt) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() {}"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
)";

  const std::string body = R"(
%main_scope = OpExtInst %void %DbgExt DebugScope %comp_unit %dbg_src
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("expected operand Inlined At"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugLocalVariable) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() { float foo; }"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%foo = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %float_info %dbg_src 1 10 %comp_unit FlagIsLocal 0
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCL100DebugInfoDebugLocalVariable, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() { float foo; }"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%foo = OpExtInst %void %DbgExt DebugLocalVariable )"
     << param.first;

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, size_const, ss.str(),
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllOpenCL100DebugInfoFail, ValidateOpenCL100DebugInfoDebugLocalVariable,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(
            R"(%void %float_info %dbg_src 1 10 %comp_unit FlagIsLocal 0)",
            "Name"),
        std::make_pair(
            R"(%foo_name %dbg_src %dbg_src 1 10 %comp_unit FlagIsLocal 0)",
            "Type"),
        std::make_pair(
            R"(%foo_name %float_info %comp_unit 1 10 %comp_unit FlagIsLocal 0)",
            "Source"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src 1 10 %dbg_src FlagIsLocal 0)",
            "Parent"),
    }));

TEST_F(ValidateVulkan100DebugInfo, DebugLocalVariable) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() { float foo; }"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_10 = OpConstant %u32 10
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%foo = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %float_info %dbg_src %u32_1 %u32_10 %comp_unit %u32_4
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateVulkan100DebugInfoDebugLocalVariable, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() { float foo; }"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_10 = OpConstant %u32 10
%u32_32 = OpConstant %u32 32
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%foo = OpExtInst %void %DbgExt DebugLocalVariable )"
     << param.first;

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, constants, ss.str(),
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllVulkan100DebugInfoFail, ValidateVulkan100DebugInfoDebugLocalVariable,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(
            R"(%void %float_info %dbg_src %u32_1 %u32_10 %comp_unit %u32_3 %u32_0)",
            "Name"),
        std::make_pair(
            R"(%foo_name %dbg_src %dbg_src %u32_1 %u32_10 %comp_unit %u32_3 %u32_0)",
            "Type"),
        std::make_pair(
            R"(%foo_name %float_info %comp_unit %u32_1 %u32_10 %comp_unit %u32_3 %u32_0)",
            "Source"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src %u32_1 %u32_10 %dbg_src %u32_3 %u32_0)",
            "Parent"),
    }));

TEST_F(ValidateOpenCL100DebugInfo, DebugDeclare) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() { float foo; }"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%null_expr = OpExtInst %void %DbgExt DebugExpression
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%foo_info = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %float_info %dbg_src 1 10 %comp_unit FlagIsLocal 0
)";

  const std::string body = R"(
%foo = OpVariable %f32_ptr_function Function
%decl = OpExtInst %void %DbgExt DebugDeclare %foo_info %foo %null_expr
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugDeclareParam) {
  CompileSuccessfully(R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %in_var_COLOR
          %4 = OpString "test.hlsl"
               OpSource HLSL 620 %4 "#line 1 \"test.hlsl\"
void main(float foo:COLOR) {}
"
         %11 = OpString "#line 1 \"test.hlsl\"
void main(float foo:COLOR) {}
"
         %14 = OpString "float"
         %17 = OpString "src.main"
         %20 = OpString "foo"
               OpName %in_var_COLOR "in.var.COLOR"
               OpName %main "main"
               OpName %param_var_foo "param.var.foo"
               OpName %src_main "src.main"
               OpName %foo "foo"
               OpName %bb_entry "bb.entry"
               OpDecorate %in_var_COLOR Location 0
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
      %float = OpTypeFloat 32
%_ptr_Input_float = OpTypePointer Input %float
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
         %29 = OpTypeFunction %void %_ptr_Function_float
               OpLine %4 1 21
%in_var_COLOR = OpVariable %_ptr_Input_float Input
         %10 = OpExtInst %void %1 DebugExpression
         %12 = OpExtInst %void %1 DebugSource %4 %11
         %13 = OpExtInst %void %1 DebugCompilationUnit 1 4 %12 HLSL
         %15 = OpExtInst %void %1 DebugTypeBasic %14 %uint_32 Float
         %16 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %void %15
         %18 = OpExtInst %void %1 DebugFunction %17 %16 %12 1 1 %13 %17 FlagIsProtected|FlagIsPrivate 1 %src_main
         %21 = OpExtInst %void %1 DebugLocalVariable %20 %15 %12 1 17 %18 FlagIsLocal 0
         %22 = OpExtInst %void %1 DebugLexicalBlock %12 1 28 %18
               OpLine %4 1 1
       %main = OpFunction %void None %23
         %24 = OpLabel
               OpLine %4 1 17
%param_var_foo = OpVariable %_ptr_Function_float Function
         %27 = OpLoad %float %in_var_COLOR
               OpLine %4 1 1
         %28 = OpFunctionCall %void %src_main %param_var_foo
               OpReturn
               OpFunctionEnd
   %src_main = OpFunction %void None %29
               OpLine %4 1 17
        %foo = OpFunctionParameter %_ptr_Function_float
         %31 = OpExtInst %void %1 DebugDeclare %21 %foo %10
   %bb_entry = OpLabel
               OpLine %4 1 29
               OpReturn
               OpFunctionEnd
)");
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCL100DebugInfoDebugDeclare, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() { float foo; }"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%null_expr = OpExtInst %void %DbgExt DebugExpression
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%foo_info = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %float_info %dbg_src 1 10 %comp_unit FlagIsLocal 0
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%foo = OpVariable %f32_ptr_function Function
%decl = OpExtInst %void %DbgExt DebugDeclare )"
     << param.first;

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, ss.str(), extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllOpenCL100DebugInfoFail, ValidateOpenCL100DebugInfoDebugDeclare,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(R"(%dbg_src %foo %null_expr)", "Local Variable"),
        std::make_pair(R"(%foo_info %void %null_expr)", "Variable"),
        std::make_pair(R"(%foo_info %foo %dbg_src)", "Expression"),
    }));

TEST_F(ValidateVulkan100DebugInfo, DebugDeclare) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() { float foo; }"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_10 = OpConstant %u32 10
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%null_expr = OpExtInst %void %DbgExt DebugExpression
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%foo_info = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %float_info %dbg_src %u32_1 %u32_10 %comp_unit %u32_4
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  const std::string body = R"(
%foo = OpVariable %f32_ptr_function Function
%decl = OpExtInst %void %DbgExt DebugDeclare %foo_info %foo %null_expr
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateVulkan100DebugInfo, DebugDeclareParam) {
  CompileSuccessfully(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %in_var_COLOR
          %4 = OpString "test.hlsl"
               OpSource HLSL 620 %4 "#line 1 \"test.hlsl\"
void main(float foo:COLOR) {}
"
         %11 = OpString "#line 1 \"test.hlsl\"
void main(float foo:COLOR) {}
"
         %14 = OpString "float"
         %17 = OpString "src.main"
         %20 = OpString "foo"
               OpName %in_var_COLOR "in.var.COLOR"
               OpName %main "main"
               OpName %param_var_foo "param.var.foo"
               OpName %src_main "src.main"
               OpName %foo "foo"
               OpName %bb_entry "bb.entry"
               OpDecorate %in_var_COLOR Location 0
       %uint = OpTypeInt 32 0
      %u32_0 = OpConstant %uint 0
      %u32_1 = OpConstant %uint 1
      %u32_2 = OpConstant %uint 2
      %u32_3 = OpConstant %uint 3
      %u32_4 = OpConstant %uint 4
      %u32_5 = OpConstant %uint 5
     %u32_10 = OpConstant %uint 10
     %u32_17 = OpConstant %uint 17
     %u32_28 = OpConstant %uint 28
     %u32_32 = OpConstant %uint 32
    %uint_32 = OpConstant %uint 32
      %float = OpTypeFloat 32
%_ptr_Input_float = OpTypePointer Input %float
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
         %29 = OpTypeFunction %void %_ptr_Function_float
               OpLine %4 1 21
%in_var_COLOR = OpVariable %_ptr_Input_float Input
         %10 = OpExtInst %void %1 DebugExpression
         %12 = OpExtInst %void %1 DebugSource %4 %11
         %13 = OpExtInst %void %1 DebugCompilationUnit %u32_1 %u32_4 %12 %u32_5
         %15 = OpExtInst %void %1 DebugTypeBasic %14 %uint_32 %u32_3 %u32_0
         %16 = OpExtInst %void %1 DebugTypeFunction %u32_3 %void %15
         %18 = OpExtInst %void %1 DebugFunction %17 %16 %12 %u32_1 %u32_1 %13 %17 %u32_3 %u32_1
         %21 = OpExtInst %void %1 DebugLocalVariable %20 %15 %12 %u32_1 %u32_17 %18 %u32_4 %u32_0
         %22 = OpExtInst %void %1 DebugLexicalBlock %12 %u32_1 %u32_28 %18
       %main = OpFunction %void None %23
         %24 = OpLabel
%param_var_foo = OpVariable %_ptr_Function_float Function
         %27 = OpLoad %float %in_var_COLOR
         %28 = OpFunctionCall %void %src_main %param_var_foo
               OpReturn
               OpFunctionEnd
   %src_main = OpFunction %void None %29
        %foo = OpFunctionParameter %_ptr_Function_float
         %31 = OpExtInst %void %1 DebugDeclare %21 %foo %10
   %bb_entry = OpLabel
               OpReturn
               OpFunctionEnd
)");
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateVulkan100DebugInfoDebugDeclare, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() { float foo; }"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_10 = OpConstant %u32 10
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%null_expr = OpExtInst %void %DbgExt DebugExpression
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%foo_info = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %float_info %dbg_src %u32_1 %u32_10 %comp_unit %u32_4
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%foo = OpVariable %f32_ptr_function Function
%decl = OpExtInst %void %DbgExt DebugDeclare )"
     << param.first;

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, ss.str(), extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllVulkan100DebugInfoFail, ValidateVulkan100DebugInfoDebugDeclare,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(R"(%dbg_src %foo %null_expr)", "Local Variable"),
        std::make_pair(R"(%foo_info %void %null_expr)", "Variable"),
        std::make_pair(R"(%foo_info %foo %dbg_src)", "Expression"),
    }));

TEST_F(ValidateOpenCL100DebugInfo, DebugExpression) {
  const std::string dbg_inst_header = R"(
%op0 = OpExtInst %void %DbgExt DebugOperation Deref
%op1 = OpExtInst %void %DbgExt DebugOperation Plus
%null_expr = OpExtInst %void %DbgExt DebugExpression %op0 %op1
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo("", "", dbg_inst_header,
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugExpressionFail) {
  const std::string dbg_inst_header = R"(
%op = OpExtInst %void %DbgExt DebugOperation Deref
%null_expr = OpExtInst %void %DbgExt DebugExpression %op %void
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo("", "", dbg_inst_header,
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "expected operand Operation must be a result id of DebugOperation"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugExpression) {
  const std::string dbg_inst_header = R"(
%op0 = OpExtInst %void %DbgExt DebugOperation %u32_0
%op1 = OpExtInst %void %DbgExt DebugOperation %u32_1
%null_expr = OpExtInst %void %DbgExt DebugExpression %op0 %op1
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo("", "", dbg_inst_header,
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateVulkan100DebugInfo, DebugExpressionFail) {
  const std::string dbg_inst_header = R"(
%op = OpExtInst %void %DbgExt DebugOperation %u32_0
%null_expr = OpExtInst %void %DbgExt DebugExpression %op %void
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo("", "", dbg_inst_header,
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "expected operand Operation must be a result id of DebugOperation"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeTemplate) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "OpaqueType foo;
main() {}
"
%float_name = OpString "float"
%ty_name = OpString "Texture"
%t_name = OpString "T"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
%int_128 = OpConstant %u32 128
)";

  const std::string dbg_inst_header = R"(
%dbg_none = OpExtInst %void %DbgExt DebugInfoNone
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%opaque = OpExtInst %void %DbgExt DebugTypeComposite %ty_name Class %dbg_src 1 1 %comp_unit %ty_name %dbg_none FlagIsPublic
%param = OpExtInst %void %DbgExt DebugTypeTemplateParameter %t_name %float_info %dbg_none %dbg_src 0 0
%temp = OpExtInst %void %DbgExt DebugTypeTemplate %opaque %param
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeTemplateUsedForVariableType) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "OpaqueType foo;
main() {}
"
%float_name = OpString "float"
%ty_name = OpString "Texture"
%t_name = OpString "T"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
%int_128 = OpConstant %u32 128
)";

  const std::string dbg_inst_header = R"(
%dbg_none = OpExtInst %void %DbgExt DebugInfoNone
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%opaque = OpExtInst %void %DbgExt DebugTypeComposite %ty_name Class %dbg_src 1 1 %comp_unit %ty_name %dbg_none FlagIsPublic
%param = OpExtInst %void %DbgExt DebugTypeTemplateParameter %t_name %float_info %dbg_none %dbg_src 0 0
%temp = OpExtInst %void %DbgExt DebugTypeTemplate %opaque %param
%foo = OpExtInst %void %DbgExt DebugGlobalVariable %foo_name %temp %dbg_src 0 0 %comp_unit %foo_name %f32_input FlagIsProtected|FlagIsPrivate
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeTemplateFunction) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "OpaqueType foo;
main() {}
"
%float_name = OpString "float"
%ty_name = OpString "Texture"
%t_name = OpString "T"
%main_name = OpString "main"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
%int_128 = OpConstant %u32 128
)";

  const std::string dbg_inst_header = R"(
%dbg_none = OpExtInst %void %DbgExt DebugInfoNone
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%param = OpExtInst %void %DbgExt DebugTypeTemplateParameter %t_name %float_info %dbg_none %dbg_src 0 0
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %param %param
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src 1 1 %comp_unit %main_name FlagIsPublic 1 %main
%temp = OpExtInst %void %DbgExt DebugTypeTemplate %main_info %param
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeTemplateFailTarget) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "OpaqueType foo;
main() {}
"
%float_name = OpString "float"
%ty_name = OpString "Texture"
%t_name = OpString "T"
%main_name = OpString "main"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
%int_128 = OpConstant %u32 128
)";

  const std::string dbg_inst_header = R"(
%dbg_none = OpExtInst %void %DbgExt DebugInfoNone
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%param = OpExtInst %void %DbgExt DebugTypeTemplateParameter %t_name %float_info %dbg_none %dbg_src 0 0
%temp = OpExtInst %void %DbgExt DebugTypeTemplate %float_info %param
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Target must be DebugTypeComposite or "
                        "DebugFunction"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugTypeTemplateFailParam) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "OpaqueType foo;
main() {}
"
%float_name = OpString "float"
%ty_name = OpString "Texture"
%t_name = OpString "T"
%main_name = OpString "main"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
%int_128 = OpConstant %u32 128
)";

  const std::string dbg_inst_header = R"(
%dbg_none = OpExtInst %void %DbgExt DebugInfoNone
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%param = OpExtInst %void %DbgExt DebugTypeTemplateParameter %t_name %float_info %dbg_none %dbg_src 0 0
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %param %param
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src 1 1 %comp_unit %main_name FlagIsPublic 1 %main
%temp = OpExtInst %void %DbgExt DebugTypeTemplate %main_info %float_info
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "expected operand Parameters must be DebugTypeTemplateParameter or "
          "DebugTypeTemplateTemplateParameter"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeTemplate) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "OpaqueType foo;
main() {}
"
%float_name = OpString "float"
%ty_name = OpString "Texture"
%t_name = OpString "T"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_none = OpExtInst %void %DbgExt DebugInfoNone
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%opaque = OpExtInst %void %DbgExt DebugTypeComposite %ty_name %u32_1 %dbg_src %u32_1 %u32_1 %comp_unit %ty_name %dbg_none %u32_3
%param = OpExtInst %void %DbgExt DebugTypeTemplateParameter %t_name %float_info %dbg_none %dbg_src %u32_0 %u32_0
%temp = OpExtInst %void %DbgExt DebugTypeTemplate %opaque %param
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeTemplateUsedForVariableType) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "OpaqueType foo;
main() {}
"
%float_name = OpString "float"
%ty_name = OpString "Texture"
%t_name = OpString "T"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_none = OpExtInst %void %DbgExt DebugInfoNone
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%opaque = OpExtInst %void %DbgExt DebugTypeComposite %ty_name %u32_1 %dbg_src %u32_1 %u32_1 %comp_unit %ty_name %dbg_none %u32_3
%param = OpExtInst %void %DbgExt DebugTypeTemplateParameter %t_name %float_info %dbg_none %dbg_src %u32_0 %u32_0
%temp = OpExtInst %void %DbgExt DebugTypeTemplate %opaque %param
%foo = OpExtInst %void %DbgExt DebugGlobalVariable %foo_name %temp %dbg_src %u32_0 %u32_0 %comp_unit %foo_name %f32_input %u32_3
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeTemplateFunction) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "OpaqueType foo;
main() {}
"
%float_name = OpString "float"
%ty_name = OpString "Texture"
%t_name = OpString "T"
%main_name = OpString "main"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_none = OpExtInst %void %DbgExt DebugInfoNone
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%param = OpExtInst %void %DbgExt DebugTypeTemplateParameter %t_name %float_info %dbg_none %dbg_src %u32_0 %u32_0
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction %u32_3 %param %param
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src %u32_1 %u32_1 %comp_unit %main_name %u32_3 %u32_1
%temp = OpExtInst %void %DbgExt DebugTypeTemplate %main_info %param
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeTemplateFailTarget) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "OpaqueType foo;
main() {}
"
%float_name = OpString "float"
%ty_name = OpString "Texture"
%t_name = OpString "T"
%main_name = OpString "main"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_none = OpExtInst %void %DbgExt DebugInfoNone
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%param = OpExtInst %void %DbgExt DebugTypeTemplateParameter %t_name %float_info %dbg_none %dbg_src %u32_0 %u32_0
%temp = OpExtInst %void %DbgExt DebugTypeTemplate %float_info %param
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Target must be DebugTypeComposite or "
                        "DebugFunction"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugTypeTemplateFailParam) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "OpaqueType foo;
main() {}
"
%float_name = OpString "float"
%ty_name = OpString "Texture"
%t_name = OpString "T"
%main_name = OpString "main"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_none = OpExtInst %void %DbgExt DebugInfoNone
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%opaque = OpExtInst %void %DbgExt DebugTypeComposite %ty_name %u32_1 %dbg_src %u32_1 %u32_1 %comp_unit %ty_name %dbg_none %u32_3
%param = OpExtInst %void %DbgExt DebugTypeTemplateParameter %t_name %float_info %dbg_none %dbg_src %u32_0 %u32_0
%temp = OpExtInst %void %DbgExt DebugTypeTemplate %opaque %float_info
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "expected operand Parameters must be DebugTypeTemplateParameter or "
          "DebugTypeTemplateTemplateParameter"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugGlobalVariable) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float foo; void main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%foo = OpExtInst %void %DbgExt DebugGlobalVariable %foo_name %float_info %dbg_src 0 0 %comp_unit %foo_name %f32_input FlagIsProtected|FlagIsPrivate
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugGlobalVariableStaticMember) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float foo; void main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%t = OpExtInst %void %DbgExt DebugTypeComposite %foo_name Class %dbg_src 0 0 %comp_unit %foo_name %int_32 FlagIsPublic %a
%a = OpExtInst %void %DbgExt DebugTypeMember %foo_name %float_info %dbg_src 0 0 %t %u32_0 %int_32 FlagIsPublic
%foo = OpExtInst %void %DbgExt DebugGlobalVariable %foo_name %float_info %dbg_src 0 0 %comp_unit %foo_name %f32_input FlagIsProtected|FlagIsPrivate %a
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugGlobalVariableDebugInfoNone) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float foo; void main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbgNone = OpExtInst %void %DbgExt DebugInfoNone
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%foo = OpExtInst %void %DbgExt DebugGlobalVariable %foo_name %float_info %dbg_src 0 0 %comp_unit %foo_name %dbgNone FlagIsProtected|FlagIsPrivate
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugGlobalVariableConst) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float foo; void main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%foo = OpExtInst %void %DbgExt DebugGlobalVariable %foo_name %float_info %dbg_src 0 0 %comp_unit %foo_name %int_32 FlagIsProtected|FlagIsPrivate
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCL100DebugInfoDebugGlobalVariable, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float foo; void main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%foo = OpExtInst %void %DbgExt DebugGlobalVariable )"
     << param.first;

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, size_const, ss.str(),
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllOpenCL100DebugInfoFail, ValidateOpenCL100DebugInfoDebugGlobalVariable,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(
            R"(%void %float_info %dbg_src 0 0 %comp_unit %foo_name %f32_input FlagIsProtected|FlagIsPrivate)",
            "Name"),
        std::make_pair(
            R"(%foo_name %dbg_src %dbg_src 0 0 %comp_unit %foo_name %f32_input FlagIsProtected|FlagIsPrivate)",
            "Type"),
        std::make_pair(
            R"(%foo_name %float_info %comp_unit 0 0 %comp_unit %foo_name %f32_input FlagIsProtected|FlagIsPrivate)",
            "Source"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src 0 0 %dbg_src %foo_name %f32_input FlagIsProtected|FlagIsPrivate)",
            "Scope"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src 0 0 %comp_unit %void %f32_input FlagIsProtected|FlagIsPrivate)",
            "Linkage Name"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src 0 0 %comp_unit %foo_name %void FlagIsProtected|FlagIsPrivate)",
            "Variable"),
    }));

TEST_F(ValidateVulkan100DebugInfo, DebugGlobalVariable) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float foo; void main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%foo = OpExtInst %void %DbgExt DebugGlobalVariable %foo_name %float_info %dbg_src %u32_0 %u32_0 %comp_unit %foo_name %f32_input %u32_3
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateVulkan100DebugInfo, DebugGlobalVariableStaticMember) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float foo; void main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%a = OpExtInst %void %DbgExt DebugTypeMember %foo_name %float_info %dbg_src %u32_0 %u32_0 %u32_0 %u32_32 %u32_3
%t = OpExtInst %void %DbgExt DebugTypeComposite %foo_name %u32_1 %dbg_src %u32_0 %u32_0 %comp_unit %foo_name %u32_32 %u32_3 %a
%foo = OpExtInst %void %DbgExt DebugGlobalVariable %foo_name %t %dbg_src %u32_0 %u32_0 %comp_unit %foo_name %f32_input %u32_3
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateVulkan100DebugInfo, DebugGlobalVariableDebugInfoNone) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float foo; void main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbgNone = OpExtInst %void %DbgExt DebugInfoNone
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%foo = OpExtInst %void %DbgExt DebugGlobalVariable %foo_name %float_info %dbg_src %u32_0 %u32_0 %comp_unit %foo_name %dbgNone %u32_3
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateVulkan100DebugInfo, DebugGlobalVariableConst) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float foo; void main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%foo = OpExtInst %void %DbgExt DebugGlobalVariable %foo_name %float_info %dbg_src %u32_0 %u32_0 %comp_unit %foo_name %u32_32 %u32_3
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, "", extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateVulkan100DebugInfoDebugGlobalVariable, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "float foo; void main() {}"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%foo = OpExtInst %void %DbgExt DebugGlobalVariable )"
     << param.first;

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, constants, ss.str(),
                                                     "", extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllOpenCL100DebugInfoFail, ValidateVulkan100DebugInfoDebugGlobalVariable,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(
            R"(%void %float_info %dbg_src %u32_0 %u32_0 %comp_unit %foo_name %f32_input %u32_3)",
            "Name"),
        std::make_pair(
            R"(%foo_name %dbg_src %dbg_src %u32_0 %u32_0 %comp_unit %foo_name %f32_input %u32_3)",
            "Type"),
        std::make_pair(
            R"(%foo_name %float_info %comp_unit %u32_0 %u32_0 %comp_unit %foo_name %f32_input %u32_3)",
            "Source"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src %u32_0 %u32_0 %dbg_src %foo_name %f32_input %u32_3)",
            "Scope"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src %u32_0 %u32_0 %comp_unit %void %f32_input %u32_3)",
            "Linkage Name"),
        std::make_pair(
            R"(%foo_name %float_info %dbg_src %u32_0 %u32_0 %comp_unit %foo_name %void %u32_3)",
            "Variable"),
    }));

TEST_F(ValidateOpenCL100DebugInfo, DebugInlinedAt) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() {}"
%void_name = OpString "void"
%main_name = OpString "main"
%main_linkage_name = OpString "v_main"
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %void
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src 1 1 %comp_unit %main_linkage_name FlagIsPublic 1 %main
%inlined_at = OpExtInst %void %DbgExt DebugInlinedAt 0 %main_info
%inlined_at_recursive = OpExtInst %void %DbgExt DebugInlinedAt 0 %main_info %inlined_at
)";

  const std::string body = R"(
%main_scope = OpExtInst %void %DbgExt DebugScope %main_info %inlined_at
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, "", dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugInlinedAtFail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() {}"
%void_name = OpString "void"
%main_name = OpString "main"
%main_linkage_name = OpString "v_main"
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %void
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src 1 1 %comp_unit %main_linkage_name FlagIsPublic 1 %main
%inlined_at = OpExtInst %void %DbgExt DebugInlinedAt 0 %main_info
%inlined_at_recursive = OpExtInst %void %DbgExt DebugInlinedAt 0 %inlined_at
)";

  const std::string body = R"(
%main_scope = OpExtInst %void %DbgExt DebugScope %main_info %inlined_at
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, "", dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("expected operand Scope"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugInlinedAtFail2) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() {}"
%void_name = OpString "void"
%main_name = OpString "main"
%main_linkage_name = OpString "v_main"
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction FlagIsPublic %void
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src 1 1 %comp_unit %main_linkage_name FlagIsPublic 1 %main
%inlined_at = OpExtInst %void %DbgExt DebugInlinedAt 0 %main_info
%inlined_at_recursive = OpExtInst %void %DbgExt DebugInlinedAt 0 %main_info %main_info
)";

  const std::string body = R"(
%main_scope = OpExtInst %void %DbgExt DebugScope %main_info %inlined_at
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, "", dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("expected operand Inlined"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugInlinedAt) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() {}"
%void_name = OpString "void"
%main_name = OpString "main"
%main_linkage_name = OpString "v_main"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction %u32_3 %void
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src %u32_1 %u32_1 %comp_unit %main_name %u32_3 %u32_1
%inlined_at = OpExtInst %void %DbgExt DebugInlinedAt %u32_0 %main_info
%inlined_at_recursive = OpExtInst %void %DbgExt DebugInlinedAt %u32_0 %main_info %inlined_at
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  const std::string body = R"(
%main_scope = OpExtInst %void %DbgExt DebugScope %main_info %inlined_at
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateVulkan100DebugInfo, DebugInlinedAtFail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() {}"
%void_name = OpString "void"
%main_name = OpString "main"
%main_linkage_name = OpString "v_main"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction %u32_3 %void
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src %u32_1 %u32_1 %comp_unit %main_name %u32_3 %u32_1
%inlined_at = OpExtInst %void %DbgExt DebugInlinedAt %u32_0 %main_info
%inlined_at_recursive = OpExtInst %void %DbgExt DebugInlinedAt %u32_0 %inlined_at %inlined_at
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  const std::string body = R"(
%main_scope = OpExtInst %void %DbgExt DebugScope %main_info %inlined_at
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("expected operand Scope"));
}

TEST_F(ValidateVulkan100DebugInfo, DebugInlinedAtFail2) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() {}"
%void_name = OpString "void"
%main_name = OpString "main"
%main_linkage_name = OpString "v_main"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%main_type_info = OpExtInst %void %DbgExt DebugTypeFunction %u32_3 %void
%main_info = OpExtInst %void %DbgExt DebugFunction %main_name %main_type_info %dbg_src %u32_1 %u32_1 %comp_unit %main_name %u32_3 %u32_1
%inlined_at = OpExtInst %void %DbgExt DebugInlinedAt %u32_0 %main_info
%inlined_at_recursive = OpExtInst %void %DbgExt DebugInlinedAt %u32_0 %main_info %main_info
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  const std::string body = R"(
%main_scope = OpExtInst %void %DbgExt DebugScope %main_info %inlined_at
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("expected operand Inlined"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugValue) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() { float foo; }"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_3 = OpConstant %u32 3
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%null_expr = OpExtInst %void %DbgExt DebugExpression
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%v4float_info = OpExtInst %void %DbgExt DebugTypeVector %float_info 4
%foo_info = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %v4float_info %dbg_src 1 10 %comp_unit FlagIsLocal 0
)";

  const std::string body = R"(
%value = OpExtInst %void %DbgExt DebugValue %foo_info %int_32 %null_expr %int_3
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateOpenCL100DebugInfo, DebugValueWithVariableIndex) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() { float foo; }"
%float_name = OpString "float"
%int_name = OpString "int"
%foo_name = OpString "foo"
%len_name = OpString "length"
)";

  const std::string size_const = R"(
%int_3 = OpConstant %u32 3
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%null_expr = OpExtInst %void %DbgExt DebugExpression
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%int_info = OpExtInst %void %DbgExt DebugTypeBasic %int_name %int_32 Signed
%v4float_info = OpExtInst %void %DbgExt DebugTypeVector %float_info 4
%foo_info = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %v4float_info %dbg_src 1 10 %comp_unit FlagIsLocal
%len_info = OpExtInst %void %DbgExt DebugLocalVariable %len_name %int_info %dbg_src 0 0 %comp_unit FlagIsLocal
)";

  const std::string body = R"(
%value = OpExtInst %void %DbgExt DebugValue %foo_info %int_32 %null_expr %len_info
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCL100DebugInfoDebugValue, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() { float foo; }"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string size_const = R"(
%int_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit 2 4 %dbg_src HLSL
%null_expr = OpExtInst %void %DbgExt DebugExpression
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %int_32 Float
%foo_info = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %float_info %dbg_src 1 10 %comp_unit FlagIsLocal 0
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%decl = OpExtInst %void %DbgExt DebugValue )"
     << param.first;

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, size_const, dbg_inst_header, ss.str(), extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllOpenCL100DebugInfoFail, ValidateOpenCL100DebugInfoDebugValue,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(R"(%dbg_src %int_32 %null_expr)", "Local Variable"),
        std::make_pair(R"(%foo_info %int_32 %dbg_src)", "Expression"),
        std::make_pair(R"(%foo_info %int_32 %null_expr %dbg_src)", "Indexes"),
    }));

TEST_F(ValidateVulkan100DebugInfo, DebugValue) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() { float foo; }"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_10 = OpConstant %u32 10
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%null_expr = OpExtInst %void %DbgExt DebugExpression
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%v4float_info = OpExtInst %void %DbgExt DebugTypeVector %float_info %u32_4
%foo_info = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %v4float_info %dbg_src %u32_1 %u32_10 %comp_unit %u32_4
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  const std::string body = R"(
%value = OpExtInst %void %DbgExt DebugValue %foo_info %u32_32 %null_expr %u32_3
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateVulkan100DebugInfo, DebugValueWithVariableIndex) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() { float foo; }"
%float_name = OpString "float"
%int_name = OpString "int"
%foo_name = OpString "foo"
%len_name = OpString "length"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_10 = OpConstant %u32 10
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%null_expr = OpExtInst %void %DbgExt DebugExpression
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%int_info = OpExtInst %void %DbgExt DebugTypeBasic %int_name %u32_32 %u32_4 %u32_0
%v4float_info = OpExtInst %void %DbgExt DebugTypeVector %float_info %u32_4
%foo_info = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %v4float_info %dbg_src %u32_1 %u32_10 %comp_unit %u32_4 %u32_0
%len_info = OpExtInst %void %DbgExt DebugLocalVariable %len_name %int_info %dbg_src %u32_0 %u32_0 %comp_unit %u32_4
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  const std::string body = R"(
%value = OpExtInst %void %DbgExt DebugValue %foo_info %u32_32 %null_expr %len_info
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, body, extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateVulkan100DebugInfoDebugValue, Fail) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "void main() { float foo; }"
%float_name = OpString "float"
%foo_name = OpString "foo"
)";

  const std::string constants = R"(
%u32_4 = OpConstant %u32 4
%u32_5 = OpConstant %u32 5
%u32_10 = OpConstant %u32 10
%u32_32 = OpConstant %u32 32
)";

  const std::string dbg_inst_header = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %code
%comp_unit = OpExtInst %void %DbgExt DebugCompilationUnit %u32_2 %u32_4 %dbg_src %u32_5
%null_expr = OpExtInst %void %DbgExt DebugExpression
%float_info = OpExtInst %void %DbgExt DebugTypeBasic %float_name %u32_32 %u32_3 %u32_0
%v4float_info = OpExtInst %void %DbgExt DebugTypeVector %float_info %u32_4
%foo_info = OpExtInst %void %DbgExt DebugLocalVariable %foo_name %v4float_info %dbg_src %u32_1 %u32_10 %comp_unit %u32_4 %u32_0
)";

  const std::string extension = R"(
OpExtension "SPV_KHR_non_semantic_info"
%DbgExt = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
)";

  const auto& param = GetParam();

  std::ostringstream ss;
  ss << R"(
%decl = OpExtInst %void %DbgExt DebugValue )"
     << param.first;

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(
      src, constants, dbg_inst_header, ss.str(), extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand " + param.second));
}

INSTANTIATE_TEST_SUITE_P(
    AllOpenCL100DebugInfoFail, ValidateVulkan100DebugInfoDebugValue,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair(R"(%dbg_src %u32_32 %null_expr %u32_3)",
                       "Local Variable"),
        std::make_pair(R"(%foo_info %u32_32 %dbg_src %u32_3)", "Expression"),
        std::make_pair(R"(%foo_info %u32_32 %null_expr %dbg_src)", "Indexes"),
    }));

TEST_F(ValidateVulkan100DebugInfo, VulkanDebugInfoSample) {
  std::ostringstream ss;
  ss << R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %id_1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %id_MainPs "MainPs" %id_in_var_TEXCOORD2 %id_out_var_SV_Target0
               OpExecutionMode %id_MainPs OriginUpperLeft
          %id_7 = OpString "foo.frag"
         %id_27 = OpString "float"
         %id_32 = OpString "vColor"
         %id_36 = OpString "PS_OUTPUT"
         %id_42 = OpString "vTextureCoords"
         %id_46 = OpString "PS_INPUT"
         %id_49 = OpString "MainPs"
         %id_50 = OpString ""
         %id_55 = OpString "ps_output"
         %id_59 = OpString "i"
         %id_63 = OpString "@type.sampler"
         %id_64 = OpString "type.sampler"
         %id_66 = OpString "g_sAniso"
         %id_69 = OpString "@type.2d.image"
         %id_70 = OpString "type.2d.image"
         %id_72 = OpString "TemplateParam"
         %id_75 = OpString "g_tColor"
               OpName %id_type_2d_image "type.2d.image"
               OpName %id_g_tColor "g_tColor"
               OpName %id_type_sampler "type.sampler"
               OpName %id_g_sAniso "g_sAniso"
               OpName %id_in_var_TEXCOORD2 "in.var.TEXCOORD2"
               OpName %id_out_var_SV_Target0 "out.var.SV_Target0"
               OpName %id_MainPs "MainPs"
               OpName %id_PS_INPUT "PS_INPUT"
               OpMemberName %id_PS_INPUT 0 "vTextureCoords"
               OpName %id_param_var_i "param.var.i"
               OpName %id_PS_OUTPUT "PS_OUTPUT"
               OpMemberName %id_PS_OUTPUT 0 "vColor"
               OpName %id_src_MainPs "src.MainPs"
               OpName %id_i "i"
               OpName %id_bb_entry "bb.entry"
               OpName %id_ps_output "ps_output"
               OpName %id_type_sampled_image "type.sampled.image"
               OpDecorate %id_in_var_TEXCOORD2 Location 0
               OpDecorate %id_out_var_SV_Target0 Location 0
               OpDecorate %id_g_tColor DescriptorSet 0
               OpDecorate %id_g_tColor Binding 0
               OpDecorate %id_g_sAniso DescriptorSet 0
               OpDecorate %id_g_sAniso Binding 1
        %id_int = OpTypeInt 32 1
      %id_int_0 = OpConstant %id_int 0
       %id_uint = OpTypeInt 32 0
    %id_uint_32 = OpConstant %id_uint 32
      %id_float = OpTypeFloat 32
%id_type_2d_image = OpTypeImage %id_float 2D 2 0 0 1 Unknown
%id__ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %id_type_2d_image
%id_type_sampler = OpTypeSampler
%id__ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %id_type_sampler
    %id_v2float = OpTypeVector %id_float 2
%id__ptr_Input_v2float = OpTypePointer Input %id_v2float
    %id_v4float = OpTypeVector %id_float 4
%id__ptr_Output_v4float = OpTypePointer Output %id_v4float
       %id_void = OpTypeVoid
     %id_uint_1 = OpConstant %id_uint 1
     %id_uint_4 = OpConstant %id_uint 4
     %id_uint_5 = OpConstant %id_uint 5
     %id_uint_3 = OpConstant %id_uint 3
     %id_uint_0 = OpConstant %id_uint 0
   %id_uint_128 = OpConstant %id_uint 128
    %id_uint_12 = OpConstant %id_uint 12
    %id_uint_10 = OpConstant %id_uint 10
     %id_uint_8 = OpConstant %id_uint 8
     %id_uint_2 = OpConstant %id_uint 2
    %id_uint_64 = OpConstant %id_uint 64
     %id_uint_7 = OpConstant %id_uint 7
    %id_uint_15 = OpConstant %id_uint 15
    %id_uint_16 = OpConstant %id_uint 16
    %id_uint_17 = OpConstant %id_uint 17
    %id_uint_29 = OpConstant %id_uint 29
    %id_uint_14 = OpConstant %id_uint 14
    %id_uint_11 = OpConstant %id_uint 11
         %id_78 = OpTypeFunction %id_void
   %id_PS_INPUT = OpTypeStruct %id_v2float
%id__ptr_Function_PS_INPUT = OpTypePointer Function %id_PS_INPUT
  %id_PS_OUTPUT = OpTypeStruct %id_v4float
         %id_89 = OpTypeFunction %id_PS_OUTPUT %id__ptr_Function_PS_INPUT
%id__ptr_Function_PS_OUTPUT = OpTypePointer Function %id_PS_OUTPUT
    %id_uint_20 = OpConstant %id_uint 20
    %id_uint_19 = OpConstant %id_uint 19
    %id_uint_26 = OpConstant %id_uint 26
    %id_uint_46 = OpConstant %id_uint 46
%id__ptr_Function_v2float = OpTypePointer Function %id_v2float
    %id_uint_57 = OpConstant %id_uint 57
    %id_uint_78 = OpConstant %id_uint 78
%id_type_sampled_image = OpTypeSampledImage %id_type_2d_image
    %id_uint_81 = OpConstant %id_uint 81
%id__ptr_Function_v4float = OpTypePointer Function %id_v4float
   %id_g_tColor = OpVariable %id__ptr_UniformConstant_type_2d_image UniformConstant
   %id_g_sAniso = OpVariable %id__ptr_UniformConstant_type_sampler UniformConstant
%id_in_var_TEXCOORD2 = OpVariable %id__ptr_Input_v2float Input
%id_out_var_SV_Target0 = OpVariable %id__ptr_Output_v4float Output
         %id_22 = OpExtInst %id_void %id_1 DebugSource %id_7
         %id_23 = OpExtInst %id_void %id_1 DebugCompilationUnit %id_uint_1 %id_uint_4 %id_22 %id_uint_5
         %id_28 = OpExtInst %id_void %id_1 DebugTypeBasic %id_27 %id_uint_32 %id_uint_3 %id_uint_0
         %id_31 = OpExtInst %id_void %id_1 DebugTypeVector %id_28 %id_uint_4
         %id_34 = OpExtInst %id_void %id_1 DebugTypeMember %id_32 %id_31 %id_22 %id_uint_12 %id_uint_12 %id_uint_0 %id_uint_128 %id_uint_3
         %id_37 = OpExtInst %id_void %id_1 DebugTypeComposite %id_36 %id_uint_1 %id_22 %id_uint_10 %id_uint_8 %id_23 %id_36 %id_uint_128 %id_uint_3 %id_34
         %id_40 = OpExtInst %id_void %id_1 DebugTypeVector %id_28 %id_uint_2
         %id_44 = OpExtInst %id_void %id_1 DebugTypeMember %id_42 %id_40 %id_22 %id_uint_7 %id_uint_12 %id_uint_0 %id_uint_64 %id_uint_3
         %id_47 = OpExtInst %id_void %id_1 DebugTypeComposite %id_46 %id_uint_1 %id_22 %id_uint_5 %id_uint_8 %id_23 %id_46 %id_uint_64 %id_uint_3 %id_44
         %id_48 = OpExtInst %id_void %id_1 DebugTypeFunction %id_uint_3 %id_37 %id_47
         %id_51 = OpExtInst %id_void %id_1 DebugFunction %id_49 %id_48 %id_22 %id_uint_15 %id_uint_1 %id_23 %id_50 %id_uint_3 %id_uint_16
         %id_54 = OpExtInst %id_void %id_1 DebugLexicalBlock %id_22 %id_uint_16 %id_uint_1 %id_51
         %id_56 = OpExtInst %id_void %id_1 DebugLocalVariable %id_55 %id_37 %id_22 %id_uint_17 %id_uint_15 %id_54 %id_uint_4
         %id_58 = OpExtInst %id_void %id_1 DebugExpression
         %id_60 = OpExtInst %id_void %id_1 DebugLocalVariable %id_59 %id_47 %id_22 %id_uint_15 %id_uint_29 %id_51 %id_uint_4 %id_uint_1
         %id_62 = OpExtInst %id_void %id_1 DebugInfoNone
         %id_65 = OpExtInst %id_void %id_1 DebugTypeComposite %id_63 %id_uint_1 %id_22 %id_uint_0 %id_uint_0 %id_23 %id_64 %id_62 %id_uint_3
         %id_67 = OpExtInst %id_void %id_1 DebugGlobalVariable %id_66 %id_65 %id_22 %id_uint_3 %id_uint_14 %id_23 %id_66 %id_g_sAniso %id_uint_8
         %id_71 = OpExtInst %id_void %id_1 DebugTypeComposite %id_69 %id_uint_0 %id_22 %id_uint_0 %id_uint_0 %id_23 %id_70 %id_62 %id_uint_3
         %id_73 = OpExtInst %id_void %id_1 DebugTypeTemplateParameter %id_72 %id_31 %id_62 %id_22 %id_uint_0 %id_uint_0
         %id_74 = OpExtInst %id_void %id_1 DebugTypeTemplate %id_71 %id_73
         %id_76 = OpExtInst %id_void %id_1 DebugGlobalVariable %id_75 %id_74 %id_22 %id_uint_1 %id_uint_11 %id_23 %id_75 %id_g_tColor %id_uint_8
     %id_MainPs = OpFunction %id_void None %id_78
         %id_79 = OpLabel
%id_param_var_i = OpVariable %id__ptr_Function_PS_INPUT Function
         %id_83 = OpLoad %id_v2float %id_in_var_TEXCOORD2
         %id_84 = OpCompositeConstruct %id_PS_INPUT %id_83
               OpStore %id_param_var_i %id_84
         %id_86 = OpFunctionCall %id_PS_OUTPUT %id_src_MainPs %id_param_var_i
         %id_88 = OpCompositeExtract %id_v4float %id_86 0
               OpStore %id_out_var_SV_Target0 %id_88
               OpReturn
               OpFunctionEnd
 %id_src_MainPs = OpFunction %id_PS_OUTPUT None %id_89
          %id_i = OpFunctionParameter %id__ptr_Function_PS_INPUT
   %id_bb_entry = OpLabel
  %id_ps_output = OpVariable %id__ptr_Function_PS_OUTPUT Function
         %id_94 = OpExtInst %id_void %id_1 DebugScope %id_51
         %id_97 = OpExtInst %id_void %id_1 DebugDeclare %id_60 %id_i %id_58
         %id_99 = OpExtInst %id_void %id_1 DebugFunctionDefinition %id_51 %id_src_MainPs
        %id_100 = OpExtInst %id_void %id_1 DebugScope %id_54
        %id_102 = OpExtInst %id_void %id_1 DebugDeclare %id_56 %id_ps_output %id_58
        %id_106 = OpLoad %id_type_2d_image %id_g_tColor
        %id_109 = OpLoad %id_type_sampler %id_g_sAniso
        %id_114 = OpAccessChain %id__ptr_Function_v2float %id_i %id_int_0
        %id_115 = OpLoad %id_v2float %id_114
        %id_119 = OpSampledImage %id_type_sampled_image %id_106 %id_109
        %id_120 = OpImageSampleImplicitLod %id_v4float %id_119 %id_115 None
        %id_123 = OpAccessChain %id__ptr_Function_v4float %id_ps_output %id_int_0
               OpStore %id_123 %id_120
        %id_125 = OpLoad %id_PS_OUTPUT %id_ps_output
               OpReturnValue %id_125
               OpFunctionEnd
)";

  CompileSuccessfully(ss.str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

}  // namespace
}  // namespace val
}  // namespace spvtools
