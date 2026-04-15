// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/raise/case_switch_to_if_else.h"

#include <algorithm>
#include <cstdint>
#include <limits>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/value.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::spirv::writer::raise {

namespace {

const core::ir::Capabilities kCaseSwitchToIfElseCapabilities{
    core::ir::Capability::kAllowDuplicateBindings,
    core::ir::Capability::kAllowAnyInputAttachmentIndexType,
    core::ir::Capability::kAllowNonCoreTypes,
};

/// PIMPL state for the transform, for a single function.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the IR
    void Process() {
        Vector<core::ir::Switch*, 4> worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* s = inst->As<core::ir::Switch>()) {
                // Even though the switch param could be signed we use u32 as that is likely the
                // internal representation of the compiler.
                uint32_t max_sel_case = 0u;
                uint32_t min_sel_case = std::numeric_limits<uint32_t>().max();
                for (auto& c : s->Cases()) {
                    for (auto& sel : c.selectors) {
                        if (!sel.IsDefault() && sel.val && sel.val->Value()) {
                            auto val = sel.val->Value()->ValueAs<u32>().value;
                            max_sel_case = std::max(max_sel_case, val);
                            min_sel_case = std::min(min_sel_case, val);
                        }
                    }
                }

                // Our concern is around handling of signed range calculations (vs unsigned). Any
                // range that gets close we will polyfill.
                const uint32_t kSignedRangeLimit =
                    static_cast<uint32_t>(std::numeric_limits<int32_t>().max() - 1);
                if ((max_sel_case - min_sel_case) >= kSignedRangeLimit) {
                    worklist.Push(s);
                }
            }
        }

        for (auto* s : worklist) {
            auto* switch_cond = s->Condition();
            Vector<core::ir::Value*, 4> conditions;
            core::ir::Switch::Case* default_case = nullptr;
            // We take the cases here because we're going to attach the case block to an `if`
            // statement and the switch will now be replaced with a single default case block.
            auto cases = s->TakeCases();
            auto* def = b.DefaultCase(s);
            b.Append(def, [&] {
                for (auto& c : cases) {
                    // Default block is required by spec. It will need to be treated special.
                    // It is possible that default case will also have non default selectors.
                    // These additional selectors are superfluous as they will just form one
                    // default.
                    bool found_default = false;
                    for (auto& sel : c.selectors) {
                        if (sel.IsDefault()) {
                            default_case = &c;
                            found_default = true;
                            break;
                        }
                    }
                    if (found_default) {
                        continue;
                    }

                    core::ir::Value* case_cond = nullptr;
                    for (auto& sel : c.selectors) {
                        auto* curr_selector = b.Equal(switch_cond, sel.val->As<core::ir::Value>());
                        if (case_cond) {
                            case_cond = b.Or(curr_selector, case_cond)->Result();
                        } else {
                            case_cond = curr_selector->Result();
                        }
                    }
                    conditions.Push(case_cond);
                    auto* if_cond = b.If(case_cond);
                    if_cond->SetTrue(c.block);
                }

                TINT_ASSERT(default_case);
                // Special handling required for default case. All non-default cases have exited the
                // switch by this point so the only possibility that remains is the default.
                auto* if_cond = b.If(b.Constant(true));
                if_cond->SetTrue(default_case->block);
            });

            b.Append(s->Cases()[0].block, [&] { b.Unreachable(); });
        }
    }
};

}  // namespace

Result<SuccessType> CaseSwitchToIfElse(core::ir::Module& ir) {
    core::ir::AssertValid(ir, kCaseSwitchToIfElseCapabilities, "before spirv.CaseSwitchToIfElse");

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::writer::raise
