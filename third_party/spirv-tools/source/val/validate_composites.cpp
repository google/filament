// Copyright (c) 2017 Google Inc.
// Modifications Copyright (C) 2024 Advanced Micro Devices, Inc. All rights
// reserved.
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

// Validates correctness of composite SPIR-V instructions.

#include <climits>

#include "source/opcode.h"
#include "source/spirv_target_env.h"
#include "source/val/instruction.h"
#include "source/val/validate.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

// Returns the type of the value accessed by OpCompositeExtract or
// OpCompositeInsert instruction. The function traverses the hierarchy of
// nested data structures (structs, arrays, vectors, matrices) as directed by
// the sequence of indices in the instruction. May return error if traversal
// fails (encountered non-composite, out of bounds, no indices, nesting too
// deep).
spv_result_t GetExtractInsertValueType(ValidationState_t& _,
                                       const Instruction* inst,
                                       uint32_t* member_type) {
  const spv::Op opcode = inst->opcode();
  assert(opcode == spv::Op::OpCompositeExtract ||
         opcode == spv::Op::OpCompositeInsert);
  uint32_t word_index = opcode == spv::Op::OpCompositeExtract ? 4 : 5;
  const uint32_t num_words = static_cast<uint32_t>(inst->words().size());
  const uint32_t composite_id_index = word_index - 1;
  const uint32_t num_indices = num_words - word_index;
  const uint32_t kCompositeExtractInsertMaxNumIndices = 255;

  if (num_indices == 0) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected at least one index to Op"
           << spvOpcodeString(inst->opcode()) << ", zero found";

  } else if (num_indices > kCompositeExtractInsertMaxNumIndices) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "The number of indexes in Op" << spvOpcodeString(opcode)
           << " may not exceed " << kCompositeExtractInsertMaxNumIndices
           << ". Found " << num_indices << " indexes.";
  }

  *member_type = _.GetTypeId(inst->word(composite_id_index));
  if (*member_type == 0) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Composite to be an object of composite type";
  }

  for (; word_index < num_words; ++word_index) {
    const uint32_t component_index = inst->word(word_index);
    const Instruction* const type_inst = _.FindDef(*member_type);
    assert(type_inst);
    switch (type_inst->opcode()) {
      case spv::Op::OpTypeVector: {
        *member_type = type_inst->word(2);
        const uint32_t vector_size = type_inst->word(3);
        if (component_index >= vector_size) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Vector access is out of bounds, vector size is "
                 << vector_size << ", but access index is " << component_index;
        }
        break;
      }
      case spv::Op::OpTypeMatrix: {
        *member_type = type_inst->word(2);
        const uint32_t num_cols = type_inst->word(3);
        if (component_index >= num_cols) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Matrix access is out of bounds, matrix has " << num_cols
                 << " columns, but access index is " << component_index;
        }
        break;
      }
      case spv::Op::OpTypeArray: {
        uint64_t array_size = 0;
        auto size = _.FindDef(type_inst->word(3));
        *member_type = type_inst->word(2);
        if (spvOpcodeIsSpecConstant(size->opcode())) {
          // Cannot verify against the size of this array.
          break;
        }

        if (!_.EvalConstantValUint64(type_inst->word(3), &array_size)) {
          assert(0 && "Array type definition is corrupt");
        }
        if (component_index >= array_size) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Array access is out of bounds, array size is "
                 << array_size << ", but access index is " << component_index;
        }
        break;
      }
      case spv::Op::OpTypeRuntimeArray:
      case spv::Op::OpTypeNodePayloadArrayAMDX: {
        *member_type = type_inst->word(2);
        // Array size is unknown.
        break;
      }
      case spv::Op::OpTypeStruct: {
        const size_t num_struct_members = type_inst->words().size() - 2;
        if (component_index >= num_struct_members) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Index is out of bounds, can not find index "
                 << component_index << " in the structure <id> '"
                 << type_inst->id() << "'. This structure has "
                 << num_struct_members << " members. Largest valid index is "
                 << num_struct_members - 1 << ".";
        }
        *member_type = type_inst->word(component_index + 2);
        break;
      }
      case spv::Op::OpTypeVectorIdEXT:
      case spv::Op::OpTypeCooperativeMatrixKHR:
      case spv::Op::OpTypeCooperativeMatrixNV: {
        *member_type = type_inst->word(2);
        break;
      }
      default:
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Reached non-composite type while indexes still remain to "
                  "be traversed.";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateVectorExtractDynamic(ValidationState_t& _,
                                          const Instruction* inst) {
  const uint32_t result_type = inst->type_id();
  const spv::Op result_opcode = _.GetIdOpcode(result_type);
  if (!spvOpcodeIsScalarType(result_opcode)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Result Type to be a scalar type";
  }

  const uint32_t vector_type = _.GetOperandTypeId(inst, 2);
  const spv::Op vector_opcode = _.GetIdOpcode(vector_type);
  if (vector_opcode != spv::Op::OpTypeVector &&
      vector_opcode != spv::Op::OpTypeVectorIdEXT) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Vector type to be OpTypeVector";
  }

  if (_.GetComponentType(vector_type) != result_type) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Vector component type to be equal to Result Type";
  }

  const auto index = _.FindDef(inst->GetOperandAs<uint32_t>(3));
  if (!index || index->type_id() == 0 || !_.IsIntScalarType(index->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Index to be int scalar";
  }

  if (_.HasCapability(spv::Capability::Shader) &&
      _.ContainsLimitedUseIntOrFloatType(inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Cannot extract from a vector of 8- or 16-bit types";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateVectorInsertDyanmic(ValidationState_t& _,
                                         const Instruction* inst) {
  const uint32_t result_type = inst->type_id();
  const spv::Op result_opcode = _.GetIdOpcode(result_type);
  if (result_opcode != spv::Op::OpTypeVector &&
      result_opcode != spv::Op::OpTypeVectorIdEXT) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Result Type to be OpTypeVector";
  }

  const uint32_t vector_type = _.GetOperandTypeId(inst, 2);
  if (vector_type != result_type) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Vector type to be equal to Result Type";
  }

  const uint32_t component_type = _.GetOperandTypeId(inst, 3);
  if (_.GetComponentType(result_type) != component_type) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Component type to be equal to Result Type "
           << "component type";
  }

  const uint32_t index_type = _.GetOperandTypeId(inst, 4);
  if (!_.IsIntScalarType(index_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Index to be int scalar";
  }

  if (_.HasCapability(spv::Capability::Shader) &&
      _.ContainsLimitedUseIntOrFloatType(inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Cannot insert into a vector of 8- or 16-bit types";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateCompositeConstruct(ValidationState_t& _,
                                        const Instruction* inst) {
  const uint32_t num_operands = static_cast<uint32_t>(inst->operands().size());
  const uint32_t result_type = inst->type_id();
  const spv::Op result_opcode = _.GetIdOpcode(result_type);
  switch (result_opcode) {
    case spv::Op::OpTypeVector:
    case spv::Op::OpTypeVectorIdEXT: {
      uint32_t num_result_components = _.GetDimension(result_type);
      const uint32_t result_component_type = _.GetComponentType(result_type);
      uint32_t given_component_count = 0;

      bool comp_is_int32 = true, comp_is_const_int32 = true;

      if (result_opcode == spv::Op::OpTypeVector) {
        if (num_operands <= 3 &&
            !_.HasCapability(spv::Capability::LongVectorEXT)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Expected number of constituents to be at least 2";
        }
      } else {
        uint32_t comp_count_id =
            _.FindDef(result_type)->GetOperandAs<uint32_t>(2);
        std::tie(comp_is_int32, comp_is_const_int32, num_result_components) =
            _.EvalInt32IfConst(comp_count_id);
      }

      for (uint32_t operand_index = 2; operand_index < num_operands;
           ++operand_index) {
        const uint32_t operand_type = _.GetOperandTypeId(inst, operand_index);
        if (operand_type == result_component_type) {
          ++given_component_count;
        } else {
          if (_.GetIdOpcode(operand_type) != spv::Op::OpTypeVector ||
              _.GetComponentType(operand_type) != result_component_type) {
            return _.diag(SPV_ERROR_INVALID_DATA, inst)
                   << "Expected Constituents to be scalars or vectors of"
                   << " the same type as Result Type components";
          }

          given_component_count += _.GetDimension(operand_type);
        }
      }

      if (comp_is_const_int32 &&
          num_result_components != given_component_count) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected total number of given components to be equal "
               << "to the size of Result Type vector";
      }

      break;
    }
    case spv::Op::OpTypeMatrix: {
      uint32_t result_num_rows = 0;
      uint32_t result_num_cols = 0;
      uint32_t result_col_type = 0;
      uint32_t result_component_type = 0;
      if (!_.GetMatrixTypeInfo(result_type, &result_num_rows, &result_num_cols,
                               &result_col_type, &result_component_type)) {
        assert(0);
      }

      if (result_num_cols + 2 != num_operands) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected total number of Constituents to be equal "
               << "to the number of columns of Result Type matrix";
      }

      for (uint32_t operand_index = 2; operand_index < num_operands;
           ++operand_index) {
        const uint32_t operand_type = _.GetOperandTypeId(inst, operand_index);
        if (operand_type != result_col_type) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Expected Constituent type to be equal to the column "
                 << "type Result Type matrix";
        }
      }

      break;
    }
    case spv::Op::OpTypeArray: {
      const Instruction* const array_inst = _.FindDef(result_type);
      assert(array_inst);
      assert(array_inst->opcode() == spv::Op::OpTypeArray);

      auto size = _.FindDef(array_inst->word(3));
      if (spvOpcodeIsSpecConstant(size->opcode())) {
        // Cannot verify against the size of this array.
        break;
      }

      uint64_t array_size = 0;
      if (!_.EvalConstantValUint64(array_inst->word(3), &array_size)) {
        assert(0 && "Array type definition is corrupt");
      }

      if (array_size + 2 != num_operands) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected total number of Constituents to be equal "
               << "to the number of elements of Result Type array";
      }

      const uint32_t result_component_type = array_inst->word(2);
      for (uint32_t operand_index = 2; operand_index < num_operands;
           ++operand_index) {
        const uint32_t operand_type = _.GetOperandTypeId(inst, operand_index);
        if (operand_type != result_component_type) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Expected Constituent type to be equal to the column "
                 << "type Result Type array";
        }
      }

      break;
    }
    case spv::Op::OpTypeStruct: {
      const Instruction* const struct_inst = _.FindDef(result_type);
      assert(struct_inst);
      assert(struct_inst->opcode() == spv::Op::OpTypeStruct);

      if (struct_inst->operands().size() + 1 != num_operands) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected total number of Constituents to be equal "
               << "to the number of members of Result Type struct";
      }

      for (uint32_t operand_index = 2; operand_index < num_operands;
           ++operand_index) {
        const uint32_t operand_type = _.GetOperandTypeId(inst, operand_index);
        const uint32_t member_type = struct_inst->word(operand_index);
        if (operand_type != member_type) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Expected Constituent type to be equal to the "
                 << "corresponding member type of Result Type struct";
        }
      }

      break;
    }
    case spv::Op::OpTypeCooperativeMatrixKHR: {
      const auto result_type_inst = _.FindDef(result_type);
      assert(result_type_inst);
      const auto component_type_id =
          result_type_inst->GetOperandAs<uint32_t>(1);

      if (3 != num_operands) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Must be only one constituent";
      }

      const uint32_t operand_type_id = _.GetOperandTypeId(inst, 2);

      if (operand_type_id != component_type_id) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected Constituent type to be equal to the component type";
      }
      break;
    }
    case spv::Op::OpTypeCooperativeMatrixNV: {
      const auto result_type_inst = _.FindDef(result_type);
      assert(result_type_inst);
      const auto component_type_id =
          result_type_inst->GetOperandAs<uint32_t>(1);

      if (3 != num_operands) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected single constituent";
      }

      const uint32_t operand_type_id = _.GetOperandTypeId(inst, 2);

      if (operand_type_id != component_type_id) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected Constituent type to be equal to the component type";
      }

      break;
    }
    default: {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Expected Result Type to be a composite type";
    }
  }

  if (_.HasCapability(spv::Capability::Shader) &&
      _.ContainsLimitedUseIntOrFloatType(inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Cannot create a composite containing 8- or 16-bit types";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateCompositeExtract(ValidationState_t& _,
                                      const Instruction* inst) {
  uint32_t member_type = 0;
  if (spv_result_t error = GetExtractInsertValueType(_, inst, &member_type)) {
    return error;
  }

  const uint32_t result_type = inst->type_id();
  if (result_type != member_type) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result type (Op" << spvOpcodeString(_.GetIdOpcode(result_type))
           << ") does not match the type that results from indexing into "
              "the composite (Op"
           << spvOpcodeString(_.GetIdOpcode(member_type)) << ").";
  }

  if (_.HasCapability(spv::Capability::Shader) &&
      _.ContainsLimitedUseIntOrFloatType(inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Cannot extract from a composite of 8- or 16-bit types";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateCompositeInsert(ValidationState_t& _,
                                     const Instruction* inst) {
  const uint32_t object_type = _.GetOperandTypeId(inst, 2);
  const uint32_t composite_type = _.GetOperandTypeId(inst, 3);
  const uint32_t result_type = inst->type_id();
  if (result_type != composite_type) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "The Result Type must be the same as Composite type in Op"
           << spvOpcodeString(inst->opcode()) << " yielding Result Id "
           << result_type << ".";
  }

  uint32_t member_type = 0;
  if (spv_result_t error = GetExtractInsertValueType(_, inst, &member_type)) {
    return error;
  }

  if (object_type != member_type) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "The Object type (Op"
           << spvOpcodeString(_.GetIdOpcode(object_type))
           << ") does not match the type that results from indexing into the "
              "Composite (Op"
           << spvOpcodeString(_.GetIdOpcode(member_type)) << ").";
  }

  if (_.HasCapability(spv::Capability::Shader) &&
      _.ContainsLimitedUseIntOrFloatType(inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Cannot insert into a composite of 8- or 16-bit types";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateCopyObject(ValidationState_t& _, const Instruction* inst) {
  const uint32_t result_type = inst->type_id();
  const uint32_t operand_type = _.GetOperandTypeId(inst, 2);
  if (operand_type != result_type) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Result Type and Operand type to be the same";
  }
  if (_.IsVoidType(result_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "OpCopyObject cannot have void result type";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateTranspose(ValidationState_t& _, const Instruction* inst) {
  uint32_t result_num_rows = 0;
  uint32_t result_num_cols = 0;
  uint32_t result_col_type = 0;
  uint32_t result_component_type = 0;
  const uint32_t result_type = inst->type_id();
  if (!_.GetMatrixTypeInfo(result_type, &result_num_rows, &result_num_cols,
                           &result_col_type, &result_component_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Result Type to be a matrix type";
  }

  const uint32_t matrix_type = _.GetOperandTypeId(inst, 2);
  uint32_t matrix_num_rows = 0;
  uint32_t matrix_num_cols = 0;
  uint32_t matrix_col_type = 0;
  uint32_t matrix_component_type = 0;
  if (!_.GetMatrixTypeInfo(matrix_type, &matrix_num_rows, &matrix_num_cols,
                           &matrix_col_type, &matrix_component_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Matrix to be of type OpTypeMatrix";
  }

  if (result_component_type != matrix_component_type) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected component types of Matrix and Result Type to be "
           << "identical";
  }

  if (result_num_rows != matrix_num_cols ||
      result_num_cols != matrix_num_rows) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected number of columns and the column size of Matrix "
           << "to be the reverse of those of Result Type";
  }

  if (_.HasCapability(spv::Capability::Shader) &&
      _.ContainsLimitedUseIntOrFloatType(inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Cannot transpose matrices of 16-bit floats";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateVectorShuffle(ValidationState_t& _,
                                   const Instruction* inst) {
  auto resultType = _.FindDef(inst->type_id());
  if (!_.IsVectorType(resultType->id())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "The Result Type of OpVectorShuffle must be"
           << " a vector type. Found Op"
           << spvOpcodeString(resultType->opcode()) << ".";
  }

  // The number of components in Result Type must be the same as the number of
  // Component operands.
  auto componentCount = inst->operands().size() - 4;
  auto resultVectorDimension = _.GetDimension(resultType->id());
  if (resultVectorDimension > 0 && componentCount != resultVectorDimension) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpVectorShuffle component literals count does not match "
              "Result Type <id> "
           << _.getIdName(resultType->id()) << "s vector component count.";
  }

  // Vector 1 and Vector 2 must both have vector types, with the same Component
  // Type as Result Type.
  auto vector1Object = _.FindDef(inst->GetOperandAs<uint32_t>(2));
  auto vector1Type = _.FindDef(vector1Object->type_id());
  auto vector2Object = _.FindDef(inst->GetOperandAs<uint32_t>(3));
  auto vector2Type = _.FindDef(vector2Object->type_id());
  if (!_.IsVectorType(vector1Type->id())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "The type of Vector 1 must be a vector type.";
  }
  if (!_.IsVectorType(vector2Type->id())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "The type of Vector 2 must be a vector type.";
  }

  auto resultComponentType = resultType->GetOperandAs<uint32_t>(1);
  if (vector1Type->GetOperandAs<uint32_t>(1) != resultComponentType) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "The Component Type of Vector 1 must be the same as ResultType.";
  }
  if (vector2Type->GetOperandAs<uint32_t>(1) != resultComponentType) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "The Component Type of Vector 2 must be the same as ResultType.";
  }

  // All Component literals must either be FFFFFFFF or in [0, N - 1].
  auto vector1ComponentCount = vector1Type->GetOperandAs<uint32_t>(2);
  auto vector2ComponentCount = vector2Type->GetOperandAs<uint32_t>(2);
  auto N = vector1ComponentCount + vector2ComponentCount;
  auto firstLiteralIndex = 4;
  for (size_t i = firstLiteralIndex; i < inst->operands().size(); ++i) {
    auto literal = inst->GetOperandAs<uint32_t>(i);
    if (literal != 0xFFFFFFFF && literal >= N) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Component index " << literal << " is out of bounds for "
             << "combined (Vector1 + Vector2) size of " << N << ".";
    }
  }

  if (_.HasCapability(spv::Capability::Shader) &&
      _.ContainsLimitedUseIntOrFloatType(inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Cannot shuffle a vector of 8- or 16-bit types";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateCopyLogical(ValidationState_t& _,
                                 const Instruction* inst) {
  const auto result_type = _.FindDef(inst->type_id());
  const auto source = _.FindDef(inst->GetOperandAs<uint32_t>(2u));
  const auto source_type = _.FindDef(source->type_id());
  if (!source_type || !result_type || source_type == result_type) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Result Type must not equal the Operand type";
  }

  if (!_.LogicallyMatch(source_type, result_type, false)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Result Type does not logically match the Operand type";
  }

  if (_.HasCapability(spv::Capability::Shader) &&
      _.ContainsLimitedUseIntOrFloatType(inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Cannot copy composites of 8- or 16-bit types";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateCompositeConstructCoopMatQCOM(ValidationState_t& _,
                                                   const Instruction* inst) {
  // Is the result of coop mat ?
  const auto result_type_inst = _.FindDef(inst->type_id());
  if (!result_type_inst ||
      result_type_inst->opcode() != spv::Op::OpTypeCooperativeMatrixKHR) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Opcode " << spvOpcodeString(inst->opcode())
           << " requires the result type be OpTypeCooperativeMatrixKHR";
  }

  const auto source = _.FindDef(inst->GetOperandAs<uint32_t>(2u));
  const auto source_type_inst = _.FindDef(source->type_id());

  if (!source_type_inst || source_type_inst->opcode() != spv::Op::OpTypeArray) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Opcode " << spvOpcodeString(inst->opcode())
           << " requires the input operand be an OpTypeArray.";
  }

  // Is the scope Subgrouop ?
  {
    unsigned scope = UINT_MAX;
    unsigned scope_id = result_type_inst->GetOperandAs<unsigned>(2u);
    bool status = _.GetConstantValueAs<unsigned>(scope_id, scope);
    bool is_scope_spec_const =
        spvOpcodeIsSpecConstant(_.FindDef(scope_id)->opcode());
    if (!is_scope_spec_const &&
        (!status || scope != static_cast<uint64_t>(spv::Scope::Subgroup))) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Opcode " << spvOpcodeString(inst->opcode())
             << " requires the result type's scope be Subgroup.";
    }
  }

  unsigned ar_len = UINT_MAX;
  unsigned src_arr_len_id = source_type_inst->GetOperandAs<unsigned>(2u);
  bool ar_len_status = _.GetConstantValueAs<unsigned>(src_arr_len_id, ar_len);
  bool is_src_arr_len_spec_const =
      spvOpcodeIsSpecConstant(_.FindDef(src_arr_len_id)->opcode());

  const auto source_elt_type = _.GetComponentType(source_type_inst->id());
  const auto result_elt_type = result_type_inst->GetOperandAs<uint32_t>(1u);

  if ((source_elt_type != result_elt_type) &&
      !(_.ContainsSizedIntOrFloatType(source_elt_type, spv::Op::OpTypeInt,
                                      32) &&
        _.IsUnsignedIntScalarType(source_elt_type))) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Opcode " << spvOpcodeString(inst->opcode())
           << " requires ether the input element type is equal to the result "
              "element type or it is the unsigned 32-bit integer.";
  }

  unsigned res_row_id = result_type_inst->GetOperandAs<unsigned>(3u);
  unsigned res_col_id = result_type_inst->GetOperandAs<unsigned>(4u);
  unsigned res_use_id = result_type_inst->GetOperandAs<unsigned>(5u);

  unsigned cm_use = UINT_MAX;
  bool cm_use_status = _.GetConstantValueAs<unsigned>(res_use_id, cm_use);

  switch (static_cast<spv::CooperativeMatrixUse>(cm_use)) {
    case spv::CooperativeMatrixUse::MatrixAKHR: {
      // result coopmat component type check
      if (!_.IsIntNOrFP32OrFP16<8>(result_elt_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the result element type is one of 8-bit OpTypeInt "
                  "signed/unsigned, 16- or 32-bit OpTypeFloat"
               << " when result coopmat's use is MatrixAKHR";
      }

      // result coopmat column length check
      unsigned n_cols = UINT_MAX;
      bool status = _.GetConstantValueAs<unsigned>(res_col_id, n_cols);
      bool is_res_col_spec_const =
          spvOpcodeIsSpecConstant(_.FindDef(res_col_id)->opcode());
      if (!is_res_col_spec_const &&
          (!status || (!(_.ContainsSizedIntOrFloatType(result_elt_type,
                                                       spv::Op::OpTypeInt, 8) &&
                         n_cols == 32) &&
                       !(_.ContainsSizedIntOrFloatType(
                             result_elt_type, spv::Op::OpTypeFloat, 16) &&
                         n_cols == 16) &&
                       !(_.ContainsSizedIntOrFloatType(
                             result_elt_type, spv::Op::OpTypeFloat, 32) &&
                         n_cols == 8)))) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the columns of the result coopmat have the bit "
                  "length of 256"
               << " when result coopmat's use is MatrixAKHR";
      }
      // source array length check
      if (!is_src_arr_len_spec_const &&
          (!ar_len_status ||
           (!(_.ContainsSizedIntOrFloatType(source_elt_type, spv::Op::OpTypeInt,
                                            32) &&
              _.IsUnsignedIntScalarType(source_elt_type) && (ar_len == 8)) &&
            !(n_cols == ar_len)))) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the source array length be 8 if its elt type is "
                  "32-bit unsigned OpTypeInt and be the result's number of "
                  "columns, otherwise"
               << " when result coopmat's use is MatrixAKHR";
      }
      break;
    }
    case spv::CooperativeMatrixUse::MatrixBKHR: {
      // result coopmat component type check
      if (!_.IsIntNOrFP32OrFP16<8>(result_elt_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the result element type is one of 8-bit OpTypeInt "
                  "signed/unsigned, 16- or 32-bit OpTypeFloat"
               << " when result coopmat's use is MatrixBKHR";
      }

      // result coopmat row length check
      unsigned n_rows = UINT_MAX;
      bool status = _.GetConstantValueAs<unsigned>(res_row_id, n_rows);
      bool is_res_row_spec_const =
          spvOpcodeIsSpecConstant(_.FindDef(res_row_id)->opcode());
      if (!is_res_row_spec_const &&
          (!status || (!(_.ContainsSizedIntOrFloatType(result_elt_type,
                                                       spv::Op::OpTypeInt, 8) &&
                         n_rows == 32) &&
                       !(_.ContainsSizedIntOrFloatType(
                             result_elt_type, spv::Op::OpTypeFloat, 16) &&
                         n_rows == 16) &&
                       !(_.ContainsSizedIntOrFloatType(
                             result_elt_type, spv::Op::OpTypeFloat, 32) &&
                         n_rows == 8)))) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the rows of the result operand have the bit "
                  "length of 256"
               << " when result coopmat's use is MatrixBKHR";
      }
      // source array length check
      if (!is_src_arr_len_spec_const &&
          (!ar_len_status ||
           (!(_.ContainsSizedIntOrFloatType(source_elt_type, spv::Op::OpTypeInt,
                                            32) &&
              _.IsUnsignedIntScalarType(source_elt_type) && (ar_len == 8)) &&
            !(n_rows == ar_len)))) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the source array length be 8 if its elt type is "
                  "32-bit unsigned OpTypeInt and be the result's number of "
                  "rows, otherwise"
               << " when result coopmat's use is MatrixBKHR";
      }
      break;
    }
    case spv::CooperativeMatrixUse::MatrixAccumulatorKHR: {
      // result coopmat component type check
      if (!_.IsIntNOrFP32OrFP16<32>(result_elt_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the result element type is one of 32-bit "
                  "OpTypeInt signed/unsigned, 16- or 32-bit OpTypeFloat"
               << " when result coopmat's use is MatrixAccumulatorKHR";
      }

      // source array length check
      unsigned n_cols = UINT_MAX;
      bool status = _.GetConstantValueAs<unsigned>(res_col_id, n_cols);
      bool is_res_col_spec_const =
          spvOpcodeIsSpecConstant(_.FindDef(res_col_id)->opcode());
      if (!is_res_col_spec_const && !is_src_arr_len_spec_const &&
          (!status || !ar_len_status ||
           (!(_.ContainsSizedIntOrFloatType(source_elt_type, spv::Op::OpTypeInt,
                                            32) &&
              _.IsUnsignedIntScalarType(source_elt_type) &&
              (_.ContainsSizedIntOrFloatType(result_elt_type,
                                             spv::Op::OpTypeFloat, 16)
                   ? (n_cols / 2 == ar_len)
                   : n_cols == ar_len)) &&
            (n_cols != ar_len)))) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the source array length be a half of the number "
                  "of columns of the resulting cooerative matrix if the "
                  "matrix's componet type is 16-bit OpTypeFloat and be equal "
                  "to the number of columns, otherwise,"
               << " when result coopmat's use is MatrixAccumulatorKHR";
      }
      break;
    }
    default: {
      bool is_cm_use_spec_const =
          spvOpcodeIsSpecConstant(_.FindDef(res_use_id)->opcode());
      if (!is_cm_use_spec_const || !cm_use_status) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the the resulting cooerative matrix's use be "
               << " one of MatrixAKHR (== 0), MatrixBKHR (== 1), and "
                  "MatrixAccumulatorKHR (== 2)";
      }
      break;
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateCompositeExtractCoopMatQCOM(ValidationState_t& _,
                                                 const Instruction* inst) {
  const auto result_type_inst = _.FindDef(inst->type_id());
  if (!result_type_inst || result_type_inst->opcode() != spv::Op::OpTypeArray) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Opcode " << spvOpcodeString(inst->opcode())
           << " requires the input operand be an OpTypeArray.";
  }

  const auto source = _.FindDef(inst->GetOperandAs<uint32_t>(2u));
  const auto source_type_inst = _.FindDef(source->type_id());

  // Is the source of coop mat ?
  if (!source_type_inst ||
      source_type_inst->opcode() != spv::Op::OpTypeCooperativeMatrixKHR) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Opcode " << spvOpcodeString(inst->opcode())
           << " requires the source type be OpTypeCooperativeMatrixKHR";
  }

  // Is the scope Subgrouop ?
  {
    unsigned scope = UINT_MAX;
    unsigned scope_id = source_type_inst->GetOperandAs<unsigned>(2u);
    bool status = _.GetConstantValueAs<unsigned>(scope_id, scope);
    bool is_scope_spec_const =
        spvOpcodeIsSpecConstant(_.FindDef(scope_id)->opcode());
    if (!is_scope_spec_const &&
        (!status || scope != static_cast<uint64_t>(spv::Scope::Subgroup))) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Opcode " << spvOpcodeString(inst->opcode())
             << " requires the source type's scope be Subgroup.";
    }
  }

  unsigned ar_len = UINT_MAX;
  unsigned res_arr_len_id = result_type_inst->GetOperandAs<unsigned>(2u);
  bool ar_len_status = _.GetConstantValueAs<unsigned>(res_arr_len_id, ar_len);
  bool is_res_arr_len_spec_const =
      spvOpcodeIsSpecConstant(_.FindDef(res_arr_len_id)->opcode());

  const auto source_elt_type = _.GetComponentType(source_type_inst->id());
  const auto result_elt_type = result_type_inst->GetOperandAs<uint32_t>(1u);

  unsigned src_row_id = source_type_inst->GetOperandAs<unsigned>(3u);
  unsigned src_col_id = source_type_inst->GetOperandAs<unsigned>(4u);
  unsigned src_use_id = source_type_inst->GetOperandAs<unsigned>(5u);

  unsigned cm_use = UINT_MAX;
  bool cm_use_status = _.GetConstantValueAs<unsigned>(src_use_id, cm_use);

  switch (static_cast<spv::CooperativeMatrixUse>(cm_use)) {
    case spv::CooperativeMatrixUse::MatrixAKHR: {
      // source coopmat component type check
      if (!_.IsIntNOrFP32OrFP16<8>(source_elt_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the source element type be one of 8-bit OpTypeInt "
                  "signed/unsigned, 16- or 32-bit OpTypeFloat"
               << " when source coopmat's use is MatrixAKHR";
      }

      // source coopmat column length check
      unsigned n_cols = UINT_MAX;
      bool status = _.GetConstantValueAs<unsigned>(src_col_id, n_cols);
      bool is_src_col_spec_const =
          spvOpcodeIsSpecConstant(_.FindDef(src_col_id)->opcode());
      if (!is_src_col_spec_const &&
          (!status || (!(_.ContainsSizedIntOrFloatType(source_elt_type,
                                                       spv::Op::OpTypeInt, 8) &&
                         n_cols == 32) &&
                       !(_.ContainsSizedIntOrFloatType(
                             source_elt_type, spv::Op::OpTypeFloat, 16) &&
                         n_cols == 16) &&
                       !(_.ContainsSizedIntOrFloatType(
                             source_elt_type, spv::Op::OpTypeFloat, 32) &&
                         n_cols == 8)))) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the columns of the source coopmat have the bit "
                  "length of 256"
               << " when source coopmat's use is MatrixAKHR";
      }
      // result type check
      if (!is_res_arr_len_spec_const &&
          !(source_elt_type == result_elt_type && (n_cols == ar_len)) &&
          !(_.ContainsSizedIntOrFloatType(result_elt_type, spv::Op::OpTypeInt,
                                          32) &&
            _.IsUnsignedIntScalarType(result_elt_type) && (ar_len == 8))) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires either the result element type be the same as the "
                  "source cooperative matrix's component type"
               << " and its length be the same as the number of columns of the "
                  "matrix or the result element type be"
               << " unsigned 32-bit OpTypeInt and the length be 8"
               << " when source coopmat's use is MatrixAKHR";
      }
      break;
    }
    case spv::CooperativeMatrixUse::MatrixBKHR: {
      // source coopmat component type check
      if (!_.IsIntNOrFP32OrFP16<8>(source_elt_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the source element type be one of 8-bit OpTypeInt "
                  "signed/unsigned, 16- or 32-bit OpTypeFloat"
               << " when source coopmat's use is MatrixBKHR";
      }

      // source coopmat row length check
      unsigned n_rows = UINT_MAX;
      bool status = _.GetConstantValueAs<unsigned>(src_row_id, n_rows);
      bool is_src_row_spec_const =
          spvOpcodeIsSpecConstant(_.FindDef(src_row_id)->opcode());
      if (!is_src_row_spec_const &&
          (!status || (!(_.ContainsSizedIntOrFloatType(source_elt_type,
                                                       spv::Op::OpTypeInt, 8) &&
                         n_rows == 32) &&
                       !(_.ContainsSizedIntOrFloatType(
                             source_elt_type, spv::Op::OpTypeFloat, 16) &&
                         n_rows == 16) &&
                       !(_.ContainsSizedIntOrFloatType(
                             source_elt_type, spv::Op::OpTypeFloat, 32) &&
                         n_rows == 8)))) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the rows of the source coopmat have the bit "
                  "length of 256"
               << " when source coopmat's use is MatrixBKHR";
      }
      // result type check
      if (!is_res_arr_len_spec_const &&
          !(source_elt_type == result_elt_type && (n_rows == ar_len)) &&
          !(_.ContainsSizedIntOrFloatType(result_elt_type, spv::Op::OpTypeInt,
                                          32) &&
            _.IsUnsignedIntScalarType(result_elt_type) && (ar_len == 8))) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires either the result element type be the same as the "
                  "source cooperative matrix's component type"
               << " and its length be the same as the number of rows of the "
                  "matrix or the result element type be"
               << " unsigned 32-bit OpTypeInt and the length be 8"
               << " when source coopmat's use is MatrixBKHR";
      }
      break;
    }
    case spv::CooperativeMatrixUse::MatrixAccumulatorKHR: {
      // source coopmat component type check
      if (!_.IsIntNOrFP32OrFP16<32>(source_elt_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the source element type be one of 32-bit "
                  "OpTypeInt signed/unsigned, 16- or 32-bit OpTypeFloat"
               << " when source coopmat's use is MatrixAccumulatorKHR";
      }

      // result type check
      unsigned n_cols = UINT_MAX;
      bool status = _.GetConstantValueAs<unsigned>(src_col_id, n_cols);
      bool is_src_col_spec_const =
          spvOpcodeIsSpecConstant(_.FindDef(src_col_id)->opcode());
      if (!is_src_col_spec_const && !is_res_arr_len_spec_const &&
          (!status || !ar_len_status ||
           (!(source_elt_type == result_elt_type && (n_cols == ar_len)) &&
            !(_.ContainsSizedIntOrFloatType(result_elt_type, spv::Op::OpTypeInt,
                                            32) &&
              _.IsUnsignedIntScalarType(result_elt_type) &&
              (_.ContainsSizedIntOrFloatType(source_elt_type,
                                             spv::Op::OpTypeFloat, 16)
                   ? (n_cols / 2 == ar_len)
                   : (n_cols == ar_len)))))) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires either the result element type be the same as the "
                  "source cooperative matrix's component type"
               << " and its length be the same as the number of columns of the "
                  "matrix or the result element type be"
               << " unsigned 32-bit OpTypeInt and the length be the number of "
                  "the columns of the matrix if its component"
               << " type is 32-bit OpTypeFloat and be a half of the number of "
                  "the columns of the matrix if its component"
               << " type is 16-bit OpTypeFloat"
               << " when source coopmat's use is MatrixAccumulatorKHR";
      }
      break;
    }
    default: {
      bool is_cm_use_spec_const =
          spvOpcodeIsSpecConstant(_.FindDef(src_use_id)->opcode());
      if (!is_cm_use_spec_const || !cm_use_status) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the the source cooerative matrix's use be "
               << " one of MatrixAKHR (== 0), MatrixBKHR (== 1), and "
                  "MatrixAccumulatorKHR (== 2)";
      }
      break;
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateExtractSubArrayQCOM(ValidationState_t& _,
                                         const Instruction* inst) {
  const auto result_type_inst = _.FindDef(inst->type_id());
  const auto source = _.FindDef(inst->GetOperandAs<uint32_t>(2u));
  const auto source_type_inst = _.FindDef(source->type_id());

  // Are the input and the result arrays?
  if (result_type_inst->opcode() != spv::Op::OpTypeArray ||
      source_type_inst->opcode() != spv::Op::OpTypeArray) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Opcode " << spvOpcodeString(inst->opcode())
           << " requires OpTypeArray operands for the input and the result.";
  }

  const auto source_elt_type = _.GetComponentType(source_type_inst->id());
  const auto result_elt_type = _.GetComponentType(result_type_inst->id());

  // Do the input and result element types match?
  if (source_elt_type != result_elt_type) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Opcode " << spvOpcodeString(inst->opcode())
           << " requires the input and result element types match.";
  }

  // Elt type must be one of int32_t/uint32_t/float32/float16
  if (!_.IsIntNOrFP32OrFP16<32>(source_elt_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Opcode " << spvOpcodeString(inst->opcode())
           << " requires the element type be one of 32-bit OpTypeInt "
              "(signed/unsigned), 32-bit OpTypeFloat and 16-bit OpTypeFloat";
  }

  const auto start_index = _.FindDef(inst->GetOperandAs<uint32_t>(3u));
  if (!start_index || !_.ContainsSizedIntOrFloatType(start_index->type_id(),
                                                     spv::Op::OpTypeInt, 32)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Opcode " << spvOpcodeString(inst->opcode())
           << " requires the type of the start index operand be 32-bit "
              "OpTypeInt";
  }

  return SPV_SUCCESS;
}

}  // anonymous namespace
// Validates correctness of composite instructions.
spv_result_t CompositesPass(ValidationState_t& _, const Instruction* inst) {
  switch (inst->opcode()) {
    case spv::Op::OpVectorExtractDynamic:
      return ValidateVectorExtractDynamic(_, inst);
    case spv::Op::OpVectorInsertDynamic:
      return ValidateVectorInsertDyanmic(_, inst);
    case spv::Op::OpVectorShuffle:
      return ValidateVectorShuffle(_, inst);
    case spv::Op::OpCompositeConstruct:
      return ValidateCompositeConstruct(_, inst);
    case spv::Op::OpCompositeExtract:
      return ValidateCompositeExtract(_, inst);
    case spv::Op::OpCompositeInsert:
      return ValidateCompositeInsert(_, inst);
    case spv::Op::OpCopyObject:
      return ValidateCopyObject(_, inst);
    case spv::Op::OpTranspose:
      return ValidateTranspose(_, inst);
    case spv::Op::OpCopyLogical:
      return ValidateCopyLogical(_, inst);
    case spv::Op::OpCompositeConstructCoopMatQCOM:
      return ValidateCompositeConstructCoopMatQCOM(_, inst);
    case spv::Op::OpCompositeExtractCoopMatQCOM:
      return ValidateCompositeExtractCoopMatQCOM(_, inst);
    case spv::Op::OpExtractSubArrayQCOM:
      return ValidateExtractSubArrayQCOM(_, inst);
    default:
      break;
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
