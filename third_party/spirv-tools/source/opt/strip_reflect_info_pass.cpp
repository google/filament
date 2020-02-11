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

#include "source/opt/strip_reflect_info_pass.h"

#include <cstring>
#include <vector>

#include "source/opt/instruction.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace opt {

Pass::Status StripReflectInfoPass::Process() {
  bool modified = false;

  std::vector<Instruction*> to_remove;

  bool other_uses_for_decorate_string = false;
  for (auto& inst : context()->module()->annotations()) {
    switch (inst.opcode()) {
      case SpvOpDecorateStringGOOGLE:
        if (inst.GetSingleWordInOperand(1) == SpvDecorationHlslSemanticGOOGLE) {
          to_remove.push_back(&inst);
        } else {
          other_uses_for_decorate_string = true;
        }
        break;

      case SpvOpMemberDecorateStringGOOGLE:
        if (inst.GetSingleWordInOperand(2) == SpvDecorationHlslSemanticGOOGLE) {
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
    const char* ext_name =
        reinterpret_cast<const char*>(&inst.GetInOperand(0).words[0]);
    if (0 == std::strcmp(ext_name, "SPV_GOOGLE_hlsl_functionality1")) {
      to_remove.push_back(&inst);
    } else if (!other_uses_for_decorate_string &&
               0 == std::strcmp(ext_name, "SPV_GOOGLE_decorate_string")) {
      to_remove.push_back(&inst);
    } else if (0 == std::strcmp(ext_name, "SPV_KHR_non_semantic_info")) {
      to_remove.push_back(&inst);
    }
  }

  // clear all debug data now if it hasn't been cleared already, to remove any
  // remaining OpString that may have been referenced by non-semantic extinsts
  for (auto& dbg : context()->debugs1()) to_remove.push_back(&dbg);
  for (auto& dbg : context()->debugs2()) to_remove.push_back(&dbg);
  for (auto& dbg : context()->debugs3()) to_remove.push_back(&dbg);
  for (auto& dbg : context()->ext_inst_debuginfo()) to_remove.push_back(&dbg);

  // remove any extended inst imports that are non semantic
  std::unordered_set<uint32_t> non_semantic_sets;
  for (auto& inst : context()->module()->ext_inst_imports()) {
    assert(inst.opcode() == SpvOpExtInstImport &&
           "Expecting an import of an extension's instruction set.");
    const char* extension_name =
        reinterpret_cast<const char*>(&inst.GetInOperand(0).words[0]);
    if (0 == std::strncmp(extension_name, "NonSemantic.", 12)) {
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
        });
  }

  // OpName must come first, since they may refer to other debug instructions.
  // If they are after the instructions that refer to, then they will be killed
  // when that instruction is killed, which will lead to a double kill.
  std::sort(to_remove.begin(), to_remove.end(),
            [](Instruction* lhs, Instruction* rhs) -> bool {
              if (lhs->opcode() == SpvOpName && rhs->opcode() != SpvOpName)
                return true;
              return false;
            });

  for (auto* inst : to_remove) {
    modified = true;
    context()->KillInst(inst);
  }

  return modified ? Status::SuccessWithChange : Status::SuccessWithoutChange;
}

}  // namespace opt
}  // namespace spvtools
