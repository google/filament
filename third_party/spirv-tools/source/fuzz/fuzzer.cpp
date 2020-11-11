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

#include "source/fuzz/fuzzer.h"

#include <cassert>
#include <memory>
#include <numeric>
#include <sstream>

#include "source/fuzz/fact_manager/fact_manager.h"
#include "source/fuzz/fuzzer_context.h"
#include "source/fuzz/fuzzer_pass_add_access_chains.h"
#include "source/fuzz/fuzzer_pass_add_bit_instruction_synonyms.h"
#include "source/fuzz/fuzzer_pass_add_composite_extract.h"
#include "source/fuzz/fuzzer_pass_add_composite_inserts.h"
#include "source/fuzz/fuzzer_pass_add_composite_types.h"
#include "source/fuzz/fuzzer_pass_add_copy_memory.h"
#include "source/fuzz/fuzzer_pass_add_dead_blocks.h"
#include "source/fuzz/fuzzer_pass_add_dead_breaks.h"
#include "source/fuzz/fuzzer_pass_add_dead_continues.h"
#include "source/fuzz/fuzzer_pass_add_equation_instructions.h"
#include "source/fuzz/fuzzer_pass_add_function_calls.h"
#include "source/fuzz/fuzzer_pass_add_global_variables.h"
#include "source/fuzz/fuzzer_pass_add_image_sample_unused_components.h"
#include "source/fuzz/fuzzer_pass_add_loads.h"
#include "source/fuzz/fuzzer_pass_add_local_variables.h"
#include "source/fuzz/fuzzer_pass_add_loop_preheaders.h"
#include "source/fuzz/fuzzer_pass_add_loops_to_create_int_constant_synonyms.h"
#include "source/fuzz/fuzzer_pass_add_no_contraction_decorations.h"
#include "source/fuzz/fuzzer_pass_add_opphi_synonyms.h"
#include "source/fuzz/fuzzer_pass_add_parameters.h"
#include "source/fuzz/fuzzer_pass_add_relaxed_decorations.h"
#include "source/fuzz/fuzzer_pass_add_stores.h"
#include "source/fuzz/fuzzer_pass_add_synonyms.h"
#include "source/fuzz/fuzzer_pass_add_vector_shuffle_instructions.h"
#include "source/fuzz/fuzzer_pass_adjust_branch_weights.h"
#include "source/fuzz/fuzzer_pass_adjust_function_controls.h"
#include "source/fuzz/fuzzer_pass_adjust_loop_controls.h"
#include "source/fuzz/fuzzer_pass_adjust_memory_operands_masks.h"
#include "source/fuzz/fuzzer_pass_adjust_selection_controls.h"
#include "source/fuzz/fuzzer_pass_apply_id_synonyms.h"
#include "source/fuzz/fuzzer_pass_construct_composites.h"
#include "source/fuzz/fuzzer_pass_copy_objects.h"
#include "source/fuzz/fuzzer_pass_donate_modules.h"
#include "source/fuzz/fuzzer_pass_duplicate_regions_with_selections.h"
#include "source/fuzz/fuzzer_pass_expand_vector_reductions.h"
#include "source/fuzz/fuzzer_pass_flatten_conditional_branches.h"
#include "source/fuzz/fuzzer_pass_inline_functions.h"
#include "source/fuzz/fuzzer_pass_interchange_signedness_of_integer_operands.h"
#include "source/fuzz/fuzzer_pass_interchange_zero_like_constants.h"
#include "source/fuzz/fuzzer_pass_invert_comparison_operators.h"
#include "source/fuzz/fuzzer_pass_make_vector_operations_dynamic.h"
#include "source/fuzz/fuzzer_pass_merge_blocks.h"
#include "source/fuzz/fuzzer_pass_merge_function_returns.h"
#include "source/fuzz/fuzzer_pass_mutate_pointers.h"
#include "source/fuzz/fuzzer_pass_obfuscate_constants.h"
#include "source/fuzz/fuzzer_pass_outline_functions.h"
#include "source/fuzz/fuzzer_pass_permute_blocks.h"
#include "source/fuzz/fuzzer_pass_permute_function_parameters.h"
#include "source/fuzz/fuzzer_pass_permute_instructions.h"
#include "source/fuzz/fuzzer_pass_permute_phi_operands.h"
#include "source/fuzz/fuzzer_pass_propagate_instructions_down.h"
#include "source/fuzz/fuzzer_pass_propagate_instructions_up.h"
#include "source/fuzz/fuzzer_pass_push_ids_through_variables.h"
#include "source/fuzz/fuzzer_pass_replace_adds_subs_muls_with_carrying_extended.h"
#include "source/fuzz/fuzzer_pass_replace_branches_from_dead_blocks_with_exits.h"
#include "source/fuzz/fuzzer_pass_replace_copy_memories_with_loads_stores.h"
#include "source/fuzz/fuzzer_pass_replace_copy_objects_with_stores_loads.h"
#include "source/fuzz/fuzzer_pass_replace_irrelevant_ids.h"
#include "source/fuzz/fuzzer_pass_replace_linear_algebra_instructions.h"
#include "source/fuzz/fuzzer_pass_replace_loads_stores_with_copy_memories.h"
#include "source/fuzz/fuzzer_pass_replace_opphi_ids_from_dead_predecessors.h"
#include "source/fuzz/fuzzer_pass_replace_opselects_with_conditional_branches.h"
#include "source/fuzz/fuzzer_pass_replace_parameter_with_global.h"
#include "source/fuzz/fuzzer_pass_replace_params_with_struct.h"
#include "source/fuzz/fuzzer_pass_split_blocks.h"
#include "source/fuzz/fuzzer_pass_swap_commutable_operands.h"
#include "source/fuzz/fuzzer_pass_swap_conditional_branch_operands.h"
#include "source/fuzz/fuzzer_pass_toggle_access_chain_instruction.h"
#include "source/fuzz/fuzzer_pass_wrap_regions_in_selections.h"
#include "source/fuzz/pass_management/repeated_pass_manager.h"
#include "source/fuzz/pass_management/repeated_pass_manager_looped_with_recommendations.h"
#include "source/fuzz/pass_management/repeated_pass_manager_random_with_recommendations.h"
#include "source/fuzz/pass_management/repeated_pass_manager_simple.h"
#include "source/fuzz/pass_management/repeated_pass_recommender_standard.h"
#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/fuzz/transformation_context.h"
#include "source/opt/build_module.h"
#include "source/spirv_fuzzer_options.h"
#include "source/util/make_unique.h"

