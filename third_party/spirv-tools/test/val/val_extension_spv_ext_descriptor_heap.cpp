// Copyright 2025 The Khronos Group Inc.
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

// Tests for SPV_EXT_descriptor_heap

#include <string>

#include "gmock/gmock.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;

using ValidateSpvEXTDescriptorHeap = spvtest::ValidateBase<bool>;

TEST_F(ValidateSpvEXTDescriptorHeap, Valid) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability ShaderNonUniform
               OpCapability UniformBufferArrayNonUniformIndexing
               OpCapability StorageBufferArrayNonUniformIndexing
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %resource_heap %sampler_heap %fragColor %uvs %index
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpSourceExtension "GL_EXT_nonuniform_qualifier"
               OpName %main "main"
               OpName %fragColor "fragColor"
               OpName %resource_heap "resource_heap"
               OpName %sampler_heap "sampler_heap"
               OpName %uvs "uvs"
               OpName %index "index"
               OpName %UniformBuffer "UniformBuffer"
               OpMemberName %UniformBuffer 0 "colorOffset"
               OpName %StorageBufferA "StorageBufferA"
               OpMemberName %StorageBufferA 0 "a"
               OpName %StorageBufferB "StorageBufferB"
               OpMemberName %StorageBufferB 0 "b"
               OpDecorate %fragColor Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorateId %_runtimearr_14 ArrayStrideIdEXT %17
               OpDecorate %sampler_heap BuiltIn SamplerHeapEXT
               OpDecorateId %_runtimearr_22 ArrayStrideIdEXT %uint_2
               OpDecorate %uvs Location 0
               OpDecorate %index Flat
               OpDecorate %index Location 1
               OpDecorate %38 NonUniform
               OpDecorate %UniformBuffer Block
               OpMemberDecorate %UniformBuffer 0 Offset 0
               OpDecorateId %_runtimearr_41 ArrayStrideIdEXT %42
               OpDecorate %44 NonUniform
               OpDecorate %46 NonUniform
               OpDecorate %48 NonUniform
               OpDecorate %StorageBufferA Block
               OpMemberDecorate %StorageBufferA 0 Offset 0
               OpDecorateId %_runtimearr_58 ArrayStrideIdEXT %59
               OpDecorate %64 NonUniform
               OpDecorate %StorageBufferB Block
               OpMemberDecorate %StorageBufferB 0 Offset 0
               OpDecorateId %_runtimearr_58_0 ArrayStrideIdEXT %69
               OpDecorate %71 NonUniform
               OpDecorate %73 NonUniform
               OpDecorate %77 NonUniform
               OpDecorate %44 NonReadable
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %fragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
     %int_27 = OpConstant %int 27
         %14 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_Uniform = OpTypeUntypedPointerKHR Uniform
         %17 = OpConstantSizeOfEXT %int %14
%_runtimearr_14 = OpTypeRuntimeArray %14
%sampler_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
      %int_0 = OpConstant %int 0
         %22 = OpTypeSampler
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_runtimearr_22 = OpTypeRuntimeArray %22
         %28 = OpTypeSampledImage %14
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
        %uvs = OpVariable %_ptr_Input_v2float Input
%_ptr_Input_uint = OpTypePointer Input %uint
      %index = OpVariable %_ptr_Input_uint Input
%UniformBuffer = OpTypeStruct %v4float
         %41 = OpTypeBufferEXT Uniform
         %42 = OpConstantSizeOfEXT %int %41
%_runtimearr_41 = OpTypeRuntimeArray %41
      %int_1 = OpConstant %int 1
%StorageBufferA = OpTypeStruct %v4float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %55 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
         %58 = OpTypeBufferEXT StorageBuffer
         %59 = OpConstantSizeOfEXT %int %58
%_runtimearr_58 = OpTypeRuntimeArray %58
    %v3float = OpTypeVector %float 3
%StorageBufferB = OpTypeStruct %v3float
     %uint_0 = OpConstant %uint 0
         %69 = OpConstantSizeOfEXT %int %58
%_runtimearr_58_0 = OpTypeRuntimeArray %58
%_ptr_Output_float = OpTypePointer Output %float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_14 %resource_heap %int_27
         %19 = OpLoad %14 %16
         %23 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_22 %sampler_heap %int_0
         %27 = OpLoad %22 %23
         %29 = OpSampledImage %28 %19 %27
         %33 = OpLoad %v2float %uvs
         %34 = OpImageSampleImplicitLod %v4float %29 %33
               OpStore %fragColor %34
         %37 = OpLoad %uint %index
         %38 = OpCopyObject %uint %37
         %40 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_41 %resource_heap %38
         %44 = OpBufferPointerEXT %_ptr_Uniform %40
         %45 = OpUntypedAccessChainKHR %_ptr_Uniform %UniformBuffer %44 %int_0
         %46 = OpLoad %v4float %45
         %47 = OpLoad %v4float %fragColor
         %48 = OpFAdd %v4float %47 %46
               OpStore %fragColor %48
         %57 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_58 %resource_heap %int_1
         %61 = OpBufferPointerEXT %_ptr_StorageBuffer %57
         %62 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %StorageBufferA %61 %int_0
               OpStore %62 %55
         %63 = OpLoad %uint %index
         %64 = OpCopyObject %uint %63
         %68 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_58_0 %resource_heap %64
         %71 = OpBufferPointerEXT %_ptr_StorageBuffer %68
         %72 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %StorageBufferB %71 %int_0 %uint_0
         %73 = OpLoad %float %72
         %75 = OpAccessChain %_ptr_Output_float %fragColor %uint_0
         %76 = OpLoad %float %75
         %77 = OpFAdd %float %76 %73
         %78 = OpAccessChain %_ptr_Output_float %fragColor %uint_0
               OpStore %78 %77
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
}

TEST_F(ValidateSpvEXTDescriptorHeap, OffsetId64BitIndexingGood) {
  const std::string str = R"(
        OpCapability Shader
        OpCapability Shader64BitIndexingEXT
        OpCapability DescriptorHeapEXT
        OpCapability Int64
        OpExtension "SPV_EXT_descriptor_heap"
        OpExtension "SPV_EXT_shader_64bit_indexing"
  %1 = OpExtInstImport "GLSL.std.450"
        OpMemoryModel Logical GLSL450
        OpEntryPoint GLCompute %2 "main" %3
        OpExecutionMode %2 LocalSize 1 1 1
        OpSource GLSL 450
        OpMemberDecorate %4 0 Offset 0
        OpMemberDecorate %4 1 Offset 4
        OpMemberDecorate %4 2 Offset 8
        OpMemberDecorate %4 3 Offset 12
        OpDecorate %4 Block
        OpDecorate %3 Binding 0
        OpDecorate %3 DescriptorSet 0
  %5 = OpTypeVoid
  %6 = OpTypeFunction %5
  %7 = OpTypeInt 32 0
  %8 = OpTypeInt 64 0
  %4 = OpTypeStruct %7 %7 %7 %7
  %9 = OpTypePointer StorageBuffer %4
  %3 = OpVariable %9 StorageBuffer
  %10 = OpTypeInt 32 1
  %11 = OpConstant %10 0
  %12 = OpConstant %10 1
  %13 = OpConstant %10 2
  %14 = OpConstant %10 3
  %15 = OpTypePointer StorageBuffer %7
  %16 = OpTypeBufferEXT StorageBuffer
  %17 = OpTypeBufferEXT Uniform
  %18 = OpTypeImage %7 2D 0 0 0 1 Unknown
  %19 = OpTypeSampler
  %20 = OpConstantSizeOfEXT %8 %16
  %21 = OpConstantSizeOfEXT %8 %17
  %22 = OpConstantSizeOfEXT %8 %18
  %23 = OpConstantSizeOfEXT %8 %19
  %2 = OpFunction %5 None %6
  %24 = OpLabel
  %25 = OpAccessChain %15 %3 %11
  %26 = OpAccessChain %15 %3 %12
  %27 = OpAccessChain %15 %3 %13
  %28 = OpAccessChain %15 %3 %14
  %29 = OpUConvert %7 %20
  %30 = OpUConvert %7 %21
  %31 = OpUConvert %7 %22
  %32 = OpUConvert %7 %23
        OpStore %25 %29
        OpStore %26 %30
        OpStore %27 %31
        OpStore %28 %32
        OpReturn
        OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
}

TEST_F(ValidateSpvEXTDescriptorHeap, AtomicImageFuncs) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability Image1D
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpName %main "main"
               OpName %resource_heap "resource_heap"
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorateId %_runtimearr_10 ArrayStrideIdEXT %13
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
         %10 = OpTypeImage %int 1D 0 0 0 2 R32i
         %13 = OpConstantSizeOfEXT %int %10
%_runtimearr_10 = OpTypeRuntimeArray %10
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Image_int = OpTypePointer Image %int
 %_ptr_Image = OpTypeUntypedPointerKHR Image
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %12 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_10 %resource_heap %int_1
         %19 = OpUntypedImageTexelPointerEXT %_ptr_Image %10 %12 %int_1 %uint_0
         %21 = OpAtomicIAdd %int %19 %uint_1 %uint_0 %int_1
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
}

