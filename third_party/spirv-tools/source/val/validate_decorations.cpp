// Copyright (c) 2017 Google Inc.
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
#include <cassert>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "source/diagnostic.h"
#include "source/latest_version_opencl_std_header.h"
#include "source/opcode.h"
#include "source/spirv_constant.h"
#include "source/spirv_target_env.h"
#include "source/spirv_validator_options.h"
#include "source/util/string_utils.h"
#include "source/val/validate_scopes.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

// Returns true if the given structure type has a Block decoration.
bool isBlock(uint32_t struct_id, ValidationState_t& vstate) {
  const auto& decorations = vstate.id_decorations(struct_id);
  return std::any_of(decorations.begin(), decorations.end(),
                     [](const Decoration& d) {
                       return spv::Decoration::Block == d.dec_type();
                     });
}

// Returns true if the given ID has the Import LinkageAttributes decoration.
bool hasImportLinkageAttribute(uint32_t id, ValidationState_t& vstate) {
  const auto& decorations = vstate.id_decorations(id);
  return std::any_of(
      decorations.begin(), decorations.end(), [](const Decoration& d) {
        return spv::Decoration::LinkageAttributes == d.dec_type() &&
               d.params().size() >= 2u &&
               spv::LinkageType(d.params().back()) == spv::LinkageType::Import;
      });
}

// Returns a vector of all members of a structure.
std::vector<uint32_t> getStructMembers(uint32_t struct_id,
                                       ValidationState_t& vstate) {
  const auto inst = vstate.FindDef(struct_id);
  return std::vector<uint32_t>(inst->words().begin() + 2, inst->words().end());
}

// Returns a vector of all members of a structure that have specific type.
std::vector<uint32_t> getStructMembers(uint32_t struct_id, spv::Op type,
                                       ValidationState_t& vstate) {
  std::vector<uint32_t> members;
  for (auto id : getStructMembers(struct_id, vstate)) {
    if (type == vstate.FindDef(id)->opcode()) {
      members.push_back(id);
    }
  }
  return members;
}

// Returns true if variable or structure id has given decoration. Handles also
// nested structures.
bool hasDecoration(uint32_t id, spv::Decoration decoration,
                   ValidationState_t& vstate) {
  for (auto& dec : vstate.id_decorations(id)) {
    if (decoration == dec.dec_type()) return true;
  }
  if (spv::Op::OpTypeStruct != vstate.FindDef(id)->opcode()) {
    return false;
  }
  for (auto member_id : getStructMembers(id, spv::Op::OpTypeStruct, vstate)) {
    if (hasDecoration(member_id, decoration, vstate)) {
      return true;
    }
  }
  return false;
}

spv_result_t CheckLinkageAttrOfFunctions(ValidationState_t& vstate) {
  for (const auto& function : vstate.functions()) {
    if (function.block_count() == 0u) {
      // A function declaration (an OpFunction with no basic blocks), must have
      // a Linkage Attributes Decoration with the Import Linkage Type.
      if (!hasImportLinkageAttribute(function.id(), vstate)) {
        return vstate.diag(SPV_ERROR_INVALID_BINARY,
                           vstate.FindDef(function.id()))
               << "Function declaration (id " << function.id()
               << ") must have a LinkageAttributes decoration with the Import "
                  "Linkage type.";
      }
    } else {
      if (hasImportLinkageAttribute(function.id(), vstate)) {
        return vstate.diag(SPV_ERROR_INVALID_BINARY,
                           vstate.FindDef(function.id()))
               << "Function definition (id " << function.id()
               << ") may not be decorated with Import Linkage type.";
      }
    }
  }
  return SPV_SUCCESS;
}

// Checks whether an imported variable is initialized by this module.
spv_result_t CheckImportedVariableInitialization(ValidationState_t& vstate) {
  // According the SPIR-V Spec 2.16.1, it is illegal to initialize an imported
  // variable. This means that a module-scope OpVariable with initialization
  // value cannot be marked with the Import Linkage Type (import type id = 1).
  for (auto global_var_id : vstate.global_vars()) {
    // Initializer <id> is an optional argument for OpVariable. If initializer
    // <id> is present, the instruction will have 5 words.
    auto variable_instr = vstate.FindDef(global_var_id);
    if (variable_instr->words().size() == 5u &&
        hasImportLinkageAttribute(global_var_id, vstate)) {
      return vstate.diag(SPV_ERROR_INVALID_ID, variable_instr)
             << "A module-scope OpVariable with initialization value "
                "cannot be marked with the Import Linkage Type.";
    }
  }
  return SPV_SUCCESS;
}

// Checks whether a builtin variable is valid.
spv_result_t CheckBuiltInVariable(uint32_t var_id, ValidationState_t& vstate) {
  const auto& decorations = vstate.id_decorations(var_id);
  for (const auto& d : decorations) {
    if (spvIsVulkanEnv(vstate.context()->target_env)) {
      if (d.dec_type() == spv::Decoration::Location ||
          d.dec_type() == spv::Decoration::Component) {
        return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(var_id))
               << vstate.VkErrorID(4915) << "A BuiltIn variable (id " << var_id
               << ") cannot have any Location or Component decorations";
      }
    }
  }
  return SPV_SUCCESS;
}

