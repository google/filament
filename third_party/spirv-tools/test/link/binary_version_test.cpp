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

using BinaryVersion = spvtest::LinkerTest;

TEST_F(BinaryVersion, LinkerChoosesMaxSpirvVersion) {
  // clang-format off
  spvtest::Binaries binaries = {
      {
          SpvMagicNumber,
          0x00010300u,
          SPV_GENERATOR_CODEPLAY,
          1u,  // NOTE: Bound
          0u   // NOTE: Schema; reserved
      },
      {
          SpvMagicNumber,
          0x00010500u,
          SPV_GENERATOR_CODEPLAY,
          1u,  // NOTE: Bound
          0u   // NOTE: Schema; reserved
      },
      {
          SpvMagicNumber,
          0x00010100u,
          SPV_GENERATOR_CODEPLAY,
          1u,  // NOTE: Bound
          0u   // NOTE: Schema; reserved
      }
  };
  // clang-format on
  spvtest::Binary linked_binary;

  ASSERT_EQ(SPV_SUCCESS, Link(binaries, &linked_binary));
  EXPECT_THAT(GetErrorMessage(), std::string());

  EXPECT_EQ(0x00010500u, linked_binary[1]);
}

}  // namespace
}  // namespace spvtools