TEST_F(ValidateSpvEXTDescriptorHeap, ValidMemberDecorateIdExt) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability BindlessTextureNV
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpExtension "SPV_NV_bindless_texture"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpSamplerImageAddressingModeNV 64
               OpEntryPoint Fragment %main "main" %resource_heap %fragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpSourceExtension "GL_EXT_nonuniform_qualifier"
               OpName %main "main"
               OpName %fragColor "fragColor"
               OpName %resource_heap "resource_heap"
               OpName %U "U"
               OpMemberName %U 0 "uv"
               OpMemberName %U 1 "k"
               OpMemberName %U 2 "g"
               OpDecorate %fragColor Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %U Block
               OpMemberDecorateIdEXT %U 0 OffsetIdEXT %specA
               OpMemberDecorateIdEXT %U 1 OffsetIdEXT %specB
               OpMemberDecorateIdEXT %U 2 OffsetIdEXT %specC
               OpDecorateId %_runtimearr_20 ArrayStrideIdEXT %uint_2
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%_ptr_Output_v2float = OpTypePointer Output %v2float
  %fragColor = OpVariable %_ptr_Output_v2float Output
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
  %samplerTy = OpTypeSampler
%_runtimearr_sampler = OpTypeRuntimeArray %samplerTy
      %specA = OpSpecConstant %int 0
      %specB = OpSpecConstant %int 4
      %specC = OpSpecConstant %int 8
          %U = OpTypeStruct %samplerTy %samplerTy %_runtimearr_sampler
      %int_0 = OpConstant %int 0
%_ptr_Uniform = OpTypeUntypedPointerKHR Uniform
         %20 = OpTypeBufferEXT Uniform
     %uint_2 = OpConstant %uint 2
%_runtimearr_20 = OpTypeRuntimeArray %20
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_20 %resource_heap %int_3
         %23 = OpBufferPointerEXT %_ptr_Uniform %19
         %24 = OpUntypedAccessChainKHR %_ptr_Uniform %U %23 %int_0
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
}

TEST_F(ValidateSpvEXTDescriptorHeap, ValidMemberDecorateIdExt2) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability BindlessTextureNV
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpExtension "SPV_NV_bindless_texture"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpSamplerImageAddressingModeNV 64
               OpEntryPoint Fragment %main "main" %resource_heap %fragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpSourceExtension "GL_EXT_nonuniform_qualifier"
               OpName %main "main"
               OpName %fragColor "fragColor"
               OpName %resource_heap "resource_heap"
               OpName %U "U"
               OpMemberName %U 0 "uv"
               OpMemberName %U 1 "k"
               OpMemberName %U 2 "g"
               OpDecorate %fragColor Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %U Block
               OpMemberDecorateIdEXT %U 0 OffsetIdEXT %specA
               OpMemberDecorateIdEXT %U 1 OffsetIdEXT %specB
               OpMemberDecorateIdEXT %U 2 OffsetIdEXT %specC
               OpDecorateId %_runtimearr_20 ArrayStrideIdEXT %uint_2
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%_ptr_Output_v2float = OpTypePointer Output %v2float
  %fragColor = OpVariable %_ptr_Output_v2float Output
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
  %samplerTy = OpTypeSampler
%_runtimearr_sampler = OpTypeRuntimeArray %samplerTy
      %specA = OpSpecConstant %int 0
      %specB = OpSpecConstant %int 4
      %specC = OpSpecConstant %int 8
          %U = OpTypeStruct %v2float %int %_runtimearr_sampler
      %int_0 = OpConstant %int 0
%_ptr_Uniform = OpTypeUntypedPointerKHR Uniform
         %20 = OpTypeBufferEXT Uniform
     %uint_2 = OpConstant %uint 2
%_runtimearr_20 = OpTypeRuntimeArray %20
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_20 %resource_heap %int_3
         %23 = OpBufferPointerEXT %_ptr_Uniform %19
         %24 = OpUntypedAccessChainKHR %_ptr_Uniform %U %23 %int_0
         %25 = OpLoad %v2float %24
               OpStore %fragColor %25
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
}

// From VVL CTS
TEST_F(ValidateSpvEXTDescriptorHeap, Alignment) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability SampledBuffer
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %2 "main" %3 %4 %5
               OpExecutionMode %2 LocalSize 1 1 1
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpDecorate %3 BuiltIn ResourceHeapEXT
               OpDecorateId %_runtimearr_12 ArrayStrideIdEXT %7
               OpDecorate %4 Binding 0
               OpDecorate %4 DescriptorSet 0
               OpDecorate %_struct_8 Block
               OpMemberDecorate %_struct_8 0 Offset 0
               OpDecorate %5 Binding 1
               OpDecorate %5 DescriptorSet 0
       %void = OpTypeVoid
         %10 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
         %12 = OpTypeImage %uint Buffer 0 0 0 1 Unknown
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
         %15 = OpTypeFunction %uint %_ptr_UniformConstant
     %uint_0 = OpConstant %uint 0
     %v4uint = OpTypeVector %uint 4
     %v2uint = OpTypeVector %uint 2
          %3 = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
          %7 = OpConstantSizeOfEXT %uint %12
%_runtimearr_12 = OpTypeRuntimeArray %12
%_ptr_Function_uint = OpTypePointer Function %uint
          %4 = OpVariable %_ptr_UniformConstant_12 UniformConstant
     %uint_1 = OpConstant %uint 1
  %_struct_8 = OpTypeStruct %v2uint
%_ptr_StorageBuffer__struct_8 = OpTypePointer StorageBuffer %_struct_8
          %5 = OpVariable %_ptr_StorageBuffer__struct_8 StorageBuffer
%_ptr_StorageBuffer_v2uint = OpTypePointer StorageBuffer %v2uint
     %v3uint = OpTypeVector %uint 3
         %24 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
         %25 = OpFunction %uint None %15
         %26 = OpFunctionParameter %_ptr_UniformConstant
         %27 = OpLabel
         %28 = OpLoad %12 %26
         %29 = OpImageFetch %v4uint %28 %uint_0 ZeroExtend
         %30 = OpCompositeExtract %uint %29 0
               OpReturnValue %30
               OpFunctionEnd
          %2 = OpFunction %void None %10
         %31 = OpLabel
         %32 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_12 %3 %uint_0
         %33 = OpUntypedAccessChainKHR %_ptr_UniformConstant %12 %4
         %34 = OpFunctionCall %uint %25 %32
         %35 = OpFunctionCall %uint %25 %33
         %36 = OpCompositeConstruct %v2uint %34 %35
         %37 = OpAccessChain %_ptr_StorageBuffer_v2uint %5 %uint_0
               OpStore %37 %36
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
}

TEST_F(ValidateSpvEXTDescriptorHeap, HeapBaseVarStorageClassResource) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %resource_heap %o
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpName %main "main"
               OpName %o "o"
               OpName %resource_heap "resource_heap"
               OpName %U "U"
               OpMemberName %U 0 "inputData"
               OpDecorate %o Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %U Block
               OpMemberDecorate %U 0 Offset 0
               OpDecorateId %_runtimearr_17 ArrayStrideIdEXT %18
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Output_uint = OpTypePointer Output %uint
          %o = OpVariable %_ptr_Output_uint Output
%_ptr_Uniform = OpTypeUntypedPointerKHR Uniform
%resource_heap = OpUntypedVariableKHR %_ptr_Uniform Uniform
        %int = OpTypeInt 32 1
      %int_9 = OpConstant %int 9
          %U = OpTypeStruct %uint
      %int_0 = OpConstant %int 0
         %17 = OpTypeBufferEXT Uniform
         %18 = OpConstantSizeOfEXT %int %17
