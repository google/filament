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

#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "error_message/logging.h"
#include "error_message/log_message_type.h"

#include <iostream>

#include "generated/device_features.h"

namespace gpuav {
namespace spirv {

Module::Module(vvl::span<const uint32_t> words, DebugReport* debug_report, const Settings& settings,
               const DeviceFeatures& enabled_features,
               const std::vector<std::vector<BindingLayout>>& set_index_to_bindings_layout_lut)
    : type_manager_(*this),
      max_instrumentations_count_(settings.max_instrumentations_count),
      shader_id_(settings.shader_id),
      output_buffer_descriptor_set_(settings.output_buffer_descriptor_set),
      support_non_semantic_info_(settings.support_non_semantic_info),
      enabled_features_(enabled_features),
      has_bindless_descriptors_(settings.has_bindless_descriptors),
      print_debug_info_(settings.print_debug_info),
      debug_report_(debug_report),
      set_index_to_bindings_layout_lut_(set_index_to_bindings_layout_lut) {
    uint32_t instruction_count = 0;
    spirv_iterator it = words.begin();
    header_.magic_number = *it++;
    header_.version = *it++;
    header_.generator = *it++;
    header_.bound = *it++;
    header_.schema = *it++;
    // Parse everything up until the first function and sort into seperate lists
    while (it != words.end()) {
        const uint32_t opcode = *it & 0x0ffffu;
        const uint32_t length = *it >> 16;
        if (opcode == spv::OpFunction) {
            break;
        }
        auto new_inst = std::make_unique<Instruction>(it, instruction_count++);

        switch (opcode) {
            case spv::OpCapability:
                capabilities_.emplace_back(std::move(new_inst));
                break;
            case spv::OpExtension:
                extensions_.emplace_back(std::move(new_inst));
                break;
            case spv::OpExtInstImport:
                ext_inst_imports_.emplace_back(std::move(new_inst));
                break;
            case spv::OpMemoryModel:
                memory_model_.emplace_back(std::move(new_inst));
                break;
            case spv::OpEntryPoint:
                entry_points_.emplace_back(std::move(new_inst));
                break;
            case spv::OpExecutionMode:
            case spv::OpExecutionModeId:
                execution_modes_.emplace_back(std::move(new_inst));
                break;
            case spv::OpString:
            case spv::OpSourceExtension:
            case spv::OpSource:
            case spv::OpSourceContinued:
                debug_source_.emplace_back(std::move(new_inst));
                break;
            case spv::OpName:
            case spv::OpMemberName:
                debug_name_.emplace_back(std::move(new_inst));
                break;
            case spv::OpModuleProcessed:
                debug_module_processed_.emplace_back(std::move(new_inst));
                break;
            case spv::OpLine:
            case spv::OpNoLine:
                // OpLine must not be groupped in between other debug operations
                // https://github.com/KhronosGroup/SPIRV-Tools/issues/5513
                types_values_constants_.emplace_back(std::move(new_inst));
                break;
            case spv::OpDecorate:
            case spv::OpMemberDecorate:
            case spv::OpDecorationGroup:
            case spv::OpGroupDecorate:
            case spv::OpGroupMemberDecorate:
            case spv::OpDecorateId:
            case spv::OpDecorateString:
            case spv::OpMemberDecorateString:
                annotations_.emplace_back(std::move(new_inst));
                break;

            case spv::OpSpecConstantTrue:
            case spv::OpSpecConstantFalse:
            case spv::OpConstantTrue:
            case spv::OpConstantFalse: {
                const Type& type = type_manager_.GetTypeBool();
                type_manager_.AddConstant(std::move(new_inst), type);
                break;
            }
            case spv::OpSpecConstant:
            case spv::OpConstant:
            case spv::OpConstantNull:
            case spv::OpConstantComposite: {
                const Type* type = type_manager_.FindTypeById(new_inst->TypeId());
                type_manager_.AddConstant(std::move(new_inst), *type);
                break;
            }
            case spv::OpVariable: {
                const Type* type = type_manager_.FindTypeById(new_inst->TypeId());
                const Variable& new_var = type_manager_.AddVariable(std::move(new_inst), *type);

                // While adding the global variables, detect if descriptors is bindless or not
                spv::StorageClass storage_class = new_var.StorageClass();
                // These are the only storage classes that interface with a descriptor
                // see vkspec.html#interfaces-resources-descset
                if (storage_class == spv::StorageClassUniform || storage_class == spv::StorageClassUniformConstant ||
                    storage_class == spv::StorageClassStorageBuffer) {
                    const Type* ptr_type = new_var.PointerType(type_manager_);
                    // The shader will also have OpCapability RuntimeDescriptorArray
                    if (ptr_type->spv_type_ == SpvType::kRuntimeArray) {
                        // TODO - This might not actually need to be marked as bindless
                        has_bindless_descriptors_ = true;
                    }
                }

                break;
            }
            default: {
                SpvType spv_type = GetSpvType(new_inst->Opcode());
                if (spv_type != SpvType::Empty) {
                    type_manager_.AddType(std::move(new_inst), spv_type);
                } else {
                    // unknown instruction, try and just keep in last section to not just crash
                    // example: OpSpecConstant
                    types_values_constants_.emplace_back(std::move(new_inst));
                }
                break;
            }
        }

        it += length;
    }

    // each function is broken up to 3 stage, pre/during/post basic_blocks
    BasicBlock* current_block = nullptr;
    Function* current_function = nullptr;
    bool block_found = false;
    bool function_end_found = false;
    while (it != words.end()) {
        const uint32_t opcode = *it & 0x0ffffu;
        const uint32_t length = *it >> 16;
        auto new_inst = std::make_unique<Instruction>(it, instruction_count++);

        if (opcode == spv::OpFunction) {
            auto new_function = std::make_unique<Function>(*this, std::move(new_inst));
            auto& added_function = functions_.emplace_back(std::move(new_function));
            current_function = &(*added_function);
            block_found = false;
            function_end_found = false;
            it += length;
            continue;
        }

        const uint32_t result_id = new_inst->ResultId();
        if (result_id != 0) {
            current_function->inst_map_[result_id] = new_inst.get();
        }

        if (opcode == spv::OpFunctionEnd) {
            function_end_found = true;
        }

        if (opcode == spv::OpLoopMerge) {
            current_block->loop_header_ = true;
        }

        if (opcode == spv::OpLabel) {
            block_found = true;
            auto new_block = std::make_unique<BasicBlock>(std::move(new_inst), *current_function);
            auto& added_block = current_function->blocks_.emplace_back(std::move(new_block));
            current_block = &(*added_block);
        } else if (function_end_found) {
            current_function->post_block_inst_.emplace_back(std::move(new_inst));
        } else if (block_found) {
            current_block->instructions_.emplace_back(std::move(new_inst));
        } else {
            current_function->pre_block_inst_.emplace_back(std::move(new_inst));
        }

        it += length;
    }
}

bool Module::HasCapability(spv::Capability capability) {
    for (const auto& inst : capabilities_) {
        if (inst->Word(1) == capability) {
            return true;
        }
    }
    return false;
}

static void StringToSpirv(const char* input, std::vector<uint32_t>& output) {
    uint32_t i = 0;
    while (*input != '\0') {
        uint32_t new_word = 0;
        for (i = 0; i < 4; i++) {
            if (*input == '\0') break;
            uint32_t value = static_cast<uint32_t>(*input);
            new_word |= value << (8 * i);
            input++;
        }
        output.push_back(new_word);
    }
    // add full null pad if word didn't end with null
    if (i == 4) {
        output.push_back(0);
    }
}

// Will only add if not already added
void Module::AddCapability(spv::Capability capability) {
    if (!HasCapability(capability)) {
        auto new_inst = std::make_unique<Instruction>(2, spv::OpCapability);
        new_inst->Fill({(uint32_t)capability});
        capabilities_.emplace_back(std::move(new_inst));
    }
}

void Module::AddExtension(const char* extension) {
    std::vector<uint32_t> words;
    StringToSpirv(extension, words);
    auto new_inst = std::make_unique<Instruction>((uint32_t)(words.size() + 1), spv::OpExtension);
    new_inst->Fill(words);
    extensions_.emplace_back(std::move(new_inst));
}

void Module::AddDebugName(const char* name, uint32_t id) {
    std::vector<uint32_t> words = {id};
    StringToSpirv(name, words);
    auto new_inst = std::make_unique<Instruction>((uint32_t)(words.size() + 1), spv::OpName);
    new_inst->Fill(words);
    debug_name_.emplace_back(std::move(new_inst));
}

void Module::AddDecoration(uint32_t target_id, spv::Decoration decoration, const std::vector<uint32_t>& operands) {
    auto new_inst = std::make_unique<Instruction>((uint32_t)(operands.size() + 3), spv::OpDecorate);
    new_inst->Fill({target_id, (uint32_t)decoration});
    if (!operands.empty()) {
        new_inst->Fill(operands);
    }
    annotations_.emplace_back(std::move(new_inst));
}

void Module::AddMemberDecoration(uint32_t target_id, uint32_t index, spv::Decoration decoration,
                                 const std::vector<uint32_t>& operands) {
    auto new_inst = std::make_unique<Instruction>((uint32_t)(operands.size() + 4), spv::OpMemberDecorate);
    new_inst->Fill({target_id, index, (uint32_t)decoration});
    if (!operands.empty()) {
        new_inst->Fill(operands);
    }
    annotations_.emplace_back(std::move(new_inst));
}

uint32_t Module::TakeNextId() {
    // SPIR-V limit.
    assert(header_.bound < 0x3FFFFF);
    return header_.bound++;
}

// walk through each list and append the buffer
void Module::ToBinary(std::vector<uint32_t>& out) {
    out.clear();
    out.push_back(header_.magic_number);
    out.push_back(header_.version);
    out.push_back(header_.generator);
    out.push_back(header_.bound);
    out.push_back(header_.schema);

    for (const auto& inst : capabilities_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : extensions_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : ext_inst_imports_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : memory_model_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : entry_points_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : execution_modes_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : debug_source_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : debug_name_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : debug_module_processed_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : annotations_) {
        inst->ToBinary(out);
    }
    for (const auto& inst : types_values_constants_) {
        inst->ToBinary(out);
    }
    for (const auto& function : functions_) {
        function->ToBinary(out);
    }
}

// We need to apply variable to the Entry Point interface if using SPIR-V 1.4+ (or input/output)
void Module::AddInterfaceVariables(uint32_t id, spv::StorageClass storage_class) {
    const uint32_t spirv_version_1_4 = 0x00010400;
    if (header_.version >= spirv_version_1_4 || storage_class == spv::StorageClassInput ||
        storage_class == spv::StorageClassOutput) {
        // Currently just apply to all Entrypoint as it should be ok to have a global variable in there even if it can't dynamically
        // touch the new function
        for (auto& entry_point : entry_points_) {
            entry_point->AppendWord(id);
        }
    }
}

// Takes the current module and injects the function into it
// This is done by first apply any new Types/Constants/Variables and then copying in the instructions of the Function
void Module::LinkFunction(const LinkInfo& info) {
    // track the incoming SSA IDs with what they are in the module
    // < old_id, new_id >
    vvl::unordered_map<uint32_t, uint32_t> id_swap_map;
    uint32_t function_type_id = 0;

    // Track all decorations and add after when have full id_swap_map
    InstructionList decorations;

    // find all constant and types, add any the module doesn't have
    uint32_t offset = 5;  // skip header
    while (offset < info.word_count) {
        const uint32_t* inst_word = &info.words[offset];
        const uint32_t opcode = *inst_word & 0x0ffffu;
        const uint32_t length = *inst_word >> 16;
        if (opcode == spv::OpFunction) {
            break;
        }

        auto new_inst = std::make_unique<Instruction>(inst_word, kLinkedInstruction);
        uint32_t old_result_id = new_inst->ResultId();

        SpvType spv_type = GetSpvType(opcode);
        if (spv_type != SpvType::Empty) {
            // will find (or create if not found) the matching OpType
            uint32_t type_id = 0;
            switch (spv_type) {
                case SpvType::kVoid:
                    type_id = type_manager_.GetTypeVoid().Id();
                    break;
                case SpvType::kBool:
                    type_id = type_manager_.GetTypeBool().Id();
                    break;
                case SpvType::kSampler:
                    type_id = type_manager_.GetTypeSampler().Id();
                    break;
                case SpvType::kRayQueryKHR:
                    type_id = type_manager_.GetTypeRayQuery().Id();
                    break;
                case SpvType::kAccelerationStructureKHR:
                    type_id = type_manager_.GetTypeAccelerationStructure().Id();
                    break;
                case SpvType::kInt: {
                    uint32_t bit_width = new_inst->Word(2);
                    bool is_signed = new_inst->Word(3) != 0;
                    type_id = type_manager_.GetTypeInt(bit_width, is_signed).Id();
                    break;
                }
                case SpvType::kFloat: {
                    uint32_t bit_width = new_inst->Word(2);
                    type_id = type_manager_.GetTypeFloat(bit_width).Id();
                    break;
                }
                case SpvType::kArray: {
                    const Type* element_type = type_manager_.FindTypeById(id_swap_map[new_inst->Word(2)]);
                    const Constant* element_length = type_manager_.FindConstantById(id_swap_map[new_inst->Word(3)]);
                    type_id = type_manager_.GetTypeArray(*element_type, *element_length).Id();
                    break;
                }
                case SpvType::kRuntimeArray: {
                    const Type* element_type = type_manager_.FindTypeById(id_swap_map[new_inst->Word(2)]);
                    type_id = type_manager_.GetTypeRuntimeArray(*element_type).Id();
                    break;
                }
                case SpvType::kVector: {
                    const Type* component_type = type_manager_.FindTypeById(id_swap_map[new_inst->Word(2)]);
                    uint32_t component_count = new_inst->Word(3);
                    type_id = type_manager_.GetTypeVector(*component_type, component_count).Id();
                    break;
                }
                case SpvType::kMatrix: {
                    const Type* column_type = type_manager_.FindTypeById(id_swap_map[new_inst->Word(2)]);
                    uint32_t column_count = new_inst->Word(3);
                    type_id = type_manager_.GetTypeMatrix(*column_type, column_count).Id();
                    break;
                }
                case SpvType::kSampledImage: {
                    const Type* image_type = type_manager_.FindTypeById(id_swap_map[new_inst->Word(2)]);
                    type_id = type_manager_.GetTypeSampledImage(*image_type).Id();
                    break;
                }
                case SpvType::kPointer: {
                    auto it = id_swap_map.find(new_inst->ResultId());
                    if (it != id_swap_map.end()) {
                        // already had a OpTypeForwardPointer, so will automatically need a new a new OpTypePointer
                        type_id = it->second;  // id_swap_map will just update with same value
                        new_inst->ReplaceResultId(type_id);
                        new_inst->ReplaceLinkedId(id_swap_map);
                        type_manager_.AddType(std::move(new_inst), spv_type).Id();
                    } else {
                        spv::StorageClass storage_class = spv::StorageClass(new_inst->Word(2));
                        const Type* pointer_type = type_manager_.FindTypeById(id_swap_map[new_inst->Word(3)]);
                        type_id = type_manager_.GetTypePointer(storage_class, *pointer_type).Id();
                    }
                    break;
                }
                case SpvType::kForwardPointer: {
                    // forward reference id swap
                    type_id = TakeNextId();
                    old_result_id = new_inst->words_[1];
                    new_inst->words_[1] = type_id;
                    type_manager_.AddType(std::move(new_inst), spv_type);
                    break;
                }
                case SpvType::kStruct: {
                    // For OpTypeStruct, we just add it regardless since low chance to find for the amount of time to search all
                    // struct (which there can be quite a bit of)
                    type_id = TakeNextId();
                    new_inst->ReplaceResultId(type_id);
                    new_inst->ReplaceLinkedId(id_swap_map);
                    type_manager_.AddType(std::move(new_inst), spv_type).Id();
                    break;
                }
                case SpvType::kFunction: {
                    // It is not valid to have duplicate OpTypeFunction and some linked in functions will have the same signature
                    new_inst->ReplaceLinkedId(id_swap_map);
                    // First swap out IDs so comparison will be the same
                    const Type* function_type = type_manager_.FindFunctionType(*new_inst.get());
                    if (function_type) {
                        // Just reuse non-unique OpTypeFunction
                        function_type_id = function_type->Id();
                    } else {
                        function_type_id = TakeNextId();
                        type_id = function_type_id;
                        new_inst->ReplaceResultId(type_id);
                        type_manager_.AddType(std::move(new_inst), spv_type).Id();
                    }
                    break;
                }
                default:
                    break;
            }

            id_swap_map[old_result_id] = type_id;

        } else if (ConstantOperation(opcode)) {
            const Type& type = *type_manager_.FindTypeById(id_swap_map[new_inst->TypeId()]);
            const Constant* constant = nullptr;
            // for simplicity, just create a new constant for things other than 32-bit OpConstant as there are rarely-to-none
            // composite/null/true/false constants in linked functions. The extra logic to try and find them is much larger and cost
            // time failing most the searches.
            if (opcode == spv::OpConstant) {
                const uint32_t constant_value = new_inst->Word(3);
                if (type.inst_.Opcode() == spv::OpTypeInt && type.inst_.Word(2) == 32) {
                    constant = type_manager_.FindConstantInt32(type.Id(), constant_value);
                } else if (type.inst_.Opcode() == spv::OpTypeFloat && type.inst_.Word(2) == 32) {
                    constant = type_manager_.FindConstantFloat32(type.Id(), constant_value);
                }

                // Replace LinkConstants
                if (constant_value == glsl::kLinkShaderId) {
                    new_inst->words_[3] = shader_id_;
                }
            }

            if (!constant) {
                const uint32_t new_result_id = TakeNextId();
                new_inst->ReplaceResultId(new_result_id);
                new_inst->ReplaceLinkedId(id_swap_map);
                constant = &type_manager_.AddConstant(std::move(new_inst), type);
            }
            id_swap_map[old_result_id] = constant->Id();
        } else if (opcode == spv::OpVariable) {
            // Add in all variables outside of functions
            const uint32_t new_result_id = TakeNextId();
            AddInterfaceVariables(new_result_id, (spv::StorageClass)new_inst->Word(3));
            id_swap_map[old_result_id] = new_result_id;
            new_inst->ReplaceResultId(new_result_id);
            new_inst->ReplaceLinkedId(id_swap_map);

            const Type* type = type_manager_.FindTypeById(new_inst->TypeId());
            type_manager_.AddVariable(std::move(new_inst), *type);
        } else if (opcode == spv::OpDecorate || opcode == spv::OpMemberDecorate) {
            decorations.emplace_back(std::move(new_inst));
        } else if (opcode == spv::OpCapability) {
            spv::Capability capability = spv::Capability(new_inst->Word(1));
            // Shader is required and we want to remove Linkage from final shader
            if (capability != spv::CapabilityShader && capability != spv::CapabilityLinkage) {
                // It is valid to have duplicated Capabilities
                capabilities_.emplace_back(std::move(new_inst));
            }
        } else if (opcode == spv::OpExtInstImport) {
            const uint32_t new_result_id = TakeNextId();
            id_swap_map[old_result_id] = new_result_id;
            new_inst->ReplaceResultId(new_result_id);
            ext_inst_imports_.emplace_back(std::move(new_inst));
        } else if (opcode == spv::OpString) {
            const uint32_t new_result_id = TakeNextId();
            id_swap_map[old_result_id] = new_result_id;
            new_inst->ReplaceResultId(new_result_id);
            debug_source_.emplace_back(std::move(new_inst));
        } else if (opcode == spv::OpExtension) {
            extensions_.emplace_back(std::move(new_inst));
        }

        offset += length;
    }

    // because flow-control instructions (ex. OpBranch) do forward references to IDs, do an initial loop to get all OpLabel to have
    // in id_swap_map
    uint32_t offset_copy = offset;
    while (offset_copy < info.word_count) {
        const uint32_t* inst_word = &info.words[offset_copy];
        const uint32_t opcode = *inst_word & 0x0ffffu;
        const uint32_t length = *inst_word >> 16;
        if (opcode == spv::OpLabel) {
            Instruction inst(inst_word, kLinkedInstruction);
            uint32_t new_result_id = TakeNextId();
            id_swap_map[inst.ResultId()] = new_result_id;
        }
        offset_copy += length;
    }

    AddDebugName(info.opname, info.function_id);

    // Add function and copy all instructions to it, while adjusting any IDs
    auto& new_function = functions_.emplace_back(std::make_unique<Function>(*this));
    // We make things simpler by just putting everything in the first BasicBlock
    // (We need it in a block incase we want to alter this function later with something like DebugPrintf)
    BasicBlock* link_basic_block = nullptr;
    while (offset < info.word_count) {
        const uint32_t* inst_word = &info.words[offset];
        auto new_inst = std::make_unique<Instruction>(inst_word, kLinkedInstruction);
        const uint32_t opcode = new_inst->Opcode();
        const uint32_t length = new_inst->Length();

        if (opcode == spv::OpFunction) {
            new_inst->words_[1] = id_swap_map[new_inst->words_[1]];
            new_inst->words_[2] = info.function_id;
            // We can easily inject the same function hundreds of times and really don't want to inline it.
            // Have found that if drivers don't inline, can get a 20x speed-up at compiling large bloated shaders.
            // There is no way to query this or test if the driver does consume this, also currently most drivers
            // will ignore this as it is not hooked up... but worth trying
            new_inst->words_[3] = spv::FunctionControlDontInlineMask;
            new_inst->words_[4] = function_type_id;
        } else if (opcode == spv::OpLabel) {
            uint32_t new_result_id = id_swap_map[new_inst->ResultId()];
            new_inst->ReplaceResultId(new_result_id);

            // Only do on first label at top of function
            if (!link_basic_block) {
                auto new_block = std::make_unique<BasicBlock>(std::move(new_inst), *new_function);
                auto& added_block = new_function->blocks_.emplace_back(std::move(new_block));
                link_basic_block = &(*added_block);
                offset += length;
                continue;  // prevent adding a null new_inst below
            }
        } else {
            uint32_t result_id = new_inst->ResultId();
            if (result_id != 0) {
                uint32_t new_result_id = TakeNextId();
                id_swap_map[result_id] = new_result_id;
                new_inst->ReplaceResultId(new_result_id);
            }
            new_inst->ReplaceLinkedId(id_swap_map);
        }

        if (link_basic_block) {
            // Need for a possible FindInstruction() lookup
            const uint32_t result_id = new_inst->ResultId();
            if (result_id != 0) {
                new_function->inst_map_[result_id] = new_inst.get();
            }

            link_basic_block->instructions_.emplace_back(std::move(new_inst));
        } else {
            new_function->pre_block_inst_.emplace_back(std::move(new_inst));
        }

        offset += length;
    }

    // if 2 OpTypeRuntimeArray are combined, we can't have ArrayStride twice
    vvl::unordered_set<uint32_t> array_strides;
    for (const auto& annotation : annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(2) == spv::DecorationArrayStride) {
            array_strides.insert(annotation->Word(1));
        }
    }

