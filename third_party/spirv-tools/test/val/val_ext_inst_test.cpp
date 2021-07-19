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

using ValidateExtInst = spvtest::ValidateBase<bool>;
using ValidateOldDebugInfo = spvtest::ValidateBase<std::string>;
using ValidateOpenCL100DebugInfo = spvtest::ValidateBase<std::string>;
using ValidateLocalDebugInfoOutOfFunction = spvtest::ValidateBase<std::string>;
using ValidateOpenCL100DebugInfoDebugTypedef =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugTypeEnum =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugTypeComposite =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugTypeMember =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugTypeInheritance =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugFunction =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugFunctionDeclaration =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugLexicalBlock =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugLocalVariable =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugGlobalVariable =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugDeclare =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateOpenCL100DebugInfoDebugValue =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;
using ValidateGlslStd450SqrtLike = spvtest::ValidateBase<std::string>;
using ValidateGlslStd450FMinLike = spvtest::ValidateBase<std::string>;
using ValidateGlslStd450FClampLike = spvtest::ValidateBase<std::string>;
using ValidateGlslStd450SAbsLike = spvtest::ValidateBase<std::string>;
using ValidateGlslStd450UMinLike = spvtest::ValidateBase<std::string>;
using ValidateGlslStd450UClampLike = spvtest::ValidateBase<std::string>;
using ValidateGlslStd450SinLike = spvtest::ValidateBase<std::string>;
using ValidateGlslStd450PowLike = spvtest::ValidateBase<std::string>;
using ValidateGlslStd450Pack = spvtest::ValidateBase<std::string>;
using ValidateGlslStd450Unpack = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdSqrtLike = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdFMinLike = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdFClampLike = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdSAbsLike = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdUMinLike = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdUClampLike = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdUMul24Like = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdUMad24Like = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdLengthLike = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdDistanceLike = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdNormalizeLike = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdVStoreHalfLike = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdVLoadHalfLike = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdFractLike = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdFrexpLike = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdLdexpLike = spvtest::ValidateBase<std::string>;
using ValidateOpenCLStdUpsampleLike = spvtest::ValidateBase<std::string>;
using ValidateClspvReflection = spvtest::ValidateBase<bool>;

// Returns number of components in Pack/Unpack extended instructions.
// |ext_inst_name| is expected to be of the format "PackHalf2x16".
// Number of components is assumed to be single-digit.
uint32_t GetPackedNumComponents(const std::string& ext_inst_name) {
  const size_t x_index = ext_inst_name.find_last_of('x');
  const std::string num_components_str =
      ext_inst_name.substr(x_index - 1, x_index);
  return uint32_t(std::stoul(num_components_str));
}

// Returns packed bit width in Pack/Unpack extended instructions.
// |ext_inst_name| is expected to be of the format "PackHalf2x16".
uint32_t GetPackedBitWidth(const std::string& ext_inst_name) {
  const size_t x_index = ext_inst_name.find_last_of('x');
  const std::string packed_bit_width_str = ext_inst_name.substr(x_index + 1);
  return uint32_t(std::stoul(packed_bit_width_str));
}

std::string GenerateShaderCode(
    const std::string& body,
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

%f32_input = OpVariable %f32_ptr_input Input
%f32vec2_input = OpVariable %f32vec2_ptr_input Input

%u32_input = OpVariable %u32_ptr_input Input
%u32vec2_input = OpVariable %u32vec2_ptr_input Input

%u64_input = OpVariable %u64_ptr_input Input

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

%main = OpFunction %void None %func
%main_entry = OpLabel
)";

  ss << body;

  ss << R"(
OpReturn
OpFunctionEnd)";

  return ss.str();
}

std::string GenerateKernelCode(
    const std::string& body,
    const std::string& capabilities_and_extensions = "",
    const std::string& memory_model = "Physical32") {
  std::ostringstream ss;
  ss << R"(
OpCapability Addresses
OpCapability Kernel
OpCapability Linkage
OpCapability GenericPointer
OpCapability Int8
OpCapability Int16
OpCapability Int64
OpCapability Float16
OpCapability Float64
OpCapability Vector16
OpCapability Matrix
)";

  ss << capabilities_and_extensions;
  ss << "%extinst = OpExtInstImport \"OpenCL.std\"\n";
  ss << "OpMemoryModel " << memory_model << " OpenCL\n";

  ss << R"(
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f16 = OpTypeFloat 16
%f32 = OpTypeFloat 32
%f64 = OpTypeFloat 64
%u32 = OpTypeInt 32 0
%u64 = OpTypeInt 64 0
%u16 = OpTypeInt 16 0
%u8 = OpTypeInt 8 0
%f32vec2 = OpTypeVector %f32 2
%f32vec3 = OpTypeVector %f32 3
%f32vec4 = OpTypeVector %f32 4
%f32vec8 = OpTypeVector %f32 8
%f16vec8 = OpTypeVector %f16 8
%f32vec16 = OpTypeVector %f32 16
%f64vec2 = OpTypeVector %f64 2
%f64vec3 = OpTypeVector %f64 3
%f64vec4 = OpTypeVector %f64 4
%u32vec2 = OpTypeVector %u32 2
%u32vec3 = OpTypeVector %u32 3
%u32vec4 = OpTypeVector %u32 4
%u32vec8 = OpTypeVector %u32 8
%u64vec2 = OpTypeVector %u64 2
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
%f32vec8_01010101 = OpConstantComposite %f32vec8 %f32_0 %f32_1 %f32_0 %f32_1 %f32_0 %f32_1 %f32_0 %f32_1

%f64_0 = OpConstant %f64 0
%f64_1 = OpConstant %f64 1
%f64_2 = OpConstant %f64 2
%f64_3 = OpConstant %f64 3
%f64vec2_01 = OpConstantComposite %f64vec2 %f64_0 %f64_1
%f64vec3_012 = OpConstantComposite %f64vec3 %f64_0 %f64_1 %f64_2
%f64vec4_0123 = OpConstantComposite %f64vec4 %f64_0 %f64_1 %f64_2 %f64_3

%f16_0 = OpConstant %f16 0
%f16_1 = OpConstant %f16 1

%u8_0 = OpConstant %u8 0
%u8_1 = OpConstant %u8 1
%u8_2 = OpConstant %u8 2
%u8_3 = OpConstant %u8 3

%u16_0 = OpConstant %u16 0
%u16_1 = OpConstant %u16 1
%u16_2 = OpConstant %u16 2
%u16_3 = OpConstant %u16 3

%u32_0 = OpConstant %u32 0
%u32_1 = OpConstant %u32 1
%u32_2 = OpConstant %u32 2
%u32_3 = OpConstant %u32 3
%u32_256 = OpConstant %u32 256

%u64_0 = OpConstant %u64 0
%u64_1 = OpConstant %u64 1
%u64_2 = OpConstant %u64 2
%u64_3 = OpConstant %u64 3
%u64_256 = OpConstant %u64 256

%u32vec2_01 = OpConstantComposite %u32vec2 %u32_0 %u32_1
%u32vec2_12 = OpConstantComposite %u32vec2 %u32_1 %u32_2
%u32vec3_012 = OpConstantComposite %u32vec3 %u32_0 %u32_1 %u32_2
%u32vec4_0123 = OpConstantComposite %u32vec4 %u32_0 %u32_1 %u32_2 %u32_3

%u64vec2_01 = OpConstantComposite %u64vec2 %u64_0 %u64_1

%f32mat22_1212 = OpConstantComposite %f32mat22 %f32vec2_12 %f32vec2_12
%f32mat23_121212 = OpConstantComposite %f32mat23 %f32vec2_12 %f32vec2_12 %f32vec2_12

%struct_f32_f32 = OpTypeStruct %f32 %f32
%struct_f32_f32_f32 = OpTypeStruct %f32 %f32 %f32
%struct_f32_u32 = OpTypeStruct %f32 %u32
%struct_f32_u32_f32 = OpTypeStruct %f32 %u32 %f32
%struct_u32_f32 = OpTypeStruct %u32 %f32
%struct_u32_u32 = OpTypeStruct %u32 %u32
%struct_f32_f64 = OpTypeStruct %f32 %f64
%struct_f32vec2_f32vec2 = OpTypeStruct %f32vec2 %f32vec2
%struct_f32vec2_u32vec2 = OpTypeStruct %f32vec2 %u32vec2

%f16vec8_ptr_workgroup = OpTypePointer Workgroup %f16vec8
%f16vec8_workgroup = OpVariable %f16vec8_ptr_workgroup Workgroup
%f16_ptr_workgroup = OpTypePointer Workgroup %f16

%u32vec8_ptr_workgroup = OpTypePointer Workgroup %u32vec8
%u32vec8_workgroup = OpVariable %u32vec8_ptr_workgroup Workgroup
%u32_ptr_workgroup = OpTypePointer Workgroup %u32

%f32vec8_ptr_workgroup = OpTypePointer Workgroup %f32vec8
%f32vec8_workgroup = OpVariable %f32vec8_ptr_workgroup Workgroup
%f32_ptr_workgroup = OpTypePointer Workgroup %f32

%u32arr = OpTypeArray %u32 %u32_256
%u32arr_ptr_cross_workgroup = OpTypePointer CrossWorkgroup %u32arr
%u32arr_cross_workgroup = OpVariable %u32arr_ptr_cross_workgroup CrossWorkgroup
%u32_ptr_cross_workgroup = OpTypePointer CrossWorkgroup %u32

%f32arr = OpTypeArray %f32 %u32_256
%f32arr_ptr_cross_workgroup = OpTypePointer CrossWorkgroup %f32arr
%f32arr_cross_workgroup = OpVariable %f32arr_ptr_cross_workgroup CrossWorkgroup
%f32_ptr_cross_workgroup = OpTypePointer CrossWorkgroup %f32

%f32vec2arr = OpTypeArray %f32vec2 %u32_256
%f32vec2arr_ptr_cross_workgroup = OpTypePointer CrossWorkgroup %f32vec2arr
%f32vec2arr_cross_workgroup = OpVariable %f32vec2arr_ptr_cross_workgroup CrossWorkgroup
%f32vec2_ptr_cross_workgroup = OpTypePointer CrossWorkgroup %f32vec2

%struct_arr = OpTypeArray %struct_f32_f32 %u32_256
%struct_arr_ptr_cross_workgroup = OpTypePointer CrossWorkgroup %struct_arr
%struct_arr_cross_workgroup = OpVariable %struct_arr_ptr_cross_workgroup CrossWorkgroup
%struct_ptr_cross_workgroup = OpTypePointer CrossWorkgroup %struct_f32_f32

%f16vec8_ptr_uniform_constant = OpTypePointer UniformConstant %f16vec8
%f16vec8_uniform_constant = OpVariable %f16vec8_ptr_uniform_constant UniformConstant
%f16_ptr_uniform_constant = OpTypePointer UniformConstant %f16

%u32vec8_ptr_uniform_constant = OpTypePointer UniformConstant %u32vec8
%u32vec8_uniform_constant = OpVariable %u32vec8_ptr_uniform_constant UniformConstant
%u32_ptr_uniform_constant = OpTypePointer UniformConstant %u32

%f32vec8_ptr_uniform_constant = OpTypePointer UniformConstant %f32vec8
%f32vec8_uniform_constant = OpVariable %f32vec8_ptr_uniform_constant UniformConstant
%f32_ptr_uniform_constant = OpTypePointer UniformConstant %f32

%f16vec8_ptr_input = OpTypePointer Input %f16vec8
%f16vec8_input = OpVariable %f16vec8_ptr_input Input
%f16_ptr_input = OpTypePointer Input %f16

%u32vec8_ptr_input = OpTypePointer Input %u32vec8
%u32vec8_input = OpVariable %u32vec8_ptr_input Input
%u32_ptr_input = OpTypePointer Input %u32

%f32_ptr_generic = OpTypePointer Generic %f32
%u32_ptr_generic = OpTypePointer Generic %u32

%f32_ptr_function = OpTypePointer Function %f32
%f32vec2_ptr_function = OpTypePointer Function %f32vec2
%u32_ptr_function = OpTypePointer Function %u32
%u64_ptr_function = OpTypePointer Function %u64
%u32vec2_ptr_function = OpTypePointer Function %u32vec2

%u8arr = OpTypeArray %u8 %u32_256
%u8arr_ptr_uniform_constant = OpTypePointer UniformConstant %u8arr
%u8arr_uniform_constant = OpVariable %u8arr_ptr_uniform_constant UniformConstant
%u8_ptr_uniform_constant = OpTypePointer UniformConstant %u8
%u8_ptr_generic = OpTypePointer Generic %u8

%main = OpFunction %void None %func
%main_entry = OpLabel
)";

  ss << body;

  ss << R"(
OpReturn
OpFunctionEnd)";

  return ss.str();
}

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

TEST_F(ValidateOpenCL100DebugInfo, DebugInstructionWrongResultType) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
%code = OpString "main() {}"
)";

  const std::string dbg_inst = R"(
%dbg_src = OpExtInst %bool %DbgExt DebugSource %src %code
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", dbg_inst, "",
                                                     extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected result type must be a result id of "
                        "OpTypeVoid"));
}

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

TEST_F(ValidateOpenCL100DebugInfo, DebugSourceFailFile) {
  const std::string src = R"(
%code = OpString "main() {}"
)";

  const std::string dbg_inst = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %DbgExt %code
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", dbg_inst, "",
                                                     extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand File must be a result id of "
                        "OpString"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugSourceFailSource) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
)";

  const std::string dbg_inst = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src %DbgExt
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", dbg_inst, "",
                                                     extension, "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("expected operand Text must be a result id of "
                        "OpString"));
}

TEST_F(ValidateOpenCL100DebugInfo, DebugSourceNoText) {
  const std::string src = R"(
%src = OpString "simple.hlsl"
)";

  const std::string dbg_inst = R"(
%dbg_src = OpExtInst %void %DbgExt DebugSource %src
)";

  const std::string extension = R"(
%DbgExt = OpExtInstImport "OpenCL.DebugInfo.100"
)";

  CompileSuccessfully(GenerateShaderCodeForDebugInfo(src, "", dbg_inst, "",
                                                     extension, "Vertex"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
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
TEST_P(ValidateGlslStd450SqrtLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name << " %f32_0\n";
  ss << "%val2 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %f32vec2_01\n";
  ss << "%val3 = OpExtInst %f64 %extinst " << ext_inst_name << " %f64_0\n";
  CompileSuccessfully(GenerateShaderCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

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

TEST_P(ValidateGlslStd450SqrtLike, IntResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %f32_0\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected Result Type to be a float scalar "
                        "or vector type"));
}

TEST_P(ValidateGlslStd450SqrtLike, IntOperand) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f32 %extinst " + ext_inst_name + " %u32_0\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected types of all operands to be equal to "
                        "Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllSqrtLike, ValidateGlslStd450SqrtLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "Round",
                             "RoundEven",
                             "FAbs",
                             "Trunc",
                             "FSign",
                             "Floor",
                             "Ceil",
                             "Fract",
                             "Sqrt",
                             "InverseSqrt",
                             "Normalize",
                         }));

TEST_P(ValidateGlslStd450FMinLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_0 %f32_1\n";
  ss << "%val2 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %f32vec2_01 %f32vec2_12\n";
  ss << "%val3 = OpExtInst %f64 %extinst " << ext_inst_name
     << " %f64_0 %f64_0\n";
  CompileSuccessfully(GenerateShaderCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateGlslStd450FMinLike, IntResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %f32_0 %f32_1\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected Result Type to be a float scalar "
                        "or vector type"));
}

TEST_P(ValidateGlslStd450FMinLike, IntOperand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f32 %extinst " + ext_inst_name + " %u32_0 %f32_1\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected types of all operands to be equal to "
                        "Result Type"));
}

TEST_P(ValidateGlslStd450FMinLike, IntOperand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f32 %extinst " + ext_inst_name + " %f32_0 %u32_1\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected types of all operands to be equal to "
                        "Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllFMinLike, ValidateGlslStd450FMinLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "FMin",
                             "FMax",
                             "Step",
                             "Reflect",
                             "NMin",
                             "NMax",
                         }));

TEST_P(ValidateGlslStd450FClampLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_0 %f32_1 %f32_2\n";
  ss << "%val2 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %f32vec2_01 %f32vec2_01 %f32vec2_12\n";
  ss << "%val3 = OpExtInst %f64 %extinst " << ext_inst_name
     << " %f64_0 %f64_0 %f64_1\n";
  CompileSuccessfully(GenerateShaderCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateGlslStd450FClampLike, IntResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %u32 %extinst " + ext_inst_name +
                           " %f32_0 %f32_1 %f32_2\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected Result Type to be a float scalar "
                        "or vector type"));
}

TEST_P(ValidateGlslStd450FClampLike, IntOperand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %f32 %extinst " + ext_inst_name +
                           " %u32_0 %f32_0 %f32_1\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected types of all operands to be equal to "
                        "Result Type"));
}

TEST_P(ValidateGlslStd450FClampLike, IntOperand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %f32 %extinst " + ext_inst_name +
                           " %f32_0 %u32_0 %f32_1\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected types of all operands to be equal to "
                        "Result Type"));
}

TEST_P(ValidateGlslStd450FClampLike, IntOperand3) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %f32 %extinst " + ext_inst_name +
                           " %f32_1 %f32_0 %u32_2\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected types of all operands to be equal to "
                        "Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllFClampLike, ValidateGlslStd450FClampLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "FClamp",
                             "FMix",
                             "SmoothStep",
                             "Fma",
                             "FaceForward",
                             "NClamp",
                         }));

TEST_P(ValidateGlslStd450SAbsLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %s32 %extinst " << ext_inst_name << " %u32_1\n";
  ss << "%val2 = OpExtInst %s32 %extinst " << ext_inst_name << " %s32_1\n";
  ss << "%val3 = OpExtInst %u32 %extinst " << ext_inst_name << " %u32_1\n";
  ss << "%val4 = OpExtInst %u32 %extinst " << ext_inst_name << " %s32_1\n";
  ss << "%val5 = OpExtInst %s32vec2 %extinst " << ext_inst_name
     << " %s32vec2_01\n";
  ss << "%val6 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01\n";
  ss << "%val7 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %s32vec2_01\n";
  ss << "%val8 = OpExtInst %s32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01\n";
  CompileSuccessfully(GenerateShaderCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateGlslStd450SAbsLike, FloatResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f32 %extinst " + ext_inst_name + " %u32_0\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected Result Type to be an int scalar "
                        "or vector type"));
}

TEST_P(ValidateGlslStd450SAbsLike, FloatOperand) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %s32 %extinst " + ext_inst_name + " %f32_0\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to be int scalars or "
                        "vectors"));
}

TEST_P(ValidateGlslStd450SAbsLike, WrongDimOperand) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %s32 %extinst " + ext_inst_name + " %s32vec2_01\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to have the same dimension as "
                        "Result Type"));
}

TEST_P(ValidateGlslStd450SAbsLike, WrongBitWidthOperand) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %s64 %extinst " + ext_inst_name + " %s32_0\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to have the same bit width as "
                        "Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllSAbsLike, ValidateGlslStd450SAbsLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "SAbs",
                             "SSign",
                             "FindILsb",
                             "FindUMsb",
                             "FindSMsb",
                         }));

TEST_F(ValidateExtInst, FindUMsbNot32Bit) {
  const std::string body = R"(
%val1 = OpExtInst %s64 %extinst FindUMsb %u64_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 FindUMsb: this instruction is currently "
                        "limited to 32-bit width components"));
}

TEST_F(ValidateExtInst, FindSMsbNot32Bit) {
  const std::string body = R"(
%val1 = OpExtInst %s64 %extinst FindSMsb %u64_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 FindSMsb: this instruction is currently "
                        "limited to 32-bit width components"));
}

