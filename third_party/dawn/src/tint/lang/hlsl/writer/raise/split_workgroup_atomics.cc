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

#include "src/tint/lang/hlsl/writer/raise/split_workgroup_atomics.h"

#include <optional>
#include <string>
#include <utility>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/vector.h"

namespace tint::hlsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// Describes the path to an atomic leaf member in a structure.
struct AtomicLeaf {
    /// The member index path from the structure root to the atomic member.
    /// For example, S.members[1].type is atomic<u32>, so path = {1}.
    /// If S.members[2].type is Inner and Inner.members[0] is atomic<i32>, path = {2, 0}.
    Vector<uint32_t, 4> member_path;

    /// The array dimensions encountered from the variable root to the atomic member.
    Vector<uint32_t, 4> array_counts;

    /// The atomic element type, such as u32 for atomic<u32>.
    const core::type::Type* inner_type = nullptr;
};

/// State for the transform.
struct State {
    explicit State(core::ir::Module& module) : ir(module) {}

    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Information about a separated atomic variable.
    struct AtomicVarInfo {
        core::ir::Var* var;
        Vector<uint32_t, 4> member_path;
    };

    /// Checks whether a type contains an atomic.
    bool ContainsAtomic(const core::type::Type* type) const {
        return tint::Switch(
            type, [&](const core::type::Atomic*) { return true; },
            [&](const core::type::Array* arr) { return ContainsAtomic(arr->ElemType()); },
            [&](const core::type::Struct* str) {
                for (auto* member : str->Members()) {
                    if (ContainsAtomic(member->Type())) {
                        return true;
                    }
                }
                return false;
            },
            [&](Default) { return false; });
    }

    /// Collects the paths to all atomic leaf members in a structure.
    void CollectAtomicLeaves(const core::type::Type* type,
                             Vector<uint32_t, 4> current_path,
                             Vector<uint32_t, 4> array_counts,
                             Vector<AtomicLeaf, 4>& leaves) {
        tint::Switch(
            type,
            [&](const core::type::Atomic* atomic) {
                leaves.Push(AtomicLeaf{current_path, array_counts, atomic->Type()});
            },
            [&](const core::type::Array* arr) {
                auto counts = array_counts;
                auto count = arr->ConstantCount();
                TINT_IR_ASSERT(ir, count);
                counts.Push(*count);
                CollectAtomicLeaves(arr->ElemType(), current_path, counts, leaves);
            },
            [&](const core::type::Struct* str) {
                for (auto* member : str->Members()) {
                    auto path = current_path;
                    path.Push(member->Index());
                    CollectAtomicLeaves(member->Type(), path, array_counts, leaves);
                }
            },
            [&](Default) {
                // There is nothing to recurse into for other types.
            });
    }

    /// Creates a type with every atomic replaced by its element type.
    const core::type::Type* CreateDeatomicizedType(const core::type::Type* type) {
        if (auto* str = type->As<core::type::Struct>()) {
            return CreateDeatomicizedStruct(str);
        }
        if (auto* arr = type->As<core::type::Array>()) {
            return CreateDeatomicizedArrayType(arr);
        }
        if (auto* atomic = type->As<core::type::Atomic>()) {
            return atomic->Type();
        }
        return type;
    }

    /// Creates a structure type with each atomic<T> replaced by T while preserving its layout.
    const core::type::Struct* CreateDeatomicizedStruct(const core::type::Struct* original) {
        // Reuse a previously created replacement structure.
        auto it = deatomicized_structs_.Get(original);
        if (it) {
            return *it;
        }

        Vector<core::type::Manager::StructMemberDesc, 4> new_members;
        for (auto* member : original->Members()) {
            auto* member_type = member->Type();

            if (ContainsAtomic(member_type)) {
                member_type = CreateDeatomicizedType(member_type);
            }

            new_members.Push({member->Name(), member_type, member->Attributes()});
        }

        auto new_name = ir.symbols.New(std::string(original->Name().NameView()) + "_data");
        auto* new_struct = ty.Struct(new_name, std::move(new_members));
        deatomicized_structs_.Add(original, new_struct);
        return new_struct;
    }

