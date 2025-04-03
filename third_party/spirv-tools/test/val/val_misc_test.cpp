// Copyright (c) 2019 Google LLC.
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

// Validation tests for misc instructions

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

using ValidateMisc = spvtest::ValidateBase<bool>;

TEST_F(ValidateMisc, UndefRestrictedShort) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpCapability StorageBuffer16BitAccess
OpExtension "SPV_KHR_16bit_storage"
OpMemoryModel Logical GLSL450
%short = OpTypeInt 16 0
%undef = OpUndef %short
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Cannot create undefined values with 8- or 16-bit types"));
}

TEST_F(ValidateMisc, UndefRestrictedChar) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpCapability StorageBuffer8BitAccess
OpExtension "SPV_KHR_8bit_storage"
OpMemoryModel Logical GLSL450
%char = OpTypeInt 8 0
%undef = OpUndef %char
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Cannot create undefined values with 8- or 16-bit types"));
}

TEST_F(ValidateMisc, UndefRestrictedHalf) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpCapability StorageBuffer16BitAccess
OpExtension "SPV_KHR_16bit_storage"
OpMemoryModel Logical GLSL450
%half = OpTypeFloat 16
%undef = OpUndef %half
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Cannot create undefined values with 8- or 16-bit types"));
}

TEST_F(ValidateMisc, SizeOfValid) {
  const std::string spirv = R"(
               OpCapability Addresses
               OpCapability Kernel
               OpMemoryModel Physical64 OpenCL
               OpEntryPoint Kernel %f "f"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 0
        %ptr = OpTypePointer CrossWorkgroup %i32
       %fnTy = OpTypeFunction %void
          %f = OpFunction %void None %fnTy
      %entry = OpLabel
          %s = OpSizeOf %i32 %ptr
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_1);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_1));
}

const std::string ShaderClockSpirv = R"(
OpCapability Shader
OpCapability Int64
OpCapability ShaderClockKHR
OpExtension "SPV_KHR_shader_clock"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpSourceExtension "GL_ARB_gpu_shader_int64"
OpSourceExtension "GL_ARB_shader_clock"
OpSourceExtension "GL_EXT_shader_realtime_clock"
OpName %main "main"
OpName %time1 "time1"
%void = OpTypeVoid
)";

TEST_F(ValidateMisc, ShaderClockInt64) {
  const std::string spirv = ShaderClockSpirv + R"(
%3 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%uint_3 = OpConstant %uint 3
%uint_1 = OpConstant %uint 1
%main = OpFunction %void None %3
%5 = OpLabel
%time1 = OpVariable %_ptr_Function_uint Function
%11 = OpReadClockKHR %uint %uint_3
OpStore %time1 %11
OpReturn
OpFunctionEnd)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("or 64bit unsigned integer"));
}

TEST_F(ValidateMisc, ShaderClockVec2) {
  const std::string spirv = ShaderClockSpirv + R"(
%3 = OpTypeFunction %void
%ulong = OpTypeInt 64 0
%_ptr_Function_ulong = OpTypePointer Function %ulong
%uint = OpTypeInt 32 0
%uint_3 = OpConstant %uint 3
%v2uint = OpTypeVector %ulong 2
%_ptr_Function_v2uint = OpTypePointer Function %v2uint
%main = OpFunction %void None %3
%5 = OpLabel
%time1 = OpVariable %_ptr_Function_v2uint Function
%15 = OpReadClockKHR %v2uint %uint_3
OpStore %time1 %15
OpReturn
OpFunctionEnd)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("vector of two components"));
}

TEST_F(ValidateMisc, ShaderClockInvalidScopeValue) {
  const std::string spirv = ShaderClockSpirv + R"(
%3 = OpTypeFunction %void
%ulong = OpTypeInt 64 0
%uint = OpTypeInt 32 0
%_ptr_Function_ulong = OpTypePointer Function %ulong
%uint_10 = OpConstant %uint 10
%uint_1 = OpConstant %uint 1
%main = OpFunction %void None %3
%5 = OpLabel
%time1 = OpVariable %_ptr_Function_ulong Function
%11 = OpReadClockKHR %ulong %uint_10
OpStore %time1 %11
OpReturn
OpFunctionEnd)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("Invalid scope value"));
}

