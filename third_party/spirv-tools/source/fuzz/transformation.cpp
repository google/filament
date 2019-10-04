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

#include "source/util/make_unique.h"
#include "transformation_add_constant_boolean.h"
#include "transformation_add_constant_scalar.h"
#include "transformation_add_dead_break.h"
#include "transformation_add_dead_continue.h"
#include "transformation_add_type_boolean.h"
#include "transformation_add_type_float.h"
#include "transformation_add_type_int.h"
#include "transformation_add_type_pointer.h"
#include "transformation_copy_object.h"
#include "transformation_move_block_down.h"
#include "transformation_replace_boolean_constant_with_constant_binary.h"
#include "transformation_replace_constant_with_uniform.h"
#include "transformation_replace_id_with_synonym.h"
#include "transformation_split_block.h"

namespace spvtools {
namespace fuzz {

Transformation::~Transformation() = default;

std::unique_ptr<Transformation> Transformation::FromMessage(
    const protobufs::Transformation& message) {
  switch (message.transformation_case()) {
    case protobufs::Transformation::TransformationCase::kAddConstantBoolean:
      return MakeUnique<TransformationAddConstantBoolean>(
          message.add_constant_boolean());
    case protobufs::Transformation::TransformationCase::kAddConstantScalar:
      return MakeUnique<TransformationAddConstantScalar>(
          message.add_constant_scalar());
    case protobufs::Transformation::TransformationCase::kAddDeadBreak:
      return MakeUnique<TransformationAddDeadBreak>(message.add_dead_break());
    case protobufs::Transformation::TransformationCase::kAddDeadContinue:
      return MakeUnique<TransformationAddDeadContinue>(
          message.add_dead_continue());
    case protobufs::Transformation::TransformationCase::kAddTypeBoolean:
      return MakeUnique<TransformationAddTypeBoolean>(
          message.add_type_boolean());
    case protobufs::Transformation::TransformationCase::kAddTypeFloat:
      return MakeUnique<TransformationAddTypeFloat>(message.add_type_float());
    case protobufs::Transformation::TransformationCase::kAddTypeInt:
      return MakeUnique<TransformationAddTypeInt>(message.add_type_int());
    case protobufs::Transformation::TransformationCase::kAddTypePointer:
      return MakeUnique<TransformationAddTypePointer>(
          message.add_type_pointer());
    case protobufs::Transformation::TransformationCase::kCopyObject:
      return MakeUnique<TransformationCopyObject>(message.copy_object());
    case protobufs::Transformation::TransformationCase::kMoveBlockDown:
      return MakeUnique<TransformationMoveBlockDown>(message.move_block_down());
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
    case protobufs::Transformation::TransformationCase::kSplitBlock:
      return MakeUnique<TransformationSplitBlock>(message.split_block());
    case protobufs::Transformation::TRANSFORMATION_NOT_SET:
      assert(false && "An unset transformation was encountered.");
      return nullptr;
  }
  assert(false && "Should be unreachable as all cases must be handled above.");
  return nullptr;
}

}  // namespace fuzz
}  // namespace spvtools
