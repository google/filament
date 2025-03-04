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

#include "src/tint/lang/spirv/writer/raise/pass_matrix_by_pointer.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::spirv::writer::raise {

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
        // Find user-declared functions that have value arguments containing matrices.
        for (auto& func : ir.functions) {
            for (auto* param : func->Params()) {
                if (ContainsMatrix(param->Type())) {
                    TransformFunction(func);
                    break;
                }
            }
        }
    }

    /// Checks if a type contains a matrix.
    /// @param type the type to check
    /// @returns true if the type contains a matrix, otherwise false
    bool ContainsMatrix(const core::type::Type* type) {
        return tint::Switch(
            type,  //
            [&](const core::type::Matrix*) { return true; },
            [&](const core::type::Array* arr) { return ContainsMatrix(arr->ElemType()); },
            [&](const core::type::Struct* str) {
                for (auto* member : str->Members()) {
                    if (ContainsMatrix(member->Type())) {
                        return true;
                    }
                }
                return false;
            },
            [&](Default) { return false; });
    }

    /// Transform a function that has value parameters containing matrices.
    /// @param func the function to transform
    void TransformFunction(core::ir::Function* func) {
        Vector<core::ir::FunctionParam*, 4> replacement_params;
        for (auto* param : func->Params()) {
            if (ContainsMatrix(param->Type())) {
                // Replace the value parameter with a pointer.
                auto* new_param = b.FunctionParam(ty.ptr(function, param->Type()));

                // Load from the pointer to get the value.
                auto* load = b.Load(new_param);
                func->Block()->Prepend(load);
                param->ReplaceAllUsesWith(load->Result(0));

                // Modify all of the callsites.
                func->ForEachUseUnsorted([&](core::ir::Usage use) {
                    if (auto* call = use.instruction->As<core::ir::UserCall>()) {
                        ReplaceCallArgument(call, replacement_params.Length());
                    }
                });

                replacement_params.Push(new_param);
            } else {
                // No matrices, so just copy the parameter as is.
                replacement_params.Push(param);
            }
        }
        func->SetParams(std::move(replacement_params));
    }

    /// Replace a function call argument with an equivalent passed by pointer.
    void ReplaceCallArgument(core::ir::UserCall* call, size_t arg_index) {
        // Copy the argument to a locally declared variable.
        auto* arg = call->Args()[arg_index];
        auto* local_var = b.Var(ty.ptr(function, arg->Type()));
        local_var->SetInitializer(arg);
        local_var->InsertBefore(call);

        call->SetOperand(core::ir::UserCall::kArgsOperandOffset + arg_index, local_var->Result(0));
    }
};

}  // namespace

Result<SuccessType> PassMatrixByPointer(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.PassMatrixByPointer");
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::writer::raise