TEST_F(ValidateMisc, ShaderClockSubgroupScope) {
  const std::string spirv = ShaderClockSpirv + R"(
%3 = OpTypeFunction %void
%ulong = OpTypeInt 64 0
%uint = OpTypeInt 32 0
%_ptr_Function_ulong = OpTypePointer Function %ulong
%subgroup = OpConstant %uint 3
%uint_1 = OpConstant %uint 1
%main = OpFunction %void None %3
%5 = OpLabel
%time1 = OpVariable %_ptr_Function_ulong Function
%11 = OpReadClockKHR %ulong %subgroup
OpStore %time1 %11
OpReturn
OpFunctionEnd)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMisc, ShaderClockDeviceScope) {
  const std::string spirv = ShaderClockSpirv + R"(
%3 = OpTypeFunction %void
%ulong = OpTypeInt 64 0
%uint = OpTypeInt 32 0
%_ptr_Function_ulong = OpTypePointer Function %ulong
%device = OpConstant %uint 1
%uint_1 = OpConstant %uint 1
%main = OpFunction %void None %3
%5 = OpLabel
%time1 = OpVariable %_ptr_Function_ulong Function
%11 = OpReadClockKHR %ulong %device
OpStore %time1 %11
OpReturn
OpFunctionEnd)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMisc, ShaderClockWorkgroupScope) {
  const std::string spirv = ShaderClockSpirv + R"(
%3 = OpTypeFunction %void
%ulong = OpTypeInt 64 0
%uint = OpTypeInt 32 0
%_ptr_Function_ulong = OpTypePointer Function %ulong
%workgroup = OpConstant %uint 2
%uint_1 = OpConstant %uint 1
%main = OpFunction %void None %3
%5 = OpLabel
%time1 = OpVariable %_ptr_Function_ulong Function
%11 = OpReadClockKHR %ulong %workgroup
OpStore %time1 %11
OpReturn
OpFunctionEnd)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Scope must be Subgroup or Device"));
}

TEST_F(ValidateMisc, VulkanShaderClockWorkgroupScope) {
  const std::string spirv = ShaderClockSpirv + R"(
%3 = OpTypeFunction %void
%ulong = OpTypeInt 64 0
%uint = OpTypeInt 32 0
%_ptr_Function_ulong = OpTypePointer Function %ulong
%workgroup = OpConstant %uint 2
%uint_1 = OpConstant %uint 1
%main = OpFunction %void None %3
%5 = OpLabel
%time1 = OpVariable %_ptr_Function_ulong Function
%11 = OpReadClockKHR %ulong %workgroup
OpStore %time1 %11
OpReturn
OpFunctionEnd)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-OpReadClockKHR-04652"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Scope must be Subgroup or Device"));
}

std::string GenKernelClockSpirv(const std::string& scope) {
  const std::string s = R"(
OpCapability Kernel
OpCapability Addresses
OpCapability Int64
OpCapability ShaderClockKHR
OpExtension "SPV_KHR_shader_clock"
OpMemoryModel Physical32 OpenCL
OpEntryPoint Kernel %main "main"
OpExecutionMode %main ContractionOff
OpSource OpenCL_C 200000
OpName %main "main"
OpName %time1 "time1"
%void = OpTypeVoid
%3 = OpTypeFunction %void
%ulong = OpTypeInt 64 0
%uint = OpTypeInt 32 0
%_ptr_Function_ulong = OpTypePointer Function %ulong
%scope = OpConstant %uint )" +
                        scope + R"(
%main = OpFunction %void None %3
%5 = OpLabel
%time1 = OpVariable %_ptr_Function_ulong Function
%11 = OpReadClockKHR %ulong %scope
OpStore %time1 %11
OpReturn
OpFunctionEnd
)";
  return s;
}

