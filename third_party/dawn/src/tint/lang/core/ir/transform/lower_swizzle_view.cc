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

#include "src/tint/lang/core/ir/transform/lower_swizzle_view.h"

#include <utility>

#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/swizzle_view.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/containers/reverse.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// State holds the transform's internal state.
struct State {
    /// The IR module to transform.
    core::ir::Module& ir;
    /// The IR builder.
    core::ir::Builder b{ir};
    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Processes the module and lowers all swizzled loads/stores.
    /// @returns success or failure
    diag::Result<SuccessType> Process() {
        tint::Vector<core::ir::Store*, 16> stores;
        tint::Vector<core::ir::Load*, 16> loads;
        tint::Vector<core::ir::Instruction*, 16> swizzle_views;

        // Collect all target stores, source loads, and swizzle views.
        for (auto* inst : ir.Instructions()) {
            tint::Switch(
                inst,
                [&](core::ir::Store* store) {
                    if (store->To()->Type()->Is<core::type::SwizzleView>()) {
                        stores.Push(store);
                    }
                },
                [&](core::ir::Load* load) {
                    if (load->From()->Type()->Is<core::type::SwizzleView>()) {
                        loads.Push(load);
                    }
                },
                [&](core::ir::Swizzle* swizzle) {
                    if (swizzle->Result()->Type()->Is<core::type::SwizzleView>()) {
                        swizzle_views.Push(swizzle);
                    }
                },
                [&](core::ir::Access* access) {
                    if (access->Result()->Type()->Is<core::type::SwizzleView>()) {
                        swizzle_views.Push(access);
                    }
                });
        }

        // Lower collected stores.
        for (auto* store : stores) {
            LowerStore(store);
        }

        // Lower collected loads.
        for (auto* load : loads) {
            LowerLoad(load);
        }

        // Perform a backward sweep to destroy unused swizzle view instructions.
        for (auto* inst : tint::Reverse(swizzle_views)) {
            if (inst->Result(0)->NumUsages() != 0) {
                diag::Diagnostic error{};
                error.severity = diag::Severity::Error;
                error.source = ir.SourceOf(inst);
                error << "swizzle view instruction still has usages after lowering";
                return diag::Failure{std::move(error)};
            }
            inst->Destroy();
        }

        // SwizzleViews not allowed on IR modules after this pass.
        ir.properties.Remove(core::ir::Property::kAllowSwizzleView);
        return Success;
    }

  private:
    /// Lowers a load from a swizzle view.
    /// @param load the load instruction to lower
    void LowerLoad(core::ir::Load* load) {
        auto* inst = load->From()->As<core::ir::InstructionResult>()->Instruction();
        auto* swizzle = inst->As<core::ir::Swizzle>();

        // If loading through an accessor on a swizzle view (i.e. v.zyx[0]), extract the accessor
        // index first.
        core::ir::Value* accessor_idx = nullptr;
        if (auto* access = inst->As<core::ir::Access>()) {
            accessor_idx = access->Indices()[0];
            swizzle = access->Object()
                          ->As<core::ir::InstructionResult>()
                          ->Instruction()
                          ->As<core::ir::Swizzle>();
        }

        auto collapsed = Collapse(swizzle);

        b.InsertBefore(load, [&] {
            core::ir::InstructionResult* new_result = nullptr;
            if (accessor_idx || collapsed.indices.Length() == 1) {
                // Lowers to a single vector element load.
                auto* idx = accessor_idx ? GetTargetIndex(accessor_idx, collapsed.indices)
                                         : b.Constant(u32(collapsed.indices[0]));
                new_result = b.LoadVectorElement(collapsed.vector, idx)->Result();
            } else {
                // Extract the target elements from the loaded vector.
                auto* loaded_vec = b.Load(collapsed.vector);
                new_result =
                    b.Swizzle(load->Result()->Type(), loaded_vec, collapsed.indices)->Result();
            }
            load->Result()->ReplaceAllUsesWith(new_result);
        });

        load->Destroy();
    }

