// Copyright (c) 2021 Google Inc.
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
#include "source/spirv_target_env.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::Values;
using ::testing::ValuesIn;

using ValidateSpvKHRSubgroupUniformControlFlow = spvtest::ValidateBase<bool>;

TEST_F(ValidateSpvKHRSubgroupUniformControlFlow, Valid) {
  const std::string str = R"(
    OpCapability Shader
    OpExtension "SPV_KHR_subgroup_uniform_control_flow"
    OpMemoryModel Logical Simple
    OpEntryPoint GLCompute %main "main"
    OpExecutionMode %main LocalSize 1 1 1
    OpExecutionMode %main SubgroupUniformControlFlowKHR
    
    %void    = OpTypeVoid
    %void_fn = OpTypeFunction %void

    %main = OpFunction %void None %void_fn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateSpvKHRSubgroupUniformControlFlow, RequiresExtension) {
  const std::string str = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint GLCompute %main "main"
    OpExecutionMode %main LocalSize 1 1 1
    OpExecutionMode %main SubgroupUniformControlFlowKHR
    
    %void    = OpTypeVoid
    %void_fn = OpTypeFunction %void

    %main = OpFunction %void None %void_fn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("2nd operand of ExecutionMode: operand "
                        "SubgroupUniformControlFlowKHR(4421) "
                        "requires one of these extensions: "
                        "SPV_KHR_subgroup_uniform_control_flow"));
}

TEST_F(ValidateSpvKHRSubgroupUniformControlFlow, RequiresShaderCapability) {
  const std::string str = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpExtension "SPV_KHR_subgroup_uniform_control_flow"
    OpMemoryModel Physical32 OpenCL
    OpEntryPoint Kernel %main "main"
    OpExecutionMode %main SubgroupUniformControlFlowKHR
    
    %void    = OpTypeVoid
    %void_fn = OpTypeFunction %void

    %main = OpFunction %void None %void_fn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Operand 2 of ExecutionMode requires one of these "
                        "capabilities: Shader"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
