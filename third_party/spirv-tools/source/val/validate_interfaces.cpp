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

#include <algorithm>
#include <vector>

#include "source/diagnostic.h"
#include "source/spirv_constant.h"
#include "source/spirv_target_env.h"
#include "source/val/function.h"
#include "source/val/instruction.h"
#include "source/val/validate.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

// Returns true if \c inst is an input or output variable.
bool is_interface_variable(const Instruction* inst, bool is_spv_1_4) {
  if (is_spv_1_4) {
    // Starting in SPIR-V 1.4, all global variables are interface variables.
    return inst->opcode() == SpvOpVariable &&
           inst->word(3u) != SpvStorageClassFunction;
  } else {
    return inst->opcode() == SpvOpVariable &&
           (inst->word(3u) == SpvStorageClassInput ||
            inst->word(3u) == SpvStorageClassOutput);
  }
}

// Checks that \c var is listed as an interface in all the entry points that use
// it.
spv_result_t check_interface_variable(ValidationState_t& _,
                                      const Instruction* var) {
  std::vector<const Function*> functions;
  std::vector<const Instruction*> uses;
  for (auto use : var->uses()) {
    uses.push_back(use.first);
  }
  for (uint32_t i = 0; i < uses.size(); ++i) {
    const auto user = uses[i];
    if (const Function* func = user->function()) {
      functions.push_back(func);
    } else {
      // In the rare case that the variable is used by another instruction in
      // the global scope, continue searching for an instruction used in a
      // function.
      for (auto use : user->uses()) {
        uses.push_back(use.first);
      }
    }
  }

  std::sort(functions.begin(), functions.end(),
            [](const Function* lhs, const Function* rhs) {
              return lhs->id() < rhs->id();
            });
  functions.erase(std::unique(functions.begin(), functions.end()),
                  functions.end());

  std::vector<uint32_t> entry_points;
  for (const auto func : functions) {
    for (auto id : _.FunctionEntryPoints(func->id())) {
      entry_points.push_back(id);
    }
  }

  std::sort(entry_points.begin(), entry_points.end());
  entry_points.erase(std::unique(entry_points.begin(), entry_points.end()),
                     entry_points.end());

  for (auto id : entry_points) {
    for (const auto& desc : _.entry_point_descriptions(id)) {
      bool found = false;
      for (auto interface : desc.interfaces) {
        if (var->id() == interface) {
          found = true;
          break;
        }
      }
      if (!found) {
        return _.diag(SPV_ERROR_INVALID_ID, var)
               << "Interface variable id <" << var->id()
               << "> is used by entry point '" << desc.name << "' id <" << id
               << ">, but is not listed as an interface";
      }
    }
  }

  return SPV_SUCCESS;
}

// This function assumes a base location has been determined already. As such
// any further location decorations are invalid.
// TODO: if this code turns out to be slow, there is an opportunity to cache
// the result for a given type id.
spv_result_t NumConsumedLocations(ValidationState_t& _, const Instruction* type,
                                  uint32_t* num_locations) {
  *num_locations = 0;
  switch (type->opcode()) {
    case SpvOpTypeInt:
    case SpvOpTypeFloat:
      // Scalars always consume a single location.
      *num_locations = 1;
      break;
    case SpvOpTypeVector:
      // 3- and 4-component 64-bit vectors consume two locations.
      if ((_.ContainsSizedIntOrFloatType(type->id(), SpvOpTypeInt, 64) ||
           _.ContainsSizedIntOrFloatType(type->id(), SpvOpTypeFloat, 64)) &&
          (type->GetOperandAs<uint32_t>(2) > 2)) {
        *num_locations = 2;
      } else {
        *num_locations = 1;
      }
      break;
    case SpvOpTypeMatrix:
      // Matrices consume locations equal to the underlying vector type for
      // each column.
      NumConsumedLocations(_, _.FindDef(type->GetOperandAs<uint32_t>(1)),
                           num_locations);
      *num_locations *= type->GetOperandAs<uint32_t>(2);
      break;
    case SpvOpTypeArray: {
      // Arrays consume locations equal to the underlying type times the number
      // of elements in the vector.
      NumConsumedLocations(_, _.FindDef(type->GetOperandAs<uint32_t>(1)),
                           num_locations);
      bool is_int = false;
      bool is_const = false;
      uint32_t value = 0;
      // Attempt to evaluate the number of array elements.
      std::tie(is_int, is_const, value) =
          _.EvalInt32IfConst(type->GetOperandAs<uint32_t>(2));
      if (is_int && is_const) *num_locations *= value;
      break;
    }
    case SpvOpTypeStruct: {
      // Members cannot have location decorations at this point.
      if (_.HasDecoration(type->id(), SpvDecorationLocation)) {
        return _.diag(SPV_ERROR_INVALID_DATA, type)
               << "Members cannot be assigned a location";
      }

      // Structs consume locations equal to the sum of the locations consumed
      // by the members.
      for (uint32_t i = 1; i < type->operands().size(); ++i) {
        uint32_t member_locations = 0;
        if (auto error = NumConsumedLocations(
                _, _.FindDef(type->GetOperandAs<uint32_t>(i)),
                &member_locations)) {
          return error;
        }
        *num_locations += member_locations;
      }
      break;
    }
    default:
      break;
  }

  return SPV_SUCCESS;
}