    /// Lowers a store to a swizzle view.
    /// @param store the store instruction to lower
    void LowerStore(core::ir::Store* store) {
        auto* swizzle = store->To()->As<core::ir::InstructionResult>();
        auto* rhs = store->From();

        // If storing through an accessor on a swizzle view (e.g. v.zyx[0] = rhs),
        // extract the accessor index first.
        core::ir::Value* accessor_idx = nullptr;
        if (auto* access = swizzle->Instruction()->As<core::ir::Access>()) {
            swizzle = access->Object()->As<core::ir::InstructionResult>();
            accessor_idx = access->Indices()[0];
        }

        auto collapsed = Collapse(As<core::ir::Swizzle>(swizzle->Instruction()));

        b.InsertBefore(store, [&] {
            if (accessor_idx || collapsed.indices.Length() == 1) {
                // Lowers to a single vector element store.
                auto* idx = accessor_idx ? GetTargetIndex(accessor_idx, collapsed.indices)
                                         : b.Constant(u32(collapsed.indices[0]));
                b.StoreVectorElement(collapsed.vector, idx, rhs);
            } else {
                auto* vec_ty = collapsed.vector->Type()
                                   ->As<core::type::Pointer>()
                                   ->StoreType()
                                   ->As<core::type::Vector>();
                // Load the old vector value.
                auto* old_vec = b.Load(collapsed.vector);

                // Reserve the result vector which will eventually be stored.
                tint::Vector<core::ir::Value*, 4> new_vec_args;
                new_vec_args.Resize(vec_ty->Width());

                // For indices that are referenced in the swizzle, use the appropriate new vals from
                // the rhs to populate the result vector.
                for (size_t i = 0; i < collapsed.indices.Length(); i++) {
                    auto* access = b.Access(vec_ty->Type(), rhs, b.Constant(u32(i)));
                    uint32_t target_index = collapsed.indices[i];
                    new_vec_args[target_index] = access->Result();
                }

                // For indices which were not referenced in the swizzle, fill in the old vals from
                // the loaded lhs vector.
                for (uint32_t i = 0; i < vec_ty->Width(); i++) {
                    if (new_vec_args[i] == nullptr) {
                        auto* access = b.Access(vec_ty->Type(), old_vec, b.Constant(u32(i)));
                        new_vec_args[i] = access->Result();
                    }
                }

                b.Store(collapsed.vector, b.Construct(vec_ty, std::move(new_vec_args)));
            }
        });

        store->Destroy();
    }

    /// CollapsedSwizzle holds the collapsed root pointer and accumulated indices.
    struct CollapsedSwizzle {
        /// The root vector pointer.
        core::ir::Value* vector = nullptr;
        /// The collapsed indices relative to the root vector.
        tint::Vector<uint32_t, 4> indices;
    };

    /// Recursively collapses nested swizzles down to the innermost vector.
    /// @param swizzle the swizzle instruction
    /// @returns the collapsed root and indices
    CollapsedSwizzle Collapse(core::ir::Swizzle* swizzle) {
        CollapsedSwizzle result;
        result.indices = swizzle->Indices();
        auto* obj = swizzle->Object();

        while (auto* inner_res = obj->As<core::ir::InstructionResult>()) {
            auto* inner_swizzle = inner_res->Instruction()->As<core::ir::Swizzle>();
            if (!inner_swizzle) {
                break;
            }

            tint::Vector<uint32_t, 4> combined;
            for (uint32_t i : result.indices) {
                combined.Push(inner_swizzle->Indices()[i]);
            }
            result.indices = std::move(combined);
            obj = inner_swizzle->Object();
        }

        result.vector = obj;
        return result;
    }

    /// Maps an accessor index dynamically or statically to the target swizzle vector element index.
    /// @param accessor_idx the accessor index (can be static constant or dynamic)
    /// @param indices the collapsed swizzle indices
    /// @returns the mapped index value
    core::ir::Value* GetTargetIndex(core::ir::Value* accessor_idx,
                                    const tint::Vector<uint32_t, 4>& indices) {
        // Index is constant.
        if (auto* const_idx = accessor_idx->As<core::ir::Constant>()) {
            uint32_t extra_idx = const_idx->Value()->ValueAs<uint32_t>();
            return b.Constant(u32(indices[extra_idx]));
        }

        // Index is dynamic, and must be mapped into a composite array at runtime.
        tint::Vector<const core::constant::Value*, 4> const_indices;
        for (uint32_t idx : indices) {
            const_indices.Push(b.ConstantValue(u32(idx)));
        }
        auto* arr_ty = ty.array(ty.u32(), static_cast<uint32_t>(indices.Length()));
        auto* arr_val = b.Composite(arr_ty, std::move(const_indices));
        return b.Access(ty.u32(), arr_val, accessor_idx)->Result();
    }
};

}  // namespace

Result<SuccessType> LowerSwizzleView(core::ir::Module& ir) {
    auto res = State{ir}.Process();
    if (res != Success) {
        return Failure{res.Failure().reason.Str()};
    }
    return Success;
}

}  // namespace tint::core::ir::transform
