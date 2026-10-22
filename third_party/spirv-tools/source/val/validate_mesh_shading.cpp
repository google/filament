// Copyright (c) 2022 The Khronos Group Inc.
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

// Validates ray query instructions from SPV_KHR_ray_query

#include <string>

#include "source/val/instruction.h"
#include "source/val/validate.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {

bool IsInterfaceVariable(ValidationState_t& _, const Instruction* inst,
                         spv::ExecutionModel model) {
  bool foundInterface = false;
  for (auto entry_point : _.entry_points()) {
    const auto* models = _.GetExecutionModels(entry_point);
    if (models->find(model) == models->end()) return false;
    for (const auto& desc : _.entry_point_descriptions(entry_point)) {
      for (auto interface : desc.interfaces) {
        if (inst->id() == interface) {
          foundInterface = true;
          break;
        }
      }
    }
  }
  return foundInterface;
}

spv_result_t ValidateEmitMeshTasks(ValidationState_t& _,
                                   const Instruction* inst) {
  _.function(inst->function()->id())
      ->RegisterExecutionModelLimitation([](spv::ExecutionModel model,
                                            std::string* message) {
        if (model != spv::ExecutionModel::TaskEXT) {
          if (message) {
            *message = "OpEmitMeshTasksEXT requires TaskEXT execution model";
          }
          return false;
        }
        return true;
      });

  const uint32_t group_count_x = _.GetOperandTypeId(inst, 0);
  if (!_.IsUnsignedIntScalarType(group_count_x) ||
      _.GetBitWidth(group_count_x) != 32) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Group Count X must be a 32-bit unsigned int scalar";
  }

  const uint32_t group_count_y = _.GetOperandTypeId(inst, 1);
  if (!_.IsUnsignedIntScalarType(group_count_y) ||
      _.GetBitWidth(group_count_y) != 32) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Group Count Y must be a 32-bit unsigned int scalar";
  }

  const uint32_t group_count_z = _.GetOperandTypeId(inst, 2);
  if (!_.IsUnsignedIntScalarType(group_count_z) ||
      _.GetBitWidth(group_count_z) != 32) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Group Count Z must be a 32-bit unsigned int scalar";
  }

  if (inst->operands().size() == 4) {
    const auto payload = _.FindDef(inst->GetOperandAs<uint32_t>(3));
    if (payload->opcode() != spv::Op::OpVariable) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Payload must be the result of a OpVariable";
    }
    if (payload->GetOperandAs<spv::StorageClass>(2) !=
        spv::StorageClass::TaskPayloadWorkgroupEXT) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Payload OpVariable must have a storage class of "
                "TaskPayloadWorkgroupEXT";
    }
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateSetMeshOutputs(ValidationState_t& _,
                                    const Instruction* inst) {
  _.function(inst->function()->id())
      ->RegisterExecutionModelLimitation([](spv::ExecutionModel model,
                                            std::string* message) {
        if (model != spv::ExecutionModel::MeshEXT) {
          if (message) {
            *message = "OpSetMeshOutputsEXT requires MeshEXT execution model";
          }
          return false;
        }
        return true;
      });

  const uint32_t vertex_count = _.GetOperandTypeId(inst, 0);
  if (!_.IsUnsignedIntScalarType(vertex_count) ||
      _.GetBitWidth(vertex_count) != 32) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Vertex Count must be a 32-bit unsigned int scalar";
  }

  const uint32_t primitive_count = _.GetOperandTypeId(inst, 1);
  if (!_.IsUnsignedIntScalarType(primitive_count) ||
      _.GetBitWidth(primitive_count) != 32) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Primitive Count must be a 32-bit unsigned int scalar";
  }

  // Will only validate if constants are used (or spec constant frozen)
  uint64_t vertex_count_value = 0;
  if (_.EvalConstantValUint64(inst->GetOperandAs<uint32_t>(0),
                              &vertex_count_value)) {
    _.function(inst->function()->id())
        ->RegisterLimitation(
            [vertex_count_value](const ValidationState_t& state,
                                 const Function* entry_point,
                                 std::string* message) {
              const uint32_t output_vertices =
                  state.GetOutputVertices(entry_point->id());
              if (vertex_count_value > output_vertices) {
                *message =
                    "OpSetMeshOutputsEXT Vertex Count (" +
                    std::to_string(vertex_count_value) +
                    ") is larger than the OutputVertices in OpExecutionMode (" +
                    std::to_string(output_vertices) + ").";
                return false;
              }
              return true;
            });
  }
  uint64_t primitive_count_value = 0;
  if (_.EvalConstantValUint64(inst->GetOperandAs<uint32_t>(1),
                              &primitive_count_value)) {
    _.function(inst->function()->id())
        ->RegisterLimitation(
            [primitive_count_value](const ValidationState_t& state,
                                    const Function* entry_point,
                                    std::string* message) {
              const uint32_t output_primitives =
                  state.GetOutputPrimitivesEXT(entry_point->id());
              if (primitive_count_value > output_primitives) {
                *message = "OpSetMeshOutputsEXT Primitive Count (" +
                           std::to_string(primitive_count_value) +
                           ") is larger than the OutputPrimitivesEXT in "
                           "OpExecutionMode (" +
                           std::to_string(output_primitives) + ").";
                return false;
              }
              return true;
            });
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateMeshVariable(ValidationState_t& _,
                                  const Instruction* inst) {
  if (!_.HasCapability(spv::Capability::MeshShadingEXT)) {
    return SPV_SUCCESS;
  }
  bool is_mesh_interface_var =
      IsInterfaceVariable(_, inst, spv::ExecutionModel::MeshEXT);
  bool is_frag_interface_var =
      IsInterfaceVariable(_, inst, spv::ExecutionModel::Fragment);

  const spv::StorageClass storage_class =
      inst->GetOperandAs<spv::StorageClass>(2);
  bool storage_output = (storage_class == spv::StorageClass::Output);
  bool storage_input = (storage_class == spv::StorageClass::Input);

  if (_.HasDecoration(inst->id(), spv::Decoration::PerPrimitiveEXT)) {
    if (is_frag_interface_var && !storage_input) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "PerPrimitiveEXT decoration must be applied only to "
                "variables in the Input Storage Class in the Fragment "
                "Execution Model.";
    }

    if (is_mesh_interface_var && !storage_output) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << _.VkErrorID(4336)
             << "PerPrimitiveEXT decoration must be applied only to "
                "variables in the Output Storage Class in the "
                "Storage Class in the MeshEXT Execution Model.";
    }
  }

  // This only applies to user interface variables, not built-ins (they
  // are validated with the rest of the builtin)
  if (is_mesh_interface_var && storage_output &&
      !_.HasDecoration(inst->id(), spv::Decoration::BuiltIn)) {
    const Instruction* pointer_inst = _.FindDef(inst->type_id());
    if (pointer_inst->opcode() == spv::Op::OpTypePointer) {
      if (!_.IsArrayType(pointer_inst->word(3))) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "In the MeshEXT Execution Mode, all Output Variables "
                  "must contain an Array.";
      }
    }
  }

  return SPV_SUCCESS;
}

spv_result_t MeshShadingPass(ValidationState_t& _, const Instruction* inst) {
  const spv::Op opcode = inst->opcode();
  switch (opcode) {
    case spv::Op::OpEmitMeshTasksEXT:
      return ValidateEmitMeshTasks(_, inst);
    case spv::Op::OpSetMeshOutputsEXT:
      return ValidateSetMeshOutputs(_, inst);
    case spv::Op::OpVariable:
      return ValidateMeshVariable(_, inst);
    // No validation rules (for the moment).
    case spv::Op::OpWritePackedPrimitiveIndices4x8NV:
    default:
      break;
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
