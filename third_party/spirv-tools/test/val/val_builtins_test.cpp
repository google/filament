// Copyright (c) 2018 Google LLC.
// Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights
// reserved.
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
#include "source/spirv_target_env.h"
#include "test/unit_spirv.h"
#include "test/val/val_code_generator.h"
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
using ValidateVulkanSubgroupBuiltIns =
    spvtest::ValidateBase<std::tuple<const char*, const char*, const char*,
                                     const char*, const char*, TestResult>>;
using ValidateVulkanCombineBuiltInExecutionModelDataTypeResult =
    spvtest::ValidateBase<std::tuple<const char*, const char*, const char*,
                                     const char*, const char*, TestResult>>;
using ValidateVulkanCombineBuiltInArrayedVariable = spvtest::ValidateBase<
    std::tuple<const char*, const char*, const char*, const char*, TestResult>>;
using ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult =
    spvtest::ValidateBase<
        std::tuple<const char*, const char*, const char*, const char*,
                   const char*, const char*, const char*, TestResult>>;

using ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult =
    spvtest::ValidateBase<std::tuple<spv_target_env, const char*, const char*,
                                     const char*, const char*, const char*,
                                     const char*, const char*, TestResult>>;

bool InitializerRequired(const char* const storage_class) {
  return (strncmp(storage_class, "Output", 6) == 0 ||
          strncmp(storage_class, "Private", 7) == 0 ||
          strncmp(storage_class, "Function", 8) == 0);
}

CodeGenerator GetInMainCodeGenerator(const char* const built_in,
                                     const char* const execution_model,
                                     const char* const storage_class,
                                     const char* const capabilities,
                                     const char* const extensions,
                                     const char* const data_type) {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();

  if (capabilities) {
    generator.capabilities_ += capabilities;
  }
  if (extensions) {
    generator.extensions_ += extensions;
  }

  generator.before_types_ = R"(OpDecorate %built_in_type Block
                               OpMemberDecorate %built_in_type 0 BuiltIn )";
  generator.before_types_ += built_in;
  generator.before_types_ += "\n";

  std::ostringstream after_types;

  after_types << "%built_in_type = OpTypeStruct " << data_type << "\n";
  if (InitializerRequired(storage_class)) {
    after_types << "%built_in_null = OpConstantNull %built_in_type\n";
  }
  after_types << "%built_in_ptr = OpTypePointer " << storage_class
              << " %built_in_type\n";
  after_types << "%built_in_var = OpVariable %built_in_ptr " << storage_class;
  if (InitializerRequired(storage_class)) {
    after_types << " %built_in_null";
  }
  after_types << "\n";
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
    if (0 == std::strcmp(built_in, "FragDepth")) {
      execution_modes << "OpExecutionMode %" << entry_point.name
                      << " DepthReplacing\n";
    }
  }
  if (0 == std::strcmp(execution_model, "Geometry")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " InputPoints\n";
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " OutputPoints\n";
  }
  if (0 == std::strcmp(execution_model, "GLCompute")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " LocalSize 1 1 1\n";
  }
  entry_point.execution_modes = execution_modes.str();

  entry_point.body = R"(
%ptr = OpAccessChain %data_ptr %built_in_var %u32_0
)";
  generator.entry_points_.push_back(std::move(entry_point));

  return generator;
}

TEST_P(ValidateVulkanCombineBuiltInExecutionModelDataTypeResult, InMain) {
  const char* const built_in = std::get<0>(GetParam());
  const char* const execution_model = std::get<1>(GetParam());
  const char* const storage_class = std::get<2>(GetParam());
  const char* const data_type = std::get<3>(GetParam());
  const char* const vuid = std::get<4>(GetParam());
  const TestResult& test_result = std::get<5>(GetParam());

  CodeGenerator generator = GetInMainCodeGenerator(
      built_in, execution_model, storage_class, NULL, NULL, data_type);

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(test_result.validation_result,
            ValidateInstructions(SPV_ENV_VULKAN_1_0));
  if (test_result.error_str) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str));
  }
  if (test_result.error_str2) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str2));
  }
  if (vuid) {
    EXPECT_THAT(getDiagnosticString(), AnyVUID(vuid));
  }
}

TEST_P(
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    InMain) {
  const char* const built_in = std::get<0>(GetParam());
  const char* const execution_model = std::get<1>(GetParam());
  const char* const storage_class = std::get<2>(GetParam());
  const char* const data_type = std::get<3>(GetParam());
  const char* const capabilities = std::get<4>(GetParam());
  const char* const extensions = std::get<5>(GetParam());
  const char* const vuid = std::get<6>(GetParam());
  const TestResult& test_result = std::get<7>(GetParam());

  CodeGenerator generator =
      GetInMainCodeGenerator(built_in, execution_model, storage_class,
                             capabilities, extensions, data_type);

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(test_result.validation_result,
            ValidateInstructions(SPV_ENV_VULKAN_1_0));
  if (test_result.error_str) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str));
  }
  if (test_result.error_str2) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str2));
  }
  if (vuid) {
    EXPECT_THAT(getDiagnosticString(), AnyVUID(vuid));
  }
}

TEST_P(
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    InMain) {
  const spv_target_env env = std::get<0>(GetParam());
  const char* const built_in = std::get<1>(GetParam());
  const char* const execution_model = std::get<2>(GetParam());
  const char* const storage_class = std::get<3>(GetParam());
  const char* const data_type = std::get<4>(GetParam());
  const char* const capabilities = std::get<5>(GetParam());
  const char* const extensions = std::get<6>(GetParam());
  const char* const vuid = std::get<7>(GetParam());
  const TestResult& test_result = std::get<8>(GetParam());

  CodeGenerator generator =
      GetInMainCodeGenerator(built_in, execution_model, storage_class,
                             capabilities, extensions, data_type);

  CompileSuccessfully(generator.Build(), env);
  ASSERT_EQ(test_result.validation_result, ValidateInstructions(env));
  if (test_result.error_str) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str));
  }
  if (test_result.error_str2) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str2));
  }
  if (vuid) {
    EXPECT_THAT(getDiagnosticString(), AnyVUID(vuid));
  }
}

CodeGenerator GetInFunctionCodeGenerator(const char* const built_in,
                                         const char* const execution_model,
                                         const char* const storage_class,
                                         const char* const capabilities,
                                         const char* const extensions,
                                         const char* const data_type) {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();

  if (capabilities) {
    generator.capabilities_ += capabilities;
  }
  if (extensions) {
    generator.extensions_ += extensions;
  }

  generator.before_types_ = R"(OpDecorate %built_in_type Block
                              OpMemberDecorate %built_in_type 0 BuiltIn )";
  generator.before_types_ += built_in;
  generator.before_types_ += "\n";

  std::ostringstream after_types;
  after_types << "%built_in_type = OpTypeStruct " << data_type << "\n";
  if (InitializerRequired(storage_class)) {
    after_types << "%built_in_null = OpConstantNull %built_in_type\n";
  }
  after_types << "%built_in_ptr = OpTypePointer " << storage_class
              << " %built_in_type\n";
  after_types << "%built_in_var = OpVariable %built_in_ptr " << storage_class;
  if (InitializerRequired(storage_class)) {
    after_types << " %built_in_null";
  }
  after_types << "\n";
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
    if (0 == std::strcmp(built_in, "FragDepth")) {
      execution_modes << "OpExecutionMode %" << entry_point.name
                      << " DepthReplacing\n";
    }
  }
  if (0 == std::strcmp(execution_model, "Geometry")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " InputPoints\n";
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " OutputPoints\n";
  }
  if (0 == std::strcmp(execution_model, "GLCompute")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " LocalSize 1 1 1\n";
  }
  entry_point.execution_modes = execution_modes.str();

  entry_point.body = R"(
%val2 = OpFunctionCall %void %foo
)";

  std::string function_body = R"(
%foo = OpFunction %void None %func
%foo_entry = OpLabel
%ptr = OpAccessChain %data_ptr %built_in_var %u32_0
OpReturn
OpFunctionEnd
)";

  generator.add_at_the_end_ = function_body;

  generator.entry_points_.push_back(std::move(entry_point));

  return generator;
}

TEST_P(ValidateVulkanCombineBuiltInExecutionModelDataTypeResult, InFunction) {
  const char* const built_in = std::get<0>(GetParam());
  const char* const execution_model = std::get<1>(GetParam());
  const char* const storage_class = std::get<2>(GetParam());
  const char* const data_type = std::get<3>(GetParam());
  const char* const vuid = std::get<4>(GetParam());
  const TestResult& test_result = std::get<5>(GetParam());

  CodeGenerator generator = GetInFunctionCodeGenerator(
      built_in, execution_model, storage_class, NULL, NULL, data_type);

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(test_result.validation_result,
            ValidateInstructions(SPV_ENV_VULKAN_1_0));
  if (test_result.error_str) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str));
  }
  if (test_result.error_str2) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str2));
  }
  if (vuid) {
    EXPECT_THAT(getDiagnosticString(), AnyVUID(vuid));
  }
}

TEST_P(
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    InFunction) {
  const char* const built_in = std::get<0>(GetParam());
  const char* const execution_model = std::get<1>(GetParam());
  const char* const storage_class = std::get<2>(GetParam());
  const char* const data_type = std::get<3>(GetParam());
  const char* const capabilities = std::get<4>(GetParam());
  const char* const extensions = std::get<5>(GetParam());
  const char* const vuid = std::get<6>(GetParam());
  const TestResult& test_result = std::get<7>(GetParam());

  CodeGenerator generator =
      GetInFunctionCodeGenerator(built_in, execution_model, storage_class,
                                 capabilities, extensions, data_type);

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(test_result.validation_result,
            ValidateInstructions(SPV_ENV_VULKAN_1_0));
  if (test_result.error_str) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str));
  }
  if (test_result.error_str2) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str2));
  }
  if (vuid) {
    EXPECT_THAT(getDiagnosticString(), AnyVUID(vuid));
  }
}

CodeGenerator GetVariableCodeGenerator(const char* const built_in,
                                       const char* const execution_model,
                                       const char* const storage_class,
                                       const char* const capabilities,
                                       const char* const extensions,
                                       const char* const data_type) {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();

  if (capabilities) {
    generator.capabilities_ += capabilities;
  }
  if (extensions) {
    generator.extensions_ += extensions;
  }

  generator.before_types_ = "OpDecorate %built_in_var BuiltIn ";
  generator.before_types_ += built_in;
  generator.before_types_ += "\n";

  std::ostringstream after_types;
  if (InitializerRequired(storage_class)) {
    after_types << "%built_in_null = OpConstantNull " << data_type << "\n";
  }
  after_types << "%built_in_ptr = OpTypePointer " << storage_class << " "
              << data_type << "\n";
  after_types << "%built_in_var = OpVariable %built_in_ptr " << storage_class;
  if (InitializerRequired(storage_class)) {
    after_types << " %built_in_null";
  }
  after_types << "\n";
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
%val = OpBitcast %u32 %built_in_var
)";

  std::ostringstream execution_modes;
  if (0 == std::strcmp(execution_model, "Fragment")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " OriginUpperLeft\n";
    if (0 == std::strcmp(built_in, "FragDepth")) {
      execution_modes << "OpExecutionMode %" << entry_point.name
                      << " DepthReplacing\n";
    }
  }
  if (0 == std::strcmp(execution_model, "Geometry")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " InputPoints\n";
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " OutputPoints\n";
  }
  if (0 == std::strcmp(execution_model, "GLCompute")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " LocalSize 1 1 1\n";
  }
  entry_point.execution_modes = execution_modes.str();

  generator.entry_points_.push_back(std::move(entry_point));

  return generator;
}

TEST_P(ValidateVulkanCombineBuiltInExecutionModelDataTypeResult, Variable) {
  const char* const built_in = std::get<0>(GetParam());
  const char* const execution_model = std::get<1>(GetParam());
  const char* const storage_class = std::get<2>(GetParam());
  const char* const data_type = std::get<3>(GetParam());
  const char* const vuid = std::get<4>(GetParam());
  const TestResult& test_result = std::get<5>(GetParam());

  CodeGenerator generator = GetVariableCodeGenerator(
      built_in, execution_model, storage_class, NULL, NULL, data_type);

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(test_result.validation_result,
            ValidateInstructions(SPV_ENV_VULKAN_1_0));
  if (test_result.error_str) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str));
  }
  if (test_result.error_str2) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str2));
  }
  if (vuid) {
    EXPECT_THAT(getDiagnosticString(), AnyVUID(vuid));
  }
}

TEST_P(
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Variable) {
  const char* const built_in = std::get<0>(GetParam());
  const char* const execution_model = std::get<1>(GetParam());
  const char* const storage_class = std::get<2>(GetParam());
  const char* const data_type = std::get<3>(GetParam());
  const char* const capabilities = std::get<4>(GetParam());
  const char* const extensions = std::get<5>(GetParam());
  const char* const vuid = std::get<6>(GetParam());
  const TestResult& test_result = std::get<7>(GetParam());

  CodeGenerator generator =
      GetVariableCodeGenerator(built_in, execution_model, storage_class,
                               capabilities, extensions, data_type);

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(test_result.validation_result,
            ValidateInstructions(SPV_ENV_VULKAN_1_0));
  if (test_result.error_str) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str));
  }
  if (test_result.error_str2) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str2));
  }
  if (vuid) {
    EXPECT_THAT(getDiagnosticString(), AnyVUID(vuid));
  }
}

INSTANTIATE_TEST_SUITE_P(
    ClipAndCullDistanceOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"),
            Values("Vertex", "Geometry", "TessellationControl",
                   "TessellationEvaluation"),
            Values("Output"), Values("%f32arr2", "%f32arr4"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    ClipAndCullDistanceInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"),
            Values("Fragment", "Geometry", "TessellationControl",
                   "TessellationEvaluation"),
            Values("Input"), Values("%f32arr2", "%f32arr4"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    ClipAndCullDistanceInvalidStorageClass,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"),
            Values("Vertex", "Geometry", "TessellationControl",
                   "TessellationEvaluation"),
            Values("Private"), Values("%f32arr2", "%f32arr4"),
            Values("VUID-ClipDistance-ClipDistance-04190 "
                   "VUID-CullDistance-CullDistance-04199"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input or Output storage "
                "class."))));

INSTANTIATE_TEST_SUITE_P(
    ClipAndCullDistanceFragmentOutput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"), Values("Fragment"),
            Values("Output"), Values("%f32arr4"),
            Values("VUID-ClipDistance-ClipDistance-04189 "
                   "VUID-CullDistance-CullDistance-04198"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec doesn't allow BuiltIn ClipDistance/CullDistance "
                "to be used for variables with Output storage class if "
                "execution model is Fragment.",
                "which is called with execution model Fragment."))));

INSTANTIATE_TEST_SUITE_P(
    VertexIdVertexInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("VertexId"), Values("Vertex"), Values("Input"), Values("%u32"),
        Values(nullptr),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "Vulkan spec doesn't allow BuiltIn VertexId to be "
                          "used."))));

INSTANTIATE_TEST_SUITE_P(
    ClipAndCullDistanceVertexInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"), Values("Vertex"),
            Values("Input"), Values("%f32arr4"),
            Values("VUID-ClipDistance-ClipDistance-04188 "
                   "VUID-CullDistance-CullDistance-04197"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec doesn't allow BuiltIn ClipDistance/CullDistance "
                "to be used for variables with Input storage class if "
                "execution model is Vertex.",
                "which is called with execution model Vertex."))));

INSTANTIATE_TEST_SUITE_P(
    ClipAndCullInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"), Values("GLCompute"),
            Values("Input", "Output"), Values("%f32arr4"),
            Values("VUID-ClipDistance-ClipDistance-04187 "
                   "VUID-CullDistance-CullDistance-04196"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be used only with Fragment, Vertex, TessellationControl, "
                "TessellationEvaluation or Geometry execution models"))));

