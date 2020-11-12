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
#include <utility>
#include <vector>

#include "source/fuzz/fuzzer_context.h"
#include "source/fuzz/fuzzer_pass.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/pass_management/repeated_pass_instances.h"
#include "source/fuzz/pass_management/repeated_pass_recommender.h"
#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/fuzz/random_generator.h"
#include "source/opt/ir_context.h"
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
    kFuzzerPassLedToInvalidModule,
    kInitialBinaryInvalid,
  };

  struct FuzzerResult {
    FuzzerResultStatus status;
    std::vector<uint32_t> transformed_binary;
    protobufs::TransformationSequence applied_transformations;
  };

  // Each field of this enum corresponds to an available repeated pass
  // strategy, and is used to decide which kind of RepeatedPassManager object
  // to create.
  enum class RepeatedPassStrategy {
    kSimple,
    kRandomWithRecommendations,
    kLoopedWithRecommendations
  };

  Fuzzer(spv_target_env target_env, MessageConsumer consumer,
         const std::vector<uint32_t>& binary_in,
         const protobufs::FactSequence& initial_facts,
         const std::vector<fuzzerutil::ModuleSupplier>& donor_suppliers,
         std::unique_ptr<RandomGenerator> random_generator,
         bool enable_all_passes, RepeatedPassStrategy repeated_pass_strategy,
         bool validate_after_each_fuzzer_pass,
         spv_validator_options validator_options);

  // Disables copy/move constructor/assignment operations.
  Fuzzer(const Fuzzer&) = delete;
  Fuzzer(Fuzzer&&) = delete;
  Fuzzer& operator=(const Fuzzer&) = delete;
  Fuzzer& operator=(Fuzzer&&) = delete;

  ~Fuzzer();

  // Transforms |binary_in_| by running a number of randomized fuzzer passes.
  // Initial facts about the input binary and the context in which it will
  // execute are provided via |initial_facts_|.  A source of donor modules to be
  // used by transformations is provided via |donor_suppliers_|.  On success,
  // returns a successful result status together with the transformed binary and
  // the sequence of transformations that were applied.  Otherwise, returns an
  // appropriate result status together with an empty binary and empty
  // transformation sequence.
  FuzzerResult Run();

 private:
  // A convenience method to add a repeated fuzzer pass to |pass_instances| with
  // probability |percentage_chance_of_adding_pass|%, or with probability 100%
  // if |enable_all_passes_| is true.
  //
  // All fuzzer passes take members |ir_context_|, |transformation_context_|,
  // |fuzzer_context_| and |transformation_sequence_out_| as parameters.  Extra
  // arguments can be provided via |extra_args|.
  template <typename FuzzerPassT, typename... Args>
  void MaybeAddRepeatedPass(uint32_t percentage_chance_of_adding_pass,
                            RepeatedPassInstances* pass_instances,
                            Args&&... extra_args);

  // The same as the above, with |percentage_chance_of_adding_pass| == 50%.
  template <typename FuzzerPassT, typename... Args>
  void MaybeAddRepeatedPass(RepeatedPassInstances* pass_instances,
                            Args&&... extra_args) {
    MaybeAddRepeatedPass<FuzzerPassT>(50, pass_instances,
                                      std::forward<Args>(extra_args)...);
  }

  // A convenience method to add a final fuzzer pass to |passes| with
  // probability 50%, or with probability 100% if |enable_all_passes_| is true.
  //
  // All fuzzer passes take members |ir_context_|, |transformation_context_|,
  // |fuzzer_context_| and |transformation_sequence_out_| as parameters.  Extra
  // arguments can be provided via |extra_args|.
  template <typename FuzzerPassT, typename... Args>
  void MaybeAddFinalPass(std::vector<std::unique_ptr<FuzzerPass>>* passes,
                         Args&&... extra_args);

  // Decides whether to apply more repeated passes. The probability decreases as
  // the number of transformations that have been applied increases.
  bool ShouldContinueFuzzing();

  // Applies |pass|, which must be a pass constructed with |ir_context|.
  // If |validate_after_each_fuzzer_pass_| is not set, true is always returned.
  // Otherwise, true is returned if and only if |ir_context| passes validation,
  // every block has its enclosing function as its parent, and every
  // instruction has a distinct unique id.
  bool ApplyPassAndCheckValidity(FuzzerPass* pass) const;

  // Target environment.
  const spv_target_env target_env_;

  // Message consumer that will be invoked once for each message communicated
  // from the library.
  MessageConsumer consumer_;

  // The initial binary to which fuzzing should be applied.
  const std::vector<uint32_t>& binary_in_;

  // Initial facts known to hold in advance of applying any transformations.
  const protobufs::FactSequence& initial_facts_;

  // A source of modules whose contents can be donated into the module being
  // fuzzed.
  const std::vector<fuzzerutil::ModuleSupplier>& donor_suppliers_;

  // Random number generator to control decision making during fuzzing.
  std::unique_ptr<RandomGenerator> random_generator_;

  // Determines whether all passes should be enabled, vs. having passes be
  // probabilistically enabled.
  bool enable_all_passes_;

  // Controls which type of RepeatedPassManager object to create.
  RepeatedPassStrategy repeated_pass_strategy_;

  // Determines whether the validator should be invoked after every fuzzer pass.
  bool validate_after_each_fuzzer_pass_;

  // Options to control validation.
  spv_validator_options validator_options_;

  // The number of repeated fuzzer passes that have been applied is kept track
  // of, in order to enforce a hard limit on the number of times such passes
  // can be applied.
  uint32_t num_repeated_passes_applied_;

  // Intermediate representation for the module being fuzzed, which gets
  // mutated as fuzzing proceeds.
  std::unique_ptr<opt::IRContext> ir_context_;

  // Provides probabilities that control the fuzzing process.
  std::unique_ptr<FuzzerContext> fuzzer_context_;

  // Contextual information that is required in order to apply transformations.
  std::unique_ptr<TransformationContext> transformation_context_;

  // The sequence of transformations that have been applied during fuzzing.  It
  // is initially empty and grows as fuzzer passes are applied.
  protobufs::TransformationSequence transformation_sequence_out_;
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_FUZZER_H_
