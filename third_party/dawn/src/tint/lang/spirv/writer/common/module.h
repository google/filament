// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_SPIRV_WRITER_COMMON_MODULE_H_
#define SRC_TINT_LANG_SPIRV_WRITER_COMMON_MODULE_H_

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "src/tint/lang/spirv/writer/common/function.h"
#include "src/tint/lang/spirv/writer/common/instruction.h"
#include "src/tint/utils/containers/hashset.h"

namespace tint::spirv::writer {

/// A SPIR-V module.
class Module {
  public:
    /// Constructor
    Module();

    /// Copy constructor
    /// @param other the other Module to copy
    Module(const Module& other);

    /// Move constructor
    /// @param other the other Module to move
    Module(Module&& other);

    /// Destructor
    ~Module();

    /// Copy-assignment operator
    /// @param other the other Module to copy
    /// @returns this Module
    Module& operator=(const Module& other);

    /// Move-assignment operator
    /// @param other the other Module to move
    /// @returns this Module
    Module& operator=(Module&& other);

    /// @returns the number of uint32_t's needed to make up the results
    uint32_t TotalSize() const;

    /// @returns the id bound for this program
    uint32_t IdBound() const { return next_id_; }

    /// @returns the next id to be used
    uint32_t NextId() {
        auto id = next_id_;
        next_id_ += 1;
        return id;
    }

    /// Iterates over all the instructions in the correct order and calls the given callback.
    /// @param cb the callback to execute
    void Iterate(std::function<void(const Instruction&)> cb) const;

    /// Add an instruction to the list of capabilities, if the capability hasn't already been added.
    /// @param cap the capability to set
    void PushCapability(uint32_t cap);

    /// @returns the capabilities
    const InstructionList& Capabilities() const { return capabilities_; }

    /// Add an instruction to the list of extensions.
    /// @param extension the name of the extension
    void PushExtension(const char* extension);

    /// @returns the extensions
    const InstructionList& Extensions() const { return extensions_; }

    /// Add an instruction to the list of imported extension instructions.
    /// @param op the op to set
    /// @param operands the operands for the instruction
    void PushExtImport(spv::Op op, const OperandList& operands) {
        ext_imports_.push_back(Instruction{op, operands});
    }

    /// @returns the ext imports
    const InstructionList& ExtImports() const { return ext_imports_; }

    /// Add an instruction to the memory model.
    /// @param op the op to set
    /// @param operands the operands for the instruction
    void PushMemoryModel(spv::Op op, const OperandList& operands) {
        memory_model_.push_back(Instruction{op, operands});
    }

    /// @returns the memory model
    const InstructionList& MemoryModel() const { return memory_model_; }

    /// Add an instruction to the list pf entry points.
    /// @param op the op to set
    /// @param operands the operands for the instruction
    void PushEntryPoint(spv::Op op, const OperandList& operands) {
        entry_points_.push_back(Instruction{op, operands});
    }
    /// @returns the entry points
    const InstructionList& EntryPoints() const { return entry_points_; }

    /// Add an instruction to the execution mode declarations.
    /// @param op the op to set
    /// @param operands the operands for the instruction
    void PushExecutionMode(spv::Op op, const OperandList& operands) {
        execution_modes_.push_back(Instruction{op, operands});
    }

    /// @returns the execution modes
    const InstructionList& ExecutionModes() const { return execution_modes_; }

    /// Add an instruction to the debug declarations.
    /// @param op the op to set
    /// @param operands the operands for the instruction
    void PushDebug(spv::Op op, const OperandList& operands) {
        debug_.push_back(Instruction{op, operands});
    }

    /// @returns the debug instructions
    const InstructionList& Debug() const { return debug_; }

    /// Add an instruction to the type declarations.
    /// @param op the op to set
    /// @param operands the operands for the instruction
    void PushType(spv::Op op, const OperandList& operands) {
        types_.push_back(Instruction{op, operands});
    }

    /// @returns the type instructions
    const InstructionList& Types() const { return types_; }

    /// Add an instruction to the annotations.
    /// @param op the op to set
    /// @param operands the operands for the instruction
    void PushAnnot(spv::Op op, const OperandList& operands) {
        annotations_.push_back(Instruction{op, operands});
    }

    /// @returns the annotations
    const InstructionList& Annots() const { return annotations_; }

    /// Add a function to the module.
    /// @param func the function to add
    void PushFunction(const Function& func) { functions_.push_back(func); }

    /// @returns the functions
    const std::vector<Function>& Functions() const { return functions_; }

    /// @returns the SPIR-V code as a vector of uint32_t
    std::vector<uint32_t>& Code() { return code_; }

  private:
    uint32_t next_id_ = 1;
    InstructionList capabilities_;
    InstructionList extensions_;
    InstructionList ext_imports_;
    InstructionList memory_model_;
    InstructionList entry_points_;
    InstructionList execution_modes_;
    InstructionList debug_;
    InstructionList types_;
    InstructionList annotations_;
    std::vector<Function> functions_;
    Hashset<uint32_t, 8> capability_set_;
    Hashset<std::string, 8> extension_set_;
    std::vector<uint32_t> code_;
};

}  // namespace tint::spirv::writer

#endif  // SRC_TINT_LANG_SPIRV_WRITER_COMMON_MODULE_H_
