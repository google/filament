// Copyright (c) 2018 Google LLC.
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

#include <cstring>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

struct TestResult {
  TestResult(spv_result_t in_validation_result = SPV_SUCCESS,
             const char* in_error_str = nullptr,
             const char* in_error_str2 = nullptr)
      : validation_result(in_validation_result),
        error_str(in_error_str),
        error_str2(in_error_str2) {}
  spv_result_t validation_result;
  const char* error_str;
  const char* error_str2;
};

using ::testing::Combine;
using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::Values;
using ::testing::ValuesIn;

using ValidateBuiltIns = spvtest::ValidateBase<bool>;
using ValidateVulkanCombineBuiltInExecutionModelDataTypeResult =
    spvtest::ValidateBase<std::tuple<const char*, const char*, const char*,
                                     const char*, TestResult>>;
using ValidateVulkanCombineBuiltInArrayedVariable = spvtest::ValidateBase<
    std::tuple<const char*, const char*, const char*, const char*, TestResult>>;

struct EntryPoint {
  std::string name;
  std::string execution_model;
  std::string execution_modes;
  std::string body;
  std::string interfaces;
};

class CodeGenerator {
 public:
  std::string Build() const;

  std::vector<EntryPoint> entry_points_;
  std::string capabilities_;
  std::string extensions_;
  std::string memory_model_;
  std::string before_types_;
  std::string types_;
  std::string after_types_;
  std::string add_at_the_end_;
};

std::string CodeGenerator::Build() const {
  std::ostringstream ss;

  ss << capabilities_;
  ss << extensions_;
  ss << memory_model_;

  for (const EntryPoint& entry_point : entry_points_) {
    ss << "OpEntryPoint " << entry_point.execution_model << " %"
       << entry_point.name << " \"" << entry_point.name << "\" "
       << entry_point.interfaces << "\n";
  }

  for (const EntryPoint& entry_point : entry_points_) {
    ss << entry_point.execution_modes << "\n";
  }

  ss << before_types_;
  ss << types_;
  ss << after_types_;

  for (const EntryPoint& entry_point : entry_points_) {
    ss << "\n";
    ss << "%" << entry_point.name << " = OpFunction %void None %func\n";
    ss << "%" << entry_point.name << "_entry = OpLabel\n";
    ss << entry_point.body;
    ss << "\nOpReturn\nOpFunctionEnd\n";
  }

  ss << add_at_the_end_;

  return ss.str();
}

std::string GetDefaultShaderCapabilities() {
  return R"(
OpCapability Shader
OpCapability Geometry
OpCapability Tessellation
OpCapability Float64
OpCapability Int64
OpCapability MultiViewport
OpCapability SampleRateShading
)";
}

std::string GetDefaultShaderTypes() {
  return R"(
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f32 = OpTypeFloat 32
%f64 = OpTypeFloat 64
%u32 = OpTypeInt 32 0
%u64 = OpTypeInt 64 0
%f32vec2 = OpTypeVector %f32 2
%f32vec3 = OpTypeVector %f32 3
%f32vec4 = OpTypeVector %f32 4
%f64vec2 = OpTypeVector %f64 2
%f64vec3 = OpTypeVector %f64 3
%f64vec4 = OpTypeVector %f64 4
%u32vec2 = OpTypeVector %u32 2
%u32vec3 = OpTypeVector %u32 3
%u64vec3 = OpTypeVector %u64 3
%u32vec4 = OpTypeVector %u32 4
%u64vec2 = OpTypeVector %u64 2

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

%u32_0 = OpConstant %u32 0
%u32_1 = OpConstant %u32 1
%u32_2 = OpConstant %u32 2
%u32_3 = OpConstant %u32 3
%u32_4 = OpConstant %u32 4

%u64_0 = OpConstant %u64 0
%u64_1 = OpConstant %u64 1
%u64_2 = OpConstant %u64 2
%u64_3 = OpConstant %u64 3

%u32vec2_01 = OpConstantComposite %u32vec2 %u32_0 %u32_1
%u32vec2_12 = OpConstantComposite %u32vec2 %u32_1 %u32_2
%u32vec4_0123 = OpConstantComposite %u32vec4 %u32_0 %u32_1 %u32_2 %u32_3
%u64vec2_01 = OpConstantComposite %u64vec2 %u64_0 %u64_1

%u32arr2 = OpTypeArray %u32 %u32_2
%u32arr3 = OpTypeArray %u32 %u32_3
%u32arr4 = OpTypeArray %u32 %u32_4
%u64arr2 = OpTypeArray %u64 %u32_2
%u64arr3 = OpTypeArray %u64 %u32_3
%u64arr4 = OpTypeArray %u64 %u32_4
%f32arr2 = OpTypeArray %f32 %u32_2
%f32arr3 = OpTypeArray %f32 %u32_3
%f32arr4 = OpTypeArray %f32 %u32_4
%f64arr2 = OpTypeArray %f64 %u32_2
%f64arr3 = OpTypeArray %f64 %u32_3
%f64arr4 = OpTypeArray %f64 %u32_4

%f32vec3arr3 = OpTypeArray %f32vec3 %u32_3
%f32vec4arr3 = OpTypeArray %f32vec4 %u32_3
%f64vec4arr3 = OpTypeArray %f64vec4 %u32_3
)";
}

CodeGenerator GetDefaultShaderCodeGenerator() {
  CodeGenerator generator;
  generator.capabilities_ = GetDefaultShaderCapabilities();
  generator.memory_model_ = "OpMemoryModel Logical GLSL450\n";
  generator.types_ = GetDefaultShaderTypes();
  return generator;
}

TEST_P(ValidateVulkanCombineBuiltInExecutionModelDataTypeResult, InMain) {
  const char* const built_in = std::get<0>(GetParam());
  const char* const execution_model = std::get<1>(GetParam());
  const char* const storage_class = std::get<2>(GetParam());
  const char* const data_type = std::get<3>(GetParam());
  const TestResult& test_result = std::get<4>(GetParam());

  CodeGenerator generator = GetDefaultShaderCodeGenerator();
  generator.before_types_ = "OpMemberDecorate %built_in_type 0 BuiltIn ";
  generator.before_types_ += built_in;
  generator.before_types_ += "\n";

  std::ostringstream after_types;
  after_types << "%built_in_type = OpTypeStruct " << data_type << "\n";
  after_types << "%built_in_ptr = OpTypePointer " << storage_class
              << " %built_in_type\n";
  after_types << "%built_in_var = OpVariable %built_in_ptr " << storage_class
              << "\n";
  after_types << "%data_ptr = OpTypePointer " << storage_class << " "
              << data_type << "\n";
  generator.after_types_ = after_types.str();

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = execution_model;
  if (strncmp(storage_class, "Input", 5) == 0 ||
      strncmp(storage_class, "Output", 6) == 0) {
    entry_point.interfaces = "%built_in_var";
  }

  std::ostringstream execution_modes;
  if (0 == std::strcmp(execution_model, "Fragment")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " OriginUpperLeft\n";
  }
  if (0 == std::strcmp(built_in, "FragDepth")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " DepthReplacing\n";
  }
  entry_point.execution_modes = execution_modes.str();

  entry_point.body = R"(
%ptr = OpAccessChain %data_ptr %built_in_var %u32_0
)";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(test_result.validation_result,
            ValidateInstructions(SPV_ENV_VULKAN_1_0));
  if (test_result.error_str) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str));
  }
  if (test_result.error_str2) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str2));
  }
}

