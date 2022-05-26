// Copyright (c) 2016 Google Inc.
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

#include <vector>

#include "gmock/gmock.h"
#include "source/spirv_target_env.h"
#include "test/unit_spirv.h"

namespace spvtools {
namespace {

using ::testing::AnyOf;
using ::testing::Eq;
using ::testing::StartsWith;
using ::testing::ValuesIn;

using TargetEnvTest = ::testing::TestWithParam<spv_target_env>;
TEST_P(TargetEnvTest, CreateContext) {
  spv_target_env env = GetParam();
  spv_context context = spvContextCreate(env);
  ASSERT_NE(nullptr, context);
  spvContextDestroy(context);  // Avoid leaking
}

TEST_P(TargetEnvTest, ValidDescription) {
  const char* description = spvTargetEnvDescription(GetParam());
  ASSERT_NE(nullptr, description);
  ASSERT_THAT(description, StartsWith("SPIR-V "));
}

TEST_P(TargetEnvTest, ValidSpirvVersion) {
  auto spirv_version = spvVersionForTargetEnv(GetParam());
  ASSERT_THAT(spirv_version, AnyOf(0x10000, 0x10100, 0x10200, 0x10300));
}

INSTANTIATE_TEST_SUITE_P(AllTargetEnvs, TargetEnvTest,
                         ValuesIn(spvtest::AllTargetEnvironments()));

TEST(GetContextTest, InvalidTargetEnvProducesNull) {
  // Use a value beyond the last valid enum value.
  spv_context context = spvContextCreate(static_cast<spv_target_env>(30));
  EXPECT_EQ(context, nullptr);
}

// A test case for parsing an environment string.
struct ParseCase {
  const char* input;
  bool success;        // Expect to successfully parse?
  spv_target_env env;  // The parsed environment, if successful.
};

using TargetParseTest = ::testing::TestWithParam<ParseCase>;

TEST_P(TargetParseTest, Samples) {
  spv_target_env env;
  bool parsed = spvParseTargetEnv(GetParam().input, &env);
  EXPECT_THAT(parsed, Eq(GetParam().success));
  if (parsed) {
    EXPECT_THAT(env, Eq(GetParam().env));
  }
}

INSTANTIATE_TEST_SUITE_P(
    TargetParsing, TargetParseTest,
    ValuesIn(std::vector<ParseCase>{
        {"spv1.0", true, SPV_ENV_UNIVERSAL_1_0},
        {"spv1.1", true, SPV_ENV_UNIVERSAL_1_1},
        {"spv1.2", true, SPV_ENV_UNIVERSAL_1_2},
        {"spv1.3", true, SPV_ENV_UNIVERSAL_1_3},
        {"vulkan1.0", true, SPV_ENV_VULKAN_1_0},
        {"vulkan1.1", true, SPV_ENV_VULKAN_1_1},
        {"vulkan1.2", true, SPV_ENV_VULKAN_1_2},
        {"opencl2.1", true, SPV_ENV_OPENCL_2_1},
        {"opencl2.2", true, SPV_ENV_OPENCL_2_2},
        {"opengl4.0", true, SPV_ENV_OPENGL_4_0},
        {"opengl4.1", true, SPV_ENV_OPENGL_4_1},
        {"opengl4.2", true, SPV_ENV_OPENGL_4_2},
        {"opengl4.3", true, SPV_ENV_OPENGL_4_3},
        {"opengl4.5", true, SPV_ENV_OPENGL_4_5},
        {"opencl1.2", true, SPV_ENV_OPENCL_1_2},
        {"opencl1.2embedded", true, SPV_ENV_OPENCL_EMBEDDED_1_2},
        {"opencl2.0", true, SPV_ENV_OPENCL_2_0},
        {"opencl2.0embedded", true, SPV_ENV_OPENCL_EMBEDDED_2_0},
        {"opencl2.1embedded", true, SPV_ENV_OPENCL_EMBEDDED_2_1},
        {"opencl2.2embedded", true, SPV_ENV_OPENCL_EMBEDDED_2_2},
        {"opencl2.3", false, SPV_ENV_UNIVERSAL_1_0},
        {"opencl3.0", false, SPV_ENV_UNIVERSAL_1_0},
        {"vulkan1.9", false, SPV_ENV_UNIVERSAL_1_0},
        {"vulkan2.0", false, SPV_ENV_UNIVERSAL_1_0},
        {nullptr, false, SPV_ENV_UNIVERSAL_1_0},
        {"", false, SPV_ENV_UNIVERSAL_1_0},
        {"abc", false, SPV_ENV_UNIVERSAL_1_0},
    }));

// A test case for parsing an environment string.
struct ParseVulkanCase {
  uint32_t vulkan;
  uint32_t spirv;
  bool success;        // Expect to successfully parse?
  spv_target_env env;  // The parsed environment, if successful.
};

using TargetParseVulkanTest = ::testing::TestWithParam<ParseVulkanCase>;

TEST_P(TargetParseVulkanTest, Samples) {
  spv_target_env env;
  bool parsed = spvParseVulkanEnv(GetParam().vulkan, GetParam().spirv, &env);
  EXPECT_THAT(parsed, Eq(GetParam().success));
  if (parsed) {
    EXPECT_THAT(env, Eq(GetParam().env));
  }
}

#define VK(MAJ, MIN) ((MAJ << 22) | (MIN << 12))
#define SPV(MAJ, MIN) ((MAJ << 16) | (MIN << 8))
INSTANTIATE_TEST_SUITE_P(
    TargetVulkanParsing, TargetParseVulkanTest,
    ValuesIn(std::vector<ParseVulkanCase>{
        // Vulkan 1.0 cases
        {VK(1, 0), SPV(1, 0), true, SPV_ENV_VULKAN_1_0},
        {VK(1, 0), SPV(1, 1), true, SPV_ENV_VULKAN_1_1},
        {VK(1, 0), SPV(1, 2), true, SPV_ENV_VULKAN_1_1},
        {VK(1, 0), SPV(1, 3), true, SPV_ENV_VULKAN_1_1},
        {VK(1, 0), SPV(1, 4), true, SPV_ENV_VULKAN_1_1_SPIRV_1_4},
        {VK(1, 0), SPV(1, 5), true, SPV_ENV_VULKAN_1_2},
        {VK(1, 0), SPV(1, 6), true, SPV_ENV_VULKAN_1_3},
        {VK(1, 0), SPV(1, 7), false, SPV_ENV_UNIVERSAL_1_0},
        // Vulkan 1.1 cases
        {VK(1, 1), SPV(1, 0), true, SPV_ENV_VULKAN_1_1},
        {VK(1, 1), SPV(1, 1), true, SPV_ENV_VULKAN_1_1},
        {VK(1, 1), SPV(1, 2), true, SPV_ENV_VULKAN_1_1},
        {VK(1, 1), SPV(1, 3), true, SPV_ENV_VULKAN_1_1},
        {VK(1, 1), SPV(1, 4), true, SPV_ENV_VULKAN_1_1_SPIRV_1_4},
        {VK(1, 1), SPV(1, 5), true, SPV_ENV_VULKAN_1_2},
        {VK(1, 1), SPV(1, 6), true, SPV_ENV_VULKAN_1_3},
        {VK(1, 1), SPV(1, 7), false, SPV_ENV_UNIVERSAL_1_0},
        // Vulkan 1.2 cases
        {VK(1, 2), SPV(1, 0), true, SPV_ENV_VULKAN_1_2},
        {VK(1, 2), SPV(1, 1), true, SPV_ENV_VULKAN_1_2},
        {VK(1, 2), SPV(1, 2), true, SPV_ENV_VULKAN_1_2},
        {VK(1, 2), SPV(1, 3), true, SPV_ENV_VULKAN_1_2},
        {VK(1, 2), SPV(1, 4), true, SPV_ENV_VULKAN_1_2},
        {VK(1, 2), SPV(1, 5), true, SPV_ENV_VULKAN_1_2},
        {VK(1, 2), SPV(1, 6), true, SPV_ENV_VULKAN_1_3},
        {VK(1, 2), SPV(1, 7), false, SPV_ENV_UNIVERSAL_1_0},
        // Vulkan 1.3 cases
        {VK(1, 3), SPV(1, 0), true, SPV_ENV_VULKAN_1_3},
        {VK(1, 3), SPV(1, 1), true, SPV_ENV_VULKAN_1_3},
        {VK(1, 3), SPV(1, 2), true, SPV_ENV_VULKAN_1_3},
        {VK(1, 3), SPV(1, 3), true, SPV_ENV_VULKAN_1_3},
        {VK(1, 3), SPV(1, 4), true, SPV_ENV_VULKAN_1_3},
        {VK(1, 3), SPV(1, 5), true, SPV_ENV_VULKAN_1_3},
        {VK(1, 3), SPV(1, 6), true, SPV_ENV_VULKAN_1_3},
        {VK(1, 3), SPV(1, 7), false, SPV_ENV_UNIVERSAL_1_0},
        // Vulkan 2.0 cases
        {VK(2, 0), SPV(1, 0), false, SPV_ENV_UNIVERSAL_1_0},
        // Vulkan 99.0 cases
        {VK(99, 0), SPV(1, 0), false, SPV_ENV_UNIVERSAL_1_0},
    }));

}  // namespace
}  // namespace spvtools
