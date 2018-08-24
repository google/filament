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

#include <sstream>
#include <string>

#include "gmock/gmock.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::Not;

using ValidateBarriers = spvtest::ValidateBase<bool>;

std::string GenerateShaderCode(
    const std::string& body,
    const std::string& capabilities_and_extensions = "",
    const std::string& execution_model = "GLCompute") {
  std::ostringstream ss;
  ss << R"(
OpCapability Shader
OpCapability Int64
)";

  ss << capabilities_and_extensions;
  ss << "OpMemoryModel Logical GLSL450\n";
  ss << "OpEntryPoint " << execution_model << " %main \"main\"\n";

  ss << R"(
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f32 = OpTypeFloat 32
%u32 = OpTypeInt 32 0
%u64 = OpTypeInt 64 0

%f32_0 = OpConstant %f32 0
%f32_1 = OpConstant %f32 1
%u32_0 = OpConstant %u32 0
%u32_1 = OpConstant %u32 1
%u32_4 = OpConstant %u32 4
%u64_0 = OpConstant %u64 0
%u64_1 = OpConstant %u64 1

%cross_device = OpConstant %u32 0
%device = OpConstant %u32 1
%workgroup = OpConstant %u32 2
%subgroup = OpConstant %u32 3
%invocation = OpConstant %u32 4

%none = OpConstant %u32 0
%acquire = OpConstant %u32 2
%release = OpConstant %u32 4
%acquire_release = OpConstant %u32 8
%acquire_and_release = OpConstant %u32 6
%sequentially_consistent = OpConstant %u32 16
%acquire_release_uniform_workgroup = OpConstant %u32 328
%acquire_and_release_uniform = OpConstant %u32 70
%acquire_release_subgroup = OpConstant %u32 136
%uniform = OpConstant %u32 64

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
    const std::string& capabilities_and_extensions = "") {
  std::ostringstream ss;
  ss << R"(
OpCapability Addresses
OpCapability Kernel
OpCapability Linkage
OpCapability Int64
OpCapability NamedBarrier
)";

  ss << capabilities_and_extensions;
  ss << R"(
OpMemoryModel Physical32 OpenCL
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f32 = OpTypeFloat 32
%u32 = OpTypeInt 32 0
%u64 = OpTypeInt 64 0

%f32_0 = OpConstant %f32 0
%f32_1 = OpConstant %f32 1
%f32_4 = OpConstant %f32 4
%u32_0 = OpConstant %u32 0
%u32_1 = OpConstant %u32 1
%u32_4 = OpConstant %u32 4
%u64_0 = OpConstant %u64 0
%u64_1 = OpConstant %u64 1
%u64_4 = OpConstant %u64 4

%cross_device = OpConstant %u32 0
%device = OpConstant %u32 1
%workgroup = OpConstant %u32 2
%subgroup = OpConstant %u32 3
%invocation = OpConstant %u32 4

%none = OpConstant %u32 0
%acquire = OpConstant %u32 2
%release = OpConstant %u32 4
%acquire_release = OpConstant %u32 8
%acquire_and_release = OpConstant %u32 6
%sequentially_consistent = OpConstant %u32 16
%acquire_release_uniform_workgroup = OpConstant %u32 328
%acquire_and_release_uniform = OpConstant %u32 70
%uniform = OpConstant %u32 64

%named_barrier = OpTypeNamedBarrier

%main = OpFunction %void None %func
%main_entry = OpLabel
)";

  ss << body;

  ss << R"(
OpReturn
OpFunctionEnd)";

  return ss.str();
}

