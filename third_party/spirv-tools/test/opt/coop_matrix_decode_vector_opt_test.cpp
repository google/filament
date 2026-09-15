// Copyright (c) 2026 NVIDIA Corporation
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

// Optimizer regression tests for SPV_NV_cooperative_matrix_decode_vector.
// Performance passes must not trip def-use / pass bugs on
// OpCooperativeMatrixLoadTensorNV with DecodeFunc|DecodeVectorFunc.

#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "spirv-tools/libspirv.hpp"
#include "spirv-tools/optimizer.hpp"

namespace spvtools {
namespace opt {
namespace {

// Same generator as val_memory_test.cpp (GenCoopMat2DecodeVecShader and
// helpers), kept in sync for valid cooperative-matrix decode-vector modules.
std::string GenCoopMat2DecodeVecShader(
    const std::string& extra_types, const std::string& main_body,
    const std::string& after_main, bool include_decode_vec_cap = true,
    const std::string& matrix_id = "%f16matA",
    const std::string& extra_decorations = "") {
  std::string caps = R"(
OpCapability Shader
OpCapability Float16
OpCapability PhysicalStorageBufferAddresses
OpCapability VulkanMemoryModel
OpCapability CooperativeMatrixKHR
OpCapability TensorAddressingNV
OpCapability CooperativeMatrixTensorAddressingNV
OpCapability CooperativeMatrixBlockLoadsNV
OpCapability LongVectorEXT
)";
  if (include_decode_vec_cap) {
    caps += "OpCapability CooperativeMatrixDecodeVectorNV\n";
  }
  std::string exts = R"(OpExtension "SPV_KHR_physical_storage_buffer"
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpExtension "SPV_NV_tensor_addressing"
OpExtension "SPV_NV_cooperative_matrix2"
OpExtension "SPV_KHR_cooperative_matrix"
OpExtension "SPV_KHR_vulkan_memory_model"
OpExtension "SPV_EXT_long_vector"
)";
  if (include_decode_vec_cap) {
    exts += "OpExtension \"SPV_NV_cooperative_matrix_decode_vector\"\n";
  }
  std::string body = R"(
OpMemoryModel Logical VulkanKHR
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1

OpDecorate %f16_arr ArrayStride 2
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
OpDecorate %ssbo Binding 0
OpDecorate %ssbo DescriptorSet 0
)" + extra_decorations +
                     R"(

%void = OpTypeVoid
%bool = OpTypeBool
%func = OpTypeFunction %void
%f16 = OpTypeFloat 16
%f32 = OpTypeFloat 32
%u32 = OpTypeInt 32 0
%s32 = OpTypeInt 32 1

%v2f16 = OpTypeVector %f16 2
%v3f16 = OpTypeVector %f16 3
%v4f16 = OpTypeVector %f16 4
%v8f16 = OpTypeVector %f16 8
%v2f32 = OpTypeVector %f32 2

%s32_0 = OpConstant %s32 0
%f16_0 = OpConstant %f16 0
%v2f16_0 = OpConstantNull %v2f16
%v3f16_0 = OpConstantNull %v3f16
%v4f16_0 = OpConstantNull %v4f16
%v8f16_0 = OpConstantNull %v8f16
%v2f32_0 = OpConstantNull %v2f32
%u32_2 = OpConstant %u32 2
%u32_4 = OpConstant %u32 4
%u32_8 = OpConstant %u32 8
%use_A = OpConstant %u32 0
%use_B = OpConstant %u32 1
%use_Acc = OpConstant %u32 2
%workgroup = OpConstant %u32 2
%subgroup = OpConstant %u32 3

%f16_arr = OpTypeRuntimeArray %f16
%struct = OpTypeStruct %f16_arr
%ssbo_ptr = OpTypePointer StorageBuffer %struct
%ssbo = OpVariable %ssbo_ptr StorageBuffer
%array_ssbo_ptr = OpTypePointer StorageBuffer %f16_arr
%psbptr = OpTypePointer PhysicalStorageBuffer %f16_arr

%f16matA = OpTypeCooperativeMatrixKHR %f16 %workgroup %u32_8 %u32_8 %use_A
%f16matB = OpTypeCooperativeMatrixKHR %f16 %workgroup %u32_8 %u32_8 %use_B
%f16matAcc = OpTypeCooperativeMatrixKHR %f16 %subgroup %u32_8 %u32_8 %use_Acc

%arr2 = OpTypeArray %u32 %u32_2
%functy_f16 = OpTypeFunction %f16 %psbptr %arr2 %arr2
%functy_v2f16 = OpTypeFunction %v2f16 %psbptr %arr2 %arr2
%functy_v3f16 = OpTypeFunction %v3f16 %psbptr %arr2 %arr2
%functy_v4f16 = OpTypeFunction %v4f16 %psbptr %arr2 %arr2
%functy_v8f16 = OpTypeFunction %v8f16 %psbptr %arr2 %arr2
%functy_v2f32 = OpTypeFunction %v2f32 %psbptr %arr2 %arr2

%clamp_const = OpConstant %u32 0
%dim_const = OpConstant %u32 2
%hasdim = OpConstantFalse %bool
%p0 = OpConstant %u32 0
%p1 = OpConstant %u32 1
%layout = OpTypeTensorLayoutNV %dim_const %clamp_const
%view = OpTypeTensorViewNV %dim_const %hasdim %p0 %p1
)" + extra_types + R"(

%main = OpFunction %void None %func
%main_entry = OpLabel
%array_ptr = OpAccessChain %array_ssbo_ptr %ssbo %s32_0
%mat = OpUndef )" + matrix_id +
                     R"(

