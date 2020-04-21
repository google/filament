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

#include <cmath>
#include <memory>

#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/transformation_composite_construct.h"
#include "source/util/make_unique.h"

namespace spvtools {
namespace fuzz {

FuzzerPassConstructComposites::FuzzerPassConstructComposites(
    opt::IRContext* ir_context, FactManager* fact_manager,
    FuzzerContext* fuzzer_context,
    protobufs::TransformationSequence* transformations)
    : FuzzerPass(ir_context, fact_manager, fuzzer_context, transformations) {}

FuzzerPassConstructComposites::~FuzzerPassConstructComposites() = default;

void FuzzerPassConstructComposites::Apply() {
  // Gather up the ids of all composite types.
  std::vector<uint32_t> composite_type_ids;
  for (auto& inst : GetIRContext()->types_values()) {
    if (fuzzerutil::IsCompositeType(
            GetIRContext()->get_type_mgr()->GetType(inst.result_id()))) {
      composite_type_ids.push_back(inst.result_id());
    }
  }

  MaybeAddTransformationBeforeEachInstruction(
      [this, &composite_type_ids](
          const opt::Function& function, opt::BasicBlock* block,
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
        for (auto instruction : FindAvailableInstructions(
                 function, block, inst_it, fuzzerutil::CanMakeSynonymOf)) {
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
        std::unique_ptr<std::vector<uint32_t>> constructor_arguments = nullptr;

        // Initially, all composite type ids are available for us to try.  Keep
        // trying until we run out of options.
        auto composites_to_try_constructing = composite_type_ids;
        while (!composites_to_try_constructing.empty()) {
          // Remove a composite type from the composite types left for us to
          // try.
          auto index =
              GetFuzzerContext()->RandomIndex(composites_to_try_constructing);
          auto next_composite_to_try_constructing =
              composites_to_try_constructing[index];
          composites_to_try_constructing.erase(
              composites_to_try_constructing.begin() + index);

          // Now try to construct a composite of this type, using an appropriate
          // helper method depending on the kind of composite type.
          auto composite_type = GetIRContext()->get_type_mgr()->GetType(
              next_composite_to_try_constructing);
          if (auto array_type = composite_type->AsArray()) {
            constructor_arguments = TryConstructingArrayComposite(
                *array_type, type_id_to_available_instructions);
          } else if (auto matrix_type = composite_type->AsMatrix()) {
            constructor_arguments = TryConstructingMatrixComposite(
                *matrix_type, type_id_to_available_instructions);
          } else if (auto struct_type = composite_type->AsStruct()) {
            constructor_arguments = TryConstructingStructComposite(
                *struct_type, type_id_to_available_instructions);
          } else {
            auto vector_type = composite_type->AsVector();
            assert(vector_type &&
                   "The space of possible composite types should be covered by "
                   "the above cases.");
            constructor_arguments = TryConstructingVectorComposite(
                *vector_type, type_id_to_available_instructions);
          }
          if (constructor_arguments != nullptr) {
            // We succeeded!  Note the composite type we finally settled on, and
            // exit from the loop.
            chosen_composite_type = next_composite_to_try_constructing;
            break;
          }
        }

        if (!chosen_composite_type) {
          // We did not manage to make a composite; return 0 to indicate that no
          // instructions were added.
          assert(constructor_arguments == nullptr);
          return;
        }
        assert(constructor_arguments != nullptr);

        // Make and apply a transformation.
        TransformationCompositeConstruct transformation(
            chosen_composite_type, *constructor_arguments,
            instruction_descriptor, GetFuzzerContext()->GetFreshId());
        assert(transformation.IsApplicable(GetIRContext(), *GetFactManager()) &&
               "This transformation should be applicable by construction.");
        transformation.Apply(GetIRContext(), GetFactManager());
        *GetTransformations()->add_transformation() =
            transformation.ToMessage();
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

std::unique_ptr<std::vector<uint32_t>>
FuzzerPassConstructComposites::TryConstructingArrayComposite(
    const opt::analysis::Array& array_type,
    const TypeIdToInstructions& type_id_to_available_instructions) {
  // At present we assume arrays have a constant size.
  assert(array_type.length_info().words.size() == 2);
  assert(array_type.length_info().words[0] ==
         opt::analysis::Array::LengthInfo::kConstant);

  auto result = MakeUnique<std::vector<uint32_t>>();

  // Get the element type for the array.
  auto element_type_id =
      GetIRContext()->get_type_mgr()->GetId(array_type.element_type());

  // Get all instructions at our disposal that compute something of this element
  // type.
  auto available_instructions =
      type_id_to_available_instructions.find(element_type_id);

  if (available_instructions == type_id_to_available_instructions.cend()) {
    // If there are not any instructions available that compute the element type
    // of the array then we are not in a position to construct a composite with
    // this array type.
    return nullptr;
  }
  for (uint32_t index = 0; index < array_type.length_info().words[1]; index++) {
    result->push_back(available_instructions
                          ->second[GetFuzzerContext()->RandomIndex(
                              available_instructions->second)]
                          ->result_id());
  }
  return result;
}

std::unique_ptr<std::vector<uint32_t>>
FuzzerPassConstructComposites::TryConstructingMatrixComposite(
    const opt::analysis::Matrix& matrix_type,
    const TypeIdToInstructions& type_id_to_available_instructions) {
  auto result = MakeUnique<std::vector<uint32_t>>();

  // Get the element type for the matrix.
  auto element_type_id =
      GetIRContext()->get_type_mgr()->GetId(matrix_type.element_type());

  // Get all instructions at our disposal that compute something of this element
  // type.
  auto available_instructions =
      type_id_to_available_instructions.find(element_type_id);

  if (available_instructions == type_id_to_available_instructions.cend()) {
    // If there are not any instructions available that compute the element type
    // of the matrix then we are not in a position to construct a composite with
    // this matrix type.
    return nullptr;
  }
  for (uint32_t index = 0; index < matrix_type.element_count(); index++) {
    result->push_back(available_instructions
                          ->second[GetFuzzerContext()->RandomIndex(
                              available_instructions->second)]
                          ->result_id());
  }
  return result;
}

std::unique_ptr<std::vector<uint32_t>>
FuzzerPassConstructComposites::TryConstructingStructComposite(
    const opt::analysis::Struct& struct_type,
    const TypeIdToInstructions& type_id_to_available_instructions) {
  auto result = MakeUnique<std::vector<uint32_t>>();
  // Consider the type of each field of the struct.
  for (auto element_type : struct_type.element_types()) {
    auto element_type_id = GetIRContext()->get_type_mgr()->GetId(element_type);
    // Find the instructions at our disposal that compute something of the field
    // type.
    auto available_instructions =
        type_id_to_available_instructions.find(element_type_id);
    if (available_instructions == type_id_to_available_instructions.cend()) {
      // If there are no such instructions, we cannot construct a composite of
      // this struct type.
      return nullptr;
    }
    result->push_back(available_instructions
                          ->second[GetFuzzerContext()->RandomIndex(
                              available_instructions->second)]
                          ->result_id());
  }
  return result;
}

std::unique_ptr<std::vector<uint32_t>>
FuzzerPassConstructComposites::TryConstructingVectorComposite(
    const opt::analysis::Vector& vector_type,
    const TypeIdToInstructions& type_id_to_available_instructions) {
  // Get details of the type underlying the vector, and the width of the vector,
  // for convenience.
  auto element_type = vector_type.element_type();
  auto element_count = vector_type.element_count();

  // Collect a mapping, from type id to width, for scalar/vector types that are
  // smaller in width than |vector_type|, but that have the same underlying
  // type.  For example, if |vector_type| is vec4, the mapping will be:
  //   { float -> 1, vec2 -> 2, vec3 -> 3 }
  // The mapping will have missing entries if some of these types do not exist.

  std::map<uint32_t, uint32_t> smaller_vector_type_id_to_width;
  // Add the underlying type.  This id must exist, in order for |vector_type| to
  // exist.
  auto scalar_type_id = GetIRContext()->get_type_mgr()->GetId(element_type);
  smaller_vector_type_id_to_width[scalar_type_id] = 1;

  // Now add every vector type with width at least 2, and less than the width of
  // |vector_type|.
  for (uint32_t width = 2; width < element_count; width++) {
    opt::analysis::Vector smaller_vector_type(vector_type.element_type(),
                                              width);
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

  while (vector_slots_used < vector_type.element_count()) {
    std::vector<opt::Instruction*> instructions_to_choose_from;
    for (auto& entry : smaller_vector_type_id_to_width) {
      if (entry.second >
          std::min(vector_type.element_count() - 1,
                   vector_type.element_count() - vector_slots_used)) {
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
      return nullptr;
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
  assert(vector_slots_used == vector_type.element_count());

  auto result = MakeUnique<std::vector<uint32_t>>();
  std::vector<uint32_t> operands;
  while (!instructions_to_use.empty()) {
    auto index = GetFuzzerContext()->RandomIndex(instructions_to_use);
    result->push_back(instructions_to_use[index]->result_id());
    instructions_to_use.erase(instructions_to_use.begin() + index);
  }
  assert(result->size() > 1);
  return result;
}

}  // namespace fuzz
}  // namespace spvtools