TEST_F(ValidateBarriers, OpControlBarrierGLComputeSuccess) {
  const std::string body = R"(
OpControlBarrier %device %device %none
OpControlBarrier %workgroup %workgroup %acquire
OpControlBarrier %workgroup %device %release
OpControlBarrier %cross_device %cross_device %acquire_release
OpControlBarrier %cross_device %cross_device %sequentially_consistent
OpControlBarrier %cross_device %cross_device %acquire_release_uniform_workgroup
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateBarriers, OpControlBarrierKernelSuccess) {
  const std::string body = R"(
OpControlBarrier %device %device %none
OpControlBarrier %workgroup %workgroup %acquire
OpControlBarrier %workgroup %device %release
OpControlBarrier %cross_device %cross_device %acquire_release
OpControlBarrier %cross_device %cross_device %sequentially_consistent
OpControlBarrier %cross_device %cross_device %acquire_release_uniform_workgroup
)";

  CompileSuccessfully(GenerateKernelCode(body), SPV_ENV_UNIVERSAL_1_1);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_1));
}

TEST_F(ValidateBarriers, OpControlBarrierTesselationControlSuccess) {
  const std::string body = R"(
OpControlBarrier %device %device %none
OpControlBarrier %workgroup %workgroup %acquire
OpControlBarrier %workgroup %device %release
OpControlBarrier %cross_device %cross_device %acquire_release
OpControlBarrier %cross_device %cross_device %sequentially_consistent
OpControlBarrier %cross_device %cross_device %acquire_release_uniform_workgroup
)";

  CompileSuccessfully(GenerateShaderCode(body, "OpCapability Tessellation\n",
                                         "TessellationControl"));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateBarriers, OpControlBarrierVulkanSuccess) {
  const std::string body = R"(
OpControlBarrier %workgroup %device %none
OpControlBarrier %workgroup %workgroup %acquire_release_uniform_workgroup
)";

  CompileSuccessfully(GenerateShaderCode(body), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateBarriers, OpControlBarrierExecutionModelFragmentSpirv12) {
  const std::string body = R"(
OpControlBarrier %device %device %none
)";

  CompileSuccessfully(GenerateShaderCode(body, "", "Fragment"),
                      SPV_ENV_UNIVERSAL_1_2);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_2));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpControlBarrier requires one of the following Execution "
                "Models: TessellationControl, GLCompute or Kernel"));
}

TEST_F(ValidateBarriers, OpControlBarrierExecutionModelFragmentSpirv13) {
  const std::string body = R"(
OpControlBarrier %device %device %none
)";

  CompileSuccessfully(GenerateShaderCode(body, "", "Fragment"),
                      SPV_ENV_UNIVERSAL_1_3);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
}

TEST_F(ValidateBarriers, OpControlBarrierFloatExecutionScope) {
  const std::string body = R"(
OpControlBarrier %f32_1 %device %none
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("ControlBarrier: expected Execution Scope to be a 32-bit int"));
}

TEST_F(ValidateBarriers, OpControlBarrierU64ExecutionScope) {
  const std::string body = R"(
OpControlBarrier %u64_1 %device %none
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("ControlBarrier: expected Execution Scope to be a 32-bit int"));
}

TEST_F(ValidateBarriers, OpControlBarrierFloatMemoryScope) {
  const std::string body = R"(
OpControlBarrier %device %f32_1 %none
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("ControlBarrier: expected Memory Scope to be a 32-bit int"));
}

TEST_F(ValidateBarriers, OpControlBarrierU64MemoryScope) {
  const std::string body = R"(
OpControlBarrier %device %u64_1 %none
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("ControlBarrier: expected Memory Scope to be a 32-bit int"));
}

TEST_F(ValidateBarriers, OpControlBarrierFloatMemorySemantics) {
  const std::string body = R"(
OpControlBarrier %device %device %f32_0
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "ControlBarrier: expected Memory Semantics to be a 32-bit int"));
}

TEST_F(ValidateBarriers, OpControlBarrierU64MemorySemantics) {
  const std::string body = R"(
OpControlBarrier %device %device %u64_0
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "ControlBarrier: expected Memory Semantics to be a 32-bit int"));
}

TEST_F(ValidateBarriers, OpControlBarrierVulkanExecutionScopeDevice) {
  const std::string body = R"(
OpControlBarrier %device %workgroup %none
)";

  CompileSuccessfully(GenerateShaderCode(body), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("ControlBarrier: in Vulkan environment Execution Scope "
                        "is limited to Workgroup and Subgroup"));
}

