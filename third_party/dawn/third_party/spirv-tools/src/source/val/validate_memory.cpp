// Copyright (c) 2018 Google LLC.
// Modifications Copyright (C) 2020-2024 Advanced Micro Devices, Inc. All
// rights reserved.
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

#include <algorithm>
#include <string>
#include <vector>

#include "source/opcode.h"
#include "source/spirv_target_env.h"
#include "source/val/instruction.h"
#include "source/val/validate.h"
#include "source/val/validate_scopes.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

bool AreLayoutCompatibleStructs(ValidationState_t&, const Instruction*,
                                const Instruction*);
bool HaveLayoutCompatibleMembers(ValidationState_t&, const Instruction*,
                                 const Instruction*);
bool HaveSameLayoutDecorations(ValidationState_t&, const Instruction*,
                               const Instruction*);
bool HasConflictingMemberOffsets(const std::set<Decoration>&,
                                 const std::set<Decoration>&);

bool IsAllowedTypeOrArrayOfSame(ValidationState_t& _, const Instruction* type,
                                std::initializer_list<spv::Op> allowed) {
  if (std::find(allowed.begin(), allowed.end(), type->opcode()) !=
      allowed.end()) {
    return true;
  }
  if (type->opcode() == spv::Op::OpTypeArray ||
      type->opcode() == spv::Op::OpTypeRuntimeArray) {
    auto elem_type = _.FindDef(type->word(2));
    return std::find(allowed.begin(), allowed.end(), elem_type->opcode()) !=
           allowed.end();
  }
  return false;
}

// Returns true if the two instructions represent structs that, as far as the
// validator can tell, have the exact same data layout.
bool AreLayoutCompatibleStructs(ValidationState_t& _, const Instruction* type1,
                                const Instruction* type2) {
  if (type1->opcode() != spv::Op::OpTypeStruct) {
    return false;
  }
  if (type2->opcode() != spv::Op::OpTypeStruct) {
    return false;
  }

  if (!HaveLayoutCompatibleMembers(_, type1, type2)) return false;

  return HaveSameLayoutDecorations(_, type1, type2);
}

// Returns true if the operands to the OpTypeStruct instruction defining the
// types are the same or are layout compatible types. |type1| and |type2| must
// be OpTypeStruct instructions.
bool HaveLayoutCompatibleMembers(ValidationState_t& _, const Instruction* type1,
                                 const Instruction* type2) {
  assert(type1->opcode() == spv::Op::OpTypeStruct &&
         "type1 must be an OpTypeStruct instruction.");
  assert(type2->opcode() == spv::Op::OpTypeStruct &&
         "type2 must be an OpTypeStruct instruction.");
  const auto& type1_operands = type1->operands();
  const auto& type2_operands = type2->operands();
  if (type1_operands.size() != type2_operands.size()) {
    return false;
  }

  for (size_t operand = 2; operand < type1_operands.size(); ++operand) {
    if (type1->word(operand) != type2->word(operand)) {
      auto def1 = _.FindDef(type1->word(operand));
      auto def2 = _.FindDef(type2->word(operand));
      if (!AreLayoutCompatibleStructs(_, def1, def2)) {
        return false;
      }
    }
  }
  return true;
}

// Returns true if all decorations that affect the data layout of the struct
// (like Offset), are the same for the two types. |type1| and |type2| must be
// OpTypeStruct instructions.
bool HaveSameLayoutDecorations(ValidationState_t& _, const Instruction* type1,
                               const Instruction* type2) {
  assert(type1->opcode() == spv::Op::OpTypeStruct &&
         "type1 must be an OpTypeStruct instruction.");
  assert(type2->opcode() == spv::Op::OpTypeStruct &&
         "type2 must be an OpTypeStruct instruction.");
  const std::set<Decoration>& type1_decorations = _.id_decorations(type1->id());
  const std::set<Decoration>& type2_decorations = _.id_decorations(type2->id());

  // TODO: Will have to add other check for arrays an matricies if we want to
  // handle them.
  if (HasConflictingMemberOffsets(type1_decorations, type2_decorations)) {
    return false;
  }

  return true;
}

bool HasConflictingMemberOffsets(
    const std::set<Decoration>& type1_decorations,
    const std::set<Decoration>& type2_decorations) {
  {
    // We are interested in conflicting decoration.  If a decoration is in one
    // list but not the other, then we will assume the code is correct.  We are
    // looking for things we know to be wrong.
    //
    // We do not have to traverse type2_decoration because, after traversing
    // type1_decorations, anything new will not be found in
    // type1_decoration.  Therefore, it cannot lead to a conflict.
    for (const Decoration& decoration : type1_decorations) {
      switch (decoration.dec_type()) {
        case spv::Decoration::Offset: {
          // Since these affect the layout of the struct, they must be present
          // in both structs.
          auto compare = [&decoration](const Decoration& rhs) {
            if (rhs.dec_type() != spv::Decoration::Offset) return false;
            return decoration.struct_member_index() ==
                   rhs.struct_member_index();
          };
          auto i = std::find_if(type2_decorations.begin(),
                                type2_decorations.end(), compare);
          if (i != type2_decorations.end() &&
              decoration.params().front() != i->params().front()) {
            return true;
          }
        } break;
        default:
          // This decoration does not affect the layout of the structure, so
          // just moving on.
          break;
      }
    }
  }
  return false;
}

// If |skip_builtin| is true, returns true if |storage| contains bool within
// it and no storage that contains the bool is builtin.
// If |skip_builtin| is false, returns true if |storage| contains bool within
// it.
bool ContainsInvalidBool(ValidationState_t& _, const Instruction* storage,
                         bool skip_builtin) {
  if (skip_builtin) {
    for (const Decoration& decoration : _.id_decorations(storage->id())) {
      if (decoration.dec_type() == spv::Decoration::BuiltIn) return false;
    }
  }

  const size_t elem_type_index = 1;
  uint32_t elem_type_id;
  Instruction* elem_type;

  switch (storage->opcode()) {
    case spv::Op::OpTypeBool:
      return true;
    case spv::Op::OpTypeVector:
    case spv::Op::OpTypeMatrix:
    case spv::Op::OpTypeArray:
    case spv::Op::OpTypeRuntimeArray:
      elem_type_id = storage->GetOperandAs<uint32_t>(elem_type_index);
      elem_type = _.FindDef(elem_type_id);
      return ContainsInvalidBool(_, elem_type, skip_builtin);
    case spv::Op::OpTypeStruct:
      for (size_t member_type_index = 1;
           member_type_index < storage->operands().size();
           ++member_type_index) {
        auto member_type_id =
            storage->GetOperandAs<uint32_t>(member_type_index);
        auto member_type = _.FindDef(member_type_id);
        if (ContainsInvalidBool(_, member_type, skip_builtin)) return true;
      }
    default:
      break;
  }
  return false;
}

std::pair<spv::StorageClass, spv::StorageClass> GetStorageClass(
    ValidationState_t& _, const Instruction* inst) {
  spv::StorageClass dst_sc = spv::StorageClass::Max;
  spv::StorageClass src_sc = spv::StorageClass::Max;
  switch (inst->opcode()) {
    case spv::Op::OpCooperativeMatrixLoadNV:
    case spv::Op::OpCooperativeMatrixLoadTensorNV:
    case spv::Op::OpCooperativeMatrixLoadKHR:
    case spv::Op::OpCooperativeVectorLoadNV:
    case spv::Op::OpLoad: {
      auto load_pointer = _.FindDef(inst->GetOperandAs<uint32_t>(2));
      auto load_pointer_type = _.FindDef(load_pointer->type_id());
      dst_sc = load_pointer_type->GetOperandAs<spv::StorageClass>(1);
      break;
    }
    case spv::Op::OpCooperativeMatrixStoreNV:
    case spv::Op::OpCooperativeMatrixStoreTensorNV:
    case spv::Op::OpCooperativeMatrixStoreKHR:
    case spv::Op::OpCooperativeVectorStoreNV:
    case spv::Op::OpStore: {
      auto store_pointer = _.FindDef(inst->GetOperandAs<uint32_t>(0));
      auto store_pointer_type = _.FindDef(store_pointer->type_id());
      dst_sc = store_pointer_type->GetOperandAs<spv::StorageClass>(1);
      break;
    }
    case spv::Op::OpCopyMemory:
    case spv::Op::OpCopyMemorySized: {
      auto dst = _.FindDef(inst->GetOperandAs<uint32_t>(0));
      auto dst_type = _.FindDef(dst->type_id());
      dst_sc = dst_type->GetOperandAs<spv::StorageClass>(1);
      auto src = _.FindDef(inst->GetOperandAs<uint32_t>(1));
      auto src_type = _.FindDef(src->type_id());
      src_sc = src_type->GetOperandAs<spv::StorageClass>(1);
      break;
    }
    default:
      break;
  }

  return std::make_pair(dst_sc, src_sc);
}

// Returns the number of instruction words taken up by a memory access
// argument and its implied operands.
int MemoryAccessNumWords(uint32_t mask) {
  int result = 1;  // Count the mask
  if (mask & uint32_t(spv::MemoryAccessMask::Aligned)) ++result;
  if (mask & uint32_t(spv::MemoryAccessMask::MakePointerAvailableKHR)) ++result;
  if (mask & uint32_t(spv::MemoryAccessMask::MakePointerVisibleKHR)) ++result;
  return result;
}

// Returns the scope ID operand for MakeAvailable memory access with mask
// at the given operand index.
// This function is only called for OpLoad, OpStore, OpCopyMemory and
// OpCopyMemorySized, OpCooperativeMatrixLoadNV,
// OpCooperativeMatrixStoreNV, OpCooperativeVectorLoadNV,
// OpCooperativeVectorStoreNV.
uint32_t GetMakeAvailableScope(const Instruction* inst, uint32_t mask,
                               uint32_t mask_index) {
  assert(mask & uint32_t(spv::MemoryAccessMask::MakePointerAvailableKHR));
  uint32_t this_bit = uint32_t(spv::MemoryAccessMask::MakePointerAvailableKHR);
  uint32_t index =
      mask_index - 1 + MemoryAccessNumWords(mask & (this_bit | (this_bit - 1)));
  return inst->GetOperandAs<uint32_t>(index);
}

// This function is only called for OpLoad, OpStore, OpCopyMemory,
// OpCopyMemorySized, OpCooperativeMatrixLoadNV,
// OpCooperativeMatrixStoreNV, OpCooperativeVectorLoadNV,
// OpCooperativeVectorStoreNV.
uint32_t GetMakeVisibleScope(const Instruction* inst, uint32_t mask,
                             uint32_t mask_index) {
  assert(mask & uint32_t(spv::MemoryAccessMask::MakePointerVisibleKHR));
  uint32_t this_bit = uint32_t(spv::MemoryAccessMask::MakePointerVisibleKHR);
  uint32_t index =
      mask_index - 1 + MemoryAccessNumWords(mask & (this_bit | (this_bit - 1)));
  return inst->GetOperandAs<uint32_t>(index);
}

bool DoesStructContainRTA(const ValidationState_t& _, const Instruction* inst) {
  for (size_t member_index = 1; member_index < inst->operands().size();
       ++member_index) {
    const auto member_id = inst->GetOperandAs<uint32_t>(member_index);
    const auto member_type = _.FindDef(member_id);
    if (member_type->opcode() == spv::Op::OpTypeRuntimeArray) return true;
  }
  return false;
}