%tl = OpCreateTensorLayoutNV %layout
%tv = OpCreateTensorViewNV %view
)" + main_body + R"(
OpReturn
OpFunctionEnd
)" + after_main;

  return caps + exts + body;
}

std::string ScalarDecodeFuncF16(const std::string& name) {
  return "\n%" + name + " = OpFunction %f16 None %functy_f16\n%psb_" + name +
         " = OpFunctionParameter %psbptr\n%c0_" + name +
         " = OpFunctionParameter %arr2\n%c1_" + name +
         " = OpFunctionParameter %arr2\n%entry_" + name +
         " = OpLabel\nOpReturnValue %f16_0\nOpFunctionEnd\n";
}

std::string VectorDecodeFunc(const std::string& name, const std::string& functy,
                             const std::string& vec_t,
                             const std::string& null_id) {
  return "\n%" + name + " = OpFunction %" + vec_t + " None %" + functy +
         "\n%psb_" + name + " = OpFunctionParameter %psbptr\n%c0_" + name +
         " = OpFunctionParameter %arr2\n%c1_" + name +
         " = OpFunctionParameter %arr2\n%entry_" + name +
         " = OpLabel\nOpReturnValue %" + null_id + "\nOpFunctionEnd\n";
}

void ExpectOptSucceedsAndStaysValid(const std::string& spirv_text,
                                    void (*register_passes)(Optimizer*)) {
  SpirvTools tools(SPV_ENV_UNIVERSAL_1_3);
  std::vector<uint32_t> binary;
  ASSERT_TRUE(tools.Assemble(spirv_text.c_str(), &binary))
      << "assemble failed:\n"
      << spirv_text;
  ASSERT_TRUE(tools.Validate(binary.data(), binary.size()))
      << "input invalid:\n"
      << spirv_text;

  Optimizer opt(SPV_ENV_UNIVERSAL_1_3);
  register_passes(&opt);
  std::vector<uint32_t> optimized;
  ASSERT_TRUE(opt.Run(binary.data(), binary.size(), &optimized))
      << "optimizer failed:\n"
      << spirv_text;
  ASSERT_TRUE(tools.Validate(optimized.data(), optimized.size()))
      << "optimized module invalid";
}

void RegisterPerformanceOnly(Optimizer* opt) {
  opt->RegisterPerformancePasses();
}

void RegisterPerformanceAndAggressiveDce(Optimizer* opt) {
  opt->RegisterPerformancePasses().RegisterPass(CreateAggressiveDCEPass());
}

void RegisterEliminateDeadFunctionsOnly(Optimizer* opt) {
  opt->RegisterPass(CreateEliminateDeadFunctionsPass());
}

TEST(CoopMatrixDecodeVectorOpt, PerformancePasses_ScalarDecodeOnly) {
  std::string spirv =
      GenCoopMat2DecodeVecShader("",
                                 R"(
      %loaded = OpCooperativeMatrixLoadTensorNV %f16matA %array_ptr %mat %tl None DecodeFunc %scalar
      )",
                                 ScalarDecodeFuncF16("scalar"),
                                 /*include_decode_vec_cap=*/false);
  ExpectOptSucceedsAndStaysValid(spirv, RegisterPerformanceOnly);
}

TEST(CoopMatrixDecodeVectorOpt,
     PerformancePasses_ScalarDecodeWithDecodeVecExt) {
  std::string spirv = GenCoopMat2DecodeVecShader("",
                                                 R"(
      %loaded = OpCooperativeMatrixLoadTensorNV %f16matA %array_ptr %mat %tl None DecodeFunc %scalar
      )",
                                                 ScalarDecodeFuncF16("scalar"));
  ExpectOptSucceedsAndStaysValid(spirv, RegisterPerformanceOnly);
}

