// Copyright (c) 2026 LunarG Inc.
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

// Validates correctness of Pipe SPIR-V instructions.

#include "source/val/instruction.h"
#include "source/val/validate.h"
#include "source/val/validate_scopes.h"
#include "source/val/validation_state.h"
#include "spirv/unified1/spirv.hpp11"

namespace spvtools {
namespace val {
namespace {

enum class ValidPipeType {
  READ_ONLY,
  WRITE_ONLY,
  READ_OR_WRITE,  // still excludes Read AND Write
};

spv_result_t ValidatePipeType(ValidationState_t& _, const Instruction* inst,
                              uint32_t operand, ValidPipeType valid_pt) {
  const Instruction* pipe_type = _.FindDef(_.GetOperandTypeId(inst, operand));
  if (pipe_type->opcode() != spv::Op::OpTypePipe) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Pipe must be a type of OpTypePipe.";
  }

  const auto access_qualifier =
      pipe_type->GetOperandAs<spv::AccessQualifier>(1);
  if (valid_pt == ValidPipeType::READ_ONLY) {
    if (access_qualifier != spv::AccessQualifier::ReadOnly) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Pipe must have a OpTypePipe with ReadOnly access qualifier.";
    }
  } else if (valid_pt == ValidPipeType::WRITE_ONLY) {
    if (access_qualifier != spv::AccessQualifier::WriteOnly) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Pipe must have a OpTypePipe with WriteOnly access qualifier.";
    }
  } else if (valid_pt == ValidPipeType::READ_OR_WRITE) {
    if (access_qualifier != spv::AccessQualifier::ReadOnly &&
        access_qualifier != spv::AccessQualifier::WriteOnly) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Pipe must have a OpTypePipe with ReadOnly or WriteOnly access "
                "qualifier.";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidatePacketSizeAlign(ValidationState_t& _,
                                     const Instruction* inst,
                                     uint32_t size_operand,
                                     uint32_t alignment_operand) {
  const uint32_t packet_size_id = _.GetOperandTypeId(inst, size_operand);
  if (!_.IsIntScalarType(packet_size_id, 32)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Packet Size must be a 32-bit scalar integer.";
  }

  const uint32_t packet_alignment_id =
      _.GetOperandTypeId(inst, alignment_operand);
  if (!_.IsIntScalarType(packet_alignment_id, 32)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Packet Alignment must be a 32-bit scalar integer.";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateReadWritePipe(ValidationState_t& _,
                                   const Instruction* inst) {
  const uint32_t result_type = inst->type_id();
  if (!_.IsIntScalarType(result_type, 32)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result Type must be a 32-bit int scalar.";
  }

  if (inst->opcode() == spv::Op::OpReadPipe) {
    if (auto error = ValidatePipeType(_, inst, 2, ValidPipeType::READ_ONLY))
      return error;
  } else if (inst->opcode() == spv::Op::OpWritePipe) {
    if (auto error = ValidatePipeType(_, inst, 2, ValidPipeType::WRITE_ONLY))
      return error;
  }

  const Instruction* pointer_type = _.FindDef(_.GetOperandTypeId(inst, 3));
  if (pointer_type->opcode() != spv::Op::OpTypePointer) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Pointer must be a type of OpTypePointer.";
  }
  if (pointer_type->GetOperandAs<spv::StorageClass>(1) !=
      spv::StorageClass::Generic) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Pointer must be a OpTypePointer with a Generic storage class.";
  }

  if (auto error = ValidatePacketSizeAlign(_, inst, 4, 5)) return error;

  return SPV_SUCCESS;
}

spv_result_t ValidateReservedReadWritePipe(ValidationState_t& _,
                                           const Instruction* inst) {
  const uint32_t result_type = inst->type_id();
  if (!_.IsIntScalarType(result_type, 32)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result Type must be a 32-bit int scalar.";
  }

  if (inst->opcode() == spv::Op::OpReservedReadPipe) {
    if (auto error = ValidatePipeType(_, inst, 2, ValidPipeType::READ_ONLY))
      return error;
  } else if (inst->opcode() == spv::Op::OpReservedWritePipe) {
    if (auto error = ValidatePipeType(_, inst, 2, ValidPipeType::WRITE_ONLY))
      return error;
  }

  const Instruction* reserve_id = _.FindDef(_.GetOperandTypeId(inst, 3));
  if (reserve_id->opcode() != spv::Op::OpTypeReserveId) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Reserve Id type must be OpTypeReserveId.";
  }

