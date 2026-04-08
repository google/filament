// Copyright (c) 2023-2025 Arm Ltd.
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

// Validates correctness of graph instructions.

#include <deque>

#include "source/opcode.h"
#include "source/val/validate.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

bool IsTensorArray(ValidationState_t& _, uint32_t id) {
  auto def = _.FindDef(id);
  if (!def || (def->opcode() != spv::Op::OpTypeArray &&
               def->opcode() != spv::Op::OpTypeRuntimeArray)) {
    return false;
  }
  auto tdef = _.FindDef(def->word(2));
  if (!tdef || tdef->opcode() != spv::Op::OpTypeTensorARM) {
    return false;
  }
  return true;
}

bool IsGraphInterfaceType(ValidationState_t& _, uint32_t id) {
  return _.IsTensorType(id) || IsTensorArray(_, id);
}

bool IsGraph(ValidationState_t& _, uint32_t id) {
  auto def = _.FindDef(id);
  if (!def || def->opcode() != spv::Op::OpGraphARM) {
    return false;
  }
  return true;
}

bool IsGraphType(ValidationState_t& _, uint32_t id) {
  auto def = _.FindDef(id);
  if (!def || def->opcode() != spv::Op::OpTypeGraphARM) {
    return false;
  }
  return true;
}

const uint32_t kGraphTypeIOStartWord = 3;

uint32_t GraphTypeInstNumIO(const Instruction* inst) {
  return static_cast<uint32_t>(inst->words().size()) - kGraphTypeIOStartWord;
}

uint32_t GraphTypeInstNumInputs(const Instruction* inst) {
  return inst->word(2);
}

uint32_t GraphTypeInstNumOutputs(const Instruction* inst) {
  return GraphTypeInstNumIO(inst) - GraphTypeInstNumInputs(inst);
}

uint32_t GraphTypeInstGetOutputAtIndex(const Instruction* inst,
                                       uint64_t index) {
  return inst->word(kGraphTypeIOStartWord + GraphTypeInstNumInputs(inst) +
                    static_cast<uint32_t>(index));
}

uint32_t GraphTypeInstGetInputAtIndex(const Instruction* inst, uint64_t index) {
  return inst->word(kGraphTypeIOStartWord + static_cast<uint32_t>(index));
}

