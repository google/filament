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

#include "src/tint/lang/msl/writer/raise/polyfill_bool_vector_dynamic_stores.h"

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/ice/ice.h"

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

    /// Process the module.
    void Process() {
        // Collect all stores to be polyfilled.
        Vector<core::ir::StoreVectorElement*, 16> stores;
        for (auto* inst : ir.Instructions()) {
            if (auto* sve = inst->As<core::ir::StoreVectorElement>()) {
                // We only care about dynamic indices on boolean vectors.
                if (!sve->Index()->Is<core::ir::Constant>()) {
                    auto* to_ptr = sve->To()->Type()->As<core::type::Pointer>();
                    if (to_ptr && to_ptr->StoreType()->IsBoolVector()) {
                        stores.Push(sve);
                    }
                }
            }
        }

        // Replace dynamic vector element store:
        //     vec_param[index_param] = value_param
        //
        // with a branchless select-based whole vector write operation:
        //     let orig_val = load(vec_param)
        //     let new_val = construct(value_param) // broadcast to vector
        //     let cond = (construct(index_param) == construct(0, 1, ...))
        //     let result = select(orig_val, new_val, cond)
        //     store(vec_param, result)
        //
        // This is semantically equivalent to a dynamic element store. For each component `i`:
        // - If index_param == i, cond[i] is true, selecting new_val[i] (which is value_param).
        // - If index_param != i, cond[i] is false, selecting orig_val[i] (the original element
        // value).
        for (auto* store : stores) {
            b.InsertBefore(store, [&] {
                auto* vec_param = store->To();
                auto* vec_ty = vec_param->Type()
                                   ->As<core::type::Pointer>()
                                   ->StoreType()
                                   ->As<core::type::Vector>();

                auto select_indices = b.Values(0_u, 1_u, 2_u, 3_u);
                select_indices.Resize(vec_ty->Width());

                auto* orig_val = b.Load(vec_param);
                auto* new_val = b.Construct(vec_ty, store->Value());

                auto* uint_vec_ty = ty.MatchWidth(ty.u32(), vec_ty);
                auto* lhs =
                    b.Construct(uint_vec_ty, b.InsertConvertIfNeeded(ty.u32(), store->Index()));
                auto* rhs = b.Construct(uint_vec_ty, select_indices);
                auto* cond = b.Equal(lhs, rhs);

                auto* result = b.Call(vec_ty, core::BuiltinFn::kSelect, orig_val, new_val, cond);
                b.Store(vec_param, result);

                store->Destroy();
            });
        }
    }
};

}  // namespace

Result<SuccessType> PolyfillBoolVectorDynamicStores(core::ir::Module& ir) {
    AssertValid(ir, "before msl.PolyfillBoolVectorDynamicStores");

    State{ir}.Process();

    return Success;
}

}  // namespace tint::msl::writer::raise
