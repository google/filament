// Copyright (c) 2025 Google Inc.
// Copyright (c) 2025 Arm Ltd.
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

// Tests for invalid types.

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
using ::testing::Not;
using ::testing::Values;

using ValidateInvalidType = spvtest::ValidateBase<bool>;

std::string GenerateBFloatCode(const std::string& main_body) {
  const std::string prefix =
      R"(
OpCapability Shader
OpCapability BFloat16TypeKHR
OpCapability AtomicFloat16AddEXT
OpCapability FMAKHR
OpCapability GroupNonUniformShuffle
OpExtension "SPV_EXT_shader_atomic_float16_add"
OpExtension "SPV_KHR_fma"
OpExtension "SPV_KHR_bfloat16"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpSource GLSL 450
OpName %main "main"
%bool = OpTypeBool
%void = OpTypeVoid
%bfloat16 = OpTypeFloat 16 BFloat16KHR
%func = OpTypeFunction %void
%u32 = OpTypeInt 32 0
%u1 = OpConstant %u32 1
%u0 = OpConstant %u32 0
%u3 = OpConstant %u32 3
%bf16_1 = OpConstant %bfloat16 1
%_ptr_Function_bfloat16 = OpTypePointer Function %bfloat16
%v2bfloat16 = OpTypeVector %bfloat16 2
%_ptr_Function_v2bfloat16 = OpTypePointer Function %v2bfloat16
%bf16_ptr = OpTypePointer Workgroup %bfloat16
%bf16_var = OpVariable %bf16_ptr Workgroup
%main = OpFunction %void None %func
%main_entry = OpLabel)";

  const std::string suffix =
      R"(
OpReturn
OpFunctionEnd)";

  return prefix + main_body + suffix;
}

std::string GenerateBFloatCoopMatCode(const std::string& main_body) {
  const std::string prefix =
      R"(
OpCapability Shader
OpCapability VulkanMemoryModel
OpCapability BFloat16TypeKHR
OpCapability BFloat16CooperativeMatrixKHR
OpCapability CooperativeMatrixKHR
OpExtension "SPV_KHR_bfloat16"
OpExtension "SPV_KHR_cooperative_matrix"
OpExtension "SPV_KHR_vulkan_memory_model"
OpMemoryModel Logical Vulkan
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 32 1 1
OpSource GLSL 450
OpName %main "main"
%void = OpTypeVoid
%bfloat16 = OpTypeFloat 16 BFloat16KHR
%func = OpTypeFunction %void
%u32 = OpTypeInt 32 0
%u32_8 = OpConstant %u32 8
%subgroup = OpConstant %u32 3
%useA = OpConstant %u32 0
%bf16_1 = OpConstant %bfloat16 1
%bf16matA = OpTypeCooperativeMatrixKHR %bfloat16 %subgroup %u32_8 %u32_8 %useA
%bf16mat_A_1 = OpConstantComposite %bf16matA %bf16_1
%main = OpFunction %void None %func
%main_entry = OpLabel)";

  const std::string suffix =
      R"(
OpReturn
OpFunctionEnd)";

  return prefix + main_body + suffix;
}

TEST_F(ValidateInvalidType, Bfloat16InvalidArithmeticInstruction) {
  const std::string body = R"(
%v1 = OpVariable %_ptr_Function_bfloat16 Function
%v2 = OpVariable %_ptr_Function_bfloat16 Function
%12 = OpLoad %bfloat16 %v1
%14 = OpLoad %bfloat16 %v2
%15 = OpFMul %bfloat16 %12 %14
)";

  CompileSuccessfully(GenerateBFloatCode(body).c_str(), SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("FMul doesn't support BFloat16 type."));
}

TEST_F(ValidateInvalidType, Bfloat16InvalidFmaInstruction) {
  const std::string body = R"(
%15 = OpFmaKHR %bfloat16 %bf16_1 %bf16_1 %bf16_1
)";

  CompileSuccessfully(GenerateBFloatCode(body).c_str(), SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("FmaKHR doesn't support BFloat16 type."));
}

TEST_F(ValidateInvalidType, Bfloat16InvalidVectorTimesScalarInstruction) {
  const std::string body = R"(
%v1 = OpVariable %_ptr_Function_v2bfloat16 Function
%12 = OpLoad %v2bfloat16 %v1
%15 = OpVectorTimesScalar %v2bfloat16 %12 %bf16_1
)";

  CompileSuccessfully(GenerateBFloatCode(body).c_str(), SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("VectorTimesScalar doesn't support BFloat16 type."));
}

