// Copyright (c) 2018 Google LLC.
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

#include "source/opcode.h"
#include "source/val/instruction.h"
#include "source/val/validate.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

spv_result_t ValidateConstantBool(ValidationState_t& _,
                                  const Instruction* inst) {
  auto type = _.FindDef(inst->type_id());
  if (!type || type->opcode() != spv::Op::OpTypeBool) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Op" << spvOpcodeString(inst->opcode()) << " Result Type <id> "
           << _.getIdName(inst->type_id()) << " is not a boolean type.";
  }

  return SPV_SUCCESS;
}

bool isCompositeType(const Instruction* inst) {
  bool is_tensor = inst->opcode() == spv::Op::OpTypeTensorARM;
  bool tensor_is_shaped = inst->words().size() == 5;
  return spvOpcodeIsComposite(inst->opcode()) ||
         (is_tensor && tensor_is_shaped);
}

spv_result_t ValidateConstantOperand(ValidationState_t& _,
                                     const Instruction* inst, size_t operand) {
  std::string opcode_name = std::string("Op") + spvOpcodeString(inst->opcode());

  const auto operand_id = inst->GetOperandAs<uint32_t>(operand);
  const bool inst_is_spec_constant = spvOpcodeIsSpecConstant(inst->opcode());
  const auto operand_opcode = _.GetIdOpcode(operand_id);
  const bool is_constant = spvOpcodeIsConstantOrUndef(operand_opcode);
  const bool is_spec_constant = spvOpcodeIsSpecConstant(operand_opcode);
  if (!is_constant) {
    // All operands must be constant, undef, or poison.
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opcode_name
           << " must only have constant, undef, or poison operands: <id> "
           << _.getIdName(operand_id);
  } else if (!inst_is_spec_constant && is_spec_constant) {
    // Spec constants are only allowed for spec constant opcodes.
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opcode_name << " must not have spec constant operands: <id> "
           << _.getIdName(operand_id);
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateConstantComposite(ValidationState_t& _,
                                       const Instruction* inst) {
  std::string opcode_name = std::string("Op") + spvOpcodeString(inst->opcode());

  const auto result_type = _.FindDef(inst->type_id());
  if (!result_type || !isCompositeType(result_type)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opcode_name << " Result Type <id> "
           << _.getIdName(inst->type_id()) << " is not a composite type.";
  }

  const auto constituent_count = inst->operands().size() - 2;
  switch (result_type->opcode()) {
    case spv::Op::OpTypeVector:
    case spv::Op::OpTypeVectorIdEXT: {
      uint32_t num_result_components = _.GetDimension(result_type->id());
      bool comp_is_int32 = true, comp_is_const_int32 = true;

      if (result_type->opcode() == spv::Op::OpTypeVectorIdEXT) {
        uint32_t comp_count_id = result_type->GetOperandAs<uint32_t>(2);
        std::tie(comp_is_int32, comp_is_const_int32, num_result_components) =
            _.EvalInt32IfConst(comp_count_id);
      }

      if (comp_is_const_int32 && num_result_components != constituent_count) {
        // TODO: Output ID's on diagnostic
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << opcode_name
               << " Constituent <id> count does not match "
                  "Result Type <id> "
               << _.getIdName(result_type->id()) << "s vector component count.";
      }
      const auto component_type =
          _.FindDef(result_type->GetOperandAs<uint32_t>(1));
      if (!component_type) {
        return _.diag(SPV_ERROR_INVALID_ID, result_type)
               << "Component type is not defined.";
      }
      for (size_t constituent_index = 2;
           constituent_index < inst->operands().size(); constituent_index++) {
        const auto constituent_id =
            inst->GetOperandAs<uint32_t>(constituent_index);
        const auto constituent = _.FindDef(constituent_id);
        const auto constituent_result_type = _.FindDef(constituent->type_id());
        if (!constituent_result_type ||
            component_type->id() != constituent_result_type->id()) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << opcode_name << " Constituent <id> "
                 << _.getIdName(constituent_id)
                 << "s type does not match Result Type <id> "
                 << _.getIdName(result_type->id()) << "s vector element type.";
        }
      }
    } break;
    case spv::Op::OpTypeMatrix: {
      const auto column_count = result_type->GetOperandAs<uint32_t>(2);
      if (column_count != constituent_count) {
        // TODO: Output ID's on diagnostic
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << opcode_name
               << " Constituent <id> count does not match "
                  "Result Type <id> "
               << _.getIdName(result_type->id()) << "s matrix column count.";
      }

      const auto column_type =
          _.FindDef(result_type->GetOperandAs<uint32_t>(1));
      if (!column_type) {
        return _.diag(SPV_ERROR_INVALID_ID, result_type)
               << "Column type is not defined.";
      }
      const auto component_count = column_type->GetOperandAs<uint32_t>(2);
      const auto component_type =
          _.FindDef(column_type->GetOperandAs<uint32_t>(1));
      if (!component_type) {
        return _.diag(SPV_ERROR_INVALID_ID, column_type)
               << "Component type is not defined.";
      }

      for (size_t constituent_index = 2;
           constituent_index < inst->operands().size(); constituent_index++) {
        const auto constituent_id =
            inst->GetOperandAs<uint32_t>(constituent_index);
        const auto constituent = _.FindDef(constituent_id);
        const auto vector = _.FindDef(constituent->type_id());
        if (!vector) {
          return _.diag(SPV_ERROR_INVALID_ID, constituent)
                 << "Result type is not defined.";
        }
        if (column_type->opcode() != vector->opcode()) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << opcode_name << " Constituent <id> "
                 << _.getIdName(constituent_id)
                 << " type does not match Result Type <id> "
                 << _.getIdName(result_type->id()) << "s matrix column type.";
        }
        const auto vector_component_type =
            _.FindDef(vector->GetOperandAs<uint32_t>(1));
        if (component_type->id() != vector_component_type->id()) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << opcode_name << " Constituent <id> "
                 << _.getIdName(constituent_id)
                 << " component type does not match Result Type <id> "
                 << _.getIdName(result_type->id())
                 << "s matrix column component type.";
        }
        if (component_count != vector->GetOperandAs<uint32_t>(2)) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << opcode_name << " Constituent <id> "
                 << _.getIdName(constituent_id)
                 << " vector component count does not match Result Type <id> "
                 << _.getIdName(result_type->id())
                 << "s vector component count.";
        }
      }
    } break;
    case spv::Op::OpTypeArray: {
      auto element_type = _.FindDef(result_type->GetOperandAs<uint32_t>(1));
      if (!element_type) {
        return _.diag(SPV_ERROR_INVALID_ID, result_type)
               << "Element type is not defined.";
      }
      const auto length = _.FindDef(result_type->GetOperandAs<uint32_t>(2));
      if (!length) {
        return _.diag(SPV_ERROR_INVALID_ID, result_type)
               << "Length is not defined.";
      }
      bool is_int32;
      bool is_const;
      uint32_t value;
      std::tie(is_int32, is_const, value) = _.EvalInt32IfConst(length->id());
      if (is_int32 && is_const && value != constituent_count) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << opcode_name
               << " Constituent count does not match "
                  "Result Type <id> "
               << _.getIdName(result_type->id()) << "s array length.";
      }
      for (size_t constituent_index = 2;
           constituent_index < inst->operands().size(); constituent_index++) {
        const auto constituent_id =
            inst->GetOperandAs<uint32_t>(constituent_index);
        const auto constituent = _.FindDef(constituent_id);
        const auto constituent_type = _.FindDef(constituent->type_id());
        if (!constituent_type) {
          return _.diag(SPV_ERROR_INVALID_ID, constituent)
                 << "Result type is not defined.";
        }
        if (element_type->id() != constituent_type->id()) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << opcode_name << " Constituent <id> "
                 << _.getIdName(constituent_id)
                 << "s type does not match Result Type <id> "
                 << _.getIdName(result_type->id()) << "s array element type.";
        }
      }
    } break;
    case spv::Op::OpTypeStruct: {
      const auto member_count = result_type->operands().size() - 1;
      if (member_count != constituent_count) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << opcode_name << " Constituent <id> "
               << _.getIdName(inst->type_id())
               << " count does not match Result Type <id> "
               << _.getIdName(result_type->id()) << "s struct member count.";
      }
      for (uint32_t constituent_index = 2, member_index = 1;
           constituent_index < inst->operands().size();
           constituent_index++, member_index++) {
        const auto constituent_id =
            inst->GetOperandAs<uint32_t>(constituent_index);
        const auto constituent = _.FindDef(constituent_id);
        const auto constituent_type = _.FindDef(constituent->type_id());
        if (!constituent_type) {
          return _.diag(SPV_ERROR_INVALID_ID, constituent)
                 << "Result type is not defined.";
        }

        const auto member_type_id =
            result_type->GetOperandAs<uint32_t>(member_index);
        const auto member_type = _.FindDef(member_type_id);
        if (!member_type || member_type->id() != constituent_type->id()) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << opcode_name << " Constituent <id> "
                 << _.getIdName(constituent_id)
                 << " type does not match the Result Type <id> "
                 << _.getIdName(result_type->id()) << "s member type.";
        }
      }
    } break;
    case spv::Op::OpTypeCooperativeMatrixKHR:
    case spv::Op::OpTypeCooperativeMatrixNV: {
      if (1 != constituent_count) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << opcode_name << " Constituent <id> "
               << _.getIdName(inst->type_id()) << " count must be one.";
      }
      const auto constituent_id = inst->GetOperandAs<uint32_t>(2);
      const auto constituent = _.FindDef(constituent_id);
      const auto constituent_type = _.FindDef(constituent->type_id());
      if (!constituent_type) {
        return _.diag(SPV_ERROR_INVALID_ID, constituent)
               << "Result type is not defined.";
      }

      const auto component_type_id = result_type->GetOperandAs<uint32_t>(1);
      const auto component_type = _.FindDef(component_type_id);
      if (!component_type || component_type->id() != constituent_type->id()) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << opcode_name << " Constituent <id> "
               << _.getIdName(constituent_id)
               << " type does not match the Result Type <id> "
               << _.getIdName(result_type->id()) << "s component type.";
      }
    } break;
    case spv::Op::OpTypeTensorARM: {
      auto inst_element_type =
          _.FindDef(result_type->GetOperandAs<uint32_t>(1));
      if (!inst_element_type) {
        return _.diag(SPV_ERROR_INVALID_ID, result_type)
               << "Element type is not defined.";
      }
      const auto inst_rank = _.FindDef(result_type->GetOperandAs<uint32_t>(2));
      if (!inst_rank) {
        return _.diag(SPV_ERROR_INVALID_ID, result_type)
               << "Rank is not defined.";
      }
      const auto inst_shape = _.FindDef(result_type->GetOperandAs<uint32_t>(3));
      if (!inst_shape) {
        return _.diag(SPV_ERROR_INVALID_ID, result_type)
               << "Shape is not defined.";
      }

      uint64_t rank = 0;
      _.EvalConstantValUint64(inst_rank->id(), &rank);

      uint64_t outermost_shape = 0;
      if (_.EvalConstantValUint64(inst_shape->GetOperandAs<uint32_t>(2),
                                  &outermost_shape) &&
          (outermost_shape != constituent_count)) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << opcode_name
               << " Constituent count does not match "
                  "the shape of Result Type <id> "
               << _.getIdName(result_type->id())
               << " along its outermost dimension, " << "expected "
               << outermost_shape << " but got " << constituent_count << ".";
      }

      for (size_t constituent_index = 2;
           constituent_index < inst->operands().size(); constituent_index++) {
        const auto constituent_id =
            inst->GetOperandAs<uint32_t>(constituent_index);
        const auto constituent = _.FindDef(constituent_id);
        const auto constituent_type = _.FindDef(constituent->type_id());
        if (!constituent_type) {
          return _.diag(SPV_ERROR_INVALID_ID, constituent)
                 << "Type of Constituent " << constituent_index - 2
                 << " is not defined.";
        }

        if (rank == 0) {
          // The rank of the returned tensor constant is not known.
          // Skip rank-dependent validation.
          continue;
        }

        if (rank == 1) {
          if (inst_element_type->id() != constituent_type->id()) {
            return _.diag(SPV_ERROR_INVALID_ID, inst)
                   << opcode_name << " Constituent <id> "
                   << _.getIdName(constituent_id)
                   << " type does not match the element type of the tensor ("
                   << _.getIdName(result_type->id()) << ").";
          }
        } else {
          if (constituent_type->opcode() != spv::Op::OpTypeTensorARM) {
            return _.diag(SPV_ERROR_INVALID_ID, inst)
                   << opcode_name << " Constituent <id> "
                   << _.getIdName(constituent_id)
                   << " must be an OpTypeTensorARM.";
          }
          auto inst_constituent_element_type =
              _.FindDef(constituent_type->GetOperandAs<uint32_t>(1));
          if (!inst_constituent_element_type ||
              inst_constituent_element_type->id() != inst_element_type->id()) {
            return _.diag(SPV_ERROR_INVALID_ID, inst)
                   << opcode_name << " Constituent <id> "
                   << _.getIdName(constituent_id)
                   << " must have the same Element Type as Result Type <id> "
                   << _.getIdName(result_type->id()) << ".";
          }
          auto inst_constituent_rank =
              _.FindDef(constituent_type->GetOperandAs<uint32_t>(2));
          uint64_t constituent_rank;
          if (inst_constituent_rank &&
              _.EvalConstantValUint64(inst_constituent_rank->id(),
                                      &constituent_rank) &&
              (constituent_rank != rank - 1)) {
            return _.diag(SPV_ERROR_INVALID_ID, inst)
                   << opcode_name << " Constituent <id> "
                   << _.getIdName(constituent_id)
                   << " must have a Rank that is 1 less than the Rank of "
                      "Result Type <id> "
                   << _.getIdName(result_type->id()) << ", expected "
                   << rank - 1 << " but got " << constituent_rank << ".";
          }

          auto inst_constituent_shape =
              _.FindDef(constituent_type->GetOperandAs<uint32_t>(3));
          if (!inst_constituent_shape) {
            return _.diag(SPV_ERROR_INVALID_ID, result_type)
                   << "Shape of Constituent " << constituent_index - 2
                   << " is not defined.";
          }
          for (size_t constituent_shape_index = 2;
               constituent_shape_index <
               inst_constituent_shape->operands().size();
               constituent_shape_index++) {
            size_t shape_index = constituent_shape_index + 1;
            uint64_t constituent_shape = 0, shape = 1;
            if (_.EvalConstantValUint64(
                    inst_constituent_shape->GetOperandAs<uint32_t>(
                        constituent_shape_index),
                    &constituent_shape) &&
                _.EvalConstantValUint64(
                    inst_shape->GetOperandAs<uint32_t>(shape_index), &shape) &&
                (constituent_shape != shape)) {
              return _.diag(SPV_ERROR_INVALID_ID, inst)
                     << opcode_name << " Constituent <id> "
                     << _.getIdName(constituent_id)
                     << " must have a Shape that matches that of Result Type "
                        "<id> "
                     << _.getIdName(result_type->id())
                     << " along all inner dimensions of Result Type, expected "
                     << shape << " for dimension "
                     << constituent_shape_index - 2
                     << " of Constituent but got " << constituent_shape << ".";
            }
          }
        }
      }
    } break;
    default:
      break;
  }

  for (size_t i = 2; i < inst->operands().size(); i++) {
    if (auto error = ValidateConstantOperand(_, inst, i)) {
      return error;
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateConstantCompositeReplicate(ValidationState_t& _,
                                                const Instruction* inst) {
  std::string opcode_name = std::string("Op") + spvOpcodeString(inst->opcode());

  const auto result_type = _.FindDef(inst->type_id());
  if (!result_type || !isCompositeType(result_type)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opcode_name << " Result Type <id> "
           << _.getIdName(inst->type_id()) << " is not a composite type.";
  }

  const auto constituent_id = inst->GetOperandAs<uint32_t>(2);
  const auto constituent = _.FindDef(constituent_id);
  switch (result_type->opcode()) {
    case spv::Op::OpTypeVector:
    case spv::Op::OpTypeVectorIdEXT:
    case spv::Op::OpTypeMatrix:
    case spv::Op::OpTypeArray:
    case spv::Op::OpTypeCooperativeMatrixKHR:
    case spv::Op::OpTypeCooperativeMatrixNV:
    case spv::Op::OpTypeTensorARM: {
      const auto component_type = result_type->GetOperandAs<uint32_t>(1);
      if (component_type != constituent->type_id()) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << opcode_name << " Constituent <id> "
               << _.getIdName(constituent_id)
               << "s type does not match Result Type <id> "
               << _.getIdName(result_type->id()) << "s element type.";
      }
      break;
    }
    case spv::Op::OpTypeStruct: {
      const auto member_count = result_type->operands().size() - 1;
      for (uint32_t member_index = 1; member_index <= member_count;
           member_index++) {
        const auto member_type_id =
            result_type->GetOperandAs<uint32_t>(member_index);
        if (member_type_id != constituent->type_id()) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << opcode_name << " Constituent <id> "
                 << _.getIdName(constituent_id)
                 << " type does not match the Result Type <id> "
                 << _.getIdName(result_type->id()) << "s member type.";
        }
      }
      break;
    }
    default:
      break;
  }

  return ValidateConstantOperand(_, inst, 2);
}

