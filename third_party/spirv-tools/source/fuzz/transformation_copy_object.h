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

#ifndef SOURCE_FUZZ_TRANSFORMATION_COPY_OBJECT_H_
#define SOURCE_FUZZ_TRANSFORMATION_COPY_OBJECT_H_

#include "source/fuzz/fact_manager.h"
#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/fuzz/transformation.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace fuzz {

class TransformationCopyObject : public Transformation {
 public:
  explicit TransformationCopyObject(
      const protobufs::TransformationCopyObject& message);

  TransformationCopyObject(uint32_t object, uint32_t base_instruction_id,
                           uint32_t offset, uint32_t fresh_id);

  // - |message_.fresh_id| must not be used by the module.
  // - |message_.object| must be a result id that is a legitimate operand for
  //   OpCopyObject.  In particular, it must be the id of an instruction that
  //   has a result type
  // - |message_.object| must not be the target of any decoration.
  //   TODO(afd): consider copying decorations along with objects.
  // - |message_.insert_after_id| must be the result id of an instruction
  //   'base' in some block 'blk'.
  // - 'blk' must contain an instruction 'inst' located |message_.offset|
  //   instructions after 'base' (if |message_.offset| = 0 then 'inst' =
  //   'base').
  // - It must be legal to insert an OpCopyObject instruction directly
  //   before 'inst'.
  // - |message_object| must be available directly before 'inst'.
  bool IsApplicable(opt::IRContext* context,
                    const FactManager& fact_manager) const override;

  // - A new instruction,
  //     %|message_.fresh_id| = OpCopyObject %ty %|message_.object|
  //   is added directly before the instruction at |message_.insert_after_id| +
  //   |message_|.offset, where %ty is the type of |message_.object|.
  // - The fact that |message_.fresh_id| and |message_.object| are synonyms
  //   is added to the fact manager.
  void Apply(opt::IRContext* context, FactManager* fact_manager) const override;

  protobufs::Transformation ToMessage() const override;

  // Determines whether it is OK to make a copy of |inst|.
  static bool IsCopyable(opt::IRContext* ir_context, opt::Instruction* inst);

  // Determines whether it is OK to insert a copy instruction before the given
  // instruction.
  static bool CanInsertCopyBefore(
      const opt::BasicBlock::iterator& instruction_in_block);

 private:
  protobufs::TransformationCopyObject message_;
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_TRANSFORMATION_COPY_OBJECT_H_