TEST_P(ValidateGlslStd450UMinLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %s32 %extinst " << ext_inst_name
     << " %u32_1 %s32_2\n";
  ss << "%val2 = OpExtInst %s32 %extinst " << ext_inst_name
     << " %s32_1 %u32_2\n";
  ss << "%val3 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_1 %s32_2\n";
  ss << "%val4 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %s32_1 %u32_2\n";
  ss << "%val5 = OpExtInst %s32vec2 %extinst " << ext_inst_name
     << " %s32vec2_01 %u32vec2_01\n";
  ss << "%val6 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %s32vec2_01\n";
  ss << "%val7 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %s32vec2_01 %u32vec2_01\n";
  ss << "%val8 = OpExtInst %s32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %s32vec2_01\n";
  ss << "%val9 = OpExtInst %s64 %extinst " << ext_inst_name
     << " %u64_1 %s64_0\n";
  CompileSuccessfully(GenerateShaderCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateGlslStd450UMinLike, FloatResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f32 %extinst " + ext_inst_name + " %u32_0 %u32_0\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected Result Type to be an int scalar "
                        "or vector type"));
}

TEST_P(ValidateGlslStd450UMinLike, FloatOperand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %s32 %extinst " + ext_inst_name + " %f32_0 %u32_0\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to be int scalars or "
                        "vectors"));
}

TEST_P(ValidateGlslStd450UMinLike, FloatOperand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %s32 %extinst " + ext_inst_name + " %u32_0 %f32_0\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to be int scalars or "
                        "vectors"));
}

TEST_P(ValidateGlslStd450UMinLike, WrongDimOperand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %s32 %extinst " + ext_inst_name +
                           " %s32vec2_01 %s32_0\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to have the same dimension as "
                        "Result Type"));
}

TEST_P(ValidateGlslStd450UMinLike, WrongDimOperand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %s32 %extinst " + ext_inst_name +
                           " %s32_0 %s32vec2_01\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to have the same dimension as "
                        "Result Type"));
}

TEST_P(ValidateGlslStd450UMinLike, WrongBitWidthOperand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %s64 %extinst " + ext_inst_name + " %s32_0 %s64_0\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to have the same bit width as "
                        "Result Type"));
}

TEST_P(ValidateGlslStd450UMinLike, WrongBitWidthOperand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %s64 %extinst " + ext_inst_name + " %s64_0 %s32_0\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to have the same bit width as "
                        "Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllUMinLike, ValidateGlslStd450UMinLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "UMin",
                             "SMin",
                             "UMax",
                             "SMax",
                         }));

TEST_P(ValidateGlslStd450UClampLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %s32 %extinst " << ext_inst_name
     << " %s32_0 %u32_1 %s32_2\n";
  ss << "%val2 = OpExtInst %s32 %extinst " << ext_inst_name
     << " %u32_0 %s32_1 %u32_2\n";
  ss << "%val3 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %s32_0 %u32_1 %s32_2\n";
  ss << "%val4 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_0 %s32_1 %u32_2\n";
  ss << "%val5 = OpExtInst %s32vec2 %extinst " << ext_inst_name
     << " %s32vec2_01 %u32vec2_01 %u32vec2_12\n";
  ss << "%val6 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %s32vec2_01 %s32vec2_12\n";
  ss << "%val7 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %s32vec2_01 %u32vec2_01 %u32vec2_12\n";
  ss << "%val8 = OpExtInst %s32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %s32vec2_01 %s32vec2_12\n";
  ss << "%val9 = OpExtInst %s64 %extinst " << ext_inst_name
     << " %u64_1 %s64_0 %s64_1\n";
  CompileSuccessfully(GenerateShaderCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateGlslStd450UClampLike, FloatResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %f32 %extinst " + ext_inst_name +
                           " %u32_0 %u32_0 %u32_1\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected Result Type to be an int scalar "
                        "or vector type"));
}

TEST_P(ValidateGlslStd450UClampLike, FloatOperand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %s32 %extinst " + ext_inst_name +
                           " %f32_0 %u32_0 %u32_1\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to be int scalars or "
                        "vectors"));
}

TEST_P(ValidateGlslStd450UClampLike, FloatOperand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %s32 %extinst " + ext_inst_name +
                           " %u32_0 %f32_0 %u32_1\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to be int scalars or "
                        "vectors"));
}

TEST_P(ValidateGlslStd450UClampLike, FloatOperand3) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %s32 %extinst " + ext_inst_name +
                           " %u32_0 %u32_0 %f32_1\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to be int scalars or "
                        "vectors"));
}

TEST_P(ValidateGlslStd450UClampLike, WrongDimOperand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %s32 %extinst " + ext_inst_name +
                           " %s32vec2_01 %s32_0 %u32_1\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to have the same dimension as "
                        "Result Type"));
}

TEST_P(ValidateGlslStd450UClampLike, WrongDimOperand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %s32 %extinst " + ext_inst_name +
                           " %s32_0 %s32vec2_01 %u32_1\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to have the same dimension as "
                        "Result Type"));
}

TEST_P(ValidateGlslStd450UClampLike, WrongDimOperand3) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %s32 %extinst " + ext_inst_name +
                           " %s32_0 %u32_1 %s32vec2_01\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to have the same dimension as "
                        "Result Type"));
}

TEST_P(ValidateGlslStd450UClampLike, WrongBitWidthOperand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %s64 %extinst " + ext_inst_name +
                           " %s32_0 %s64_0 %s64_1\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to have the same bit width as "
                        "Result Type"));
}

TEST_P(ValidateGlslStd450UClampLike, WrongBitWidthOperand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %s64 %extinst " + ext_inst_name +
                           " %s64_0 %s32_0 %s64_1\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to have the same bit width as "
                        "Result Type"));
}

TEST_P(ValidateGlslStd450UClampLike, WrongBitWidthOperand3) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %s64 %extinst " + ext_inst_name +
                           " %s64_0 %s64_0 %s32_1\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected all operands to have the same bit width as "
                        "Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllUClampLike, ValidateGlslStd450UClampLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "UClamp",
                             "SClamp",
                         }));

TEST_P(ValidateGlslStd450SinLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name << " %f32_0\n";
  ss << "%val2 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %f32vec2_01\n";
  CompileSuccessfully(GenerateShaderCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateGlslStd450SinLike, IntResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %f32_0\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected Result Type to be a 16 or 32-bit scalar "
                        "or vector float type"));
}

TEST_P(ValidateGlslStd450SinLike, F64ResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f64 %extinst " + ext_inst_name + " %f32_0\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected Result Type to be a 16 or 32-bit scalar "
                        "or vector float type"));
}

TEST_P(ValidateGlslStd450SinLike, IntOperand) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f32 %extinst " + ext_inst_name + " %u32_0\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected types of all operands to be equal to "
                        "Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllSinLike, ValidateGlslStd450SinLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "Radians",
                             "Degrees",
                             "Sin",
                             "Cos",
                             "Tan",
                             "Asin",
                             "Acos",
                             "Atan",
                             "Sinh",
                             "Cosh",
                             "Tanh",
                             "Asinh",
                             "Acosh",
                             "Atanh",
                             "Exp",
                             "Exp2",
                             "Log",
                             "Log2",
                         }));

TEST_P(ValidateGlslStd450PowLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_1 %f32_1\n";
  ss << "%val2 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %f32vec2_01 %f32vec2_12\n";
  CompileSuccessfully(GenerateShaderCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateGlslStd450PowLike, IntResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %f32_1 %f32_0\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected Result Type to be a 16 or 32-bit scalar "
                        "or vector float type"));
}

TEST_P(ValidateGlslStd450PowLike, F64ResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f64 %extinst " + ext_inst_name + " %f32_1 %f32_0\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected Result Type to be a 16 or 32-bit scalar "
                        "or vector float type"));
}

TEST_P(ValidateGlslStd450PowLike, IntOperand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f32 %extinst " + ext_inst_name + " %u32_0 %f32_1\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected types of all operands to be equal to "
                        "Result Type"));
}

TEST_P(ValidateGlslStd450PowLike, IntOperand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f32 %extinst " + ext_inst_name + " %f32_0 %u32_1\n";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 " + ext_inst_name +
                        ": expected types of all operands to be equal to "
                        "Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllPowLike, ValidateGlslStd450PowLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "Atan2",
                             "Pow",
                         }));

TEST_F(ValidateExtInst, GlslStd450DeterminantSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Determinant %f32mat22_1212
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450DeterminantIncompatibleResultType) {
  const std::string body = R"(
%val1 = OpExtInst %f64 %extinst Determinant %f32mat22_1212
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Determinant: "
                        "expected operand X component type to be equal to "
                        "Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450DeterminantNotMatrix) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Determinant %f32_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Determinant: "
                        "expected operand X to be a square matrix"));
}

TEST_F(ValidateExtInst, GlslStd450DeterminantMatrixNotSquare) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Determinant %f32mat23_121212
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Determinant: "
                        "expected operand X to be a square matrix"));
}

TEST_F(ValidateExtInst, GlslStd450MatrixInverseSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32mat22 %extinst MatrixInverse %f32mat22_1212
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450MatrixInverseIncompatibleResultType) {
  const std::string body = R"(
%val1 = OpExtInst %f32mat33 %extinst MatrixInverse %f32mat22_1212
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 MatrixInverse: "
                        "expected operand X type to be equal to "
                        "Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450MatrixInverseNotMatrix) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst MatrixInverse %f32mat22_1212
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 MatrixInverse: "
                        "expected Result Type to be a square matrix"));
}

TEST_F(ValidateExtInst, GlslStd450MatrixInverseMatrixNotSquare) {
  const std::string body = R"(
%val1 = OpExtInst %f32mat23 %extinst MatrixInverse %f32mat23_121212
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 MatrixInverse: "
                        "expected Result Type to be a square matrix"));
}

TEST_F(ValidateExtInst, GlslStd450ModfSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Modf %f32_h %f32_output
%val2 = OpExtInst %f32vec2 %extinst Modf %f32vec2_01 %f32vec2_output
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450ModfIntResultType) {
  const std::string body = R"(
%val1 = OpExtInst %u32 %extinst Modf %f32_h %f32_output
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Modf: "
                        "expected Result Type to be a scalar or vector "
                        "float type"));
}

TEST_F(ValidateExtInst, GlslStd450ModfXNotOfResultType) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Modf %f64_0 %f32_output
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Modf: "
                        "expected operand X type to be equal to Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450ModfINotPointer) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Modf %f32_h %f32_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Modf: "
                        "expected operand I to be a pointer"));
}

TEST_F(ValidateExtInst, GlslStd450ModfIDataNotOfResultType) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Modf %f32_h %f32vec2_output
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Modf: "
                        "expected operand I data type to be equal to "
                        "Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450ModfStructSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %struct_f32_f32 %extinst ModfStruct %f32_h
%val2 = OpExtInst %struct_f32vec2_f32vec2 %extinst ModfStruct %f32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450ModfStructResultTypeNotStruct) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst ModfStruct %f32_h
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 ModfStruct: "
                        "expected Result Type to be a struct with two "
                        "identical scalar or vector float type members"));
}

TEST_F(ValidateExtInst, GlslStd450ModfStructResultTypeStructWrongSize) {
  const std::string body = R"(
%val1 = OpExtInst %struct_f32_f32_f32 %extinst ModfStruct %f32_h
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 ModfStruct: "
                        "expected Result Type to be a struct with two "
                        "identical scalar or vector float type members"));
}

TEST_F(ValidateExtInst, GlslStd450ModfStructResultTypeStructWrongFirstMember) {
  const std::string body = R"(
%val1 = OpExtInst %struct_u32_f32 %extinst ModfStruct %f32_h
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 ModfStruct: "
                        "expected Result Type to be a struct with two "
                        "identical scalar or vector float type members"));
}

TEST_F(ValidateExtInst, GlslStd450ModfStructResultTypeStructMembersNotEqual) {
  const std::string body = R"(
%val1 = OpExtInst %struct_f32_f64 %extinst ModfStruct %f32_h
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 ModfStruct: "
                        "expected Result Type to be a struct with two "
                        "identical scalar or vector float type members"));
}

TEST_F(ValidateExtInst, GlslStd450ModfStructXWrongType) {
  const std::string body = R"(
%val1 = OpExtInst %struct_f32_f32 %extinst ModfStruct %f64_0
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 ModfStruct: "
                        "expected operand X type to be equal to members of "
                        "Result Type struct"));
}

TEST_F(ValidateExtInst, GlslStd450FrexpSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Frexp %f32_h %u32_output
%val2 = OpExtInst %f32vec2 %extinst Frexp %f32vec2_01 %u32vec2_output
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450FrexpIntResultType) {
  const std::string body = R"(
%val1 = OpExtInst %u32 %extinst Frexp %f32_h %u32_output
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Frexp: "
                        "expected Result Type to be a scalar or vector "
                        "float type"));
}

TEST_F(ValidateExtInst, GlslStd450FrexpWrongXType) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Frexp %u32_1 %u32_output
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Frexp: "
                        "expected operand X type to be equal to Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450FrexpExpNotPointer) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Frexp %f32_1 %u32_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Frexp: "
                        "expected operand Exp to be a pointer"));
}

TEST_F(ValidateExtInst, GlslStd450FrexpExpNotInt32Pointer) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Frexp %f32_1 %f32_output
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Frexp: "
                        "expected operand Exp data type to be a 32-bit int "
                        "scalar or vector type"));
}

TEST_F(ValidateExtInst, GlslStd450FrexpExpWrongComponentNumber) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst Frexp %f32vec2_01 %u32_output
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Frexp: "
                        "expected operand Exp data type to have the same "
                        "component number as Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450LdexpSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Ldexp %f32_h %u32_2
%val2 = OpExtInst %f32vec2 %extinst Ldexp %f32vec2_01 %u32vec2_12
%val3 = OpExtInst %f32 %extinst Ldexp %f32_h %u64_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450LdexpIntResultType) {
  const std::string body = R"(
%val1 = OpExtInst %u32 %extinst Ldexp %f32_h %u32_2
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Ldexp: "
                        "expected Result Type to be a scalar or vector "
                        "float type"));
}

TEST_F(ValidateExtInst, GlslStd450LdexpWrongXType) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Ldexp %u32_1 %u32_2
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Ldexp: "
                        "expected operand X type to be equal to Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450LdexpFloatExp) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Ldexp %f32_1 %f32_2
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Ldexp: "
                        "expected operand Exp to be a 32-bit int scalar "
                        "or vector type"));
}

TEST_F(ValidateExtInst, GlslStd450LdexpExpWrongSize) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst Ldexp %f32vec2_12 %u32_2
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Ldexp: "
                        "expected operand Exp to have the same component "
                        "number as Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450FrexpStructSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %struct_f32_u32 %extinst FrexpStruct %f32_h
%val2 = OpExtInst %struct_f32vec2_u32vec2 %extinst FrexpStruct %f32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450FrexpStructResultTypeNotStruct) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst FrexpStruct %f32_h
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 FrexpStruct: "
                        "expected Result Type to be a struct with two members, "
                        "first member a float scalar or vector, second member "
                        "a 32-bit int scalar or vector with the same number of "
                        "components as the first member"));
}

TEST_F(ValidateExtInst, GlslStd450FrexpStructResultTypeStructWrongSize) {
  const std::string body = R"(
%val1 = OpExtInst %struct_f32_u32_f32 %extinst FrexpStruct %f32_h
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 FrexpStruct: "
                        "expected Result Type to be a struct with two members, "
                        "first member a float scalar or vector, second member "
                        "a 32-bit int scalar or vector with the same number of "
                        "components as the first member"));
}

TEST_F(ValidateExtInst, GlslStd450FrexpStructResultTypeStructWrongMember1) {
  const std::string body = R"(
%val1 = OpExtInst %struct_u32_u32 %extinst FrexpStruct %f32_h
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 FrexpStruct: "
                        "expected Result Type to be a struct with two members, "
                        "first member a float scalar or vector, second member "
                        "a 32-bit int scalar or vector with the same number of "
                        "components as the first member"));
}

TEST_F(ValidateExtInst, GlslStd450FrexpStructResultTypeStructWrongMember2) {
  const std::string body = R"(
%val1 = OpExtInst %struct_f32_f32 %extinst FrexpStruct %f32_h
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 FrexpStruct: "
                        "expected Result Type to be a struct with two members, "
                        "first member a float scalar or vector, second member "
                        "a 32-bit int scalar or vector with the same number of "
                        "components as the first member"));
}

TEST_F(ValidateExtInst, GlslStd450FrexpStructXWrongType) {
  const std::string body = R"(
%val1 = OpExtInst %struct_f32_u32 %extinst FrexpStruct %f64_0
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 FrexpStruct: "
                        "expected operand X type to be equal to the first "
                        "member of Result Type struct"));
}

TEST_F(ValidateExtInst,
       GlslStd450FrexpStructResultTypeStructRightInt16Member2) {
  const std::string body = R"(
%val1 = OpExtInst %struct_f16_u16 %extinst FrexpStruct %f16_h
)";

  const std::string extension = R"(
OpExtension  "SPV_AMD_gpu_shader_int16"
)";

  CompileSuccessfully(GenerateShaderCode(body, extension));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst,
       GlslStd450FrexpStructResultTypeStructWrongInt16Member2) {
  const std::string body = R"(
%val1 = OpExtInst %struct_f16_u16 %extinst FrexpStruct %f16_h
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 FrexpStruct: "
                        "expected Result Type to be a struct with two members, "
                        "first member a float scalar or vector, second member "
                        "a 32-bit int scalar or vector with the same number of "
                        "components as the first member"));
}

