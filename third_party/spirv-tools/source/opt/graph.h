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

#ifndef SOURCE_OPT_GRAPH_H_
#define SOURCE_OPT_GRAPH_H_

#include "source/opt/instruction.h"

namespace spvtools {
namespace opt {

struct Graph {
  // Creates a graph instance declared by the given OpGraph instruction
  // |def_inst|.
  inline explicit Graph(std::unique_ptr<Instruction> def_inst);
  explicit Graph(const Graph& f) = delete;

  // Creates a clone of the graph in the given |context|
  //
  // The parent module will default to null and needs to be explicitly set by
  // the user.
  Graph* Clone(IRContext*) const;

  // The OpGraph instruction that begins the definition of this graph.
  Instruction& DefInst() { return *def_inst_; }
  const Instruction& DefInst() const { return *def_inst_; }

  // Appends an input to this graph.
  inline void AddInput(std::unique_ptr<Instruction> inst);

  // Appends an instruction to this graph.
  inline void AddInstruction(std::unique_ptr<Instruction> inst);

  // Appends an output to this graph.
  inline void AddOutput(std::unique_ptr<Instruction> inst);

  // Saves the given graph end instruction.
  void SetGraphEnd(std::unique_ptr<Instruction> end_inst);

  // Returns the given graph end instruction.
  inline Instruction* EndInst() { return end_inst_.get(); }
  inline const Instruction* EndInst() const { return end_inst_.get(); }

  // Returns graph's id
  inline uint32_t result_id() const { return def_inst_->result_id(); }

  // Returns graph's return type id
  inline uint32_t type_id() const { return def_inst_->type_id(); }

  // Return a read-only reference to the instructions that define the body of
  // the graph.
  const std::vector<std::unique_ptr<Instruction>>& instructions() const {
    return insts_;
  }

  // Return a read-only reference to the instructions that define the inputs
  // of the graph.
  const std::vector<std::unique_ptr<Instruction>>& inputs() const {
    return inputs_;
  }

  // Return a read-only reference to the instructions that define the outputs
  // of the graph.
  const std::vector<std::unique_ptr<Instruction>>& outputs() const {
    return outputs_;
  }

  // Runs the given function |f| on instructions in this graph, in order,
  // and optionally on debug line instructions that might precede them and
  // non-semantic instructions that succceed the function.
  void ForEachInst(const std::function<void(Instruction*)>& f,
                   bool run_on_debug_line_insts = false,
                   bool run_on_non_semantic_insts = false);
  void ForEachInst(const std::function<void(const Instruction*)>& f,
                   bool run_on_debug_line_insts = false,
                   bool run_on_non_semantic_insts = false) const;

 private:
  // The OpGraph instruction that begins the definition of this graph.
  std::unique_ptr<Instruction> def_inst_;
  // All inputs to this graph.
  std::vector<std::unique_ptr<Instruction>> inputs_;
  // All instructions describing this graph
  std::vector<std::unique_ptr<Instruction>> insts_;
  // All outputs of this graph.
  std::vector<std::unique_ptr<Instruction>> outputs_;
  // The OpGraphEnd instruction.
  std::unique_ptr<Instruction> end_inst_;
};

inline Graph::Graph(std::unique_ptr<Instruction> def_inst)
    : def_inst_(std::move(def_inst)) {}

inline void Graph::AddInput(std::unique_ptr<Instruction> inst) {
  inputs_.emplace_back(std::move(inst));
}

inline void Graph::AddInstruction(std::unique_ptr<Instruction> inst) {
  insts_.emplace_back(std::move(inst));
}

inline void Graph::AddOutput(std::unique_ptr<Instruction> inst) {
  outputs_.emplace_back(std::move(inst));
}

inline void Graph::SetGraphEnd(std::unique_ptr<Instruction> end_inst) {
  end_inst_ = std::move(end_inst);
}

}  // namespace opt
}  // namespace spvtools

#endif  // SOURCE_OPT_GRAPH_H_
