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

#include "src/tint/lang/hlsl/writer/raise/replace_default_only_switch.h"

#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/exit.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/result/result.h"

namespace tint::hlsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        Vector<core::ir::Switch*, 4> worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* swtch = inst->As<core::ir::Switch>()) {
                if (swtch->Cases().Length() == 1 && swtch->Cases()[0].selectors.Length() == 1 &&
                    swtch->Cases()[0].selectors[0].IsDefault()) {
                    ProcessSwitch(swtch);
                }
            }
        }
    }

    void ProcessSwitch(core::ir::Switch* swtch) {
        // Replace the switch with a one-iteration loop. We use a loop so that 'break's
        // in switch used for control flow will continue to work.
        // Note that we can't just add a 'case 0' fallthrough on the 'default' as FXC treates it
        // the same as just a 'default', resulting in the same miscompilation.

        auto* loop = b.Loop();
        loop->InsertBefore(swtch);

        // Replace all switch exits with loop exits. This includes nested exits.
        auto exits = swtch->Exits().Vector();  // Copy to avoid iterator invalidation
        for (auto& exit : exits) {
            exit->ReplaceWith(b.ExitLoop(loop));
            exit->Destroy();
        }

        // Move all instructions from the default case to the loop body
        auto swtch_default_block = swtch->Cases()[0].block;
        for (auto* inst = *swtch_default_block->begin(); inst;) {
            // Remember next instruction as we're about to remove the current one from its block
            auto* next = inst->next.Get();
            TINT_DEFER(inst = next);
            inst->Remove();
            loop->Body()->Append(inst);
        }

        swtch->Destroy();
    }
};

}  // namespace

Result<SuccessType> ReplaceDefaultOnlySwitch(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "hlsl.ReplaceDefaultOnlySwitch");
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::hlsl::writer::raise
