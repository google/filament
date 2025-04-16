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

#include "src/tint/lang/core/ir/transform/value_to_let.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module& ir;

    /// The configuration
    const ValueToLetConfig& cfg;

    /// The IR builder.
    Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    // A set of possibly-inlinable values returned by a instructions that has not yet been
    // marked-for or ruled-out-for inlining.
    Hashset<ir::InstructionResult*, 32> pending_resolution{};
    // The accesses of the values in pending_resolution.
    Instruction::Access pending_access = Instruction::Access::kLoad;

    /// Process the module.
    void Process() {
        // Process each block.
        for (auto* block : ir.blocks.Objects()) {
            Process(block);
        }
    }

  private:
    void Process(ir::Block* block) {
        // Replace all pointer lets with the value they point too.
        ReplacePointerLetsWithValues();

        for (ir::Instruction* inst = block->Front(); inst; inst = inst->next) {
            // This transform assumes that all multi-result instructions have been replaced
            TINT_ASSERT(inst->Results().Length() < 2);

            // The memory accesses of this instruction
            auto accesses = inst->GetSideEffects();

            // A pointer access chain will be inlined by the backends. For backends which don't
            // provide pointer lets we need to force any arguments into lets such the occur in the
            // correct order. Without this it's possible to shift function calls around if they're
            // only used in the access.
            //
            // e.g.
            // ```
            // var arr : array<i32, 4>;
            // let p = val[f() + 1];
            // g();
            // let x = p;
            // ```
            //
            // If the access at `p` is inlined to `x` then the function `f()` will be called after
            // `g()` instead of before as is required. So, we pessimize and for all access operands
            // to lets to maintain ordering.
            //
            // This flush only needs to take place if the access chain contains operands which are
            // pending resolution. If nothing is pending, we don't need to flush at all.
            if (cfg.replace_pointer_lets && IsPointerAccess(inst)) {
                for (auto* operand : inst->Operands()) {
                    if (auto* result = operand->As<InstructionResult>()) {
                        if (pending_resolution.Contains(result)) {
                            PutPendingInLets();
                            break;
                        }
                    }
                }
            }

            for (auto* operand : inst->Operands()) {
                // If the operand is in pending_resolution, then we know it has a single use and
                // because it hasn't been removed with put_pending_in_lets(), we know its safe to
                // inline without breaking access ordering. By inlining the operand, we are pulling
                // the operand's instruction into the same statement as this instruction, so this
                // instruction adopts the access of the operand.
                if (auto* result = As<InstructionResult>(operand)) {
                    if (pending_resolution.Remove(result)) {
                        // Var and Let are always statements, and so can never be inlined. As such,
                        // they do not need to propagate the pending resolution through them.
                        if (!inst->IsAnyOf<Var, Let>()) {
                            accesses.Add(pending_access);
                        }
                    }
                }
            }

            if (accesses.Contains(
                    Instruction::Access::kStore)) {  // Note: Also handles load + store
                PutPendingInLets();
                pending_access = Instruction::Access::kStore;
                inst = MaybePutInLet(inst, accesses);
            } else if (accesses.Contains(Instruction::Access::kLoad)) {
                if (pending_access != Instruction::Access::kLoad) {
                    PutPendingInLets();
                    pending_access = Instruction::Access::kLoad;
                }
                inst = MaybePutInLet(inst, accesses);
            }
        }
    }

    void PutPendingInLets() {
        for (auto& pending : pending_resolution) {
            PutInLet(pending);
        }
        pending_resolution.Clear();
    }

    core::ir::Instruction* MaybePutInLet(core::ir::Instruction* inst,
                                         Instruction::Accesses& accesses) {
        if (auto* result = inst->Result(0)) {
            auto& usages = result->UsagesUnsorted();
            switch (result->NumUsages()) {
                case 0:  // No usage
                    if (accesses.Contains(Instruction::Access::kStore)) {
                        // This instruction needs to be emitted but has no uses, so we need to
                        // make sure that it will be used in a statement. Function call
                        // instructions with no uses will be emitted as call statements, so we
                        // just need to put other instructions in `let`s to force them to be
                        // emitted.
                        if (!inst->IsAnyOf<core::ir::Call>() ||
                            inst->IsAnyOf<core::ir::Construct, core::ir::Convert>()) {
                            inst = PutInLet(result);
                        }
                    }
                    break;
                case 1: {  // Single usage
                    auto usage = (*usages.begin())->instruction;
                    if (usage->Block() == inst->Block()) {
                        // Usage in same block. Assign to pending_resolution, as we don't
                        // know whether its safe to inline yet.
                        pending_resolution.Add(result);
                    } else {
                        // Usage from another block. Cannot inline.
                        inst = PutInLet(result);
                    }
                    break;
                }
                default:  // Value has multiple usages. Cannot inline.
                    inst = PutInLet(result);
                    break;
            }
        }
        return inst;
    }

    bool IsPointerAccess(core::ir::Instruction* inst) {
        return inst->Is<core::ir::Access>() && inst->Result()->Type()->Is<core::type::Pointer>();
    }

    /// PutInLet places the value into a new 'let' instruction, immediately after the value's
    /// instruction
    /// @param value the value to place into the 'let'
    /// @return the created 'let' instruction.
    ir::Instruction* PutInLet(ir::InstructionResult* value) {
        auto* inst = value->Instruction();

        if (cfg.replace_pointer_lets && IsPointerAccess(inst)) {
            return inst;
        }

        auto* let = b.Let(value->Type());
        value->ReplaceAllUsesWith(let->Result());
        let->SetValue(value);
        let->InsertAfter(inst);
        if (auto name = b.ir.NameOf(value); name.IsValid()) {
            b.ir.SetName(let->Result(), name);
            b.ir.ClearName(value);
        }
        return let;
    }

    void ReplacePointerLetsWithValues() {
        if (!cfg.replace_pointer_lets) {
            return;
        }

        for (auto* inst : ir.Instructions()) {
            if (auto* l = inst->As<ir::Let>()) {
                if (!l->Result()->Type()->Is<core::type::Pointer>()) {
                    continue;
                }
                l->Result()->ReplaceAllUsesWith(l->Value());
                l->Destroy();
            }
        }
    }
};

}  // namespace

Result<SuccessType> ValueToLet(Module& ir, const ValueToLetConfig& cfg) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.ValueToLet", kValueToLetCapabilities);
    if (result != Success) {
        return result;
    }

    State{ir, cfg}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