TEST_P(ValidateGlslStd450Pack, Success) {
  const std::string ext_inst_name = GetParam();
  const uint32_t num_components = GetPackedNumComponents(ext_inst_name);
  const uint32_t packed_bit_width = GetPackedBitWidth(ext_inst_name);
  const uint32_t total_bit_width = num_components * packed_bit_width;
  const std::string vec_str =
      num_components == 2 ? " %f32vec2_01\n" : " %f32vec4_0123\n";

  std::ostringstream body;
  body << "%val1 = OpExtInst %u" << total_bit_width << " %extinst "
       << ext_inst_name << vec_str;
  body << "%val2 = OpExtInst %s" << total_bit_width << " %extinst "
       << ext_inst_name << vec_str;
  CompileSuccessfully(GenerateShaderCode(body.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateGlslStd450Pack, Float32ResultType) {
  const std::string ext_inst_name = GetParam();
  const uint32_t num_components = GetPackedNumComponents(ext_inst_name);
  const uint32_t packed_bit_width = GetPackedBitWidth(ext_inst_name);
  const uint32_t total_bit_width = num_components * packed_bit_width;
  const std::string vec_str =
      num_components == 2 ? " %f32vec2_01\n" : " %f32vec4_0123\n";

  std::ostringstream body;
  body << "%val1 = OpExtInst %f" << total_bit_width << " %extinst "
       << ext_inst_name << vec_str;

  std::ostringstream expected;
  expected << "GLSL.std.450 " << ext_inst_name
           << ": expected Result Type to be " << total_bit_width
           << "-bit int scalar type";

  CompileSuccessfully(GenerateShaderCode(body.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(expected.str()));
}

TEST_P(ValidateGlslStd450Pack, Int16ResultType) {
  const std::string ext_inst_name = GetParam();
  const uint32_t num_components = GetPackedNumComponents(ext_inst_name);
  const uint32_t packed_bit_width = GetPackedBitWidth(ext_inst_name);
  const uint32_t total_bit_width = num_components * packed_bit_width;
  const std::string vec_str =
      num_components == 2 ? " %f32vec2_01\n" : " %f32vec4_0123\n";

  std::ostringstream body;
  body << "%val1 = OpExtInst %u16 %extinst " << ext_inst_name << vec_str;

  std::ostringstream expected;
  expected << "GLSL.std.450 " << ext_inst_name
           << ": expected Result Type to be " << total_bit_width
           << "-bit int scalar type";

  CompileSuccessfully(GenerateShaderCode(body.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(expected.str()));
}

TEST_P(ValidateGlslStd450Pack, VNotVector) {
  const std::string ext_inst_name = GetParam();
  const uint32_t num_components = GetPackedNumComponents(ext_inst_name);
  const uint32_t packed_bit_width = GetPackedBitWidth(ext_inst_name);
  const uint32_t total_bit_width = num_components * packed_bit_width;

  std::ostringstream body;
  body << "%val1 = OpExtInst %u" << total_bit_width << " %extinst "
       << ext_inst_name << " %f32_1\n";

  std::ostringstream expected;
  expected << "GLSL.std.450 " << ext_inst_name
           << ": expected operand V to be a 32-bit float vector of size "
           << num_components;

  CompileSuccessfully(GenerateShaderCode(body.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(expected.str()));
}

TEST_P(ValidateGlslStd450Pack, VNotFloatVector) {
  const std::string ext_inst_name = GetParam();
  const uint32_t num_components = GetPackedNumComponents(ext_inst_name);
  const uint32_t packed_bit_width = GetPackedBitWidth(ext_inst_name);
  const uint32_t total_bit_width = num_components * packed_bit_width;
  const std::string vec_str =
      num_components == 2 ? " %u32vec2_01\n" : " %u32vec4_0123\n";

  std::ostringstream body;
  body << "%val1 = OpExtInst %u" << total_bit_width << " %extinst "
       << ext_inst_name << vec_str;

  std::ostringstream expected;
  expected << "GLSL.std.450 " << ext_inst_name
           << ": expected operand V to be a 32-bit float vector of size "
           << num_components;

  CompileSuccessfully(GenerateShaderCode(body.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(expected.str()));
}

TEST_P(ValidateGlslStd450Pack, VNotFloat32Vector) {
  const std::string ext_inst_name = GetParam();
  const uint32_t num_components = GetPackedNumComponents(ext_inst_name);
  const uint32_t packed_bit_width = GetPackedBitWidth(ext_inst_name);
  const uint32_t total_bit_width = num_components * packed_bit_width;
  const std::string vec_str =
      num_components == 2 ? " %f64vec2_01\n" : " %f64vec4_0123\n";

  std::ostringstream body;
  body << "%val1 = OpExtInst %u" << total_bit_width << " %extinst "
       << ext_inst_name << vec_str;

  std::ostringstream expected;
  expected << "GLSL.std.450 " << ext_inst_name
           << ": expected operand V to be a 32-bit float vector of size "
           << num_components;

  CompileSuccessfully(GenerateShaderCode(body.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(expected.str()));
}

TEST_P(ValidateGlslStd450Pack, VWrongSizeVector) {
  const std::string ext_inst_name = GetParam();
  const uint32_t num_components = GetPackedNumComponents(ext_inst_name);
  const uint32_t packed_bit_width = GetPackedBitWidth(ext_inst_name);
  const uint32_t total_bit_width = num_components * packed_bit_width;
  const std::string vec_str =
      num_components == 4 ? " %f32vec2_01\n" : " %f32vec4_0123\n";

  std::ostringstream body;
  body << "%val1 = OpExtInst %u" << total_bit_width << " %extinst "
       << ext_inst_name << vec_str;

  std::ostringstream expected;
  expected << "GLSL.std.450 " << ext_inst_name
           << ": expected operand V to be a 32-bit float vector of size "
           << num_components;

  CompileSuccessfully(GenerateShaderCode(body.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(expected.str()));
}

INSTANTIATE_TEST_SUITE_P(AllPack, ValidateGlslStd450Pack,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "PackSnorm4x8",
                             "PackUnorm4x8",
                             "PackSnorm2x16",
                             "PackUnorm2x16",
                             "PackHalf2x16",
                         }));

TEST_F(ValidateExtInst, PackDouble2x32Success) {
  const std::string body = R"(
%val1 = OpExtInst %f64 %extinst PackDouble2x32 %u32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, PackDouble2x32Float32ResultType) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst PackDouble2x32 %u32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 PackDouble2x32: expected Result Type to "
                        "be 64-bit float scalar type"));
}

TEST_F(ValidateExtInst, PackDouble2x32Int64ResultType) {
  const std::string body = R"(
%val1 = OpExtInst %u64 %extinst PackDouble2x32 %u32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 PackDouble2x32: expected Result Type to "
                        "be 64-bit float scalar type"));
}

TEST_F(ValidateExtInst, PackDouble2x32VNotVector) {
  const std::string body = R"(
%val1 = OpExtInst %f64 %extinst PackDouble2x32 %u64_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 PackDouble2x32: expected operand V to be "
                        "a 32-bit int vector of size 2"));
}

TEST_F(ValidateExtInst, PackDouble2x32VNotIntVector) {
  const std::string body = R"(
%val1 = OpExtInst %f64 %extinst PackDouble2x32 %f32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 PackDouble2x32: expected operand V to be "
                        "a 32-bit int vector of size 2"));
}

TEST_F(ValidateExtInst, PackDouble2x32VNotInt32Vector) {
  const std::string body = R"(
%val1 = OpExtInst %f64 %extinst PackDouble2x32 %u64vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 PackDouble2x32: expected operand V to be "
                        "a 32-bit int vector of size 2"));
}

TEST_F(ValidateExtInst, PackDouble2x32VWrongSize) {
  const std::string body = R"(
%val1 = OpExtInst %f64 %extinst PackDouble2x32 %u32vec4_0123
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 PackDouble2x32: expected operand V to be "
                        "a 32-bit int vector of size 2"));
}

TEST_P(ValidateGlslStd450Unpack, Success) {
  const std::string ext_inst_name = GetParam();
  const uint32_t num_components = GetPackedNumComponents(ext_inst_name);
  const uint32_t packed_bit_width = GetPackedBitWidth(ext_inst_name);
  const uint32_t total_bit_width = num_components * packed_bit_width;
  const std::string result_type_str =
      num_components == 2 ? "%f32vec2" : " %f32vec4";

  std::ostringstream body;
  body << "%val1 = OpExtInst " << result_type_str << " %extinst "
       << ext_inst_name << " %u" << total_bit_width << "_1\n";
  body << "%val2 = OpExtInst " << result_type_str << " %extinst "
       << ext_inst_name << " %s" << total_bit_width << "_1\n";
  CompileSuccessfully(GenerateShaderCode(body.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateGlslStd450Unpack, ResultTypeNotVector) {
  const std::string ext_inst_name = GetParam();
  const uint32_t num_components = GetPackedNumComponents(ext_inst_name);
  const uint32_t packed_bit_width = GetPackedBitWidth(ext_inst_name);
  const uint32_t total_bit_width = num_components * packed_bit_width;
  const std::string result_type_str = "%f32";

  std::ostringstream body;
  body << "%val1 = OpExtInst " << result_type_str << " %extinst "
       << ext_inst_name << " %u" << total_bit_width << "_1\n";

  std::ostringstream expected;
  expected << "GLSL.std.450 " << ext_inst_name
           << ": expected Result Type to be a 32-bit float vector of size "
           << num_components;

  CompileSuccessfully(GenerateShaderCode(body.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(expected.str()));
}

TEST_P(ValidateGlslStd450Unpack, ResultTypeNotFloatVector) {
  const std::string ext_inst_name = GetParam();
  const uint32_t num_components = GetPackedNumComponents(ext_inst_name);
  const uint32_t packed_bit_width = GetPackedBitWidth(ext_inst_name);
  const uint32_t total_bit_width = num_components * packed_bit_width;
  const std::string result_type_str =
      num_components == 2 ? "%u32vec2" : " %u32vec4";

  std::ostringstream body;
  body << "%val1 = OpExtInst " << result_type_str << " %extinst "
       << ext_inst_name << " %u" << total_bit_width << "_1\n";

  std::ostringstream expected;
  expected << "GLSL.std.450 " << ext_inst_name
           << ": expected Result Type to be a 32-bit float vector of size "
           << num_components;

  CompileSuccessfully(GenerateShaderCode(body.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(expected.str()));
}

TEST_P(ValidateGlslStd450Unpack, ResultTypeNotFloat32Vector) {
  const std::string ext_inst_name = GetParam();
  const uint32_t num_components = GetPackedNumComponents(ext_inst_name);
  const uint32_t packed_bit_width = GetPackedBitWidth(ext_inst_name);
  const uint32_t total_bit_width = num_components * packed_bit_width;
  const std::string result_type_str =
      num_components == 2 ? "%f64vec2" : " %f64vec4";

  std::ostringstream body;
  body << "%val1 = OpExtInst " << result_type_str << " %extinst "
       << ext_inst_name << " %u" << total_bit_width << "_1\n";

  std::ostringstream expected;
  expected << "GLSL.std.450 " << ext_inst_name
           << ": expected Result Type to be a 32-bit float vector of size "
           << num_components;

  CompileSuccessfully(GenerateShaderCode(body.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(expected.str()));
}

TEST_P(ValidateGlslStd450Unpack, ResultTypeWrongSize) {
  const std::string ext_inst_name = GetParam();
  const uint32_t num_components = GetPackedNumComponents(ext_inst_name);
  const uint32_t packed_bit_width = GetPackedBitWidth(ext_inst_name);
  const uint32_t total_bit_width = num_components * packed_bit_width;
  const std::string result_type_str =
      num_components == 4 ? "%f32vec2" : " %f32vec4";

  std::ostringstream body;
  body << "%val1 = OpExtInst " << result_type_str << " %extinst "
       << ext_inst_name << " %u" << total_bit_width << "_1\n";

  std::ostringstream expected;
  expected << "GLSL.std.450 " << ext_inst_name
           << ": expected Result Type to be a 32-bit float vector of size "
           << num_components;

  CompileSuccessfully(GenerateShaderCode(body.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(expected.str()));
}

TEST_P(ValidateGlslStd450Unpack, ResultPNotInt) {
  const std::string ext_inst_name = GetParam();
  const uint32_t num_components = GetPackedNumComponents(ext_inst_name);
  const uint32_t packed_bit_width = GetPackedBitWidth(ext_inst_name);
  const uint32_t total_bit_width = num_components * packed_bit_width;
  const std::string result_type_str =
      num_components == 2 ? "%f32vec2" : " %f32vec4";

  std::ostringstream body;
  body << "%val1 = OpExtInst " << result_type_str << " %extinst "
       << ext_inst_name << " %f" << total_bit_width << "_1\n";

  std::ostringstream expected;
  expected << "GLSL.std.450 " << ext_inst_name
           << ": expected operand P to be a " << total_bit_width
           << "-bit int scalar";

  CompileSuccessfully(GenerateShaderCode(body.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(expected.str()));
}

TEST_P(ValidateGlslStd450Unpack, ResultPWrongBitWidth) {
  const std::string ext_inst_name = GetParam();
  const uint32_t num_components = GetPackedNumComponents(ext_inst_name);
  const uint32_t packed_bit_width = GetPackedBitWidth(ext_inst_name);
  const uint32_t total_bit_width = num_components * packed_bit_width;
  const uint32_t wrong_bit_width = total_bit_width == 32 ? 64 : 32;
  const std::string result_type_str =
      num_components == 2 ? "%f32vec2" : " %f32vec4";

  std::ostringstream body;
  body << "%val1 = OpExtInst " << result_type_str << " %extinst "
       << ext_inst_name << " %u" << wrong_bit_width << "_1\n";

  std::ostringstream expected;
  expected << "GLSL.std.450 " << ext_inst_name
           << ": expected operand P to be a " << total_bit_width
           << "-bit int scalar";

  CompileSuccessfully(GenerateShaderCode(body.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(expected.str()));
}

INSTANTIATE_TEST_SUITE_P(AllUnpack, ValidateGlslStd450Unpack,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "UnpackSnorm4x8",
                             "UnpackUnorm4x8",
                             "UnpackSnorm2x16",
                             "UnpackUnorm2x16",
                             "UnpackHalf2x16",
                         }));

TEST_F(ValidateExtInst, UnpackDouble2x32Success) {
  const std::string body = R"(
%val1 = OpExtInst %u32vec2 %extinst UnpackDouble2x32 %f64_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, UnpackDouble2x32ResultTypeNotVector) {
  const std::string body = R"(
%val1 = OpExtInst %u64 %extinst UnpackDouble2x32 %f64_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 UnpackDouble2x32: expected Result Type "
                        "to be a 32-bit int vector of size 2"));
}

TEST_F(ValidateExtInst, UnpackDouble2x32ResultTypeNotIntVector) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst UnpackDouble2x32 %f64_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 UnpackDouble2x32: expected Result Type "
                        "to be a 32-bit int vector of size 2"));
}

TEST_F(ValidateExtInst, UnpackDouble2x32ResultTypeNotInt32Vector) {
  const std::string body = R"(
%val1 = OpExtInst %u64vec2 %extinst UnpackDouble2x32 %f64_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 UnpackDouble2x32: expected Result Type "
                        "to be a 32-bit int vector of size 2"));
}

TEST_F(ValidateExtInst, UnpackDouble2x32ResultTypeWrongSize) {
  const std::string body = R"(
%val1 = OpExtInst %u32vec4 %extinst UnpackDouble2x32 %f64_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 UnpackDouble2x32: expected Result Type "
                        "to be a 32-bit int vector of size 2"));
}

TEST_F(ValidateExtInst, UnpackDouble2x32VNotFloat) {
  const std::string body = R"(
%val1 = OpExtInst %u32vec2 %extinst UnpackDouble2x32 %u64_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 UnpackDouble2x32: expected operand V to "
                        "be a 64-bit float scalar"));
}

TEST_F(ValidateExtInst, UnpackDouble2x32VNotFloat64) {
  const std::string body = R"(
%val1 = OpExtInst %u32vec2 %extinst UnpackDouble2x32 %f32_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 UnpackDouble2x32: expected operand V to "
                        "be a 64-bit float scalar"));
}

TEST_F(ValidateExtInst, GlslStd450LengthSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Length %f32_1
%val2 = OpExtInst %f32 %extinst Length %f32vec2_01
%val3 = OpExtInst %f32 %extinst Length %f32vec4_0123
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450LengthIntResultType) {
  const std::string body = R"(
%val1 = OpExtInst %u32 %extinst Length %f32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Length: "
                        "expected Result Type to be a float scalar type"));
}

TEST_F(ValidateExtInst, GlslStd450LengthIntX) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Length %u32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Length: "
                        "expected operand X to be of float scalar or "
                        "vector type"));
}

TEST_F(ValidateExtInst, GlslStd450LengthDifferentType) {
  const std::string body = R"(
%val1 = OpExtInst %f64 %extinst Length %f32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Length: "
                        "expected operand X component type to be equal to "
                        "Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450DistanceSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Distance %f32_0 %f32_1
%val2 = OpExtInst %f32 %extinst Distance %f32vec2_01 %f32vec2_12
%val3 = OpExtInst %f32 %extinst Distance %f32vec4_0123 %f32vec4_1234
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450DistanceIntResultType) {
  const std::string body = R"(
%val1 = OpExtInst %u32 %extinst Distance %f32vec2_01 %f32vec2_12
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Distance: "
                        "expected Result Type to be a float scalar type"));
}

TEST_F(ValidateExtInst, GlslStd450DistanceIntP0) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Distance %u32_0 %f32_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Distance: "
                        "expected operand P0 to be of float scalar or "
                        "vector type"));
}

TEST_F(ValidateExtInst, GlslStd450DistanceF64VectorP0) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Distance %f64vec2_01 %f32vec2_12
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Distance: "
                        "expected operand P0 component type to be equal to "
                        "Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450DistanceIntP1) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Distance %f32_0 %u32_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Distance: "
                        "expected operand P1 to be of float scalar or "
                        "vector type"));
}

TEST_F(ValidateExtInst, GlslStd450DistanceF64VectorP1) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Distance %f32vec2_12 %f64vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Distance: "
                        "expected operand P1 component type to be equal to "
                        "Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450DistanceDifferentSize) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Distance %f32vec2_01 %f32vec4_0123
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Distance: "
                        "expected operands P0 and P1 to have the same number "
                        "of components"));
}

TEST_F(ValidateExtInst, GlslStd450CrossSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec3 %extinst Cross %f32vec3_012 %f32vec3_123
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450CrossIntVectorResultType) {
  const std::string body = R"(
%val1 = OpExtInst %u32vec3 %extinst Cross %f32vec3_012 %f32vec3_123
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Cross: "
                        "expected Result Type to be a float vector type"));
}

TEST_F(ValidateExtInst, GlslStd450CrossResultTypeWrongSize) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst Cross %f32vec3_012 %f32vec3_123
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Cross: "
                        "expected Result Type to have 3 components"));
}

TEST_F(ValidateExtInst, GlslStd450CrossXWrongType) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec3 %extinst Cross %f64vec3_012 %f32vec3_123
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Cross: "
                        "expected operand X type to be equal to Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450CrossYWrongType) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec3 %extinst Cross %f32vec3_123 %f64vec3_012
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Cross: "
                        "expected operand Y type to be equal to Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450RefractSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst Refract %f32_1 %f32_1 %f32_1
%val2 = OpExtInst %f32vec2 %extinst Refract %f32vec2_01 %f32vec2_01 %f16_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450RefractIntVectorResultType) {
  const std::string body = R"(
%val1 = OpExtInst %u32vec2 %extinst Refract %f32vec2_01 %f32vec2_01 %f32_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Refract: "
                        "expected Result Type to be a float scalar or "
                        "vector type"));
}

TEST_F(ValidateExtInst, GlslStd450RefractIntVectorI) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst Refract %u32vec2_01 %f32vec2_01 %f32_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Refract: "
                        "expected operand I to be of type equal to "
                        "Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450RefractIntVectorN) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst Refract %f32vec2_01 %u32vec2_01 %f32_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Refract: "
                        "expected operand N to be of type equal to "
                        "Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450RefractIntEta) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst Refract %f32vec2_01 %f32vec2_01 %u32_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Refract: "
                        "expected operand Eta to be a float scalar"));
}

TEST_F(ValidateExtInst, GlslStd450RefractFloat64Eta) {
  // SPIR-V issue 337: Eta can be 64-bit float scalar.
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst Refract %f32vec2_01 %f32vec2_01 %f64_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(ValidateExtInst, GlslStd450RefractVectorEta) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst Refract %f32vec2_01 %f32vec2_01 %f32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 Refract: "
                        "expected operand Eta to be a float scalar"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtCentroidSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtCentroid %f32_input
%val2 = OpExtInst %f32vec2 %extinst InterpolateAtCentroid %f32vec2_input
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtCentroidInternalSuccess) {
  const std::string body = R"(
%ld1  = OpLoad %f32 %f32_input
%val1 = OpExtInst %f32 %extinst InterpolateAtCentroid %ld1
%ld2  = OpLoad %f32vec2 %f32vec2_input
%val2 = OpExtInst %f32vec2 %extinst InterpolateAtCentroid %ld2
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  getValidatorOptions()->before_hlsl_legalization = true;
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtCentroidInternalInvalidDataF32) {
  const std::string body = R"(
%ld1  = OpLoad %f32 %f32_input
%val1 = OpExtInst %f32 %extinst InterpolateAtCentroid %ld1
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtCentroid: "
                        "expected Interpolant to be a pointer"));
}

