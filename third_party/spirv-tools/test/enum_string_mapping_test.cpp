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
#include "source/table2.h"

namespace spvtools {
namespace {

using ::testing::Values;
using ::testing::ValuesIn;

using ExtensionTest =
    ::testing::TestWithParam<std::pair<Extension, std::string>>;
using UnknownExtensionTest = ::testing::TestWithParam<std::string>;

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

}  // namespace
}  // namespace spvtools
