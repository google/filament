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

#include "source/fuzz/shrinker.h"

#include <sstream>

#include "source/fuzz/pseudo_random_generator.h"
#include "source/fuzz/replayer.h"
#include "source/spirv_fuzzer_options.h"
#include "source/util/make_unique.h"

namespace spvtools {
namespace fuzz {

namespace {

// A helper to get the size of a protobuf transformation sequence in a less
// verbose manner.
uint32_t NumRemainingTransformations(
    const protobufs::TransformationSequence& transformation_sequence) {
  return static_cast<uint32_t>(transformation_sequence.transformation_size());
}

// A helper to return a transformation sequence identical to |transformations|,
// except that a chunk of size |chunk_size| starting from |chunk_index| x
// |chunk_size| is removed (or as many transformations as available if the whole
// chunk is not).
protobufs::TransformationSequence RemoveChunk(
    const protobufs::TransformationSequence& transformations,
    uint32_t chunk_index, uint32_t chunk_size) {
  uint32_t lower = chunk_index * chunk_size;
  uint32_t upper = std::min((chunk_index + 1) * chunk_size,
                            NumRemainingTransformations(transformations));
  assert(lower < upper);
  assert(upper <= NumRemainingTransformations(transformations));
  protobufs::TransformationSequence result;
  for (uint32_t j = 0; j < NumRemainingTransformations(transformations); j++) {
    if (j >= lower && j < upper) {
      continue;
    }
    protobufs::Transformation transformation =
        transformations.transformation()[j];
    *result.mutable_transformation()->Add() = transformation;
  }
  return result;
}

}  // namespace

struct Shrinker::Impl {
  explicit Impl(spv_target_env env, uint32_t limit, bool validate)
      : target_env(env), step_limit(limit), validate_during_replay(validate) {}

