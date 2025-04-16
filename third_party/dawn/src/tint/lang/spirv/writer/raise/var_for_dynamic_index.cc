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

#include "src/tint/lang/spirv/writer/raise/var_for_dynamic_index.h"

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/containers/hashmap.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::spirv::writer::raise {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// An access that needs replacing.
    struct AccessToReplace {
        /// The access instruction.
        core::ir::Access* access = nullptr;
        /// The index of the first dynamic index.
        size_t first_dynamic_index = 0;
        /// The object type that corresponds to the source of the first dynamic index.
        const core::type::Type* dynamic_index_source_type = nullptr;
        /// If the access indexes a vector, then the type of that vector
        const core::type::Vector* vector_access_type = nullptr;
    };

    /// A partial access chain that uses constant indices to get to an object that will be
    /// dynamically indexed.
    struct PartialAccess {
        /// The base object.
        core::ir::Value* base = nullptr;
        /// The list of constant indices to get from the base to the source object.
        Vector<core::ir::Value*, 4> indices;

        /// @returns the hash code of the PartialAccess
        tint::HashCode HashCode() const { return Hash(base, indices); }

        /// An equality helper for PartialAccess.
        bool operator==(const PartialAccess& other) const {
            return base == other.base && indices == other.indices;
        }
    };

    /// Traversal action for WalkAccessChain.
    enum class Action { kStop, kContinue };

    /// Walk an the access chain @p access, calling @p callback for each intermediate type.
    template <typename CALLBACK>
    void WalkAccessChain(core::ir::Access* access, CALLBACK&& callback) {
        auto indices = access->Indices();
        auto* type = access->Object()->Type();
        for (size_t i = 0; i < indices.Length(); i++) {
            if (callback(i, indices[i], type) == Action::kStop) {
                break;
            }
            auto* const_idx = indices[i]->As<core::ir::Constant>();
            type = const_idx ? type->Element(const_idx->Value()->ValueAs<u32>())
                             : type->Elements().type;
        }
    }

    /// Check if @p access needs to be replaced.
    /// @returns the access descriptor or std::nullopt
    std::optional<AccessToReplace> ShouldReplace(core::ir::Access* access) {
        if (access->Result()->Type()->Is<core::type::Pointer>()) {
            // No need to modify accesses into pointer types.
            return {};
        }

        std::optional<AccessToReplace> result;
        WalkAccessChain(access,
                        [&](size_t i, core::ir::Value* index, const core::type::Type* type) {
                            if (auto* vec = type->As<core::type::Vector>()) {
                                // If we haven't found a dynamic index before the vector, then the
                                // transform doesn't need to hoist the access into a var as a vector
                                // value can be dynamically indexed. If we have found a dynamic
                                // index before the vector, then make a note that we're indexing a
                                // vector as we can't obtain a pointer to a vector element, so this
                                // needs to be handled specially.
                                if (result) {
                                    result->vector_access_type = vec;
                                }
                                return Action::kStop;
                            }

                            // Check if this is the first dynamic index.
                            if (!result && !index->Is<core::ir::Constant>()) {
                                result = AccessToReplace{access, i, type};
                            }

                            return Action::kContinue;
                        });

        return result;
    }

    /// Process the module.
    void Process() {
        // Find the access instructions that need replacing.
        Vector<AccessToReplace, 4> worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* access = inst->As<core::ir::Access>()) {
                if (auto to_replace = ShouldReplace(access)) {
                    worklist.Push(to_replace.value());
                }
            }
        }

        // Replace each access instruction that we recorded.
        Hashmap<core::ir::Value*, core::ir::Value*, 4> object_to_var;
        Hashmap<PartialAccess, core::ir::Value*, 4> source_object_to_value;
        for (const auto& to_replace : worklist) {
            auto* access = to_replace.access;
            auto* source_object = access->Object();

            // If the access starts with at least one constant index, extract the source of the
            // first dynamic access to avoid copying the whole object.
            if (to_replace.first_dynamic_index > 0) {
                PartialAccess partial_access = {
                    access->Object(), access->Indices().Truncate(to_replace.first_dynamic_index)};
                source_object =
                    source_object_to_value.GetOrAdd(partial_access, [&]() -> core::ir::Value* {
                        // If the source is a constant, then the partial access will also produce a
                        // constant. Extract the constant::Value and use that as the new source
                        // object.
                        if (source_object->Is<core::ir::Constant>()) {
                            for (const auto& i : partial_access.indices) {
                                auto idx =
                                    i->As<core::ir::Constant>()->Value()->ValueAs<uint32_t>();
                                source_object = b.Constant(
                                    source_object->As<core::ir::Constant>()->Value()->Index(idx));
                            }
                            return source_object;
                        }

                        // Extract a non-constant intermediate source using an access instruction
                        // that we insert immediately after the definition of the root source
                        // object.
                        auto* intermediate_source = b.Access(to_replace.dynamic_index_source_type,
                                                             source_object, partial_access.indices);
                        b.InsertInBlockAfter(source_object, [&] { b.Append(intermediate_source); });
                        return intermediate_source->Result();
                    });
            }

            // Declare a variable and copy the source object to it.
            auto* var = object_to_var.GetOrAdd(source_object, [&] {
                // If the source object is a constant we use a module-scope variable, as it could be
                // indexed by multiple functions. Otherwise, we declare a function-scope variable
                // immediately after the definition of the source object.
                core::ir::Var* decl = nullptr;
                if (source_object->Is<core::ir::Constant>()) {
                    decl = b.Var(ty.ptr(core::AddressSpace::kPrivate, source_object->Type(),
                                        core::Access::kReadWrite));
                    ir.root_block->Append(decl);
                } else {
                    b.InsertInBlockAfter(source_object, [&] {
                        decl = b.Var(ty.ptr(core::AddressSpace::kFunction, source_object->Type(),
                                            core::Access::kReadWrite));

                        // If we ever support value declarations at module-scope, we will need to
                        // modify the partial access logic above since `access` instructions cannot
                        // be used in the root block.
                        TINT_ASSERT(decl->Block() != ir.root_block);
                    });
                }

                decl->SetInitializer(source_object);
                return decl->Result();
            });

            // Create a new access instruction using the new variable as the source.
            Vector<core::ir::Value*, 4> indices{
                access->Indices().Offset(to_replace.first_dynamic_index)};
            const core::type::Type* access_type = access->Result()->Type();
            core::ir::Value* vector_index = nullptr;
            if (to_replace.vector_access_type) {
                // The old access indexed the element of a vector.
                // Its not valid to obtain the address of an element of a vector, so we need to
                // access up to the vector, then use LoadVectorElement to load the element. As a
                // vector element is always a scalar, we know the last index of the access is the
                // index on the vector. Pop that index to obtain the index to pass to
                // LoadVectorElement(), and perform the rest of the access chain.
                access_type = to_replace.vector_access_type;
                vector_index = indices.Pop();
            }

            auto addrspace = var->Type()->As<core::type::Pointer>()->AddressSpace();
            core::ir::Instruction* new_access =
                b.Access(ty.ptr(addrspace, access_type, core::Access::kReadWrite), var, indices);
            new_access->InsertBefore(access);

            core::ir::Instruction* load = nullptr;
            if (to_replace.vector_access_type) {
                load = b.LoadVectorElementWithResult(access->DetachResult(), new_access->Result(),
                                                     vector_index);
            } else {
                load = b.LoadWithResult(access->DetachResult(), new_access);
            }
            access->ReplaceWith(load);
            access->Destroy();
        }
    }
};

}  // namespace

Result<SuccessType> VarForDynamicIndex(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.VarForDynamicIndex");
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::writer::raise
