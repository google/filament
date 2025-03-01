// Copyright (c) 2024 The Khronos Group Inc.
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

#include <string>

#include "gmock/gmock.h"
#include "source/spirv_target_env.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;

using ValidateSpvNVRawAccessChains = spvtest::ValidateBase<bool>;

TEST_F(ValidateSpvNVRawAccessChains, Valid) {
  const std::string str = R"(
    OpCapability Shader
    OpCapability RawAccessChainsNV
    OpExtension "SPV_KHR_storage_buffer_storage_class"
    OpExtension "SPV_NV_raw_access_chains"
    OpMemoryModel Logical GLSL450

    OpEntryPoint GLCompute %main "main"
    OpExecutionMode %main LocalSize 1 1 1

    OpDecorate %intStruct Block
    OpMemberDecorate %intStruct 0 Offset 0
    OpDecorate %ssbo DescriptorSet 0
    OpDecorate %ssbo Binding 0

    %int = OpTypeInt 32 1
    %void = OpTypeVoid
    %mainFunctionType = OpTypeFunction %void
    %intStruct = OpTypeStruct %int
    %intStructPtr = OpTypePointer StorageBuffer %intStruct
    %ssbo = OpVariable %intStructPtr StorageBuffer
    %intPtr = OpTypePointer StorageBuffer %int

    %int_0 = OpConstant %int 0
    %int_16 = OpConstant %int 16

    %main = OpFunction %void None %mainFunctionType
    %label = OpLabel
    %rawChain = OpRawAccessChainNV %intPtr %ssbo %int_16 %int_0 %int_0 RobustnessPerComponentNV
    %unused = OpLoad %int %rawChain
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateSpvNVRawAccessChains, NoCapability) {
  const std::string str = R"(
    OpCapability Shader
    OpExtension "SPV_KHR_storage_buffer_storage_class"
    OpExtension "SPV_NV_raw_access_chains"
    OpMemoryModel Logical GLSL450

    OpEntryPoint GLCompute %main "main"
    OpExecutionMode %main LocalSize 1 1 1

    OpDecorate %intStruct Block
    OpMemberDecorate %intStruct 0 Offset 0
    OpDecorate %ssbo DescriptorSet 0
    OpDecorate %ssbo Binding 0

    %int = OpTypeInt 32 1
    %void = OpTypeVoid
    %mainFunctionType = OpTypeFunction %void
    %intStruct = OpTypeStruct %int
    %intStructPtr = OpTypePointer StorageBuffer %intStruct
    %ssbo = OpVariable %intStructPtr StorageBuffer
    %intPtr = OpTypePointer StorageBuffer %int

    %int_0 = OpConstant %int 0
    %int_16 = OpConstant %int 16

    %main = OpFunction %void None %mainFunctionType
    %label = OpLabel
    %rawChain = OpRawAccessChainNV %intPtr %ssbo %int_16 %int_0 %int_0 RobustnessPerComponentNV
    %unused = OpLoad %int %rawChain
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_CAPABILITY, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("requires one of these capabilities: RawAccessChainsNV"));
}

TEST_F(ValidateSpvNVRawAccessChains, NoExtension) {
  const std::string str = R"(
    OpCapability Shader
    OpCapability RawAccessChainsNV
    OpExtension "SPV_KHR_storage_buffer_storage_class"
    OpMemoryModel Logical GLSL450

    OpEntryPoint GLCompute %main "main"
    OpExecutionMode %main LocalSize 1 1 1

    OpDecorate %intStruct Block
    OpMemberDecorate %intStruct 0 Offset 0
    OpDecorate %ssbo DescriptorSet 0
    OpDecorate %ssbo Binding 0

    %int = OpTypeInt 32 1
    %void = OpTypeVoid
    %mainFunctionType = OpTypeFunction %void
    %intStruct = OpTypeStruct %int
    %intStructPtr = OpTypePointer StorageBuffer %intStruct
    %ssbo = OpVariable %intStructPtr StorageBuffer
    %intPtr = OpTypePointer StorageBuffer %int

    %int_0 = OpConstant %int 0
    %int_16 = OpConstant %int 16

    %main = OpFunction %void None %mainFunctionType
    %label = OpLabel
    %rawChain = OpRawAccessChainNV %intPtr %ssbo %int_16 %int_0 %int_0 RobustnessPerComponentNV
    %unused = OpLoad %int %rawChain
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_ERROR_MISSING_EXTENSION, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("requires one of these extensions: SPV_NV_raw_access_chains"));
}

