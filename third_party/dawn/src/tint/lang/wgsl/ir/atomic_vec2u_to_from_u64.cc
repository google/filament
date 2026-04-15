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

#include "src/tint/lang/wgsl/ir/atomic_vec2u_to_from_u64.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/u64.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/wgsl/ir/builtin_call.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/rtti/switch.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::wgsl::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;
    /// The direction of the transform.
    AtomicVec2uU64Direction direction;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The symbol table.
    tint::SymbolTable& sym{ir.symbols};

    /// Map from original type to a new type with atomic types rewritten.
    Hashmap<const core::type::Type*, const core::type::Type*, 4> rewritten_types{};

    /// Process the module.
    void Process() {
        // Find storage buffers that contain atomic types that need to be converted.
        for (auto inst : *ir.root_block) {
            if (auto* var = inst->As<core::ir::Var>()) {
                auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
                if (!ptr || ptr->AddressSpace() != core::AddressSpace::kStorage) {
                    continue;
                }
                auto* store_type = RewriteType(ptr->StoreType());
                if (store_type != ptr->StoreType()) {
                    var->Result()->SetType(ty.ptr(storage, store_type));
                    var->Result()->ForEachUseSorted(
                        [&](core::ir::Usage use) { Replace(use.instruction); });
                }
            }
        }

        for (auto func : ir.functions) {
            for (auto* param : func->Params()) {
                auto* ptr = param->Type()->As<core::type::Pointer>();
                if (!ptr || ptr->AddressSpace() != core::AddressSpace::kStorage) {
                    continue;
                }
                auto* store_type = RewriteType(ptr->StoreType());
                if (store_type != ptr->StoreType()) {
                    param->SetType(ty.ptr(storage, store_type));
                    param->ForEachUseSorted([&](core::ir::Usage use) { Replace(use.instruction); });
                }
            }
        }
    }

    void Replace(core::ir::Instruction* inst) {
        b.InsertBefore(inst, [&] {
            tint::Switch(
                inst,
                [&](core::ir::Access* access) {
                    auto* replacement = RewriteType(access->Result()->Type());
                    if (replacement != access->Result()->Type()) {
                        access->Result()->SetType(replacement);
                        access->Result()->ForEachUseSorted(
                            [&](core::ir::Usage use) { Replace(use.instruction); });
                    }
                },
                [&](core::ir::Let* let) {
                    auto* replacement = RewriteType(let->Result()->Type());
                    if (replacement != let->Result()->Type()) {
                        let->Result()->SetType(replacement);
                        let->Result()->ForEachUseSorted(
                            [&](core::ir::Usage use) { Replace(use.instruction); });
                    }
                },
                [&](wgsl::ir::BuiltinCall* call) {
                    if (direction != AtomicVec2uU64Direction::kToU64) {
                        return;
                    }
                    if (call->Func() != wgsl::BuiltinFn::kAtomicStoreMax &&
                        call->Func() != wgsl::BuiltinFn::kAtomicStoreMin) {
                        return;
                    }

                    auto* ptr_atomic = call->Args()[0];
                    auto* param_vec2u = call->Args()[1];

                    auto as_ptr = ptr_atomic->Type()->As<core::type::Pointer>();
                    TINT_ASSERT(as_ptr);
                    TINT_ASSERT(as_ptr->AddressSpace() == core::AddressSpace::kStorage);
                    auto as_store = as_ptr->StoreType()->As<core::type::Atomic>();
                    TINT_ASSERT(as_store->Type()->Is<core::type::U64>());

                    auto casted_u64 = b.Bitcast(ty.u64(), param_vec2u);

                    core::BuiltinFn core_fn = call->Func() == wgsl::BuiltinFn::kAtomicStoreMax
                                                  ? core::BuiltinFn::kAtomicStoreMax
                                                  : core::BuiltinFn::kAtomicStoreMin;

                    b.CallWithResult(call->DetachResult(), core_fn, ptr_atomic, casted_u64);
                    call->Destroy();
                },
                [&](core::ir::CoreBuiltinCall* call) {
                    if (direction != AtomicVec2uU64Direction::kFromU64) {
                        return;
                    }
                    if (call->Func() != core::BuiltinFn::kAtomicStoreMax &&
                        call->Func() != core::BuiltinFn::kAtomicStoreMin) {
                        return;
                    }

                    auto* ptr_atomic = call->Args()[0];
                    auto* param_u64 = call->Args()[1];

                    auto as_ptr = ptr_atomic->Type()->As<core::type::Pointer>();
                    TINT_ASSERT(as_ptr);
                    TINT_ASSERT(as_ptr->AddressSpace() == core::AddressSpace::kStorage);
                    auto as_store = as_ptr->StoreType()->As<core::type::Atomic>();
                    TINT_ASSERT(as_store->Type()->Is<core::type::Vector>());

                    auto* as_inst_result = param_u64->As<core::ir::InstructionResult>();
                    TINT_ASSERT(as_inst_result);
                    auto* as_inst = as_inst_result->Instruction();
                    TINT_ASSERT(as_inst);
                    auto* as_bitcast = as_inst->As<core::ir::CoreBuiltinCall>();
                    TINT_ASSERT(as_bitcast);
                    TINT_ASSERT(as_bitcast->Func() == core::BuiltinFn::kBitcast);
                    TINT_ASSERT(as_bitcast->Result()->NumUsages() == 1);
                    param_u64 = as_bitcast->Args()[0];
                    as_bitcast->Destroy();

                    wgsl::BuiltinFn wgsl_fn = call->Func() == core::BuiltinFn::kAtomicStoreMax
                                                  ? wgsl::BuiltinFn::kAtomicStoreMax
                                                  : wgsl::BuiltinFn::kAtomicStoreMin;

                    b.CallWithResult<wgsl::ir::BuiltinCall>(call->DetachResult(), wgsl_fn,
                                                            ptr_atomic, param_u64);
                    call->Destroy();
                });
        });
    }

    const core::type::Type* RewriteType(const core::type::Type* type) {
        return rewritten_types.GetOrAdd(type, [&] {
            return tint::Switch(
                type,
                [&](const core::type::Array* arr) {
                    if (arr->ConstantCount().has_value()) {
                        return ty.array(RewriteType(arr->ElemType()), arr->ConstantCount().value());
                    }
                    return ty.runtime_array(RewriteType(arr->ElemType()));
                },
                [&](const core::type::Struct* str) -> const core::type::Type* {
                    bool needs_rewrite = false;
                    Vector<const core::type::StructMember*, 4> new_members;
                    for (auto* member : str->Members()) {
                        auto* new_member_ty = RewriteType(member->Type());
                        new_members.Push(ty.Get<core::type::StructMember>(
                            member->Name(), new_member_ty, member->Index(), member->Offset(),
                            member->Align(), member->Size(), core::IOAttributes{}));
                        if (new_member_ty != member->Type()) {
                            needs_rewrite = true;
                        }
                    }

                    if (!needs_rewrite) {
                        return str;
                    }

                    auto* new_str = ty.Get<core::type::Struct>(sym.New(str->Name().Name() + "_a64"),
                                                               std::move(new_members), str->Size());
                    for (auto flag : str->StructFlags()) {
                        new_str->SetStructFlag(flag);
                    }
                    return new_str;
                },
                [&](const core::type::Pointer* ptr) {
                    return ty.ptr(ptr->AddressSpace(), RewriteType(ptr->StoreType()));
                },
                [&](const core::type::Atomic* atomic) -> const core::type::Type* {
                    if (direction == AtomicVec2uU64Direction::kToU64) {
                        auto* vec = atomic->Type()->As<core::type::Vector>();
                        if (vec && vec->Width() == 2 && vec->Type()->Is<core::type::U32>()) {
                            return ty.atomic(ty.u64());
                        }
                        return atomic;
                    }

                    if (atomic->Type()->Is<core::type::U64>()) {
                        return ty.atomic(ty.vec2u());
                    }
                    return atomic;
                },
                [&](Default) { return type; });
        });
    }
};

}  // namespace

Result<SuccessType> AtomicVec2uToFromU64(core::ir::Module& ir, AtomicVec2uU64Direction direction) {
    core::ir::AssertValid(ir,
                          core::ir::Capabilities{
                              core::ir::Capability::kAllowMultipleEntryPoints,
                              core::ir::Capability::kAllowOverrides,
                          },
                          "before transform::AtomicVec2uToFromU64");

    State{ir, direction}.Process();

    return Success;
}

}  // namespace tint::wgsl::ir::transform
