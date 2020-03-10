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

#include "source/fuzz/transformation.h"

#include <cassert>

#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/transformation_add_constant_boolean.h"
#include "source/fuzz/transformation_add_constant_composite.h"
#include "source/fuzz/transformation_add_constant_scalar.h"
#include "source/fuzz/transformation_add_dead_block.h"
#include "source/fuzz/transformation_add_dead_break.h"
#include "source/fuzz/transformation_add_dead_continue.h"
#include "source/fuzz/transformation_add_function.h"
#include "source/fuzz/transformation_add_global_undef.h"
#include "source/fuzz/transformation_add_global_variable.h"
#include "source/fuzz/transformation_add_no_contraction_decoration.h"
#include "source/fuzz/transformation_add_type_array.h"
#include "source/fuzz/transformation_add_type_boolean.h"
#include "source/fuzz/transformation_add_type_float.h"
#include "source/fuzz/transformation_add_type_function.h"
#include "source/fuzz/transformation_add_type_int.h"
#include "source/fuzz/transformation_add_type_matrix.h"
#include "source/fuzz/transformation_add_type_pointer.h"
#include "source/fuzz/transformation_add_type_struct.h"
#include "source/fuzz/transformation_add_type_vector.h"
#include "source/fuzz/transformation_composite_construct.h"
#include "source/fuzz/transformation_composite_extract.h"
#include "source/fuzz/transformation_copy_object.h"
#include "source/fuzz/transformation_merge_blocks.h"
#include "source/fuzz/transformation_move_block_down.h"
#include "source/fuzz/transformation_outline_function.h"
#include "source/fuzz/transformation_replace_boolean_constant_with_constant_binary.h"
#include "source/fuzz/transformation_replace_constant_with_uniform.h"
#include "source/fuzz/transformation_replace_id_with_synonym.h"
#include "source/fuzz/transformation_set_function_control.h"
#include "source/fuzz/transformation_set_loop_control.h"
#include "source/fuzz/transformation_set_memory_operands_mask.h"
#include "source/fuzz/transformation_set_selection_control.h"
#include "source/fuzz/transformation_split_block.h"
#include "source/fuzz/transformation_vector_shuffle.h"
#include "source/util/make_unique.h"