TEST_P(ValidateVulkanCombineBuiltInExecutionModelDataTypeResult, InFunction) {
  const char* const built_in = std::get<0>(GetParam());
  const char* const execution_model = std::get<1>(GetParam());
  const char* const storage_class = std::get<2>(GetParam());
  const char* const data_type = std::get<3>(GetParam());
  const TestResult& test_result = std::get<4>(GetParam());

  CodeGenerator generator = GetDefaultShaderCodeGenerator();
  generator.before_types_ = "OpMemberDecorate %built_in_type 0 BuiltIn ";
  generator.before_types_ += built_in;
  generator.before_types_ += "\n";

  std::ostringstream after_types;
  after_types << "%built_in_type = OpTypeStruct " << data_type << "\n";
  after_types << "%built_in_ptr = OpTypePointer " << storage_class
              << " %built_in_type\n";
  after_types << "%built_in_var = OpVariable %built_in_ptr " << storage_class
              << "\n";
  after_types << "%data_ptr = OpTypePointer " << storage_class << " "
              << data_type << "\n";
  generator.after_types_ = after_types.str();

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = execution_model;
  if (strncmp(storage_class, "Input", 5) == 0 ||
      strncmp(storage_class, "Output", 6) == 0) {
    entry_point.interfaces = "%built_in_var";
  }

  std::ostringstream execution_modes;
  if (0 == std::strcmp(execution_model, "Fragment")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " OriginUpperLeft\n";
  }
  if (0 == std::strcmp(built_in, "FragDepth")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " DepthReplacing\n";
  }
  entry_point.execution_modes = execution_modes.str();

  entry_point.body = R"(
%val2 = OpFunctionCall %void %foo
)";

  generator.add_at_the_end_ = R"(
%foo = OpFunction %void None %func
%foo_entry = OpLabel
%ptr = OpAccessChain %data_ptr %built_in_var %u32_0
OpReturn
OpFunctionEnd
)";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(test_result.validation_result,
            ValidateInstructions(SPV_ENV_VULKAN_1_0));
  if (test_result.error_str) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str));
  }
  if (test_result.error_str2) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str2));
  }
}

TEST_P(ValidateVulkanCombineBuiltInExecutionModelDataTypeResult, Variable) {
  const char* const built_in = std::get<0>(GetParam());
  const char* const execution_model = std::get<1>(GetParam());
  const char* const storage_class = std::get<2>(GetParam());
  const char* const data_type = std::get<3>(GetParam());
  const TestResult& test_result = std::get<4>(GetParam());

  CodeGenerator generator = GetDefaultShaderCodeGenerator();
  generator.before_types_ = "OpDecorate %built_in_var BuiltIn ";
  generator.before_types_ += built_in;
  generator.before_types_ += "\n";

  std::ostringstream after_types;
  after_types << "%built_in_ptr = OpTypePointer " << storage_class << " "
              << data_type << "\n";
  after_types << "%built_in_var = OpVariable %built_in_ptr " << storage_class
              << "\n";
  generator.after_types_ = after_types.str();

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = execution_model;
  if (strncmp(storage_class, "Input", 5) == 0 ||
      strncmp(storage_class, "Output", 6) == 0) {
    entry_point.interfaces = "%built_in_var";
  }
  // Any kind of reference would do.
  entry_point.body = R"(
%val = OpBitcast %u64 %built_in_var
)";

  std::ostringstream execution_modes;
  if (0 == std::strcmp(execution_model, "Fragment")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " OriginUpperLeft\n";
  }
  if (0 == std::strcmp(built_in, "FragDepth")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " DepthReplacing\n";
  }
  entry_point.execution_modes = execution_modes.str();

  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(test_result.validation_result,
            ValidateInstructions(SPV_ENV_VULKAN_1_0));
  if (test_result.error_str) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str));
  }
  if (test_result.error_str2) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str2));
  }
}

INSTANTIATE_TEST_CASE_P(
    ClipAndCullDistanceOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"),
            Values("Vertex", "Geometry", "TessellationControl",
                   "TessellationEvaluation"),
            Values("Output"), Values("%f32arr2", "%f32arr4"),
            Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    ClipAndCullDistanceInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"),
            Values("Fragment", "Geometry", "TessellationControl",
                   "TessellationEvaluation"),
            Values("Input"), Values("%f32arr2", "%f32arr4"),
            Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    ClipAndCullDistanceFragmentOutput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"), Values("Fragment"),
            Values("Output"), Values("%f32arr4"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec doesn't allow BuiltIn ClipDistance/CullDistance "
                "to be used for variables with Output storage class if "
                "execution model is Fragment.",
                "which is called with execution model Fragment."))), );

INSTANTIATE_TEST_CASE_P(
    ClipAndCullDistanceVertexInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"), Values("Vertex"),
            Values("Input"), Values("%f32arr4"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec doesn't allow BuiltIn ClipDistance/CullDistance "
                "to be used for variables with Input storage class if "
                "execution model is Vertex.",
                "which is called with execution model Vertex."))), );

INSTANTIATE_TEST_CASE_P(
    ClipAndCullInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"), Values("GLCompute"),
            Values("Input", "Output"), Values("%f32arr4"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be used only with Fragment, Vertex, TessellationControl, "
                "TessellationEvaluation or Geometry execution models"))), );

INSTANTIATE_TEST_CASE_P(
    ClipAndCullDistanceNotArray,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"), Values("Fragment"),
            Values("Input"), Values("%f32vec2", "%f32vec4", "%f32"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float array",
                              "is not an array"))), );

INSTANTIATE_TEST_CASE_P(
    ClipAndCullDistanceNotFloatArray,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"), Values("Fragment"),
            Values("Input"), Values("%u32arr2", "%u64arr4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float array",
                              "components are not float scalar"))), );

INSTANTIATE_TEST_CASE_P(
    ClipAndCullDistanceNotF32Array,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"), Values("Fragment"),
            Values("Input"), Values("%f64arr2", "%f64arr4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float array",
                              "has components with bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    FragCoordSuccess, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragCoord"), Values("Fragment"), Values("Input"),
            Values("%f32vec4"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    FragCoordNotFragment,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("FragCoord"),
        Values("Vertex", "GLCompute", "Geometry", "TessellationControl",
               "TessellationEvaluation"),
        Values("Input"), Values("%f32vec4"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "to be used only with Fragment execution model"))), );

INSTANTIATE_TEST_CASE_P(
    FragCoordNotInput, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragCoord"), Values("Fragment"), Values("Output"),
            Values("%f32vec4"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))), );

INSTANTIATE_TEST_CASE_P(
    FragCoordNotFloatVector,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragCoord"), Values("Fragment"), Values("Input"),
            Values("%f32arr4", "%u32vec4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float vector",
                              "is not a float vector"))), );