%_runtimearr_17 = OpTypeRuntimeArray %17
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpUntypedAccessChainKHR %_ptr_Uniform %_runtimearr_17 %resource_heap %int_9
         %20 = OpBufferPointerEXT %_ptr_Uniform %16
         %21 = OpUntypedAccessChainKHR %_ptr_Uniform %U %20 %int_0
         %22 = OpLoad %uint %21
               OpStore %o %22
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, AnyVUID("VUID-StandaloneSpirv-OpUntypedVariableKHR-11167"));
  EXPECT_THAT(
      diag,
      HasSubstr("Storage class is Uniform, but Vulkan requires that Data Type "
                "be specified when not using UniformConstant storage class"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, HeapBaseVarStorageClassSampler) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %resource_heap %sampler_heap %fragColor %uvs
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpSourceExtension "GL_EXT_nonuniform_qualifier"
               OpName %main "main"
               OpName %fragColor "fragColor"
               OpName %resource_heap "resource_heap"
               OpName %sampler_heap "sampler_heap"
               OpName %uvs "uvs"
               OpDecorate %fragColor Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorateId %_runtimearr_14 ArrayStrideIdEXT %17
               OpDecorate %sampler_heap BuiltIn SamplerHeapEXT
               OpDecorateId %_runtimearr_22 ArrayStrideIdEXT %uint_2
               OpDecorate %uvs Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %fragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
        %int = OpTypeInt 32 1
     %int_27 = OpConstant %int 27
         %14 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
         %17 = OpConstantSizeOfEXT %int %14
%_runtimearr_14 = OpTypeRuntimeArray %14
%sampler_heap = OpUntypedVariableKHR %_ptr_StorageBuffer StorageBuffer
%resource_heap = OpUntypedVariableKHR %_ptr_StorageBuffer StorageBuffer
      %int_0 = OpConstant %int 0
         %22 = OpTypeSampler
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_runtimearr_22 = OpTypeRuntimeArray %22
         %28 = OpTypeSampledImage %14
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
        %uvs = OpVariable %_ptr_Input_v2float Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %_runtimearr_14 %resource_heap %int_27
         %19 = OpLoad %14 %16
         %23 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %_runtimearr_22 %sampler_heap %int_0
         %27 = OpLoad %22 %23
         %29 = OpSampledImage %28 %19 %27
         %33 = OpLoad %v2float %uvs
         %34 = OpImageSampleImplicitLod %v4float %29 %33
               OpStore %fragColor %34
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, AnyVUID("VUID-StandaloneSpirv-OpUntypedVariableKHR-11167"));
  EXPECT_THAT(
      diag,
      HasSubstr(
          "Storage class is StorageBuffer, but Vulkan requires that Data Type "
          "be specified when not using UniformConstant storage class"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, ResourceHeapWorkgroup) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability WorkgroupMemoryExplicitLayoutKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
         %15 = OpTypeBufferEXT StorageBuffer
%_ptr_workgroup = OpTypeUntypedPointerKHR Workgroup
%resource_heap = OpUntypedVariableKHR %_ptr_workgroup Workgroup %15
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, AnyVUID("VUID-ResourceHeapEXT-ResourceHeapEXT-11241"));
  EXPECT_THAT(diag,
              HasSubstr("The variable decorated with ResourceHeapEXT must be "
                        "declared using the UniformConstant storage class"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, SamplerHeapWorkgroup) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability WorkgroupMemoryExplicitLayoutKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn SamplerHeapEXT
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
         %15 = OpTypeBufferEXT StorageBuffer
%_ptr_workgroup = OpTypeUntypedPointerKHR Workgroup
%resource_heap = OpUntypedVariableKHR %_ptr_workgroup Workgroup %15
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, AnyVUID("VUID-SamplerHeapEXT-SamplerHeapEXT-11239"));
  EXPECT_THAT(diag,
              HasSubstr("The variable decorated with SamplerHeapEXT must be "
                        "declared using the UniformConstant storage class"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, LoadDescHeapDerivedSampler) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %resource_heap %sampler_heap %fragColor %uvs
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpSourceExtension "GL_EXT_nonuniform_qualifier"
               OpName %main "main"
               OpName %fragColor "fragColor"
               OpName %resource_heap "resource_heap"
               OpName %sampler_heap "sampler_heap"
               OpName %uvs "uvs"
               OpDecorate %fragColor Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorateId %_runtimearr_14 ArrayStrideIdEXT %17
               OpDecorate %sampler_heap BuiltIn ResourceHeapEXT
               OpDecorateId %_runtimearr_22 ArrayStrideIdEXT %uint_2
               OpDecorate %uvs Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %fragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
     %int_27 = OpConstant %int 27
         %14 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %17 = OpConstantSizeOfEXT %int %14
%_runtimearr_14 = OpTypeRuntimeArray %14
%sampler_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
      %int_0 = OpConstant %int 0
         %22 = OpTypeSampler
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_runtimearr_22 = OpTypeRuntimeArray %22
         %28 = OpTypeSampledImage %14
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
        %uvs = OpVariable %_ptr_Input_v2float Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_14 %resource_heap %int_27
         %19 = OpLoad %14 %16
         %23 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_22 %sampler_heap %int_0
         %27 = OpLoad %22 %23
         %29 = OpSampledImage %28 %19 %27
         %33 = OpLoad %v2float %uvs
         %34 = OpImageSampleImplicitLod %v4float %29 %33
               OpStore %fragColor %34
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, AnyVUID("VUID-StandaloneSpirv-Result-11336"));
  EXPECT_THAT(
      diag,
      HasSubstr(
          "pointer instruction has no descriptor set or binding "
          "and is not derived from a variable decorated with SamplerHeapEXT"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, LoadDescHeapDerivedImage) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %resource_heap %sampler_heap %fragColor %uvs
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpSourceExtension "GL_EXT_nonuniform_qualifier"
               OpName %main "main"
               OpName %fragColor "fragColor"
               OpName %resource_heap "resource_heap"
               OpName %sampler_heap "sampler_heap"
               OpName %uvs "uvs"
               OpDecorate %fragColor Location 0
               OpDecorate %resource_heap BuiltIn SamplerHeapEXT
               OpDecorateId %_runtimearr_14 ArrayStrideIdEXT %17
               OpDecorate %sampler_heap BuiltIn SamplerHeapEXT
               OpDecorateId %_runtimearr_22 ArrayStrideIdEXT %uint_2
               OpDecorate %uvs Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %fragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
     %int_27 = OpConstant %int 27
         %14 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %17 = OpConstantSizeOfEXT %int %14
%_runtimearr_14 = OpTypeRuntimeArray %14
%sampler_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
      %int_0 = OpConstant %int 0
         %22 = OpTypeSampler
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_runtimearr_22 = OpTypeRuntimeArray %22
         %28 = OpTypeSampledImage %14
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
        %uvs = OpVariable %_ptr_Input_v2float Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_14 %resource_heap %int_27
         %19 = OpLoad %14 %16
         %23 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_22 %sampler_heap %int_0
         %27 = OpLoad %22 %23
         %29 = OpSampledImage %28 %19 %27
         %33 = OpLoad %v2float %uvs
         %34 = OpImageSampleImplicitLod %v4float %29 %33
               OpStore %fragColor %34
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, AnyVUID("VUID-StandaloneSpirv-Result-11337"));
  EXPECT_THAT(
      diag,
      HasSubstr(
          "pointer instruction has no descriptor set or binding "
          "and is not derived from a variable decorated with ResourceHeapEXT"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, LoadDescHeapDerivedAccStruct) {
  const std::string str = R"(
                OpCapability Shader
                OpCapability RayQueryKHR
                OpCapability UntypedPointersKHR
                OpCapability DescriptorHeapEXT
                OpExtension "SPV_EXT_descriptor_heap"
                OpExtension "SPV_KHR_untyped_pointers"
                OpExtension "SPV_KHR_ray_query"
                OpMemoryModel Logical GLSL450
                OpEntryPoint GLCompute %main "main" %resource_heap
                OpExecutionMode %main LocalSize 1 1 1
                OpName %resource_heap "resource_heap"
                OpDecorate %resource_heap BuiltIn SamplerHeapEXT
                OpDecorateId %_runtimearr_14 ArrayStrideIdEXT %17
                %_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
                %resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
                %int = OpTypeInt 32 1
                %int_27 = OpConstant %int 27
                %void = OpTypeVoid
                %as = OpTypeAccelerationStructureKHR
                %ptr_as = OpTypePointer Function %as
                %void_fn = OpTypeFunction %void
                %17 = OpConstantSizeOfEXT %int %as
                %_runtimearr_14 = OpTypeRuntimeArray %as
                %main = OpFunction %void None %void_fn
                %entry = OpLabel
                %var = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_14 %resource_heap %int_27
                %value = OpLoad %as %var
                OpReturn
                OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, AnyVUID("VUID-StandaloneSpirv-Result-11339"));
  EXPECT_THAT(
      diag,
      HasSubstr(
          "pointer instruction has no descriptor set or binding "
          "and is not derived from a variable decorated with ResourceHeapEXT"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, ConstantSizeOfScalarIntType) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %resource_heap %o
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpName %main "main"
               OpName %o "o"
               OpName %resource_heap "resource_heap"
               OpName %U "U"
               OpMemberName %U 0 "inputData"
               OpDecorate %o Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %U Block
               OpMemberDecorate %U 0 Offset 0
               OpDecorateId %_runtimearr_17 ArrayStrideIdEXT %18
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Output_uint = OpTypePointer Output %uint
          %o = OpVariable %_ptr_Output_uint Output
%_ptr_Uniform = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_Uniform UniformConstant
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %int_9 = OpConstant %int 9
          %U = OpTypeStruct %uint
      %int_0 = OpConstant %int 0
         %17 = OpTypeBufferEXT Uniform
         %18 = OpConstantSizeOfEXT %v2int %17
%_runtimearr_17 = OpTypeRuntimeArray %17
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpUntypedAccessChainKHR %_ptr_Uniform %_runtimearr_17 %resource_heap %int_9
         %20 = OpBufferPointerEXT %_ptr_Uniform %16
         %21 = OpUntypedAccessChainKHR %_ptr_Uniform %U %20 %int_0
         %22 = OpLoad %uint %21
               OpStore %o %22
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(
      diag,
      HasSubstr(
          "ArrayStrideIdEXT extra operand must be a 32-bit int scalar type"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, ConstantSizeOfDescriptorTypeTarget) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %resource_heap %o
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpName %main "main"
               OpName %o "o"
               OpName %resource_heap "resource_heap"
               OpName %U "U"
               OpMemberName %U 0 "inputData"
               OpDecorate %o Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %U Block
               OpMemberDecorate %U 0 Offset 0
               OpDecorateId %_runtimearr_17 ArrayStrideIdEXT %18
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Output_uint = OpTypePointer Output %uint
          %o = OpVariable %_ptr_Output_uint Output
%_ptr_Uniform = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_Uniform UniformConstant
        %int = OpTypeInt 32 1
      %int_9 = OpConstant %int 9
          %U = OpTypeStruct %uint
      %int_0 = OpConstant %int 0
         %17 = OpTypeBufferEXT Uniform
         %18 = OpConstantSizeOfEXT %int %U
%_runtimearr_17 = OpTypeRuntimeArray %17
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpUntypedAccessChainKHR %_ptr_Uniform %_runtimearr_17 %resource_heap %int_9
         %20 = OpBufferPointerEXT %_ptr_Uniform %16
         %21 = OpUntypedAccessChainKHR %_ptr_Uniform %U %20 %int_0
         %22 = OpLoad %uint %21
               OpStore %o %22
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag,
              HasSubstr("For OpConstantSizeOfEXT instruction, its Type operand "
                        "<Id> '5[%U]' must be a Descriptor type"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, OffsetId64BitIndexingBad) {
  const std::string str = R"(
        OpCapability Shader
        OpCapability DescriptorHeapEXT
        OpExtension "SPV_EXT_descriptor_heap"
  %1 = OpExtInstImport "GLSL.std.450"
        OpMemoryModel Logical GLSL450
        OpEntryPoint GLCompute %2 "main" %3
        OpExecutionMode %2 LocalSize 1 1 1
        OpSource GLSL 450
        OpMemberDecorate %4 0 Offset 0
        OpMemberDecorate %4 1 Offset 4
        OpMemberDecorate %4 2 Offset 8
        OpMemberDecorate %4 3 Offset 12
        OpDecorate %4 Block
        OpDecorate %3 Binding 0
        OpDecorate %3 DescriptorSet 0
  %5 = OpTypeVoid
  %6 = OpTypeFunction %5
  %7 = OpTypeInt 32 0
  %8 = OpTypeInt 64 0
  %4 = OpTypeStruct %7 %7 %7 %7
  %9 = OpTypePointer StorageBuffer %4
  %3 = OpVariable %9 StorageBuffer
  %10 = OpTypeInt 32 1
  %11 = OpConstant %10 0
  %12 = OpConstant %10 1
  %13 = OpConstant %10 2
  %14 = OpConstant %10 3
  %15 = OpTypePointer StorageBuffer %7
  %16 = OpTypeBufferEXT StorageBuffer
  %17 = OpTypeBufferEXT Uniform
  %18 = OpTypeImage %7 2D 0 0 0 1 Unknown
  %19 = OpTypeSampler
  %20 = OpConstantSizeOfEXT %8 %16
  %21 = OpConstantSizeOfEXT %8 %17
  %22 = OpConstantSizeOfEXT %8 %18
  %23 = OpConstantSizeOfEXT %8 %19
  %2 = OpFunction %5 None %6
  %24 = OpLabel
  %25 = OpAccessChain %15 %3 %11
  %26 = OpAccessChain %15 %3 %12
  %27 = OpAccessChain %15 %3 %13
  %28 = OpAccessChain %15 %3 %14
  %29 = OpUConvert %7 %20
  %30 = OpUConvert %7 %21
  %31 = OpUConvert %7 %22
  %32 = OpUConvert %7 %23
        OpStore %25 %29
        OpStore %26 %30
        OpStore %27 %31
        OpStore %28 %32
        OpReturn
        OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(
      diag,
      HasSubstr("Using a 64-bit integer type requires the Int64 capability."));
}

TEST_F(ValidateSpvEXTDescriptorHeap, TypeBufferEXTStorageClass) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %resource_heap %o
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpName %main "main"
               OpName %o "o"
               OpName %resource_heap "resource_heap"
               OpName %U "U"
               OpMemberName %U 0 "inputData"
               OpDecorate %o Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %U Block
               OpMemberDecorate %U 0 Offset 0
               OpDecorateId %_runtimearr_17 ArrayStrideIdEXT %18
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Output_uint = OpTypePointer Output %uint
          %o = OpVariable %_ptr_Output_uint Output
%_ptr_Uniform = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_Uniform UniformConstant
        %int = OpTypeInt 32 1
      %int_9 = OpConstant %int 9
          %U = OpTypeStruct %uint
      %int_0 = OpConstant %int 0
         %17 = OpTypeBufferEXT UniformConstant
         %18 = OpConstantSizeOfEXT %int %17
%_runtimearr_17 = OpTypeRuntimeArray %17
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpUntypedAccessChainKHR %_ptr_Uniform %_runtimearr_17 %resource_heap %int_9
         %20 = OpBufferPointerEXT %_ptr_Uniform %16
         %21 = OpUntypedAccessChainKHR %_ptr_Uniform %U %20 %int_0
         %22 = OpLoad %uint %21
               OpStore %o %22
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, HasSubstr("TypeBufferEXT StorageClass could only be "
                              "StorageBuffer or Uniform."));
}

TEST_F(ValidateSpvEXTDescriptorHeap, BufferPointerEXTStorageClass) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %resource_heap %o
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpName %main "main"
               OpName %o "o"
               OpName %resource_heap "resource_heap"
               OpName %U "U"
               OpMemberName %U 0 "inputData"
               OpDecorate %o Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %U Block
               OpMemberDecorate %U 0 Offset 0
               OpDecorateId %_runtimearr_17 ArrayStrideIdEXT %18
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Output_uint = OpTypePointer Output %uint
          %o = OpVariable %_ptr_Output_uint Output
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_9 = OpConstant %int 9
          %U = OpTypeStruct %uint
      %int_0 = OpConstant %int 0
         %17 = OpTypeBufferEXT Uniform
         %18 = OpConstantSizeOfEXT %int %17
%_runtimearr_17 = OpTypeRuntimeArray %17
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_17 %resource_heap %int_9
         %20 = OpBufferPointerEXT %_ptr_UniformConstant %16
         %21 = OpUntypedAccessChainKHR %_ptr_UniformConstant %U %20 %int_0
         %22 = OpLoad %uint %21
               OpStore %o %22
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(
      diag,
      HasSubstr("OpBufferPointerEXT's Result Type must be a pointer "
                "type with a Storage Class of Uniform or StorageBuffer."));
}

