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
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/containers/vector.h"

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

    /// The `ir::Value`s which have been converted
    Hashset<core::ir::Value*, 8> converted_{};

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
                builtin_worklist.Push(builtin);
            }
        }

        for (auto* builtin : builtin_worklist) {
            switch (builtin->Func()) {
                case spirv::BuiltinFn::kAtomicLoad:
                    break;
                case spirv::BuiltinFn::kAtomicStore:
                    AtomicOp(builtin, core::BuiltinFn::kAtomicStore);
                    break;
                case spirv::BuiltinFn::kAtomicExchange:
                case spirv::BuiltinFn::kAtomicCompareExchange:
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
                case spirv::BuiltinFn::kAtomicIDecrement:
                    break;
                default:
                    TINT_UNREACHABLE() << "unknown spirv builtin: " << builtin->Func();
            }
        }

        while (!values_to_convert_.IsEmpty()) {
            auto* val = values_to_convert_.Pop();

            if (converted_.Add(val)) {
                ConvertAtomicValue(val);
            }
        }

        ProcessForkedStructs();
        ReplaceStructTypes();
    }

    void AtomicOp(spirv::ir::BuiltinCall* call, core::BuiltinFn fn) {
        auto args = call->Args();

        b.InsertBefore(call, [&] {
            auto* var = args[0];
            values_to_convert_.Push(var);

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
        auto* res = val->As<core::ir::InstructionResult>();
        TINT_ASSERT(res);

        auto* orig_ty = res->Type();
        auto* atomic_ty = AtomicTypeFor(val, orig_ty);
        res->SetType(atomic_ty);

        tint::Switch(            //
            res->Instruction(),  //
            [&](core::ir::Access* a) {
                CheckForStructForking(a);
                values_to_convert_.Push(a->Object());
            },                                                               //
            [&](core::ir::Let* l) { values_to_convert_.Push(l->Value()); },  //
            [&](core::ir::Var*) {},                                          //
            TINT_ICE_ON_NO_MATCH);
    }

    void CheckForStructForking(core::ir::Access* access) {
        auto* cur_ty = access->Object()->Type()->UnwrapPtr();
        for (auto* idx : access->Indices()) {
            tint::Switch(
                cur_ty,  //
                [&](const core::type::Struct* str) {
                    auto& forked = Fork(str);

                    auto* const_val = idx->As<core::ir::Constant>();
                    TINT_ASSERT(const_val);

                    auto const_idx = const_val->Value()->ValueAs<uint32_t>();
                    forked.atomic_members.Add(const_idx);

                    cur_ty = str->Members()[const_idx]->Type();
                },                                                                //
                [&](const core::type::Array* ary) { cur_ty = ary->ElemType(); },  //
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
                    TINT_ASSERT(res);

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
                TINT_ASSERT(count);

                return ty.array(AtomicTypeFor(val, arr->ElemType()), u32(count.value()));
            },
            [&](const core::type::Pointer* ptr) {
                return ty.ptr(ptr->AddressSpace(), AtomicTypeFor(val, ptr->StoreType()),
                              ptr->Access());
            },
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
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.Atomics",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowOverrides,
                                          });
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::reader::lower