// Checks whether proper decorations have been applied to the entry points.
spv_result_t CheckDecorationsOfEntryPoints(ValidationState_t& vstate) {
  for (uint32_t entry_point : vstate.entry_points()) {
    const auto& descs = vstate.entry_point_descriptions(entry_point);
    int num_builtin_block_inputs = 0;
    int num_builtin_block_outputs = 0;
    int num_workgroup_variables = 0;
    int num_workgroup_variables_with_block = 0;
    int num_workgroup_variables_with_aliased = 0;
    bool has_task_payload = false;
    for (const auto& desc : descs) {
      std::unordered_set<Instruction*> seen_vars;
      std::unordered_set<spv::BuiltIn> input_var_builtin;
      std::unordered_set<spv::BuiltIn> output_var_builtin;
      for (auto interface : desc.interfaces) {
        Instruction* var_instr = vstate.FindDef(interface);
        if (!var_instr ||
            (spv::Op::OpVariable != var_instr->opcode() &&
             spv::Op::OpUntypedVariableKHR != var_instr->opcode())) {
          return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                 << "Interfaces passed to OpEntryPoint must be variables. "
                    "Found Op"
                 << spvOpcodeString(var_instr->opcode()) << ".";
        }
        const bool untyped_pointers =
            var_instr->opcode() == spv::Op::OpUntypedVariableKHR;
        const auto sc_index = 2u;
        const spv::StorageClass storage_class =
            var_instr->GetOperandAs<spv::StorageClass>(sc_index);
        if (vstate.version() >= SPV_SPIRV_VERSION_WORD(1, 4)) {
          // SPV_EXT_mesh_shader, at most one task payload is permitted
          // per entry point
          if (storage_class == spv::StorageClass::TaskPayloadWorkgroupEXT) {
            if (has_task_payload) {
              return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                     << "There can be at most one "
                        "OpVariable with storage "
                        "class TaskPayloadWorkgroupEXT associated with "
                        "an OpEntryPoint";
            }
            has_task_payload = true;
          }

          // Starting in 1.4, OpEntryPoint must list all global variables
          // it statically uses and those interfaces must be unique.
          if (storage_class == spv::StorageClass::Function) {
            return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                   << "In SPIR-V 1.4 or later, OpEntryPoint interfaces should "
                      "only list global "
                      "variables";
          }

          if (!seen_vars.insert(var_instr).second) {
            return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                   << "In SPIR-V 1.4 or later, non-unique OpEntryPoint "
                      "interface "
                   << vstate.getIdName(interface) << " is disallowed";
          }
        } else {
          if (storage_class != spv::StorageClass::Input &&
              storage_class != spv::StorageClass::Output) {
            return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                   << "In SPIR-V 1.3 or earlier, OpEntryPoint interfaces must "
                      "be OpVariables with "
                      "Storage Class of Input(1) or Output(3). Found Storage "
                      "Class "
                   << uint32_t(storage_class) << " for Entry Point id "
                   << entry_point << ".";
          }
        }

        // Descriptor heap's base variables have no data type in declaration.
        if (untyped_pointers && var_instr->words().size() < 5 &&
            vstate.IsDescriptorHeapBaseVariable(var_instr))
          continue;

        // It is guaranteed (by validator ID checks) that ptr_instr is
        // OpTypePointer. Word 3 of this instruction is the type being pointed
        // to. For untyped variables, the pointee type comes from the data type
        // operand.
        const uint32_t type_id =
            untyped_pointers ? var_instr->word(4)
                             : vstate.FindDef(var_instr->word(1))->word(3);
        Instruction* type_instr = vstate.FindDef(type_id);
        const bool is_struct =
            type_instr && spv::Op::OpTypeStruct == type_instr->opcode();

        // Search all Built-in (on the variable or the struct)
        bool has_built_in = false;
        for (auto& dec :
             vstate.id_decorations(is_struct ? type_id : interface)) {
          if (dec.dec_type() != spv::Decoration::BuiltIn) continue;
          has_built_in = true;

          if (!spvIsVulkanEnv(vstate.context()->target_env)) continue;

          const spv::BuiltIn builtin = dec.builtin();
          if (storage_class == spv::StorageClass::Input) {
            if (!input_var_builtin.insert(builtin).second) {
              return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                     << vstate.VkErrorID(9658)
                     << "OpEntryPoint contains duplicate input variables "
                        "with "
                     << vstate.grammar().lookupOperandName(
                            SPV_OPERAND_TYPE_BUILT_IN, (uint32_t)builtin)
                     << " builtin";
            }
          }
          if (storage_class == spv::StorageClass::Output) {
            if (!output_var_builtin.insert(builtin).second) {
              return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                     << vstate.VkErrorID(9659)
                     << "OpEntryPoint contains duplicate output variables "
                        "with "
                     << vstate.grammar().lookupOperandName(
                            SPV_OPERAND_TYPE_BUILT_IN, (uint32_t)builtin)
                     << " builtin";
            }
          }
        }

        if (has_built_in) {
          if (auto error = CheckBuiltInVariable(interface, vstate))
            return error;

          if (is_struct) {
            if (!isBlock(type_id, vstate)) {
              return vstate.diag(SPV_ERROR_INVALID_DATA,
                                 vstate.FindDef(type_id))
                     << vstate.VkErrorID(4919)
                     << "Interface struct has no Block decoration but has "
                        "BuiltIn members. "
                        "Location decorations must be used on each member of "
                        "OpVariable with a structure type that is a block not "
                        "decorated with Location.";
            }
            if (storage_class == spv::StorageClass::Input)
              ++num_builtin_block_inputs;
            if (storage_class == spv::StorageClass::Output)
              ++num_builtin_block_outputs;
            if (num_builtin_block_inputs > 1 || num_builtin_block_outputs > 1)
              break;
          }
        }

        if (storage_class == spv::StorageClass::Workgroup) {
          ++num_workgroup_variables;
          if (type_instr) {
            if (spv::Op::OpTypeStruct == type_instr->opcode()) {
              if (hasDecoration(type_id, spv::Decoration::Block, vstate)) {
                ++num_workgroup_variables_with_block;
              } else if (untyped_pointers &&
                         vstate.HasCapability(spv::Capability::Shader)) {
                return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                       << "Untyped workgroup variables in shaders must be "
                          "block decorated";
              }
              if (hasDecoration(var_instr->id(), spv::Decoration::Aliased,
                                vstate))
                ++num_workgroup_variables_with_aliased;
            } else if (untyped_pointers &&
                       vstate.HasCapability(spv::Capability::Shader)) {
              return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                     << "Untyped workgroup variables in shaders must be block "
                        "decorated structs";
            }
          }
        }

        if (spvIsVulkanEnv(vstate.context()->target_env)) {
          const auto* models = vstate.GetExecutionModels(entry_point);
          const bool has_frag =
              models->find(spv::ExecutionModel::Fragment) != models->end();
          const bool has_vert =
              models->find(spv::ExecutionModel::Vertex) != models->end();
          for (const auto& decoration :
               vstate.id_decorations(var_instr->id())) {
            if (decoration == spv::Decoration::Flat ||
                decoration == spv::Decoration::NoPerspective ||
                decoration == spv::Decoration::Sample ||
                decoration == spv::Decoration::Centroid) {
              // VUID 04670 already validates these decorations are input/output
              if (storage_class == spv::StorageClass::Input &&
                  (models->size() > 1 || has_vert)) {
                return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                       << vstate.VkErrorID(6202)
                       << vstate.SpvDecorationString(decoration.dec_type())
                       << " decorated variable must not be used in vertex "
                          "execution model as an Input storage class for Entry "
                          "Point id "
                       << entry_point << ".";
              } else if (storage_class == spv::StorageClass::Output &&
                         (models->size() > 1 || has_frag)) {
                return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                       << vstate.VkErrorID(6201)
                       << vstate.SpvDecorationString(decoration.dec_type())
                       << " decorated variable must not be used in fragment "
                          "execution model as an Output storage class for "
                          "Entry Point id "
                       << entry_point << ".";
              }
            }
          }

          const bool has_flat =
              hasDecoration(var_instr->id(), spv::Decoration::Flat, vstate);
          if (has_frag && storage_class == spv::StorageClass::Input &&
              !has_flat &&
              (vstate.IsFloatScalarType(type_id, 64) ||
               vstate.IsIntScalarOrVectorType(type_id))) {
            return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                     << vstate.VkErrorID(4744)
                     << "Fragment OpEntryPoint operand "
                     << interface << " with Input interfaces with integer or "
                                     "float type must have a Flat decoration "
                                     "for Entry Point id "
                     << entry_point << ".";
          }
        }
      }
      if (num_builtin_block_inputs > 1 || num_builtin_block_outputs > 1) {
        return vstate.diag(SPV_ERROR_INVALID_BINARY,
                           vstate.FindDef(entry_point))
               << "There must be at most one object per Storage Class that can "
                  "contain a structure type containing members decorated with "
                  "BuiltIn, consumed per entry-point. Entry Point id "
               << entry_point << " does not meet this requirement.";
      }
      // The LinkageAttributes Decoration cannot be applied to functions
      // targeted by an OpEntryPoint instruction
      for (auto& decoration : vstate.id_decorations(entry_point)) {
        if (spv::Decoration::LinkageAttributes == decoration.dec_type()) {
          const std::string linkage_name =
              spvtools::utils::MakeString(decoration.params());
          return vstate.diag(SPV_ERROR_INVALID_BINARY,
                             vstate.FindDef(entry_point))
                 << "The LinkageAttributes Decoration (Linkage name: "
                 << linkage_name << ") cannot be applied to function id "
                 << entry_point
                 << " because it is targeted by an OpEntryPoint instruction.";
        }
      }

      const bool workgroup_blocks_allowed = vstate.HasCapability(
          spv::Capability::WorkgroupMemoryExplicitLayoutKHR);
      if (workgroup_blocks_allowed &&
          !vstate.HasCapability(spv::Capability::UntypedPointersKHR) &&
          num_workgroup_variables > 0 &&
          num_workgroup_variables_with_block > 0) {
        if (num_workgroup_variables != num_workgroup_variables_with_block) {
          return vstate.diag(SPV_ERROR_INVALID_BINARY,
                             vstate.FindDef(entry_point))
                 << "When declaring WorkgroupMemoryExplicitLayoutKHR, "
                    "either all or none of the Workgroup Storage Class "
                    "variables "
                    "in the entry point interface must point to struct types "
                    "decorated with Block (unless the "
                    "UntypedPointersKHR capability is declared).  "
                    "Entry point id "
                 << entry_point << " does not meet this requirement.";
        }
        if (num_workgroup_variables_with_block > 1 &&
            num_workgroup_variables_with_block !=
                num_workgroup_variables_with_aliased) {
          return vstate.diag(SPV_ERROR_INVALID_BINARY,
                             vstate.FindDef(entry_point))
                 << "When declaring WorkgroupMemoryExplicitLayoutKHR, "
                    "if more than one Workgroup Storage Class variable in "
                    "the entry point interface point to a type decorated "
                    "with Block, all of them must be decorated with Aliased "
                    "(unless the UntypedPointerWorkgroupKHR capability is "
                    "declared). Entry point id "
                 << entry_point << " does not meet this requirement.";
        }
      } else if (!workgroup_blocks_allowed &&
                 num_workgroup_variables_with_block > 0) {
        return vstate.diag(SPV_ERROR_INVALID_BINARY,
                           vstate.FindDef(entry_point))
               << "Workgroup Storage Class variables can't be decorated with "
                  "Block unless declaring the WorkgroupMemoryExplicitLayoutKHR "
                  "capability.";
      }
    }
  }
  return SPV_SUCCESS;
}