spv_result_t CheckMemoryAccess(ValidationState_t& _, const Instruction* inst,
                               uint32_t index) {
  spv::StorageClass dst_sc, src_sc;
  std::tie(dst_sc, src_sc) = GetStorageClass(_, inst);
  if (inst->operands().size() <= index) {
    // Cases where lack of some operand is invalid
    if (src_sc == spv::StorageClass::PhysicalStorageBuffer ||
        dst_sc == spv::StorageClass::PhysicalStorageBuffer) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << _.VkErrorID(4708)
             << "Memory accesses with PhysicalStorageBuffer must use Aligned.";
    }
    return SPV_SUCCESS;
  }

  const uint32_t mask = inst->GetOperandAs<uint32_t>(index);
  if (mask & uint32_t(spv::MemoryAccessMask::MakePointerAvailableKHR)) {
    if (inst->opcode() == spv::Op::OpLoad ||
        inst->opcode() == spv::Op::OpCooperativeMatrixLoadNV ||
        inst->opcode() == spv::Op::OpCooperativeMatrixLoadTensorNV ||
        inst->opcode() == spv::Op::OpCooperativeMatrixLoadKHR ||
        inst->opcode() == spv::Op::OpCooperativeVectorLoadNV) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "MakePointerAvailableKHR cannot be used with OpLoad.";
    }

    if (!(mask & uint32_t(spv::MemoryAccessMask::NonPrivatePointerKHR))) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "NonPrivatePointerKHR must be specified if "
                "MakePointerAvailableKHR is specified.";
    }

    // Check the associated scope for MakeAvailableKHR.
    const auto available_scope = GetMakeAvailableScope(inst, mask, index);
    if (auto error = ValidateMemoryScope(_, inst, available_scope))
      return error;
  }

  if (mask & uint32_t(spv::MemoryAccessMask::MakePointerVisibleKHR)) {
    if (inst->opcode() == spv::Op::OpStore ||
        inst->opcode() == spv::Op::OpCooperativeMatrixStoreNV ||
        inst->opcode() == spv::Op::OpCooperativeMatrixStoreKHR ||
        inst->opcode() == spv::Op::OpCooperativeMatrixStoreTensorNV ||
        inst->opcode() == spv::Op::OpCooperativeVectorStoreNV) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "MakePointerVisibleKHR cannot be used with OpStore.";
    }

    if (!(mask & uint32_t(spv::MemoryAccessMask::NonPrivatePointerKHR))) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "NonPrivatePointerKHR must be specified if "
             << "MakePointerVisibleKHR is specified.";
    }

    // Check the associated scope for MakeVisibleKHR.
    const auto visible_scope = GetMakeVisibleScope(inst, mask, index);
    if (auto error = ValidateMemoryScope(_, inst, visible_scope)) return error;
  }

  if (mask & uint32_t(spv::MemoryAccessMask::NonPrivatePointerKHR)) {
    if (dst_sc != spv::StorageClass::Uniform &&
        dst_sc != spv::StorageClass::Workgroup &&
        dst_sc != spv::StorageClass::CrossWorkgroup &&
        dst_sc != spv::StorageClass::Generic &&
        dst_sc != spv::StorageClass::Image &&
        dst_sc != spv::StorageClass::StorageBuffer &&
        dst_sc != spv::StorageClass::PhysicalStorageBuffer) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "NonPrivatePointerKHR requires a pointer in Uniform, "
             << "Workgroup, CrossWorkgroup, Generic, Image or StorageBuffer "
             << "storage classes.";
    }
    if (src_sc != spv::StorageClass::Max &&
        src_sc != spv::StorageClass::Uniform &&
        src_sc != spv::StorageClass::Workgroup &&
        src_sc != spv::StorageClass::CrossWorkgroup &&
        src_sc != spv::StorageClass::Generic &&
        src_sc != spv::StorageClass::Image &&
        src_sc != spv::StorageClass::StorageBuffer &&
        src_sc != spv::StorageClass::PhysicalStorageBuffer) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "NonPrivatePointerKHR requires a pointer in Uniform, "
             << "Workgroup, CrossWorkgroup, Generic, Image or StorageBuffer "
             << "storage classes.";
    }
  }

  if (!(mask & uint32_t(spv::MemoryAccessMask::Aligned))) {
    if (src_sc == spv::StorageClass::PhysicalStorageBuffer ||
        dst_sc == spv::StorageClass::PhysicalStorageBuffer) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << _.VkErrorID(4708)
             << "Memory accesses with PhysicalStorageBuffer must use Aligned.";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateVariable(ValidationState_t& _, const Instruction* inst) {
  const bool untyped_pointer = inst->opcode() == spv::Op::OpUntypedVariableKHR;

  auto result_type = _.FindDef(inst->type_id());
  if (untyped_pointer) {
    if (!result_type ||
        result_type->opcode() != spv::Op::OpTypeUntypedPointerKHR)
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Result type must be an untyped pointer";
  } else {
    if (!result_type || result_type->opcode() != spv::Op::OpTypePointer) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpVariable Result Type <id> " << _.getIdName(inst->type_id())
             << " is not a pointer type.";
    }
  }

  const auto storage_class_index = 2u;
  auto storage_class =
      inst->GetOperandAs<spv::StorageClass>(storage_class_index);
  uint32_t value_id = 0;
  if (untyped_pointer) {
    const auto has_data_type = 3u < inst->operands().size();
    if (has_data_type) {
      value_id = inst->GetOperandAs<uint32_t>(3u);
      auto data_type = _.FindDef(value_id);
      if (!data_type || !spvOpcodeGeneratesType(data_type->opcode())) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "Data type must be a type instruction";
      }
    } else {
      if (storage_class == spv::StorageClass::Function ||
          storage_class == spv::StorageClass::Private ||
          storage_class == spv::StorageClass::Workgroup) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "Data type must be specified for Function, Private, and "
                  "Workgroup storage classes";
      }
      if (spvIsVulkanEnv(_.context()->target_env)) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "Vulkan requires that data type be specified";
      }
    }
  }

  // For OpVariable the data type comes from pointee type of the result type,
  // while for OpUntypedVariableKHR the data type comes from the operand.
  if (!untyped_pointer) {
    value_id = result_type->GetOperandAs<uint32_t>(2);
  }
  auto value_type = value_id == 0 ? nullptr : _.FindDef(value_id);

  const auto initializer_index = untyped_pointer ? 4u : 3u;
  if (initializer_index < inst->operands().size()) {
    const auto initializer_id = inst->GetOperandAs<uint32_t>(initializer_index);
    const auto initializer = _.FindDef(initializer_id);
    const auto is_module_scope_var =
        initializer &&
        (initializer->opcode() == spv::Op::OpVariable ||
         initializer->opcode() == spv::Op::OpUntypedVariableKHR) &&
        (initializer->GetOperandAs<spv::StorageClass>(storage_class_index) !=
         spv::StorageClass::Function);
    const auto is_constant =
        initializer && spvOpcodeIsConstant(initializer->opcode());
    if (!initializer || !(is_constant || is_module_scope_var)) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Variable Initializer <id> " << _.getIdName(initializer_id)
             << " is not a constant or module-scope variable.";
    }
    if (initializer->type_id() != value_id) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Initializer type must match the data type";
    }
  }

  if (storage_class != spv::StorageClass::Workgroup &&
      storage_class != spv::StorageClass::CrossWorkgroup &&
      storage_class != spv::StorageClass::Private &&
      storage_class != spv::StorageClass::Function &&
      storage_class != spv::StorageClass::UniformConstant &&
      storage_class != spv::StorageClass::RayPayloadKHR &&
      storage_class != spv::StorageClass::IncomingRayPayloadKHR &&
      storage_class != spv::StorageClass::HitAttributeKHR &&
      storage_class != spv::StorageClass::CallableDataKHR &&
      storage_class != spv::StorageClass::IncomingCallableDataKHR &&
      storage_class != spv::StorageClass::TaskPayloadWorkgroupEXT &&
      storage_class != spv::StorageClass::HitObjectAttributeNV &&
      storage_class != spv::StorageClass::NodePayloadAMDX) {
    bool storage_input_or_output = storage_class == spv::StorageClass::Input ||
                                   storage_class == spv::StorageClass::Output;
    bool builtin = false;
    if (storage_input_or_output) {
      for (const Decoration& decoration : _.id_decorations(inst->id())) {
        if (decoration.dec_type() == spv::Decoration::BuiltIn) {
          builtin = true;
          break;
        }
      }
    }
    if (!builtin && value_type &&
        ContainsInvalidBool(_, value_type, storage_input_or_output)) {
      if (storage_input_or_output) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << _.VkErrorID(7290)
               << "If OpTypeBool is stored in conjunction with OpVariable "
                  "using Input or Output Storage Classes it requires a BuiltIn "
                  "decoration";

      } else {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "If OpTypeBool is stored in conjunction with OpVariable, it "
                  "can only be used with non-externally visible shader Storage "
                  "Classes: Workgroup, CrossWorkgroup, Private, Function, "
                  "Input, Output, RayPayloadKHR, IncomingRayPayloadKHR, "
                  "HitAttributeKHR, CallableDataKHR, "
                  "IncomingCallableDataKHR, NodePayloadAMDX, or "
                  "UniformConstant";
      }
    }
  }

  if (!_.IsValidStorageClass(storage_class)) {
    return _.diag(SPV_ERROR_INVALID_BINARY, inst)
           << _.VkErrorID(4643)
           << "Invalid storage class for target environment";
  }

  if (storage_class == spv::StorageClass::Generic) {
    return _.diag(SPV_ERROR_INVALID_BINARY, inst)
           << "Variable storage class cannot be Generic";
  }

  if (inst->function() && storage_class != spv::StorageClass::Function) {
    return _.diag(SPV_ERROR_INVALID_LAYOUT, inst)
           << "Variables must have a function[7] storage class inside"
              " of a function";
  }

  if (!inst->function() && storage_class == spv::StorageClass::Function) {
    return _.diag(SPV_ERROR_INVALID_LAYOUT, inst)
           << "Variables can not have a function[7] storage class "
              "outside of a function";
  }

  // SPIR-V 3.32.8: Check that pointer type and variable type have the same
  // storage class.
  const auto result_storage_class_index = 1;
  const auto result_storage_class =
      result_type->GetOperandAs<spv::StorageClass>(result_storage_class_index);
  if (storage_class != result_storage_class) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Storage class must match result type storage class";
  }

  // Variable pointer related restrictions.
  const auto pointee = untyped_pointer
                           ? value_id == 0 ? nullptr : _.FindDef(value_id)
                           : _.FindDef(result_type->word(3));
  if (_.addressing_model() == spv::AddressingModel::Logical &&
      !_.options()->relax_logical_pointer) {
    // VariablePointersStorageBuffer is implied by VariablePointers.
    if (pointee && pointee->opcode() == spv::Op::OpTypePointer) {
      if (!_.HasCapability(spv::Capability::VariablePointersStorageBuffer)) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "In Logical addressing, variables may not allocate a pointer "
               << "type";
      } else if (storage_class != spv::StorageClass::Function &&
                 storage_class != spv::StorageClass::Private) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "In Logical addressing with variable pointers, variables "
               << "that allocate pointers must be in Function or Private "
               << "storage classes";
      }
    }
  }

  if (spvIsVulkanEnv(_.context()->target_env)) {
    // Vulkan Push Constant Interface section: Check type of PushConstant
    // variables.
    if (storage_class == spv::StorageClass::PushConstant) {
      if (pointee && pointee->opcode() != spv::Op::OpTypeStruct) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << _.VkErrorID(6808) << "PushConstant OpVariable <id> "
               << _.getIdName(inst->id()) << " has illegal type.\n"
               << "From Vulkan spec, Push Constant Interface section:\n"
               << "Such variables must be typed as OpTypeStruct";
      }
    }

    // Vulkan Descriptor Set Interface: Check type of UniformConstant and
    // Uniform variables.
    if (storage_class == spv::StorageClass::UniformConstant) {
      if (pointee && !IsAllowedTypeOrArrayOfSame(
                         _, pointee,
                         {spv::Op::OpTypeImage, spv::Op::OpTypeSampler,
                          spv::Op::OpTypeSampledImage,
                          spv::Op::OpTypeAccelerationStructureKHR})) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << _.VkErrorID(4655) << "UniformConstant OpVariable <id> "
               << _.getIdName(inst->id()) << " has illegal type.\n"
               << "Variables identified with the UniformConstant storage class "
               << "are used only as handles to refer to opaque resources. Such "
               << "variables must be typed as OpTypeImage, OpTypeSampler, "
               << "OpTypeSampledImage, OpTypeAccelerationStructureKHR, "
               << "or an array of one of these types.";
      }
    }

    if (storage_class == spv::StorageClass::Uniform) {
      if (pointee &&
          !IsAllowedTypeOrArrayOfSame(_, pointee, {spv::Op::OpTypeStruct})) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << _.VkErrorID(6807) << "Uniform OpVariable <id> "
               << _.getIdName(inst->id()) << " has illegal type.\n"
               << "From Vulkan spec:\n"
               << "Variables identified with the Uniform storage class are "
               << "used to access transparent buffer backed resources. Such "
               << "variables must be typed as OpTypeStruct, or an array of "
               << "this type";
      }
    }

    if (storage_class == spv::StorageClass::StorageBuffer) {
      if (pointee &&
          !IsAllowedTypeOrArrayOfSame(_, pointee, {spv::Op::OpTypeStruct})) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << _.VkErrorID(6807) << "StorageBuffer OpVariable <id> "
               << _.getIdName(inst->id()) << " has illegal type.\n"
               << "From Vulkan spec:\n"
               << "Variables identified with the StorageBuffer storage class "
                  "are used to access transparent buffer backed resources. "
                  "Such variables must be typed as OpTypeStruct, or an array "
                  "of this type";
      }
    }

    // Check for invalid use of Invariant
    if (storage_class != spv::StorageClass::Input &&
        storage_class != spv::StorageClass::Output) {
      if (_.HasDecoration(inst->id(), spv::Decoration::Invariant)) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << _.VkErrorID(4677)
               << "Variable decorated with Invariant must only be identified "
                  "with the Input or Output storage class in Vulkan "
                  "environment.";
      }
      // Need to check if only the members in a struct are decorated
      if (value_type && value_type->opcode() == spv::Op::OpTypeStruct) {
        if (_.HasDecoration(value_id, spv::Decoration::Invariant)) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << _.VkErrorID(4677)
                 << "Variable struct member decorated with Invariant must only "
                    "be identified with the Input or Output storage class in "
                    "Vulkan environment.";
        }
      }
    }
  }

  // Vulkan Appendix A: Check that if contains initializer, then
  // storage class is Output, Private, or Function.
  if (inst->operands().size() > initializer_index &&
      storage_class != spv::StorageClass::Output &&
      storage_class != spv::StorageClass::Private &&
      storage_class != spv::StorageClass::Function) {
    if (spvIsVulkanEnv(_.context()->target_env)) {
      if (storage_class == spv::StorageClass::Workgroup) {
        auto init_id = inst->GetOperandAs<uint32_t>(initializer_index);
        auto init = _.FindDef(init_id);
        if (init->opcode() != spv::Op::OpConstantNull) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << _.VkErrorID(4734) << "OpVariable, <id> "
                 << _.getIdName(inst->id())
                 << ", initializers are limited to OpConstantNull in "
                    "Workgroup "
                    "storage class";
        }
      } else if (storage_class != spv::StorageClass::Output &&
                 storage_class != spv::StorageClass::Private &&
                 storage_class != spv::StorageClass::Function) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << _.VkErrorID(4651) << "OpVariable, <id> "
               << _.getIdName(inst->id())
               << ", has a disallowed initializer & storage class "
               << "combination.\n"
               << "From " << spvLogStringForEnv(_.context()->target_env)
               << " spec:\n"
               << "Variable declarations that include initializers must have "
               << "one of the following storage classes: Output, Private, "
               << "Function or Workgroup";
      }
    }
  }

  if (initializer_index < inst->operands().size()) {
    if (storage_class == spv::StorageClass::TaskPayloadWorkgroupEXT) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpVariable, <id> " << _.getIdName(inst->id())
             << ", initializer are not allowed for TaskPayloadWorkgroupEXT";
    }
    if (storage_class == spv::StorageClass::Input) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpVariable, <id> " << _.getIdName(inst->id())
             << ", initializer are not allowed for Input";
    }
    if (storage_class == spv::StorageClass::HitObjectAttributeNV) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpVariable, <id> " << _.getIdName(inst->id())
             << ", initializer are not allowed for HitObjectAttributeNV";
    }
  }

  if (storage_class == spv::StorageClass::PhysicalStorageBuffer) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "PhysicalStorageBuffer must not be used with OpVariable.";
  }

  // Vulkan specific validation rules for OpTypeRuntimeArray
  if (spvIsVulkanEnv(_.context()->target_env)) {
    // OpTypeRuntimeArray should only ever be in a container like OpTypeStruct,
    // so should never appear as a bare variable.
    // Unless the module has the RuntimeDescriptorArrayEXT capability.
    if (value_type && value_type->opcode() == spv::Op::OpTypeRuntimeArray) {
      if (!_.HasCapability(spv::Capability::RuntimeDescriptorArrayEXT)) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << _.VkErrorID(4680) << "OpVariable, <id> "
               << _.getIdName(inst->id())
               << ", is attempting to create memory for an illegal type, "
               << "OpTypeRuntimeArray.\nFor Vulkan OpTypeRuntimeArray can only "
               << "appear as the final member of an OpTypeStruct, thus cannot "
               << "be instantiated via OpVariable";
      } else {
        // A bare variable OpTypeRuntimeArray is allowed in this context, but
        // still need to check the storage class.
        if (storage_class != spv::StorageClass::StorageBuffer &&
            storage_class != spv::StorageClass::Uniform &&
            storage_class != spv::StorageClass::UniformConstant) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << _.VkErrorID(4680)
                 << "For Vulkan with RuntimeDescriptorArrayEXT, a variable "
                 << "containing OpTypeRuntimeArray must have storage class of "
                 << "StorageBuffer, Uniform, or UniformConstant.";
        }
      }
    }

    // If an OpStruct has an OpTypeRuntimeArray somewhere within it, then it
    // must either have the storage class StorageBuffer and be decorated
    // with Block, or it must be in the Uniform storage class and be decorated
    // as BufferBlock.
    if (value_type && value_type->opcode() == spv::Op::OpTypeStruct) {
      if (DoesStructContainRTA(_, value_type)) {
        if (storage_class == spv::StorageClass::StorageBuffer ||
            storage_class == spv::StorageClass::PhysicalStorageBuffer) {
          if (!_.HasDecoration(value_id, spv::Decoration::Block)) {
            return _.diag(SPV_ERROR_INVALID_ID, inst)
                   << _.VkErrorID(4680)
                   << "For Vulkan, an OpTypeStruct variable containing an "
                   << "OpTypeRuntimeArray must be decorated with Block if it "
                   << "has storage class StorageBuffer or "
                      "PhysicalStorageBuffer.";
          }
        } else if (storage_class == spv::StorageClass::Uniform) {
          if (!_.HasDecoration(value_id, spv::Decoration::BufferBlock)) {
            return _.diag(SPV_ERROR_INVALID_ID, inst)
                   << _.VkErrorID(4680)
                   << "For Vulkan, an OpTypeStruct variable containing an "
                   << "OpTypeRuntimeArray must be decorated with BufferBlock "
                   << "if it has storage class Uniform.";
          }
        } else {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << _.VkErrorID(4680)
                 << "For Vulkan, OpTypeStruct variables containing "
                 << "OpTypeRuntimeArray must have storage class of "
                 << "StorageBuffer, PhysicalStorageBuffer, or Uniform.";
        }
      }
    }
  }

  // Cooperative matrix types can only be allocated in Function or Private
  if ((storage_class != spv::StorageClass::Function &&
       storage_class != spv::StorageClass::Private) &&
      pointee &&
      _.ContainsType(pointee->id(), [](const Instruction* type_inst) {
        auto opcode = type_inst->opcode();
        return opcode == spv::Op::OpTypeCooperativeMatrixNV ||
               opcode == spv::Op::OpTypeCooperativeMatrixKHR;
      })) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Cooperative matrix types (or types containing them) can only be "
              "allocated "
           << "in Function or Private storage classes or as function "
              "parameters";
  }

  if ((storage_class != spv::StorageClass::Function &&
       storage_class != spv::StorageClass::Private) &&
      pointee &&
      _.ContainsType(pointee->id(), [](const Instruction* type_inst) {
        auto opcode = type_inst->opcode();
        return opcode == spv::Op::OpTypeCooperativeVectorNV;
      })) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Cooperative vector types (or types containing them) can only be "
              "allocated "
           << "in Function or Private storage classes or as function "
              "parameters";
  }

  if (_.HasCapability(spv::Capability::Shader)) {
    // Don't allow variables containing 16-bit elements without the appropriate
    // capabilities.
    if ((!_.HasCapability(spv::Capability::Int16) &&
         _.ContainsSizedIntOrFloatType(value_id, spv::Op::OpTypeInt, 16)) ||
        (!_.HasCapability(spv::Capability::Float16) &&
         _.ContainsSizedIntOrFloatType(value_id, spv::Op::OpTypeFloat, 16))) {
      auto underlying_type = value_type;
      while (underlying_type &&
             underlying_type->opcode() == spv::Op::OpTypePointer) {
        storage_class = underlying_type->GetOperandAs<spv::StorageClass>(1u);
        underlying_type =
            _.FindDef(underlying_type->GetOperandAs<uint32_t>(2u));
      }
      bool storage_class_ok = true;
      std::string sc_name = _.grammar().lookupOperandName(
          SPV_OPERAND_TYPE_STORAGE_CLASS, uint32_t(storage_class));
      switch (storage_class) {
        case spv::StorageClass::StorageBuffer:
        case spv::StorageClass::PhysicalStorageBuffer:
          if (!_.HasCapability(spv::Capability::StorageBuffer16BitAccess)) {
            storage_class_ok = false;
          }
          break;
        case spv::StorageClass::Uniform:
          if (underlying_type &&
              !_.HasCapability(
                  spv::Capability::UniformAndStorageBuffer16BitAccess)) {
            if (underlying_type->opcode() == spv::Op::OpTypeArray ||
                underlying_type->opcode() == spv::Op::OpTypeRuntimeArray) {
              underlying_type =
                  _.FindDef(underlying_type->GetOperandAs<uint32_t>(1u));
            }
            if (!_.HasCapability(spv::Capability::StorageBuffer16BitAccess) ||
                !_.HasDecoration(underlying_type->id(),
                                 spv::Decoration::BufferBlock)) {
              storage_class_ok = false;
            }
          }
          break;
        case spv::StorageClass::PushConstant:
          if (!_.HasCapability(spv::Capability::StoragePushConstant16)) {
            storage_class_ok = false;
          }
          break;
        case spv::StorageClass::Input:
        case spv::StorageClass::Output:
          if (!_.HasCapability(spv::Capability::StorageInputOutput16)) {
            storage_class_ok = false;
          }
          break;
        case spv::StorageClass::Workgroup:
          if (!_.HasCapability(
                  spv::Capability::
                      WorkgroupMemoryExplicitLayout16BitAccessKHR)) {
            storage_class_ok = false;
          }
          break;
        default:
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << "Cannot allocate a variable containing a 16-bit type in "
                 << sc_name << " storage class";
      }
      if (!storage_class_ok) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "Allocating a variable containing a 16-bit element in "
               << sc_name << " storage class requires an additional capability";
      }
    }
    // Don't allow variables containing 8-bit elements without the appropriate
    // capabilities.
    if (!_.HasCapability(spv::Capability::Int8) &&
        _.ContainsSizedIntOrFloatType(value_id, spv::Op::OpTypeInt, 8)) {
      auto underlying_type = value_type;
      while (underlying_type &&
             underlying_type->opcode() == spv::Op::OpTypePointer) {
        storage_class = underlying_type->GetOperandAs<spv::StorageClass>(1u);
        underlying_type =
            _.FindDef(underlying_type->GetOperandAs<uint32_t>(2u));
      }
      bool storage_class_ok = true;
      std::string sc_name = _.grammar().lookupOperandName(
          SPV_OPERAND_TYPE_STORAGE_CLASS, uint32_t(storage_class));
      switch (storage_class) {
        case spv::StorageClass::StorageBuffer:
        case spv::StorageClass::PhysicalStorageBuffer:
          if (!_.HasCapability(spv::Capability::StorageBuffer8BitAccess)) {
            storage_class_ok = false;
          }
          break;
        case spv::StorageClass::Uniform:
          if (underlying_type &&
              !_.HasCapability(
                  spv::Capability::UniformAndStorageBuffer8BitAccess)) {
            if (underlying_type->opcode() == spv::Op::OpTypeArray ||
                underlying_type->opcode() == spv::Op::OpTypeRuntimeArray) {
              underlying_type =
                  _.FindDef(underlying_type->GetOperandAs<uint32_t>(1u));
            }
            if (!_.HasCapability(spv::Capability::StorageBuffer8BitAccess) ||
                !_.HasDecoration(underlying_type->id(),
                                 spv::Decoration::BufferBlock)) {
              storage_class_ok = false;
            }
          }
          break;
        case spv::StorageClass::PushConstant:
          if (!_.HasCapability(spv::Capability::StoragePushConstant8)) {
            storage_class_ok = false;
          }
          break;
        case spv::StorageClass::Workgroup:
          if (!_.HasCapability(
                  spv::Capability::
                      WorkgroupMemoryExplicitLayout8BitAccessKHR)) {
            storage_class_ok = false;
          }
          break;
        default:
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << "Cannot allocate a variable containing a 8-bit type in "
                 << sc_name << " storage class";
      }
      if (!storage_class_ok) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "Allocating a variable containing a 8-bit element in "
               << sc_name << " storage class requires an additional capability";
      }
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateLoad(ValidationState_t& _, const Instruction* inst) {
  const auto result_type = _.FindDef(inst->type_id());
  if (!result_type) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpLoad Result Type <id> " << _.getIdName(inst->type_id())
           << " is not defined.";
  }

  const auto pointer_index = 2;
  const auto pointer_id = inst->GetOperandAs<uint32_t>(pointer_index);
  const auto pointer = _.FindDef(pointer_id);
  if (!pointer ||
      ((_.addressing_model() == spv::AddressingModel::Logical) &&
       ((!_.features().variable_pointers &&
         !spvOpcodeReturnsLogicalPointer(pointer->opcode())) ||
        (_.features().variable_pointers &&
         !spvOpcodeReturnsLogicalVariablePointer(pointer->opcode()))))) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpLoad Pointer <id> " << _.getIdName(pointer_id)
           << " is not a logical pointer.";
  }

  const auto pointer_type = _.FindDef(pointer->type_id());
  if (!pointer_type ||
      (pointer_type->opcode() != spv::Op::OpTypePointer &&
       pointer_type->opcode() != spv::Op::OpTypeUntypedPointerKHR)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpLoad type for pointer <id> " << _.getIdName(pointer_id)
           << " is not a pointer type.";
  }

  if (pointer_type->opcode() == spv::Op::OpTypePointer) {
    const auto pointee_type =
        _.FindDef(pointer_type->GetOperandAs<uint32_t>(2));
    if (!pointee_type || result_type->id() != pointee_type->id()) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpLoad Result Type <id> " << _.getIdName(inst->type_id())
             << " does not match Pointer <id> " << _.getIdName(pointer->id())
             << "s type.";
    }
  }

  if (!_.options()->before_hlsl_legalization &&
      _.ContainsRuntimeArray(inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Cannot load a runtime-sized array";
  }

  if (auto error = CheckMemoryAccess(_, inst, 3)) return error;

  if (_.HasCapability(spv::Capability::Shader) &&
      _.ContainsLimitedUseIntOrFloatType(inst->type_id()) &&
      result_type->opcode() != spv::Op::OpTypePointer) {
    if (result_type->opcode() != spv::Op::OpTypeInt &&
        result_type->opcode() != spv::Op::OpTypeFloat &&
        result_type->opcode() != spv::Op::OpTypeVector &&
        result_type->opcode() != spv::Op::OpTypeMatrix) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "8- or 16-bit loads must be a scalar, vector or matrix type";
    }
  }

  _.RegisterQCOMImageProcessingTextureConsumer(pointer_id, inst, nullptr);

  return SPV_SUCCESS;
}