  const uint32_t index_id = _.GetOperandTypeId(inst, 4);
  if (!_.IsIntScalarType(index_id, 32)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Index must be a 32-bit scalar integer.";
  }

  const Instruction* pointer_type = _.FindDef(_.GetOperandTypeId(inst, 5));
  if (pointer_type->opcode() != spv::Op::OpTypePointer) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Pointer must be a type of OpTypePointer.";
  }
  if (pointer_type->GetOperandAs<spv::StorageClass>(1) !=
      spv::StorageClass::Generic) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Pointer must be a OpTypePointer with a Generic storage class.";
  }

  if (auto error = ValidatePacketSizeAlign(_, inst, 6, 7)) return error;

  return SPV_SUCCESS;
}

spv_result_t ValidateReservePackets(ValidationState_t& _,
                                    const Instruction* inst) {
  const Instruction* result_type = _.FindDef(inst->type_id());
  if (result_type->opcode() != spv::Op::OpTypeReserveId) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result Type must be OpTypeReserveId.";
  }

  if (inst->opcode() == spv::Op::OpReserveReadPipePackets) {
    if (auto error = ValidatePipeType(_, inst, 2, ValidPipeType::READ_ONLY))
      return error;
  } else if (inst->opcode() == spv::Op::OpReserveWritePipePackets) {
    if (auto error = ValidatePipeType(_, inst, 2, ValidPipeType::WRITE_ONLY))
      return error;
  }

  const uint32_t num_packets_id = _.GetOperandTypeId(inst, 3);
  if (!_.IsIntScalarType(num_packets_id, 32)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Num Packets must be a 32-bit scalar integer.";
  }

  if (auto error = ValidatePacketSizeAlign(_, inst, 4, 5)) return error;

  return SPV_SUCCESS;
}

spv_result_t ValidateGroupReservePackets(ValidationState_t& _,
                                         const Instruction* inst) {
  const Instruction* result_type = _.FindDef(inst->type_id());
  if (result_type->opcode() != spv::Op::OpTypeReserveId) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result Type must be OpTypeReserveId.";
  }

  if (inst->opcode() == spv::Op::OpGroupReserveReadPipePackets) {
    if (auto error = ValidatePipeType(_, inst, 3, ValidPipeType::READ_ONLY))
      return error;
  } else if (inst->opcode() == spv::Op::OpGroupReserveWritePipePackets) {
    if (auto error = ValidatePipeType(_, inst, 3, ValidPipeType::WRITE_ONLY))
      return error;
  }

  const uint32_t num_packets_id = _.GetOperandTypeId(inst, 4);
  if (!_.IsIntScalarType(num_packets_id, 32)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Num Packets must be a 32-bit scalar integer.";
  }

  if (auto error = ValidatePacketSizeAlign(_, inst, 5, 6)) return error;

  return SPV_SUCCESS;
}

spv_result_t ValidateCommitPipe(ValidationState_t& _, const Instruction* inst) {
  if (inst->opcode() == spv::Op::OpCommitReadPipe) {
    if (auto error = ValidatePipeType(_, inst, 0, ValidPipeType::READ_ONLY))
      return error;
  } else if (inst->opcode() == spv::Op::OpCommitWritePipe) {
    if (auto error = ValidatePipeType(_, inst, 0, ValidPipeType::WRITE_ONLY))
      return error;
  }

  const Instruction* reserve_id = _.FindDef(_.GetOperandTypeId(inst, 1));
  if (reserve_id->opcode() != spv::Op::OpTypeReserveId) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Reserve Id type must be OpTypeReserveId.";
  }

  if (auto error = ValidatePacketSizeAlign(_, inst, 2, 3)) return error;

  return SPV_SUCCESS;
}

