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

#include "src/tint/lang/core/ir/transform/prevent_infinite_loops.h"

#include <utility>

#include "src/tint/lang/core/ir/analysis/loop_analysis.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/traverse.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/ice/ice.h"

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

    /// Process the module.
    void Process() {
        for (auto func : ir.functions) {
            // Look for loops in the function that we cannot detect as being finite, and inject a
            // a new loop index that bounds the loop to 2^64 iterations.
            analysis::LoopAnalysis analysis(*func);
            Traverse(func->Block(), [&](Loop* loop) {
                if (!analysis.GetInfo(*loop)->IsFinite()) {
                    InjectExitCondition(loop);
                }
            });
        }
    }

    /// Inject an exit condition into @p loop.
    /// @param loop the `loop` to inject the condition into
    void InjectExitCondition(Loop* loop) {
        // Initializer:
        //   var idx: vec2u;
        //
        // Body:
        //   if all(idx == vec2(UINT32_MAX)) { break; }
        //
        // Continuing:
        //   idx.x += 1;
        //   idx.y += u32(idx.x == 0);

        // Declare a new index variable at the top of the loop initializer.
        auto* idx = b.Var<function, vec2<u32>>("tint_loop_idx");
        if (loop->Initializer()->IsEmpty()) {
            loop->Initializer()->Append(b.NextIteration(loop));
        }
        loop->Initializer()->Prepend(idx);

        // Insert the new exit condition at the top of the loop body.
        b.InsertBefore(loop->Body()->Front(), [&] {
            auto* ifelse = b.If(
                b.Call<bool>(BuiltinFn::kAll,
                             b.Equal<vec2<bool>>(b.Load(idx), b.Splat<vec2<u32>>(u32::Highest()))));
            b.Append(ifelse->True(), [&] {
                // If the loop produces result values, just use `undef` as this exit condition
                // should never actually be hit.
                Vector<Value*, 1> results;
                results.Resize(loop->Results().Length(), nullptr);
                b.ExitLoop(loop, std::move(results));
            });
        });

        // Increment the index variable at the top of the continuing block.
        if (loop->Continuing()->IsEmpty()) {
            loop->Continuing()->Append(b.NextIteration(loop));
        }
        b.InsertBefore(loop->Continuing()->Front(), [&] {
            auto* low_inc = b.Add<u32>(b.LoadVectorElement(idx, 0_u), 1_u);
            ir.SetName(low_inc->Result(0), ir.symbols.New("tint_low_inc"));
            b.StoreVectorElement(idx, 0_u, low_inc);

            auto* carry = b.Convert<u32>(b.Equal<bool>(low_inc, 0_u));
            ir.SetName(carry->Result(0), ir.symbols.New("tint_carry"));
            b.StoreVectorElement(idx, 1_u, b.Add<u32>(b.LoadVectorElement(idx, 1_u), carry));
        });
    }
};

}  // namespace

Result<SuccessType> PreventInfiniteLoops(Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.PreventInfiniteLoops");
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
