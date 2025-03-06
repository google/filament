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

#ifndef TEST_FUZZERS_SPVTOOLS_OPT_FUZZER_COMMON_H_
#define TEST_FUZZERS_SPVTOOLS_OPT_FUZZER_COMMON_H_

#include <cinttypes>
#include <cstddef>
#include <functional>

#include "spirv-tools/optimizer.hpp"

namespace spvtools {
namespace fuzzers {

// Helper function capturing the common logic for the various optimizer fuzzers.
int OptFuzzerTestOneInput(
    const uint8_t* data, size_t size,
    const std::function<void(spvtools::Optimizer&)>& register_passes);

}  // namespace fuzzers
}  // namespace spvtools

#endif  // TEST_FUZZERS_SPVTOOLS_OPT_FUZZER_COMMON_H_