spv_result_t ValidateConstantSampler(ValidationState_t& _,
                                     const Instruction* inst) {
  const auto result_type = _.FindDef(inst->type_id());
  if (!result_type || result_type->opcode() != spv::Op::OpTypeSampler) {
    return _.diag(SPV_ERROR_INVALID_ID, result_type)
           << "OpConstantSampler Result Type <id> "
           << _.getIdName(inst->type_id()) << " is not a sampler type.";
  }

  return SPV_SUCCESS;
}

// True if instruction defines a type that can have a null value, as defined by
// the SPIR-V spec.  Tracks composite-type components through module to check
// nullability transitively.
bool IsTypeNullable(const std::vector<uint32_t>& instruction,
                    const ValidationState_t& _) {
  uint16_t opcode;
  uint16_t word_count;
  spvOpcodeSplit(instruction[0], &word_count, &opcode);
  switch (static_cast<spv::Op>(opcode)) {
    case spv::Op::OpTypeBool:
    case spv::Op::OpTypeInt:
    case spv::Op::OpTypeFloat:
    case spv::Op::OpTypeEvent:
    case spv::Op::OpTypeDeviceEvent:
    case spv::Op::OpTypeReserveId:
    case spv::Op::OpTypeQueue:
      return true;
    case spv::Op::OpTypeArray:
    case spv::Op::OpTypeMatrix:
    case spv::Op::OpTypeCooperativeMatrixNV:
    case spv::Op::OpTypeCooperativeMatrixKHR:
    case spv::Op::OpTypeVectorIdEXT:
    case spv::Op::OpTypeVector: {
      auto base_type = _.FindDef(instruction[2]);
      return base_type && IsTypeNullable(base_type->words(), _);
    }
    case spv::Op::OpTypeStruct: {
      for (size_t elementIndex = 2; elementIndex < instruction.size();
           ++elementIndex) {
        auto element = _.FindDef(instruction[elementIndex]);
        if (!element || !IsTypeNullable(element->words(), _)) return false;
      }
      return true;
    }
    case spv::Op::OpTypeUntypedPointerKHR:
    case spv::Op::OpTypePointer:
      if (spv::StorageClass(instruction[2]) ==
          spv::StorageClass::PhysicalStorageBuffer) {
        return false;
      }
      return true;
    case spv::Op::OpTypeTensorARM: {
      auto elem_type = _.FindDef(instruction[2]);
      return (instruction.size() > 4) && elem_type &&
             IsTypeNullable(elem_type->words(), _);
    }
    default:
      return false;
  }
}