spv_result_t CheckDecorationsOfVariables(ValidationState_t& vstate) {
  if (!spvIsVulkanEnv(vstate.context()->target_env)) {
    return SPV_SUCCESS;
  }
  for (const auto& inst : vstate.ordered_instructions()) {
    if ((spv::Op::OpVariable == inst.opcode()) ||
        (spv::Op::OpUntypedVariableKHR == inst.opcode())) {
      const auto var_id = inst.id();
      const auto storageClass = inst.GetOperandAs<spv::StorageClass>(2);
      const bool uniform = storageClass == spv::StorageClass::Uniform;
      const bool uniform_constant =
          storageClass == spv::StorageClass::UniformConstant;
      const bool storage_buffer =
          storageClass == spv::StorageClass::StorageBuffer;

      const char* sc_str = uniform            ? "Uniform"
                           : uniform_constant ? "UniformConstant"
                                              : "StorageBuffer";
      // Check variables in the UniformConstant, StorageBuffer, and Uniform
      // storage classes are decorated with DescriptorSet and Binding
      // (VUID-06677).
      if (uniform_constant || storage_buffer || uniform) {
        if (vstate.IsDescriptorHeapBaseVariable(&inst)) {
          continue;
        }
        // Skip validation if the variable is not used and we're looking
        // at a module coming from HLSL that has not been legalized yet.
        if (vstate.options()->before_hlsl_legalization &&
            vstate.EntryPointReferences(var_id).empty()) {
          continue;
        }
        if (!hasDecoration(var_id, spv::Decoration::DescriptorSet, vstate)) {
          return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(var_id))
                 << vstate.VkErrorID(6677) << sc_str << " id '" << var_id
                 << "' is missing DescriptorSet decoration.\n"
                 << "From Vulkan spec:\n"
                 << "These variables must have DescriptorSet and Binding "
                    "decorations specified";
        }
        if (!hasDecoration(var_id, spv::Decoration::Binding, vstate)) {
          return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(var_id))
                 << vstate.VkErrorID(6677) << sc_str << " id '" << var_id
                 << "' is missing Binding decoration.\n"
                 << "From Vulkan spec:\n"
                 << "These variables must have DescriptorSet and Binding "
                    "decorations specified";
        }
      }
      if (storageClass == spv::StorageClass::TileImageEXT) {
        if (!hasDecoration(var_id, spv::Decoration::Location, vstate)) {
          return vstate.diag(SPV_ERROR_INVALID_DATA, vstate.FindDef(var_id))
                 << vstate.VkErrorID(8723)
                 << "Variable with TileImageEXT Storage Class must be "
                    "decorated with Location.";
        }
      }
    }
  }
  return SPV_SUCCESS;
}

