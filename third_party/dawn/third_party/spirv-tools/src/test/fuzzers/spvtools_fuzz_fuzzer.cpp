// Copyright (c) 2021 Google LLC
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

#include <cstdint>
#include <vector>

#include "source/fuzz/fuzzer.h"
#include "source/fuzz/pseudo_random_generator.h"
#include "spirv-tools/libspirv.hpp"
#include "test/fuzzers/random_generator.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size == 0 || (size % sizeof(uint32_t)) != 0) {
    // An empty binary, or a binary whose size is not a multiple of word-size,
    // cannot be valid, so can be rejected immediately.
    return 0;
  }

  std::vector<uint32_t> initial_binary(size / sizeof(uint32_t));
  memcpy(initial_binary.data(), data, size);

  spvtools::ValidatorOptions validator_options;

  spvtools::MessageConsumer message_consumer =
      [](spv_message_level_t, const char*, const spv_position_t&, const char*) {
      };

  spvtools::fuzzers::RandomGenerator random_gen(data, size);
  auto target_env = random_gen.GetTargetEnv();
  std::unique_ptr<spvtools::opt::IRContext> ir_context;
  if (!spvtools::fuzz::fuzzerutil::BuildIRContext(
          target_env, message_consumer, initial_binary, validator_options,
          &ir_context)) {
    // The input is invalid - give up.
    return 0;
  }

  std::vector<spvtools::fuzz::fuzzerutil::ModuleSupplier> donor_suppliers = {
      [&initial_binary, message_consumer, target_env,
       &validator_options]() -> std::unique_ptr<spvtools::opt::IRContext> {
        std::unique_ptr<spvtools::opt::IRContext> result;
        if (!spvtools::fuzz::fuzzerutil::BuildIRContext(
                target_env, message_consumer, initial_binary, validator_options,
                &result)) {
          // The input was successfully parsed and validated first time around,
          // so something is wrong if it is now invalid.
          abort();
        }
        return result;
      }};

  uint32_t seed = random_gen.GetUInt32(std::numeric_limits<uint32_t>::max());
  auto fuzzer_context = spvtools::MakeUnique<spvtools::fuzz::FuzzerContext>(
      spvtools::MakeUnique<spvtools::fuzz::PseudoRandomGenerator>(seed),
      spvtools::fuzz::FuzzerContext::GetMinFreshId(ir_context.get()), false);

  auto transformation_context =
      spvtools::MakeUnique<spvtools::fuzz::TransformationContext>(
          spvtools::MakeUnique<spvtools::fuzz::FactManager>(ir_context.get()),
          validator_options);

  spvtools::fuzz::Fuzzer fuzzer(
      std::move(ir_context), std::move(transformation_context),
      std::move(fuzzer_context), message_consumer, donor_suppliers, false,
      spvtools::fuzz::RepeatedPassStrategy::kLoopedWithRecommendations, true,
      validator_options);
  fuzzer.Run(0);
  return 0;
}