INSTANTIATE_TEST_CASE_P(
    FragCoordNotFloatVec4,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragCoord"), Values("Fragment"), Values("Input"),
            Values("%f32vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float vector",
                              "has 3 components"))), );

INSTANTIATE_TEST_CASE_P(
    FragCoordNotF32Vec4,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragCoord"), Values("Fragment"), Values("Input"),
            Values("%f64vec4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float vector",
                              "has components with bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    FragDepthSuccess, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragDepth"), Values("Fragment"), Values("Output"),
            Values("%f32"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    FragDepthNotFragment,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("FragDepth"),
        Values("Vertex", "GLCompute", "Geometry", "TessellationControl",
               "TessellationEvaluation"),
        Values("Output"), Values("%f32"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "to be used only with Fragment execution model"))), );

INSTANTIATE_TEST_CASE_P(
    FragDepthNotOutput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragDepth"), Values("Fragment"), Values("Input"),
            Values("%f32"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Output storage class",
                "uses storage class Input"))), );

INSTANTIATE_TEST_CASE_P(
    FragDepthNotFloatScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragDepth"), Values("Fragment"), Values("Output"),
            Values("%f32vec4", "%u32"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float scalar",
                              "is not a float scalar"))), );

INSTANTIATE_TEST_CASE_P(
    FragDepthNotF32, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragDepth"), Values("Fragment"), Values("Output"),
            Values("%f64"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float scalar",
                              "has bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    FrontFacingAndHelperInvocationSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FrontFacing", "HelperInvocation"), Values("Fragment"),
            Values("Input"), Values("%bool"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    FrontFacingAndHelperInvocationNotFragment,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("FrontFacing", "HelperInvocation"),
        Values("Vertex", "GLCompute", "Geometry", "TessellationControl",
               "TessellationEvaluation"),
        Values("Input"), Values("%bool"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "to be used only with Fragment execution model"))), );

INSTANTIATE_TEST_CASE_P(
    FrontFacingAndHelperInvocationNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FrontFacing", "HelperInvocation"), Values("Fragment"),
            Values("Output"), Values("%bool"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))), );

INSTANTIATE_TEST_CASE_P(
    FrontFacingAndHelperInvocationNotBool,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FrontFacing", "HelperInvocation"), Values("Fragment"),
            Values("Input"), Values("%f32", "%u32"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a bool scalar",
                              "is not a bool scalar"))), );

INSTANTIATE_TEST_CASE_P(
    ComputeShaderInputInt32Vec3Success,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("GlobalInvocationId", "LocalInvocationId", "NumWorkgroups",
                   "WorkgroupId"),
            Values("GLCompute"), Values("Input"), Values("%u32vec3"),
            Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    ComputeShaderInputInt32Vec3NotGLCompute,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("GlobalInvocationId", "LocalInvocationId", "NumWorkgroups",
                   "WorkgroupId"),
            Values("Vertex", "Fragment", "Geometry", "TessellationControl",
                   "TessellationEvaluation"),
            Values("Input"), Values("%u32vec3"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be used only with GLCompute execution model"))), );

INSTANTIATE_TEST_CASE_P(
    ComputeShaderInputInt32Vec3NotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("GlobalInvocationId", "LocalInvocationId", "NumWorkgroups",
                   "WorkgroupId"),
            Values("GLCompute"), Values("Output"), Values("%u32vec3"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))), );

INSTANTIATE_TEST_CASE_P(
    ComputeShaderInputInt32Vec3NotIntVector,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("GlobalInvocationId", "LocalInvocationId", "NumWorkgroups",
                   "WorkgroupId"),
            Values("GLCompute"), Values("Input"),
            Values("%u32arr3", "%f32vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 3-component 32-bit int vector",
                              "is not an int vector"))), );

INSTANTIATE_TEST_CASE_P(
    ComputeShaderInputInt32Vec3NotIntVec3,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("GlobalInvocationId", "LocalInvocationId", "NumWorkgroups",
                   "WorkgroupId"),
            Values("GLCompute"), Values("Input"), Values("%u32vec4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 3-component 32-bit int vector",
                              "has 4 components"))), );

INSTANTIATE_TEST_CASE_P(
    ComputeShaderInputInt32Vec3NotInt32Vec,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("GlobalInvocationId", "LocalInvocationId", "NumWorkgroups",
                   "WorkgroupId"),
            Values("GLCompute"), Values("Input"), Values("%u64vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 3-component 32-bit int vector",
                              "has components with bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    InvocationIdSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InvocationId"), Values("Geometry", "TessellationControl"),
            Values("Input"), Values("%u32"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    InvocationIdInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InvocationId"),
            Values("Vertex", "Fragment", "GLCompute", "TessellationEvaluation"),
            Values("Input"), Values("%u32"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "to be used only with TessellationControl or "
                              "Geometry execution models"))), );

INSTANTIATE_TEST_CASE_P(
    InvocationIdNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InvocationId"), Values("Geometry", "TessellationControl"),
            Values("Output"), Values("%u32"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))), );

INSTANTIATE_TEST_CASE_P(
    InvocationIdNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InvocationId"), Values("Geometry", "TessellationControl"),
            Values("Input"), Values("%f32", "%u32vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))), );

INSTANTIATE_TEST_CASE_P(
    InvocationIdNotInt32,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InvocationId"), Values("Geometry", "TessellationControl"),
            Values("Input"), Values("%u64"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "has bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    InstanceIndexSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InstanceIndex"), Values("Vertex"), Values("Input"),
            Values("%u32"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    InstanceIndexInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("InstanceIndex"),
        Values("Geometry", "Fragment", "GLCompute", "TessellationControl",
               "TessellationEvaluation"),
        Values("Input"), Values("%u32"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "to be used only with Vertex execution model"))), );

INSTANTIATE_TEST_CASE_P(
    InstanceIndexNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InstanceIndex"), Values("Vertex"), Values("Output"),
            Values("%u32"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))), );

INSTANTIATE_TEST_CASE_P(
    InstanceIndexNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InstanceIndex"), Values("Vertex"), Values("Input"),
            Values("%f32", "%u32vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))), );

INSTANTIATE_TEST_CASE_P(
    InstanceIndexNotInt32,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InstanceIndex"), Values("Vertex"), Values("Input"),
            Values("%u64"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "has bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    LayerAndViewportIndexInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Layer", "ViewportIndex"), Values("Fragment"),
            Values("Input"), Values("%u32"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    LayerAndViewportIndexOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Layer", "ViewportIndex"), Values("Geometry"),
            Values("Output"), Values("%u32"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    LayerAndViewportIndexInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Layer", "ViewportIndex"),
            Values("TessellationControl", "GLCompute"), Values("Input"),
            Values("%u32"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be used only with Vertex, TessellationEvaluation, "
                "Geometry, or Fragment execution models"))), );

INSTANTIATE_TEST_CASE_P(
    LayerAndViewportIndexExecutionModelEnabledByCapability,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Layer", "ViewportIndex"),
            Values("Vertex", "TessellationEvaluation"), Values("Output"),
            Values("%u32"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "requires the ShaderViewportIndexLayerEXT capability"))), );