spv_result_t CheckDecorationsOfBuffers(ValidationState_t& vstate) {
  // Set of entry points that are known to use a push constant.
  std::unordered_set<uint32_t> uses_push_constant;
  for (const auto& inst : vstate.ordered_instructions()) {
    const auto& words = inst.words();
    if (spv::Op::OpVariable == inst.opcode() ||
        spv::Op::OpUntypedVariableKHR == inst.opcode()) {
      const bool untyped_pointer =
          inst.opcode() == spv::Op::OpUntypedVariableKHR;
      const auto var_id = inst.id();
      // For storage class / decoration combinations, see Vulkan 14.5.4 "Offset
      // and Stride Assignment".
      const auto storageClassVal = words[3];
      const auto storageClass = spv::StorageClass(storageClassVal);
      const bool uniform = storageClass == spv::StorageClass::Uniform;
      const bool push_constant =
          storageClass == spv::StorageClass::PushConstant;
      const bool storage_buffer =
          storageClass == spv::StorageClass::StorageBuffer;

      if (spvIsVulkanEnv(vstate.context()->target_env)) {
        // Vulkan: There must be no more than one PushConstant block per entry
        // point.
        if (push_constant &&
            !(vstate.HasCapability(spv::Capability::PushConstantBanksNV))) {
          auto entry_points = vstate.EntryPointReferences(var_id);
          for (auto ep_id : entry_points) {
            const bool already_used = !uses_push_constant.insert(ep_id).second;
            if (already_used) {
              return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(var_id))
                     << vstate.VkErrorID(6674) << "Entry point id '" << ep_id
                     << "' uses more than one PushConstant interface.\n"
                     << "From Vulkan spec:\n"
                     << "There must be no more than one push constant block "
                     << "statically used per shader entry point.";
            }
          }
        }
      }

      if (spvIsOpenGLEnv(vstate.context()->target_env)) {
        bool has_block = hasDecoration(var_id, spv::Decoration::Block, vstate);
        bool has_buffer_block =
            hasDecoration(var_id, spv::Decoration::BufferBlock, vstate);
        if ((uniform && (has_block || has_buffer_block)) ||
            (storage_buffer && has_block)) {
          auto entry_points = vstate.EntryPointReferences(var_id);
          if (!entry_points.empty() &&
              !hasDecoration(var_id, spv::Decoration::Binding, vstate)) {
            return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(var_id))
                   << StorageClassToString(storageClass) << " id '" << var_id
                   << "' is missing Binding decoration.\n"
                   << "From ARB_gl_spirv extension:\n"
                   << "Uniform and shader storage block variables must "
                   << "also be decorated with a *Binding*.";
          }
        }
      }

      const bool phys_storage_buffer =
          storageClass == spv::StorageClass::PhysicalStorageBuffer;
      const bool workgroup =
          storageClass == spv::StorageClass::Workgroup &&
          vstate.HasCapability(
              spv::Capability::WorkgroupMemoryExplicitLayoutKHR);

      if (spvIsVulkanEnv(vstate.context()->target_env) &&
          inst.opcode() == spv::Op::OpUntypedVariableKHR &&
          storageClass != spv::StorageClass::UniformConstant &&
          vstate.IsDescriptorHeapBaseVariable(&inst)) {
        if (vstate.IsBuiltin(inst.id(), spv::BuiltIn::ResourceHeapEXT)) {
          return vstate.diag(SPV_ERROR_INVALID_DATA, &inst)
                 << vstate.VkErrorID(11241)
                 << "The variable decorated with ResourceHeapEXT must be "
                 << "declared using the UniformConstant storage class.";
        }
        if (vstate.IsBuiltin(inst.id(), spv::BuiltIn::SamplerHeapEXT)) {
          return vstate.diag(SPV_ERROR_INVALID_DATA, &inst)
                 << vstate.VkErrorID(11239)
                 << "The variable decorated with SamplerHeapEXT must be "
                 << "declared using the UniformConstant storage class.";
        }
      }
      if (uniform || push_constant || storage_buffer || phys_storage_buffer ||
          workgroup) {
        const auto ptrInst = vstate.FindDef(words[1]);
        assert(spv::Op::OpTypePointer == ptrInst->opcode() ||
               spv::Op::OpTypeUntypedPointerKHR == ptrInst->opcode());
        auto id = untyped_pointer ? (words.size() > 4 ? words[4] : 0)
                                  : ptrInst->words()[3];
        if (id != 0) {
          auto id_inst = vstate.FindDef(id);
          // Jump through one level of arraying.
          if (!workgroup &&
              (id_inst->opcode() == spv::Op::OpTypeArray ||
               id_inst->opcode() == spv::Op::OpTypeRuntimeArray)) {
            id = id_inst->GetOperandAs<uint32_t>(1u);
            id_inst = vstate.FindDef(id);
          }
        }

        if (spvIsVulkanEnv(vstate.context()->target_env)) {
          const bool block = hasDecoration(id, spv::Decoration::Block, vstate);
          const bool buffer_block =
              hasDecoration(id, spv::Decoration::BufferBlock, vstate);
          if (storage_buffer && buffer_block) {
            return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(var_id))
                   << vstate.VkErrorID(6675) << "Storage buffer id '" << var_id
                   << " In Vulkan, BufferBlock is disallowed on variables in "
                      "the StorageBuffer storage class";
          }
          // Vulkan: Check Block decoration for PushConstant, Uniform
          // and StorageBuffer variables. Uniform can also use BufferBlock.
          if (push_constant && !block) {
            return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
                   << vstate.VkErrorID(6675) << "PushConstant id '" << id
                   << "' is missing Block decoration.\n"
                   << "From Vulkan spec:\n"
                   << "Such variables must be identified with a Block "
                      "decoration";
          }
          if (storage_buffer && !block) {
            return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
                   << vstate.VkErrorID(6675) << "StorageBuffer id '" << id
                   << "' is missing Block decoration.\n"
                   << "From Vulkan spec:\n"
                   << "Such variables must be identified with a Block "
                      "decoration";
          }
          if (uniform && !block && !buffer_block) {
            return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
                   << vstate.VkErrorID(6676) << "Uniform id '" << id
                   << "' is missing Block or BufferBlock decoration.\n"
                   << "From Vulkan spec:\n"
                   << "Such variables must be identified with a Block or "
                      "BufferBlock decoration";
          }
        }

        if (id != 0) {
          for (const auto& dec : vstate.id_decorations(id)) {
            const bool blockDeco = spv::Decoration::Block == dec.dec_type();
            const bool bufferDeco =
                spv::Decoration::BufferBlock == dec.dec_type();
            if (uniform && blockDeco) {
              vstate.RegisterPointerToUniformBlock(ptrInst->id());
              vstate.RegisterStructForUniformBlock(id);
            }
            if ((uniform && bufferDeco) ||
                ((storage_buffer || phys_storage_buffer) && blockDeco)) {
              vstate.RegisterPointerToStorageBuffer(ptrInst->id());
              vstate.RegisterStructForStorageBuffer(id);
            }
          }
        }
      }
    }
  }
  return SPV_SUCCESS;
}

