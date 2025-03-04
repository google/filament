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

#include "src/tint/lang/hlsl/writer/raise/replace_non_indexable_mat_vec_stores.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/result/result.h"

namespace tint::hlsl::writer::raise {
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

    const core::type::Type* GetElementType(const core::type::Type* type,
                                           const core::ir::Value* idx) {
        if (auto* constant = idx->As<core::ir::Constant>()) {
            auto i = constant->Value()->ValueAs<u32>();
            return type->Element(i);
        }
        return type->Elements().type;
    }

    bool IsConstant(core::ir::Value* value) {
        if (value->Is<core::ir::Constant>()) {
            return true;
        }
        // Also check for lets to constant, which is a common use-case
        if (auto* result = value->As<core::ir::InstructionResult>()) {
            if (auto* let = result->Instruction()->As<core::ir::Let>()) {
                if (let->Value()->Is<core::ir::Constant>()) {
                    return true;
                }
            }
        }
        return false;
    }

    // If the input access chain is a dynamic index to a matrix, this function will replace the
    // input store (either a Store or a StoreVectorElement) with a switch statement over the
    // dynamic index, with a case for each column index used to index the matrix directly.
    // Returns a vector of the new Store/StoreVectorElements that were created in each case block.
    // NOTE: `store` may be destroyed
    Vector<core::ir::Instruction*, 4> ProcessMatrixColumnAccess(core::ir::Instruction* store,
                                                                core::ir::Access* const to_access) {
        // Look for a dynamically indexed matrix through the access chain.
        // This will always be the last index in the chain.

        // The last index must be dynamic
        if (IsConstant(to_access->Indices().Back())) {
            return {};
        }
        // Get the root object type
        const auto* object_ty = to_access->Object()->Type()->As<core::type::Pointer>()->StoreType();
        const auto indicesButLast =
            to_access->Indices().Truncate(to_access->Indices().Length() - 1);
        for (auto* idx : indicesButLast) {
            object_ty = GetElementType(object_ty, idx);
        }
        // It must be a matrix
        const auto* mat_ty = object_ty->As<core::type::Matrix>();
        if (!mat_ty) {
            return {};
        }

        core::ir::Value* const to = Switch(
            store,                                                     //
            [&](core::ir::Store* s) { return s->To(); },               //
            [&](core::ir::StoreVectorElement* s) { return s->To(); },  //
            TINT_ICE_ON_NO_MATCH);

        auto* const to_ptr = to->Type()->As<core::type::Pointer>();

        // Instead of indexing the matrix dynamically, we want to replace it with a switch statement
        // over the dynamic index, with each case in [0-3] emitting the store with a constant index.
        Vector<core::ir::Instruction*, 4> new_stores;

        b.InsertBefore(store, [&] {
            // Create access to the matrix we're dynamically indexing
            core::ir::Value* matrix = to_access->Object();
            if (!indicesButLast.IsEmpty()) {
                // Matrix is in a struct or array, for example
                matrix = b.Access(ty.ptr(to_ptr->AddressSpace(), mat_ty), to_access->Object(),
                                  ToVector<4>(indicesButLast))
                             ->Result(0);
            }
            // Switch over dynamic index, emitting a case for all possible column indices
            auto* switch_ = b.Switch(to_access->Indices().Back());
            for (uint32_t i = 0; i < mat_ty->Columns(); ++i) {
                b.Append(b.Case(switch_, {b.Constant(u32(i))}), [&] {
                    auto* const vec_ty = to_ptr->StoreType();
                    auto* access = b.Access(ty.ptr(to_ptr->AddressSpace(), vec_ty), matrix, u32(i));
                    auto* new_store = Switch(
                        store,
                        [&](core::ir::Store* s) {  //
                            return b.Store(access, s->From());
                        },
                        [&](core::ir::StoreVectorElement* s) {
                            return b.StoreVectorElement(access, s->Index(), s->Value());
                        },
                        TINT_ICE_ON_NO_MATCH);
                    new_stores.Push(new_store);
                    b.ExitSwitch(switch_);
                });
            }
            b.Append(b.DefaultCase(switch_), [&] { b.ExitSwitch(switch_); });

            store->Destroy();
        });
        TINT_ASSERT(!new_stores.IsEmpty());
        return new_stores;
    }

    // Process store to matrix column
    void ProcessStore(core::ir::Store* store) {
        // Must be storing to a shader-local variable
        auto* to_ptr = store->To()->Type()->As<core::type::Pointer>();
        if (to_ptr->AddressSpace() != core::AddressSpace::kFunction &&
            to_ptr->AddressSpace() != core::AddressSpace::kPrivate) {
            return;
        }
        // Must be storing a vector
        if (!to_ptr->StoreType()->Is<core::type::Vector>()) {
            return;
        }
        // Must be storing via an access
        auto* to = store->To()->As<core::ir::InstructionResult>();
        if (!to) {
            return;
        }
        auto* to_access = to->Instruction()->As<core::ir::Access>();
        if (!to_access) {
            return;
        }
        ProcessMatrixColumnAccess(store, to_access);
    }