spv_result_t ValidateStore(ValidationState_t& _, const Instruction* inst) {
  const auto pointer_index = 0;
  const auto pointer_id = inst->GetOperandAs<uint32_t>(pointer_index);
  const auto pointer = _.FindDef(pointer_id);
  if (!pointer ||
      (_.addressing_model() == spv::AddressingModel::Logical &&
       ((!_.features().variable_pointers &&
         !spvOpcodeReturnsLogicalPointer(pointer->opcode())) ||
        (_.features().variable_pointers &&
         !spvOpcodeReturnsLogicalVariablePointer(pointer->opcode()))))) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpStore Pointer <id> " << _.getIdName(pointer_id)
           << " is not a logical pointer.";
  }
  const auto pointer_type = _.FindDef(pointer->type_id());
  if (!pointer_type ||
      (pointer_type->opcode() != spv::Op::OpTypePointer &&
       pointer_type->opcode() != spv::Op::OpTypeUntypedPointerKHR)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpStore type for pointer <id> " << _.getIdName(pointer_id)
           << " is not a pointer type.";
  }

  Instruction* type = nullptr;
  if (pointer_type->opcode() == spv::Op::OpTypePointer) {
    const auto type_id = pointer_type->GetOperandAs<uint32_t>(2);
    type = _.FindDef(type_id);
    if (!type || spv::Op::OpTypeVoid == type->opcode()) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpStore Pointer <id> " << _.getIdName(pointer_id)
             << "s type is void.";
    }
  }

  // validate storage class
  {
    uint32_t data_type;
    spv::StorageClass storage_class;
    if (!_.GetPointerTypeInfo(pointer_type->id(), &data_type, &storage_class)) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpStore Pointer <id> " << _.getIdName(pointer_id)
             << " is not pointer type";
    }

    if (storage_class == spv::StorageClass::UniformConstant ||
        storage_class == spv::StorageClass::Input ||
        storage_class == spv::StorageClass::PushConstant) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpStore Pointer <id> " << _.getIdName(pointer_id)
             << " storage class is read-only";
    } else if (storage_class == spv::StorageClass::ShaderRecordBufferKHR) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "ShaderRecordBufferKHR Storage Class variables are read only";
    } else if (storage_class == spv::StorageClass::HitAttributeKHR) {
      std::string errorVUID = _.VkErrorID(4703);
      _.function(inst->function()->id())
          ->RegisterExecutionModelLimitation(
              [errorVUID](spv::ExecutionModel model, std::string* message) {
                if (model == spv::ExecutionModel::AnyHitKHR ||
                    model == spv::ExecutionModel::ClosestHitKHR) {
                  if (message) {
                    *message =
                        errorVUID +
                        "HitAttributeKHR Storage Class variables are read only "
                        "with AnyHitKHR and ClosestHitKHR";
                  }
                  return false;
                }
                return true;
              });
    }

    if (spvIsVulkanEnv(_.context()->target_env) &&
        storage_class == spv::StorageClass::Uniform) {
      auto base_ptr = _.TracePointer(pointer);
      if (base_ptr->opcode() == spv::Op::OpVariable) {
        // If it's not a variable a different check should catch the problem.
        auto base_type = _.FindDef(base_ptr->GetOperandAs<uint32_t>(0));
        // Get the pointed-to type.
        base_type = _.FindDef(base_type->GetOperandAs<uint32_t>(2u));
        if (base_type->opcode() == spv::Op::OpTypeArray ||
            base_type->opcode() == spv::Op::OpTypeRuntimeArray) {
          base_type = _.FindDef(base_type->GetOperandAs<uint32_t>(1u));
        }
        if (_.HasDecoration(base_type->id(), spv::Decoration::Block)) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << _.VkErrorID(6925)
                 << "In the Vulkan environment, cannot store to Uniform Blocks";
        }
      }
    }
  }

  const auto object_index = 1;
  const auto object_id = inst->GetOperandAs<uint32_t>(object_index);
  const auto object = _.FindDef(object_id);
  if (!object || !object->type_id()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpStore Object <id> " << _.getIdName(object_id)
           << " is not an object.";
  }
  const auto object_type = _.FindDef(object->type_id());
  if (!object_type || spv::Op::OpTypeVoid == object_type->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpStore Object <id> " << _.getIdName(object_id)
           << "s type is void.";
  }

  if (type && (type->id() != object_type->id())) {
    if (!_.options()->relax_struct_store ||
        type->opcode() != spv::Op::OpTypeStruct ||
        object_type->opcode() != spv::Op::OpTypeStruct) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpStore Pointer <id> " << _.getIdName(pointer_id)
             << "s type does not match Object <id> "
             << _.getIdName(object->id()) << "s type.";
    }

    // TODO: Check for layout compatible matricies and arrays as well.
    if (!AreLayoutCompatibleStructs(_, type, object_type)) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpStore Pointer <id> " << _.getIdName(pointer_id)
             << "s layout does not match Object <id> "
             << _.getIdName(object->id()) << "s layout.";
    }
  }

  if (auto error = CheckMemoryAccess(_, inst, 2)) return error;

  if (_.HasCapability(spv::Capability::Shader) &&
      _.ContainsLimitedUseIntOrFloatType(inst->type_id()) &&
      object_type->opcode() != spv::Op::OpTypePointer) {
    if (object_type->opcode() != spv::Op::OpTypeInt &&
        object_type->opcode() != spv::Op::OpTypeFloat &&
        object_type->opcode() != spv::Op::OpTypeVector &&
        object_type->opcode() != spv::Op::OpTypeMatrix) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "8- or 16-bit stores must be a scalar, vector or matrix type";
    }
  }

  if (spvIsVulkanEnv(_.context()->target_env) &&
      !_.options()->before_hlsl_legalization) {
    const auto isForbiddenType = [](const Instruction* type_inst) {
      auto opcode = type_inst->opcode();
      return opcode == spv::Op::OpTypeImage ||
             opcode == spv::Op::OpTypeSampler ||
             opcode == spv::Op::OpTypeSampledImage ||
             opcode == spv::Op::OpTypeAccelerationStructureKHR;
    };
    if (_.ContainsType(object_type->id(), isForbiddenType)) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << _.VkErrorID(6924)
             << "Cannot store to OpTypeImage, OpTypeSampler, "
                "OpTypeSampledImage, or OpTypeAccelerationStructureKHR objects";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateCopyMemoryMemoryAccess(ValidationState_t& _,
                                            const Instruction* inst) {
  assert(inst->opcode() == spv::Op::OpCopyMemory ||
         inst->opcode() == spv::Op::OpCopyMemorySized);
  const uint32_t first_access_index =
      inst->opcode() == spv::Op::OpCopyMemory ? 2 : 3;
  if (inst->operands().size() > first_access_index) {
    if (auto error = CheckMemoryAccess(_, inst, first_access_index))
      return error;

    const auto first_access = inst->GetOperandAs<uint32_t>(first_access_index);
    const uint32_t second_access_index =
        first_access_index + MemoryAccessNumWords(first_access);
    if (inst->operands().size() > second_access_index) {
      if (_.features().copy_memory_permits_two_memory_accesses) {
        if (auto error = CheckMemoryAccess(_, inst, second_access_index))
          return error;

        // In the two-access form in SPIR-V 1.4 and later:
        //  - the first is the target (write) access and it can't have
        //  make-visible.
        //  - the second is the source (read) access and it can't have
        //  make-available.
        if (first_access &
            uint32_t(spv::MemoryAccessMask::MakePointerVisibleKHR)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Target memory access must not include "
                    "MakePointerVisibleKHR";
        }
        const auto second_access =
            inst->GetOperandAs<uint32_t>(second_access_index);
        if (second_access &
            uint32_t(spv::MemoryAccessMask::MakePointerAvailableKHR)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Source memory access must not include "
                    "MakePointerAvailableKHR";
        }
      } else {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(static_cast<spv::Op>(inst->opcode()))
               << " with two memory access operands requires SPIR-V 1.4 or "
                  "later";
      }
    }
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateCopyMemory(ValidationState_t& _, const Instruction* inst) {
  const auto target_index = 0;
  const auto target_id = inst->GetOperandAs<uint32_t>(target_index);
  const auto target = _.FindDef(target_id);
  if (!target) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Target operand <id> " << _.getIdName(target_id)
           << " is not defined.";
  }

  const auto source_index = 1;
  const auto source_id = inst->GetOperandAs<uint32_t>(source_index);
  const auto source = _.FindDef(source_id);
  if (!source) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Source operand <id> " << _.getIdName(source_id)
           << " is not defined.";
  }

  const auto target_pointer_type = _.FindDef(target->type_id());
  if (!target_pointer_type ||
      (target_pointer_type->opcode() != spv::Op::OpTypePointer &&
       target_pointer_type->opcode() != spv::Op::OpTypeUntypedPointerKHR)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Target operand <id> " << _.getIdName(target_id)
           << " is not a pointer.";
  }

  const auto source_pointer_type = _.FindDef(source->type_id());
  if (!source_pointer_type ||
      (source_pointer_type->opcode() != spv::Op::OpTypePointer &&
       source_pointer_type->opcode() != spv::Op::OpTypeUntypedPointerKHR)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Source operand <id> " << _.getIdName(source_id)
           << " is not a pointer.";
  }

  if (inst->opcode() == spv::Op::OpCopyMemory) {
    const bool target_typed =
        target_pointer_type->opcode() == spv::Op::OpTypePointer;
    const bool source_typed =
        source_pointer_type->opcode() == spv::Op::OpTypePointer;
    Instruction* target_type = nullptr;
    Instruction* source_type = nullptr;
    if (target_typed) {
      target_type = _.FindDef(target_pointer_type->GetOperandAs<uint32_t>(2));

      if (!target_type || target_type->opcode() == spv::Op::OpTypeVoid) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "Target operand <id> " << _.getIdName(target_id)
               << " cannot be a void pointer.";
      }
    }

    if (source_typed) {
      source_type = _.FindDef(source_pointer_type->GetOperandAs<uint32_t>(2));
      if (!source_type || source_type->opcode() == spv::Op::OpTypeVoid) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "Source operand <id> " << _.getIdName(source_id)
               << " cannot be a void pointer.";
      }
    }

    if (target_type && source_type && target_type->id() != source_type->id()) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Target <id> " << _.getIdName(source_id)
             << "s type does not match Source <id> "
             << _.getIdName(source_type->id()) << "s type.";
    }

    if (!target_type && !source_type) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "One of Source or Target must be a typed pointer";
    }

    if (auto error = CheckMemoryAccess(_, inst, 2)) return error;
  } else {
    const auto size_id = inst->GetOperandAs<uint32_t>(2);
    const auto size = _.FindDef(size_id);
    if (!size) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Size operand <id> " << _.getIdName(size_id)
             << " is not defined.";
    }

    const auto size_type = _.FindDef(size->type_id());
    if (!_.IsIntScalarType(size_type->id())) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Size operand <id> " << _.getIdName(size_id)
             << " must be a scalar integer type.";
    }
    bool is_zero = true;
    switch (size->opcode()) {
      case spv::Op::OpConstantNull:
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "Size operand <id> " << _.getIdName(size_id)
               << " cannot be a constant zero.";
      case spv::Op::OpConstant:
        if (size_type->word(3) == 1 &&
            size->word(size->words().size() - 1) & 0x80000000) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << "Size operand <id> " << _.getIdName(size_id)
                 << " cannot have the sign bit set to 1.";
        }
        for (size_t i = 3; is_zero && i < size->words().size(); ++i) {
          is_zero &= (size->word(i) == 0);
        }
        if (is_zero) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << "Size operand <id> " << _.getIdName(size_id)
                 << " cannot be a constant zero.";
        }
        break;
      default:
        // Cannot infer any other opcodes.
        break;
    }

    if (_.HasCapability(spv::Capability::Shader)) {
      bool is_int = false;
      bool is_const = false;
      uint32_t value = 0;
      std::tie(is_int, is_const, value) = _.EvalInt32IfConst(size_id);
      if (is_const) {
        if (value % 4 != 0) {
          const auto source_sc =
              source_pointer_type->GetOperandAs<spv::StorageClass>(1);
          const auto target_sc =
              target_pointer_type->GetOperandAs<spv::StorageClass>(1);
          const bool int8 = _.HasCapability(spv::Capability::Int8);
          const bool ubo_int8 = _.HasCapability(
              spv::Capability::UniformAndStorageBuffer8BitAccess);
          const bool ssbo_int8 =
              _.HasCapability(spv::Capability::StorageBuffer8BitAccess) ||
              ubo_int8;
          const bool pc_int8 =
              _.HasCapability(spv::Capability::StoragePushConstant8);
          const bool wg_int8 = _.HasCapability(
              spv::Capability::WorkgroupMemoryExplicitLayout8BitAccessKHR);
          const bool int16 = _.HasCapability(spv::Capability::Int16) || int8;
          const bool ubo_int16 =
              _.HasCapability(
                  spv::Capability::UniformAndStorageBuffer16BitAccess) ||
              ubo_int8;
          const bool ssbo_int16 =
              _.HasCapability(spv::Capability::StorageBuffer16BitAccess) ||
              ubo_int16 || ssbo_int8;
          const bool pc_int16 =
              _.HasCapability(spv::Capability::StoragePushConstant16) ||
              pc_int8;
          const bool io_int16 =
              _.HasCapability(spv::Capability::StorageInputOutput16);
          const bool wg_int16 = _.HasCapability(
              spv::Capability::WorkgroupMemoryExplicitLayout16BitAccessKHR);

          bool source_int16_match = false;
          bool target_int16_match = false;
          bool source_int8_match = false;
          bool target_int8_match = false;
          switch (source_sc) {
            case spv::StorageClass::StorageBuffer:
              source_int16_match = ssbo_int16;
              source_int8_match = ssbo_int8;
              break;
            case spv::StorageClass::Uniform:
              source_int16_match = ubo_int16;
              source_int8_match = ubo_int8;
              break;
            case spv::StorageClass::PushConstant:
              source_int16_match = pc_int16;
              source_int8_match = pc_int8;
              break;
            case spv::StorageClass::Input:
            case spv::StorageClass::Output:
              source_int16_match = io_int16;
              break;
            case spv::StorageClass::Workgroup:
              source_int16_match = wg_int16;
              source_int8_match = wg_int8;
              break;
            default:
              break;
          }
          switch (target_sc) {
            case spv::StorageClass::StorageBuffer:
              target_int16_match = ssbo_int16;
              target_int8_match = ssbo_int8;
              break;
            case spv::StorageClass::Uniform:
              target_int16_match = ubo_int16;
              target_int8_match = ubo_int8;
              break;
            case spv::StorageClass::PushConstant:
              target_int16_match = pc_int16;
              target_int8_match = pc_int8;
              break;
            // Input is read-only so it cannot be the target pointer.
            case spv::StorageClass::Output:
              target_int16_match = io_int16;
              break;
            case spv::StorageClass::Workgroup:
              target_int16_match = wg_int16;
              target_int8_match = wg_int8;
              break;
            default:
              break;
          }
          if (!int8 && !int16 && !(source_int16_match && target_int16_match)) {
            return _.diag(SPV_ERROR_INVALID_ID, inst)
                   << "Size must be a multiple of 4";
          }
          if (value % 2 != 0) {
            if (!int8 && !(source_int8_match && target_int8_match)) {
              return _.diag(SPV_ERROR_INVALID_ID, inst)
                     << "Size must be a multiple of 2";
            }
          }
        }
      }
    }

    if (auto error = CheckMemoryAccess(_, inst, 3)) return error;
  }
  if (auto error = ValidateCopyMemoryMemoryAccess(_, inst)) return error;

  // Get past the pointers to avoid checking a pointer copy.
  if (target_pointer_type->opcode() == spv::Op::OpTypePointer) {
    auto sub_type = _.FindDef(target_pointer_type->GetOperandAs<uint32_t>(2));
    while (sub_type->opcode() == spv::Op::OpTypePointer) {
      sub_type = _.FindDef(sub_type->GetOperandAs<uint32_t>(2));
    }
    if (_.HasCapability(spv::Capability::Shader) &&
        _.ContainsLimitedUseIntOrFloatType(sub_type->id())) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Cannot copy memory of objects containing 8- or 16-bit types";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateAccessChain(ValidationState_t& _,
                                 const Instruction* inst) {
  std::string instr_name =
      "Op" + std::string(spvOpcodeString(static_cast<spv::Op>(inst->opcode())));

  const bool untyped_pointer = spvOpcodeGeneratesUntypedPointer(inst->opcode());

  // The result type must be OpTypePointer for regular access chains and an
  // OpTypeUntypedPointerKHR for untyped access chains.
  auto result_type = _.FindDef(inst->type_id());
  if (untyped_pointer) {
    if (!result_type ||
        spv::Op::OpTypeUntypedPointerKHR != result_type->opcode()) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "The Result Type of " << instr_name << " <id> "
             << _.getIdName(inst->id())
             << " must be OpTypeUntypedPointerKHR. Found Op"
             << spvOpcodeString(static_cast<spv::Op>(result_type->opcode()))
             << ".";
    }
  } else {
    if (!result_type || spv::Op::OpTypePointer != result_type->opcode()) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "The Result Type of " << instr_name << " <id> "
             << _.getIdName(inst->id()) << " must be OpTypePointer. Found Op"
             << spvOpcodeString(static_cast<spv::Op>(result_type->opcode()))
             << ".";
    }
  }

  if (untyped_pointer) {
    // Base type must be a non-pointer type.
    const auto base_type = _.FindDef(inst->GetOperandAs<uint32_t>(2));
    if (!base_type || !spvOpcodeGeneratesType(base_type->opcode()) ||
        base_type->opcode() == spv::Op::OpTypePointer ||
        base_type->opcode() == spv::Op::OpTypeUntypedPointerKHR) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Base type must be a non-pointer type";
    }
  }

  // Base must be a pointer, pointing to the base of a composite object.
  const auto base_index = untyped_pointer ? 3 : 2;
  const auto base_id = inst->GetOperandAs<uint32_t>(base_index);
  const auto base = _.FindDef(base_id);
  const auto base_type = _.FindDef(base->type_id());
  if (!base_type || !(spv::Op::OpTypePointer == base_type->opcode() ||
                      (untyped_pointer && spv::Op::OpTypeUntypedPointerKHR ==
                                              base_type->opcode()))) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "The Base <id> " << _.getIdName(base_id) << " in " << instr_name
           << " instruction must be a pointer.";
  }

  // The result pointer storage class and base pointer storage class must match.
  // Word 2 of OpTypePointer is the Storage Class.
  auto result_type_storage_class = result_type->word(2);
  auto base_type_storage_class = base_type->word(2);
  if (result_type_storage_class != base_type_storage_class) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "The result pointer storage class and base "
              "pointer storage class in "
           << instr_name << " do not match.";
  }

  // The type pointed to by OpTypePointer (word 3) must be a composite type.
  auto type_pointee = untyped_pointer
                          ? _.FindDef(inst->GetOperandAs<uint32_t>(2))
                          : _.FindDef(base_type->word(3));

  // Check Universal Limit (SPIR-V Spec. Section 2.17).
  // The number of indexes passed to OpAccessChain may not exceed 255
  // The instruction includes 4 words + N words (for N indexes)
  size_t num_indexes = inst->words().size() - 4;
  if (inst->opcode() == spv::Op::OpPtrAccessChain ||
      inst->opcode() == spv::Op::OpInBoundsPtrAccessChain ||
      inst->opcode() == spv::Op::OpUntypedPtrAccessChainKHR ||
      inst->opcode() == spv::Op::OpUntypedInBoundsPtrAccessChainKHR) {
    // In pointer access chains, the element operand is required, but not
    // counted as an index.
    --num_indexes;
  }
  const size_t num_indexes_limit =
      _.options()->universal_limits_.max_access_chain_indexes;
  if (num_indexes > num_indexes_limit) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "The number of indexes in " << instr_name << " may not exceed "
           << num_indexes_limit << ". Found " << num_indexes << " indexes.";
  }
  // Indexes walk the type hierarchy to the desired depth, potentially down to
  // scalar granularity. The first index in Indexes will select the top-level
  // member/element/component/element of the base composite. All composite
  // constituents use zero-based numbering, as described by their OpType...
  // instruction. The second index will apply similarly to that result, and so
  // on. Once any non-composite type is reached, there must be no remaining
  // (unused) indexes.
  auto starting_index = untyped_pointer ? 5 : 4;
  if (inst->opcode() == spv::Op::OpPtrAccessChain ||
      inst->opcode() == spv::Op::OpInBoundsPtrAccessChain ||
      inst->opcode() == spv::Op::OpUntypedPtrAccessChainKHR ||
      inst->opcode() == spv::Op::OpUntypedInBoundsPtrAccessChainKHR) {
    ++starting_index;
  }
  for (size_t i = starting_index; i < inst->words().size(); ++i) {
    const uint32_t cur_word = inst->words()[i];
    // Earlier ID checks ensure that cur_word definition exists.
    auto cur_word_instr = _.FindDef(cur_word);
    // The index must be a scalar integer type (See OpAccessChain in the Spec.)
    auto index_type = _.FindDef(cur_word_instr->type_id());
    if (!index_type || spv::Op::OpTypeInt != index_type->opcode()) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Indexes passed to " << instr_name
             << " must be of type integer.";
    }
    switch (type_pointee->opcode()) {
      case spv::Op::OpTypeMatrix:
      case spv::Op::OpTypeVector:
      case spv::Op::OpTypeCooperativeVectorNV:
      case spv::Op::OpTypeCooperativeMatrixNV:
      case spv::Op::OpTypeCooperativeMatrixKHR:
      case spv::Op::OpTypeArray:
      case spv::Op::OpTypeRuntimeArray:
      case spv::Op::OpTypeNodePayloadArrayAMDX: {
        // In OpTypeMatrix, OpTypeVector, spv::Op::OpTypeCooperativeMatrixNV,
        // OpTypeCooperativeVectorNV, OpTypeArray, and OpTypeRuntimeArray, word
        // 2 is the Element Type.
        type_pointee = _.FindDef(type_pointee->word(2));
        break;
      }
      case spv::Op::OpTypeStruct: {
        // In case of structures, there is an additional constraint on the
        // index: the index must be an OpConstant.
        int64_t cur_index;
        if (!_.EvalConstantValInt64(cur_word, &cur_index)) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << "The <id> passed to " << instr_name << " to index "
                 << _.getIdName(cur_word)
                 << " into a "
                    "structure must be an OpConstant.";
        }

        // The index points to the struct member we want, therefore, the index
        // should be less than the number of struct members.
        const int64_t num_struct_members =
            static_cast<int64_t>(type_pointee->words().size() - 2);
        if (cur_index >= num_struct_members || cur_index < 0) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << "Index " << _.getIdName(cur_word)
                 << " is out of bounds: " << instr_name << " cannot find index "
                 << cur_index << " into the structure <id> "
                 << _.getIdName(type_pointee->id()) << ". This structure has "
                 << num_struct_members << " members. Largest valid index is "
                 << num_struct_members - 1 << ".";
        }
        // Struct members IDs start at word 2 of OpTypeStruct.
        const size_t word_index = static_cast<size_t>(cur_index) + 2;
        auto structMemberId = type_pointee->word(word_index);
        type_pointee = _.FindDef(structMemberId);
        break;
      }
      default: {
        // Give an error. reached non-composite type while indexes still remain.
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << instr_name
               << " reached non-composite type while indexes "
                  "still remain to be traversed.";
      }
    }
  }

  if (!untyped_pointer) {
    // Result type is a pointer. Find out what it's pointing to.
    // This will be used to make sure the indexing results in the same type.
    // OpTypePointer word 3 is the type being pointed to.
    const auto result_type_pointee = _.FindDef(result_type->word(3));
    // At this point, we have fully walked down from the base using the indeces.
    // The type being pointed to should be the same as the result type.
    if (type_pointee->id() != result_type_pointee->id()) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << instr_name << " result type (Op"
             << spvOpcodeString(
                    static_cast<spv::Op>(result_type_pointee->opcode()))
             << ") does not match the type that results from indexing into the "
                "base "
                "<id> (Op"
             << spvOpcodeString(static_cast<spv::Op>(type_pointee->opcode()))
             << ").";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateRawAccessChain(ValidationState_t& _,
                                    const Instruction* inst) {
  std::string instr_name = "Op" + std::string(spvOpcodeString(inst->opcode()));

  // The result type must be OpTypePointer.
  const auto result_type = _.FindDef(inst->type_id());
  if (spv::Op::OpTypePointer != result_type->opcode()) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "The Result Type of " << instr_name << " <id> "
           << _.getIdName(inst->id()) << " must be OpTypePointer. Found Op"
           << spvOpcodeString(result_type->opcode()) << '.';
  }

  // The pointed storage class must be valid.
  const auto storage_class = result_type->GetOperandAs<spv::StorageClass>(1);
  if (storage_class != spv::StorageClass::StorageBuffer &&
      storage_class != spv::StorageClass::PhysicalStorageBuffer &&
      storage_class != spv::StorageClass::Uniform) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "The Result Type of " << instr_name << " <id> "
           << _.getIdName(inst->id())
           << " must point to a storage class of "
              "StorageBuffer, PhysicalStorageBuffer, or Uniform.";
  }

  // The pointed type must not be one in the list below.
  const auto result_type_pointee =
      _.FindDef(result_type->GetOperandAs<uint32_t>(2));
  if (result_type_pointee->opcode() == spv::Op::OpTypeArray ||
      result_type_pointee->opcode() == spv::Op::OpTypeMatrix ||
      result_type_pointee->opcode() == spv::Op::OpTypeStruct) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "The Result Type of " << instr_name << " <id> "
           << _.getIdName(inst->id())
           << " must not point to "
              "OpTypeArray, OpTypeMatrix, or OpTypeStruct.";
  }

  // Validate Stride is a OpConstant.
  const auto stride = _.FindDef(inst->GetOperandAs<uint32_t>(3));
  if (stride->opcode() != spv::Op::OpConstant) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "The Stride of " << instr_name << " <id> "
           << _.getIdName(inst->id()) << " must be OpConstant. Found Op"
           << spvOpcodeString(stride->opcode()) << '.';
  }
  // Stride type must be OpTypeInt
  const auto stride_type = _.FindDef(stride->type_id());
  if (stride_type->opcode() != spv::Op::OpTypeInt) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "The type of Stride of " << instr_name << " <id> "
           << _.getIdName(inst->id()) << " must be OpTypeInt. Found Op"
           << spvOpcodeString(stride_type->opcode()) << '.';
  }

  // Index and Offset type must be OpTypeInt with a width of 32
  const auto ValidateType = [&](const char* name,
                                int operandIndex) -> spv_result_t {
    const auto value = _.FindDef(inst->GetOperandAs<uint32_t>(operandIndex));
    const auto value_type = _.FindDef(value->type_id());
    if (value_type->opcode() != spv::Op::OpTypeInt) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "The type of " << name << " of " << instr_name << " <id> "
             << _.getIdName(inst->id()) << " must be OpTypeInt. Found Op"
             << spvOpcodeString(value_type->opcode()) << '.';
    }
    const auto width = value_type->GetOperandAs<uint32_t>(1);
    if (width != 32) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "The integer width of " << name << " of " << instr_name
             << " <id> " << _.getIdName(inst->id()) << " must be 32. Found "
             << width << '.';
    }
    return SPV_SUCCESS;
  };
  spv_result_t result;
  result = ValidateType("Index", 4);
  if (result != SPV_SUCCESS) {
    return result;
  }
  result = ValidateType("Offset", 5);
  if (result != SPV_SUCCESS) {
    return result;
  }

  uint32_t access_operands = 0;
  if (inst->operands().size() >= 7) {
    access_operands = inst->GetOperandAs<uint32_t>(6);
  }
  if (access_operands &
      uint32_t(spv::RawAccessChainOperandsMask::RobustnessPerElementNV)) {
    uint64_t stride_value = 0;
    if (_.EvalConstantValUint64(stride->id(), &stride_value) &&
        stride_value == 0) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Stride must not be zero when per-element robustness is used.";
    }
  }
  if (access_operands &
          uint32_t(spv::RawAccessChainOperandsMask::RobustnessPerComponentNV) ||
      access_operands &
          uint32_t(spv::RawAccessChainOperandsMask::RobustnessPerElementNV)) {
    if (storage_class == spv::StorageClass::PhysicalStorageBuffer) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Storage class cannot be PhysicalStorageBuffer when "
                "raw access chain robustness is used.";
    }
  }
  if (access_operands &
          uint32_t(spv::RawAccessChainOperandsMask::RobustnessPerComponentNV) &&
      access_operands &
          uint32_t(spv::RawAccessChainOperandsMask::RobustnessPerElementNV)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Per-component robustness and per-element robustness are "
              "mutually exclusive.";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidatePtrAccessChain(ValidationState_t& _,
                                    const Instruction* inst) {
  if (_.addressing_model() == spv::AddressingModel::Logical &&
      inst->opcode() == spv::Op::OpPtrAccessChain) {
    if (!_.features().variable_pointers) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Generating variable pointers requires capability "
             << "VariablePointers or VariablePointersStorageBuffer";
    }
  }

  // Need to call first, will make sure Base is a valid ID
  if (auto error = ValidateAccessChain(_, inst)) return error;

  const bool untyped_pointer = spvOpcodeGeneratesUntypedPointer(inst->opcode());

  const auto base_id = inst->GetOperandAs<uint32_t>(2);
  const auto base = _.FindDef(base_id);
  const auto base_type = untyped_pointer
                             ? _.FindDef(inst->GetOperandAs<uint32_t>(2))
                             : _.FindDef(base->type_id());
  const auto base_type_storage_class =
      base_type->GetOperandAs<spv::StorageClass>(1);

  if (_.HasCapability(spv::Capability::Shader) &&
      (base_type_storage_class == spv::StorageClass::Uniform ||
       base_type_storage_class == spv::StorageClass::StorageBuffer ||
       base_type_storage_class == spv::StorageClass::PhysicalStorageBuffer ||
       base_type_storage_class == spv::StorageClass::PushConstant ||
       (_.HasCapability(spv::Capability::WorkgroupMemoryExplicitLayoutKHR) &&
        base_type_storage_class == spv::StorageClass::Workgroup)) &&
      !_.HasDecoration(base_type->id(), spv::Decoration::ArrayStride)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "OpPtrAccessChain must have a Base whose type is decorated "
              "with ArrayStride";
  }

  if (spvIsVulkanEnv(_.context()->target_env)) {
    const auto untyped_cap =
        untyped_pointer && _.HasCapability(spv::Capability::UntypedPointersKHR);
    if (base_type_storage_class == spv::StorageClass::Workgroup) {
      if (!_.HasCapability(spv::Capability::VariablePointers) && !untyped_cap) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << _.VkErrorID(7651)
               << "OpPtrAccessChain Base operand pointing to Workgroup "
                  "storage class must use VariablePointers capability";
      }
    } else if (base_type_storage_class == spv::StorageClass::StorageBuffer) {
      if (!_.features().variable_pointers && !untyped_cap) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << _.VkErrorID(7652)
               << "OpPtrAccessChain Base operand pointing to StorageBuffer "
                  "storage class must use VariablePointers or "
                  "VariablePointersStorageBuffer capability";
      }
    } else if (base_type_storage_class !=
                   spv::StorageClass::PhysicalStorageBuffer &&
               !untyped_cap) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << _.VkErrorID(7650)
             << "OpPtrAccessChain Base operand must point to Workgroup, "
                "StorageBuffer, or PhysicalStorageBuffer storage class";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateArrayLength(ValidationState_t& state,
                                 const Instruction* inst) {
  std::string instr_name =
      "Op" + std::string(spvOpcodeString(static_cast<spv::Op>(inst->opcode())));

  // Result type must be a 32-bit unsigned int.
  auto result_type = state.FindDef(inst->type_id());
  if (result_type->opcode() != spv::Op::OpTypeInt ||
      result_type->GetOperandAs<uint32_t>(1) != 32 ||
      result_type->GetOperandAs<uint32_t>(2) != 0) {
    return state.diag(SPV_ERROR_INVALID_ID, inst)
           << "The Result Type of " << instr_name << " <id> "
           << state.getIdName(inst->id())
           << " must be OpTypeInt with width 32 and signedness 0.";
  }

  const bool untyped = inst->opcode() == spv::Op::OpUntypedArrayLengthKHR;
  auto pointer_ty_id = state.GetOperandTypeId(inst, (untyped ? 3 : 2));
  auto pointer_ty = state.FindDef(pointer_ty_id);
  if (untyped) {
    if (pointer_ty->opcode() != spv::Op::OpTypeUntypedPointerKHR) {
      return state.diag(SPV_ERROR_INVALID_ID, inst)
             << "Pointer must be an untyped pointer";
    }
  } else if (pointer_ty->opcode() != spv::Op::OpTypePointer) {
    return state.diag(SPV_ERROR_INVALID_ID, inst)
           << "The Structure's type in " << instr_name << " <id> "
           << state.getIdName(inst->id())
           << " must be a pointer to an OpTypeStruct.";
  }

  Instruction* structure_type = nullptr;
  if (untyped) {
    structure_type = state.FindDef(inst->GetOperandAs<uint32_t>(2));
  } else {
    structure_type = state.FindDef(pointer_ty->GetOperandAs<uint32_t>(2));
  }

  if (structure_type->opcode() != spv::Op::OpTypeStruct) {
    return state.diag(SPV_ERROR_INVALID_ID, inst)
           << "The Structure's type in " << instr_name << " <id> "
           << state.getIdName(inst->id())
           << " must be a pointer to an OpTypeStruct.";
  }

  auto num_of_members = structure_type->operands().size() - 1;
  auto last_member =
      state.FindDef(structure_type->GetOperandAs<uint32_t>(num_of_members));
  if (last_member->opcode() != spv::Op::OpTypeRuntimeArray) {
    return state.diag(SPV_ERROR_INVALID_ID, inst)
           << "The Structure's last member in " << instr_name << " <id> "
           << state.getIdName(inst->id()) << " must be an OpTypeRuntimeArray.";
  }

  // The array member must the index of the last element (the run time
  // array).
  const auto index = untyped ? 4 : 3;
  if (inst->GetOperandAs<uint32_t>(index) != num_of_members - 1) {
    return state.diag(SPV_ERROR_INVALID_ID, inst)
           << "The array member in " << instr_name << " <id> "
           << state.getIdName(inst->id())
           << " must be the last member of the struct.";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateCooperativeMatrixLengthNV(ValidationState_t& state,
                                               const Instruction* inst) {
  std::string instr_name =
      "Op" + std::string(spvOpcodeString(static_cast<spv::Op>(inst->opcode())));

  // Result type must be a 32-bit unsigned int.
  auto result_type = state.FindDef(inst->type_id());
  if (result_type->opcode() != spv::Op::OpTypeInt ||
      result_type->GetOperandAs<uint32_t>(1) != 32 ||
      result_type->GetOperandAs<uint32_t>(2) != 0) {
    return state.diag(SPV_ERROR_INVALID_ID, inst)
           << "The Result Type of " << instr_name << " <id> "
           << state.getIdName(inst->id())
           << " must be OpTypeInt with width 32 and signedness 0.";
  }

  bool isKhr = inst->opcode() == spv::Op::OpCooperativeMatrixLengthKHR;
  auto type_id = inst->GetOperandAs<uint32_t>(2);
  auto type = state.FindDef(type_id);
  if (isKhr && type->opcode() != spv::Op::OpTypeCooperativeMatrixKHR) {
    return state.diag(SPV_ERROR_INVALID_ID, inst)
           << "The type in " << instr_name << " <id> "
           << state.getIdName(type_id)
           << " must be OpTypeCooperativeMatrixKHR.";
  } else if (!isKhr && type->opcode() != spv::Op::OpTypeCooperativeMatrixNV) {
    return state.diag(SPV_ERROR_INVALID_ID, inst)
           << "The type in " << instr_name << " <id> "
           << state.getIdName(type_id) << " must be OpTypeCooperativeMatrixNV.";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateCooperativeMatrixLoadStoreNV(ValidationState_t& _,
                                                  const Instruction* inst) {
  uint32_t type_id;
  const char* opname;
  if (inst->opcode() == spv::Op::OpCooperativeMatrixLoadNV) {
    type_id = inst->type_id();
    opname = "spv::Op::OpCooperativeMatrixLoadNV";
  } else {
    // get Object operand's type
    type_id = _.FindDef(inst->GetOperandAs<uint32_t>(1))->type_id();
    opname = "spv::Op::OpCooperativeMatrixStoreNV";
  }

  auto matrix_type = _.FindDef(type_id);

  if (matrix_type->opcode() != spv::Op::OpTypeCooperativeMatrixNV) {
    if (inst->opcode() == spv::Op::OpCooperativeMatrixLoadNV) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "spv::Op::OpCooperativeMatrixLoadNV Result Type <id> "
             << _.getIdName(type_id) << " is not a cooperative matrix type.";
    } else {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "spv::Op::OpCooperativeMatrixStoreNV Object type <id> "
             << _.getIdName(type_id) << " is not a cooperative matrix type.";
    }
  }

  const auto pointer_index =
      (inst->opcode() == spv::Op::OpCooperativeMatrixLoadNV) ? 2u : 0u;
  const auto pointer_id = inst->GetOperandAs<uint32_t>(pointer_index);
  const auto pointer = _.FindDef(pointer_id);
  if (!pointer ||
      ((_.addressing_model() == spv::AddressingModel::Logical) &&
       ((!_.features().variable_pointers &&
         !spvOpcodeReturnsLogicalPointer(pointer->opcode())) ||
        (_.features().variable_pointers &&
         !spvOpcodeReturnsLogicalVariablePointer(pointer->opcode()))))) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opname << " Pointer <id> " << _.getIdName(pointer_id)
           << " is not a logical pointer.";
  }

  const auto pointer_type_id = pointer->type_id();
  const auto pointer_type = _.FindDef(pointer_type_id);
  if (!pointer_type || pointer_type->opcode() != spv::Op::OpTypePointer) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opname << " type for pointer <id> " << _.getIdName(pointer_id)
           << " is not a pointer type.";
  }

  const auto storage_class_index = 1u;
  const auto storage_class =
      pointer_type->GetOperandAs<spv::StorageClass>(storage_class_index);

  if (storage_class != spv::StorageClass::Workgroup &&
      storage_class != spv::StorageClass::StorageBuffer &&
      storage_class != spv::StorageClass::PhysicalStorageBuffer) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opname << " storage class for pointer type <id> "
           << _.getIdName(pointer_type_id)
           << " is not Workgroup or StorageBuffer.";
  }

  const auto pointee_id = pointer_type->GetOperandAs<uint32_t>(2);
  const auto pointee_type = _.FindDef(pointee_id);
  if (!pointee_type || !(_.IsIntScalarOrVectorType(pointee_id) ||
                         _.IsFloatScalarOrVectorType(pointee_id))) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opname << " Pointer <id> " << _.getIdName(pointer->id())
           << "s Type must be a scalar or vector type.";
  }

  const auto stride_index =
      (inst->opcode() == spv::Op::OpCooperativeMatrixLoadNV) ? 3u : 2u;
  const auto stride_id = inst->GetOperandAs<uint32_t>(stride_index);
  const auto stride = _.FindDef(stride_id);
  if (!stride || !_.IsIntScalarType(stride->type_id())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Stride operand <id> " << _.getIdName(stride_id)
           << " must be a scalar integer type.";
  }

  const auto colmajor_index =
      (inst->opcode() == spv::Op::OpCooperativeMatrixLoadNV) ? 4u : 3u;
  const auto colmajor_id = inst->GetOperandAs<uint32_t>(colmajor_index);
  const auto colmajor = _.FindDef(colmajor_id);
  if (!colmajor || !_.IsBoolScalarType(colmajor->type_id()) ||
      !(spvOpcodeIsConstant(colmajor->opcode()) ||
        spvOpcodeIsSpecConstant(colmajor->opcode()))) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Column Major operand <id> " << _.getIdName(colmajor_id)
           << " must be a boolean constant instruction.";
  }

  const auto memory_access_index =
      (inst->opcode() == spv::Op::OpCooperativeMatrixLoadNV) ? 5u : 4u;
  if (inst->operands().size() > memory_access_index) {
    if (auto error = CheckMemoryAccess(_, inst, memory_access_index))
      return error;
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateCooperativeMatrixLoadStoreKHR(ValidationState_t& _,
                                                   const Instruction* inst) {
  uint32_t type_id;
  const char* opname;
  if (inst->opcode() == spv::Op::OpCooperativeMatrixLoadKHR) {
    type_id = inst->type_id();
    opname = "spv::Op::OpCooperativeMatrixLoadKHR";
  } else {
    // get Object operand's type
    type_id = _.FindDef(inst->GetOperandAs<uint32_t>(1))->type_id();
    opname = "spv::Op::OpCooperativeMatrixStoreKHR";
  }

  auto matrix_type = _.FindDef(type_id);

  if (matrix_type->opcode() != spv::Op::OpTypeCooperativeMatrixKHR) {
    if (inst->opcode() == spv::Op::OpCooperativeMatrixLoadKHR) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "spv::Op::OpCooperativeMatrixLoadKHR Result Type <id> "
             << _.getIdName(type_id) << " is not a cooperative matrix type.";
    } else {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "spv::Op::OpCooperativeMatrixStoreKHR Object type <id> "
             << _.getIdName(type_id) << " is not a cooperative matrix type.";
    }
  }

  const auto pointer_index =
      (inst->opcode() == spv::Op::OpCooperativeMatrixLoadKHR) ? 2u : 0u;
  const auto pointer_id = inst->GetOperandAs<uint32_t>(pointer_index);
  const auto pointer = _.FindDef(pointer_id);
  if (!pointer ||
      ((_.addressing_model() == spv::AddressingModel::Logical) &&
       ((!_.features().variable_pointers &&
         !spvOpcodeReturnsLogicalPointer(pointer->opcode())) ||
        (_.features().variable_pointers &&
         !spvOpcodeReturnsLogicalVariablePointer(pointer->opcode()))))) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opname << " Pointer <id> " << _.getIdName(pointer_id)
           << " is not a logical pointer.";
  }

  const auto pointer_type_id = pointer->type_id();
  const auto pointer_type = _.FindDef(pointer_type_id);
  if (!pointer_type ||
      !(pointer_type->opcode() == spv::Op::OpTypePointer ||
        pointer_type->opcode() == spv::Op::OpTypeUntypedPointerKHR)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opname << " type for pointer <id> " << _.getIdName(pointer_id)
           << " is not a pointer type.";
  }

  const bool untyped =
      pointer_type->opcode() == spv::Op::OpTypeUntypedPointerKHR;
  const auto storage_class_index = 1u;
  const auto storage_class =
      pointer_type->GetOperandAs<spv::StorageClass>(storage_class_index);

  if (spvIsVulkanEnv(_.context()->target_env)) {
    if (storage_class != spv::StorageClass::Workgroup &&
        storage_class != spv::StorageClass::StorageBuffer &&
        storage_class != spv::StorageClass::PhysicalStorageBuffer) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << _.VkErrorID(8973) << opname
             << " storage class for pointer type <id> "
             << _.getIdName(pointer_type_id)
             << " is not Workgroup, StorageBuffer, or PhysicalStorageBuffer.";
    }
  }

  if (!untyped) {
    const auto pointee_id = pointer_type->GetOperandAs<uint32_t>(2);
    const auto pointee_type = _.FindDef(pointee_id);
    if (!pointee_type || !(_.IsIntScalarOrVectorType(pointee_id) ||
                           _.IsFloatScalarOrVectorType(pointee_id))) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << opname << " Pointer <id> " << _.getIdName(pointer->id())
             << "s Type must be a scalar or vector type.";
    }
  }

  const auto layout_index =
      (inst->opcode() == spv::Op::OpCooperativeMatrixLoadKHR) ? 3u : 2u;
  const auto layout_id = inst->GetOperandAs<uint32_t>(layout_index);
  const auto layout_inst = _.FindDef(layout_id);
  if (!layout_inst || !_.IsIntScalarType(layout_inst->type_id()) ||
      !spvOpcodeIsConstant(layout_inst->opcode())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "MemoryLayout operand <id> " << _.getIdName(layout_id)
           << " must be a 32-bit integer constant instruction.";
  }

  bool stride_required = false;
  uint64_t layout;
  if (_.EvalConstantValUint64(layout_id, &layout)) {
    stride_required =
        (layout == (uint64_t)spv::CooperativeMatrixLayout::RowMajorKHR) ||
        (layout == (uint64_t)spv::CooperativeMatrixLayout::ColumnMajorKHR);
  }

  const auto stride_index =
      (inst->opcode() == spv::Op::OpCooperativeMatrixLoadKHR) ? 4u : 3u;
  if (inst->operands().size() > stride_index) {
    const auto stride_id = inst->GetOperandAs<uint32_t>(stride_index);
    const auto stride = _.FindDef(stride_id);
    if (!stride || !_.IsIntScalarType(stride->type_id())) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Stride operand <id> " << _.getIdName(stride_id)
             << " must be a scalar integer type.";
    }
  } else if (stride_required) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "MemoryLayout " << layout << " requires a Stride.";
  }

  const auto memory_access_index =
      (inst->opcode() == spv::Op::OpCooperativeMatrixLoadKHR) ? 5u : 4u;
  if (inst->operands().size() > memory_access_index) {
    if (auto error = CheckMemoryAccess(_, inst, memory_access_index))
      return error;
  }

  return SPV_SUCCESS;
}

