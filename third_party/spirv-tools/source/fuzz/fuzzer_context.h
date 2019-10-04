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

#ifndef SOURCE_FUZZ_FUZZER_CONTEXT_H_
#define SOURCE_FUZZ_FUZZER_CONTEXT_H_

#include <functional>
#include <utility>

#include "source/fuzz/random_generator.h"
#include "source/opt/function.h"

namespace spvtools {
namespace fuzz {

// Encapsulates all parameters that control the fuzzing process, such as the
// source of randomness and the probabilities with which transformations are
// applied.
class FuzzerContext {
 public:
  // Constructs a fuzzer context with a given random generator and the minimum
  // value that can be used for fresh ids.
  FuzzerContext(RandomGenerator* random_generator, uint32_t min_fresh_id);

  ~FuzzerContext();

  // Returns a random boolean.
  bool ChooseEven();

  // Returns true if and only if a randomly-chosen integer in the range [0, 100]
  // is less than |percentage_chance|.
  bool ChoosePercentage(uint32_t percentage_chance);

  // Returns a random index into |sequence|, which is expected to have a 'size'
  // method, and which must be non-empty.  Typically 'HasSizeMethod' will be an
  // std::vector.
  template <typename HasSizeMethod>
  uint32_t RandomIndex(const HasSizeMethod& sequence) {
    assert(sequence.size() > 0);
    return random_generator_->RandomUint32(
        static_cast<uint32_t>(sequence.size()));
  }

  // Yields an id that is guaranteed not to be used in the module being fuzzed,
  // or to have been issued before.
  uint32_t GetFreshId();

  // Probabilities associated with applying various transformations.
  // Keep them in alphabetical order.
  uint32_t GetChanceOfAddingDeadBreak() { return chance_of_adding_dead_break_; }
  uint32_t GetChanceOfAddingDeadContinue() {
    return chance_of_adding_dead_continue_;
  }
  uint32_t GetChanceOfCopyingObject() { return chance_of_copying_object_; }
  uint32_t GetChanceOfMovingBlockDown() { return chance_of_moving_block_down_; }
  uint32_t GetChanceOfObfuscatingConstant() {
    return chance_of_obfuscating_constant_;
  }
  uint32_t GetChanceOfReplacingIdWithSynonym() {
    return chance_of_replacing_id_with_synonym_;
  }
  uint32_t GetChanceOfSplittingBlock() { return chance_of_splitting_block_; }

  // Functions to control how deeply to recurse.
  // Keep them in alphabetical order.
  bool GoDeeperInConstantObfuscation(uint32_t depth) {
    return go_deeper_in_constant_obfuscation_(depth, random_generator_);
  }

 private:
  // The source of randomness.
  RandomGenerator* random_generator_;
  // The next fresh id to be issued.
  uint32_t next_fresh_id_;

  // Probabilities associated with applying various transformations.
  // Keep them in alphabetical order.
  uint32_t chance_of_adding_dead_break_;
  uint32_t chance_of_adding_dead_continue_;
  uint32_t chance_of_copying_object_;
  uint32_t chance_of_moving_block_down_;
  uint32_t chance_of_obfuscating_constant_;
  uint32_t chance_of_replacing_id_with_synonym_;
  uint32_t chance_of_splitting_block_;

  // Functions to determine with what probability to go deeper when generating
  // or mutating constructs recursively.
  const std::function<bool(uint32_t, RandomGenerator*)>&
      go_deeper_in_constant_obfuscation_;

  // Requires |min_max.first| <= |min_max.second|, and returns a value in the
  // range [ |min_max.first|, |min_max.second| ]
  uint32_t ChooseBetweenMinAndMax(const std::pair<uint32_t, uint32_t>& min_max);
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_FUZZER_CONTEXT_H_