INSTANTIATE_TEST_SUITE_P(
    ClipAndCullDistanceNotArray,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"), Values("Fragment"),
            Values("Input"), Values("%f32vec2", "%f32vec4", "%f32"),
            Values("VUID-ClipDistance-ClipDistance-04191 "
                   "VUID-CullDistance-CullDistance-04200"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float array",
                              "is not an array"))));

INSTANTIATE_TEST_SUITE_P(
    ClipAndCullDistanceNotFloatArray,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"), Values("Fragment"),
            Values("Input"), Values("%u32arr2", "%u64arr4"),
            Values("VUID-ClipDistance-ClipDistance-04191 "
                   "VUID-CullDistance-CullDistance-04200"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float array",
                              "components are not float scalar"))));

INSTANTIATE_TEST_SUITE_P(
    ClipAndCullDistanceNotF32Array,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ClipDistance", "CullDistance"), Values("Fragment"),
            Values("Input"), Values("%f64arr2", "%f64arr4"),
            Values("VUID-ClipDistance-ClipDistance-04191 "
                   "VUID-CullDistance-CullDistance-04200"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float array",
                              "has components with bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    FragCoordSuccess, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragCoord"), Values("Fragment"), Values("Input"),
            Values("%f32vec4"), Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    FragCoordNotFragment,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("FragCoord"),
        Values("Vertex", "GLCompute", "Geometry", "TessellationControl",
               "TessellationEvaluation"),
        Values("Input"), Values("%f32vec4"),
        Values("VUID-FragCoord-FragCoord-04210"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "to be used only with Fragment execution model"))));

INSTANTIATE_TEST_SUITE_P(
    FragCoordNotInput, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragCoord"), Values("Fragment"), Values("Output"),
            Values("%f32vec4"), Values("VUID-FragCoord-FragCoord-04211"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))));

INSTANTIATE_TEST_SUITE_P(
    FragCoordNotFloatVector,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragCoord"), Values("Fragment"), Values("Input"),
            Values("%f32arr4", "%u32vec4"),
            Values("VUID-FragCoord-FragCoord-04212"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float vector",
                              "is not a float vector"))));

INSTANTIATE_TEST_SUITE_P(
    FragCoordNotFloatVec4,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragCoord"), Values("Fragment"), Values("Input"),
            Values("%f32vec3"), Values("VUID-FragCoord-FragCoord-04212"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float vector",
                              "has 3 components"))));

INSTANTIATE_TEST_SUITE_P(
    FragCoordNotF32Vec4,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragCoord"), Values("Fragment"), Values("Input"),
            Values("%f64vec4"), Values("VUID-FragCoord-FragCoord-04212"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float vector",
                              "has components with bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    FragDepthSuccess, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragDepth"), Values("Fragment"), Values("Output"),
            Values("%f32"), Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    FragDepthNotFragment,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("FragDepth"),
        Values("Vertex", "GLCompute", "Geometry", "TessellationControl",
               "TessellationEvaluation"),
        Values("Output"), Values("%f32"),
        Values("VUID-FragDepth-FragDepth-04213"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "to be used only with Fragment execution model"))));

INSTANTIATE_TEST_SUITE_P(
    FragDepthNotOutput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragDepth"), Values("Fragment"), Values("Input"),
            Values("%f32"), Values("VUID-FragDepth-FragDepth-04214"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Output storage class",
                "uses storage class Input"))));

INSTANTIATE_TEST_SUITE_P(
    FragDepthNotFloatScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragDepth"), Values("Fragment"), Values("Output"),
            Values("%f32vec4", "%u32"),
            Values("VUID-FragDepth-FragDepth-04215"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float scalar",
                              "is not a float scalar"))));

INSTANTIATE_TEST_SUITE_P(
    FragDepthNotF32, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FragDepth"), Values("Fragment"), Values("Output"),
            Values("%f64"), Values("VUID-FragDepth-FragDepth-04215"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float scalar",
                              "has bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    FrontFacingAndHelperInvocationSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FrontFacing", "HelperInvocation"), Values("Fragment"),
            Values("Input"), Values("%bool"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    FrontFacingAndHelperInvocationNotFragment,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("FrontFacing", "HelperInvocation"),
        Values("Vertex", "GLCompute", "Geometry", "TessellationControl",
               "TessellationEvaluation"),
        Values("Input"), Values("%bool"),
        Values("VUID-FrontFacing-FrontFacing-04229 "
               "VUID-HelperInvocation-HelperInvocation-04239"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "to be used only with Fragment execution model"))));

INSTANTIATE_TEST_SUITE_P(
    FrontFacingAndHelperInvocationNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FrontFacing", "HelperInvocation"), Values("Fragment"),
            Values("Output"), Values("%bool"),
            Values("VUID-FrontFacing-FrontFacing-04230 "
                   "VUID-HelperInvocation-HelperInvocation-04240"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))));

INSTANTIATE_TEST_SUITE_P(
    FrontFacingAndHelperInvocationNotBool,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("FrontFacing", "HelperInvocation"), Values("Fragment"),
            Values("Input"), Values("%f32", "%u32"),
            Values("VUID-FrontFacing-FrontFacing-04231 "
                   "VUID-HelperInvocation-HelperInvocation-04241"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a bool scalar",
                              "is not a bool scalar"))));

INSTANTIATE_TEST_SUITE_P(
    ComputeShaderInputInt32Vec3Success,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("GlobalInvocationId", "LocalInvocationId", "NumWorkgroups",
                   "WorkgroupId"),
            Values("GLCompute"), Values("Input"), Values("%u32vec3"),
            Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    ComputeShaderInputInt32Vec3NotGLCompute,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("GlobalInvocationId", "LocalInvocationId", "NumWorkgroups",
                   "WorkgroupId"),
            Values("Vertex", "Fragment", "Geometry", "TessellationControl",
                   "TessellationEvaluation"),
            Values("Input"), Values("%u32vec3"),
            Values("VUID-GlobalInvocationId-GlobalInvocationId-04236 "
                   "VUID-LocalInvocationId-LocalInvocationId-04281 "
                   "VUID-NumWorkgroups-NumWorkgroups-04296 "
                   "VUID-WorkgroupId-WorkgroupId-04422"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "to be used only with GLCompute, MeshNV, or "
                              "TaskNV execution model"))));

INSTANTIATE_TEST_SUITE_P(
    ComputeShaderInputInt32Vec3NotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("GlobalInvocationId", "LocalInvocationId", "NumWorkgroups",
                   "WorkgroupId"),
            Values("GLCompute"), Values("Output"), Values("%u32vec3"),
            Values("VUID-GlobalInvocationId-GlobalInvocationId-04237 "
                   "VUID-LocalInvocationId-LocalInvocationId-04282 "
                   "VUID-NumWorkgroups-NumWorkgroups-04297 "
                   "VUID-WorkgroupId-WorkgroupId-04423"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))));

INSTANTIATE_TEST_SUITE_P(
    ComputeShaderInputInt32Vec3NotIntVector,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("GlobalInvocationId", "LocalInvocationId", "NumWorkgroups",
                   "WorkgroupId"),
            Values("GLCompute"), Values("Input"),
            Values("%u32arr3", "%f32vec3"),
            Values("VUID-GlobalInvocationId-GlobalInvocationId-04238 "
                   "VUID-LocalInvocationId-LocalInvocationId-04283 "
                   "VUID-NumWorkgroups-NumWorkgroups-04298 "
                   "VUID-WorkgroupId-WorkgroupId-04424"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 3-component 32-bit int vector",
                              "is not an int vector"))));

INSTANTIATE_TEST_SUITE_P(
    ComputeShaderInputInt32Vec3NotIntVec3,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("GlobalInvocationId", "LocalInvocationId", "NumWorkgroups",
                   "WorkgroupId"),
            Values("GLCompute"), Values("Input"), Values("%u32vec4"),
            Values("VUID-GlobalInvocationId-GlobalInvocationId-04238 "
                   "VUID-LocalInvocationId-LocalInvocationId-04283 "
                   "VUID-NumWorkgroups-NumWorkgroups-04298 "
                   "VUID-WorkgroupId-WorkgroupId-04424"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 3-component 32-bit int vector",
                              "has 4 components"))));

INSTANTIATE_TEST_SUITE_P(
    ComputeShaderInputInt32Vec3NotInt32Vec,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("GlobalInvocationId", "LocalInvocationId", "NumWorkgroups",
                   "WorkgroupId"),
            Values("GLCompute"), Values("Input"), Values("%u64vec3"),
            Values("VUID-GlobalInvocationId-GlobalInvocationId-04238 "
                   "VUID-LocalInvocationId-LocalInvocationId-04283 "
                   "VUID-NumWorkgroups-NumWorkgroups-04298 "
                   "VUID-WorkgroupId-WorkgroupId-04424"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 3-component 32-bit int vector",
                              "has components with bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    InvocationIdSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InvocationId"), Values("Geometry", "TessellationControl"),
            Values("Input"), Values("%u32"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    InvocationIdInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InvocationId"),
            Values("Vertex", "Fragment", "GLCompute", "TessellationEvaluation"),
            Values("Input"), Values("%u32"),
            Values("VUID-InvocationId-InvocationId-04257"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "to be used only with TessellationControl or "
                              "Geometry execution models"))));

INSTANTIATE_TEST_SUITE_P(
    InvocationIdNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InvocationId"), Values("Geometry", "TessellationControl"),
            Values("Output"), Values("%u32"),
            Values("VUID-InvocationId-InvocationId-04258"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))));

INSTANTIATE_TEST_SUITE_P(
    InvocationIdNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InvocationId"), Values("Geometry", "TessellationControl"),
            Values("Input"), Values("%f32", "%u32vec3"),
            Values("VUID-InvocationId-InvocationId-04259"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))));

INSTANTIATE_TEST_SUITE_P(
    InvocationIdNotInt32,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InvocationId"), Values("Geometry", "TessellationControl"),
            Values("Input"), Values("%u64"),
            Values("VUID-InvocationId-InvocationId-04259"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "has bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    InstanceIndexSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InstanceIndex"), Values("Vertex"), Values("Input"),
            Values("%u32"), Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    InstanceIndexInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InstanceIndex"),
            Values("Geometry", "Fragment", "GLCompute", "TessellationControl",
                   "TessellationEvaluation"),
            Values("Input"), Values("%u32"),
            Values("VUID-InstanceIndex-InstanceIndex-04263"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "to be used only with Vertex execution model"))));

INSTANTIATE_TEST_SUITE_P(
    InstanceIndexNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InstanceIndex"), Values("Vertex"), Values("Output"),
            Values("%u32"), Values("VUID-InstanceIndex-InstanceIndex-04264"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))));

INSTANTIATE_TEST_SUITE_P(
    InstanceIndexNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InstanceIndex"), Values("Vertex"), Values("Input"),
            Values("%f32", "%u32vec3"),
            Values("VUID-InstanceIndex-InstanceIndex-04265"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))));

INSTANTIATE_TEST_SUITE_P(
    InstanceIndexNotInt32,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("InstanceIndex"), Values("Vertex"), Values("Input"),
            Values("%u64"), Values("VUID-InstanceIndex-InstanceIndex-04265"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "has bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    LayerAndViewportIndexInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Layer", "ViewportIndex"), Values("Fragment"),
            Values("Input"), Values("%u32"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    LayerAndViewportIndexOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Layer", "ViewportIndex"), Values("Geometry"),
            Values("Output"), Values("%u32"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    LayerAndViewportIndexInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("Layer", "ViewportIndex"),
        Values("TessellationControl", "GLCompute"), Values("Input"),
        Values("%u32"),
        Values("VUID-Layer-Layer-04272 VUID-ViewportIndex-ViewportIndex-04404"),
        Values(
            TestResult(SPV_ERROR_INVALID_DATA,
                       "to be used only with Vertex, TessellationEvaluation, "
                       "Geometry, or Fragment execution models"))));

INSTANTIATE_TEST_SUITE_P(
    ViewportIndexExecutionModelEnabledByCapability,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("ViewportIndex"), Values("Vertex", "TessellationEvaluation"),
            Values("Output"), Values("%u32"),
            Values("VUID-ViewportIndex-ViewportIndex-04405"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "ShaderViewportIndexLayerEXT or ShaderViewportIndex"))));

INSTANTIATE_TEST_SUITE_P(
    LayerExecutionModelEnabledByCapability,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Layer"), Values("Vertex", "TessellationEvaluation"),
            Values("Output"), Values("%u32"), Values("VUID-Layer-Layer-04273"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "ShaderViewportIndexLayerEXT or ShaderLayer"))));

INSTANTIATE_TEST_SUITE_P(
    LayerAndViewportIndexFragmentNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("Layer", "ViewportIndex"), Values("Fragment"), Values("Output"),
        Values("%u32"),
        Values("VUID-Layer-Layer-04275 VUID-ViewportIndex-ViewportIndex-04407"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "Output storage class if execution model is Fragment",
                          "which is called with execution model Fragment"))));

INSTANTIATE_TEST_SUITE_P(
    LayerAndViewportIndexGeometryNotOutput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("Layer", "ViewportIndex"),
        Values("Vertex", "TessellationEvaluation", "Geometry"), Values("Input"),
        Values("%u32"),
        Values("VUID-Layer-Layer-04274 VUID-ViewportIndex-ViewportIndex-04406"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "Input storage class if execution model is Vertex, "
                          "TessellationEvaluation, Geometry, or MeshNV",
                          "which is called with execution model"))));

INSTANTIATE_TEST_SUITE_P(
    LayerAndViewportIndexNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("Layer", "ViewportIndex"), Values("Fragment"), Values("Input"),
        Values("%f32", "%u32vec3"),
        Values("VUID-Layer-Layer-04276 VUID-ViewportIndex-ViewportIndex-04408"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "needs to be a 32-bit int scalar",
                          "is not an int scalar"))));

INSTANTIATE_TEST_SUITE_P(
    LayerAndViewportIndexNotInt32,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("Layer", "ViewportIndex"), Values("Fragment"), Values("Input"),
        Values("%u64"),
        Values("VUID-Layer-Layer-04276 VUID-ViewportIndex-ViewportIndex-04408"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "needs to be a 32-bit int scalar",
                          "has bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    LayerCapability,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("Layer"), Values("Vertex"),
            Values("Output"), Values("%u32"),
            Values("OpCapability ShaderLayer\n"), Values(nullptr),
            Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    ViewportIndexCapability,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("ViewportIndex"),
            Values("Vertex"), Values("Output"), Values("%u32"),
            Values("OpCapability ShaderViewportIndex\n"), Values(nullptr),
            Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    PatchVerticesSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PatchVertices"),
            Values("TessellationEvaluation", "TessellationControl"),
            Values("Input"), Values("%u32"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    PatchVerticesInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PatchVertices"),
            Values("Vertex", "Fragment", "GLCompute", "Geometry"),
            Values("Input"), Values("%u32"),
            Values("VUID-PatchVertices-PatchVertices-04308"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "to be used only with TessellationControl or "
                              "TessellationEvaluation execution models"))));

INSTANTIATE_TEST_SUITE_P(
    PatchVerticesNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PatchVertices"),
            Values("TessellationEvaluation", "TessellationControl"),
            Values("Output"), Values("%u32"),
            Values("VUID-PatchVertices-PatchVertices-04309"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))));

INSTANTIATE_TEST_SUITE_P(
    PatchVerticesNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PatchVertices"),
            Values("TessellationEvaluation", "TessellationControl"),
            Values("Input"), Values("%f32", "%u32vec3"),
            Values("VUID-PatchVertices-PatchVertices-04310"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))));