TEST(CoopMatrixDecodeVectorOpt, PerformancePasses_DecodeVectorFuncV2) {
  std::string spirv = GenCoopMat2DecodeVecShader(
      "",
      R"(
      %loaded = OpCooperativeMatrixLoadTensorNV %f16matA %array_ptr %mat %tl None DecodeFunc|DecodeVectorFunc %scalar %vec
      )",
      ScalarDecodeFuncF16("scalar") +
          VectorDecodeFunc("vec", "functy_v2f16", "v2f16", "v2f16_0"));
  ExpectOptSucceedsAndStaysValid(spirv, RegisterPerformanceOnly);
}

TEST(CoopMatrixDecodeVectorOpt, PerformancePasses_DecodeVectorFuncV4) {
  std::string spirv = GenCoopMat2DecodeVecShader(
      "",
      R"(
      %loaded = OpCooperativeMatrixLoadTensorNV %f16matA %array_ptr %mat %tl None DecodeFunc|DecodeVectorFunc %scalar %vec
      )",
      ScalarDecodeFuncF16("scalar") +
          VectorDecodeFunc("vec", "functy_v4f16", "v4f16", "v4f16_0"));
  ExpectOptSucceedsAndStaysValid(spirv, RegisterPerformanceOnly);
}

TEST(CoopMatrixDecodeVectorOpt, PerformancePasses_DecodeVectorFuncV8) {
  std::string spirv = GenCoopMat2DecodeVecShader(
      "",
      R"(
      %loaded = OpCooperativeMatrixLoadTensorNV %f16matA %array_ptr %mat %tl None DecodeFunc|DecodeVectorFunc %scalar %vec
      )",
      ScalarDecodeFuncF16("scalar") +
          VectorDecodeFunc("vec", "functy_v8f16", "v8f16", "v8f16_0"));
  ExpectOptSucceedsAndStaysValid(spirv, RegisterPerformanceOnly);
}

TEST(CoopMatrixDecodeVectorOpt, PerformancePasses_DecodeVectorFuncMatrixUseB) {
  std::string spirv = GenCoopMat2DecodeVecShader(
      "",
      R"(
      %loaded = OpCooperativeMatrixLoadTensorNV %f16matB %array_ptr %mat %tl None DecodeFunc|DecodeVectorFunc %scalar %vec
      )",
      ScalarDecodeFuncF16("scalar") +
          VectorDecodeFunc("vec", "functy_v2f16", "v2f16", "v2f16_0"),
      /*include_decode_vec_cap=*/true, /*matrix_id=*/"%f16matB");
  ExpectOptSucceedsAndStaysValid(spirv, RegisterPerformanceOnly);
}

TEST(CoopMatrixDecodeVectorOpt,
     PerformancePasses_DecodeVectorFuncWithTensorView) {
  std::string spirv = GenCoopMat2DecodeVecShader(
      "",
      R"(
      %loaded = OpCooperativeMatrixLoadTensorNV %f16matA %array_ptr %mat %tl None TensorView|DecodeFunc|DecodeVectorFunc %tv %scalar %vec
      )",
      ScalarDecodeFuncF16("scalar") +
          VectorDecodeFunc("vec", "functy_v2f16", "v2f16", "v2f16_0"));
  ExpectOptSucceedsAndStaysValid(spirv, RegisterPerformanceOnly);
}

TEST(CoopMatrixDecodeVectorOpt,
     AggressiveDceAfterPerformance_DecodeVectorFuncV4) {
  std::string spirv = GenCoopMat2DecodeVecShader(
      "",
      R"(
      %loaded = OpCooperativeMatrixLoadTensorNV %f16matA %array_ptr %mat %tl None DecodeFunc|DecodeVectorFunc %scalar %vec
      )",
      ScalarDecodeFuncF16("scalar") +
          VectorDecodeFunc("vec", "functy_v4f16", "v4f16", "v4f16_0"));
  ExpectOptSucceedsAndStaysValid(spirv, RegisterPerformanceAndAggressiveDce);
}

// Regression: IRContext::AddCalls must treat DecodeVectorFunc like a callee so
// EliminateDeadFunctionsPass does not erase the vector decode function while
// OpCooperativeMatrixLoadTensorNV still references it.
TEST(CoopMatrixDecodeVectorOpt, EliminateDeadFunctionsKeepsDecodeVectorFunc) {
  std::string spirv = GenCoopMat2DecodeVecShader(
      "",
      R"(
      %loaded = OpCooperativeMatrixLoadTensorNV %f16matA %array_ptr %mat %tl None DecodeFunc|DecodeVectorFunc %scalar %vec
      )",
      ScalarDecodeFuncF16("scalar") +
          VectorDecodeFunc("vec", "functy_v4f16", "v4f16", "v4f16_0"));
  ExpectOptSucceedsAndStaysValid(spirv, RegisterEliminateDeadFunctionsOnly);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
