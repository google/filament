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
#include <sstream>

#include "fuzzer_pass_adjust_memory_operands_masks.h"
#include "source/fuzz/fact_manager.h"
#include "source/fuzz/fuzzer_context.h"
#include "source/fuzz/fuzzer_pass_add_composite_types.h"
#include "source/fuzz/fuzzer_pass_add_dead_blocks.h"
#include "source/fuzz/fuzzer_pass_add_dead_breaks.h"
#include "source/fuzz/fuzzer_pass_add_dead_continues.h"
#include "source/fuzz/fuzzer_pass_add_no_contraction_decorations.h"
#include "source/fuzz/fuzzer_pass_add_useful_constructs.h"
#include "source/fuzz/fuzzer_pass_adjust_function_controls.h"
#include "source/fuzz/fuzzer_pass_adjust_loop_controls.h"
#include "source/fuzz/fuzzer_pass_adjust_selection_controls.h"
#include "source/fuzz/fuzzer_pass_apply_id_synonyms.h"
#include "source/fuzz/fuzzer_pass_construct_composites.h"
#include "source/fuzz/fuzzer_pass_copy_objects.h"
#include "source/fuzz/fuzzer_pass_donate_modules.h"
#include "source/fuzz/fuzzer_pass_merge_blocks.h"
#include "source/fuzz/fuzzer_pass_obfuscate_constants.h"
#include "source/fuzz/fuzzer_pass_outline_functions.h"
#include "source/fuzz/fuzzer_pass_permute_blocks.h"
#include "source/fuzz/fuzzer_pass_split_blocks.h"
#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/fuzz/pseudo_random_generator.h"
#include "source/opt/build_module.h"
#include "source/spirv_fuzzer_options.h"
#include "source/util/make_unique.h"

namespace spvtools {
namespace fuzz {

namespace {
const uint32_t kIdBoundGap = 100;

const uint32_t kTransformationLimit = 500;

const uint32_t kChanceOfApplyingAnotherPass = 85;

// A convenience method to add a fuzzer pass to |passes| with probability 0.5.
// All fuzzer passes take |ir_context|, |fact_manager|, |fuzzer_context| and
// |transformation_sequence_out| as parameters.  Extra arguments can be provided
// via |extra_args|.
template <typename T, typename... Args>
void MaybeAddPass(
    std::vector<std::unique_ptr<FuzzerPass>>* passes,
    opt::IRContext* ir_context, FactManager* fact_manager,
    FuzzerContext* fuzzer_context,
    protobufs::TransformationSequence* transformation_sequence_out,
    Args&&... extra_args) {
  if (fuzzer_context->ChooseEven()) {
    passes->push_back(MakeUnique<T>(ir_context, fact_manager, fuzzer_context,
                                    transformation_sequence_out,
                                    std::forward<Args>(extra_args)...));
  }
}

}  // namespace

struct Fuzzer::Impl {
  explicit Impl(spv_target_env env, uint32_t random_seed,
                bool validate_after_each_pass)
      : target_env(env),
        seed(random_seed),
        validate_after_each_fuzzer_pass(validate_after_each_pass) {}

  bool ApplyPassAndCheckValidity(FuzzerPass* pass,
                                 const opt::IRContext& ir_context,
                                 const spvtools::SpirvTools& tools) const;

