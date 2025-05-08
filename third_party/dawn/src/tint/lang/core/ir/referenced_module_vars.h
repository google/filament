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

#ifndef SRC_TINT_LANG_CORE_IR_REFERENCED_MODULE_VARS_H_
#define SRC_TINT_LANG_CORE_IR_REFERENCED_MODULE_VARS_H_

#include <functional>

#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/var.h"
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

/// ReferencedModuleVars is a helper to determine the set of module-scope variables that are
/// transitively referenced by functions in a module.
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
class ReferencedModuleVars {
    // Replace this with concepts when C++20 is available
    static_assert(std::is_same<std::remove_cv_t<M>, Module>());

  public:
    /// Short form aliases for types that have the same constant-ness as M.
    /// (The single use types are not aliased)
    using BlockT = TranscribeConst<M, Block>;
    using VarT = TranscribeConst<M, Var>;
    using FunctionT = TranscribeConst<M, Function>;

    /// A set of a variables referenced by a function (in declaration order).
    using VarSet = UniqueVector<VarT*, 16>;

    /// Constructor.
    /// @param ir the module
    /// @param pred an predicate function for filtering variables
    /// Note: @p pred is not stored by the class, so can be a lambda that captures by reference.
    template <typename Predicate>
    ReferencedModuleVars(M& ir, Predicate&& pred) {
        // Loop over module-scope variables, recording the blocks that they are referenced from.
        BlockT* root_block = ir.root_block;
        for (auto* inst : *root_block) {
            if (!inst) {
                continue;
            }
            if (auto* var = inst->template As<VarT>()) {
                if (pred(var)) {
                    if (!var->Result(0)) {
                        continue;
                    }
                    var->Result()->ForEachUseUnsorted([&](const Usage& use) {
                        block_to_direct_vars_.GetOrAddZero(use.instruction->Block()).Add(var);
                    });
                }
            }
        }
    }

    /// Constructor.
    /// Provided default predicate that accepts all variables.
    explicit ReferencedModuleVars(M& ir) : ReferencedModuleVars(ir, [](VarT*) { return true; }) {}

    /// Get the set of transitively referenced module-scope variables for a function, filtered by
    /// the predicate function if provided.
    /// @param func the function
    /// @returns the set of (possibly filtered) transitively reference module-scope variables
    VarSet& TransitiveReferences(FunctionT* func) {
        return transitive_references_.GetOrAdd(func, [&] {
            VarSet vars;
            GetTransitiveReferences(func ? func->Block() : nullptr, vars);
            return vars;
        });
    }

  private:
    /// A map from blocks to their directly referenced variables.
    Hashmap<BlockT*, VarSet, 64> block_to_direct_vars_{};

    /// A map from functions to their transitively referenced variables.
    Hashmap<FunctionT*, VarSet, 8> transitive_references_;

    /// Get the set of transitively referenced module-scope variables for a block.
    /// @param block the block
    /// @param vars the set of transitively reference module-scope variables to populate
    void GetTransitiveReferences(BlockT* block, VarSet& vars) {
        if (!block) {
            return;
        }

        // Add directly referenced vars.
        if (auto itr = block_to_direct_vars_.Get(block)) {
            for (auto& var : *itr) {
                vars.Add(var);
            }
        }

        // Loop over instructions in the block to find indirectly referenced vars.
        for (auto* inst : *block) {
            tint::Switch(
                inst,
                [&](TranscribeConst<M, UserCall>* call) {
                    // Get variables referenced by a function called from this block.
                    const auto& callee_vars = TransitiveReferences(call->Target());
                    for (auto* var : callee_vars) {
                        vars.Add(var);
                    }
                },
                [&](TranscribeConst<M, ControlInstruction>* ctrl) {
                    // Recurse into control instructions and gather their referenced vars.
                    ctrl->ForeachBlock([&](BlockT* blk) { GetTransitiveReferences(blk, vars); });
                });
        }
    }
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_REFERENCED_MODULE_VARS_H_
