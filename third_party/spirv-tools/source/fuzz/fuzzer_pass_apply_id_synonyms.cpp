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

#include "source/fuzz/fuzzer_pass_apply_id_synonyms.h"

#include "source/fuzz/data_descriptor.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/id_use_descriptor.h"
#include "source/fuzz/instruction_descriptor.h"
#include "source/fuzz/transformation_composite_extract.h"
#include "source/fuzz/transformation_replace_id_with_synonym.h"

namespace spvtools {
namespace fuzz {

FuzzerPassApplyIdSynonyms::FuzzerPassApplyIdSynonyms(
    opt::IRContext* ir_context, FactManager* fact_manager,
    FuzzerContext* fuzzer_context,
    protobufs::TransformationSequence* transformations)
    : FuzzerPass(ir_context, fact_manager, fuzzer_context, transformations) {}

FuzzerPassApplyIdSynonyms::~FuzzerPassApplyIdSynonyms() = default;

void FuzzerPassApplyIdSynonyms::Apply() {
  for (auto id_with_known_synonyms :
       GetFactManager()->GetIdsForWhichSynonymsAreKnown(GetIRContext())) {
    // Gather up all uses of |id_with_known_synonym|, and then subsequently
    // iterate over these uses.  We use this separation because, when
    // considering a given use, we might apply a transformation that will
    // invalidate the def-use manager.
    std::vector<std::pair<opt::Instruction*, uint32_t>> uses;
    GetIRContext()->get_def_use_mgr()->ForEachUse(
        id_with_known_synonyms,
        [&uses](opt::Instruction* use_inst, uint32_t use_index) -> void {
          uses.emplace_back(
              std::pair<opt::Instruction*, uint32_t>(use_inst, use_index));
        });

    for (auto& use : uses) {
      auto use_inst = use.first;
      auto use_index = use.second;
      auto block_containing_use = GetIRContext()->get_instr_block(use_inst);
      // The use might not be in a block; e.g. it could be a decoration.
      if (!block_containing_use) {
        continue;
      }
      if (!GetFuzzerContext()->ChoosePercentage(
              GetFuzzerContext()->GetChanceOfReplacingIdWithSynonym())) {
        continue;
      }
      // |use_index| is the absolute index of the operand.  We require
      // the index of the operand restricted to input operands only, so
      // we subtract the number of non-input operands from |use_index|.
      uint32_t use_in_operand_index =
          use_index - use_inst->NumOperands() + use_inst->NumInOperands();
      if (!TransformationReplaceIdWithSynonym::UseCanBeReplacedWithSynonym(
              GetIRContext(), use_inst, use_in_operand_index)) {
        continue;
      }

      std::vector<const protobufs::DataDescriptor*> synonyms_to_try;
      for (auto& data_descriptor : GetFactManager()->GetSynonymsForId(
               id_with_known_synonyms, GetIRContext())) {
        protobufs::DataDescriptor descriptor_for_this_id =
            MakeDataDescriptor(id_with_known_synonyms, {});
        if (DataDescriptorEquals()(data_descriptor, &descriptor_for_this_id)) {
          // Exclude the fact that the id is synonymous with itself.
          continue;
        }
        synonyms_to_try.push_back(data_descriptor);
      }
      while (!synonyms_to_try.empty()) {
        auto synonym_index = GetFuzzerContext()->RandomIndex(synonyms_to_try);
        auto synonym_to_try = synonyms_to_try[synonym_index];
        synonyms_to_try.erase(synonyms_to_try.begin() + synonym_index);

        if (synonym_to_try->index_size() > 0 &&
            use_inst->opcode() == SpvOpPhi) {
          // We are trying to replace an operand to an OpPhi.  This means
          // we cannot use a composite synonym, because that requires
          // extracting a component from a composite and we cannot insert
          // an extract instruction before an OpPhi.
          //
          // TODO(afd): We could consider inserting the extract instruction
          //  into the relevant parent block of the OpPhi.
          continue;
        }

        if (!TransformationReplaceIdWithSynonym::IdsIsAvailableAtUse(
                GetIRContext(), use_inst, use_in_operand_index,
                synonym_to_try->object())) {
          continue;
        }

        // We either replace the use with an id known to be synonymous, or
        // an id that will hold the result of extracting a synonym from a
        // composite.
        uint32_t id_with_which_to_replace_use;
        if (synonym_to_try->index_size() == 0) {
          id_with_which_to_replace_use = synonym_to_try->object();
        } else {
          id_with_which_to_replace_use = GetFuzzerContext()->GetFreshId();
          protobufs::InstructionDescriptor instruction_to_insert_before =
              MakeInstructionDescriptor(GetIRContext(), use_inst);
          TransformationCompositeExtract composite_extract_transformation(
              instruction_to_insert_before, id_with_which_to_replace_use,
              synonym_to_try->object(),
              fuzzerutil::RepeatedFieldToVector(synonym_to_try->index()));
          assert(composite_extract_transformation.IsApplicable(
                     GetIRContext(), *GetFactManager()) &&
                 "Transformation should be applicable by construction.");
          composite_extract_transformation.Apply(GetIRContext(),
                                                 GetFactManager());
          *GetTransformations()->add_transformation() =
              composite_extract_transformation.ToMessage();
        }

        TransformationReplaceIdWithSynonym replace_id_transformation(
            MakeIdUseDescriptorFromUse(GetIRContext(), use_inst,
                                       use_in_operand_index),
            id_with_which_to_replace_use);

        // The transformation should be applicable by construction.
        assert(replace_id_transformation.IsApplicable(GetIRContext(),
                                                      *GetFactManager()));
        replace_id_transformation.Apply(GetIRContext(), GetFactManager());
        *GetTransformations()->add_transformation() =
            replace_id_transformation.ToMessage();
        break;
      }
    }
  }
}

}  // namespace fuzz
}  // namespace spvtools
