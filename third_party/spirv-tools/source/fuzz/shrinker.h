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

#ifndef SOURCE_FUZZ_SHRINKER_H_
#define SOURCE_FUZZ_SHRINKER_H_

#include <memory>
#include <vector>

#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "spirv-tools/libspirv.hpp"

namespace spvtools {
namespace fuzz {

// Shrinks a sequence of transformations that lead to an interesting SPIR-V
// binary to yield a smaller sequence of transformations that still produce an
// interesting binary.
class Shrinker {
 public:
  // Possible statuses that can result from running the shrinker.
  enum ShrinkerResultStatus {
    kComplete,
    kFailedToCreateSpirvToolsInterface,
    kInitialBinaryInvalid,
    kInitialBinaryNotInteresting,
    kReplayFailed,
    kStepLimitReached,
  };

  // The type for a function that will take a binary, |binary|, and return true
  // if and only if the binary is deemed interesting. (The function also takes
  // an integer argument, |counter|, that will be incremented each time the
  // function is called; this is for debugging purposes).
  //
  // The notion of "interesting" depends on what properties of the binary or
  // tools that process the binary we are trying to maintain during shrinking.
  using InterestingnessFunction = std::function<bool(
      const std::vector<uint32_t>& binary, uint32_t counter)>;

  // Constructs a shrinker from the given target environment.
  Shrinker(spv_target_env env, uint32_t step_limit,
           bool validate_during_replay);

  // Disables copy/move constructor/assignment operations.
  Shrinker(const Shrinker&) = delete;
  Shrinker(Shrinker&&) = delete;
  Shrinker& operator=(const Shrinker&) = delete;
  Shrinker& operator=(Shrinker&&) = delete;

  ~Shrinker();

  // Sets the message consumer to the given |consumer|. The |consumer| will be
  // invoked once for each message communicated from the library.
  void SetMessageConsumer(MessageConsumer consumer);

  // Requires that when |transformation_sequence_in| is applied to |binary_in|
  // with initial facts |initial_facts|, the resulting binary is interesting
  // according to |interestingness_function|.
  //
  // Produces, via |transformation_sequence_out|, a subsequence of
  // |transformation_sequence_in| that, when applied with initial facts
  // |initial_facts|, produces a binary (captured via |binary_out|) that is
  // also interesting according to |interestingness_function|.
  ShrinkerResultStatus Run(
      const std::vector<uint32_t>& binary_in,
      const protobufs::FactSequence& initial_facts,
      const protobufs::TransformationSequence& transformation_sequence_in,
      const InterestingnessFunction& interestingness_function,
      std::vector<uint32_t>* binary_out,
      protobufs::TransformationSequence* transformation_sequence_out) const;

 private:
  struct Impl;                  // Opaque struct for holding internal data.
  std::unique_ptr<Impl> impl_;  // Unique pointer to internal data.
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_SHRINKER_H_
