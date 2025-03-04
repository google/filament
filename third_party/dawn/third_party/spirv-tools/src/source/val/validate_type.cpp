// Copyright (c) 2018 Google LLC.
// Modifications Copyright (C) 2024 Advanced Micro Devices, Inc. All rights
// reserved.
// Copyright (c) 2024 NVIDIA Corporation
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

// Ensures type declarations are unique unless allowed by the specification.

#include "source/opcode.h"
#include "source/spirv_target_env.h"
#include "source/val/instruction.h"
#include "source/val/validate.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

// Validates that type declarations are unique, unless multiple declarations
// of the same data type are allowed by the specification.
// (see section 2.8 Types and Variables)
// Doesn't do anything if SPV_VAL_ignore_type_decl_unique was declared in the
// module.
spv_result_t ValidateUniqueness(ValidationState_t& _, const Instruction* inst) {
  if (_.HasExtension(Extension::kSPV_VALIDATOR_ignore_type_decl_unique))
    return SPV_SUCCESS;

  const auto opcode = inst->opcode();
  if (opcode != spv::Op::OpTypeArray && opcode != spv::Op::OpTypeRuntimeArray &&
      opcode != spv::Op::OpTypeNodePayloadArrayAMDX &&
      opcode != spv::Op::OpTypeStruct && opcode != spv::Op::OpTypePointer &&
      opcode != spv::Op::OpTypeUntypedPointerKHR &&
      !_.RegisterUniqueTypeDeclaration(inst)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Duplicate non-aggregate type declarations are not allowed. "
              "Opcode: "
           << spvOpcodeString(opcode) << " id: " << inst->id();
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateTypeInt(ValidationState_t& _, const Instruction* inst) {
  // Validates that the number of bits specified for an Int type is valid.
  // Scalar integer types can be parameterized only with 32-bits.
  // Int8, Int16, and Int64 capabilities allow using 8-bit, 16-bit, and 64-bit
  // integers, respectively.
  auto num_bits = inst->GetOperandAs<const uint32_t>(1);
  if (num_bits != 32) {
    if (num_bits == 8) {
      if (_.features().declare_int8_type) {
        return SPV_SUCCESS;
      }
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Using an 8-bit integer type requires the Int8 capability,"
                " or an extension that explicitly enables 8-bit integers.";
    } else if (num_bits == 16) {
      if (_.features().declare_int16_type) {
        return SPV_SUCCESS;
      }
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Using a 16-bit integer type requires the Int16 capability,"
                " or an extension that explicitly enables 16-bit integers.";
    } else if (num_bits == 64) {
      if (_.HasCapability(spv::Capability::Int64)) {
        return SPV_SUCCESS;
      }
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Using a 64-bit integer type requires the Int64 capability.";
    } else {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Invalid number of bits (" << num_bits
             << ") used for OpTypeInt.";
    }
  }

  const auto signedness_index = 2;
  const auto signedness = inst->GetOperandAs<uint32_t>(signedness_index);
  if (signedness != 0 && signedness != 1) {
    return _.diag(SPV_ERROR_INVALID_VALUE, inst)
           << "OpTypeInt has invalid signedness:";
  }

  // SPIR-V Spec 2.16.3: Validation Rules for Kernel Capabilities: The
  // Signedness in OpTypeInt must always be 0.
  if (spv::Op::OpTypeInt == inst->opcode() &&
      _.HasCapability(spv::Capability::Kernel) &&
      inst->GetOperandAs<uint32_t>(2) != 0u) {
    return _.diag(SPV_ERROR_INVALID_BINARY, inst)
           << "The Signedness in OpTypeInt "
              "must always be 0 when Kernel "
              "capability is used.";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateTypeFloat(ValidationState_t& _, const Instruction* inst) {
  // Validates that the number of bits specified for an Int type is valid.
  // Scalar integer types can be parameterized only with 32-bits.
  // Int8, Int16, and Int64 capabilities allow using 8-bit, 16-bit, and 64-bit
  // integers, respectively.
  auto num_bits = inst->GetOperandAs<const uint32_t>(1);
  if (num_bits == 32) {
    return SPV_SUCCESS;
  }
  if (num_bits == 16) {
    if (_.features().declare_float16_type) {
      return SPV_SUCCESS;
    }
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Using a 16-bit floating point "
           << "type requires the Float16 or Float16Buffer capability,"
              " or an extension that explicitly enables 16-bit floating point.";
  }
  if (num_bits == 64) {
    if (_.HasCapability(spv::Capability::Float64)) {
      return SPV_SUCCESS;
    }
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Using a 64-bit floating point "
           << "type requires the Float64 capability.";
  }
  return _.diag(SPV_ERROR_INVALID_DATA, inst)
         << "Invalid number of bits (" << num_bits << ") used for OpTypeFloat.";
}

spv_result_t ValidateTypeVector(ValidationState_t& _, const Instruction* inst) {
  const auto component_index = 1;
  const auto component_id = inst->GetOperandAs<uint32_t>(component_index);
  const auto component_type = _.FindDef(component_id);
  if (!component_type || !spvOpcodeIsScalarType(component_type->opcode())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypeVector Component Type <id> " << _.getIdName(component_id)
           << " is not a scalar type.";
  }

  // Validates that the number of components in the vector is valid.
  // Vector types can only be parameterized as having 2, 3, or 4 components.
  // If the Vector16 capability is added, 8 and 16 components are also allowed.
  auto num_components = inst->GetOperandAs<const uint32_t>(2);
  if (num_components == 2 || num_components == 3 || num_components == 4) {
    return SPV_SUCCESS;
  } else if (num_components == 8 || num_components == 16) {
    if (_.HasCapability(spv::Capability::Vector16)) {
      return SPV_SUCCESS;
    }
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Having " << num_components << " components for "
           << spvOpcodeString(inst->opcode())
           << " requires the Vector16 capability";
  } else {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Illegal number of components (" << num_components << ") for "
           << spvOpcodeString(inst->opcode());
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateTypeCooperativeVectorNV(ValidationState_t& _,
                                             const Instruction* inst) {
  const auto component_index = 1;
  const auto component_type_id = inst->GetOperandAs<uint32_t>(component_index);
  const auto component_type = _.FindDef(component_type_id);
  if (!component_type || (spv::Op::OpTypeFloat != component_type->opcode() &&
                          spv::Op::OpTypeInt != component_type->opcode())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypeCooperativeVectorNV Component Type <id> "
           << _.getIdName(component_type_id)
           << " is not a scalar numerical type.";
  }

  const auto num_components_index = 2;
  const auto num_components_id =
      inst->GetOperandAs<uint32_t>(num_components_index);
  const auto num_components = _.FindDef(num_components_id);
  if (!num_components || !spvOpcodeIsConstant(num_components->opcode())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypeCooperativeVectorNV component count <id> "
           << _.getIdName(num_components_id)
           << " is not a scalar constant type.";
  }

  // NOTE: Check the initialiser value of the constant
  const auto const_inst = num_components->words();
  const auto const_result_type_index = 1;
  const auto const_result_type = _.FindDef(const_inst[const_result_type_index]);
  if (!const_result_type || spv::Op::OpTypeInt != const_result_type->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypeCooperativeVectorNV component count <id> "
           << _.getIdName(num_components_id)
           << " is not a constant integer type.";
  }

  int64_t num_components_value;
  if (_.EvalConstantValInt64(num_components_id, &num_components_value)) {
    auto& type_words = const_result_type->words();
    const bool is_signed = type_words[3] > 0;
    if (num_components_value == 0 || (num_components_value < 0 && is_signed)) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpTypeCooperativeVectorNV component count <id> "
             << _.getIdName(num_components_id)
             << " default value must be at least 1: found "
             << num_components_value;
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateTypeMatrix(ValidationState_t& _, const Instruction* inst) {
  const auto column_type_index = 1;
  const auto column_type_id = inst->GetOperandAs<uint32_t>(column_type_index);
  const auto column_type = _.FindDef(column_type_id);
  if (!column_type || spv::Op::OpTypeVector != column_type->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Columns in a matrix must be of type vector.";
  }

  // Trace back once more to find out the type of components in the vector.
  // Operand 1 is the <id> of the type of data in the vector.
  const auto comp_type_id = column_type->GetOperandAs<uint32_t>(1);
  auto comp_type_instruction = _.FindDef(comp_type_id);
  if (comp_type_instruction->opcode() != spv::Op::OpTypeFloat) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst) << "Matrix types can only be "
                                                   "parameterized with "
                                                   "floating-point types.";
  }

  // Validates that the matrix has 2,3, or 4 columns.
  auto num_cols = inst->GetOperandAs<const uint32_t>(2);
  if (num_cols != 2 && num_cols != 3 && num_cols != 4) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst) << "Matrix types can only be "
                                                   "parameterized as having "
                                                   "only 2, 3, or 4 columns.";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateTypeArray(ValidationState_t& _, const Instruction* inst) {
  const auto element_type_index = 1;
  const auto element_type_id = inst->GetOperandAs<uint32_t>(element_type_index);
  const auto element_type = _.FindDef(element_type_id);
  if (!element_type || !spvOpcodeGeneratesType(element_type->opcode())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypeArray Element Type <id> " << _.getIdName(element_type_id)
           << " is not a type.";
  }

  if (element_type->opcode() == spv::Op::OpTypeVoid) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypeArray Element Type <id> " << _.getIdName(element_type_id)
           << " is a void type.";
  }

  if (_.HasCapability(spv::Capability::Shader)) {
    if (element_type->opcode() == spv::Op::OpTypeStruct &&
        (_.HasDecoration(element_type->id(), spv::Decoration::Block) ||
         _.HasDecoration(element_type->id(), spv::Decoration::BufferBlock))) {
      if (_.HasDecoration(inst->id(), spv::Decoration::ArrayStride)) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "Array containing a Block or BufferBlock must not be "
                  "decorated with ArrayStride";
      }
    }
  }

  if (spvIsVulkanEnv(_.context()->target_env) &&
      element_type->opcode() == spv::Op::OpTypeRuntimeArray) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << _.VkErrorID(4680) << "OpTypeArray Element Type <id> "
           << _.getIdName(element_type_id) << " is not valid in "
           << spvLogStringForEnv(_.context()->target_env) << " environments.";
  }

  const auto length_index = 2;
  const auto length_id = inst->GetOperandAs<uint32_t>(length_index);
  const auto length = _.FindDef(length_id);
  if (!length || !spvOpcodeIsConstant(length->opcode())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypeArray Length <id> " << _.getIdName(length_id)
           << " is not a scalar constant type.";
  }

  // NOTE: Check the initialiser value of the constant
  const auto const_inst = length->words();
  const auto const_result_type_index = 1;
  const auto const_result_type = _.FindDef(const_inst[const_result_type_index]);
  if (!const_result_type || spv::Op::OpTypeInt != const_result_type->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypeArray Length <id> " << _.getIdName(length_id)
           << " is not a constant integer type.";
  }

  int64_t length_value;
  if (_.EvalConstantValInt64(length_id, &length_value)) {
    auto& type_words = const_result_type->words();
    const bool is_signed = type_words[3] > 0;
    if (length_value == 0 || (length_value < 0 && is_signed)) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpTypeArray Length <id> " << _.getIdName(length_id)
             << " default value must be at least 1: found " << length_value;
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateTypeRuntimeArray(ValidationState_t& _,
                                      const Instruction* inst) {
  const auto element_type_index = 1;
  const auto element_id = inst->GetOperandAs<uint32_t>(element_type_index);
  const auto element_type = _.FindDef(element_id);
  if (!element_type || !spvOpcodeGeneratesType(element_type->opcode())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypeRuntimeArray Element Type <id> " << _.getIdName(element_id)
           << " is not a type.";
  }

  if (element_type->opcode() == spv::Op::OpTypeVoid) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypeRuntimeArray Element Type <id> " << _.getIdName(element_id)
           << " is a void type.";
  }

  if (_.HasCapability(spv::Capability::Shader)) {
    if (element_type->opcode() == spv::Op::OpTypeStruct &&
        (_.HasDecoration(element_type->id(), spv::Decoration::Block) ||
         _.HasDecoration(element_type->id(), spv::Decoration::BufferBlock))) {
      if (_.HasDecoration(inst->id(), spv::Decoration::ArrayStride)) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "Array containing a Block or BufferBlock must not be "
                  "decorated with ArrayStride";
      }
    }
  }

  if (spvIsVulkanEnv(_.context()->target_env) &&
      element_type->opcode() == spv::Op::OpTypeRuntimeArray) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << _.VkErrorID(4680) << "OpTypeRuntimeArray Element Type <id> "
           << _.getIdName(element_id) << " is not valid in "
           << spvLogStringForEnv(_.context()->target_env) << " environments.";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateTypeStruct(ValidationState_t& _, const Instruction* inst) {
  const uint32_t struct_id = inst->GetOperandAs<uint32_t>(0);
  for (size_t member_type_index = 1;
       member_type_index < inst->operands().size(); ++member_type_index) {
    auto member_type_id = inst->GetOperandAs<uint32_t>(member_type_index);
    if (member_type_id == inst->id()) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Structure members may not be self references";
    }

    auto member_type = _.FindDef(member_type_id);
    if (!member_type || !spvOpcodeGeneratesType(member_type->opcode())) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpTypeStruct Member Type <id> " << _.getIdName(member_type_id)
             << " is not a type.";
    }
    if (member_type->opcode() == spv::Op::OpTypeVoid) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Structures cannot contain a void type.";
    }
    if (spv::Op::OpTypeStruct == member_type->opcode() &&
        _.IsStructTypeWithBuiltInMember(member_type_id)) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Structure <id> " << _.getIdName(member_type_id)
             << " contains members with BuiltIn decoration. Therefore this "
             << "structure may not be contained as a member of another "
             << "structure "
             << "type. Structure <id> " << _.getIdName(struct_id)
             << " contains structure <id> " << _.getIdName(member_type_id)
             << ".";
    }

    if (spvIsVulkanEnv(_.context()->target_env) &&
        member_type->opcode() == spv::Op::OpTypeRuntimeArray) {
      const bool is_last_member =
          member_type_index == inst->operands().size() - 1;
      if (!is_last_member) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << _.VkErrorID(4680) << "In "
               << spvLogStringForEnv(_.context()->target_env)
               << ", OpTypeRuntimeArray must only be used for the last member "
                  "of an OpTypeStruct";
      }

      if (!_.HasDecoration(inst->id(), spv::Decoration::Block) &&
          !_.HasDecoration(inst->id(), spv::Decoration::BufferBlock)) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << _.VkErrorID(4680)
               << spvLogStringForEnv(_.context()->target_env)
               << ", OpTypeStruct containing an OpTypeRuntimeArray "
               << "must be decorated with Block or BufferBlock.";
      }
    }
  }

  bool has_nested_blockOrBufferBlock_struct = false;
  // Struct members start at word 2 of OpTypeStruct instruction.
  for (size_t word_i = 2; word_i < inst->words().size(); ++word_i) {
    auto member = inst->word(word_i);
    if (_.ContainsType(
            member,
            [&_](const Instruction* type_inst) {
              if (type_inst->opcode() == spv::Op::OpTypeStruct &&
                  (_.HasDecoration(type_inst->id(), spv::Decoration::Block) ||
                   _.HasDecoration(type_inst->id(),
                                   spv::Decoration::BufferBlock))) {
                return true;
              }
              return false;
            },
            /* traverse_all_types = */ false)) {
      has_nested_blockOrBufferBlock_struct = true;
      break;
    }
  }

  _.SetHasNestedBlockOrBufferBlockStruct(inst->id(),
                                         has_nested_blockOrBufferBlock_struct);
  if (_.GetHasNestedBlockOrBufferBlockStruct(inst->id()) &&
      (_.HasDecoration(inst->id(), spv::Decoration::BufferBlock) ||
       _.HasDecoration(inst->id(), spv::Decoration::Block))) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "rules: A Block or BufferBlock cannot be nested within another "
              "Block or BufferBlock. ";
  }

  std::unordered_set<uint32_t> built_in_members;
  for (auto decoration : _.id_decorations(struct_id)) {
    if (decoration.dec_type() == spv::Decoration::BuiltIn &&
        decoration.struct_member_index() != Decoration::kInvalidMember) {
      built_in_members.insert(decoration.struct_member_index());
    }
  }
  int num_struct_members = static_cast<int>(inst->operands().size() - 1);
  int num_builtin_members = static_cast<int>(built_in_members.size());
  if (num_builtin_members > 0 && num_builtin_members != num_struct_members) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "When BuiltIn decoration is applied to a structure-type member, "
           << "all members of that structure type must also be decorated with "
           << "BuiltIn (No allowed mixing of built-in variables and "
           << "non-built-in variables within a single structure). Structure id "
           << struct_id << " does not meet this requirement.";
  }
  if (num_builtin_members > 0) {
    _.RegisterStructTypeWithBuiltInMember(struct_id);
  }

  const auto isOpaqueType = [&_](const Instruction* opaque_inst) {
    auto opcode = opaque_inst->opcode();
    if (_.HasCapability(spv::Capability::BindlessTextureNV) &&
        (opcode == spv::Op::OpTypeImage || opcode == spv::Op::OpTypeSampler ||
         opcode == spv::Op::OpTypeSampledImage)) {
      return false;
    } else if (spvOpcodeIsBaseOpaqueType(opcode)) {
      return true;
    }
    return false;
  };

  if (spvIsVulkanEnv(_.context()->target_env) &&
      !_.options()->before_hlsl_legalization &&
      _.ContainsType(inst->id(), isOpaqueType)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << _.VkErrorID(4667) << "In "
           << spvLogStringForEnv(_.context()->target_env)
           << ", OpTypeStruct must not contain an opaque type.";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateTypePointer(ValidationState_t& _,
                                 const Instruction* inst) {
  auto type_id = inst->GetOperandAs<uint32_t>(2);
  auto type = _.FindDef(type_id);
  if (!type || !spvOpcodeGeneratesType(type->opcode())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypePointer Type <id> " << _.getIdName(type_id)
           << " is not a type.";
  }
  // See if this points to a storage image.
  const auto storage_class = inst->GetOperandAs<spv::StorageClass>(1);
  if (storage_class == spv::StorageClass::UniformConstant) {
    // Unpack an optional level of arraying.
    if (type->opcode() == spv::Op::OpTypeArray ||
        type->opcode() == spv::Op::OpTypeRuntimeArray) {
      type_id = type->GetOperandAs<uint32_t>(1);
      type = _.FindDef(type_id);
    }
    if (type->opcode() == spv::Op::OpTypeImage) {
      const auto sampled = type->GetOperandAs<uint32_t>(6);
      // 2 indicates this image is known to be be used without a sampler, i.e.
      // a storage image.
      if (sampled == 2) _.RegisterPointerToStorageImage(inst->id());
    }
  }

  if (!_.IsValidStorageClass(storage_class)) {
    return _.diag(SPV_ERROR_INVALID_BINARY, inst)
           << _.VkErrorID(4643)
           << "Invalid storage class for target environment";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateTypeFunction(ValidationState_t& _,
                                  const Instruction* inst) {
  const auto return_type_id = inst->GetOperandAs<uint32_t>(1);
  const auto return_type = _.FindDef(return_type_id);
  if (!return_type || !spvOpcodeGeneratesType(return_type->opcode())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypeFunction Return Type <id> " << _.getIdName(return_type_id)
           << " is not a type.";
  }
  size_t num_args = 0;
  for (size_t param_type_index = 2; param_type_index < inst->operands().size();
       ++param_type_index, ++num_args) {
    const auto param_id = inst->GetOperandAs<uint32_t>(param_type_index);
    const auto param_type = _.FindDef(param_id);
    if (!param_type || !spvOpcodeGeneratesType(param_type->opcode())) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpTypeFunction Parameter Type <id> " << _.getIdName(param_id)
             << " is not a type.";
    }

    if (param_type->opcode() == spv::Op::OpTypeVoid) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpTypeFunction Parameter Type <id> " << _.getIdName(param_id)
             << " cannot be OpTypeVoid.";
    }
  }
  const uint32_t num_function_args_limit =
      _.options()->universal_limits_.max_function_args;
  if (num_args > num_function_args_limit) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypeFunction may not take more than "
           << num_function_args_limit << " arguments. OpTypeFunction <id> "
           << _.getIdName(inst->GetOperandAs<uint32_t>(0)) << " has "
           << num_args << " arguments.";
  }

  // The only valid uses of OpTypeFunction are in an OpFunction, debugging, or
  // decoration instruction.
  for (auto& pair : inst->uses()) {
    const auto* use = pair.first;
    if (use->opcode() != spv::Op::OpFunction &&
        !spvOpcodeIsDebug(use->opcode()) && !use->IsNonSemantic() &&
        !spvOpcodeIsDecoration(use->opcode())) {
      return _.diag(SPV_ERROR_INVALID_ID, use)
             << "Invalid use of function type result id "
             << _.getIdName(inst->id()) << ".";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateTypeForwardPointer(ValidationState_t& _,
                                        const Instruction* inst) {
  const auto pointer_type_id = inst->GetOperandAs<uint32_t>(0);
  const auto pointer_type_inst = _.FindDef(pointer_type_id);
  if (pointer_type_inst->opcode() != spv::Op::OpTypePointer) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Pointer type in OpTypeForwardPointer is not a pointer type.";
  }

  const auto storage_class = inst->GetOperandAs<spv::StorageClass>(1);
  if (storage_class != pointer_type_inst->GetOperandAs<spv::StorageClass>(1)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Storage class in OpTypeForwardPointer does not match the "
           << "pointer definition.";
  }

  const auto pointee_type_id = pointer_type_inst->GetOperandAs<uint32_t>(2);
  const auto pointee_type = _.FindDef(pointee_type_id);
  if (!pointee_type || pointee_type->opcode() != spv::Op::OpTypeStruct) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Forward pointers must point to a structure";
  }

  if (spvIsVulkanEnv(_.context()->target_env)) {
    if (storage_class != spv::StorageClass::PhysicalStorageBuffer) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << _.VkErrorID(4711)
             << "In Vulkan, OpTypeForwardPointer must have "
             << "a storage class of PhysicalStorageBuffer.";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateTypeCooperativeMatrix(ValidationState_t& _,
                                           const Instruction* inst) {
  const auto component_type_index = 1;
  const auto component_type_id =
      inst->GetOperandAs<uint32_t>(component_type_index);
  const auto component_type = _.FindDef(component_type_id);
  if (!component_type || (spv::Op::OpTypeFloat != component_type->opcode() &&
                          spv::Op::OpTypeInt != component_type->opcode())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypeCooperativeMatrix Component Type <id> "
           << _.getIdName(component_type_id)
           << " is not a scalar numerical type.";
  }

  const auto scope_index = 2;
  const auto scope_id = inst->GetOperandAs<uint32_t>(scope_index);
  const auto scope = _.FindDef(scope_id);
  if (!scope || !_.IsIntScalarType(scope->type_id()) ||
      !spvOpcodeIsConstant(scope->opcode())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypeCooperativeMatrix Scope <id> " << _.getIdName(scope_id)
           << " is not a constant instruction with scalar integer type.";
  }

  const auto rows_index = 3;
  const auto rows_id = inst->GetOperandAs<uint32_t>(rows_index);
  const auto rows = _.FindDef(rows_id);
  if (!rows || !_.IsIntScalarType(rows->type_id()) ||
      !spvOpcodeIsConstant(rows->opcode())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypeCooperativeMatrix Rows <id> " << _.getIdName(rows_id)
           << " is not a constant instruction with scalar integer type.";
  }

  const auto cols_index = 4;
  const auto cols_id = inst->GetOperandAs<uint32_t>(cols_index);
  const auto cols = _.FindDef(cols_id);
  if (!cols || !_.IsIntScalarType(cols->type_id()) ||
      !spvOpcodeIsConstant(cols->opcode())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpTypeCooperativeMatrix Cols <id> " << _.getIdName(cols_id)
           << " is not a constant instruction with scalar integer type.";
  }

  if (inst->opcode() == spv::Op::OpTypeCooperativeMatrixKHR) {
    const auto use_index = 5;
    const auto use_id = inst->GetOperandAs<uint32_t>(use_index);
    const auto use = _.FindDef(use_id);
    if (!use || !_.IsIntScalarType(use->type_id()) ||
        !spvOpcodeIsConstant(use->opcode())) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpTypeCooperativeMatrixKHR Use <id> " << _.getIdName(use_id)
             << " is not a constant instruction with scalar integer type.";
    }
  }

  uint64_t scope_value;
  if (_.EvalConstantValUint64(scope_id, &scope_value)) {
    if (scope_value == static_cast<uint32_t>(spv::Scope::Workgroup)) {
      for (auto entry_point_id : _.entry_points()) {
        if (!_.EntryPointHasLocalSizeOrId(entry_point_id)) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << "OpTypeCooperativeMatrixKHR with ScopeWorkgroup "
                 << "used without specifying LocalSize or LocalSizeId "
                 << "for entry point <id> " << _.getIdName(entry_point_id);
        }
        const auto local_size = _.EntryPointLocalSizeOrId(entry_point_id);
        const auto mode = local_size->GetOperandAs<spv::ExecutionMode>(1);
        if (mode == spv::ExecutionMode::LocalSizeId) {
          uint32_t local_size_ids[3] = {
              local_size->GetOperandAs<uint32_t>(2),
              local_size->GetOperandAs<uint32_t>(3),
              local_size->GetOperandAs<uint32_t>(4),
          };
          for (auto id : local_size_ids) {
            if (_.FindDef(id) > inst) {
              return _.diag(SPV_ERROR_INVALID_ID, inst)
                     << "OpTypeCooperativeMatrixKHR with ScopeWorkgroup "
                     << "used before LocalSizeId constant value <id> "
                     << _.getIdName(id) << " is defined.";
            }
          }
        }
      }
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateTypeUntypedPointerKHR(ValidationState_t& _,
                                           const Instruction* inst) {
  if (spvIsVulkanEnv(_.context()->target_env)) {
    const auto sc = inst->GetOperandAs<spv::StorageClass>(1);
    switch (sc) {
      case spv::StorageClass::Workgroup:
        if (!_.HasCapability(
                spv::Capability::WorkgroupMemoryExplicitLayoutKHR)) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << "Workgroup storage class untyped pointers in Vulkan "
                    "require WorkgroupMemoryExplicitLayoutKHR be declared";
        }
        break;
      case spv::StorageClass::StorageBuffer:
      case spv::StorageClass::PhysicalStorageBuffer:
      case spv::StorageClass::Uniform:
      case spv::StorageClass::PushConstant:
        break;
      default:
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "In Vulkan, untyped pointers can only be used in an "
                  "explicitly laid out storage class";
    }
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateTensorDim(ValidationState_t& _, const Instruction* inst) {
  const auto dim_index = 1;
  const auto dim_id = inst->GetOperandAs<uint32_t>(dim_index);
  const auto dim = _.FindDef(dim_id);
  if (!dim || !_.IsIntScalarType(dim->type_id()) ||
      _.GetBitWidth(dim->type_id()) != 32) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << spvOpcodeString(inst->opcode()) << " Dim <id> "
           << _.getIdName(dim_id) << " is not a 32-bit integer.";
  }

  constexpr uint32_t max_tensor_dim = 5;

  uint64_t dim_value;
  if (_.EvalConstantValUint64(dim_id, &dim_value)) {
    if (dim_value == 0 || dim_value > max_tensor_dim) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << spvOpcodeString(inst->opcode()) << " Dim <id> "
             << _.getIdName(dim_id) << " must be between 1 and "
             << max_tensor_dim << ".";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateTypeTensorLayoutNV(ValidationState_t& _,
                                        const Instruction* inst) {
  if (auto error = ValidateTensorDim(_, inst)) return error;

  const auto clamp_index = 2;
  const auto clamp_id = inst->GetOperandAs<uint32_t>(clamp_index);
  const auto clamp = _.FindDef(clamp_id);
  if (!clamp || !_.IsIntScalarType(clamp->type_id()) ||
      _.GetBitWidth(clamp->type_id()) != 32) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << spvOpcodeString(inst->opcode()) << " ClampMode <id> "
           << _.getIdName(clamp_id) << " is not a 32-bit integer.";
  }

  uint64_t clamp_value;
  if (_.EvalConstantValUint64(clamp_id, &clamp_value)) {
    if (clamp_value >
        static_cast<uint32_t>(spv::TensorClampMode::RepeatMirrored)) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << spvOpcodeString(inst->opcode()) << " ClampMode <id> "
             << _.getIdName(clamp_id) << " must be a valid TensorClampMode.";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateTypeTensorViewNV(ValidationState_t& _,
                                      const Instruction* inst) {
  if (auto error = ValidateTensorDim(_, inst)) return error;

  const auto has_dim_index = 2;
  const auto has_dim_id = inst->GetOperandAs<uint32_t>(has_dim_index);
  const auto has_dim = _.FindDef(has_dim_id);
  if (!has_dim || !_.IsBoolScalarType(has_dim->type_id())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << spvOpcodeString(inst->opcode()) << " HasDimensions <id> "
           << _.getIdName(has_dim_id) << " is not a boolean value.";
  }

  uint32_t permutation_mask = 0;
  bool all_constant = true;
  const auto num_dim = inst->operands().size() - 3;
  for (size_t p_index = 3; p_index < inst->operands().size(); ++p_index) {
    auto p_id = inst->GetOperandAs<uint32_t>(p_index);
    const auto p = _.FindDef(p_id);
    if (!p || !_.IsIntScalarType(p->type_id()) ||
        _.GetBitWidth(p->type_id()) != 32) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << spvOpcodeString(inst->opcode()) << " Permutation <id> "
             << _.getIdName(p_id) << " is not a 32-bit integer.";
    }

    uint64_t p_value;
    if (_.EvalConstantValUint64(p_id, &p_value)) {
      if (p_value >= num_dim) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << spvOpcodeString(inst->opcode()) << " Permutation <id> "
               << _.getIdName(p_id) << " must be a valid dimension.";
      }
      permutation_mask |= 1 << p_value;
    } else {
      all_constant = false;
    }
  }
  if (all_constant && permutation_mask != (1U << num_dim) - 1U) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << spvOpcodeString(inst->opcode())
           << " Permutation values don't form a valid permutation.";
  }

  uint64_t dim_value;
  if (_.EvalConstantValUint64(inst->GetOperandAs<uint32_t>(1), &dim_value)) {
    if (dim_value != num_dim) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << spvOpcodeString(inst->opcode())
             << " Incorrect number of permutation values.";
    }
  }

  return SPV_SUCCESS;
}
}  // namespace

spv_result_t TypePass(ValidationState_t& _, const Instruction* inst) {
  if (!spvOpcodeGeneratesType(inst->opcode()) &&
      inst->opcode() != spv::Op::OpTypeForwardPointer) {
    return SPV_SUCCESS;
  }

  if (auto error = ValidateUniqueness(_, inst)) return error;

  switch (inst->opcode()) {
    case spv::Op::OpTypeInt:
      if (auto error = ValidateTypeInt(_, inst)) return error;
      break;
    case spv::Op::OpTypeFloat:
      if (auto error = ValidateTypeFloat(_, inst)) return error;
      break;
    case spv::Op::OpTypeVector:
      if (auto error = ValidateTypeVector(_, inst)) return error;
      break;
    case spv::Op::OpTypeMatrix:
      if (auto error = ValidateTypeMatrix(_, inst)) return error;
      break;
    case spv::Op::OpTypeArray:
      if (auto error = ValidateTypeArray(_, inst)) return error;
      break;
    case spv::Op::OpTypeRuntimeArray:
      if (auto error = ValidateTypeRuntimeArray(_, inst)) return error;
      break;
    case spv::Op::OpTypeStruct:
      if (auto error = ValidateTypeStruct(_, inst)) return error;
      break;
    case spv::Op::OpTypePointer:
      if (auto error = ValidateTypePointer(_, inst)) return error;
      break;
    case spv::Op::OpTypeFunction:
      if (auto error = ValidateTypeFunction(_, inst)) return error;
      break;
    case spv::Op::OpTypeForwardPointer:
      if (auto error = ValidateTypeForwardPointer(_, inst)) return error;
      break;
    case spv::Op::OpTypeCooperativeMatrixNV:
    case spv::Op::OpTypeCooperativeMatrixKHR:
      if (auto error = ValidateTypeCooperativeMatrix(_, inst)) return error;
      break;
    case spv::Op::OpTypeCooperativeVectorNV:
      if (auto error = ValidateTypeCooperativeVectorNV(_, inst)) return error;
      break;
    case spv::Op::OpTypeUntypedPointerKHR:
      if (auto error = ValidateTypeUntypedPointerKHR(_, inst)) return error;
      break;
    case spv::Op::OpTypeTensorLayoutNV:
      if (auto error = ValidateTypeTensorLayoutNV(_, inst)) return error;
      break;
    case spv::Op::OpTypeTensorViewNV:
      if (auto error = ValidateTypeTensorViewNV(_, inst)) return error;
      break;
    default:
      break;
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