TEST_F(ValidateBarriers, OpControlBarrierVulkanMemoryScopeSubgroup) {
  const std::string body = R"(
OpControlBarrier %subgroup %subgroup %none
)";

  CompileSuccessfully(GenerateShaderCode(body), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("ControlBarrier: in Vulkan 1.0 environment Memory Scope is "
                "limited to Device, Workgroup and Invocation"));
}

TEST_F(ValidateBarriers, OpControlBarrierVulkan1p1MemoryScopeSubgroup) {
  const std::string body = R"(
OpControlBarrier %subgroup %subgroup %none
)";

  CompileSuccessfully(GenerateShaderCode(body), SPV_ENV_VULKAN_1_1);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_1));
}

TEST_F(ValidateBarriers, OpControlBarrierVulkan1p1MemoryScopeCrossDevice) {
  const std::string body = R"(
OpControlBarrier %subgroup %cross_device %none
)";

  CompileSuccessfully(GenerateShaderCode(body), SPV_ENV_VULKAN_1_1);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("ControlBarrier: in Vulkan environment, Memory Scope "
                        "cannot be CrossDevice"));
}

TEST_F(ValidateBarriers, OpControlBarrierAcquireAndRelease) {
  const std::string body = R"(
OpControlBarrier %device %device %acquire_and_release_uniform
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("ControlBarrier: Memory Semantics can have at most one "
                        "of the following bits set: Acquire, Release, "
                        "AcquireRelease or SequentiallyConsistent"));
}

// TODO(atgoo@github.com): the corresponding check fails Vulkan CTS,
// reenable once fixed.
TEST_F(ValidateBarriers, DISABLED_OpControlBarrierVulkanSubgroupStorageClass) {
  const std::string body = R"(
OpControlBarrier %workgroup %device %acquire_release_subgroup
)";

  CompileSuccessfully(GenerateShaderCode(body), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "ControlBarrier: expected Memory Semantics to include a "
          "Vulkan-supported storage class if Memory Semantics is not None"));
}

TEST_F(ValidateBarriers, OpControlBarrierSubgroupExecutionFragment1p1) {
  const std::string body = R"(
OpControlBarrier %subgroup %subgroup %acquire_release_subgroup
)";

  CompileSuccessfully(GenerateShaderCode(body, "", "Fragment"),
                      SPV_ENV_VULKAN_1_1);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_1));
}

TEST_F(ValidateBarriers, OpControlBarrierWorkgroupExecutionFragment1p1) {
  const std::string body = R"(
OpControlBarrier %workgroup %workgroup %acquire_release
)";

  CompileSuccessfully(GenerateShaderCode(body, "", "Fragment"),
                      SPV_ENV_VULKAN_1_1);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpControlBarrier execution scope must be Subgroup for "
                        "Fragment, Vertex, Geometry and TessellationEvaluation "
                        "execution models"));
}

TEST_F(ValidateBarriers, OpControlBarrierSubgroupExecutionFragment1p0) {
  const std::string body = R"(
OpControlBarrier %subgroup %workgroup %acquire_release
)";

  CompileSuccessfully(GenerateShaderCode(body, "", "Fragment"),
                      SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpControlBarrier requires one of the following Execution "
                "Models: TessellationControl, GLCompute or Kernel"));
}

TEST_F(ValidateBarriers, OpControlBarrierSubgroupExecutionVertex1p1) {
  const std::string body = R"(
OpControlBarrier %subgroup %subgroup %acquire_release_subgroup
)";

  CompileSuccessfully(GenerateShaderCode(body, "", "Vertex"),
                      SPV_ENV_VULKAN_1_1);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_1));
}

TEST_F(ValidateBarriers, OpControlBarrierWorkgroupExecutionVertex1p1) {
  const std::string body = R"(
OpControlBarrier %workgroup %workgroup %acquire_release
)";

  CompileSuccessfully(GenerateShaderCode(body, "", "Vertex"),
                      SPV_ENV_VULKAN_1_1);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpControlBarrier execution scope must be Subgroup for "
                        "Fragment, Vertex, Geometry and TessellationEvaluation "
                        "execution models"));
}

