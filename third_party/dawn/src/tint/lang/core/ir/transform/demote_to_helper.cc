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

#include "src/tint/lang/core/ir/transform/demote_to_helper.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/ice/ice.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module& ir;

    /// The IR builder.
    Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The global "has not discarded" flag.
    Var* continue_execution = nullptr;

    /// Map from function to a flag that indicates whether it (transitively) contains a discard.
    Hashmap<Function*, bool, 4> function_discard_status{};

    /// Set of functions that have been processed.
    Hashset<Function*, 4> processed_functions{};

    /// Process the module.
    void Process() {
        // Check each function for discard instructions, potentially inside other functions called
        // (transitively) by the function.
        Vector<Function*, 4> to_process;
        for (auto& func : ir.functions) {
            // If the function contains a discard (directly or indirectly), we need to process it.
            if (HasDiscard(func)) {
                to_process.Push(func);
            }
        }
        if (to_process.IsEmpty()) {
            return;
        }

        // Create a boolean variable that can be used to check whether the shader has discarded.
        continue_execution = b.Var("continue_execution", ty.ptr<private_, bool>());
        continue_execution->SetInitializer(b.Constant(true));
        ir.root_block->Append(continue_execution);

        // Process each function that directly or indirectly discards.
        for (auto* ep : to_process) {
            ProcessFunction(ep);
        }
    }

    /// Check if a function (transitively) contains a discard instruction.
    /// @param func the function to check
    /// @returns true if @p func contains a discard instruction
    bool HasDiscard(Function* func) {
        return function_discard_status.GetOrAdd(func, [&] { return HasDiscard(func->Block()); });
    }

    /// Check if a block (transitively) contains a discard instruction.
    /// @param block the block to check
    /// @returns true if @p block contains a discard instruction
    bool HasDiscard(Block* block) {
        // Loop over all instructions in the block.
        for (auto* inst : *block) {
            bool discard = false;
            tint::Switch(
                inst,
                [&](Discard*) {
                    // Found a discard.
                    discard = true;
                },
                [&](UserCall* call) {
                    // Check if we are calling a function that contains a discard.
                    discard = HasDiscard(call->Target());
                },
                [&](ControlInstruction* ctrl) {
                    // Recurse into control instructions and check their blocks.
                    ctrl->ForeachBlock([&](Block* blk) { discard = discard || HasDiscard(blk); });
                });
            if (discard) {
                return true;
            }
        }
        return false;
    }

    /// Process a function to replace its discard instruction and conditionalize its stores.
    /// @param func the function to process
    void ProcessFunction(Function* func) {
        if (processed_functions.Add(func)) {
            ProcessBlock(func->Block());
        }
    }

    /// Process a block to replace its discard instruction and conditionalize its stores.
    /// @param block the block to process
    void ProcessBlock(Block* block) {
        // Helper that wraps an instruction in an if statement so that it only executes if the
        // invocation has not discarded.
        auto conditionalize = [&](Instruction* inst) {
            // Create an if instruction in place of the original instruction.
            auto* cond = b.Load(continue_execution);
            auto* ifelse = b.If(cond);
            cond->InsertBefore(inst);
            inst->ReplaceWith(ifelse);

            // Move the original instruction into the if-true block.
            auto* result = ifelse->True()->Append(inst);

            auto results = inst->Results();
            TINT_ASSERT(results.Length() < 2);
            if (!results.IsEmpty() && !results[0]->Type()->Is<core::type::Void>()) {
                // The original instruction had a result, so return it from the if instruction.
                ifelse->SetResults(Vector{b.InstructionResult(results[0]->Type())});
                results[0]->ReplaceAllUsesWith(ifelse->Result(0));
                ifelse->True()->Append(b.ExitIf(ifelse, result));
            } else {
                ifelse->True()->Append(b.ExitIf(ifelse));
            }
        };

        // Loop over all instructions in the block.
        for (auto* inst = *block->begin(); inst;) {
            // As we're (potentially) modifying the block that we're iterating over, grab a pointer
            // to the next instruction before we make any changes.
            auto* next = inst->next.Get();
            TINT_DEFER(inst = next);

            tint::Switch(
                inst,
                [&](Discard* discard) {
                    // Replace every discard instruction with a store to the global flag.
                    discard->ReplaceWith(b.Store(continue_execution, false));
                    discard->Destroy();
                },
                [&](UserCall* call) {
                    // Recurse into user functions.
                    ProcessFunction(call->Target());
                },
                [&](Store* store) {
                    // Conditionalize stores to host-visible address spaces.
                    auto* ptr = store->To()->Type()->As<core::type::Pointer>();
                    if (ptr && ptr->AddressSpace() == core::AddressSpace::kStorage) {
                        conditionalize(store);
                    }
                },
                [&](CoreBuiltinCall* builtin) {
                    // Conditionalize calls to builtins that have side effects.
                    if (core::HasSideEffects(builtin->Func())) {
                        conditionalize(builtin);
                    }
                },
                [&](Return* ret) {
                    // Insert a conditional terminate invocation instruction before each return
                    // instruction in the entry point function.
                    if (ret->Func()->IsFragment()) {
                        b.InsertBefore(ret, [&] {
                            auto* cond = b.Load(continue_execution);
                            auto* ifelse = b.If(b.Not<bool>(cond));
                            b.Append(ifelse->True(), [&] {  //
                                b.TerminateInvocation();
                            });
                        });
                    }
                },
                [&](ControlInstruction* ctrl) {
                    // Recurse into control instructions.
                    ctrl->ForeachBlock([&](Block* blk) { ProcessBlock(blk); });
                },
                [&](BuiltinCall*) {
                    // TODO(crbug.com/tint/2102): Catch this with the validator instead.
                    TINT_UNREACHABLE() << "unexpected non-core instruction";
                });
        }
    }
};

}  // namespace

Result<SuccessType> DemoteToHelper(Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.DemoteToHelper", kDemoteToHelperCapabilities);
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