TEST_F(ValidateExtInst,
       GlslStd450InterpolateAtCentroidInternalInvalidDataF32Vec2) {
  const std::string body = R"(
%ld2  = OpLoad %f32vec2 %f32vec2_input
%val2 = OpExtInst %f32vec2 %extinst InterpolateAtCentroid %ld2
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtCentroid: "
                        "expected Interpolant to be a pointer"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtCentroidNoCapability) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtCentroid %f32_input
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_CAPABILITY, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtCentroid requires "
                        "capability InterpolationFunction"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtCentroidIntResultType) {
  const std::string body = R"(
%val1 = OpExtInst %u32 %extinst InterpolateAtCentroid %f32_input
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtCentroid: "
                        "expected Result Type to be a 32-bit float scalar "
                        "or vector type"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtCentroidF64ResultType) {
  const std::string body = R"(
%val1 = OpExtInst %f64 %extinst InterpolateAtCentroid %f32_input
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtCentroid: "
                        "expected Result Type to be a 32-bit float scalar "
                        "or vector type"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtCentroidNotPointer) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtCentroid %f32_1
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtCentroid: "
                        "expected Interpolant to be a pointer"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtCentroidWrongDataType) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtCentroid %f32vec2_input
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtCentroid: "
                        "expected Interpolant data type to be equal to "
                        "Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtCentroidWrongStorageClass) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtCentroid %f32_output
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtCentroid: "
                        "expected Interpolant storage class to be Input"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtCentroidWrongExecutionModel) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtCentroid %f32_input
)";

  CompileSuccessfully(GenerateShaderCode(
      body, "OpCapability InterpolationFunction\n", "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtCentroid requires "
                        "Fragment execution model"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtSampleSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtSample %f32_input %u32_1
%val2 = OpExtInst %f32vec2 %extinst InterpolateAtSample %f32vec2_input %u32_1
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtSampleInternalSuccess) {
  const std::string body = R"(
%ld1  = OpLoad %f32 %f32_input
%val1 = OpExtInst %f32 %extinst InterpolateAtSample %ld1 %u32_1
%ld2  = OpLoad %f32vec2 %f32vec2_input
%val2 = OpExtInst %f32vec2 %extinst InterpolateAtSample %ld2 %u32_1
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  getValidatorOptions()->before_hlsl_legalization = true;
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtSampleInternalInvalidDataF32) {
  const std::string body = R"(
%ld1  = OpLoad %f32 %f32_input
%val1 = OpExtInst %f32 %extinst InterpolateAtSample %ld1 %u32_1
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtSample: "
                        "expected Interpolant to be a pointer"));
}

TEST_F(ValidateExtInst,
       GlslStd450InterpolateAtSampleInternalInvalidDataF32Vec2) {
  const std::string body = R"(
%ld2  = OpLoad %f32vec2 %f32vec2_input
%val2 = OpExtInst %f32vec2 %extinst InterpolateAtSample %ld2 %u32_1
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtSample: "
                        "expected Interpolant to be a pointer"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtSampleNoCapability) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtSample %f32_input %u32_1
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_CAPABILITY, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtSample requires "
                        "capability InterpolationFunction"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtSampleIntResultType) {
  const std::string body = R"(
%val1 = OpExtInst %u32 %extinst InterpolateAtSample %f32_input %u32_1
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtSample: "
                        "expected Result Type to be a 32-bit float scalar "
                        "or vector type"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtSampleF64ResultType) {
  const std::string body = R"(
%val1 = OpExtInst %f64 %extinst InterpolateAtSample %f32_input %u32_1
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtSample: "
                        "expected Result Type to be a 32-bit float scalar "
                        "or vector type"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtSampleNotPointer) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtSample %f32_1 %u32_1
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtSample: "
                        "expected Interpolant to be a pointer"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtSampleWrongDataType) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtSample %f32vec2_input %u32_1
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtSample: "
                        "expected Interpolant data type to be equal to "
                        "Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtSampleWrongStorageClass) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtSample %f32_output %u32_1
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtSample: "
                        "expected Interpolant storage class to be Input"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtSampleFloatSample) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtSample %f32_input %f32_1
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtSample: "
                        "expected Sample to be 32-bit integer"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtSampleU64Sample) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtSample %f32_input %u64_1
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtSample: "
                        "expected Sample to be 32-bit integer"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtSampleWrongExecutionModel) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtSample %f32_input %u32_1
)";

  CompileSuccessfully(GenerateShaderCode(
      body, "OpCapability InterpolationFunction\n", "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtSample requires "
                        "Fragment execution model"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtOffsetSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtOffset %f32_input %f32vec2_01
%val2 = OpExtInst %f32vec2 %extinst InterpolateAtOffset %f32vec2_input %f32vec2_01
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtOffsetInternalSuccess) {
  const std::string body = R"(
%ld1  = OpLoad %f32 %f32_input
%val1 = OpExtInst %f32 %extinst InterpolateAtOffset %ld1 %f32vec2_01
%ld2  = OpLoad %f32vec2 %f32vec2_input
%val2 = OpExtInst %f32vec2 %extinst InterpolateAtOffset %ld2 %f32vec2_01
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  getValidatorOptions()->before_hlsl_legalization = true;
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtOffsetInternalInvalidDataF32) {
  const std::string body = R"(
%ld1  = OpLoad %f32 %f32_input
%val1 = OpExtInst %f32 %extinst InterpolateAtOffset %ld1 %f32vec2_01
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtOffset: "
                        "expected Interpolant to be a pointer"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtOffsetInternalInvalidDataF32Vec2) {
  const std::string body = R"(
%ld2  = OpLoad %f32vec2 %f32vec2_input
%val2 = OpExtInst %f32vec2 %extinst InterpolateAtOffset %ld2 %f32vec2_01
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtOffset: "
                        "expected Interpolant to be a pointer"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtOffsetNoCapability) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtOffset %f32_input %f32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_CAPABILITY, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtOffset requires "
                        "capability InterpolationFunction"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtOffsetIntResultType) {
  const std::string body = R"(
%val1 = OpExtInst %u32 %extinst InterpolateAtOffset %f32_input %f32vec2_01
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtOffset: "
                        "expected Result Type to be a 32-bit float scalar "
                        "or vector type"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtOffsetF64ResultType) {
  const std::string body = R"(
%val1 = OpExtInst %f64 %extinst InterpolateAtOffset %f32_input %f32vec2_01
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtOffset: "
                        "expected Result Type to be a 32-bit float scalar "
                        "or vector type"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtOffsetNotPointer) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtOffset %f32_1 %f32vec2_01
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtOffset: "
                        "expected Interpolant to be a pointer"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtOffsetWrongDataType) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtOffset %f32vec2_input %f32vec2_01
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtOffset: "
                        "expected Interpolant data type to be equal to "
                        "Result Type"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtOffsetWrongStorageClass) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtOffset %f32_output %f32vec2_01
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtOffset: "
                        "expected Interpolant storage class to be Input"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtOffsetOffsetNotVector) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtOffset %f32_input %f32_0
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtOffset: "
                        "expected Offset to be a vector of 2 32-bit floats"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtOffsetOffsetNotVector2) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtOffset %f32_input %f32vec3_012
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtOffset: "
                        "expected Offset to be a vector of 2 32-bit floats"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtOffsetOffsetNotFloatVector) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtOffset %f32_input %u32vec2_01
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtOffset: "
                        "expected Offset to be a vector of 2 32-bit floats"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtOffsetOffsetNotFloat32Vector) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtOffset %f32_input %f64vec2_01
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability InterpolationFunction\n"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtOffset: "
                        "expected Offset to be a vector of 2 32-bit floats"));
}

TEST_F(ValidateExtInst, GlslStd450InterpolateAtOffsetWrongExecutionModel) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst InterpolateAtOffset %f32_input %f32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(
      body, "OpCapability InterpolationFunction\n", "Vertex"));
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GLSL.std.450 InterpolateAtOffset requires "
                        "Fragment execution model"));
}

TEST_P(ValidateOpenCLStdSqrtLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name << " %f32_0\n";
  ss << "%val2 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %f32vec2_01\n";
  ss << "%val3 = OpExtInst %f32vec4 %extinst " << ext_inst_name
     << " %f32vec4_0123\n";
  ss << "%val4 = OpExtInst %f64 %extinst " << ext_inst_name << " %f64_0\n";
  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdSqrtLike, IntResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %f32_0\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected Result Type to be a float scalar "
                        "or vector type"));
}

TEST_P(ValidateOpenCLStdSqrtLike, IntOperand) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f32 %extinst " + ext_inst_name + " %u32_0\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected types of all operands to be equal to "
                        "Result Type"));
}

INSTANTIATE_TEST_SUITE_P(
    AllSqrtLike, ValidateOpenCLStdSqrtLike,
    ::testing::ValuesIn(std::vector<std::string>{
        "acos",         "acosh",       "acospi",       "asin",
        "asinh",        "asinpi",      "atan",         "atanh",
        "atanpi",       "cbrt",        "ceil",         "cos",
        "cosh",         "cospi",       "erfc",         "erf",
        "exp",          "exp2",        "exp10",        "expm1",
        "fabs",         "floor",       "log",          "log2",
        "log10",        "log1p",       "logb",         "rint",
        "round",        "rsqrt",       "sin",          "sinh",
        "sinpi",        "sqrt",        "tan",          "tanh",
        "tanpi",        "tgamma",      "trunc",        "half_cos",
        "half_exp",     "half_exp2",   "half_exp10",   "half_log",
        "half_log2",    "half_log10",  "half_recip",   "half_rsqrt",
        "half_sin",     "half_sqrt",   "half_tan",     "lgamma",
        "native_cos",   "native_exp",  "native_exp2",  "native_exp10",
        "native_log",   "native_log2", "native_log10", "native_recip",
        "native_rsqrt", "native_sin",  "native_sqrt",  "native_tan",
        "degrees",      "radians",     "sign",
    }));

TEST_P(ValidateOpenCLStdFMinLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_0 %f32_1\n";
  ss << "%val2 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %f32vec2_01 %f32vec2_12\n";
  ss << "%val3 = OpExtInst %f64 %extinst " << ext_inst_name
     << " %f64_0 %f64_0\n";
  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdFMinLike, IntResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %f32_0 %f32_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected Result Type to be a float scalar "
                        "or vector type"));
}

TEST_P(ValidateOpenCLStdFMinLike, IntOperand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f32 %extinst " + ext_inst_name + " %u32_0 %f32_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected types of all operands to be equal to "
                        "Result Type"));
}

TEST_P(ValidateOpenCLStdFMinLike, IntOperand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f32 %extinst " + ext_inst_name + " %f32_0 %u32_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected types of all operands to be equal to "
                        "Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllFMinLike, ValidateOpenCLStdFMinLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "atan2",     "atan2pi",       "copysign",
                             "fdim",      "fmax",          "fmin",
                             "fmod",      "maxmag",        "minmag",
                             "hypot",     "nextafter",     "pow",
                             "powr",      "remainder",     "half_divide",
                             "half_powr", "native_divide", "native_powr",
                             "step",      "fmax_common",   "fmin_common",
                         }));

TEST_P(ValidateOpenCLStdFClampLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_0 %f32_1 %f32_2\n";
  ss << "%val2 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %f32vec2_01 %f32vec2_01 %f32vec2_12\n";
  ss << "%val3 = OpExtInst %f64 %extinst " << ext_inst_name
     << " %f64_0 %f64_0 %f64_1\n";
  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdFClampLike, IntResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %u32 %extinst " + ext_inst_name +
                           " %f32_0 %f32_1 %f32_2\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected Result Type to be a float scalar "
                        "or vector type"));
}

TEST_P(ValidateOpenCLStdFClampLike, IntOperand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %f32 %extinst " + ext_inst_name +
                           " %u32_0 %f32_0 %f32_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected types of all operands to be equal to "
                        "Result Type"));
}

TEST_P(ValidateOpenCLStdFClampLike, IntOperand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %f32 %extinst " + ext_inst_name +
                           " %f32_0 %u32_0 %f32_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected types of all operands to be equal to "
                        "Result Type"));
}

TEST_P(ValidateOpenCLStdFClampLike, IntOperand3) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %f32 %extinst " + ext_inst_name +
                           " %f32_1 %f32_0 %u32_2\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected types of all operands to be equal to "
                        "Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllFClampLike, ValidateOpenCLStdFClampLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "fma",
                             "mad",
                             "fclamp",
                             "mix",
                             "smoothstep",
                         }));

TEST_P(ValidateOpenCLStdSAbsLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %u32 %extinst " << ext_inst_name << " %u32_1\n";
  ss << "%val2 = OpExtInst %u32 %extinst " << ext_inst_name << " %u32_1\n";
  ss << "%val3 = OpExtInst %u32 %extinst " << ext_inst_name << " %u32_1\n";
  ss << "%val4 = OpExtInst %u32 %extinst " << ext_inst_name << " %u32_1\n";
  ss << "%val5 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01\n";
  ss << "%val6 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01\n";
  ss << "%val7 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01\n";
  ss << "%val8 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01\n";
  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdSAbsLike, FloatResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f32 %extinst " + ext_inst_name + " %u32_0\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected Result Type to be an int scalar "
                        "or vector type"));
}

TEST_P(ValidateOpenCLStdSAbsLike, FloatOperand) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %f32_0\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdSAbsLike, U64Operand) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %u64_0\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllSAbsLike, ValidateOpenCLStdSAbsLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "s_abs",
                             "clz",
                             "ctz",
                             "popcount",
                             "u_abs",
                         }));

TEST_P(ValidateOpenCLStdUMinLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_1 %u32_2\n";
  ss << "%val2 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_1 %u32_2\n";
  ss << "%val3 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_1 %u32_2\n";
  ss << "%val4 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_1 %u32_2\n";
  ss << "%val5 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01\n";
  ss << "%val6 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01\n";
  ss << "%val7 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01\n";
  ss << "%val8 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01\n";
  ss << "%val9 = OpExtInst %u64 %extinst " << ext_inst_name
     << " %u64_1 %u64_0\n";
  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdUMinLike, FloatResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f32 %extinst " + ext_inst_name + " %u32_0 %u32_0\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected Result Type to be an int scalar "
                        "or vector type"));
}

TEST_P(ValidateOpenCLStdUMinLike, FloatOperand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %f32_0 %u32_0\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdUMinLike, FloatOperand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %u32_0 %f32_0\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdUMinLike, U64Operand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %u64_0 %u32_0\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdUMinLike, U64Operand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %u32_0 %u64_0\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllUMinLike, ValidateOpenCLStdUMinLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "s_max",
                             "u_max",
                             "s_min",
                             "u_min",
                             "s_abs_diff",
                             "s_add_sat",
                             "u_add_sat",
                             "s_mul_hi",
                             "rotate",
                             "s_sub_sat",
                             "u_sub_sat",
                             "s_hadd",
                             "u_hadd",
                             "s_rhadd",
                             "u_rhadd",
                             "u_abs_diff",
                             "u_mul_hi",
                         }));

TEST_P(ValidateOpenCLStdUClampLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_0 %u32_1 %u32_2\n";
  ss << "%val2 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_0 %u32_1 %u32_2\n";
  ss << "%val3 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_0 %u32_1 %u32_2\n";
  ss << "%val4 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_0 %u32_1 %u32_2\n";
  ss << "%val5 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01 %u32vec2_12\n";
  ss << "%val6 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01 %u32vec2_12\n";
  ss << "%val7 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01 %u32vec2_12\n";
  ss << "%val8 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01 %u32vec2_12\n";
  ss << "%val9 = OpExtInst %u64 %extinst " << ext_inst_name
     << " %u64_1 %u64_0 %u64_1\n";
  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdUClampLike, FloatResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %f32 %extinst " + ext_inst_name +
                           " %u32_0 %u32_0 %u32_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected Result Type to be an int scalar "
                        "or vector type"));
}

TEST_P(ValidateOpenCLStdUClampLike, FloatOperand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %u32 %extinst " + ext_inst_name +
                           " %f32_0 %u32_0 %u32_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdUClampLike, FloatOperand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %u32 %extinst " + ext_inst_name +
                           " %u32_0 %f32_0 %u32_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdUClampLike, FloatOperand3) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %u32 %extinst " + ext_inst_name +
                           " %u32_0 %u32_0 %f32_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdUClampLike, U64Operand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %u32 %extinst " + ext_inst_name +
                           " %f32_0 %u32_0 %u64_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdUClampLike, U64Operand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %u32 %extinst " + ext_inst_name +
                           " %u32_0 %f32_0 %u64_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdUClampLike, U64Operand3) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %u32 %extinst " + ext_inst_name +
                           " %u32_0 %u32_0 %u64_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllUClampLike, ValidateOpenCLStdUClampLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "s_clamp",
                             "u_clamp",
                             "s_mad_hi",
                             "u_mad_sat",
                             "s_mad_sat",
                             "u_mad_hi",
                         }));

// -------------------------------------------------------------
TEST_P(ValidateOpenCLStdUMul24Like, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_1 %u32_2\n";
  ss << "%val2 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_1 %u32_2\n";
  ss << "%val3 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_1 %u32_2\n";
  ss << "%val4 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_1 %u32_2\n";
  ss << "%val5 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01\n";
  ss << "%val6 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01\n";
  ss << "%val7 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01\n";
  ss << "%val8 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01\n";
  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdUMul24Like, FloatResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f32 %extinst " + ext_inst_name + " %u32_0 %u32_0\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std " + ext_inst_name +
          ": expected Result Type to be a 32-bit int scalar or vector type"));
}

TEST_P(ValidateOpenCLStdUMul24Like, U64ResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u64 %extinst " + ext_inst_name + " %u64_0 %u64_0\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std " + ext_inst_name +
          ": expected Result Type to be a 32-bit int scalar or vector type"));
}

TEST_P(ValidateOpenCLStdUMul24Like, FloatOperand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %f32_0 %u32_0\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdUMul24Like, FloatOperand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %u32_0 %f32_0\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdUMul24Like, U64Operand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %u64_0 %u32_0\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdUMul24Like, U64Operand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %u32_0 %u64_0\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllUMul24Like, ValidateOpenCLStdUMul24Like,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "s_mul24",
                             "u_mul24",
                         }));

TEST_P(ValidateOpenCLStdUMad24Like, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_0 %u32_1 %u32_2\n";
  ss << "%val2 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_0 %u32_1 %u32_2\n";
  ss << "%val3 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_0 %u32_1 %u32_2\n";
  ss << "%val4 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u32_0 %u32_1 %u32_2\n";
  ss << "%val5 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01 %u32vec2_12\n";
  ss << "%val6 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01 %u32vec2_12\n";
  ss << "%val7 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01 %u32vec2_12\n";
  ss << "%val8 = OpExtInst %u32vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01 %u32vec2_12\n";
  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdUMad24Like, FloatResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %f32 %extinst " + ext_inst_name +
                           " %u32_0 %u32_0 %u32_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std " + ext_inst_name +
          ": expected Result Type to be a 32-bit int scalar or vector type"));
}

TEST_P(ValidateOpenCLStdUMad24Like, U64ResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %u64 %extinst " + ext_inst_name +
                           " %u64_0 %u64_0 %u64_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std " + ext_inst_name +
          ": expected Result Type to be a 32-bit int scalar or vector type"));
}

TEST_P(ValidateOpenCLStdUMad24Like, FloatOperand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %u32 %extinst " + ext_inst_name +
                           " %f32_0 %u32_0 %u32_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdUMad24Like, FloatOperand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %u32 %extinst " + ext_inst_name +
                           " %u32_0 %f32_0 %u32_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdUMad24Like, FloatOperand3) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %u32 %extinst " + ext_inst_name +
                           " %u32_0 %u32_0 %f32_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdUMad24Like, U64Operand1) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %u32 %extinst " + ext_inst_name +
                           " %f32_0 %u32_0 %u64_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdUMad24Like, U64Operand2) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %u32 %extinst " + ext_inst_name +
                           " %u32_0 %f32_0 %u64_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdUMad24Like, U64Operand3) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %u32 %extinst " + ext_inst_name +
                           " %u32_0 %u32_0 %u64_1\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected types of all operands to be equal to Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllUMad24Like, ValidateOpenCLStdUMad24Like,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "s_mad24",
                             "u_mad24",
                         }));

