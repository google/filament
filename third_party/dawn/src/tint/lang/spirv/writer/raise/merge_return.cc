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
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/rtti/switch.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer::raise {

namespace {

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

    /// The final return at the end of the function block.
    /// May be null when the function returns in all blocks of a control instruction.
    core::ir::Return* fn_return = nullptr;

    /// A set of control instructions that transitively hold a return instruction
    Hashset<core::ir::ControlInstruction*, 8> holds_return_{};

    /// Process the function.
    /// @param fn the function to process
    void Process(core::ir::Function* fn) {
        if (fn->IsEntryPoint()) {
            // Entry points are not called and do not require this transformation to ensure
            // convergence.
            return;
        }

        // Find all of the nested return instructions in the function.
        for (const auto& usage : fn->UsagesUnsorted()) {
            if (auto* ret = usage->instruction->As<core::ir::Return>()) {
                TransitivelyMarkAsReturning(ret->Block()->Parent());
            }
        }

        if (holds_return_.IsEmpty()) {
            // No control instructions hold a return statement, so nothing needs to be done.
            return;
        }

        // Create a boolean variable that can be used to check whether the function is returning.
        continue_execution = b.Var("continue_execution", ty.ptr<function, bool>());
        continue_execution->SetInitializer(b.Constant(true));
        fn->Block()->Prepend(continue_execution);

        // Create a variable to hold the return value if needed.
        if (!fn->ReturnType()->Is<core::type::Void>()) {
            return_val = b.Var("return_value", ty.ptr(function, fn->ReturnType()));
            fn->Block()->Prepend(return_val);
        }

        // Look to see if the function ends with a return
        fn_return = tint::As<core::ir::Return>(fn->Block()->Terminator());

        // Process the function's block.
        // This will traverse into control instructions that hold returns, and apply the necessary
        // changes to remove returns.
        ProcessBlock(fn->Block());

        // If the function didn't end with a return, add one
        if (!fn_return) {
            AppendFinalReturn(fn);
        }

        // Cleanup - if 'continue_execution' was only ever assigned, remove it.
        continue_execution->DestroyIfOnlyAssigned();
    }

    /// Marks all the control instructions from ctrl to the function as holding a return.
    /// @param ctrl the control instruction to mark as returning, along with all ancestor control
    /// instructions.
    void TransitivelyMarkAsReturning(core::ir::ControlInstruction* ctrl) {
        for (; ctrl; ctrl = ctrl->Block()->Parent()) {
            if (!holds_return_.Add(ctrl)) {
                return;
            }
        }
    }

    /// Walks the instructions of @p block, processing control instructions that are marked as
    /// holding a return instruction. After processing a control instruction with a return, the
    /// instructions following the control instruction will be wrapped in a 'if' that only executes
    /// if a return was not reached.
    /// @param block the block to process
    void ProcessBlock(core::ir::Block* block) {
        core::ir::If* inner_if = nullptr;
        Vector<core::ir::If*, 4> inner_if_stack;
        for (auto* inst = *block->begin(); inst;) {  // For each instruction in 'block'
            // As we're modifying the block that we're iterating over, grab the pointer to the next
            // instruction before (potentially) moving 'inst' to another block.
            auto* next = inst->next.Get();
            TINT_DEFER(inst = next);

            if (auto* ret = inst->As<core::ir::Return>()) {
                // Note: Return instructions are processed without being moved into the 'if' block.
                ProcessReturn(ret, inner_if);
                break;  // All instructions processed.
            }

            if (inst->Is<core::ir::Unreachable>()) {
                // Unreachable can become reachable once returns are turned into exits.
                // As this is the terminator for the block, simply stop processing the
                // instructions. A appropriate terminator will be created for this block below.
                inst->Destroy();
                break;
            }

            // If we've already passed a control instruction holding a return, then we need to move
            // the instructions that follow the control instruction into the inner-most 'if'.
            if (inner_if) {
                inst->Remove();
                inner_if->True()->Append(inst);
            }

            // Control instructions holding a return need to be processed, and then a new 'if' needs
            // to be created to hold the instructions that are between the control instruction and
            // the block's terminating instruction.
            if (auto* ctrl = inst->As<core::ir::ControlInstruction>()) {
                if (holds_return_.Contains(ctrl)) {
                    // Control instruction transitively holds a return.
                    ctrl->ForeachBlock(
                        [&](core::ir::Block* ctrl_block) { ProcessBlock(ctrl_block); });
                    if (next && (next != fn_return || fn_return->Value()) &&
                        !tint::IsAnyOf<core::ir::Exit, core::ir::Unreachable>(next)) {
                        inner_if = CreateIfContinueExecution(ctrl);
                        inner_if_stack.Push(inner_if);
                    }
                }
            }
        }

        if (inner_if) {
            // new_value_with_type returns a new RuntimeValue with the same type as 'v'
            auto new_value_with_type = [&](core::ir::Value* v) {
                return b.InstructionResult(v->Type());
            };

            if (auto* terminator = inner_if->True()->Terminator()) {
                if (auto* exit_if = terminator->As<core::ir::ExitIf>()) {
                    // Ensure the associated 'if' is updated.
                    exit_if->SetIf(inner_if);

                    if (!exit_if->Args().IsEmpty()) {
                        // Inner-most 'if' has a 'exit_if' that returns values.
                        // These need propagating through the if stack.
                        inner_if->SetResults(
                            tint::Transform<8>(exit_if->Args(), new_value_with_type));
                    }
                }
            } else {
                // Inner-most if doesn't have a terminating instruction. Add an 'exit_if'.
                inner_if->True()->Append(b.ExitIf(inner_if));
            }

            // Walk back down the stack of 'if' instructions that were created, and add any missing
            // terminating instructions to the blocks holding them.
            while (!inner_if_stack.IsEmpty()) {
                auto* i = inner_if_stack.Pop();
                if (!i->Block()->Terminator() && i->Block()->Parent()) {
                    // Append the exit instruction to the block holding the 'if'.
                    Vector<core::ir::InstructionResult*, 8> exit_args = i->Results();
                    if (i->Results().IsEmpty()) {
                        i->SetResults(tint::Transform(exit_args, new_value_with_type));
                    }
                    auto* exit = b.Exit(i->Block()->Parent(), std::move(exit_args));
                    i->Block()->Append(exit);
                }
            }
        }

        // If this is a non-empty block that still has no terminator, we need to insert an exit
        // instruction (unless it is the function's top-level block).
        if (!block->IsEmpty() && !block->Terminator() && block->Parent()) {
            ExitFromBlock(block);
        }
    }

    /// Transforms a return instruction.
    /// @param ret the return instruction
    /// @param cond the possibly null 'if(continue_execution)' instruction for the current block.
    /// @note unlike other instructions, return instructions are not automatically moved into the
    /// 'if(continue_execution)' block.
    void ProcessReturn(core::ir::Return* ret, core::ir::If* cond) {
        if (ret == fn_return) {
            // 'ret' is the final instruction for the function.
            ProcessFunctionBlockReturn(ret, cond);
        } else {
            // Return is in a nested block
            ProcessNestedReturn(ret, cond);
        }
    }

    /// Transforms the return instruction that is the last instruction in the function's block.
    /// @param ret the return instruction
    /// @param cond the possibly null 'if(continue_execution)' instruction for the current block.
    void ProcessFunctionBlockReturn(core::ir::Return* ret, core::ir::If* cond) {
        if (!return_val) {
            return;  // No need to transform non-value, end-of-function returns
        }

        // Assign the return's value to 'return_val' inside a 'if(continue_execution)'
        if (!cond) {
            cond = CreateIfContinueExecution(ret->prev);
        }
        cond->True()->Append(b.Store(return_val, ret->Value()));
        cond->True()->Append(b.ExitIf(cond));

        // Change the function return to unconditionally load 'return_val' and return it
        auto* load = b.Load(return_val);
        load->InsertBefore(ret);
        ret->SetValue(load->Result(0));
    }

    /// Transforms the return instruction that is found in a control instruction.
    /// @param ret the return instruction
    /// @param cond the possibly null 'if(continue_execution)' instruction for the current block.
    void ProcessNestedReturn(core::ir::Return* ret, core::ir::If* cond) {
        // If we have a 'if(continue_execution)' block, then insert instructions into that,
        // otherwise insert into the block holding the return.
        auto* block = cond ? cond->True() : ret->Block();

        // Set the 'continue_execution' flag to false, and store the return value into 'return_val',
        // if present.
        block->Append(b.Store(continue_execution, false));
        if (return_val) {
            block->Append(b.Store(return_val, ret->Args()[0]));
        }

        // Replace the return instruction with an exit instruction.
        ExitFromBlock(block);
        ret->Destroy();
    }

    /// Append an instruction to @p block that will exit from its containing control instruction.
    /// @param block the instruction to exit from
    void ExitFromBlock(core::ir::Block* block) {
        // If the outermost control instruction is expecting exit values, then return them as
        // 'undef' values.
        auto* ctrl = block->Parent();
        Vector<core::ir::Value*, 8> exit_args;
        exit_args.Resize(ctrl->Results().Length());

        // Replace the return instruction with an exit instruction.
        block->Append(b.Exit(ctrl, std::move(exit_args)));
    }

    /// Builds instructions to create a 'if(continue_execution)' conditional.
    /// @param after new instructions will be inserted after this instruction
    /// @return the 'If' control instruction
    core::ir::If* CreateIfContinueExecution(core::ir::Instruction* after) {
        auto* load = b.Load(continue_execution);
        auto* cond = b.If(load);
        load->InsertAfter(after);
        cond->InsertAfter(load);
        return cond;
    }

    /// Adds a final return instruction to the end of @p fn
    /// @param fn the function
    void AppendFinalReturn(core::ir::Function* fn) {
        b.Append(fn->Block(), [&] {
            if (return_val) {
                b.Return(fn, b.Load(return_val));
            } else {
                b.Return(fn);
            }
        });
    }
};

}  // namespace

Result<SuccessType> MergeReturn(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.MergeReturn");
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