    /// Creates a replacement array type when its element type contains an atomic.
    const core::type::Type* CreateDeatomicizedArrayType(const core::type::Array* arr) {
        auto* elem = arr->ElemType();
        auto* new_elem = ContainsAtomic(elem) ? CreateDeatomicizedType(elem) : elem;

        if (new_elem == elem) {
            return arr;
        }

        if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
            return ty.runtime_array(new_elem);
        }
        return ty.array(new_elem, arr->ConstantCount().value());
    }

    /// Process the module.
    void Process() {
        if (ir.root_block->IsEmpty()) {
            return;
        }

        // Collect all workgroup variables. SplitVariable will skip variables without atomic
        // structure members while collecting their atomic leaves.
        Vector<core::ir::Var*, 4> vars_to_split;
        for (auto* inst : *ir.root_block) {
            auto* var = inst->As<core::ir::Var>();
            if (!var) {
                continue;
            }

            auto* ptr_ty = var->Result()->Type()->As<core::type::Pointer>();
            if (!ptr_ty) {
                continue;
            }

            if (ptr_ty->AddressSpace() != core::AddressSpace::kWorkgroup) {
                continue;
            }

            vars_to_split.Push(var);
        }

        // Split each collected variable.
        for (auto* var : vars_to_split) {
            SplitVariable(var);
        }
    }

    /// Splits a workgroup variable that contains atomics.
    void SplitVariable(core::ir::Var* var) {
        auto* ptr_ty = var->Result()->Type()->As<core::type::Pointer>();
        auto* store_ty = ptr_ty->StoreType();

        // Find the structure beneath any outer arrays.
        const core::type::Type* struct_ty = store_ty;
        while (auto* arr = struct_ty->As<core::type::Array>()) {
            struct_ty = arr->ElemType();
        }

        auto* original_struct = struct_ty->As<core::type::Struct>();
        if (!original_struct) {
            return;  // Skip store types that are not structures or arrays of structures.
        }

        // Collect the paths to the atomic leaves.
        Vector<AtomicLeaf, 4> atomic_leaves;
        CollectAtomicLeaves(store_ty, {}, {}, atomic_leaves);
        if (atomic_leaves.IsEmpty()) {
            return;
        }

        NormalizeAccessChains(var);

        // Create the data variable while preserving the original layout and padding.
        auto* data_store_ty = CreateDeatomicizedType(store_ty);
        auto* data_ptr_ty = ty.ptr(core::AddressSpace::kWorkgroup, data_store_ty);

        // Create a separate atomic array variable for each atomic leaf.
        Vector<AtomicVarInfo, 4> atomic_vars;

        core::ir::Var* data_var = nullptr;
        b.InsertBefore(var, [&] {
            // Create the atomic variables first.
            for (auto& leaf : atomic_leaves) {
                std::string name = BuildAtomicVarName(original_struct, leaf.member_path);

                const core::type::Type* atomic_store_ty = ty.atomic(leaf.inner_type);
                // Dimensions are collected outermost first, so rebuild them inside-out.
                for (auto count : Reverse(leaf.array_counts)) {
                    atomic_store_ty = ty.array(atomic_store_ty, count);
                }
                auto* atomic_ptr_ty = ty.ptr(core::AddressSpace::kWorkgroup, atomic_store_ty);
                auto* atomic_var = b.Var(name, atomic_ptr_ty);
                atomic_vars.Push(AtomicVarInfo{atomic_var, leaf.member_path});
            }

            // Create the non-atomic data variable.
            data_var = b.Var("data", data_ptr_ty);
        });

        // Rewrite all uses of the original variable.
        RewriteUsages(var, data_var, atomic_vars, store_ty);

        // Remove the original variable.
        var->Destroy();
    }

    /// Builds a name for an atomic variable from its structure member path.
    std::string BuildAtomicVarName(const core::type::Struct* str, const Vector<uint32_t, 4>& path) {
        std::string name;
        const core::type::Type* current = str;
        for (size_t i = 0; i < path.Length(); i++) {
            while (auto* arr = current->As<core::type::Array>()) {
                current = arr->ElemType();
            }
            auto idx = path[i];
            if (auto* s = current->As<core::type::Struct>()) {
                auto* member = s->Members()[idx];
                if (!name.empty()) {
                    name += '_';
                }
                name += member->Name().NameView();
                current = member->Type();
            }
        }
        return name;
    }

    /// Inlines pointer aliases and combines access chains derived from @p var.
    void NormalizeAccessChains(core::ir::Var* var) {
        auto usages = var->Result()->UsagesSorted();
        while (!usages.IsEmpty()) {
            auto usage = usages.Pop();
            auto* instruction = usage.instruction;
            if (!instruction->Alive()) {
                continue;
            }

            if (auto* let = instruction->As<core::ir::Let>()) {
                for (auto& child_usage : let->Result()->UsagesSorted()) {
                    usages.Push(child_usage);
                }
                let->Result()->ReplaceAllUsesWith(let->Value());
                let->Destroy();
                continue;
            }

            auto* access = instruction->As<core::ir::Access>();
            if (!access) {
                continue;
            }
            for (auto& child_usage : access->Result()->UsagesSorted()) {
                usages.Push(child_usage);
            }

            auto* object_result = access->Object()->As<core::ir::InstructionResult>();
            TINT_IR_ASSERT(ir, object_result);

            auto* parent = object_result->Instruction()->As<core::ir::Access>();
            if (!parent) {
                continue;
            }

            Vector<core::ir::Value*, 8> operands;
            operands.Push(parent->Object());
            for (auto* index : parent->Indices()) {
                operands.Push(index);
            }
            for (auto* index : access->Indices()) {
                operands.Push(index);
            }
            access->SetOperands(std::move(operands));
            if (!parent->Result()->IsUsed()) {
                parent->Destroy();
            }
        }
    }

    /// Rewrites all uses of the original variable.
    void RewriteUsages(core::ir::Var* original_var,
                       core::ir::Var* data_var,
                       const Vector<AtomicVarInfo, 4>& atomic_vars,
                       const core::type::Type* original_store_type) {
        // Snapshot the usages before modifying them.
        auto usages = original_var->Result()->UsagesSorted();

        for (auto usage : usages) {
            auto* inst = usage.instruction;
            if (!inst->Alive()) {
                continue;
            }

            tint::Switch(
                inst,
                [&](core::ir::Access* access) {
                    RewriteAccess(access, data_var, atomic_vars, original_store_type);
                },
                [&](Default) {
                    // Redirect other uses, such as loads and stores, to the data variable.
                    inst->SetOperand(usage.operand_index, data_var->Result());
                });
        }
    }

    /// Describes whether an access targets an atomic member and its array indices.
    struct AccessAnalysis {
        std::optional<size_t> leaf_index;
        Vector<core::ir::Value*, 4> array_indices;
    };

    AccessAnalysis AnalyzeAccess(core::ir::Access* access,
                                 const core::type::Type* original_store_type,
                                 const Vector<AtomicVarInfo, 4>& atomic_vars) {
        AccessAnalysis result;

        auto indices = access->Indices();

        // Trace the member path from the structure root.
        const core::type::Type* current_type = original_store_type;
        Vector<uint32_t, 4> member_path;

        for (size_t idx_pos = 0; idx_pos < indices.size(); idx_pos++) {
            if (auto* str = current_type->As<core::type::Struct>()) {
                // Structure member indices must be constants.
                auto* constant = indices[idx_pos]->As<core::ir::Constant>();
                TINT_IR_ASSERT(ir, constant);
                uint32_t member_idx = constant->Value()->ValueAs<uint32_t>();
                member_path.Push(member_idx);
                current_type = str->Members()[member_idx]->Type();
            } else if (auto* arr = current_type->As<core::type::Array>()) {
                result.array_indices.Push(indices[idx_pos]);
                current_type = arr->ElemType();
            } else {
                break;
            }
        }

        // Check whether the access chain ends at an atomic.
        if (current_type->Is<core::type::Atomic>()) {
            for (size_t i = 0; i < atomic_vars.Length(); i++) {
                if (atomic_vars[i].member_path == member_path) {
                    result.leaf_index = i;
                    break;
                }
            }
        }

        return result;
    }

    /// Rewrites an access instruction.
    void RewriteAccess(core::ir::Access* access,
                       core::ir::Var* data_var,
                       const Vector<AtomicVarInfo, 4>& atomic_vars,
                       const core::type::Type* original_store_type) {
        auto analysis = AnalyzeAccess(access, original_store_type, atomic_vars);

        if (analysis.leaf_index) {
            // Redirect an atomic member access to its separate atomic variable.
            auto* atomic_var = atomic_vars[*analysis.leaf_index].var;

            if (!analysis.array_indices.IsEmpty()) {
                auto* atomic_ptr_ty =
                    ty.ptr(core::AddressSpace::kWorkgroup,
                           access->Result()->Type()->As<core::type::Pointer>()->StoreType());

                b.InsertBefore(access, [&] {
                    auto* new_access = b.Access(atomic_ptr_ty, atomic_var, analysis.array_indices);
                    access->Result()->ReplaceAllUsesWith(new_access->Result());
                });
            } else {
                // Use the atomic variable directly when there is no outer array.
                access->Result()->ReplaceAllUsesWith(atomic_var->Result());
            }
            access->Destroy();
        } else {
            // Redirect non-atomic accesses to the data variable.
            access->SetOperand(core::ir::Access::kObjectOperandOffset, data_var->Result());

            // Update the result type to use the corresponding deatomicized store type.
            auto* ptr_ty = access->Result()->Type()->As<core::type::Pointer>();
            TINT_IR_ASSERT(ir, ptr_ty);
            auto* store_ty = CreateDeatomicizedType(ptr_ty->StoreType());
            access->Result()->SetType(ty.ptr(ptr_ty->AddressSpace(), store_ty, ptr_ty->Access()));
        }
    }

    /// Cache for deatomicized struct types.
    Hashmap<const core::type::Struct*, const core::type::Struct*, 4> deatomicized_structs_;
};

}  // namespace

Result<SuccessType> SplitWorkgroupAtomics(core::ir::Module& ir) {
    core::ir::AssertValid(ir, "before hlsl.SplitWorkgroupAtomics");

    State state{ir};
    state.Process();
    return Success;
}

}  // namespace tint::hlsl::writer::raise
