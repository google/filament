// Copyright (c) 2026 Google LLC
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

#include "assembly_builder.h"
#include "pass_fixture.h"
#include "pass_utils.h"

namespace {

using namespace spvtools;

using ConvertToUntypedTest = opt::PassTest<::testing::Test>;

TEST_F(ConvertToUntypedTest, DoNotCapability) {
  const std::string text = R"(
; CHECK-NOT: OpCapability UntypedPointersKHR
; CHECK-NOT: OpExtension "SPV_KHR_untyped_pointers"
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, AddCapability) {
  const std::string text = R"(
; CHECK: OpCapability UntypedPointersKHR
; CHECK: OpExtension "SPV_KHR_untyped_pointers"
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%ptr = OpTypePointer StorageBuffer %uint
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, SupportedStorageClasses_NoWorkgroup) {
  const std::string text = R"(
; CHECK-DAG: OpTypeUntypedPointerKHR StorageBuffer
; CHECK-DAG: OpTypeUntypedPointerKHR Uniform
; CHECK-DAG: OpTypeUntypedPointerKHR PushConstant
; CHECK-DAG: OpTypeUntypedPointerKHR PhysicalStorageBuffer
; CHECK-NOT: OpTypeUntypedPointerKHR Workgroup
OpCapability Shader
OpCapability PhysicalStorageBufferAddresses
OpExtension "SPV_KHR_physical_storage_buffer"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%int = OpTypeInt 32 1
%ptr_ssbo_int = OpTypePointer StorageBuffer %int
%ptr_ubo_int = OpTypePointer Uniform %int
%ptr_pc_int = OpTypePointer PushConstant %int
%ptr_pssbo_int = OpTypePointer PhysicalStorageBuffer %int
%ptr_wg_int = OpTypePointer Workgroup %int
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, SupportedStorageClasses_Workgroup) {
  const std::string text = R"(
; CHECK-DAG: OpTypeUntypedPointerKHR StorageBuffer
; CHECK-DAG: OpTypeUntypedPointerKHR Uniform
; CHECK-DAG: OpTypeUntypedPointerKHR PushConstant
; CHECK-DAG: OpTypeUntypedPointerKHR PhysicalStorageBuffer
; CHECK-DAG: OpTypeUntypedPointerKHR Workgroup
OpCapability Shader
OpCapability WorkgroupMemoryExplicitLayoutKHR
OpCapability PhysicalStorageBufferAddresses
OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
OpExtension "SPV_KHR_physical_storage_buffer"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%int = OpTypeInt 32 1
%ptr_ssbo_int = OpTypePointer StorageBuffer %int
%ptr_ubo_int = OpTypePointer Uniform %int
%ptr_pc_int = OpTypePointer PushConstant %int
%ptr_pssbo_int = OpTypePointer PhysicalStorageBuffer %int
%ptr_wg_int = OpTypePointer Workgroup %int
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, Pointer_Deduplicate) {
  const std::string text = R"(
; CHECK-NOT: OpTypePointer
; CHECK: OpTypeUntypedPointerKHR StorageBuffer
; CHECK-NOT: OpTypeUntypedPointerKHR
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%int = OpTypeInt 32 1
%uint = OpTypeInt 32 0
%ptr_int = OpTypePointer StorageBuffer %int
%ptr_uint = OpTypePointer StorageBuffer %uint
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, Pointer_DistinguishDecorations) {
  const std::string text = R"(
; CHECK: OpDecorate [[ptr_uint:%\w+]] ArrayStride 4
; CHECK: [[int:%\w+]] = OpTypeInt 32 1
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK-NOT: OpTypePointer
; CHECK-DAG: OpTypeUntypedPointerKHR StorageBuffer
; CHECK-DAG: [[ptr_uint]] = OpTypeUntypedPointerKHR StorageBuffer
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %ptr_uint ArrayStride 4
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%int = OpTypeInt 32 1
%uint = OpTypeInt 32 0
%ptr_int = OpTypePointer StorageBuffer %int
%ptr_uint = OpTypePointer StorageBuffer %uint
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, Pointer_PreExsitingBase) {
  const std::string text = R"(
; CHECK-NOT: OpTypePointer
; CHECK: OpTypeUntypedPointerKHR StorageBuffer
; CHECK-NOT: OpTypeUntypedPointerKHR
OpCapability Shader
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%int = OpTypeInt 32 1
%ptr = OpTypeUntypedPointerKHR StorageBuffer
%ptr_uint = OpTypePointer StorageBuffer %uint
%ptr_int = OpTypePointer StorageBuffer %int
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, Pointer_PreExsitingBase_Unusable) {
  const std::string text = R"(
; CHECK: OpDecorate [[ptr:%\w+]] ArrayStride 4
; CHECK-NOT: OpTypePointer
; CHECK: [[ptr]] = OpTypeUntypedPointerKHR StorageBuffer
; CHECK: OpTypeUntypedPointerKHR StorageBuffer
; CHECK-NOT: OpTypeUntypedPointerKHR
OpCapability Shader
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %ptr ArrayStride 4
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%int = OpTypeInt 32 1
%ptr = OpTypeUntypedPointerKHR StorageBuffer
%ptr_uint = OpTypePointer StorageBuffer %uint
%ptr_int = OpTypePointer StorageBuffer %int
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, Variable_Descriptor) {
  const std::string text = R"(
; CHECK-DAG: OpDecorate [[var:%\w+]] DescriptorSet 0
; CHECK-DAG: OpDecorate [[var]] Binding 0
; CHECK: [[block:%\w+]] = OpTypeStruct
; CHECK-NOT: OpVariable
; CHECK: [[ptr:%\w+]] = OpTypeUntypedPointerKHR Uniform
; CHECK: [[var]] = OpUntypedVariableKHR [[ptr]] Uniform [[block]]
; CHECK: OpCopyObject [[ptr]] [[var]]
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %block 0 Offset 0
OpDecorate %block Block
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%block = OpTypeStruct %uint
%ptr_block = OpTypePointer Uniform %block
%var = OpVariable %ptr_block Uniform
%main = OpFunction %void None %void_fn
%entry = OpLabel
%copy = OpCopyObject %ptr_block %var
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, AccessChain) {
  const std::string text = R"(
; CHECK-DAG: OpDecorate [[var:%\w+]] DescriptorSet 0
; CHECK-DAG: OpDecorate [[var]] Binding 0
; CHECK: [[zero:%\w+]] = OpConstant {{%\w+}} 0
; CHECK: [[block:%\w+]] = OpTypeStruct
; CHECK: [[ptr:%\w+]] = OpTypeUntypedPointerKHR Uniform
; CHECK: [[var]] = OpUntypedVariableKHR [[ptr]] Uniform [[block]]
; CHECK: OpLabel
; CHECK: OpUntypedAccessChainKHR [[ptr]] [[block]] [[var]] [[zero]]
; CHECK-NOT: OpAccessChain
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %block 0 Offset 0
OpDecorate %block Block
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%block = OpTypeStruct %uint
%ptr_block = OpTypePointer Uniform %block
%ptr_uint = OpTypePointer Uniform %uint
%var = OpVariable %ptr_block Uniform
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpAccessChain %ptr_uint %var %uint_0
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, AccessChain_InBounds) {
  const std::string text = R"(
; CHECK-DAG: OpDecorate [[var:%\w+]] DescriptorSet 0
; CHECK-DAG: OpDecorate [[var]] Binding 0
; CHECK: [[zero:%\w+]] = OpConstant {{%\w+}} 0
; CHECK: [[block:%\w+]] = OpTypeStruct
; CHECK: [[ptr:%\w+]] = OpTypeUntypedPointerKHR Uniform
; CHECK: [[var]] = OpUntypedVariableKHR [[ptr]] Uniform [[block]]
; CHECK: OpLabel
; CHECK: OpUntypedInBoundsAccessChainKHR [[ptr]] [[block]] [[var]] [[zero]]
; CHECK-NOT: OpInBoundsAccessChain
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %block 0 Offset 0
OpDecorate %block Block
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%block = OpTypeStruct %uint
%ptr_block = OpTypePointer Uniform %block
%ptr_uint = OpTypePointer Uniform %uint
%var = OpVariable %ptr_block Uniform
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpInBoundsAccessChain %ptr_uint %var %uint_0
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, PtrAccessChain) {
  const std::string text = R"(
; CHECK: OpDecorate {{%\w+}} ArrayStride 4
; CHECK: OpDecorate [[stride_ptr:%\w+]] ArrayStride 4
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[uint_0:%\w+]] = OpConstant [[uint]] 0
; CHECK: [[ptr:%\w+]] = OpTypeUntypedPointerKHR StorageBuffer
; CHECK: [[stride_ptr]] = OpTypeUntypedPointerKHR StorageBuffer
; CHECK: [[access:%\w+]] = OpUntypedAccessChainKHR [[stride_ptr]]
; CHECK: OpUntypedPtrAccessChainKHR [[stride_ptr]] [[uint]] [[access]] [[uint_0]]
OpCapability Shader
OpCapability VariablePointersStorageBuffer
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpDecorate %rta ArrayStride 4
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %ptr_uint ArrayStride 4
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%rta = OpTypeRuntimeArray %uint
%block = OpTypeStruct %rta
%ptr_block = OpTypePointer StorageBuffer %block
%ptr_uint = OpTypePointer StorageBuffer %uint
%var = OpVariable %ptr_block StorageBuffer
%main = OpFunction %void None %void_fn
%entry = OpLabel
%access = OpAccessChain %ptr_uint %var %uint_0 %uint_0
%ptr_access = OpPtrAccessChain %ptr_uint %access %uint_0
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, ArrayLength) {
  const std::string text = R"(
; CHECK: [[uint:%\w+]] = OpTypeInt
; CHECK: [[block:%\w+]] = OpTypeStruct
; CHECK: [[var:%\w+]] = OpUntypedVariableKHR
; CHECK: OpLabel
; CHECK: OpUntypedArrayLengthKHR [[uint]] [[block]] [[var]] 0
; CHECK-NOT: OpArrayLength
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %block 0 Offset 0
OpDecorate %block Block
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %rta ArrayStride 4
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%rta = OpTypeRuntimeArray %uint
%block = OpTypeStruct %rta
%ptr_block = OpTypePointer StorageBuffer %block
%var = OpVariable %ptr_block StorageBuffer
%main = OpFunction %void None %void_fn
%entry = OpLabel
%len = OpArrayLength %uint %var 0
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, CopyMemory_RowMajorColumn) {
  const std::string text = R"(
; CHECK-DAG: OpDecorate [[in:%\w+]] Binding 0
; CHECK-DAG: OpDecorate [[out:%\w+]] Binding 1
; CHECK-DAG: [[float:%\w+]] = OpTypeFloat 32
; CHECK-DAG: [[vec:%\w+]] = OpTypeVector [[float]] 3
; CHECK-DAG: [[block:%\w+]] = OpTypeStruct
; CHECK-DAG: [[ptr:%\w+]] = OpTypeUntypedPointerKHR StorageBuffer
; CHECK-DAG: [[in]] = OpUntypedVariableKHR [[ptr]]
; CHECK-DAG: [[out]] = OpUntypedVariableKHR [[ptr]]
; CHECK-DAG: [[zero:%\w+]] = OpConstant {{%\w+}} 0
; CHECK-DAG: [[one:%\w+]] = OpConstant {{%\w+}} 1
; CHECK-DAG: [[two:%\w+]] = OpConstant {{%\w+}} 2
; CHECK: OpLabel
; CHECK: [[in_access:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[block]] [[in]] [[zero]] [[two]]
; CHECK: [[out_access:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[block]] [[out]] [[zero]] [[two]]
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[in_access]] [[zero]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]]
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[out_access]] [[zero]]
; CHECK: OpStore [[out_gep]] [[in_ld]]
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[in_access]] [[one]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]]
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[out_access]] [[one]]
; CHECK: OpStore [[out_gep]] [[in_ld]]
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[in_access]] [[two]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]]
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[out_access]] [[two]]
; CHECK: OpStore [[out_gep]] [[in_ld]]
; CHECK-NOT: OpCopyMemory
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 RowMajor
OpMemberDecorate %block 0 MatrixStride 12
OpDecorate %block Block
OpDecorate %in DescriptorSet 0
OpDecorate %in Binding 0
OpDecorate %out DescriptorSet 0
OpDecorate %out Binding 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_2 = OpConstant %uint 2
%float = OpTypeFloat 32
%vec3 = OpTypeVector %float 3
%mat3x3 = OpTypeMatrix %vec3 3
%block = OpTypeStruct %mat3x3
%ptr_block = OpTypePointer StorageBuffer %block
%ptr_vec = OpTypePointer StorageBuffer %vec3
%in = OpVariable %ptr_block StorageBuffer
%out = OpVariable %ptr_block StorageBuffer
%main = OpFunction %void None %void_fn
%entry = OpLabel
%in_access = OpAccessChain %ptr_vec %in %uint_0 %uint_2
%out_access = OpAccessChain %ptr_vec %out %uint_0 %uint_2
OpCopyMemory %out_access %in_access
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, CopyMemory_RowMajorElement) {
  const std::string text = R"(
; CHECK-DAG: OpDecorate [[in:%\w+]] Binding 0
; CHECK-DAG: OpDecorate [[out:%\w+]] Binding 1
; CHECK-DAG: [[float:%\w+]] = OpTypeFloat 32
; CHECK-DAG: [[vec:%\w+]] = OpTypeVector [[float]] 3
; CHECK-DAG: [[block:%\w+]] = OpTypeStruct
; CHECK-DAG: [[ptr:%\w+]] = OpTypeUntypedPointerKHR StorageBuffer
; CHECK-DAG: [[in]] = OpUntypedVariableKHR [[ptr]]
; CHECK-DAG: [[out]] = OpUntypedVariableKHR [[ptr]]
; CHECK-DAG: [[zero:%\w+]] = OpConstant {{%\w+}} 0
; CHECK-DAG: [[two:%\w+]] = OpConstant {{%\w+}} 2
; CHECK: OpLabel
; CHECK: [[in_access:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[block]] [[in]] [[zero]] [[two]]
; CHECK: [[out_access:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[block]] [[out]] [[zero]] [[two]]
; CHECK: [[ld:%\w+]] = OpLoad [[float]] [[in_access]]
; CHECK: OpStore [[out_access]] [[ld]]
; CHECK-NOT: OpCopyMemory
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 RowMajor
OpMemberDecorate %block 0 MatrixStride 12
OpDecorate %block Block
OpDecorate %in DescriptorSet 0
OpDecorate %in Binding 0
OpDecorate %out DescriptorSet 0
OpDecorate %out Binding 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_2 = OpConstant %uint 2
%float = OpTypeFloat 32
%vec3 = OpTypeVector %float 3
%mat3x3 = OpTypeMatrix %vec3 3
%block = OpTypeStruct %mat3x3
%ptr_block = OpTypePointer StorageBuffer %block
%ptr_vec = OpTypePointer StorageBuffer %vec3
%ptr_float = OpTypePointer StorageBuffer %float
%in = OpVariable %ptr_block StorageBuffer
%out = OpVariable %ptr_block StorageBuffer
%main = OpFunction %void None %void_fn
%entry = OpLabel
%in_access = OpAccessChain %ptr_float %in %uint_0 %uint_2 %uint_0
%out_access = OpAccessChain %ptr_float %out %uint_0 %uint_2 %uint_0
OpCopyMemory %out_access %in_access
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, CopyMemory_RowMajorColumn_SharedMemoryOperand) {
  const std::string text = R"(
; CHECK-DAG: OpDecorate [[in:%\w+]] Binding 0
; CHECK-DAG: OpDecorate [[out:%\w+]] Binding 1
; CHECK-DAG: [[float:%\w+]] = OpTypeFloat 32
; CHECK-DAG: [[vec:%\w+]] = OpTypeVector [[float]] 3
; CHECK-DAG: [[block:%\w+]] = OpTypeStruct
; CHECK-DAG: [[ptr:%\w+]] = OpTypeUntypedPointerKHR StorageBuffer
; CHECK-DAG: [[in]] = OpUntypedVariableKHR [[ptr]]
; CHECK-DAG: [[out]] = OpUntypedVariableKHR [[ptr]]
; CHECK-DAG: [[zero:%\w+]] = OpConstant {{%\w+}} 0
; CHECK-DAG: [[one:%\w+]] = OpConstant {{%\w+}} 1
; CHECK-DAG: [[two:%\w+]] = OpConstant {{%\w+}} 2
; CHECK-DAG: [[five:%\w+]] = OpConstant {{%\w+}} 5
; CHECK: OpLabel
; CHECK: [[in_access:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[block]] [[in]] [[zero]] [[two]]
; CHECK: [[out_access:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[block]] [[out]] [[zero]] [[two]]
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[in_access]] [[zero]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]] [[ld_access:Aligned\|MakePointerVisible\|NonPrivatePointer]] 4 [[five]]
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[out_access]] [[zero]]
; CHECK: OpStore [[out_gep]] [[in_ld]] [[st_access:Aligned\|MakePointerAvailable\|NonPrivatePointer]] 4 [[five]]
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[in_access]] [[one]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]] [[ld_access]] 4 [[five]]
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[out_access]] [[one]]
; CHECK: OpStore [[out_gep]] [[in_ld]] [[st_access]] 4 [[five]]
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[in_access]] [[two]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]] [[ld_access]] 4 [[five]]
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[out_access]] [[two]]
; CHECK: OpStore [[out_gep]] [[in_ld]] [[st_access]] 4 [[five]]
; CHECK-NOT: OpCopyMemory
OpCapability Shader
OpCapability VulkanMemoryModel
OpExtension "SPV_KHR_vulkan_memory_model"
OpMemoryModel Logical Vulkan
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 RowMajor
OpMemberDecorate %block 0 MatrixStride 12
OpDecorate %block Block
OpDecorate %in DescriptorSet 0
OpDecorate %in Binding 0
OpDecorate %out DescriptorSet 0
OpDecorate %out Binding 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_2 = OpConstant %uint 2
%uint_5 = OpConstant %uint 5
%float = OpTypeFloat 32
%vec3 = OpTypeVector %float 3
%mat3x3 = OpTypeMatrix %vec3 3
%block = OpTypeStruct %mat3x3
%ptr_block = OpTypePointer StorageBuffer %block
%ptr_vec = OpTypePointer StorageBuffer %vec3
%in = OpVariable %ptr_block StorageBuffer
%out = OpVariable %ptr_block StorageBuffer
%main = OpFunction %void None %void_fn
%entry = OpLabel
%in_access = OpAccessChain %ptr_vec %in %uint_0 %uint_2
%out_access = OpAccessChain %ptr_vec %out %uint_0 %uint_2
OpCopyMemory %out_access %in_access Aligned|MakePointerAvailable|MakePointerVisible|NonPrivatePointer 16 %uint_5 %uint_5
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest,
       CopyMemory_RowMajorColumn_SharedMemoryOperand_AddAligned) {
  const std::string text = R"(
; CHECK-DAG: OpDecorate [[in:%\w+]] Binding 0
; CHECK-DAG: OpDecorate [[out:%\w+]] Binding 1
; CHECK-DAG: [[float:%\w+]] = OpTypeFloat 32
; CHECK-DAG: [[vec:%\w+]] = OpTypeVector [[float]] 3
; CHECK-DAG: [[block:%\w+]] = OpTypeStruct
; CHECK-DAG: [[ptr:%\w+]] = OpTypeUntypedPointerKHR StorageBuffer
; CHECK-DAG: [[in]] = OpUntypedVariableKHR [[ptr]]
; CHECK-DAG: [[out]] = OpUntypedVariableKHR [[ptr]]
; CHECK-DAG: [[zero:%\w+]] = OpConstant {{%\w+}} 0
; CHECK-DAG: [[one:%\w+]] = OpConstant {{%\w+}} 1
; CHECK-DAG: [[two:%\w+]] = OpConstant {{%\w+}} 2
; CHECK-DAG: [[five:%\w+]] = OpConstant {{%\w+}} 5
; CHECK: OpLabel
; CHECK: [[in_access:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[block]] [[in]] [[zero]] [[two]]
; CHECK: [[out_access:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[block]] [[out]] [[zero]] [[two]]
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[in_access]] [[zero]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]] [[ld_access:Aligned\|MakePointerVisible\|NonPrivatePointer]] 4 [[five]]
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[out_access]] [[zero]]
; CHECK: OpStore [[out_gep]] [[in_ld]] [[st_access:Aligned\|MakePointerAvailable\|NonPrivatePointer]] 4 [[two]]
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[in_access]] [[one]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]] [[ld_access]] 4 [[five]]
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[out_access]] [[one]]
; CHECK: OpStore [[out_gep]] [[in_ld]] [[st_access]] 4 [[two]]
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[in_access]] [[two]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]] [[ld_access]] 4 [[five]]
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[out_access]] [[two]]
; CHECK: OpStore [[out_gep]] [[in_ld]] [[st_access]] 4 [[two]]
; CHECK-NOT: OpCopyMemory
OpCapability Shader
OpCapability VulkanMemoryModel
OpExtension "SPV_KHR_vulkan_memory_model"
OpMemoryModel Logical Vulkan
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 RowMajor
OpMemberDecorate %block 0 MatrixStride 12
OpDecorate %block Block
OpDecorate %in DescriptorSet 0
OpDecorate %in Binding 0
OpDecorate %out DescriptorSet 0
OpDecorate %out Binding 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_2 = OpConstant %uint 2
%uint_5 = OpConstant %uint 5
%float = OpTypeFloat 32
%vec3 = OpTypeVector %float 3
%mat3x3 = OpTypeMatrix %vec3 3
%block = OpTypeStruct %mat3x3
%ptr_block = OpTypePointer StorageBuffer %block
%ptr_vec = OpTypePointer StorageBuffer %vec3
%in = OpVariable %ptr_block StorageBuffer
%out = OpVariable %ptr_block StorageBuffer
%main = OpFunction %void None %void_fn
%entry = OpLabel
%in_access = OpAccessChain %ptr_vec %in %uint_0 %uint_2
%out_access = OpAccessChain %ptr_vec %out %uint_0 %uint_2
OpCopyMemory %out_access %in_access MakePointerAvailable|MakePointerVisible|NonPrivatePointer %uint_2 %uint_5
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest,
       CopyMemory_RowMajorColumn_SeparatedMemoryOperands) {
  const std::string text = R"(
; CHECK-DAG: OpDecorate [[in:%\w+]] Binding 0
; CHECK-DAG: OpDecorate [[out:%\w+]] Binding 1
; CHECK-DAG: [[float:%\w+]] = OpTypeFloat 32
; CHECK-DAG: [[vec:%\w+]] = OpTypeVector [[float]] 3
; CHECK-DAG: [[block:%\w+]] = OpTypeStruct
; CHECK-DAG: [[ptr:%\w+]] = OpTypeUntypedPointerKHR StorageBuffer
; CHECK-DAG: [[in]] = OpUntypedVariableKHR [[ptr]]
; CHECK-DAG: [[out]] = OpUntypedVariableKHR [[ptr]]
; CHECK-DAG: [[zero:%\w+]] = OpConstant {{%\w+}} 0
; CHECK-DAG: [[one:%\w+]] = OpConstant {{%\w+}} 1
; CHECK-DAG: [[two:%\w+]] = OpConstant {{%\w+}} 2
; CHECK-DAG: [[five:%\w+]] = OpConstant {{%\w+}} 5
; CHECK: OpLabel
; CHECK: [[in_access:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[block]] [[in]] [[zero]] [[two]]
; CHECK: [[out_access:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[block]] [[out]] [[zero]] [[two]]
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[in_access]] [[zero]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]] [[ld_access:Aligned\|MakePointerVisible\|NonPrivatePointer]] 4 [[two]]
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[out_access]] [[zero]]
; CHECK: OpStore [[out_gep]] [[in_ld]] [[st_access:Aligned\|MakePointerAvailable\|NonPrivatePointer]] 4 [[five]]
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[in_access]] [[one]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]] [[ld_access]] 4 [[two]]
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[out_access]] [[one]]
; CHECK: OpStore [[out_gep]] [[in_ld]] [[st_access]] 4 [[five]]
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[in_access]] [[two]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]] [[ld_access]] 4 [[two]]
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[out_access]] [[two]]
; CHECK: OpStore [[out_gep]] [[in_ld]] [[st_access]] 4 [[five]]
; CHECK-NOT: OpCopyMemory
OpCapability Shader
OpCapability VulkanMemoryModel
OpExtension "SPV_KHR_vulkan_memory_model"
OpMemoryModel Logical Vulkan
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 RowMajor
OpMemberDecorate %block 0 MatrixStride 12
OpDecorate %block Block
OpDecorate %in DescriptorSet 0
OpDecorate %in Binding 0
OpDecorate %out DescriptorSet 0
OpDecorate %out Binding 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_2 = OpConstant %uint 2
%uint_5 = OpConstant %uint 5
%float = OpTypeFloat 32
%vec3 = OpTypeVector %float 3
%mat3x3 = OpTypeMatrix %vec3 3
%block = OpTypeStruct %mat3x3
%ptr_block = OpTypePointer StorageBuffer %block
%ptr_vec = OpTypePointer StorageBuffer %vec3
%in = OpVariable %ptr_block StorageBuffer
%out = OpVariable %ptr_block StorageBuffer
%main = OpFunction %void None %void_fn
%entry = OpLabel
%in_access = OpAccessChain %ptr_vec %in %uint_0 %uint_2
%out_access = OpAccessChain %ptr_vec %out %uint_0 %uint_2
OpCopyMemory %out_access %in_access Aligned|MakePointerAvailable|NonPrivatePointer 16 %uint_5 Aligned|MakePointerVisible|NonPrivatePointer 16 %uint_2
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest,
       CopyMemory_RowMajorColumn_SeparatedMemoryOperands_AddAligned) {
  const std::string text = R"(
; CHECK-DAG: OpDecorate [[in:%\w+]] Binding 0
; CHECK-DAG: OpDecorate [[out:%\w+]] Binding 1
; CHECK-DAG: [[float:%\w+]] = OpTypeFloat 32
; CHECK-DAG: [[vec:%\w+]] = OpTypeVector [[float]] 3
; CHECK-DAG: [[block:%\w+]] = OpTypeStruct
; CHECK-DAG: [[ptr:%\w+]] = OpTypeUntypedPointerKHR StorageBuffer
; CHECK-DAG: [[in]] = OpUntypedVariableKHR [[ptr]]
; CHECK-DAG: [[out]] = OpUntypedVariableKHR [[ptr]]
; CHECK-DAG: [[zero:%\w+]] = OpConstant {{%\w+}} 0
; CHECK-DAG: [[one:%\w+]] = OpConstant {{%\w+}} 1
; CHECK-DAG: [[two:%\w+]] = OpConstant {{%\w+}} 2
; CHECK-DAG: [[five:%\w+]] = OpConstant {{%\w+}} 5
; CHECK: OpLabel
; CHECK: [[in_access:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[block]] [[in]] [[zero]] [[two]]
; CHECK: [[out_access:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[block]] [[out]] [[zero]] [[two]]
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[in_access]] [[zero]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]] [[ld_access:Aligned\|Nontemporal]] 4
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[out_access]] [[zero]]
; CHECK: OpStore [[out_gep]] [[in_ld]] [[st_access:Volatile\|Aligned]] 4
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[in_access]] [[one]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]] [[ld_access]] 4
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[out_access]] [[one]]
; CHECK: OpStore [[out_gep]] [[in_ld]] [[st_access]] 4
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[in_access]] [[two]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]] [[ld_access]] 4
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[vec]] [[out_access]] [[two]]
; CHECK: OpStore [[out_gep]] [[in_ld]] [[st_access]] 4
; CHECK-NOT: OpCopyMemory
OpCapability Shader
OpCapability VulkanMemoryModel
OpExtension "SPV_KHR_vulkan_memory_model"
OpMemoryModel Logical Vulkan
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 RowMajor
OpMemberDecorate %block 0 MatrixStride 12
OpDecorate %block Block
OpDecorate %in DescriptorSet 0
OpDecorate %in Binding 0
OpDecorate %out DescriptorSet 0
OpDecorate %out Binding 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_2 = OpConstant %uint 2
%uint_5 = OpConstant %uint 5
%float = OpTypeFloat 32
%vec3 = OpTypeVector %float 3
%mat3x3 = OpTypeMatrix %vec3 3
%block = OpTypeStruct %mat3x3
%ptr_block = OpTypePointer StorageBuffer %block
%ptr_vec = OpTypePointer StorageBuffer %vec3
%in = OpVariable %ptr_block StorageBuffer
%out = OpVariable %ptr_block StorageBuffer
%main = OpFunction %void None %void_fn
%entry = OpLabel
%in_access = OpAccessChain %ptr_vec %in %uint_0 %uint_2
%out_access = OpAccessChain %ptr_vec %out %uint_0 %uint_2
OpCopyMemory %out_access %in_access Volatile Nontemporal
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, CopyMemory_Matrix_NoOperands) {
  const std::string text = R"(
; CHECK-DAG: OpDecorate [[in:%\w+]] Binding 0
; CHECK-DAG: OpDecorate [[out:%\w+]] Binding 1
; CHECK-DAG: [[float:%\w+]] = OpTypeFloat 32
; CHECK-DAG: [[vec:%\w+]] = OpTypeVector [[float]] 2
; CHECK-DAG: [[mat:%\w+]] = OpTypeMatrix [[vec]] 2
; CHECK-DAG: [[ptr:%\w+]] = OpTypeUntypedPointerKHR StorageBuffer
; CHECK-DAG: [[in]] = OpUntypedVariableKHR [[ptr]]
; CHECK-DAG: [[out]] = OpUntypedVariableKHR [[ptr]]
; CHECK-DAG: [[zero:%\w+]] = OpConstant {{%\w+}} 0
; CHECK-DAG: [[one:%\w+]] = OpConstant {{%\w+}} 1
; CHECK-DAG: [[two:%\w+]] = OpConstant {{%\w+}} 2
; CHECK-DAG: [[five:%\w+]] = OpConstant {{%\w+}} 5
; CHECK: OpLabel
; CHECK: [[in_access:%\w+]] = OpUntypedAccessChainKHR [[ptr]] {{%\w+}} [[in]] [[zero]]
; CHECK: [[out_access:%\w+]] = OpUntypedAccessChainKHR [[ptr]] {{%\w+}} [[out]] [[zero]]
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[mat]] [[in_access]] [[zero]] [[zero]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]] [[ld_access:Aligned]] 4
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[mat]] [[out_access]] [[zero]] [[zero]]
; CHECK: OpStore [[out_gep]] [[in_ld]] [[st_access:Aligned]] 4
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[mat]] [[in_access]] [[zero]] [[one]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]] [[ld_access]] 4
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[mat]] [[out_access]] [[zero]] [[one]]
; CHECK: OpStore [[out_gep]] [[in_ld]] [[st_access]] 4
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[mat]] [[in_access]] [[one]] [[zero]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]] [[ld_access]] 4
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[mat]] [[out_access]] [[one]] [[zero]]
; CHECK: OpStore [[out_gep]] [[in_ld]] [[st_access]] 4
; CHECK: [[in_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[mat]] [[in_access]] [[one]] [[one]]
; CHECK: [[in_ld:%\w+]] = OpLoad [[float]] [[in_gep]] [[ld_access]] 4
; CHECK: [[out_gep:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[mat]] [[out_access]] [[one]] [[one]]
; CHECK: OpStore [[out_gep]] [[in_ld]] [[st_access]] 4
; CHECK-NOT: OpCopyMemory
OpCapability Shader
OpCapability VulkanMemoryModel
OpExtension "SPV_KHR_vulkan_memory_model"
OpMemoryModel Logical Vulkan
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %block1 0 Offset 0
OpMemberDecorate %block1 0 ColMajor
OpMemberDecorate %block1 0 MatrixStride 8
OpDecorate %block1 Block
OpMemberDecorate %block2 0 Offset 0
OpMemberDecorate %block2 0 RowMajor
OpMemberDecorate %block2 0 MatrixStride 24
OpDecorate %block2 Block
OpDecorate %in DescriptorSet 0
OpDecorate %in Binding 0
OpDecorate %out DescriptorSet 0
OpDecorate %out Binding 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%uint_5 = OpConstant %uint 5
%float = OpTypeFloat 32
%vec2 = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %vec2 2
%block1 = OpTypeStruct %mat2x2
%block2 = OpTypeStruct %mat2x2
%ptr_block1 = OpTypePointer StorageBuffer %block1
%ptr_block2 = OpTypePointer StorageBuffer %block2
%ptr_mat2x2 = OpTypePointer StorageBuffer %mat2x2
%in = OpVariable %ptr_block1 StorageBuffer
%out = OpVariable %ptr_block2 StorageBuffer
%main = OpFunction %void None %void_fn
%entry = OpLabel
%in_access = OpAccessChain %ptr_mat2x2 %in %uint_0
%out_access = OpAccessChain %ptr_mat2x2 %out %uint_0
OpCopyMemory %out_access %in_access
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, CopyMemory_Array) {
  const std::string text = R"(
; CHECK-DAG: OpDecorate [[in:%\w+]] Binding 0
; CHECK-DAG: OpDecorate [[out:%\w+]] Binding 1
; CHECK-DAG: [[ptr:%\w+]] = OpTypeUntypedPointerKHR StorageBuffer
; CHECK-DAG: [[in]] = OpUntypedVariableKHR [[ptr]]
; CHECK-DAG: [[out]] = OpUntypedVariableKHR [[ptr]]
; CHECK-DAG: [[zero:%\w+]] = OpConstant {{%\w+}} 0
; CHECK-DAG: [[one:%\w+]] = OpConstant {{%\w+}} 1
; CHECK-DAG: [[two:%\w+]] = OpConstant {{%\w+}} 2
; CHECK-DAG: [[five:%\w+]] = OpConstant {{%\w+}} 5
; CHECK-DAG: [[float:%\w+]] = OpTypeFloat 32
; CHECK-DAG: [[array:%\w+]] = OpTypeArray [[float]] [[five]]
; CHECK: OpLabel
; CHECK: [[in_access:%\w+]] = OpUntypedAccessChainKHR [[ptr]] {{%\w+}} [[in]] [[zero]]
; CHECK: [[out_access:%\w+]] = OpUntypedAccessChainKHR [[ptr]] {{%\w+}} [[out]] [[zero]]
; CHECK: [[ld:%\w+]] = OpLoad [[array]] [[in_access]]
; CHECK: OpStore [[out_access]] [[ld]]
; CHECK-NOT: OpCopyMemory
OpCapability Shader
OpCapability VulkanMemoryModel
OpExtension "SPV_KHR_vulkan_memory_model"
OpMemoryModel Logical Vulkan
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %block1 0 Offset 0
OpDecorate %block1 Block
OpMemberDecorate %block2 0 Offset 0
OpDecorate %block2 Block
OpDecorate %array ArrayStride 4
OpDecorate %in DescriptorSet 0
OpDecorate %in Binding 0
OpDecorate %out DescriptorSet 0
OpDecorate %out Binding 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%uint_5 = OpConstant %uint 5
%float = OpTypeFloat 32
%array = OpTypeArray %float %uint_5
%block1 = OpTypeStruct %array
%block2 = OpTypeStruct %array
%ptr_block1 = OpTypePointer StorageBuffer %block1
%ptr_block2 = OpTypePointer StorageBuffer %block2
%ptr_array = OpTypePointer StorageBuffer %array
%in = OpVariable %ptr_block1 StorageBuffer
%out = OpVariable %ptr_block2 StorageBuffer
%main = OpFunction %void None %void_fn
%entry = OpLabel
%in_access = OpAccessChain %ptr_array %in %uint_0
%out_access = OpAccessChain %ptr_array %out %uint_0
OpCopyMemory %out_access %in_access
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, CooperativeMatrixLoadStore) {
  const std::string text = R"(
; CHECK-DAG: [[uint_4:%\w+]] = OpConstant {{%\w+}} 4
; CHECK-DAG: [[uint_8:%\w+]] = OpConstant {{%\w+}} 8
; CHECK: [[in_stride:%\w+]] = OpFunctionParameter
; CHECK: [[out_stride:%\w+]] = OpFunctionParameter
; CHECK: [[mul:%\w+]] = OpIMul {{%\w+}} [[in_stride]] [[uint_4]]
; CHECK: OpCooperativeMatrixLoadKHR {{%\w+}} {{%\w+}} {{%\w+}} [[mul]]
; CHECK: [[mul:%\w+]] = OpIMul {{%\w+}} [[out_stride]] [[uint_8]]
; CHECK: OpCooperativeMatrixStoreKHR {{%\w+}} {{%\w+}} {{%\w+}} [[mul]]
OpCapability Shader
OpCapability Float16
OpCapability VulkanMemoryModel
OpCapability CooperativeMatrixKHR
OpExtension "SPV_KHR_cooperative_matrix"
OpExtension "SPV_KHR_vulkan_memory_model"
OpMemoryModel Logical Vulkan
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %rta_float ArrayStride 4
OpDecorate %rta_vec2 ArrayStride 8
OpDecorate %in_block Block
OpMemberDecorate %in_block 0 Offset 0
OpDecorate %out_block Block
OpMemberDecorate %out_block 0 Offset 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_3 = OpConstant %uint 3
%uint_16 = OpConstant %uint 16
%half = OpTypeFloat 16
%float = OpTypeFloat 32
%vec2 = OpTypeVector %float 2
%rta_float = OpTypeRuntimeArray %float
%rta_vec2 = OpTypeRuntimeArray %vec2
%in_block = OpTypeStruct %rta_float
%out_block = OpTypeStruct %rta_vec2
%ptr_in_block = OpTypePointer StorageBuffer %in_block
%ptr_out_block = OpTypePointer StorageBuffer %out_block
%ptr_float = OpTypePointer StorageBuffer %float
%ptr_vec2 = OpTypePointer StorageBuffer %vec2
%in_var = OpVariable %ptr_in_block StorageBuffer
%out_var = OpVariable %ptr_out_block StorageBuffer
%coop_mat_a = OpTypeCooperativeMatrixKHR %half %uint_3 %uint_16 %uint_16 %uint_0
%fn_ty = OpTypeFunction %void %uint %uint
%fn = OpFunction %void None %fn_ty
%in_stride = OpFunctionParameter %uint
%out_stride = OpFunctionParameter %uint
%entry = OpLabel
%in_gep = OpAccessChain %ptr_float %in_var %uint_0 %uint_0
%ld = OpCooperativeMatrixLoadKHR %coop_mat_a %in_gep %uint_0 %in_stride
%out_gep = OpAccessChain %ptr_vec2 %out_var %uint_0 %uint_0
OpCooperativeMatrixStoreKHR %out_gep %ld %uint_0 %out_stride
OpReturn
OpFunctionEnd
%main = OpFunction %void None %void_fn
%lab = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, UpdateBufferPointer) {
  const std::string text = R"(
; CHECK: [[ptr:%\w+]] = OpTypeUntypedPointerKHR StorageBuffer
; CHECK: OpBufferPointerEXT [[ptr]]
OpCapability Shader
OpCapability DescriptorHeapEXT
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %heap BuiltIn ResourceHeapEXT
OpDecorate %heap_ty Block
OpMemberDecorate %heap_ty 0 Offset 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%heap_ptr = OpTypeUntypedPointerKHR UniformConstant
%buffer_ty = OpTypeBufferEXT StorageBuffer
%heap_ty = OpTypeStruct %buffer_ty
%ptr_uint = OpTypePointer StorageBuffer %uint
%heap = OpUntypedVariableKHR %heap_ptr UniformConstant
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpUntypedAccessChainKHR %heap_ptr %heap_ty %heap %uint_0
%buffer = OpBufferPointerEXT %ptr_uint %gep
%ld = OpLoad %uint %buffer
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, VariablePointers_StorageBuffer) {
  const std::string text = R"(
; CHECK: OpDecorate {{%\w+}} ArrayStride 4
; CHECK: OpDecorate [[stride_ptr:%\w+]] ArrayStride 4
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[uint_0:%\w+]] = OpConstant [[uint]] 0
; CHECK: [[ptr:%\w+]] = OpTypeUntypedPointerKHR StorageBuffer
; CHECK: [[stride_ptr]] = OpTypeUntypedPointerKHR StorageBuffer
; CHECK: [[null:%\w+]] = OpConstantNull [[stride_ptr]]
; CHECK: [[var:%\w+]] = OpUntypedVariableKHR
; CHECK: [[fn_ty:%\w+]] = OpTypeFunction [[stride_ptr]] [[stride_ptr]]
; CHECK: [[fn_ptr:%\w+]] = OpTypePointer Function [[ptr]]
; CHECK: [[fn_var:%\w+]] = OpVariable [[fn_ptr]] Function [[var]]
; CHECK: [[ld:%\w+]] = OpLoad [[ptr]] [[fn_var]]
; CHECK: [[access:%\w+]] = OpUntypedAccessChainKHR [[stride_ptr]] {{%\w+}} [[ld]] [[uint_0]] [[uint_0]]
; CHECK: [[ptr_access:%\w+]] = OpUntypedPtrAccessChainKHR [[stride_ptr]] [[uint]] [[access]] [[uint_0]]
; CHECK: OpFunctionCall [[stride_ptr]] [[fn:%\w+]] [[ptr_access]]
; CHECK: [[fn]] = OpFunction [[stride_ptr]] None [[fn_ty]]
; CHECK: [[param:%\w+]] = OpFunctionParameter [[stride_ptr]]
; CHECK: [[sel:%\w+]] = OpSelect [[stride_ptr]] {{%\w+}} [[null]] [[param]]
; CHECK: [[phi:%\w+]] = OpPhi [[stride_ptr]] [[sel]] {{%\w+}} [[null]] {{%\w+}}
; CHECK: OpReturnValue [[phi]]
OpCapability Shader
OpCapability VariablePointersStorageBuffer
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpDecorate %rta ArrayStride 4
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %ptr_uint ArrayStride 4
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%bool = OpTypeBool
%cond = OpConstantFalse %bool
%rta = OpTypeRuntimeArray %uint
%block = OpTypeStruct %rta
%ptr_block = OpTypePointer StorageBuffer %block
%ptr_uint = OpTypePointer StorageBuffer %uint
%null = OpConstantNull %ptr_uint
%var = OpVariable %ptr_block StorageBuffer
%fn_ty = OpTypeFunction %ptr_uint %ptr_uint
%ptr_func = OpTypePointer Function %ptr_block
%main = OpFunction %void None %void_fn
%main_entry = OpLabel
%fn_var = OpVariable %ptr_func Function %var
%ld_ptr = OpLoad %ptr_block %fn_var
%access = OpAccessChain %ptr_uint %ld_ptr %uint_0 %uint_0
%ptr_access = OpPtrAccessChain %ptr_uint %access %uint_0
%call = OpFunctionCall %ptr_uint %fn %ptr_access
OpReturn
OpFunctionEnd
%fn = OpFunction %ptr_uint None %fn_ty
%param = OpFunctionParameter %ptr_uint
%fn_entry = OpLabel
%sel = OpSelect %ptr_uint %cond %null %param
OpSelectionMerge %merge None
OpBranchConditional %cond %merge %else
%else = OpLabel
OpBranch %merge
%merge = OpLabel
%phi = OpPhi %ptr_uint %sel %fn_entry %null %else
OpReturnValue %phi
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, WorkgroupMemoryExplicitLayout) {
  const std::string text = R"(
; CHECK: OpDecorate [[block1:%\w+]] Block
; CHECK: OpDecorate [[block2:%\w+]] Block
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[uint_0:%\w+]] = OpConstant [[uint]] 0
; CHECK: [[float:%\w+]] = OpTypeFloat 32
; CHECK: [[ptr:%\w+]] = OpTypeUntypedPointerKHR Workgroup
; CHECK: [[v1:%\w+]] = OpUntypedVariableKHR [[ptr]] Workgroup [[block1]]
; CHECK: [[v2:%\w+]] = OpUntypedVariableKHR [[ptr]] Workgroup [[block2]]
; CHECK: [[access1:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[block1]] [[v1]] [[uint_0]]
; CHECK: OpLoad [[uint]] [[access1]]
; CHECK: [[access2:%\w+]] = OpUntypedAccessChainKHR [[ptr]] [[block2]] [[v2]] [[uint_0]]
; CHECK: OpLoad [[float]] [[access2]]
OpCapability Shader
OpCapability WorkgroupMemoryExplicitLayoutKHR
OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %v1 %v2
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block1 Block
OpMemberDecorate %block1 0 Offset 0
OpDecorate %block2 Block
OpMemberDecorate %block2 0 Offset 0
OpDecorate %v1 Aliased
OpDecorate %v2 Aliased
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%float = OpTypeFloat 32
%block1 = OpTypeStruct %uint
%block2 = OpTypeStruct %float
%ptr_block1 = OpTypePointer Workgroup %block1
%ptr_block2 = OpTypePointer Workgroup %block2
%ptr_uint = OpTypePointer Workgroup %uint
%ptr_float = OpTypePointer Workgroup %float
%v1 = OpVariable %ptr_block1 Workgroup
%v2 = OpVariable %ptr_block2 Workgroup
%main = OpFunction %void None %void_fn
%entry = OpLabel
%access1 = OpAccessChain %ptr_uint %v1 %uint_0
%ld1 = OpLoad %uint %access1
%access2 = OpAccessChain %ptr_float %v2 %uint_0
%ld2 = OpLoad %float %access2
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

TEST_F(ConvertToUntypedTest, PhysicalStorageBuffer) {
  const std::string text = R"(
; CHECK: [[ptr:%\w+]] = OpTypeUntypedPointerKHR PhysicalStorageBuffer
; CHECK: [[conv:%\w+]] = OpConvertUToPtr [[ptr]]
; CHECK: OpConvertPtrToU {{%\w+}} [[conv]]
; CHECK: OpBitcast [[ptr]]
OpCapability Shader
OpCapability Int64
OpCapability PhysicalStorageBufferAddresses
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint GLCompute %main "main" %pc
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint2 = OpTypeVector %uint 2
%uint2_0 = OpConstantComposite %uint2 %uint_0 %uint_0
%ulong = OpTypeInt 64 0
%block = OpTypeStruct %ulong
%ptr_block = OpTypePointer PushConstant %block
%ptr_ulong = OpTypePointer PushConstant %ulong
%ptr_pssbo_uint = OpTypePointer PhysicalStorageBuffer %uint
%pc = OpVariable %ptr_block PushConstant
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpAccessChain %ptr_ulong %pc %uint_0
%ld = OpLoad %ulong %gep
%convert = OpConvertUToPtr %ptr_pssbo_uint %ld
%reconvert = OpConvertPtrToU %ulong %convert
%bitcast = OpBitcast %ptr_pssbo_uint %uint2_0
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SinglePassRunAndMatch<opt::ConvertToUntyped>(text, true);
}

}  // namespace