INSTANTIATE_TEST_CASE_P(
    LayerAndViewportIndexFragmentNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("Layer", "ViewportIndex"), Values("Fragment"), Values("Output"),
        Values("%u32"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "Output storage class if execution model is Fragment",
                          "which is called with execution model Fragment"))), );

INSTANTIATE_TEST_CASE_P(
    LayerAndViewportIndexGeometryNotOutput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("Layer", "ViewportIndex"),
        Values("Vertex", "TessellationEvaluation", "Geometry"), Values("Input"),
        Values("%u32"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "Input storage class if execution model is Vertex, "
                          "TessellationEvaluation, or Geometry",
                          "which is called with execution model"))), );

INSTANTIATE_TEST_CASE_P(
    LayerAndViewportIndexNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Layer", "ViewportIndex"), Values("Fragment"),
            Values("Input"), Values("%f32", "%u32vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))), );

INSTANTIATE_TEST_CASE_P(
    LayerAndViewportIndexNotInt32,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Layer", "ViewportIndex"), Values("Fragment"),
            Values("Input"), Values("%u64"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "has bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    PatchVerticesSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PatchVertices"),
            Values("TessellationEvaluation", "TessellationControl"),
            Values("Input"), Values("%u32"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    PatchVerticesInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PatchVertices"),
            Values("Vertex", "Fragment", "GLCompute", "Geometry"),
            Values("Input"), Values("%u32"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "to be used only with TessellationControl or "
                              "TessellationEvaluation execution models"))), );

INSTANTIATE_TEST_CASE_P(
    PatchVerticesNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PatchVertices"),
            Values("TessellationEvaluation", "TessellationControl"),
            Values("Output"), Values("%u32"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))), );

INSTANTIATE_TEST_CASE_P(
    PatchVerticesNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PatchVertices"),
            Values("TessellationEvaluation", "TessellationControl"),
            Values("Input"), Values("%f32", "%u32vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))), );

INSTANTIATE_TEST_CASE_P(
    PatchVerticesNotInt32,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PatchVertices"),
            Values("TessellationEvaluation", "TessellationControl"),
            Values("Input"), Values("%u64"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "has bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    PointCoordSuccess, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointCoord"), Values("Fragment"), Values("Input"),
            Values("%f32vec2"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    PointCoordNotFragment,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("PointCoord"),
        Values("Vertex", "GLCompute", "Geometry", "TessellationControl",
               "TessellationEvaluation"),
        Values("Input"), Values("%f32vec2"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "to be used only with Fragment execution model"))), );

INSTANTIATE_TEST_CASE_P(
    PointCoordNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointCoord"), Values("Fragment"), Values("Output"),
            Values("%f32vec2"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))), );

INSTANTIATE_TEST_CASE_P(
    PointCoordNotFloatVector,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointCoord"), Values("Fragment"), Values("Input"),
            Values("%f32arr2", "%u32vec2"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float vector",
                              "is not a float vector"))), );

INSTANTIATE_TEST_CASE_P(
    PointCoordNotFloatVec3,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointCoord"), Values("Fragment"), Values("Input"),
            Values("%f32vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float vector",
                              "has 3 components"))), );

INSTANTIATE_TEST_CASE_P(
    PointCoordNotF32Vec4,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointCoord"), Values("Fragment"), Values("Input"),
            Values("%f64vec2"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float vector",
                              "has components with bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    PointSizeOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointSize"),
            Values("Vertex", "Geometry", "TessellationControl",
                   "TessellationEvaluation"),
            Values("Output"), Values("%f32"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    PointSizeInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointSize"),
            Values("Geometry", "TessellationControl", "TessellationEvaluation"),
            Values("Input"), Values("%f32"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    PointSizeVertexInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointSize"), Values("Vertex"), Values("Input"),
            Values("%f32"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec doesn't allow BuiltIn PointSize "
                "to be used for variables with Input storage class if "
                "execution model is Vertex.",
                "which is called with execution model Vertex."))), );

INSTANTIATE_TEST_CASE_P(
    PointSizeInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointSize"), Values("GLCompute", "Fragment"),
            Values("Input", "Output"), Values("%f32"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be used only with Vertex, TessellationControl, "
                "TessellationEvaluation or Geometry execution models"))), );

INSTANTIATE_TEST_CASE_P(
    PointSizeNotFloatScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointSize"), Values("Vertex"), Values("Output"),
            Values("%f32vec4", "%u32"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float scalar",
                              "is not a float scalar"))), );

INSTANTIATE_TEST_CASE_P(
    PointSizeNotF32, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointSize"), Values("Vertex"), Values("Output"),
            Values("%f64"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float scalar",
                              "has bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    PositionOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Position"),
            Values("Vertex", "Geometry", "TessellationControl",
                   "TessellationEvaluation"),
            Values("Output"), Values("%f32vec4"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    PositionInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Position"),
            Values("Geometry", "TessellationControl", "TessellationEvaluation"),
            Values("Input"), Values("%f32vec4"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    PositionVertexInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Position"), Values("Vertex"), Values("Input"),
            Values("%f32vec4"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec doesn't allow BuiltIn Position "
                "to be used for variables with Input storage class if "
                "execution model is Vertex.",
                "which is called with execution model Vertex."))), );

INSTANTIATE_TEST_CASE_P(
    PositionInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Position"), Values("GLCompute", "Fragment"),
            Values("Input", "Output"), Values("%f32vec4"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be used only with Vertex, TessellationControl, "
                "TessellationEvaluation or Geometry execution models"))), );

INSTANTIATE_TEST_CASE_P(
    PositionNotFloatVector,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Position"), Values("Geometry"), Values("Input"),
            Values("%f32arr4", "%u32vec4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float vector",
                              "is not a float vector"))), );

INSTANTIATE_TEST_CASE_P(
    PositionNotFloatVec4,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Position"), Values("Geometry"), Values("Input"),
            Values("%f32vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float vector",
                              "has 3 components"))), );

INSTANTIATE_TEST_CASE_P(
    PositionNotF32Vec4,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Position"), Values("Geometry"), Values("Input"),
            Values("%f64vec4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float vector",
                              "has components with bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    PrimitiveIdInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PrimitiveId"),
            Values("Fragment", "TessellationControl", "TessellationEvaluation",
                   "Geometry"),
            Values("Input"), Values("%u32"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    PrimitiveIdOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PrimitiveId"), Values("Geometry"), Values("Output"),
            Values("%u32"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    PrimitiveIdInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PrimitiveId"), Values("Vertex", "GLCompute"),
            Values("Input"), Values("%u32"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be used only with Fragment, TessellationControl, "
                "TessellationEvaluation or Geometry execution models"))), );

INSTANTIATE_TEST_CASE_P(
    PrimitiveIdFragmentNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("PrimitiveId"), Values("Fragment"), Values("Output"),
        Values("%u32"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "Output storage class if execution model is Fragment",
                          "which is called with execution model Fragment"))), );

INSTANTIATE_TEST_CASE_P(
    PrimitiveIdGeometryNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PrimitiveId"),
            Values("TessellationControl", "TessellationEvaluation"),
            Values("Output"), Values("%u32"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Output storage class if execution model is Tessellation",
                "which is called with execution model Tessellation"))), );