namespace spvtools {
namespace fuzz {

Transformation::~Transformation() = default;

std::unique_ptr<Transformation> Transformation::FromMessage(
    const protobufs::Transformation& message) {
  switch (message.transformation_case()) {
    case protobufs::Transformation::TransformationCase::kAddConstantBoolean:
      return MakeUnique<TransformationAddConstantBoolean>(
          message.add_constant_boolean());
    case protobufs::Transformation::TransformationCase::kAddConstantComposite:
      return MakeUnique<TransformationAddConstantComposite>(
          message.add_constant_composite());
    case protobufs::Transformation::TransformationCase::kAddConstantScalar:
      return MakeUnique<TransformationAddConstantScalar>(
          message.add_constant_scalar());
    case protobufs::Transformation::TransformationCase::kAddDeadBlock:
      return MakeUnique<TransformationAddDeadBlock>(message.add_dead_block());
    case protobufs::Transformation::TransformationCase::kAddDeadBreak:
      return MakeUnique<TransformationAddDeadBreak>(message.add_dead_break());
    case protobufs::Transformation::TransformationCase::kAddDeadContinue:
      return MakeUnique<TransformationAddDeadContinue>(
          message.add_dead_continue());
    case protobufs::Transformation::TransformationCase::kAddFunction:
      return MakeUnique<TransformationAddFunction>(message.add_function());
    case protobufs::Transformation::TransformationCase::kAddGlobalUndef:
      return MakeUnique<TransformationAddGlobalUndef>(
          message.add_global_undef());
    case protobufs::Transformation::TransformationCase::kAddGlobalVariable:
      return MakeUnique<TransformationAddGlobalVariable>(
          message.add_global_variable());
    case protobufs::Transformation::TransformationCase::
        kAddNoContractionDecoration:
      return MakeUnique<TransformationAddNoContractionDecoration>(
          message.add_no_contraction_decoration());
    case protobufs::Transformation::TransformationCase::kAddTypeArray:
      return MakeUnique<TransformationAddTypeArray>(message.add_type_array());
    case protobufs::Transformation::TransformationCase::kAddTypeBoolean:
      return MakeUnique<TransformationAddTypeBoolean>(
          message.add_type_boolean());
    case protobufs::Transformation::TransformationCase::kAddTypeFloat:
      return MakeUnique<TransformationAddTypeFloat>(message.add_type_float());
    case protobufs::Transformation::TransformationCase::kAddTypeFunction:
      return MakeUnique<TransformationAddTypeFunction>(
          message.add_type_function());
    case protobufs::Transformation::TransformationCase::kAddTypeInt:
      return MakeUnique<TransformationAddTypeInt>(message.add_type_int());
    case protobufs::Transformation::TransformationCase::kAddTypeMatrix:
      return MakeUnique<TransformationAddTypeMatrix>(message.add_type_matrix());
    case protobufs::Transformation::TransformationCase::kAddTypePointer:
      return MakeUnique<TransformationAddTypePointer>(
          message.add_type_pointer());
    case protobufs::Transformation::TransformationCase::kAddTypeStruct:
      return MakeUnique<TransformationAddTypeStruct>(message.add_type_struct());
    case protobufs::Transformation::TransformationCase::kAddTypeVector:
      return MakeUnique<TransformationAddTypeVector>(message.add_type_vector());
    case protobufs::Transformation::TransformationCase::kCompositeConstruct:
      return MakeUnique<TransformationCompositeConstruct>(
          message.composite_construct());
    case protobufs::Transformation::TransformationCase::kCompositeExtract:
      return MakeUnique<TransformationCompositeExtract>(
          message.composite_extract());
    case protobufs::Transformation::TransformationCase::kCopyObject:
      return MakeUnique<TransformationCopyObject>(message.copy_object());
    case protobufs::Transformation::TransformationCase::kMergeBlocks:
      return MakeUnique<TransformationMergeBlocks>(message.merge_blocks());
    case protobufs::Transformation::TransformationCase::kMoveBlockDown:
      return MakeUnique<TransformationMoveBlockDown>(message.move_block_down());
    case protobufs::Transformation::TransformationCase::kOutlineFunction:
      return MakeUnique<TransformationOutlineFunction>(
          message.outline_function());
    case protobufs::Transformation::TransformationCase::
        kReplaceBooleanConstantWithConstantBinary:
      return MakeUnique<TransformationReplaceBooleanConstantWithConstantBinary>(
          message.replace_boolean_constant_with_constant_binary());
    case protobufs::Transformation::TransformationCase::
        kReplaceConstantWithUniform:
      return MakeUnique<TransformationReplaceConstantWithUniform>(
          message.replace_constant_with_uniform());
    case protobufs::Transformation::TransformationCase::kReplaceIdWithSynonym:
      return MakeUnique<TransformationReplaceIdWithSynonym>(
          message.replace_id_with_synonym());
    case protobufs::Transformation::TransformationCase::kSetFunctionControl:
      return MakeUnique<TransformationSetFunctionControl>(
          message.set_function_control());
    case protobufs::Transformation::TransformationCase::kSetLoopControl:
      return MakeUnique<TransformationSetLoopControl>(
          message.set_loop_control());
    case protobufs::Transformation::TransformationCase::kSetMemoryOperandsMask:
      return MakeUnique<TransformationSetMemoryOperandsMask>(
          message.set_memory_operands_mask());
    case protobufs::Transformation::TransformationCase::kSetSelectionControl:
      return MakeUnique<TransformationSetSelectionControl>(
          message.set_selection_control());
    case protobufs::Transformation::TransformationCase::kSplitBlock:
      return MakeUnique<TransformationSplitBlock>(message.split_block());
    case protobufs::Transformation::TransformationCase::kVectorShuffle:
      return MakeUnique<TransformationVectorShuffle>(message.vector_shuffle());
    case protobufs::Transformation::TRANSFORMATION_NOT_SET:
      assert(false && "An unset transformation was encountered.");
      return nullptr;
  }
  assert(false && "Should be unreachable as all cases must be handled above.");
  return nullptr;
}

bool Transformation::CheckIdIsFreshAndNotUsedByThisTransformation(
    uint32_t id, opt::IRContext* context,
    std::set<uint32_t>* ids_used_by_this_transformation) {
  if (!fuzzerutil::IsFreshId(context, id)) {
    return false;
  }
  if (ids_used_by_this_transformation->count(id) != 0) {
    return false;
  }
  ids_used_by_this_transformation->insert(id);
  return true;
}

}  // namespace fuzz
}  // namespace spvtools