TEST_F(ValidateSpvEXTDescriptorHeap, BufferPointerEXTLayout) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %resource_heap %o
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpName %main "main"
               OpName %o "o"
               OpName %resource_heap "resource_heap"
               OpName %U "U"
               OpMemberName %U 0 "inputData"
               OpDecorate %o Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %U Block
               OpMemberDecorate %U 0 Offset 0
               OpDecorateId %_runtimearr_17 ArrayStrideIdEXT %18
               OpDecorate %_ptr_UniformConstant Offset 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Output_uint = OpTypePointer Output %uint
          %o = OpVariable %_ptr_Output_uint Output
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_9 = OpConstant %int 9
          %U = OpTypeStruct %uint
      %int_0 = OpConstant %int 0
         %17 = OpTypeBufferEXT Uniform
         %18 = OpConstantSizeOfEXT %int %17
%_runtimearr_17 = OpTypeRuntimeArray %17
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_17 %resource_heap %int_9
         %20 = OpBufferPointerEXT %_ptr_UniformConstant %16
         %21 = OpUntypedAccessChainKHR %_ptr_UniformConstant %U %20 %int_0
         %22 = OpLoad %uint %21
               OpStore %o %22
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, AnyVUID("VUID-StandaloneSpirv-Result-11346"));
  EXPECT_THAT(
      diag, HasSubstr("The result type operand of OpBufferPointerEXT "
                      "must have a Type operand that is explicitly laid out"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, BufferPointerEXTDecorate) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %resource_heap %o
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpName %main "main"
               OpName %o "o"
               OpName %resource_heap "resource_heap"
               OpName %U "U"
               OpMemberName %U 0 "inputData"
               OpDecorate %o Location 0
               OpDecorate %resource_heap BuiltIn SamplerHeapEXT
               OpDecorate %U Block
               OpMemberDecorate %U 0 Offset 0
               OpDecorateId %_runtimearr_17 ArrayStrideIdEXT %18
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Output_uint = OpTypePointer Output %uint
          %o = OpVariable %_ptr_Output_uint Output
%_ptr_Uniform = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_Uniform UniformConstant
        %int = OpTypeInt 32 1
      %int_9 = OpConstant %int 9
          %U = OpTypeStruct %uint
      %int_0 = OpConstant %int 0
         %17 = OpTypeBufferEXT Uniform
         %18 = OpConstantSizeOfEXT %int %17
%_runtimearr_17 = OpTypeRuntimeArray %17
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpUntypedAccessChainKHR %_ptr_Uniform %_runtimearr_17 %resource_heap %int_9
         %20 = OpBufferPointerEXT %_ptr_Uniform %16
         %21 = OpUntypedAccessChainKHR %_ptr_Uniform %U %20 %int_0
         %22 = OpLoad %uint %21
               OpStore %o %22
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, HasSubstr("OpBufferPointerEXT's buffer must be an untyped "
                              "pointer into a variable declared with the "
                              "ResourceHeapEXT built-in"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, BufferPointerEXTNonWritable) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %resource_heap %fragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpSourceExtension "GL_EXT_nonuniform_qualifier"
               OpName %main "main"
               OpName %fragColor "fragColor"
               OpName %resource_heap "resource_heap"
               OpName %U "U"
               OpMemberName %U 0 "uv"
               OpDecorate %fragColor Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %U Block
               OpMemberDecorate %U 0 Offset 0
               OpDecorateId %_runtimearr_18 ArrayStrideIdEXT %uint_2
               OpDecorate %22 NonWritable
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%_ptr_Output_v2float = OpTypePointer Output %v2float
  %fragColor = OpVariable %_ptr_Output_v2float Output
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
          %U = OpTypeStruct %v2float
      %int_0 = OpConstant %int 0
%_ptr_Uniform = OpTypeUntypedPointerKHR Uniform
         %18 = OpTypeBufferEXT Uniform
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_runtimearr_18 = OpTypeRuntimeArray %18
       %main = OpFunction %void None %3
          %5 = OpLabel
         %17 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_18 %resource_heap %int_3
         %22 = OpBufferPointerEXT %_ptr_Uniform %17
         %23 = OpUntypedAccessChainKHR %_ptr_Uniform %U %22 %int_0
         %24 = OpLoad %v2float %23
               OpStore %fragColor %24
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, HasSubstr("Target of NonWritable decoration is invalid"));
  EXPECT_THAT(
      diag,
      HasSubstr(
          "cannot be used to OpBufferPointerEXT with Uniform storage class"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, BufferPointerEXTNonWritableWithSSBO) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %resource_heap %fragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpSourceExtension "GL_EXT_nonuniform_qualifier"
               OpName %main "main"
               OpName %fragColor "fragColor"
               OpName %resource_heap "resource_heap"
               OpName %U "U"
               OpMemberName %U 0 "uv"
               OpDecorate %fragColor Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %U Block
               OpMemberDecorate %U 0 Offset 0
               OpDecorateId %_runtimearr_18 ArrayStrideIdEXT %uint_2
               OpDecorate %22 NonWritable
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%_ptr_Output_v2float = OpTypePointer Output %v2float
  %fragColor = OpVariable %_ptr_Output_v2float Output
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
          %U = OpTypeStruct %v2float
      %int_0 = OpConstant %int 0
%_ptr_Uniform = OpTypeUntypedPointerKHR Uniform
%_ptr_Buffer = OpTypeUntypedPointerKHR StorageBuffer
         %18 = OpTypeBufferEXT Uniform
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_runtimearr_18 = OpTypeRuntimeArray %18
       %main = OpFunction %void None %3
          %5 = OpLabel
         %17 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_18 %resource_heap %int_3
         %22 = OpBufferPointerEXT %_ptr_Buffer %17
         %23 = OpUntypedAccessChainKHR %_ptr_Buffer %U %22 %int_0
         %24 = OpLoad %v2float %23
               OpStore %fragColor %24
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
}

