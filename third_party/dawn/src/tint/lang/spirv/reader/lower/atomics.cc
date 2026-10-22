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

#include "src/tint/lang/spirv/reader/lower/atomics.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/hashset.h"

namespace tint::spirv::reader::lower {
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

    /// The `ir::Value`s to be converted to atomics
    Vector<core::ir::Value*, 8> values_to_convert_{};

    /// The `ir::Values`s which have had their types changed, they then need to have their
    /// loads/stores updated to match. This maps to the root FunctionParam or Var for each atomic.
    Vector<core::ir::Value*, 8> values_to_fix_usages_{};

    /// Any `ir::UserCall` instructions which have atomic params which need to
    /// be updated.
    Hashset<core::ir::UserCall*, 2> user_calls_to_convert_{};

    /// The `ir::Value`s which have been converted
    Hashset<core::ir::Value*, 8> converted_{};

    /// Function to atomic replacements, this is done by hashcode since the
    /// function pointer is combined with the parameters which are converted to
    /// atomics.
    Hashmap<size_t, core::ir::Function*, 4> func_hash_to_func_{};

    struct ForkedStruct {
        const core::type::Struct* src_struct = nullptr;
        const core::type::Struct* dst_struct = nullptr;
        Hashset<size_t, 4> atomic_members;
    };

    /// Map of original structure to forked structure information
    Hashmap<const core::type::Struct*, ForkedStruct, 4> forked_structs_{};
    /// List of value objects to update with new struct types
    Hashset<core::ir::InstructionResult*, 4> values_needing_struct_update_{};