INSTANTIATE_TEST_CASE_P(
    PrimitiveIdNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PrimitiveId"), Values("Fragment"), Values("Input"),
            Values("%f32", "%u32vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))), );

INSTANTIATE_TEST_CASE_P(
    PrimitiveIdNotInt32,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PrimitiveId"), Values("Fragment"), Values("Input"),
            Values("%u64"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "has bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    SampleIdSuccess, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SampleId"), Values("Fragment"), Values("Input"),
            Values("%u32"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    SampleIdInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("SampleId"),
        Values("Vertex", "GLCompute", "Geometry", "TessellationControl",
               "TessellationEvaluation"),
        Values("Input"), Values("%u32"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "to be used only with Fragment execution model"))), );

INSTANTIATE_TEST_CASE_P(
    SampleIdNotInput, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("SampleId"), Values("Fragment"), Values("Output"),
        Values("%u32"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "Vulkan spec allows BuiltIn SampleId to be only used "
                          "for variables with Input storage class"))), );

INSTANTIATE_TEST_CASE_P(
    SampleIdNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SampleId"), Values("Fragment"), Values("Input"),
            Values("%f32", "%u32vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))), );

INSTANTIATE_TEST_CASE_P(
    SampleIdNotInt32, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SampleId"), Values("Fragment"), Values("Input"),
            Values("%u64"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "has bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    SampleMaskSuccess, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SampleMask"), Values("Fragment"), Values("Input", "Output"),
            Values("%u32arr2", "%u32arr4"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    SampleMaskInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("SampleMask"),
        Values("Vertex", "GLCompute", "Geometry", "TessellationControl",
               "TessellationEvaluation"),
        Values("Input"), Values("%u32arr2"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "to be used only with Fragment execution model"))), );

INSTANTIATE_TEST_CASE_P(
    SampleMaskWrongStorageClass,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SampleMask"), Values("Fragment"), Values("UniformConstant"),
            Values("%u32arr2"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec allows BuiltIn SampleMask to be only used for "
                "variables with Input or Output storage class"))), );

INSTANTIATE_TEST_CASE_P(
    SampleMaskNotArray,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SampleMask"), Values("Fragment"), Values("Input"),
            Values("%f32", "%u32vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int array",
                              "is not an array"))), );

INSTANTIATE_TEST_CASE_P(
    SampleMaskNotIntArray,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SampleMask"), Values("Fragment"), Values("Input"),
            Values("%f32arr2"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int array",
                              "components are not int scalar"))), );

INSTANTIATE_TEST_CASE_P(
    SampleMaskNotInt32Array,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SampleMask"), Values("Fragment"), Values("Input"),
            Values("%u64arr2"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int array",
                              "has components with bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    SamplePositionSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SamplePosition"), Values("Fragment"), Values("Input"),
            Values("%f32vec2"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    SamplePositionNotFragment,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("SamplePosition"),
        Values("Vertex", "GLCompute", "Geometry", "TessellationControl",
               "TessellationEvaluation"),
        Values("Input"), Values("%f32vec2"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "to be used only with Fragment execution model"))), );

INSTANTIATE_TEST_CASE_P(
    SamplePositionNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SamplePosition"), Values("Fragment"), Values("Output"),
            Values("%f32vec2"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))), );

INSTANTIATE_TEST_CASE_P(
    SamplePositionNotFloatVector,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SamplePosition"), Values("Fragment"), Values("Input"),
            Values("%f32arr2", "%u32vec4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float vector",
                              "is not a float vector"))), );

INSTANTIATE_TEST_CASE_P(
    SamplePositionNotFloatVec2,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SamplePosition"), Values("Fragment"), Values("Input"),
            Values("%f32vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float vector",
                              "has 3 components"))), );

INSTANTIATE_TEST_CASE_P(
    SamplePositionNotF32Vec2,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SamplePosition"), Values("Fragment"), Values("Input"),
            Values("%f64vec2"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float vector",
                              "has components with bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    TessCoordSuccess, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessCoord"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f32vec3"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    TessCoordNotFragment,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("TessCoord"),
        Values("Vertex", "GLCompute", "Geometry", "TessellationControl",
               "Fragment"),
        Values("Input"), Values("%f32vec3"),
        Values(TestResult(
            SPV_ERROR_INVALID_DATA,
            "to be used only with TessellationEvaluation execution model"))), );

INSTANTIATE_TEST_CASE_P(
    TessCoordNotInput, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessCoord"), Values("Fragment"), Values("Output"),
            Values("%f32vec3"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))), );

INSTANTIATE_TEST_CASE_P(
    TessCoordNotFloatVector,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessCoord"), Values("Fragment"), Values("Input"),
            Values("%f32arr3", "%u32vec4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 3-component 32-bit float vector",
                              "is not a float vector"))), );

INSTANTIATE_TEST_CASE_P(
    TessCoordNotFloatVec3,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessCoord"), Values("Fragment"), Values("Input"),
            Values("%f32vec2"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 3-component 32-bit float vector",
                              "has 2 components"))), );

INSTANTIATE_TEST_CASE_P(
    TessCoordNotF32Vec3,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessCoord"), Values("Fragment"), Values("Input"),
            Values("%f64vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 3-component 32-bit float vector",
                              "has components with bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    TessLevelOuterTeseInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f32arr4"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    TessLevelOuterTescOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"), Values("TessellationControl"),
            Values("Output"), Values("%f32arr4"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    TessLevelOuterInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"),
            Values("Vertex", "GLCompute", "Geometry", "Fragment"),
            Values("Input"), Values("%f32arr4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "to be used only with TessellationControl or "
                              "TessellationEvaluation execution models."))), );

INSTANTIATE_TEST_CASE_P(
    TessLevelOuterOutputTese,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"), Values("TessellationEvaluation"),
            Values("Output"), Values("%f32arr4"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec doesn't allow TessLevelOuter/TessLevelInner to be "
                "used for variables with Output storage class if execution "
                "model is TessellationEvaluation."))), );

INSTANTIATE_TEST_CASE_P(
    TessLevelOuterInputTesc,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"), Values("TessellationControl"),
            Values("Input"), Values("%f32arr4"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec doesn't allow TessLevelOuter/TessLevelInner to be "
                "used for variables with Input storage class if execution "
                "model is TessellationControl."))), );

INSTANTIATE_TEST_CASE_P(
    TessLevelOuterNotArray,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f32vec4", "%f32"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float array",
                              "is not an array"))), );

INSTANTIATE_TEST_CASE_P(
    TessLevelOuterNotFloatArray,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"), Values("TessellationEvaluation"),
            Values("Input"), Values("%u32arr4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float array",
                              "components are not float scalar"))), );

INSTANTIATE_TEST_CASE_P(
    TessLevelOuterNotFloatArr4,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f32arr3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float array",
                              "has 3 components"))), );

INSTANTIATE_TEST_CASE_P(
    TessLevelOuterNotF32Arr4,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f64arr4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float array",
                              "has components with bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    TessLevelInnerTeseInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f32arr2"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    TessLevelInnerTescOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"), Values("TessellationControl"),
            Values("Output"), Values("%f32arr2"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    TessLevelInnerInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"),
            Values("Vertex", "GLCompute", "Geometry", "Fragment"),
            Values("Input"), Values("%f32arr2"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "to be used only with TessellationControl or "
                              "TessellationEvaluation execution models."))), );

