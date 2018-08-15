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

#include "source/opt/function.h"

#include <ostream>
#include <sstream>

namespace spvtools {
namespace opt {

Function* Function::Clone(IRContext* ctx) const {
  Function* clone =
      new Function(std::unique_ptr<Instruction>(DefInst().Clone(ctx)));
  clone->params_.reserve(params_.size());
  ForEachParam(
      [clone, ctx](const Instruction* inst) {
        clone->AddParameter(std::unique_ptr<Instruction>(inst->Clone(ctx)));
      },
      true);

  clone->blocks_.reserve(blocks_.size());
  for (const auto& b : blocks_) {
    std::unique_ptr<BasicBlock> bb(b->Clone(ctx));
    bb->SetParent(clone);
    clone->AddBasicBlock(std::move(bb));
  }

  clone->SetFunctionEnd(std::unique_ptr<Instruction>(EndInst()->Clone(ctx)));
  return clone;
}

void Function::ForEachInst(const std::function<void(Instruction*)>& f,
                           bool run_on_debug_line_insts) {
  if (def_inst_) def_inst_->ForEachInst(f, run_on_debug_line_insts);
  for (auto& param : params_) param->ForEachInst(f, run_on_debug_line_insts);
  for (auto& bb : blocks_) bb->ForEachInst(f, run_on_debug_line_insts);
  if (end_inst_) end_inst_->ForEachInst(f, run_on_debug_line_insts);
}

void Function::ForEachInst(const std::function<void(const Instruction*)>& f,
                           bool run_on_debug_line_insts) const {
  if (def_inst_)
    static_cast<const Instruction*>(def_inst_.get())
        ->ForEachInst(f, run_on_debug_line_insts);

  for (const auto& param : params_)
    static_cast<const Instruction*>(param.get())
        ->ForEachInst(f, run_on_debug_line_insts);

  for (const auto& bb : blocks_)
    static_cast<const BasicBlock*>(bb.get())->ForEachInst(
        f, run_on_debug_line_insts);

  if (end_inst_)
    static_cast<const Instruction*>(end_inst_.get())
        ->ForEachInst(f, run_on_debug_line_insts);
}

void Function::ForEachParam(const std::function<void(const Instruction*)>& f,
                            bool run_on_debug_line_insts) const {
  for (const auto& param : params_)
    static_cast<const Instruction*>(param.get())
        ->ForEachInst(f, run_on_debug_line_insts);
}

BasicBlock* Function::InsertBasicBlockAfter(
    std::unique_ptr<BasicBlock>&& new_block, BasicBlock* position) {
  for (auto bb_iter = begin(); bb_iter != end(); ++bb_iter) {
    if (&*bb_iter == position) {
      new_block->SetParent(this);
      ++bb_iter;
      bb_iter = bb_iter.InsertBefore(std::move(new_block));
      return &*bb_iter;
    }
  }
  assert(false && "Could not find insertion point.");
  return nullptr;
}

std::ostream& operator<<(std::ostream& str, const Function& func) {
  str << func.PrettyPrint();
  return str;
}

std::string Function::PrettyPrint(uint32_t options) const {
  std::ostringstream str;
  ForEachInst([&str, options](const Instruction* inst) {
    str << inst->PrettyPrint(options);
    if (inst->opcode() != SpvOpFunctionEnd) {
      str << std::endl;
    }
  });
  return str.str();
}

}  // namespace opt
}  // namespace spvtools