TEST_F(ValidateExtInst, OpenCLStdCrossSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec3 %extinst cross %f32vec3_012 %f32vec3_123
%val2 = OpExtInst %f32vec4 %extinst cross %f32vec4_0123 %f32vec4_0123
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, OpenCLStdCrossIntVectorResultType) {
  const std::string body = R"(
%val1 = OpExtInst %u32vec3 %extinst cross %f32vec3_012 %f32vec3_123
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std cross: "
                        "expected Result Type to be a float vector type"));
}

TEST_F(ValidateExtInst, OpenCLStdCrossResultTypeWrongSize) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst cross %f32vec3_012 %f32vec3_123
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std cross: "
                        "expected Result Type to have 3 or 4 components"));
}

TEST_F(ValidateExtInst, OpenCLStdCrossXWrongType) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec3 %extinst cross %f64vec3_012 %f32vec3_123
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std cross: "
                        "expected operand X type to be equal to Result Type"));
}

TEST_F(ValidateExtInst, OpenCLStdCrossYWrongType) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec3 %extinst cross %f32vec3_123 %f64vec3_012
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std cross: "
                        "expected operand Y type to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdLengthLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name << " %f32vec2_01\n";
  ss << "%val2 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32vec4_0123\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdLengthLike, IntResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %f32vec2_01\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": "
                        "expected Result Type to be a float scalar type"));
}

TEST_P(ValidateOpenCLStdLengthLike, IntX) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f32 %extinst " + ext_inst_name + " %u32vec2_01\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": "
                        "expected operand P to be a float scalar or vector"));
}

TEST_P(ValidateOpenCLStdLengthLike, VectorTooBig) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %f32 %extinst " + ext_inst_name +
                           " %f32vec8_01010101\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": "
                "expected operand P to have no more than 4 components"));
}

TEST_P(ValidateOpenCLStdLengthLike, DifferentType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f64 %extinst " + ext_inst_name + " %f32vec2_01\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": "
                        "expected operand P component type to be equal to "
                        "Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllLengthLike, ValidateOpenCLStdLengthLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "length",
                             "fast_length",
                         }));

TEST_P(ValidateOpenCLStdDistanceLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32vec2_01 %f32vec2_01\n";
  ss << "%val2 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32vec4_0123 %f32vec4_1234\n";
  ss << "%val3 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_0 %f32_1\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdDistanceLike, IntResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %u32 %extinst " + ext_inst_name +
                           " %f32vec2_01 %f32vec2_12\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": "
                        "expected Result Type to be a float scalar type"));
}

TEST_P(ValidateOpenCLStdDistanceLike, IntP0) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %f32 %extinst " + ext_inst_name +
                           " %u32vec2_01 %f32vec2_12\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": "
                "expected operand P0 to be of float scalar or vector type"));
}

TEST_P(ValidateOpenCLStdDistanceLike, VectorTooBig) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %f32 %extinst " + ext_inst_name +
                           " %f32vec8_01010101 %f32vec8_01010101\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": "
                "expected operand P0 to have no more than 4 components"));
}

TEST_P(ValidateOpenCLStdDistanceLike, F64P0) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %f32 %extinst " + ext_inst_name +
                           " %f64vec2_01 %f32vec2_12\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std " + ext_inst_name +
          ": "
          "expected operand P0 component type to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdDistanceLike, DifferentOperands) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %f64 %extinst " + ext_inst_name +
                           " %f64vec2_01 %f32vec2_12\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": "
                        "expected operands P0 and P1 to be of the same type"));
}

INSTANTIATE_TEST_SUITE_P(AllDistanceLike, ValidateOpenCLStdDistanceLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "distance",
                             "fast_distance",
                         }));

TEST_P(ValidateOpenCLStdNormalizeLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %f32vec2_01\n";
  ss << "%val2 = OpExtInst %f32vec4 %extinst " << ext_inst_name
     << " %f32vec4_0123\n";
  ss << "%val3 = OpExtInst %f32 %extinst " << ext_inst_name << " %f32_2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdNormalizeLike, IntResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %u32 %extinst " + ext_inst_name + " %f32_2\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": "
                "expected Result Type to be a float scalar or vector type"));
}

TEST_P(ValidateOpenCLStdNormalizeLike, VectorTooBig) {
  const std::string ext_inst_name = GetParam();
  const std::string body = "%val1 = OpExtInst %f32vec8 %extinst " +
                           ext_inst_name + " %f32vec8_01010101\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": "
                "expected Result Type to have no more than 4 components"));
}

TEST_P(ValidateOpenCLStdNormalizeLike, DifferentType) {
  const std::string ext_inst_name = GetParam();
  const std::string body =
      "%val1 = OpExtInst %f64vec2 %extinst " + ext_inst_name + " %f32vec2_01\n";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": "
                        "expected operand P type to be equal to Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllNormalizeLike, ValidateOpenCLStdNormalizeLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "normalize",
                             "fast_normalize",
                         }));

TEST_F(ValidateExtInst, OpenCLStdBitselectSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst bitselect %f32_2 %f32_1 %f32_1
%val2 = OpExtInst %f32vec4 %extinst bitselect %f32vec4_0123 %f32vec4_1234 %f32vec4_0123
%val3 = OpExtInst %u32 %extinst bitselect %u32_2 %u32_1 %u32_1
%val4 = OpExtInst %u32vec4 %extinst bitselect %u32vec4_0123 %u32vec4_0123 %u32vec4_0123
%val5 = OpExtInst %u64 %extinst bitselect %u64_2 %u64_1 %u64_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, OpenCLStdBitselectWrongResultType) {
  const std::string body = R"(
%val3 = OpExtInst %struct_f32_f32 %extinst bitselect %u32_2 %u32_1 %u32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std bitselect: "
          "expected Result Type to be an int or float scalar or vector type"));
}

TEST_F(ValidateExtInst, OpenCLStdBitselectAWrongType) {
  const std::string body = R"(
%val3 = OpExtInst %u32 %extinst bitselect %f32_2 %u32_1 %u32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std bitselect: "
                "expected types of all operands to be equal to Result Type"));
}

TEST_F(ValidateExtInst, OpenCLStdBitselectBWrongType) {
  const std::string body = R"(
%val3 = OpExtInst %u32 %extinst bitselect %u32_2 %f32_1 %u32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std bitselect: "
                "expected types of all operands to be equal to Result Type"));
}

TEST_F(ValidateExtInst, OpenCLStdBitselectCWrongType) {
  const std::string body = R"(
%val3 = OpExtInst %u32 %extinst bitselect %u32_2 %u32_1 %f32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std bitselect: "
                "expected types of all operands to be equal to Result Type"));
}

TEST_F(ValidateExtInst, OpenCLStdSelectSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst select %f32_2 %f32_1 %u32_1
%val2 = OpExtInst %f32vec4 %extinst select %f32vec4_0123 %f32vec4_1234 %u32vec4_0123
%val3 = OpExtInst %u32 %extinst select %u32_2 %u32_1 %u32_1
%val4 = OpExtInst %u32vec4 %extinst select %u32vec4_0123 %u32vec4_0123 %u32vec4_0123
%val5 = OpExtInst %u64 %extinst select %u64_2 %u64_1 %u64_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, OpenCLStdSelectWrongResultType) {
  const std::string body = R"(
%val3 = OpExtInst %struct_f32_f32 %extinst select %u32_2 %u32_1 %u32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std select: "
          "expected Result Type to be an int or float scalar or vector type"));
}

TEST_F(ValidateExtInst, OpenCLStdSelectAWrongType) {
  const std::string body = R"(
%val3 = OpExtInst %u32 %extinst select %f32_2 %u32_1 %u32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std select: "
                        "expected operand A type to be equal to Result Type"));
}

TEST_F(ValidateExtInst, OpenCLStdSelectBWrongType) {
  const std::string body = R"(
%val3 = OpExtInst %u32 %extinst select %u32_2 %f32_1 %u32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std select: "
                        "expected operand B type to be equal to Result Type"));
}

TEST_F(ValidateExtInst, OpenCLStdSelectCWrongType) {
  const std::string body = R"(
%val3 = OpExtInst %f32 %extinst select %f32_2 %f32_1 %f32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std select: "
                        "expected operand C to be an int scalar or vector"));
}

TEST_F(ValidateExtInst, OpenCLStdSelectCWrongComponentNumber) {
  const std::string body = R"(
%val3 = OpExtInst %f32vec2 %extinst select %f32vec2_12 %f32vec2_01 %u32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std select: "
                        "expected operand C to have the same number of "
                        "components as Result Type"));
}

TEST_F(ValidateExtInst, OpenCLStdSelectCWrongBitWidth) {
  const std::string body = R"(
%val3 = OpExtInst %f32vec2 %extinst select %f32vec2_12 %f32vec2_01 %u64vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std select: "
          "expected operand C to have the same bit width as Result Type"));
}

TEST_P(ValidateOpenCLStdVStoreHalfLike, SuccessPhysical32) {
  const std::string ext_inst_name = GetParam();
  const std::string rounding_mode =
      ext_inst_name.substr(ext_inst_name.length() - 2) == "_r" ? " RTE" : "";

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_workgroup %f16vec8_workgroup %u32_1\n";
  if (std::string::npos == ext_inst_name.find("halfn")) {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32_1 %u32_1 %ptr" << rounding_mode << "\n";
    ss << "%val2 = OpExtInst %void %extinst " << ext_inst_name
       << " %f64_0 %u32_2 %ptr" << rounding_mode << "\n";
  } else {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32vec2_01 %u32_1 %ptr" << rounding_mode << "\n";
    ss << "%val2 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32vec4_0123 %u32_0 %ptr" << rounding_mode << "\n";
    ss << "%val3 = OpExtInst %void %extinst " << ext_inst_name
       << " %f64vec2_01 %u32_2 %ptr" << rounding_mode << "\n";
  }

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdVStoreHalfLike, SuccessPhysical64) {
  const std::string ext_inst_name = GetParam();
  const std::string rounding_mode =
      ext_inst_name.substr(ext_inst_name.length() - 2) == "_r" ? " RTE" : "";

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_workgroup %f16vec8_workgroup %u32_1\n";
  if (std::string::npos == ext_inst_name.find("halfn")) {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32_1 %u64_1 %ptr" << rounding_mode << "\n";
    ss << "%val2 = OpExtInst %void %extinst " << ext_inst_name
       << " %f64_0 %u64_2 %ptr" << rounding_mode << "\n";
  } else {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32vec2_01 %u64_1 %ptr" << rounding_mode << "\n";
    ss << "%val2 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32vec4_0123 %u64_0 %ptr" << rounding_mode << "\n";
    ss << "%val3 = OpExtInst %void %extinst " << ext_inst_name
       << " %f64vec2_01 %u64_2 %ptr" << rounding_mode << "\n";
  }

  CompileSuccessfully(GenerateKernelCode(ss.str(), "", "Physical64"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdVStoreHalfLike, NonVoidResultType) {
  const std::string ext_inst_name = GetParam();
  const std::string rounding_mode =
      ext_inst_name.substr(ext_inst_name.length() - 2) == "_r" ? " RTE" : "";

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_workgroup %f16vec8_workgroup %u32_1\n";
  if (std::string::npos == ext_inst_name.find("halfn")) {
    ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
       << " %f32_1 %u32_1 %ptr" << rounding_mode << "\n";
  } else {
    ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
       << " %f32vec2_01 %u32_1 %ptr" << rounding_mode << "\n";
  }

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected Result Type to be void"));
}

TEST_P(ValidateOpenCLStdVStoreHalfLike, WrongDataType) {
  const std::string ext_inst_name = GetParam();
  const std::string rounding_mode =
      ext_inst_name.substr(ext_inst_name.length() - 2) == "_r" ? " RTE" : "";

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_workgroup %f16vec8_workgroup %u32_1\n";
  if (std::string::npos == ext_inst_name.find("halfn")) {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f64vec2_01 %u32_1 %ptr" << rounding_mode << "\n";
    CompileSuccessfully(GenerateKernelCode(ss.str()));
    ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("OpenCL.std " + ext_inst_name +
                          ": expected Data to be a 32 or 64-bit float scalar"));
  } else {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f64_0 %u32_1 %ptr" << rounding_mode << "\n";
    CompileSuccessfully(GenerateKernelCode(ss.str()));
    ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("OpenCL.std " + ext_inst_name +
                          ": expected Data to be a 32 or 64-bit float vector"));
  }
}

TEST_P(ValidateOpenCLStdVStoreHalfLike, AddressingModelLogical) {
  const std::string ext_inst_name = GetParam();
  const std::string rounding_mode =
      ext_inst_name.substr(ext_inst_name.length() - 2) == "_r" ? " RTE" : "";

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_workgroup %f16vec8_workgroup %u32_1\n";
  if (std::string::npos == ext_inst_name.find("halfn")) {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32_0 %u32_1 %ptr" << rounding_mode << "\n";
  } else {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32vec2_01 %u32_1 %ptr" << rounding_mode << "\n";
  }

  CompileSuccessfully(GenerateKernelCode(ss.str(), "", "Logical"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        " can only be used with physical addressing models"));
}

TEST_P(ValidateOpenCLStdVStoreHalfLike, OffsetNotSizeT) {
  const std::string ext_inst_name = GetParam();
  const std::string rounding_mode =
      ext_inst_name.substr(ext_inst_name.length() - 2) == "_r" ? " RTE" : "";

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_workgroup %f16vec8_workgroup %u32_1\n";
  if (std::string::npos == ext_inst_name.find("halfn")) {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32_0 %u32_1 %ptr" << rounding_mode << "\n";
  } else {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32vec2_01 %u32_1 %ptr" << rounding_mode << "\n";
  }

  CompileSuccessfully(GenerateKernelCode(ss.str(), "", "Physical64"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": "
                "expected operand Offset to be of type size_t (64-bit integer "
                "for the addressing model used in the module)"));
}

TEST_P(ValidateOpenCLStdVStoreHalfLike, PNotPointer) {
  const std::string ext_inst_name = GetParam();
  const std::string rounding_mode =
      ext_inst_name.substr(ext_inst_name.length() - 2) == "_r" ? " RTE" : "";

  std::ostringstream ss;
  if (std::string::npos == ext_inst_name.find("halfn")) {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32_0 %u32_1 %f16_ptr_workgroup" << rounding_mode << "\n";
  } else {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32vec2_01 %u32_1 %f16_ptr_workgroup" << rounding_mode << "\n";
  }

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Operand 89[%_ptr_Workgroup_half] cannot be a type"));
}

TEST_P(ValidateOpenCLStdVStoreHalfLike, ConstPointer) {
  const std::string ext_inst_name = GetParam();
  const std::string rounding_mode =
      ext_inst_name.substr(ext_inst_name.length() - 2) == "_r" ? " RTE" : "";

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_uniform_constant "
        "%f16vec8_uniform_constant %u32_1\n";
  if (std::string::npos == ext_inst_name.find("halfn")) {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32_0 %u32_1 %ptr" << rounding_mode << "\n";
  } else {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32vec2_01 %u32_1 %ptr" << rounding_mode << "\n";
  }

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected operand P storage class to be Generic, "
                        "CrossWorkgroup, Workgroup or Function"));
}

TEST_P(ValidateOpenCLStdVStoreHalfLike, PDataTypeInt) {
  const std::string ext_inst_name = GetParam();
  const std::string rounding_mode =
      ext_inst_name.substr(ext_inst_name.length() - 2) == "_r" ? " RTE" : "";

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %u32_ptr_workgroup %u32vec8_workgroup %u32_1\n";
  if (std::string::npos == ext_inst_name.find("halfn")) {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32_0 %u32_1 %ptr" << rounding_mode << "\n";
  } else {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32vec2_01 %u32_1 %ptr" << rounding_mode << "\n";
  }

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected operand P data type to be 16-bit float scalar"));
}

TEST_P(ValidateOpenCLStdVStoreHalfLike, PDataTypeFloat32) {
  const std::string ext_inst_name = GetParam();
  const std::string rounding_mode =
      ext_inst_name.substr(ext_inst_name.length() - 2) == "_r" ? " RTE" : "";

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f32_ptr_workgroup %f32vec8_workgroup %u32_1\n";
  if (std::string::npos == ext_inst_name.find("halfn")) {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32_0 %u32_1 %ptr" << rounding_mode << "\n";
  } else {
    ss << "%val1 = OpExtInst %void %extinst " << ext_inst_name
       << " %f32vec2_01 %u32_1 %ptr" << rounding_mode << "\n";
  }

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected operand P data type to be 16-bit float scalar"));
}

INSTANTIATE_TEST_SUITE_P(AllVStoreHalfLike, ValidateOpenCLStdVStoreHalfLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "vstore_half",
                             "vstore_half_r",
                             "vstore_halfn",
                             "vstore_halfn_r",
                             "vstorea_halfn",
                             "vstorea_halfn_r",
                         }));

TEST_P(ValidateOpenCLStdVLoadHalfLike, SuccessPhysical32) {
  const std::string ext_inst_name = GetParam();

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_workgroup %f16vec8_workgroup %u32_1\n";
  ss << "%val1 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %u32_1 %ptr 2\n";
  ss << "%val2 = OpExtInst %f32vec3 %extinst " << ext_inst_name
     << " %u32_1 %ptr 3\n";
  ss << "%val3 = OpExtInst %f32vec4 %extinst " << ext_inst_name
     << " %u32_1 %ptr 4\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdVLoadHalfLike, SuccessPhysical64) {
  const std::string ext_inst_name = GetParam();

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_workgroup %f16vec8_workgroup %u32_1\n";
  ss << "%val1 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %u64_1 %ptr 2\n";
  ss << "%val2 = OpExtInst %f32vec3 %extinst " << ext_inst_name
     << " %u64_1 %ptr 3\n";
  ss << "%val3 = OpExtInst %f32vec4 %extinst " << ext_inst_name
     << " %u64_1 %ptr 4\n";

  CompileSuccessfully(GenerateKernelCode(ss.str(), "", "Physical64"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdVLoadHalfLike, ResultTypeNotFloatVector) {
  const std::string ext_inst_name = GetParam();

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_workgroup %f16vec8_workgroup %u32_1\n";
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %u32_1 %ptr 1\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected Result Type to be a float vector type"));
}

TEST_P(ValidateOpenCLStdVLoadHalfLike, AddressingModelLogical) {
  const std::string ext_inst_name = GetParam();

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_workgroup %f16vec8_workgroup %u32_1\n";
  ss << "%val1 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %u32_1 %ptr 2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str(), "", "Logical"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        " can only be used with physical addressing models"));
}

TEST_P(ValidateOpenCLStdVLoadHalfLike, OffsetNotSizeT) {
  const std::string ext_inst_name = GetParam();

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_workgroup %f16vec8_workgroup %u32_1\n";
  ss << "%val1 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %u64_1 %ptr 2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected operand Offset to be of type size_t (32-bit "
                "integer for the addressing model used in the module)"));
}

TEST_P(ValidateOpenCLStdVLoadHalfLike, PNotPointer) {
  const std::string ext_inst_name = GetParam();

  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %u32_1 %f16_ptr_workgroup 2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Operand 89[%_ptr_Workgroup_half] cannot be a type"));
}

TEST_P(ValidateOpenCLStdVLoadHalfLike, OffsetWrongStorageType) {
  const std::string ext_inst_name = GetParam();

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_input %f16vec8_input %u32_1\n";
  ss << "%val1 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %u32_1 %ptr 2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected operand P storage class to be UniformConstant, "
                "Generic, CrossWorkgroup, Workgroup or Function"));
}

TEST_P(ValidateOpenCLStdVLoadHalfLike, PDataTypeInt) {
  const std::string ext_inst_name = GetParam();

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %u32_ptr_workgroup %u32vec8_workgroup %u32_1\n";
  ss << "%val1 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %u32_1 %ptr 2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected operand P data type to be 16-bit float scalar"));
}

