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

#include "src/tint/lang/hlsl/writer/raise/localize_struct_array_assignment.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"

namespace tint::hlsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    // Iterates through the access chain, returning the one object and ordered set of indices.
    void FlattenAccessChain(core::ir::Access* access,
                            core::ir::Value*& object,
                            Vector<core::ir::Value*, 4>& indices) {
        bool is_access = false;
        if (auto* inst_result = access->Object()->As<core::ir::InstructionResult>()) {
            if (auto* obj_access = inst_result->Instruction()->As<core::ir::Access>()) {
                FlattenAccessChain(obj_access, object, indices);
                is_access = true;
            }
        }
        if (!is_access) {
            object = access->Object();
        }
        for (auto& i : access->Indices()) {
            indices.Push(i);
        }
    }

    const core::type::Type* GetElementType(const core::type::Type* type,
                                           const core::ir::Value* idx) {
        if (auto* constant = idx->As<core::ir::Constant>()) {
            auto i = constant->Value()->ValueAs<u32>();
            return type->Element(i);
        }
        return type->Elements().type;
    }

    void ProcessStore(core::ir::Store* store) {
        // Must be storing to a shader-local variable
        auto* to_ptr = store->To()->Type()->As<core::type::Pointer>();
        if (to_ptr->AddressSpace() != core::AddressSpace::kFunction &&
            to_ptr->AddressSpace() != core::AddressSpace::kPrivate) {
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

        // Flatten the access chain
        core::ir::Value* object = nullptr;
        Vector<core::ir::Value*, 4> indices;
        FlattenAccessChain(to_access, object, indices);

        auto* type = object->Type()->As<core::type::Pointer>()->StoreType();
        if (!type->Is<core::type::Struct>()) {
            // FXC fails when the top-level object is a struct. An array of struct of array indexed
            // dynamically is fine, but a struct (of struct/array)* of array indexed dynamically is
            // not.
            return;
        }
        // Slide through the indices and look for array[dyn_index]
        for (size_t i = 0; i < indices.Length() - 1; ++i) {
            type = GetElementType(type, indices[i]);
            if (type->Is<core::type::Array>() && !indices[i + 1]->Is<core::ir::Constant>()) {
                // Found one, replace the store.
                b.InsertBefore(store, [&] {
                    // Create an access to the array in the struct to copy from
                    auto* array_access = b.Access(ty.ptr(to_ptr->AddressSpace(), type), object,
                                                  ToVector<4>(indices.Slice().Truncate(i + 1)));
                    // Copy the struct array to a local variable
                    auto* local_array = b.Var("tint_array_copy", b.Load(array_access));
                    // Store the previous store's From to the local array at the same index
                    auto* local_array_value_access =
                        b.Access(ty.ptr<function>(to_ptr->StoreType()), local_array,
                                 ToVector<4>(indices.Slice().Offset(i + 1)));
                    b.Store(local_array_value_access, store->From());
                    // Finally, copy back the data from the local array to the struct array
                    b.Store(array_access, b.Load(local_array));
                    // Destroy the old store
                    store->Destroy();
                });
                // Done
                return;
            }
        }
    }

    /// Process the module.
    void Process() {
        // Collect stores before processing them
        Vector<core::ir::Store*, 16> worklist;
        for (auto* inst : ir.Instructions()) {
            // Inline pointers
            if (auto* l = inst->As<core::ir::Let>()) {
                if (l->Result()->Type()->Is<core::type::Pointer>()) {
                    l->Result()->ReplaceAllUsesWith(l->Value());
                    l->Destroy();
                }
            }

            if (auto* store = inst->As<core::ir::Store>()) {
                worklist.Push(store);
            }
        }
        // Process the stores
        for (auto* store : worklist) {
            ProcessStore(store);
        }
    }
};

}  // namespace

Result<SuccessType> LocalizeStructArrayAssignment(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "hlsl.LocalizeStructArrayAssignment");
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::hlsl::writer::raise