namespace spvtools {
namespace fuzz {

namespace {
const uint32_t kIdBoundGap = 100;

}  // namespace

Fuzzer::Fuzzer(spv_target_env target_env, MessageConsumer consumer,
               const std::vector<uint32_t>& binary_in,
               const protobufs::FactSequence& initial_facts,
               const std::vector<fuzzerutil::ModuleSupplier>& donor_suppliers,
               std::unique_ptr<RandomGenerator> random_generator,
               bool enable_all_passes,
               RepeatedPassStrategy repeated_pass_strategy,
               bool validate_after_each_fuzzer_pass,
               spv_validator_options validator_options)
    : target_env_(target_env),
      consumer_(std::move(consumer)),
      binary_in_(binary_in),
      initial_facts_(initial_facts),
      donor_suppliers_(donor_suppliers),
      random_generator_(std::move(random_generator)),
      enable_all_passes_(enable_all_passes),
      repeated_pass_strategy_(repeated_pass_strategy),
      validate_after_each_fuzzer_pass_(validate_after_each_fuzzer_pass),
      validator_options_(validator_options),
      num_repeated_passes_applied_(0),
      ir_context_(nullptr),
      fuzzer_context_(nullptr),
      transformation_context_(nullptr),
      transformation_sequence_out_() {}

Fuzzer::~Fuzzer() = default;

template <typename FuzzerPassT, typename... Args>
void Fuzzer::MaybeAddRepeatedPass(uint32_t percentage_chance_of_adding_pass,
                                  RepeatedPassInstances* pass_instances,
                                  Args&&... extra_args) {
  if (enable_all_passes_ ||
      fuzzer_context_->ChoosePercentage(percentage_chance_of_adding_pass)) {
    pass_instances->SetPass(MakeUnique<FuzzerPassT>(
        ir_context_.get(), transformation_context_.get(), fuzzer_context_.get(),
        &transformation_sequence_out_, std::forward<Args>(extra_args)...));
  }
}

template <typename FuzzerPassT, typename... Args>
void Fuzzer::MaybeAddFinalPass(std::vector<std::unique_ptr<FuzzerPass>>* passes,
                               Args&&... extra_args) {
  if (enable_all_passes_ || fuzzer_context_->ChooseEven()) {
    passes->push_back(MakeUnique<FuzzerPassT>(
        ir_context_.get(), transformation_context_.get(), fuzzer_context_.get(),
        &transformation_sequence_out_, std::forward<Args>(extra_args)...));
  }
}

bool Fuzzer::ApplyPassAndCheckValidity(FuzzerPass* pass) const {
  pass->Apply();
  return !validate_after_each_fuzzer_pass_ ||
         fuzzerutil::IsValidAndWellFormed(ir_context_.get(), validator_options_,
                                          consumer_);
}

Fuzzer::FuzzerResult Fuzzer::Run() {
  // Check compatibility between the library version being linked with and the
  // header files being used.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  assert(ir_context_ == nullptr && fuzzer_context_ == nullptr &&
         transformation_context_ == nullptr &&
         transformation_sequence_out_.transformation_size() == 0 &&
         "'Run' must not be invoked more than once.");

