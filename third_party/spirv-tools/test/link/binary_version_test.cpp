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
using BinaryVersion = spvtest::LinkerTest;

spvtest::Binary CreateBinary(uint32_t version) {
  return {
      // clang-format off
      // Header
      static_cast<uint32_t>(spv::MagicNumber),
      version,
      SPV_GENERATOR_WORD(SPV_GENERATOR_KHRONOS, 0),
      1u,  // NOTE: Bound
      0u,  // NOTE: Schema; reserved

      // OpCapability Shader
      static_cast<uint32_t>(spv::Op::OpCapability) | 2u << spv::WordCountShift,
      static_cast<uint32_t>(spv::Capability::Shader),

      // OpMemoryModel Logical Simple
      static_cast<uint32_t>(spv::Op::OpMemoryModel) | 3u << spv::WordCountShift,
      static_cast<uint32_t>(spv::AddressingModel::Logical),
      static_cast<uint32_t>(spv::MemoryModel::Simple)
      // clang-format on
  };
}

TEST_F(BinaryVersion, Match) {
  // clang-format off
  spvtest::Binaries binaries = {
      CreateBinary(SPV_SPIRV_VERSION_WORD(1, 3)),
      CreateBinary(SPV_SPIRV_VERSION_WORD(1, 3)),
  };
  // clang-format on
  spvtest::Binary linked_binary;
  ASSERT_EQ(SPV_SUCCESS, Link(binaries, &linked_binary)) << GetErrorMessage();
  EXPECT_THAT(GetErrorMessage(), std::string());
  EXPECT_EQ(SPV_SPIRV_VERSION_WORD(1, 3), linked_binary[1]);
}

TEST_F(BinaryVersion, Mismatch) {
  // clang-format off
  spvtest::Binaries binaries = {
      CreateBinary(SPV_SPIRV_VERSION_WORD(1, 3)),
      CreateBinary(SPV_SPIRV_VERSION_WORD(1, 5)),
  };
  // clang-format on
  spvtest::Binary linked_binary;
  ASSERT_EQ(SPV_ERROR_INTERNAL, Link(binaries, &linked_binary))
      << GetErrorMessage();
  EXPECT_THAT(GetErrorMessage(),
              HasSubstr("Conflicting SPIR-V versions: 1.3 (input modules 1 "
                        "through 1) vs 1.5 (input module 2)."));
}

}  // namespace
}  // namespace spvtools