TEST_P(ValidateOpenCLStdVLoadHalfLike, PDataTypeFloat32) {
  const std::string ext_inst_name = GetParam();

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f32_ptr_workgroup %f32vec8_workgroup %u32_1\n";
  ss << "%val1 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %u32_1 %ptr 2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected operand P data type to be 16-bit float scalar"));
}

TEST_P(ValidateOpenCLStdVLoadHalfLike, WrongN) {
  const std::string ext_inst_name = GetParam();

  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_workgroup %f16vec8_workgroup %u32_1\n";
  ss << "%val1 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %u32_1 %ptr 3\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected literal N to be equal to the number of "
                        "components of Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllVLoadHalfLike, ValidateOpenCLStdVLoadHalfLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "vload_halfn",
                             "vloada_halfn",
                         }));

TEST_F(ValidateExtInst, VLoadNSuccessFloatPhysical32) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f32_ptr_uniform_constant "
        "%f32vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %f32vec2 %extinst vloadn %u32_1 %ptr 2\n";
  ss << "%val2 = OpExtInst %f32vec3 %extinst vloadn %u32_1 %ptr 3\n";
  ss << "%val3 = OpExtInst %f32vec4 %extinst vloadn %u32_1 %ptr 4\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, VLoadNSuccessIntPhysical32) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %u32_ptr_uniform_constant "
        "%u32vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %u32vec2 %extinst vloadn %u32_1 %ptr 2\n";
  ss << "%val2 = OpExtInst %u32vec3 %extinst vloadn %u32_1 %ptr 3\n";
  ss << "%val3 = OpExtInst %u32vec4 %extinst vloadn %u32_1 %ptr 4\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, VLoadNSuccessFloatPhysical64) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f32_ptr_uniform_constant "
        "%f32vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %f32vec2 %extinst vloadn %u64_1 %ptr 2\n";
  ss << "%val2 = OpExtInst %f32vec3 %extinst vloadn %u64_1 %ptr 3\n";
  ss << "%val3 = OpExtInst %f32vec4 %extinst vloadn %u64_1 %ptr 4\n";

  CompileSuccessfully(GenerateKernelCode(ss.str(), "", "Physical64"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, VLoadNSuccessIntPhysical64) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %u32_ptr_uniform_constant "
        "%u32vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %u32vec2 %extinst vloadn %u64_1 %ptr 2\n";
  ss << "%val2 = OpExtInst %u32vec3 %extinst vloadn %u64_1 %ptr 3\n";
  ss << "%val3 = OpExtInst %u32vec4 %extinst vloadn %u64_1 %ptr 4\n";

  CompileSuccessfully(GenerateKernelCode(ss.str(), "", "Physical64"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, VLoadNWrongResultType) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f32_ptr_uniform_constant "
        "%f32vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %f32 %extinst vloadn %u32_1 %ptr 2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std vloadn: "
                "expected Result Type to be an int or float vector type"));
}

TEST_F(ValidateExtInst, VLoadNAddressingModelLogical) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f32_ptr_uniform_constant "
        "%f32vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %f32vec2 %extinst vloadn %u32_1 %ptr 2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str(), "", "Logical"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std vloadn can only be used with physical "
                        "addressing models"));
}

TEST_F(ValidateExtInst, VLoadNOffsetNotSizeT) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f32_ptr_uniform_constant "
        "%f32vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %f32vec2 %extinst vloadn %u64_1 %ptr 2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std vloadn: expected operand Offset to be of type size_t "
          "(32-bit integer for the addressing model used in the module)"));
}

TEST_F(ValidateExtInst, VLoadNPNotPointer) {
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32vec2 %extinst vloadn %u32_1 "
        "%f32_ptr_uniform_constant 2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Operand 120[%_ptr_UniformConstant_float] cannot be a "
                        "type"));
}

TEST_F(ValidateExtInst, VLoadNWrongStorageClass) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %u32_ptr_input %u32vec8_input %u32_1\n";
  ss << "%val1 = OpExtInst %u32vec2 %extinst vloadn %u32_1 %ptr 2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std vloadn: expected operand P storage class "
                        "to be UniformConstant, Generic, CrossWorkgroup, "
                        "Workgroup or Function"));
}

TEST_F(ValidateExtInst, VLoadNWrongComponentType) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f32_ptr_uniform_constant "
        "%f32vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %u32vec2 %extinst vloadn %u32_1 %ptr 2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std vloadn: expected operand P data type to be "
                        "equal to component type of Result Type"));
}

TEST_F(ValidateExtInst, VLoadNWrongN) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f32_ptr_uniform_constant "
        "%f32vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %f32vec2 %extinst vloadn %u32_1 %ptr 3\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std vloadn: expected literal N to be equal to "
                        "the number of components of Result Type"));
}

TEST_F(ValidateExtInst, VLoadHalfSuccessPhysical32) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_uniform_constant "
        "%f16vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %f32 %extinst vload_half %u32_1 %ptr\n";
  ss << "%val2 = OpExtInst %f64 %extinst vload_half %u32_1 %ptr\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, VLoadHalfSuccessPhysical64) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_uniform_constant "
        "%f16vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %f32 %extinst vload_half %u64_1 %ptr\n";
  ss << "%val2 = OpExtInst %f64 %extinst vload_half %u64_1 %ptr\n";

  CompileSuccessfully(GenerateKernelCode(ss.str(), "", "Physical64"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, VLoadHalfWrongResultType) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_uniform_constant "
        "%f16vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %u32 %extinst vload_half %u32_1 %ptr\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std vload_half: "
                        "expected Result Type to be a float scalar type"));
}

TEST_F(ValidateExtInst, VLoadHalfAddressingModelLogical) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_uniform_constant "
        "%f16vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %f32 %extinst vload_half %u32_1 %ptr\n";

  CompileSuccessfully(GenerateKernelCode(ss.str(), "", "Logical"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std vload_half can only be used with physical "
                        "addressing models"));
}

TEST_F(ValidateExtInst, VLoadHalfOffsetNotSizeT) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_uniform_constant "
        "%f16vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %f32 %extinst vload_half %u64_1 %ptr\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std vload_half: expected operand Offset to be of type size_t "
          "(32-bit integer for the addressing model used in the module)"));
}

TEST_F(ValidateExtInst, VLoadHalfPNotPointer) {
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst vload_half %u32_1 "
        "%f16_ptr_uniform_constant\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Operand 114[%_ptr_UniformConstant_half] cannot be a "
                        "type"));
}

TEST_F(ValidateExtInst, VLoadHalfWrongStorageClass) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f16_ptr_input %f16vec8_input %u32_1\n";
  ss << "%val1 = OpExtInst %f32 %extinst vload_half %u32_1 %ptr\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std vload_half: expected operand P storage class to be "
          "UniformConstant, Generic, CrossWorkgroup, Workgroup or Function"));
}

TEST_F(ValidateExtInst, VLoadHalfPDataTypeInt) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %u32_ptr_uniform_constant "
        "%u32vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %f32 %extinst vload_half %u32_1 %ptr\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std vload_half: expected operand P data type "
                        "to be 16-bit float scalar"));
}

TEST_F(ValidateExtInst, VLoadHalfPDataTypeFloat32) {
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f32_ptr_uniform_constant "
        "%f32vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %f32 %extinst vload_half %u32_1 %ptr\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std vload_half: expected operand P data type "
                        "to be 16-bit float scalar"));
}

TEST_F(ValidateExtInst, VStoreNSuccessFloatPhysical32) {
  std::ostringstream ss;
  ss << "%ptr_w = OpAccessChain %f32_ptr_workgroup %f32vec8_workgroup %u32_1\n";
  ss << "%ptr_g = OpPtrCastToGeneric %f32_ptr_generic %ptr_w\n";
  ss << "%val1 = OpExtInst %void %extinst vstoren %f32vec2_01 %u32_1 %ptr_g\n";
  ss << "%val2 = OpExtInst %void %extinst vstoren %f32vec4_0123 %u32_1 "
        "%ptr_g\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, VStoreNSuccessFloatPhysical64) {
  std::ostringstream ss;
  ss << "%ptr_w = OpAccessChain %f32_ptr_workgroup %f32vec8_workgroup %u32_1\n";
  ss << "%ptr_g = OpPtrCastToGeneric %f32_ptr_generic %ptr_w\n";
  ss << "%val1 = OpExtInst %void %extinst vstoren %f32vec2_01 %u64_1 %ptr_g\n";
  ss << "%val2 = OpExtInst %void %extinst vstoren %f32vec4_0123 %u64_1 "
        "%ptr_g\n";

  CompileSuccessfully(GenerateKernelCode(ss.str(), "", "Physical64"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, VStoreNSuccessIntPhysical32) {
  std::ostringstream ss;
  ss << "%ptr_w = OpAccessChain %u32_ptr_workgroup %u32vec8_workgroup %u32_1\n";
  ss << "%ptr_g = OpPtrCastToGeneric %u32_ptr_generic %ptr_w\n";
  ss << "%val1 = OpExtInst %void %extinst vstoren %u32vec2_01 %u32_1 %ptr_g\n";
  ss << "%val2 = OpExtInst %void %extinst vstoren %u32vec4_0123 %u32_1 "
        "%ptr_g\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, VStoreNSuccessIntPhysical64) {
  std::ostringstream ss;
  ss << "%ptr_w = OpAccessChain %u32_ptr_workgroup %u32vec8_workgroup %u32_1\n";
  ss << "%ptr_g = OpPtrCastToGeneric %u32_ptr_generic %ptr_w\n";
  ss << "%val1 = OpExtInst %void %extinst vstoren %u32vec2_01 %u64_1 %ptr_g\n";
  ss << "%val2 = OpExtInst %void %extinst vstoren %u32vec4_0123 %u64_1 "
        "%ptr_g\n";

  CompileSuccessfully(GenerateKernelCode(ss.str(), "", "Physical64"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, VStoreNResultTypeNotVoid) {
  std::ostringstream ss;
  ss << "%ptr_w = OpAccessChain %f32_ptr_workgroup %f32vec8_workgroup %u32_1\n";
  ss << "%ptr_g = OpPtrCastToGeneric %f32_ptr_generic %ptr_w\n";
  ss << "%val1 = OpExtInst %f32 %extinst vstoren %f32vec2_01 %u32_1 %ptr_g\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std vstoren: expected Result Type to be void"));
}

TEST_F(ValidateExtInst, VStoreNDataWrongType) {
  std::ostringstream ss;
  ss << "%ptr_w = OpAccessChain %f32_ptr_workgroup %f32vec8_workgroup %u32_1\n";
  ss << "%ptr_g = OpPtrCastToGeneric %f32_ptr_generic %ptr_w\n";
  ss << "%val1 = OpExtInst %void %extinst vstoren %f32_1 %u32_1 %ptr_g\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std vstoren: expected Data to be an int or float vector"));
}

TEST_F(ValidateExtInst, VStoreNAddressingModelLogical) {
  std::ostringstream ss;
  ss << "%ptr_w = OpAccessChain %f32_ptr_workgroup %f32vec8_workgroup %u32_1\n";
  ss << "%ptr_g = OpPtrCastToGeneric %f32_ptr_generic %ptr_w\n";
  ss << "%val1 = OpExtInst %void %extinst vstoren %f32vec2_01 %u32_1 %ptr_g\n";

  CompileSuccessfully(GenerateKernelCode(ss.str(), "", "Logical"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std vstoren can only be used with physical "
                        "addressing models"));
}

TEST_F(ValidateExtInst, VStoreNOffsetNotSizeT) {
  std::ostringstream ss;
  ss << "%ptr_w = OpAccessChain %f32_ptr_workgroup %f32vec8_workgroup %u32_1\n";
  ss << "%ptr_g = OpPtrCastToGeneric %f32_ptr_generic %ptr_w\n";
  ss << "%val1 = OpExtInst %void %extinst vstoren %f32vec2_01 %u32_1 %ptr_g\n";

  CompileSuccessfully(GenerateKernelCode(ss.str(), "", "Physical64"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std vstoren: expected operand Offset to be of type size_t "
          "(64-bit integer for the addressing model used in the module)"));
}

TEST_F(ValidateExtInst, VStoreNPNotPointer) {
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %void %extinst vstoren %f32vec2_01 %u32_1 "
        "%f32_ptr_generic\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Operand 127[%_ptr_Generic_float] cannot be a type"));
}

TEST_F(ValidateExtInst, VStoreNWrongStorageClass) {
  std::ostringstream ss;
  ss << "%ptr_w = OpAccessChain %f32_ptr_uniform_constant "
        "%f32vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %void %extinst vstoren %f32vec2_01 %u32_1 %ptr_w\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std vstoren: expected operand P storage class "
                "to be Generic, CrossWorkgroup, Workgroup or Function"));
}

TEST_F(ValidateExtInst, VStorePWrongDataType) {
  std::ostringstream ss;
  ss << "%ptr_w = OpAccessChain %f32_ptr_workgroup %f32vec8_workgroup %u32_1\n";
  ss << "%ptr_g = OpPtrCastToGeneric %f32_ptr_generic %ptr_w\n";
  ss << "%val1 = OpExtInst %void %extinst vstoren %u32vec2_01 %u32_1 %ptr_g\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std vstoren: expected operand P data type to "
                        "be equal to the type of operand Data components"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffleSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst shuffle %f32vec4_0123 %u32vec2_01
%val2 = OpExtInst %f32vec4 %extinst shuffle %f32vec4_0123 %u32vec4_0123
%val3 = OpExtInst %u32vec2 %extinst shuffle %u32vec4_0123 %u32vec2_01
%val4 = OpExtInst %u32vec4 %extinst shuffle %u32vec4_0123 %u32vec4_0123
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, OpenCLStdShuffleWrongResultType) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst shuffle %f32vec4_0123 %u32vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std shuffle: "
                "expected Result Type to be an int or float vector type"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffleResultTypeInvalidNumComponents) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec3 %extinst shuffle %f32vec4_0123 %u32vec3_012
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std shuffle: "
                "expected Result Type to have 2, 4, 8 or 16 components"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffleXWrongType) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst shuffle %f32_0 %u32vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std shuffle: "
                        "expected operand X to be an int or float vector"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffleXInvalidNumComponents) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst shuffle %f32vec3_012 %u32vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std shuffle: "
                        "expected operand X to have 2, 4, 8 or 16 components"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffleXInvalidComponentType) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst shuffle %f64vec4_0123 %u32vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std shuffle: "
          "expected operand X and Result Type to have equal component types"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffleShuffleMaskNotIntVector) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst shuffle %f32vec4_0123 %f32vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std shuffle: "
                        "expected operand Shuffle Mask to be an int vector"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffleShuffleMaskInvalidNumComponents) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec4 %extinst shuffle %f32vec4_0123 %u32vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std shuffle: "
                        "expected operand Shuffle Mask to have the same number "
                        "of components as Result Type"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffleShuffleMaskInvalidBitWidth) {
  const std::string body = R"(
%val1 = OpExtInst %f64vec2 %extinst shuffle %f64vec4_0123 %u32vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std shuffle: "
                        "expected operand Shuffle Mask components to have the "
                        "same bit width as Result Type components"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffle2Success) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst shuffle2 %f32vec4_0123 %f32vec4_0123 %u32vec2_01
%val2 = OpExtInst %f32vec4 %extinst shuffle2 %f32vec4_0123 %f32vec4_0123 %u32vec4_0123
%val3 = OpExtInst %u32vec2 %extinst shuffle2 %u32vec4_0123 %u32vec4_0123 %u32vec2_01
%val4 = OpExtInst %u32vec4 %extinst shuffle2 %u32vec4_0123 %u32vec4_0123 %u32vec4_0123
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, OpenCLStdShuffle2WrongResultType) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst shuffle2 %f32vec4_0123 %f32vec4_0123 %u32vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std shuffle2: "
                "expected Result Type to be an int or float vector type"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffle2ResultTypeInvalidNumComponents) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec3 %extinst shuffle2 %f32vec4_0123 %f32vec4_0123 %u32vec3_012
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std shuffle2: "
                "expected Result Type to have 2, 4, 8 or 16 components"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffle2XWrongType) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst shuffle2 %f32_0 %f32_0 %u32vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std shuffle2: "
                        "expected operand X to be an int or float vector"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffle2YTypeDifferentFromX) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst shuffle2 %f32vec2_01 %f32vec4_0123 %u32vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std shuffle2: "
                        "expected operands X and Y to be of the same type"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffle2XInvalidNumComponents) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst shuffle2 %f32vec3_012 %f32vec3_012 %u32vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std shuffle2: "
                        "expected operand X to have 2, 4, 8 or 16 components"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffle2XInvalidComponentType) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst shuffle2 %f64vec4_0123 %f64vec4_0123 %u32vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std shuffle2: "
          "expected operand X and Result Type to have equal component types"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffle2ShuffleMaskNotIntVector) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec2 %extinst shuffle2 %f32vec4_0123 %f32vec4_0123 %f32vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std shuffle2: "
                        "expected operand Shuffle Mask to be an int vector"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffle2ShuffleMaskInvalidNumComponents) {
  const std::string body = R"(
%val1 = OpExtInst %f32vec4 %extinst shuffle2 %f32vec4_0123 %f32vec4_0123 %u32vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std shuffle2: "
                        "expected operand Shuffle Mask to have the same number "
                        "of components as Result Type"));
}

TEST_F(ValidateExtInst, OpenCLStdShuffle2ShuffleMaskInvalidBitWidth) {
  const std::string body = R"(
%val1 = OpExtInst %f64vec2 %extinst shuffle2 %f64vec4_0123 %f64vec4_0123 %u32vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std shuffle2: "
                        "expected operand Shuffle Mask components to have the "
                        "same bit width as Result Type components"));
}

TEST_F(ValidateExtInst, OpenCLStdPrintfSuccess) {
  const std::string body = R"(
%format = OpAccessChain %u8_ptr_uniform_constant %u8arr_uniform_constant %u32_0
%val1 = OpExtInst %u32 %extinst printf %format %u32_0 %u32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, OpenCLStdPrintfBoolResultType) {
  const std::string body = R"(
%format = OpAccessChain %u8_ptr_uniform_constant %u8arr_uniform_constant %u32_0
%val1 = OpExtInst %bool %extinst printf %format %u32_0 %u32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std printf: expected Result Type to be a 32-bit int type"));
}

TEST_F(ValidateExtInst, OpenCLStdPrintfU64ResultType) {
  const std::string body = R"(
%format = OpAccessChain %u8_ptr_uniform_constant %u8arr_uniform_constant %u32_0
%val1 = OpExtInst %u64 %extinst printf %format %u32_0 %u32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std printf: expected Result Type to be a 32-bit int type"));
}

TEST_F(ValidateExtInst, OpenCLStdPrintfFormatNotPointer) {
  const std::string body = R"(
%val1 = OpExtInst %u32 %extinst printf %u8_ptr_uniform_constant %u32_0 %u32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Operand 137[%_ptr_UniformConstant_uchar] cannot be a "
                        "type"));
}

TEST_F(ValidateExtInst, OpenCLStdPrintfFormatNotUniformConstStorageClass) {
  const std::string body = R"(
%format_const = OpAccessChain %u8_ptr_uniform_constant %u8arr_uniform_constant %u32_0
%format = OpBitcast %u8_ptr_generic %format_const
%val1 = OpExtInst %u32 %extinst printf %format %u32_0 %u32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std printf: expected Format storage class to "
                        "be UniformConstant"));
}

TEST_F(ValidateExtInst, OpenCLStdPrintfFormatNotU8Pointer) {
  const std::string body = R"(
%format = OpAccessChain %u32_ptr_uniform_constant %u32vec8_uniform_constant %u32_0
%val1 = OpExtInst %u32 %extinst printf %format %u32_0 %u32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std printf: expected Format data type to be 8-bit int"));
}