INSTANTIATE_TEST_SUITE_P(
    PatchVerticesNotInt32,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PatchVertices"),
            Values("TessellationEvaluation", "TessellationControl"),
            Values("Input"), Values("%u64"),
            Values("VUID-PatchVertices-PatchVertices-04310"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "has bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    PointCoordSuccess, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointCoord"), Values("Fragment"), Values("Input"),
            Values("%f32vec2"), Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    PointCoordNotFragment,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("PointCoord"),
        Values("Vertex", "GLCompute", "Geometry", "TessellationControl",
               "TessellationEvaluation"),
        Values("Input"), Values("%f32vec2"),
        Values("VUID-PointCoord-PointCoord-04311"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "to be used only with Fragment execution model"))));

INSTANTIATE_TEST_SUITE_P(
    PointCoordNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointCoord"), Values("Fragment"), Values("Output"),
            Values("%f32vec2"), Values("VUID-PointCoord-PointCoord-04312"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))));

INSTANTIATE_TEST_SUITE_P(
    PointCoordNotFloatVector,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointCoord"), Values("Fragment"), Values("Input"),
            Values("%f32arr2", "%u32vec2"),
            Values("VUID-PointCoord-PointCoord-04313"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float vector",
                              "is not a float vector"))));

INSTANTIATE_TEST_SUITE_P(
    PointCoordNotFloatVec3,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointCoord"), Values("Fragment"), Values("Input"),
            Values("%f32vec3"), Values("VUID-PointCoord-PointCoord-04313"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float vector",
                              "has 3 components"))));

INSTANTIATE_TEST_SUITE_P(
    PointCoordNotF32Vec4,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointCoord"), Values("Fragment"), Values("Input"),
            Values("%f64vec2"), Values("VUID-PointCoord-PointCoord-04313"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float vector",
                              "has components with bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    PointSizeOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointSize"),
            Values("Vertex", "Geometry", "TessellationControl",
                   "TessellationEvaluation"),
            Values("Output"), Values("%f32"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    PointSizeInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointSize"),
            Values("Geometry", "TessellationControl", "TessellationEvaluation"),
            Values("Input"), Values("%f32"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    PointSizeVertexInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointSize"), Values("Vertex"), Values("Input"),
            Values("%f32"), Values("VUID-PointSize-PointSize-04315"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec doesn't allow BuiltIn PointSize "
                "to be used for variables with Input storage class if "
                "execution model is Vertex.",
                "which is called with execution model Vertex."))));

INSTANTIATE_TEST_SUITE_P(
    PointSizeInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointSize"), Values("GLCompute", "Fragment"),
            Values("Input", "Output"), Values("%f32"),
            Values("VUID-PointSize-PointSize-04314"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be used only with Vertex, TessellationControl, "
                "TessellationEvaluation or Geometry execution models"))));

INSTANTIATE_TEST_SUITE_P(
    PointSizeNotFloatScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointSize"), Values("Vertex"), Values("Output"),
            Values("%f32vec4", "%u32"),
            Values("VUID-PointSize-PointSize-04317"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float scalar",
                              "is not a float scalar"))));

INSTANTIATE_TEST_SUITE_P(
    PointSizeNotF32, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PointSize"), Values("Vertex"), Values("Output"),
            Values("%f64"), Values("VUID-PointSize-PointSize-04317"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float scalar",
                              "has bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    PositionOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Position"),
            Values("Vertex", "Geometry", "TessellationControl",
                   "TessellationEvaluation"),
            Values("Output"), Values("%f32vec4"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    PositionInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Position"),
            Values("Geometry", "TessellationControl", "TessellationEvaluation"),
            Values("Input"), Values("%f32vec4"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    PositionInvalidStorageClass,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Position"),
            Values("Geometry", "TessellationControl", "TessellationEvaluation"),
            Values("Private"), Values("%f32vec4"),
            Values("VUID-Position-Position-04320"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec allows BuiltIn Position to be only used for "
                "variables with Input or Output storage class."))));

INSTANTIATE_TEST_SUITE_P(
    PositionVertexInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Position"), Values("Vertex"), Values("Input"),
            Values("%f32vec4"), Values("VUID-Position-Position-04319"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec doesn't allow BuiltIn Position "
                "to be used for variables with Input storage class if "
                "execution model is Vertex.",
                "which is called with execution model Vertex."))));

INSTANTIATE_TEST_SUITE_P(
    PositionInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Position"), Values("GLCompute", "Fragment"),
            Values("Input", "Output"), Values("%f32vec4"),
            Values("VUID-Position-Position-04318"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be used only with Vertex, TessellationControl, "
                "TessellationEvaluation or Geometry execution models"))));

INSTANTIATE_TEST_SUITE_P(
    PositionNotFloatVector,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Position"), Values("Geometry"), Values("Input"),
            Values("%f32arr4", "%u32vec4"),
            Values("VUID-Position-Position-04321"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float vector",
                              "is not a float vector"))));

INSTANTIATE_TEST_SUITE_P(
    PositionNotFloatVec4,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Position"), Values("Geometry"), Values("Input"),
            Values("%f32vec3"), Values("VUID-Position-Position-04321"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float vector",
                              "has 3 components"))));

INSTANTIATE_TEST_SUITE_P(
    PositionNotF32Vec4,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("Position"), Values("Geometry"), Values("Input"),
            Values("%f64vec4"), Values("VUID-Position-Position-04321"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float vector",
                              "has components with bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    PrimitiveIdInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PrimitiveId"),
            Values("Fragment", "TessellationControl", "TessellationEvaluation",
                   "Geometry"),
            Values("Input"), Values("%u32"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    PrimitiveIdOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PrimitiveId"), Values("Geometry"), Values("Output"),
            Values("%u32"), Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    PrimitiveIdInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PrimitiveId"), Values("Vertex", "GLCompute"),
            Values("Input"), Values("%u32"),
            Values("VUID-PrimitiveId-PrimitiveId-04330"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be used only with Fragment, TessellationControl, "
                "TessellationEvaluation, Geometry, MeshNV, IntersectionKHR, "
                "AnyHitKHR, and ClosestHitKHR execution models"))));

INSTANTIATE_TEST_SUITE_P(
    PrimitiveIdFragmentNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("PrimitiveId"), Values("Fragment"), Values("Output"),
        Values("%u32"), Values("VUID-PrimitiveId-PrimitiveId-04334"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "Output storage class if execution model is Fragment",
                          "which is called with execution model Fragment"))));

INSTANTIATE_TEST_SUITE_P(
    PrimitiveIdTessellationNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PrimitiveId"),
            Values("TessellationControl", "TessellationEvaluation"),
            Values("Output"), Values("%u32"),
            Values("VUID-PrimitiveId-PrimitiveId-04334"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Output storage class if execution model is Tessellation",
                "which is called with execution model Tessellation"))));

INSTANTIATE_TEST_SUITE_P(
    PrimitiveIdNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PrimitiveId"), Values("Fragment"), Values("Input"),
            Values("%f32", "%u32vec3"),
            Values("VUID-PrimitiveId-PrimitiveId-04337"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))));

INSTANTIATE_TEST_SUITE_P(
    PrimitiveIdNotInt32,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("PrimitiveId"), Values("Fragment"), Values("Input"),
            Values("%u64"), Values("VUID-PrimitiveId-PrimitiveId-04337"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "has bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    SampleIdSuccess, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SampleId"), Values("Fragment"), Values("Input"),
            Values("%u32"), Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    SampleIdInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("SampleId"),
        Values("Vertex", "GLCompute", "Geometry", "TessellationControl",
               "TessellationEvaluation"),
        Values("Input"), Values("%u32"), Values("VUID-SampleId-SampleId-04354"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "to be used only with Fragment execution model"))));

INSTANTIATE_TEST_SUITE_P(
    SampleIdNotInput, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("SampleId"), Values("Fragment"), Values("Output"),
        Values("%u32"), Values("VUID-SampleId-SampleId-04355"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "Vulkan spec allows BuiltIn SampleId to be only used "
                          "for variables with Input storage class"))));

INSTANTIATE_TEST_SUITE_P(
    SampleIdNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SampleId"), Values("Fragment"), Values("Input"),
            Values("%f32", "%u32vec3"), Values("VUID-SampleId-SampleId-04356"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))));

INSTANTIATE_TEST_SUITE_P(
    SampleIdNotInt32, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SampleId"), Values("Fragment"), Values("Input"),
            Values("%u64"), Values("VUID-SampleId-SampleId-04356"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "has bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    SampleMaskSuccess, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SampleMask"), Values("Fragment"), Values("Input", "Output"),
            Values("%u32arr2", "%u32arr4"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    SampleMaskInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("SampleMask"),
        Values("Vertex", "GLCompute", "Geometry", "TessellationControl",
               "TessellationEvaluation"),
        Values("Input"), Values("%u32arr2"),
        Values("VUID-SampleMask-SampleMask-04357"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "to be used only with Fragment execution model"))));

INSTANTIATE_TEST_SUITE_P(
    SampleMaskWrongStorageClass,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SampleMask"), Values("Fragment"), Values("Workgroup"),
            Values("%u32arr2"), Values("VUID-SampleMask-SampleMask-04358"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec allows BuiltIn SampleMask to be only used for "
                "variables with Input or Output storage class"))));

INSTANTIATE_TEST_SUITE_P(
    SampleMaskNotArray,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SampleMask"), Values("Fragment"), Values("Input"),
            Values("%f32", "%u32vec3"),
            Values("VUID-SampleMask-SampleMask-04359"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int array",
                              "is not an array"))));

INSTANTIATE_TEST_SUITE_P(
    SampleMaskNotIntArray,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SampleMask"), Values("Fragment"), Values("Input"),
            Values("%f32arr2"), Values("VUID-SampleMask-SampleMask-04359"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int array",
                              "components are not int scalar"))));

INSTANTIATE_TEST_SUITE_P(
    SampleMaskNotInt32Array,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SampleMask"), Values("Fragment"), Values("Input"),
            Values("%u64arr2"), Values("VUID-SampleMask-SampleMask-04359"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int array",
                              "has components with bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    SamplePositionSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SamplePosition"), Values("Fragment"), Values("Input"),
            Values("%f32vec2"), Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    SamplePositionNotFragment,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("SamplePosition"),
        Values("Vertex", "GLCompute", "Geometry", "TessellationControl",
               "TessellationEvaluation"),
        Values("Input"), Values("%f32vec2"),
        Values("VUID-SamplePosition-SamplePosition-04360"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "to be used only with Fragment execution model"))));

INSTANTIATE_TEST_SUITE_P(
    SamplePositionNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SamplePosition"), Values("Fragment"), Values("Output"),
            Values("%f32vec2"),
            Values("VUID-SamplePosition-SamplePosition-04361"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))));

INSTANTIATE_TEST_SUITE_P(
    SamplePositionNotFloatVector,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SamplePosition"), Values("Fragment"), Values("Input"),
            Values("%f32arr2", "%u32vec4"),
            Values("VUID-SamplePosition-SamplePosition-04362"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float vector",
                              "is not a float vector"))));

INSTANTIATE_TEST_SUITE_P(
    SamplePositionNotFloatVec2,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SamplePosition"), Values("Fragment"), Values("Input"),
            Values("%f32vec3"),
            Values("VUID-SamplePosition-SamplePosition-04362"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float vector",
                              "has 3 components"))));

INSTANTIATE_TEST_SUITE_P(
    SamplePositionNotF32Vec2,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("SamplePosition"), Values("Fragment"), Values("Input"),
            Values("%f64vec2"),
            Values("VUID-SamplePosition-SamplePosition-04362"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float vector",
                              "has components with bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    TessCoordSuccess, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessCoord"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f32vec3"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    TessCoordNotFragment,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("TessCoord"),
        Values("Vertex", "GLCompute", "Geometry", "TessellationControl",
               "Fragment"),
        Values("Input"), Values("%f32vec3"),
        Values("VUID-TessCoord-TessCoord-04387"),
        Values(TestResult(
            SPV_ERROR_INVALID_DATA,
            "to be used only with TessellationEvaluation execution model"))));

INSTANTIATE_TEST_SUITE_P(
    TessCoordNotInput, ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessCoord"), Values("Fragment"), Values("Output"),
            Values("%f32vec3"), Values("VUID-TessCoord-TessCoord-04388"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))));

INSTANTIATE_TEST_SUITE_P(
    TessCoordNotFloatVector,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessCoord"), Values("Fragment"), Values("Input"),
            Values("%f32arr3", "%u32vec4"),
            Values("VUID-TessCoord-TessCoord-04389"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 3-component 32-bit float vector",
                              "is not a float vector"))));

INSTANTIATE_TEST_SUITE_P(
    TessCoordNotFloatVec3,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessCoord"), Values("Fragment"), Values("Input"),
            Values("%f32vec2"), Values("VUID-TessCoord-TessCoord-04389"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 3-component 32-bit float vector",
                              "has 2 components"))));

INSTANTIATE_TEST_SUITE_P(
    TessCoordNotF32Vec3,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessCoord"), Values("Fragment"), Values("Input"),
            Values("%f64vec3"), Values("VUID-TessCoord-TessCoord-04389"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 3-component 32-bit float vector",
                              "has components with bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    TessLevelOuterTeseInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f32arr4"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    TessLevelOuterTescOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"), Values("TessellationControl"),
            Values("Output"), Values("%f32arr4"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    TessLevelOuterInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"),
            Values("Vertex", "GLCompute", "Geometry", "Fragment"),
            Values("Input"), Values("%f32arr4"),
            Values("VUID-TessLevelOuter-TessLevelOuter-04390"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "to be used only with TessellationControl or "
                              "TessellationEvaluation execution models."))));

INSTANTIATE_TEST_SUITE_P(
    TessLevelOuterOutputTese,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"), Values("TessellationEvaluation"),
            Values("Output"), Values("%f32arr4"),
            Values("VUID-TessLevelOuter-TessLevelOuter-04392"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec doesn't allow TessLevelOuter/TessLevelInner to be "
                "used for variables with Output storage class if execution "
                "model is TessellationEvaluation."))));

INSTANTIATE_TEST_SUITE_P(
    TessLevelOuterInputTesc,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"), Values("TessellationControl"),
            Values("Input"), Values("%f32arr4"),
            Values("VUID-TessLevelOuter-TessLevelOuter-04391"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec doesn't allow TessLevelOuter/TessLevelInner to be "
                "used for variables with Input storage class if execution "
                "model is TessellationControl."))));

INSTANTIATE_TEST_SUITE_P(
    TessLevelOuterNotArray,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f32vec4", "%f32"),
            Values("VUID-TessLevelOuter-TessLevelOuter-04393"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float array",
                              "is not an array"))));

INSTANTIATE_TEST_SUITE_P(
    TessLevelOuterNotFloatArray,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"), Values("TessellationEvaluation"),
            Values("Input"), Values("%u32arr4"),
            Values("VUID-TessLevelOuter-TessLevelOuter-04393"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float array",
                              "components are not float scalar"))));

INSTANTIATE_TEST_SUITE_P(
    TessLevelOuterNotFloatArr4,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f32arr3"),
            Values("VUID-TessLevelOuter-TessLevelOuter-04393"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float array",
                              "has 3 components"))));

INSTANTIATE_TEST_SUITE_P(
    TessLevelOuterNotF32Arr4,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelOuter"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f64arr4"),
            Values("VUID-TessLevelOuter-TessLevelOuter-04393"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float array",
                              "has components with bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    TessLevelInnerTeseInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f32arr2"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    TessLevelInnerTescOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"), Values("TessellationControl"),
            Values("Output"), Values("%f32arr2"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    TessLevelInnerInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"),
            Values("Vertex", "GLCompute", "Geometry", "Fragment"),
            Values("Input"), Values("%f32arr2"),
            Values("VUID-TessLevelInner-TessLevelInner-04394"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "to be used only with TessellationControl or "
                              "TessellationEvaluation execution models."))));

INSTANTIATE_TEST_SUITE_P(
    TessLevelInnerOutputTese,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"), Values("TessellationEvaluation"),
            Values("Output"), Values("%f32arr2"),
            Values("VUID-TessLevelInner-TessLevelInner-04396"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec doesn't allow TessLevelOuter/TessLevelInner to be "
                "used for variables with Output storage class if execution "
                "model is TessellationEvaluation."))));