// Returns true if |decoration| cannot be applied to the same id more than once.
bool AtMostOncePerId(spv::Decoration decoration) {
  return decoration != spv::Decoration::UserSemantic &&
         decoration != spv::Decoration::FuncParamAttr;
}

// Returns true if |decoration| cannot be applied to the same member more than
// once.
bool AtMostOncePerMember(spv::Decoration decoration) {
  return decoration != spv::Decoration::UserSemantic;
}

spv_result_t CheckDecorationsCompatibility(ValidationState_t& vstate) {
  using PerIDKey = std::tuple<spv::Decoration, uint32_t>;
  using PerMemberKey = std::tuple<spv::Decoration, uint32_t, uint32_t>;

  // An Array of pairs where the decorations in the pair cannot both be applied
  // to the same id.
  static const spv::Decoration mutually_exclusive_per_id[][2] = {
      {spv::Decoration::Block, spv::Decoration::BufferBlock},
      {spv::Decoration::Restrict, spv::Decoration::Aliased},
      {spv::Decoration::RestrictPointer, spv::Decoration::AliasedPointer}};
  static const auto num_mutually_exclusive_per_id_pairs =
      sizeof(mutually_exclusive_per_id) / (2 * sizeof(spv::Decoration));

  // An Array of pairs where the decorations in the pair cannot both be applied
  // to the same member.
  static const spv::Decoration mutually_exclusive_per_member[][2] = {
      {spv::Decoration::RowMajor, spv::Decoration::ColMajor},
      {spv::Decoration::Offset, spv::Decoration::OffsetIdEXT}};
  static const auto num_mutually_exclusive_per_mem_pairs =
      sizeof(mutually_exclusive_per_member) / (2 * sizeof(spv::Decoration));

  std::set<PerIDKey> seen_per_id;
  std::set<PerMemberKey> seen_per_member;

  for (const auto& inst : vstate.ordered_instructions()) {
    const auto& words = inst.words();
    if (spv::Op::OpDecorate == inst.opcode()) {
      const auto id = words[1];
      const auto dec_type = static_cast<spv::Decoration>(words[2]);
      const auto k = PerIDKey(dec_type, id);
      const auto already_used = !seen_per_id.insert(k).second;
      if (already_used && AtMostOncePerId(dec_type)) {
        return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
               << "ID '" << id << "' decorated with "
               << vstate.SpvDecorationString(dec_type)
               << " multiple times is not allowed.";
      }
      // Verify certain mutually exclusive decorations are not both applied on
      // an ID.
      for (uint32_t pair_idx = 0;
           pair_idx < num_mutually_exclusive_per_id_pairs; ++pair_idx) {
        spv::Decoration excl_dec_type = spv::Decoration::Max;
        if (mutually_exclusive_per_id[pair_idx][0] == dec_type) {
          excl_dec_type = mutually_exclusive_per_id[pair_idx][1];
        } else if (mutually_exclusive_per_id[pair_idx][1] == dec_type) {
          excl_dec_type = mutually_exclusive_per_id[pair_idx][0];
        } else {
          continue;
        }

        const auto excl_k = PerIDKey(excl_dec_type, id);
        if (seen_per_id.find(excl_k) != seen_per_id.end()) {
          return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
                 << "ID '" << id << "' decorated with both "
                 << vstate.SpvDecorationString(dec_type) << " and "
                 << vstate.SpvDecorationString(excl_dec_type)
                 << " is not allowed.";
        }
      }
    } else if (spv::Op::OpMemberDecorate == inst.opcode() ||
               spv::Op::OpMemberDecorateIdEXT == inst.opcode()) {
      const auto id = words[1];
      const auto member_id = words[2];
      const auto dec_type = static_cast<spv::Decoration>(words[3]);
      const auto k = PerMemberKey(dec_type, id, member_id);
      const auto already_used = !seen_per_member.insert(k).second;
      if (already_used && AtMostOncePerMember(dec_type)) {
        return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
               << "ID '" << id << "', member '" << member_id
               << "' decorated with " << vstate.SpvDecorationString(dec_type)
               << " multiple times is not allowed.";
      }
      // Verify certain mutually exclusive decorations are not both applied on
      // a (ID, member) tuple.
      for (uint32_t pair_idx = 0;
           pair_idx < num_mutually_exclusive_per_mem_pairs; ++pair_idx) {
        spv::Decoration excl_dec_type = spv::Decoration::Max;
        if (mutually_exclusive_per_member[pair_idx][0] == dec_type) {
          excl_dec_type = mutually_exclusive_per_member[pair_idx][1];
        } else if (mutually_exclusive_per_member[pair_idx][1] == dec_type) {
          excl_dec_type = mutually_exclusive_per_member[pair_idx][0];
        } else {
          continue;
        }

        const auto excl_k = PerMemberKey(excl_dec_type, id, member_id);
        if (seen_per_member.find(excl_k) != seen_per_member.end()) {
          return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
                 << "ID '" << id << "', member '" << member_id
                 << "' decorated with both "
                 << vstate.SpvDecorationString(dec_type) << " and "
                 << vstate.SpvDecorationString(excl_dec_type)
                 << " is not allowed.";
        }
      }
    }
  }
  return SPV_SUCCESS;
}

spv_result_t CheckVulkanMemoryModelDeprecatedDecorations(
    ValidationState_t& vstate) {
  if (vstate.memory_model() != spv::MemoryModel::VulkanKHR) return SPV_SUCCESS;

  std::string msg;
  std::ostringstream str(msg);
  for (const auto& def : vstate.all_definitions()) {
    const auto inst = def.second;
    const auto id = inst->id();
    for (const auto& dec : vstate.id_decorations(id)) {
      const auto member = dec.struct_member_index();
      if (dec.dec_type() == spv::Decoration::Coherent ||
          dec.dec_type() == spv::Decoration::Volatile) {
        str << (dec.dec_type() == spv::Decoration::Coherent ? "Coherent"
                                                            : "Volatile");
        str << " decoration targeting " << vstate.getIdName(id);
        if (member != Decoration::kInvalidMember) {
          str << " (member index " << member << ")";
        }
        str << " is banned when using the Vulkan memory model.";
        return vstate.diag(SPV_ERROR_INVALID_ID, inst) << str.str();
      }
    }
  }
  return SPV_SUCCESS;
}

