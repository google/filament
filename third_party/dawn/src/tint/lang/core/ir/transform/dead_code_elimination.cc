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

#include "src/tint/lang/core/ir/transform/dead_code_elimination.h"

#include <vector>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/ice/ice.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module& ir_;

    /// The IR builder.
    Builder b_{ir_};

    /// Process the module.
    void Process() {
        auto funcs = ir_.DependencyOrderedFunctions();
        for (auto iter = funcs.rbegin(); iter != funcs.rend(); ++iter) {
            auto func = *iter;
            if (func->IsEntryPoint()) {
                continue;
            }

            bool used = false;
            for (auto usage : func->UsagesUnsorted()) {
                if (usage->instruction->Is<core::ir::Call>()) {
                    used = true;
                    break;
                }
            }

            if (!used) {
                ir_.Destroy(func);
            }
        }

        // Find any unused vars, do this after removing functions in case the usage was in a
        // function which is unused.
        for (auto* inst : *ir_.root_block) {
            if (inst->Result()->IsUsed()) {
                continue;
            }

            // Only want to process `var`s
            auto* var = inst->As<core::ir::Var>();
            if (!var) {
                continue;
            }

            auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
            TINT_ASSERT(ptr);

            auto space = ptr->AddressSpace();
            if (space != core::AddressSpace::kOut && space != core::AddressSpace::kIn &&
                space != core::AddressSpace::kPrivate) {
                continue;
            }

            inst->Destroy();
        }
    }
};

}  // namespace

Result<SuccessType> DeadCodeElimination(Module& ir) {
    auto result =
        ValidateAndDumpIfNeeded(ir, "core.DeadCodeElimination", kDeadCodeEliminationCapabilities);
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
