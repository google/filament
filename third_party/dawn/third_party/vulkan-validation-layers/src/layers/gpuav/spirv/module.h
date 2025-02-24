/* Copyright (c) 2024-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <stdint.h>
#include <vector>
#include "link.h"
#include "interface.h"
#include "function_basic_block.h"
#include "type_manager.h"
#include "containers/custom_containers.h"

class DebugReport;
struct DeviceFeatures;

namespace gpuav {
namespace spirv {

struct ModuleHeader {
    uint32_t magic_number;
    uint32_t version;
    uint32_t generator;
    uint32_t bound;
    uint32_t schema;
};

struct Settings {
    uint32_t shader_id;
    uint32_t output_buffer_descriptor_set;
    bool print_debug_info;
    uint32_t max_instrumentations_count;
    bool support_non_semantic_info;
    bool has_bindless_descriptors;
};

// This is the "brain" of SPIR-V logic, it stores the memory of all the Instructions and is the main context.
// There are other helper classes that are charge of handling the various parts of the module.
// The Module takes SPIR-V, has each pass modify it, then dumps it out into the instrumented SPIR-V
class Module {
  public:
    Module(vvl::span<const uint32_t> words, DebugReport* debug_report, const Settings& settings,
           const DeviceFeatures& enabled_features, const std::vector<std::vector<BindingLayout>>& set_index_to_bindings_layout_lut);

    // Memory that holds all the actual SPIR-V data, replicate the "Logical Layout of a Module" of SPIR-V.
    // Divided into sections to make easier to modify each part at different times, but still keeps it simple to write out all the
    // instructions to a binary format.
    ModuleHeader header_;
    InstructionList capabilities_;
    InstructionList extensions_;
    InstructionList ext_inst_imports_;
    InstructionList memory_model_;
    InstructionList entry_points_;
    InstructionList execution_modes_;
    InstructionList debug_source_;
    InstructionList debug_name_;
    InstructionList debug_module_processed_;
    InstructionList annotations_;
    InstructionList types_values_constants_;
    FunctionList functions_;

    // Handles all types and constants
    TypeManager type_manager_;

    // When adding a new instruction with result ID, will need to grab the next ID
    uint32_t TakeNextId();

    // Order of functions that will try to be linked in
    std::vector<LinkInfo> link_info_;
    void LinkFunction(const LinkInfo& info);
    void PostProcess();

    // The class is designed to be written out to a binary file.
    void ToBinary(std::vector<uint32_t>& out);

    void AddInterfaceVariables(uint32_t id, spv::StorageClass storage_class);

    // Helpers
    bool HasCapability(spv::Capability capability);
    void AddCapability(spv::Capability capability);
    void AddExtension(const char* extension);
    void AddDebugName(const char* name, uint32_t id);
    void AddDecoration(uint32_t target_id, spv::Decoration decoration, const std::vector<uint32_t>& operands);
    void AddMemberDecoration(uint32_t target_id, uint32_t index, spv::Decoration decoration, const std::vector<uint32_t>& operands);

    const uint32_t max_instrumentations_count_ = 0;  // zero is same as "unlimited"
    bool use_bda_ = false;
    // provides a way to map back and know which original SPIR-V this was from
    const uint32_t shader_id_;
    // Will replace the "OpDecorate DescriptorSet" for the output buffer in the incoming linked module
    // This allows anything to be set in the GLSL for the set value, as we change it at runtime
    const uint32_t output_buffer_descriptor_set_;

    const bool support_non_semantic_info_;
    const DeviceFeatures& enabled_features_;

    // TODO - To make things simple to start, decide if the whole shader has anything bindless or not. The next step will be a
    // system to pass in the information from the descriptor set layout to build a LUT of which OpVariable point to bindless
    // descriptors. This will require special consideration as it will break a simple way to test standalone version of the
    // instrumentation
    bool has_bindless_descriptors_ = false;

    // Used to help debug
    const bool print_debug_info_;

    // To keep the GPU Shader Instrumentation a standalone sub-project, the runtime version needs to pass in info to allow for
    // warnings/errors to be piped into the normal callback (otherwise will be sent to stdout)
    DebugReport* debug_report_ = nullptr;
    void InternalWarning(const char* tag, const char* message);
    void InternalError(const char* tag, const char* message);

    // < set, [ bindings ] >
    const std::vector<std::vector<BindingLayout>>& set_index_to_bindings_layout_lut_;
};

}  // namespace spirv
}  // namespace gpuav