// Returns the number of instruction words taken up by a tensor addressing
// operands argument and its implied operands.
int TensorAddressingOperandsNumWords(spv::TensorAddressingOperandsMask mask) {
  int result = 1;  // Count the mask
  if ((mask & spv::TensorAddressingOperandsMask::TensorView) !=
      spv::TensorAddressingOperandsMask::MaskNone)
    ++result;
  if ((mask & spv::TensorAddressingOperandsMask::DecodeFunc) !=
      spv::TensorAddressingOperandsMask::MaskNone)
    ++result;
  return result;
}

spv_result_t ValidateCooperativeMatrixLoadStoreTensorNV(
    ValidationState_t& _, const Instruction* inst) {
  uint32_t type_id;
  const char* opname;
  if (inst->opcode() == spv::Op::OpCooperativeMatrixLoadTensorNV) {
    type_id = inst->type_id();
    opname = "spv::Op::OpCooperativeMatrixLoadTensorNV";
  } else {
    // get Object operand's type
    type_id = _.FindDef(inst->GetOperandAs<uint32_t>(1))->type_id();
    opname = "spv::Op::OpCooperativeMatrixStoreTensorNV";
  }

  auto matrix_type = _.FindDef(type_id);

  if (matrix_type->opcode() != spv::Op::OpTypeCooperativeMatrixKHR) {
    if (inst->opcode() == spv::Op::OpCooperativeMatrixLoadTensorNV) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "spv::Op::OpCooperativeMatrixLoadTensorNV Result Type <id> "
             << _.getIdName(type_id) << " is not a cooperative matrix type.";
    } else {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "spv::Op::OpCooperativeMatrixStoreTensorNV Object type <id> "
             << _.getIdName(type_id) << " is not a cooperative matrix type.";
    }
  }

  const auto pointer_index =
      (inst->opcode() == spv::Op::OpCooperativeMatrixLoadTensorNV) ? 2u : 0u;
  const auto pointer_id = inst->GetOperandAs<uint32_t>(pointer_index);
  const auto pointer = _.FindDef(pointer_id);
  if (!pointer ||
      ((_.addressing_model() == spv::AddressingModel::Logical) &&
       ((!_.features().variable_pointers &&
         !spvOpcodeReturnsLogicalPointer(pointer->opcode())) ||
        (_.features().variable_pointers &&
         !spvOpcodeReturnsLogicalVariablePointer(pointer->opcode()))))) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opname << " Pointer <id> " << _.getIdName(pointer_id)
           << " is not a logical pointer.";
  }

  const auto pointer_type_id = pointer->type_id();
  const auto pointer_type = _.FindDef(pointer_type_id);
  if (!pointer_type || pointer_type->opcode() != spv::Op::OpTypePointer) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opname << " type for pointer <id> " << _.getIdName(pointer_id)
           << " is not a pointer type.";
  }

  const auto storage_class_index = 1u;
  const auto storage_class =
      pointer_type->GetOperandAs<spv::StorageClass>(storage_class_index);

  if (storage_class != spv::StorageClass::Workgroup &&
      storage_class != spv::StorageClass::StorageBuffer &&
      storage_class != spv::StorageClass::PhysicalStorageBuffer) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << _.VkErrorID(8973) << opname
           << " storage class for pointer type <id> "
           << _.getIdName(pointer_type_id)
           << " is not Workgroup, StorageBuffer, or PhysicalStorageBuffer.";
  }

  if (inst->opcode() == spv::Op::OpCooperativeMatrixLoadTensorNV) {
    const auto object_index = 3;
    const auto object_id = inst->GetOperandAs<uint32_t>(object_index);
    const auto object = _.FindDef(object_id);
    if (!object || object->type_id() != type_id) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << opname << " Object <id> " << _.getIdName(object_id)
             << " type does not match Result Type.";
    }
  }

  const auto tensor_layout_index =
      (inst->opcode() == spv::Op::OpCooperativeMatrixLoadTensorNV) ? 4u : 2u;
  const auto tensor_layout_id =
      inst->GetOperandAs<uint32_t>(tensor_layout_index);
  const auto tensor_layout = _.FindDef(tensor_layout_id);
  if (!tensor_layout || _.FindDef(tensor_layout->type_id())->opcode() !=
                            spv::Op::OpTypeTensorLayoutNV) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opname << " TensorLayout <id> " << _.getIdName(tensor_layout_id)
           << " does not have a tensor layout type.";
  }

  const auto memory_access_index =
      (inst->opcode() == spv::Op::OpCooperativeMatrixLoadTensorNV) ? 5u : 3u;
  if (inst->operands().size() > memory_access_index) {
    if (auto error = CheckMemoryAccess(_, inst, memory_access_index))
      return error;
  }

  const auto memory_access_mask =
      inst->GetOperandAs<uint32_t>(memory_access_index);
  const auto tensor_operands_index =
      memory_access_index + MemoryAccessNumWords(memory_access_mask);
  const auto tensor_operands =
      inst->GetOperandAs<spv::TensorAddressingOperandsMask>(
          tensor_operands_index);

  if (inst->operands().size() <
      tensor_operands_index +
          TensorAddressingOperandsNumWords(tensor_operands)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opname << " not enough tensor addressing operands.";
  }

  uint32_t tensor_operand_index = tensor_operands_index + 1;
  if ((tensor_operands & spv::TensorAddressingOperandsMask::TensorView) !=
      spv::TensorAddressingOperandsMask::MaskNone) {
    const auto tensor_view_id =
        inst->GetOperandAs<uint32_t>(tensor_operand_index);
    const auto tensor_view = _.FindDef(tensor_view_id);
    if (!tensor_view || _.FindDef(tensor_view->type_id())->opcode() !=
                            spv::Op::OpTypeTensorViewNV) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << opname << " TensorView <id> " << _.getIdName(tensor_view_id)
             << " does not have a tensor view type.";
    }

    tensor_operand_index++;
  }

  if ((tensor_operands & spv::TensorAddressingOperandsMask::DecodeFunc) !=
      spv::TensorAddressingOperandsMask::MaskNone) {
    if (inst->opcode() == spv::Op::OpCooperativeMatrixStoreTensorNV) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpCooperativeMatrixStoreTensorNV does not support DecodeFunc.";
    }
    const auto decode_func_id =
        inst->GetOperandAs<uint32_t>(tensor_operand_index);
    const auto decode_func = _.FindDef(decode_func_id);

    if (!decode_func || decode_func->opcode() != spv::Op::OpFunction) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << opname << " DecodeFunc <id> " << _.getIdName(decode_func_id)
             << " is not a function.";
    }

    const auto component_type_index = 1;
    const auto component_type_id =
        matrix_type->GetOperandAs<uint32_t>(component_type_index);

    const auto function_type =
        _.FindDef(decode_func->GetOperandAs<uint32_t>(3));
    if (function_type->GetOperandAs<uint32_t>(1) != component_type_id) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << opname << " DecodeFunc <id> " << _.getIdName(decode_func_id)
             << " return type must match matrix component type.";
    }

    const auto decode_ptr_type_id = function_type->GetOperandAs<uint32_t>(2);
    const auto decode_ptr_type = _.FindDef(decode_ptr_type_id);
    auto decode_storage_class =
        decode_ptr_type->GetOperandAs<spv::StorageClass>(storage_class_index);

    if (decode_storage_class != spv::StorageClass::PhysicalStorageBuffer) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << opname << " DecodeFunc <id> " << _.getIdName(decode_func_id)
             << " first parameter must be pointer to PhysicalStorageBuffer.";
    }

    const auto tensor_layout_type = _.FindDef(tensor_layout->type_id());

    for (uint32_t param = 3; param < 5; ++param) {
      const auto param_type_id = function_type->GetOperandAs<uint32_t>(param);
      const auto param_type = _.FindDef(param_type_id);
      if (param_type->opcode() != spv::Op::OpTypeArray) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << opname << " DecodeFunc <id> " << _.getIdName(decode_func_id)
               << " second/third parameter must be array of 32-bit integer "
                  "with "
               << " dimension equal to the tensor dimension.";
      }
      const auto length_index = 2u;
      uint64_t array_length;
      if (_.EvalConstantValUint64(
              param_type->GetOperandAs<uint32_t>(length_index),
              &array_length)) {
        const auto tensor_layout_dim_id =
            tensor_layout_type->GetOperandAs<uint32_t>(1);
        uint64_t dim_value;
        if (_.EvalConstantValUint64(tensor_layout_dim_id, &dim_value)) {
          if (array_length != dim_value) {
            return _.diag(SPV_ERROR_INVALID_ID, inst)
                   << opname << " DecodeFunc <id> "
                   << _.getIdName(decode_func_id)
                   << " second/third parameter must be array of 32-bit integer "
                      "with "
                   << " dimension equal to the tensor dimension.";
          }
        }
      }
    }

    tensor_operand_index++;
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateInt32Operand(ValidationState_t& _, const Instruction* inst,
                                  uint32_t operand_index,
                                  const char* opcode_name,
                                  const char* operand_name) {
  const auto type_id =
      _.FindDef(inst->GetOperandAs<uint32_t>(operand_index))->type_id();
  if (!_.IsIntScalarType(type_id) || _.GetBitWidth(type_id) != 32) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opcode_name << " " << operand_name << " type <id> "
           << _.getIdName(type_id) << " is not a 32 bit integer.";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateCooperativeVectorPointer(ValidationState_t& _,
                                              const Instruction* inst,
                                              const char* opname,
                                              uint32_t pointer_index) {
  const auto pointer_id = inst->GetOperandAs<uint32_t>(pointer_index);
  const auto pointer = _.FindDef(pointer_id);
  if (!pointer ||
      ((_.addressing_model() == spv::AddressingModel::Logical) &&
       ((!_.features().variable_pointers &&
         !spvOpcodeReturnsLogicalPointer(pointer->opcode())) ||
        (_.features().variable_pointers &&
         !spvOpcodeReturnsLogicalVariablePointer(pointer->opcode()))))) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opname << " Pointer <id> " << _.getIdName(pointer_id)
           << " is not a logical pointer.";
  }

  const auto pointer_type_id = pointer->type_id();
  const auto pointer_type = _.FindDef(pointer_type_id);
  if (!pointer_type || pointer_type->opcode() != spv::Op::OpTypePointer) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opname << " type for pointer <id> " << _.getIdName(pointer_id)
           << " is not a pointer type.";
  }

  const auto storage_class_index = 1u;
  const auto storage_class =
      pointer_type->GetOperandAs<spv::StorageClass>(storage_class_index);

  if (storage_class != spv::StorageClass::Workgroup &&
      storage_class != spv::StorageClass::StorageBuffer &&
      storage_class != spv::StorageClass::PhysicalStorageBuffer) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opname << " storage class for pointer type <id> "
           << _.getIdName(pointer_type_id)
           << " is not Workgroup or StorageBuffer.";
  }

  const auto pointee_id = pointer_type->GetOperandAs<uint32_t>(2);
  const auto pointee_type = _.FindDef(pointee_id);
  if (!pointee_type ||
      (pointee_type->opcode() != spv::Op::OpTypeArray &&
       pointee_type->opcode() != spv::Op::OpTypeRuntimeArray)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opname << " Pointer <id> " << _.getIdName(pointer->id())
           << "s Type must be an array type.";
  }

  const auto array_elem_type_id = pointee_type->GetOperandAs<uint32_t>(1);
  auto array_elem_type = _.FindDef(array_elem_type_id);
  if (!array_elem_type || !(_.IsIntScalarOrVectorType(array_elem_type_id) ||
                            _.IsFloatScalarOrVectorType(array_elem_type_id))) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opname << " Pointer <id> " << _.getIdName(pointer->id())
           << "s Type must be an array of scalar or vector type.";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateCooperativeVectorLoadStoreNV(ValidationState_t& _,
                                                  const Instruction* inst) {
  uint32_t type_id;
  const char* opname;
  if (inst->opcode() == spv::Op::OpCooperativeVectorLoadNV) {
    type_id = inst->type_id();
    opname = "spv::Op::OpCooperativeVectorLoadNV";
  } else {
    // get Object operand's type
    type_id = _.FindDef(inst->GetOperandAs<uint32_t>(2))->type_id();
    opname = "spv::Op::OpCooperativeVectorStoreNV";
  }

  auto vector_type = _.FindDef(type_id);

  if (vector_type->opcode() != spv::Op::OpTypeCooperativeVectorNV) {
    if (inst->opcode() == spv::Op::OpCooperativeVectorLoadNV) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "spv::Op::OpCooperativeVectorLoadNV Result Type <id> "
             << _.getIdName(type_id) << " is not a cooperative vector type.";
    } else {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "spv::Op::OpCooperativeVectorStoreNV Object type <id> "
             << _.getIdName(type_id) << " is not a cooperative vector type.";
    }
  }

  const auto pointer_index =
      (inst->opcode() == spv::Op::OpCooperativeVectorLoadNV) ? 2u : 0u;

  if (auto error =
          ValidateCooperativeVectorPointer(_, inst, opname, pointer_index)) {
    return error;
  }

  const auto memory_access_index =
      (inst->opcode() == spv::Op::OpCooperativeVectorLoadNV) ? 4u : 3u;
  if (inst->operands().size() > memory_access_index) {
    if (auto error = CheckMemoryAccess(_, inst, memory_access_index))
      return error;
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateCooperativeVectorOuterProductNV(ValidationState_t& _,
                                                     const Instruction* inst) {
  const auto pointer_index = 0u;
  const auto opcode_name =
      "spv::Op::OpCooperativeVectorOuterProductAccumulateNV";

  if (auto error = ValidateCooperativeVectorPointer(_, inst, opcode_name,
                                                    pointer_index)) {
    return error;
  }

  auto type_id = _.FindDef(inst->GetOperandAs<uint32_t>(2))->type_id();
  auto a_type = _.FindDef(type_id);

  if (a_type->opcode() != spv::Op::OpTypeCooperativeVectorNV) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opcode_name << " A type <id> " << _.getIdName(type_id)
           << " is not a cooperative vector type.";
  }

  type_id = _.FindDef(inst->GetOperandAs<uint32_t>(3))->type_id();
  auto b_type = _.FindDef(type_id);

  if (b_type->opcode() != spv::Op::OpTypeCooperativeVectorNV) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opcode_name << " B type <id> " << _.getIdName(type_id)
           << " is not a cooperative vector type.";
  }

  const auto a_component_type_id = a_type->GetOperandAs<uint32_t>(1);
  const auto b_component_type_id = b_type->GetOperandAs<uint32_t>(1);

  if (a_component_type_id != b_component_type_id) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opcode_name << " A and B component types "
           << _.getIdName(a_component_type_id) << " and "
           << _.getIdName(b_component_type_id) << " do not match.";
  }

  if (auto error = ValidateInt32Operand(_, inst, 1, opcode_name, "Offset")) {
    return error;
  }

  if (auto error =
          ValidateInt32Operand(_, inst, 4, opcode_name, "MemoryLayout")) {
    return error;
  }

  if (auto error = ValidateInt32Operand(_, inst, 5, opcode_name,
                                        "MatrixInterpretation")) {
    return error;
  }

  if (inst->operands().size() > 6) {
    if (auto error =
            ValidateInt32Operand(_, inst, 6, opcode_name, "MatrixStride")) {
      return error;
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateCooperativeVectorReduceSumNV(ValidationState_t& _,
                                                  const Instruction* inst) {
  const auto opcode_name = "spv::Op::OpCooperativeVectorReduceSumAccumulateNV";
  const auto pointer_index = 0u;

  if (auto error = ValidateCooperativeVectorPointer(_, inst, opcode_name,
                                                    pointer_index)) {
    return error;
  }

  auto type_id = _.FindDef(inst->GetOperandAs<uint32_t>(2))->type_id();
  auto v_type = _.FindDef(type_id);

  if (v_type->opcode() != spv::Op::OpTypeCooperativeVectorNV) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opcode_name << " V type <id> " << _.getIdName(type_id)
           << " is not a cooperative vector type.";
  }

  if (auto error = ValidateInt32Operand(_, inst, 1, opcode_name, "Offset")) {
    return error;
  }

  return SPV_SUCCESS;
}

bool InterpretationIsPacked(spv::ComponentType interp) {
  switch (interp) {
    case spv::ComponentType::SignedInt8PackedNV:
    case spv::ComponentType::UnsignedInt8PackedNV:
      return true;
    default:
      return false;
  }
}

using std::get;

spv_result_t ValidateCooperativeVectorMatrixMulNV(ValidationState_t& _,
                                                  const Instruction* inst) {
  const bool has_bias =
      inst->opcode() == spv::Op::OpCooperativeVectorMatrixMulAddNV;
  const auto opcode_name = has_bias
                               ? "spv::Op::OpCooperativeVectorMatrixMulAddNV"
                               : "spv::Op::OpCooperativeVectorMatrixMulNV";

  const auto bias_offset = has_bias ? 3 : 0;

  const auto result_type_index = 0u;
  const auto input_index = 2u;
  const auto input_interpretation_index = 3u;
  const auto matrix_index = 4u;
  const auto matrix_interpretation_index = 6u;
  const auto bias_index = 7u;
  const auto bias_interpretation_index = 9u;
  const auto m_index = 7u + bias_offset;
  const auto k_index = 8u + bias_offset;
  const auto memory_layout_index = 9u + bias_offset;
  const auto transpose_index = 10u + bias_offset;

  const auto result_type_id = inst->GetOperandAs<uint32_t>(result_type_index);
  const auto input_id = inst->GetOperandAs<uint32_t>(input_index);
  const auto input_interpretation_id =
      inst->GetOperandAs<uint32_t>(input_interpretation_index);
  const auto matrix_interpretation_id =
      inst->GetOperandAs<uint32_t>(matrix_interpretation_index);
  const auto bias_interpretation_id =
      inst->GetOperandAs<uint32_t>(bias_interpretation_index);
  const auto m_id = inst->GetOperandAs<uint32_t>(m_index);
  const auto k_id = inst->GetOperandAs<uint32_t>(k_index);
  const auto memory_layout_id =
      inst->GetOperandAs<uint32_t>(memory_layout_index);
  const auto transpose_id = inst->GetOperandAs<uint32_t>(transpose_index);

  if (auto error = ValidateCooperativeVectorPointer(_, inst, opcode_name,
                                                    matrix_index)) {
    return error;
  }

  if (inst->opcode() == spv::Op::OpCooperativeVectorMatrixMulAddNV) {
    if (auto error = ValidateCooperativeVectorPointer(_, inst, opcode_name,
                                                      bias_index)) {
      return error;
    }
  }

  const auto result_type = _.FindDef(result_type_id);

  if (result_type->opcode() != spv::Op::OpTypeCooperativeVectorNV) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opcode_name << " result type <id> " << _.getIdName(result_type_id)
           << " is not a cooperative vector type.";
  }

  const auto result_component_type_id = result_type->GetOperandAs<uint32_t>(1u);
  if (!(_.IsIntScalarType(result_component_type_id) &&
        _.GetBitWidth(result_component_type_id) == 32) &&
      !(_.IsFloatScalarType(result_component_type_id) &&
        (_.GetBitWidth(result_component_type_id) == 32 ||
         _.GetBitWidth(result_component_type_id) == 16))) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opcode_name << " result component type <id> "
           << _.getIdName(result_component_type_id)
           << " is not a 32 bit int or 16/32 bit float.";
  }

  const auto m_eval = _.EvalInt32IfConst(m_id);
  const auto rc_eval =
      _.EvalInt32IfConst(result_type->GetOperandAs<uint32_t>(2u));
  if (get<1>(m_eval) && get<1>(rc_eval) && get<2>(m_eval) != get<2>(rc_eval)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opcode_name << " result type number of components "
           << get<2>(rc_eval) << " does not match M " << get<2>(m_eval);
  }

  const auto k_eval = _.EvalInt32IfConst(k_id);

  const auto input = _.FindDef(input_id);
  const auto input_type = _.FindDef(input->type_id());
  const auto input_num_components_id = input_type->GetOperandAs<uint32_t>(2u);

  auto input_interp_eval = _.EvalInt32IfConst(input_interpretation_id);
  if (get<1>(input_interp_eval) &&
      !InterpretationIsPacked(spv::ComponentType{get<2>(input_interp_eval)})) {
    const auto inc_eval = _.EvalInt32IfConst(input_num_components_id);
    if (get<1>(inc_eval) && get<1>(k_eval) &&
        get<2>(inc_eval) != get<2>(k_eval)) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << opcode_name << " input number of components "
             << get<2>(inc_eval) << " does not match K " << get<2>(k_eval);
    }
  }

  if (!_.IsBoolScalarType(_.FindDef(transpose_id)->type_id())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << opcode_name << " Transpose <id> " << _.getIdName(transpose_id)
           << " is not a scalar boolean.";
  }

  const auto check_constant = [&](uint32_t id,
                                  const char* operand_name) -> spv_result_t {
    if (!spvOpcodeIsConstant(_.GetIdOpcode(id))) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << opcode_name << " " << operand_name << " <id> "
             << _.getIdName(id) << " is not a constant instruction.";
    }
    return SPV_SUCCESS;
  };

  if (auto error =
          check_constant(input_interpretation_id, "InputInterpretation")) {
    return error;
  }
  if (auto error =
          check_constant(matrix_interpretation_id, "MatrixInterpretation")) {
    return error;
  }
  if (has_bias) {
    if (auto error =
            check_constant(bias_interpretation_id, "BiasInterpretation")) {
      return error;
    }
  }
  if (auto error = check_constant(m_id, "M")) {
    return error;
  }
  if (auto error = check_constant(k_id, "K")) {
    return error;
  }
  if (auto error = check_constant(memory_layout_id, "MemoryLayout")) {
    return error;
  }
  if (auto error = check_constant(transpose_id, "Transpose")) {
    return error;
  }

  if (auto error = ValidateInt32Operand(_, inst, input_interpretation_index,
                                        opcode_name, "InputInterpretation")) {
    return error;
  }
  if (auto error = ValidateInt32Operand(_, inst, matrix_interpretation_index,
                                        opcode_name, "MatrixInterpretation")) {
    return error;
  }
  if (has_bias) {
    if (auto error = ValidateInt32Operand(_, inst, bias_interpretation_index,
                                          opcode_name, "BiasInterpretation")) {
      return error;
    }
  }
  if (auto error = ValidateInt32Operand(_, inst, m_index, opcode_name, "M")) {
    return error;
  }
  if (auto error = ValidateInt32Operand(_, inst, k_index, opcode_name, "K")) {
    return error;
  }
  if (auto error = ValidateInt32Operand(_, inst, memory_layout_index,
                                        opcode_name, "MemoryLayout")) {
    return error;
  }

  return SPV_SUCCESS;
}

