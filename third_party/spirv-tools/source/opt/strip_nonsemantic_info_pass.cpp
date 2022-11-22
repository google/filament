// Copyright (c) 2018 Google LLC
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

#include "source/opt/strip_nonsemantic_info_pass.h"

#include <cstring>
#include <vector>

#include "source/opt/instruction.h"
#include "source/opt/ir_context.h"
#include "source/util/string_utils.h"

namespace spvtools {
namespace opt {

Pass::Status StripNonSemanticInfoPass::Process() {
  bool modified = false;

  std::vector<Instruction*> to_remove;

  bool other_uses_for_decorate_string = false;
  for (auto& inst : context()->module()->annotations()) {
    switch (inst.opcode()) {
      case SpvOpDecorateStringGOOGLE:
        if (inst.GetSingleWordInOperand(1) == SpvDecorationHlslSemanticGOOGLE ||
            inst.GetSingleWordInOperand(1) == SpvDecorationUserTypeGOOGLE) {
          to_remove.push_back(&inst);
        } else {
          other_uses_for_decorate_string = true;
        }
        break;

      case SpvOpMemberDecorateStringGOOGLE:
        if (inst.GetSingleWordInOperand(2) == SpvDecorationHlslSemanticGOOGLE ||
            inst.GetSingleWordInOperand(2) == SpvDecorationUserTypeGOOGLE) {
          to_remove.push_back(&inst);
        } else {
          other_uses_for_decorate_string = true;
        }
        break;

      case SpvOpDecorateId:
        if (inst.GetSingleWordInOperand(1) ==
            SpvDecorationHlslCounterBufferGOOGLE) {
          to_remove.push_back(&inst);
        }
        break;

      default:
        break;
    }
  }

  for (auto& inst : context()->module()->extensions()) {
    const std::string ext_name = inst.GetInOperand(0).AsString();
    if (ext_name == "SPV_GOOGLE_hlsl_functionality1") {
      to_remove.push_back(&inst);
    } else if (ext_name == "SPV_GOOGLE_user_type") {
      to_remove.push_back(&inst);
    } else if (!other_uses_for_decorate_string &&
               ext_name == "SPV_GOOGLE_decorate_string") {
      to_remove.push_back(&inst);
    } else if (ext_name == "SPV_KHR_non_semantic_info") {
      to_remove.push_back(&inst);
    }
  }

  // remove any extended inst imports that are non semantic
  std::unordered_set<uint32_t> non_semantic_sets;
  for (auto& inst : context()->module()->ext_inst_imports()) {
    assert(inst.opcode() == SpvOpExtInstImport &&
           "Expecting an import of an extension's instruction set.");
    const std::string extension_name = inst.GetInOperand(0).AsString();
    if (spvtools::utils::starts_with(extension_name, "NonSemantic.")) {
      non_semantic_sets.insert(inst.result_id());
      to_remove.push_back(&inst);
    }
  }

  // if we removed some non-semantic sets, then iterate over the instructions in
  // the module to remove any OpExtInst that referenced those sets
  if (!non_semantic_sets.empty()) {
    context()->module()->ForEachInst(
        [&non_semantic_sets, &to_remove](Instruction* inst) {
          if (inst->opcode() == SpvOpExtInst) {
            if (non_semantic_sets.find(inst->GetSingleWordInOperand(0)) !=
                non_semantic_sets.end()) {
              to_remove.push_back(inst);
            }
          }
        },
        true);
  }

  for (auto* inst : to_remove) {
    modified = true;
    context()->KillInst(inst);
  }

  return modified ? Status::SuccessWithChange : Status::SuccessWithoutChange;
}

}  // namespace opt
}  // namespace spvtools