spv_result_t ValidateConstantNull(ValidationState_t& _,
                                  const Instruction* inst) {
  const auto result_type = _.FindDef(inst->type_id());
  if (!result_type || !IsTypeNullable(result_type->words(), _)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpConstantNull Result Type <id> " << _.getIdName(inst->type_id())
           << " cannot have a null value.";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateConstantSizeOfEXT(ValidationState_t& _,
                                       const Instruction* inst) {
  const Instruction* result_type = _.FindDef(inst->type_id());
  const uint32_t bit_width = result_type->GetOperandAs<uint32_t>(1);
  // VVL will validate the SPV_EXT_shader_64bit_indexing interaction
  if (result_type->opcode() != spv::Op::OpTypeInt ||
      (bit_width != 64 && bit_width != 32)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "For OpConstantSizeOfEXT instruction, its result type "
           << "must be a 32-bit or 64-bit integer type scalar."
           << " (OpCapability Int64 is required for 64-bit)";
  }

  const uint32_t type_operand = inst->GetOperandAs<uint32_t>(2);
  if (!_.IsDescriptorType(type_operand)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "For OpConstantSizeOfEXT instruction, its Type operand <Id> "
           << _.getIdName(type_operand) << " must be a Descriptor type.";
  }
  return SPV_SUCCESS;
}

// Validates that OpSpecConstant specializes to either int or float type.
spv_result_t ValidateSpecConstant(ValidationState_t& _,
                                  const Instruction* inst) {
  // Operand 0 is the <id> of the type that we're specializing to.
  auto type_id = inst->GetOperandAs<const uint32_t>(0);
  auto type_instruction = _.FindDef(type_id);
  auto type_opcode = type_instruction->opcode();
  if (type_opcode != spv::Op::OpTypeInt &&
      type_opcode != spv::Op::OpTypeFloat) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst) << "Specialization constant "
                                                   "must be an integer or "
                                                   "floating-point number.";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateSpecConstantOp(ValidationState_t& _,
                                    const Instruction* inst) {
  const auto op = inst->GetOperandAs<spv::Op>(2);

  // The binary parser already ensures that the op is valid for *some*
  // environment.  Here we check restrictions.
  switch (op) {
    case spv::Op::OpQuantizeToF16:
      if (!_.HasCapability(spv::Capability::Shader)) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "Specialization constant operation " << spvOpcodeString(op)
               << " requires Shader capability";
      }
      break;

    case spv::Op::OpUConvert:
      if (!_.features().uconvert_spec_constant_op &&
          !_.HasCapability(spv::Capability::Kernel)) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "Prior to SPIR-V 1.4, specialization constant operation "
                  "UConvert requires Kernel capability or extension "
                  "SPV_AMD_gpu_shader_int16";
      }
      break;

    case spv::Op::OpConvertFToS:
    case spv::Op::OpConvertSToF:
    case spv::Op::OpConvertFToU:
    case spv::Op::OpConvertUToF:
    case spv::Op::OpConvertPtrToU:
    case spv::Op::OpConvertUToPtr:
    case spv::Op::OpGenericCastToPtr:
    case spv::Op::OpPtrCastToGeneric:
    case spv::Op::OpBitcast:
    case spv::Op::OpFNegate:
    case spv::Op::OpFAdd:
    case spv::Op::OpFSub:
    case spv::Op::OpFMul:
    case spv::Op::OpFDiv:
    case spv::Op::OpFRem:
    case spv::Op::OpFMod:
    case spv::Op::OpAccessChain:
    case spv::Op::OpInBoundsAccessChain:
    case spv::Op::OpPtrAccessChain:
    case spv::Op::OpInBoundsPtrAccessChain:
    case spv::Op::OpUntypedAccessChainKHR:
    case spv::Op::OpUntypedInBoundsAccessChainKHR:
    case spv::Op::OpUntypedPtrAccessChainKHR:
    case spv::Op::OpUntypedInBoundsPtrAccessChainKHR:
      if (!_.HasCapability(spv::Capability::Kernel)) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "Specialization constant operation " << spvOpcodeString(op)
               << " requires Kernel capability";
      }
      break;

    default:
      break;
  }

  // TODO(dneto): Validate result type and arguments to the various operations.
  return SPV_SUCCESS;
}

