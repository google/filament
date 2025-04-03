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

#include "src/tint/lang/core/ir/transform/combine_access_instructions.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module& ir;

    /// Process the module.
    void Process() {
        // Loop over every instruction looking for access instructions.
        for (auto* inst : ir.Instructions()) {
            if (auto* access = inst->As<ir::Access>()) {
                // Look for places where the result of this access instruction is used as a base
                // pointer for another access instruction.
                access->Result()->ForEachUseUnsorted([&](Usage use) {
                    auto* child = use.instruction->As<ir::Access>();
                    if (child && use.operand_index == ir::Access::kObjectOperandOffset) {
                        // Push the indices of the parent access instruction into the child.
                        Vector<ir::Value*, 4> operands;
                        operands.Push(access->Object());
                        for (auto* idx : access->Indices()) {
                            operands.Push(idx);
                        }
                        for (auto* idx : child->Indices()) {
                            operands.Push(idx);
                        }
                        child->SetOperands(std::move(operands));
                    }
                });

                // If there are no other uses of the access instruction, remove it.
                if (!access->Result()->IsUsed()) {
                    access->Destroy();
                }
            }
        }
    }
};

}  // namespace

Result<SuccessType> CombineAccessInstructions(Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.CombineAccessInstructions");
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
