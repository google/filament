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

#include "source/reduce/change_operand_reduction_opportunity.h"

#include "source/opt/ir_context.h"

namespace spvtools {
namespace reduce {

bool ChangeOperandReductionOpportunity::PreconditionHolds() {
  // Check that the instruction still has the original operand.
  return inst_->NumOperands() > operand_index_ &&
         inst_->GetOperand(operand_index_).words[0] == original_id_ &&
         inst_->GetOperand(operand_index_).type == original_type_;
}

void ChangeOperandReductionOpportunity::Apply() {
  inst_->SetOperand(operand_index_, {new_id_});
  inst_->context()->get_def_use_mgr()->UpdateDefUse(inst_);
}

}  // namespace reduce
}  // namespace spvtools