spv_result_t ValidateGraphType(ValidationState_t& _, const Instruction* inst) {
  // Check there are at least NumInputs types
  uint32_t NumInputs = GraphTypeInstNumInputs(inst);
  size_t NumIOTypes = GraphTypeInstNumIO(inst);
  if (NumIOTypes < NumInputs) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << NumIOTypes << " I/O types were provided but the graph has "
           << NumInputs << " inputs.";
  }

  // Check there is at least one output
  if (NumIOTypes == NumInputs) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "A graph type must have at least one output.";
  }

  // Check all I/O types are graph interface type
  for (unsigned i = kGraphTypeIOStartWord; i < inst->words().size(); i++) {
    auto tid = inst->word(i);
    if (!IsGraphInterfaceType(_, tid)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "I/O type " << _.getIdName(tid)
             << " is not a Graph Interface Type.";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateGraphConstant(ValidationState_t& _,
                                   const Instruction* inst) {
  // Check Result Type
  if (!_.IsTensorType(inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << spvOpcodeString(inst->opcode())
           << " must have a Result Type that is a tensor type.";
  }

  // Check the instruction is not preceded by another OpGraphConstantARM with
  // the same ID
  const uint32_t cst_id = inst->word(3);
  size_t inst_num = inst->LineNum() - 1;
  while (--inst_num) {
    auto prev_inst = &_.ordered_instructions()[inst_num];
    if (prev_inst->opcode() == spv::Op::OpGraphConstantARM) {
      const uint32_t prev_cst_id = prev_inst->word(3);
      if (prev_cst_id == cst_id) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "No two OpGraphConstantARM instructions may have the same "
                  "GraphConstantID";
      }
    }
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateGraphEntryPoint(ValidationState_t& _,
                                     const Instruction* inst) {
  // Graph must be an OpGraphARM
  uint32_t graph = inst->GetOperandAs<uint32_t>(0);
  auto graph_inst = _.FindDef(graph);
  if (!IsGraph(_, graph)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << spvOpcodeString(inst->opcode())
           << " Graph must be a OpGraphARM but found "
           << spvOpcodeString(graph_inst->opcode()) << ".";
  }

  // Check number of Interface IDs matches number of I/Os of graph
  auto graph_type_inst = _.FindDef(graph_inst->type_id());
  size_t graph_type_num_io = GraphTypeInstNumIO(graph_type_inst);
  size_t graph_entry_point_num_interface_id = inst->operands().size() - 2;
  if (graph_type_inst->opcode() != spv::Op::OpTypeGraphARM) {
    // This is invalid but we want ValidateGraph to report a clear error
    // so stop validating the graph entry point instruction
    return SPV_SUCCESS;
  }
  if (graph_type_num_io != graph_entry_point_num_interface_id) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << spvOpcodeString(inst->opcode()) << " Interface list contains "
           << graph_entry_point_num_interface_id << " IDs but Graph's type "
           << _.getIdName(graph_inst->type_id()) << " has " << graph_type_num_io
           << " inputs and outputs.";
  }

  // Check Interface IDs
  for (uint32_t i = 2; i < inst->operands().size(); i++) {
    uint32_t interface_id = inst->GetOperandAs<uint32_t>(i);
    auto interface_inst = _.FindDef(interface_id);

    // Check interface IDs come from OpVariable
    if ((interface_inst->opcode() != spv::Op::OpVariable) ||
        (interface_inst->GetOperandAs<spv::StorageClass>(2) !=
         spv::StorageClass::UniformConstant)) {
      return _.diag(SPV_ERROR_INVALID_DATA, interface_inst)
             << spvOpcodeString(inst->opcode()) << " Interface ID "
             << _.getIdName(interface_id)
             << " must come from OpVariable with UniformConstant Storage "
                "Class.";
    }

    // Check type of interface variable matches type of the corresponding graph
    // I/O
    uint32_t corresponding_graph_io_type =
        graph_type_inst->GetOperandAs<uint32_t>(i);

    uint32_t interface_ptr_type = interface_inst->type_id();
    auto interface_ptr_inst = _.FindDef(interface_ptr_type);
    auto interface_pointee_type = interface_ptr_inst->GetOperandAs<uint32_t>(2);
    if (interface_pointee_type != corresponding_graph_io_type) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << spvOpcodeString(inst->opcode()) << " Interface ID type "
             << _.getIdName(interface_pointee_type)
             << " must match the type of the corresponding graph I/O "
             << _.getIdName(corresponding_graph_io_type);
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateGraph(ValidationState_t& _, const Instruction* inst) {
  // Result Type must be an OpTypeGraphARM
  if (!IsGraphType(_, inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << spvOpcodeString(inst->opcode())
           << " Result Type must be an OpTypeGraphARM.";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateGraphInput(ValidationState_t& _, const Instruction* inst) {
  // Check type of InputIndex
  auto input_index_inst = _.FindDef(inst->GetOperandAs<uint32_t>(2));
  if (!input_index_inst ||
      !_.IsIntScalarType(input_index_inst->type_id(), 32)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << spvOpcodeString(inst->opcode())
           << " InputIndex must be a 32-bit integer.";
  }

  bool has_element_index = inst->operands().size() > 3;

  // Check type of ElementIndex
  if (has_element_index) {
    auto element_index_inst = _.FindDef(inst->GetOperandAs<uint32_t>(3));
    if (!element_index_inst ||
        !_.IsIntScalarType(element_index_inst->type_id(), 32)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << spvOpcodeString(inst->opcode())
             << " ElementIndex must be a 32-bit integer.";
    }
  }

  // Find graph definition
  size_t inst_num = inst->LineNum() - 1;
  auto graph_inst = &_.ordered_instructions()[inst_num];
  while (--inst_num) {
    graph_inst = &_.ordered_instructions()[inst_num];
    if (graph_inst->opcode() == spv::Op::OpGraphARM) {
      break;
    }
  }

  // Can the InputIndex be evaluated?
  // If not, there's nothing more we can validate here.
  uint64_t input_index;
  if (!_.EvalConstantValUint64(inst->GetOperandAs<uint32_t>(2), &input_index)) {
    return SPV_SUCCESS;
  }

  auto const graph_type_inst = _.FindDef(graph_inst->type_id());
  size_t graph_type_num_inputs = graph_type_inst->GetOperandAs<uint32_t>(1);

  // Check InputIndex is in range
  if (input_index >= graph_type_num_inputs) {
    std::string disassembly = _.Disassemble(*inst);
    return _.diag(SPV_ERROR_INVALID_DATA, nullptr)
           << "Type " << _.getIdName(graph_type_inst->id()) << " for graph "
           << _.getIdName(graph_inst->id()) << " has " << graph_type_num_inputs
           << " inputs but found an OpGraphInputARM instruction with an "
              "InputIndex that is "
           << input_index << ": " << disassembly;
  }

  uint32_t graph_type_input_type =
      GraphTypeInstGetInputAtIndex(graph_type_inst, input_index);

  if (has_element_index) {
    // Check ElementIndex is allowed
    if (!IsTensorArray(_, graph_type_input_type)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "OpGraphInputARM ElementIndex not allowed when the graph input "
                "selected by "
             << "InputIndex is not an OpTypeArray or OpTypeRuntimeArray";
    }

    // Check ElementIndex is in range if it can be evaluated and the input is a
    // fixed-sized array whose Length can be evaluated
    uint64_t element_index;
    if (_.IsArrayType(graph_type_input_type) &&
        _.EvalConstantValUint64(inst->GetOperandAs<uint32_t>(3),
                                &element_index)) {
      uint64_t array_length;
      auto graph_type_input_type_inst = _.FindDef(graph_type_input_type);
      if (_.EvalConstantValUint64(
              graph_type_input_type_inst->GetOperandAs<uint32_t>(2),
              &array_length)) {
        if (element_index >= array_length) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "OpGraphInputARM ElementIndex out of range. The type of "
                    "the graph input being accessed "
                 << _.getIdName(graph_type_input_type) << " is an array of "
                 << array_length << " elements but " << "ElementIndex is "
                 << element_index;
        }
      }
    }
  }

  // Check result type matches with graph type
  if (has_element_index) {
    uint32_t expected_type = _.GetComponentType(graph_type_input_type);
    if (inst->type_id() != expected_type) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Result Type " << _.getIdName(inst->type_id())
             << " of graph input instruction " << _.getIdName(inst->id())
             << " does not match the component type "
             << _.getIdName(expected_type) << " of input " << input_index
             << " in the graph type.";
    }
  } else {
    if (inst->type_id() != graph_type_input_type) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Result Type " << _.getIdName(inst->type_id())
             << " of graph input instruction " << _.getIdName(inst->id())
             << " does not match the type "
             << _.getIdName(graph_type_input_type) << " of input "
             << input_index << " in the graph type.";
    }
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateGraphSetOutput(ValidationState_t& _,
                                    const Instruction* inst) {
  // Check type of OutputIndex
  auto output_index_inst = _.FindDef(inst->GetOperandAs<uint32_t>(1));
  if (!output_index_inst ||
      !_.IsIntScalarType(output_index_inst->type_id(), 32)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << spvOpcodeString(inst->opcode())
           << " OutputIndex must be a 32-bit integer.";
  }

  bool has_element_index = inst->operands().size() > 2;

  // Check type of ElementIndex
  if (has_element_index) {
    auto element_index_inst = _.FindDef(inst->GetOperandAs<uint32_t>(2));
    if (!element_index_inst ||
        !_.IsIntScalarType(element_index_inst->type_id(), 32)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << spvOpcodeString(inst->opcode())
             << " ElementIndex must be a 32-bit integer.";
    }
  }

  // Find graph definition
  size_t inst_num = inst->LineNum() - 1;
  auto graph_inst = &_.ordered_instructions()[inst_num];
  while (--inst_num) {
    graph_inst = &_.ordered_instructions()[inst_num];
    if (graph_inst->opcode() == spv::Op::OpGraphARM) {
      break;
    }
  }

  // Can the OutputIndex be evaluated?
  // If not, there's nothing more we can validate here.
  uint64_t output_index;
  if (!_.EvalConstantValUint64(inst->GetOperandAs<uint32_t>(1),
                               &output_index)) {
    return SPV_SUCCESS;
  }

  // Check that the OutputIndex is valid with respect to the graph type
  auto graph_type_inst = _.FindDef(graph_inst->type_id());
  size_t graph_type_num_outputs = GraphTypeInstNumOutputs(graph_type_inst);

  if (output_index >= graph_type_num_outputs) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << spvOpcodeString(inst->opcode()) << " setting OutputIndex "
           << output_index << " but graph only has " << graph_type_num_outputs
           << " outputs.";
  }

  uint32_t graph_type_output_type =
      GraphTypeInstGetOutputAtIndex(graph_type_inst, output_index);

  if (has_element_index) {
    // Check ElementIndex is allowed
    if (!IsTensorArray(_, graph_type_output_type)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "OpGraphSetOutputARM ElementIndex not allowed when the graph "
                "output selected by "
             << "OutputIndex is not an OpTypeArray or OpTypeRuntimeArray";
    }

    // Check ElementIndex is in range if it can be evaluated and the output is a
    // fixed-sized array whose Length can be evaluated
    uint64_t element_index;
    if (_.IsArrayType(graph_type_output_type) &&
        _.EvalConstantValUint64(inst->GetOperandAs<uint32_t>(2),
                                &element_index)) {
      uint64_t array_length;
      auto graph_type_output_type_inst = _.FindDef(graph_type_output_type);
      if (_.EvalConstantValUint64(
              graph_type_output_type_inst->GetOperandAs<uint32_t>(2),
              &array_length)) {
        if (element_index >= array_length) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "OpGraphSetOutputARM ElementIndex out of range. The type "
                    "of the graph output being accessed "
                 << _.getIdName(graph_type_output_type) << " is an array of "
                 << array_length << " elements but " << "ElementIndex is "
                 << element_index;
        }
      }
    }
  }

  // Check Value's type matches with graph type
  uint32_t value = inst->GetOperandAs<uint32_t>(0);
  uint32_t value_type = _.FindDef(value)->type_id();
  if (has_element_index) {
    uint32_t expected_type = _.GetComponentType(graph_type_output_type);
    if (value_type != expected_type) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "The type " << _.getIdName(value_type)
             << " of Value provided to the graph output instruction "
             << _.getIdName(value) << " does not match the component type "
             << _.getIdName(expected_type) << " of output " << output_index
             << " in the graph type.";
    }
  } else {
    if (value_type != graph_type_output_type) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "The type " << _.getIdName(value_type)
             << " of Value provided to the graph output instruction "
             << _.getIdName(value) << " does not match the type "
             << _.getIdName(graph_type_output_type) << " of output "
             << output_index << " in the graph type.";
    }
  }
  return SPV_SUCCESS;
}

bool InputOutputInstructionsHaveDuplicateIndices(
    ValidationState_t& _, std::deque<const Instruction*>& inout_insts,
    const Instruction** first_dup) {
  std::set<std::pair<uint64_t, uint64_t>> inout_element_indices;
  for (auto const inst : inout_insts) {
    const bool is_input = inst->opcode() == spv::Op::OpGraphInputARM;
    bool has_element_index = inst->operands().size() > (is_input ? 3 : 2);
    uint64_t inout_index;
    if (!_.EvalConstantValUint64(inst->GetOperandAs<uint32_t>(is_input ? 2 : 1),
                                 &inout_index)) {
      continue;
    }
    uint64_t element_index = -1;  // -1 means no ElementIndex
    if (has_element_index) {
      if (!_.EvalConstantValUint64(
              inst->GetOperandAs<uint32_t>(is_input ? 3 : 2), &element_index)) {
        continue;
      }
    }
    auto inout_element_pair = std::make_pair(inout_index, element_index);
    auto inout_noelement_pair = std::make_pair(inout_index, -1);
    if (inout_element_indices.count(inout_element_pair) ||
        inout_element_indices.count(inout_noelement_pair)) {
      *first_dup = inst;
      return true;
    }
    inout_element_indices.insert(inout_element_pair);
  }
  return false;
}

spv_result_t ValidateGraphEnd(ValidationState_t& _, const Instruction* inst) {
  size_t end_inst_num = inst->LineNum() - 1;

  // Gather OpGraphInputARM and OpGraphSetOutputARM instructions
  std::deque<const Instruction*> graph_inputs, graph_outputs;
  size_t in_inst_num = end_inst_num;
  auto graph_inst = &_.ordered_instructions()[in_inst_num];
  while (--in_inst_num) {
    graph_inst = &_.ordered_instructions()[in_inst_num];
    if (graph_inst->opcode() == spv::Op::OpGraphInputARM) {
      graph_inputs.push_front(graph_inst);
      continue;
    }
    if (graph_inst->opcode() == spv::Op::OpGraphSetOutputARM) {
      graph_outputs.push_front(graph_inst);
      continue;
    }
    if (graph_inst->opcode() == spv::Op::OpGraphARM) {
      break;
    }
  }

  const Instruction* first_dup;

  // Check that there are no duplicate InputIndex and ElementIndex values
  if (InputOutputInstructionsHaveDuplicateIndices(_, graph_inputs,
                                                  &first_dup)) {
    return _.diag(SPV_ERROR_INVALID_DATA, first_dup)
           << "Two OpGraphInputARM instructions with the same InputIndex "
              "must not be part of the same "
           << "graph definition unless ElementIndex is present in both with "
              "different values.";
  }

  // Check that there are no duplicate OutputIndex and ElementIndex values
  if (InputOutputInstructionsHaveDuplicateIndices(_, graph_outputs,
                                                  &first_dup)) {
    return _.diag(SPV_ERROR_INVALID_DATA, first_dup)
           << "Two OpGraphSetOutputARM instructions with the same "
              "OutputIndex must not be part of the same "
           << "graph definition unless ElementIndex is present in both with "
              "different values.";
  }

  return SPV_SUCCESS;
}

}  // namespace

// Validates correctness of graph instructions.
spv_result_t GraphPass(ValidationState_t& _, const Instruction* inst) {
  switch (inst->opcode()) {
    case spv::Op::OpTypeGraphARM:
      return ValidateGraphType(_, inst);
    case spv::Op::OpGraphConstantARM:
      return ValidateGraphConstant(_, inst);
    case spv::Op::OpGraphEntryPointARM:
      return ValidateGraphEntryPoint(_, inst);
    case spv::Op::OpGraphARM:
      return ValidateGraph(_, inst);
    case spv::Op::OpGraphInputARM:
      return ValidateGraphInput(_, inst);
    case spv::Op::OpGraphSetOutputARM:
      return ValidateGraphSetOutput(_, inst);
    case spv::Op::OpGraphEndARM:
      return ValidateGraphEnd(_, inst);
    default:
      break;
  }
  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