TEST_F(ValidateBarriers, OpControlBarrierSubgroupExecutionVertex1p0) {
  const std::string body = R"(
OpControlBarrier %subgroup %workgroup %acquire_release
)";

  CompileSuccessfully(GenerateShaderCode(body, "", "Vertex"),
                      SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpControlBarrier requires one of the following Execution "
                "Models: TessellationControl, GLCompute or Kernel"));
}

TEST_F(ValidateBarriers, OpControlBarrierSubgroupExecutionGeometry1p1) {
  const std::string body = R"(
OpControlBarrier %subgroup %subgroup %acquire_release_subgroup
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability Geometry\n", "Geometry"),
      SPV_ENV_VULKAN_1_1);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_1));
}

TEST_F(ValidateBarriers, OpControlBarrierWorkgroupExecutionGeometry1p1) {
  const std::string body = R"(
OpControlBarrier %workgroup %workgroup %acquire_release
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability Geometry\n", "Geometry"),
      SPV_ENV_VULKAN_1_1);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpControlBarrier execution scope must be Subgroup for "
                        "Fragment, Vertex, Geometry and TessellationEvaluation "
                        "execution models"));
}

TEST_F(ValidateBarriers, OpControlBarrierSubgroupExecutionGeometry1p0) {
  const std::string body = R"(
OpControlBarrier %subgroup %workgroup %acquire_release
)";

  CompileSuccessfully(
      GenerateShaderCode(body, "OpCapability Geometry\n", "Geometry"),
      SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpControlBarrier requires one of the following Execution "
                "Models: TessellationControl, GLCompute or Kernel"));
}

TEST_F(ValidateBarriers,
       OpControlBarrierSubgroupExecutionTessellationEvaluation1p1) {
  const std::string body = R"(
OpControlBarrier %subgroup %subgroup %acquire_release_subgroup
)";

  CompileSuccessfully(GenerateShaderCode(body, "OpCapability Tessellation\n",
                                         "TessellationEvaluation"),
                      SPV_ENV_VULKAN_1_1);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_1));
}

TEST_F(ValidateBarriers,
       OpControlBarrierWorkgroupExecutionTessellationEvaluation1p1) {
  const std::string body = R"(
OpControlBarrier %workgroup %workgroup %acquire_release
)";

  CompileSuccessfully(GenerateShaderCode(body, "OpCapability Tessellation\n",
                                         "TessellationEvaluation"),
                      SPV_ENV_VULKAN_1_1);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpControlBarrier execution scope must be Subgroup for "
                        "Fragment, Vertex, Geometry and TessellationEvaluation "
                        "execution models"));
}

TEST_F(ValidateBarriers,
       OpControlBarrierSubgroupExecutionTessellationEvaluation1p0) {
  const std::string body = R"(
OpControlBarrier %subgroup %workgroup %acquire_release
)";

  CompileSuccessfully(GenerateShaderCode(body, "OpCapability Tessellation\n",
                                         "TessellationEvaluation"),
                      SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpControlBarrier requires one of the following Execution "
                "Models: TessellationControl, GLCompute or Kernel"));
}

TEST_F(ValidateBarriers, OpMemoryBarrierSuccess) {
  const std::string body = R"(
OpMemoryBarrier %cross_device %acquire_release_uniform_workgroup
OpMemoryBarrier %device %uniform
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateBarriers, OpMemoryBarrierKernelSuccess) {
  const std::string body = R"(
OpMemoryBarrier %cross_device %acquire_release_uniform_workgroup
OpMemoryBarrier %device %uniform
)";

  CompileSuccessfully(GenerateKernelCode(body), SPV_ENV_UNIVERSAL_1_1);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_1));
}