TEST_F(ValidateSpvEXTDescriptorHeap, MemberDecorateIdExtStruct) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %2 "main" %3 %4
               OpExecutionMode %2 LocalSize 1 1 1
               OpSource GLSL 450
               OpMemberDecorate %5 0 Offset 0
               OpDecorate %3 DescriptorSet 0
               OpDecorate %3 Binding 0
               OpDecorate %4 DescriptorSet 0
               OpDecorate %4 Binding 0
               OpDecorate %5 Block
               OpDecorate %6 Block
               OpDecorate %7 SpecId 0
               OpMemberDecorateIdEXT %11 0 OffsetIdEXT %7
               OpMemberDecorate %6 1 Offset 4
               OpMemberDecorateIdEXT %6 2 OffsetIdEXT %8
          %9 = OpTypeVoid
         %10 = OpTypeFunction %9
         %11 = OpTypeInt 32 0
          %5 = OpTypeStruct %11
         %12 = OpTypePointer StorageBuffer %5
         %13 = OpTypeImage %11 2D 0 0 0 1 Unknown
         %14 = OpTypeBufferEXT Uniform
         %15 = OpConstantSizeOfEXT %11 %13
         %16 = OpConstantSizeOfEXT %11 %14
         %17 = OpConstant %11 1
         %18 = OpConstant %11 8
         %19 = OpSpecConstantOp %11 IAdd %18 %15
         %20 = OpSpecConstantOp %11 ISub %19 %17
         %21 = OpSpecConstantOp %11 UDiv %20 %15
         %22 = OpSpecConstantOp %11 IMul %21 %15
         %23 = OpSpecConstantOp %11 IAdd %22 %15
         %24 = OpSpecConstantOp %11 IAdd %23 %16
         %25 = OpSpecConstantOp %11 ISub %24 %17
         %26 = OpSpecConstantOp %11 UDiv %25 %16
         %27 = OpSpecConstantOp %11 IMul %26 %16
          %8 = OpSpecConstantOp %11 IAdd %27 %16
          %7 = OpSpecConstant %11 4
          %3 = OpVariable %12 StorageBuffer
         %28 = OpConstant %11 0
          %6 = OpTypeStruct %11 %11 %11
         %29 = OpTypePointer Uniform %6
          %4 = OpVariable %29 Uniform
         %30 = OpTypePointer StorageBuffer %11
         %31 = OpConstant %11 2
          %2 = OpFunction %9 None %10
         %32 = OpLabel
         %33 = OpAccessChain %30 %4 %28
         %34 = OpLoad %11 %33
         %35 = OpAccessChain %30 %4 %31
         %36 = OpLoad %11 %35
         %37 = OpIAdd %11 %34 %36
         %38 = OpAccessChain %30 %3 %28
               OpStore %38 %37
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, HasSubstr("MemberDecorateIdEXT Structure type"));
  EXPECT_THAT(diag, HasSubstr("is not a struct type."));
}

TEST_F(ValidateSpvEXTDescriptorHeap, DecorateIdArrayStrideIdEXT) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %2 "main" %3 %4
               OpExecutionMode %2 LocalSize 1 1 1
               OpSource GLSL 450
               OpMemberDecorate %5 0 Offset 0
               OpDecorate %3 DescriptorSet 0
               OpDecorate %3 Binding 0
               OpDecorate %4 DescriptorSet 0
               OpDecorate %4 Binding 0
               OpDecorate %5 Block
               OpDecorate %6 Block
               OpDecorate %7 SpecId 0
               OpDecorateId %5 ArrayStrideIdEXT %7
               OpMemberDecorate %6 1 Offset 4
               OpMemberDecorateIdEXT %6 2 OffsetIdEXT %8
          %9 = OpTypeVoid
         %10 = OpTypeFunction %9
         %11 = OpTypeInt 32 0
          %5 = OpTypeStruct %11
         %12 = OpTypePointer StorageBuffer %5
         %13 = OpTypeImage %11 2D 0 0 0 1 Unknown
         %14 = OpTypeBufferEXT Uniform
         %15 = OpConstantSizeOfEXT %11 %13
         %16 = OpConstantSizeOfEXT %11 %14
         %17 = OpConstant %11 1
         %18 = OpConstant %11 8
         %19 = OpSpecConstantOp %11 IAdd %18 %15
         %20 = OpSpecConstantOp %11 ISub %19 %17
         %21 = OpSpecConstantOp %11 UDiv %20 %15
         %22 = OpSpecConstantOp %11 IMul %21 %15
         %23 = OpSpecConstantOp %11 IAdd %22 %15
         %24 = OpSpecConstantOp %11 IAdd %23 %16
         %25 = OpSpecConstantOp %11 ISub %24 %17
         %26 = OpSpecConstantOp %11 UDiv %25 %16
         %27 = OpSpecConstantOp %11 IMul %26 %16
          %8 = OpSpecConstantOp %11 IAdd %27 %16
          %7 = OpSpecConstant %11 4
          %3 = OpVariable %12 StorageBuffer
         %28 = OpConstant %11 0
          %6 = OpTypeStruct %11 %11 %11
         %29 = OpTypePointer Uniform %6
          %4 = OpVariable %29 Uniform
         %30 = OpTypePointer StorageBuffer %11
         %31 = OpConstant %11 2
          %2 = OpFunction %9 None %10
         %32 = OpLabel
         %33 = OpAccessChain %30 %4 %28
         %34 = OpLoad %11 %33
         %35 = OpAccessChain %30 %4 %31
         %36 = OpLoad %11 %35
         %37 = OpIAdd %11 %34 %36
         %38 = OpAccessChain %30 %3 %28
               OpStore %38 %37
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(
      diag,
      HasSubstr(
          "ArrayStrideIdEXT decoration must only be applied to array types."));
}