TEST_F(ValidateSpvNVRawAccessChains, ReturnTypeNotPointer) {
  const std::string str = R"(
    OpCapability Shader
    OpCapability RawAccessChainsNV
    OpExtension "SPV_KHR_storage_buffer_storage_class"
    OpExtension "SPV_NV_raw_access_chains"
    OpMemoryModel Logical GLSL450

    OpEntryPoint GLCompute %main "main"
    OpExecutionMode %main LocalSize 1 1 1

    OpDecorate %intStruct Block
    OpMemberDecorate %intStruct 0 Offset 0
    OpDecorate %ssbo DescriptorSet 0
    OpDecorate %ssbo Binding 0

    %int = OpTypeInt 32 1
    %void = OpTypeVoid
    %mainFunctionType = OpTypeFunction %void
    %intStruct = OpTypeStruct %int
    %intStructPtr = OpTypePointer StorageBuffer %intStruct
    %ssbo = OpVariable %intStructPtr StorageBuffer

    %int_0 = OpConstant %int 0
    %int_16 = OpConstant %int 16

    %main = OpFunction %void None %mainFunctionType
    %label = OpLabel
    %rawChain = OpRawAccessChainNV %int %ssbo %int_16 %int_0 %int_0 RobustnessPerComponentNV
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be OpTypePointer. Found OpTypeInt"));
}

TEST_F(ValidateSpvNVRawAccessChains, Workgroup) {
  const std::string str = R"(
    OpCapability Shader
    OpCapability RawAccessChainsNV
    OpExtension "SPV_KHR_storage_buffer_storage_class"
    OpExtension "SPV_NV_raw_access_chains"
    OpMemoryModel Logical GLSL450

    OpEntryPoint GLCompute %main "main"
    OpExecutionMode %main LocalSize 1 1 1

    OpDecorate %intStruct Block
    OpMemberDecorate %intStruct 0 Offset 0

    %int = OpTypeInt 32 1
    %void = OpTypeVoid
    %mainFunctionType = OpTypeFunction %void
    %intStruct = OpTypeStruct %int
    %intStructPtr = OpTypePointer Workgroup %intStruct
    %ssbo = OpVariable %intStructPtr Workgroup
    %intPtr = OpTypePointer Workgroup %int

    %int_0 = OpConstant %int 0
    %int_16 = OpConstant %int 16

    %main = OpFunction %void None %mainFunctionType
    %label = OpLabel
    %rawChain = OpRawAccessChainNV %intPtr %ssbo %int_16 %int_0 %int_0 RobustnessPerComponentNV
    %unused = OpLoad %int %rawChain
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must point to a storage class of"));
}

TEST_F(ValidateSpvNVRawAccessChains, ReturnTypeArray) {
  const std::string str = R"(
    OpCapability Shader
    OpCapability RawAccessChainsNV
    OpExtension "SPV_KHR_storage_buffer_storage_class"
    OpExtension "SPV_NV_raw_access_chains"
    OpMemoryModel Logical GLSL450

    OpEntryPoint GLCompute %main "main"
    OpExecutionMode %main LocalSize 1 1 1

    OpDecorate %intStruct Block
    OpMemberDecorate %intStruct 0 Offset 0
    OpDecorate %ssbo DescriptorSet 0
    OpDecorate %ssbo Binding 0

    %int = OpTypeInt 32 1
    %void = OpTypeVoid
    %mainFunctionType = OpTypeFunction %void
    %intStruct = OpTypeStruct %int
    %intStructPtr = OpTypePointer StorageBuffer %intStruct
    %ssbo = OpVariable %intStructPtr StorageBuffer
    %int_1 = OpConstant %int 1
    %intArray = OpTypeArray %int %int_1
    %intArrayPtr = OpTypePointer StorageBuffer %intArray

    %int_0 = OpConstant %int 0
    %int_16 = OpConstant %int 16

    %main = OpFunction %void None %mainFunctionType
    %label = OpLabel
    %rawChain = OpRawAccessChainNV %intArrayPtr %ssbo %int_16 %int_0 %int_0 RobustnessPerComponentNV
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("must not point to"));
}