INSTANTIATE_TEST_SUITE_P(
    TessLevelInnerInputTesc,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"), Values("TessellationControl"),
            Values("Input"), Values("%f32arr2"),
            Values("VUID-TessLevelInner-TessLevelInner-04395"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec doesn't allow TessLevelOuter/TessLevelInner to be "
                "used for variables with Input storage class if execution "
                "model is TessellationControl."))));

INSTANTIATE_TEST_SUITE_P(
    TessLevelInnerNotArray,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f32vec2", "%f32"),
            Values("VUID-TessLevelInner-TessLevelInner-04397"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float array",
                              "is not an array"))));

INSTANTIATE_TEST_SUITE_P(
    TessLevelInnerNotFloatArray,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"), Values("TessellationEvaluation"),
            Values("Input"), Values("%u32arr2"),
            Values("VUID-TessLevelInner-TessLevelInner-04397"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float array",
                              "components are not float scalar"))));

INSTANTIATE_TEST_SUITE_P(
    TessLevelInnerNotFloatArr2,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f32arr3"),
            Values("VUID-TessLevelInner-TessLevelInner-04397"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float array",
                              "has 3 components"))));

INSTANTIATE_TEST_SUITE_P(
    TessLevelInnerNotF32Arr2,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("TessLevelInner"), Values("TessellationEvaluation"),
            Values("Input"), Values("%f64arr2"),
            Values("VUID-TessLevelInner-TessLevelInner-04397"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 2-component 32-bit float array",
                              "has components with bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    VertexIndexSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("VertexIndex"), Values("Vertex"), Values("Input"),
            Values("%u32"), Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    VertexIndexInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("VertexIndex"),
            Values("Fragment", "GLCompute", "Geometry", "TessellationControl",
                   "TessellationEvaluation"),
            Values("Input"), Values("%u32"),
            Values("VUID-VertexIndex-VertexIndex-04398"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "to be used only with Vertex execution model"))));

INSTANTIATE_TEST_SUITE_P(
    VertexIndexNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(
        Values("VertexIndex"), Values("Vertex"), Values("Output"),
        Values("%u32"), Values("VUID-VertexIndex-VertexIndex-04399"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "Vulkan spec allows BuiltIn VertexIndex to be only "
                          "used for variables with Input storage class"))));

INSTANTIATE_TEST_SUITE_P(
    VertexIndexNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("VertexIndex"), Values("Vertex"), Values("Input"),
            Values("%f32", "%u32vec3"),
            Values("VUID-VertexIndex-VertexIndex-04400"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))));

INSTANTIATE_TEST_SUITE_P(
    VertexIndexNotInt32,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeResult,
    Combine(Values("VertexIndex"), Values("Vertex"), Values("Input"),
            Values("%u64"), Values("VUID-VertexIndex-VertexIndex-04400"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "has bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    BaseInstanceOrVertexSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("BaseInstance", "BaseVertex"), Values("Vertex"),
            Values("Input"), Values("%u32"),
            Values("OpCapability DrawParameters\n"),
            Values("OpExtension \"SPV_KHR_shader_draw_parameters\"\n"),
            Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    BaseInstanceOrVertexInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("BaseInstance", "BaseVertex"),
            Values("Fragment", "GLCompute", "Geometry", "TessellationControl",
                   "TessellationEvaluation"),
            Values("Input"), Values("%u32"),
            Values("OpCapability DrawParameters\n"),
            Values("OpExtension \"SPV_KHR_shader_draw_parameters\"\n"),
            Values("VUID-BaseInstance-BaseInstance-04181 "
                   "VUID-BaseVertex-BaseVertex-04184"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "to be used only with Vertex execution model"))));

INSTANTIATE_TEST_SUITE_P(
    BaseInstanceOrVertexNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("BaseInstance", "BaseVertex"), Values("Vertex"),
            Values("Output"), Values("%u32"),
            Values("OpCapability DrawParameters\n"),
            Values("OpExtension \"SPV_KHR_shader_draw_parameters\"\n"),
            Values("VUID-BaseInstance-BaseInstance-04182 "
                   "VUID-BaseVertex-BaseVertex-04185"),
            Values(TestResult(SPV_ERROR_INVALID_DATA, "Vulkan spec allows",
                              "used for variables with Input storage class"))));

INSTANTIATE_TEST_SUITE_P(
    BaseInstanceOrVertexNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("BaseInstance", "BaseVertex"), Values("Vertex"),
            Values("Input"), Values("%f32", "%u32vec3"),
            Values("OpCapability DrawParameters\n"),
            Values("OpExtension \"SPV_KHR_shader_draw_parameters\"\n"),
            Values("VUID-BaseInstance-BaseInstance-04183 "
                   "VUID-BaseVertex-BaseVertex-04186"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))));

INSTANTIATE_TEST_SUITE_P(
    DrawIndexSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("DrawIndex"), Values("Vertex"), Values("Input"),
            Values("%u32"), Values("OpCapability DrawParameters\n"),
            Values("OpExtension \"SPV_KHR_shader_draw_parameters\"\n"),
            Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    DrawIndexMeshSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(
        Values("DrawIndex"), Values("MeshNV", "TaskNV"), Values("Input"),
        Values("%u32"), Values("OpCapability MeshShadingNV\n"),
        Values("OpExtension \"SPV_KHR_shader_draw_parameters\"\nOpExtension "
               "\"SPV_NV_mesh_shader\"\n"),
        Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    DrawIndexInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("DrawIndex"),
            Values("Fragment", "GLCompute", "Geometry", "TessellationControl",
                   "TessellationEvaluation"),
            Values("Input"), Values("%u32"),
            Values("OpCapability DrawParameters\n"),
            Values("OpExtension \"SPV_KHR_shader_draw_parameters\"\n"),
            Values("VUID-DrawIndex-DrawIndex-04207"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "to be used only with Vertex, MeshNV, or TaskNV "
                              "execution model"))));

INSTANTIATE_TEST_SUITE_P(
    DrawIndexNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("DrawIndex"), Values("Vertex"), Values("Output"),
            Values("%u32"), Values("OpCapability DrawParameters\n"),
            Values("OpExtension \"SPV_KHR_shader_draw_parameters\"\n"),
            Values("VUID-DrawIndex-DrawIndex-04208"),
            Values(TestResult(SPV_ERROR_INVALID_DATA, "Vulkan spec allows",
                              "used for variables with Input storage class"))));

INSTANTIATE_TEST_SUITE_P(
    DrawIndexNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("DrawIndex"), Values("Vertex"), Values("Input"),
            Values("%f32", "%u32vec3"), Values("OpCapability DrawParameters\n"),
            Values("OpExtension \"SPV_KHR_shader_draw_parameters\"\n"),
            Values("VUID-DrawIndex-DrawIndex-04209"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))));

INSTANTIATE_TEST_SUITE_P(
    ViewIndexSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("ViewIndex"),
            Values("Fragment", "Vertex", "Geometry", "TessellationControl",
                   "TessellationEvaluation"),
            Values("Input"), Values("%u32"), Values("OpCapability MultiView\n"),
            Values("OpExtension \"SPV_KHR_multiview\"\n"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    ViewIndexInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("ViewIndex"), Values("GLCompute"), Values("Input"),
            Values("%u32"), Values("OpCapability MultiView\n"),
            Values("OpExtension \"SPV_KHR_multiview\"\n"),
            Values("VUID-ViewIndex-ViewIndex-04401"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be not be used with GLCompute execution model"))));

INSTANTIATE_TEST_SUITE_P(
    ViewIndexNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("ViewIndex"), Values("Vertex"), Values("Output"),
            Values("%u32"), Values("OpCapability MultiView\n"),
            Values("OpExtension \"SPV_KHR_multiview\"\n"),
            Values("VUID-ViewIndex-ViewIndex-04402"),
            Values(TestResult(SPV_ERROR_INVALID_DATA, "Vulkan spec allows",
                              "used for variables with Input storage class"))));

INSTANTIATE_TEST_SUITE_P(
    ViewIndexNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("ViewIndex"), Values("Vertex"), Values("Input"),
            Values("%f32", "%u32vec3"), Values("OpCapability MultiView\n"),
            Values("OpExtension \"SPV_KHR_multiview\"\n"),
            Values("VUID-ViewIndex-ViewIndex-04403"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))));

INSTANTIATE_TEST_SUITE_P(
    DeviceIndexSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("DeviceIndex"),
            Values("Fragment", "Vertex", "Geometry", "TessellationControl",
                   "TessellationEvaluation", "GLCompute"),
            Values("Input"), Values("%u32"),
            Values("OpCapability DeviceGroup\n"),
            Values("OpExtension \"SPV_KHR_device_group\"\n"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    DeviceIndexNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("DeviceIndex"), Values("Fragment", "Vertex", "GLCompute"),
            Values("Output"), Values("%u32"),
            Values("OpCapability DeviceGroup\n"),
            Values("OpExtension \"SPV_KHR_device_group\"\n"),
            Values("VUID-DeviceIndex-DeviceIndex-04205"),
            Values(TestResult(SPV_ERROR_INVALID_DATA, "Vulkan spec allows",
                              "used for variables with Input storage class"))));

INSTANTIATE_TEST_SUITE_P(
    DeviceIndexNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("DeviceIndex"), Values("Fragment", "Vertex", "GLCompute"),
            Values("Input"), Values("%f32", "%u32vec3"),
            Values("OpCapability DeviceGroup\n"),
            Values("OpExtension \"SPV_KHR_device_group\"\n"),
            Values("VUID-DeviceIndex-DeviceIndex-04206"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))));

// Test HitKind in NV RT shaders
INSTANTIATE_TEST_SUITE_P(
    HitKindNVSuccess,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("HitKindNV"),
            Values("AnyHitNV", "ClosestHitNV"), Values("Input"), Values("%u32"),
            Values("OpCapability RayTracingNV\n"),
            Values("OpExtension \"SPV_NV_ray_tracing\"\n"), Values(nullptr),
            Values(TestResult())));

// HitKind is valid in AH, CH shaders as input i32 scalar
INSTANTIATE_TEST_SUITE_P(
    HitKindSuccess,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("HitKindKHR"),
            Values("AnyHitKHR", "ClosestHitKHR"), Values("Input"),
            Values("%u32"), Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    HitKindNotExecutionMode,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("HitKindKHR"),
            Values("Vertex", "Fragment", "TessellationControl",
                   "TessellationEvaluation", "Geometry", "Fragment",
                   "GLCompute", "RayGenerationKHR", "IntersectionKHR",
                   "MissKHR", "CallableKHR"),
            Values("Input"), Values("%u32"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-HitKindKHR-HitKindKHR-04242"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "Vulkan spec does not allow BuiltIn",
                              "to be used with the execution model"))));

INSTANTIATE_TEST_SUITE_P(
    HitKindNotInput,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("HitKindKHR"),
            Values("AnyHitKHR", "ClosestHitKHR"), Values("Output"),
            Values("%u32"), Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-HitKindKHR-HitKindKHR-04243"),
            Values(TestResult(SPV_ERROR_INVALID_DATA, "Vulkan spec allows",
                              "used for variables with Input storage class"))));

INSTANTIATE_TEST_SUITE_P(
    HitKindNotIntScalar,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("HitKindKHR"),
            Values("AnyHitKHR", "ClosestHitKHR"), Values("Input"),
            Values("%f32", "%u32vec3"), Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-HitKindKHR-HitKindKHR-04244"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))));

// Ensure HitT is not supported in KHR RT shaders
INSTANTIATE_TEST_SUITE_P(
    HitTNVNotSupportedInKHR,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("HitTNV"),
            Values("AnyHitKHR", "ClosestHitKHR"), Values("Input"),
            Values("%u32"), Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"), Values(nullptr),
            Values(TestResult(
                SPV_ERROR_INVALID_CAPABILITY,
                "of MemberDecorate requires one of these capabilities"))));

// HitT is valid in AH, CH shaders as input f32 scalar (NV RT only)
INSTANTIATE_TEST_SUITE_P(
    HitTNVSuccess,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("HitTNV"),
            Values("AnyHitNV", "ClosestHitNV"), Values("Input"), Values("%f32"),
            Values("OpCapability RayTracingNV\n"),
            Values("OpExtension \"SPV_NV_ray_tracing\"\n"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    HitTNVNotExecutionMode,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("HitTNV"),
            Values("Vertex", "Fragment", "TessellationControl",
                   "TessellationEvaluation", "Geometry", "Fragment",
                   "GLCompute", "RayGenerationNV", "IntersectionNV", "MissNV",
                   "CallableNV"),
            Values("Input"), Values("%f32"),
            Values("OpCapability RayTracingNV\n"),
            Values("OpExtension \"SPV_NV_ray_tracing\"\n"),
            Values("VUID-HitTNV-HitTNV-04245"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "Vulkan spec does not allow BuiltIn",
                              "to be used with the execution model"))));

INSTANTIATE_TEST_SUITE_P(
    HitTNVNotInput,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("HitTNV"),
            Values("AnyHitNV", "ClosestHitNV"), Values("Output"),
            Values("%f32"), Values("OpCapability RayTracingNV\n"),
            Values("OpExtension \"SPV_NV_ray_tracing\"\n"),
            Values("VUID-HitTNV-HitTNV-04246"),
            Values(TestResult(SPV_ERROR_INVALID_DATA, "Vulkan spec allows",
                              "used for variables with Input storage class"))));
INSTANTIATE_TEST_SUITE_P(
    HitTNVNotIntScalar,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("HitTNV"),
            Values("AnyHitNV", "ClosestHitNV"), Values("Input"),
            Values("%u32", "%f32vec3"), Values("OpCapability RayTracingNV\n"),
            Values("OpExtension \"SPV_NV_ray_tracing\"\n"),
            Values("VUID-HitTNV-HitTNV-04247"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float scalar",
                              "is not a float scalar"))));

// InstanceCustomIndexKHR, InstanceId, PrimitiveId, RayGeometryIndexKHR are
// valid in IS, AH, CH shaders as input i32 scalars
INSTANTIATE_TEST_SUITE_P(
    RTBuiltIn3StageI32Success,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2),
            Values("InstanceCustomIndexKHR", "RayGeometryIndexKHR",
                   "InstanceId", "PrimitiveId"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR"),
            Values("Input"), Values("%u32"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    RTBuiltIn3StageI32NotExecutionMode,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2),
            Values("InstanceCustomIndexKHR", "RayGeometryIndexKHR",
                   "InstanceId"),
            Values("Vertex", "Fragment", "TessellationControl",
                   "TessellationEvaluation", "Geometry", "Fragment",
                   "GLCompute", "RayGenerationKHR", "MissKHR", "CallableKHR"),
            Values("Input"), Values("%u32"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-InstanceCustomIndexKHR-InstanceCustomIndexKHR-04251 "
                   "VUID-RayGeometryIndexKHR-RayGeometryIndexKHR-04345 "
                   "VUID-InstanceId-InstanceId-04254 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "Vulkan spec does not allow BuiltIn",
                              "to be used with the execution model"))));

INSTANTIATE_TEST_SUITE_P(
    RTBuiltIn3StageI32NotInput,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2),
            Values("InstanceCustomIndexKHR", "RayGeometryIndexKHR",
                   "InstanceId"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR"),
            Values("Output"), Values("%u32"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-InstanceCustomIndexKHR-InstanceCustomIndexKHR-04252 "
                   "VUID-RayGeometryIndexKHR-RayGeometryIndexKHR-04346 "
                   "VUID-InstanceId-InstanceId-04255 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA, "Vulkan spec allows",
                              "used for variables with Input storage class"))));