TEST_F(ValidateInvalidType, Bfloat16InvalidMatrixTimesScalarInstruction) {
  const std::string body = R"(
%15 = OpMatrixTimesScalar %bf16matA %bf16mat_A_1 %bf16_1
)";

  CompileSuccessfully(GenerateBFloatCoopMatCode(body).c_str(),
                      SPV_ENV_UNIVERSAL_1_6);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("MatrixTimesScalar doesn't support BFloat16 type."));
}

TEST_F(ValidateInvalidType, Bfloat16InvalidRelationalInstruction) {
  const std::string body = R"(
%v1 = OpVariable %_ptr_Function_bfloat16 Function
%v2 = OpVariable %_ptr_Function_bfloat16 Function
%12 = OpLoad %bfloat16 %v1
%14 = OpLoad %bfloat16 %v2
%15 = OpFOrdEqual %bool %12 %14
)";

  CompileSuccessfully(GenerateBFloatCode(body).c_str(), SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("FOrdEqual doesn't support BFloat16 type."));
}

TEST_F(ValidateInvalidType, Bfloat16InvalidAtomicInstruction) {
  const std::string body = R"(
%val1 = OpAtomicFAddEXT %bfloat16 %bf16_var %u1 %u0 %bf16_1
)";

  CompileSuccessfully(GenerateBFloatCode(body).c_str(), SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("AtomicFAddEXT doesn't support BFloat16 type."));
}

TEST_F(ValidateInvalidType, Bfloat16InvalidGroupNonUniformShuffle) {
  const std::string body = R"(
%val1 = OpGroupNonUniformShuffle %bfloat16 %u3 %bf16_1 %u0
)";

  CompileSuccessfully(GenerateBFloatCode(body).c_str(), SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("GroupNonUniformShuffle doesn't support BFloat16 type."));
}

TEST_F(ValidateInvalidType, Bfloat16ExtInstruction) {
  const std::string body = R"(
%15 = OpExtInst %bfloat16 %1 FClamp %bf16_1 %bf16_1 %bf16_1
)";

  CompileSuccessfully(GenerateBFloatCode(body).c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("FClamp: doesn't support BFloat16 type."));
}

TEST_F(ValidateInvalidType, Bfloat16ExtInstructionRequiresExtension) {
  const std::string spirv = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%func = OpTypeFunction %void
%bfloat16 = OpTypeFloat 16 BFloat16KHR
%bf16_1 = OpConstant %bfloat16 1
%main = OpFunction %void None %func
%entry = OpLabel
%15 = OpExtInst %bfloat16 %1 FClamp %bf16_1 %bf16_1 %bf16_1
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_CAPABILITY,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Operand 3 of TypeFloat requires one of these "
                        "capabilities: BFloat16TypeKHR"));
}

std::string GenerateFP8Code(const std::string& main_body) {
  const std::string prefix =
      R"(
OpCapability Shader
OpCapability Float8EXT
OpCapability AtomicFloat16AddEXT
OpCapability GroupNonUniformShuffle
OpExtension "SPV_EXT_shader_atomic_float16_add"
OpExtension "SPV_EXT_float8"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpSource GLSL 450
OpName %main "main"
%bool = OpTypeBool
%void = OpTypeVoid
%fp8e4m3 = OpTypeFloat 8 Float8E4M3EXT
%fp8e5m2 = OpTypeFloat 8 Float8E5M2EXT
%func = OpTypeFunction %void
%u32 = OpTypeInt 32 0
%u1 = OpConstant %u32 1
%u0 = OpConstant %u32 0
%u3 = OpConstant %u32 3
%fp8e4m3_1 = OpConstant %fp8e4m3 1
%fp8e5m2_1 = OpConstant %fp8e5m2 1
%_ptr_Function_fp8e4m3 = OpTypePointer Function %fp8e4m3
%_ptr_Function_fp8e5m2 = OpTypePointer Function %fp8e5m2
%v2fp8e4m3 = OpTypeVector %fp8e4m3 2
%v2fp8e5m2 = OpTypeVector %fp8e5m2 2
%_ptr_Function_v2fp8e4m3 = OpTypePointer Function %v2fp8e4m3
%_ptr_Function_v2fp8e5m2 = OpTypePointer Function %v2fp8e5m2
%fp8e4m3_ptr = OpTypePointer Workgroup %fp8e4m3
%fp8e5m2_ptr = OpTypePointer Workgroup %fp8e5m2
%fp8e4m3_var = OpVariable %fp8e4m3_ptr Workgroup
%fp8e5m2_var = OpVariable %fp8e5m2_ptr Workgroup
%main = OpFunction %void None %func
%main_entry = OpLabel)";

  const std::string suffix =
      R"(
OpReturn
OpFunctionEnd)";

  return prefix + main_body + suffix;
}

