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

#include "source/fuzz/id_use_descriptor.h"
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
  std::vector<TransformationReplaceIdWithSynonym> transformations_to_apply;

  for (auto id_with_known_synonyms :
       GetFactManager()->GetIdsForWhichSynonymsAreKnown()) {
    GetIRContext()->get_def_use_mgr()->ForEachUse(
        id_with_known_synonyms,
        [this, id_with_known_synonyms, &transformations_to_apply](
            opt::Instruction* use_inst, uint32_t use_index) -> void {
          auto block_containing_use = GetIRContext()->get_instr_block(use_inst);
          // The use might not be in a block; e.g. it could be a decoration.
          if (!block_containing_use) {
            return;
          }
          if (!GetFuzzerContext()->ChoosePercentage(
                  GetFuzzerContext()->GetChanceOfReplacingIdWithSynonym())) {
            return;
          }

          // |use_index| is the absolute index of the operand.  We require
          // the index of the operand restricted to input operands only, so
          // we subtract the number of non-input operands from |use_index|.
          uint32_t use_in_operand_index =
              use_index - use_inst->NumOperands() + use_inst->NumInOperands();

          std::vector<const protobufs::DataDescriptor*> synonyms_to_try;
          for (auto& data_descriptor :
               GetFactManager()->GetSynonymsForId(id_with_known_synonyms)) {
            synonyms_to_try.push_back(&data_descriptor);
          }
          while (!synonyms_to_try.empty()) {
            auto synonym_index =
                GetFuzzerContext()->RandomIndex(synonyms_to_try);
            auto synonym_to_try = synonyms_to_try[synonym_index];
            synonyms_to_try.erase(synonyms_to_try.begin() + synonym_index);
            assert(synonym_to_try->index().empty() &&
                   "Right now we only support id == id synonyms; supporting "
                   "e.g. id == index-into-vector will come later");

            if (!TransformationReplaceIdWithSynonym::
                    ReplacingUseWithSynonymIsOk(GetIRContext(), use_inst,
                                                use_in_operand_index,
                                                *synonym_to_try)) {
              continue;
            }

            TransformationReplaceIdWithSynonym replace_id_transformation(
                transformation::MakeIdUseDescriptorFromUse(
                    GetIRContext(), use_inst, use_in_operand_index),
                *synonym_to_try, 0);
            // The transformation should be applicable by construction.
            assert(replace_id_transformation.IsApplicable(GetIRContext(),
                                                          *GetFactManager()));
            // We cannot actually apply the transformation here, as this would
            // change the analysis results that are being depended on for usage
            // iteration.  We instead store them up and apply them at the end
            // of the method.
            transformations_to_apply.push_back(replace_id_transformation);
            break;
          }
        });
  }

  for (auto& replace_id_transformation : transformations_to_apply) {
    // Even though replacing id uses with synonyms may lead to new instructions
    // (to compute indices into composites), as these instructions will generate
    // ids, their presence should not affect the id use descriptors that were
    // computed during the creation of transformations. Thus transformations
    // should not disable one another.
    assert(replace_id_transformation.IsApplicable(GetIRContext(),
                                                  *GetFactManager()));
    replace_id_transformation.Apply(GetIRContext(), GetFactManager());
    *GetTransformations()->add_transformation() =
        replace_id_transformation.ToMessage();
  }
}

}  // namespace fuzz
}  // namespace spvtools