INSTANTIATE_TEST_CASE_P(
    TessLevelInnerOutputTese,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"), Values("TessellationEvaluation"),
            Values("Output"), Values("%f32arr2"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec doesn't allow TessLevelOuter/TessLevelInner to be "
                "used for variables with Output storage class if execution "
                "model is TessellationEvaluation."))), );

INSTANTIATE_TEST_CASE_P(
    TessLevelInnerInputTesc,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"), Values("TessellationControl"),
            Values("Input"), Values("%f32arr2"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec doesn't allow TessLevelOuter/TessLevelInner to be "
                "used for variables with Input storage class if execution "
                "model is TessellationControl."))), );

INSTANTIATE_TEST_CASE_P(
    TessLevelInnerNotArray,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f32vec2", "%f32"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float array",
                              "is not an array"))), );

INSTANTIATE_TEST_CASE_P(
    TessLevelInnerNotFloatArray,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"), Values("TessellationEvaluation"),
            Values("Input"), Values("%u32arr2"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float array",
                              "components are not float scalar"))), );

INSTANTIATE_TEST_CASE_P(
    TessLevelInnerNotFloatArr2,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f32arr3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float array",
                              "has 3 components"))), );

INSTANTIATE_TEST_CASE_P(
    TessLevelInnerNotF32Arr2,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f64arr2"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float array",
                              "has components with bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    VertexIndexSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("VertexIndex"), Values("Vertex"), Values("Input"),
            Values("%u32"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    VertexIndexInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("VertexIndex"),
        Values("Fragment", "GLCompute", "Geometry", "TessellationControl",
               "TessellationEvaluation"),
        Values("Input"), Values("%u32"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "to be used only with Vertex execution model"))), );

INSTANTIATE_TEST_CASE_P(
    VertexIndexNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("VertexIndex"), Values("Vertex"), Values("Output"),
        Values("%u32"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "Vulkan spec allows BuiltIn VertexIndex to be only "
                          "used for variables with Input storage class"))), );

INSTANTIATE_TEST_CASE_P(
    VertexIndexNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("VertexIndex"), Values("Vertex"), Values("Input"),
            Values("%f32", "%u32vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))), );

INSTANTIATE_TEST_CASE_P(
    VertexIndexNotInt32,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("VertexIndex"), Values("Vertex"), Values("Input"),
            Values("%u64"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "has bit width 64"))), );

TEST_P(ValidateVulkanCombineBuiltInArrayedVariable, Variable) {
  const char* const built_in = std::get<0>(GetParam());
  const char* const execution_model = std::get<1>(GetParam());
  const char* const storage_class = std::get<2>(GetParam());
  const char* const data_type = std::get<3>(GetParam());
  const TestResult& test_result = std::get<4>(GetParam());

  CodeGenerator generator = GetDefaultShaderCodeGenerator();
  generator.before_types_ = "OpDecorate %built_in_var BuiltIn ";
  generator.before_types_ += built_in;
  generator.before_types_ += "\n";

  std::ostringstream after_types;
  after_types << "%built_in_array = OpTypeArray " << data_type << " %u32_3\n";
  after_types << "%built_in_ptr = OpTypePointer " << storage_class
              << " %built_in_array\n";
  after_types << "%built_in_var = OpVariable %built_in_ptr " << storage_class
              << "\n";
  generator.after_types_ = after_types.str();

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = execution_model;
  entry_point.interfaces = "%built_in_var";
  // Any kind of reference would do.
  entry_point.body = R"(
%val = OpBitcast %u64 %built_in_var
)";

  std::ostringstream execution_modes;
  if (0 == std::strcmp(execution_model, "Fragment")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " OriginUpperLeft\n";
  }
  if (0 == std::strcmp(built_in, "FragDepth")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " DepthReplacing\n";
  }
  entry_point.execution_modes = execution_modes.str();

  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(test_result.validation_result,
            ValidateInstructions(SPV_ENV_VULKAN_1_0));
  if (test_result.error_str) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str));
  }
  if (test_result.error_str2) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str2));
  }
}

INSTANTIATE_TEST_CASE_P(PointSizeArrayedF32TessControl,
                        ValidateVulkanCombineBuiltInArrayedVariable,
                        Combine(Values("PointSize"),
                                Values("TessellationControl"), Values("Input"),
                                Values("%f32"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    PointSizeArrayedF64TessControl, ValidateVulkanCombineBuiltInArrayedVariable,
    Combine(Values("PointSize"), Values("TessellationControl"), Values("Input"),
            Values("%f64"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float scalar",
                              "has bit width 64"))), );

INSTANTIATE_TEST_CASE_P(
    PointSizeArrayedF32Vertex, ValidateVulkanCombineBuiltInArrayedVariable,
    Combine(Values("PointSize"), Values("Vertex"), Values("Output"),
            Values("%f32"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float scalar",
                              "is not a float scalar"))), );

INSTANTIATE_TEST_CASE_P(PositionArrayedF32Vec4TessControl,
                        ValidateVulkanCombineBuiltInArrayedVariable,
                        Combine(Values("Position"),
                                Values("TessellationControl"), Values("Input"),
                                Values("%f32vec4"), Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    PositionArrayedF32Vec3TessControl,
    ValidateVulkanCombineBuiltInArrayedVariable,
    Combine(Values("Position"), Values("TessellationControl"), Values("Input"),
            Values("%f32vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float vector",
                              "has 3 components"))), );

INSTANTIATE_TEST_CASE_P(
    PositionArrayedF32Vec4Vertex, ValidateVulkanCombineBuiltInArrayedVariable,
    Combine(Values("Position"), Values("Vertex"), Values("Output"),
            Values("%f32"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float vector",
                              "is not a float vector"))), );

INSTANTIATE_TEST_CASE_P(
    ClipAndCullDistanceOutputSuccess,
    ValidateVulkanCombineBuiltInArrayedVariable,
    Combine(Values("ClipDistance", "CullDistance"),
            Values("Geometry", "TessellationControl", "TessellationEvaluation"),
            Values("Output"), Values("%f32arr2", "%f32arr4"),
            Values(TestResult())), );

INSTANTIATE_TEST_CASE_P(
    ClipAndCullDistanceVertexInput, ValidateVulkanCombineBuiltInArrayedVariable,
    Combine(Values("ClipDistance", "CullDistance"), Values("Fragment"),
            Values("Input"), Values("%f32arr4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float array",
                              "components are not float scalar"))), );

INSTANTIATE_TEST_CASE_P(
    ClipAndCullDistanceNotArray, ValidateVulkanCombineBuiltInArrayedVariable,
    Combine(Values("ClipDistance", "CullDistance"),
            Values("Geometry", "TessellationControl", "TessellationEvaluation"),
            Values("Input"), Values("%f32vec2", "%f32vec4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float array",
                              "components are not float scalar"))), );

