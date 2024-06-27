// Copyright (c) 2023 Google Inc.
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

#include "spirv-tools/optimizer.hpp"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using TrimCapabilitiesPassTest = PassTest<::testing::Test>;

TEST_F(TrimCapabilitiesPassTest, CheckKnownAliasTransformations) {
  // Those are expected changes caused by the test process:
  //  - SPV is assembled. -> capability goes from text to number.
  //  - SPV is optimized.
  //  - SPV is disassembled -> capability goes from number to text.
  //  - CHECK rule compares both text versions.
  // Because some capabilities share the same number (aliases), the text
  // compared with the CHECK rules depends on which alias is the first on the
  // SPIRV-Headers enum. This could change, and we want to easily distinguish
  // real failure from alias order change. This test is only here to list known
  // alias transformations. If this test breaks, it's not a bug in the
  // optimization pass, but just the SPIRV-Headers enum order that has changed.
  // If that happens, tests needs to be updated to the correct alias is used in
  // the CHECK rule.
  const std::string kTest = R"(
               OpCapability Linkage
               OpCapability StorageUniform16
               OpCapability StorageUniformBufferBlock16
               OpCapability ShaderViewportIndexLayerNV
               OpCapability FragmentBarycentricNV
               OpCapability ShadingRateNV
               OpCapability ShaderNonUniformEXT
               OpCapability RuntimeDescriptorArrayEXT
               OpCapability InputAttachmentArrayDynamicIndexingEXT
               OpCapability UniformTexelBufferArrayDynamicIndexingEXT
               OpCapability StorageTexelBufferArrayDynamicIndexingEXT
               OpCapability UniformBufferArrayNonUniformIndexingEXT
               OpCapability SampledImageArrayNonUniformIndexingEXT
               OpCapability StorageBufferArrayNonUniformIndexingEXT
               OpCapability StorageImageArrayNonUniformIndexingEXT
               OpCapability InputAttachmentArrayNonUniformIndexingEXT
               OpCapability UniformTexelBufferArrayNonUniformIndexingEXT
               OpCapability StorageTexelBufferArrayNonUniformIndexingEXT
               OpCapability VulkanMemoryModelKHR
               OpCapability VulkanMemoryModelDeviceScopeKHR
               OpCapability PhysicalStorageBufferAddressesEXT
               OpCapability DemoteToHelperInvocationEXT
               OpCapability DotProductInputAllKHR
               OpCapability DotProductInput4x8BitKHR
               OpCapability DotProductInput4x8BitPackedKHR
               OpCapability DotProductKHR
               OpCapability ComputeDerivativeGroupQuadsNV
               OpCapability ComputeDerivativeGroupLinearNV
; CHECK: OpCapability Linkage
; CHECK-NOT: OpCapability StorageUniform16
; CHECK-NOT: OpCapability StorageUniformBufferBlock16
; CHECK-NOT: OpCapability ShaderViewportIndexLayerNV
; CHECK-NOT: OpCapability FragmentBarycentricNV
; CHECK-NOT: OpCapability ShadingRateNV
; CHECK-NOT: OpCapability ShaderNonUniformEXT
; CHECK-NOT: OpCapability RuntimeDescriptorArrayEXT
; CHECK-NOT: OpCapability InputAttachmentArrayDynamicIndexingEXT
; CHECK-NOT: OpCapability UniformTexelBufferArrayDynamicIndexingEXT
; CHECK-NOT: OpCapability StorageTexelBufferArrayDynamicIndexingEXT
; CHECK-NOT: OpCapability UniformBufferArrayNonUniformIndexingEXT
; CHECK-NOT: OpCapability SampledImageArrayNonUniformIndexingEXT
; CHECK-NOT: OpCapability StorageBufferArrayNonUniformIndexingEXT
; CHECK-NOT: OpCapability StorageImageArrayNonUniformIndexingEXT
; CHECK-NOT: OpCapability InputAttachmentArrayNonUniformIndexingEXT
; CHECK-NOT: OpCapability UniformTexelBufferArrayNonUniformIndexingEXT
; CHECK-NOT: OpCapability StorageTexelBufferArrayNonUniformIndexingEXT
; CHECK-NOT: OpCapability VulkanMemoryModelKHR
; CHECK-NOT: OpCapability VulkanMemoryModelDeviceScopeKHR
; CHECK-NOT: OpCapability PhysicalStorageBufferAddressesEXT
; CHECK-NOT: OpCapability DemoteToHelperInvocationEXT
; CHECK-NOT: OpCapability DotProductInputAllKHR
; CHECK-NOT: OpCapability DotProductInput4x8BitKHR
; CHECK-NOT: OpCapability DotProductInput4x8BitPackedKHR
; CHECK-NOT: OpCapability DotProductKHR
; CHECK-NOT: OpCapability ComputeDerivativeGroupQuadsNV
; CHECK-NOT: OpCapability ComputeDerivativeGroupLinearNV
; CHECK: OpCapability UniformAndStorageBuffer16BitAccess
; CHECK: OpCapability StorageBuffer16BitAccess
; CHECK: OpCapability ShaderViewportIndexLayerEXT
; CHECK: OpCapability FragmentBarycentricKHR
; CHECK: OpCapability FragmentDensityEXT
; CHECK: OpCapability ShaderNonUniform
; CHECK: OpCapability RuntimeDescriptorArray
; CHECK: OpCapability InputAttachmentArrayDynamicIndexing
; CHECK: OpCapability UniformTexelBufferArrayDynamicIndexing
; CHECK: OpCapability StorageTexelBufferArrayDynamicIndexing
; CHECK: OpCapability UniformBufferArrayNonUniformIndexing
; CHECK: OpCapability SampledImageArrayNonUniformIndexing
; CHECK: OpCapability StorageBufferArrayNonUniformIndexing
; CHECK: OpCapability StorageImageArrayNonUniformIndexing
; CHECK: OpCapability InputAttachmentArrayNonUniformIndexing
; CHECK: OpCapability UniformTexelBufferArrayNonUniformIndexing
; CHECK: OpCapability StorageTexelBufferArrayNonUniformIndexing
; CHECK: OpCapability VulkanMemoryModel
; CHECK: OpCapability VulkanMemoryModelDeviceScope
; CHECK: OpCapability PhysicalStorageBufferAddresses
; CHECK: OpCapability DemoteToHelperInvocation
; CHECK: OpCapability DotProductInputAll
; CHECK: OpCapability DotProductInput4x8Bit
; CHECK: OpCapability DotProductInput4x8BitPacked
; CHECK: OpCapability DotProduct
               OpMemoryModel Logical Vulkan
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  const auto result =
      SinglePassRunAndMatch<EmptyPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest, LinkagePreventsChanges) {
  const std::string kTest = R"(
               OpCapability Linkage
               OpCapability ClipDistance
               OpCapability CullDistance
               OpCapability DemoteToHelperInvocation
               OpCapability DeviceGroup
               OpCapability DrawParameters
               OpCapability Float16
               OpCapability Float64
               OpCapability FragmentBarycentricKHR
               OpCapability FragmentFullyCoveredEXT
               OpCapability FragmentShadingRateKHR
               OpCapability GroupNonUniform
               OpCapability GroupNonUniformArithmetic
               OpCapability GroupNonUniformBallot
               OpCapability GroupNonUniformQuad
               OpCapability GroupNonUniformShuffle
               OpCapability Image1D
               OpCapability ImageBuffer
               OpCapability ImageGatherExtended
               OpCapability ImageMSArray
               OpCapability ImageQuery
               OpCapability InputAttachment
               OpCapability InputAttachmentArrayNonUniformIndexing
               OpCapability Int16
               OpCapability Int64
               OpCapability Int64Atomics
               OpCapability Int64ImageEXT
               OpCapability MeshShadingNV
               OpCapability MinLod
               OpCapability MultiView
               OpCapability MultiViewport
               OpCapability PhysicalStorageBufferAddresses
               OpCapability RayQueryKHR
               OpCapability RayTracingKHR
               OpCapability RayTracingNV
               OpCapability RayTraversalPrimitiveCullingKHR
               OpCapability RuntimeDescriptorArray
               OpCapability SampleMaskPostDepthCoverage
               OpCapability SampleRateShading
               OpCapability Sampled1D
               OpCapability SampledBuffer
               OpCapability SampledImageArrayNonUniformIndexing
               OpCapability Shader
               OpCapability ShaderClockKHR
               OpCapability ShaderLayer
               OpCapability ShaderNonUniform
               OpCapability ShaderViewportIndex
               OpCapability ShaderViewportIndexLayerEXT
               OpCapability SparseResidency
               OpCapability StencilExportEXT
               OpCapability StorageImageArrayNonUniformIndexingEXT
               OpCapability StorageImageExtendedFormats
               OpCapability StorageImageReadWithoutFormat
               OpCapability StorageImageWriteWithoutFormat
               OpCapability StorageInputOutput16
               OpCapability StoragePushConstant16
               OpCapability StorageTexelBufferArrayNonUniformIndexing
               OpCapability StorageUniform16
               OpCapability StorageUniformBufferBlock16
               OpCapability Tessellation
               OpCapability UniformTexelBufferArrayNonUniformIndexing
               OpCapability VulkanMemoryModel
               OpExtension "SPV_EXT_fragment_fully_covered"
               OpExtension "SPV_EXT_shader_image_int64"
               OpExtension "SPV_EXT_shader_stencil_export"
               OpExtension "SPV_EXT_shader_viewport_index_layer"
               OpExtension "SPV_KHR_fragment_shader_barycentric"
               OpExtension "SPV_KHR_fragment_shading_rate"
               OpExtension "SPV_KHR_post_depth_coverage"
               OpExtension "SPV_KHR_ray_query"
               OpExtension "SPV_KHR_ray_tracing"
               OpExtension "SPV_KHR_shader_clock"
               OpExtension "SPV_NV_mesh_shader"
               OpExtension "SPV_NV_ray_tracing"
               OpExtension "SPV_NV_viewport_array2"
; CHECK: OpCapability Linkage
; CHECK: OpCapability ClipDistance
; CHECK: OpCapability CullDistance
; CHECK: OpCapability DemoteToHelperInvocation
; CHECK: OpCapability DeviceGroup
; CHECK: OpCapability DrawParameters
; CHECK: OpCapability Float16
; CHECK: OpCapability Float64
; CHECK: OpCapability FragmentBarycentricKHR
; CHECK: OpCapability FragmentFullyCoveredEXT
; CHECK: OpCapability FragmentShadingRateKHR
; CHECK: OpCapability GroupNonUniform
; CHECK: OpCapability GroupNonUniformArithmetic
; CHECK: OpCapability GroupNonUniformBallot
; CHECK: OpCapability GroupNonUniformQuad
; CHECK: OpCapability GroupNonUniformShuffle
; CHECK: OpCapability Image1D
; CHECK: OpCapability ImageBuffer
; CHECK: OpCapability ImageGatherExtended
; CHECK: OpCapability ImageMSArray
; CHECK: OpCapability ImageQuery
; CHECK: OpCapability InputAttachment
; CHECK: OpCapability InputAttachmentArrayNonUniformIndexing
; CHECK: OpCapability Int16
; CHECK: OpCapability Int64
; CHECK: OpCapability Int64Atomics
; CHECK: OpCapability Int64ImageEXT
; CHECK: OpCapability MeshShadingNV
; CHECK: OpCapability MinLod
; CHECK: OpCapability MultiView
; CHECK: OpCapability MultiViewport
; CHECK: OpCapability PhysicalStorageBufferAddresses
; CHECK: OpCapability RayQueryKHR
; CHECK: OpCapability RayTracingKHR
; CHECK: OpCapability RayTracingNV
; CHECK: OpCapability RayTraversalPrimitiveCullingKHR
; CHECK: OpCapability RuntimeDescriptorArray
; CHECK: OpCapability SampleMaskPostDepthCoverage
; CHECK: OpCapability SampleRateShading
; CHECK: OpCapability Sampled1D
; CHECK: OpCapability SampledBuffer
; CHECK: OpCapability SampledImageArrayNonUniformIndexing
; CHECK: OpCapability Shader
; CHECK: OpCapability ShaderClockKHR
; CHECK: OpCapability ShaderLayer
; CHECK: OpCapability ShaderNonUniform
; CHECK: OpCapability ShaderViewportIndex
; CHECK: OpCapability ShaderViewportIndexLayerEXT
; CHECK: OpCapability SparseResidency
; CHECK: OpCapability StencilExportEXT
; CHECK: OpCapability StorageImageArrayNonUniformIndexing
; CHECK: OpCapability StorageImageExtendedFormats
; CHECK: OpCapability StorageImageReadWithoutFormat
; CHECK: OpCapability StorageImageWriteWithoutFormat
; CHECK: OpCapability StorageInputOutput16
; CHECK: OpCapability StoragePushConstant16
; CHECK: OpCapability StorageTexelBufferArrayNonUniformIndexing
; CHECK: OpCapability Tessellation
; CHECK: OpCapability UniformTexelBufferArrayNonUniformIndex
; CHECK: OpCapability VulkanMemoryModel
; CHECK: OpExtension "SPV_EXT_fragment_fully_covered"
; CHECK: OpExtension "SPV_EXT_shader_image_int64"
; CHECK: OpExtension "SPV_EXT_shader_stencil_export"
; CHECK: OpExtension "SPV_EXT_shader_viewport_index_layer"
; CHECK: OpExtension "SPV_KHR_fragment_shader_barycentric"
; CHECK: OpExtension "SPV_KHR_fragment_shading_rate"
; CHECK: OpExtension "SPV_KHR_post_depth_coverage"
; CHECK: OpExtension "SPV_KHR_ray_query"
; CHECK: OpExtension "SPV_KHR_ray_tracing"
; CHECK: OpExtension "SPV_KHR_shader_clock"
; CHECK: OpExtension "SPV_NV_mesh_shader"
; CHECK: OpExtension "SPV_NV_ray_tracing"
; CHECK: OpExtension "SPV_NV_viewport_array2"
               OpMemoryModel Logical Vulkan
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest, KeepShader) {
  const std::string kTest = R"(
               OpCapability Shader
; CHECK: OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest, KeepShaderClockWhenInUse) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability Int64
               OpCapability ShaderClockKHR
               OpExtension "SPV_KHR_shader_clock"