spv_result_t ValidatePtrComparison(ValidationState_t& _,
                                   const Instruction* inst) {
  if (_.addressing_model() == spv::AddressingModel::Logical &&
      !_.features().variable_pointers) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Instruction cannot for logical addressing model be used without "
              "a variable pointers capability";
  }

  const auto result_type = _.FindDef(inst->type_id());
  if (inst->opcode() == spv::Op::OpPtrDiff) {
    if (!result_type || result_type->opcode() != spv::Op::OpTypeInt) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Result Type must be an integer scalar";
    }
  } else {
    if (!result_type || result_type->opcode() != spv::Op::OpTypeBool) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Result Type must be OpTypeBool";
    }
  }

  const auto op1 = _.FindDef(inst->GetOperandAs<uint32_t>(2u));
  const auto op2 = _.FindDef(inst->GetOperandAs<uint32_t>(3u));
  if (!op1 || !op2 || op1->type_id() != op2->type_id()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "The types of Operand 1 and Operand 2 must match";
  }
  const auto op1_type = _.FindDef(op1->type_id());
  if (!op1_type || (op1_type->opcode() != spv::Op::OpTypePointer &&
                    op1_type->opcode() != spv::Op::OpTypeUntypedPointerKHR)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Operand type must be a pointer";
  }

  spv::StorageClass sc = op1_type->GetOperandAs<spv::StorageClass>(1u);
  if (_.addressing_model() == spv::AddressingModel::Logical) {
    if (sc != spv::StorageClass::Workgroup &&
        sc != spv::StorageClass::StorageBuffer) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Invalid pointer storage class";
    }

    if (sc == spv::StorageClass::Workgroup &&
        !_.HasCapability(spv::Capability::VariablePointers)) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Workgroup storage class pointer requires VariablePointers "
                "capability to be specified";
    }
  } else if (sc == spv::StorageClass::PhysicalStorageBuffer) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Cannot use a pointer in the PhysicalStorageBuffer storage class";
  }

  return SPV_SUCCESS;
}

}  // namespace

