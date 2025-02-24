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

#ifndef SRC_TINT_LANG_CORE_IR_REFERENCED_FUNCTIONS_H_
#define SRC_TINT_LANG_CORE_IR_REFERENCED_FUNCTIONS_H_

#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/rtti/switch.h"

/// Utility that helps guarantee the same const-ness is applied to both types.
template <class Src, class Dst>
using TranscribeConst = std::conditional_t<std::is_const<Src>{}, std::add_const_t<Dst>, Dst>;

namespace tint::core::ir {

/// ReferencedFunctions is a helper to determine the set of functions that are transitively
/// referenced by functions in a module. References are determined lazily and cached for future
/// requests.
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
class ReferencedFunctions {
    // Replace this with concepts when C++20 is available.
    static_assert(std::is_same<std::remove_cv_t<M>, Module>());

  public:
    /// Short form aliases for types that have the same constant-ness as M.
    using BlockT = TranscribeConst<M, Block>;
    using FunctionT = TranscribeConst<M, Function>;

    /// A set of a functions referenced by a function.
    using FunctionSet = Hashset<FunctionT*, 16>;

    /// Constructor.
    /// @param ir the module
    explicit ReferencedFunctions(M& ir) {
        // Loop over functions, recording the blocks that they are called from.
        for (auto func : ir.functions) {
            if (!func) {
                continue;
            }
            func->ForEachUseUnsorted([&](const Usage& use) {
                if (auto* call = use.instruction->As<UserCall>()) {
                    block_to_direct_calls_.GetOrAddZero(call->Block()).Add(func);
                } else {
                    TINT_ASSERT(use.instruction->Is<Return>());
                }
            });
        }
    }

    /// Get the set of transitively referenced functions for a function.
    /// @param func the function
    /// @returns the set of transitively reference functions
    FunctionSet& TransitiveReferences(FunctionT* func) {
        return transitive_references_.GetOrAdd(func, [&] {
            FunctionSet functions;
            if (!func) {
                return functions;
            }

            // Walk blocks in the function to find function calls.
            Vector<BlockT*, 64> block_queue{func->Block()};
            while (!block_queue.IsEmpty()) {
                auto* next = block_queue.Pop();
                if (!next) {
                    continue;
                }

                // Add directly referenced functions.
                if (auto itr = block_to_direct_calls_.Get(next)) {
                    for (auto& callee : *itr) {
                        if (functions.Add(callee)) {
                            // Add functions transitively referenced by the callee.
                            const auto& callee_functions = TransitiveReferences(callee);
                            for (auto& transitive_func : callee_functions) {
                                functions.Add(transitive_func);
                            }
                        }
                    }
                }

                // Loop over instructions in the block to find nested blocks.
                for (auto* inst : *next) {
                    if (auto* ctrl = inst->template As<ControlInstruction>()) {
                        // Add nested blocks to the queue.
                        ctrl->ForeachBlock([&](BlockT* blk) { block_queue.Push(blk); });
                    }
                }
            }

            return functions;
        });
    }

  private:
    /// A map from blocks to their directly called functions.
    Hashmap<BlockT*, FunctionSet, 64> block_to_direct_calls_{};

    /// A map from functions to their transitively referenced functions.
    Hashmap<FunctionT*, FunctionSet, 8> transitive_references_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_REFERENCED_FUNCTIONS_H_