; CHECK: OpCapability ShaderClockKHR
; CHECK: OpExtension "SPV_KHR_shader_clock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
      %ulong = OpTypeInt 64 0
      %scope = OpConstant %uint 1
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
          %7 = OpReadClockKHR %ulong %scope
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest, TrimShaderClockWhenUnused) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability Int64
               OpCapability ShaderClockKHR
               OpExtension "SPV_KHR_shader_clock"
; CHECK-NOT: OpCapability ShaderClockKHR
; CHECK-NOT: OpExtension "SPV_KHR_shader_clock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest, AMDShaderBallotExtensionRemains) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability Groups
               OpExtension "SPV_AMD_shader_ballot"
; CHECK: OpCapability Groups
; CHECK: OpExtension "SPV_AMD_shader_ballot"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
          %1 = OpTypeFunction %void
     %uint_0 = OpConstant %uint 0
          %2 = OpFunction %void None %1
          %3 = OpLabel
          %4 = OpGroupIAddNonUniformAMD %uint %uint_0 ExclusiveScan %uint_0
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest, AMDShaderBallotExtensionRemoved) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability Groups
               OpExtension "SPV_AMD_shader_ballot"
; CHECK-NOT: OpCapability Groups
; CHECK-NOT: OpExtension "SPV_AMD_shader_ballot"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
          %2 = OpFunction %void None %1
          %3 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest, MinLod_RemovedIfNotUsed) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Sampled1D
                      OpCapability MinLod