TEST_F(ValidateBuiltIns, WorkgroupSizeSuccess) {
  CodeGenerator generator = GetDefaultShaderCodeGenerator();
  generator.before_types_ = R"(
OpDecorate %workgroup_size BuiltIn WorkgroupSize
)";

  generator.after_types_ = R"(
%workgroup_size = OpConstantComposite %u32vec3 %u32_1 %u32_1 %u32_1
)";

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = "GLCompute";
  entry_point.body = R"(
%copy = OpCopyObject %u32vec3 %workgroup_size
)";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateBuiltIns, WorkgroupSizeFragment) {
  CodeGenerator generator = GetDefaultShaderCodeGenerator();
  generator.before_types_ = R"(
OpDecorate %workgroup_size BuiltIn WorkgroupSize
)";

  generator.after_types_ = R"(
%workgroup_size = OpConstantComposite %u32vec3 %u32_1 %u32_1 %u32_1
)";

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = "Fragment";
  entry_point.execution_modes = "OpExecutionMode %main OriginUpperLeft";
  entry_point.body = R"(
%copy = OpCopyObject %u32vec3 %workgroup_size
)";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Vulkan spec allows BuiltIn WorkgroupSize to be used "
                        "only with GLCompute execution model"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("is referencing ID <2> (OpConstantComposite) which is "
                        "decorated with BuiltIn WorkgroupSize in function <1> "
                        "called with execution model Fragment"));
}

TEST_F(ValidateBuiltIns, WorkgroupSizeNotConstant) {
  CodeGenerator generator = GetDefaultShaderCodeGenerator();
  generator.before_types_ = R"(
OpDecorate %copy BuiltIn WorkgroupSize
)";

  generator.after_types_ = R"(
%workgroup_size = OpConstantComposite %u32vec3 %u32_1 %u32_1 %u32_1
)";

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = "GLCompute";
  entry_point.body = R"(
%copy = OpCopyObject %u32vec3 %workgroup_size
)";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Vulkan spec requires BuiltIn WorkgroupSize to be a "
                        "constant. ID <2> (OpCopyObject) is not a constant"));
}

TEST_F(ValidateBuiltIns, WorkgroupSizeNotVector) {
  CodeGenerator generator = GetDefaultShaderCodeGenerator();
  generator.before_types_ = R"(
OpDecorate %workgroup_size BuiltIn WorkgroupSize
)";

  generator.after_types_ = R"(
%workgroup_size = OpConstant %u32 16
)";

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = "GLCompute";
  entry_point.body = R"(
%copy = OpCopyObject %u32 %workgroup_size
)";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("According to the Vulkan spec BuiltIn WorkgroupSize "
                        "variable needs to be a 3-component 32-bit int vector. "
                        "ID <2> (OpConstant) is not an int vector."));
}

TEST_F(ValidateBuiltIns, WorkgroupSizeNotIntVector) {
  CodeGenerator generator = GetDefaultShaderCodeGenerator();
  generator.before_types_ = R"(
OpDecorate %workgroup_size BuiltIn WorkgroupSize
)";

  generator.after_types_ = R"(
%workgroup_size = OpConstantComposite %f32vec3 %f32_1 %f32_1 %f32_1
)";

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = "GLCompute";
  entry_point.body = R"(
%copy = OpCopyObject %f32vec3 %workgroup_size
)";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("According to the Vulkan spec BuiltIn WorkgroupSize "
                        "variable needs to be a 3-component 32-bit int vector. "
                        "ID <2> (OpConstantComposite) is not an int vector."));
}

TEST_F(ValidateBuiltIns, WorkgroupSizeNotVec3) {
  CodeGenerator generator = GetDefaultShaderCodeGenerator();
  generator.before_types_ = R"(
OpDecorate %workgroup_size BuiltIn WorkgroupSize
)";

  generator.after_types_ = R"(
%workgroup_size = OpConstantComposite %u32vec2 %u32_1 %u32_1
)";

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = "GLCompute";
  entry_point.body = R"(
%copy = OpCopyObject %u32vec2 %workgroup_size
)";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("According to the Vulkan spec BuiltIn WorkgroupSize "
                        "variable needs to be a 3-component 32-bit int vector. "
                        "ID <2> (OpConstantComposite) has 2 components."));
}

TEST_F(ValidateBuiltIns, WorkgroupSizeNotInt32Vec) {
  CodeGenerator generator = GetDefaultShaderCodeGenerator();
  generator.before_types_ = R"(
OpDecorate %workgroup_size BuiltIn WorkgroupSize
)";

  generator.after_types_ = R"(
%workgroup_size = OpConstantComposite %u64vec3 %u64_1 %u64_1 %u64_1
)";

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = "GLCompute";
  entry_point.body = R"(
%copy = OpCopyObject %u64vec3 %workgroup_size
)";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("According to the Vulkan spec BuiltIn WorkgroupSize variable "
                "needs to be a 3-component 32-bit int vector. ID <2> "
                "(OpConstantComposite) has components with bit width 64."));
}

TEST_F(ValidateBuiltIns, WorkgroupSizePrivateVar) {
  CodeGenerator generator = GetDefaultShaderCodeGenerator();
  generator.before_types_ = R"(
OpDecorate %workgroup_size BuiltIn WorkgroupSize
)";

  generator.after_types_ = R"(
%workgroup_size = OpConstantComposite %u32vec3 %u32_1 %u32_1 %u32_1
%private_ptr_u32vec3 = OpTypePointer Private %u32vec3
%var = OpVariable %private_ptr_u32vec3 Private %workgroup_size
)";

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = "GLCompute";
  entry_point.body = R"(
)";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateBuiltIns, GeometryPositionInOutSuccess) {
  CodeGenerator generator = GetDefaultShaderCodeGenerator();

  generator.before_types_ = R"(
OpMemberDecorate %input_type 0 BuiltIn Position
OpMemberDecorate %output_type 0 BuiltIn Position
)";

  generator.after_types_ = R"(
%input_type = OpTypeStruct %f32vec4
%arrayed_input_type = OpTypeArray %input_type %u32_3
%input_ptr = OpTypePointer Input %arrayed_input_type
%input = OpVariable %input_ptr Input
%input_f32vec4_ptr = OpTypePointer Input %f32vec4
%output_type = OpTypeStruct %f32vec4
%arrayed_output_type = OpTypeArray %output_type %u32_3
%output_ptr = OpTypePointer Output %arrayed_output_type
%output = OpVariable %output_ptr Output
%output_f32vec4_ptr = OpTypePointer Output %f32vec4
)";

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = "Geometry";
  entry_point.interfaces = "%input %output";
  entry_point.body = R"(
%input_pos = OpAccessChain %input_f32vec4_ptr %input %u32_0 %u32_0
%output_pos = OpAccessChain %output_f32vec4_ptr %output %u32_0 %u32_0
%pos = OpLoad %f32vec4 %input_pos
OpStore %output_pos %pos
)";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateBuiltIns, TwoBuiltInsFirstFails) {
  CodeGenerator generator = GetDefaultShaderCodeGenerator();

  generator.before_types_ = R"(
OpMemberDecorate %input_type 0 BuiltIn FragCoord
OpMemberDecorate %output_type 0 BuiltIn Position
)";

  generator.after_types_ = R"(
