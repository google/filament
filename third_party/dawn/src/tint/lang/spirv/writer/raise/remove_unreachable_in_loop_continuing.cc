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

#include "src/tint/lang/spirv/writer/raise/remove_unreachable_in_loop_continuing.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::spirv::writer::raise {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// Process the module.
    void Process() {
        // Find all unreachable instructions.
        for (auto* inst : ir.Instructions()) {
            if (auto* unreachable = inst->As<core::ir::Unreachable>()) {
                Process(unreachable);
            }
        }
    }

    /// Check and replace an unreachable instruction if necessary.
    /// @param unreachable the instruction to check and maybe replace
    void Process(core::ir::Unreachable* unreachable) {
        // Walk up the control stack to see if we are inside a loop continuing block.
        auto* block = unreachable->Block();
        while (block->Parent()) {
            auto* control = block->Parent();
            if (auto* loop = control->As<core::ir::Loop>()) {
                if (loop->Continuing() == block) {
                    Replace(unreachable);
                    return;
                }
            }
            block = control->Block();
        }
    }

    /// Replace an unreachable instruction.
    /// @param unreachable the instruction to replace
    void Replace(core::ir::Unreachable* unreachable) {
        auto* control = unreachable->Block()->Parent();

        // Fill the exit argument with `undef`.
        Vector<core::ir::Value*, 4> exit_args;
        exit_args.Resize(control->Results().Length());

        // Replace the `unreachable` with an instruction that exits from the control construct.
        auto* exit = b.Exit(unreachable->Block()->Parent(), std::move(exit_args));
        unreachable->ReplaceWith(exit);
        unreachable->Destroy();
    }
};

}  // namespace

Result<SuccessType> RemoveUnreachableInLoopContinuing(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.RemoveUnreachableInLoopContinuing",
                                          kRemoveUnreachableInLoopContinuingCapabilities);
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::writer::raise
