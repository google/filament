// Copyright (c) 2016 Google Inc.
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

#include "source/opt/strip_debug_info_pass.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace opt {

Pass::Status StripDebugInfoPass::Process() {
  bool modified = !context()->debugs1().empty() ||
                  !context()->debugs2().empty() ||
                  !context()->debugs3().empty();

  std::vector<Instruction*> to_kill;
  for (auto& dbg : context()->debugs1()) to_kill.push_back(&dbg);
  for (auto& dbg : context()->debugs2()) to_kill.push_back(&dbg);
  for (auto& dbg : context()->debugs3()) to_kill.push_back(&dbg);

  // OpName must come first, since they may refer to other debug instructions.
  // If they are after the instructions that refer to, then they will be killed
  // when that instruction is killed, which will lead to a double kill.
  std::sort(to_kill.begin(), to_kill.end(),
            [](Instruction* lhs, Instruction* rhs) -> bool {
              if (lhs->opcode() == SpvOpName && rhs->opcode() != SpvOpName)
                return true;
              return false;
            });

  for (auto* inst : to_kill) context()->KillInst(inst);

  context()->module()->ForEachInst([&modified](Instruction* inst) {
    modified |= !inst->dbg_line_insts().empty();
    inst->dbg_line_insts().clear();
  });

  if (!get_module()->trailing_dbg_line_info().empty()) {
    modified = true;
    get_module()->trailing_dbg_line_info().clear();
  }

  return modified ? Status::SuccessWithChange : Status::SuccessWithoutChange;
}

}  // namespace opt
}  // namespace spvtools
