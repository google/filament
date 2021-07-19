// Copyright (c) 2015-2016 The Khronos Group Inc.
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

// Validation tests for OpVariable storage class

#include <sstream>
#include <string>
#include <tuple>

#include "gmock/gmock.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::Values;
using ValidateStorage = spvtest::ValidateBase<std::string>;
using ValidateStorageClass =
    spvtest::ValidateBase<std::tuple<std::string, bool, bool, std::string>>;
using ValidateStorageExecutionModel = spvtest::ValidateBase<std::string>;

TEST_F(ValidateStorage, FunctionStorageInsideFunction) {
  char str[] = R"(
          OpCapability Shader
          OpCapability Linkage
          OpMemoryModel Logical GLSL450
%intt   = OpTypeInt 32 1
%voidt  = OpTypeVoid
%vfunct = OpTypeFunction %voidt
%ptrt   = OpTypePointer Function %intt
%func   = OpFunction %voidt None %vfunct
%funcl  = OpLabel
%var    = OpVariable %ptrt Function
          OpReturn
          OpFunctionEnd
)";

  CompileSuccessfully(str);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateStorage, FunctionStorageOutsideFunction) {
  char str[] = R"(
          OpCapability Shader
          OpCapability Linkage
          OpMemoryModel Logical GLSL450
%intt   = OpTypeInt 32 1
%voidt  = OpTypeVoid
%vfunct = OpTypeFunction %voidt
%ptrt   = OpTypePointer Function %intt
%var    = OpVariable %ptrt Function
%func   = OpFunction %voidt None %vfunct
%funcl  = OpLabel
          OpReturn
          OpFunctionEnd
)";

  CompileSuccessfully(str);
  ASSERT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Variables can not have a function[7] storage class "
                        "outside of a function"));
}

TEST_F(ValidateStorage, OtherStorageOutsideFunction) {
  char str[] = R"(
              OpCapability Shader
              OpCapability Kernel
              OpCapability AtomicStorage
              OpCapability Linkage
              OpMemoryModel Logical GLSL450
%intt       = OpTypeInt 32 0
%voidt      = OpTypeVoid
%vfunct     = OpTypeFunction %voidt
%uniconptrt = OpTypePointer UniformConstant %intt
%unicon     = OpVariable %uniconptrt UniformConstant
%inputptrt  = OpTypePointer Input %intt
%input      = OpVariable %inputptrt Input
%unifptrt   = OpTypePointer Uniform %intt
%unif       = OpVariable %unifptrt Uniform
%outputptrt = OpTypePointer Output %intt
%output     = OpVariable %outputptrt Output
%wgroupptrt = OpTypePointer Workgroup %intt
%wgroup     = OpVariable %wgroupptrt Workgroup
%xwgrpptrt  = OpTypePointer CrossWorkgroup %intt
%xwgrp      = OpVariable %xwgrpptrt CrossWorkgroup
%privptrt   = OpTypePointer Private %intt
%priv       = OpVariable %privptrt Private
%pushcoptrt = OpTypePointer PushConstant %intt
%pushco     = OpVariable %pushcoptrt PushConstant
%atomcptrt  = OpTypePointer AtomicCounter %intt
%atomct     = OpVariable %atomcptrt AtomicCounter
%imageptrt  = OpTypePointer Image %intt
%image      = OpVariable %imageptrt Image
%func       = OpFunction %voidt None %vfunct
%funcl      = OpLabel
              OpReturn
              OpFunctionEnd
)";

  CompileSuccessfully(str);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

// clang-format off
TEST_P(ValidateStorage, OtherStorageInsideFunction) {
  std::stringstream ss;
  ss << R"(
          OpCapability Shader
          OpCapability Kernel
          OpCapability AtomicStorage
          OpCapability Linkage
          OpMemoryModel Logical GLSL450
%intt   = OpTypeInt 32 0
%voidt  = OpTypeVoid
%vfunct = OpTypeFunction %voidt
%ptrt   = OpTypePointer Function %intt
%func   = OpFunction %voidt None %vfunct
%funcl  = OpLabel
%var    = OpVariable %ptrt )" << GetParam() << R"(
          OpReturn
          OpFunctionEnd
)";

  CompileSuccessfully(ss.str());
  ASSERT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(
      "Variables must have a function[7] storage class inside of a function"));
}

INSTANTIATE_TEST_SUITE_P(MatrixOp, ValidateStorage,
                        ::testing::Values(
                             "Input",
                             "Uniform",
                             "Output",
                             "Workgroup",
                             "CrossWorkgroup",
                             "Private",
                             "PushConstant",
                             "AtomicCounter",
                             "Image"));
// clang-format on

