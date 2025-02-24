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

#include "src/tint/lang/core/ir/transform/binding_remapper.h"

#include <utility>

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/utils/result/result.h"
#include "src/tint/utils/text/string.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

Result<SuccessType> Run(ir::Module& ir,
                        const std::unordered_map<BindingPoint, BindingPoint>& binding_points) {
    if (binding_points.empty()) {
        return Success;
    }
    if (ir.root_block->IsEmpty()) {
        return Success;
    }

    // Find binding resources.
    for (auto inst : *ir.root_block) {
        auto* var = inst->As<Var>();
        if (!var || !var->Alive()) {
            continue;
        }

        auto bp = var->BindingPoint();
        if (!bp) {
            continue;
        }

        // Replace group and binding index if requested.
        auto to = binding_points.find(bp.value());
        if (to != binding_points.end()) {
            var->SetBindingPoint(to->second.group, to->second.binding);
        }
    }

    return Success;
}

}  // namespace

Result<SuccessType> BindingRemapper(
    Module& ir,
    const std::unordered_map<BindingPoint, BindingPoint>& binding_points) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.BindingRemapper");
    if (result != Success) {
        return result;
    }

    return Run(ir, binding_points);
}

}  // namespace tint::core::ir::transform