; CHECK-NOT:          OpCapability MinLod
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %1 "main"
              %void = OpTypeVoid
             %float = OpTypeFloat 32
           %v3float = OpTypeVector %float 3
           %v4float = OpTypeVector %float 4
        %type_image = OpTypeImage %float Cube 2 0 0 1 Rgba32f
    %ptr_type_image = OpTypePointer UniformConstant %type_image
      %type_sampler = OpTypeSampler
  %ptr_type_sampler = OpTypePointer UniformConstant %type_sampler
           %float_0 = OpConstant %float 0
         %float_000 = OpConstantComposite %v3float %float_0 %float_0 %float_0
             %image = OpVariable %ptr_type_image UniformConstant
           %sampler = OpVariable %ptr_type_sampler UniformConstant
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                %21 = OpLoad %type_image %image
                %22 = OpLoad %type_sampler %sampler
                %24 = OpSampledImage %type_sampled_image %21 %22
                %25 = OpImageSampleImplicitLod %v4float %24 %float_000
                      OpReturn
                      OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest, MinLod_RemainsWithOpImageSampleImplicitLod) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Sampled1D
                      OpCapability MinLod
; CHECK:              OpCapability MinLod
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %1 "main"
              %void = OpTypeVoid
             %float = OpTypeFloat 32
           %v3float = OpTypeVector %float 3
           %v4float = OpTypeVector %float 4
        %type_image = OpTypeImage %float Cube 2 0 0 1 Rgba32f
    %ptr_type_image = OpTypePointer UniformConstant %type_image
      %type_sampler = OpTypeSampler
  %ptr_type_sampler = OpTypePointer UniformConstant %type_sampler
           %float_0 = OpConstant %float 0
         %float_000 = OpConstantComposite %v3float %float_0 %float_0 %float_0
             %image = OpVariable %ptr_type_image UniformConstant
           %sampler = OpVariable %ptr_type_sampler UniformConstant
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                %21 = OpLoad %type_image %image
                %22 = OpLoad %type_sampler %sampler
                %24 = OpSampledImage %type_sampled_image %21 %22
                %25 = OpImageSampleImplicitLod %v4float %24 %float_000 MinLod %float_0
                      OpReturn
                      OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       MinLod_RemainsWithOpImageSparseSampleImplicitLod) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability SparseResidency
                      OpCapability ImageGatherExtended
                      OpCapability MinLod
; CHECK:              OpCapability MinLod
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint Fragment %2 "main"
                      OpExecutionMode %2 OriginUpperLeft
              %void = OpTypeVoid
              %uint = OpTypeInt 32 0
             %float = OpTypeFloat 32
           %v2float = OpTypeVector %float 2
           %v3float = OpTypeVector %float 3
           %v4float = OpTypeVector %float 4
        %type_image = OpTypeImage %float 2D 2 0 0 1 Unknown
    %ptr_type_image = OpTypePointer UniformConstant %type_image
      %type_sampler = OpTypeSampler
  %ptr_type_sampler = OpTypePointer UniformConstant %type_sampler
%type_sampled_image = OpTypeSampledImage %type_image
     %sparse_struct = OpTypeStruct %uint %v4float
           %float_0 = OpConstant %float 0
          %float_00 = OpConstantComposite %v2float %float_0 %float_0
         %float_000 = OpConstantComposite %v3float %float_0 %float_0 %float_0
             %image = OpVariable %ptr_type_image UniformConstant
           %sampler = OpVariable %ptr_type_sampler UniformConstant
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                %21 = OpLoad %type_image %image
                %22 = OpLoad %type_sampler %sampler
                %24 = OpSampledImage %type_sampled_image %21 %22
                %25 = OpImageSparseSampleImplicitLod %sparse_struct %24 %float_00 MinLod %float_0
                      OpReturn
                      OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest, MinLod_DetectsMinLodWithBitmaskImageOperand) {
  const std::string kTest = R"(
                            OpCapability MinLod
; CHECK:                    OpCapability MinLod
                            OpCapability Shader
                            OpCapability SparseResidency
                            OpCapability ImageGatherExtended
                            OpMemoryModel Logical GLSL450
                            OpEntryPoint Fragment %1 "main"
                            OpExecutionMode %1 OriginUpperLeft
            %type_sampler = OpTypeSampler
                     %int = OpTypeInt 32 1
                   %float = OpTypeFloat 32
                   %v2int = OpTypeVector %int 2
                 %v2float = OpTypeVector %float 2
                 %v4float = OpTypeVector %float 4
             %ptr_sampler = OpTypePointer UniformConstant %type_sampler
              %type_image = OpTypeImage %float 2D 2 0 0 1 Unknown
               %ptr_image = OpTypePointer UniformConstant %type_image
                    %void = OpTypeVoid
                    %uint = OpTypeInt 32 0
      %type_sampled_image = OpTypeSampledImage %type_image
             %type_struct = OpTypeStruct %uint %v4float

                   %int_1 = OpConstant %int 1
                 %float_0 = OpConstant %float 0
                 %float_1 = OpConstant %float 1
                       %8 = OpConstantComposite %v2float %float_0 %float_0
                      %12 = OpConstantComposite %v2int %int_1 %int_1

                       %2 = OpVariable %ptr_sampler UniformConstant
                       %3 = OpVariable %ptr_image UniformConstant
                      %27 = OpTypeFunction %void
                       %1 = OpFunction %void None %27
                      %28 = OpLabel
                      %29 = OpLoad %type_image %3
                      %30 = OpLoad %type_sampler %2
                      %31 = OpSampledImage %type_sampled_image %29 %30
                      %32 = OpImageSparseSampleImplicitLod %type_struct %31 %8 ConstOffset|MinLod %12 %float_0
                            OpReturn
                            OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemainsWithInputPointer_Vulkan1_0) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StorageInputOutput16
