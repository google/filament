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

#include <cstdint>

#include "source/val/instruction.h"
#include "source/val/validate.h"
#include "source/val/validate_scopes.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

spv_result_t ValidateSameSignedDot(ValidationState_t& _,
                                   const Instruction* inst) {
  const uint32_t result_id = inst->type_id();
  if (!_.IsIntScalarType(result_id)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result must be an int scalar type.";
  }

  const spv::Op opcode = inst->opcode();
  const bool has_accumulator = opcode == spv::Op::OpSDotAccSat ||
                               opcode == spv::Op::OpUDotAccSat ||
                               opcode == spv::Op::OpSUDotAccSat;
  if (has_accumulator) {
    const uint32_t accumulator_type = _.GetOperandTypeId(inst, 4);
    if (accumulator_type != result_id) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Result must be the same as the Accumulator type.";
    }
  }

  if (opcode == spv::Op::OpUDot || opcode == spv::Op::OpUDotAccSat) {
    if (!_.IsIntScalarTypeWithSignedness(result_id, 0)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Result must be an unsigned int scalar type.";
    }
  }

  const uint32_t vec_1_id = _.GetOperandTypeId(inst, 2);
  const uint32_t vec_2_id = _.GetOperandTypeId(inst, 3);

  const bool is_vec_1_scalar = _.IsIntScalarType(vec_1_id, 32);
  const bool is_vec_2_scalar = _.IsIntScalarType(vec_2_id, 32);
  if (is_vec_1_scalar != is_vec_2_scalar) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "'Vector 1' and 'Vector 2' must be the same type.";
  } else if (is_vec_1_scalar && is_vec_2_scalar) {
    if (!_.HasCapability(spv::Capability::DotProductInput4x8BitPacked)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "DotProductInput4x8BitPacked capability is required to use "
                "scalar integers.";
    }

    // If both are scalar, spec doesn't say Signedness needs to match
    const uint32_t vec_1_width = _.GetBitWidth(vec_1_id);
    const uint32_t vec_2_width = _.GetBitWidth(vec_2_id);
    if (vec_1_width != 32) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Expected 'Vector 1' to be 32-bit when a scalar.";
    } else if (vec_2_width != 32) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Expected 'Vector 2' to be 32-bit when a scalar.";
    }

    // When packed, the result can be 8-bit
    const uint32_t result_width = _.GetBitWidth(result_id);
    if (result_width < 8) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Result width (" << result_width
             << ") must be greater than or equal to the packed vector width of "
                "8";
    }

    // PackedVectorFormat4x8Bit is used when the "Vector" operand are really
    // scalar
    const uint32_t packed_operand = has_accumulator ? 6 : 5;
    const bool has_packed_vec_format =
        inst->operands().size() == packed_operand;
    if (!has_packed_vec_format) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "'Vector 1' and 'Vector 2' are a 32-bit int scalar, but no "
                "Packed Vector "
                "Format was provided.";
    }
  } else {
    // both should be vectors

    if (!_.IsVectorType(vec_1_id)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Expected 'Vector 1' to be an int scalar or vector.";
    } else if (!_.IsVectorType(vec_2_id)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Expected 'Vector 2' to be an int scalar or vector.";
    }

    const uint32_t vec_1_length = _.GetDimension(vec_1_id);
    const uint32_t vec_2_length = _.GetDimension(vec_2_id);
    // If using OpTypeVectorIdEXT with a spec constant, this can be evaluated
    // when spec constants are frozen
    if (vec_1_length != 0 && vec_2_length != 0 &&
        vec_1_length != vec_2_length) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "'Vector 1' is " << vec_1_length
             << " components but 'Vector 2' is " << vec_2_length
             << " components";
    }

    const uint32_t vec_1_type = _.GetComponentType(vec_1_id);
    const uint32_t vec_2_type = _.GetComponentType(vec_2_id);
    if (!_.IsIntScalarType(vec_1_type)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Expected 'Vector 1' to be a vector of integers.";
    } else if (!_.IsIntScalarType(vec_2_type)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Expected 'Vector 2' to be a vector of integers.";
    }

    const uint32_t vec_1_width = _.GetBitWidth(vec_1_type);
    const uint32_t vec_2_width = _.GetBitWidth(vec_2_type);
    if (vec_1_width != vec_2_width) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "'Vector 1' component is " << vec_1_width
             << "-bit but 'Vector 2' component is " << vec_2_width << "-bit";
    }

    const uint32_t result_width = _.GetBitWidth(result_id);
    if (result_width < vec_1_width) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Result width (" << result_width
             << ") must be greater than or equal to the vectors width ("
             << vec_1_width << ").";
    }

    if (!_.HasCapability(spv::Capability::DotProductInputAll)) {
      // 4-wide 8-bit ints are special exception that has its own capability
      if (vec_1_length == 4 && vec_1_width == 8) {
        if (!_.HasCapability(spv::Capability::DotProductInput4x8Bit)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "DotProductInput4x8Bit or DotProductInputAll capability is "
                    "required to use 4-component vectors of 8-bit integers.";
        }
      } else {
        // provide a more helpful message what is going on if we are here
        // reporting this error
        if (_.HasCapability(spv::Capability::DotProductInput4x8BitPacked)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "DotProductInputAll capability is required use vectors. "
                    "(DotProductInput4x8BitPacked capability declared allows "
                    "for only 32-bit int scalars)";
        } else {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "DotProductInputAll capability is additionally required to "
                    "the DotProduct capability to use vectors. (It is possible "
                    "to set DotProductInput4x8BitPacked to only use 32-bit "
                    "scalars packed as a 4-wide 8-byte vector)";
        }
      }
    }

    if (opcode == spv::Op::OpUDot || opcode == spv::Op::OpUDotAccSat) {
      const bool vec_1_unsigned =
          _.IsIntScalarTypeWithSignedness(vec_1_type, 0);
      const bool vec_2_unsigned =
          _.IsIntScalarTypeWithSignedness(vec_2_type, 0);
      if (!vec_1_unsigned) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 'Vector 1' to be an vector of unsigned integers.";
      } else if (!vec_2_unsigned) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 'Vector 2' to be an vector of unsigned integers.";
      }
    } else if (opcode == spv::Op::OpSUDot || opcode == spv::Op::OpSUDotAccSat) {
      const bool vec_2_unsigned =
          _.IsIntScalarTypeWithSignedness(vec_2_type, 0);
      if (!vec_2_unsigned) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 'Vector 2' to be an vector of unsigned integers.";
      }
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateFDotMixVectors(ValidationState_t& _,
                                    const Instruction* inst, uint32_t vec_1_id,
                                    uint32_t vec_2_id, uint32_t length) {
  if (!_.IsVectorType(vec_1_id)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected 'Vector 1' to be an vector.";
  } else if (!_.IsVectorType(vec_2_id)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected 'Vector 2' to be an vector.";
  }

  // If using OpTypeVectorIdEXT with a spec constant,
  // this can be evaluated when spec constants are frozen
  const uint32_t vec_1_length = _.GetDimension(vec_1_id);
  const uint32_t vec_2_length = _.GetDimension(vec_2_id);
  if (vec_1_length != 0 && vec_1_length != length && vec_2_length != 0 &&
      vec_2_length != length) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "'Vector 1' is " << vec_1_length
           << " components and 'Vector 2' is " << vec_2_length
           << " components, but both need to be " << length << "-components";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateFDot2MixAcc32(ValidationState_t& _,
                                   const Instruction* inst) {
  const uint32_t result_id = inst->type_id();
  if (!_.IsFloatScalarType(result_id, 32)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result must be a 32-bit IEEE 754 float scalar type.";
  }

  const uint32_t vec_1_id = _.GetOperandTypeId(inst, 2);
  const uint32_t vec_2_id = _.GetOperandTypeId(inst, 3);

  if (auto error = ValidateFDotMixVectors(_, inst, vec_1_id, vec_2_id, 2))
    return error;

  const uint32_t vec_1_type = _.GetComponentType(vec_1_id);
  const uint32_t vec_2_type = _.GetComponentType(vec_2_id);
  if (!_.IsFloatScalarType(vec_1_type, 16)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected 'Vector 1' to be a vector of 16-bit floats.";
  } else if (!_.IsFloatScalarType(vec_2_type, 16)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected 'Vector 2' to be a vector of 16-bit floats.";
  }

  // Currently 16-bit floats are only BFloat or IEEE 754
  const bool is_vec_1_bfloat = _.IsBfloat16ScalarType(vec_1_type);
  const bool is_vec_2_bfloat = _.IsBfloat16ScalarType(vec_2_type);
  if (is_vec_1_bfloat != is_vec_2_bfloat) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "'Vector 1' and 'Vector 2' must be the same float encoding.";
  }

  if (is_vec_1_bfloat) {
    if (!_.HasCapability(spv::Capability::DotProductBFloat16AccVALVE)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "DotProductBFloat16AccVALVE capability is required to use "
                "BFloat16 encoded floats.";
    }
  } else {
    if (!_.HasCapability(spv::Capability::DotProductFloat16AccFloat32VALVE)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "DotProductFloat16AccFloat32VALVE capability is required to "
                "use "
                "IEEE 754 encoded 16-bit floats.";
    }
  }

  const uint32_t accumulator_type = _.GetOperandTypeId(inst, 4);
  if (accumulator_type != result_id) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Accumulator Type must be the same as the Result Type.";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateFDot2MixAcc16(ValidationState_t& _,
                                   const Instruction* inst) {
  const uint32_t vec_1_id = _.GetOperandTypeId(inst, 2);
  const uint32_t vec_2_id = _.GetOperandTypeId(inst, 3);

  if (auto error = ValidateFDotMixVectors(_, inst, vec_1_id, vec_2_id, 2))
    return error;

  const uint32_t vec_1_type = _.GetComponentType(vec_1_id);
  const uint32_t vec_2_type = _.GetComponentType(vec_2_id);
  if (!_.IsFloatScalarType(vec_1_type, 16)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected 'Vector 1' to be a vector of 16-bit floats.";
  } else if (!_.IsFloatScalarType(vec_2_type, 16)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected 'Vector 2' to be a vector of 16-bit floats.";
  }

  // Currently 16-bit floats are only BFloat or IEEE 754
  const bool is_vec_1_bfloat = _.IsBfloat16ScalarType(vec_1_type);
  const bool is_vec_2_bfloat = _.IsBfloat16ScalarType(vec_2_type);
  if (is_vec_1_bfloat != is_vec_2_bfloat) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "'Vector 1' and 'Vector 2' must be the same float encoding.";
  }

  if (is_vec_1_bfloat) {
    if (!_.HasCapability(spv::Capability::DotProductBFloat16AccVALVE)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "DotProductBFloat16AccVALVE capability is required to use "
                "BFloat16 encoded floats.";
    }
  } else {
    if (!_.HasCapability(spv::Capability::DotProductFloat16AccFloat16VALVE)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "DotProductFloat16AccFloat16VALVE capability is required to "
                "use "
                "IEEE 754 encoded 16-bit floats.";
    }
  }

  const uint32_t result_id = inst->type_id();
  if (!_.IsFloatScalarType(result_id, 16)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result must be a 16-bit float scalar type.";
  }

  const bool is_result_bfloat = _.IsBfloat16ScalarType(result_id);
  if (is_result_bfloat != is_vec_1_bfloat) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result must have the same float encoding as 'Vector 1' and "
              "'Vector 2'.";
  }

  const uint32_t accumulator_type = _.GetOperandTypeId(inst, 4);
  if (accumulator_type != result_id) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Accumulator Type must be the same as the Result Type.";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateFDot4MixAcc32(ValidationState_t& _,
                                   const Instruction* inst) {
  const uint32_t result_id = inst->type_id();
  if (!_.IsFloatScalarType(result_id, 32)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result must be a 32-bit IEEE 754 float scalar type.";
  }

  const uint32_t vec_1_id = _.GetOperandTypeId(inst, 2);
  const uint32_t vec_2_id = _.GetOperandTypeId(inst, 3);

  if (auto error = ValidateFDotMixVectors(_, inst, vec_1_id, vec_2_id, 4))
    return error;

  // Currently 8-bit floats are only Float8E4M3/Float8E5M2
  const uint32_t vec_1_type = _.GetComponentType(vec_1_id);
  const uint32_t vec_2_type = _.GetComponentType(vec_2_id);
  if (!_.IsFloatScalarType(vec_1_type, 8)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected 'Vector 1' to be a vector of 8-bit floats.";
  } else if (!_.IsFloatScalarType(vec_2_type, 8)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected 'Vector 2' to be a vector of 8-bit floats.";
  }

  const uint32_t accumulator_type = _.GetOperandTypeId(inst, 4);
  if (accumulator_type != result_id) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Accumulator Type must be the same as the Result Type.";
  }

  return SPV_SUCCESS;
}

}  // namespace

spv_result_t DotProductPass(ValidationState_t& _, const Instruction* inst) {
  const spv::Op opcode = inst->opcode();

  switch (opcode) {
    case spv::Op::OpSDot:
    case spv::Op::OpUDot:
    case spv::Op::OpSUDot:
    case spv::Op::OpSDotAccSat:
    case spv::Op::OpUDotAccSat:
    case spv::Op::OpSUDotAccSat:
      return ValidateSameSignedDot(_, inst);
    // Tried combining these to a single validate function, but they are less
    // similar than appeared at first glance
    case spv::Op::OpFDot2MixAcc32VALVE:
      return ValidateFDot2MixAcc32(_, inst);
    case spv::Op::OpFDot2MixAcc16VALVE:
      return ValidateFDot2MixAcc16(_, inst);
    case spv::Op::OpFDot4MixAcc32VALVE:
      return ValidateFDot4MixAcc32(_, inst);
    default:
      break;
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
