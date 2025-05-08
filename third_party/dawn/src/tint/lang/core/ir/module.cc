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

#include "src/tint/lang/core/ir/module.h"

#include <limits>
#include <utility>

#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/utils/containers/unique_vector.h"
#include "src/tint/utils/ice/ice.h"

namespace tint::core::ir {

namespace {

/// Helper to non-recursively sort a module's function in dependency order.
template <typename F>
struct FunctionSorter {
    /// The dependency-ordered list of functions.
    UniqueVector<F*, 16> ordered_functions{};

    /// The functions that have been visited and checked for dependencies.
    Hashset<F*, 16> visited{};
    /// A stack of functions that need to processed and eventually added to the ordered list.
    Vector<F*, 16> function_stack{};

    /// Visit a function and check for dependencies, and eventually add it to the ordered list.
    /// @param func the function to visit
    void Visit(F* func) {
        function_stack.Push(func);
        while (!function_stack.IsEmpty()) {
            // Visit the next function on the stack, if it hasn't already been visited.
            auto* current_function = function_stack.Back();
            if (visited.Add(current_function)) {
                // Check for dependencies inside the function, adding them to the queue if they have
                // not already been visited.
                Visit(current_function->Block());
            } else {
                // We previously visited the function, so just discard it.
                function_stack.Pop();
            }

            // If the function at the top of the stack has been visited, we know that it has no
            // unvisited dependencies. We can now add it to the ordered list, and walk back down the
            // stack until we find the next unvisited function.
            while (!function_stack.IsEmpty() && visited.Contains(function_stack.Back())) {
                ordered_functions.Add(function_stack.Pop());
            }
        }
    }

    /// Visit a function body block and look for dependencies.
    /// @param block the function body to visit
    template <typename B>
    void Visit(B* block) {
        Vector<B*, 64> block_stack;
        block_stack.Push(block);
        while (!block_stack.IsEmpty()) {
            auto* current_block = block_stack.Pop();
            for (auto* inst : *current_block) {
                if (auto* control = inst->template As<ControlInstruction>()) {
                    // Enqueue child blocks.
                    control->ForeachBlock([&](B* b) { block_stack.Push(b); });
                } else if (auto* call = inst->template As<UserCall>()) {
                    // Enqueue the function that is being called.
                    if (!visited.Contains(call->Target())) {
                        function_stack.Push(call->Target());
                    }
                }
            }
        }
    }

    /// Sort the functions of a module.
    /// @param mod the IR module
    /// @returns the sorted function list
    template <typename MOD>
    static Vector<F*, 16> SortFunctions(MOD& mod) {
        FunctionSorter<F> sorter;
        for (auto& func : mod.functions) {
            sorter.Visit(func.Get());
        }
        return std::move(sorter.ordered_functions.Release());
    }
};

}  // namespace

Module::Module() : root_block(blocks.Create<ir::Block>()) {}

Module::Module(Module&&) = default;

Module::~Module() = default;

Module& Module::operator=(Module&&) = default;

Symbol Module::NameOf(const Instruction* inst) const {
    if (inst->Results().Length() != 1) {
        return Symbol{};
    }
    return NameOf(inst->Result());
}

Symbol Module::NameOf(const Value* value) const {
    return value_to_name_.GetOr(value, Symbol{});
}

void Module::SetName(Instruction* inst, std::string_view name) {
    TINT_ASSERT(inst->Results().Length() == 1);
    SetName(inst->Result(), name);
}

void Module::SetName(Instruction* inst, Symbol name) {
    TINT_ASSERT(inst->Results().Length() == 1);
    SetName(inst->Result(), name);
}

void Module::SetName(Value* value, std::string_view name) {
    TINT_ASSERT(!name.empty());
    value_to_name_.Replace(value, symbols.Register(name));
}

void Module::SetName(Value* value, Symbol name) {
    TINT_ASSERT(name.IsValid());
    value_to_name_.Replace(value, name);
}

void Module::ClearName(Value* value) {
    value_to_name_.Remove(value);
}

void Module::SetSource(Instruction* inst, Source src) {
    TINT_ASSERT(inst->Results().Length() == 1);
    SetSource(inst->Result(), src);
}

void Module::SetSource(Value* value, Source src) {
    value_to_source_.Replace(value, src);
}

Source Module::SourceOf(const Instruction* inst) const {
    if (inst->Results().Length() != 1) {
        return Source{};
    }
    return SourceOf(inst->Result());
}

Source Module::SourceOf(const Value* value) const {
    return value_to_source_.GetOr(value, Source{});
}

Vector<Function*, 16> Module::DependencyOrderedFunctions() {
    return FunctionSorter<Function>::SortFunctions(*this);
}

Vector<const Function*, 16> Module::DependencyOrderedFunctions() const {
    return FunctionSorter<const Function>::SortFunctions(*this);
}

}  // namespace tint::core::ir