INSTANTIATE_TEST_SUITE_P(
    RTBuiltIn3StageI32NotIntScalar,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2),
            Values("InstanceCustomIndexKHR", "RayGeometryIndexKHR",
                   "InstanceId"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR"),
            Values("Input"), Values("%f32", "%u32vec3"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-InstanceCustomIndexKHR-InstanceCustomIndexKHR-04253 "
                   "VUID-RayGeometryIndexKHR-RayGeometryIndexKHR-04347 "
                   "VUID-InstanceId-InstanceId-04256 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))));

// PrimitiveId needs special negative testing because it has non-RT uses
INSTANTIATE_TEST_SUITE_P(
    PrimitiveIdRTNotExecutionMode,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("PrimitiveId"),
            Values("RayGenerationKHR", "MissKHR", "CallableKHR"),
            Values("Input"), Values("%u32"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-PrimitiveId-PrimitiveId-04330"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be used only with Fragment, TessellationControl, "
                "TessellationEvaluation, Geometry, MeshNV, IntersectionKHR, "
                "AnyHitKHR, and ClosestHitKHR execution models"))));

INSTANTIATE_TEST_SUITE_P(
    PrimitiveIdRTNotInput,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("PrimitiveId"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR"),
            Values("Output"), Values("%u32"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-PrimitiveId-PrimitiveId-04334"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "Output storage class if execution model is "))));

INSTANTIATE_TEST_SUITE_P(
    PrimitiveIdRTNotIntScalar,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("PrimitiveId"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR"),
            Values("Input"), Values("%f32", "%u32vec3"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-PrimitiveId-PrimitiveId-04337"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))));

// ObjectRayDirectionKHR and ObjectRayOriginKHR valid
// in IS, AH, CH shaders as input 32-bit float vec3
INSTANTIATE_TEST_SUITE_P(
    ObjectRayDirectionAndOriginSuccess,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2),
            Values("ObjectRayDirectionKHR", "ObjectRayOriginKHR"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR"),
            Values("Input"), Values("%f32vec3"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    ObjectRayDirectionAndOriginNotExecutionMode,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2),
            Values("ObjectRayDirectionKHR", "ObjectRayOriginKHR"),
            Values("Vertex", "Fragment", "TessellationControl",
                   "TessellationEvaluation", "Geometry", "Fragment",
                   "GLCompute", "RayGenerationKHR", "MissKHR", "CallableKHR"),
            Values("Input"), Values("%f32vec3"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-ObjectRayDirectionKHR-ObjectRayDirectionKHR-04299 "
                   "VUID-ObjectRayOriginKHR-ObjectRayOriginKHR-04302 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "Vulkan spec does not allow BuiltIn",
                              "to be used with the execution model"))));

INSTANTIATE_TEST_SUITE_P(
    ObjectRayDirectionAndOriginNotInput,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2),
            Values("ObjectRayDirectionKHR", "ObjectRayOriginKHR"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR"),
            Values("Output"), Values("%f32vec3"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-ObjectRayDirectionKHR-ObjectRayDirectionKHR-04300 "
                   "VUID-ObjectRayOriginKHR-ObjectRayOriginKHR-04303 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA, "Vulkan spec allows",
                              "used for variables with Input storage class"))));

INSTANTIATE_TEST_SUITE_P(
    ObjectRayDirectionAndOriginNotFloatVec3,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(
        Values(SPV_ENV_VULKAN_1_2),
        Values("ObjectRayDirectionKHR", "ObjectRayOriginKHR"),
        Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR"),
        Values("Input"), Values("%u32vec3", "%f32", "%f32vec2", "%f32vec4"),
        Values("OpCapability RayTracingKHR\n"),
        Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
        Values("VUID-ObjectRayDirectionKHR-ObjectRayDirectionKHR-04301 "
               "VUID-ObjectRayOriginKHR-ObjectRayOriginKHR-04304 "),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "needs to be a 3-component 32-bit float vector"))));

// ObjectToWorldKHR and WorldToObjectKHR valid
// in IS, AH, CH shaders as input mat4x3
INSTANTIATE_TEST_SUITE_P(
    RTObjectMatrixSuccess,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2),
            Values("ObjectToWorldKHR", "WorldToObjectKHR"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR"),
            Values("Input"), Values("%f32mat34"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    RTObjectMatrixNotExecutionMode,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2),
            Values("ObjectToWorldKHR", "WorldToObjectKHR"),
            Values("Vertex", "Fragment", "TessellationControl",
                   "TessellationEvaluation", "Geometry", "Fragment",
                   "GLCompute", "RayGenerationKHR", "MissKHR", "CallableKHR"),
            Values("Input"), Values("%f32mat34"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-ObjectToWorldKHR-ObjectToWorldKHR-04305 "
                   "VUID-WorldToObjectKHR-WorldToObjectKHR-04434 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "Vulkan spec does not allow BuiltIn",
                              "to be used with the execution model"))));

INSTANTIATE_TEST_SUITE_P(
    RTObjectMatrixNotInput,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2),
            Values("ObjectToWorldKHR", "WorldToObjectKHR"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR"),
            Values("Output"), Values("%f32mat34"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-ObjectToWorldKHR-ObjectToWorldKHR-04306 "
                   "VUID-WorldToObjectKHR-WorldToObjectKHR-04435 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA, "Vulkan spec allows",
                              "used for variables with Input storage class"))));

INSTANTIATE_TEST_SUITE_P(
    RTObjectMatrixNotMat4x3,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2),
            Values("ObjectToWorldKHR", "WorldToObjectKHR"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR"),
            Values("Input"), Values("%f32mat43", "%f32mat44", "%f32vec4"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-ObjectToWorldKHR-ObjectToWorldKHR-04307 "
                   "VUID-WorldToObjectKHR-WorldToObjectKHR-04436 "),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "variable needs to be a matrix with "
                "4 columns of 3-component vectors of 32-bit floats"))));

// IncomingRayFlagsKHR is valid
// in IS, AH, CH, MS shaders as an input i32 scalar
INSTANTIATE_TEST_SUITE_P(
    IncomingRayFlagsSuccess,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("IncomingRayFlagsKHR"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR", "MissKHR"),
            Values("Input"), Values("%u32"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    IncomingRayFlagsNotExecutionMode,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("IncomingRayFlagsKHR"),
            Values("Vertex", "Fragment", "TessellationControl",
                   "TessellationEvaluation", "Geometry", "Fragment",
                   "GLCompute", "RayGenerationKHR", "CallableKHR"),
            Values("Input"), Values("%u32"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-IncomingRayFlagsKHR-IncomingRayFlagsKHR-04248 "
                   "VUID-RayTmaxKHR-RayTmaxKHR-04348 "
                   "VUID-RayTminKHR-RayTminKHR-04351 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "Vulkan spec does not allow BuiltIn",
                              "to be used with the execution model"))));

INSTANTIATE_TEST_SUITE_P(
    IncomingRayFlagsNotInput,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("IncomingRayFlagsKHR"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR", "MissKHR"),
            Values("Output"), Values("%u32"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-IncomingRayFlagsKHR-IncomingRayFlagsKHR-04249 "
                   "VUID-RayTmaxKHR-RayTmaxKHR-04349 "
                   "VUID-RayTminKHR-RayTminKHR-04352 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA, "Vulkan spec allows",
                              "used for variables with Input storage class"))));
INSTANTIATE_TEST_SUITE_P(
    IncomingRayFlagsNotIntScalar,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("IncomingRayFlagsKHR"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR", "MissKHR"),
            Values("Input"), Values("%f32", "%u32vec3"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-IncomingRayFlagsKHR-IncomingRayFlagsKHR-04250 "
                   "VUID-RayTmaxKHR-RayTmaxKHR-04350 "
                   "VUID-RayTminKHR-RayTminKHR-04353 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))));

// CullMaskKHR is valid
// in IS, AH, CH, MS shaders as an input i32 scalar
INSTANTIATE_TEST_SUITE_P(
    CullMaskSuccess,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("CullMaskKHR"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR", "MissKHR"),
            Values("Input"), Values("%u32"),
            Values("OpCapability RayTracingKHR\nOpCapability RayCullMaskKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\nOpExtension "
                   "\"SPV_KHR_ray_cull_mask\"\n"),
            Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    CullMaskNotExecutionMode,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("CullMaskKHR"),
            Values("Vertex", "Fragment", "TessellationControl",
                   "TessellationEvaluation", "Geometry", "Fragment",
                   "GLCompute", "RayGenerationKHR", "CallableKHR"),
            Values("Input"), Values("%u32"),
            Values("OpCapability RayTracingKHR\nOpCapability RayCullMaskKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\nOpExtension "
                   "\"SPV_KHR_ray_cull_mask\"\n"),
            Values("VUID-CullMaskKHR-CullMaskKHR-06735 "
                   "VUID-RayTmaxKHR-RayTmaxKHR-04348 "
                   "VUID-RayTminKHR-RayTminKHR-04351 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "Vulkan spec does not allow BuiltIn",
                              "to be used with the execution model"))));

INSTANTIATE_TEST_SUITE_P(
    ICullMaskNotInput,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("CullMaskKHR"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR", "MissKHR"),
            Values("Output"), Values("%u32"),
            Values("OpCapability RayTracingKHR\nOpCapability RayCullMaskKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\nOpExtension "
                   "\"SPV_KHR_ray_cull_mask\"\n"),
            Values("VUID-CullMaskKHR-CullMaskKHR-06736 "
                   "VUID-RayTmaxKHR-RayTmaxKHR-04349 "
                   "VUID-RayTminKHR-RayTminKHR-04352 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA, "Vulkan spec allows",
                              "used for variables with Input storage class"))));
INSTANTIATE_TEST_SUITE_P(
    CullMaskNotIntScalar,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("CullMaskKHR"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR", "MissKHR"),
            Values("Input"), Values("%f32", "%u32vec3"),
            Values("OpCapability RayTracingKHR\nOpCapability RayCullMaskKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\nOpExtension "
                   "\"SPV_KHR_ray_cull_mask\"\n"),
            Values("VUID-CullMaskKHR-CullMaskKHR-06737 "
                   "VUID-RayTmaxKHR-RayTmaxKHR-04350 "
                   "VUID-RayTminKHR-RayTminKHR-04353 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))));

// RayTmaxKHR, RayTminKHR are all valid
// in IS, AH, CH, MS shaders as input f32 scalars
INSTANTIATE_TEST_SUITE_P(
    RayTSuccess,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("RayTmaxKHR", "RayTminKHR"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR", "MissKHR"),
            Values("Input"), Values("%f32"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    RayTNotExecutionMode,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("RayTmaxKHR", "RayTminKHR"),
            Values("Vertex", "Fragment", "TessellationControl",
                   "TessellationEvaluation", "Geometry", "Fragment",
                   "GLCompute", "RayGenerationKHR", "CallableKHR"),
            Values("Input"), Values("%f32"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-IncomingRayFlagsKHR-IncomingRayFlagsKHR-04248 "
                   "VUID-RayTmaxKHR-RayTmaxKHR-04348 "
                   "VUID-RayTminKHR-RayTminKHR-04351 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "Vulkan spec does not allow BuiltIn",
                              "to be used with the execution model"))));

INSTANTIATE_TEST_SUITE_P(
    RayTNotInput,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("RayTmaxKHR", "RayTminKHR"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR", "MissKHR"),
            Values("Output"), Values("%f32"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-IncomingRayFlagsKHR-IncomingRayFlagsKHR-04249 "
                   "VUID-RayTmaxKHR-RayTmaxKHR-04349 "
                   "VUID-RayTminKHR-RayTminKHR-04352 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA, "Vulkan spec allows",
                              "used for variables with Input storage class"))));
INSTANTIATE_TEST_SUITE_P(
    RayTNotFloatScalar,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("RayTmaxKHR", "RayTminKHR"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR", "MissKHR"),
            Values("Input"), Values("%u32", "%f32vec3"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-IncomingRayFlagsKHR-IncomingRayFlagsKHR-04250 "
                   "VUID-RayTmaxKHR-RayTmaxKHR-04350 "
                   "VUID-RayTminKHR-RayTminKHR-04353 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float scalar",
                              "is not a float scalar"))));

// WorldRayDirectionKHR and WorldRayOriginKHR are valid
// in IS, AH, CH, MS shaders as input 32-bit float vec3
INSTANTIATE_TEST_SUITE_P(
    WorldRayDirectionAndOriginSuccess,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2),
            Values("WorldRayDirectionKHR", "WorldRayOriginKHR"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR", "MissKHR"),
            Values("Input"), Values("%f32vec3"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    WorldRayDirectionAndOriginNotExecutionMode,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2),
            Values("WorldRayDirectionKHR", "WorldRayOriginKHR"),
            Values("Vertex", "Fragment", "TessellationControl",
                   "TessellationEvaluation", "Geometry", "Fragment",
                   "GLCompute", "RayGenerationKHR", "CallableKHR"),
            Values("Input"), Values("%f32vec3"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-WorldRayDirectionKHR-WorldRayDirectionKHR-04428 "
                   "VUID-WorldRayOriginKHR-WorldRayOriginKHR-04431 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "Vulkan spec does not allow BuiltIn",
                              "to be used with the execution model"))));

INSTANTIATE_TEST_SUITE_P(
    WorldRayDirectionAndOriginNotInput,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2),
            Values("WorldRayDirectionKHR", "WorldRayOriginKHR"),
            Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR", "MissKHR"),
            Values("Output"), Values("%f32vec3"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-WorldRayDirectionKHR-WorldRayDirectionKHR-04429 "
                   "VUID-WorldRayOriginKHR-WorldRayOriginKHR-04432 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA, "Vulkan spec allows",
                              "used for variables with Input storage class"))));

INSTANTIATE_TEST_SUITE_P(
    WorldRayDirectionAndOriginNotFloatVec3,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(
        Values(SPV_ENV_VULKAN_1_2),
        Values("WorldRayDirectionKHR", "WorldRayOriginKHR"),
        Values("AnyHitKHR", "ClosestHitKHR", "IntersectionKHR", "MissKHR"),
        Values("Input"), Values("%u32vec3", "%f32", "%f32vec2", "%f32vec4"),
        Values("OpCapability RayTracingKHR\n"),
        Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
        Values("VUID-WorldRayDirectionKHR-WorldRayDirectionKHR-04430 "
               "VUID-WorldRayOriginKHR-WorldRayOriginKHR-04433 "),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "needs to be a 3-component 32-bit float vector"))));

// LaunchIdKHR and LaunchSizeKHR are valid
// in RG, IS, AH, CH, MS shaders as input 32-bit ivec3
INSTANTIATE_TEST_SUITE_P(
    LaunchRTSuccess,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("LaunchIdKHR", "LaunchSizeKHR"),
            Values("RayGenerationKHR", "AnyHitKHR", "ClosestHitKHR",
                   "IntersectionKHR", "MissKHR", "CallableKHR"),
            Values("Input"), Values("%u32vec3"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"), Values(nullptr),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    LaunchRTNotExecutionMode,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("LaunchIdKHR", "LaunchSizeKHR"),
            Values("Vertex", "Fragment", "TessellationControl",
                   "TessellationEvaluation", "Geometry", "Fragment",
                   "GLCompute"),
            Values("Input"), Values("%u32vec3"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-LaunchIdKHR-LaunchIdKHR-04266 "
                   "VUID-LaunchSizeKHR-LaunchSizeKHR-04269 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "Vulkan spec does not allow BuiltIn",
                              "to be used with the execution model"))));

INSTANTIATE_TEST_SUITE_P(
    LaunchRTNotInput,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("LaunchIdKHR", "LaunchSizeKHR"),
            Values("RayGenerationKHR", "AnyHitKHR", "ClosestHitKHR",
                   "IntersectionKHR", "MissKHR", "CallableKHR"),
            Values("Output"), Values("%u32vec3"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-LaunchIdKHR-LaunchIdKHR-04267 "
                   "VUID-LaunchSizeKHR-LaunchSizeKHR-04270 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA, "Vulkan spec allows",
                              "used for variables with Input storage class"))));

