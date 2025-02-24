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
#pragma once

#include <vector>
#include "instruction.h"
#include "generated/spirv_grammar_helper.h"

namespace gpuav {
namespace spirv {

class Module;
class TypeManager;

// These are the constant operations that we plan to handle in for shader instrumentation
static constexpr bool ConstantOperation(uint32_t opcode) {
    switch (opcode) {
        case spv::OpConstant:
        case spv::OpConstantTrue:
        case spv::OpConstantFalse:
        case spv::OpConstantComposite:
        case spv::OpConstantNull:
            return true;
        // Using a spec constant is bad as we might alias the value and have it change on use at pipeline creation time
        case spv::OpSpecConstant:
        case spv::OpSpecConstantTrue:
        case spv::OpSpecConstantFalse:
        case spv::OpSpecConstantComposite:
        case spv::OpSpecConstantOp:  // always must be in function block
        default:
            return false;
    }
}

// There is a LOT that can be done with types, but for simplicity it only does what is needed.
// The main thing is to try find the type so we don't add a duplicate (but not end of the world if 1 or 2 are duplicated as a
// trade-off to doing complex logic to resolve more complex types). The class also takes advantage that while Instrumenting we are
// always aware of our types we are adding or just explictly found.
struct Type {
    Type(SpvType spv_type, const Instruction& inst) : spv_type_(spv_type), inst_(inst) {}

    bool operator==(Type const& other) const;
    uint32_t Id() const { return inst_.ResultId(); }
    bool IsArray() const { return spv_type_ == SpvType::kArray || spv_type_ == SpvType::kRuntimeArray; }

    const SpvType spv_type_;
    const Instruction& inst_;
};

static bool IsSpecConstant(uint32_t opcode) {
    return opcode == spv::OpSpecConstant || opcode == spv::OpSpecConstantTrue || opcode == spv::OpSpecConstantFalse ||
           opcode == spv::OpSpecConstantComposite || opcode == spv::OpSpecConstantOp;
}

// Represents a OpConstant* or OpSpecConstant*
// (Currently doesn't handle OpSpecConstantComposite or OpSpecConstantOp)
struct Constant {
    Constant(const Type& type, const Instruction& inst)
        : type_(type), inst_(inst), is_spec_constant_(IsSpecConstant(inst.Opcode())) {}

    uint32_t Id() const { return inst_.ResultId(); }

    const Type& type_;
    const Instruction& inst_;
    // Most times we just need Constant to get type or id, so being a spec const doesn't matter.
    // This boolean is here incase we do care about the value of the constant.
    const bool is_spec_constant_;
};

// Represents a global OpVariable found before the first function
struct Variable {
    Variable(const Type& type, const Instruction& inst) : type_(type), inst_(inst) {}

    uint32_t Id() const { return inst_.ResultId(); }
    spv::StorageClass StorageClass() const { return spv::StorageClass(inst_.Word(3)); }
    const Type* PointerType(TypeManager& type_manager_) const;

    const Type& type_;
    const Instruction& inst_;
};

// In charge of tracking all Types, Constants, and Variable in the module.
// Since both Variable and Constant both rely on Types, the Types are the core thing we track
//
// Function naming guide:
//      Find*() - searches if type/constant/var is already there, will return null if not
//       Get*() - searches if type/constant/var is already there, will create one if not
//    Create*() - just makes the type/constant/var, doesn't attempt to search for a duplicate
class TypeManager {
  public:
    TypeManager(Module& module) : module_(module) {}

    const Type& AddType(std::unique_ptr<Instruction> new_inst, SpvType spv_type);
    const Type* FindTypeById(uint32_t id) const;
    const Type* FindValueTypeById(uint32_t id) const;
    const Type* FindFunctionType(const Instruction& inst) const;
    // There shouldn't be a case where we need to query for a specific type, but then not add it if not found.
    const Type& GetTypeVoid();
    const Type& GetTypeBool();
    const Type& GetTypeSampler();
    const Type& GetTypeRayQuery();
    const Type& GetTypeAccelerationStructure();
    const Type& GetTypeInt(uint32_t bit_width, bool is_signed);
    const Type& GetTypeFloat(uint32_t bit_width);
    const Type& GetTypeArray(const Type& element_type, const Constant& length);
    const Type& GetTypeRuntimeArray(const Type& element_type);
    const Type& GetTypeVector(const Type& component_type, uint32_t component_count);
    const Type& GetTypeMatrix(const Type& column_type, uint32_t column_count);
    const Type& GetTypeSampledImage(const Type& image_type);
    const Type& GetTypePointer(spv::StorageClass storage_class, const Type& pointer_type);
    const Type& GetTypePointerBuiltInInput(spv::BuiltIn built_in);
    uint32_t TypeLength(const Type& type);

    const Constant& AddConstant(std::unique_ptr<Instruction> new_inst, const Type& type);
    const Constant* FindConstantById(uint32_t id) const;
    const Constant* FindConstantInt32(uint32_t type_id, uint32_t value) const;
    const Constant* FindConstantFloat32(uint32_t type_id, uint32_t value) const;
    // most constants are uint
    const Constant& CreateConstantUInt32(uint32_t value);
    const Constant& GetConstantUInt32(uint32_t value);
    const Constant& GetConstantZeroUint32();
    const Constant& GetConstantZeroFloat32();
    const Constant& GetConstantZeroVec3();
    const Constant& GetConstantNull(const Type& type);

    const Variable& AddVariable(std::unique_ptr<Instruction> new_inst, const Type& type);
    const Variable* FindVariableById(uint32_t id) const;

  private:
    Module& module_;

    // Currently we don't worry about duplicated types. If duplicate types are added from the original SPIR-V, we just use the first
    // one we fine. We should only be adding a new object because it currently doesn't exists.
    vvl::unordered_map<uint32_t, std::unique_ptr<Type>> id_to_type_;
    vvl::unordered_map<uint32_t, std::unique_ptr<Constant>> id_to_constant_;
    vvl::unordered_map<uint32_t, std::unique_ptr<Variable>> id_to_variable_;

    // Create faster lookups for specific types
    // some types are base types and only will be one
    const Type* void_type = nullptr;
    const Type* bool_type = nullptr;
    const Type* sampler_type = nullptr;
    const Type* ray_query_type = nullptr;
    const Type* acceleration_structure_type = nullptr;
    std::vector<const Type*> int_types_;
    std::vector<const Type*> float_types_;
    std::vector<const Type*> vector_types_;
    std::vector<const Type*> matrix_types_;
    std::vector<const Type*> image_types_;
    std::vector<const Type*> sampled_image_types_;
    std::vector<const Type*> array_types_;
    std::vector<const Type*> runtime_array_types_;
    std::vector<const Type*> pointer_types_;
    std::vector<const Type*> forward_pointer_types_;
    std::vector<const Type*> function_types_;

    std::vector<const Constant*> int_32bit_constants_;
    std::vector<const Constant*> float_32bit_constants_;
    const Constant* uint_32bit_zero_constants_ = nullptr;
    const Constant* float_32bit_zero_constants_ = nullptr;
    std::vector<const Constant*> null_constants_;

    std::vector<const Variable*> input_variables_;
    std::vector<const Variable*> output_variables_;
};

}  // namespace spirv
}  // namespace gpuav
