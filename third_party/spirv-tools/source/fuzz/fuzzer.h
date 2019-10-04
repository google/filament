// Copyright (c) 2019 Google LLC
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

#ifndef SOURCE_FUZZ_FUZZER_H_
#define SOURCE_FUZZ_FUZZER_H_

#include <memory>
#include <vector>

#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "spirv-tools/libspirv.hpp"

namespace spvtools {
namespace fuzz {

// Transforms a SPIR-V module into a semantically equivalent SPIR-V module by
// running a number of randomized fuzzer passes.
class Fuzzer {
 public:
  // Possible statuses that can result from running the fuzzer.
  enum class FuzzerResultStatus {
    kComplete,
    kFailedToCreateSpirvToolsInterface,
    kInitialBinaryInvalid,
  };

  // Constructs a fuzzer from the given target environment.
  explicit Fuzzer(spv_target_env env);

  // Disables copy/move constructor/assignment operations.
  Fuzzer(const Fuzzer&) = delete;
  Fuzzer(Fuzzer&&) = delete;
  Fuzzer& operator=(const Fuzzer&) = delete;
  Fuzzer& operator=(Fuzzer&&) = delete;

  ~Fuzzer();

  // Sets the message consumer to the given |consumer|. The |consumer| will be
  // invoked once for each message communicated from the library.
  void SetMessageConsumer(MessageConsumer consumer);

  // Transforms |binary_in| to |binary_out| by running a number of randomized
  // fuzzer passes, controlled via |options|.  Initial facts about the input
  // binary and the context in which it will execute are provided via
  // |initial_facts|.  The transformation sequence that was applied is returned
  // via |transformation_sequence_out|.
  FuzzerResultStatus Run(
      const std::vector<uint32_t>& binary_in,
      const protobufs::FactSequence& initial_facts,
      spv_const_fuzzer_options options, std::vector<uint32_t>* binary_out,
      protobufs::TransformationSequence* transformation_sequence_out) const;

 private:
  struct Impl;                  // Opaque struct for holding internal data.
  std::unique_ptr<Impl> impl_;  // Unique pointer to internal data.
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_FUZZER_H_