    // Replaces the vector element store with a full vector store that masks in the indexed
    // value. Example HLSL: vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
    void ReplaceStoreVectorElement(core::ir::StoreVectorElement* store) {
        TINT_ASSERT(!IsConstant(store->Index()));
        auto* to_ptr = store->To()->Type()->As<core::type::Pointer>();
        TINT_ASSERT(to_ptr);

        b.InsertBefore(store, [&] {
            auto* vec_param = store->Operands()[0];
            auto* index_param = store->Operands()[1];
            auto* value_param = store->Operands()[2];

            auto* vec_ty = to_ptr->StoreType()->As<core::type::Vector>();

            // e.g. "xxx" for vec3
            Vector<uint32_t, 4> swizzle_indices;
            swizzle_indices.Resize(vec_ty->Width(), 0);

            // e.g. "0,1,2" for vec3
            Vector<core::ir::Value*, 4> select_indices;
            switch (vec_ty->Width()) {
                case 2:
                    select_indices = b.Values(0_i, 1_i);
                    break;
                case 3:
                    select_indices = b.Values(0_i, 1_i, 2_i);
                    break;
                case 4:
                    select_indices = b.Values(0_i, 1_i, 2_i, 3_i);
                    break;
            }

            auto* false_val = b.Load(vec_param);
            auto* true_val = b.Construct(vec_ty, value_param);

            auto* lhs = b.Construct(vec_ty, index_param);
            auto* rhs = b.Construct(vec_ty, select_indices);
            auto* cond = b.Equal(ty.MatchWidth(ty.bool_(), vec_ty), lhs, rhs);

            // NOTE: Using Select means we depend on BuiltinPolyfill to run after this transform. We
            // could also just emit a Ternary instruction.
            auto* result = b.Call(vec_ty, core::BuiltinFn::kSelect, false_val, true_val, cond);
            b.Store(vec_param, result);

            store->Destroy();
        });
    }

    // Process store to vector element for both vector and matrix element stores
    void ProcessStoreVectorElement(core::ir::StoreVectorElement* store) {
        auto* to_ptr = store->To()->Type()->As<core::type::Pointer>();
        if (to_ptr->AddressSpace() != core::AddressSpace::kFunction &&
            to_ptr->AddressSpace() != core::AddressSpace::kPrivate) {
            return;
        }
        auto* to = store->To()->As<core::ir::InstructionResult>();
        if (!to) {
            return;
        }

        const bool is_dynamic_index = !IsConstant(store->Index());

        // This may be a vector store to a dynamically indexed matrix
        if (auto* to_access = to->Instruction()->As<core::ir::Access>()) {
            auto new_stores = ProcessMatrixColumnAccess(store, to_access);
            if (!new_stores.IsEmpty() && is_dynamic_index) {
                // The dynamic matrix index has been replaced by a switch with cases containing
                // multiple StoreVectorElement copies of the input one to this function. If
                // dynamically indexed, replace each of the new stores and return.
                for (auto* s : new_stores) {
                    ReplaceStoreVectorElement(s->As<core::ir::StoreVectorElement>());
                }
                return;
            }
        }

        // If the index is dynamic, replace
        if (is_dynamic_index) {
            ReplaceStoreVectorElement(store);
        }
    }

    /// Process the module.
    void Process() {
        // Collect stores before processing them
        Vector<core::ir::Store*, 16> stores;
        Vector<core::ir::StoreVectorElement*, 16> store_vec_elems;
        for (auto* inst : ir.Instructions()) {
            // Inline pointers
            if (auto* l = inst->As<core::ir::Let>()) {
                if (l->Result(0)->Type()->Is<core::type::Pointer>()) {
                    l->Result(0)->ReplaceAllUsesWith(l->Value());
                    l->Destroy();
                }
            }

            if (auto* store = inst->As<core::ir::Store>()) {
                stores.Push(store);
            } else if (auto* vector_store = inst->As<core::ir::StoreVectorElement>()) {
                store_vec_elems.Push(vector_store);
            }
        }

        // Process the stores
        for (auto* store : stores) {
            ProcessStore(store);
        }
        for (auto* store : store_vec_elems) {
            ProcessStoreVectorElement(store);
        }
    }
};

}  // namespace

Result<SuccessType> ReplaceNonIndexableMatVecStores(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "hlsl.ReplaceNonIndexableMatVecStores");
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::hlsl::writer::raise
