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

#include "source/fuzz/replayer.h"

#include <utility>

#include "source/fuzz/fact_manager.h"
#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/fuzz/transformation.h"
#include "source/fuzz/transformation_add_constant_boolean.h"
#include "source/fuzz/transformation_add_constant_scalar.h"
#include "source/fuzz/transformation_add_dead_break.h"
#include "source/fuzz/transformation_add_type_boolean.h"
#include "source/fuzz/transformation_add_type_float.h"
#include "source/fuzz/transformation_add_type_int.h"
#include "source/fuzz/transformation_add_type_pointer.h"
#include "source/fuzz/transformation_move_block_down.h"
#include "source/fuzz/transformation_replace_boolean_constant_with_constant_binary.h"
#include "source/fuzz/transformation_replace_constant_with_uniform.h"
#include "source/fuzz/transformation_split_block.h"
#include "source/opt/build_module.h"
#include "source/util/make_unique.h"

namespace spvtools {
namespace fuzz {

struct Replayer::Impl {
  explicit Impl(spv_target_env env, bool validate)
      : target_env(env), validate_during_replay(validate) {}

  const spv_target_env target_env;  // Target environment.
  MessageConsumer consumer;         // Message consumer.

  const bool validate_during_replay;  // Controls whether the validator should
                                      // be run after every replay step.
};

Replayer::Replayer(spv_target_env env, bool validate_during_replay)
    : impl_(MakeUnique<Impl>(env, validate_during_replay)) {}

Replayer::~Replayer() = default;

void Replayer::SetMessageConsumer(MessageConsumer c) {
  impl_->consumer = std::move(c);
}

Replayer::ReplayerResultStatus Replayer::Run(
    const std::vector<uint32_t>& binary_in,
    const protobufs::FactSequence& initial_facts,
    const protobufs::TransformationSequence& transformation_sequence_in,
    std::vector<uint32_t>* binary_out,
    protobufs::TransformationSequence* transformation_sequence_out) const {
  // Check compatibility between the library version being linked with and the
  // header files being used.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  spvtools::SpirvTools tools(impl_->target_env);
  if (!tools.IsValid()) {
    impl_->consumer(SPV_MSG_ERROR, nullptr, {},
                    "Failed to create SPIRV-Tools interface; stopping.");
    return Replayer::ReplayerResultStatus::kFailedToCreateSpirvToolsInterface;
  }

  // Initial binary should be valid.
  if (!tools.Validate(&binary_in[0], binary_in.size())) {
    impl_->consumer(SPV_MSG_INFO, nullptr, {},
                    "Initial binary is invalid; stopping.");
    return Replayer::ReplayerResultStatus::kInitialBinaryInvalid;
  }

  // Build the module from the input binary.
  std::unique_ptr<opt::IRContext> ir_context = BuildModule(
      impl_->target_env, impl_->consumer, binary_in.data(), binary_in.size());
  assert(ir_context);

  // For replay validation, we track the last valid SPIR-V binary that was
  // observed. Initially this is the input binary.
  std::vector<uint32_t> last_valid_binary;
  if (impl_->validate_during_replay) {
    last_valid_binary = binary_in;
  }

  FactManager fact_manager;
  fact_manager.AddFacts(impl_->consumer, initial_facts, ir_context.get());

  // Consider the transformation proto messages in turn.
  for (auto& message : transformation_sequence_in.transformation()) {
    auto transformation = Transformation::FromMessage(message);

    // Check whether the transformation can be applied.
    if (transformation->IsApplicable(ir_context.get(), fact_manager)) {
      // The transformation is applicable, so apply it, and copy it to the
      // sequence of transformations that were applied.
      transformation->Apply(ir_context.get(), &fact_manager);
      *transformation_sequence_out->add_transformation() = message;

      if (impl_->validate_during_replay) {
        std::vector<uint32_t> binary_to_validate;
        ir_context->module()->ToBinary(&binary_to_validate, false);

        // Check whether the latest transformation led to a valid binary.
        if (!tools.Validate(&binary_to_validate[0],
                            binary_to_validate.size())) {
          impl_->consumer(SPV_MSG_INFO, nullptr, {},
                          "Binary became invalid during replay (set a "
                          "breakpoint to inspect); stopping.");
          return Replayer::ReplayerResultStatus::kReplayValidationFailure;
        }

        // The binary was valid, so it becomes the latest valid binary.
        last_valid_binary = std::move(binary_to_validate);
      }
    }
  }

  // Write out the module as a binary.
  ir_context->module()->ToBinary(binary_out, false);
  return Replayer::ReplayerResultStatus::kComplete;
}

}  // namespace fuzz
}  // namespace spvtools