TEST_F(ValidateSpvNVRawAccessChains, VariableStride) {
  const std::string str = R"(
    OpCapability Shader
    OpCapability RawAccessChainsNV
    OpExtension "SPV_KHR_storage_buffer_storage_class"
    OpExtension "SPV_NV_raw_access_chains"
    OpMemoryModel Logical GLSL450

    OpEntryPoint GLCompute %main "main"
    OpExecutionMode %main LocalSize 1 1 1

    OpDecorate %intStruct Block
    OpMemberDecorate %intStruct 0 Offset 0
    OpDecorate %ssbo DescriptorSet 0
    OpDecorate %ssbo Binding 0

    %int = OpTypeInt 32 1
    %void = OpTypeVoid
    %mainFunctionType = OpTypeFunction %void
    %intStruct = OpTypeStruct %int
    %intStructPtr = OpTypePointer StorageBuffer %intStruct
    %ssbo = OpVariable %intStructPtr StorageBuffer
    %intPtr = OpTypePointer StorageBuffer %int

    %int_0 = OpConstant %int 0

    %main = OpFunction %void None %mainFunctionType
    %label = OpLabel
    %stride = OpIAdd %int %int_0 %int_0
    %rawChain = OpRawAccessChainNV %intPtr %ssbo %stride %int_0 %int_0 RobustnessPerComponentNV
    %unused = OpLoad %int %rawChain
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("must be OpConstant"));
}

TEST_F(ValidateSpvNVRawAccessChains, RobustnessPerElementZeroStride) {
  const std::string str = R"(
    OpCapability Shader
    OpCapability RawAccessChainsNV
    OpExtension "SPV_KHR_storage_buffer_storage_class"
    OpExtension "SPV_NV_raw_access_chains"
    OpMemoryModel Logical GLSL450

    OpEntryPoint GLCompute %main "main"
    OpExecutionMode %main LocalSize 1 1 1

    OpDecorate %intStruct Block
    OpMemberDecorate %intStruct 0 Offset 0
    OpDecorate %ssbo DescriptorSet 0
    OpDecorate %ssbo Binding 0

    %int = OpTypeInt 32 1
    %void = OpTypeVoid
    %mainFunctionType = OpTypeFunction %void
    %intStruct = OpTypeStruct %int
    %intStructPtr = OpTypePointer StorageBuffer %intStruct
    %ssbo = OpVariable %intStructPtr StorageBuffer
    %intPtr = OpTypePointer StorageBuffer %int

    %int_0 = OpConstant %int 0

    %main = OpFunction %void None %mainFunctionType
    %label = OpLabel
    %rawChain = OpRawAccessChainNV %intPtr %ssbo %int_0 %int_0 %int_0 RobustnessPerElementNV
    %unused = OpLoad %int %rawChain
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Stride must not be zero when per-element robustness is used"));
}

TEST_F(ValidateSpvNVRawAccessChains, BothRobustness) {
  const std::string str = R"(
    OpCapability Shader
    OpCapability RawAccessChainsNV
    OpExtension "SPV_KHR_storage_buffer_storage_class"
    OpExtension "SPV_NV_raw_access_chains"
    OpMemoryModel Logical GLSL450

    OpEntryPoint GLCompute %main "main"
    OpExecutionMode %main LocalSize 1 1 1

    OpDecorate %intStruct Block
    OpMemberDecorate %intStruct 0 Offset 0
    OpDecorate %ssbo DescriptorSet 0
    OpDecorate %ssbo Binding 0

    %int = OpTypeInt 32 1
    %void = OpTypeVoid
    %mainFunctionType = OpTypeFunction %void
    %intStruct = OpTypeStruct %int
    %intStructPtr = OpTypePointer StorageBuffer %intStruct
    %ssbo = OpVariable %intStructPtr StorageBuffer
    %intPtr = OpTypePointer StorageBuffer %int

    %int_0 = OpConstant %int 0
    %int_16 = OpConstant %int 16

    %main = OpFunction %void None %mainFunctionType
    %label = OpLabel
    %rawChain = OpRawAccessChainNV %intPtr %ssbo %int_16 %int_0 %int_0 RobustnessPerElementNV|RobustnessPerComponentNV
    %unused = OpLoad %int %rawChain
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Per-component robustness and per-element robustness "
                        "are mutually exclusive"));
}

