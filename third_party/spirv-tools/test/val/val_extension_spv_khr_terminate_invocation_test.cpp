// Copyright (c) 2020 Google LLC
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

// Tests for OpExtension validator rules.

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "source/enum_string_mapping.h"
#include "source/extensions.h"
#include "source/spirv_target_env.h"
#include "test/test_fixture.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::Values;
using ::testing::ValuesIn;

using ValidateSpvKHRTerminateInvocation = spvtest::ValidateBase<bool>;

TEST_F(ValidateSpvKHRTerminateInvocation, Valid) {
  const std::string str = R"(
    OpCapability Shader
    OpExtension "SPV_KHR_terminate_invocation"
    OpMemoryModel Logical Simple
    OpEntryPoint Fragment %main "main"
    OpExecutionMode %main OriginUpperLeft
    
    %void    = OpTypeVoid
    %void_fn = OpTypeFunction %void

    %main = OpFunction %void None %void_fn
    %entry = OpLabel
    OpTerminateInvocation
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateSpvKHRTerminateInvocation, RequiresExtensionPre1p6) {
  const std::string str = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Fragment %main "main"
    OpExecutionMode %main OriginUpperLeft
    
    %void    = OpTypeVoid
    %void_fn = OpTypeFunction %void

    %main = OpFunction %void None %void_fn
    %entry = OpLabel
    OpTerminateInvocation
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "TerminateInvocation requires SPIR-V version 1.6 at minimum or one "
          "of the following extensions: SPV_KHR_terminate_invocation"));
}

TEST_F(ValidateSpvKHRTerminateInvocation, RequiresNoExtensionPost1p6) {
  const std::string str = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Fragment %main "main"
    OpExecutionMode %main OriginUpperLeft
    
    %void    = OpTypeVoid
    %void_fn = OpTypeFunction %void

    %main = OpFunction %void None %void_fn
    %entry = OpLabel
    OpTerminateInvocation
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_UNIVERSAL_1_6);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
}

TEST_F(ValidateSpvKHRTerminateInvocation, RequiresShaderCapability) {
  const std::string str = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpExtension "SPV_KHR_terminate_invocation"
    OpMemoryModel Physical32 OpenCL
    OpEntryPoint Kernel %main "main"
    
    %void    = OpTypeVoid
    %void_fn = OpTypeFunction %void

    %main = OpFunction %void None %void_fn
    %entry = OpLabel
    OpTerminateInvocation
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "TerminateInvocation requires one of these capabilities: Shader \n"));
}

TEST_F(ValidateSpvKHRTerminateInvocation, RequiresFragmentShader) {
  const std::string str = R"(
    OpCapability Shader
    OpExtension "SPV_KHR_terminate_invocation"
    OpMemoryModel Logical Simple
    OpEntryPoint GLCompute %main "main"
    
    %void    = OpTypeVoid
    %void_fn = OpTypeFunction %void

    %main = OpFunction %void None %void_fn
    %entry = OpLabel
    OpTerminateInvocation
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpTerminateInvocation requires Fragment execution model"));
}

TEST_F(ValidateSpvKHRTerminateInvocation, IsTerminatorInstruction) {
  const std::string str = R"(
    OpCapability Shader
    OpExtension "SPV_KHR_terminate_invocation"
    OpMemoryModel Logical Simple
    OpEntryPoint GLCompute %main "main"
    
    %void    = OpTypeVoid
    %void_fn = OpTypeFunction %void

    %main = OpFunction %void None %void_fn
    %entry = OpLabel
    OpTerminateInvocation
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Return must appear in a block"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