TEST_F(ValidateSpvEXTDescriptorHeap, ArrayStrideNonDescriptor) {
  const std::string str = R"(
              OpCapability Shader
              OpCapability UntypedPointersKHR
              OpCapability DescriptorHeapEXT
              OpExtension "SPV_EXT_descriptor_heap"
              OpExtension "SPV_KHR_untyped_pointers"
              OpMemoryModel Logical GLSL450
              OpEntryPoint GLCompute %main "main" %resource_heap
              OpExecutionMode %main LocalSize 1 1 1
              OpDecorate %resource_heap BuiltIn ResourceHeapEXT
              OpDecorate %ssbo Block
              OpMemberDecorate %ssbo 0 BuiltIn ResourceHeapEXT
              OpMemberDecorate %ssbo 0 Offset 0
              OpDecorateId %_runtimearr_13 ArrayStrideIdEXT %16
              OpDecorateId %_runtimearr_24 ArrayStrideIdEXT %25
      %void = OpTypeVoid
        %3 = OpTypeFunction %void
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
      %int = OpTypeInt 32 1
    %int_16 = OpConstant %int 16
    %v4int = OpTypeVector %int 4
      %ssbo = OpTypeStruct %v4int
    %int_0 = OpConstant %int 0
        %13 = OpTypeImage %int 2D 0 0 0 2 Rgba8i
%_ptr_Uniform = OpTypeUntypedPointerKHR Uniform
        %16 = OpConstantSizeOfEXT %int %13
%_runtimearr_13 = OpTypeRuntimeArray %13
        %24 = OpTypeBufferEXT StorageBuffer
        %25 = OpConstantSizeOfEXT %int %24
%_runtimearr_24 = OpTypeRuntimeArray %int
      %main = OpFunction %void None %3
        %5 = OpLabel
              OpReturn
              OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag,
              HasSubstr("ArrayStrideIdEXT decoration must only be applied to "
                        "array type containing a Descriptor type."));
}

TEST_F(ValidateSpvEXTDescriptorHeap, MemberDecorateIdArrayStrideIdEXT) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %2 "main" %3 %4
               OpExecutionMode %2 LocalSize 1 1 1
               OpSource GLSL 450
               OpMemberDecorate %5 0 Offset 0
               OpDecorate %3 DescriptorSet 0
               OpDecorate %3 Binding 0
               OpDecorate %4 DescriptorSet 0
               OpDecorate %4 Binding 0
               OpDecorate %5 Block
               OpDecorate %6 Block
               OpDecorate %7 SpecId 0
               OpMemberDecorateIdEXT %6 0 ArrayStrideIdEXT %7
               OpMemberDecorate %6 1 Offset 4
               OpMemberDecorateIdEXT %6 2 OffsetIdEXT %8
          %9 = OpTypeVoid
         %10 = OpTypeFunction %9
         %11 = OpTypeInt 32 0
          %5 = OpTypeStruct %11
         %12 = OpTypePointer StorageBuffer %5
         %13 = OpTypeImage %11 2D 0 0 0 1 Unknown
         %14 = OpTypeBufferEXT Uniform
         %15 = OpConstantSizeOfEXT %11 %13
         %16 = OpConstantSizeOfEXT %11 %14
         %17 = OpConstant %11 1
         %18 = OpConstant %11 8
         %19 = OpSpecConstantOp %11 IAdd %18 %15
         %20 = OpSpecConstantOp %11 ISub %19 %17
         %21 = OpSpecConstantOp %11 UDiv %20 %15
         %22 = OpSpecConstantOp %11 IMul %21 %15
         %23 = OpSpecConstantOp %11 IAdd %22 %15
         %24 = OpSpecConstantOp %11 IAdd %23 %16
         %25 = OpSpecConstantOp %11 ISub %24 %17
         %26 = OpSpecConstantOp %11 UDiv %25 %16
         %27 = OpSpecConstantOp %11 IMul %26 %16
          %8 = OpSpecConstantOp %11 IAdd %27 %16
          %7 = OpSpecConstant %11 4
          %3 = OpVariable %12 StorageBuffer
         %28 = OpConstant %11 0
          %6 = OpTypeStruct %11 %11 %11
         %29 = OpTypePointer Uniform %6
          %4 = OpVariable %29 Uniform
         %30 = OpTypePointer StorageBuffer %11
         %31 = OpConstant %11 2
          %2 = OpFunction %9 None %10
         %32 = OpLabel
         %33 = OpAccessChain %30 %4 %28
         %34 = OpLoad %11 %33
         %35 = OpAccessChain %30 %4 %31
         %36 = OpLoad %11 %35
         %37 = OpIAdd %11 %34 %36
         %38 = OpAccessChain %30 %3 %28
               OpStore %38 %37
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, HasSubstr("ArrayStrideIdEXT could only be directly applied "
                              "to array type using OpDecorateId."));
}

TEST_F(ValidateSpvEXTDescriptorHeap, MemberDecorateIdOffsetIdEXT) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %2 "main" %3 %4
               OpExecutionMode %2 LocalSize 1 1 1
               OpSource GLSL 450
               OpMemberDecorate %5 0 Offset 0
               OpDecorate %3 DescriptorSet 0
               OpDecorate %3 Binding 0
               OpDecorate %4 DescriptorSet 0
               OpDecorate %4 Binding 0
               OpDecorate %5 Block
               OpDecorate %6 Block
               OpDecorate %7 SpecId 0
               OpMemberDecorateIdEXT %6 0 OffsetIdEXT %7
               OpMemberDecorate %6 1 Offset 4
               OpMemberDecorateIdEXT %6 2 ArrayStrideIdEXT %8
          %9 = OpTypeVoid
         %10 = OpTypeFunction %9
         %11 = OpTypeInt 32 0
          %5 = OpTypeStruct %11
         %12 = OpTypePointer StorageBuffer %5
         %13 = OpTypeImage %11 2D 0 0 0 1 Unknown
         %14 = OpTypeBufferEXT Uniform
         %15 = OpConstantSizeOfEXT %11 %13
         %16 = OpConstantSizeOfEXT %11 %14
         %17 = OpConstant %11 1
         %18 = OpConstant %11 8
         %19 = OpSpecConstantOp %11 IAdd %18 %15
         %20 = OpSpecConstantOp %11 ISub %19 %17
         %21 = OpSpecConstantOp %11 UDiv %20 %15
         %22 = OpSpecConstantOp %11 IMul %21 %15
         %23 = OpSpecConstantOp %11 IAdd %22 %15
         %24 = OpSpecConstantOp %11 IAdd %23 %16
         %25 = OpSpecConstantOp %11 ISub %24 %17
         %26 = OpSpecConstantOp %11 UDiv %25 %16
         %27 = OpSpecConstantOp %11 IMul %26 %16
          %8 = OpSpecConstantOp %11 IAdd %27 %16
          %7 = OpSpecConstant %11 4
          %3 = OpVariable %12 StorageBuffer
         %28 = OpConstant %11 0
          %6 = OpTypeStruct %11 %11 %11
         %29 = OpTypePointer Uniform %6
          %4 = OpVariable %29 Uniform
         %30 = OpTypePointer StorageBuffer %11
         %31 = OpConstant %11 2
          %2 = OpFunction %9 None %10
         %32 = OpLabel
         %33 = OpAccessChain %30 %4 %28
         %34 = OpLoad %11 %33
         %35 = OpAccessChain %30 %4 %31
         %36 = OpLoad %11 %35
         %37 = OpIAdd %11 %34 %36
         %38 = OpAccessChain %30 %3 %28
               OpStore %38 %37
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(
      diag,
      HasSubstr(
          "OffsetIdEXT decoration in MemberDecorateIdEXT must only be applied "
          "to members of structs where the struct contains descriptor types."));
}

TEST_F(ValidateSpvEXTDescriptorHeap, OffsetIdAllSingleImages) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability ImageBuffer
               OpExtension "SPV_KHR_untyped_pointers"
               OpExtension "SPV_EXT_descriptor_heap"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main" %2
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %2 BuiltIn ResourceHeapEXT
               OpMemberDecorate %_struct_3 0 Offset 0
               OpMemberDecorateIdEXT %_struct_3 1 OffsetIdEXT %uint_0
               OpMemberDecorate %_struct_3 2 Offset 16
       %void = OpTypeVoid
          %7 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_2 = OpConstant %uint 2
 %uint_51966 = OpConstant %uint 51966
   %uint_0_0 = OpConstant %uint 0
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
          %2 = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
         %14 = OpTypeImage %uint Buffer 0 0 0 2 R32ui
  %_struct_3 = OpTypeStruct %14 %14 %14
          %1 = OpFunction %void None %7
         %15 = OpLabel
         %16 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_struct_3 %2 %uint_2
         %17 = OpLoad %14 %16
               OpImageWrite %17 %uint_0 %uint_51966
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
}

