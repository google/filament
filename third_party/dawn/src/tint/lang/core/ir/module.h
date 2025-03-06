// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_MODULE_H_
#define SRC_TINT_LANG_CORE_IR_MODULE_H_

#include <utility>

#include "src/tint/lang/core/constant/manager.h"
#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/utils/containers/const_propagating_ptr.h"
#include "src/tint/utils/containers/filtered_iterator.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/diagnostic/source.h"
#include "src/tint/utils/generation_id.h"
#include "src/tint/utils/memory/block_allocator.h"
#include "src/tint/utils/symbol/symbol_table.h"

namespace tint::core::ir {

/// Main module class for the IR.
class Module {
    /// Program Id required to create other components
    GenerationID prog_id_;

    /// Map of value to name
    Hashmap<const Value*, Symbol, 32> value_to_name_;

    // The source information for a value
    Hashmap<const Value*, Source, 32> value_to_source_;

    /// A predicate function that returns true if the instruction or value is alive.
    struct IsAlive {
        bool operator()(const Instruction* instruction) const { return instruction->Alive(); }
        bool operator()(const Value* value) const { return value->Alive(); }
    };

  public:
    /// Constructor
    Module();
    /// Move constructor
    /// @param o the module to move from
    Module(Module&& o);
    /// Destructor
    ~Module();

    /// Move assign
    /// @param o the module to assign from
    /// @returns a reference to this module
    Module& operator=(Module&& o);

    /// Creates a new `TYPE` instruction owned by the module
    /// When the Module is destructed the object will be destructed and freed.
    /// @param args the arguments to pass to the constructor
    /// @returns the pointer to the instruction
    template <typename TYPE, typename... ARGS>
    TYPE* CreateInstruction(ARGS&&... args) {
        return allocators_.instructions.Create<TYPE>(NextInstructionId(),
                                                     std::forward<ARGS>(args)...);
    }

    /// Creates a new `TYPE` value owned by the module
    /// When the Module is destructed the object will be destructed and freed.
    /// @param args the arguments to pass to the constructor
    /// @returns the pointer to the value
    template <typename TYPE, typename... ARGS>
    TYPE* CreateValue(ARGS&&... args) {
        return allocators_.values.Create<TYPE>(std::forward<ARGS>(args)...);
    }

    /// @param inst the instruction
    /// @return the name of the given instruction, or an invalid symbol if the instruction is not
    /// named or does not have a single return value.
    Symbol NameOf(const Instruction* inst) const;

    /// @param value the value
    /// @return the name of the given value, or an invalid symbol if the value is not named.
    Symbol NameOf(const Value* value) const;

    /// @param inst the instruction to set the name of
    /// @param name the desired name of the value. May be suffixed on collision.
    /// @note requires the instruction be a single result instruction.
    void SetName(Instruction* inst, std::string_view name);

    /// @param value the value to name.
    /// @param name the desired name of the value. May be suffixed on collision.
    void SetName(Value* value, std::string_view name);

    /// @param value the value to name
    /// @param name the desired name of the value
    void SetName(Value* value, Symbol name);

    /// Removes the name from @p value
    /// @param value the value to remove the name from
    void ClearName(Value* value);

    /// @param inst the instruction to set the source of
    /// @param src the source
    /// @note requires the instruction be a single result instruction.
    void SetSource(Instruction* inst, Source src);

    /// @param value the value to set the source
    /// @param src the source
    void SetSource(Value* value, Source src);

    /// @param inst the instruction
    /// @return the source of the given instruction, or an empty source if the instruction does not
    /// have a source or does not have a single return value.
    Source SourceOf(const Instruction* inst) const;

    /// @param value the value
    /// @return the source of the given value, or an empty source if the value does not have a
    /// source.
    Source SourceOf(const Value* value) const;

    /// @return the type manager for the module
    core::type::Manager& Types() { return constant_values.types; }

    /// @return the type manager for the module
    const core::type::Manager& Types() const { return constant_values.types; }

    /// @returns a iterable of all the alive instructions
    FilteredIterable<IsAlive, BlockAllocator<Instruction>::View> Instructions() {
        return {allocators_.instructions.Objects()};
    }

    /// @returns a iterable of all the alive instructions
    FilteredIterable<IsAlive, BlockAllocator<Instruction>::ConstView> Instructions() const {
        return {allocators_.instructions.Objects()};
    }

    /// @returns a iterable of all the alive values
    FilteredIterable<IsAlive, BlockAllocator<Value>::View> Values() {
        return {allocators_.values.Objects()};
    }

    /// @returns a iterable of all the alive values
    FilteredIterable<IsAlive, BlockAllocator<Value>::ConstView> Values() const {
        return {allocators_.values.Objects()};
    }

    /// @returns the functions in the module, in dependency order
    Vector<Function*, 16> DependencyOrderedFunctions();
    /// @returns the functions in the module, in dependency order
    Vector<const Function*, 16> DependencyOrderedFunctions() const;

    /// The block allocator
    BlockAllocator<Block> blocks;

    /// The constant value manager
    core::constant::Manager constant_values;

    /// List of functions in the module.
    Vector<ConstPropagatingPtr<Function>, 8> functions;

    /// The block containing module level declarations, if any exist.
    ConstPropagatingPtr<Block> root_block;

    /// The symbol table for the module
    SymbolTable symbols{prog_id_};

    /// The map of core::constant::Value to their ir::Constant.
    Hashmap<const core::constant::Value*, ir::Constant*, 16> constants;

  private:
    /// @returns the next instruction id for this module
    Instruction::Id NextInstructionId() { return next_instruction_id_++; }

    /// The various BlockAllocators for the module
    struct {
        /// The instruction allocator
        BlockAllocator<Instruction> instructions;

        /// The value allocator
        BlockAllocator<Value> values;
    } allocators_;

    Instruction::Id next_instruction_id_ = 0;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_MODULE_H_