%input_type = OpTypeStruct %f32vec4
%input_ptr = OpTypePointer Input %input_type
%input = OpVariable %input_ptr Input
%input_f32vec4_ptr = OpTypePointer Input %f32vec4
%output_type = OpTypeStruct %f32vec4
%output_ptr = OpTypePointer Output %output_type
%output = OpVariable %output_ptr Output
%output_f32vec4_ptr = OpTypePointer Output %f32vec4
)";

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = "Geometry";
  entry_point.interfaces = "%input %output";
  entry_point.body = R"(
%input_pos = OpAccessChain %input_f32vec4_ptr %input %u32_0
%output_pos = OpAccessChain %output_f32vec4_ptr %output %u32_0
%pos = OpLoad %f32vec4 %input_pos
OpStore %output_pos %pos
)";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Vulkan spec allows BuiltIn FragCoord to be used only "
                        "with Fragment execution model"));
}

TEST_F(ValidateBuiltIns, TwoBuiltInsSecondFails) {
  CodeGenerator generator = GetDefaultShaderCodeGenerator();

  generator.before_types_ = R"(
OpMemberDecorate %input_type 0 BuiltIn Position
OpMemberDecorate %output_type 0 BuiltIn FragCoord
)";

  generator.after_types_ = R"(
%input_type = OpTypeStruct %f32vec4
%input_ptr = OpTypePointer Input %input_type
%input = OpVariable %input_ptr Input
%input_f32vec4_ptr = OpTypePointer Input %f32vec4
%output_type = OpTypeStruct %f32vec4
%output_ptr = OpTypePointer Output %output_type
%output = OpVariable %output_ptr Output
%output_f32vec4_ptr = OpTypePointer Output %f32vec4
)";

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = "Geometry";
  entry_point.interfaces = "%input %output";
  entry_point.body = R"(
%input_pos = OpAccessChain %input_f32vec4_ptr %input %u32_0
%output_pos = OpAccessChain %output_f32vec4_ptr %output %u32_0
%pos = OpLoad %f32vec4 %input_pos
OpStore %output_pos %pos
)";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Vulkan spec allows BuiltIn FragCoord to be only used "
                        "for variables with Input storage class"));
}

TEST_F(ValidateBuiltIns, VertexPositionVariableSuccess) {
  CodeGenerator generator = GetDefaultShaderCodeGenerator();
  generator.before_types_ = R"(
OpDecorate %position BuiltIn Position
)";

  generator.after_types_ = R"(
%f32vec4_ptr_output = OpTypePointer Output %f32vec4
%position = OpVariable %f32vec4_ptr_output Output
)";

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = "Vertex";
  entry_point.interfaces = "%position";
  entry_point.body = R"(
OpStore %position %f32vec4_0123
)";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateBuiltIns, FragmentPositionTwoEntryPoints) {
  CodeGenerator generator = GetDefaultShaderCodeGenerator();
  generator.before_types_ = R"(
OpMemberDecorate %output_type 0 BuiltIn Position
)";

  generator.after_types_ = R"(
%output_type = OpTypeStruct %f32vec4
%output_ptr = OpTypePointer Output %output_type
%output = OpVariable %output_ptr Output
%output_f32vec4_ptr = OpTypePointer Output %f32vec4
)";

  EntryPoint entry_point;
  entry_point.name = "vmain";
  entry_point.execution_model = "Vertex";
  entry_point.interfaces = "%output";
  entry_point.body = R"(
%val1 = OpFunctionCall %void %foo
)";
  generator.entry_points_.push_back(std::move(entry_point));

  entry_point.name = "fmain";
  entry_point.execution_model = "Fragment";
  entry_point.interfaces = "%output";
  entry_point.execution_modes = "OpExecutionMode %fmain OriginUpperLeft";
  entry_point.body = R"(
%val2 = OpFunctionCall %void %foo
)";
  generator.entry_points_.push_back(std::move(entry_point));

  generator.add_at_the_end_ = R"(
%foo = OpFunction %void None %func
%foo_entry = OpLabel
%position = OpAccessChain %output_f32vec4_ptr %output %u32_0
OpStore %position %f32vec4_0123
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Vulkan spec allows BuiltIn Position to be used only "
                        "with Vertex, TessellationControl, "
                        "TessellationEvaluation or Geometry execution models"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("called with execution model Fragment"));
}

TEST_F(ValidateBuiltIns, FragmentFragDepthNoDepthReplacing) {
  CodeGenerator generator = GetDefaultShaderCodeGenerator();
  generator.before_types_ = R"(
OpMemberDecorate %output_type 0 BuiltIn FragDepth
)";

  generator.after_types_ = R"(
%output_type = OpTypeStruct %f32
%output_ptr = OpTypePointer Output %output_type
%output = OpVariable %output_ptr Output
%output_f32_ptr = OpTypePointer Output %f32
)";

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = "Fragment";
  entry_point.interfaces = "%output";
  entry_point.execution_modes = "OpExecutionMode %main OriginUpperLeft";
  entry_point.body = R"(
%val2 = OpFunctionCall %void %foo
)";
  generator.entry_points_.push_back(std::move(entry_point));

  generator.add_at_the_end_ = R"(
%foo = OpFunction %void None %func
%foo_entry = OpLabel
%frag_depth = OpAccessChain %output_f32_ptr %output %u32_0
OpStore %frag_depth %f32_1
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Vulkan spec requires DepthReplacing execution mode to "
                        "be declared when using BuiltIn FragDepth"));
}

TEST_F(ValidateBuiltIns, FragmentFragDepthOneMainHasDepthReplacingOtherHasnt) {
  CodeGenerator generator = GetDefaultShaderCodeGenerator();
  generator.before_types_ = R"(
OpMemberDecorate %output_type 0 BuiltIn FragDepth
)";

  generator.after_types_ = R"(
%output_type = OpTypeStruct %f32
%output_ptr = OpTypePointer Output %output_type
%output = OpVariable %output_ptr Output
%output_f32_ptr = OpTypePointer Output %f32
)";

  EntryPoint entry_point;
  entry_point.name = "main_d_r";
  entry_point.execution_model = "Fragment";
  entry_point.interfaces = "%output";
  entry_point.execution_modes =
      "OpExecutionMode %main_d_r OriginUpperLeft\n"
      "OpExecutionMode %main_d_r DepthReplacing";
  entry_point.body = R"(
%val2 = OpFunctionCall %void %foo
)";
  generator.entry_points_.push_back(std::move(entry_point));

  entry_point.name = "main_no_d_r";
  entry_point.execution_model = "Fragment";
  entry_point.interfaces = "%output";
  entry_point.execution_modes = "OpExecutionMode %main_no_d_r OriginUpperLeft";
  entry_point.body = R"(
%val3 = OpFunctionCall %void %foo
)";
  generator.entry_points_.push_back(std::move(entry_point));

  generator.add_at_the_end_ = R"(
%foo = OpFunction %void None %func
%foo_entry = OpLabel
%frag_depth = OpAccessChain %output_f32_ptr %output %u32_0
OpStore %frag_depth %f32_1
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Vulkan spec requires DepthReplacing execution mode to "
                        "be declared when using BuiltIn FragDepth"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