TEST_F(ValidateSpvEXTDescriptorHeap, OffsetIdAllImages) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability ImageBuffer
               OpExtension "SPV_KHR_untyped_pointers"
               OpExtension "SPV_EXT_descriptor_heap"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main" %2
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %2 BuiltIn ResourceHeapEXT
               OpDecorate %4 SpecId 0
               OpMemberDecorate %_struct_3 0 Offset 0
               OpMemberDecorateIdEXT %_struct_3 1 OffsetIdEXT %4
               OpMemberDecorateIdEXT %_struct_3 2 OffsetIdEXT %5
       %void = OpTypeVoid
          %7 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_2 = OpConstant %uint 2
 %uint_51966 = OpConstant %uint 51966
   %uint_0_0 = OpConstant %uint 0
          %4 = OpSpecConstant %uint 0
          %5 = OpSpecConstantOp %uint IMul %4 %uint_2
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
          %2 = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
         %14 = OpTypeImage %uint Buffer 0 0 0 2 R32ui
  %_struct_3 = OpTypeStruct %14 %14 %14
          %1 = OpFunction %void None %7
         %15 = OpLabel
         %16 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_struct_3 %2 %uint_2
         %17 = OpLoad %14 %16
               OpImageWrite %17 %uint_0 %uint_51966
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
}

TEST_F(ValidateSpvEXTDescriptorHeap, OffsetIdStructOfStructOfImage) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability ImageBuffer
               OpExtension "SPV_KHR_untyped_pointers"
               OpExtension "SPV_EXT_descriptor_heap"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main" %2
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %2 BuiltIn ResourceHeapEXT
               OpMemberDecorate %struct 0 Offset 0
               OpMemberDecorateIdEXT %struct 1 OffsetIdEXT %uint_0
               OpMemberDecorate %image_struct 0 Offset 0
       %void = OpTypeVoid
          %7 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
 %uint_51966 = OpConstant %uint 51966
   %uint_0_0 = OpConstant %uint 0
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
          %2 = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
         %14 = OpTypeImage %uint Buffer 0 0 0 2 R32ui
  %image_struct = OpTypeStruct %14
  %struct = OpTypeStruct %uint %image_struct
          %1 = OpFunction %void None %7
         %15 = OpLabel
         %16 = OpUntypedAccessChainKHR %_ptr_UniformConstant %struct %2 %uint_1 %uint_0
         %17 = OpLoad %14 %16
               OpImageWrite %17 %uint_0 %uint_51966
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
}

TEST_F(ValidateSpvEXTDescriptorHeap, MemberDecorateIdDecorator) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability BindlessTextureNV
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpExtension "SPV_NV_bindless_texture"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpSamplerImageAddressingModeNV 64
               OpEntryPoint Fragment %main "main" %resource_heap %fragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpSourceExtension "GL_EXT_nonuniform_qualifier"
               OpName %main "main"
               OpName %fragColor "fragColor"
               OpName %resource_heap "resource_heap"
               OpName %U "U"
               OpMemberName %U 0 "uv"
               OpMemberName %U 1 "k"
               OpMemberName %U 2 "g"
               OpDecorate %fragColor Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %U Block
               OpMemberDecorate %U 0 Offset 0
               OpMemberDecorate %U 1 Offset 8
               OpMemberDecorateIdEXT %U 2 Offset 0
               OpDecorateId %_runtimearr_20 ArrayStrideIdEXT %uint_2
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%_ptr_Output_v2float = OpTypePointer Output %v2float
  %fragColor = OpVariable %_ptr_Output_v2float Output
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
      %specC = OpSpecConstant %int 4
  %samplerTy = OpTypeSampler
%_runtimearr_sampler = OpTypeRuntimeArray %samplerTy
          %U = OpTypeStruct %v2float %int %_runtimearr_sampler
      %int_0 = OpConstant %int 0
%_ptr_Uniform = OpTypeUntypedPointerKHR Uniform
         %20 = OpTypeBufferEXT Uniform
     %uint_2 = OpConstant %uint 2
%_runtimearr_20 = OpTypeRuntimeArray %20
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpUntypedAccessChainKHR %_ptr_Uniform %_runtimearr_20 %resource_heap %int_3
         %23 = OpBufferPointerEXT %_ptr_Uniform %19
         %24 = OpUntypedAccessChainKHR %_ptr_Uniform %U %23 %int_0
         %25 = OpLoad %v2float %24
               OpStore %fragColor %25
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, HasSubstr("Decoration operand could only be OffsetIdEXT."));
}

TEST_F(ValidateSpvEXTDescriptorHeap, MemberDecorateIdExtraIdOrder) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability BindlessTextureNV
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpExtension "SPV_NV_bindless_texture"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpSamplerImageAddressingModeNV 64
               OpEntryPoint Fragment %main "main" %resource_heap %fragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpSourceExtension "GL_EXT_nonuniform_qualifier"
               OpName %main "main"
               OpName %fragColor "fragColor"
               OpName %resource_heap "resource_heap"
               OpName %U "U"
               OpMemberName %U 0 "uv"
               OpMemberName %U 1 "k"
               OpMemberName %U 2 "g"
               OpDecorate %fragColor Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %U Block
               OpMemberDecorate %U 0 Offset 0
               OpMemberDecorate %U 1 Offset 8
               OpMemberDecorateIdEXT %U 2 OffsetIdEXT %specC
               OpDecorateId %_runtimearr_20 ArrayStrideIdEXT %uint_2
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%_ptr_Output_v2float = OpTypePointer Output %v2float
  %fragColor = OpVariable %_ptr_Output_v2float Output
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
  %samplerTy = OpTypeSampler
%_runtimearr_sampler = OpTypeRuntimeArray %samplerTy
          %U = OpTypeStruct %v2float %int %_runtimearr_sampler
      %specC = OpSpecConstant %int 4
      %int_0 = OpConstant %int 0
%_ptr_Uniform = OpTypeUntypedPointerKHR Uniform
         %20 = OpTypeBufferEXT Uniform
     %uint_2 = OpConstant %uint 2
%_runtimearr_20 = OpTypeRuntimeArray %20
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpUntypedAccessChainKHR %_ptr_Uniform %_runtimearr_20 %resource_heap %int_3
         %23 = OpBufferPointerEXT %_ptr_Uniform %19
         %24 = OpUntypedAccessChainKHR %_ptr_Uniform %U %23 %int_0
         %25 = OpLoad %v2float %24
               OpStore %fragColor %25
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(
      diag,
      HasSubstr("All <id> Extra Operands must appear before Structure Type."));
}

