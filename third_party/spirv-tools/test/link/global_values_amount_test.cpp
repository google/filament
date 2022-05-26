// Copyright (c) 2017 Pierre Moreau
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

#include "gmock/gmock.h"
#include "test/link/linker_fixture.h"

namespace spvtools {
namespace {

using ::testing::HasSubstr;

const uint32_t binary_count = 2u;

class EntryPointsAmountTest : public spvtest::LinkerTest {
 public:
  EntryPointsAmountTest() { binaries.reserve(binary_count + 1u); }

  void SetUp() override {
    const uint32_t global_variable_count_per_binary =
        (SPV_LIMIT_GLOBAL_VARIABLES_MAX - 1u) / binary_count;

    spvtest::Binary common_binary = {
        // clang-format off
        SpvMagicNumber,
        SpvVersion,
        SPV_GENERATOR_WORD(SPV_GENERATOR_KHRONOS, 0),
        3u + global_variable_count_per_binary,  // NOTE: Bound
        0u,                                     // NOTE: Schema; reserved

        SpvOpCapability | 2u << SpvWordCountShift,
        SpvCapabilityShader,

        SpvOpMemoryModel | 3u << SpvWordCountShift,
        SpvAddressingModelLogical,
        SpvMemoryModelSimple,

        SpvOpTypeFloat | 3u << SpvWordCountShift,
        1u,   // NOTE: Result ID
        32u,  // NOTE: Width

        SpvOpTypePointer | 4u << SpvWordCountShift,
        2u,  // NOTE: Result ID
        SpvStorageClassInput,
        1u  // NOTE: Type ID
        // clang-format on
    };

    binaries.push_back({});
    spvtest::Binary& binary = binaries.back();
    binary.reserve(common_binary.size() + global_variable_count_per_binary * 4);
    binary.insert(binary.end(), common_binary.cbegin(), common_binary.cend());

    for (uint32_t i = 0u; i < global_variable_count_per_binary; ++i) {
      binary.push_back(SpvOpVariable | 4u << SpvWordCountShift);
      binary.push_back(2u);      // NOTE: Type ID
      binary.push_back(3u + i);  // NOTE: Result ID
      binary.push_back(SpvStorageClassInput);
    }

    for (uint32_t i = 0u; i < binary_count - 1u; ++i) {
      binaries.push_back(binaries.back());
    }
  }
  void TearDown() override { binaries.clear(); }

  spvtest::Binaries binaries;
};

TEST_F(EntryPointsAmountTest, UnderLimit) {
  spvtest::Binary linked_binary;

  ASSERT_EQ(SPV_SUCCESS, Link(binaries, &linked_binary)) << GetErrorMessage();
  EXPECT_THAT(GetErrorMessage(), std::string());
}

TEST_F(EntryPointsAmountTest, OverLimit) {
  binaries.push_back({
      // clang-format off
      SpvMagicNumber,
      SpvVersion,
      SPV_GENERATOR_WORD(SPV_GENERATOR_KHRONOS, 0),
      5u,  // NOTE: Bound
      0u,  // NOTE: Schema; reserved

      SpvOpCapability | 2u << SpvWordCountShift,
      SpvCapabilityShader,

      SpvOpMemoryModel | 3u << SpvWordCountShift,
      SpvAddressingModelLogical,
      SpvMemoryModelSimple,

      SpvOpTypeFloat | 3u << SpvWordCountShift,
      1u,   // NOTE: Result ID
      32u,  // NOTE: Width

      SpvOpTypePointer | 4u << SpvWordCountShift,
      2u,  // NOTE: Result ID
      SpvStorageClassInput,
      1u,  // NOTE: Type ID

      SpvOpVariable | 4u << SpvWordCountShift,
      2u,  // NOTE: Type ID
      3u,  // NOTE: Result ID
      SpvStorageClassInput,

      SpvOpVariable | 4u << SpvWordCountShift,
      2u,  // NOTE: Type ID
      4u,  // NOTE: Result ID
      SpvStorageClassInput
      // clang-format on
  });

  spvtest::Binary linked_binary;
  ASSERT_EQ(SPV_SUCCESS, Link(binaries, &linked_binary)) << GetErrorMessage();
  EXPECT_THAT(
      GetErrorMessage(),
      HasSubstr("The minimum limit of global values, 65535, was exceeded; "
                "65536 global values were found."));
}

}  // namespace
}  // namespace spvtools
