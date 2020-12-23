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

#include "source/fuzz/fuzzer_pass_construct_composites.h"

#include <memory>

#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/transformation_composite_construct.h"

namespace spvtools {
namespace fuzz {

FuzzerPassConstructComposites::FuzzerPassConstructComposites(
    opt::IRContext* ir_context, TransformationContext* transformation_context,
    FuzzerContext* fuzzer_context,
    protobufs::TransformationSequence* transformations)
    : FuzzerPass(ir_context, transformation_context, fuzzer_context,
                 transformations) {}

FuzzerPassConstructComposites::~FuzzerPassConstructComposites() = default;

void FuzzerPassConstructComposites::Apply() {
  // Gather up the ids of all composite types, but skip block-/buffer
  // block-decorated struct types.
  std::vector<uint32_t> composite_type_ids;
  for (auto& inst : GetIRContext()->types_values()) {
    if (fuzzerutil::IsCompositeType(
            GetIRContext()->get_type_mgr()->GetType(inst.result_id())) &&
        !fuzzerutil::HasBlockOrBufferBlockDecoration(GetIRContext(),
                                                     inst.result_id())) {
      composite_type_ids.push_back(inst.result_id());
    }
  }

  ForEachInstructionWithInstructionDescriptor(
      [this, &composite_type_ids](
          opt::Function* function, opt::BasicBlock* block,
          opt::BasicBlock::iterator inst_it,
          const protobufs::InstructionDescriptor& instruction_descriptor)
          -> void {
        // Check whether it is legitimate to insert a composite construction
        // before the instruction.
        if (!fuzzerutil::CanInsertOpcodeBeforeInstruction(
                SpvOpCompositeConstruct, inst_it)) {
          return;
        }

        // Randomly decide whether to try inserting an object copy here.
        if (!GetFuzzerContext()->ChoosePercentage(
                GetFuzzerContext()->GetChanceOfConstructingComposite())) {
          return;
        }

        // For each instruction that is available at this program point (i.e. an
        // instruction that is global or whose definition strictly dominates the
        // program point) and suitable for making a synonym of, associate it
        // with the id of its result type.
        TypeIdToInstructions type_id_to_available_instructions;
        auto available_instructions = FindAvailableInstructions(
            function, block, inst_it,
            [this](opt::IRContext* ir_context, opt::Instruction* inst) {
              if (!inst->result_id() || !inst->type_id()) {
                return false;
              }

              // If the id is irrelevant, we can use it since it will not
              // participate in DataSynonym fact. Otherwise, we should be able
              // to produce a synonym out of the id.
              return GetTransformationContext()
                         ->GetFactManager()
                         ->IdIsIrrelevant(inst->result_id()) ||
                     fuzzerutil::CanMakeSynonymOf(
                         ir_context, *GetTransformationContext(), inst);
            });
        for (auto instruction : available_instructions) {
          RecordAvailableInstruction(instruction,
                                     &type_id_to_available_instructions);
        }

        // At this point, |composite_type_ids| captures all the composite types
        // we could try to create, while |type_id_to_available_instructions|
        // captures all the available result ids we might use, organized by
        // type.

        // Now we try to find a composite that we can construct.  We might not
        // manage, if there is a paucity of available ingredients in the module
        // (e.g. if our only available composite was a boolean vector and we had
        // no instructions generating boolean result types available).
        //
        // If we succeed, |chosen_composite_type| will end up being non-zero,
        // and |constructor_arguments| will end up giving us result ids suitable
        // for constructing a composite of that type.  Otherwise these variables
        // will remain 0 and null respectively.
        uint32_t chosen_composite_type = 0;
        std::vector<uint32_t> constructor_arguments;

        // Initially, all composite type ids are available for us to try.  Keep
        // trying until we run out of options.
        auto composites_to_try_constructing = composite_type_ids;
        while (!composites_to_try_constructing.empty()) {
          // Remove a composite type from the composite types left for us to
          // try.
          auto next_composite_to_try_constructing =
              GetFuzzerContext()->RemoveAtRandomIndex(
                  &composites_to_try_constructing);

          // Now try to construct a composite of this type, using an appropriate
          // helper method depending on the kind of composite type.
          auto composite_type_inst = GetIRContext()->get_def_use_mgr()->GetDef(
              next_composite_to_try_constructing);
          switch (composite_type_inst->opcode()) {
            case SpvOpTypeArray:
              constructor_arguments = FindComponentsToConstructArray(
                  *composite_type_inst, type_id_to_available_instructions);
              break;
            case SpvOpTypeMatrix:
              constructor_arguments = FindComponentsToConstructMatrix(
                  *composite_type_inst, type_id_to_available_instructions);
              break;
            case SpvOpTypeStruct:
              constructor_arguments = FindComponentsToConstructStruct(
                  *composite_type_inst, type_id_to_available_instructions);
              break;
            case SpvOpTypeVector:
              constructor_arguments = FindComponentsToConstructVector(
                  *composite_type_inst, type_id_to_available_instructions);
              break;
            default:
              assert(false &&
                     "The space of possible composite types should be covered "
                     "by the above cases.");
              break;
          }
          if (!constructor_arguments.empty()) {
            // We succeeded!  Note the composite type we finally settled on, and
            // exit from the loop.
            chosen_composite_type = next_composite_to_try_constructing;
            break;
          }
        }

        if (!chosen_composite_type) {
          // We did not manage to make a composite; return 0 to indicate that no
          // instructions were added.
          assert(constructor_arguments.empty());
          return;
        }
        assert(!constructor_arguments.empty());

        // Make and apply a transformation.
        ApplyTransformation(TransformationCompositeConstruct(
            chosen_composite_type, constructor_arguments,
            instruction_descriptor, GetFuzzerContext()->GetFreshId()));
      });
}

void FuzzerPassConstructComposites::RecordAvailableInstruction(
    opt::Instruction* inst,
    TypeIdToInstructions* type_id_to_available_instructions) {
  if (type_id_to_available_instructions->count(inst->type_id()) == 0) {
    (*type_id_to_available_instructions)[inst->type_id()] = {};
  }
  type_id_to_available_instructions->at(inst->type_id()).push_back(inst);
}

std::vector<uint32_t>
FuzzerPassConstructComposites::FindComponentsToConstructArray(
    const opt::Instruction& array_type_instruction,
    const TypeIdToInstructions& type_id_to_available_instructions) {
  assert(array_type_instruction.opcode() == SpvOpTypeArray &&
         "Precondition: instruction must be an array type.");

  // Get the element type for the array.
  auto element_type_id = array_type_instruction.GetSingleWordInOperand(0);

  // Get all instructions at our disposal that compute something of this element
  // type.
  auto available_instructions =
      type_id_to_available_instructions.find(element_type_id);

  if (available_instructions == type_id_to_available_instructions.cend()) {
    // If there are not any instructions available that compute the element type
    // of the array then we are not in a position to construct a composite with
    // this array type.
    return {};
  }

  uint32_t array_length =
      GetIRContext()
          ->get_def_use_mgr()
          ->GetDef(array_type_instruction.GetSingleWordInOperand(1))
          ->GetSingleWordInOperand(0);

  std::vector<uint32_t> result;
  for (uint32_t index = 0; index < array_length; index++) {
    result.push_back(available_instructions
                         ->second[GetFuzzerContext()->RandomIndex(
                             available_instructions->second)]
                         ->result_id());
  }
  return result;
}

std::vector<uint32_t>
FuzzerPassConstructComposites::FindComponentsToConstructMatrix(
    const opt::Instruction& matrix_type_instruction,
    const TypeIdToInstructions& type_id_to_available_instructions) {
  assert(matrix_type_instruction.opcode() == SpvOpTypeMatrix &&
         "Precondition: instruction must be a matrix type.");

  // Get the element type for the matrix.
  auto element_type_id = matrix_type_instruction.GetSingleWordInOperand(0);

  // Get all instructions at our disposal that compute something of this element
  // type.
  auto available_instructions =
      type_id_to_available_instructions.find(element_type_id);

  if (available_instructions == type_id_to_available_instructions.cend()) {
    // If there are not any instructions available that compute the element type
    // of the matrix then we are not in a position to construct a composite with
    // this matrix type.
    return {};
  }
  std::vector<uint32_t> result;
  for (uint32_t index = 0;
       index < matrix_type_instruction.GetSingleWordInOperand(1); index++) {
    result.push_back(available_instructions
                         ->second[GetFuzzerContext()->RandomIndex(
                             available_instructions->second)]
                         ->result_id());
  }
  return result;
}

std::vector<uint32_t>
FuzzerPassConstructComposites::FindComponentsToConstructStruct(
    const opt::Instruction& struct_type_instruction,
    const TypeIdToInstructions& type_id_to_available_instructions) {
  assert(struct_type_instruction.opcode() == SpvOpTypeStruct &&
         "Precondition: instruction must be a struct type.");
  std::vector<uint32_t> result;
  // Consider the type of each field of the struct.
  for (uint32_t in_operand_index = 0;
       in_operand_index < struct_type_instruction.NumInOperands();
       in_operand_index++) {
    auto element_type_id =
        struct_type_instruction.GetSingleWordInOperand(in_operand_index);
    // Find the instructions at our disposal that compute something of the field
    // type.
    auto available_instructions =
        type_id_to_available_instructions.find(element_type_id);
    if (available_instructions == type_id_to_available_instructions.cend()) {
      // If there are no such instructions, we cannot construct a composite of
      // this struct type.
      return {};
    }
    result.push_back(available_instructions
                         ->second[GetFuzzerContext()->RandomIndex(
                             available_instructions->second)]
                         ->result_id());
  }
  return result;
}

std::vector<uint32_t>
FuzzerPassConstructComposites::FindComponentsToConstructVector(
    const opt::Instruction& vector_type_instruction,
    const TypeIdToInstructions& type_id_to_available_instructions) {
  assert(vector_type_instruction.opcode() == SpvOpTypeVector &&
         "Precondition: instruction must be a vector type.");

  // Get details of the type underlying the vector, and the width of the vector,
  // for convenience.
  auto element_type_id = vector_type_instruction.GetSingleWordInOperand(0);
  auto element_type = GetIRContext()->get_type_mgr()->GetType(element_type_id);
  auto element_count = vector_type_instruction.GetSingleWordInOperand(1);

  // Collect a mapping, from type id to width, for scalar/vector types that are
  // smaller in width than |vector_type|, but that have the same underlying
  // type.  For example, if |vector_type| is vec4, the mapping will be:
  //   { float -> 1, vec2 -> 2, vec3 -> 3 }
  // The mapping will have missing entries if some of these types do not exist.

  std::map<uint32_t, uint32_t> smaller_vector_type_id_to_width;
  // Add the underlying type.  This id must exist, in order for |vector_type| to
  // exist.
  smaller_vector_type_id_to_width[element_type_id] = 1;

  // Now add every vector type with width at least 2, and less than the width of
  // |vector_type|.
  for (uint32_t width = 2; width < element_count; width++) {
    opt::analysis::Vector smaller_vector_type(element_type, width);
    auto smaller_vector_type_id =
        GetIRContext()->get_type_mgr()->GetId(&smaller_vector_type);
    // We might find that there is no declared type of this smaller width.
    // For example, a module can declare vec4 without having declared vec2 or
    // vec3.
    if (smaller_vector_type_id) {
      smaller_vector_type_id_to_width[smaller_vector_type_id] = width;
    }
  }

  // Now we know the types that are available to us, we set about populating a
  // vector of the right length.  We do this by deciding, with no order in mind,
  // which instructions we will use to populate the vector, and subsequently
  // randomly choosing an order.  This is to avoid biasing construction of
  // vectors with smaller vectors to the left and scalars to the right.  That is
  // a concern because, e.g. in the case of populating a vec4, if we populate
  // the constructor instructions left-to-right, we can always choose a vec3 to
  // construct the first three elements, but can only choose a vec3 to construct
  // the last three elements if we chose a float to construct the first element
  // (otherwise there will not be space left for a vec3).

  uint32_t vector_slots_used = 0;
  // The instructions we will use to construct the vector, in no particular
  // order at this stage.
  std::vector<opt::Instruction*> instructions_to_use;

  while (vector_slots_used < element_count) {
    std::vector<opt::Instruction*> instructions_to_choose_from;
    for (auto& entry : smaller_vector_type_id_to_width) {
      if (entry.second >
          std::min(element_count - 1, element_count - vector_slots_used)) {
        continue;
      }
      auto available_instructions =
          type_id_to_available_instructions.find(entry.first);
      if (available_instructions == type_id_to_available_instructions.cend()) {
        continue;
      }
      instructions_to_choose_from.insert(instructions_to_choose_from.end(),
                                         available_instructions->second.begin(),
                                         available_instructions->second.end());
    }
    if (instructions_to_choose_from.empty()) {
      // We may get unlucky and find that there are not any instructions to
      // choose from.  In this case we give up constructing a composite of this
      // vector type.  It might be that we could construct the composite in
      // another manner, so we could opt to retry a few times here, but it is
      // simpler to just give up on the basis that this will not happen
      // frequently.
      return {};
    }
    auto instruction_to_use =
        instructions_to_choose_from[GetFuzzerContext()->RandomIndex(
            instructions_to_choose_from)];
    instructions_to_use.push_back(instruction_to_use);
    auto chosen_type =
        GetIRContext()->get_type_mgr()->GetType(instruction_to_use->type_id());
    if (chosen_type->AsVector()) {
      assert(chosen_type->AsVector()->element_type() == element_type);
      assert(chosen_type->AsVector()->element_count() < element_count);
      assert(chosen_type->AsVector()->element_count() <=
             element_count - vector_slots_used);
      vector_slots_used += chosen_type->AsVector()->element_count();
    } else {
      assert(chosen_type == element_type);
      vector_slots_used += 1;
    }
  }
  assert(vector_slots_used == element_count);

  std::vector<uint32_t> result;
  std::vector<uint32_t> operands;
  while (!instructions_to_use.empty()) {
    auto index = GetFuzzerContext()->RandomIndex(instructions_to_use);
    result.push_back(instructions_to_use[index]->result_id());
    instructions_to_use.erase(instructions_to_use.begin() + index);
  }
  assert(result.size() > 1);
  return result;
}

}  // namespace fuzz
}  // namespace spvtools