  spvtools::SpirvTools tools(target_env_);
  tools.SetMessageConsumer(consumer_);
  if (!tools.IsValid()) {
    consumer_(SPV_MSG_ERROR, nullptr, {},
              "Failed to create SPIRV-Tools interface; stopping.");
    return {Fuzzer::FuzzerResultStatus::kFailedToCreateSpirvToolsInterface,
            std::vector<uint32_t>(), protobufs::TransformationSequence()};
  }

  // Initial binary should be valid.
  if (!tools.Validate(&binary_in_[0], binary_in_.size(), validator_options_)) {
    consumer_(SPV_MSG_ERROR, nullptr, {},
              "Initial binary is invalid; stopping.");
    return {Fuzzer::FuzzerResultStatus::kInitialBinaryInvalid,
            std::vector<uint32_t>(), protobufs::TransformationSequence()};
  }

  // Build the module from the input binary.
  ir_context_ =
      BuildModule(target_env_, consumer_, binary_in_.data(), binary_in_.size());
  assert(ir_context_);

  // The fuzzer will introduce new ids into the module.  The module's id bound
  // gives the smallest id that can be used for this purpose.  We add an offset
  // to this so that there is a sizeable gap between the ids used in the
  // original module and the ids used for fuzzing, as a readability aid.
  //
  // TODO(https://github.com/KhronosGroup/SPIRV-Tools/issues/2541) consider the
  //  case where the maximum id bound is reached.
  auto minimum_fresh_id = ir_context_->module()->id_bound() + kIdBoundGap;
  fuzzer_context_ =
      MakeUnique<FuzzerContext>(random_generator_.get(), minimum_fresh_id);

  transformation_context_ = MakeUnique<TransformationContext>(
      MakeUnique<FactManager>(ir_context_.get()), validator_options_);
  transformation_context_->GetFactManager()->AddInitialFacts(consumer_,
                                                             initial_facts_);

  RepeatedPassInstances pass_instances{};

  // The following passes are likely to be very useful: many other passes
  // introduce synonyms, irrelevant ids and constants that these passes can work
  // with.  We thus enable them with high probability.
  MaybeAddRepeatedPass<FuzzerPassObfuscateConstants>(90, &pass_instances);
  MaybeAddRepeatedPass<FuzzerPassApplyIdSynonyms>(90, &pass_instances);
  MaybeAddRepeatedPass<FuzzerPassReplaceIrrelevantIds>(90, &pass_instances);