TEST_F(ValidateInvalidType, FP8E4M3InvalidArithmeticInstruction) {
  const std::string body = R"(
%v1 = OpVariable %_ptr_Function_fp8e4m3 Function
%v2 = OpVariable %_ptr_Function_fp8e4m3 Function
%12 = OpLoad %fp8e4m3 %v1
%14 = OpLoad %fp8e4m3 %v2
%15 = OpFMul %fp8e4m3 %12 %14
)";

  CompileSuccessfully(GenerateFP8Code(body).c_str(), SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("FMul doesn't support FP8 E4M3/E5M2 types."));
}

TEST_F(ValidateInvalidType, FP8E5M2InvalidArithmeticInstruction) {
  const std::string body = R"(
%v1 = OpVariable %_ptr_Function_fp8e5m2 Function
%v2 = OpVariable %_ptr_Function_fp8e5m2 Function
%12 = OpLoad %fp8e5m2 %v1
%14 = OpLoad %fp8e5m2 %v2
%15 = OpFMul %fp8e5m2 %12 %14
)";

  CompileSuccessfully(GenerateFP8Code(body).c_str(), SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("FMul doesn't support FP8 E4M3/E5M2 types."));
}

TEST_F(ValidateInvalidType, FP8E4M3InvalidDotInstruction) {
  const std::string body = R"(
%v1 = OpVariable %_ptr_Function_v2fp8e4m3 Function
%v2 = OpVariable %_ptr_Function_v2fp8e4m3 Function
%12 = OpLoad %v2fp8e4m3 %v1
%14 = OpLoad %v2fp8e4m3 %v2
%15 = OpDot %fp8e4m3 %12 %14
)";

  CompileSuccessfully(GenerateFP8Code(body).c_str(), SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Dot doesn't support FP8 E4M3/E5M2 types."));
}

TEST_F(ValidateInvalidType, FP8E5M2InvalidDotInstruction) {
  const std::string body = R"(
%v1 = OpVariable %_ptr_Function_v2fp8e5m2 Function
%v2 = OpVariable %_ptr_Function_v2fp8e5m2 Function
%12 = OpLoad %v2fp8e5m2 %v1
%14 = OpLoad %v2fp8e5m2 %v2
%15 = OpDot %fp8e5m2 %12 %14
)";

  CompileSuccessfully(GenerateFP8Code(body).c_str(), SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Dot doesn't support FP8 E4M3/E5M2 types."));
}

TEST_F(ValidateInvalidType, FP8E4M3InvalidRelationalInstruction) {
  const std::string body = R"(
%v1 = OpVariable %_ptr_Function_fp8e4m3 Function
%v2 = OpVariable %_ptr_Function_fp8e4m3 Function
%12 = OpLoad %fp8e4m3 %v1
%14 = OpLoad %fp8e4m3 %v2
%15 = OpFOrdEqual %bool %12 %14
)";

  CompileSuccessfully(GenerateFP8Code(body).c_str(), SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("FOrdEqual doesn't support FP8 E4M3/E5M2 types."));
}

TEST_F(ValidateInvalidType, FP8E5M2InvalidRelationalInstruction) {
  const std::string body = R"(
%v1 = OpVariable %_ptr_Function_fp8e5m2 Function
%v2 = OpVariable %_ptr_Function_fp8e5m2 Function
%12 = OpLoad %fp8e5m2 %v1
%14 = OpLoad %fp8e5m2 %v2
%15 = OpFOrdEqual %bool %12 %14
)";

  CompileSuccessfully(GenerateFP8Code(body).c_str(), SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("FOrdEqual doesn't support FP8 E4M3/E5M2 types."));
}

TEST_F(ValidateInvalidType, FP8E4M3InvalidAtomicInstruction) {
  const std::string body = R"(
%val1 = OpAtomicFAddEXT %fp8e4m3 %fp8e4m3_var %u1 %u0 %fp8e4m3_1
)";

  CompileSuccessfully(GenerateFP8Code(body).c_str(), SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("AtomicFAddEXT doesn't support FP8 E4M3/E5M2 types."));
}

TEST_F(ValidateInvalidType, FP8E5M2InvalidAtomicInstruction) {
  const std::string body = R"(
%val1 = OpAtomicFAddEXT %fp8e5m2 %fp8e5m2_var %u1 %u0 %fp8e5m2_1
)";

  CompileSuccessfully(GenerateFP8Code(body).c_str(), SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("AtomicFAddEXT doesn't support FP8 E4M3/E5M2 types."));
}

TEST_F(ValidateInvalidType, FP8E4M3InvalidGroupNonUniformShuffle) {
  const std::string body = R"(
%val1 = OpGroupNonUniformShuffle %fp8e4m3 %u3 %fp8e4m3_1 %u0
)";

  CompileSuccessfully(GenerateFP8Code(body).c_str(), SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("GroupNonUniformShuffle doesn't support FP8 E4M3/E5M2 types."));
}

