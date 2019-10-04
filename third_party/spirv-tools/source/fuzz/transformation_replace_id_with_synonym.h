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

#ifndef SOURCE_FUZZ_TRANSFORMATION_REPLACE_ID_WITH_SYNONYM_H_
#define SOURCE_FUZZ_TRANSFORMATION_REPLACE_ID_WITH_SYNONYM_H_

#include "source/fuzz/fact_manager.h"
#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/fuzz/transformation.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace fuzz {

class TransformationReplaceIdWithSynonym : public Transformation {
 public:
  explicit TransformationReplaceIdWithSynonym(
      const protobufs::TransformationReplaceIdWithSynonym& message);

  TransformationReplaceIdWithSynonym(
      protobufs::IdUseDescriptor id_use_descriptor,
      protobufs::DataDescriptor data_descriptor,
      uint32_t fresh_id_for_temporary);

  // - The fact manager must know that the id identified by
  //   |message_.id_use_descriptor| is synonomous with
  //   |message_.data_descriptor|.
  // - Replacing the id in |message_.id_use_descriptor| by the synonym in
  //   |message_.data_descriptor| must respect SPIR-V's rules about uses being
  //   dominated by their definitions.
  // - The id must not be an index into an access chain whose base object has
  //   struct type, as such indices must be constants.
  // - The id must not be a pointer argument to a function call (because the
  //   synonym might not be a memory object declaration).
  // - |fresh_id_for_temporary| must be 0.
  // TODO(https://github.com/KhronosGroup/SPIRV-Tools/issues/2855): the
  //  motivation for the temporary is to support the case where an id is
  //  synonymous with an element of a composite.  Until support for that is
  //  implemented, 0 records that no temporary is needed.
  bool IsApplicable(opt::IRContext* context,
                    const FactManager& fact_manager) const override;

  // Replaces the use identified by |message_.id_use_descriptor| with the
  // synonymous id identified by |message_.data_descriptor|.
  // TODO(https://github.com/KhronosGroup/SPIRV-Tools/issues/2855): in due
  //  course it will also be necessary to add an additional instruction to pull
  //  the synonym out of a composite.
  void Apply(opt::IRContext* context, FactManager* fact_manager) const override;

  protobufs::Transformation ToMessage() const override;

  static bool ReplacingUseWithSynonymIsOk(
      opt::IRContext* context, opt::Instruction* use_instruction,
      uint32_t use_in_operand_index, const protobufs::DataDescriptor& synonym);

 private:
  protobufs::TransformationReplaceIdWithSynonym message_;
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_TRANSFORMATION_REPLACE_ID_WITH_SYNONYM_H_