TEST_F(ValidateSpvNVRawAccessChains, StrideFloat) {
  const std::string str = R"(
    OpCapability Shader
    OpCapability RawAccessChainsNV
    OpExtension "SPV_KHR_storage_buffer_storage_class"
    OpExtension "SPV_NV_raw_access_chains"
    OpMemoryModel Logical GLSL450

    OpEntryPoint GLCompute %main "main"
    OpExecutionMode %main LocalSize 1 1 1

    OpDecorate %intStruct Block
    OpMemberDecorate %intStruct 0 Offset 0
    OpDecorate %ssbo DescriptorSet 0
    OpDecorate %ssbo Binding 0

    %int = OpTypeInt 32 1
    %float = OpTypeFloat 32
    %void = OpTypeVoid
    %mainFunctionType = OpTypeFunction %void
    %intStruct = OpTypeStruct %int
    %intStructPtr = OpTypePointer StorageBuffer %intStruct
    %ssbo = OpVariable %intStructPtr StorageBuffer
    %intPtr = OpTypePointer StorageBuffer %int

    %int_0 = OpConstant %int 0
    %float_16 = OpConstant %float 16

    %main = OpFunction %void None %mainFunctionType
    %label = OpLabel
    %rawChain = OpRawAccessChainNV %intPtr %ssbo %float_16 %int_0 %int_0 RobustnessPerComponentNV
    %unused = OpLoad %int %rawChain
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("must be OpTypeInt"));
}

TEST_F(ValidateSpvNVRawAccessChains, IndexType) {
  const std::string str = R"(
    OpCapability Shader
    OpCapability RawAccessChainsNV
    OpCapability Int64
    OpExtension "SPV_KHR_storage_buffer_storage_class"
    OpExtension "SPV_NV_raw_access_chains"
    OpMemoryModel Logical GLSL450

    OpEntryPoint GLCompute %main "main"
    OpExecutionMode %main LocalSize 1 1 1

    OpDecorate %intStruct Block
    OpMemberDecorate %intStruct 0 Offset 0
    OpDecorate %ssbo DescriptorSet 0
    OpDecorate %ssbo Binding 0

    %int = OpTypeInt 32 1
    %long = OpTypeInt 64 1
    %void = OpTypeVoid
    %mainFunctionType = OpTypeFunction %void
    %intStruct = OpTypeStruct %int
    %intStructPtr = OpTypePointer StorageBuffer %intStruct
    %ssbo = OpVariable %intStructPtr StorageBuffer
    %intPtr = OpTypePointer StorageBuffer %int

    %int_0 = OpConstant %int 0
    %int_16 = OpConstant %int 16
    %long_0 = OpConstant %long 0

    %main = OpFunction %void None %mainFunctionType
    %label = OpLabel
    %rawChain = OpRawAccessChainNV %intPtr %ssbo %int_16 %long_0 %int_0 RobustnessPerComponentNV
    %unused = OpLoad %int %rawChain
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("The integer width of Index"));
}

TEST_F(ValidateSpvNVRawAccessChains, OffsetType) {
  const std::string str = R"(
    OpCapability Shader
    OpCapability RawAccessChainsNV
    OpCapability Int64
    OpExtension "SPV_KHR_storage_buffer_storage_class"
    OpExtension "SPV_NV_raw_access_chains"
    OpMemoryModel Logical GLSL450

    OpEntryPoint GLCompute %main "main"
    OpExecutionMode %main LocalSize 1 1 1

    OpDecorate %intStruct Block
    OpMemberDecorate %intStruct 0 Offset 0
    OpDecorate %ssbo DescriptorSet 0
    OpDecorate %ssbo Binding 0

    %int = OpTypeInt 32 1
    %long = OpTypeInt 64 1
    %void = OpTypeVoid
    %mainFunctionType = OpTypeFunction %void
    %intStruct = OpTypeStruct %int
    %intStructPtr = OpTypePointer StorageBuffer %intStruct
    %ssbo = OpVariable %intStructPtr StorageBuffer
    %intPtr = OpTypePointer StorageBuffer %int

    %int_0 = OpConstant %int 0
    %int_16 = OpConstant %int 16
    %long_0 = OpConstant %long 0

    %main = OpFunction %void None %mainFunctionType
    %label = OpLabel
    %rawChain = OpRawAccessChainNV %intPtr %ssbo %int_16 %int_0 %long_0 RobustnessPerComponentNV
    %unused = OpLoad %int %rawChain
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("The integer width of Offset"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
