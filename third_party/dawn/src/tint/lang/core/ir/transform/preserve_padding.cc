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

#include "src/tint/lang/core/ir/transform/preserve_padding.h"

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

    /// The IR builder.
    Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The symbol table.
    SymbolTable& sym{ir.symbols};

    /// Map from a type to a helper function that will store a decomposed value.
    Hashmap<const core::type::Type*, Function*, 4> helpers{};

    /// Process the module.
    void Process() {
        // Find host-visible stores of types that contain padding bytes.
        Vector<Store*, 8> worklist;
        for (auto inst : ir.Instructions()) {
            if (auto* store = inst->As<Store>()) {
                auto* ptr = store->To()->Type()->As<core::type::Pointer>();
                if (ptr->AddressSpace() == core::AddressSpace::kStorage &&
                    ContainsPadding(ptr->StoreType())) {
                    worklist.Push(store);
                }
            }
        }

        // Replace the stores we found with calls to helper functions that decompose the accesses.
        for (auto* store : worklist) {
            auto* replacement = MakeStore(store->To(), store->From());
            store->ReplaceWith(replacement);
            store->Destroy();
        }
    }

    /// Check if a type contains padding bytes.
    /// @param type the type to check
    /// @returns true if the type contains padding bytes
    bool ContainsPadding(const type::Type* type) {
        return tint::Switch(
            type,  //
            [&](const type::Array* arr) {
                auto* elem_ty = arr->ElemType();
                if (arr->Stride() > elem_ty->Size()) {
                    return true;
                }
                return ContainsPadding(elem_ty);
            },
            [&](const type::Matrix* mat) {
                return mat->ColumnStride() > mat->ColumnType()->Size();
            },
            [&](const type::Struct* str) {
                uint32_t current_offset = 0;
                for (auto* member : str->Members()) {
                    if (member->Offset() > current_offset) {
                        return true;
                    }
                    if (ContainsPadding(member->Type())) {
                        return true;
                    }
                    current_offset += member->Type()->Size();
                }
                return (current_offset < str->Size());
            });
    }

    /// Create an instruction that stores a (possibly padded) type to memory, decomposing the access
    /// into separate components to preserve padding if necessary.
    /// @param to the pointer to store to
    /// @param value the value to store
    /// @returns the instruction that performs the store
    Instruction* MakeStore(Value* to, Value* value) {
        auto* store_type = value->Type();

        // If there are no padding bytes in this type, just use a regular store instruction.
        if (!ContainsPadding(store_type)) {
            return b.Store(to, value);
        }

        // The type contains padding bytes, so call a helper function that decomposes the accesses.
        auto* helper = helpers.GetOrAdd(store_type, [&] {
            auto* func = b.Function("tint_store_and_preserve_padding", ty.void_());
            auto* target = b.FunctionParam("target", ty.ptr(storage, store_type));
            auto* value_param = b.FunctionParam("value_param", store_type);
            func->SetParams({target, value_param});

            b.Append(func->Block(), [&] {
                tint::Switch(
                    store_type,  //
                    [&](const type::Array* arr) {
                        b.LoopRange(
                            ty, 0_u, u32(arr->ConstantCount().value()), 1_u, [&](Value* idx) {
                                auto* el_ptr =
                                    b.Access(ty.ptr(storage, arr->ElemType()), target, idx);
                                auto* el_value = b.Access(arr->ElemType(), value_param, idx);
                                MakeStore(el_ptr->Result(), el_value->Result());
                            });
                    },
                    [&](const type::Matrix* mat) {
                        for (uint32_t i = 0; i < mat->Columns(); i++) {
                            auto* col_ptr =
                                b.Access(ty.ptr(storage, mat->ColumnType()), target, u32(i));
                            auto* col_value = b.Access(mat->ColumnType(), value_param, u32(i));
                            MakeStore(col_ptr->Result(), col_value->Result());
                        }
                    },
                    [&](const type::Struct* str) {
                        for (auto* member : str->Members()) {
                            auto* sub_ptr = b.Access(ty.ptr(storage, member->Type()), target,
                                                     u32(member->Index()));
                            auto* sub_value =
                                b.Access(member->Type(), value_param, u32(member->Index()));
                            MakeStore(sub_ptr->Result(), sub_value->Result());
                        }
                    });

                b.Return(func);
            });

            return func;
        });

        return b.Call(helper, to, value);
    }
};

}  // namespace

Result<SuccessType> PreservePadding(Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.PreservePadding", kPreservePaddingCapabilities);
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
