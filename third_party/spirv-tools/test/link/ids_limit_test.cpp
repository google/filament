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

class IdsLimit : public spvtest::LinkerTest {
 public:
  IdsLimit() { binaries.reserve(2u); }

  void SetUp() override {
    const uint32_t id_bound = SPV_LIMIT_RESULT_ID_BOUND - 1u;
    const uint32_t constant_count =
        id_bound -
        2u;  // One ID is used for TypeBool, and (constant_count + 1) < id_bound

    // This is needed, as otherwise the ID bound will get reset to 1 while
    // running the RemoveDuplicates pass.
    spvtest::Binary common_binary = {
        // clang-format off
        SpvMagicNumber,
        SpvVersion,
        SPV_GENERATOR_WORD(SPV_GENERATOR_KHRONOS, 0),
        id_bound,  // NOTE: Bound
        0u,        // NOTE: Schema; reserved

        SpvOpCapability | 2u << SpvWordCountShift,
        SpvCapabilityShader,

        SpvOpMemoryModel | 3u << SpvWordCountShift,
        SpvAddressingModelLogical,
        SpvMemoryModelSimple,

        SpvOpTypeBool | 2u << SpvWordCountShift,
        1u    // NOTE: Result ID
        // clang-format on
    };

    binaries.push_back({});
    spvtest::Binary& binary = binaries.back();
    binary.reserve(common_binary.size() + constant_count * 3u);
    binary.insert(binary.end(), common_binary.cbegin(), common_binary.cend());

    for (uint32_t i = 0u; i < constant_count; ++i) {
      binary.push_back(SpvOpConstantTrue | 3u << SpvWordCountShift);
      binary.push_back(1u);      // NOTE: Type ID
      binary.push_back(2u + i);  // NOTE: Result ID
    }
  }
  void TearDown() override { binaries.clear(); }

  spvtest::Binaries binaries;
};

spvtest::Binary CreateBinary(uint32_t id_bound) {
  return {
      // clang-format off
      // Header
      SpvMagicNumber,
      SpvVersion,
      SPV_GENERATOR_WORD(SPV_GENERATOR_KHRONOS, 0),
      id_bound,  // NOTE: Bound
      0u,        // NOTE: Schema; reserved

      // OpCapability Shader
      SpvOpCapability | 2u << SpvWordCountShift,
      SpvCapabilityShader,

      // OpMemoryModel Logical Simple
      SpvOpMemoryModel | 3u << SpvWordCountShift,
      SpvAddressingModelLogical,
      SpvMemoryModelSimple
      // clang-format on
  };
}

TEST_F(IdsLimit, DISABLED_UnderLimit) {
  spvtest::Binary linked_binary;
  ASSERT_EQ(SPV_SUCCESS, Link(binaries, &linked_binary)) << GetErrorMessage();
  EXPECT_THAT(GetErrorMessage(), std::string());
  EXPECT_EQ(0x3FFFFFu, linked_binary[3]);
}

TEST_F(IdsLimit, DISABLED_OverLimit) {
  spvtest::Binary& binary = binaries.back();

  const uint32_t id_bound = binary[3];
  binary[3] = id_bound + 1u;

  binary.push_back(SpvOpConstantFalse | 3u << SpvWordCountShift);
  binary.push_back(1u);        // NOTE: Type ID
  binary.push_back(id_bound);  // NOTE: Result ID

  spvtest::Binary linked_binary;
  ASSERT_EQ(SPV_SUCCESS, Link(binaries, &linked_binary)) << GetErrorMessage();
  EXPECT_THAT(
      GetErrorMessage(),
      HasSubstr("The minimum limit of IDs, 4194303, was exceeded: 4194304 is "
                "the current ID bound."));
  EXPECT_EQ(0x400000u, linked_binary[3]);
}

TEST_F(IdsLimit, DISABLED_Overflow) {
  spvtest::Binaries binaries = {CreateBinary(0xFFFFFFFFu),
                                CreateBinary(0x00000002u)};

  spvtest::Binary linked_binary;

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, Link(binaries, &linked_binary));
  EXPECT_THAT(
      GetErrorMessage(),
      HasSubstr("Too many IDs (4294967296): combining all modules would "
                "overflow the 32-bit word of the SPIR-V header."));
}

}  // namespace
}  // namespace spvtools