INSTANTIATE_TEST_SUITE_P(
    LaunchRTNotIntVec3,
    ValidateGenericCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values(SPV_ENV_VULKAN_1_2), Values("LaunchIdKHR", "LaunchSizeKHR"),
            Values("RayGenerationKHR", "AnyHitKHR", "ClosestHitKHR",
                   "IntersectionKHR", "MissKHR", "CallableKHR"),
            Values("Input"), Values("%f32vec3", "%u32", "%u32vec2", "%u32vec4"),
            Values("OpCapability RayTracingKHR\n"),
            Values("OpExtension \"SPV_KHR_ray_tracing\"\n"),
            Values("VUID-LaunchIdKHR-LaunchIdKHR-04268 "
                   "VUID-LaunchSizeKHR-LaunchSizeKHR-04271 "),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 3-component 32-bit int vector"))));

CodeGenerator GetArrayedVariableCodeGenerator(const char* const built_in,
                                              const char* const execution_model,
                                              const char* const storage_class,
                                              const char* const data_type) {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();

  generator.before_types_ = "OpDecorate %built_in_var BuiltIn ";
  generator.before_types_ += built_in;
  generator.before_types_ += "\n";

  std::ostringstream after_types;
  after_types << "%built_in_array = OpTypeArray " << data_type << " %u32_3\n";
  if (InitializerRequired(storage_class)) {
    after_types << "%built_in_array_null = OpConstantNull %built_in_array\n";
  }

  after_types << "%built_in_ptr = OpTypePointer " << storage_class
              << " %built_in_array\n";
  after_types << "%built_in_var = OpVariable %built_in_ptr " << storage_class;
  if (InitializerRequired(storage_class)) {
    after_types << " %built_in_array_null";
  }
  after_types << "\n";
  generator.after_types_ = after_types.str();

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = execution_model;
  entry_point.interfaces = "%built_in_var";
  // Any kind of reference would do.
  entry_point.body = R"(
%val = OpBitcast %u32 %built_in_var
)";

  std::ostringstream execution_modes;
  if (0 == std::strcmp(execution_model, "Fragment")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " OriginUpperLeft\n";
    if (0 == std::strcmp(built_in, "FragDepth")) {
      execution_modes << "OpExecutionMode %" << entry_point.name
                      << " DepthReplacing\n";
    }
  }
  if (0 == std::strcmp(execution_model, "Geometry")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " InputPoints\n";
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " OutputPoints\n";
  }
  if (0 == std::strcmp(execution_model, "GLCompute")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " LocalSize 1 1 1\n";
  }
  entry_point.execution_modes = execution_modes.str();

  generator.entry_points_.push_back(std::move(entry_point));

  return generator;
}

TEST_P(ValidateVulkanCombineBuiltInArrayedVariable, Variable) {
  const char* const built_in = std::get<0>(GetParam());
  const char* const execution_model = std::get<1>(GetParam());
  const char* const storage_class = std::get<2>(GetParam());
  const char* const data_type = std::get<3>(GetParam());
  const TestResult& test_result = std::get<4>(GetParam());

  CodeGenerator generator = GetArrayedVariableCodeGenerator(
      built_in, execution_model, storage_class, data_type);

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

INSTANTIATE_TEST_SUITE_P(PointSizeArrayedF32TessControl,
                         ValidateVulkanCombineBuiltInArrayedVariable,
                         Combine(Values("PointSize"),
                                 Values("TessellationControl"), Values("Input"),
                                 Values("%f32"), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    PointSizeArrayedF64TessControl, ValidateVulkanCombineBuiltInArrayedVariable,
    Combine(Values("PointSize"), Values("TessellationControl"), Values("Input"),
            Values("%f64"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float scalar",
                              "has bit width 64"))));

INSTANTIATE_TEST_SUITE_P(
    PointSizeArrayedF32Vertex, ValidateVulkanCombineBuiltInArrayedVariable,
    Combine(Values("PointSize"), Values("Vertex"), Values("Output"),
            Values("%f32"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float scalar",
                              "is not a float scalar"))));

INSTANTIATE_TEST_SUITE_P(PositionArrayedF32Vec4TessControl,
                         ValidateVulkanCombineBuiltInArrayedVariable,
                         Combine(Values("Position"),
                                 Values("TessellationControl"), Values("Input"),
                                 Values("%f32vec4"), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    PositionArrayedF32Vec3TessControl,
    ValidateVulkanCombineBuiltInArrayedVariable,
    Combine(Values("Position"), Values("TessellationControl"), Values("Input"),
            Values("%f32vec3"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float vector",
                              "has 3 components"))));

INSTANTIATE_TEST_SUITE_P(
    PositionArrayedF32Vec4Vertex, ValidateVulkanCombineBuiltInArrayedVariable,
    Combine(Values("Position"), Values("Vertex"), Values("Output"),
            Values("%f32vec4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit float vector",
                              "is not a float vector"))));

INSTANTIATE_TEST_SUITE_P(
    ClipAndCullDistanceOutputSuccess,
    ValidateVulkanCombineBuiltInArrayedVariable,
    Combine(Values("ClipDistance", "CullDistance"),
            Values("Geometry", "TessellationControl", "TessellationEvaluation"),
            Values("Output"), Values("%f32arr2", "%f32arr4"),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    ClipAndCullDistanceVertexInput, ValidateVulkanCombineBuiltInArrayedVariable,
    Combine(Values("ClipDistance", "CullDistance"), Values("Fragment"),
            Values("Input"), Values("%f32arr4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float array",
                              "components are not float scalar"))));

INSTANTIATE_TEST_SUITE_P(
    ClipAndCullDistanceNotArray, ValidateVulkanCombineBuiltInArrayedVariable,
    Combine(Values("ClipDistance", "CullDistance"),
            Values("Geometry", "TessellationControl", "TessellationEvaluation"),
            Values("Input"), Values("%f32vec2", "%f32vec4"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit float array",
                              "components are not float scalar"))));

INSTANTIATE_TEST_SUITE_P(
    SMBuiltinsInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("SMCountNV", "SMIDNV", "WarpsPerSMNV", "WarpIDNV"),
            Values("Vertex", "Fragment", "TessellationControl",
                   "TessellationEvaluation", "Geometry", "GLCompute"),
            Values("Input"), Values("%u32"),
            Values("OpCapability ShaderSMBuiltinsNV\n"),
            Values("OpExtension \"SPV_NV_shader_sm_builtins\"\n"),
            Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    SMBuiltinsInputMeshSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(
        Values("SMCountNV", "SMIDNV", "WarpsPerSMNV", "WarpIDNV"),
        Values("MeshNV", "TaskNV"), Values("Input"), Values("%u32"),
        Values("OpCapability ShaderSMBuiltinsNV\nOpCapability MeshShadingNV\n"),
        Values("OpExtension \"SPV_NV_shader_sm_builtins\"\nOpExtension "
               "\"SPV_NV_mesh_shader\"\n"),
        Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    SMBuiltinsInputRaySuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(
        Values("SMCountNV", "SMIDNV", "WarpsPerSMNV", "WarpIDNV"),
        Values("RayGenerationNV", "IntersectionNV", "AnyHitNV", "ClosestHitNV",
               "MissNV", "CallableNV"),
        Values("Input"), Values("%u32"),
        Values("OpCapability ShaderSMBuiltinsNV\nOpCapability RayTracingNV\n"),
        Values("OpExtension \"SPV_NV_shader_sm_builtins\"\nOpExtension "
               "\"SPV_NV_ray_tracing\"\n"),
        Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    SMBuiltinsNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("SMCountNV", "SMIDNV", "WarpsPerSMNV", "WarpIDNV"),
            Values("Vertex", "Fragment", "TessellationControl",
                   "TessellationEvaluation", "Geometry", "GLCompute"),
            Values("Output"), Values("%u32"),
            Values("OpCapability ShaderSMBuiltinsNV\n"),
            Values("OpExtension \"SPV_NV_shader_sm_builtins\"\n"),
            Values(nullptr),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class",
                "uses storage class Output"))));

INSTANTIATE_TEST_SUITE_P(
    SMBuiltinsNotIntScalar,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("SMCountNV", "SMIDNV", "WarpsPerSMNV", "WarpIDNV"),
            Values("Vertex", "Fragment", "TessellationControl",
                   "TessellationEvaluation", "Geometry", "GLCompute"),
            Values("Input"), Values("%f32", "%u32vec3"),
            Values("OpCapability ShaderSMBuiltinsNV\n"),
            Values("OpExtension \"SPV_NV_shader_sm_builtins\"\n"),
            Values(nullptr),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "is not an int scalar"))));

INSTANTIATE_TEST_SUITE_P(
    SMBuiltinsNotInt32,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("SMCountNV", "SMIDNV", "WarpsPerSMNV", "WarpIDNV"),
            Values("Vertex", "Fragment", "TessellationControl",
                   "TessellationEvaluation", "Geometry", "GLCompute"),
            Values("Input"), Values("%u64"),
            Values("OpCapability ShaderSMBuiltinsNV\n"),
            Values("OpExtension \"SPV_NV_shader_sm_builtins\"\n"),
            Values(nullptr),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int scalar",
                              "has bit width 64"))));

CodeGenerator GetWorkgroupSizeSuccessGenerator() {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();

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

  return generator;
}

TEST_F(ValidateBuiltIns, VulkanWorkgroupSizeSuccess) {
  CodeGenerator generator = GetWorkgroupSizeSuccessGenerator();
  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

CodeGenerator GetWorkgroupSizeFragmentGenerator() {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();

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

  return generator;
}

TEST_F(ValidateBuiltIns, VulkanWorkgroupSizeFragment) {
  CodeGenerator generator = GetWorkgroupSizeFragmentGenerator();

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Vulkan spec allows BuiltIn WorkgroupSize to be used "
                "only with GLCompute, MeshNV, or TaskNV execution model"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("is referencing ID <2> (OpConstantComposite) which is "
                        "decorated with BuiltIn WorkgroupSize in function <1> "
                        "called with execution model Fragment"));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-WorkgroupSize-WorkgroupSize-04425 "
                      "VUID-WorkgroupSize-WorkgroupSize-04427"));
}

TEST_F(ValidateBuiltIns, WorkgroupSizeNotConstant) {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();
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
              HasSubstr("BuiltIns can only target variables, structure "
                        "members or constants"));
}

CodeGenerator GetWorkgroupSizeNotVectorGenerator() {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();

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

  return generator;
}

TEST_F(ValidateBuiltIns, VulkanWorkgroupSizeNotVector) {
  CodeGenerator generator = GetWorkgroupSizeNotVectorGenerator();

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("According to the Vulkan spec BuiltIn WorkgroupSize "
                        "variable needs to be a 3-component 32-bit int vector. "
                        "ID <2> (OpConstant) is not an int vector."));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-WorkgroupSize-WorkgroupSize-04427"));
}

CodeGenerator GetWorkgroupSizeNotIntVectorGenerator() {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();

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

  return generator;
}

TEST_F(ValidateBuiltIns, VulkanWorkgroupSizeNotIntVector) {
  CodeGenerator generator = GetWorkgroupSizeNotIntVectorGenerator();

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("According to the Vulkan spec BuiltIn WorkgroupSize "
                        "variable needs to be a 3-component 32-bit int vector. "
                        "ID <2> (OpConstantComposite) is not an int vector."));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-WorkgroupSize-WorkgroupSize-04427"));
}

CodeGenerator GetWorkgroupSizeNotVec3Generator() {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();

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

  return generator;
}

TEST_F(ValidateBuiltIns, VulkanWorkgroupSizeNotVec3) {
  CodeGenerator generator = GetWorkgroupSizeNotVec3Generator();

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("According to the Vulkan spec BuiltIn WorkgroupSize "
                        "variable needs to be a 3-component 32-bit int vector. "
                        "ID <2> (OpConstantComposite) has 2 components."));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-WorkgroupSize-WorkgroupSize-04427"));
}

TEST_F(ValidateBuiltIns, WorkgroupSizeNotInt32Vec) {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();
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
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-WorkgroupSize-WorkgroupSize-04427"));
}

TEST_F(ValidateBuiltIns, WorkgroupSizePrivateVar) {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();
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
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();

  generator.before_types_ = R"(
OpDecorate %input_type Block
OpMemberDecorate %input_type 0 BuiltIn Position
OpDecorate %output_type Block
OpMemberDecorate %output_type 0 BuiltIn Position
)";

  generator.after_types_ = R"(
%input_type = OpTypeStruct %f32vec4
%arrayed_input_type = OpTypeArray %input_type %u32_3
%input_ptr = OpTypePointer Input %arrayed_input_type
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
%input_pos = OpAccessChain %input_f32vec4_ptr %input %u32_0 %u32_0
%output_pos = OpAccessChain %output_f32vec4_ptr %output %u32_0
%pos = OpLoad %f32vec4 %input_pos
OpStore %output_pos %pos
)";
  generator.entry_points_.push_back(std::move(entry_point));
  generator.entry_points_[0].execution_modes =
      "OpExecutionMode %main InputPoints\nOpExecutionMode %main OutputPoints\n";

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateBuiltIns, WorkgroupIdNotVec3) {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();
  generator.before_types_ = R"(
OpDecorate %workgroup_size BuiltIn WorkgroupSize
OpDecorate %workgroup_id BuiltIn WorkgroupId
)";

  generator.after_types_ = R"(
%workgroup_size = OpConstantComposite %u32vec3 %u32_1 %u32_1 %u32_1
     %input_ptr = OpTypePointer Input %u32vec2
  %workgroup_id = OpVariable %input_ptr Input
)";

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = "GLCompute";
  entry_point.interfaces = "%workgroup_id";
  entry_point.body = R"(
%copy_size = OpCopyObject %u32vec3 %workgroup_size
  %load_id = OpLoad %u32vec2 %workgroup_id
)";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("According to the Vulkan spec BuiltIn WorkgroupId "
                        "variable needs to be a 3-component 32-bit int vector. "
                        "ID <2> (OpVariable) has 2 components."));
}

TEST_F(ValidateBuiltIns, TwoBuiltInsFirstFails) {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();

  generator.before_types_ = R"(
OpDecorate %input_type Block
OpDecorate %output_type Block
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
  generator.entry_points_[0].execution_modes =
      "OpExecutionMode %main InputPoints\nOpExecutionMode %main OutputPoints\n";

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Vulkan spec allows BuiltIn FragCoord to be used only "
                        "with Fragment execution model"));
}

TEST_F(ValidateBuiltIns, TwoBuiltInsSecondFails) {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();

  generator.before_types_ = R"(
OpDecorate %input_type Block
OpDecorate %output_type Block
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
  generator.entry_points_[0].execution_modes =
      "OpExecutionMode %main InputPoints\nOpExecutionMode %main OutputPoints\n";

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Vulkan spec allows BuiltIn FragCoord to be only used "
                        "for variables with Input storage class"));
}

TEST_F(ValidateBuiltIns, VertexPositionVariableSuccess) {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();
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
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();
  generator.before_types_ = R"(
OpDecorate %output_type Block
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

CodeGenerator GetNoDepthReplacingGenerator() {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();

  generator.before_types_ = R"(
OpDecorate %output_type Block
OpMemberDecorate %output_type 0 BuiltIn FragDepth
)";

  generator.after_types_ = R"(
%output_type = OpTypeStruct %f32
%output_null = OpConstantNull %output_type
%output_ptr = OpTypePointer Output %output_type
%output = OpVariable %output_ptr Output %output_null
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

  const std::string function_body = R"(
%foo = OpFunction %void None %func
%foo_entry = OpLabel
%frag_depth = OpAccessChain %output_f32_ptr %output %u32_0
OpStore %frag_depth %f32_1
OpReturn
OpFunctionEnd
)";

    generator.add_at_the_end_ = function_body;

  return generator;
}

TEST_F(ValidateBuiltIns, VulkanFragmentFragDepthNoDepthReplacing) {
  CodeGenerator generator = GetNoDepthReplacingGenerator();

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Vulkan spec requires DepthReplacing execution mode to "
                        "be declared when using BuiltIn FragDepth"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("VUID-FragDepth-FragDepth-04216"));
}

