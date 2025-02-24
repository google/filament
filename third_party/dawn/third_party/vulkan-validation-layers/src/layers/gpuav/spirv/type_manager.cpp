/* Copyright (c) 2024 LunarG, Inc.
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

#include "type_manager.h"
#include "generated/spirv_grammar_helper.h"
#include "module.h"

namespace gpuav {
namespace spirv {

// Simplest way to check if same type is see if items line up.
// Even if types have an RefId, it should be the same unless there are already duplicated types.
bool Type::operator==(Type const& other) const {
    if ((spv_type_ != other.spv_type_) || (inst_.Length() != other.inst_.Length())) {
        return false;
    }
    // word[1] is the result ID which might be different
    for (uint32_t i = 2; i < inst_.Length(); i++) {
        if (inst_.words_[i] != other.inst_.words_[i]) {
            return false;
        }
    }
    return true;
}

// return %A in:
//   %B = OpTypePointer Input %A
//   %C = OpVariable %B Input
const Type* Variable::PointerType(TypeManager& type_manager_) const {
    assert(type_.spv_type_ == SpvType::kPointer || type_.spv_type_ == SpvType::kForwardPointer);
    uint32_t type_id = type_.inst_.Word(3);
    return type_manager_.FindTypeById(type_id);
}

const Type& TypeManager::AddType(std::unique_ptr<Instruction> new_inst, SpvType spv_type) {
    const auto& inst = module_.types_values_constants_.emplace_back(std::move(new_inst));

    id_to_type_[inst->ResultId()] = std::make_unique<Type>(spv_type, *inst);
    const Type* new_type = id_to_type_[inst->ResultId()].get();

    switch (spv_type) {
        case SpvType::kVoid:
            void_type = new_type;
            break;
        case SpvType::kBool:
            bool_type = new_type;
            break;
        case SpvType::kSampler:
            sampler_type = new_type;
            break;
        case SpvType::kRayQueryKHR:
            ray_query_type = new_type;
            break;
        case SpvType::kAccelerationStructureKHR:
            acceleration_structure_type = new_type;
            break;
        case SpvType::kInt:
            int_types_.push_back(new_type);
            break;
        case SpvType::kFloat:
            float_types_.push_back(new_type);
            break;
        case SpvType::kVector:
            vector_types_.push_back(new_type);
            break;
        case SpvType::kMatrix:
            matrix_types_.push_back(new_type);
            break;
        case SpvType::kImage:
            image_types_.push_back(new_type);
            break;
        case SpvType::kSampledImage:
            sampled_image_types_.push_back(new_type);
            break;
        case SpvType::kArray:
            array_types_.push_back(new_type);
            break;
        case SpvType::kRuntimeArray:
            runtime_array_types_.push_back(new_type);
            break;
        case SpvType::kPointer:
            pointer_types_.push_back(new_type);
            break;
        case SpvType::kForwardPointer:
            forward_pointer_types_.push_back(new_type);
            break;
        case SpvType::kFunction:
            function_types_.push_back(new_type);
            break;
        case SpvType::kStruct:
            break;  // don't track structs currently
        case SpvType::kCooperativeVectorNV:
            break;  // don't track coopvec currently
        default:
            assert(false && "unsupported SpvType");
            break;
    }

    return *new_type;
}

const Type* TypeManager::FindTypeById(uint32_t id) const {
    auto type = id_to_type_.find(id);
    return (type == id_to_type_.end()) ? nullptr : type->second.get();
}

// It is common to have things like
//
// %uint = OpTypeInt 32 0
// %ptr_uint = OpTypePointer StorageBuffer %uint
// %ac = OpAccessChain %ptr_uint %var %int_1
//
// Where you have %ptr_uint and want to know it is OpTypeInt
// This function is like FindTypeById() but it will bypass the OpTypePointer for you (if it is there)
// There is also a matching Variable::PointerType()
const Type* TypeManager::FindValueTypeById(uint32_t id) const {
    const Type* pointer_type = FindTypeById(id);
    if (!pointer_type) {
        return nullptr;
    } else if (pointer_type->spv_type_ != SpvType::kPointer && pointer_type->spv_type_ != SpvType::kForwardPointer) {
        return pointer_type;
    } else {
        return FindTypeById(pointer_type->inst_.Word(3));
    }
}

const Type* TypeManager::FindFunctionType(const Instruction& inst) const {
    const uint32_t inst_length = inst.Length();
    for (const auto& type : function_types_) {
        if (type->inst_.Length() != inst_length) {
            continue;
        }
        // Start at the Result Type ID (skip ResultID and the base word)
        bool found = true;
        for (uint32_t i = 2; i < inst_length; i++) {
            if (type->inst_.Word(i) != inst.Word(i)) {
                found = false;
                break;
            }
        }
        if (found) {
            return type;
        }
    }
    return nullptr;
}

const Type& TypeManager::GetTypeVoid() {
    if (void_type) {
        return *void_type;
    };

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(2, spv::OpTypeVoid);
    new_inst->Fill({type_id});
    return AddType(std::move(new_inst), SpvType::kVoid);
}

const Type& TypeManager::GetTypeBool() {
    if (bool_type) {
        return *bool_type;
    };

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(2, spv::OpTypeBool);
    new_inst->Fill({type_id});
    return AddType(std::move(new_inst), SpvType::kBool);
}

const Type& TypeManager::GetTypeSampler() {
    if (sampler_type) {
        return *sampler_type;
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(2, spv::OpTypeSampler);
    new_inst->Fill({type_id});
    return AddType(std::move(new_inst), SpvType::kSampler);
}

const Type& TypeManager::GetTypeRayQuery() {
    if (ray_query_type) {
        return *ray_query_type;
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(2, spv::OpTypeRayQueryKHR);
    new_inst->Fill({type_id});
    return AddType(std::move(new_inst), SpvType::kRayQueryKHR);
}

const Type& TypeManager::GetTypeAccelerationStructure() {
    if (acceleration_structure_type) {
        return *acceleration_structure_type;
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(2, spv::OpTypeAccelerationStructureKHR);
    new_inst->Fill({type_id});
    return AddType(std::move(new_inst), SpvType::kAccelerationStructureKHR);
}

const Type& TypeManager::GetTypeInt(uint32_t bit_width, bool is_signed) {
    for (const auto type : int_types_) {
        const auto& words = type->inst_.words_;
        const bool int_is_signed = words[3] != 0;
        if (words[2] == bit_width && int_is_signed == is_signed) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    const uint32_t signed_word = is_signed ? 1 : 0;
    auto new_inst = std::make_unique<Instruction>(4, spv::OpTypeInt);
    new_inst->Fill({type_id, bit_width, signed_word});
    return AddType(std::move(new_inst), SpvType::kInt);
}

const Type& TypeManager::GetTypeFloat(uint32_t bit_width) {
    for (const auto type : float_types_) {
        const auto& words = type->inst_.words_;
        if ((words[2] == bit_width)) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(3, spv::OpTypeFloat);
    new_inst->Fill({type_id, bit_width});
    return AddType(std::move(new_inst), SpvType::kFloat);
}

const Type& TypeManager::GetTypeArray(const Type& element_type, const Constant& length) {
    for (const auto type : array_types_) {
        const Type* this_element_type = FindTypeById(type->inst_.Word(2));
        if (this_element_type && (*this_element_type == element_type)) {
            if (type->inst_.Word(3) == length.Id()) {
                return *type;
            }
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(4, spv::OpTypeArray);
    new_inst->Fill({type_id, element_type.Id(), length.Id()});
    return AddType(std::move(new_inst), SpvType::kArray);
}

const Type& TypeManager::GetTypeRuntimeArray(const Type& element_type) {
    for (const auto type : runtime_array_types_) {
        const Type* this_element_type = FindTypeById(type->inst_.Word(2));
        if (this_element_type && (*this_element_type == element_type)) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(3, spv::OpTypeRuntimeArray);
    new_inst->Fill({type_id, element_type.Id()});
    return AddType(std::move(new_inst), SpvType::kRuntimeArray);
}

const Type& TypeManager::GetTypeVector(const Type& component_type, uint32_t component_count) {
    for (const auto type : vector_types_) {
        const auto& words = type->inst_.words_;
        if (words[3] != component_count) {
            continue;
        }

        const Type* vector_component_type = FindTypeById(words[2]);
        if (vector_component_type && (*vector_component_type == component_type)) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(4, spv::OpTypeVector);
    new_inst->Fill({type_id, component_type.Id(), component_count});
    return AddType(std::move(new_inst), SpvType::kVector);
}

const Type& TypeManager::GetTypeMatrix(const Type& column_type, uint32_t column_count) {
    for (const auto type : matrix_types_) {
        const auto& words = type->inst_.words_;
        if (words[3] != column_count) {
            continue;
        }

        const Type* matrix_column_type = FindTypeById(words[2]);
        if (matrix_column_type && (*matrix_column_type == column_type)) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(4, spv::OpTypeMatrix);
    new_inst->Fill({type_id, column_type.Id(), column_count});
    return AddType(std::move(new_inst), SpvType::kMatrix);
}

const Type& TypeManager::GetTypeSampledImage(const Type& image_type) {
    for (const auto type : sampled_image_types_) {
        const auto& words = type->inst_.words_;
        const Type* this_image_type = FindTypeById(words[2]);
        if (this_image_type && (*this_image_type == image_type)) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(3, spv::OpTypeSampledImage);
    new_inst->Fill({type_id, image_type.Id()});
    return AddType(std::move(new_inst), SpvType::kSampledImage);
}

const Type& TypeManager::GetTypePointer(spv::StorageClass storage_class, const Type& pointer_type) {
    for (const auto type : pointer_types_) {
        const auto& words = type->inst_.words_;
        if (words[2] != storage_class) {
            continue;
        }

        const Type* this_pointer_type = FindTypeById(words[3]);
        if (this_pointer_type && (*this_pointer_type == pointer_type)) {
            return *type;
        }
    }

    const uint32_t type_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(4, spv::OpTypePointer);
    new_inst->Fill({type_id, uint32_t(storage_class), pointer_type.Id()});
    return AddType(std::move(new_inst), SpvType::kPointer);
}

const Type& TypeManager::GetTypePointerBuiltInInput(spv::BuiltIn built_in) {
    switch (built_in) {
        case spv::BuiltInFragCoord: {
            const Type& float_32 = GetTypeFloat(32);
            const Type& vec4 = GetTypeVector(float_32, 4);
            return GetTypePointer(spv::StorageClassInput, vec4);
        }
        case spv::BuiltInVertexIndex:
        case spv::BuiltInInstanceIndex:
        case spv::BuiltInPrimitiveId:
        case spv::BuiltInInvocationId:
        case spv::BuiltInSubgroupLocalInvocationId: {
            const Type& uint_32 = GetTypeInt(32, false);
            return GetTypePointer(spv::StorageClassInput, uint_32);
        }
        case spv::BuiltInGlobalInvocationId:
        case spv::BuiltInLaunchIdKHR: {
            const Type& uint_32 = GetTypeInt(32, false);
            const Type& vec3 = GetTypeVector(uint_32, 3);
            return GetTypePointer(spv::StorageClassInput, vec3);
        }
        case spv::BuiltInTessCoord: {
            const Type& float_32 = GetTypeFloat(32);
            const Type& vec3 = GetTypeVector(float_32, 3);
            return GetTypePointer(spv::StorageClassInput, vec3);
        }
        case spv::BuiltInSubgroupLtMask: {
            const Type& uint_32 = GetTypeInt(32, false);
            const Type& vec4 = GetTypeVector(uint_32, 4);
            return GetTypePointer(spv::StorageClassInput, vec4);
        }
        default: {
            assert(false && "unhandled builtin");
            return *(id_to_type_.begin()->second);
        }
    }
}

uint32_t TypeManager::TypeLength(const Type& type) {
    switch (type.inst_.Opcode()) {
        case spv::OpTypeFloat:
        case spv::OpTypeInt:
            return type.inst_.Operand(0) / 8u;
        case spv::OpTypeVector:
        case spv::OpTypeMatrix: {
            const Type* count = FindTypeById(type.inst_.Operand(0));
            return type.inst_.Operand(1) * TypeLength(*count);
        }
        case spv::OpTypePointer:
            assert(type.inst_.Operand(0) == spv::StorageClassPhysicalStorageBuffer && "unexpected pointer type");
            // always will be PhysicalStorageBuffer64 addressing model
            return 8u;
        case spv::OpTypeArray: {
            const Type* element_type = FindTypeById(type.inst_.Operand(0));
            const Constant* count = FindConstantById(type.inst_.Operand(1));
            // TODO - Need to handle spec constant here, for now return zero to have things not blowup
            assert(count && !count->is_spec_constant_);
            const uint32_t array_length = (count && !count->is_spec_constant_) ? count->inst_.Operand(0) : 0;
            return array_length * TypeLength(*element_type);
        }
        case spv::OpTypeStruct: {
            // Get the offset of the last member and then figure out it's size
            // Note: the largest offset doesn't have to be the last element index of the struct
            uint32_t last_offset = 0;
            uint32_t last_offset_index = 0;
            const uint32_t struct_id = type.inst_.ResultId();
            for (const auto& annotation : module_.annotations_) {
                if (annotation->Opcode() == spv::OpMemberDecorate && annotation->Word(1) == struct_id &&
                    annotation->Word(3) == spv::DecorationOffset) {
                    const uint32_t index = annotation->Word(2);
                    const uint32_t offset = annotation->Word(4);
                    if (offset > last_offset) {
                        last_offset = offset;
                        last_offset_index = index;
                    }
                }
            }

            const Type* last_element_type = FindTypeById(type.inst_.Operand(last_offset_index));
            const uint32_t last_length = TypeLength(*last_element_type);
            return last_offset + last_length;
        }
        case spv::OpTypeRuntimeArray:
            assert(false && "unsupported type");
            break;
        default:
            assert(false && "unexpected type");
            break;
    }
    return 0;
}

const Constant& TypeManager::AddConstant(std::unique_ptr<Instruction> new_inst, const Type& type) {
    const auto& inst = module_.types_values_constants_.emplace_back(std::move(new_inst));

    id_to_constant_[inst->ResultId()] = std::make_unique<Constant>(type, *inst);
    const Constant* new_constant = id_to_constant_[inst->ResultId()].get();

    if (inst->Opcode() == spv::OpConstant) {
        if (type.inst_.Opcode() == spv::OpTypeInt && type.inst_.Word(2) == 32) {
            int_32bit_constants_.push_back(new_constant);
        } else if (type.inst_.Opcode() == spv::OpTypeFloat && type.inst_.Word(2) == 32) {
            float_32bit_constants_.push_back(new_constant);
        }
    } else if (inst->Opcode() == spv::OpConstantNull) {
        null_constants_.push_back(new_constant);
    }

    return *new_constant;
}

const Constant* TypeManager::FindConstantInt32(uint32_t type_id, uint32_t value) const {
    for (const auto constant : int_32bit_constants_) {
        if (constant->type_.Id() == type_id && value == constant->inst_.Word(3)) {
            return constant;
        }
    }
    return nullptr;
}

const Constant* TypeManager::FindConstantFloat32(uint32_t type_id, uint32_t value) const {
    for (const auto constant : float_32bit_constants_) {
        if (constant->type_.Id() == type_id && value == constant->inst_.Word(3)) {
            return constant;
        }
    }
    return nullptr;
}

const Constant* TypeManager::FindConstantById(uint32_t id) const {
    auto constant = id_to_constant_.find(id);
    return (constant == id_to_constant_.end()) ? nullptr : constant->second.get();
}

const Constant& TypeManager::CreateConstantUInt32(uint32_t value) {
    const Type& type = GetTypeInt(32, 0);
    const uint32_t constant_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(4, spv::OpConstant);
    new_inst->Fill({type.Id(), constant_id, value});
    return AddConstant(std::move(new_inst), type);
}

const Constant& TypeManager::GetConstantUInt32(uint32_t value) {
    if (value == 0) {
        return GetConstantZeroUint32();
    }

    const Type& uint32_type = module_.type_manager_.GetTypeInt(32, 0);
    const Constant* constant = module_.type_manager_.FindConstantInt32(uint32_type.Id(), value);
    if (!constant) {
        constant = &module_.type_manager_.CreateConstantUInt32(value);
    }
    return *constant;
}

// It is common to use uint32_t(0) as a default, so having it cached is helpful
const Constant& TypeManager::GetConstantZeroUint32() {
    if (!uint_32bit_zero_constants_) {
        const Type& uint_32_type = GetTypeInt(32, 0);
        uint_32bit_zero_constants_ = FindConstantInt32(uint_32_type.Id(), 0);
        if (!uint_32bit_zero_constants_) {
            uint_32bit_zero_constants_ = &CreateConstantUInt32(0);
        }
    }
    return *uint_32bit_zero_constants_;
}

// It is common to use float(0) as a default, so having it cached is helpful
const Constant& TypeManager::GetConstantZeroFloat32() {
    if (!float_32bit_zero_constants_) {
        const Type& float_32_type = GetTypeFloat(32);
        float_32bit_zero_constants_ = FindConstantFloat32(float_32_type.Id(), 0);
        if (!float_32bit_zero_constants_) {
            const uint32_t constant_id = module_.TakeNextId();
            auto new_inst = std::make_unique<Instruction>(4, spv::OpConstant);
            new_inst->Fill({float_32_type.Id(), constant_id, 0});
            float_32bit_zero_constants_ = &AddConstant(std::move(new_inst), float_32_type);
        }
    }
    return *float_32bit_zero_constants_;
}

const Constant& TypeManager::GetConstantZeroVec3() {
    const Type& float_32_type = GetTypeFloat(32);
    const Type& vec3_type = GetTypeVector(float_32_type, 3);
    const uint32_t float32_0_id = module_.type_manager_.GetConstantZeroFloat32().Id();

    const uint32_t constant_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(6, spv::OpConstantComposite);
    new_inst->Fill({vec3_type.Id(), constant_id, float32_0_id, float32_0_id, float32_0_id});
    return AddConstant(std::move(new_inst), vec3_type);
}

const Constant& TypeManager::GetConstantNull(const Type& type) {
    for (const auto& constant : null_constants_) {
        if (constant->type_.Id() == type.Id()) {
            return *constant;
        }
    }

    const uint32_t constant_id = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(3, spv::OpConstantNull);
    new_inst->Fill({type.Id(), constant_id});
    return AddConstant(std::move(new_inst), type);
}

const Variable& TypeManager::AddVariable(std::unique_ptr<Instruction> new_inst, const Type& type) {
    const auto& inst = module_.types_values_constants_.emplace_back(std::move(new_inst));

    id_to_variable_[inst->ResultId()] = std::make_unique<Variable>(type, *inst);
    const Variable* new_variable = id_to_variable_[inst->ResultId()].get();

    if (new_variable->StorageClass() == spv::StorageClassInput) {
        input_variables_.push_back(new_variable);
    } else if (new_variable->StorageClass() == spv::StorageClassOutput) {
        output_variables_.push_back(new_variable);
    }

    return *new_variable;
}

const Variable* TypeManager::FindVariableById(uint32_t id) const {
    auto variable = id_to_variable_.find(id);
    return (variable == id_to_variable_.end()) ? nullptr : variable->second.get();
}

}  // namespace spirv
}  // namespace gpuav