TEST_F(ValidateStorage, GenericVariableOutsideFunction) {
  const auto str = R"(
          OpCapability Kernel
          OpCapability Linkage
          OpCapability GenericPointer
          OpMemoryModel Logical OpenCL
%intt   = OpTypeInt 32 0
%ptrt   = OpTypePointer Function %intt
%var    = OpVariable %ptrt Generic
)";
  CompileSuccessfully(str);
  ASSERT_EQ(SPV_ERROR_INVALID_BINARY, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpVariable storage class cannot be Generic"));
}

TEST_F(ValidateStorage, GenericVariableInsideFunction) {
  const auto str = R"(
          OpCapability Shader
          OpCapability Linkage
          OpCapability GenericPointer
          OpMemoryModel Logical GLSL450
%intt   = OpTypeInt 32 1
%voidt  = OpTypeVoid
%vfunct = OpTypeFunction %voidt
%ptrt   = OpTypePointer Function %intt
%func   = OpFunction %voidt None %vfunct
%funcl  = OpLabel
%var    = OpVariable %ptrt Generic
          OpReturn
          OpFunctionEnd
)";
  CompileSuccessfully(str);
  EXPECT_EQ(SPV_ERROR_INVALID_BINARY, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpVariable storage class cannot be Generic"));
}

TEST_F(ValidateStorage, RelaxedLogicalPointerFunctionParam) {
  const auto str = R"(
          OpCapability Shader
          OpCapability Linkage
          OpMemoryModel Logical GLSL450
%intt   = OpTypeInt 32 1
%voidt  = OpTypeVoid
%ptrt   = OpTypePointer Function %intt
%vfunct = OpTypeFunction %voidt
%vifunct = OpTypeFunction %voidt %ptrt
%wgroupptrt = OpTypePointer Workgroup %intt
%wgroup = OpVariable %wgroupptrt Workgroup
%main   = OpFunction %voidt None %vfunct
%mainl  = OpLabel
%ret    = OpFunctionCall %voidt %func %wgroup
          OpReturn
          OpFunctionEnd
%func   = OpFunction %voidt None %vifunct
%arg    = OpFunctionParameter %ptrt
%funcl  = OpLabel
          OpReturn
          OpFunctionEnd
)";
  CompileSuccessfully(str);
  getValidatorOptions()->before_hlsl_legalization = true;
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateStorage, RelaxedLogicalPointerFunctionParamBad) {
  const auto str = R"(
          OpCapability Shader
          OpCapability Linkage
          OpMemoryModel Logical GLSL450
%floatt = OpTypeFloat 32
%intt   = OpTypeInt 32 1
%voidt  = OpTypeVoid
%ptrt   = OpTypePointer Function %intt
%vfunct = OpTypeFunction %voidt
%vifunct = OpTypeFunction %voidt %ptrt
%wgroupptrt = OpTypePointer Workgroup %floatt
%wgroup = OpVariable %wgroupptrt Workgroup
%main   = OpFunction %voidt None %vfunct
%mainl  = OpLabel
%ret    = OpFunctionCall %voidt %func %wgroup
          OpReturn
          OpFunctionEnd
%func   = OpFunction %voidt None %vifunct
%arg    = OpFunctionParameter %ptrt
%funcl  = OpLabel
          OpReturn
          OpFunctionEnd
)";
  CompileSuccessfully(str);
  getValidatorOptions()->relax_logical_pointer = true;
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpFunctionCall Argument <id> '"));
}

TEST_P(ValidateStorageExecutionModel, VulkanOutsideStoreFailure) {
  std::stringstream ss;
  ss << R"(
              OpCapability Shader
              OpCapability RayTracingKHR
              OpExtension "SPV_KHR_ray_tracing"
              OpMemoryModel Logical GLSL450
              OpEntryPoint )"
     << GetParam() << R"(  %func "func" %output
              OpDecorate %output Location 0
%intt       = OpTypeInt 32 0
%int0       = OpConstant %intt 0
%voidt      = OpTypeVoid
%vfunct     = OpTypeFunction %voidt
%outputptrt = OpTypePointer Output %intt
%output     = OpVariable %outputptrt Output
%func       = OpFunction %voidt None %vfunct
%funcl      = OpLabel
              OpStore %output %int0
              OpReturn
              OpFunctionEnd
)";

  CompileSuccessfully(ss.str(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-None-04644"));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("in Vulkan evironment, Output Storage Class must not be used "
                "in GLCompute, RayGenerationKHR, IntersectionKHR, AnyHitKHR, "
                "ClosestHitKHR, MissKHR, or CallableKHR execution models"));
}

INSTANTIATE_TEST_SUITE_P(MatrixExecutionModel, ValidateStorageExecutionModel,
                         ::testing::Values("RayGenerationKHR",
                                           "IntersectionKHR", "AnyHitKHR",
                                           "ClosestHitKHR", "MissKHR",
                                           "CallableKHR"));

}  // namespace
}  // namespace val
}  // namespace spvtools
