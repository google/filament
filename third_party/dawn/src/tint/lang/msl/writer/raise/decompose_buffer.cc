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

#include "src/tint/lang/msl/writer/raise/decompose_buffer.h"

#include <string>
#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/traverse.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/msl/builtin_fn.h"
#include "src/tint/lang/msl/ir/builtin_call.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/hashset.h"

namespace tint::msl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Tracks visited params.
    Hashset<core::ir::FunctionParam*, 16> visited_params_{};

    /// Tracks functions with updated parameter types
    Hashmap<core::ir::Function*, Hashset<size_t, 4>, 8> updated_func_args_{};

    /// Tracks function parameters used with arrayLength builtin functions.
    Hashmap<core::ir::FunctionParam*, bool, 16> used_with_array_length_{};

    /// Tracks buffer structs based on their pointer type.
    Hashmap<const core::type::Type*, const core::type::Struct*, 4> buffer_bundles_{};
    uint32_t bundle_name_suffix_ = 0;

    void Process() {
        LowerParameters();

        Vector<core::ir::Var*, 4> var_worklist;
        for (auto* inst : *ir.root_block) {
            auto* var = inst->As<core::ir::Var>();
            if (!var) {
                continue;
            }

            auto* var_ty = var->Result()->Type()->As<core::type::Pointer>();
            if (!var_ty) {
                continue;
            }

            // Only need to decompose buffers.
            if (var_ty->StoreType()->Is<core::type::Buffer>()) {
                var_worklist.Push(var);
            }
        }

        for (auto* var : var_worklist) {
            auto* result = var->Result();

            // Lower the variable type.
            auto* var_ty = result->Type()->As<core::type::Pointer>();
            auto* new_ty = ConvertType(var_ty->StoreType()->As<core::type::Buffer>(), var_ty);
            result->SetType(new_ty);
        }

        // Decompose bufferView and bufferArrayView
        Vector<core::ir::CoreBuiltinCall*, 16> to_delete;
        for (auto func : ir.functions) {
            Traverse(func->Block(), [&](core::ir::Instruction* inst) {
                tint::Switch(
                    inst,
                    [&](core::ir::CoreBuiltinCall* call) {
                        TINT_IR_ASSERT(ir, call->Func() != core::BuiltinFn::kBufferLength);
                        TINT_IR_ASSERT(ir, call->Func() != core::BuiltinFn::kArrayLength);

                        if (call->Func() == core::BuiltinFn::kBufferView ||
                            call->Func() == core::BuiltinFn::kBufferArrayView) {
                            BufferView(call);
                            to_delete.Push(call);
                        }
                    },
                    [&](core::ir::Let* let) {
                        if (!let->Result()->Type()->UnwrapPtr()->Is<core::type::Buffer>()) {
                            return;
                        }

                        auto* type = let->Result()->Type()->As<core::type::Pointer>();
                        auto* new_ty =
                            ConvertType(type->StoreType()->As<core::type::Buffer>(), type);
                        let->Result()->SetType(new_ty);
                    });
            });
        }
        for (auto* call : to_delete) {
            call->Destroy();
        }
    }

    /// Convert pointer to buffer to lowered type
    ///
    /// @param buffer the buffer type
    /// @param pointer the pointer type
    /// @returns a pointer to lowered type
    const core::type::Type* ConvertType(const core::type::Buffer* buffer,
                                        const core::type::Pointer* pointer) {
        const core::type::Type* arr_ty = nullptr;
        if (buffer->Count()->Is<core::type::RuntimeArrayCount>()) {
            arr_ty = ty.runtime_array(ty.u8());
        } else {
            TINT_IR_ASSERT(ir, buffer->ConstantCount());
            arr_ty = ty.array(ty.u8(), buffer->ConstantCount().value());
        }
        return ty.ptr(pointer->AddressSpace(), arr_ty, pointer->Access());
    }

    // Lowers buffer parameter types.
    //
    // buffer -> array<u8>
    // buffer<N> -> array<u8, N>
    //
    // Also fixes parameter mismatches by introducing a pointer offset call with zero offset as a
    // reinterpret cast.
    void LowerParameters() {
        auto ordered_funcs = ir.DependencyOrderedFunctions();
        for (auto func : ordered_funcs) {
            // Update mismatches.
            Traverse(func->Block(), [&](core::ir::UserCall* call) {
                auto* target = call->Target();
                size_t num_args = call->Args().size();
                for (size_t i = 0; i < num_args; i++) {
                    auto* param = target->Params()[i];
                    auto* arg = call->Args()[i];
                    if (auto* buf_ty = arg->Type()->UnwrapPtr()->As<core::type::Buffer>()) {
                        auto* ptr_ty = arg->Type()->As<core::type::Pointer>();
                        auto* converted_ty = ConvertType(buf_ty, ptr_ty);
                        if (converted_ty != param->Type()) {
                            b.InsertBefore(call, [&] {
                                auto* cast = b.CallExplicit<msl::ir::BuiltinCall>(
                                    param->Type(), msl::BuiltinFn::kPointerOffset,
                                    Vector<core::ir::TemplateParameter, 1>{
                                        param->Type()->UnwrapPtr()},
                                    arg, 0_u);
                                call->SetArg(i, cast->Result());
                            });
                        }
                    }
                }
            });

            // Lower buffer parameters.
            for (auto param : func->Params()) {
                if (auto* buf_ty = param->Type()->UnwrapPtr()->As<core::type::Buffer>()) {
                    auto* ptr_ty = param->Type()->As<core::type::Pointer>();
                    auto* new_ty = ConvertType(buf_ty, ptr_ty);
                    param->SetType(new_ty);

                    auto uses = param->UsagesSorted();
                    while (!uses.IsEmpty()) {
                        auto use = uses.Pop();
                        if (auto* let = use.instruction->As<core::ir::Let>()) {
                            let->Result()->SetType(new_ty);
                            for (auto& u : let->Result()->UsagesSorted()) {
                                uses.Push(u);
                            }
                        }
                    }
                }
            }
        }
    }

    // Transforms bufferView and bufferArrayView calls
    //
    // Propagates the offset (and size) data from the call forward to the uses.
    // Replaces the call with a pointer offset call at the same offset.
    //
    // @param call The buffer[Array]View call
    void BufferView(core::ir::CoreBuiltinCall* call) {
        auto* offset_arg = call->Args()[1];
        core::ir::Instruction* new_call = nullptr;
        b.InsertBefore(call, [&] {
            offset_arg = b.InsertBitcastIfNeeded(ty.u32(), offset_arg);
            new_call = b.CallExplicitWithResult<msl::ir::BuiltinCall>(
                call->DetachResult(), msl::BuiltinFn::kPointerOffset,
                Vector{call->ExplicitTemplateParams()[0]}, call->Args()[0], offset_arg);
        });
    }
};

}  // namespace

Result<SuccessType> DecomposeBuffer(core::ir::Module& ir) {
    AssertValid(ir, "before msl.DecomposeBuffer");

    State{ir}.Process();

    ir.properties.Add(core::ir::Property::kAllow8BitIntegers);
    ir.properties.Remove(core::ir::Property::kAllowBufferTypes);

    return Success;
}

}  // namespace tint::msl::writer::raise
