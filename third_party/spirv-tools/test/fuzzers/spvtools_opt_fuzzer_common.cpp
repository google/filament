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

#include "test/fuzzers/spvtools_opt_fuzzer_common.h"

#include "source/opt/build_module.h"
#include "test/fuzzers/random_generator.h"

namespace spvtools {
namespace fuzzers {

int OptFuzzerTestOneInput(
    const uint8_t* data, size_t size,
    const std::function<void(spvtools::Optimizer&)>& register_passes) {
  if (size < 1) {
    return 0;
  }

  spvtools::fuzzers::RandomGenerator random_gen(data, size);
  auto target_env = random_gen.GetTargetEnv();
  spvtools::Optimizer optimizer(target_env);
  optimizer.SetMessageConsumer([](spv_message_level_t, const char*,
                                  const spv_position_t&, const char*) {});

  std::vector<uint32_t> input;
  input.resize(size >> 2);

  size_t count = 0;
  for (size_t i = 0; (i + 3) < size; i += 4) {
    input[count++] = data[i] | (data[i + 1] << 8) | (data[i + 2] << 16) |
                     (data[i + 3]) << 24;
  }

  // The largest possible id bound is used when running the optimizer, to avoid
  // the problem of id overflows.
  const size_t kFinalIdLimit = UINT32_MAX;

  // The input is scanned to check that it does not already use an id too close
  // to this limit. This still gives the optimizer a large set of ids to
  // consume. It is thus very unlikely that id overflow will occur during
  // fuzzing. If it does, then the initial id limit should be decreased.
  const size_t kInitialIdLimit = kFinalIdLimit - 1000000U;

  // Build the module and scan it to check that all used ids are below the
  // initial limit.
  auto ir_context =
      spvtools::BuildModule(target_env, nullptr, input.data(), input.size());
  if (ir_context == nullptr) {
    // It was not possible to build a valid module; that's OK - skip this input.
    return 0;
  }
  if (ir_context->module()->id_bound() >= kInitialIdLimit) {
    // The input already has a very large id bound. The input is thus abandoned,
    // to avoid the possibility of ending up hitting the id bound limit.
    return 0;
  }

  // Set the optimizer and its validator up with the largest possible id bound
  // limit.
  spvtools::ValidatorOptions validator_options;
  spvtools::OptimizerOptions optimizer_options;
  optimizer_options.set_max_id_bound(kFinalIdLimit);
  validator_options.SetUniversalLimit(spv_validator_limit_max_id_bound,
                                      kFinalIdLimit);
  optimizer_options.set_validator_options(validator_options);
  register_passes(optimizer);
  optimizer.Run(input.data(), input.size(), &input, optimizer_options);

  return 0;
}

}  // namespace fuzzers
}  // namespace spvtools