TEST_F(ValidateExtInst, OpenCLStdPrefetchU32Success) {
  const std::string body = R"(
%ptr = OpAccessChain %u32_ptr_cross_workgroup %u32arr_cross_workgroup %u32_0
%val1 = OpExtInst %void %extinst prefetch %ptr %u32_256
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, OpenCLStdPrefetchU32Physical64Success) {
  const std::string body = R"(
%ptr = OpAccessChain %u32_ptr_cross_workgroup %u32arr_cross_workgroup %u32_0
%val1 = OpExtInst %void %extinst prefetch %ptr %u64_256
)";

  CompileSuccessfully(GenerateKernelCode(body, "", "Physical64"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, OpenCLStdPrefetchF32Success) {
  const std::string body = R"(
%ptr = OpAccessChain %f32_ptr_cross_workgroup %f32arr_cross_workgroup %u32_0
%val1 = OpExtInst %void %extinst prefetch %ptr %u32_256
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, OpenCLStdPrefetchF32Vec2Success) {
  const std::string body = R"(
%ptr = OpAccessChain %f32vec2_ptr_cross_workgroup %f32vec2arr_cross_workgroup %u32_0
%val1 = OpExtInst %void %extinst prefetch %ptr %u32_256
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, OpenCLStdPrefetchResultTypeNotVoid) {
  const std::string body = R"(
%ptr = OpAccessChain %u32_ptr_cross_workgroup %u32arr_cross_workgroup %u32_0
%val1 = OpExtInst %u32 %extinst prefetch %ptr %u32_256
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std prefetch: expected Result Type to be void"));
}

TEST_F(ValidateExtInst, OpenCLStdPrefetchPtrNotPointer) {
  const std::string body = R"(
%val1 = OpExtInst %void %extinst prefetch %u32_ptr_cross_workgroup %u32_256
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Operand 99[%_ptr_CrossWorkgroup_uint] cannot be a "
                        "type"));
}

TEST_F(ValidateExtInst, OpenCLStdPrefetchPtrNotCrossWorkgroup) {
  const std::string body = R"(
%ptr = OpAccessChain %u8_ptr_uniform_constant %u8arr_uniform_constant %u32_0
%val1 = OpExtInst %void %extinst prefetch %ptr %u32_256
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std prefetch: expected operand Ptr storage "
                        "class to be CrossWorkgroup"));
}

TEST_F(ValidateExtInst, OpenCLStdPrefetchInvalidDataType) {
  const std::string body = R"(
%ptr = OpAccessChain %struct_ptr_cross_workgroup %struct_arr_cross_workgroup %u32_0
%val1 = OpExtInst %void %extinst prefetch %ptr %u32_256
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std prefetch: expected Ptr data type to be int "
                        "or float scalar or vector"));
}

TEST_F(ValidateExtInst, OpenCLStdPrefetchAddressingModelLogical) {
  const std::string body = R"(
%ptr = OpAccessChain %u32_ptr_cross_workgroup %u32arr_cross_workgroup %u32_0
%val1 = OpExtInst %void %extinst prefetch %ptr %u32_256
)";

  CompileSuccessfully(GenerateKernelCode(body, "", "Logical"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std prefetch can only be used with physical "
                        "addressing models"));
}

TEST_F(ValidateExtInst, OpenCLStdPrefetchNumElementsNotSizeT) {
  const std::string body = R"(
%ptr = OpAccessChain %f32_ptr_cross_workgroup %f32arr_cross_workgroup %u32_0
%val1 = OpExtInst %void %extinst prefetch %ptr %u32_256
)";

  CompileSuccessfully(GenerateKernelCode(body, "", "Physical64"));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std prefetch: expected operand Num Elements to "
                        "be of type size_t (64-bit integer for the addressing "
                        "model used in the module)"));
}

TEST_P(ValidateOpenCLStdFractLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%var_f32 = OpVariable %f32_ptr_function Function\n";
  ss << "%var_f32vec2 = OpVariable %f32vec2_ptr_function Function\n";
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_0 %var_f32\n";
  ss << "%val2 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %f32vec2_01 %var_f32vec2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdFractLike, IntResultType) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%var_f32 = OpVariable %f32_ptr_function Function\n";
  ss << "%val1 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %f32_0 %var_f32\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected Result Type to be a float scalar or vector type"));
}

TEST_P(ValidateOpenCLStdFractLike, XWrongType) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%var_f32 = OpVariable %f32_ptr_function Function\n";
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f64_0 %var_f32\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected type of operand X to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdFractLike, NotPointer) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%var_f32 = OpVariable %f32_ptr_function Function\n";
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_0 %f32_1\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected the last operand to be a pointer"));
}

TEST_P(ValidateOpenCLStdFractLike, PointerInvalidStorageClass) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f32_ptr_uniform_constant "
        "%f32vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name << " %f32_0 %ptr\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected storage class of the pointer to be "
                        "Generic, CrossWorkgroup, Workgroup or Function"));
}

TEST_P(ValidateOpenCLStdFractLike, PointerWrongDataType) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%var_u32 = OpVariable %u32_ptr_function Function\n";
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_0 %var_u32\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std " + ext_inst_name +
          ": expected data type of the pointer to be equal to Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllFractLike, ValidateOpenCLStdFractLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "fract",
                             "modf",
                             "sincos",
                         }));

TEST_F(ValidateExtInst, OpenCLStdRemquoSuccess) {
  const std::string body = R"(
%var_u32 = OpVariable %u32_ptr_function Function
%var_u32vec2 = OpVariable %u32vec2_ptr_function Function
%val1 = OpExtInst %f32 %extinst remquo %f32_3 %f32_2 %var_u32
%val2 = OpExtInst %f32vec2 %extinst remquo %f32vec2_01 %f32vec2_12 %var_u32vec2
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, OpenCLStdRemquoIntResultType) {
  const std::string body = R"(
%var_u32 = OpVariable %u32_ptr_function Function
%val1 = OpExtInst %u32 %extinst remquo %f32_3 %f32_2 %var_u32
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std remquo: "
                "expected Result Type to be a float scalar or vector type"));
}

TEST_F(ValidateExtInst, OpenCLStdRemquoXWrongType) {
  const std::string body = R"(
%var_u32 = OpVariable %f32_ptr_function Function
%val1 = OpExtInst %f32 %extinst remquo %u32_3 %f32_2 %var_u32
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std remquo: "
                "expected type of operand X to be equal to Result Type"));
}

TEST_F(ValidateExtInst, OpenCLStdRemquoYWrongType) {
  const std::string body = R"(
%var_u32 = OpVariable %f32_ptr_function Function
%val1 = OpExtInst %f32 %extinst remquo %f32_3 %u32_2 %var_u32
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std remquo: "
                "expected type of operand Y to be equal to Result Type"));
}

TEST_F(ValidateExtInst, OpenCLStdRemquoNotPointer) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst remquo %f32_3 %f32_2 %f32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std remquo: "
                        "expected the last operand to be a pointer"));
}

TEST_F(ValidateExtInst, OpenCLStdRemquoPointerWrongStorageClass) {
  const std::string body = R"(
%ptr = OpAccessChain %f32_ptr_uniform_constant %f32vec8_uniform_constant %u32_1
%val1 = OpExtInst %f32 %extinst remquo %f32_3 %f32_2 %ptr
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std remquo: "
                        "expected storage class of the pointer to be Generic, "
                        "CrossWorkgroup, Workgroup or Function"));
}

TEST_F(ValidateExtInst, OpenCLStdRemquoPointerWrongDataType) {
  const std::string body = R"(
%var_f32 = OpVariable %f32_ptr_function Function
%val1 = OpExtInst %f32 %extinst remquo %f32_3 %f32_2 %var_f32
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std remquo: "
                        "expected data type of the pointer to be a 32-bit int "
                        "scalar or vector type"));
}

TEST_F(ValidateExtInst, OpenCLStdRemquoPointerWrongDataTypeWidth) {
  const std::string body = R"(
%var_u64 = OpVariable %u64_ptr_function Function
%val1 = OpExtInst %f32 %extinst remquo %f32_3 %f32_2 %var_u64
)";
  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std remquo: "
                        "expected data type of the pointer to be a 32-bit int "
                        "scalar or vector type"));
}

TEST_F(ValidateExtInst, OpenCLStdRemquoPointerWrongNumberOfComponents) {
  const std::string body = R"(
%var_u32vec2 = OpVariable %u32vec2_ptr_function Function
%val1 = OpExtInst %f32 %extinst remquo %f32_3 %f32_2 %var_u32vec2
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std remquo: "
                "expected data type of the pointer to have the same number "
                "of components as Result Type"));
}

TEST_P(ValidateOpenCLStdFrexpLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%var_u32 = OpVariable %u32_ptr_function Function\n";
  ss << "%var_u32vec2 = OpVariable %u32vec2_ptr_function Function\n";
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_0 %var_u32\n";
  ss << "%val2 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %f32vec2_01 %var_u32vec2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdFrexpLike, IntResultType) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%var_u32 = OpVariable %u32_ptr_function Function\n";
  ss << "%val1 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %f32_0 %var_u32\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected Result Type to be a float scalar or vector type"));
}

TEST_P(ValidateOpenCLStdFrexpLike, XWrongType) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%var_u32 = OpVariable %u32_ptr_function Function\n";
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f64_0 %var_u32\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected type of operand X to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdFrexpLike, NotPointer) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_0 %u32_1\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected the last operand to be a pointer"));
}

TEST_P(ValidateOpenCLStdFrexpLike, PointerInvalidStorageClass) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%ptr = OpAccessChain %f32_ptr_uniform_constant "
        "%f32vec8_uniform_constant %u32_1\n";
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name << " %f32_0 %ptr\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected storage class of the pointer to be "
                        "Generic, CrossWorkgroup, Workgroup or Function"));
}

TEST_P(ValidateOpenCLStdFrexpLike, PointerDataTypeFloat) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%var_f32 = OpVariable %f32_ptr_function Function\n";
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_0 %var_f32\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected data type of the pointer to be a 32-bit "
                        "int scalar or vector type"));
}

TEST_P(ValidateOpenCLStdFrexpLike, PointerDataTypeU64) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%var_u64 = OpVariable %u64_ptr_function Function\n";
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_0 %var_u64\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected data type of the pointer to be a 32-bit "
                        "int scalar or vector type"));
}

TEST_P(ValidateOpenCLStdFrexpLike, PointerDataTypeDiffSize) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%var_u32 = OpVariable %u32_ptr_function Function\n";
  ss << "%val1 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %f32vec2_01 %var_u32\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected data type of the pointer to have the same "
                        "number of components as Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllFrexpLike, ValidateOpenCLStdFrexpLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "frexp",
                             "lgamma_r",
                         }));

TEST_F(ValidateExtInst, OpenCLStdIlogbSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %u32 %extinst ilogb %f32_3
%val2 = OpExtInst %u32vec2 %extinst ilogb %f32vec2_12
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, OpenCLStdIlogbFloatResultType) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst ilogb %f32_3
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std ilogb: "
          "expected Result Type to be a 32-bit int scalar or vector type"));
}

TEST_F(ValidateExtInst, OpenCLStdIlogbIntX) {
  const std::string body = R"(
%val1 = OpExtInst %u32 %extinst ilogb %u32_3
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std ilogb: "
                        "expected operand X to be a float scalar or vector"));
}

TEST_F(ValidateExtInst, OpenCLStdIlogbDiffSize) {
  const std::string body = R"(
%val2 = OpExtInst %u32vec2 %extinst ilogb %f32_1
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std ilogb: "
                        "expected operand X to have the same number of "
                        "components as Result Type"));
}

TEST_F(ValidateExtInst, OpenCLStdNanSuccess) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst nan %u32_3
%val2 = OpExtInst %f32vec2 %extinst nan %u32vec2_12
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtInst, OpenCLStdNanIntResultType) {
  const std::string body = R"(
%val1 = OpExtInst %u32 %extinst nan %u32_3
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std nan: "
                "expected Result Type to be a float scalar or vector type"));
}

TEST_F(ValidateExtInst, OpenCLStdNanFloatNancode) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst nan %f32_3
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std nan: "
                        "expected Nancode to be an int scalar or vector type"));
}

TEST_F(ValidateExtInst, OpenCLStdNanFloatDiffSize) {
  const std::string body = R"(
%val1 = OpExtInst %f32 %extinst nan %u32vec2_12
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std nan: "
                        "expected Nancode to have the same number of "
                        "components as Result Type"));
}

TEST_F(ValidateExtInst, OpenCLStdNanFloatDiffBitWidth) {
  const std::string body = R"(
%val1 = OpExtInst %f64 %extinst nan %u32_2
)";

  CompileSuccessfully(GenerateKernelCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std nan: "
                "expected Nancode to have the same bit width as Result Type"));
}

TEST_P(ValidateOpenCLStdLdexpLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_0 %u32_1\n";
  ss << "%val2 = OpExtInst %f32vec2 %extinst " << ext_inst_name
     << " %f32vec2_12 %u32vec2_12\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdLdexpLike, IntResultType) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %f32_0 %u32_1\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected Result Type to be a float scalar or vector type"));
}

TEST_P(ValidateOpenCLStdLdexpLike, XWrongType) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %u32_0 %u32_1\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected type of operand X to be equal to Result Type"));
}

TEST_P(ValidateOpenCLStdLdexpLike, ExponentNotInt) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_0 %f32_1\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected the exponent to be a 32-bit int scalar or vector"));
}

TEST_P(ValidateOpenCLStdLdexpLike, ExponentNotInt32) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_0 %u64_1\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected the exponent to be a 32-bit int scalar or vector"));
}

TEST_P(ValidateOpenCLStdLdexpLike, ExponentWrongSize) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f32 %extinst " << ext_inst_name
     << " %f32_0 %u32vec2_01\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected the exponent to have the same number of "
                        "components as Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllLdexpLike, ValidateOpenCLStdLdexpLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "ldexp",
                             "pown",
                             "rootn",
                         }));

TEST_P(ValidateOpenCLStdUpsampleLike, Success) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %u16 %extinst " << ext_inst_name << " %u8_1 %u8_2\n";
  ss << "%val2 = OpExtInst %u32 %extinst " << ext_inst_name
     << " %u16_1 %u16_2\n";
  ss << "%val3 = OpExtInst %u64 %extinst " << ext_inst_name
     << " %u32_1 %u32_2\n";
  ss << "%val4 = OpExtInst %u64vec2 %extinst " << ext_inst_name
     << " %u32vec2_01 %u32vec2_01\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ValidateOpenCLStdUpsampleLike, FloatResultType) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %f64 %extinst " << ext_inst_name
     << " %u32_1 %u32_2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected Result Type to be an int scalar or vector type"));
}

TEST_P(ValidateOpenCLStdUpsampleLike, InvalidResultTypeBitWidth) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %u8 %extinst " << ext_inst_name << " %u8_1 %u8_2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpenCL.std " + ext_inst_name +
          ": expected bit width of Result Type components to be 16, 32 or 64"));
}

TEST_P(ValidateOpenCLStdUpsampleLike, LoHiDiffType) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %u64 %extinst " << ext_inst_name
     << " %u32_1 %u16_2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected Hi and Lo operands to have the same type"));
}

TEST_P(ValidateOpenCLStdUpsampleLike, DiffNumberOfComponents) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %u64vec2 %extinst " << ext_inst_name
     << " %u32_1 %u32_2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpenCL.std " + ext_inst_name +
                        ": expected Hi and Lo operands to have the same number "
                        "of components as Result Type"));
}

TEST_P(ValidateOpenCLStdUpsampleLike, HiLoWrongBitWidth) {
  const std::string ext_inst_name = GetParam();
  std::ostringstream ss;
  ss << "%val1 = OpExtInst %u64 %extinst " << ext_inst_name
     << " %u16_1 %u16_2\n";

  CompileSuccessfully(GenerateKernelCode(ss.str()));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpenCL.std " + ext_inst_name +
                ": expected bit width of components of Hi and Lo operands to "
                "be half of the bit width of components of Result Type"));
}

INSTANTIATE_TEST_SUITE_P(AllUpsampleLike, ValidateOpenCLStdUpsampleLike,
                         ::testing::ValuesIn(std::vector<std::string>{
                             "u_upsample",
                             "s_upsample",
                         }));

TEST_F(ValidateClspvReflection, RequiresNonSemanticExtension) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
%1 = OpExtInstImport "NonSemantic.ClspvReflection.1"
OpMemoryModel Logical GLSL450
)";

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("NonSemantic extended instruction sets cannot be "
                        "declared without SPV_KHR_non_semantic_info"));
}

TEST_F(ValidateClspvReflection, MissingVersion) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpExtension "SPV_KHR_non_semantic_info"
%1 = OpExtInstImport "NonSemantic.ClspvReflection."
OpMemoryModel Logical GLSL450
%2 = OpTypeVoid
%3 = OpTypeInt 32 0
%4 = OpConstant %3 1
%5 = OpExtInst %2 %1 SpecConstantWorkDim %4
)";

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Missing NonSemantic.ClspvReflection import version"));
}

TEST_F(ValidateClspvReflection, BadVersion0) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpExtension "SPV_KHR_non_semantic_info"
%1 = OpExtInstImport "NonSemantic.ClspvReflection.0"
OpMemoryModel Logical GLSL450
%2 = OpTypeVoid
%3 = OpTypeInt 32 0
%4 = OpConstant %3 1
%5 = OpExtInst %2 %1 SpecConstantWorkDim %4
)";

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Unknown NonSemantic.ClspvReflection import version"));
}

TEST_F(ValidateClspvReflection, BadVersionNotANumber) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpExtension "SPV_KHR_non_semantic_info"
%1 = OpExtInstImport "NonSemantic.ClspvReflection.1a"
OpMemoryModel Logical GLSL450
%2 = OpTypeVoid
%3 = OpTypeInt 32 0
%4 = OpConstant %3 1
%5 = OpExtInst %2 %1 SpecConstantWorkDim %4
)";

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("NonSemantic.ClspvReflection import does not encode "
                        "the version correctly"));
}

TEST_F(ValidateClspvReflection, Kernel) {
  const std::string text = R"(
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.ClspvReflection.1"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %foo "foo"
OpExecutionMode %foo LocalSize 1 1 1
%foo_name = OpString "foo"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%decl = OpExtInst %void %ext Kernel %foo %foo_name
)";

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateClspvReflection, KernelNotAFunction) {
  const std::string text = R"(
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.ClspvReflection.1"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %foo "foo"
OpExecutionMode %foo LocalSize 1 1 1
%foo_name = OpString "foo"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%decl = OpExtInst %void %ext Kernel %foo_name %foo_name
)";

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Kernel does not reference a function"));
}

TEST_F(ValidateClspvReflection, KernelNotAnEntryPoint) {
  const std::string text = R"(
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.ClspvReflection.1"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %foo "foo"
OpExecutionMode %foo LocalSize 1 1 1
%foo_name = OpString "foo"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%bar = OpFunction %void None %void_fn
%bar_entry = OpLabel
OpReturn
OpFunctionEnd
%decl = OpExtInst %void %ext Kernel %bar %foo_name
)";

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Kernel does not reference an entry-point"));
}

TEST_F(ValidateClspvReflection, KernelNotGLCompute) {
  const std::string text = R"(
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.ClspvReflection.1"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %foo "foo"
OpExecutionMode %foo OriginUpperLeft
%foo_name = OpString "foo"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%decl = OpExtInst %void %ext Kernel %foo %foo_name
)";

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Kernel must refer only to GLCompute entry-points"));
}

TEST_F(ValidateClspvReflection, KernelNameMismatch) {
  const std::string text = R"(
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.ClspvReflection.1"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %foo "foo"
OpExecutionMode %foo LocalSize 1 1 1
%foo_name = OpString "bar"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%decl = OpExtInst %void %ext Kernel %foo %foo_name
)";

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Name must match an entry-point for Kernel"));
}

using ArgumentBasics =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;

