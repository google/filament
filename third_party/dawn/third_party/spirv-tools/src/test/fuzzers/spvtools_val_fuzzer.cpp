// Copyright (c) 2018 Google Inc.
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

#include "spirv-tools/libspirv.hpp"
#include "test/fuzzers/random_generator.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < 1) {
    return 0;
  }

  spvtools::fuzzers::RandomGenerator random_gen(data, size);
  spvtools::SpirvTools tools(random_gen.GetTargetEnv());
  tools.SetMessageConsumer([](spv_message_level_t, const char*,
                              const spv_position_t&, const char*) {});

  std::vector<uint32_t> input;
  input.resize(size >> 2);

  size_t count = 0;
  for (size_t i = 0; (i + 3) < size; i += 4) {
    input[count++] = data[i] | (data[i + 1] << 8) | (data[i + 2] << 16) |
                     (data[i + 3]) << 24;
  }

  tools.Validate(input);
  return 0;
}
