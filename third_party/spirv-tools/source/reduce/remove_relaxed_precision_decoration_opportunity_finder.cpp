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

#include "source/reduce/remove_relaxed_precision_decoration_opportunity_finder.h"

#include "source/reduce/remove_instruction_reduction_opportunity.h"

namespace spvtools {
namespace reduce {

std::vector<std::unique_ptr<ReductionOpportunity>>
RemoveRelaxedPrecisionDecorationOpportunityFinder::GetAvailableOpportunities(
    opt::IRContext* context) const {
  std::vector<std::unique_ptr<ReductionOpportunity>> result;

  // Consider all annotation instructions
  for (auto& inst : context->module()->annotations()) {
    // We are interested in removing instructions of the form:
    //   SpvOpDecorate %id RelaxedPrecision
    // and
    //   SpvOpMemberDecorate %id member RelaxedPrecision
    if ((inst.opcode() == SpvOpDecorate &&
         inst.GetSingleWordInOperand(1) == SpvDecorationRelaxedPrecision) ||
        (inst.opcode() == SpvOpMemberDecorate &&
         inst.GetSingleWordInOperand(2) == SpvDecorationRelaxedPrecision)) {
      result.push_back(
          MakeUnique<RemoveInstructionReductionOpportunity>(&inst));
    }
  }
  return result;
}

std::string RemoveRelaxedPrecisionDecorationOpportunityFinder::GetName() const {
  return "RemoveRelaxedPrecisionDecorationOpportunityFinder";
}

}  // namespace reduce
}  // namespace spvtools
