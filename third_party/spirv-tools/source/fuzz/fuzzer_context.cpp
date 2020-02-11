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

#include "source/fuzz/fuzzer_context.h"

#include <cmath>

namespace spvtools {
namespace fuzz {

namespace {
// Default <minimum, maximum> pairs of probabilities for applying various
// transformations. All values are percentages. Keep them in alphabetical order.

const std::pair<uint32_t, uint32_t> kChanceOfAddingAnotherStructField = {20,
                                                                         90};
const std::pair<uint32_t, uint32_t> kChanceOfAddingArrayOrStructType = {20, 90};
const std::pair<uint32_t, uint32_t> kChanceOfAddingDeadBlock = {20, 90};
const std::pair<uint32_t, uint32_t> kChanceOfAddingDeadBreak = {5, 80};
const std::pair<uint32_t, uint32_t> kChanceOfAddingDeadContinue = {5, 80};
const std::pair<uint32_t, uint32_t> kChanceOfAddingMatrixType = {20, 70};
const std::pair<uint32_t, uint32_t> kChanceOfAddingNoContractionDecoration = {
    5, 70};
const std::pair<uint32_t, uint32_t> kChanceOfAddingVectorType = {20, 70};
const std::pair<uint32_t, uint32_t> kChanceOfAdjustingFunctionControl = {20,
                                                                         70};
const std::pair<uint32_t, uint32_t> kChanceOfAdjustingLoopControl = {20, 90};
const std::pair<uint32_t, uint32_t> kChanceOfAdjustingMemoryOperandsMask = {20,
                                                                            90};
const std::pair<uint32_t, uint32_t> kChanceOfAdjustingSelectionControl = {20,
                                                                          90};
const std::pair<uint32_t, uint32_t> kChanceOfChoosingStructTypeVsArrayType = {
    20, 80};
const std::pair<uint32_t, uint32_t> kChanceOfConstructingComposite = {20, 50};
const std::pair<uint32_t, uint32_t> kChanceOfCopyingObject = {20, 50};
const std::pair<uint32_t, uint32_t> kChanceOfDonatingAdditionalModule = {5, 50};
const std::pair<uint32_t, uint32_t> kChanceOfMakingDonorLivesafe = {40, 60};
const std::pair<uint32_t, uint32_t> kChanceOfMergingBlocks = {20, 95};
const std::pair<uint32_t, uint32_t> kChanceOfMovingBlockDown = {20, 50};
const std::pair<uint32_t, uint32_t> kChanceOfObfuscatingConstant = {10, 90};
const std::pair<uint32_t, uint32_t> kChanceOfOutliningFunction = {10, 90};
const std::pair<uint32_t, uint32_t> kChanceOfReplacingIdWithSynonym = {10, 90};
const std::pair<uint32_t, uint32_t> kChanceOfSplittingBlock = {40, 95};

// Default limits for various quantities that are chosen during fuzzing.
// Keep them in alphabetical order.
const uint32_t kDefaultMaxLoopControlPartialCount = 100;
const uint32_t kDefaultMaxLoopControlPeelCount = 100;
const uint32_t kDefaultMaxLoopLimit = 20;
const uint32_t kDefaultMaxNewArraySizeLimit = 100;

// Default functions for controlling how deep to go during recursive
// generation/transformation. Keep them in alphabetical order.

const std::function<bool(uint32_t, RandomGenerator*)>
    kDefaultGoDeeperInConstantObfuscation =
        [](uint32_t current_depth, RandomGenerator* random_generator) -> bool {
  double chance = 1.0 / std::pow(3.0, static_cast<float>(current_depth + 1));
  return random_generator->RandomDouble() < chance;
};

}  // namespace

FuzzerContext::FuzzerContext(RandomGenerator* random_generator,
                             uint32_t min_fresh_id)
    : random_generator_(random_generator),
      next_fresh_id_(min_fresh_id),
      go_deeper_in_constant_obfuscation_(
          kDefaultGoDeeperInConstantObfuscation) {
  chance_of_adding_another_struct_field_ =
      ChooseBetweenMinAndMax(kChanceOfAddingAnotherStructField);
  chance_of_adding_array_or_struct_type_ =
      ChooseBetweenMinAndMax(kChanceOfAddingArrayOrStructType);
  chance_of_adding_dead_block_ =
      ChooseBetweenMinAndMax(kChanceOfAddingDeadBlock);
  chance_of_adding_dead_break_ =
      ChooseBetweenMinAndMax(kChanceOfAddingDeadBreak);
  chance_of_adding_dead_continue_ =
      ChooseBetweenMinAndMax(kChanceOfAddingDeadContinue);
  chance_of_adding_matrix_type_ =
      ChooseBetweenMinAndMax(kChanceOfAddingMatrixType);
  chance_of_adding_no_contraction_decoration_ =
      ChooseBetweenMinAndMax(kChanceOfAddingNoContractionDecoration);
  chance_of_adding_vector_type_ =
      ChooseBetweenMinAndMax(kChanceOfAddingVectorType);
  chance_of_adjusting_function_control_ =
      ChooseBetweenMinAndMax(kChanceOfAdjustingFunctionControl);
  chance_of_adjusting_loop_control_ =
      ChooseBetweenMinAndMax(kChanceOfAdjustingLoopControl);
  chance_of_adjusting_memory_operands_mask_ =
      ChooseBetweenMinAndMax(kChanceOfAdjustingMemoryOperandsMask);
  chance_of_adjusting_selection_control_ =
      ChooseBetweenMinAndMax(kChanceOfAdjustingSelectionControl);
  chance_of_choosing_struct_type_vs_array_type_ =
      ChooseBetweenMinAndMax(kChanceOfChoosingStructTypeVsArrayType);
  chance_of_constructing_composite_ =
      ChooseBetweenMinAndMax(kChanceOfConstructingComposite);
  chance_of_copying_object_ = ChooseBetweenMinAndMax(kChanceOfCopyingObject);
  chance_of_donating_additional_module_ =
      ChooseBetweenMinAndMax(kChanceOfDonatingAdditionalModule);
  chance_of_making_donor_livesafe_ =
      ChooseBetweenMinAndMax(kChanceOfMakingDonorLivesafe);
  chance_of_merging_blocks_ = ChooseBetweenMinAndMax(kChanceOfMergingBlocks);
  chance_of_moving_block_down_ =
      ChooseBetweenMinAndMax(kChanceOfMovingBlockDown);
  chance_of_obfuscating_constant_ =
      ChooseBetweenMinAndMax(kChanceOfObfuscatingConstant);
  chance_of_outlining_function_ =
      ChooseBetweenMinAndMax(kChanceOfOutliningFunction);
  chance_of_replacing_id_with_synonym_ =
      ChooseBetweenMinAndMax(kChanceOfReplacingIdWithSynonym);
  chance_of_splitting_block_ = ChooseBetweenMinAndMax(kChanceOfSplittingBlock);
  max_loop_control_partial_count_ = kDefaultMaxLoopControlPartialCount;
  max_loop_control_peel_count_ = kDefaultMaxLoopControlPeelCount;
  max_loop_limit_ = kDefaultMaxLoopLimit;
  max_new_array_size_limit_ = kDefaultMaxNewArraySizeLimit;
}

FuzzerContext::~FuzzerContext() = default;

uint32_t FuzzerContext::GetFreshId() { return next_fresh_id_++; }

bool FuzzerContext::ChooseEven() { return random_generator_->RandomBool(); }

bool FuzzerContext::ChoosePercentage(uint32_t percentage_chance) {
  assert(percentage_chance <= 100);
  return random_generator_->RandomPercentage() < percentage_chance;
}

uint32_t FuzzerContext::ChooseBetweenMinAndMax(
    const std::pair<uint32_t, uint32_t>& min_max) {
  assert(min_max.first <= min_max.second);
  return min_max.first +
         random_generator_->RandomUint32(min_max.second - min_max.first + 1);
}

}  // namespace fuzz
}  // namespace spvtools
