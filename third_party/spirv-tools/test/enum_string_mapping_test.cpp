// Copyright (c) 2017 Google Inc.
// Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights
// reserved.
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
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "source/enum_string_mapping.h"
#include "source/extensions.h"

namespace spvtools {
namespace {

using ::testing::Values;
using ::testing::ValuesIn;

using ExtensionTest =
    ::testing::TestWithParam<std::pair<Extension, std::string>>;
using UnknownExtensionTest = ::testing::TestWithParam<std::string>;
using CapabilityTest =
    ::testing::TestWithParam<std::pair<spv::Capability, std::string>>;

TEST_P(ExtensionTest, TestExtensionFromString) {
  const std::pair<Extension, std::string>& param = GetParam();
  const Extension extension = param.first;
  const std::string extension_str = param.second;
  Extension result_extension;
  ASSERT_TRUE(GetExtensionFromString(extension_str.c_str(), &result_extension));
  EXPECT_EQ(extension, result_extension);
}

TEST_P(ExtensionTest, TestExtensionToString) {
  const std::pair<Extension, std::string>& param = GetParam();
  const Extension extension = param.first;
  const std::string extension_str = param.second;
  const std::string result_str = ExtensionToString(extension);
  EXPECT_EQ(extension_str, result_str);
}

TEST_P(UnknownExtensionTest, TestExtensionFromStringFails) {
  Extension result_extension;
  ASSERT_FALSE(GetExtensionFromString(GetParam().c_str(), &result_extension));
}

TEST_P(CapabilityTest, TestCapabilityToString) {
  const std::pair<spv::Capability, std::string>& param = GetParam();
  const spv::Capability capability = param.first;
  const std::string capability_str = param.second;
  const std::string result_str = CapabilityToString(capability);
  EXPECT_EQ(capability_str, result_str);
}

INSTANTIATE_TEST_SUITE_P(
    AllExtensions, ExtensionTest,
    ValuesIn(std::vector<std::pair<Extension, std::string>>({
        {Extension::kSPV_KHR_16bit_storage, "SPV_KHR_16bit_storage"},
        {Extension::kSPV_KHR_device_group, "SPV_KHR_device_group"},
        {Extension::kSPV_KHR_multiview, "SPV_KHR_multiview"},
        {Extension::kSPV_KHR_shader_ballot, "SPV_KHR_shader_ballot"},
        {Extension::kSPV_KHR_shader_draw_parameters,
         "SPV_KHR_shader_draw_parameters"},
        {Extension::kSPV_KHR_subgroup_vote, "SPV_KHR_subgroup_vote"},
        {Extension::kSPV_NVX_multiview_per_view_attributes,
         "SPV_NVX_multiview_per_view_attributes"},
        {Extension::kSPV_NV_geometry_shader_passthrough,
         "SPV_NV_geometry_shader_passthrough"},
        {Extension::kSPV_NV_sample_mask_override_coverage,
         "SPV_NV_sample_mask_override_coverage"},
        {Extension::kSPV_NV_stereo_view_rendering,
         "SPV_NV_stereo_view_rendering"},
        {Extension::kSPV_NV_viewport_array2, "SPV_NV_viewport_array2"},
        {Extension::kSPV_GOOGLE_decorate_string, "SPV_GOOGLE_decorate_string"},
        {Extension::kSPV_GOOGLE_hlsl_functionality1,
         "SPV_GOOGLE_hlsl_functionality1"},
        {Extension::kSPV_KHR_8bit_storage, "SPV_KHR_8bit_storage"},
    })));

INSTANTIATE_TEST_SUITE_P(UnknownExtensions, UnknownExtensionTest,
                         Values("", "SPV_KHR_", "SPV_KHR_device_group_ERROR",
                                /*alphabetically before all extensions*/ "A",
                                /*alphabetically after all extensions*/ "Z",
                                "SPV_ERROR_random_string_hfsdklhlktherh"));

INSTANTIATE_TEST_SUITE_P(
    AllCapabilities, CapabilityTest,
    ValuesIn(std::vector<std::pair<spv::Capability, std::string>>(
        {{spv::Capability::Matrix, "Matrix"},
         {spv::Capability::Shader, "Shader"},
         {spv::Capability::Geometry, "Geometry"},
         {spv::Capability::Tessellation, "Tessellation"},
         {spv::Capability::Addresses, "Addresses"},
         {spv::Capability::Linkage, "Linkage"},
         {spv::Capability::Kernel, "Kernel"},
         {spv::Capability::Vector16, "Vector16"},
         {spv::Capability::Float16Buffer, "Float16Buffer"},
         {spv::Capability::Float16, "Float16"},
         {spv::Capability::Float64, "Float64"},
         {spv::Capability::Int64, "Int64"},
         {spv::Capability::Int64Atomics, "Int64Atomics"},
         {spv::Capability::ImageBasic, "ImageBasic"},
         {spv::Capability::ImageReadWrite, "ImageReadWrite"},
         {spv::Capability::ImageMipmap, "ImageMipmap"},
         {spv::Capability::Pipes, "Pipes"},
         {spv::Capability::Groups, "Groups"},
         {spv::Capability::DeviceEnqueue, "DeviceEnqueue"},
         {spv::Capability::LiteralSampler, "LiteralSampler"},
         {spv::Capability::AtomicStorage, "AtomicStorage"},
         {spv::Capability::Int16, "Int16"},
         {spv::Capability::TessellationPointSize, "TessellationPointSize"},
         {spv::Capability::GeometryPointSize, "GeometryPointSize"},
         {spv::Capability::ImageGatherExtended, "ImageGatherExtended"},
         {spv::Capability::StorageImageMultisample, "StorageImageMultisample"},
         {spv::Capability::UniformBufferArrayDynamicIndexing,
          "UniformBufferArrayDynamicIndexing"},
         {spv::Capability::SampledImageArrayDynamicIndexing,
          "SampledImageArrayDynamicIndexing"},
         {spv::Capability::StorageBufferArrayDynamicIndexing,
          "StorageBufferArrayDynamicIndexing"},
         {spv::Capability::StorageImageArrayDynamicIndexing,
          "StorageImageArrayDynamicIndexing"},
         {spv::Capability::ClipDistance, "ClipDistance"},
         {spv::Capability::CullDistance, "CullDistance"},
         {spv::Capability::ImageCubeArray, "ImageCubeArray"},
         {spv::Capability::SampleRateShading, "SampleRateShading"},
         {spv::Capability::ImageRect, "ImageRect"},
         {spv::Capability::SampledRect, "SampledRect"},
         {spv::Capability::GenericPointer, "GenericPointer"},
         {spv::Capability::Int8, "Int8"},
         {spv::Capability::InputAttachment, "InputAttachment"},
         {spv::Capability::SparseResidency, "SparseResidency"},
         {spv::Capability::MinLod, "MinLod"},
         {spv::Capability::Sampled1D, "Sampled1D"},
         {spv::Capability::Image1D, "Image1D"},
         {spv::Capability::SampledCubeArray, "SampledCubeArray"},
         {spv::Capability::SampledBuffer, "SampledBuffer"},
         {spv::Capability::ImageBuffer, "ImageBuffer"},
         {spv::Capability::ImageMSArray, "ImageMSArray"},
         {spv::Capability::StorageImageExtendedFormats,
          "StorageImageExtendedFormats"},
         {spv::Capability::ImageQuery, "ImageQuery"},
         {spv::Capability::DerivativeControl, "DerivativeControl"},
         {spv::Capability::InterpolationFunction, "InterpolationFunction"},
         {spv::Capability::TransformFeedback, "TransformFeedback"},
         {spv::Capability::GeometryStreams, "GeometryStreams"},
         {spv::Capability::StorageImageReadWithoutFormat,
          "StorageImageReadWithoutFormat"},
         {spv::Capability::StorageImageWriteWithoutFormat,
          "StorageImageWriteWithoutFormat"},
         {spv::Capability::MultiViewport, "MultiViewport"},
         {spv::Capability::SubgroupDispatch, "SubgroupDispatch"},
         {spv::Capability::NamedBarrier, "NamedBarrier"},
         {spv::Capability::PipeStorage, "PipeStorage"},
         {spv::Capability::SubgroupBallotKHR, "SubgroupBallotKHR"},
         {spv::Capability::DrawParameters, "DrawParameters"},
         {spv::Capability::SubgroupVoteKHR, "SubgroupVoteKHR"},
         {spv::Capability::StorageBuffer16BitAccess,
          "StorageBuffer16BitAccess"},
         {spv::Capability::StorageUniformBufferBlock16,
          "StorageBuffer16BitAccess"},  // Preferred name
         {spv::Capability::UniformAndStorageBuffer16BitAccess,
          "UniformAndStorageBuffer16BitAccess"},
         {spv::Capability::StorageUniform16,
          "UniformAndStorageBuffer16BitAccess"},  // Preferred name
         {spv::Capability::StoragePushConstant16, "StoragePushConstant16"},
         {spv::Capability::StorageInputOutput16, "StorageInputOutput16"},
         {spv::Capability::DeviceGroup, "DeviceGroup"},
         {spv::Capability::AtomicFloat32AddEXT, "AtomicFloat32AddEXT"},
         {spv::Capability::AtomicFloat64AddEXT, "AtomicFloat64AddEXT"},
         {spv::Capability::AtomicFloat32MinMaxEXT, "AtomicFloat32MinMaxEXT"},
         {spv::Capability::AtomicFloat64MinMaxEXT, "AtomicFloat64MinMaxEXT"},
         {spv::Capability::MultiView, "MultiView"},
         {spv::Capability::Int64ImageEXT, "Int64ImageEXT"},
         {spv::Capability::SampleMaskOverrideCoverageNV,
          "SampleMaskOverrideCoverageNV"},
         {spv::Capability::GeometryShaderPassthroughNV,
          "GeometryShaderPassthroughNV"},
         // The next two are different names for the same token.
         {spv::Capability::ShaderViewportIndexLayerNV,
          "ShaderViewportIndexLayerEXT"},
         {spv::Capability::ShaderViewportIndexLayerEXT,
          "ShaderViewportIndexLayerEXT"},
         {spv::Capability::ShaderViewportMaskNV, "ShaderViewportMaskNV"},
         {spv::Capability::ShaderStereoViewNV, "ShaderStereoViewNV"},
         {spv::Capability::PerViewAttributesNV, "PerViewAttributesNV"}})));

}  // namespace
}  // namespace spvtools