// Returns SPV_SUCCESS if validation rules are satisfied for FPRoundingMode
// decorations.  Otherwise emits a diagnostic and returns something other than
// SPV_SUCCESS.
spv_result_t CheckFPRoundingModeForShaders(ValidationState_t& vstate,
                                           const Instruction& inst,
                                           const Decoration& decoration) {
  // Validates width-only conversion instruction for floating-point object
  // i.e., OpFConvert
  if (inst.opcode() != spv::Op::OpFConvert) {
    return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
           << "FPRoundingMode decoration can be applied only to a "
              "width-only conversion instruction for floating-point "
              "object.";
  }

  if (spvIsVulkanEnv(vstate.context()->target_env)) {
    const auto mode = spv::FPRoundingMode(decoration.params()[0]);
    if ((mode != spv::FPRoundingMode::RTE) &&
        (mode != spv::FPRoundingMode::RTZ)) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << vstate.VkErrorID(4675)
             << "In Vulkan, the FPRoundingMode mode must only by RTE or RTZ.";
    }
  }

  // Validates Object operand of an OpStore
  for (const auto& use : inst.uses()) {
    const auto store = use.first;
    if (store->opcode() == spv::Op::OpFConvert) continue;
    if (spvOpcodeIsDebug(store->opcode())) continue;
    if (store->IsNonSemantic()) continue;
    if (spvOpcodeIsDecoration(store->opcode())) continue;
    if (store->opcode() != spv::Op::OpStore) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "FPRoundingMode decoration can be applied only to the "
                "Object operand of an OpStore.";
    }

    if (use.second != 2) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "FPRoundingMode decoration can be applied only to the "
                "Object operand of an OpStore.";
    }

    const auto ptr_inst = vstate.FindDef(store->GetOperandAs<uint32_t>(0));
    const auto ptr_type = vstate.FindDef(ptr_inst->GetOperandAs<uint32_t>(0));

    const auto half_float_id = ptr_type->GetOperandAs<uint32_t>(2);
    if (!vstate.IsFloatScalarOrVectorType(half_float_id) ||
        vstate.GetBitWidth(half_float_id) != 16) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "FPRoundingMode decoration can be applied only to the "
                "Object operand of an OpStore storing through a pointer "
                "to "
                "a 16-bit floating-point scalar or vector object.";
    }

    // Validates storage class of the pointer to the OpStore
    const auto storage = ptr_type->GetOperandAs<spv::StorageClass>(1);
    if (storage != spv::StorageClass::StorageBuffer &&
        storage != spv::StorageClass::Uniform &&
        storage != spv::StorageClass::PushConstant &&
        storage != spv::StorageClass::Input &&
        storage != spv::StorageClass::Output &&
        storage != spv::StorageClass::PhysicalStorageBuffer) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "FPRoundingMode decoration can be applied only to the "
                "Object operand of an OpStore in the StorageBuffer, "
                "PhysicalStorageBuffer, Uniform, PushConstant, Input, or "
                "Output Storage Classes.";
    }
  }
  return SPV_SUCCESS;
}

spv_result_t CheckFPRoundingModeForKernels(ValidationState_t& vstate,
                                           const Instruction& inst) {
  const auto opcode = inst.opcode();
  const bool isSqrtExtendedInstruction =
      spvIsExtendedInstruction(inst.opcode()) &&
      inst.ext_inst_type() == SPV_EXT_INST_TYPE_OPENCL_STD &&
      inst.word(4) == OpenCLLIB::Sqrt;
  if (opcode == spv::Op::OpFDiv || isSqrtExtendedInstruction) {
    if (!vstate.HasCapability(spv::Capability::RoundedDivideSqrtINTEL)) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "FPRoundingMode decoration can be applied to OpFDiv and "
                "sqrt extended instructions only if the RoundedDivideSqrtINTEL "
                "capability is enabled.";
    }
  } else if (opcode != spv::Op::OpConvertFToU &&
             opcode != spv::Op::OpConvertFToS &&
             opcode != spv::Op::OpConvertSToF &&
             opcode != spv::Op::OpConvertUToF &&
             opcode != spv::Op::OpFConvert) {
    return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
           << "FPRoundingMode decoration can be applied only to a conversion "
              "instruction to or from a floating-point type.";
  }
  return SPV_SUCCESS;
}

// Returns SPV_SUCCESS if validation rules are satisfied for the NonReadable or
// NonWritable
// decoration.  Otherwise emits a diagnostic and returns something other than
// SPV_SUCCESS.  The |inst| parameter is the object being decorated.  This must
// be called after TypePass and AnnotateCheckDecorationsOfBuffers are called.
spv_result_t CheckNonReadableWritableDecorations(ValidationState_t& vstate,
                                                 const Instruction& inst,
                                                 const Decoration& decoration) {
  assert(inst.id() && "Parser ensures the target of the decoration has an ID");
  const bool is_non_writable =
      decoration.dec_type() == spv::Decoration::NonWritable;
  assert(is_non_writable ||
         decoration.dec_type() == spv::Decoration::NonReadable);

  if (decoration.struct_member_index() == Decoration::kInvalidMember) {
    // The target must be a memory object declaration.
    // First, it must be a variable or function parameter.
    const auto opcode = inst.opcode();
    const auto type_id = inst.type_id();
    if (opcode != spv::Op::OpVariable &&
        opcode != spv::Op::OpUntypedVariableKHR &&
        opcode != spv::Op::OpBufferPointerEXT &&
        opcode != spv::Op::OpFunctionParameter &&
        opcode != spv::Op::OpRawAccessChainNV) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "Target of "
             << (is_non_writable ? "NonWritable" : "NonReadable")
             << " decoration must be a "
                "memory object "
                "declaration (a variable or a function parameter)";
    }
    const auto var_storage_class = opcode == spv::Op::OpVariable
                                       ? inst.GetOperandAs<spv::StorageClass>(2)
                                   : opcode == spv::Op::OpUntypedVariableKHR
                                       ? inst.GetOperandAs<spv::StorageClass>(3)
                                       : spv::StorageClass::Max;

    if (opcode == spv::Op::OpBufferPointerEXT) {
      auto result_type = vstate.FindDef(inst.type_id());
      auto sc = result_type->GetOperandAs<spv::StorageClass>(1);
      if (sc == spv::StorageClass::Uniform && is_non_writable) {
        return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
               << "Target of NonWritable decoration is invalid: "
               << "cannot be used to OpBufferPointerEXT "
               << "with Uniform storage class";
      }
      return SPV_SUCCESS;
    }

    if ((var_storage_class == spv::StorageClass::Function ||
         var_storage_class == spv::StorageClass::Private) &&
        vstate.features().nonwritable_var_in_function_or_private &&
        is_non_writable) {
      // New permitted feature in SPIR-V 1.4.
    } else if (var_storage_class == spv::StorageClass::TileAttachmentQCOM) {
    } else if (
        // It may point to a UBO, SSBO, storage image, or raw access chain.
        vstate.IsPointerToUniformBlock(type_id) ||
        vstate.IsPointerToStorageBuffer(type_id) ||
        vstate.IsPointerToStorageImage(type_id) ||
        vstate.IsPointerToTensor(type_id) ||
        opcode == spv::Op::OpRawAccessChainNV) {
    } else {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "Target of "
             << (is_non_writable ? "NonWritable" : "NonReadable")
             << " decoration is invalid: "
                "must point to a "
                "storage image, tensor variable in UniformConstant storage "
                "class, uniform block, "
             << (vstate.features().nonwritable_var_in_function_or_private &&
                         is_non_writable
                     ? "storage buffer, or variable in Private or Function "
                       "storage class"
                     : "or storage buffer");
    }
  }

  return SPV_SUCCESS;
}

