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

#include "src/tint/lang/spirv/reader/lower/vector_element_pointer.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

namespace tint::spirv::reader::lower {

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

    /// Access is an access instruction and the type of the vector that it produces a pointer to.
    struct Access {
        /// The access instruction.
        core::ir::Access* inst;
        /// The vector type being accessed.
        const core::type::Type* type;
    };

    /// Process the module.
    void Process() {
        // Find the access instructions that need to be replaced.
        Vector<Access, 8> worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* access = inst->As<core::ir::Access>()) {
                auto* source_ty = access->Object()->Type();
                if (!source_ty->Is<core::type::Pointer>()) {
                    continue;
                }
                source_ty = source_ty->UnwrapPtr();

                // Step through the indices of the access instruction to check for vector types.
                for (auto* idx : access->Indices()) {
                    if (source_ty->Is<core::type::Vector>()) {
                        // Found an access that is indexing into a vector pointer.
                        worklist.Push(Access{access, source_ty});
                        break;
                    }

                    // Update the current source type based on the next index.
                    if (auto* constant = idx->As<core::ir::Constant>()) {
                        auto i = constant->Value()->ValueAs<u32>();
                        source_ty = source_ty->Element(i);
                    } else {
                        source_ty = source_ty->Elements().type;
                    }
                }
            }
        }

        // Replace the access instructions that we found.
        for (const auto& access : worklist) {
            ReplaceAccess(access);
        }
    }

    /// Replace an access instruction with {load,store}_vector_element instructions.
    /// @param access the access instruction to replace
    void ReplaceAccess(const Access& access) {
        auto* object = access.inst->Object();

        if (access.inst->Indices().Length() > 1) {
            // Create a new access instruction that stops at the vector pointer.
            Vector<core::ir::Value*, 8> partial_indices{access.inst->Indices()};
            partial_indices.Pop();
            auto addrspace = object->Type()->As<core::type::Pointer>()->AddressSpace();
            auto* access_to_vec = b.Access(ty.ptr(addrspace, access.type), object, partial_indices);
            access_to_vec->InsertBefore(access.inst);

            object = access_to_vec->Result();
        }

        // Replace all uses of the original access instruction.
        auto* index = access.inst->Indices().Back();
        ReplaceAccessUses(access.inst, object, index);

        // Destroy the original access instruction.
        access.inst->Destroy();
    }

    /// Replace all uses of an access instruction with {load,store}_vector_element instructions.
    /// @param access the access instruction to replace
    /// @param object the pointer-to-vector source object
    /// @param index the index of the vector element
    void ReplaceAccessUses(core::ir::Access* access,
                           core::ir::Value* object,
                           core::ir::Value* index) {
        Vector<core::ir::Instruction*, 4> to_destroy;
        access->Result()->ForEachUseUnsorted([&](core::ir::Usage use) {
            Switch(
                use.instruction,
                [&](core::ir::Load* load) {
                    auto* lve = b.LoadVectorElementWithResult(load->DetachResult(), object, index);
                    lve->InsertBefore(load);
                    to_destroy.Push(load);
                },
                [&](core::ir::Store* store) {
                    auto* sve = b.StoreVectorElement(object, index, store->From());
                    sve->InsertBefore(store);
                    to_destroy.Push(store);
                },
                TINT_ICE_ON_NO_MATCH);
        });

        // Clean up old instructions.
        for (auto* inst : to_destroy) {
            inst->Destroy();
        }
    }
};

}  // namespace

Result<SuccessType> VectorElementPointer(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.VectorElementPointer",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowOverrides,
                                              core::ir::Capability::kAllowVectorElementPointer,
                                          });
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::reader::lower