    for (auto& decoration : decorations) {
        if (decoration->Word(2) == spv::DecorationLinkageAttributes) {
            continue;  // remove linkage info
        } else if (decoration->Word(2) == spv::DecorationDescriptorSet) {
            // only should be one DescriptorSet to update
            decoration->words_[3] = output_buffer_descriptor_set_;
        }

        decoration->ReplaceLinkedId(id_swap_map);

        if (decoration->Word(2) == spv::DecorationArrayStride) {
            if (!array_strides.insert(decoration->Word(1)).second) {
                continue;
            }
        }

        annotations_.emplace_back(std::move(decoration));
    }
}

// Things that need to be done once if there is any instrumentation.
void Module::PostProcess() {
    if (use_bda_) {
        // Adjust the original addressing model to be PhysicalStorageBuffer64 if not already.
        // A module can only have one OpMemoryModel
        memory_model_[0]->words_[1] = spv::AddressingModelPhysicalStorageBuffer64;
        if (!HasCapability(spv::CapabilityPhysicalStorageBufferAddresses)) {
            AddCapability(spv::CapabilityPhysicalStorageBufferAddresses);
            AddExtension("SPV_KHR_physical_storage_buffer");
        }
    }

    // The instrumentation code has atomicAdd() to update the output buffer
    // If the incoming code only has VulkanMemoryModel it will need to support device scope
    //
    // Found that QueueFamily was added to mostly solve this, if a device doesn't support Device scope we could use QueueFamily, the
    // issue is that the GLSL we have is static and if we use QueueFamily then we "need" the MemoryModel enabled
    if (HasCapability(spv::CapabilityVulkanMemoryModel)) {
        if (!enabled_features_.vulkanMemoryModelDeviceScope) {
            InternalError(
                "GPU-SHADER-INSTRUMENT-SUPPORT",
                "vulkanMemoryModelDeviceScope feature is not supported, but need to let us call atomicAdd to the output buffer");
        }
        AddCapability(spv::CapabilityVulkanMemoryModelDeviceScope);
    }

    // Vulkan 1.1 is required, so if incoming SPIR-V is 1.0, might need to adjust it
    const uint32_t spirv_version_1_0 = 0x00010000;
    if (header_.version == spirv_version_1_0) {
        // SPV_KHR_storage_buffer_storage_class is needed, but glslang removes it from linking functions
        AddExtension("SPV_KHR_storage_buffer_storage_class");
    }
}

void Module::InternalWarning(const char* tag, const char* message) {
    if (debug_report_) {
        debug_report_->DebugLogMsg(kWarningBit, {}, message, tag);
    } else {
        std::cout << "[" << tag << "] " << message << '\n';
    }
}

void Module::InternalError(const char* tag, const char* message) {
    if (debug_report_) {
        debug_report_->DebugLogMsg(kErrorBit, {}, message, tag);
    } else {
        std::cerr << "[" << tag << "] " << message << '\n';
    }
}

}  // namespace spirv
}  // namespace gpuav