INSTANTIATE_TEST_SUITE_P(
    ValidateClspvReflectionArgumentKernel, ArgumentBasics,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair("ArgumentStorageBuffer", "%int_0 %int_0"),
        std::make_pair("ArgumentUniform", "%int_0 %int_0"),
        std::make_pair("ArgumentPodStorageBuffer",
                       "%int_0 %int_0 %int_0 %int_4"),
        std::make_pair("ArgumentPodUniform", "%int_0 %int_0 %int_0 %int_4"),
        std::make_pair("ArgumentPodPushConstant", "%int_0 %int_4"),
        std::make_pair("ArgumentSampledImage", "%int_0 %int_0"),
        std::make_pair("ArgumentStorageImage", "%int_0 %int_0"),
        std::make_pair("ArgumentSampler", "%int_0 %int_0"),
        std::make_pair("ArgumentWorkgroup", "%int_0 %int_0")}));

TEST_P(ArgumentBasics, KernelNotAnExtendedInstruction) {
  const std::string ext_inst = std::get<0>(GetParam());
  const std::string extra = std::get<1>(GetParam());
  const std::string text = R"(
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.ClspvReflection.1"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %foo "foo"
OpExecutionMode %foo LocalSize 1 1 1
%foo_name = OpString "foo"
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%in = OpExtInst %void %ext )" +
                           ext_inst + " %int_0 %int_0 " + extra;

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Kernel must be a Kernel extended instruction"));
}

TEST_P(ArgumentBasics, KernelFromDifferentImport) {
  const std::string ext_inst = std::get<0>(GetParam());
  const std::string extra = std::get<1>(GetParam());
  const std::string text = R"(
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.ClspvReflection.1"
%ext2 = OpExtInstImport "NonSemantic.ClspvReflection.1"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %foo "foo"
OpExecutionMode %foo LocalSize 1 1 1
%foo_name = OpString "foo"
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%decl = OpExtInst %void %ext2 Kernel %foo %foo_name
%in = OpExtInst %void %ext )" +
                           ext_inst + " %decl %int_0 " + extra;

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Kernel must be from the same extended instruction import"));
}

TEST_P(ArgumentBasics, KernelWrongExtendedInstruction) {
  const std::string ext_inst = std::get<0>(GetParam());
  const std::string extra = std::get<1>(GetParam());
  const std::string text = R"(
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.ClspvReflection.1"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %foo "foo"
OpExecutionMode %foo LocalSize 1 1 1
%foo_name = OpString "foo"
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%decl = OpExtInst %void %ext ArgumentInfo %foo_name
%in = OpExtInst %void %ext )" +
                           ext_inst + " %decl %int_0 " + extra;

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Kernel must be a Kernel extended instruction"));
}

TEST_P(ArgumentBasics, ArgumentInfo) {
  const std::string ext_inst = std::get<0>(GetParam());
  const std::string operands = std::get<1>(GetParam());
  const std::string text = R"(
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.ClspvReflection.1"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %foo "foo"
OpExecutionMode %foo LocalSize 1 1 1
%foo_name = OpString "foo"
%in_name = OpString "in"
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%decl = OpExtInst %void %ext Kernel %foo %foo_name
%info = OpExtInst %void %ext ArgumentInfo %in_name
%in = OpExtInst %void %ext )" +
                           ext_inst + " %decl %int_0 " + operands + " %info";

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ArgumentBasics, ArgumentInfoNotAnExtendedInstruction) {
  const std::string ext_inst = std::get<0>(GetParam());
  const std::string operands = std::get<1>(GetParam());
  const std::string text = R"(
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.ClspvReflection.1"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %foo "foo"
OpExecutionMode %foo LocalSize 1 1 1
%foo_name = OpString "foo"
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%decl = OpExtInst %void %ext Kernel %foo %foo_name
%in = OpExtInst %void %ext )" +
                           ext_inst + " %decl %int_0 " + operands + " %int_0";

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("ArgInfo must be an ArgumentInfo extended instruction"));
}

TEST_P(ArgumentBasics, ArgumentInfoFromDifferentImport) {
  const std::string ext_inst = std::get<0>(GetParam());
  const std::string operands = std::get<1>(GetParam());
  const std::string text = R"(
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.ClspvReflection.1"
%ext2 = OpExtInstImport "NonSemantic.ClspvReflection.1"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %foo "foo"
OpExecutionMode %foo LocalSize 1 1 1
%foo_name = OpString "foo"
%in_name = OpString "in"
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%decl = OpExtInst %void %ext Kernel %foo %foo_name
%info = OpExtInst %void %ext2 ArgumentInfo %in_name
%in = OpExtInst %void %ext )" +
                           ext_inst + " %decl %int_0 " + operands + " %info";

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("ArgInfo must be from the same extended instruction import"));
}

using Uint32Constant =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;

INSTANTIATE_TEST_SUITE_P(
    ValidateClspvReflectionUint32Constants, Uint32Constant,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair("ArgumentStorageBuffer %decl %float_0 %int_0 %int_0",
                       "Ordinal"),
        std::make_pair("ArgumentStorageBuffer %decl %null %int_0 %int_0",
                       "Ordinal"),
        std::make_pair("ArgumentStorageBuffer %decl %int_0 %float_0 %int_0",
                       "DescriptorSet"),
        std::make_pair("ArgumentStorageBuffer %decl %int_0 %null %int_0",
                       "DescriptorSet"),
        std::make_pair("ArgumentStorageBuffer %decl %int_0 %int_0 %float_0",
                       "Binding"),
        std::make_pair("ArgumentStorageBuffer %decl %int_0 %int_0 %null",
                       "Binding"),
        std::make_pair("ArgumentUniform %decl %float_0 %int_0 %int_0",
                       "Ordinal"),
        std::make_pair("ArgumentUniform %decl %null %int_0 %int_0", "Ordinal"),
        std::make_pair("ArgumentUniform %decl %int_0 %float_0 %int_0",
                       "DescriptorSet"),
        std::make_pair("ArgumentUniform %decl %int_0 %null %int_0",
                       "DescriptorSet"),
        std::make_pair("ArgumentUniform %decl %int_0 %int_0 %float_0",
                       "Binding"),
        std::make_pair("ArgumentUniform %decl %int_0 %int_0 %null", "Binding"),
        std::make_pair("ArgumentSampledImage %decl %float_0 %int_0 %int_0",
                       "Ordinal"),
        std::make_pair("ArgumentSampledImage %decl %null %int_0 %int_0",
                       "Ordinal"),
        std::make_pair("ArgumentSampledImage %decl %int_0 %float_0 %int_0",
                       "DescriptorSet"),
        std::make_pair("ArgumentSampledImage %decl %int_0 %null %int_0",
                       "DescriptorSet"),
        std::make_pair("ArgumentSampledImage %decl %int_0 %int_0 %float_0",
                       "Binding"),
        std::make_pair("ArgumentSampledImage %decl %int_0 %int_0 %null",
                       "Binding"),
        std::make_pair("ArgumentStorageImage %decl %float_0 %int_0 %int_0",
                       "Ordinal"),
        std::make_pair("ArgumentStorageImage %decl %null %int_0 %int_0",
                       "Ordinal"),
        std::make_pair("ArgumentStorageImage %decl %int_0 %float_0 %int_0",
                       "DescriptorSet"),
        std::make_pair("ArgumentStorageImage %decl %int_0 %null %int_0",
                       "DescriptorSet"),
        std::make_pair("ArgumentStorageImage %decl %int_0 %int_0 %float_0",
                       "Binding"),
        std::make_pair("ArgumentStorageImage %decl %int_0 %int_0 %null",
                       "Binding"),
        std::make_pair("ArgumentSampler %decl %float_0 %int_0 %int_0",
                       "Ordinal"),
        std::make_pair("ArgumentSampler %decl %null %int_0 %int_0", "Ordinal"),
        std::make_pair("ArgumentSampler %decl %int_0 %float_0 %int_0",
                       "DescriptorSet"),
        std::make_pair("ArgumentSampler %decl %int_0 %null %int_0",
                       "DescriptorSet"),
        std::make_pair("ArgumentSampler %decl %int_0 %int_0 %float_0",
                       "Binding"),
        std::make_pair("ArgumentSampler %decl %int_0 %int_0 %null", "Binding"),
        std::make_pair("ArgumentPodStorageBuffer %decl %float_0 %int_0 %int_0 "
                       "%int_0 %int_4",
                       "Ordinal"),
        std::make_pair(
            "ArgumentPodStorageBuffer %decl %null %int_0 %int_0 %int_0 %int_4",
            "Ordinal"),
        std::make_pair("ArgumentPodStorageBuffer %decl %int_0 %float_0 %int_0 "
                       "%int_0 %int_4",
                       "DescriptorSet"),
        std::make_pair(
            "ArgumentPodStorageBuffer %decl %int_0 %null %int_0 %int_0 %int_4",
            "DescriptorSet"),
        std::make_pair("ArgumentPodStorageBuffer %decl %int_0 %int_0 %float_0 "
                       "%int_0 %int_4",
                       "Binding"),
        std::make_pair(
            "ArgumentPodStorageBuffer %decl %int_0 %int_0 %null %int_0 %int_4",
            "Binding"),
        std::make_pair("ArgumentPodStorageBuffer %decl %int_0 %int_0 %int_0 "
                       "%float_0 %int_4",
                       "Offset"),
        std::make_pair(
            "ArgumentPodStorageBuffer %decl %int_0 %int_0 %int_0 %null %int_4",
            "Offset"),
        std::make_pair("ArgumentPodStorageBuffer %decl %int_0 %int_0 %int_0 "
                       "%int_0 %float_0",
                       "Size"),
        std::make_pair(
            "ArgumentPodStorageBuffer %decl %int_0 %int_0 %int_0 %int_0 %null",
            "Size"),
        std::make_pair(
            "ArgumentPodUniform %decl %float_0 %int_0 %int_0 %int_0 %int_4",
            "Ordinal"),
        std::make_pair(
            "ArgumentPodUniform %decl %null %int_0 %int_0 %int_0 %int_4",
            "Ordinal"),
        std::make_pair(
            "ArgumentPodUniform %decl %int_0 %float_0 %int_0 %int_0 %int_4",
            "DescriptorSet"),
        std::make_pair(
            "ArgumentPodUniform %decl %int_0 %null %int_0 %int_0 %int_4",
            "DescriptorSet"),
        std::make_pair(
            "ArgumentPodUniform %decl %int_0 %int_0 %float_0 %int_0 %int_4",
            "Binding"),
        std::make_pair(
            "ArgumentPodUniform %decl %int_0 %int_0 %null %int_0 %int_4",
            "Binding"),
        std::make_pair(
            "ArgumentPodUniform %decl %int_0 %int_0 %int_0 %float_0 %int_4",
            "Offset"),
        std::make_pair(
            "ArgumentPodUniform %decl %int_0 %int_0 %int_0 %null %int_4",
            "Offset"),
        std::make_pair(
            "ArgumentPodUniform %decl %int_0 %int_0 %int_0 %int_0 %float_0",
            "Size"),
        std::make_pair(
            "ArgumentPodUniform %decl %int_0 %int_0 %int_0 %int_0 %null",
            "Size"),
        std::make_pair("ArgumentPodPushConstant %decl %float_0 %int_0 %int_4",
                       "Ordinal"),
        std::make_pair("ArgumentPodPushConstant %decl %null %int_0 %int_4",
                       "Ordinal"),
        std::make_pair("ArgumentPodPushConstant %decl %int_0 %float_0 %int_4",
                       "Offset"),
        std::make_pair("ArgumentPodPushConstant %decl %int_0 %null %int_4",
                       "Offset"),
        std::make_pair("ArgumentPodPushConstant %decl %int_0 %int_0 %float_0",
                       "Size"),
        std::make_pair("ArgumentPodPushConstant %decl %int_0 %int_0 %null",
                       "Size"),
        std::make_pair("ArgumentWorkgroup %decl %float_0 %int_0 %int_4",
                       "Ordinal"),
        std::make_pair("ArgumentWorkgroup %decl %null %int_0 %int_4",
                       "Ordinal"),
        std::make_pair("ArgumentWorkgroup %decl %int_0 %float_0 %int_4",
                       "SpecId"),
        std::make_pair("ArgumentWorkgroup %decl %int_0 %null %int_4", "SpecId"),
        std::make_pair("ArgumentWorkgroup %decl %int_0 %int_0 %float_0",
                       "ElemSize"),
        std::make_pair("ArgumentWorkgroup %decl %int_0 %int_0 %null",
                       "ElemSize"),
        std::make_pair("SpecConstantWorkgroupSize %float_0 %int_0 %int_4", "X"),
        std::make_pair("SpecConstantWorkgroupSize %null %int_0 %int_4", "X"),
        std::make_pair("SpecConstantWorkgroupSize %int_0 %float_0 %int_4", "Y"),
        std::make_pair("SpecConstantWorkgroupSize %int_0 %null %int_4", "Y"),
        std::make_pair("SpecConstantWorkgroupSize %int_0 %int_0 %float_0", "Z"),
        std::make_pair("SpecConstantWorkgroupSize %int_0 %int_0 %null", "Z"),
        std::make_pair("SpecConstantGlobalOffset %float_0 %int_0 %int_4", "X"),
        std::make_pair("SpecConstantGlobalOffset %null %int_0 %int_4", "X"),
        std::make_pair("SpecConstantGlobalOffset %int_0 %float_0 %int_4", "Y"),
        std::make_pair("SpecConstantGlobalOffset %int_0 %null %int_4", "Y"),
        std::make_pair("SpecConstantGlobalOffset %int_0 %int_0 %float_0", "Z"),
        std::make_pair("SpecConstantGlobalOffset %int_0 %int_0 %null", "Z"),
        std::make_pair("SpecConstantWorkDim %float_0", "Dim"),
        std::make_pair("SpecConstantWorkDim %null", "Dim"),
        std::make_pair("PushConstantGlobalOffset %float_0 %int_0", "Offset"),
        std::make_pair("PushConstantGlobalOffset %null %int_0", "Offset"),
        std::make_pair("PushConstantGlobalOffset %int_0 %float_0", "Size"),
        std::make_pair("PushConstantGlobalOffset %int_0 %null", "Size"),
        std::make_pair("PushConstantEnqueuedLocalSize %float_0 %int_0",
                       "Offset"),
        std::make_pair("PushConstantEnqueuedLocalSize %null %int_0", "Offset"),
        std::make_pair("PushConstantEnqueuedLocalSize %int_0 %float_0", "Size"),
        std::make_pair("PushConstantEnqueuedLocalSize %int_0 %null", "Size"),
        std::make_pair("PushConstantGlobalSize %float_0 %int_0", "Offset"),
        std::make_pair("PushConstantGlobalSize %null %int_0", "Offset"),
        std::make_pair("PushConstantGlobalSize %int_0 %float_0", "Size"),
        std::make_pair("PushConstantGlobalSize %int_0 %null", "Size"),
        std::make_pair("PushConstantRegionOffset %float_0 %int_0", "Offset"),
        std::make_pair("PushConstantRegionOffset %null %int_0", "Offset"),
        std::make_pair("PushConstantRegionOffset %int_0 %float_0", "Size"),
        std::make_pair("PushConstantRegionOffset %int_0 %null", "Size"),
        std::make_pair("PushConstantNumWorkgroups %float_0 %int_0", "Offset"),
        std::make_pair("PushConstantNumWorkgroups %null %int_0", "Offset"),
        std::make_pair("PushConstantNumWorkgroups %int_0 %float_0", "Size"),
        std::make_pair("PushConstantNumWorkgroups %int_0 %null", "Size"),
        std::make_pair("PushConstantRegionGroupOffset %float_0 %int_0",
                       "Offset"),
        std::make_pair("PushConstantRegionGroupOffset %null %int_0", "Offset"),
        std::make_pair("PushConstantRegionGroupOffset %int_0 %float_0", "Size"),
        std::make_pair("PushConstantRegionGroupOffset %int_0 %null", "Size"),
        std::make_pair("ConstantDataStorageBuffer %float_0 %int_0 %data",
                       "DescriptorSet"),
        std::make_pair("ConstantDataStorageBuffer %null %int_0 %data",
                       "DescriptorSet"),
        std::make_pair("ConstantDataStorageBuffer %int_0 %float_0 %data",
                       "Binding"),
        std::make_pair("ConstantDataStorageBuffer %int_0 %null %data",
                       "Binding"),
        std::make_pair("ConstantDataUniform %float_0 %int_0 %data",
                       "DescriptorSet"),
        std::make_pair("ConstantDataUniform %null %int_0 %data",
                       "DescriptorSet"),
        std::make_pair("ConstantDataUniform %int_0 %float_0 %data", "Binding"),
        std::make_pair("ConstantDataUniform %int_0 %null %data", "Binding"),
        std::make_pair("LiteralSampler %float_0 %int_0 %int_4",
                       "DescriptorSet"),
        std::make_pair("LiteralSampler %null %int_0 %int_4", "DescriptorSet"),
        std::make_pair("LiteralSampler %int_0 %float_0 %int_4", "Binding"),
        std::make_pair("LiteralSampler %int_0 %null %int_4", "Binding"),
        std::make_pair("LiteralSampler %int_0 %int_0 %float_0", "Mask"),
        std::make_pair("LiteralSampler %int_0 %int_0 %null", "Mask"),
        std::make_pair(
            "PropertyRequiredWorkgroupSize %decl %float_0 %int_1 %int_4", "X"),
        std::make_pair(
            "PropertyRequiredWorkgroupSize %decl %null %int_1 %int_4", "X"),
        std::make_pair(
            "PropertyRequiredWorkgroupSize %decl %int_1 %float_0 %int_4", "Y"),
        std::make_pair(
            "PropertyRequiredWorkgroupSize %decl %int_1 %null %int_4", "Y"),
        std::make_pair(
            "PropertyRequiredWorkgroupSize %decl %int_1 %int_1 %float_0", "Z"),
        std::make_pair(
            "PropertyRequiredWorkgroupSize %decl %int_1 %int_1 %null", "Z")}));

TEST_P(Uint32Constant, Invalid) {
  const std::string ext_inst = std::get<0>(GetParam());
  const std::string name = std::get<1>(GetParam());
  const std::string text = R"(
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.ClspvReflection.1"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %foo "foo"
OpExecutionMode %foo LocalSize 1 1 1
%foo_name = OpString "foo"
%data = OpString "1234"
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_4 = OpConstant %int 4
%null = OpConstantNull %int
%float = OpTypeFloat 32
%float_0 = OpConstant %float 0
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%decl = OpExtInst %void %ext Kernel %foo %foo_name
%inst = OpExtInst %void %ext )" +
                           ext_inst;

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(name + " must be a 32-bit unsigned integer OpConstant"));
}

using StringOperand =
    spvtest::ValidateBase<std::pair<std::string, std::string>>;

INSTANTIATE_TEST_SUITE_P(
    ValidateClspvReflectionStringOperands, StringOperand,
    ::testing::ValuesIn(std::vector<std::pair<std::string, std::string>>{
        std::make_pair("ConstantDataStorageBuffer %int_0 %int_0 %int_0",
                       "Data"),
        std::make_pair("ConstantDataUniform %int_0 %int_0 %int_0", "Data")}));

TEST_P(StringOperand, Invalid) {
  const std::string ext_inst = std::get<0>(GetParam());
  const std::string name = std::get<1>(GetParam());
  const std::string text = R"(
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.ClspvReflection.1"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %foo "foo"
OpExecutionMode %foo LocalSize 1 1 1
%foo_name = OpString "foo"
%data = OpString "1234"
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_4 = OpConstant %int 4
%null = OpConstantNull %int
%float = OpTypeFloat 32
%float_0 = OpConstant %float 0
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%decl = OpExtInst %void %ext Kernel %foo %foo_name
%inst = OpExtInst %void %ext )" +
                           ext_inst;

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(name + " must be an OpString"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
