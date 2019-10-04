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

#ifndef SOURCE_FUZZ_REPLAYER_H_
#define SOURCE_FUZZ_REPLAYER_H_

#include <memory>
#include <vector>

#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "spirv-tools/libspirv.hpp"

namespace spvtools {
namespace fuzz {

// Transforms a SPIR-V module into a semantically equivalent SPIR-V module by
// applying a series of pre-defined transformations.
class Replayer {
 public:
  // Possible statuses that can result from running the replayer.
  enum ReplayerResultStatus {
    kComplete,
    kFailedToCreateSpirvToolsInterface,
    kInitialBinaryInvalid,
    kReplayValidationFailure,
  };

  // Constructs a replayer from the given target environment.
  explicit Replayer(spv_target_env env, bool validate_during_replay);

  // Disables copy/move constructor/assignment operations.
  Replayer(const Replayer&) = delete;
  Replayer(Replayer&&) = delete;
  Replayer& operator=(const Replayer&) = delete;
  Replayer& operator=(Replayer&&) = delete;

  ~Replayer();

  // Sets the message consumer to the given |consumer|. The |consumer| will be
  // invoked once for each message communicated from the library.
  void SetMessageConsumer(MessageConsumer consumer);

  // Transforms |binary_in| to |binary_out| by attempting to apply the
  // transformations from |transformation_sequence_in|.  Initial facts about the
  // input binary and the context in which it will execute are provided via
  // |initial_facts|.  The transformations that were successfully applied are
  // returned via |transformation_sequence_out|.
  ReplayerResultStatus Run(
      const std::vector<uint32_t>& binary_in,
      const protobufs::FactSequence& initial_facts,
      const protobufs::TransformationSequence& transformation_sequence_in,
      std::vector<uint32_t>* binary_out,
      protobufs::TransformationSequence* transformation_sequence_out) const;

 private:
  struct Impl;                  // Opaque struct for holding internal data.
  std::unique_ptr<Impl> impl_;  // Unique pointer to internal data.
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_REPLAYER_H_