    /// Process the module.
    void Process() {
        Vector<spirv::ir::BuiltinCall*, 4> builtin_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* builtin = inst->As<spirv::ir::BuiltinCall>()) {
                switch (builtin->Func()) {
                    case spirv::BuiltinFn::kAtomicLoad:
                    case spirv::BuiltinFn::kAtomicStore:
                    case spirv::BuiltinFn::kAtomicExchange:
                    case spirv::BuiltinFn::kAtomicCompareExchange:
                    case spirv::BuiltinFn::kAtomicIAdd:
                    case spirv::BuiltinFn::kAtomicISub:
                    case spirv::BuiltinFn::kAtomicSMax:
                    case spirv::BuiltinFn::kAtomicSMin:
                    case spirv::BuiltinFn::kAtomicUMax:
                    case spirv::BuiltinFn::kAtomicUMin:
                    case spirv::BuiltinFn::kAtomicAnd:
                    case spirv::BuiltinFn::kAtomicOr:
                    case spirv::BuiltinFn::kAtomicXor:
                    case spirv::BuiltinFn::kAtomicIIncrement:
                    case spirv::BuiltinFn::kAtomicIDecrement:
                        builtin_worklist.Push(builtin);
                        break;
                    default:
                        // Ignore any unknown instruction. The `Texture` transform runs after this
                        // one which will catch any unknown instructions.
                        break;
                }
            }
        }

        for (auto* builtin : builtin_worklist) {
            values_to_convert_.Push(builtin->Args()[0]);

            switch (builtin->Func()) {
                case spirv::BuiltinFn::kAtomicLoad:
                    AtomicOpNoArgs(builtin, core::BuiltinFn::kAtomicLoad);
                    break;
                case spirv::BuiltinFn::kAtomicStore:
                    AtomicOp(builtin, core::BuiltinFn::kAtomicStore);
                    break;
                case spirv::BuiltinFn::kAtomicExchange:
                    AtomicOp(builtin, core::BuiltinFn::kAtomicExchange);
                    break;
                case spirv::BuiltinFn::kAtomicCompareExchange:
                    AtomicCompareExchange(builtin);
                    break;
                case spirv::BuiltinFn::kAtomicIAdd:
                    AtomicOp(builtin, core::BuiltinFn::kAtomicAdd);
                    break;
                case spirv::BuiltinFn::kAtomicISub:
                    AtomicOp(builtin, core::BuiltinFn::kAtomicSub);
                    break;
                case spirv::BuiltinFn::kAtomicSMax:
                    AtomicOp(builtin, core::BuiltinFn::kAtomicMax);
                    break;
                case spirv::BuiltinFn::kAtomicSMin:
                    AtomicOp(builtin, core::BuiltinFn::kAtomicMin);
                    break;
                case spirv::BuiltinFn::kAtomicUMax:
                    AtomicOp(builtin, core::BuiltinFn::kAtomicMax);
                    break;
                case spirv::BuiltinFn::kAtomicUMin:
                    AtomicOp(builtin, core::BuiltinFn::kAtomicMin);
                    break;
                case spirv::BuiltinFn::kAtomicAnd:
                    AtomicOp(builtin, core::BuiltinFn::kAtomicAnd);
                    break;
                case spirv::BuiltinFn::kAtomicOr:
                    AtomicOp(builtin, core::BuiltinFn::kAtomicOr);
                    break;
                case spirv::BuiltinFn::kAtomicXor:
                    AtomicOp(builtin, core::BuiltinFn::kAtomicXor);
                    break;
                case spirv::BuiltinFn::kAtomicIIncrement:
                    AtomicChangeByOne(builtin, core::BuiltinFn::kAtomicAdd);
                    break;
                case spirv::BuiltinFn::kAtomicIDecrement:
                    AtomicChangeByOne(builtin, core::BuiltinFn::kAtomicSub);
                    break;
                default:
                    TINT_UNREACHABLE() << "unknown spirv builtin: " << builtin->Func();
            }
        }

        // Propagate up the instruction list until we get to the root identifier
        while (!values_to_convert_.IsEmpty()) {
            auto* val = values_to_convert_.Pop();

            if (converted_.Add(val)) {
                ConvertAtomicValue(val);
            }
        }

        ProcessForkedStructs();
        ReplaceStructTypes();

        // The double loop happens because when we convert user calls, that will
        // add more values to convert, but those values can find user calls to
        // convert, so we have to work until we stabilize
        while (!values_to_fix_usages_.IsEmpty()) {
            while (!values_to_fix_usages_.IsEmpty()) {
                auto* val = values_to_fix_usages_.Pop();
                ConvertUsagesToAtomic(val);
            }

            auto user_calls = user_calls_to_convert_.Vector();
            // Sort for deterministic output
            user_calls.Sort();
            for (auto& call : user_calls) {
                ConvertUserCall(call);
            }
            user_calls_to_convert_.Clear();
        }
    }

    core::ir::Value* One(const core::type::Type* const_ty) {
        return tint::Switch(
            const_ty,  //
            [&](const core::type::I32*) { return b.Constant(1_i); },
            [&](const core::type::U32*) { return b.Constant(1_u); },  //
            TINT_ICE_ON_NO_MATCH);
    }

    void AtomicCompareExchange(spirv::ir::BuiltinCall* call) {
        auto args = call->Args();

        b.InsertBefore(call, [&] {
            auto* var = args[0];

            auto* val = args[4];
            auto* comp = args[5];

            auto* strct =
                core::type::CreateAtomicCompareExchangeResult(ty, ir.symbols, val->Type());

            auto* bi = b.Call(strct, core::BuiltinFn::kAtomicCompareExchangeWeak, var, comp, val);
            b.AccessWithResult(call->DetachResult(), bi, 0_u);
        });
        call->Destroy();
    }

    void AtomicChangeByOne(spirv::ir::BuiltinCall* call, core::BuiltinFn fn) {
        auto args = call->Args();

        b.InsertBefore(call, [&] {
            auto* var = args[0];
            auto* one = One(call->Result()->Type());
            b.CallWithResult(call->DetachResult(), fn, var, one);
        });
        call->Destroy();
    }

    void AtomicOpNoArgs(spirv::ir::BuiltinCall* call, core::BuiltinFn fn) {
        auto args = call->Args();

        b.InsertBefore(call, [&] {
            auto* var = args[0];
            b.CallWithResult(call->DetachResult(), fn, var);
        });
        call->Destroy();
    }

    void AtomicOp(spirv::ir::BuiltinCall* call, core::BuiltinFn fn) {
        auto args = call->Args();

        b.InsertBefore(call, [&] {
            auto* var = args[0];
            auto* val = args[3];
            b.CallWithResult(call->DetachResult(), fn, var, val);
        });
        call->Destroy();
    }

    void ProcessForkedStructs() {
        for (auto iter : forked_structs_) {
            auto& forked = iter.value;
            CreateForkIfNeeded(forked.src_struct);
        }
    }

    const core::type::Struct* CreateForkIfNeeded(const core::type::Struct* src_struct) {
        if (!forked_structs_.Contains(src_struct)) {
            return src_struct;
        }

        auto forked = forked_structs_.Get(src_struct);
        if (forked->dst_struct != nullptr) {
            return forked->dst_struct;
        }

        auto members = forked->src_struct->Members();
        Vector<const core::type::StructMember*, 8> new_members;
        for (size_t i = 0; i < members.Length(); ++i) {
            auto* member = members[i];
            const core::type::Type* new_member_type = nullptr;
            if (forked->atomic_members.Contains(i)) {
                new_member_type = AtomicTypeFor(nullptr, member->Type());
            } else {
                new_member_type = member->Type();
            }
            auto index = static_cast<uint32_t>(i);
            new_members.Push(ty.Get<core::type::StructMember>(
                member->Name(), new_member_type, index, member->Offset(), member->Align(),
                member->Size(), core::IOAttributes{}));
        }

        // Create a new struct with the rewritten members.
        auto name = ir.symbols.New(forked->src_struct->Name().Name() + "_atomic");
        forked->dst_struct = ty.Struct(name, std::move(new_members));

        return forked->dst_struct;
    }

    void ReplaceStructTypes() {
        for (auto iter : values_needing_struct_update_) {
            auto* orig_ty = iter->Type();
            iter->SetType(AtomicTypeFor(nullptr, orig_ty));
        }
    }

    void ConvertAtomicValue(core::ir::Value* val) {
        tint::Switch(  //
            val,       //
            [&](core::ir::InstructionResult* res) {
                auto* orig_ty = res->Type();
                auto* atomic_ty = AtomicTypeFor(val, orig_ty);
                res->SetType(atomic_ty);

                tint::Switch(
                    res->Instruction(),
                    [&](core::ir::Access* a) {
                        CheckForStructForking(a);
                        values_to_convert_.Push(a->Object());
                        values_to_fix_usages_.Push(a->Result());
                    },
                    [&](core::ir::Let* l) {
                        values_to_convert_.Push(l->Value());
                        values_to_fix_usages_.Push(l->Result());
                    },
                    [&](core::ir::Var* v) {
                        auto* var_res = v->Result();
                        values_to_fix_usages_.Push(var_res);
                    },
                    TINT_ICE_ON_NO_MATCH);
            },
            [&](core::ir::FunctionParam* param) {
                auto* orig_ty = param->Type();
                auto* atomic_ty = AtomicTypeFor(val, orig_ty);
                param->SetType(atomic_ty);
                values_to_fix_usages_.Push(param);

                for (auto& usage : param->Function()->UsagesUnsorted()) {
                    if (usage->instruction->Is<core::ir::Return>()) {
                        continue;
                    }

                    auto* call = usage->instruction->As<core::ir::Call>();
                    TINT_IR_ASSERT(ir, call);
                    values_to_convert_.Push(call->Args()[param->Index()]);
                }
            },
            TINT_ICE_ON_NO_MATCH);
    }

    void DecomposeStructLoad(core::ir::Load* ld) {
        auto* base_ptr = ld->From();
        auto* ptr_ty = base_ptr->Type()->As<core::type::Pointer>();
        auto* dst_str = ptr_ty->StoreType()->As<core::type::Struct>();
        auto* src_str = ld->Result()->Type()->As<core::type::Struct>();
        TINT_IR_ASSERT(ir, dst_str && src_str);

        Vector<core::ir::Value*, 8> member_values;
        b.InsertBefore(ld, [&] {
            for (uint32_t i = 0; i < dst_str->Members().Length(); ++i) {
                auto* member = dst_str->Members()[i];
                auto* member_ptr_ty =
                    ty.ptr(ptr_ty->AddressSpace(), member->Type(), ptr_ty->Access());
                auto* member_ptr = b.Access(member_ptr_ty, base_ptr, u32(i))->Result();

                auto* member_ld = b.Load(member_ptr);
                member_values.Push(member_ld->Result());

                // If the member type does not match between the source and
                // destination type, then the member has been modified to
                // contain an atomic. Queue the member access instruction to
                // be fixed up (so its load usages are converted to atomic).
                auto* src_member_ty = src_str->Members()[i]->Type();
                if (src_member_ty != member->Type()) {
                    member_ld->Result()->SetType(src_member_ty);
                    values_to_fix_usages_.Push(member_ptr);
                }
            }

            b.ConstructWithResult(ld->DetachResult(), std::move(member_values));
        });
        ld->Destroy();
    }

    void DecomposeStructStore(core::ir::Store* st) {
        auto* base_ptr = st->To();
        auto* src_val = st->From();
        auto* ptr_ty = base_ptr->Type()->As<core::type::Pointer>();
        auto* dst_str = ptr_ty->StoreType()->As<core::type::Struct>();
        auto* src_str = src_val->Type()->As<core::type::Struct>();
        TINT_IR_ASSERT(ir, dst_str && src_str);

        b.InsertBefore(st, [&] {
            for (uint32_t i = 0; i < dst_str->Members().Length(); ++i) {
                auto* member = dst_str->Members()[i];
                auto* member_ptr_ty =
                    ty.ptr(ptr_ty->AddressSpace(), member->Type(), ptr_ty->Access());
                auto* member_ptr = b.Access(member_ptr_ty, base_ptr, u32(i))->Result();

                auto* src_member_ty = src_str->Members()[i]->Type();
                auto* member_val = b.Access(src_member_ty, src_val, u32(i))->Result();

                b.Store(member_ptr, member_val);

                // If the member type does not match between the source and
                // destination type, then the member has been modified to
                // contain an atomic. Queue the member access instruction to
                // be fixed up (so its store usages are converted to atomic).
                if (src_member_ty != member_ptr_ty->UnwrapPtr()) {
                    values_to_fix_usages_.Push(member_ptr);
                }
            }
        });
        st->Destroy();
    }

    void DecomposeArrayLoad(core::ir::Load* ld) {
        auto* base_ptr = ld->From();
        auto* ptr_ty = base_ptr->Type()->As<core::type::Pointer>();
        auto* dst_arr = ptr_ty->StoreType()->As<core::type::Array>();
        auto* src_arr = ld->Result()->Type()->As<core::type::Array>();
        TINT_IR_ASSERT(ir, dst_arr && src_arr);

        Vector<core::ir::Value*, 8> elem_values;
        b.InsertBefore(ld, [&] {
            auto count = dst_arr->ConstantCount();
            TINT_IR_ASSERT(ir, count);
            for (uint32_t i = 0; i < *count; ++i) {
                auto* elem_ptr_ty =
                    ty.ptr(ptr_ty->AddressSpace(), dst_arr->ElemType(), ptr_ty->Access());
                auto* elem_ptr = b.Access(elem_ptr_ty, base_ptr, u32(i))->Result();

                auto* elem_ld = b.Load(elem_ptr);
                // If the element type does not match between the source and
                // destination type, then the element has been modified to
                // contain an atomic. Leave the element load's type as the
                // source type and queue the element access instruction to be
                // fixed up (so its load usages are converted to atomic).
                auto* src_elem_ty = src_arr->ElemType();
                if (src_elem_ty != elem_ptr_ty->UnwrapPtr()) {
                    elem_ld->Result()->SetType(src_elem_ty);
                    values_to_fix_usages_.Push(elem_ptr);
                }

                elem_values.Push(elem_ld->Result());
            }

            b.ConstructWithResult(ld->DetachResult(), std::move(elem_values));
        });
        ld->Destroy();
    }

    void DecomposeArrayStore(core::ir::Store* st) {
        auto* base_ptr = st->To();
        auto* src_val = st->From();
        auto* ptr_ty = base_ptr->Type()->As<core::type::Pointer>();
        auto* dst_arr = ptr_ty->StoreType()->As<core::type::Array>();
        auto* src_arr = src_val->Type()->As<core::type::Array>();
        TINT_IR_ASSERT(ir, dst_arr && src_arr);

        b.InsertBefore(st, [&] {
            auto count = dst_arr->ConstantCount();
            TINT_IR_ASSERT(ir, count);
            for (uint32_t i = 0; i < *count; ++i) {
                auto* elem_ptr_ty =
                    ty.ptr(ptr_ty->AddressSpace(), dst_arr->ElemType(), ptr_ty->Access());
                auto* elem_ptr = b.Access(elem_ptr_ty, base_ptr, u32(i))->Result();

                auto* elem_val = b.Access(src_arr->ElemType(), src_val, u32(i))->Result();

                b.Store(elem_ptr, elem_val);

                // If the element type does not match between the source and
                // destination type, then the element has been modified to
                // contain an atomic. Queue the element access instruction to
                // be fixed up (so its store usages are converted to atomic).
                if (src_arr->ElemType() != elem_ptr_ty->UnwrapPtr()) {
                    values_to_fix_usages_.Push(elem_ptr);
                }
            }
        });
        st->Destroy();
    }

    void ConvertUsagesToAtomic(core::ir::Value* val) {
        val->ForEachUseUnsorted([&](const core::ir::Usage& usage) {
            auto* inst = usage.instruction;

            tint::Switch(  //
                inst,
                [&](core::ir::Load* ld) {
                    auto* el_ty = ld->From()->Type()->UnwrapPtr();
                    if (el_ty->Is<core::type::Struct>()) {
                        DecomposeStructLoad(ld);
                    } else if (el_ty->Is<core::type::Array>()) {
                        DecomposeArrayLoad(ld);
                    } else {
                        TINT_IR_ASSERT(ir, el_ty->Is<core::type::Atomic>());

                        b.InsertBefore(ld, [&] {
                            b.CallWithResult(ld->DetachResult(), core::BuiltinFn::kAtomicLoad,
                                             ld->From());
                        });
                        ld->Destroy();
                    }
                },
                [&](core::ir::Store* st) {
                    auto* el_ty = st->To()->Type()->UnwrapPtr();
                    if (el_ty->Is<core::type::Struct>()) {
                        DecomposeStructStore(st);
                    } else if (el_ty->Is<core::type::Array>()) {
                        DecomposeArrayStore(st);
                    } else {
                        TINT_IR_ASSERT(ir, el_ty->Is<core::type::Atomic>());

                        b.InsertBefore(st, [&] {
                            b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, st->To(), st->From());
                        });
                        st->Destroy();
                    }
                },
                [&](core::ir::Access* access) {
                    auto* res = access->Result();
                    auto* new_ty = TypeForAccess(access);
                    if (new_ty == res->Type()) {
                        return;
                    }

                    res->SetType(new_ty);
                    if (converted_.Add(res)) {
                        values_to_fix_usages_.Push(res);
                    }
                },
                [&](core::ir::Let* l) {
                    auto* res = l->Result();
                    auto* orig_ty = res->Type();
                    auto* new_ty = AtomicTypeFor(nullptr, orig_ty);
                    if (new_ty == orig_ty) {
                        return;
                    }

                    res->SetType(new_ty);
                    if (converted_.Add(res)) {
                        values_to_fix_usages_.Push(res);
                    }
                },
                [&](core::ir::UserCall* uc) { user_calls_to_convert_.Add(uc); },
                [&](core::ir::CoreBuiltinCall* bc) {
                    // The only non-atomic builtin that can touch something that contains an atomic
                    // is arrayLength.
                    if (bc->Func() == core::BuiltinFn::kArrayLength) {
                        return;
                    }

                    // This was converted when we switched from a SPIR-V intrinsic to core
                    TINT_IR_ASSERT(ir, core::IsAtomic(bc->Func()));
                    TINT_IR_ASSERT(ir,
                                   bc->Args()[0]->Type()->UnwrapPtr()->Is<core::type::Atomic>());
                },
                TINT_ICE_ON_NO_MATCH);
        });
    }

    // The user calls need to check all of the parameters which were converted
    // to atomics and create a forked function call for that combination of
    // parameters.
    void ConvertUserCall(core::ir::UserCall* uc) {
        auto* target = uc->Target();
        auto& params = target->Params();
        const auto& args = uc->Args();

        Vector<size_t, 2> to_convert;
        for (size_t i = 0; i < args.size(); ++i) {
            if (params[i]->Type() != args[i]->Type()) {
                to_convert.Push(i);
            }
        }
        // Everything is already converted we're done.
        if (to_convert.IsEmpty()) {
            return;
        }

        // Hash based on the original function pointer and the specific
        // parameters we're converting.
        auto hash = Hash(target);
        hash = HashCombine(hash, to_convert);

        auto* new_fn = func_hash_to_func_.GetOrAdd(hash, [&] {
            core::ir::CloneContext ctx{ir};
            auto* fn = uc->Target()->Clone(ctx);
            ir.functions.Push(fn);

            for (auto idx : to_convert) {
                auto* p = fn->Params()[idx];
                p->SetType(args[idx]->Type());

                values_to_fix_usages_.Push(p);
            }
            return fn;
        });
        uc->SetTarget(new_fn);
    }

    const core::type::Type* TypeForAccess(core::ir::Access* access) {
        auto* ptr = access->Object()->Type()->As<core::type::Pointer>();
        TINT_IR_ASSERT(ir, ptr);

        auto* cur_ty = ptr->UnwrapPtr();
        for (auto& idx : access->Indices()) {
            tint::Switch(  //
                cur_ty,    //
                [&](const core::type::Struct* str) {
                    if (forked_structs_.Contains(str)) {
                        str = forked_structs_.Get(str)->dst_struct;
                    }

                    auto* const_val = idx->As<core::ir::Constant>();
                    TINT_IR_ASSERT(ir, const_val);

                    auto const_idx = const_val->Value()->ValueAs<uint32_t>();
                    cur_ty = str->Members()[const_idx]->Type();
                },
                [&](const core::type::Array* arr) { cur_ty = arr->ElemType(); },
                [&](const core::type::Matrix* mat) { cur_ty = mat->ColumnType(); },
                [&](const core::type::Vector* vec) { cur_ty = vec->Type(); },  //
                TINT_ICE_ON_NO_MATCH);
        }
        return ty.ptr(ptr->AddressSpace(), cur_ty, ptr->Access());
    }

    void CheckForStructForking(core::ir::Access* access) {
        auto* cur_ty = access->Object()->Type()->UnwrapPtr();
        for (auto* idx : access->Indices()) {
            tint::Switch(
                cur_ty,  //
                [&](const core::type::Struct* str) {
                    auto& forked = Fork(str);

                    auto* const_val = idx->As<core::ir::Constant>();
                    TINT_IR_ASSERT(ir, const_val);

                    auto const_idx = const_val->Value()->ValueAs<uint32_t>();
                    forked.atomic_members.Add(const_idx);

                    cur_ty = str->Members()[const_idx]->Type();
                },                                                                //
                [&](const core::type::Array* ary) { cur_ty = ary->ElemType(); },  //
                [&](const core::type::Matrix* mat) { cur_ty = mat->ColumnType(); },
                [&](const core::type::Vector* vec) { cur_ty = vec->Type(); },  //
                TINT_ICE_ON_NO_MATCH);
        }
    }

    const core::type::Type* AtomicTypeFor(core::ir::Value* val, const core::type::Type* orig_ty) {
        return tint::Switch(
            orig_ty,  //
            [&](const core::type::I32*) { return ty.atomic(orig_ty); },
            [&](const core::type::U32*) { return ty.atomic(orig_ty); },
            [&](const core::type::Struct* str) {
                // If a `val` is provided, then we're getting the atomic type for a value as we walk
                // the full instruction list. This means we can't replace structs at this point
                // because we may not have all the information about what members are atomics. So,
                // we record the type needs to be updated for `val` after we've created the structs.
                // (This works through pointers because this method is recursive, so a pointer to a
                // struct will record the type needs updating).
                //
                // In the case `val` is a nullptr then we've gathered all the needed information for
                // which members are atomics and can create the forked strut.
                if (val) {
                    auto* res = val->As<core::ir::InstructionResult>();
                    TINT_IR_ASSERT(ir, res);

                    values_needing_struct_update_.Add(res);
                    Fork(str);
                    return str;
                }

                return CreateForkIfNeeded(str);
            },
            [&](const core::type::Array* arr) {
                if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
                    return ty.runtime_array(AtomicTypeFor(val, arr->ElemType()));
                }
                auto count = arr->ConstantCount();
                TINT_IR_ASSERT(ir, count);

                return ty.array(AtomicTypeFor(val, arr->ElemType()), u32(count.value()));
            },
            [&](const core::type::Pointer* ptr) {
                return ty.ptr(ptr->AddressSpace(), AtomicTypeFor(val, ptr->StoreType()),
                              ptr->Access());
            },
            [&](const core::type::Atomic* atomic) { return atomic; },  //
            TINT_ICE_ON_NO_MATCH);
    }

    ForkedStruct& Fork(const core::type::Struct* str) {
        return forked_structs_.GetOrAdd(str, [&]() {
            ForkedStruct forked;
            forked.src_struct = str;
            return forked;
        });
    }
};

}  // namespace

Result<SuccessType> Atomics(core::ir::Module& ir) {
    AssertValid(ir, "before spirv.Atomics");

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::reader::lower
