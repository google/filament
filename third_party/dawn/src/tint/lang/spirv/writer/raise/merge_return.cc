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

#include "src/tint/lang/spirv/writer/raise/merge_return.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/containers/transform.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::spirv::writer::raise {

namespace {

// The capabilities that the transform can support.
const core::ir::Capabilities kMergeReturnCapabilities{
    core::ir::Capability::kAllowAnyInputAttachmentIndexType,
    core::ir::Capability::kAllowNonCoreTypes,
};

/// PIMPL state for the transform, for a single function.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The "has not returned" flag.
    core::ir::Var* continue_execution = nullptr;

    /// The variable that holds the return value.
    /// Null when the function does not return a value.
    core::ir::Var* return_val = nullptr;

    /// The set of control instructions whose subsequent instructions need to be conditionalized.
    UniqueVector<core::ir::ControlInstruction*, 8> needs_conditionalized_merge{};

    /// The set of control instructions whose subsequent instructions have been conditionalized.
    Hashset<core::ir::ControlInstruction*, 8> has_conditionalized_merge{};

    /// Process the function.
    /// @param fn the function to process
    void Process(core::ir::Function* fn) {
        if (fn->IsEntryPoint()) {
            // Entry points are not called and do not require this transformation to ensure
            // convergence.
            return;
        }

        // Find all of the return instructions.
        Vector<core::ir::Return*, 8> returns;
        fn->ForEachUseSorted([&](const core::ir::Usage& usage) {
            if (auto* ret = usage.instruction->As<core::ir::Return>()) {
                returns.Push(ret);
            }
        });

        // If there exists at least one return that is not the final function body return, then we
        // need to transform the function. Otherwise we can bail early here without making changes.
        if (returns.IsEmpty() ||
            (returns.Length() == 1 && returns[0] == fn->Block()->Terminator())) {
            return;
        }

        // Create a boolean variable that can be used to check whether the function is returning,
        // and a variable to hold the return value if needed.
        b.InsertBefore(fn->Block()->Front(), [&] {
            if (!fn->ReturnType()->Is<core::type::Void>()) {
                return_val = b.Var("return_value", ty.ptr(function, fn->ReturnType()));
            }
            continue_execution = b.Var("continue_execution", true);
        });

        // Replace every return instruction with an exit instruction, setting a flag to signal that
        // execution should not continue.
        for (auto* ret : returns) {
            ReplaceReturn(ret);
        }

        // Conditionalize instructions after control flow instructions that are exited from the site
        // of a return instruction..
        // This may discover additional control instructions that need conditionalized merges as we
        // introduce new exit instructions that walk back up the control flow stack.
        while (!needs_conditionalized_merge.IsEmpty()) {
            auto* control = needs_conditionalized_merge.Pop();
            if (has_conditionalized_merge.Add(control)) {
                ConditionalizeMerge(control);
            }
        }

        // Insert the final return at the end of the function.
        b.Append(fn->Block(), [&] {
            if (return_val) {
                b.Return(fn, b.Load(return_val));
            } else {
                b.Return(fn);
            }
        });

        // Cleanup: if 'continue_execution' was only ever assigned, remove it.
        if (continue_execution) {
            continue_execution->DestroyIfOnlyAssigned();
        }
    }

    /// Replace a return instruction with an exit instruction, setting the flag to signal that
    /// execution should not continue, and capturing the return value if present.
    /// @param ret the return instruction to replace
    void ReplaceReturn(core::ir::Return* ret) {
        // We will exit out of the current control instruction to the enclosing block.
        auto* control = ret->Block()->Parent();

        // Set the 'continue_execution' flag to false, and store the return value into
        // 'return_value', if present.
        b.InsertBefore(ret, [&] {
            // Set the flag only if we are not in the function block, where it would be redundant.
            if (control) {
                b.Store(continue_execution, false);
            }
            if (return_val) {
                b.Store(return_val, ret->Value());
            }
            if (control) {
                ExitFromControl(control);
            }
        });
        ret->Destroy();
    }