CodeGenerator GetOneMainHasDepthReplacingOtherHasntGenerator() {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();

  generator.before_types_ = R"(
OpDecorate %output_type Block
OpMemberDecorate %output_type 0 BuiltIn FragDepth
)";

  generator.after_types_ = R"(
%output_type = OpTypeStruct %f32
%output_null = OpConstantNull %output_type
%output_ptr = OpTypePointer Output %output_type
%output = OpVariable %output_ptr Output %output_null
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

  const std::string function_body = R"(
%foo = OpFunction %void None %func
%foo_entry = OpLabel
%frag_depth = OpAccessChain %output_f32_ptr %output %u32_0
OpStore %frag_depth %f32_1
OpReturn
OpFunctionEnd
)";

    generator.add_at_the_end_ = function_body;

  return generator;
}

TEST_F(ValidateBuiltIns,
       VulkanFragmentFragDepthOneMainHasDepthReplacingOtherHasnt) {
  CodeGenerator generator = GetOneMainHasDepthReplacingOtherHasntGenerator();

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Vulkan spec requires DepthReplacing execution mode to "
                        "be declared when using BuiltIn FragDepth"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("VUID-FragDepth-FragDepth-04216"));
}


TEST_F(ValidateBuiltIns, AllowInstanceIdWithIntersectionShader) {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();
  generator.capabilities_ += R"(
OpCapability RayTracingNV
)";

  generator.extensions_ = R"(
OpExtension "SPV_NV_ray_tracing"
)";

  generator.before_types_ = R"(
OpDecorate %input_type Block
OpMemberDecorate %input_type 0 BuiltIn InstanceId
)";

  generator.after_types_ = R"(
%input_type = OpTypeStruct %u32
%input_ptr = OpTypePointer Input %input_type
%input = OpVariable %input_ptr Input
)";

  EntryPoint entry_point;
  entry_point.name = "main_d_r";
  entry_point.execution_model = "IntersectionNV";
  entry_point.interfaces = "%input";
  entry_point.body = R"(
%val2 = OpFunctionCall %void %foo
)";
  generator.entry_points_.push_back(std::move(entry_point));

  generator.add_at_the_end_ = R"(
%foo = OpFunction %void None %func
%foo_entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateBuiltIns, ValidBuiltinsForMeshShader) {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();
  generator.capabilities_ += R"(
OpCapability MeshShadingNV
)";

  generator.extensions_ = R"(
OpExtension "SPV_NV_mesh_shader"
)";

  generator.before_types_ = R"(
OpDecorate %gl_PrimitiveID BuiltIn PrimitiveId
OpDecorate %gl_PrimitiveID PerPrimitiveNV
OpDecorate %gl_Layer BuiltIn Layer
OpDecorate %gl_Layer PerPrimitiveNV
OpDecorate %gl_ViewportIndex BuiltIn ViewportIndex
OpDecorate %gl_ViewportIndex PerPrimitiveNV
)";

  generator.after_types_ = R"(
%u32_81 = OpConstant %u32 81
%_arr_int_uint_81 = OpTypeArray %i32 %u32_81
%_ptr_Output__arr_int_uint_81 = OpTypePointer Output %_arr_int_uint_81
%gl_PrimitiveID = OpVariable %_ptr_Output__arr_int_uint_81 Output
%gl_Layer = OpVariable %_ptr_Output__arr_int_uint_81 Output
%gl_ViewportIndex = OpVariable %_ptr_Output__arr_int_uint_81 Output
)";

  EntryPoint entry_point;
  entry_point.name = "main_d_r";
  entry_point.execution_model = "MeshNV";
  entry_point.interfaces = "%gl_PrimitiveID %gl_Layer %gl_ViewportIndex";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_1);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_1));
}

TEST_F(ValidateBuiltIns, InvalidBuiltinsForMeshShader) {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();
  generator.capabilities_ += R"(
OpCapability MeshShadingNV
)";

  generator.extensions_ = R"(
OpExtension "SPV_NV_mesh_shader"
)";

  generator.before_types_ = R"(
OpDecorate %gl_PrimitiveID BuiltIn PrimitiveId
OpDecorate %gl_PrimitiveID PerPrimitiveNV
OpDecorate %gl_Layer BuiltIn Layer
OpDecorate %gl_Layer PerPrimitiveNV
OpDecorate %gl_ViewportIndex BuiltIn ViewportIndex
OpDecorate %gl_ViewportIndex PerPrimitiveNV
)";

  generator.after_types_ = R"(
%u32_81 = OpConstant %u32 81
%_arr_float_uint_81 = OpTypeArray %f32 %u32_81
%_ptr_Output__arr_float_uint_81 = OpTypePointer Output %_arr_float_uint_81
%gl_PrimitiveID = OpVariable %_ptr_Output__arr_float_uint_81 Output
%gl_Layer = OpVariable %_ptr_Output__arr_float_uint_81 Output
%gl_ViewportIndex = OpVariable %_ptr_Output__arr_float_uint_81 Output
)";

  EntryPoint entry_point;
  entry_point.name = "main_d_r";
  entry_point.execution_model = "MeshNV";
  entry_point.interfaces = "%gl_PrimitiveID %gl_Layer %gl_ViewportIndex";
  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_1);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("needs to be a 32-bit int scalar"));
  EXPECT_THAT(getDiagnosticString(), HasSubstr("is not an int scalar"));
}

TEST_P(ValidateVulkanSubgroupBuiltIns, InMain) {
  const char* const built_in = std::get<0>(GetParam());
  const char* const execution_model = std::get<1>(GetParam());
  const char* const storage_class = std::get<2>(GetParam());
  const char* const data_type = std::get<3>(GetParam());
  const char* const vuid = std::get<4>(GetParam());
  const TestResult& test_result = std::get<5>(GetParam());

  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();
  generator.capabilities_ += R"(
OpCapability GroupNonUniformBallot
)";

  generator.before_types_ = "OpDecorate %built_in_var BuiltIn ";
  generator.before_types_ += built_in;
  generator.before_types_ += "\n";

  std::ostringstream after_types;
  after_types << "%built_in_ptr = OpTypePointer " << storage_class << " "
              << data_type << "\n";
  after_types << "%built_in_var = OpVariable %built_in_ptr " << storage_class;
  after_types << "\n";
  generator.after_types_ = after_types.str();

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = execution_model;
  if (strncmp(storage_class, "Input", 5) == 0 ||
      strncmp(storage_class, "Output", 6) == 0) {
    entry_point.interfaces = "%built_in_var";
  }
  entry_point.body =
      std::string("%ld = OpLoad ") + data_type + " %built_in_var\n";

  std::ostringstream execution_modes;
  if (0 == std::strcmp(execution_model, "Fragment")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " OriginUpperLeft\n";
    if (0 == std::strcmp(built_in, "FragDepth")) {
      execution_modes << "OpExecutionMode %" << entry_point.name
                      << " DepthReplacing\n";
    }
  }
  if (0 == std::strcmp(execution_model, "Geometry")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " InputPoints\n";
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " OutputPoints\n";
  }
  if (0 == std::strcmp(execution_model, "GLCompute")) {
    execution_modes << "OpExecutionMode %" << entry_point.name
                    << " LocalSize 1 1 1\n";
  }
  entry_point.execution_modes = execution_modes.str();

  generator.entry_points_.push_back(std::move(entry_point));

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_1);
  ASSERT_EQ(test_result.validation_result,
            ValidateInstructions(SPV_ENV_VULKAN_1_1));
  if (test_result.error_str) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str));
  }
  if (test_result.error_str2) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(test_result.error_str2));
  }
  if (vuid) {
    EXPECT_THAT(getDiagnosticString(), AnyVUID(vuid));
  }
}

INSTANTIATE_TEST_SUITE_P(
    SubgroupMaskNotVec4, ValidateVulkanSubgroupBuiltIns,
    Combine(Values("SubgroupEqMask", "SubgroupGeMask", "SubgroupGtMask",
                   "SubgroupLeMask", "SubgroupLtMask"),
            Values("GLCompute"), Values("Input"), Values("%u32vec3"),
            Values("VUID-SubgroupEqMask-SubgroupEqMask-04371 "
                   "VUID-SubgroupGeMask-SubgroupGeMask-04373 "
                   "VUID-SubgroupGtMask-SubgroupGtMask-04375 "
                   "VUID-SubgroupLeMask-SubgroupLeMask-04377 "
                   "VUID-SubgroupLtMask-SubgroupLtMask-04379"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit int vector"))));

INSTANTIATE_TEST_SUITE_P(
    SubgroupMaskNotU32, ValidateVulkanSubgroupBuiltIns,
    Combine(Values("SubgroupEqMask", "SubgroupGeMask", "SubgroupGtMask",
                   "SubgroupLeMask", "SubgroupLtMask"),
            Values("GLCompute"), Values("Input"), Values("%f32vec4"),
            Values("VUID-SubgroupEqMask-SubgroupEqMask-04371 "
                   "VUID-SubgroupGeMask-SubgroupGeMask-04373 "
                   "VUID-SubgroupGtMask-SubgroupGtMask-04375 "
                   "VUID-SubgroupLeMask-SubgroupLeMask-04377 "
                   "VUID-SubgroupLtMask-SubgroupLtMask-04379"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 4-component 32-bit int vector"))));

INSTANTIATE_TEST_SUITE_P(
    SubgroupMaskNotInput, ValidateVulkanSubgroupBuiltIns,
    Combine(Values("SubgroupEqMask", "SubgroupGeMask", "SubgroupGtMask",
                   "SubgroupLeMask", "SubgroupLtMask"),
            Values("GLCompute"), Values("Output", "Workgroup", "Private"),
            Values("%u32vec4"),
            Values("VUID-SubgroupEqMask-SubgroupEqMask-04370 "
                   "VUID-SubgroupGeMask-SubgroupGeMask-04372 "
                   "VUID-SubgroupGtMask-SubgroupGtMask-04374 "
                   "VUID-SubgroupLeMask-SubgroupLeMask-04376  "
                   "VUID-SubgroupLtMask-SubgroupLtMask-04378"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class"))));

INSTANTIATE_TEST_SUITE_P(SubgroupMaskOk, ValidateVulkanSubgroupBuiltIns,
                         Combine(Values("SubgroupEqMask", "SubgroupGeMask",
                                        "SubgroupGtMask", "SubgroupLeMask",
                                        "SubgroupLtMask"),
                                 Values("GLCompute"), Values("Input"),
                                 Values("%u32vec4"), Values(nullptr),
                                 Values(TestResult(SPV_SUCCESS, ""))));

TEST_F(ValidateBuiltIns, SubgroupMaskMemberDecorate) {
  const std::string text = R"(
OpCapability Shader
OpCapability GroupNonUniformBallot
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %foo "foo"
OpExecutionMode %foo LocalSize 1 1 1
OpDecorate %struct Block
OpMemberDecorate %struct 0 BuiltIn SubgroupEqMask
%void = OpTypeVoid
%int = OpTypeInt 32 0
%struct = OpTypeStruct %int
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_VULKAN_1_1);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "BuiltIn SubgroupEqMask cannot be used as a member decoration"));
}

INSTANTIATE_TEST_SUITE_P(
    SubgroupInvocationIdAndSizeNotU32, ValidateVulkanSubgroupBuiltIns,
    Combine(
        Values("SubgroupLocalInvocationId", "SubgroupSize"),
        Values("GLCompute"), Values("Input"), Values("%f32"),
        Values("VUID-SubgroupLocalInvocationId-SubgroupLocalInvocationId-04381 "
               "VUID-SubgroupSize-SubgroupSize-04383"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "needs to be a 32-bit int"))));

INSTANTIATE_TEST_SUITE_P(
    SubgroupInvocationIdAndSizeNotInput, ValidateVulkanSubgroupBuiltIns,
    Combine(
        Values("SubgroupLocalInvocationId", "SubgroupSize"),
        Values("GLCompute"), Values("Output", "Workgroup", "Private"),
        Values("%u32"),
        Values("VUID-SubgroupLocalInvocationId-SubgroupLocalInvocationId-04380 "
               "VUID-SubgroupSize-SubgroupSize-04382"),
        Values(TestResult(
            SPV_ERROR_INVALID_DATA,
            "to be only used for variables with Input storage class"))));

INSTANTIATE_TEST_SUITE_P(
    SubgroupInvocationIdAndSizeOk, ValidateVulkanSubgroupBuiltIns,
    Combine(Values("SubgroupLocalInvocationId", "SubgroupSize"),
            Values("GLCompute"), Values("Input"), Values("%u32"),
            Values(nullptr), Values(TestResult(SPV_SUCCESS, ""))));

TEST_F(ValidateBuiltIns, SubgroupSizeMemberDecorate) {
  const std::string text = R"(
OpCapability Shader
OpCapability GroupNonUniform
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %foo "foo"
OpExecutionMode %foo LocalSize 1 1 1
OpDecorate %struct Block
OpMemberDecorate %struct 0 BuiltIn SubgroupSize
%void = OpTypeVoid
%int = OpTypeInt 32 0
%struct = OpTypeStruct %int
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_VULKAN_1_1);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("BuiltIn SubgroupSize cannot be used as a member decoration"));
}

INSTANTIATE_TEST_SUITE_P(
    SubgroupNumAndIdNotCompute, ValidateVulkanSubgroupBuiltIns,
    Combine(Values("SubgroupId", "NumSubgroups"), Values("Vertex"),
            Values("Input"), Values("%u32"),
            Values("VUID-SubgroupId-SubgroupId-04367 "
                   "VUID-NumSubgroups-NumSubgroups-04293"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "to be used only with GLCompute, MeshNV, or "
                              "TaskNV execution model"))));

INSTANTIATE_TEST_SUITE_P(
    SubgroupNumAndIdNotU32, ValidateVulkanSubgroupBuiltIns,
    Combine(Values("SubgroupId", "NumSubgroups"), Values("GLCompute"),
            Values("Input"), Values("%f32"),
            Values("VUID-SubgroupId-SubgroupId-04369 "
                   "VUID-NumSubgroups-NumSubgroups-04295"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "needs to be a 32-bit int"))));

INSTANTIATE_TEST_SUITE_P(
    SubgroupNumAndIdNotInput, ValidateVulkanSubgroupBuiltIns,
    Combine(Values("SubgroupId", "NumSubgroups"), Values("GLCompute"),
            Values("Output", "Workgroup", "Private"), Values("%u32"),
            Values("VUID-SubgroupId-SubgroupId-04368 "
                   "VUID-NumSubgroups-NumSubgroups-04294"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "to be only used for variables with Input storage class"))));

INSTANTIATE_TEST_SUITE_P(SubgroupNumAndIdOk, ValidateVulkanSubgroupBuiltIns,
                         Combine(Values("SubgroupId", "NumSubgroups"),
                                 Values("GLCompute"), Values("Input"),
                                 Values("%u32"), Values(nullptr),
                                 Values(TestResult(SPV_SUCCESS, ""))));

TEST_F(ValidateBuiltIns, SubgroupIdMemberDecorate) {
  const std::string text = R"(
OpCapability Shader
OpCapability GroupNonUniform
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %foo "foo"
OpExecutionMode %foo LocalSize 1 1 1
OpDecorate %struct Block
OpMemberDecorate %struct 0 BuiltIn SubgroupId
%void = OpTypeVoid
%int = OpTypeInt 32 0
%struct = OpTypeStruct %int
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_VULKAN_1_1);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("BuiltIn SubgroupId cannot be used as a member decoration"));
}

TEST_F(ValidateBuiltIns, TargetIsType) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %void BuiltIn Position
%void = OpTypeVoid
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("BuiltIns can only target variables, structure members "
                        "or constants"));
}

TEST_F(ValidateBuiltIns, TargetIsVariable) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %wg_var BuiltIn Position
%int = OpTypeInt 32 0
%int_wg_ptr = OpTypePointer Workgroup %int
%wg_var = OpVariable %int_wg_ptr Workgroup
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