// Returns SPV_SUCCESS if validation rules are satisfied for Uniform or
// UniformId decorations. Otherwise emits a diagnostic and returns something
// other than SPV_SUCCESS. Assumes each decoration on a group has been
// propagated down to the group members.  The |inst| parameter is the object
// being decorated.
spv_result_t CheckUniformDecoration(ValidationState_t& vstate,
                                    const Instruction& inst,
                                    const Decoration& decoration) {
  const char* const dec_name = decoration.dec_type() == spv::Decoration::Uniform
                                   ? "Uniform"
                                   : "UniformId";

  // Uniform or UniformId must decorate an "object"
  //  - has a result ID
  //  - is an instantiation of a non-void type.  So it has a type ID, and that
  //  type is not void.

  // We already know the result ID is non-zero.

  if (inst.type_id() == 0) {
    return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
           << dec_name << " decoration applied to a non-object";
  }
  if (Instruction* type_inst = vstate.FindDef(inst.type_id())) {
    if (type_inst->opcode() == spv::Op::OpTypeVoid) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << dec_name << " decoration applied to a value with void type";
    }
  } else {
    // We might never get here because this would have been rejected earlier in
    // the flow.
    return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
           << dec_name << " decoration applied to an object with invalid type";
  }

  // Use of Uniform with OpDecorate is checked elsewhere.
  // Use of UniformId with OpDecorateId is checked elsewhere.

  if (decoration.dec_type() == spv::Decoration::UniformId) {
    assert(decoration.params().size() == 1 &&
           "Grammar ensures UniformId has one parameter");

    // The scope id is an execution scope.
    if (auto error =
            ValidateExecutionScope(vstate, &inst, decoration.params()[0]))
      return error;
  }

  return SPV_SUCCESS;
}

// Returns SPV_SUCCESS if validation rules are satisfied for NoSignedWrap or
// NoUnsignedWrap decorations. Otherwise emits a diagnostic and returns
// something other than SPV_SUCCESS. Assumes each decoration on a group has been
// propagated down to the group members.
spv_result_t CheckIntegerWrapDecoration(ValidationState_t& vstate,
                                        const Instruction& inst,
                                        const Decoration& decoration) {
  switch (inst.opcode()) {
    case spv::Op::OpIAdd:
    case spv::Op::OpISub:
    case spv::Op::OpIMul:
    case spv::Op::OpShiftLeftLogical:
    case spv::Op::OpSNegate:
      return SPV_SUCCESS;
    case spv::Op::OpExtInst:
    case spv::Op::OpExtInstWithForwardRefsKHR:
      // TODO(dneto): Only certain extended instructions allow these
      // decorations.  For now allow anything.
      return SPV_SUCCESS;
    default:
      break;
  }

  return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
         << (decoration.dec_type() == spv::Decoration::NoSignedWrap
                 ? "NoSignedWrap"
                 : "NoUnsignedWrap")
         << " decoration may not be applied to "
         << spvOpcodeString(inst.opcode());
}

// Returns SPV_SUCCESS if validation rules are satisfied for the Component
// decoration.  Otherwise emits a diagnostic and returns something other than
// SPV_SUCCESS.
spv_result_t CheckComponentDecoration(ValidationState_t& vstate,
                                      const Instruction& inst,
                                      const Decoration& decoration) {
  assert(inst.id() && "Parser ensures the target of the decoration has an ID");
  assert(decoration.params().size() == 1 &&
         "Grammar ensures Component has one parameter");

  uint32_t type_id;
  if (decoration.struct_member_index() == Decoration::kInvalidMember) {
    // The target must be a memory object declaration.
    const auto opcode = inst.opcode();
    if (opcode != spv::Op::OpVariable &&
        opcode != spv::Op::OpFunctionParameter) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "Target of Component decoration must be a memory object "
                "declaration (a variable or a function parameter)";
    }

    // Only valid for the Input and Output Storage Classes.
    const auto storage_class = opcode == spv::Op::OpVariable
                                   ? inst.GetOperandAs<spv::StorageClass>(2)
                                   : spv::StorageClass::Max;
    if (storage_class != spv::StorageClass::Input &&
        storage_class != spv::StorageClass::Output &&
        storage_class != spv::StorageClass::Max) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "Target of Component decoration is invalid: must point to a "
                "Storage Class of Input(1) or Output(3). Found Storage "
                "Class "
             << uint32_t(storage_class);
    }

    type_id = inst.type_id();
    if (vstate.IsPointerType(type_id)) {
      const auto pointer = vstate.FindDef(type_id);
      type_id = pointer->GetOperandAs<uint32_t>(2);
    }
  } else {
    if (inst.opcode() != spv::Op::OpTypeStruct) {
      return vstate.diag(SPV_ERROR_INVALID_DATA, &inst)
             << "Attempted to get underlying data type via member index for "
                "non-struct type.";
    }
    type_id = inst.word(decoration.struct_member_index() + 2);
  }

  if (spvIsVulkanEnv(vstate.context()->target_env)) {
    // Strip the array, if present.
    while (vstate.GetIdOpcode(type_id) == spv::Op::OpTypeArray) {
      type_id = vstate.FindDef(type_id)->word(2u);
    }

    if (!vstate.IsIntScalarOrVectorType(type_id) &&
        !vstate.IsFloatScalarOrVectorType(type_id)) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << vstate.VkErrorID(10583)
             << "Component decoration specified for type "
             << vstate.getIdName(type_id) << " that is not a scalar or vector";
    }

    const auto component = decoration.params()[0];
    if (component > 3) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << vstate.VkErrorID(4920)
             << "Component decoration value must not be greater than 3";
    }

    const auto dimension = vstate.GetDimension(type_id);
    const auto bit_width = vstate.GetBitWidth(type_id);
    if (bit_width == 16 || bit_width == 32) {
      const auto sum_component = component + dimension;
      if (sum_component > 4) {
        return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
               << vstate.VkErrorID(4921)
               << "Sequence of components starting with " << component
               << " and ending with " << (sum_component - 1)
               << " gets larger than 3";
      }
    } else if (bit_width == 64) {
      if (dimension > 2) {
        return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
               << vstate.VkErrorID(7703)
               << "Component decoration only allowed on 64-bit scalar and "
                  "2-component vector";
      }
      if (component == 1 || component == 3) {
        return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
               << vstate.VkErrorID(4923)
               << "Component decoration value must not be 1 or 3 for 64-bit "
                  "data types";
      }
      // 64-bit is double per component dimension
      const auto sum_component = component + (2 * dimension);
      if (sum_component > 4) {
        return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
               << vstate.VkErrorID(4922)
               << "Sequence of components starting with " << component
               << " and ending with " << (sum_component - 1)
               << " gets larger than 3";
      }
    }
  }

  return SPV_SUCCESS;
}

