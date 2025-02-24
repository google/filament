// Copyright (c) 2022 Google LLC.
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

#include "diff_test_utils.h"

#include "source/opt/build_module.h"
#include "source/opt/ir_context.h"

#include "spirv-tools/libspirv.hpp"
#include "tools/io.h"
#include "tools/util/cli_consumer.h"

#include "gtest/gtest.h"

namespace spvtools {
namespace diff {

static constexpr auto kDefaultEnvironment = SPV_ENV_UNIVERSAL_1_6;

void DoStringDiffTest(const std::string& src_spirv,
                      const std::string& dst_spirv,
                      const std::string& expected_diff, Options options) {
  // Load the src and dst modules
  std::unique_ptr<spvtools::opt::IRContext> src = spvtools::BuildModule(
      kDefaultEnvironment, spvtools::utils::CLIMessageConsumer, src_spirv,
      spvtools::SpirvTools::kDefaultAssembleOption |
          SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_TRUE(src);

  std::unique_ptr<spvtools::opt::IRContext> dst = spvtools::BuildModule(
      kDefaultEnvironment, spvtools::utils::CLIMessageConsumer, dst_spirv,
      spvtools::SpirvTools::kDefaultAssembleOption |
          SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_TRUE(dst);

  // Take the diff
  std::ostringstream diff_result;
  spv_result_t result =
      spvtools::diff::Diff(src.get(), dst.get(), diff_result, options);
  ASSERT_EQ(result, SPV_SUCCESS);

  // Expect they match
  EXPECT_EQ(diff_result.str(), expected_diff);
}

}  // namespace diff
}  // namespace spvtools