INSTANTIATE_TEST_SUITE_P(
    PrimitiveShadingRateOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("PrimitiveShadingRateKHR"), Values("Vertex", "Geometry"),
            Values("Output"), Values("%u32"),
            Values("OpCapability FragmentShadingRateKHR\n"),
            Values("OpExtension \"SPV_KHR_fragment_shading_rate\"\n"),
            Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    PrimitiveShadingRateMeshOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("PrimitiveShadingRateKHR"), Values("MeshNV"),
            Values("Output"), Values("%u32"),
            Values("OpCapability FragmentShadingRateKHR\nOpCapability "
                   "MeshShadingNV\n"),
            Values("OpExtension \"SPV_KHR_fragment_shading_rate\"\nOpExtension "
                   "\"SPV_NV_mesh_shader\"\n"),
            Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    PrimitiveShadingRateInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(
        Values("PrimitiveShadingRateKHR"), Values("Fragment"), Values("Output"),
        Values("%u32"), Values("OpCapability FragmentShadingRateKHR\n"),
        Values("OpExtension \"SPV_KHR_fragment_shading_rate\"\n"),
        Values("VUID-PrimitiveShadingRateKHR-PrimitiveShadingRateKHR-04484 "),
        Values(TestResult(
            SPV_ERROR_INVALID_DATA,
            "Vulkan spec allows BuiltIn PrimitiveShadingRateKHR to be used "
            "only with Vertex, Geometry, or MeshNV execution models."))));

INSTANTIATE_TEST_SUITE_P(
    PrimitiveShadingRateInvalidStorageClass,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(
        Values("PrimitiveShadingRateKHR"), Values("Vertex"), Values("Input"),
        Values("%u32"), Values("OpCapability FragmentShadingRateKHR\n"),
        Values("OpExtension \"SPV_KHR_fragment_shading_rate\"\n"),
        Values("VUID-PrimitiveShadingRateKHR-PrimitiveShadingRateKHR-04485 "),
        Values(TestResult(
            SPV_ERROR_INVALID_DATA,
            "Vulkan spec allows BuiltIn PrimitiveShadingRateKHR to be only "
            "used for variables with Output storage class."))));

INSTANTIATE_TEST_SUITE_P(
    PrimitiveShadingRateInvalidType,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(
        Values("PrimitiveShadingRateKHR"), Values("Vertex"), Values("Output"),
        Values("%f32"), Values("OpCapability FragmentShadingRateKHR\n"),
        Values("OpExtension \"SPV_KHR_fragment_shading_rate\"\n"),
        Values("VUID-PrimitiveShadingRateKHR-PrimitiveShadingRateKHR-04486 "),
        Values(TestResult(
            SPV_ERROR_INVALID_DATA,
            "According to the Vulkan spec BuiltIn PrimitiveShadingRateKHR "
            "variable needs to be a 32-bit int scalar."))));

INSTANTIATE_TEST_SUITE_P(
    ShadingRateInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("ShadingRateKHR"), Values("Fragment"), Values("Input"),
            Values("%u32"), Values("OpCapability FragmentShadingRateKHR\n"),
            Values("OpExtension \"SPV_KHR_fragment_shading_rate\"\n"),
            Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    ShadingRateInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("ShadingRateKHR"), Values("Vertex"), Values("Input"),
            Values("%u32"), Values("OpCapability FragmentShadingRateKHR\n"),
            Values("OpExtension \"SPV_KHR_fragment_shading_rate\"\n"),
            Values("VUID-ShadingRateKHR-ShadingRateKHR-04490 "),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec allows BuiltIn ShadingRateKHR to be used "
                "only with the Fragment execution model."))));

INSTANTIATE_TEST_SUITE_P(
    ShadingRateInvalidStorageClass,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("ShadingRateKHR"), Values("Fragment"), Values("Output"),
            Values("%u32"), Values("OpCapability FragmentShadingRateKHR\n"),
            Values("OpExtension \"SPV_KHR_fragment_shading_rate\"\n"),
            Values("VUID-ShadingRateKHR-ShadingRateKHR-04491 "),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec allows BuiltIn ShadingRateKHR to be only "
                "used for variables with Input storage class."))));

INSTANTIATE_TEST_SUITE_P(
    ShadingRateInvalidType,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(
        Values("ShadingRateKHR"), Values("Fragment"), Values("Input"),
        Values("%f32"), Values("OpCapability FragmentShadingRateKHR\n"),
        Values("OpExtension \"SPV_KHR_fragment_shading_rate\"\n"),
        Values("VUID-ShadingRateKHR-ShadingRateKHR-04492 "),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "According to the Vulkan spec BuiltIn ShadingRateKHR "
                          "variable needs to be a 32-bit int scalar."))));

INSTANTIATE_TEST_SUITE_P(
    FragInvocationCountInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("FragInvocationCountEXT"), Values("Fragment"),
            Values("Input"), Values("%u32"),
            Values("OpCapability FragmentDensityEXT\n"),
            Values("OpExtension \"SPV_EXT_fragment_invocation_density\"\n"),
            Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    FragInvocationCountInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(
        Values("FragInvocationCountEXT"), Values("Vertex"), Values("Input"),
        Values("%u32"), Values("OpCapability FragmentDensityEXT\n"),
        Values("OpExtension \"SPV_EXT_fragment_invocation_density\"\n"),
        Values("VUID-FragInvocationCountEXT-FragInvocationCountEXT-04217"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "Vulkan spec allows BuiltIn FragInvocationCountEXT "
                          "to be used only with Fragment execution model."))));

INSTANTIATE_TEST_SUITE_P(
    FragInvocationCountInvalidStorageClass,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("FragInvocationCountEXT"), Values("Fragment"),
            Values("Output"), Values("%u32"),
            Values("OpCapability FragmentDensityEXT\n"),
            Values("OpExtension \"SPV_EXT_fragment_invocation_density\"\n"),
            Values("VUID-FragInvocationCountEXT-FragInvocationCountEXT-04218"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec allows BuiltIn FragInvocationCountEXT to be only "
                "used for variables with Input storage class."))));

INSTANTIATE_TEST_SUITE_P(
    FragInvocationCountInvalidType,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("FragInvocationCountEXT"), Values("Fragment"),
            Values("Input"), Values("%f32"),
            Values("OpCapability FragmentDensityEXT\n"),
            Values("OpExtension \"SPV_EXT_fragment_invocation_density\"\n"),
            Values("VUID-FragInvocationCountEXT-FragInvocationCountEXT-04219"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "According to the Vulkan spec BuiltIn FragInvocationCountEXT "
                "variable needs to be a 32-bit int scalar."))));

INSTANTIATE_TEST_SUITE_P(
    FragSizeInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("FragSizeEXT"), Values("Fragment"), Values("Input"),
            Values("%u32vec2"), Values("OpCapability FragmentDensityEXT\n"),
            Values("OpExtension \"SPV_EXT_fragment_invocation_density\"\n"),
            Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    FragSizeInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("FragSizeEXT"), Values("Vertex"), Values("Input"),
            Values("%u32vec2"), Values("OpCapability FragmentDensityEXT\n"),
            Values("OpExtension \"SPV_EXT_fragment_invocation_density\"\n"),
            Values("VUID-FragSizeEXT-FragSizeEXT-04220"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "Vulkan spec allows BuiltIn FragSizeEXT to be "
                              "used only with Fragment execution model."))));

INSTANTIATE_TEST_SUITE_P(
    FragSizeInvalidStorageClass,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(
        Values("FragSizeEXT"), Values("Fragment"), Values("Output"),
        Values("%u32vec2"), Values("OpCapability FragmentDensityEXT\n"),
        Values("OpExtension \"SPV_EXT_fragment_invocation_density\"\n"),
        Values("VUID-FragSizeEXT-FragSizeEXT-04221"),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "Vulkan spec allows BuiltIn FragSizeEXT to be only "
                          "used for variables with Input storage class."))));

INSTANTIATE_TEST_SUITE_P(
    FragSizeInvalidType,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("FragSizeEXT"), Values("Fragment"), Values("Input"),
            Values("%u32vec3"), Values("OpCapability FragmentDensityEXT\n"),
            Values("OpExtension \"SPV_EXT_fragment_invocation_density\"\n"),
            Values("VUID-FragSizeEXT-FragSizeEXT-04222"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "According to the Vulkan spec BuiltIn FragSizeEXT variable "
                "needs to be a 2-component 32-bit int vector."))));

INSTANTIATE_TEST_SUITE_P(
    FragStencilRefOutputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("FragStencilRefEXT"), Values("Fragment"), Values("Output"),
            Values("%u32", "%u64"), Values("OpCapability StencilExportEXT\n"),
            Values("OpExtension \"SPV_EXT_shader_stencil_export\"\n"),
            Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    FragStencilRefInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("FragStencilRefEXT"), Values("Vertex"), Values("Output"),
            Values("%u32", "%u64"), Values("OpCapability StencilExportEXT\n"),
            Values("OpExtension \"SPV_EXT_shader_stencil_export\"\n"),
            Values("VUID-FragStencilRefEXT-FragStencilRefEXT-04223"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "Vulkan spec allows BuiltIn FragStencilRefEXT to "
                              "be used only with Fragment execution model."))));

INSTANTIATE_TEST_SUITE_P(
    FragStencilRefInvalidStorageClass,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("FragStencilRefEXT"), Values("Fragment"), Values("Input"),
            Values("%u32", "%u64"), Values("OpCapability StencilExportEXT\n"),
            Values("OpExtension \"SPV_EXT_shader_stencil_export\"\n"),
            Values("VUID-FragStencilRefEXT-FragStencilRefEXT-04224"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec allows BuiltIn FragStencilRefEXT to be only used "
                "for variables with Output storage class."))));

INSTANTIATE_TEST_SUITE_P(
    FragStencilRefInvalidType,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("FragStencilRefEXT"), Values("Fragment"), Values("Output"),
            Values("%f32", "%f64", "%u32vec2"),
            Values("OpCapability StencilExportEXT\n"),
            Values("OpExtension \"SPV_EXT_shader_stencil_export\"\n"),
            Values("VUID-FragStencilRefEXT-FragStencilRefEXT-04225"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "According to the Vulkan spec BuiltIn FragStencilRefEXT "
                "variable needs to be a int scalar."))));

INSTANTIATE_TEST_SUITE_P(
    FullyCoveredEXTInputSuccess,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("FullyCoveredEXT"), Values("Fragment"), Values("Input"),
            Values("%bool"), Values("OpCapability FragmentFullyCoveredEXT\n"),
            Values("OpExtension \"SPV_EXT_fragment_fully_covered\"\n"),
            Values(nullptr), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    FullyCoveredEXTInvalidExecutionModel,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("FullyCoveredEXT"), Values("Vertex"), Values("Input"),
            Values("%bool"), Values("OpCapability FragmentFullyCoveredEXT\n"),
            Values("OpExtension \"SPV_EXT_fragment_fully_covered\"\n"),
            Values("VUID-FullyCoveredEXT-FullyCoveredEXT-04232"),
            Values(TestResult(SPV_ERROR_INVALID_DATA,
                              "Vulkan spec allows BuiltIn FullyCoveredEXT to "
                              "be used only with Fragment execution model."))));

INSTANTIATE_TEST_SUITE_P(
    FullyCoveredEXTInvalidStorageClass,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("FullyCoveredEXT"), Values("Fragment"), Values("Output"),
            Values("%bool"), Values("OpCapability FragmentFullyCoveredEXT\n"),
            Values("OpExtension \"SPV_EXT_fragment_fully_covered\"\n"),
            Values("VUID-FullyCoveredEXT-FullyCoveredEXT-04233"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "Vulkan spec allows BuiltIn FullyCoveredEXT to be only used "
                "for variables with Input storage class."))));

INSTANTIATE_TEST_SUITE_P(
    FullyCoveredEXTInvalidType,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("FullyCoveredEXT"), Values("Fragment"), Values("Input"),
            Values("%f32"), Values("OpCapability FragmentFullyCoveredEXT\n"),
            Values("OpExtension \"SPV_EXT_fragment_fully_covered\"\n"),
            Values("VUID-FullyCoveredEXT-FullyCoveredEXT-04234"),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "According to the Vulkan spec BuiltIn FullyCoveredEXT variable "
                "needs to be a bool scalar."))));

INSTANTIATE_TEST_SUITE_P(
    BaryCoordNotFragment,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(
        Values("BaryCoordKHR", "BaryCoordNoPerspKHR"), Values("Vertex"),
        Values("Input"), Values("%f32vec3"),
        Values("OpCapability FragmentBarycentricKHR\n"),
        Values("OpExtension \"SPV_KHR_fragment_shader_barycentric\"\n"),
        Values("VUID-BaryCoordKHR-BaryCoordKHR-04154 "
               "VUID-BaryCoordNoPerspKHR-BaryCoordNoPerspKHR-04160 "),
        Values(TestResult(SPV_ERROR_INVALID_DATA, "Vulkan spec allows BuiltIn",
                          "to be used only with Fragment execution model"))));

INSTANTIATE_TEST_SUITE_P(
    BaryCoordNotInput,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(Values("BaryCoordKHR", "BaryCoordNoPerspKHR"), Values("Fragment"),
            Values("Output"), Values("%f32vec3"),
            Values("OpCapability FragmentBarycentricKHR\n"),
            Values("OpExtension \"SPV_KHR_fragment_shader_barycentric\"\n"),
            Values("VUID-BaryCoordKHR-BaryCoordKHR-04155 "
                   "VUID-BaryCoordNoPerspKHR-BaryCoordNoPerspKHR-04161 "),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA, "Vulkan spec allows BuiltIn",
                "to be only used for variables with Input storage class"))));

INSTANTIATE_TEST_SUITE_P(
    BaryCoordNotFloatVector,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(
        Values("BaryCoordKHR", "BaryCoordNoPerspKHR"), Values("Fragment"),
        Values("Output"), Values("%f32arr3", "%u32vec4"),
        Values("OpCapability FragmentBarycentricKHR\n"),
        Values("OpExtension \"SPV_KHR_fragment_shader_barycentric\"\n"),
        Values("VUID-BaryCoordKHR-BaryCoordKHR-04156 "
               "VUID-BaryCoordNoPerspKHR-BaryCoordNoPerspKHR-04162 "),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "needs to be a 3-component 32-bit float vector"))));

INSTANTIATE_TEST_SUITE_P(
    BaryCoordNotFloatVec3,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(
        Values("BaryCoordKHR", "BaryCoordNoPerspKHR"), Values("Fragment"),
        Values("Output"), Values("%f32vec2"),
        Values("OpCapability FragmentBarycentricKHR\n"),
        Values("OpExtension \"SPV_KHR_fragment_shader_barycentric\"\n"),
        Values("VUID-BaryCoordKHR-BaryCoordKHR-04156 "
               "VUID-BaryCoordNoPerspKHR-BaryCoordNoPerspKHR-04162 "),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "needs to be a 3-component 32-bit float vector"))));

INSTANTIATE_TEST_SUITE_P(
    BaryCoordNotF32Vec3,
    ValidateVulkanCombineBuiltInExecutionModelDataTypeCapabilityExtensionResult,
    Combine(
        Values("BaryCoordKHR", "BaryCoordNoPerspKHR"), Values("Fragment"),
        Values("Output"), Values("%f64vec3"),
        Values("OpCapability FragmentBarycentricKHR\n"),
        Values("OpExtension \"SPV_KHR_fragment_shader_barycentric\"\n"),
        Values("VUID-BaryCoordKHR-BaryCoordKHR-04156 "
               "VUID-BaryCoordNoPerspKHR-BaryCoordNoPerspKHR-04162 "),
        Values(TestResult(SPV_ERROR_INVALID_DATA,
                          "needs to be a 3-component 32-bit float vector"))));

}  // namespace
}  // namespace val
}  // namespace spvtools
