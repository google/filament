// Copyright (c) 2018 Google Inc.
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

#include "source/reduce/remove_unreferenced_instruction_reduction_opportunity_finder.h"

#include "source/opcode.h"
#include "source/opt/instruction.h"
#include "source/reduce/remove_instruction_reduction_opportunity.h"

namespace spvtools {
namespace reduce {

RemoveUnreferencedInstructionReductionOpportunityFinder::
    RemoveUnreferencedInstructionReductionOpportunityFinder(
        bool remove_constants_and_undefs)
    : remove_constants_and_undefs_(remove_constants_and_undefs) {}

std::vector<std::unique_ptr<ReductionOpportunity>>
RemoveUnreferencedInstructionReductionOpportunityFinder::
    GetAvailableOpportunities(opt::IRContext* context) const {
  std::vector<std::unique_ptr<ReductionOpportunity>> result;

  for (auto& inst : context->module()->debugs1()) {
    if (context->get_def_use_mgr()->NumUses(&inst) > 0) {
      continue;
    }
    result.push_back(MakeUnique<RemoveInstructionReductionOpportunity>(&inst));
  }

  for (auto& inst : context->module()->debugs2()) {
    if (context->get_def_use_mgr()->NumUses(&inst) > 0) {
      continue;
    }
    result.push_back(MakeUnique<RemoveInstructionReductionOpportunity>(&inst));
  }

  for (auto& inst : context->module()->debugs3()) {
    if (context->get_def_use_mgr()->NumUses(&inst) > 0) {
      continue;
    }
    result.push_back(MakeUnique<RemoveInstructionReductionOpportunity>(&inst));
  }

  for (auto& inst : context->module()->ext_inst_debuginfo()) {
    if (context->get_def_use_mgr()->NumUses(&inst) > 0) {
      continue;
    }
    result.push_back(MakeUnique<RemoveInstructionReductionOpportunity>(&inst));
  }

  for (auto& inst : context->module()->types_values()) {
    if (context->get_def_use_mgr()->NumUsers(&inst) > 0) {
      continue;
    }
    if (!remove_constants_and_undefs_ &&
        spvOpcodeIsConstantOrUndef(inst.opcode())) {
      continue;
    }
    result.push_back(MakeUnique<RemoveInstructionReductionOpportunity>(&inst));
  }

  for (auto& inst : context->module()->annotations()) {
    if (context->get_def_use_mgr()->NumUsers(&inst) > 0) {
      continue;
    }

    uint32_t decoration = SpvDecorationMax;
    switch (inst.opcode()) {
      case SpvOpDecorate:
      case SpvOpDecorateId:
      case SpvOpDecorateString:
        decoration = inst.GetSingleWordInOperand(1u);
        break;
      case SpvOpMemberDecorate:
      case SpvOpMemberDecorateString:
        decoration = inst.GetSingleWordInOperand(2u);
        break;
      default:
        break;
    }

    // We conservatively only remove specific decorations that we believe will
    // not change the shader interface, will not make the shader invalid, will
    // actually be found in practice, etc.

    switch (decoration) {
      case SpvDecorationRelaxedPrecision:
      case SpvDecorationNoSignedWrap:
      case SpvDecorationNoContraction:
      case SpvDecorationNoUnsignedWrap:
      case SpvDecorationUserSemantic:
        break;
      default:
        // Give up.
        continue;
    }

    result.push_back(MakeUnique<RemoveInstructionReductionOpportunity>(&inst));
  }

  for (auto& function : *context->module()) {
    for (auto& block : function) {
      for (auto& inst : block) {
        if (context->get_def_use_mgr()->NumUses(&inst) > 0) {
          continue;
        }
        if (!remove_constants_and_undefs_ &&
            spvOpcodeIsConstantOrUndef(inst.opcode())) {
          continue;
        }
        if (spvOpcodeIsBlockTerminator(inst.opcode()) ||
            inst.opcode() == SpvOpSelectionMerge ||
            inst.opcode() == SpvOpLoopMerge) {
          // In this reduction pass we do not want to affect static
          // control flow.
          continue;
        }
        // Given that we're in a block, we should only get here if
        // the instruction is not directly related to control flow;
        // i.e., it's some straightforward instruction with an
        // unused result, like an arithmetic operation or function
        // call.
        result.push_back(
            MakeUnique<RemoveInstructionReductionOpportunity>(&inst));
      }
    }
  }
  return result;
}

std::string RemoveUnreferencedInstructionReductionOpportunityFinder::GetName()
    const {
  return "RemoveUnreferencedInstructionReductionOpportunityFinder";
}

}  // namespace reduce
}  // namespace spvtools
