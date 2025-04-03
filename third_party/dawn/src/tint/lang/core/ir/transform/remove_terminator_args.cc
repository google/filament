// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/transform/remove_terminator_args.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

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

    /// A list of terminators that need to have their arguments cleared.
    Vector<Terminator*, 8> terminators_to_clear{};

    /// Process the module.
    void Process() {
        // Loop over every instruction looking for control instructions.
        for (auto* inst : ir.Instructions()) {
            tint::Switch(
                inst,
                [&](If* i) {  //
                    RemoveExitArgs(i);
                },
                [&](Loop* l) {  //
                    RemoveExitArgs(l);
                    RemoveBlockParams(l->Body(), l->Initializer()->Front());
                    RemoveBlockParams(l->Continuing(), l->Body()->Front());
                },
                [&](Switch* s) {  //
                    RemoveExitArgs(s);
                });

            // Remove arguments from all terminators that we found.
            for (auto* terminator : terminators_to_clear) {
                if (auto* breakif = terminator->As<BreakIf>()) {
                    // We retain the condition operand on break_if instructions.
                    breakif->SetOperands(Vector{breakif->Condition()});
                } else {
                    terminator->ClearOperands();
                }
            }
            terminators_to_clear.Clear();
        }
    }

    /// Remove the arguments from all exit instructions inside a control instruction.
    /// @param ci the control instruction
    void RemoveExitArgs(ControlInstruction* ci) {
        // Loop over all of the instruction results.
        for (size_t i = 0; i < ci->Results().Length(); i++) {
            auto* result = ci->Result(i);

            // Create a variable to hold the result, and insert it before the control instruction.
            auto* var = b.Var(ty.ptr<function>(result->Type()));
            var->InsertBefore(ci);

            // Store to the variable before each exit instruction.
            for (auto exit : ci->Exits()) {
                Value* value = nullptr;
                if (auto* breakif = exit.Value()->As<BreakIf>()) {
                    value = breakif->ExitValues()[i];
                } else {
                    value = exit.Value()->Args()[i];
                }
                if (value) {
                    auto* store = b.Store(var, value);
                    store->InsertBefore(exit.Value());
                }
            }

            // Replace the original result with a load from the variable that we created above.
            auto* load = b.LoadWithResult(result, var);
            load->InsertAfter(ci);
        }

        // Remove the arguments from the exits and the results from the control instruction.
        for (auto exit : ci->Exits()) {
            terminators_to_clear.Push(exit);
        }
        ci->ClearResults();
    }

    /// Remove block parameters and arguments from all branches to a block.
    /// @param block the block
    /// @param var_insertion_point the insertion point for variables used to replace parameters
    void RemoveBlockParams(MultiInBlock* block, Instruction* var_insertion_point) {
        for (size_t i = 0; i < block->Params().Length(); i++) {
            auto* param = block->Params()[i];

            // Create a variable to hold the parameter value, and insert it in the parent block.
            auto* var = b.Var(ty.ptr<function>(param->Type()));
            var->InsertBefore(var_insertion_point);

            // Store to the variable before each branch.
            for (auto* branch : block->InboundSiblingBranches()) {
                Value* value = nullptr;
                if (auto* breakif = branch->As<BreakIf>()) {
                    value = breakif->NextIterValues()[i];
                } else {
                    value = branch->Args()[i];
                }
                if (value) {
                    auto* store = b.Store(var, value);
                    store->InsertBefore(branch);
                }
            }

            // Replace the original result with a load from the variable that we created above.
            auto* load = b.Load(var);
            load->InsertBefore(block->Front());
            param->ReplaceAllUsesWith(load->Result());
        }

        // Remove the arguments from the branches and the parameters from the block.
        for (auto exit : block->InboundSiblingBranches()) {
            terminators_to_clear.Push(exit);
        }
        block->SetParams({});
    }
};

}  // namespace

Result<SuccessType> RemoveTerminatorArgs(Module& ir) {
    auto result =
        ValidateAndDumpIfNeeded(ir, "core.RemoveTerminatorArgs", kRemoveTerminatorArgsCapabilities);
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