    /// Conditionalize the instructions that will be in the merge block for @p control by wrapping
    /// them in a new `if` instruction that checks the `continue_execution` flag.
    /// @param control the control instruction that should have its merge conditionalized
    void ConditionalizeMerge(core::ir::ControlInstruction* control) {
        auto* next = control->next.Get();

        // If there are no instructions after the control instruction then we must be at the
        // top-level function block (where the final return has been removed), so there's nothing to
        // conditionalize.
        if (next == nullptr) {
            return;
        }

        // If we are nested somewhere inside a loop/switch instruction, we can just jump out of that
        // instead of conditionalizing the subsequent instructions.
        auto* exit_target = control->Block()->Parent();
        while (exit_target) {
            if (exit_target->IsAnyOf<core::ir::Loop, core::ir::Switch>()) {
                b.InsertBefore(next, [&] {
                    auto* load = b.Load(continue_execution);
                    auto* cond = b.If(b.Not<bool>(load));
                    b.Append(cond->True(), [&] {  //
                        ExitFromControl(exit_target);
                    });
                });
                return;
            }
            exit_target = exit_target->Block()->Parent();
        }

        // If the next instruction is an unreachable, we've made it reachable and need to exit up
        // the control flow stack instead.
        if (next->Is<core::ir::Unreachable>()) {
            if (control->Block()->Parent()) {
                b.InsertBefore(next, [&] {  //
                    ExitFromControl(next->Block()->Parent());
                });
            }
            next->Destroy();
            return;
        }

        // If the next instruction is already an exit instruction, we need to conditionalize the
        // merge that it will exit to.
        if (auto* exit = next->As<core::ir::Exit>()) {
            needs_conditionalized_merge.Add(exit->ControlInstruction());
            return;
        }

        // The merge block is non-trivial, so we need to wrap all subsequent instructions in a new
        // conditional that is based on the `continue_execution` flag.
        auto* load = b.Load(continue_execution);
        auto* cond = b.If(load);
        load->InsertAfter(control);
        cond->InsertAfter(load);
        while (cond->next) {
            auto inst = cond->next;
            inst->Remove();
            cond->True()->Append(inst);

            // If the instruction is an exit if, we need to re-target it to the new `if` that we
            // created and propagate any of its operands through the new `if` as results.
            if (auto* exit_if = inst->As<core::ir::ExitIf>()) {
                exit_if->SetIf(cond);

                auto exit_args = exit_if->Args();
                if (!exit_args.IsEmpty()) {
                    cond->SetResults(tint::Transform<8>(exit_args, [&](auto* arg) {  //
                        return b.InstructionResult(arg->Type());
                    }));
                }
            }
        }
        // We might not have moved a terminator instruction if we are conditionalizing instructions
        // in the function block, since the final return may have been removed.
        if (!cond->True()->Back()->Is<core::ir::Terminator>()) {
            cond->True()->Append(b.ExitIf(cond));
        }

        // Now exit up the control flow stack and conditionalize the containing merge block as well.
        if (auto* parent = control->Block()->Parent()) {
            // Propagate results from the conditional `if` through the new exit.
            Vector<core::ir::Value*, 8> exit_args;
            exit_args.Resize(control->Block()->Parent()->Results().Length());
            TINT_ASSERT(cond->Results().Length() == exit_args.Length());
            for (size_t i = 0; i < cond->Results().Length(); ++i) {
                exit_args[i] = cond->Results()[i];
            }
            b.Exit(parent, std::move(exit_args))->InsertAfter(cond);

            needs_conditionalized_merge.Add(parent);
        }
    }

    /// Exit from the target control flow instruction.
    /// @param control the control flow instruction to exit from
    void ExitFromControl(core::ir::ControlInstruction* control) {
        // Produce `undef` values for any results, since they will never be used.
        Vector<core::ir::Value*, 8> exit_args;
        exit_args.Resize(control->Results().Length());
        b.Exit(control, std::move(exit_args));

        // Mark the control flow instruction as requiring its merge to be conditionalized.
        needs_conditionalized_merge.Add(control);
    }
};

}  // namespace

Result<SuccessType> MergeReturn(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.MergeReturn", kMergeReturnCapabilities);
    if (result != Success) {
        return result;
    }

    // Process each function.
    for (auto& fn : ir.functions) {
        State{ir}.Process(fn);
    }

    return Success;
}

}  // namespace tint::spirv::writer::raise
