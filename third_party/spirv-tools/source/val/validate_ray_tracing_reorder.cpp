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

// Validates ray tracing instructions from SPV_NV_shader_invocation_reorder and
// SPV_EXT_shader_invocation_reorder

#include "source/opcode.h"
#include "source/val/instruction.h"
#include "source/val/validate.h"
#include "source/val/validation_state.h"

#include <limits>

namespace spvtools {
namespace val {

static const uint32_t KRayParamInvalidId = std::numeric_limits<uint32_t>::max();

uint32_t GetArrayLength(ValidationState_t& _, const Instruction* array_type) {
  assert(array_type->opcode() == spv::Op::OpTypeArray);
  uint32_t const_int_id = array_type->GetOperandAs<uint32_t>(2U);
  Instruction* array_length_inst = _.FindDef(const_int_id);
  uint32_t array_length = 0;
  if (array_length_inst->opcode() == spv::Op::OpConstant) {
    array_length = array_length_inst->GetOperandAs<uint32_t>(2);
  }
  return array_length;
}

spv_result_t ValidateRayQueryPointer(ValidationState_t& _,
                                     const Instruction* inst,
                                     uint32_t ray_query_index) {
  const uint32_t ray_query_id = inst->GetOperandAs<uint32_t>(ray_query_index);
  auto variable = _.FindDef(ray_query_id);
  auto pointer = _.FindDef(variable->GetOperandAs<uint32_t>(0));
  if (!pointer || pointer->opcode() != spv::Op::OpTypePointer) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Ray Query must be a pointer";
  }
  auto type = _.FindDef(pointer->GetOperandAs<uint32_t>(2));
  if (!type || type->opcode() != spv::Op::OpTypeRayQueryKHR) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Ray Query must be a pointer to OpTypeRayQueryKHR";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateHitObjectPointer(ValidationState_t& _,
                                      const Instruction* inst,
                                      uint32_t hit_object_index) {
  const uint32_t hit_object_id = inst->GetOperandAs<uint32_t>(hit_object_index);
  auto variable = _.FindDef(hit_object_id);
  auto pointer = _.FindDef(variable->GetOperandAs<uint32_t>(0));
  if (!pointer || pointer->opcode() != spv::Op::OpTypePointer) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Hit Object must be a pointer";
  }
  auto type = _.FindDef(pointer->GetOperandAs<uint32_t>(2));
  if (!type || type->opcode() != spv::Op::OpTypeHitObjectNV) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Type must be OpTypeHitObjectNV";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateHitObjectPointerEXT(ValidationState_t& _,
                                         const Instruction* inst,
                                         uint32_t hit_object_index) {
  const uint32_t hit_object_id = inst->GetOperandAs<uint32_t>(hit_object_index);
  auto variable = _.FindDef(hit_object_id);
  auto pointer = _.FindDef(variable->GetOperandAs<uint32_t>(0));
  if (!pointer || pointer->opcode() != spv::Op::OpTypePointer) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Hit Object must be a pointer";
  }
  auto type = _.FindDef(pointer->GetOperandAs<uint32_t>(2));
  if (!type || type->opcode() != spv::Op::OpTypeHitObjectEXT) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Type must be OpTypeHitObjectEXT";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateHitObjectInstructionCommonParameters(
    ValidationState_t& _, const Instruction* inst,
    uint32_t acceleration_struct_index, uint32_t instance_id_index,
    uint32_t primtive_id_index, uint32_t geometry_index,
    uint32_t ray_flags_index, uint32_t cull_mask_index, uint32_t hit_kind_index,
    uint32_t sbt_index, uint32_t sbt_offset_index, uint32_t sbt_stride_index,
    uint32_t sbt_record_offset_index, uint32_t sbt_record_stride_index,
    uint32_t miss_index, uint32_t ray_origin_index, uint32_t ray_tmin_index,
    uint32_t ray_direction_index, uint32_t ray_tmax_index,
    uint32_t payload_index, uint32_t hit_object_attr_index) {
  auto isValidId = [](uint32_t spvid) { return spvid < KRayParamInvalidId; };
  if (isValidId(acceleration_struct_index) &&
      _.GetIdOpcode(_.GetOperandTypeId(inst, acceleration_struct_index)) !=
          spv::Op::OpTypeAccelerationStructureKHR) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Acceleration Structure to be of type "
              "OpTypeAccelerationStructureKHR";
  }

  if (isValidId(instance_id_index)) {
    const uint32_t instance_id = _.GetOperandTypeId(inst, instance_id_index);
    if (!_.IsIntScalarType(instance_id, 32)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Instance Id must be a 32-bit int scalar";
    }
  }

  if (isValidId(primtive_id_index)) {
    const uint32_t primitive_id = _.GetOperandTypeId(inst, primtive_id_index);
    if (!_.IsIntScalarType(primitive_id, 32)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Primitive Id must be a 32-bit int scalar";
    }
  }

  if (isValidId(geometry_index)) {
    const uint32_t geometry_index_id = _.GetOperandTypeId(inst, geometry_index);
    if (!_.IsIntScalarType(geometry_index_id, 32)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Geometry Index must be a 32-bit int scalar";
    }
  }

  if (isValidId(miss_index)) {
    const uint32_t miss_index_id = _.GetOperandTypeId(inst, miss_index);
    if (!_.IsUnsignedIntScalarType(miss_index_id) ||
        _.GetBitWidth(miss_index_id) != 32) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Miss Index must be a 32-bit int scalar";
    }
  }

  if (isValidId(cull_mask_index)) {
    const uint32_t cull_mask_id = _.GetOperandTypeId(inst, cull_mask_index);
    if (!_.IsUnsignedIntScalarType(cull_mask_id) ||
        _.GetBitWidth(cull_mask_id) != 32) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Cull mask must be a 32-bit int scalar";
    }
  }

  if (isValidId(sbt_index)) {
    const uint32_t sbt_index_id = _.GetOperandTypeId(inst, sbt_index);
    if (!_.IsUnsignedIntScalarType(sbt_index_id) ||
        _.GetBitWidth(sbt_index_id) != 32) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "SBT Index must be a 32-bit unsigned int scalar";
    }
  }

  if (isValidId(sbt_offset_index)) {
    const uint32_t sbt_offset_id = _.GetOperandTypeId(inst, sbt_offset_index);
    if (!_.IsUnsignedIntScalarType(sbt_offset_id) ||
        _.GetBitWidth(sbt_offset_id) != 32) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "SBT Offset must be a 32-bit unsigned int scalar";
    }
  }

  if (isValidId(sbt_stride_index)) {
    const uint32_t sbt_stride_index_id =
        _.GetOperandTypeId(inst, sbt_stride_index);
    if (!_.IsUnsignedIntScalarType(sbt_stride_index_id) ||
        _.GetBitWidth(sbt_stride_index_id) != 32) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "SBT Stride must be a 32-bit unsigned int scalar";
    }
  }

  if (isValidId(sbt_record_offset_index)) {
    const uint32_t sbt_record_offset_index_id =
        _.GetOperandTypeId(inst, sbt_record_offset_index);
    if (!_.IsUnsignedIntScalarType(sbt_record_offset_index_id) ||
        _.GetBitWidth(sbt_record_offset_index_id) != 32) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "SBT record offset must be a 32-bit unsigned int scalar";
    }
  }

  if (isValidId(sbt_record_stride_index)) {
    const uint32_t sbt_record_stride_index_id =
        _.GetOperandTypeId(inst, sbt_record_stride_index);
    if (!_.IsUnsignedIntScalarType(sbt_record_stride_index_id) ||
        _.GetBitWidth(sbt_record_stride_index_id) != 32) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "SBT record stride must be a 32-bit unsigned int scalar";
    }
  }

  if (isValidId(ray_origin_index)) {
    const uint32_t ray_origin_id = _.GetOperandTypeId(inst, ray_origin_index);
    if (!_.IsFloatVectorType(ray_origin_id) ||
        _.GetDimension(ray_origin_id) != 3 ||
        _.GetBitWidth(ray_origin_id) != 32) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Ray Origin must be a 32-bit float 3-component vector";
    }
  }

  if (isValidId(ray_tmin_index)) {
    const uint32_t ray_tmin_id = _.GetOperandTypeId(inst, ray_tmin_index);
    if (!_.IsFloatScalarType(ray_tmin_id, 32)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Ray TMin must be a 32-bit float scalar";
    }
  }

  if (isValidId(ray_direction_index)) {
    const uint32_t ray_direction_id =
        _.GetOperandTypeId(inst, ray_direction_index);
    if (!_.IsFloatVectorType(ray_direction_id) ||
        _.GetDimension(ray_direction_id) != 3 ||
        _.GetBitWidth(ray_direction_id) != 32) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Ray Direction must be a 32-bit float 3-component vector";
    }
  }

  if (isValidId(ray_tmax_index)) {
    const uint32_t ray_tmax_id = _.GetOperandTypeId(inst, ray_tmax_index);
    if (!_.IsFloatScalarType(ray_tmax_id, 32)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Ray TMax must be a 32-bit float scalar";
    }
  }

  if (isValidId(ray_flags_index)) {
    const uint32_t ray_flags_id = _.GetOperandTypeId(inst, ray_flags_index);
    if (!_.IsIntScalarType(ray_flags_id, 32)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Ray Flags must be a 32-bit int scalar";
    }
  }

  if (isValidId(payload_index)) {
    const uint32_t payload_id = inst->GetOperandAs<uint32_t>(payload_index);
    auto variable = _.FindDef(payload_id);
    const auto var_opcode = variable->opcode();
    if (!variable || var_opcode != spv::Op::OpVariable ||
        (variable->GetOperandAs<spv::StorageClass>(2) !=
             spv::StorageClass::RayPayloadKHR &&
         variable->GetOperandAs<spv::StorageClass>(2) !=
             spv::StorageClass::IncomingRayPayloadKHR)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "payload must be a OpVariable of storage "
                "class RayPayloadKHR or IncomingRayPayloadKHR";
    }
  }

  if (isValidId(hit_kind_index)) {
    const uint32_t hit_kind_id = _.GetOperandTypeId(inst, hit_kind_index);
    if (!_.IsUnsignedIntScalarType(hit_kind_id) ||
        _.GetBitWidth(hit_kind_id) != 32) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Hit Kind must be a 32-bit unsigned int scalar";
    }
  }

  if (isValidId(hit_object_attr_index)) {
    const uint32_t hit_object_attr_id =
        inst->GetOperandAs<uint32_t>(hit_object_attr_index);
    auto variable = _.FindDef(hit_object_attr_id);
    const auto var_opcode = variable->opcode();
    if (!variable || var_opcode != spv::Op::OpVariable ||
        !((variable->GetOperandAs<spv::StorageClass>(2) ==
           spv::StorageClass::HitObjectAttributeNV) ||
          (variable->GetOperandAs<spv::StorageClass>(2) ==
           spv::StorageClass::HitObjectAttributeEXT))) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Hit Object Attributes id must be a OpVariable of storage "
                "class HitObjectAttributeNV";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t RayReorderNVPass(ValidationState_t& _, const Instruction* inst) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();

  auto RegisterOpcodeForValidModel = [](ValidationState_t& vs,
                                        const Instruction* rtinst) {
    std::string opcode_name = spvOpcodeString(rtinst->opcode());
    vs.function(rtinst->function()->id())
        ->RegisterExecutionModelLimitation(
            [opcode_name](spv::ExecutionModel model, std::string* message) {
              if (model != spv::ExecutionModel::RayGenerationKHR &&
                  model != spv::ExecutionModel::ClosestHitKHR &&
                  model != spv::ExecutionModel::MissKHR) {
                if (message) {
                  *message = opcode_name +
                             " requires RayGenerationKHR, ClosestHitKHR and "
                             "MissKHR execution models";
                }
                return false;
              }
              return true;
            });
    return;
  };

  switch (opcode) {
    case spv::Op::OpHitObjectIsMissNV:
    case spv::Op::OpHitObjectIsHitNV:
    case spv::Op::OpHitObjectIsEmptyNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (!_.IsBoolScalarType(result_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "expected Result Type to be bool scalar type";
      }

      if (auto error = ValidateHitObjectPointer(_, inst, 2)) return error;
      break;
    }

    case spv::Op::OpHitObjectGetShaderRecordBufferHandleNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 2)) return error;

      if (!_.IsIntVectorType(result_type) ||
          (_.GetDimension(result_type) != 2) ||
          (_.GetBitWidth(result_type) != 32))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 32-bit integer type 2-component vector as Result "
                  "Type: "
               << spvOpcodeString(opcode);
      break;
    }

    case spv::Op::OpHitObjectGetHitKindNV:
    case spv::Op::OpHitObjectGetPrimitiveIndexNV:
    case spv::Op::OpHitObjectGetGeometryIndexNV:
    case spv::Op::OpHitObjectGetInstanceIdNV:
    case spv::Op::OpHitObjectGetInstanceCustomIndexNV:
    case spv::Op::OpHitObjectGetShaderBindingTableRecordIndexNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 2)) return error;

      if (!_.IsIntScalarType(result_type, 32))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 32-bit integer type scalar as Result Type: "
               << spvOpcodeString(opcode);
      break;
    }

    case spv::Op::OpHitObjectGetCurrentTimeNV:
    case spv::Op::OpHitObjectGetRayTMaxNV:
    case spv::Op::OpHitObjectGetRayTMinNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 2)) return error;

      if (!_.IsFloatScalarType(result_type, 32))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 32-bit floating-point type scalar as Result Type: "
               << spvOpcodeString(opcode);
      break;
    }

    case spv::Op::OpHitObjectGetObjectToWorldNV:
    case spv::Op::OpHitObjectGetWorldToObjectNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 2)) return error;

      uint32_t num_rows = 0;
      uint32_t num_cols = 0;
      uint32_t col_type = 0;
      uint32_t component_type = 0;

      if (!_.GetMatrixTypeInfo(result_type, &num_rows, &num_cols, &col_type,
                               &component_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "expected matrix type as Result Type: "
               << spvOpcodeString(opcode);
      }

      if (num_cols != 4) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "expected Result Type matrix to have a Column Count of 4"
               << spvOpcodeString(opcode);
      }

      if (!_.IsFloatScalarType(component_type) ||
          _.GetBitWidth(result_type) != 32 || num_rows != 3) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "expected Result Type matrix to have a Column Type of "
                  "3-component 32-bit float vectors: "
               << spvOpcodeString(opcode);
      }
      break;
    }

    case spv::Op::OpHitObjectGetObjectRayOriginNV:
    case spv::Op::OpHitObjectGetObjectRayDirectionNV:
    case spv::Op::OpHitObjectGetWorldRayDirectionNV:
    case spv::Op::OpHitObjectGetWorldRayOriginNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 2)) return error;

      if (!_.IsFloatVectorType(result_type) ||
          (_.GetDimension(result_type) != 3) ||
          (_.GetBitWidth(result_type) != 32))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 32-bit floating-point type 3-component vector as "
                  "Result Type: "
               << spvOpcodeString(opcode);
      break;
    }

    case spv::Op::OpHitObjectGetAttributesNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 0)) return error;

      const uint32_t hit_object_attr_id = inst->GetOperandAs<uint32_t>(1);
      auto variable = _.FindDef(hit_object_attr_id);
      const auto var_opcode = variable->opcode();
      if (!variable || var_opcode != spv::Op::OpVariable ||
          variable->GetOperandAs<spv::StorageClass>(2) !=
              spv::StorageClass::HitObjectAttributeNV) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Hit Object Attributes id must be a OpVariable of storage "
                  "class HitObjectAttributeNV";
      }
      break;
    }

    case spv::Op::OpHitObjectExecuteShaderNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 0)) return error;

      const uint32_t hit_object_attr_id = inst->GetOperandAs<uint32_t>(1);
      auto variable = _.FindDef(hit_object_attr_id);
      const auto var_opcode = variable->opcode();
      if (!variable || var_opcode != spv::Op::OpVariable ||
          (variable->GetOperandAs<spv::StorageClass>(2)) !=
              spv::StorageClass::RayPayloadKHR) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Hit Object Attributes id must be a OpVariable of storage "
                  "class RayPayloadKHR";
      }
      break;
    }

    case spv::Op::OpHitObjectRecordEmptyNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 0)) return error;
      break;
    }

    case spv::Op::OpHitObjectRecordMissNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 0)) return error;

      const uint32_t miss_index = _.GetOperandTypeId(inst, 1);
      if (!_.IsUnsignedIntScalarType(miss_index) ||
          _.GetBitWidth(miss_index) != 32) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Miss Index must be a 32-bit int scalar";
      }

      const uint32_t ray_origin = _.GetOperandTypeId(inst, 2);
      if (!_.IsFloatVectorType(ray_origin) || _.GetDimension(ray_origin) != 3 ||
          _.GetBitWidth(ray_origin) != 32) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Ray Origin must be a 32-bit float 3-component vector";
      }

      const uint32_t ray_tmin = _.GetOperandTypeId(inst, 3);
      if (!_.IsFloatScalarType(ray_tmin, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Ray TMin must be a 32-bit float scalar";
      }

      const uint32_t ray_direction = _.GetOperandTypeId(inst, 4);
      if (!_.IsFloatVectorType(ray_direction) ||
          _.GetDimension(ray_direction) != 3 ||
          _.GetBitWidth(ray_direction) != 32) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Ray Direction must be a 32-bit float 3-component vector";
      }

      const uint32_t ray_tmax = _.GetOperandTypeId(inst, 5);
      if (!_.IsFloatScalarType(ray_tmax, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Ray TMax must be a 32-bit float scalar";
      }
      break;
    }

    case spv::Op::OpHitObjectRecordHitWithIndexNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 0)) return error;

      if (auto error = ValidateHitObjectInstructionCommonParameters(
              _, inst, 1 /* Acceleration Struct */, 2 /* Instance Id */,
              3 /* Primtive Id */, 4 /* Geometry Index */,
              KRayParamInvalidId /* Ray Flags */,
              KRayParamInvalidId /* Cull Mask */, 5 /* Hit Kind*/,
              6 /* SBT index */, KRayParamInvalidId /* SBT Offset */,
              KRayParamInvalidId /* SBT Stride */,
              KRayParamInvalidId /* SBT Record Offset */,
              KRayParamInvalidId /* SBT Record Stride */,
              KRayParamInvalidId /* Miss Index */, 7 /* Ray Origin */,
              8 /* Ray TMin */, 9 /* Ray Direction */, 10 /* Ray TMax */,
              KRayParamInvalidId /* Payload */, 11 /* Hit Object Attribute */))
        return error;

      break;
    }

    case spv::Op::OpHitObjectRecordHitNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 0)) return error;

      if (auto error = ValidateHitObjectInstructionCommonParameters(
              _, inst, 1 /* Acceleration Struct */, 2 /* Instance Id */,
              3 /* Primtive Id */, 4 /* Geometry Index */,
              KRayParamInvalidId /* Ray Flags */,
              KRayParamInvalidId /* Cull Mask */, 5 /* Hit Kind*/,
              KRayParamInvalidId /* SBT index */,
              KRayParamInvalidId /* SBT Offset */,
              KRayParamInvalidId /* SBT Stride */, 6 /* SBT Record Offset */,
              7 /* SBT Record Stride */, KRayParamInvalidId /* Miss Index */,
              8 /* Ray Origin */, 9 /* Ray TMin */, 10 /* Ray Direction */,
              11 /* Ray TMax */, KRayParamInvalidId /* Payload */,
              12 /* Hit Object Attribute */))
        return error;

      break;
    }

    case spv::Op::OpHitObjectTraceRayMotionNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 0)) return error;

      if (auto error = ValidateHitObjectInstructionCommonParameters(
              _, inst, 1 /* Acceleration Struct */,
              KRayParamInvalidId /* Instance Id */,
              KRayParamInvalidId /* Primtive Id */,
              KRayParamInvalidId /* Geometry Index */, 2 /* Ray Flags */,
              3 /* Cull Mask */, KRayParamInvalidId /* Hit Kind*/,
              KRayParamInvalidId /* SBT index */, 4 /* SBT Offset */,
              5 /* SBT Stride */, KRayParamInvalidId /* SBT Record Offset */,
              KRayParamInvalidId /* SBT Record Stride */, 6 /* Miss Index */,
              7 /* Ray Origin */, 8 /* Ray TMin */, 9 /* Ray Direction */,
              10 /* Ray TMax */, 12 /* Payload */,
              KRayParamInvalidId /* Hit Object Attribute */))
        return error;
      // Current Time
      const uint32_t current_time_id = _.GetOperandTypeId(inst, 11);
      if (!_.IsFloatScalarType(current_time_id, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Current Times must be a 32-bit float scalar type";
      }

      break;
    }

    case spv::Op::OpHitObjectTraceRayNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 0)) return error;

      if (auto error = ValidateHitObjectInstructionCommonParameters(
              _, inst, 1 /* Acceleration Struct */,
              KRayParamInvalidId /* Instance Id */,
              KRayParamInvalidId /* Primtive Id */,
              KRayParamInvalidId /* Geometry Index */, 2 /* Ray Flags */,
              3 /* Cull Mask */, KRayParamInvalidId /* Hit Kind*/,
              KRayParamInvalidId /* SBT index */, 4 /* SBT Offset */,
              5 /* SBT Stride */, KRayParamInvalidId /* SBT Record Offset */,
              KRayParamInvalidId /* SBT Record Stride */, 6 /* Miss Index */,
              7 /* Ray Origin */, 8 /* Ray TMin */, 9 /* Ray Direction */,
              10 /* Ray TMax */, 11 /* Payload */,
              KRayParamInvalidId /* Hit Object Attribute */))
        return error;
      break;
    }

    case spv::Op::OpReorderThreadWithHitObjectNV: {
      std::string opcode_name = spvOpcodeString(inst->opcode());
      _.function(inst->function()->id())
          ->RegisterExecutionModelLimitation(
              [opcode_name](spv::ExecutionModel model, std::string* message) {
                if (model != spv::ExecutionModel::RayGenerationKHR) {
                  if (message) {
                    *message = opcode_name +
                               " requires RayGenerationKHR execution model";
                  }
                  return false;
                }
                return true;
              });

      if (auto error = ValidateHitObjectPointer(_, inst, 0)) return error;

      if (inst->operands().size() > 1) {
        if (inst->operands().size() != 3) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Hint and Bits are optional together i.e "
                 << " Either both Hint and Bits should be provided or neither.";
        }

        // Validate the optional opreands Hint and Bits
        const uint32_t hint_id = _.GetOperandTypeId(inst, 1);
        if (!_.IsIntScalarType(hint_id, 32)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Hint must be a 32-bit int scalar";
        }
        const uint32_t bits_id = _.GetOperandTypeId(inst, 2);
        if (!_.IsIntScalarType(bits_id, 32)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "bits must be a 32-bit int scalar";
        }
      }
      break;
    }

    case spv::Op::OpReorderThreadWithHintNV: {
      std::string opcode_name = spvOpcodeString(inst->opcode());
      _.function(inst->function()->id())
          ->RegisterExecutionModelLimitation(
              [opcode_name](spv::ExecutionModel model, std::string* message) {
                if (model != spv::ExecutionModel::RayGenerationKHR) {
                  if (message) {
                    *message = opcode_name +
                               " requires RayGenerationKHR execution model";
                  }
                  return false;
                }
                return true;
              });

      const uint32_t hint_id = _.GetOperandTypeId(inst, 0);
      if (!_.IsIntScalarType(hint_id, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Hint must be a 32-bit int scalar";
      }

      const uint32_t bits_id = _.GetOperandTypeId(inst, 1);
      if (!_.IsIntScalarType(bits_id, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "bits must be a 32-bit int scalar";
      }
      break;
    }

    case spv::Op::OpHitObjectGetClusterIdNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 2)) return error;

      if (!_.IsIntScalarType(result_type, 32))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 32-bit integer type scalar as Result Type: "
               << spvOpcodeString(opcode);
      break;
    }

    case spv::Op::OpHitObjectGetSpherePositionNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 2)) return error;

      if (!_.IsFloatVectorType(result_type) ||
          _.GetDimension(result_type) != 3 ||
          _.GetBitWidth(result_type) != 32) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 32-bit floating point 2 component vector type as "
                  "Result Type: "
               << spvOpcodeString(opcode);
      }
      break;
    }

    case spv::Op::OpHitObjectGetSphereRadiusNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 2)) return error;

      if (!_.IsFloatScalarType(result_type, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 32-bit floating point scalar as Result Type: "
               << spvOpcodeString(opcode);
      }
      break;
    }

    case spv::Op::OpHitObjectGetLSSPositionsNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 2)) return error;

      auto result_id = _.FindDef(result_type);
      if ((result_id->opcode() != spv::Op::OpTypeArray) ||
          (GetArrayLength(_, result_id) != 2) ||
          !_.IsFloatVectorType(_.GetComponentType(result_type)) ||
          _.GetDimension(_.GetComponentType(result_type)) != 3) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 2 element array of 32-bit 3 component float point "
                  "vector as Result Type: "
               << spvOpcodeString(opcode);
      }
      break;
    }

    case spv::Op::OpHitObjectGetLSSRadiiNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 2)) return error;

      if (!_.IsFloatArrayType(result_type) ||
          (GetArrayLength(_, _.FindDef(result_type)) != 2) ||
          !_.IsFloatScalarType(_.GetComponentType(result_type))) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 2 element array of 32-bit floating point scalar as "
                  "Result Type: "
               << spvOpcodeString(opcode);
      }
      break;
    }

    case spv::Op::OpHitObjectIsSphereHitNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 2)) return error;

      if (!_.IsBoolScalarType(result_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected Boolean scalar as Result Type: "
               << spvOpcodeString(opcode);
      }
      break;
    }

    case spv::Op::OpHitObjectIsLSSHitNV: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointer(_, inst, 2)) return error;

      if (!_.IsBoolScalarType(result_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected Boolean scalar as Result Type: "
               << spvOpcodeString(opcode);
      }
      break;
    }

    default:
      break;
  }
  return SPV_SUCCESS;
}