; CHECK:              OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
               %ptr = OpTypePointer Input %half
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_0);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemainsWithInputPointer_Vulkan1_1) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StorageInputOutput16
; CHECK-NOT:          OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
               %ptr = OpTypePointer Input %half
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_1);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemainsWithInputPointerArray_Vulkan1_0) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StorageInputOutput16
; CHECK:              OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
              %uint = OpTypeInt 32 0
            %uint_1 = OpConstant %uint 1
             %array = OpTypeArray %half %uint_1
               %ptr = OpTypePointer Input %array
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_0);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemainsWithInputPointerArray_Vulkan1_1) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StorageInputOutput16
; CHECK-NOT:          OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
              %uint = OpTypeInt 32 0
            %uint_1 = OpConstant %uint 1
             %array = OpTypeArray %half %uint_1
               %ptr = OpTypePointer Input %array
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_1);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemainsWithInputPointerStruct_Vulkan1_0) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StorageInputOutput16
; CHECK:              OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
            %struct = OpTypeStruct %half
               %ptr = OpTypePointer Input %struct
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_0);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemainsWithInputPointerStruct_Vulkan1_1) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StorageInputOutput16
; CHECK-NOT:          OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
            %struct = OpTypeStruct %half
               %ptr = OpTypePointer Input %struct
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_1);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemainsWithInputPointerStructOfStruct_Vulkan1_0) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StorageInputOutput16
; CHECK:              OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
             %float = OpTypeFloat 32
            %struct = OpTypeStruct %float %half
            %parent = OpTypeStruct %float %struct
               %ptr = OpTypePointer Input %parent
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_0);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemainsWithInputPointerStructOfStruct_Vulkan1_1) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StorageInputOutput16
; CHECK-NOT:          OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
             %float = OpTypeFloat 32
            %struct = OpTypeStruct %float %half
            %parent = OpTypeStruct %float %struct
               %ptr = OpTypePointer Input %parent
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_1);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemainsWithInputPointerArrayOfStruct_Vulkan1_0) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StorageInputOutput16
; CHECK:              OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
            %struct = OpTypeStruct %half
              %uint = OpTypeInt 32 0
            %uint_1 = OpConstant %uint 1
             %array = OpTypeArray %struct %uint_1
               %ptr = OpTypePointer Input %array
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_0);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemainsWithInputPointerArrayOfStruct_Vulkan1_1) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StorageInputOutput16
; CHECK-NOT:          OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
            %struct = OpTypeStruct %half
              %uint = OpTypeInt 32 0
            %uint_1 = OpConstant %uint 1
             %array = OpTypeArray %struct %uint_1
               %ptr = OpTypePointer Input %array
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_1);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemainsWithInputPointerVector_Vulkan1_0) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StorageInputOutput16
; CHECK:              OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
            %vector = OpTypeVector %half 4
               %ptr = OpTypePointer Input %vector
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_0);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemainsWithInputPointerVector_Vulkan1_1) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StorageInputOutput16
; CHECK-NOT:          OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
            %vector = OpTypeVector %half 4
               %ptr = OpTypePointer Input %vector
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_1);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemainsWithInputPointerMatrix_Vulkan1_0) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StorageInputOutput16
; CHECK:              OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
            %vector = OpTypeVector %half 4
            %matrix = OpTypeMatrix %vector 4
               %ptr = OpTypePointer Input %matrix
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_0);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemainsWithInputPointerMatrix_Vulkan1_1) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StorageInputOutput16
; CHECK-NOT:          OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
            %vector = OpTypeVector %half 4
            %matrix = OpTypeMatrix %vector 4
               %ptr = OpTypePointer Input %matrix
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_1);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_IsRemovedWithoutInputPointer) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK-NOT:          OpCapability StorageInputOutput16
; CHECK-NOT:          OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_0);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemainsWithOutputPointer_Vulkan1_0) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StorageInputOutput16
; CHECK:              OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
               %ptr = OpTypePointer Output %half
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_0);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemainsWithOutputPointer_Vulkan1_1) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StorageInputOutput16
; CHECK-NOT:          OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
               %ptr = OpTypePointer Output %half
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_1);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageInputOutput16_RemovedWithoutOutputPointer) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageInputOutput16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK-NOT:          OpCapability StorageInputOutput16
; CHECK-NOT:          OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StoragePushConstant16_RemainsSimplePointer_Vulkan1_0) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StoragePushConstant16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StoragePushConstant16
; CHECK:              OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
               %ptr = OpTypePointer PushConstant %half
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_0);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StoragePushConstant16_RemainsSimplePointer_Vulkan1_1) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StoragePushConstant16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK:              OpCapability StoragePushConstant16
; CHECK-NOT:          OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
               %ptr = OpTypePointer PushConstant %half
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_1);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest, StoragePushConstant16_RemovedSimplePointer) {
  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StoragePushConstant16
                      OpExtension "SPV_KHR_16bit_storage"
; CHECK-NOT:          OpCapability StoragePushConstant16
; CHECK-NOT:          OpExtension "SPV_KHR_16bit_storage"
                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
               %ptr = OpTypePointer Function %half
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageUniformBufferBlock16_RemainsSimplePointer_Vulkan1_0) {
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/5354
  static_assert(spv::Capability::StorageUniformBufferBlock16 ==
                spv::Capability::StorageBuffer16BitAccess);

  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageBuffer16BitAccess
                      OpExtension "SPV_KHR_16bit_storage"

; CHECK:              OpCapability StorageBuffer16BitAccess
;                                   `-> StorageUniformBufferBlock16
; CHECK:              OpExtension "SPV_KHR_16bit_storage"

                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
                      OpDecorate %struct BufferBlock
              %void = OpTypeVoid
              %half = OpTypeFloat 16
            %struct = OpTypeStruct %half
               %ptr = OpTypePointer Uniform %struct
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_0);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageUniformBufferBlock16_RemainsSimplePointer_Vulkan1_1) {
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/5354
  static_assert(spv::Capability::StorageUniformBufferBlock16 ==
                spv::Capability::StorageBuffer16BitAccess);

  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageBuffer16BitAccess
                      OpExtension "SPV_KHR_16bit_storage"

; CHECK:              OpCapability StorageBuffer16BitAccess
;                                   `-> StorageUniformBufferBlock16
; CHECK-NOT:          OpExtension "SPV_KHR_16bit_storage"

                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
                      OpDecorate %struct BufferBlock
              %void = OpTypeVoid
              %half = OpTypeFloat 16
            %struct = OpTypeStruct %half
               %ptr = OpTypePointer Uniform %struct
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_1);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageUniformBufferBlock16_RemovedSimplePointer) {
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/5354
  static_assert(spv::Capability::StorageUniformBufferBlock16 ==
                spv::Capability::StorageBuffer16BitAccess);

  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageBuffer16BitAccess
                      OpExtension "SPV_KHR_16bit_storage"

; CHECK-NOT:          OpCapability StorageBuffer16BitAccess
;                                   `-> StorageUniformBufferBlock16
; CHECK-NOT:          OpExtension "SPV_KHR_16bit_storage"

                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
            %struct = OpTypeStruct %half
               %ptr = OpTypePointer Function %struct
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageUniform16_RemovedWithBufferBlockPointer_Vulkan1_0) {
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/5354
  static_assert(spv::Capability::StorageUniformBufferBlock16 ==
                spv::Capability::StorageBuffer16BitAccess);
  static_assert(spv::Capability::StorageUniform16 ==
                spv::Capability::UniformAndStorageBuffer16BitAccess);

  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageBuffer16BitAccess
                      OpCapability UniformAndStorageBuffer16BitAccess
                      OpExtension "SPV_KHR_16bit_storage"

; CHECK:              OpCapability StorageBuffer16BitAccess
;                                   `-> StorageUniformBufferBlock16
; CHECK-NOT:          OpCapability UniformAndStorageBuffer16BitAccess
;                                   `-> StorageUniform16
; CHECK:              OpExtension "SPV_KHR_16bit_storage"

                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
                      OpDecorate %struct BufferBlock
              %void = OpTypeVoid
              %half = OpTypeFloat 16
            %struct = OpTypeStruct %half
               %ptr = OpTypePointer Uniform %struct
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_0);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageUniform16_RemovedWithBufferBlockPointer_Vulkan1_1) {
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/5354
  static_assert(spv::Capability::StorageUniformBufferBlock16 ==
                spv::Capability::StorageBuffer16BitAccess);
  static_assert(spv::Capability::StorageUniform16 ==
                spv::Capability::UniformAndStorageBuffer16BitAccess);

  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageBuffer16BitAccess
                      OpCapability UniformAndStorageBuffer16BitAccess
                      OpExtension "SPV_KHR_16bit_storage"

; CHECK:              OpCapability StorageBuffer16BitAccess
;                                   `-> StorageUniformBufferBlock16
; CHECK-NOT:          OpCapability UniformAndStorageBuffer16BitAccess
;                                   `-> StorageUniform16
; CHECK-NOT:          OpExtension "SPV_KHR_16bit_storage"

                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
                      OpDecorate %struct BufferBlock
              %void = OpTypeVoid
              %half = OpTypeFloat 16
            %struct = OpTypeStruct %half
               %ptr = OpTypePointer Uniform %struct
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_1);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageUniform16_RemovedWithNonBlockUniformPointer_Vulkan1_0) {
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/5354
  static_assert(spv::Capability::StorageUniformBufferBlock16 ==
                spv::Capability::StorageBuffer16BitAccess);
  static_assert(spv::Capability::StorageUniform16 ==
                spv::Capability::UniformAndStorageBuffer16BitAccess);

  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageBuffer16BitAccess
                      OpCapability UniformAndStorageBuffer16BitAccess
                      OpExtension "SPV_KHR_16bit_storage"

; CHECK-NOT:          OpCapability StorageBuffer16BitAccess
;                                   `-> StorageUniformBufferBlock16
; CHECK:              OpCapability UniformAndStorageBuffer16BitAccess
;                                   `-> StorageUniform16
; CHECK:              OpExtension "SPV_KHR_16bit_storage"

                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
            %struct = OpTypeStruct %half
               %ptr = OpTypePointer Uniform %struct
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_0);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageUniform16_RemovedWithNonBlockUniformPointer_Vulkan1_1) {
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/5354
  static_assert(spv::Capability::StorageUniformBufferBlock16 ==
                spv::Capability::StorageBuffer16BitAccess);
  static_assert(spv::Capability::StorageUniform16 ==
                spv::Capability::UniformAndStorageBuffer16BitAccess);

  const std::string kTest = R"(
                      OpCapability Shader
                      OpCapability Float16
                      OpCapability StorageBuffer16BitAccess
                      OpCapability UniformAndStorageBuffer16BitAccess
                      OpExtension "SPV_KHR_16bit_storage"

; CHECK-NOT:          OpCapability StorageBuffer16BitAccess
;                                   `-> StorageUniformBufferBlock16
; CHECK:              OpCapability UniformAndStorageBuffer16BitAccess
;                                   `-> StorageUniform16
; CHECK-NOT:          OpExtension "SPV_KHR_16bit_storage"

                      OpMemoryModel Logical GLSL450
                      OpEntryPoint GLCompute %2 "main"
              %void = OpTypeVoid
              %half = OpTypeFloat 16
            %struct = OpTypeStruct %half
               %ptr = OpTypePointer Uniform %struct
                 %1 = OpTypeFunction %void
                 %2 = OpFunction %void None %1
                 %3 = OpLabel
                      OpReturn
                      OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_1);
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest, FragmentShaderInterlock_RemovedIfNotUsed) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderPixelInterlockEXT
               OpCapability FragmentShaderSampleInterlockEXT
               OpCapability FragmentShaderShadingRateInterlockEXT
               OpExtension "SPV_EXT_fragment_shader_interlock"
; CHECK-NOT:   OpCapability FragmentShaderPixelInterlockEXT
; CHECK-NOT:   OpCapability FragmentShaderSampleInterlockEXT
; CHECK-NOT:   OpCapability FragmentShaderShadingRateInterlockEXT
; CHECK-NOT:   OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
          %2 = OpFunction %void None %1
          %3 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       FragmentShaderPixelInterlock_RemainsWhenOrderedIsUsed) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderPixelInterlockEXT
               OpCapability FragmentShaderSampleInterlockEXT
               OpCapability FragmentShaderShadingRateInterlockEXT
               OpExtension "SPV_EXT_fragment_shader_interlock"
; CHECK:       OpCapability FragmentShaderPixelInterlockEXT
; CHECK-NOT:   OpCapability FragmentShaderSampleInterlockEXT
; CHECK-NOT:   OpCapability FragmentShaderShadingRateInterlockEXT
; CHECK:       OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %main PixelInterlockOrderedEXT
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
          %2 = OpFunction %void None %1
          %3 = OpLabel
               OpBeginInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       FragmentShaderPixelInterlock_RemainsWhenUnorderedIsUsed) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderPixelInterlockEXT
               OpCapability FragmentShaderSampleInterlockEXT
               OpCapability FragmentShaderShadingRateInterlockEXT
               OpExtension "SPV_EXT_fragment_shader_interlock"
; CHECK:       OpCapability FragmentShaderPixelInterlockEXT
; CHECK-NOT:   OpCapability FragmentShaderSampleInterlockEXT
; CHECK-NOT:   OpCapability FragmentShaderShadingRateInterlockEXT
; CHECK:       OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %main PixelInterlockUnorderedEXT
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
          %2 = OpFunction %void None %1
          %3 = OpLabel
               OpBeginInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       FragmentShaderSampleInterlock_RemainsWhenOrderedIsUsed) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderPixelInterlockEXT
               OpCapability FragmentShaderSampleInterlockEXT
               OpCapability FragmentShaderShadingRateInterlockEXT
               OpExtension "SPV_EXT_fragment_shader_interlock"
; CHECK-NOT:   OpCapability FragmentShaderPixelInterlockEXT
; CHECK:       OpCapability FragmentShaderSampleInterlockEXT
; CHECK-NOT:   OpCapability FragmentShaderShadingRateInterlockEXT
; CHECK:       OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %main SampleInterlockOrderedEXT
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
          %2 = OpFunction %void None %1
          %3 = OpLabel
               OpBeginInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       FragmentShaderSampleInterlock_RemainsWhenUnorderedIsUsed) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderPixelInterlockEXT
               OpCapability FragmentShaderSampleInterlockEXT
               OpCapability FragmentShaderShadingRateInterlockEXT
               OpExtension "SPV_EXT_fragment_shader_interlock"
; CHECK-NOT:   OpCapability FragmentShaderPixelInterlockEXT
; CHECK:       OpCapability FragmentShaderSampleInterlockEXT
; CHECK-NOT:   OpCapability FragmentShaderShadingRateInterlockEXT
; CHECK:       OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %main SampleInterlockUnorderedEXT
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
          %2 = OpFunction %void None %1
          %3 = OpLabel
               OpBeginInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       FragmentShaderShadingRateInterlock_RemainsWhenOrderedIsUsed) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderPixelInterlockEXT
               OpCapability FragmentShaderSampleInterlockEXT
               OpCapability FragmentShaderShadingRateInterlockEXT
               OpExtension "SPV_EXT_fragment_shader_interlock"
; CHECK-NOT:   OpCapability FragmentShaderPixelInterlockEXT
; CHECK-NOT:   OpCapability FragmentShaderSampleInterlockEXT
; CHECK:       OpCapability FragmentShaderShadingRateInterlockEXT
; CHECK:       OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %main ShadingRateInterlockOrderedEXT
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
          %2 = OpFunction %void None %1
          %3 = OpLabel
               OpBeginInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       FragmentShaderShadingRateInterlock_RemainsWhenUnorderedIsUsed) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderPixelInterlockEXT
               OpCapability FragmentShaderSampleInterlockEXT
               OpCapability FragmentShaderShadingRateInterlockEXT
               OpExtension "SPV_EXT_fragment_shader_interlock"
; CHECK-NOT:   OpCapability FragmentShaderPixelInterlockEXT
; CHECK-NOT:   OpCapability FragmentShaderSampleInterlockEXT
; CHECK:       OpCapability FragmentShaderShadingRateInterlockEXT
; CHECK:       OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %main ShadingRateInterlockUnorderedEXT
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
          %2 = OpFunction %void None %1
          %3 = OpLabel
               OpBeginInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest, Int64_RemovedWhenUnused) {
  const std::string kTest = R"(
               OpCapability Int64
; CHECK-NOT:   OpCapability Int64
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest, Int64_RemainsWhenUsed) {
  const std::string kTest = R"(
               OpCapability Int64
; CHECK:       OpCapability Int64
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
        %int = OpTypeInt 64 0
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest, RayQueryKHR_RemovedWhenUnused) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability RayQueryKHR
               OpExtension "SPV_KHR_ray_query"
; CHECK-NOT:   OpCapability RayQueryKHR
; CHECK-NOT:   OpExtension "SPV_KHR_ray_query"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %out_var_TEXCOORD1
               OpSource HLSL 660
               OpName %out_var_TEXCOORD1 "out.var.TEXCOORD1"
               OpName %main "main"
               OpDecorate %out_var_TEXCOORD1 Flat
               OpDecorate %out_var_TEXCOORD1 Location 0
       %uint = OpTypeInt 32 0
  %uint_1234 = OpConstant %uint 1234
%_ptr_Output_uint = OpTypePointer Output %uint
       %void = OpTypeVoid
          %7 = OpTypeFunction %void
%out_var_TEXCOORD1 = OpVariable %_ptr_Output_uint Output
       %main = OpFunction %void None %7
          %8 = OpLabel
               OpStore %out_var_TEXCOORD1 %uint_1234
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       RayQueryKHR_RemainsWhenAccelerationStructureIsPresent) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability RayQueryKHR
               OpExtension "SPV_KHR_ray_query"
; CHECK:       OpCapability RayQueryKHR
; CHECK:       OpExtension "SPV_KHR_ray_query"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 2 4
               OpDecorate %var_bvh DescriptorSet 0
               OpDecorate %var_bvh Binding 0
        %bvh = OpTypeAccelerationStructureKHR
    %ptr_bvh = OpTypePointer UniformConstant %bvh
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
    %var_bvh = OpVariable %ptr_bvh UniformConstant
       %main = OpFunction %void None %20
         %30 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest, RayQueryKHR_RemainsWhenRayQueryTypeIsPresent) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability RayQueryKHR
               OpExtension "SPV_KHR_ray_query"
; CHECK:       OpCapability RayQueryKHR
; CHECK:       OpExtension "SPV_KHR_ray_query"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 2 4
      %query = OpTypeRayQueryKHR
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
  %ptr_query = OpTypePointer Function %query
       %main = OpFunction %void None %20
         %30 = OpLabel
  %var_query = OpVariable %ptr_query Function
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest, RayQueryKHR_RemainsWhenUsed) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability RayQueryKHR
               OpExtension "SPV_KHR_ray_query"
; CHECK:       OpCapability RayQueryKHR
; CHECK:       OpExtension "SPV_KHR_ray_query"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 2 4
               OpDecorate %bvh DescriptorSet 0
               OpDecorate %bvh Binding 0
               OpDecorate %output DescriptorSet 0
               OpDecorate %output Binding 1
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %type_RWStructuredBuffer_float 0 Offset 0
               OpDecorate %type_RWStructuredBuffer_float BufferBlock
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
        %int = OpTypeInt 32 1
    %v3float = OpTypeVector %float 3
         %12 = OpConstantComposite %v3float %float_0 %float_0 %float_0
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
%accelerationStructureKHR = OpTypeAccelerationStructureKHR
%_ptr_UniformConstant_accelerationStructureKHR = OpTypePointer UniformConstant %accelerationStructureKHR
%_runtimearr_float = OpTypeRuntimeArray %float
%type_RWStructuredBuffer_float = OpTypeStruct %_runtimearr_float
%_ptr_Uniform_type_RWStructuredBuffer_float = OpTypePointer Uniform %type_RWStructuredBuffer_float
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
%rayQueryKHR = OpTypeRayQueryKHR
%_ptr_Function_rayQueryKHR = OpTypePointer Function %rayQueryKHR
       %bool = OpTypeBool
%_ptr_Uniform_float = OpTypePointer Uniform %float
        %bvh = OpVariable %_ptr_UniformConstant_accelerationStructureKHR UniformConstant
     %output = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_float Uniform
       %main = OpFunction %void None %20
         %24 = OpLabel
         %25 = OpVariable %_ptr_Function_rayQueryKHR Function
         %26 = OpLoad %accelerationStructureKHR %bvh
               OpRayQueryInitializeKHR %25 %26 %uint_0 %uint_0 %12 %float_0 %12 %float_0
         %27 = OpRayQueryProceedKHR %bool %25
         %28 = OpRayQueryGetIntersectionTypeKHR %uint %25 %uint_1
         %29 = OpIEqual %bool %28 %uint_1
               OpSelectionMerge %30 None
               OpBranchConditional %29 %31 %30
         %31 = OpLabel
         %32 = OpAccessChain %_ptr_Uniform_float %output %int_0 %uint_0
               OpStore %32 %float_0
               OpBranch %30
         %30 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       RayTracingKHR_RemainsWithIntersectionExecutionMode) {
  const std::string kTest = R"(
               OpCapability RayTracingKHR
               OpExtension "SPV_KHR_ray_tracing"
; CHECK:       OpCapability RayTracingKHR
; CHECK:       OpExtension "SPV_KHR_ray_tracing"
               OpMemoryModel Logical GLSL450
               OpEntryPoint IntersectionKHR %main "main"
               OpSource HLSL 660
               OpName %main "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %4 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       RayTracingKHR_RemainsWithClosestHitExecutionMode) {
  const std::string kTest = R"(
               OpCapability RayTracingKHR
               OpExtension "SPV_KHR_ray_tracing"
; CHECK:       OpCapability RayTracingKHR
; CHECK:       OpExtension "SPV_KHR_ray_tracing"
               OpMemoryModel Logical GLSL450
               OpEntryPoint ClosestHitKHR %main "main" %a
               OpSource HLSL 630
               OpName %Payload "Payload"
               OpMemberName %Payload 0 "color"
               OpName %a "a"
               OpName %main "main"
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %Payload = OpTypeStruct %v4float
%ptr_payload = OpTypePointer IncomingRayPayloadKHR %Payload
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
          %a = OpVariable %ptr_payload IncomingRayPayloadKHR
       %main = OpFunction %void None %8
          %9 = OpLabel
         %10 = OpLoad %Payload %a
               OpStore %a %10
               OpReturn
               OpFunctionEnd

  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest, RayTracingKHR_RemainsWithAnyHitExecutionMode) {
  const std::string kTest = R"(
               OpCapability RayTracingKHR
               OpExtension "SPV_KHR_ray_tracing"
; CHECK:       OpCapability RayTracingKHR
; CHECK:       OpExtension "SPV_KHR_ray_tracing"
               OpMemoryModel Logical GLSL450
               OpEntryPoint AnyHitKHR %main "main" %a
               OpSource HLSL 630
               OpName %Payload "Payload"
               OpMemberName %Payload 0 "color"
               OpName %a "a"
               OpName %main "main"
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %Payload = OpTypeStruct %v4float
%ptr_payload = OpTypePointer IncomingRayPayloadKHR %Payload
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
          %a = OpVariable %ptr_payload IncomingRayPayloadKHR
       %main = OpFunction %void None %8
          %9 = OpLabel
         %10 = OpLoad %Payload %a
               OpStore %a %10
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest, RayTracingKHR_RemainsWithMissExecutionMode) {
  const std::string kTest = R"(
               OpCapability RayTracingKHR
               OpExtension "SPV_KHR_ray_tracing"
; CHECK:       OpCapability RayTracingKHR
; CHECK:       OpExtension "SPV_KHR_ray_tracing"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MissKHR %main "main" %a
               OpSource HLSL 630
               OpName %Payload "Payload"
               OpMemberName %Payload 0 "color"
               OpName %a "a"
               OpName %main "main"
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %Payload = OpTypeStruct %v4float
%ptr_payload = OpTypePointer IncomingRayPayloadKHR %Payload
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
          %a = OpVariable %ptr_payload IncomingRayPayloadKHR
       %main = OpFunction %void None %8
          %9 = OpLabel
         %10 = OpLoad %Payload %a
               OpStore %a %10
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       RayTracingKHR_RemainsWithRayGenerationExecutionMode) {
  const std::string kTest = R"(
               OpCapability RayTracingKHR
               OpExtension "SPV_KHR_ray_tracing"
; CHECK:       OpCapability RayTracingKHR
; CHECK:       OpExtension "SPV_KHR_ray_tracing"
               OpMemoryModel Logical GLSL450
               OpEntryPoint RayGenerationKHR %main "main"
               OpSource HLSL 630
               OpName %main "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %4 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       RayTracingKHR_RemainsWithCallableExecutionMode) {
  const std::string kTest = R"(
; CHECK:       OpCapability RayTracingKHR
; CHECK:       OpExtension "SPV_KHR_ray_tracing"
               OpCapability RayTracingKHR
               OpExtension "SPV_KHR_ray_tracing"
               OpMemoryModel Logical GLSL450
               OpEntryPoint CallableKHR %main "main" %a
               OpSource HLSL 660
               OpName %Payload "Payload"
               OpMemberName %Payload 0 "data"
               OpName %a "a"
               OpName %main "main"
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %Payload = OpTypeStruct %v4float
%ptr_payload = OpTypePointer IncomingCallableDataKHR %Payload
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
          %a = OpVariable %ptr_payload IncomingCallableDataKHR
       %main = OpFunction %void None %8
          %9 = OpLabel
         %10 = OpLoad %Payload %a
               OpStore %a %10
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       ImageMSArray_RemainsIfSampledIs2AndArrayedIs1) {
  const std::string kTest = R"(
               OpCapability ImageMSArray
 ; CHECK:      OpCapability ImageMSArray
               OpCapability Shader
               OpCapability StorageImageMultisample
               OpCapability StorageImageReadWithoutFormat
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %var_image DescriptorSet 0
               OpDecorate %var_image Binding 1
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %f32 = OpTypeFloat 32
        %u32 = OpTypeInt 32 0
     %uint_2 = OpConstant %u32 2
     %uint_1 = OpConstant %u32 1
     %v2uint = OpTypeVector %u32 2
    %v4float = OpTypeVector %f32 4
    %image = OpTypeImage %f32 2D 2 1 1 2 Unknown
%ptr_image = OpTypePointer UniformConstant %image
       %10 = OpConstantComposite %v2uint %uint_1 %uint_2
%var_image = OpVariable %ptr_image UniformConstant
     %main = OpFunction %void None %func
 %main_lab = OpLabel
       %18 = OpLoad %image %var_image
       %19 = OpImageRead %v4float %18 %10 Sample %uint_2
             OpReturn
             OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest, ImageMSArray_RemovedIfNotUsed) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability ImageMSArray
; CHECK-NOT:   OpCapability ImageMSArray
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %out_var_SV_Target
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 660
               OpName %out_var_SV_Target "out.var.SV_Target"
               OpName %main "main"
               OpDecorate %out_var_SV_Target Location 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
          %7 = OpTypeFunction %void
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest, ImageMSArray_RemovedIfArrayedIsNot1) {
  const std::string kTest = R"(
               OpCapability ImageMSArray
 ; CHECK-NOT:  OpCapability ImageMSArray
               OpCapability Shader
               OpCapability StorageImageMultisample
               OpCapability StorageImageReadWithoutFormat
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %var_image DescriptorSet 0
               OpDecorate %var_image Binding 1
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %f32 = OpTypeFloat 32
        %u32 = OpTypeInt 32 0
     %uint_2 = OpConstant %u32 2
     %uint_1 = OpConstant %u32 1
     %v2uint = OpTypeVector %u32 2
    %v4float = OpTypeVector %f32 4
    %image = OpTypeImage %f32 2D 2 0 1 2 Unknown
%ptr_image = OpTypePointer UniformConstant %image
       %10 = OpConstantComposite %v2uint %uint_1 %uint_2
%var_image = OpVariable %ptr_image UniformConstant
     %main = OpFunction %void None %func
 %main_lab = OpLabel
       %18 = OpLoad %image %var_image
       %19 = OpImageRead %v4float %18 %10 Sample %uint_2
             OpReturn
             OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest, ImageMSArray_RemovedIfSampledNot2) {
  const std::string kTest = R"(
               OpCapability ImageMSArray
 ; CHECK-NOT:  OpCapability ImageMSArray
               OpCapability Shader
               OpCapability StorageImageReadWithoutFormat
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %var_image DescriptorSet 0
               OpDecorate %var_image Binding 1
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %f32 = OpTypeFloat 32
        %u32 = OpTypeInt 32 0
     %uint_3 = OpConstant %u32 3
     %uint_2 = OpConstant %u32 2
     %uint_1 = OpConstant %u32 1
     %v3uint = OpTypeVector %u32 3
    %v4float = OpTypeVector %f32 4
    %image = OpTypeImage %f32 2D 2 1 0 2 Unknown
%ptr_image = OpTypePointer UniformConstant %image
       %10 = OpConstantComposite %v3uint %uint_1 %uint_2 %uint_3
%var_image = OpVariable %ptr_image UniformConstant
     %main = OpFunction %void None %func
 %main_lab = OpLabel
       %18 = OpLoad %image %var_image
       %19 = OpImageRead %v4float %18 %10
             OpReturn
             OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest, Float64_RemovedWhenUnused) {
  const std::string kTest = R"(
               OpCapability Float64
; CHECK-NOT:   OpCapability Float64
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest, Float64_RemainsWhenUsed) {
  const std::string kTest = R"(
               OpCapability Float64
; CHECK:       OpCapability Float64
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
      %float = OpTypeFloat 64
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       ComputeDerivativeGroupQuads_ReamainsWithExecMode) {
  const std::string kTest = R"(
               OpCapability ComputeDerivativeGroupQuadsNV
               OpCapability ComputeDerivativeGroupLinearNV
; CHECK-NOT:   OpCapability ComputeDerivativeGroupLinearNV
; CHECK:       OpCapability ComputeDerivativeGroupQuadsNV
; CHECK-NOT:   OpCapability ComputeDerivativeGroupLinearNV
               OpCapability Shader
; CHECK:       OpExtension "SPV_NV_compute_shader_derivatives"
               OpExtension "SPV_NV_compute_shader_derivatives"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 DerivativeGroupQuadsNV
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       ComputeDerivativeGroupLinear_ReamainsWithExecMode) {
  const std::string kTest = R"(
               OpCapability ComputeDerivativeGroupLinearNV
               OpCapability ComputeDerivativeGroupQuadsNV
; CHECK-NOT:   OpCapability ComputeDerivativeGroupQuadsNV
; CHECK:       OpCapability ComputeDerivativeGroupLinearNV
; CHECK-NOT:   OpCapability ComputeDerivativeGroupQuadsNV
               OpCapability Shader
; CHECK:       OpExtension "SPV_NV_compute_shader_derivatives"
               OpExtension "SPV_NV_compute_shader_derivatives"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 DerivativeGroupLinearNV
       %void = OpTypeVoid
      %float = OpTypeFloat 64
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageImageReadWithoutFormat_RemovedIfUnused) {
  const std::string kTest = R"(
               OpCapability StorageImageReadWithoutFormat
; CHECK-NOT:   OpCapability StorageImageReadWithoutFormat
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %PSMain "PSMain" %out_var
               OpExecutionMode %PSMain OriginUpperLeft
               OpDecorate %out_var Location 0
      %float = OpTypeFloat 32
     %float4 = OpTypeVector %float 4
    %float_0 = OpConstant %float 0
%float4_0000 = OpConstantComposite %float4 %float_0 %float_0 %float_0 %float_0
 %ptr_float4 = OpTypePointer Output %float4
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
    %out_var = OpVariable %ptr_float4 Output
     %PSMain = OpFunction %void None %9
         %10 = OpLabel
               OpStore %out_var %float4_0000
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageImageReadWithoutFormat_RemovedIfUnusedOpImageFetch) {
  const std::string kTest = R"(
               OpCapability StorageImageReadWithoutFormat
; CHECK-NOT:   OpCapability StorageImageReadWithoutFormat
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %PSMain "PSMain" %out_var
               OpExecutionMode %PSMain OriginUpperLeft
               OpDecorate %out_var Location 0
               OpDecorate %texture DescriptorSet 0
               OpDecorate %texture Binding 1
      %float = OpTypeFloat 32
     %float4 = OpTypeVector %float 4
        %int = OpTypeInt 32 1
       %int2 = OpTypeVector %int 2
 %type_image = OpTypeImage %float 2D 2 0 0 1 Unknown
  %ptr_image = OpTypePointer UniformConstant %type_image
      %int_0 = OpConstant %int 0
    %int2_00 = OpConstantComposite %int2 %int_0 %int_0
 %ptr_float4 = OpTypePointer Output %float4
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
    %texture = OpVariable %ptr_image UniformConstant
    %out_var = OpVariable %ptr_float4 Output
     %PSMain = OpFunction %void None %9
         %10 = OpLabel
         %11 = OpLoad %type_image %texture
         %12 = OpImageFetch %float4 %11 %int2_00 Lod %int_0
               OpStore %out_var %12
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageImageReadWithoutFormat_RemainsWhenRequiredWithRead) {
  const std::string kTest = R"(
               OpCapability StorageImageReadWithoutFormat
; CHECK:       OpCapability StorageImageReadWithoutFormat
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %PSMain "PSMain" %out_var
               OpExecutionMode %PSMain OriginUpperLeft
               OpDecorate %out_var Location 0
               OpDecorate %texture DescriptorSet 0
               OpDecorate %texture Binding 1
      %float = OpTypeFloat 32
     %float4 = OpTypeVector %float 4
        %int = OpTypeInt 32 1
       %int2 = OpTypeVector %int 2
 %type_image = OpTypeImage %float 2D 2 0 0 1 Unknown
  %ptr_image = OpTypePointer UniformConstant %type_image
      %int_0 = OpConstant %int 0
    %int2_00 = OpConstantComposite %int2 %int_0 %int_0
 %ptr_float4 = OpTypePointer Output %float4
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
    %texture = OpVariable %ptr_image UniformConstant
    %out_var = OpVariable %ptr_float4 Output
     %PSMain = OpFunction %void None %9
         %10 = OpLabel
         %11 = OpLoad %type_image %texture
         %12 = OpImageRead %float4 %11 %int2_00 Lod %int_0
               OpStore %out_var %12
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageImageReadWithoutFormat_RemainsWhenRequiredWithSparseRead) {
  const std::string kTest = R"(
               OpCapability StorageImageReadWithoutFormat
; CHECK:       OpCapability StorageImageReadWithoutFormat
               OpCapability SparseResidency
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %PSMain "PSMain"
               OpExecutionMode %PSMain OriginUpperLeft
               OpDecorate %texture DescriptorSet 0
               OpDecorate %texture Binding 1
      %float = OpTypeFloat 32
     %float4 = OpTypeVector %float 4
        %int = OpTypeInt 32 1
       %int2 = OpTypeVector %int 2
 %type_image = OpTypeImage %float 2D 2 0 0 2 Unknown
     %struct = OpTypeStruct %int %float4
  %ptr_image = OpTypePointer UniformConstant %type_image
      %int_0 = OpConstant %int 0
    %int2_00 = OpConstantComposite %int2 %int_0 %int_0
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
    %texture = OpVariable %ptr_image UniformConstant
     %PSMain = OpFunction %void None %9
         %10 = OpLabel
         %11 = OpLoad %type_image %texture
         %12 = OpImageSparseRead %struct %11 %int2_00
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       StorageImageReadWithoutFormat_RemovedWithReadOnSubpassData) {
  const std::string kTest = R"(
               OpCapability StorageImageReadWithoutFormat
; CHECK-NOT:   OpCapability StorageImageReadWithoutFormat
               OpCapability InputAttachment
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %PSMain "PSMain" %out_var
               OpExecutionMode %PSMain OriginUpperLeft
               OpDecorate %out_var Location 0
               OpDecorate %texture DescriptorSet 0
               OpDecorate %texture Binding 1
      %float = OpTypeFloat 32
     %float4 = OpTypeVector %float 4
        %int = OpTypeInt 32 1
       %int2 = OpTypeVector %int 2
 %type_image = OpTypeImage %float SubpassData 2 0 0 2 Unknown
  %ptr_image = OpTypePointer UniformConstant %type_image
      %int_0 = OpConstant %int 0
    %int2_00 = OpConstantComposite %int2 %int_0 %int_0
 %ptr_float4 = OpTypePointer Output %float4
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
    %texture = OpVariable %ptr_image UniformConstant
    %out_var = OpVariable %ptr_float4 Output
     %PSMain = OpFunction %void None %9
         %10 = OpLabel
         %11 = OpLoad %type_image %texture
         %12 = OpImageRead %float4 %11 %int2_00
               OpStore %out_var %12
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest, PhysicalStorageBuffer_RemovedWhenUnused) {
  const std::string kTest = R"(
               OpCapability PhysicalStorageBufferAddresses
; CHECK-NOT:   OpCapability PhysicalStorageBufferAddresses
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       PhysicalStorageBuffer_RemainsWithOpTypeForwardPointer) {
  const std::string kTest = R"(
               OpCapability PhysicalStorageBufferAddresses
; CHECK:       OpCapability PhysicalStorageBufferAddresses
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 2 4
       %void = OpTypeVoid
        %int = OpTypeInt 32 0
     %struct = OpTypeStruct %int
               OpTypeForwardPointer %ptr PhysicalStorageBuffer
        %ptr = OpTypePointer PhysicalStorageBuffer %struct
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       PhysicalStorageBuffer_RemainsWithPhysicalStorageBufferStorage) {
  const std::string kTest = R"(
               OpCapability PhysicalStorageBufferAddresses
; CHECK:       OpCapability PhysicalStorageBufferAddresses
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 2 4
       %void = OpTypeVoid
        %int = OpTypeInt 32 0
     %struct = OpTypeStruct %int
        %ptr = OpTypePointer PhysicalStorageBuffer %struct
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       PhysicalStorageBuffer_RemainsWithRestrictDecoration) {
  const std::string kTest = R"(
               OpCapability PhysicalStorageBufferAddresses
; CHECK:       OpCapability PhysicalStorageBufferAddresses
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 2 4
               OpDecorate %var RestrictPointer
       %void = OpTypeVoid
        %int = OpTypeInt 32 0
     %struct = OpTypeStruct %int
        %ptr = OpTypePointer Function %struct
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %6 = OpLabel
        %var = OpVariable %ptr Function
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       PhysicalStorageBuffer_RemainsWithAliasedDecoration) {
  const std::string kTest = R"(
               OpCapability PhysicalStorageBufferAddresses
; CHECK:       OpCapability PhysicalStorageBufferAddresses
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 2 4
               OpDecorate %var AliasedPointer
       %void = OpTypeVoid
        %int = OpTypeInt 32 0
     %struct = OpTypeStruct %int
        %ptr = OpTypePointer Function %struct
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %6 = OpLabel
        %var = OpVariable %ptr Function
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest, Float16_RemovedWhenUnused) {
  const std::string kTest = R"(
               OpCapability Float16
; CHECK-NOT:   OpCapability Float16
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest, Float16_RemainsWhenUsed) {
  const std::string kTest = R"(
               OpCapability Float16
; CHECK:       OpCapability Float16
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
      %float = OpTypeFloat 16
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest, Int16_RemovedWhenUnused) {
  const std::string kTest = R"(
               OpCapability Int16
; CHECK-NOT:   OpCapability Int16
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest, Int16_RemainsWhenUsed) {
  const std::string kTest = R"(
               OpCapability Int16
; CHECK:       OpCapability Int16
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
        %int = OpTypeInt 16 1
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest, UInt16_RemainsWhenUsed) {
  const std::string kTest = R"(
               OpCapability Int16
; CHECK:       OpCapability Int16
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
       %uint = OpTypeInt 16 0
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(TrimCapabilitiesPassTest,
       VulkanMemoryModelDeviceScope_RemovedWhenUnused) {
  const std::string kTest = R"(
               OpCapability VulkanMemoryModelDeviceScope
; CHECK-NOT:   OpCapability VulkanMemoryModelDeviceScope
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
          %1 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       VulkanMemoryModelDeviceScope_RemovedWhenUsedWithGLSL450) {
  const std::string kTest = R"(
               OpCapability VulkanMemoryModelDeviceScope
; CHECK-NOT:   OpCapability VulkanMemoryModelDeviceScope
               OpCapability Shader
               OpCapability ShaderClockKHR
               OpCapability Int64
               OpExtension "SPV_KHR_shader_clock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 2 4
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
      %ulong = OpTypeInt 64 0
     %uint_1 = OpConstant %uint 1
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %6 = OpLabel
         %22 = OpReadClockKHR %ulong %uint_1 ; Device Scope
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(TrimCapabilitiesPassTest,
       VulkanMemoryModelDeviceScope_RemainsWhenUsedWithVulkan) {
  const std::string kTest = R"(
               OpCapability VulkanMemoryModelDeviceScope
; CHECK:       OpCapability VulkanMemoryModelDeviceScope
               OpCapability Shader
               OpCapability ShaderClockKHR
               OpCapability Int64
               OpExtension "SPV_KHR_shader_clock"
               OpMemoryModel Logical Vulkan
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 2 4
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
      %ulong = OpTypeInt 64 0
     %uint_1 = OpConstant %uint 1
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %6 = OpLabel
         %22 = OpReadClockKHR %ulong %uint_1 ; Device Scope
               OpReturn
               OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<TrimCapabilitiesPass>(kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