spv_result_t ValidateGroupCommitPipe(ValidationState_t& _,
                                     const Instruction* inst) {
  if (inst->opcode() == spv::Op::OpGroupCommitReadPipe) {
    if (auto error = ValidatePipeType(_, inst, 1, ValidPipeType::READ_ONLY))
      return error;
  } else if (inst->opcode() == spv::Op::OpGroupCommitWritePipe) {
    if (auto error = ValidatePipeType(_, inst, 1, ValidPipeType::WRITE_ONLY))
      return error;
  }

  const Instruction* reserve_id = _.FindDef(_.GetOperandTypeId(inst, 2));
  if (reserve_id->opcode() != spv::Op::OpTypeReserveId) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Reserve Id type must be OpTypeReserveId.";
  }

  if (auto error = ValidatePacketSizeAlign(_, inst, 3, 4)) return error;

  return SPV_SUCCESS;
}

spv_result_t ValidatePipePacketsQuery(ValidationState_t& _,
                                      const Instruction* inst) {
  const uint32_t result_type = inst->type_id();
  if (!_.IsIntScalarType(result_type, 32)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result Type must be a 32-bit int scalar.";
  }

  if (auto error = ValidatePipeType(_, inst, 2, ValidPipeType::READ_OR_WRITE))
    return error;

  if (auto error = ValidatePacketSizeAlign(_, inst, 3, 4)) return error;

  return SPV_SUCCESS;
}

spv_result_t ValidateIsValidReserveId(ValidationState_t& _,
                                      const Instruction* inst) {
  if (!_.IsBoolScalarType(inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result Type must be a bool scalar";
  }

  const Instruction* reserve_id = _.FindDef(_.GetOperandTypeId(inst, 2));
  if (reserve_id->opcode() != spv::Op::OpTypeReserveId) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Reserve Id type must be OpTypeReserveId.";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateCreatePipeFromPipeStorage(ValidationState_t& _,
                                               const Instruction* inst) {
  const Instruction* result_type = _.FindDef(inst->type_id());
  if (result_type->opcode() != spv::Op::OpTypePipe) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result Type must be OpTypePipe.";
  }

  // TODO - Need to check OpTypeStorage is from OpConstantPipeStorage
  return SPV_SUCCESS;
}

spv_result_t ValidateConstantPipeStorage(ValidationState_t& _,
                                         const Instruction* inst) {
  const Instruction* result_type = _.FindDef(inst->type_id());
  if (result_type->opcode() != spv::Op::OpTypePipeStorage) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result Type must be OpTypePipeStorage.";
  }

  // TODO - Should we validate the literal values?
  // https://gitlab.khronos.org/spirv/SPIR-V/-/issues/914
  return SPV_SUCCESS;
}

}  // namespace

// Validates correctness of pipe instructions.
spv_result_t PipePass(ValidationState_t& _, const Instruction* inst) {
  switch (inst->opcode()) {
    case spv::Op::OpReadPipe:
    case spv::Op::OpWritePipe:
      return ValidateReadWritePipe(_, inst);
    case spv::Op::OpReservedReadPipe:
    case spv::Op::OpReservedWritePipe:
      return ValidateReservedReadWritePipe(_, inst);
    case spv::Op::OpReserveReadPipePackets:
    case spv::Op::OpReserveWritePipePackets:
      return ValidateReservePackets(_, inst);
    case spv::Op::OpGroupReserveReadPipePackets:
    case spv::Op::OpGroupReserveWritePipePackets:
      return ValidateGroupReservePackets(_, inst);
    case spv::Op::OpCommitReadPipe:
    case spv::Op::OpCommitWritePipe:
      return ValidateCommitPipe(_, inst);
    case spv::Op::OpGroupCommitReadPipe:
    case spv::Op::OpGroupCommitWritePipe:
      return ValidateGroupCommitPipe(_, inst);
    case spv::Op::OpGetNumPipePackets:
    case spv::Op::OpGetMaxPipePackets:
      return ValidatePipePacketsQuery(_, inst);
    case spv::Op::OpIsValidReserveId:
      return ValidateIsValidReserveId(_, inst);
    case spv::Op::OpCreatePipeFromPipeStorage:
      return ValidateCreatePipeFromPipeStorage(_, inst);
    case spv::Op::OpConstantPipeStorage:
      return ValidateConstantPipeStorage(_, inst);
    default:
      break;
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