spv_result_t ValidateConstantFunctionPointerINTEL(ValidationState_t& _,
                                                  const Instruction* inst) {
  const auto result_type = _.FindDef(inst->type_id());
  // Result Type must be a pointer type
  if (result_type->opcode() != spv::Op::OpTypePointer &&
      result_type->opcode() != spv::Op::OpTypeUntypedPointerKHR) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpConstantFunctionPointerINTEL Result Type <id> "
           << _.getIdName(inst->type_id()) << " is not a pointer type";
  }

  // For typed pointers, check that pointee is a function type
  const Instruction* pointee_type = nullptr;
  if (result_type->opcode() == spv::Op::OpTypePointer) {
    pointee_type = _.FindDef(result_type->GetOperandAs<uint32_t>(2));
    if (pointee_type->opcode() != spv::Op::OpTypeFunction) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpConstantFunctionPointerINTEL Result Type <id> "
             << _.getIdName(inst->type_id())
             << " must be a pointer to function type";
    }
  }

  // Validate that the function operand refers to an OpFunction
  const uint32_t function_id = inst->GetOperandAs<uint32_t>(2);
  const auto function_inst = _.FindDef(function_id);
  if (function_inst->opcode() != spv::Op::OpFunction) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpConstantFunctionPointerINTEL Function operand <id> "
           << _.getIdName(function_id) << " is not an OpFunction";
  }

  // For typed pointers, validate that function type matches pointee type
  if (pointee_type) {
    const uint32_t function_type_id = function_inst->GetOperandAs<uint32_t>(3);
    if (function_type_id != pointee_type->id()) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpConstantFunctionPointerINTEL Function operand <id> "
             << _.getIdName(function_id)
             << " type does not match the pointer's function type";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateConstantData(ValidationState_t& _,
                                  const Instruction* inst) {
  const auto array_inst = _.FindDef(inst->type_id());
  if (array_inst->opcode() != spv::Op::OpTypeArray) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Result type must be an array.";
  }

  const auto element_type_inst =
      _.FindDef(array_inst->GetOperandAs<uint32_t>(1));
  if (!_.IsIntScalarType(element_type_inst->id())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Result type must be an array of integer scalar type.";
  }

  const uint32_t int_width = element_type_inst->word(2);
  const uint32_t data_words = static_cast<uint32_t>(inst->words().size() - 3);

  if (data_words == 0) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "There must be at least 1 literal integer (because an array of "
              "zero is not allowed).";
  }

  uint64_t array_length = 0;
  if (!_.EvalConstantValUint64(array_inst->GetOperandAs<uint32_t>(2),
                               &array_length)) {
    // The length could be a SpecConstant, will need to be frozen to validate
    return SPV_SUCCESS;
  }

  const uint32_t words_needed =
      (((int_width / 8) * static_cast<uint32_t>(array_length) + 3) & ~3) / 4;
  if (data_words != words_needed) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "contains " << data_words << " words of data, but needs to have "
           << words_needed << " words to match the array of " << array_length
           << " of " << int_width << "-bit ints.";
  }

  return SPV_SUCCESS;
}

}  // namespace

