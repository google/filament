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

#include "src/tint/lang/core/ir/transform/remove_continue_in_switch.h"

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

    /// A map from `switch` instruction to the flag used to indicate whether a `continue` was hit.
    Hashmap<Switch*, Var*, 4> continue_flag_for_switch{};

    /// Process the module.
    void Process() {
        // Look for `continue` instructions.
        for (auto* inst : ir.Instructions()) {
            auto* cont = inst->As<Continue>();
            if (!cont) {
                continue;
            }

            // Check if this `continue` is inside a `switch` that is inside the `loop`.
            // Do this by walking up the stack of control flow instructions until we see a `loop`.
            // If we hit a `switch` before we see the loop, we need to replace the `continue`.
            auto* parent = cont->Block()->Parent();
            while (!parent->Is<Loop>()) {
                if (auto* swtch = parent->As<Switch>()) {
                    ReplaceContinue(cont, swtch);
                    break;
                }
                parent = parent->Block()->Parent();
            }
        }
    }

    /// Replace a `continue` instruction.
    /// @param cont the `continue` to replace
    /// @param swtch the `switch` instruction that it is nested inside
    void ReplaceContinue(Continue* cont, Switch* swtch) {
        auto* flag = GetContinueFlag(swtch, cont->Loop());
        b.InsertBefore(cont, [&] {
            b.Store(flag, true);
            b.ExitSwitch(swtch);
        });
        cont->Destroy();
    }

    /// Get or create the flag used to indicate whether a `continue` was hit.
    /// @param swtch the `switch` instruction to get the flag for
    /// @param loop the `loop` that is the target of `continue` instruction in this switch
    /// @returns the flag variable
    Var* GetContinueFlag(Switch* swtch, Loop* loop) {
        return continue_flag_for_switch.GetOrAdd(swtch, [&] {
            // Declare the flag before the switch statement.
            auto* flag = b.Var<function, bool>("tint_continue");
            flag->InsertBefore(swtch);

            // Check the flag after the `switch` instruction and `continue` if it was set.
            b.InsertAfter(swtch, [&] {
                auto* check = b.If(b.Load(flag));
                b.Append(check->True(), [&] {  //
                    b.Continue(loop);
                });
            });

            return flag;
        });
    }
};

}  // namespace

Result<SuccessType> RemoveContinueInSwitch(Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.RemoveContinueInSwitch",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowVectorElementPointer,
                                              core::ir::Capability::kAllowHandleVarsWithoutBindings,
                                              core::ir::Capability::kAllowClipDistancesOnF32,
                                              core::ir::Capability::kAllowDuplicateBindings,
                                              core::ir::Capability::kAllowNonCoreTypes,
                                          });
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
