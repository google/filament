// Copyright (c) 2022-2025 Arm Ltd.
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

#include "source/opt/graph.h"

namespace spvtools {
namespace opt {

Graph* Graph::Clone(IRContext* ctx) const {
  Graph* clone = new Graph(std::unique_ptr<Instruction>(DefInst().Clone(ctx)));

  clone->inputs_.reserve(inputs_.size());
  for (const auto& i : inputs()) {
    clone->AddInput(std::unique_ptr<Instruction>(i->Clone(ctx)));
  }

  clone->insts_.reserve(insts_.size());
  for (const auto& i : instructions()) {
    clone->AddInstruction(std::unique_ptr<Instruction>(i->Clone(ctx)));
  }

  clone->outputs_.reserve(outputs_.size());
  for (const auto& i : outputs()) {
    clone->AddOutput(std::unique_ptr<Instruction>(i->Clone(ctx)));
  }

  clone->SetGraphEnd(std::unique_ptr<Instruction>(EndInst()->Clone(ctx)));

  return clone;
}

void Graph::ForEachInst(const std::function<void(Instruction*)>& f,
                        bool run_on_debug_line_insts,
                        bool run_on_non_semantic_insts) {
  (void)run_on_debug_line_insts;
  (void)run_on_non_semantic_insts;

  f(def_inst_.get());

  for (auto& inst : inputs_) {
    f(inst.get());
  }

  for (auto& inst : insts_) {
    f(inst.get());
  }

  for (auto& inst : outputs_) {
    f(inst.get());
  }

  f(end_inst_.get());
}

void Graph::ForEachInst(const std::function<void(const Instruction*)>& f,
                        bool run_on_debug_line_insts,
                        bool run_on_non_semantic_insts) const {
  (void)run_on_debug_line_insts;
  (void)run_on_non_semantic_insts;

  f(def_inst_.get());

  for (auto& inst : inputs_) {
    f(inst.get());
  }

  for (auto& inst : insts_) {
    f(inst.get());
  }

  for (auto& inst : outputs_) {
    f(inst.get());
  }

  f(end_inst_.get());
}

}  // namespace opt
}  // namespace spvtools