TEST_F(ValidateBarriers, OpMemoryBarrierVulkanSuccess) {
  const std::string body = R"(
OpMemoryBarrier %workgroup %acquire_release_uniform_workgroup
)";

  CompileSuccessfully(GenerateShaderCode(body), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateBarriers, OpMemoryBarrierFloatMemoryScope) {
  const std::string body = R"(
OpMemoryBarrier %f32_1 %acquire_release_uniform_workgroup
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("MemoryBarrier: expected Memory Scope to be a 32-bit int"));
}

TEST_F(ValidateBarriers, OpMemoryBarrierU64MemoryScope) {
  const std::string body = R"(
OpMemoryBarrier %u64_1 %acquire_release_uniform_workgroup
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("MemoryBarrier: expected Memory Scope to be a 32-bit int"));
}

TEST_F(ValidateBarriers, OpMemoryBarrierFloatMemorySemantics) {
  const std::string body = R"(
OpMemoryBarrier %device %f32_0
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("MemoryBarrier: expected Memory Semantics to be a 32-bit int"));
}

TEST_F(ValidateBarriers, OpMemoryBarrierU64MemorySemantics) {
  const std::string body = R"(
OpMemoryBarrier %device %u64_0
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("MemoryBarrier: expected Memory Semantics to be a 32-bit int"));
}

TEST_F(ValidateBarriers, OpMemoryBarrierVulkanMemoryScopeSubgroup) {
  const std::string body = R"(
OpMemoryBarrier %subgroup %acquire_release_uniform_workgroup
)";

  CompileSuccessfully(GenerateShaderCode(body), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("MemoryBarrier: in Vulkan 1.0 environment Memory Scope is "
                "limited to Device, Workgroup and Invocation"));
}

TEST_F(ValidateBarriers, OpMemoryBarrierVulkan1p1MemoryScopeSubgroup) {
  const std::string body = R"(
OpMemoryBarrier %subgroup %acquire_release_uniform_workgroup
)";

  CompileSuccessfully(GenerateShaderCode(body), SPV_ENV_VULKAN_1_1);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_1));
}

TEST_F(ValidateBarriers, OpMemoryBarrierAcquireAndRelease) {
  const std::string body = R"(
OpMemoryBarrier %device %acquire_and_release_uniform
)";

  CompileSuccessfully(GenerateShaderCode(body));
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("MemoryBarrier: Memory Semantics can have at most one "
                        "of the following bits set: Acquire, Release, "
                        "AcquireRelease or SequentiallyConsistent"));
}

TEST_F(ValidateBarriers, OpMemoryBarrierVulkanMemorySemanticsNone) {
  const std::string body = R"(
OpMemoryBarrier %device %none
)";

  CompileSuccessfully(GenerateShaderCode(body), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("MemoryBarrier: Vulkan specification requires Memory Semantics "
                "to have one of the following bits set: Acquire, Release, "
                "AcquireRelease or SequentiallyConsistent"));
}

TEST_F(ValidateBarriers, OpMemoryBarrierVulkanMemorySemanticsAcquire) {
  const std::string body = R"(
OpMemoryBarrier %device %acquire
)";

  CompileSuccessfully(GenerateShaderCode(body), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("MemoryBarrier: expected Memory Semantics to include a "
                        "Vulkan-supported storage class"));
}

TEST_F(ValidateBarriers, OpMemoryBarrierVulkanSubgroupStorageClass) {
  const std::string body = R"(
OpMemoryBarrier %device %acquire_release_subgroup
)";

  CompileSuccessfully(GenerateShaderCode(body), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("MemoryBarrier: expected Memory Semantics to include a "
                        "Vulkan-supported storage class"));
}

TEST_F(ValidateBarriers, OpNamedBarrierInitializeSuccess) {
  const std::string body = R"(
%barrier = OpNamedBarrierInitialize %named_barrier %u32_4
)";

  CompileSuccessfully(GenerateKernelCode(body), SPV_ENV_UNIVERSAL_1_1);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_1));
}