TEST_F(ValidateSpvEXTDescriptorHeap, OpMemberDecorateIdDuplicateOffset) {
  const std::string str = R"(
          OpCapability Shader
          OpCapability SampledBuffer
          OpCapability DescriptorHeapEXT
          OpExtension "SPV_EXT_descriptor_heap"
          OpMemoryModel Logical GLSL450
          OpEntryPoint GLCompute %main "main"
          OpExecutionMode %main LocalSize 1 1 1
          OpMemberDecorateIdEXT %struct 0 OffsetIdEXT %uint_0
          OpMemberDecorateIdEXT %struct 1 OffsetIdEXT %uint_0
          OpMemberDecorateIdEXT %struct 1 OffsetIdEXT %uint_0
          OpMemberDecorateIdEXT %struct 2 OffsetIdEXT %uint_0
          %uint = OpTypeInt 32 0
          %uint_0 = OpConstant %uint 0
          %14 = OpTypeImage %uint Buffer 0 0 0 2 R32ui
%struct = OpTypeStruct %14 %uint %uint
  %void = OpTypeVoid
     %3 = OpTypeFunction %void
  %main = OpFunction %void None %3
     %5 = OpLabel
          OpReturn
          OpFunctionEnd

  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("member '1' decorated with OffsetIdEXT multiple times "
                        "is not allowed"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, OffsetAndOffsetId) {
  const std::string str = R"(
          OpCapability Shader
          OpCapability SampledBuffer
          OpCapability DescriptorHeapEXT
          OpExtension "SPV_EXT_descriptor_heap"
          OpMemoryModel Logical GLSL450
          OpEntryPoint GLCompute %main "main"
          OpExecutionMode %main LocalSize 1 1 1
          OpMemberDecorate %struct 0 Offset 0
          OpMemberDecorateIdEXT %struct 0 OffsetIdEXT %uint_0
          OpMemberDecorateIdEXT %struct 1 OffsetIdEXT %uint_0
          OpMemberDecorateIdEXT %struct 2 OffsetIdEXT %uint_0
          %uint = OpTypeInt 32 0
          %uint_0 = OpConstant %uint 0
          %14 = OpTypeImage %uint Buffer 0 0 0 2 R32ui
%struct = OpTypeStruct %14 %uint %uint
  %void = OpTypeVoid
     %3 = OpTypeFunction %void
  %main = OpFunction %void None %3
     %5 = OpLabel
          OpReturn
          OpFunctionEnd

  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("member '0' decorated with both OffsetIdEXT and Offset "
                        "is not allowed"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, AtomicImageType) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability Image1D
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpName %main "main"
               OpName %resource_heap "resource_heap"
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorateId %_runtimearr_10 ArrayStrideIdEXT %13
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
         %10 = OpTypeImage %int 1D 0 0 0 2 Rgba16i
         %13 = OpConstantSizeOfEXT %int %10
%_runtimearr_10 = OpTypeRuntimeArray %10
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Image_int = OpTypePointer Image %int
 %_ptr_Image = OpTypeUntypedPointerKHR Image
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %12 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_10 %resource_heap %int_1
         %19 = OpUntypedImageTexelPointerEXT %_ptr_Image %10 %12 %int_1 %uint_0
         %21 = OpAtomicIAdd %int %19 %uint_1 %uint_0 %int_1
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(
      diag,
      AnyVUID("VUID-StandaloneSpirv-OpUntypedImageTexelPointerEXT-11416"));
  EXPECT_THAT(diag,
              HasSubstr("Expected the Image Format in Image to be R64i, R64ui, "
                        "R32f, R32i, or R32ui for Vulkan environment using "
                        "OpUntypedImageTexelPointerEXT"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, UntypedPointerStorageClass) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability Image1D
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpName %main "main"
               OpName %resource_heap "resource_heap"
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorateId %_runtimearr_10 ArrayStrideIdEXT %13
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
         %10 = OpTypeImage %int 1D 0 0 0 2 R32i
%_ptr_Uniform = OpTypeUntypedPointerKHR Private
         %13 = OpConstantSizeOfEXT %int %10
%_runtimearr_10 = OpTypeRuntimeArray %10
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Image_int = OpTypePointer Image %int
 %_ptr_Image = OpTypeUntypedPointerKHR Image
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %12 = OpUntypedAccessChainKHR %_ptr_Uniform %_runtimearr_10 %resource_heap %int_1
         %19 = OpUntypedImageTexelPointerEXT %_ptr_Image %10 %12 %int_1 %uint_0
         %21 = OpAtomicIAdd %int %19 %uint_1 %uint_0 %int_1
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag,
              AnyVUID("VUID-StandaloneSpirv-OpTypeUntypedPointerKHR-11417"));
  EXPECT_THAT(diag, HasSubstr("In Vulkan, untyped pointers can only be used "
                              "in an explicitly laid out storage class"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, InvalidStoreToHeap) {
  const std::string str = R"(
                              OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap %_
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpName %main "main"
               OpName %resource_heap "resource_heap"
               OpName %A "A"
               OpMemberName %A 0 "a"
               OpName %PushConstant "PushConstant"
               OpMemberName %PushConstant 0 "b"
               OpName %_ ""
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %A Block
               OpMemberDecorate %A 0 BuiltIn ResourceHeapEXT
               OpMemberDecorate %A 0 Offset 0
               OpDecorate %PushConstant Block
               OpMemberDecorate %PushConstant 0 Offset 0
               OpDecorateId %_runtimearr_20 ArrayStrideIdEXT %21
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
          %A = OpTypeStruct %uint
%PushConstant = OpTypeStruct %uint
%_ptr_PushConstant_PushConstant = OpTypePointer PushConstant %PushConstant
          %_ = OpVariable %_ptr_PushConstant_PushConstant PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
         %20 = OpTypeBufferEXT StorageBuffer
         %21 = OpConstantSizeOfEXT %int %20
%_runtimearr_20 = OpTypeRuntimeArray %20
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
         %27 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
         %17 = OpLoad %uint %16
               OpStore %resource_heap %17
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, HasSubstr("OpStore Pointer <id>"));
  EXPECT_THAT(diag, HasSubstr("storage class is read-only"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, NonDescriptorOpaqueType) {
  std::string spirv = R"(
        OpCapability Shader
        OpCapability DescriptorHeapEXT
        OpExtension "SPV_EXT_descriptor_heap"
        OpMemoryModel Logical GLSL450
        OpEntryPoint Vertex %main "main"
%float = OpTypeFloat 32
%image = OpTypeImage %float 2D 0 0 0 1 Unknown
%sampled = OpTypeSampledImage %image
%struct = OpTypeStruct %sampled
%void = OpTypeVoid
%func = OpTypeFunction %void
%main = OpFunction %void None %func
   %l = OpLabel
        OpReturn
        OpFunctionEnd
)";
  CompileSuccessfully(spirv.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, AnyVUID("VUID-StandaloneSpirv-DescriptorHeapEXT-11482"));
  EXPECT_THAT(
      diag,
      HasSubstr(
          "In Vulkan, OpTypeStruct must not contain an invalid opaque type."));
}

TEST_F(ValidateSpvEXTDescriptorHeap, ResourceHeapBuiltinOnMemeber) {
  std::string spirv = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap %_
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %A Block
               OpMemberDecorate %A 0 BuiltIn ResourceHeapEXT
               OpMemberDecorate %A 0 Offset 0
               OpDecorate %PushConstant Block
               OpMemberDecorate %PushConstant 0 Offset 0
               OpDecorateId %_runtimearr_21 ArrayStrideIdEXT %22
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
       %uint = OpTypeInt 32 0
          %A = OpTypeStruct %uint
      %int_0 = OpConstant %int 0
%PushConstant = OpTypeStruct %uint
%_ptr_PushConstant_PushConstant = OpTypePointer PushConstant %PushConstant
          %_ = OpVariable %_ptr_PushConstant_PushConstant PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
         %21 = OpTypeBufferEXT StorageBuffer
         %22 = OpConstantSizeOfEXT %int %21
%_runtimearr_21 = OpTypeRuntimeArray %21
       %main = OpFunction %void None %3
          %5 = OpLabel
         %17 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
         %18 = OpLoad %uint %17
         %20 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_21 %resource_heap %int_2
         %24 = OpBufferPointerEXT %_ptr_StorageBuffer %20
         %25 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %A %24 %int_0
               OpStore %25 %18
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(spirv.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "BuiltIn ResourceHeapEXT cannot be used as a member decoration"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, OffsetIdFloat) {
  const std::string str = R"(
          OpCapability Shader
          OpCapability SampledBuffer
          OpCapability DescriptorHeapEXT
          OpExtension "SPV_EXT_descriptor_heap"
          OpMemoryModel Logical GLSL450
          OpEntryPoint GLCompute %main "main"
          OpExecutionMode %main LocalSize 1 1 1
          OpMemberDecorateIdEXT %struct 0 OffsetIdEXT %float_0
          OpMemberDecorateIdEXT %struct 1 OffsetIdEXT %uint_0
          OpMemberDecorateIdEXT %struct 2 OffsetIdEXT %uint_0
          %uint = OpTypeInt 32 0
          %uint_0 = OpConstant %uint 0
          %float = OpTypeFloat 32
          %float_0 = OpConstant %float 0
          %14 = OpTypeImage %uint Buffer 0 0 0 2 R32ui
%struct = OpTypeStruct %14 %uint %uint
  %void = OpTypeVoid
     %3 = OpTypeFunction %void
  %main = OpFunction %void None %3
     %5 = OpLabel
          OpReturn
          OpFunctionEnd

  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OffsetIdEXT extra operand must be a 32-bit int scalar type"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, OffsetIdNonConstant) {
  const std::string str = R"(
          OpCapability Shader
          OpCapability SampledBuffer
          OpCapability DescriptorHeapEXT
          OpExtension "SPV_EXT_descriptor_heap"
          OpMemoryModel Logical GLSL450
          OpEntryPoint GLCompute %main "main"
          OpExecutionMode %main LocalSize 1 1 1
          OpMemberDecorateIdEXT %struct 0 OffsetIdEXT %void
          OpMemberDecorateIdEXT %struct 1 OffsetIdEXT %uint_0
          OpMemberDecorateIdEXT %struct 2 OffsetIdEXT %uint_0
          %uint = OpTypeInt 32 0
          %uint_0 = OpConstant %uint 0
          %float = OpTypeFloat 32
          %float_0 = OpConstant %float 0
          %14 = OpTypeImage %uint Buffer 0 0 0 2 R32ui
%struct = OpTypeStruct %14 %uint %uint
  %void = OpTypeVoid
     %3 = OpTypeFunction %void
  %main = OpFunction %void None %3
     %5 = OpLabel
          OpReturn
          OpFunctionEnd

  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OffsetIdEXT extra operand must be a 32-bit int scalar type"));
}

TEST_F(ValidateSpvEXTDescriptorHeap, ArrayStrideFloat) {
  const std::string str = R"(
      OpCapability Shader
      OpCapability DescriptorHeapEXT
      OpExtension "SPV_EXT_descriptor_heap"
      OpMemoryModel Logical GLSL450
      OpEntryPoint GLCompute %main "main"
      OpExecutionMode %main LocalSize 1 1 1
      OpDecorateId %array ArrayStrideIdEXT %float_4
%float = OpTypeFloat 32
%float_4 = OpConstant %float 4
%int = OpTypeInt 32 0
%int_2 = OpConstant %int 2
%sampler = OpTypeSampler
%array = OpTypeArray %sampler %int_2
%void = OpTypeVoid
  %3 = OpTypeFunction %void
%main = OpFunction %void None %3
  %5 = OpLabel
      OpReturn
      OpFunctionEnd

  )";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_4);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "ArrayStrideIdEXT extra operand must be a 32-bit int scalar type"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