// Returns SPV_SUCCESS if validation rules are satisfied for the Block
// decoration.  Otherwise emits a diagnostic and returns something other than
// SPV_SUCCESS.
spv_result_t CheckBlockDecoration(ValidationState_t& vstate,
                                  const Instruction& inst,
                                  const Decoration& decoration) {
  assert(inst.id() && "Parser ensures the target of the decoration has an ID");
  if (inst.opcode() != spv::Op::OpTypeStruct) {
    const char* const dec_name = decoration.dec_type() == spv::Decoration::Block
                                     ? "Block"
                                     : "BufferBlock";
    return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
           << dec_name << " decoration on a non-struct type.";
  }
  return SPV_SUCCESS;
}

spv_result_t CheckLocationDecoration(ValidationState_t& vstate,
                                     const Instruction& inst,
                                     const Decoration& decoration) {
  if (inst.opcode() == spv::Op::OpVariable) return SPV_SUCCESS;

  if (decoration.struct_member_index() != Decoration::kInvalidMember &&
      inst.opcode() == spv::Op::OpTypeStruct) {
    return SPV_SUCCESS;
  }

  return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
         << "Location decoration can only be applied to a variable or member "
            "of a structure type";
}

spv_result_t CheckRelaxPrecisionDecoration(ValidationState_t& vstate,
                                           const Instruction& inst,
                                           const Decoration& decoration) {
  // This is not the most precise check, but the rules for RelaxPrecision are
  // very general, and it will be difficult to implement precisely.  For now,
  // I will only check for the cases that cause problems for the optimizer.
  if (!spvOpcodeGeneratesType(inst.opcode())) {
    return SPV_SUCCESS;
  }

  if (decoration.struct_member_index() != Decoration::kInvalidMember &&
      inst.opcode() == spv::Op::OpTypeStruct) {
    return SPV_SUCCESS;
  }
  return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
         << "RelaxPrecision decoration cannot be applied to a type";
}

#define PASS_OR_BAIL_AT_LINE(X, LINE)           \
  {                                             \
    spv_result_t e##LINE = (X);                 \
    if (e##LINE != SPV_SUCCESS) return e##LINE; \
  }                                             \
  static_assert(true, "require extra semicolon")
#define PASS_OR_BAIL(X) PASS_OR_BAIL_AT_LINE(X, __LINE__)

// Check rules for decorations where we start from the decoration rather
// than the decorated object.  Assumes each decoration on a group have been
// propagated down to the group members.
spv_result_t CheckDecorationsFromDecoration(ValidationState_t& vstate) {
  const bool is_shader = vstate.HasCapability(spv::Capability::Shader);
  const bool is_kernel = vstate.HasCapability(spv::Capability::Kernel);

  for (const auto& kv : vstate.id_decorations()) {
    const uint32_t id = kv.first;
    const auto& decorations = kv.second;
    if (decorations.empty()) continue;

    const Instruction* inst = vstate.FindDef(id);
    assert(inst);

    // We assume the decorations applied to a decoration group have already
    // been propagated down to the group members.
    if (inst->opcode() == spv::Op::OpDecorationGroup) continue;

    for (const auto& decoration : decorations) {
      switch (decoration.dec_type()) {
        case spv::Decoration::Component:
          PASS_OR_BAIL(CheckComponentDecoration(vstate, *inst, decoration));
          break;
        case spv::Decoration::FPRoundingMode:
          if (is_shader)
            PASS_OR_BAIL(
                CheckFPRoundingModeForShaders(vstate, *inst, decoration));
          if (is_kernel)
            PASS_OR_BAIL(CheckFPRoundingModeForKernels(vstate, *inst));
          break;
        case spv::Decoration::NonReadable:
        case spv::Decoration::NonWritable:
          PASS_OR_BAIL(
              CheckNonReadableWritableDecorations(vstate, *inst, decoration));
          break;
        case spv::Decoration::Uniform:
        case spv::Decoration::UniformId:
          PASS_OR_BAIL(CheckUniformDecoration(vstate, *inst, decoration));
          break;
        case spv::Decoration::NoSignedWrap:
        case spv::Decoration::NoUnsignedWrap:
          PASS_OR_BAIL(CheckIntegerWrapDecoration(vstate, *inst, decoration));
          break;
        case spv::Decoration::Block:
        case spv::Decoration::BufferBlock:
          PASS_OR_BAIL(CheckBlockDecoration(vstate, *inst, decoration));
          break;
        case spv::Decoration::Location:
          PASS_OR_BAIL(CheckLocationDecoration(vstate, *inst, decoration));
          break;
        case spv::Decoration::RelaxedPrecision:
          PASS_OR_BAIL(
              CheckRelaxPrecisionDecoration(vstate, *inst, decoration));
          break;
        default:
          break;
      }
    }
  }
  return SPV_SUCCESS;
}

}  // namespace

spv_result_t ValidateDecorations(ValidationState_t& vstate) {
  if (auto error = CheckImportedVariableInitialization(vstate)) return error;
  if (auto error = CheckDecorationsOfEntryPoints(vstate)) return error;
  if (auto error = CheckDecorationsOfBuffers(vstate)) return error;
  if (auto error = CheckDecorationsOfVariables(vstate)) return error;
  if (auto error = CheckDecorationsCompatibility(vstate)) return error;
  if (auto error = CheckLinkageAttrOfFunctions(vstate)) return error;
  if (auto error = CheckVulkanMemoryModelDeprecatedDecorations(vstate))
    return error;
  if (auto error = CheckDecorationsFromDecoration(vstate)) return error;
  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
