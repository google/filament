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

#include "src/tint/lang/wgsl/writer/raise/value_to_let.h"

#include <algorithm>
#include <limits>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/phony.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/containers/reverse.h"

namespace tint::wgsl::writer::raise {

namespace {

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
        // Process each block.
        for (auto* block : ir.blocks.Objects()) {
            if (block != ir.root_block) {
                Process(block);
            }
        }
    }

  private:
    void Process(core::ir::Block* block) {
        // An ordered list of possibly-inlinable values returned by sequenced instructions that have
        // not yet been marked-for or ruled-out-for inlining.
        UniqueVector<core::ir::InstructionResult*, 32> pending_resolution;

        auto hoist_pending = [&](size_t count = std::numeric_limits<size_t>::max()) {
            size_t n = std::min(count, pending_resolution.Length());
            if (n > 0) {
                for (size_t i = 0; i < n; i++) {
                    MaybeReplaceWithLetOrPhony(pending_resolution[i]);
                }
                pending_resolution.Erase(0, n);
            }
        };

        // Walk the instructions of the block starting with the first.
        for (auto* inst = block->Front(); inst;) {
            auto next = inst->next;
            TINT_DEFER(inst = next);

            // Is the instruction sequenced?
            bool sequenced = inst->Sequenced();

            // Walk the instruction's operands starting with the right-most.
            auto operands = inst->Operands();
            for (auto* operand : tint::Reverse(operands)) {
                if (!operand) {
                    continue;
                }

                auto* value = operand->As<core::ir::InstructionResult>();
                if (!pending_resolution.Contains(value)) {
                    continue;
                }
                // Operand is in 'pending_resolution'

                if (pending_resolution.TryPop(value)) {
                    // Operand was the last sequenced value to be added to 'pending_resolution'
                    // This operand can be inlined as it does not change the sequencing order.
                    sequenced = true;  // Inherit the 'sequenced' flag from the inlined value
                } else {
                    // Operand was in 'pending_resolution', but was not the last sequenced value to
                    // be added. Inlining this operand would break the sequencing order, so must be
                    // emitted as a let. All preceding pending values must also be emitted as a
                    // let to prevent them being inlined and breaking the sequencing order.
                    // Remove all the values in pending up to and including 'operand'.
                    for (size_t i = 0; i < pending_resolution.Length(); i++) {
                        if (pending_resolution[i] == operand) {
                            hoist_pending(i + 1);
                            break;
                        }
                    }
                }
            }

            if (inst->Results().Length() == 1) {
                // Instruction has a single result value.
                // Check to see if the result of this instruction is a candidate for inlining.
                auto* result = inst->Result();
                // Only values with a single usage can be inlined.
                // Named values are not inlined, as we want to emit the name for a let.
                if (CanInline(result)) {
                    if (sequenced) {
                        // The value comes from a sequenced instruction. We need to ensure
                        // instruction ordering so add it to 'pending_resolution'.
                        pending_resolution.Add(result);
                    }
                    continue;
                }

                MaybeReplaceWithLetOrPhony(result);
            }

            // At this point the value has been ruled out for inlining.

            if (sequenced) {
                // A sequenced instruction with zero or multiple return values cannot be inlined.
                // All preceding sequenced instructions cannot be inlined past this point.
                hoist_pending();
            }
        }

        hoist_pending();
    }

    bool CanInline(core::ir::InstructionResult* value) {
        if (ir.NameOf(value).IsValid()) {
            // Named values should become lets
            return false;
        }

        if (value->NumUsages() != 1) {
            // Zero or multiple uses cannot be inlined
            return false;
        }

        return true;
    }

    void MaybeReplaceWithLetOrPhony(core::ir::InstructionResult* value) {
        auto* inst = value->Instruction();
        if (inst->IsAnyOf<core::ir::Var, core::ir::Let, core::ir::Phony>()) {
            return;
        }
        // Never put handle types in lets or phonys
        if (inst->Result()->Type()->IsHandle()) {
            return;
        }
        if (inst->Is<core::ir::Call>() && !value->IsUsed()) {
            bool must_use =
                inst->Is<core::ir::BuiltinCall>() && !value->Type()->Is<core::type::Void>();
            if (!must_use) {
                return;  // Call statement
            }
        }

        if (!value->IsUsed() && !ir.NameOf(value).IsValid()) {
            auto* phony = b.Phony(value);
            phony->InsertAfter(inst);
            return;
        }

        auto* let = b.Let(value->Type());
        value->ReplaceAllUsesWith(let->Result());
        let->SetValue(value);
        let->InsertAfter(inst);
        if (auto name = ir.NameOf(value); name.IsValid()) {
            ir.SetName(let, name.Name());
            ir.ClearName(value);
        }
    }
};

}  // namespace

Result<SuccessType> ValueToLet(core::ir::Module& ir) {
    auto result =
        core::ir::ValidateAndDumpIfNeeded(ir, "wgsl.ValueToLet",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowMultipleEntryPoints,
                                              core::ir::Capability::kAllowOverrides,
                                          }

        );
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::wgsl::writer::raise
