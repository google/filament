// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/msl/writer/raise/switch_return.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/msl/ir/builtin_call.h"

namespace tint::msl::writer::raise {

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
        // Find all return instructions inside switches.
        Vector<core::ir::Return*, 8> returns_to_wrap;

        for (auto* fn : ir.functions) {
            fn->ForEachUseSorted([&](const core::ir::Usage& usage) {
                if (auto* ret = usage.instruction->As<core::ir::Return>()) {
                    auto* parent = ret->Block()->Parent();
                    if (parent && parent->Is<core::ir::Switch>()) {
                        returns_to_wrap.Push(ret);
                    }
                }
            });
        }

        // Wrap return in volatile zero conditional to work around a driver bug
        // (crbug.com/508638064).
        for (auto* ret : returns_to_wrap) {
            auto* sw = ret->Block()->Parent()->As<core::ir::Switch>();
            b.InsertBefore(ret, [&] {
                auto* zero = b.Call<msl::ir::BuiltinCall>(ty.u32(), msl::BuiltinFn::kVolatileZero);
                auto* cond = b.If(b.Equal(zero, b.Constant(core::u32(0))));

                // If the switch produces result values, use nullptr since this exit path is
                // dynamically unreachable.
                Vector<core::ir::Value*, 4> args;
                args.Resize(sw->Results().Length(), nullptr);
                b.Exit(sw, std::move(args));

                ret->Remove();
                cond->True()->Append(ret);
            });
        }
    }
};

}  // namespace

Result<SuccessType> SwitchReturn(core::ir::Module& ir) {
    core::ir::AssertValid(ir, "before msl.SwitchReturn");

    State{ir}.Process();

    return Success;
}

}  // namespace tint::msl::writer::raise