// Returns the number of components consumed by types that support a component
// decoration.
uint32_t NumConsumedComponents(ValidationState_t& _, const Instruction* type) {
  uint32_t num_components = 0;
  switch (type->opcode()) {
    case SpvOpTypeInt:
    case SpvOpTypeFloat:
      // 64-bit types consume two components.
      if (type->GetOperandAs<uint32_t>(1) == 64) {
        num_components = 2;
      } else {
        num_components = 1;
      }
      break;
    case SpvOpTypeVector:
      // Vectors consume components equal to the underlying type's consumption
      // times the number of elements in the vector. Note that 3- and 4-element
      // vectors cannot have a component decoration (i.e. assumed to be zero).
      num_components =
          NumConsumedComponents(_, _.FindDef(type->GetOperandAs<uint32_t>(1)));
      num_components *= type->GetOperandAs<uint32_t>(2);
      break;
    default:
      // This is an error that is validated elsewhere.
      break;
  }

  return num_components;
}

// Populates |locations| (and/or |output_index1_locations|) with the use
// location and component coordinates for |variable|. Indices are calculated as
// 4 * location + component.
spv_result_t GetLocationsForVariable(
    ValidationState_t& _, const Instruction* entry_point,
    const Instruction* variable, std::unordered_set<uint32_t>* locations,
    std::unordered_set<uint32_t>* output_index1_locations) {
  const bool is_fragment = entry_point->GetOperandAs<SpvExecutionModel>(0) ==
                           SpvExecutionModelFragment;
  const bool is_output =
      variable->GetOperandAs<SpvStorageClass>(2) == SpvStorageClassOutput;
  auto ptr_type_id = variable->GetOperandAs<uint32_t>(0);
  auto ptr_type = _.FindDef(ptr_type_id);
  auto type_id = ptr_type->GetOperandAs<uint32_t>(2);
  auto type = _.FindDef(type_id);

  // Check for Location, Component and Index decorations on the variable. The
  // validator allows duplicate decorations if the location/component/index are
  // equal. Also track Patch and PerTaskNV decorations.
  bool has_location = false;
  uint32_t location = 0;
  bool has_component = false;
  uint32_t component = 0;
  bool has_index = false;
  uint32_t index = 0;
  bool has_patch = false;
  bool has_per_task_nv = false;
  bool has_per_vertex_nv = false;
  for (auto& dec : _.id_decorations(variable->id())) {
    if (dec.dec_type() == SpvDecorationLocation) {
      if (has_location && dec.params()[0] != location) {
        return _.diag(SPV_ERROR_INVALID_DATA, variable)
               << "Variable has conflicting location decorations";
      }
      has_location = true;
      location = dec.params()[0];
    } else if (dec.dec_type() == SpvDecorationComponent) {
      if (has_component && dec.params()[0] != component) {
        return _.diag(SPV_ERROR_INVALID_DATA, variable)
               << "Variable has conflicting component decorations";
      }
      has_component = true;
      component = dec.params()[0];
    } else if (dec.dec_type() == SpvDecorationIndex) {
      if (!is_output || !is_fragment) {
        return _.diag(SPV_ERROR_INVALID_DATA, variable)
               << "Index can only be applied to Fragment output variables";
      }
      if (has_index && dec.params()[0] != index) {
        return _.diag(SPV_ERROR_INVALID_DATA, variable)
               << "Variable has conflicting index decorations";
      }
      has_index = true;
      index = dec.params()[0];
    } else if (dec.dec_type() == SpvDecorationBuiltIn) {
      // Don't check built-ins.
      return SPV_SUCCESS;
    } else if (dec.dec_type() == SpvDecorationPatch) {
      has_patch = true;
    } else if (dec.dec_type() == SpvDecorationPerTaskNV) {
      has_per_task_nv = true;
    } else if (dec.dec_type() == SpvDecorationPerVertexNV) {
      has_per_vertex_nv = true;
    }
  }

  // Vulkan 14.1.3: Tessellation control and mesh per-vertex outputs and
  // tessellation control, evaluation and geometry per-vertex inputs have a
  // layer of arraying that is not included in interface matching.
  bool is_arrayed = false;
  switch (entry_point->GetOperandAs<SpvExecutionModel>(0)) {
    case SpvExecutionModelTessellationControl:
      if (!has_patch) {
        is_arrayed = true;
      }
      break;
    case SpvExecutionModelTessellationEvaluation:
      if (!is_output && !has_patch) {
        is_arrayed = true;
      }
      break;
    case SpvExecutionModelGeometry:
      if (!is_output) {
        is_arrayed = true;
      }
      break;
    case SpvExecutionModelFragment:
      if (!is_output && has_per_vertex_nv) {
        is_arrayed = true;
      }
      break;
    case SpvExecutionModelMeshNV:
      if (is_output && !has_per_task_nv) {
        is_arrayed = true;
      }
      break;
    default:
      break;
  }

  // Unpack arrayness.
  if (is_arrayed && (type->opcode() == SpvOpTypeArray ||
                     type->opcode() == SpvOpTypeRuntimeArray)) {
    type_id = type->GetOperandAs<uint32_t>(1);
    type = _.FindDef(type_id);
  }

  if (type->opcode() == SpvOpTypeStruct) {
    // Don't check built-ins.
    if (_.HasDecoration(type_id, SpvDecorationBuiltIn)) return SPV_SUCCESS;
  }

  // Only block-decorated structs don't need a location on the variable.
  const bool is_block = _.HasDecoration(type_id, SpvDecorationBlock);
  if (!has_location && !is_block) {
    return _.diag(SPV_ERROR_INVALID_DATA, variable)
           << "Variable must be decorated with a location";
  }

  const std::string storage_class = is_output ? "output" : "input";
  if (has_location) {
    auto sub_type = type;
    bool is_int = false;
    bool is_const = false;
    uint32_t array_size = 1;
    // If the variable is still arrayed, mark the locations/components per
    // index.
    if (type->opcode() == SpvOpTypeArray) {
      // Determine the array size if possible and get the element type.
      std::tie(is_int, is_const, array_size) =
          _.EvalInt32IfConst(type->GetOperandAs<uint32_t>(2));
      if (!is_int || !is_const) array_size = 1;
      auto sub_type_id = type->GetOperandAs<uint32_t>(1);
      sub_type = _.FindDef(sub_type_id);
    }

    for (uint32_t array_idx = 0; array_idx < array_size; ++array_idx) {
      uint32_t num_locations = 0;
      if (auto error = NumConsumedLocations(_, sub_type, &num_locations))
        return error;

      uint32_t num_components = NumConsumedComponents(_, sub_type);
      uint32_t array_location = location + (num_locations * array_idx);
      uint32_t start = array_location * 4;
      uint32_t end = (array_location + num_locations) * 4;
      if (num_components != 0) {
        start += component;
        end = array_location * 4 + component + num_components;
      }

      auto locs = locations;
      if (has_index && index == 1) locs = output_index1_locations;

      for (uint32_t i = start; i < end; ++i) {
        if (!locs->insert(i).second) {
          return _.diag(SPV_ERROR_INVALID_DATA, entry_point)
                 << "Entry-point has conflicting " << storage_class
                 << " location assignment at location " << i / 4
                 << ", component " << i % 4;
        }
      }
    }
  } else {
    // For Block-decorated structs with no location assigned to the variable,
    // each member of the block must be assigned a location. Also record any
    // member component assignments. The validator allows duplicate decorations
    // if they agree on the location/component.
    std::unordered_map<uint32_t, uint32_t> member_locations;
    std::unordered_map<uint32_t, uint32_t> member_components;
    for (auto& dec : _.id_decorations(type_id)) {
      if (dec.dec_type() == SpvDecorationLocation) {
        auto where = member_locations.find(dec.struct_member_index());
        if (where == member_locations.end()) {
          member_locations[dec.struct_member_index()] = dec.params()[0];
        } else if (where->second != dec.params()[0]) {
          return _.diag(SPV_ERROR_INVALID_DATA, type)
                 << "Member index " << dec.struct_member_index()
                 << " has conflicting location assignments";
        }
      } else if (dec.dec_type() == SpvDecorationComponent) {
        auto where = member_components.find(dec.struct_member_index());
        if (where == member_components.end()) {
          member_components[dec.struct_member_index()] = dec.params()[0];
        } else if (where->second != dec.params()[0]) {
          return _.diag(SPV_ERROR_INVALID_DATA, type)
                 << "Member index " << dec.struct_member_index()
                 << " has conflicting component assignments";
        }
      }
    }

    for (uint32_t i = 1; i < type->operands().size(); ++i) {
      auto where = member_locations.find(i - 1);
      if (where == member_locations.end()) {
        return _.diag(SPV_ERROR_INVALID_DATA, type)
               << "Member index " << i - 1
               << " is missing a location assignment";
      }

      location = where->second;
      auto member = _.FindDef(type->GetOperandAs<uint32_t>(i));
      uint32_t num_locations = 0;
      if (auto error = NumConsumedLocations(_, member, &num_locations))
        return error;

      // If the component is not specified, it is assumed to be zero.
      uint32_t num_components = NumConsumedComponents(_, member);
      component = 0;
      if (member_components.count(i - 1)) {
        component = member_components[i - 1];
      }

      uint32_t start = location * 4;
      uint32_t end = (location + num_locations) * 4;
      if (num_components != 0) {
        start += component;
        end = location * 4 + component + num_components;
      }
      for (uint32_t l = start; l < end; ++l) {
        if (!locations->insert(l).second) {
          return _.diag(SPV_ERROR_INVALID_DATA, entry_point)
                 << "Entry-point has conflicting " << storage_class
                 << " location assignment at location " << l / 4
                 << ", component " << l % 4;
        }
      }
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateLocations(ValidationState_t& _,
                               const Instruction* entry_point) {
  // According to Vulkan 14.1 only the following execution models have
  // locations assigned.
  switch (entry_point->GetOperandAs<SpvExecutionModel>(0)) {
    case SpvExecutionModelVertex:
    case SpvExecutionModelTessellationControl:
    case SpvExecutionModelTessellationEvaluation:
    case SpvExecutionModelGeometry:
    case SpvExecutionModelFragment:
      break;
    default:
      return SPV_SUCCESS;
  }

  // Locations are stored as a combined location and component values.
  std::unordered_set<uint32_t> input_locations;
  std::unordered_set<uint32_t> output_locations_index0;
  std::unordered_set<uint32_t> output_locations_index1;
  for (uint32_t i = 3; i < entry_point->operands().size(); ++i) {
    auto interface_id = entry_point->GetOperandAs<uint32_t>(i);
    auto interface_var = _.FindDef(interface_id);
    auto storage_class = interface_var->GetOperandAs<SpvStorageClass>(2);
    if (storage_class != SpvStorageClassInput &&
        storage_class != SpvStorageClassOutput) {
      continue;
    }

    auto locations = (storage_class == SpvStorageClassInput)
                         ? &input_locations
                         : &output_locations_index0;
    if (auto error = GetLocationsForVariable(
            _, entry_point, interface_var, locations, &output_locations_index1))
      return error;
  }

  return SPV_SUCCESS;
}

}  // namespace

spv_result_t ValidateInterfaces(ValidationState_t& _) {
  bool is_spv_1_4 = _.version() >= SPV_SPIRV_VERSION_WORD(1, 4);
  for (auto& inst : _.ordered_instructions()) {
    if (is_interface_variable(&inst, is_spv_1_4)) {
      if (auto error = check_interface_variable(_, &inst)) {
        return error;
      }
    }
  }

  if (spvIsVulkanEnv(_.context()->target_env)) {
    for (auto& inst : _.ordered_instructions()) {
      if (inst.opcode() == SpvOpEntryPoint) {
        if (auto error = ValidateLocations(_, &inst)) {
          return error;
        }
      }
      if (inst.opcode() == SpvOpTypeVoid) break;
    }
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