  const spv_target_env target_env;       // Target environment.
  const uint32_t seed;                   // Seed for random number generator.
  bool validate_after_each_fuzzer_pass;  // Determines whether the validator
  // should be invoked after every fuzzer pass.
  MessageConsumer consumer;  // Message consumer.
};

Fuzzer::Fuzzer(spv_target_env env, uint32_t seed,
               bool validate_after_each_fuzzer_pass)
    : impl_(MakeUnique<Impl>(env, seed, validate_after_each_fuzzer_pass)) {}

Fuzzer::~Fuzzer() = default;

void Fuzzer::SetMessageConsumer(MessageConsumer c) {
  impl_->consumer = std::move(c);
}

bool Fuzzer::Impl::ApplyPassAndCheckValidity(
    FuzzerPass* pass, const opt::IRContext& ir_context,
    const spvtools::SpirvTools& tools) const {
  pass->Apply();
  if (validate_after_each_fuzzer_pass) {
    std::vector<uint32_t> binary_to_validate;
    ir_context.module()->ToBinary(&binary_to_validate, false);
    if (!tools.Validate(&binary_to_validate[0], binary_to_validate.size())) {
      consumer(SPV_MSG_INFO, nullptr, {},
               "Binary became invalid during fuzzing (set a breakpoint to "
               "inspect); stopping.");
      return false;
    }
  }
  return true;
}

Fuzzer::FuzzerResultStatus Fuzzer::Run(
    const std::vector<uint32_t>& binary_in,
    const protobufs::FactSequence& initial_facts,
    const std::vector<fuzzerutil::ModuleSupplier>& donor_suppliers,
    std::vector<uint32_t>* binary_out,
    protobufs::TransformationSequence* transformation_sequence_out) const {
  // Check compatibility between the library version being linked with and the
  // header files being used.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  spvtools::SpirvTools tools(impl_->target_env);
  tools.SetMessageConsumer(impl_->consumer);
  if (!tools.IsValid()) {
    impl_->consumer(SPV_MSG_ERROR, nullptr, {},
                    "Failed to create SPIRV-Tools interface; stopping.");
    return Fuzzer::FuzzerResultStatus::kFailedToCreateSpirvToolsInterface;
  }

  // Initial binary should be valid.
  if (!tools.Validate(&binary_in[0], binary_in.size())) {
    impl_->consumer(SPV_MSG_ERROR, nullptr, {},
                    "Initial binary is invalid; stopping.");
    return Fuzzer::FuzzerResultStatus::kInitialBinaryInvalid;
  }

  // Build the module from the input binary.
  std::unique_ptr<opt::IRContext> ir_context = BuildModule(
      impl_->target_env, impl_->consumer, binary_in.data(), binary_in.size());
  assert(ir_context);

  // Make a PRNG from the seed passed to the fuzzer on creation.
  PseudoRandomGenerator random_generator(impl_->seed);

  // The fuzzer will introduce new ids into the module.  The module's id bound
  // gives the smallest id that can be used for this purpose.  We add an offset
  // to this so that there is a sizeable gap between the ids used in the
  // original module and the ids used for fuzzing, as a readability aid.
  //
  // TODO(https://github.com/KhronosGroup/SPIRV-Tools/issues/2541) consider the
  //  case where the maximum id bound is reached.
  auto minimum_fresh_id = ir_context->module()->id_bound() + kIdBoundGap;
  FuzzerContext fuzzer_context(&random_generator, minimum_fresh_id);

  FactManager fact_manager;
  fact_manager.AddFacts(impl_->consumer, initial_facts, ir_context.get());

  // Add some essential ingredients to the module if they are not already
  // present, such as boolean constants.
  FuzzerPassAddUsefulConstructs add_useful_constructs(
      ir_context.get(), &fact_manager, &fuzzer_context,
      transformation_sequence_out);
  if (!impl_->ApplyPassAndCheckValidity(&add_useful_constructs, *ir_context,
                                        tools)) {
    return Fuzzer::FuzzerResultStatus::kFuzzerPassLedToInvalidModule;
  }

  // Apply some semantics-preserving passes.
  std::vector<std::unique_ptr<FuzzerPass>> passes;
  while (passes.empty()) {
    MaybeAddPass<FuzzerPassAddCompositeTypes>(&passes, ir_context.get(),
                                              &fact_manager, &fuzzer_context,
                                              transformation_sequence_out);
    MaybeAddPass<FuzzerPassAddDeadBlocks>(&passes, ir_context.get(),
                                          &fact_manager, &fuzzer_context,
                                          transformation_sequence_out);
    MaybeAddPass<FuzzerPassAddDeadBreaks>(&passes, ir_context.get(),
                                          &fact_manager, &fuzzer_context,
                                          transformation_sequence_out);
    MaybeAddPass<FuzzerPassAddDeadContinues>(&passes, ir_context.get(),
                                             &fact_manager, &fuzzer_context,
                                             transformation_sequence_out);
    MaybeAddPass<FuzzerPassApplyIdSynonyms>(&passes, ir_context.get(),
                                            &fact_manager, &fuzzer_context,
                                            transformation_sequence_out);
    MaybeAddPass<FuzzerPassConstructComposites>(&passes, ir_context.get(),
                                                &fact_manager, &fuzzer_context,
                                                transformation_sequence_out);
    MaybeAddPass<FuzzerPassCopyObjects>(&passes, ir_context.get(),
                                        &fact_manager, &fuzzer_context,
                                        transformation_sequence_out);
    MaybeAddPass<FuzzerPassDonateModules>(
        &passes, ir_context.get(), &fact_manager, &fuzzer_context,
        transformation_sequence_out, donor_suppliers);
    MaybeAddPass<FuzzerPassMergeBlocks>(&passes, ir_context.get(),
                                        &fact_manager, &fuzzer_context,
                                        transformation_sequence_out);
    MaybeAddPass<FuzzerPassObfuscateConstants>(&passes, ir_context.get(),
                                               &fact_manager, &fuzzer_context,
                                               transformation_sequence_out);
    MaybeAddPass<FuzzerPassOutlineFunctions>(&passes, ir_context.get(),
                                             &fact_manager, &fuzzer_context,
                                             transformation_sequence_out);
    MaybeAddPass<FuzzerPassPermuteBlocks>(&passes, ir_context.get(),
                                          &fact_manager, &fuzzer_context,
                                          transformation_sequence_out);
    MaybeAddPass<FuzzerPassSplitBlocks>(&passes, ir_context.get(),
                                        &fact_manager, &fuzzer_context,
                                        transformation_sequence_out);
  }

  bool is_first = true;
  while (static_cast<uint32_t>(
             transformation_sequence_out->transformation_size()) <
             kTransformationLimit &&
         (is_first ||
          fuzzer_context.ChoosePercentage(kChanceOfApplyingAnotherPass))) {
    is_first = false;
    if (!impl_->ApplyPassAndCheckValidity(
            passes[fuzzer_context.RandomIndex(passes)].get(), *ir_context,
            tools)) {
      return Fuzzer::FuzzerResultStatus::kFuzzerPassLedToInvalidModule;
    }
  }

  // Now apply some passes that it does not make sense to apply repeatedly,
  // as they do not unlock other passes.
  std::vector<std::unique_ptr<FuzzerPass>> final_passes;
  MaybeAddPass<FuzzerPassAdjustFunctionControls>(
      &final_passes, ir_context.get(), &fact_manager, &fuzzer_context,
      transformation_sequence_out);
  MaybeAddPass<FuzzerPassAdjustLoopControls>(&final_passes, ir_context.get(),
                                             &fact_manager, &fuzzer_context,
                                             transformation_sequence_out);
  MaybeAddPass<FuzzerPassAdjustMemoryOperandsMasks>(
      &final_passes, ir_context.get(), &fact_manager, &fuzzer_context,
      transformation_sequence_out);
  MaybeAddPass<FuzzerPassAdjustSelectionControls>(
      &final_passes, ir_context.get(), &fact_manager, &fuzzer_context,
      transformation_sequence_out);
  MaybeAddPass<FuzzerPassAddNoContractionDecorations>(
      &final_passes, ir_context.get(), &fact_manager, &fuzzer_context,
      transformation_sequence_out);
  for (auto& pass : final_passes) {
    if (!impl_->ApplyPassAndCheckValidity(pass.get(), *ir_context, tools)) {
      return Fuzzer::FuzzerResultStatus::kFuzzerPassLedToInvalidModule;
    }
  }

  // Encode the module as a binary.
  ir_context->module()->ToBinary(binary_out, false);

  return Fuzzer::FuzzerResultStatus::kComplete;
}

}  // namespace fuzz
}  // namespace spvtools