TEST_F(ValidateInvalidType, FP8E5M2InvalidGroupNonUniformShuffle) {
  const std::string body = R"(
%val1 = OpGroupNonUniformShuffle %fp8e5m2 %u3 %fp8e5m2_1 %u0
)";

  CompileSuccessfully(GenerateFP8Code(body).c_str(), SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("GroupNonUniformShuffle doesn't support FP8 E4M3/E5M2 types."));
}

TEST_F(ValidateInvalidType, FP8E4M3ExtInstruction) {
  const std::string body = R"(
%15 = OpExtInst %fp8e4m3 %1 FClamp %fp8e4m3_1 %fp8e4m3_1 %fp8e4m3_1
)";

  CompileSuccessfully(GenerateFP8Code(body).c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("FClamp: doesn't support FP8 E4M3/E5M2 types."));
}

TEST_F(ValidateInvalidType, FP8E5M2ExtInstruction) {
  const std::string body = R"(
%15 = OpExtInst %fp8e5m2 %1 FClamp %fp8e5m2_1 %fp8e5m2_1 %fp8e5m2_1
)";

  CompileSuccessfully(GenerateFP8Code(body).c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("FClamp: doesn't support FP8 E4M3/E5M2 types."));
}

TEST_F(ValidateInvalidType, FP8ExtInstructionRequiresExtension) {
  const std::string spirv = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%func = OpTypeFunction %void
%fp8e4m3 = OpTypeFloat 8 Float8E4M3EXT
%fp8e4m3_1 = OpConstant %fp8e4m3 1
%main = OpFunction %void None %func
%entry = OpLabel
%15 = OpExtInst %fp8e4m3 %1 FClamp %fp8e4m3_1 %fp8e4m3_1 %fp8e4m3_1
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_CAPABILITY,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Operand 3 of TypeFloat requires one of these "
                        "capabilities: Float8EXT"));
}

struct OCPMicroscalingCase {
  const char* capability;
  const char* type_decl;
};

using ValidateInvalidOCPMicroscalingType =
    spvtest::ValidateBase<OCPMicroscalingCase>;

std::string GenerateOCPMicroscalingCode(const OCPMicroscalingCase& test_case,
                                        const std::string& main_body) {
  const std::string prefix = std::string(R"(
OpCapability Shader
OpCapability )") + test_case.capability +
                             R"(
OpExtension "SPV_EXT_ocp_microscaling_types"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%bool = OpTypeBool
%void = OpTypeVoid
)" + test_case.type_decl +
                             R"(
%func = OpTypeFunction %void
%ocp_undef = OpUndef %ocp
%main = OpFunction %void None %func
%main_entry = OpLabel)";

  const std::string suffix = R"(
OpReturn
OpFunctionEnd)";

  return prefix + main_body + suffix;
}

TEST_P(ValidateInvalidOCPMicroscalingType, InvalidArithmeticInstruction) {
  const std::string body = R"(
%val = OpFMul %ocp %ocp_undef %ocp_undef
)";

  CompileSuccessfully(GenerateOCPMicroscalingCode(GetParam(), body).c_str(),
                      SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("FMul doesn't support OCP microscaling types."));
}

TEST_P(ValidateInvalidOCPMicroscalingType, InvalidRelationalInstruction) {
  const std::string body = R"(
%val = OpFOrdEqual %bool %ocp_undef %ocp_undef
)";

  CompileSuccessfully(GenerateOCPMicroscalingCode(GetParam(), body).c_str(),
                      SPV_ENV_VULKAN_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("FOrdEqual doesn't support OCP microscaling types."));
}

INSTANTIATE_TEST_SUITE_P(
    ValidateInvalidType, ValidateInvalidOCPMicroscalingType,
    Values(
        OCPMicroscalingCase{"Float4EXT", "%ocp = OpTypeFloat 4 Float4E2M1EXT"},
        OCPMicroscalingCase{"Float6EXT", "%ocp = OpTypeFloat 6 Float6E2M3EXT"},
        OCPMicroscalingCase{"Float6EXT", "%ocp = OpTypeFloat 6 Float6E3M2EXT"},
        OCPMicroscalingCase{"Float8UnsignedE8M0EXT",
                            "%ocp = OpTypeFloat 8 Float8UnsignedE8M0EXT"},
        OCPMicroscalingCase{"MXInt8EXT", "%ocp = OpTypeFloat 8 MXInt8EXT"}));

}  // namespace
}  // namespace val
}  // namespace spvtools