spv_result_t ConstantPass(ValidationState_t& _, const Instruction* inst) {
  switch (inst->opcode()) {
    case spv::Op::OpConstantTrue:
    case spv::Op::OpConstantFalse:
    case spv::Op::OpSpecConstantTrue:
    case spv::Op::OpSpecConstantFalse:
      if (auto error = ValidateConstantBool(_, inst)) return error;
      break;
    case spv::Op::OpConstantComposite:
    case spv::Op::OpSpecConstantComposite:
      if (auto error = ValidateConstantComposite(_, inst)) return error;
      break;
    case spv::Op::OpConstantCompositeReplicateEXT:
    case spv::Op::OpSpecConstantCompositeReplicateEXT:
      if (auto error = ValidateConstantCompositeReplicate(_, inst))
        return error;
      break;
    case spv::Op::OpConstantSampler:
      if (auto error = ValidateConstantSampler(_, inst)) return error;
      break;
    case spv::Op::OpConstantNull:
      if (auto error = ValidateConstantNull(_, inst)) return error;
      break;
    case spv::Op::OpSpecConstant:
      if (auto error = ValidateSpecConstant(_, inst)) return error;
      break;
    case spv::Op::OpSpecConstantOp:
      if (auto error = ValidateSpecConstantOp(_, inst)) return error;
      break;
    case spv::Op::OpConstantSizeOfEXT:
      if (auto error = ValidateConstantSizeOfEXT(_, inst)) return error;
      break;
    case spv::Op::OpConstantFunctionPointerINTEL:
      if (auto error = ValidateConstantFunctionPointerINTEL(_, inst))
        return error;
      break;
    case spv::Op::OpConstantDataKHR:
      if (auto error = ValidateConstantData(_, inst)) return error;
      break;
    default:
      break;
  }

  // Generally disallow creating 8- or 16-bit constants unless the full
  // capabilities are present.
  if (spvOpcodeIsConstant(inst->opcode()) &&
      _.HasCapability(spv::Capability::Shader) &&
      !_.IsPointerType(inst->type_id()) &&
      _.ContainsLimitedUseIntOrFloatType(inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Cannot form constants of 8- or 16-bit types";
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
