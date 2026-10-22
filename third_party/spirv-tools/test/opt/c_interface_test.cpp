// Copyright (c) 2023 Nintendo
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
#include <iostream>

#include "gtest/gtest.h"
#include "spirv-tools/libspirv.h"

namespace spvtools {
namespace {

TEST(OptimizerCInterface, DefaultConsumerWithValidationNoPassesForInvalidInput) {
  const uint32_t spirv[] = {
      0xDEADFEED, // Invalid Magic
      0x00010100, // Version 1.1
      0x00000000, // No Generator
      0x01000000, // Bound
      0x00000000, // Schema
      0x00020011, // OpCapability
      0x00000001, // Shader
      0x00020011, // OpCapability
      0x00000005, // Linkage
      0x0003000E, // OpMemoryModel
      0x00000000, // Logical
      0x00000001  // GLSL450
  };

  auto optimizer = spvOptimizerCreate(SPV_ENV_UNIVERSAL_1_1);
  ASSERT_NE(optimizer, nullptr);

  // Do not register any passes

  auto options = spvOptimizerOptionsCreate();
  ASSERT_NE(options, nullptr);
  spvOptimizerOptionsSetRunValidator(options, true);

  spv_binary binary = nullptr;
  EXPECT_NE(SPV_SUCCESS,
            spvOptimizerRun(optimizer, spirv, sizeof(spirv) / sizeof(uint32_t),
                            &binary, options));
  ASSERT_EQ(binary, nullptr);

  spvOptimizerOptionsDestroy(options);

  spvOptimizerDestroy(optimizer);
}

TEST(OptimizerCInterface, SpecifyConsumerWithValidationNoPassesForInvalidInput) {
  const uint32_t spirv[] = {
      0xDEADFEED, // Invalid Magic
      0x00010100, // Version 1.1
      0x00000000, // No Generator
      0x01000000, // Bound
      0x00000000, // Schema
      0x00020011, // OpCapability
      0x00000001, // Shader
      0x00020011, // OpCapability
      0x00000005, // Linkage
      0x0003000E, // OpMemoryModel
      0x00000000, // Logical
      0x00000001  // GLSL450
  };

  auto optimizer = spvOptimizerCreate(SPV_ENV_UNIVERSAL_1_1);
  ASSERT_NE(optimizer, nullptr);

  spvOptimizerSetMessageConsumer(
      optimizer,
      [](spv_message_level_t, const char*, const spv_position_t*,
         const char* message) {
        std::cout << message << std::endl;
      });

  // Do not register any passes

  auto options = spvOptimizerOptionsCreate();
  ASSERT_NE(options, nullptr);
  spvOptimizerOptionsSetRunValidator(options, true);

  testing::internal::CaptureStdout();

  spv_binary binary = nullptr;
  EXPECT_NE(SPV_SUCCESS,
            spvOptimizerRun(optimizer, spirv, sizeof(spirv) / sizeof(uint32_t),
                            &binary, options));
  ASSERT_EQ(binary, nullptr);

  auto output = testing::internal::GetCapturedStdout();
  EXPECT_STRNE(output.c_str(), "");

  spvOptimizerOptionsDestroy(options);

  spvOptimizerDestroy(optimizer);
}

TEST(OptimizerCInterface, DefaultConsumerWithValidationNoPassesForValidInput) {
  const uint32_t spirv[] = {
      0x07230203, // Magic
      0x00010100, // Version 1.1
      0x00000000, // No Generator
      0x00000001, // Bound
      0x00000000, // Schema
      0x00020011, // OpCapability
      0x00000001, // Shader
      0x00020011, // OpCapability
      0x00000005, // Linkage
      0x0003000E, // OpMemoryModel
      0x00000000, // Logical
      0x00000001  // GLSL450
  };

  auto optimizer = spvOptimizerCreate(SPV_ENV_UNIVERSAL_1_1);
  ASSERT_NE(optimizer, nullptr);

  // Do not register any passes

  auto options = spvOptimizerOptionsCreate();
  ASSERT_NE(options, nullptr);
  spvOptimizerOptionsSetRunValidator(options, true);

  spv_binary binary = nullptr;
  EXPECT_EQ(SPV_SUCCESS,
            spvOptimizerRun(optimizer, spirv, sizeof(spirv) / sizeof(uint32_t),
                            &binary, options));
  ASSERT_NE(binary, nullptr);

  spvOptimizerOptionsDestroy(options);

  // Should remain unchanged
  EXPECT_EQ(binary->wordCount, sizeof(spirv) / sizeof(uint32_t));
  EXPECT_EQ(memcmp(binary->code, spirv, sizeof(spirv) / sizeof(uint32_t)), 0);

  spvBinaryDestroy(binary);
  spvOptimizerDestroy(optimizer);
}

TEST(OptimizerCInterface, DefaultConsumerNoPassesForValidInput) {
  const uint32_t spirv[] = {
      0x07230203, // Magic
      0x00010100, // Version 1.1
      0x00000000, // No Generator
      0x00000003, // Bound
      0x00000000, // Schema
      0x00020011, // OpCapability
      0x00000001, // Shader
      0x00020011, // OpCapability
      0x00000005, // Linkage
      0x0003000E, // OpMemoryModel
      0x00000000, // Logical
      0x00000001, // GLSL450
      0x00040015, // OpTypeInt
      0x00000001, // %1
      0x00000020, // 32 Bits
      0x00000000, // Unsigned
      0x0004002B, // OpConstant
      0x00000001, // %1
      0x00000002, // %2
      0x00000001  // 1
  };

  auto optimizer = spvOptimizerCreate(SPV_ENV_UNIVERSAL_1_1);
  ASSERT_NE(optimizer, nullptr);

  // Do not register any passes

  auto options = spvOptimizerOptionsCreate();
  ASSERT_NE(options, nullptr);
  spvOptimizerOptionsSetRunValidator(options, true);

  spv_binary binary = nullptr;
  EXPECT_EQ(SPV_SUCCESS,
            spvOptimizerRun(optimizer, spirv, sizeof(spirv) / sizeof(uint32_t),
                            &binary, options));
  ASSERT_NE(binary, nullptr);

  spvOptimizerOptionsDestroy(options);

  // Should remain unchanged
  EXPECT_EQ(binary->wordCount, sizeof(spirv) / sizeof(uint32_t));
  EXPECT_EQ(memcmp(binary->code, spirv, sizeof(spirv) / sizeof(uint32_t)), 0);

  spvBinaryDestroy(binary);
  spvOptimizerDestroy(optimizer);
}

TEST(OptimizerCInterface, DefaultConsumerLegalizationPassesForValidInput) {
  const uint32_t spirv[] = {
      0x07230203, // Magic
      0x00010100, // Version 1.1
      0x00000000, // No Generator
      0x00000003, // Bound
      0x00000000, // Schema
      0x00020011, // OpCapability
      0x00000001, // Shader
      0x00020011, // OpCapability
      0x00000005, // Linkage
      0x0003000E, // OpMemoryModel
      0x00000000, // Logical
      0x00000001, // GLSL450
      0x00040015, // OpTypeInt
      0x00000001, // %1
      0x00000020, // 32 Bits
      0x00000000, // Unsigned
      0x0004002B, // OpConstant
      0x00000001, // %1
      0x00000002, // %2
      0x00000001  // 1
  };

  auto optimizer = spvOptimizerCreate(SPV_ENV_UNIVERSAL_1_1);
  ASSERT_NE(optimizer, nullptr);

  spvOptimizerRegisterLegalizationPasses(optimizer);

  auto options = spvOptimizerOptionsCreate();
  ASSERT_NE(options, nullptr);
  spvOptimizerOptionsSetRunValidator(options, false);

  spv_binary binary = nullptr;
  EXPECT_EQ(SPV_SUCCESS,
            spvOptimizerRun(optimizer, spirv, sizeof(spirv) / sizeof(uint32_t),
                            &binary, options));
  ASSERT_NE(binary, nullptr);

  spvOptimizerOptionsDestroy(options);

  // Only check that SPV_SUCCESS is returned, do not verify output

  spvBinaryDestroy(binary);
  spvOptimizerDestroy(optimizer);
}

TEST(OptimizerCInterface, DefaultConsumerPerformancePassesForValidInput) {
  const uint32_t spirv[] = {
      0x07230203, // Magic
      0x00010100, // Version 1.1
      0x00000000, // No Generator
      0x00000003, // Bound
      0x00000000, // Schema
      0x00020011, // OpCapability
      0x00000001, // Shader
      0x00020011, // OpCapability
      0x00000005, // Linkage
      0x0003000E, // OpMemoryModel
      0x00000000, // Logical
      0x00000001, // GLSL450
      0x00040015, // OpTypeInt
      0x00000001, // %1
      0x00000020, // 32 Bits
      0x00000000, // Unsigned
      0x0004002B, // OpConstant
      0x00000001, // %1
      0x00000002, // %2
      0x00000001  // 1
  };
  const uint32_t expected_spirv[] = {
      0x07230203, // Magic
      0x00010100, // Version 1.1
      0x00000000, // No Generator
      0x00000001, // Bound
      0x00000000, // Schema
      0x00020011, // OpCapability
      0x00000001, // Shader
      0x00020011, // OpCapability
      0x00000005, // Linkage
      0x0003000E, // OpMemoryModel
      0x00000000, // Logical
      0x00000001  // GLSL450
  };

  auto optimizer = spvOptimizerCreate(SPV_ENV_UNIVERSAL_1_1);
  ASSERT_NE(optimizer, nullptr);

  spvOptimizerRegisterPerformancePasses(optimizer);

  auto options = spvOptimizerOptionsCreate();
  ASSERT_NE(options, nullptr);
  spvOptimizerOptionsSetRunValidator(options, false);

  spv_binary binary = nullptr;
  EXPECT_EQ(SPV_SUCCESS,
            spvOptimizerRun(optimizer, spirv, sizeof(spirv) / sizeof(uint32_t),
                            &binary, options));
  ASSERT_NE(binary, nullptr);

  spvOptimizerOptionsDestroy(options);

  // Unreferenced OpTypeInt and OpConstant should be removed
  EXPECT_EQ(binary->wordCount, sizeof(expected_spirv) / sizeof(uint32_t));
  EXPECT_EQ(memcmp(binary->code, expected_spirv,
                   sizeof(expected_spirv) / sizeof(uint32_t)), 0);

  spvBinaryDestroy(binary);
  spvOptimizerDestroy(optimizer);
}

TEST(OptimizerCInterface, DefaultConsumerSizePassesForValidInput) {
  const uint32_t spirv[] = {
      0x07230203, // Magic
      0x00010100, // Version 1.1
      0x00000000, // No Generator
      0x00000003, // Bound
      0x00000000, // Schema
      0x00020011, // OpCapability
      0x00000001, // Shader
      0x00020011, // OpCapability
      0x00000005, // Linkage
      0x0003000E, // OpMemoryModel
      0x00000000, // Logical
      0x00000001, // GLSL450
      0x00040015, // OpTypeInt
      0x00000001, // %1
      0x00000020, // 32 Bits
      0x00000000, // Unsigned
      0x0004002B, // OpConstant
      0x00000001, // %1
      0x00000002, // %2
      0x00000001  // 1
  };
  const uint32_t expected_spirv[] = {
      0x07230203, // Magic
      0x00010100, // Version 1.1
      0x00000000, // No Generator
      0x00000001, // Bound
      0x00000000, // Schema
      0x00020011, // OpCapability
      0x00000001, // Shader
      0x00020011, // OpCapability
      0x00000005, // Linkage
      0x0003000E, // OpMemoryModel
      0x00000000, // Logical
      0x00000001  // GLSL450
  };

  auto optimizer = spvOptimizerCreate(SPV_ENV_UNIVERSAL_1_1);
  ASSERT_NE(optimizer, nullptr);

  spvOptimizerRegisterSizePasses(optimizer);

  auto options = spvOptimizerOptionsCreate();
  ASSERT_NE(options, nullptr);
  spvOptimizerOptionsSetRunValidator(options, false);

  spv_binary binary = nullptr;
  EXPECT_EQ(SPV_SUCCESS,
            spvOptimizerRun(optimizer, spirv, sizeof(spirv) / sizeof(uint32_t),
                            &binary, options));
  ASSERT_NE(binary, nullptr);

  spvOptimizerOptionsDestroy(options);

  // Unreferenced OpTypeInt and OpConstant should be removed
  EXPECT_EQ(binary->wordCount, sizeof(expected_spirv) / sizeof(uint32_t));
  EXPECT_EQ(memcmp(binary->code, expected_spirv,
                   sizeof(expected_spirv) / sizeof(uint32_t)), 0);

  spvBinaryDestroy(binary);
  spvOptimizerDestroy(optimizer);
}

TEST(OptimizerCInterface, DefaultConsumerPassFromFlagForValidInput) {
  const uint32_t spirv[] = {
      0x07230203, // Magic
      0x00010100, // Version 1.1
      0x00000000, // No Generator
      0x00000003, // Bound
      0x00000000, // Schema
      0x00020011, // OpCapability
      0x00000001, // Shader
      0x00020011, // OpCapability
      0x00000005, // Linkage
      0x0003000E, // OpMemoryModel
      0x00000000, // Logical
      0x00000001, // GLSL450
      0x00040015, // OpTypeInt
      0x00000001, // %1
      0x00000020, // 32 Bits
      0x00000000, // Unsigned
      0x0004002B, // OpConstant
      0x00000001, // %1
      0x00000002, // %2
      0x00000001  // 1
  };
  const uint32_t expected_spirv[] = {
      0x07230203, // Magic
      0x00010100, // Version 1.1
      0x00000000, // No Generator
      0x00000001, // Bound
      0x00000000, // Schema
      0x00020011, // OpCapability
      0x00000001, // Shader
      0x00020011, // OpCapability
      0x00000005, // Linkage
      0x0003000E, // OpMemoryModel
      0x00000000, // Logical
      0x00000001  // GLSL450
  };

  auto optimizer = spvOptimizerCreate(SPV_ENV_UNIVERSAL_1_1);
  ASSERT_NE(optimizer, nullptr);

  EXPECT_TRUE(spvOptimizerRegisterPassFromFlag(
      optimizer, "--eliminate-dead-code-aggressive"));

  auto options = spvOptimizerOptionsCreate();
  ASSERT_NE(options, nullptr);
  spvOptimizerOptionsSetRunValidator(options, false);

  spv_binary binary = nullptr;
  EXPECT_EQ(SPV_SUCCESS,
            spvOptimizerRun(optimizer, spirv, sizeof(spirv) / sizeof(uint32_t),
                            &binary, options));
  ASSERT_NE(binary, nullptr);

  spvOptimizerOptionsDestroy(options);

  // Unreferenced OpTypeInt and OpConstant should be removed
  EXPECT_EQ(binary->wordCount, sizeof(expected_spirv) / sizeof(uint32_t));
  EXPECT_EQ(memcmp(binary->code, expected_spirv,
                   sizeof(expected_spirv) / sizeof(uint32_t)), 0);

  spvBinaryDestroy(binary);
  spvOptimizerDestroy(optimizer);
}

TEST(OptimizerCInterface, DefaultConsumerPassesFromFlagsForValidInput) {
  const uint32_t spirv[] = {
      0x07230203, // Magic
      0x00010100, // Version 1.1
      0x00000000, // No Generator
      0x00000003, // Bound
      0x00000000, // Schema
      0x00020011, // OpCapability
      0x00000001, // Shader
      0x00020011, // OpCapability
      0x00000005, // Linkage
      0x0003000E, // OpMemoryModel
      0x00000000, // Logical
      0x00000001, // GLSL450
      0x00040015, // OpTypeInt
      0x00000001, // %1
      0x00000020, // 32 Bits
      0x00000000, // Unsigned
      0x0004002B, // OpConstant
      0x00000001, // %1
      0x00000002, // %2
      0x00000001  // 1
  };
  const uint32_t expected_spirv[] = {
      0x07230203, // Magic
      0x00010100, // Version 1.1
      0x00000000, // No Generator
      0x00000001, // Bound
      0x00000000, // Schema
      0x00020011, // OpCapability
      0x00000001, // Shader
      0x00020011, // OpCapability
      0x00000005, // Linkage
      0x0003000E, // OpMemoryModel
      0x00000000, // Logical
      0x00000001  // GLSL450
  };

  auto optimizer = spvOptimizerCreate(SPV_ENV_UNIVERSAL_1_1);
  ASSERT_NE(optimizer, nullptr);

  const char* flags[2] = {
      "--eliminate-dead-const",
      "--eliminate-dead-code-aggressive"
  };

  EXPECT_TRUE(spvOptimizerRegisterPassesFromFlags(
      optimizer, flags, sizeof(flags) / sizeof(const char*)));

  auto options = spvOptimizerOptionsCreate();
  ASSERT_NE(options, nullptr);
  spvOptimizerOptionsSetRunValidator(options, false);

  spv_binary binary = nullptr;
  EXPECT_EQ(SPV_SUCCESS,
            spvOptimizerRun(optimizer, spirv, sizeof(spirv) / sizeof(uint32_t),
                            &binary, options));
  ASSERT_NE(binary, nullptr);

  spvOptimizerOptionsDestroy(options);

  // Unreferenced OpTypeInt and OpConstant should be removed
  EXPECT_EQ(binary->wordCount, sizeof(expected_spirv) / sizeof(uint32_t));
  EXPECT_EQ(memcmp(binary->code, expected_spirv,
                   sizeof(expected_spirv) / sizeof(uint32_t)), 0);

  spvBinaryDestroy(binary);
  spvOptimizerDestroy(optimizer);
}

TEST(OptimizerCInterface, DefaultConsumerInvalidPassFromFlag) {
  auto optimizer = spvOptimizerCreate(SPV_ENV_UNIVERSAL_1_1);
  ASSERT_NE(optimizer, nullptr);

  EXPECT_FALSE(spvOptimizerRegisterPassFromFlag(
      optimizer, "--this-is-not-a-valid-pass"));

  spvOptimizerDestroy(optimizer);
}

TEST(OptimizerCInterface, DefaultConsumerInvalidPassesFromFlags) {
  auto optimizer = spvOptimizerCreate(SPV_ENV_UNIVERSAL_1_1);
  ASSERT_NE(optimizer, nullptr);

  const char* flags[2] = {
      "--eliminate-dead-const",
      "--this-is-not-a-valid-pass"
  };

  EXPECT_FALSE(spvOptimizerRegisterPassesFromFlags(
      optimizer, flags, sizeof(flags) / sizeof(const char*)));

  spvOptimizerDestroy(optimizer);
}

}  // namespace
}  // namespace spvtools