TEST_F(ValidateBarriers, OpNamedBarrierInitializeWrongResultType) {
  const std::string body = R"(
%barrier = OpNamedBarrierInitialize %u32 %u32_4
)";

  CompileSuccessfully(GenerateKernelCode(body), SPV_ENV_UNIVERSAL_1_1);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_1));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("NamedBarrierInitialize: expected Result Type to be "
                        "OpTypeNamedBarrier"));
}

TEST_F(ValidateBarriers, OpNamedBarrierInitializeFloatSubgroupCount) {
  const std::string body = R"(
%barrier = OpNamedBarrierInitialize %named_barrier %f32_4
)";

  CompileSuccessfully(GenerateKernelCode(body), SPV_ENV_UNIVERSAL_1_1);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_1));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("NamedBarrierInitialize: expected Subgroup Count to be "
                        "a 32-bit int"));
}

TEST_F(ValidateBarriers, OpNamedBarrierInitializeU64SubgroupCount) {
  const std::string body = R"(
%barrier = OpNamedBarrierInitialize %named_barrier %u64_4
)";

  CompileSuccessfully(GenerateKernelCode(body), SPV_ENV_UNIVERSAL_1_1);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_1));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("NamedBarrierInitialize: expected Subgroup Count to be "
                        "a 32-bit int"));
}

TEST_F(ValidateBarriers, OpMemoryNamedBarrierSuccess) {
  const std::string body = R"(
%barrier = OpNamedBarrierInitialize %named_barrier %u32_4
OpMemoryNamedBarrier %barrier %workgroup %acquire_release_uniform_workgroup
)";

  CompileSuccessfully(GenerateKernelCode(body), SPV_ENV_UNIVERSAL_1_1);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_1));
}

TEST_F(ValidateBarriers, OpMemoryNamedBarrierNotNamedBarrier) {
  const std::string body = R"(
OpMemoryNamedBarrier %u32_1 %workgroup %acquire_release_uniform_workgroup
)";

  CompileSuccessfully(GenerateKernelCode(body), SPV_ENV_UNIVERSAL_1_1);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_1));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("MemoryNamedBarrier: expected Named Barrier to be of "
                        "type OpTypeNamedBarrier"));
}

TEST_F(ValidateBarriers, OpMemoryNamedBarrierFloatMemoryScope) {
  const std::string body = R"(
%barrier = OpNamedBarrierInitialize %named_barrier %u32_4
OpMemoryNamedBarrier %barrier %f32_1 %acquire_release_uniform_workgroup
)";

  CompileSuccessfully(GenerateKernelCode(body), SPV_ENV_UNIVERSAL_1_1);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_1));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "MemoryNamedBarrier: expected Memory Scope to be a 32-bit int"));
}

TEST_F(ValidateBarriers, OpMemoryNamedBarrierFloatMemorySemantics) {
  const std::string body = R"(
%barrier = OpNamedBarrierInitialize %named_barrier %u32_4
OpMemoryNamedBarrier %barrier %workgroup %f32_0
)";

  CompileSuccessfully(GenerateKernelCode(body), SPV_ENV_UNIVERSAL_1_1);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_1));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "MemoryNamedBarrier: expected Memory Semantics to be a 32-bit int"));
}

TEST_F(ValidateBarriers, OpMemoryNamedBarrierAcquireAndRelease) {
  const std::string body = R"(
%barrier = OpNamedBarrierInitialize %named_barrier %u32_4
OpMemoryNamedBarrier %barrier %workgroup %acquire_and_release_uniform
)";

  CompileSuccessfully(GenerateKernelCode(body), SPV_ENV_UNIVERSAL_1_1);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_1));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("MemoryNamedBarrier: Memory Semantics can have at most "
                        "one of the following bits set: Acquire, Release, "
                        "AcquireRelease or SequentiallyConsistent"));
}

TEST_F(ValidateBarriers, TypeAsMemoryScope) {
  const std::string body = R"(
OpMemoryBarrier %u32 %u32_0
)";

  CompileSuccessfully(GenerateKernelCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("MemoryBarrier: expected Memory Scope to be a 32-bit int"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
