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

#ifndef SRC_TINT_LANG_CORE_IR_REFERENCED_MODULE_DECLS_H_
#define SRC_TINT_LANG_CORE_IR_REFERENCED_MODULE_DECLS_H_

#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/override.h"
#include "src/tint/lang/core/ir/type/array_count.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/unique_vector.h"
#include "src/tint/utils/rtti/switch.h"

// Forward declarations.
namespace tint::core::ir {
class Block;
class Function;
}  // namespace tint::core::ir

/// Utility that helps guarantee the same const-ness is applied to both types.
template <class Src, class Dst>
using TranscribeConst = std::conditional_t<std::is_const<Src>{}, std::add_const_t<Dst>, Dst>;

namespace tint::core::ir {

/// ReferencedModuleDecls is a helper to determine the set of module-scope declarations that are
/// transitively referenced by functions in a module.
///
/// References are determined lazily and cached for future requests.
///
/// Note:
///      The template param M is used to ensure that inputs and outputs of this class have the same
///      const-ness. If 'Module' is supplied then the internal operations and output will not be
///      const, which is needed for transforms. Whereas if the param is 'const Module' the internals
///      and outputs will be const, which is needed for the IR validator.
/// Note:
///      Changes to the module can invalidate the cached data. This is intended to be created by
///      operations that need this information, and discarded when they complete. Tracking this
///      information inside the IR module would add overhead any time an instruction is added or
///      removed from the module. Since only a few operations need this information, it is expected
///      to be more efficient to generate it on demand.
template <typename M>
class ReferencedModuleDecls {
    // Replace this with concepts when C++20 is available
    static_assert(std::is_same<std::remove_cv_t<M>, Module>());

  public:
    /// Short form aliases for types that have the same constant-ness as M.
    /// (The single use types are not aliased)
    using BlockT = TranscribeConst<M, Block>;
    using DeclT = TranscribeConst<M, Instruction>;
    using FunctionT = TranscribeConst<M, Function>;

    /// A set of a declarations referenced by a function (in declaration order).
    using DeclSet = UniqueVector<DeclT*, 16>;

    /// Constructor.
    /// @param ir the module
    explicit ReferencedModuleDecls(M& ir) {
        // Loop over module-scope declarations, recording the blocks that they are referenced from.
        BlockT* root_block = ir.root_block;
        for (auto* inst : *root_block) {
            if (!inst || !inst->Result(0)) {
                continue;
            }

            inst->Result(0)->ForEachUseUnsorted([&](const Usage& use) {
                auto& decls = block_to_direct_decls_.GetOrAddZero(use.instruction->Block());

                // If this is an override we need to add the initializer to used instructions
                if (inst->template Is<core::ir::Override>()) {
                    AddToBlock(decls, inst);
                } else {
                    decls.Add(inst);
                }
            });

            // If the instruction is a `var<workgroup>` we have to check the type. If the type is an
            // array with a `ValueArrayCount` then we need to check the count. If it's not
            // `Constant` we need to add the instruction and any referenced instructions to the used
            // set.
            auto* var = inst->template As<core::ir::Var>();
            if (!var) {
                continue;
            }
            auto* ptr = var->Result(0)->Type()->template As<core::type::Pointer>();
            TINT_ASSERT(ptr);

            if (ptr->AddressSpace() != core::AddressSpace::kWorkgroup) {
                continue;
            }
            auto* ary = ptr->UnwrapPtr()->template As<core::type::Array>();
            if (!ary) {
                continue;
            }
            auto* cnt = ary->Count()->template As<core::ir::type::ValueArrayCount>();
            if (!cnt || cnt->value->template Is<core::ir::Constant>()) {
                continue;
            }

            auto* cnt_inst = cnt->value->template As<core::ir::InstructionResult>();
            TINT_ASSERT(cnt_inst);

            // The usage of the var is as a `let` initializer. The array count needs to
            // propagate to the `let` block.
            var->Result(0)->ForEachUseUnsorted([&](const Usage& use) {
                AddToBlock(block_to_direct_decls_.GetOrAddZero(use.instruction->Block()),
                           cnt_inst->Instruction());
            });
        }
    }

    /// Get the set of transitively referenced module-scope declarations for a function.
    /// @param func the function
    /// @returns the set of transitively reference module-scope declarations
    DeclSet& TransitiveReferences(FunctionT* func) {
        return transitive_references_.GetOrAdd(func, [&] {
            DeclSet decls;
            GetTransitiveReferences(func ? func->Block() : nullptr, decls);

            // For a compute entry point, we need to check if any of the workgroup sizes are built
            // on overrides.
            if (func && func->Stage() == core::ir::Function::PipelineStage::kCompute) {
                TINT_ASSERT(func->WorkgroupSize().has_value());

                const auto workgroup_size = func->WorkgroupSize();
                for (auto wg_size : *workgroup_size) {
                    if (wg_size->template Is<core::ir::Constant>()) {
                        continue;
                    }

                    // Workgroup size is based on instructions, walk up the chain adding those
                    // instructions to the `decls` list.
                    auto* inst = wg_size->template As<core::ir::InstructionResult>();
                    TINT_ASSERT(inst);

                    AddToBlock(decls, inst->Instruction());
                }
            }
            return decls;
        });
    }

    void AddToBlock(DeclSet& decls, core::ir::Instruction* inst) {
        Vector<DeclT*, 4> worklist;
        worklist.Push(inst);

        while (!worklist.IsEmpty()) {
            auto* wl_inst = worklist.Pop();
            if (decls.Add(wl_inst)) {
                for (auto* operand : wl_inst->Operands()) {
                    if (!operand) {
                        continue;
                    }
                    auto* res = operand->template As<core::ir::InstructionResult>();
                    if (!res) {
                        continue;
                    }
                    worklist.Push(res->Instruction());
                }
            }

            for (auto* operand : wl_inst->Operands()) {
                if (!operand) {
                    continue;
                }
                auto* res = operand->template As<core::ir::InstructionResult>();
                if (!res) {
                    continue;
                }
                worklist.Push(res->Instruction());
            }
        }
    }

  private:
    /// A map from blocks to their directly referenced declarations.
    Hashmap<BlockT*, DeclSet, 64> block_to_direct_decls_{};

    /// A map from functions to their transitively referenced declarations.
    Hashmap<FunctionT*, DeclSet, 8> transitive_references_;

    /// Get the set of transitively referenced module-scope declarations for a block.
    /// @param block the block
    /// @param decls the set of transitively reference module-scope declarations to populate
    void GetTransitiveReferences(BlockT* block, DeclSet& decls) {
        if (!block) {
            return;
        }

        // Add directly referenced declarations.
        if (auto itr = block_to_direct_decls_.Get(block)) {
            for (auto& decl : *itr) {
                decls.Add(decl);
            }
        }

        // Loop over instructions in the block to find indirectly referenced vars.
        for (auto* inst : *block) {
            tint::Switch(
                inst,
                [&](TranscribeConst<M, UserCall>* call) {
                    // Get declarations referenced by a function called from this block.
                    const auto& callee_decls = TransitiveReferences(call->Target());
                    for (auto* decl : callee_decls) {
                        decls.Add(decl);
                    }
                },
                [&](TranscribeConst<M, ControlInstruction>* ctrl) {
                    // Recurse into control instructions and gather their referenced declarations.
                    ctrl->ForeachBlock([&](BlockT* blk) { GetTransitiveReferences(blk, decls); });
                });
        }
    }
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_REFERENCED_MODULE_DECLS_H_
