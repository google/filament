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

#include "src/tint/lang/core/ir/transform/zero_init_workgroup_memory.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/containers/reverse.h"

#if TINT_BUILD_IS_MSVC
#if _MSC_VER > 1930 && _MSC_VER < 1939
// MSVC raises an internal compiler error in Vector::Sort(), when optimizations are enabled.
// Later versions are fixed.
TINT_BEGIN_DISABLE_OPTIMIZATIONS();
#endif
#endif

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module& ir;

    /// The IR builder.
    Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The mapping from functions to their transitively referenced workgroup variables.
    ReferencedModuleVars<Module> referenced_module_vars_{
        ir, [](const Var* var) {
            auto* view = var->Result()->Type()->As<type::MemoryView>();
            return view && view->AddressSpace() == AddressSpace::kWorkgroup;
        }};

    /// ArrayIndex represents a required array index for an access instruction.
    struct ArrayIndex {
        /// The size of the array that will be indexed.
        uint32_t count = 0u;
    };

    /// Index represents an index for an access instruction, which is either a constant value or
    /// an array index that will be dynamically calculated from an array size.
    using Index = std::variant<uint32_t, ArrayIndex>;

    /// Store describes a store to a sub-element of a workgroup variable.
    struct Store {
        /// The workgroup variable.
        Var* var = nullptr;
        /// The store type of the element.
        const type::Type* store_type = nullptr;
        /// The list of index operands to get to the element.
        Vector<Index, 4> indices;
    };

    /// StoreList is a list of `Store` descriptors.
    using StoreList = Vector<Store, 8>;

    /// StoreMap is a map from iteration count to a list of `Store` descriptors.
    using StoreMap = Hashmap<uint32_t, StoreList, 8>;

    /// Process the module.
    void Process() {
        if (ir.root_block->IsEmpty()) {
            return;
        }
        // Process each entry point function.
        for (auto& func : ir.functions) {
            if (func->Stage() == Function::PipelineStage::kCompute) {
                ProcessEntryPoint(func);
            }
        }
    }

    /// Process an entry point function to zero-initialize the workgroup variables that it uses.
    /// @param func the entry point function
    void ProcessEntryPoint(Function* func) {
        // Get list of transitively referenced workgroup variables.
        const auto& vars = referenced_module_vars_.TransitiveReferences(func);
        if (vars.IsEmpty()) {
            return;
        }

        // Build list of store descriptors for all workgroup variables.
        StoreMap stores;
        for (auto* var : vars) {
            PrepareStores(var, var->Result()->Type()->UnwrapPtr(), 1, {}, stores);
        }

        // Sort the iteration counts to get deterministic output in tests.
        auto sorted_iteration_counts = stores.Keys();
        sorted_iteration_counts.Sort();

        // Capture the first instruction of the function.
        // All new instructions will be inserted before this.
        auto* function_start = func->Block()->Front();

        // Get the local invocation index and the linearized workgroup size.
        auto* local_index = GetLocalInvocationIndex(func);

        auto wgsizes = func->WorkgroupSizeAsConst();
        TINT_ASSERT(wgsizes);
        auto wgsize = wgsizes.value()[0] * wgsizes.value()[1] * wgsizes.value()[2];

        // Insert instructions to zero-initialize every variable.
        b.InsertBefore(function_start, [&] {
            for (auto count : sorted_iteration_counts) {
                auto element_stores = stores.Get(count);
                TINT_ASSERT(count);
                // No loop is required if we have at least as many invocations than counts.
                if (count <= wgsize) {
                    // Make the first |count| invocations in the group perform the arrayed stores.
                    auto* ifelse = b.If(b.LessThan(ty.bool_(), local_index, u32(count)));
                    b.Append(ifelse->True(), [&] {
                        for (auto& store : *element_stores) {
                            GenerateStore(store, count, local_index);
                        }
                        b.ExitIf(ifelse);
                    });
                } else {
                    // Use a loop for arrayed stores that exceed the wgsize
                    b.LoopRange(ty, local_index, u32(count), u32(wgsize), [&](Value* index) {
                        for (auto& store : *element_stores) {
                            GenerateStore(store, count, index);
                        }
                    });
                }
            }
            b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        });
    }

    /// Recursively generate store descriptors for a workgroup variable.
    /// Determines the combined array iteration count of each inner element.
    /// @param var the workgroup variable
    /// @param type the current element type
    /// @param iteration_count the iteration count of this inner element of the variable
    /// @param indices the access indices needed to get to this element
    /// @param stores the map of stores to populate
    void PrepareStores(Var* var,
                       const type::Type* type,
                       uint32_t iteration_count,
                       Vector<Index, 4> indices,
                       StoreMap& stores) {
        // If this type can be trivially zeroed, store to the whole element.
        if (CanTriviallyZero(type)) {
            stores.GetOrAddZero(iteration_count).Push(Store{var, type, indices});
            return;
        }

        tint::Switch(
            type,
            [&](const type::Array* arr) {
                // Add an array index to the list and recurse into the element type.
                TINT_ASSERT(arr->ConstantCount());
                auto count = arr->ConstantCount().value();
                auto new_indices = indices;
                if (count > 1) {
                    new_indices.Push(ArrayIndex{count});
                } else {
                    new_indices.Push(0u);
                }
                PrepareStores(var, arr->ElemType(), iteration_count * count, new_indices, stores);
            },
            [&](const type::Atomic*) {
                stores.GetOrAddZero(iteration_count).Push(Store{var, type, indices});
            },
            [&](const type::Struct* str) {
                for (auto* member : str->Members()) {
                    // Add the member index to the index list and recurse into its type.
                    auto new_indices = indices;
                    new_indices.Push(member->Index());
                    PrepareStores(var, member->Type(), iteration_count, new_indices, stores);
                }
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Get or inject an entry point builtin for the local invocation index.
    /// @param func the entry point function
    /// @returns the local invocation index builtin
    Value* GetLocalInvocationIndex(Function* func) {
        // Look for an existing local_invocation_index builtin parameter.
        for (auto* param : func->Params()) {
            if (auto* str = param->Type()->As<type::Struct>()) {
                // Check each member for the local invocation index builtin attribute.
                for (auto* member : str->Members()) {
                    if (member->Attributes().builtin == BuiltinValue::kLocalInvocationIndex) {
                        auto* access = b.Access(ty.u32(), param, u32(member->Index()));
                        access->InsertBefore(func->Block()->Front());
                        return access->Result();
                    }
                }
            } else {
                // Check if the parameter is the local invocation index.
                if (param->Builtin() == BuiltinValue::kLocalInvocationIndex) {
                    return param;
                }
            }
        }

        // No local invocation index was found, so add one to the parameter list and use that.
        auto* param = b.FunctionParam("tint_local_index", ty.u32());
        func->AppendParam(param);
        param->SetBuiltin(BuiltinValue::kLocalInvocationIndex);
        return param;
    }

    /// Generate the store instruction for a given store descriptor.
    /// @param store the store descriptor
    /// @param total_count the total number of elements that will be zeroed
    /// @param linear_index the linear index of the single element that will be zeroed
    void GenerateStore(const Store& store, uint32_t total_count, Value* linear_index) {
        auto* to = store.var->Result();
        if (!store.indices.IsEmpty()) {
            // Build the access indices to get to the target element.
            // We walk backwards along the index list so that adjacent invocation store to
            // adjacent array elements.
            uint32_t count = 1;
            Vector<Value*, 4> indices;
            for (auto idx : Reverse(store.indices)) {
                if (std::holds_alternative<ArrayIndex>(idx)) {
                    // Array indices are computed from the linear index based on the size of the
                    // array and the size of the sub-arrays that have already been indexed.
                    auto array_index = std::get<ArrayIndex>(idx);
                    Value* index = linear_index;
                    if (count > 1) {
                        index = b.Divide(ty.u32(), index, u32(count))->Result();
                    }
                    if (total_count > count * array_index.count) {
                        index = b.Modulo(ty.u32(), index, u32(array_index.count))->Result();
                    }
                    indices.Push(index);
                    count *= array_index.count;
                } else {
                    // Constant indices are added to the list unmodified.
                    indices.Push(b.Constant(u32(std::get<uint32_t>(idx))));
                }
            }
            indices.Reverse();
            to = b.Access(ty.ptr(workgroup, store.store_type), to, indices)->Result();
        }

        // Generate the store instruction.
        if (auto* atomic = store.store_type->As<type::Atomic>()) {
            auto* zero = b.Constant(ir.constant_values.Zero(atomic->Type()));
            b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, to, zero);
        } else {
            auto* zero = b.Constant(ir.constant_values.Zero(store.store_type));
            b.Store(to, zero);
        }
    }

    /// Check if a type can be efficiently zeroed with a single store. Returns `false` if there are
    /// any nested arrays or atomics.
    /// @param type the type to inspect
    /// @returns true if a variable with store type @p ty can be efficiently zeroed
    bool CanTriviallyZero(const core::type::Type* type) {
        if (type->IsAnyOf<core::type::Atomic, core::type::Array>()) {
            return false;
        }
        if (auto* str = type->As<core::type::Struct>()) {
            for (auto* member : str->Members()) {
                if (!CanTriviallyZero(member->Type())) {
                    return false;
                }
            }
        }
        return true;
    }
};

}  // namespace

Result<SuccessType> ZeroInitWorkgroupMemory(Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.ZeroInitWorkgroupMemory");
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