spv_result_t RayReorderEXTPass(ValidationState_t& _, const Instruction* inst) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();

  auto RegisterOpcodeForValidModel = [](ValidationState_t& vs,
                                        const Instruction* rtinst) {
    std::string opcode_name = spvOpcodeString(rtinst->opcode());
    vs.function(rtinst->function()->id())
        ->RegisterExecutionModelLimitation(
            [opcode_name](spv::ExecutionModel model, std::string* message) {
              if (model != spv::ExecutionModel::RayGenerationKHR &&
                  model != spv::ExecutionModel::ClosestHitKHR &&
                  model != spv::ExecutionModel::MissKHR) {
                if (message) {
                  *message = opcode_name +
                             " requires RayGenerationKHR, ClosestHitKHR and "
                             "MissKHR execution models";
                }
                return false;
              }
              return true;
            });
    return;
  };

  switch (opcode) {
    case spv::Op::OpHitObjectIsMissEXT:
    case spv::Op::OpHitObjectIsHitEXT:
    case spv::Op::OpHitObjectIsEmptyEXT: {
      RegisterOpcodeForValidModel(_, inst);
      if (!_.IsBoolScalarType(result_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "expected Result Type to be bool scalar type";
      }

      if (auto error = ValidateHitObjectPointerEXT(_, inst, 2)) return error;
      break;
    }

    case spv::Op::OpHitObjectGetShaderRecordBufferHandleEXT: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointerEXT(_, inst, 2)) return error;

      if (!_.IsIntVectorType(result_type) ||
          (_.GetDimension(result_type) != 2) ||
          (_.GetBitWidth(result_type) != 32))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 32-bit integer type 2-component vector as Result "
                  "Type: "
               << spvOpcodeString(opcode);
      break;
    }

    case spv::Op::OpHitObjectGetHitKindEXT:
    case spv::Op::OpHitObjectGetPrimitiveIndexEXT:
    case spv::Op::OpHitObjectGetGeometryIndexEXT:
    case spv::Op::OpHitObjectGetInstanceIdEXT:
    case spv::Op::OpHitObjectGetInstanceCustomIndexEXT:
    case spv::Op::OpHitObjectGetShaderBindingTableRecordIndexEXT:
    case spv::Op::OpHitObjectGetRayFlagsEXT: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointerEXT(_, inst, 2)) return error;

      if (!_.IsIntScalarType(result_type, 32))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 32-bit integer type scalar as Result Type: "
               << spvOpcodeString(opcode);
      break;
    }

    case spv::Op::OpHitObjectGetCurrentTimeEXT:
    case spv::Op::OpHitObjectGetRayTMaxEXT:
    case spv::Op::OpHitObjectGetRayTMinEXT: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointerEXT(_, inst, 2)) return error;

      if (!_.IsFloatScalarType(result_type, 32))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 32-bit floating-point type scalar as Result Type: "
               << spvOpcodeString(opcode);
      break;
    }

    case spv::Op::OpHitObjectGetObjectToWorldEXT:
    case spv::Op::OpHitObjectGetWorldToObjectEXT: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointerEXT(_, inst, 2)) return error;

      uint32_t num_rows = 0;
      uint32_t num_cols = 0;
      uint32_t col_type = 0;
      uint32_t component_type = 0;

      if (!_.GetMatrixTypeInfo(result_type, &num_rows, &num_cols, &col_type,
                               &component_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "expected matrix type as Result Type: "
               << spvOpcodeString(opcode);
      }

      if (num_cols != 4) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "expected Result Type matrix to have a Column Count of 4"
               << spvOpcodeString(opcode);
      }

      if (!_.IsFloatScalarType(component_type) ||
          _.GetBitWidth(result_type) != 32 || num_rows != 3) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "expected Result Type matrix to have a Column Type of "
                  "3-component 32-bit float vectors: "
               << spvOpcodeString(opcode);
      }
      break;
    }

    case spv::Op::OpHitObjectGetObjectRayOriginEXT:
    case spv::Op::OpHitObjectGetObjectRayDirectionEXT:
    case spv::Op::OpHitObjectGetWorldRayDirectionEXT:
    case spv::Op::OpHitObjectGetWorldRayOriginEXT: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointerEXT(_, inst, 2)) return error;

      if (!_.IsFloatVectorType(result_type) ||
          (_.GetDimension(result_type) != 3) ||
          (_.GetBitWidth(result_type) != 32))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 32-bit floating-point type 3-component vector as "
                  "Result Type: "
               << spvOpcodeString(opcode);
      break;
    }

    case spv::Op::OpHitObjectGetIntersectionTriangleVertexPositionsEXT: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointerEXT(_, inst, 2)) return error;

      auto result_id = _.FindDef(result_type);
      if ((result_id->opcode() != spv::Op::OpTypeArray) ||
          (GetArrayLength(_, result_id) != 3) ||
          !_.IsFloatVectorType(_.GetComponentType(result_type)) ||
          _.GetDimension(_.GetComponentType(result_type)) != 3 ||
          _.GetBitWidth(_.GetComponentType(result_type)) != 32) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 3 element array of 32-bit 3 component float "
                  "vectors as Result Type: "
               << spvOpcodeString(opcode);
      }
      break;
    }

    case spv::Op::OpHitObjectGetAttributesEXT: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointerEXT(_, inst, 0)) return error;

      const uint32_t hit_object_attr_id = inst->GetOperandAs<uint32_t>(1);
      auto variable = _.FindDef(hit_object_attr_id);
      const auto var_opcode = variable->opcode();
      if (!variable || var_opcode != spv::Op::OpVariable ||
          variable->GetOperandAs<spv::StorageClass>(2) !=
              spv::StorageClass::HitObjectAttributeEXT) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Hit Object Attributes id must be a OpVariable of storage "
                  "class HitObjectAttributeEXT";
      }
      break;
    }

    case spv::Op::OpHitObjectSetShaderBindingTableRecordIndexEXT: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointerEXT(_, inst, 0)) return error;

      const uint32_t sbt_index_id = _.GetOperandTypeId(inst, 1);
      if (!_.IsIntScalarType(sbt_index_id, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "SBT Index must be a 32-bit integer scalar";
      }
      break;
    }

    case spv::Op::OpHitObjectExecuteShaderEXT: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointerEXT(_, inst, 0)) return error;

      const uint32_t payload_id = inst->GetOperandAs<uint32_t>(1);
      auto variable = _.FindDef(payload_id);
      const auto var_opcode = variable->opcode();
      if (!variable || var_opcode != spv::Op::OpVariable ||
          (variable->GetOperandAs<spv::StorageClass>(2) !=
               spv::StorageClass::RayPayloadKHR &&
           variable->GetOperandAs<spv::StorageClass>(2) !=
               spv::StorageClass::IncomingRayPayloadKHR)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Payload must be a OpVariable of storage "
                  "class RayPayloadKHR or IncomingRayPayloadKHR";
      }
      break;
    }

    case spv::Op::OpHitObjectRecordEmptyEXT: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointerEXT(_, inst, 0)) return error;
      break;
    }

    case spv::Op::OpHitObjectRecordFromQueryEXT: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointerEXT(_, inst, 0)) return error;
      if (auto error = ValidateRayQueryPointer(_, inst, 1)) return error;

      if (!_.HasCapability(spv::Capability::RayQueryKHR))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << ": requires RayQueryKHR capability";

      // Validate SBT Record Index (operand 2)
      const uint32_t sbt_record_index_id = _.GetOperandTypeId(inst, 2);
      if (!_.IsIntScalarType(sbt_record_index_id, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "SBT Record Index must be a 32-bit integer scalar";
      }

      // Validate Hit Object Attributes (operand 3)
      const uint32_t hit_object_attr_id = inst->GetOperandAs<uint32_t>(3);
      auto attr_variable = _.FindDef(hit_object_attr_id);
      const auto attr_var_opcode = attr_variable->opcode();
      if (!attr_variable || attr_var_opcode != spv::Op::OpVariable ||
          attr_variable->GetOperandAs<spv::StorageClass>(2) !=
              spv::StorageClass::HitObjectAttributeEXT) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Hit Object Attributes id must be a OpVariable of storage "
                  "class HitObjectAttributeEXT";
      }
      break;
    }

    case spv::Op::OpHitObjectRecordMissEXT: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointerEXT(_, inst, 0)) return error;

      // Ray Flags (operand 1)
      const uint32_t ray_flags_id = _.GetOperandTypeId(inst, 1);
      if (!_.IsIntScalarType(ray_flags_id, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Ray Flags must be a 32-bit int scalar";
      }

      // Miss Index (operand 2)
      const uint32_t miss_index = _.GetOperandTypeId(inst, 2);
      if (!_.IsUnsignedIntScalarType(miss_index) ||
          _.GetBitWidth(miss_index) != 32) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Miss Index must be a 32-bit unsigned int scalar";
      }

      // Ray Origin (operand 3)
      const uint32_t ray_origin = _.GetOperandTypeId(inst, 3);
      if (!_.IsFloatVectorType(ray_origin) || _.GetDimension(ray_origin) != 3 ||
          _.GetBitWidth(ray_origin) != 32) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Ray Origin must be a 32-bit float 3-component vector";
      }

      // Ray TMin (operand 4)
      const uint32_t ray_tmin = _.GetOperandTypeId(inst, 4);
      if (!_.IsFloatScalarType(ray_tmin, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Ray TMin must be a 32-bit float scalar";
      }

      // Ray Direction (operand 5)
      const uint32_t ray_direction = _.GetOperandTypeId(inst, 5);
      if (!_.IsFloatVectorType(ray_direction) ||
          _.GetDimension(ray_direction) != 3 ||
          _.GetBitWidth(ray_direction) != 32) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Ray Direction must be a 32-bit float 3-component vector";
      }

      // Ray TMax (operand 6)
      const uint32_t ray_tmax = _.GetOperandTypeId(inst, 6);
      if (!_.IsFloatScalarType(ray_tmax, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Ray TMax must be a 32-bit float scalar";
      }
      break;
    }

    case spv::Op::OpHitObjectRecordMissMotionEXT: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointerEXT(_, inst, 0)) return error;

      // Ray Flags (operand 1)
      const uint32_t ray_flags_id = _.GetOperandTypeId(inst, 1);
      if (!_.IsIntScalarType(ray_flags_id, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Ray Flags must be a 32-bit int scalar";
      }

      // Miss Index (operand 2)
      const uint32_t miss_index = _.GetOperandTypeId(inst, 2);
      if (!_.IsUnsignedIntScalarType(miss_index) ||
          _.GetBitWidth(miss_index) != 32) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Miss Index must be a 32-bit unsigned int scalar";
      }

      // Ray Origin (operand 3)
      const uint32_t ray_origin = _.GetOperandTypeId(inst, 3);
      if (!_.IsFloatVectorType(ray_origin) || _.GetDimension(ray_origin) != 3 ||
          _.GetBitWidth(ray_origin) != 32) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Ray Origin must be a 32-bit float 3-component vector";
      }

      // Ray TMin (operand 4)
      const uint32_t ray_tmin = _.GetOperandTypeId(inst, 4);
      if (!_.IsFloatScalarType(ray_tmin, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Ray TMin must be a 32-bit float scalar";
      }

      // Ray Direction (operand 5)
      const uint32_t ray_direction = _.GetOperandTypeId(inst, 5);
      if (!_.IsFloatVectorType(ray_direction) ||
          _.GetDimension(ray_direction) != 3 ||
          _.GetBitWidth(ray_direction) != 32) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Ray Direction must be a 32-bit float 3-component vector";
      }

      // Ray TMax (operand 6)
      const uint32_t ray_tmax = _.GetOperandTypeId(inst, 6);
      if (!_.IsFloatScalarType(ray_tmax, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Ray TMax must be a 32-bit float scalar";
      }

      // Current Time (operand 7)
      const uint32_t current_time_id = _.GetOperandTypeId(inst, 7);
      if (!_.IsFloatScalarType(current_time_id, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Current Time must be a 32-bit float scalar";
      }
      break;
    }

    case spv::Op::OpReorderThreadWithHintEXT: {
      std::string opcode_name = spvOpcodeString(inst->opcode());
      _.function(inst->function()->id())
          ->RegisterExecutionModelLimitation(
              [opcode_name](spv::ExecutionModel model, std::string* message) {
                if (model != spv::ExecutionModel::RayGenerationKHR) {
                  if (message) {
                    *message = opcode_name +
                               " requires RayGenerationKHR execution model";
                  }
                  return false;
                }
                return true;
              });

      const uint32_t hint_id = _.GetOperandTypeId(inst, 0);
      if (!_.IsIntScalarType(hint_id, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Hint must be a 32-bit int scalar";
      }

      const uint32_t bits_id = _.GetOperandTypeId(inst, 1);
      if (!_.IsIntScalarType(bits_id, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Bits must be a 32-bit int scalar";
      }
      break;
    }

    case spv::Op::OpReorderThreadWithHitObjectEXT: {
      std::string opcode_name = spvOpcodeString(inst->opcode());
      _.function(inst->function()->id())
          ->RegisterExecutionModelLimitation(
              [opcode_name](spv::ExecutionModel model, std::string* message) {
                if (model != spv::ExecutionModel::RayGenerationKHR) {
                  if (message) {
                    *message = opcode_name +
                               " requires RayGenerationKHR execution model";
                  }
                  return false;
                }
                return true;
              });

      if (auto error = ValidateHitObjectPointerEXT(_, inst, 0)) return error;

      if (inst->operands().size() > 1) {
        if (inst->operands().size() != 3) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Hint and Bits are optional together i.e "
                 << " Either both Hint and Bits should be provided or neither.";
        }

        // Validate the optional operands Hint and Bits
        const uint32_t hint_id = _.GetOperandTypeId(inst, 1);
        if (!_.IsIntScalarType(hint_id, 32)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Hint must be a 32-bit int scalar";
        }
        const uint32_t bits_id = _.GetOperandTypeId(inst, 2);
        if (!_.IsIntScalarType(bits_id, 32)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Bits must be a 32-bit int scalar";
        }
      }
      break;
    }

    case spv::Op::OpHitObjectTraceRayEXT: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointerEXT(_, inst, 0)) return error;

      if (auto error = ValidateHitObjectInstructionCommonParameters(
              _, inst, 1 /* Acceleration Struct */,
              KRayParamInvalidId /* Instance Id */,
              KRayParamInvalidId /* Primitive Id */,
              KRayParamInvalidId /* Geometry Index */, 2 /* Ray Flags */,
              3 /* Cull Mask */, KRayParamInvalidId /* Hit Kind*/,
              KRayParamInvalidId /* SBT index */, 4 /* SBT Offset */,
              5 /* SBT Stride */, KRayParamInvalidId /* SBT Record Offset */,
              KRayParamInvalidId /* SBT Record Stride */, 6 /* Miss Index */,
              7 /* Ray Origin */, 8 /* Ray TMin */, 9 /* Ray Direction */,
              10 /* Ray TMax */, 11 /* Payload */,
              KRayParamInvalidId /* Hit Object Attribute */))
        return error;
      break;
    }

    case spv::Op::OpHitObjectTraceRayMotionEXT: {
      RegisterOpcodeForValidModel(_, inst);
      if (auto error = ValidateHitObjectPointerEXT(_, inst, 0)) return error;

      if (auto error = ValidateHitObjectInstructionCommonParameters(
              _, inst, 1 /* Acceleration Struct */,
              KRayParamInvalidId /* Instance Id */,
              KRayParamInvalidId /* Primitive Id */,
              KRayParamInvalidId /* Geometry Index */, 2 /* Ray Flags */,
              3 /* Cull Mask */, KRayParamInvalidId /* Hit Kind*/,
              KRayParamInvalidId /* SBT index */, 4 /* SBT Offset */,
              5 /* SBT Stride */, KRayParamInvalidId /* SBT Record Offset */,
              KRayParamInvalidId /* SBT Record Stride */, 6 /* Miss Index */,
              7 /* Ray Origin */, 8 /* Ray TMin */, 9 /* Ray Direction */,
              10 /* Ray TMax */, 12 /* Payload */,
              KRayParamInvalidId /* Hit Object Attribute */))
        return error;

      // Current Time (operand 11)
      const uint32_t current_time_id = _.GetOperandTypeId(inst, 11);
      if (!_.IsFloatScalarType(current_time_id, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Current Time must be a 32-bit float scalar";
      }
      break;
    }

    case spv::Op::OpHitObjectReorderExecuteShaderEXT: {
      std::string opcode_name = spvOpcodeString(inst->opcode());
      _.function(inst->function()->id())
          ->RegisterExecutionModelLimitation(
              [opcode_name](spv::ExecutionModel model, std::string* message) {
                if (model != spv::ExecutionModel::RayGenerationKHR) {
                  if (message) {
                    *message = opcode_name +
                               " requires RayGenerationKHR execution model";
                  }
                  return false;
                }
                return true;
              });

      if (auto error = ValidateHitObjectPointerEXT(_, inst, 0)) return error;

      // Validate Payload (operand 1)
      const uint32_t payload_id = inst->GetOperandAs<uint32_t>(1);
      auto variable = _.FindDef(payload_id);
      const auto var_opcode = variable->opcode();
      if (!variable || var_opcode != spv::Op::OpVariable ||
          (variable->GetOperandAs<spv::StorageClass>(2) !=
               spv::StorageClass::RayPayloadKHR &&
           variable->GetOperandAs<spv::StorageClass>(2) !=
               spv::StorageClass::IncomingRayPayloadKHR)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Payload must be a OpVariable of storage "
                  "class RayPayloadKHR or IncomingRayPayloadKHR";
      }

      // Check for optional Hint and Bits (operands 2 and 3)
      if (inst->operands().size() > 2) {
        if (inst->operands().size() != 4) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Hint and Bits are optional together i.e "
                 << " Either both Hint and Bits should be provided or neither.";
        }

        // Validate optional Hint and Bits
        const uint32_t hint_id = _.GetOperandTypeId(inst, 2);
        if (!_.IsIntScalarType(hint_id, 32)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Hint must be a 32-bit int scalar";
        }
        const uint32_t bits_id = _.GetOperandTypeId(inst, 3);
        if (!_.IsIntScalarType(bits_id, 32)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Bits must be a 32-bit int scalar";
        }
      }
      break;
    }

    case spv::Op::OpHitObjectTraceReorderExecuteEXT: {
      std::string opcode_name = spvOpcodeString(inst->opcode());
      _.function(inst->function()->id())
          ->RegisterExecutionModelLimitation(
              [opcode_name](spv::ExecutionModel model, std::string* message) {
                if (model != spv::ExecutionModel::RayGenerationKHR) {
                  if (message) {
                    *message = opcode_name +
                               " requires RayGenerationKHR execution model";
                  }
                  return false;
                }
                return true;
              });

      if (auto error = ValidateHitObjectPointerEXT(_, inst, 0)) return error;

      // Validate base trace ray parameters (operands 1-11)
      if (auto error = ValidateHitObjectInstructionCommonParameters(
              _, inst, 1 /* Acceleration Struct */,
              KRayParamInvalidId /* Instance Id */,
              KRayParamInvalidId /* Primitive Id */,
              KRayParamInvalidId /* Geometry Index */, 2 /* Ray Flags */,
              3 /* Cull Mask */, KRayParamInvalidId /* Hit Kind*/,
              KRayParamInvalidId /* SBT index */, 4 /* SBT Offset */,
              5 /* SBT Stride */, KRayParamInvalidId /* SBT Record Offset */,
              KRayParamInvalidId /* SBT Record Stride */, 6 /* Miss Index */,
              7 /* Ray Origin */, 8 /* Ray TMin */, 9 /* Ray Direction */,
              10 /* Ray TMax */, 11 /* Payload */,
              KRayParamInvalidId /* Hit Object Attribute */))
        return error;

      // Check for optional Hint and Bits (operands 12 and 13)
      if (inst->operands().size() > 12) {
        if (inst->operands().size() != 14) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Hint and Bits are optional together i.e "
                 << " Either both Hint and Bits should be provided or neither.";
        }

        // Validate optional Hint and Bits
        const uint32_t hint_id = _.GetOperandTypeId(inst, 12);
        if (!_.IsIntScalarType(hint_id, 32)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Hint must be a 32-bit int scalar";
        }
        const uint32_t bits_id = _.GetOperandTypeId(inst, 13);
        if (!_.IsIntScalarType(bits_id, 32)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Bits must be a 32-bit int scalar";
        }
      }
      break;
    }

    case spv::Op::OpHitObjectTraceMotionReorderExecuteEXT: {
      std::string opcode_name = spvOpcodeString(inst->opcode());
      _.function(inst->function()->id())
          ->RegisterExecutionModelLimitation(
              [opcode_name](spv::ExecutionModel model, std::string* message) {
                if (model != spv::ExecutionModel::RayGenerationKHR) {
                  if (message) {
                    *message = opcode_name +
                               " requires RayGenerationKHR execution model";
                  }
                  return false;
                }
                return true;
              });

      if (auto error = ValidateHitObjectPointerEXT(_, inst, 0)) return error;

      // Validate base trace ray parameters (operands 1-12)
      if (auto error = ValidateHitObjectInstructionCommonParameters(
              _, inst, 1 /* Acceleration Struct */,
              KRayParamInvalidId /* Instance Id */,
              KRayParamInvalidId /* Primitive Id */,
              KRayParamInvalidId /* Geometry Index */, 2 /* Ray Flags */,
              3 /* Cull Mask */, KRayParamInvalidId /* Hit Kind*/,
              KRayParamInvalidId /* SBT index */, 4 /* SBT Offset */,
              5 /* SBT Stride */, KRayParamInvalidId /* SBT Record Offset */,
              KRayParamInvalidId /* SBT Record Stride */, 6 /* Miss Index */,
              7 /* Ray Origin */, 8 /* Ray TMin */, 9 /* Ray Direction */,
              10 /* Ray TMax */, 12 /* Payload */,
              KRayParamInvalidId /* Hit Object Attribute */))
        return error;

      // Current Time (operand 11)
      const uint32_t current_time_id = _.GetOperandTypeId(inst, 11);
      if (!_.IsFloatScalarType(current_time_id, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Current Time must be a 32-bit float scalar";
      }

      // Check for optional Hint and Bits (operands 13 and 14)
      if (inst->operands().size() > 13) {
        if (inst->operands().size() != 15) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Hint and Bits are optional together i.e "
                 << " Either both Hint and Bits should be provided or neither.";
        }

        // Validate optional Hint and Bits
        const uint32_t hint_id = _.GetOperandTypeId(inst, 13);
        if (!_.IsIntScalarType(hint_id, 32)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Hint must be a 32-bit int scalar";
        }
        const uint32_t bits_id = _.GetOperandTypeId(inst, 14);
        if (!_.IsIntScalarType(bits_id, 32)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Bits must be a 32-bit int scalar";
        }
      }
      break;
    }

    default:
      break;
  }
  return SPV_SUCCESS;
}
}  // namespace val
}  // namespace spvtools