  const spv_target_env target_env;    // Target environment.
  MessageConsumer consumer;           // Message consumer.
  const uint32_t step_limit;          // Step limit for reductions.
  const bool validate_during_replay;  // Determines whether to check for
                                      // validity during the replaying of
                                      // transformations.
};

Shrinker::Shrinker(spv_target_env env, uint32_t step_limit,
                   bool validate_during_replay)
    : impl_(MakeUnique<Impl>(env, step_limit, validate_during_replay)) {}

Shrinker::~Shrinker() = default;

void Shrinker::SetMessageConsumer(MessageConsumer c) {
  impl_->consumer = std::move(c);
}

Shrinker::ShrinkerResultStatus Shrinker::Run(
    const std::vector<uint32_t>& binary_in,
    const protobufs::FactSequence& initial_facts,
    const protobufs::TransformationSequence& transformation_sequence_in,
    const Shrinker::InterestingnessFunction& interestingness_function,
    std::vector<uint32_t>* binary_out,
    protobufs::TransformationSequence* transformation_sequence_out) const {
  // Check compatibility between the library version being linked with and the
  // header files being used.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  spvtools::SpirvTools tools(impl_->target_env);
  if (!tools.IsValid()) {
    impl_->consumer(SPV_MSG_ERROR, nullptr, {},
                    "Failed to create SPIRV-Tools interface; stopping.");
    return Shrinker::ShrinkerResultStatus::kFailedToCreateSpirvToolsInterface;
  }

  // Initial binary should be valid.
  if (!tools.Validate(&binary_in[0], binary_in.size())) {
    impl_->consumer(SPV_MSG_INFO, nullptr, {},
                    "Initial binary is invalid; stopping.");
    return Shrinker::ShrinkerResultStatus::kInitialBinaryInvalid;
  }

  std::vector<uint32_t> current_best_binary;
  protobufs::TransformationSequence current_best_transformations;

  // Run a replay of the initial transformation sequence to (a) check that it
  // succeeds, (b) get the binary that results from running these
  // transformations, and (c) get the subsequence of the initial transformations
  // that actually apply (in principle this could be a strict subsequence).
  if (Replayer(impl_->target_env, impl_->validate_during_replay)
          .Run(binary_in, initial_facts, transformation_sequence_in,
               &current_best_binary, &current_best_transformations) !=
      Replayer::ReplayerResultStatus::kComplete) {
    return ShrinkerResultStatus::kReplayFailed;
  }

  // Check that the binary produced by applying the initial transformations is
  // indeed interesting.
  if (!interestingness_function(current_best_binary, 0)) {
    impl_->consumer(SPV_MSG_INFO, nullptr, {},
                    "Initial binary is not interesting; stopping.");
    return ShrinkerResultStatus::kInitialBinaryNotInteresting;
  }

  uint32_t attempt = 0;  // Keeps track of the number of shrink attempts that
                         // have been tried, whether successful or not.

  uint32_t chunk_size =
      std::max(1u, NumRemainingTransformations(current_best_transformations) /
                       2);  // The number of contiguous transformations that the
                            // shrinker will try to remove in one go; starts
                            // high and decreases during the shrinking process.

  // Keep shrinking until we:
  // - reach the step limit,
  // - run out of transformations to remove, or
  // - cannot make the chunk size any smaller.
  while (attempt < impl_->step_limit &&
         !current_best_transformations.transformation().empty() &&
         chunk_size > 0) {
    bool progress_this_round =
        false;  // Used to decide whether to make the chunk size with which we
                // remove transformations smaller.  If we managed to remove at
                // least one chunk of transformations at a particular chunk
                // size, we set this flag so that we do not yet decrease the
                // chunk size.

    assert(chunk_size <=
               NumRemainingTransformations(current_best_transformations) &&
           "Chunk size should never exceed the number of transformations that "
           "remain.");

    // The number of chunks is the ceiling of (#remaining_transformations /
    // chunk_size).
    const uint32_t num_chunks =
        (NumRemainingTransformations(current_best_transformations) +
         chunk_size - 1) /
        chunk_size;
    assert(num_chunks >= 1 && "There should be at least one chunk.");
    assert(num_chunks * chunk_size >=
               NumRemainingTransformations(current_best_transformations) &&
           "All transformations should be in some chunk.");

    // We go through the transformations in reverse, in chunks of size
    // |chunk_size|, using |chunk_index| to track which chunk to try removing
    // next.  The loop exits early if we reach the shrinking step limit.
    for (int chunk_index = num_chunks - 1;
         attempt < impl_->step_limit && chunk_index >= 0; chunk_index--) {
      // Remove a chunk of transformations according to the current index and
      // chunk size.
      auto transformations_with_chunk_removed =
          RemoveChunk(current_best_transformations, chunk_index, chunk_size);

      // Replay the smaller sequence of transformations to get a next binary and
      // transformation sequence. Note that the transformations arising from
      // replay might be even smaller than the transformations with the chunk
      // removed, because removing those transformations might make further
      // transformations inapplicable.
      std::vector<uint32_t> next_binary;
      protobufs::TransformationSequence next_transformation_sequence;
      if (Replayer(impl_->target_env, false)
              .Run(binary_in, initial_facts, transformations_with_chunk_removed,
                   &next_binary, &next_transformation_sequence) !=
          Replayer::ReplayerResultStatus::kComplete) {
        // Replay should not fail; if it does, we need to abort shrinking.
        return ShrinkerResultStatus::kReplayFailed;
      }

      assert(NumRemainingTransformations(next_transformation_sequence) >=
                 chunk_index * chunk_size &&
             "Removing this chunk of transformations should not have an effect "
             "on earlier chunks.");

      if (interestingness_function(next_binary, attempt)) {
        // If the binary arising from the smaller transformation sequence is
        // interesting, this becomes our current best binary and transformation
        // sequence.
        current_best_binary = next_binary;
        current_best_transformations = next_transformation_sequence;
        progress_this_round = true;
      }
      // Either way, this was a shrink attempt, so increment our count of shrink
      // attempts.
      attempt++;
    }
    if (!progress_this_round) {
      // If we didn't manage to remove any chunks at this chunk size, try a
      // smaller chunk size.
      chunk_size /= 2;
    }
    // Decrease the chunk size until it becomes no larger than the number of
    // remaining transformations.
    while (chunk_size >
           NumRemainingTransformations(current_best_transformations)) {
      chunk_size /= 2;
    }
  }

  // The output from the shrinker is the best binary we saw, and the
  // transformations that led to it.
  *binary_out = current_best_binary;
  *transformation_sequence_out = current_best_transformations;

  // Indicate whether shrinking completed or was truncated due to reaching the
  // step limit.
  assert(attempt <= impl_->step_limit);
  if (attempt == impl_->step_limit) {
    std::stringstream strstream;
    strstream << "Shrinking did not complete; step limit " << impl_->step_limit
              << " was reached.";
    impl_->consumer(SPV_MSG_WARNING, nullptr, {}, strstream.str().c_str());
    return Shrinker::ShrinkerResultStatus::kStepLimitReached;
  }
  return Shrinker::ShrinkerResultStatus::kComplete;
}

}  // namespace fuzz
}  // namespace spvtools