  do {
    // Each call to MaybeAddRepeatedPass randomly decides whether the given pass
    // should be enabled, and adds an instance of the pass to |pass_instances|
    // if it is enabled.
    MaybeAddRepeatedPass<FuzzerPassAddAccessChains>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddBitInstructionSynonyms>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddCompositeExtract>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddCompositeInserts>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddCompositeTypes>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddCopyMemory>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddDeadBlocks>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddDeadBreaks>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddDeadContinues>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddEquationInstructions>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddFunctionCalls>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddGlobalVariables>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddImageSampleUnusedComponents>(
        &pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddLoads>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddLocalVariables>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddLoopPreheaders>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddLoopsToCreateIntConstantSynonyms>(
        &pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddOpPhiSynonyms>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddParameters>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddRelaxedDecorations>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddStores>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddSynonyms>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassAddVectorShuffleInstructions>(
        &pass_instances);
    MaybeAddRepeatedPass<FuzzerPassConstructComposites>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassCopyObjects>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassDonateModules>(&pass_instances,
                                                  donor_suppliers_);
    MaybeAddRepeatedPass<FuzzerPassDuplicateRegionsWithSelections>(
        &pass_instances);
    MaybeAddRepeatedPass<FuzzerPassExpandVectorReductions>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassFlattenConditionalBranches>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassInlineFunctions>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassInvertComparisonOperators>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassMakeVectorOperationsDynamic>(
        &pass_instances);
    MaybeAddRepeatedPass<FuzzerPassMergeBlocks>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassMergeFunctionReturns>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassMutatePointers>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassOutlineFunctions>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassPermuteBlocks>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassPermuteFunctionParameters>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassPermuteInstructions>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassPropagateInstructionsDown>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassPropagateInstructionsUp>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassPushIdsThroughVariables>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassReplaceAddsSubsMulsWithCarryingExtended>(
        &pass_instances);
    MaybeAddRepeatedPass<FuzzerPassReplaceBranchesFromDeadBlocksWithExits>(
        &pass_instances);
    MaybeAddRepeatedPass<FuzzerPassReplaceCopyMemoriesWithLoadsStores>(
        &pass_instances);
    MaybeAddRepeatedPass<FuzzerPassReplaceCopyObjectsWithStoresLoads>(
        &pass_instances);
    MaybeAddRepeatedPass<FuzzerPassReplaceLoadsStoresWithCopyMemories>(
        &pass_instances);
    MaybeAddRepeatedPass<FuzzerPassReplaceParameterWithGlobal>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassReplaceLinearAlgebraInstructions>(
        &pass_instances);
    MaybeAddRepeatedPass<FuzzerPassReplaceOpPhiIdsFromDeadPredecessors>(
        &pass_instances);
    MaybeAddRepeatedPass<FuzzerPassReplaceOpSelectsWithConditionalBranches>(
        &pass_instances);
    MaybeAddRepeatedPass<FuzzerPassReplaceParamsWithStruct>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassSplitBlocks>(&pass_instances);
    MaybeAddRepeatedPass<FuzzerPassSwapBranchConditionalOperands>(
        &pass_instances);
    MaybeAddRepeatedPass<FuzzerPassWrapRegionsInSelections>(&pass_instances);
    // There is a theoretical possibility that no pass instances were created
    // until now; loop again if so.
  } while (pass_instances.GetPasses().empty());

  RepeatedPassRecommenderStandard pass_recommender(&pass_instances,
                                                   fuzzer_context_.get());

  std::unique_ptr<RepeatedPassManager> repeated_pass_manager = nullptr;
  switch (repeated_pass_strategy_) {
    case RepeatedPassStrategy::kSimple:
      repeated_pass_manager = MakeUnique<RepeatedPassManagerSimple>(
          fuzzer_context_.get(), &pass_instances);
      break;
    case RepeatedPassStrategy::kLoopedWithRecommendations:
      repeated_pass_manager =
          MakeUnique<RepeatedPassManagerLoopedWithRecommendations>(
              fuzzer_context_.get(), &pass_instances, &pass_recommender);
      break;
    case RepeatedPassStrategy::kRandomWithRecommendations:
      repeated_pass_manager =
          MakeUnique<RepeatedPassManagerRandomWithRecommendations>(
              fuzzer_context_.get(), &pass_instances, &pass_recommender);
      break;
  }

  do {
    if (!ApplyPassAndCheckValidity(
            repeated_pass_manager->ChoosePass(transformation_sequence_out_))) {
      return {Fuzzer::FuzzerResultStatus::kFuzzerPassLedToInvalidModule,
              std::vector<uint32_t>(), protobufs::TransformationSequence()};
    }
  } while (ShouldContinueFuzzing());

  // Now apply some passes that it does not make sense to apply repeatedly,
  // as they do not unlock other passes.
  std::vector<std::unique_ptr<FuzzerPass>> final_passes;
  MaybeAddFinalPass<FuzzerPassAdjustBranchWeights>(&final_passes);
  MaybeAddFinalPass<FuzzerPassAdjustFunctionControls>(&final_passes);
  MaybeAddFinalPass<FuzzerPassAdjustLoopControls>(&final_passes);
  MaybeAddFinalPass<FuzzerPassAdjustMemoryOperandsMasks>(&final_passes);
  MaybeAddFinalPass<FuzzerPassAdjustSelectionControls>(&final_passes);
  MaybeAddFinalPass<FuzzerPassAddNoContractionDecorations>(&final_passes);
  MaybeAddFinalPass<FuzzerPassInterchangeSignednessOfIntegerOperands>(
      &final_passes);
  MaybeAddFinalPass<FuzzerPassInterchangeZeroLikeConstants>(&final_passes);
  MaybeAddFinalPass<FuzzerPassPermutePhiOperands>(&final_passes);
  MaybeAddFinalPass<FuzzerPassSwapCommutableOperands>(&final_passes);
  MaybeAddFinalPass<FuzzerPassToggleAccessChainInstruction>(&final_passes);
  for (auto& pass : final_passes) {
    if (!ApplyPassAndCheckValidity(pass.get())) {
      return {Fuzzer::FuzzerResultStatus::kFuzzerPassLedToInvalidModule,
              std::vector<uint32_t>(), protobufs::TransformationSequence()};
    }
  }
  // Encode the module as a binary.
  std::vector<uint32_t> binary_out;
  ir_context_->module()->ToBinary(&binary_out, false);

  return {Fuzzer::FuzzerResultStatus::kComplete, std::move(binary_out),
          std::move(transformation_sequence_out_)};
}

bool Fuzzer::ShouldContinueFuzzing() {
  // There's a risk that fuzzing could get stuck, if none of the enabled fuzzer
  // passes are able to apply any transformations.  To guard against this we
  // count the number of times some repeated pass has been applied and ensure
  // that fuzzing stops if the number of repeated passes hits the limit on the
  // number of transformations that can be applied.
  assert(
      num_repeated_passes_applied_ <=
          fuzzer_context_->GetTransformationLimit() &&
      "The number of repeated passes applied must not exceed its upper limit.");
  if (ir_context_->module()->id_bound() >= fuzzer_context_->GetIdBoundLimit()) {
    return false;
  }
  if (num_repeated_passes_applied_ ==
      fuzzer_context_->GetTransformationLimit()) {
    // Stop because fuzzing has got stuck.
    return false;
  }
  auto transformations_applied_so_far =
      static_cast<uint32_t>(transformation_sequence_out_.transformation_size());
  if (transformations_applied_so_far >=
      fuzzer_context_->GetTransformationLimit()) {
    // Stop because we have reached the transformation limit.
    return false;
  }
  // If we have applied T transformations so far, and the limit on the number of
  // transformations to apply is L (where T < L), the chance that we will
  // continue fuzzing is:
  //
  //     1 - T/(2*L)
  //
  // That is, the chance of continuing decreases as more transformations are
  // applied.  Using 2*L instead of L increases the number of transformations
  // that are applied on average.
  auto chance_of_continuing = static_cast<uint32_t>(
      100.0 * (1.0 - (static_cast<double>(transformations_applied_so_far) /
                      (2.0 * static_cast<double>(
                                 fuzzer_context_->GetTransformationLimit())))));
  if (!fuzzer_context_->ChoosePercentage(chance_of_continuing)) {
    // We have probabilistically decided to stop.
    return false;
  }
  // Continue fuzzing!
  num_repeated_passes_applied_++;
  return true;
}

}  // namespace fuzz
}  // namespace spvtools