TEST_F(ValidateMisc, KernelClockScopeDevice) {
  CompileSuccessfully(GenKernelClockSpirv("1"), SPV_ENV_OPENCL_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_OPENCL_1_2));
}

TEST_F(ValidateMisc, KernelClockScopeWorkgroup) {
  CompileSuccessfully(GenKernelClockSpirv("2"), SPV_ENV_OPENCL_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_OPENCL_1_2));
}

TEST_F(ValidateMisc, KernelClockScopeSubgroup) {
  CompileSuccessfully(GenKernelClockSpirv("3"), SPV_ENV_OPENCL_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_OPENCL_1_2));
}

TEST_F(ValidateMisc, KernelClockScopeInvalid) {
  CompileSuccessfully(GenKernelClockSpirv("0"), SPV_ENV_OPENCL_1_2);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Scope must be Subgroup, Workgroup, or Device"));
}

TEST_F(ValidateMisc, UndefVoid) {
  const std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
         %10 = OpUndef %2
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Cannot create undefined values with void type"));
}

TEST_F(ValidateMisc, VulkanInvalidStorageClass) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %func "shader"
%int = OpTypeInt 32 0
%ptr = OpTypePointer CrossWorkgroup %int
%var = OpVariable %ptr CrossWorkgroup
%void   = OpTypeVoid
%void_f = OpTypeFunction %void
%func   = OpFunction %void None %void_f
%label  = OpLabel
          OpReturn
          OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_BINARY, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-None-04643"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid storage class for target environment"));
}

TEST_F(ValidateMisc, CoopMat2WorkgroupLocalSizeIdPass) {
  const std::string body = R"(
OpCapability Shader
OpCapability Float16
OpCapability Int16
OpCapability CooperativeMatrixKHR
OpCapability VulkanMemoryModelKHR
OpExtension "SPV_KHR_cooperative_matrix"
OpExtension "SPV_KHR_vulkan_memory_model"
OpMemoryModel Logical VulkanKHR
OpEntryPoint GLCompute %main "main"
OpExecutionModeId %main LocalSizeId %u32_16 %u32_16 %u32_16
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f16 = OpTypeFloat 16
%u32 = OpTypeInt 32 0

%u32_16 = OpConstant %u32 16
%use_Acc = OpConstant %u32 2
%workgroup = OpConstant %u32 2

%f16mat = OpTypeCooperativeMatrixKHR %f16 %workgroup %u32_16 %u32_16 %use_Acc

%main = OpFunction %void None %func
%main_entry = OpLabel

OpReturn
OpFunctionEnd)";

  CompileSuccessfully(body.c_str(), SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
}

TEST_F(ValidateMisc, CoopMat2WorkgroupLocalSizeIdConstantNotDeclaredYetFail) {
  const std::string body = R"(
OpCapability Shader
OpCapability Float16
OpCapability Int16
OpCapability CooperativeMatrixKHR
OpCapability VulkanMemoryModelKHR
OpExtension "SPV_KHR_cooperative_matrix"
OpExtension "SPV_KHR_vulkan_memory_model"
OpMemoryModel Logical VulkanKHR
OpEntryPoint GLCompute %main "main"
OpExecutionModeId %main LocalSizeId %u32_16 %u32_8 %u32_16
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f16 = OpTypeFloat 16
%u32 = OpTypeInt 32 0

%u32_16 = OpConstant %u32 16
%use_Acc = OpConstant %u32 2
%workgroup = OpConstant %u32 2

%f16mat = OpTypeCooperativeMatrixKHR %f16 %workgroup %u32_16 %u32_16 %use_Acc
%u32_8 = OpConstant %u32 8

%main = OpFunction %void None %func
%main_entry = OpLabel

OpReturn
OpFunctionEnd)";

  CompileSuccessfully(body.c_str(), SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpTypeCooperativeMatrixKHR with ScopeWorkgroup used "
                        "before LocalSizeId constant value"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
