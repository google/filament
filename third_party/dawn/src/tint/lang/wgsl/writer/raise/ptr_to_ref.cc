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

#include "src/tint/lang/wgsl/writer/raise/ptr_to_ref.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/phony.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/wgsl/ir/unary.h"
#include "src/tint/utils/containers/reverse.h"

namespace tint::wgsl::writer::raise {
namespace {

struct Impl {
    core::ir::Module& mod;
    core::ir::Builder b{mod};

    void Run() {
        Vector<core::ir::Block*, 32> blocks;
        for (auto fn : mod.functions) {
            blocks.Push(fn->Block());
        }
        blocks.Push(mod.root_block);

        while (!blocks.IsEmpty()) {
            auto* block = blocks.Pop();
            for (auto* inst : *block) {
                tint::Switch(
                    inst,  //
                    [&](core::ir::Var* var) { ResultPtrToRef(var); },
                    [&](core::ir::Let* let) {
                        OperandRefToPtr({let, core::ir::Let::kValueOperandOffset});
                    },
                    [&](core::ir::Phony* p) {
                        OperandRefToPtr({p, core::ir::Phony::kValueOperandOffset});
                    },
                    [&](core::ir::Call* call) { OperandsRefToPtr(call); },
                    [&](core::ir::Access* access) {
                        OperandPtrToRef({access, core::ir::Access::kObjectOperandOffset});
                        ResultPtrToRef(access);
                    },
                    [&](core::ir::Store* store) {
                        OperandPtrToRef({store, core::ir::Store::kToOperandOffset});
                    },
                    [&](core::ir::StoreVectorElement* store) {
                        OperandPtrToRef({store, core::ir::StoreVectorElement::kToOperandOffset});
                    },
                    [&](core::ir::Load* load) {
                        OperandPtrToRef({load, core::ir::Load::kFromOperandOffset});
                    },
                    [&](core::ir::LoadVectorElement* load) {
                        OperandPtrToRef({load, core::ir::LoadVectorElement::kFromOperandOffset});
                    },
                    [&](core::ir::ControlInstruction* ctrl) {
                        Vector<core::ir::Block*, 3> children;
                        ctrl->ForeachBlock([&](core::ir::Block* child) { children.Push(child); });
                        for (auto* child : Reverse(children)) {
                            blocks.Push(child);
                        }
                    });
            }
        }
    }

    void OperandsRefToPtr(core::ir::Instruction* inst) {
        for (size_t i = 0, n = inst->Operands().Length(); i < n; i++) {
            OperandRefToPtr({inst, i});
        }
    }

    void OperandRefToPtr(const core::ir::Usage& use) {
        auto* operand = use.instruction->Operand(use.operand_index);
        TINT_ASSERT(operand);
        if (auto* ref_ty = As<core::type::Reference>(operand->Type())) {
            auto* as_ptr = b.InstructionResult(RefToPtr(ref_ty));
            mod.CreateInstruction<wgsl::ir::Unary>(as_ptr, core::UnaryOp::kAddressOf, operand)
                ->InsertBefore(use.instruction);
            use.instruction->SetOperand(use.operand_index, as_ptr);
        }
    }

    const core::type::Pointer* RefToPtr(const core::type::Reference* ref_ty) {
        return mod.Types().ptr(ref_ty->AddressSpace(), ref_ty->StoreType(), ref_ty->Access());
    }

    void OperandPtrToRef(const core::ir::Usage& use) {
        auto* operand = use.instruction->Operand(use.operand_index);
        if (auto* ptr_ty = As<core::type::Pointer>(operand->Type())) {
            auto* as_ptr = b.InstructionResult(PtrToRef(ptr_ty));
            mod.CreateInstruction<wgsl::ir::Unary>(as_ptr, core::UnaryOp::kIndirection, operand)
                ->InsertBefore(use.instruction);
            use.instruction->SetOperand(use.operand_index, as_ptr);
        }
    }

    void ResultPtrToRef(core::ir::Instruction* inst) {
        auto* result = inst->Result();
        if (auto* ptr = result->Type()->As<core::type::Pointer>()) {
            result->SetType(PtrToRef(ptr));
        }
    }

    const core::type::Reference* PtrToRef(const core::type::Pointer* ptr_ty) {
        return mod.Types().ref(ptr_ty->AddressSpace(), ptr_ty->StoreType(), ptr_ty->Access());
    }
};

}  // namespace

Result<SuccessType> PtrToRef(core::ir::Module& mod) {
    auto result =
        core::ir::ValidateAndDumpIfNeeded(mod, "wgsl.PtrToRef",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowMultipleEntryPoints,
                                              core::ir::Capability::kAllowOverrides,
                                              core::ir::Capability::kAllowPhonyInstructions,
                                          }

        );
    if (result != Success) {
        return result;
    }

    Impl{mod}.Run();

    return Success;
}

}  // namespace tint::wgsl::writer::raise