spv_result_t MemoryPass(ValidationState_t& _, const Instruction* inst) {
  switch (inst->opcode()) {
    case spv::Op::OpVariable:
    case spv::Op::OpUntypedVariableKHR:
      if (auto error = ValidateVariable(_, inst)) return error;
      break;
    case spv::Op::OpLoad:
      if (auto error = ValidateLoad(_, inst)) return error;
      break;
    case spv::Op::OpStore:
      if (auto error = ValidateStore(_, inst)) return error;
      break;
    case spv::Op::OpCopyMemory:
    case spv::Op::OpCopyMemorySized:
      if (auto error = ValidateCopyMemory(_, inst)) return error;
      break;
    case spv::Op::OpPtrAccessChain:
    case spv::Op::OpUntypedPtrAccessChainKHR:
    case spv::Op::OpUntypedInBoundsPtrAccessChainKHR:
      if (auto error = ValidatePtrAccessChain(_, inst)) return error;
      break;
    case spv::Op::OpAccessChain:
    case spv::Op::OpInBoundsAccessChain:
    case spv::Op::OpInBoundsPtrAccessChain:
    case spv::Op::OpUntypedAccessChainKHR:
    case spv::Op::OpUntypedInBoundsAccessChainKHR:
      if (auto error = ValidateAccessChain(_, inst)) return error;
      break;
    case spv::Op::OpRawAccessChainNV:
      if (auto error = ValidateRawAccessChain(_, inst)) return error;
      break;
    case spv::Op::OpArrayLength:
    case spv::Op::OpUntypedArrayLengthKHR:
      if (auto error = ValidateArrayLength(_, inst)) return error;
      break;
    case spv::Op::OpCooperativeMatrixLoadNV:
    case spv::Op::OpCooperativeMatrixStoreNV:
      if (auto error = ValidateCooperativeMatrixLoadStoreNV(_, inst))
        return error;
      break;
    case spv::Op::OpCooperativeMatrixLengthKHR:
    case spv::Op::OpCooperativeMatrixLengthNV:
      if (auto error = ValidateCooperativeMatrixLengthNV(_, inst)) return error;
      break;
    case spv::Op::OpCooperativeMatrixLoadKHR:
    case spv::Op::OpCooperativeMatrixStoreKHR:
      if (auto error = ValidateCooperativeMatrixLoadStoreKHR(_, inst))
        return error;
      break;
    case spv::Op::OpCooperativeMatrixLoadTensorNV:
    case spv::Op::OpCooperativeMatrixStoreTensorNV:
      if (auto error = ValidateCooperativeMatrixLoadStoreTensorNV(_, inst))
        return error;
      break;
    case spv::Op::OpCooperativeVectorLoadNV:
    case spv::Op::OpCooperativeVectorStoreNV:
      if (auto error = ValidateCooperativeVectorLoadStoreNV(_, inst))
        return error;
      break;
    case spv::Op::OpCooperativeVectorOuterProductAccumulateNV:
      if (auto error = ValidateCooperativeVectorOuterProductNV(_, inst))
        return error;
      break;
    case spv::Op::OpCooperativeVectorReduceSumAccumulateNV:
      if (auto error = ValidateCooperativeVectorReduceSumNV(_, inst))
        return error;
      break;
    case spv::Op::OpCooperativeVectorMatrixMulNV:
    case spv::Op::OpCooperativeVectorMatrixMulAddNV:
      if (auto error = ValidateCooperativeVectorMatrixMulNV(_, inst))
        return error;
      break;
    case spv::Op::OpPtrEqual:
    case spv::Op::OpPtrNotEqual:
    case spv::Op::OpPtrDiff:
      if (auto error = ValidatePtrComparison(_, inst)) return error;
      break;
    case spv::Op::OpImageTexelPointer:
    case spv::Op::OpGenericPtrMemSemantics:
    default:
      break;
  }

  return SPV_SUCCESS;
}
}  // namespace val
}  // namespace spvtools
