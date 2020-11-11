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

#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/fuzz/transformation.h"
#include "source/fuzz/transformation_context.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace fuzz {

class TransformationReplaceIdWithSynonym : public Transformation {
 public:
  explicit TransformationReplaceIdWithSynonym(
      const protobufs::TransformationReplaceIdWithSynonym& message);

  TransformationReplaceIdWithSynonym(
      protobufs::IdUseDescriptor id_use_descriptor, uint32_t synonymous_id);

  // - The fact manager must know that the id identified by
  //   |message_.id_use_descriptor| is synonomous with |message_.synonymous_id|.
  // - Replacing the id in |message_.id_use_descriptor| by
  //   |message_.synonymous_id| must respect SPIR-V's rules about uses being
  //   dominated by their definitions.
  // - The id use must be replaceable in principle. See
  //   fuzzerutil::IdUseCanBeReplaced for details.
  // - |fresh_id_for_temporary| must be 0.
  bool IsApplicable(
      opt::IRContext* ir_context,
      const TransformationContext& transformation_context) const override;

  // Replaces the use identified by |message_.id_use_descriptor| with the
  // synonymous id identified by |message_.synonymous_id|.
  void Apply(opt::IRContext* ir_context,
             TransformationContext* transformation_context) const override;

  std::unordered_set<uint32_t> GetFreshIds() const override;

  protobufs::Transformation ToMessage() const override;

  // Returns true if |type_id_1| and |type_id_2| represent compatible types
  // given the context of the instruction with |opcode| (i.e. we can replace
  // an operand of |opcode| of the first type with an id of the second type
  // and vice-versa).
  static bool TypesAreCompatible(opt::IRContext* ir_context, SpvOp opcode,
                                 uint32_t use_in_operand_index,
                                 uint32_t type_id_1, uint32_t type_id_2);

 private:
  // Returns true if the instruction with opcode |opcode| does not change its
  // behaviour depending on the signedness of the operand at
  // |use_in_operand_index|.
  // Assumes that the operand must be the id of an integer scalar or vector.
  static bool IsAgnosticToSignednessOfOperand(SpvOp opcode,
                                              uint32_t use_in_operand_index);

  protobufs::TransformationReplaceIdWithSynonym message_;
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_TRANSFORMATION_REPLACE_ID_WITH_SYNONYM_H_
