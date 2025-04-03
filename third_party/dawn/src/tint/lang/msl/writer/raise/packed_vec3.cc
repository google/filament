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

#include "src/tint/lang/msl/writer/raise/packed_vec3.h"

#include <cstdint>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/msl/ir/builtin_call.h"

namespace tint::msl::writer::raise {
namespace {

using namespace tint::core::fluent_types;  // NOLINT

/// Arrays larger than this will be packed/unpacked with a for loop.
/// Arrays up to this size will be packed/unpacked with a sequence of instructions.
static constexpr uint32_t kMaxSeriallyUnpackedArraySize = 8;

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The symbol table.
    SymbolTable& sym{ir.symbols};

    /// Map from original type to a new type that uses packed vectors.
    Hashmap<const core::type::Type*, const core::type::Type*, 4> rewritten_types{};

    /// Map from a scalar type to the structure that can be used for a packed vector of that type
    /// when inside an array.
    Hashmap<const core::type::Type*, const core::type::Struct*, 4> packed_array_element_types{};

    // A map from a packed pointer type to a helper function that will load it to an unpacked type.
    Hashmap<const core::type::Pointer*, core::ir::Function*, 4> packed_load_helpers{};

    // A map from a packed pointer type to a helper function that will store an unpacked type to it.
    Hashmap<const core::type::Pointer*, core::ir::Function*, 4> packed_store_helpers{};

    /// Process the module.
    void Process() {
        // Find all module-scope variables that contain vec3 types in host-shareable address spaces
        // and update them to use packed vec3 types instead.
        for (auto* inst : *ir.root_block) {
            auto* var = inst->As<core::ir::Var>();
            if (!var) {
                continue;
            }
            auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
            if (!AddressSpaceNeedsPacking(ptr->AddressSpace())) {
                continue;
            }

            // Rewrite the type, and if anything changed update the variable and its usages.
            auto* packed_store_type = RewriteType(ptr->StoreType());
            if (packed_store_type != ptr->StoreType()) {
                auto* new_ptr = ty.ptr(ptr->AddressSpace(), packed_store_type, ptr->Access());
                var->Result()->SetType(new_ptr);
                var->Result()->ForEachUseSorted([&](core::ir::Usage use) {  //
                    UpdateUsage(use, ptr->StoreType(), packed_store_type);
                });
            }
        }

        // Find all function parameters that contain vec3 types in host-shareable address spaces and
        // update them to use packed vec3 types instead.
        for (auto func : ir.functions) {
            for (auto* param : func->Params()) {
                auto* ptr = param->Type()->As<core::type::Pointer>();
                if (!ptr || !AddressSpaceNeedsPacking(ptr->AddressSpace())) {
                    continue;
                }

                // Rewrite the type, and if anything changed update the parameter and its usages.
                auto* packed_store_type = RewriteType(ptr->StoreType());
                if (packed_store_type != ptr->StoreType()) {
                    auto* new_ptr = ty.ptr(ptr->AddressSpace(), packed_store_type, ptr->Access());
                    param->SetType(new_ptr);
                    param->ForEachUseSorted([&](core::ir::Usage use) {  //
                        UpdateUsage(use, ptr->StoreType(), packed_store_type);
                    });
                }
            }
        }
    }

    /// @returns true if @p addrspace requires vec3 types to be packed
    bool AddressSpaceNeedsPacking(core::AddressSpace addrspace) {
        // Host-shareable address spaces need to be packed to match the memory layout on the host.
        // The workgroup address space needs to be packed so that the size of generated threadgroup
        // variables matches the size of the original WGSL declarations.
        return core::IsHostShareable(addrspace) || addrspace == core::AddressSpace::kWorkgroup;
    }

    /// Rewrite a type if necessary, decomposing contained matrices.
    /// @param type the type to rewrite
    /// @returns the new type, or the original type if no changes were needed
    const core::type::Type* RewriteType(const core::type::Type* type) {
        return rewritten_types.GetOrAdd(type, [&] {
            return tint::Switch(
                type,
                [&](const core::type::Array* arr) {  //
                    return RewriteArray(arr);
                },
                [&](const core::type::Matrix* mat) -> const core::type::Type* {
                    if (mat->Rows() == 3) {
                        return ty.array(GetPackedVec3ArrayElementStruct(mat->Type()),
                                        mat->Columns());
                    }
                    return mat;
                },
                [&](const core::type::Pointer* ptr) {
                    auto* store_type = RewriteType(ptr->StoreType());
                    if (store_type != ptr->StoreType()) {
                        return ty.ptr(ptr->AddressSpace(), store_type, ptr->Access());
                    }
                    return ptr;
                },
                [&](const core::type::Struct* str) {  //
                    return RewriteStruct(str);
                },
                [&](const core::type::Vector* vec) {
                    if (vec->Width() == 3) {
                        return ty.packed_vec(vec->Type(), 3);
                    }
                    return vec;
                },
                [&](Default) {
                    // This type cannot contain a vec3, so no changes needed.
                    return type;
                });
        });
    }

    /// @param arr the array type to rewrite if necessary
    /// @returns the new type, or the original type if no changes were needed
    const core::type::Array* RewriteArray(const core::type::Array* arr) {
        // If the element type is a vec3, we need to wrap it in a structure to give it the correct
        // alignment. Otherwise, just recurse and rewrite it.
        const core::type::Type* new_elem_type = nullptr;
        auto* vec = arr->ElemType()->As<core::type::Vector>();
        if (vec && vec->Width() == 3) {
            new_elem_type = GetPackedVec3ArrayElementStruct(vec->Type());
        } else {
            new_elem_type = RewriteType(arr->ElemType());
        }

        if (new_elem_type == arr->ElemType()) {
            // No changes needed.
            return arr;
        }

        if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
            return ty.runtime_array(new_elem_type);
        } else if (auto count = arr->ConstantCount()) {
            return ty.array(new_elem_type, u32(count.value()));
        }
        TINT_UNREACHABLE();
    }

    /// @param str the struct type to rewrite if necessary
    /// @returns the new type, or the original type if no changes were needed
    const core::type::Struct* RewriteStruct(const core::type::Struct* str) {
        // Recursively rewrite the type of a each member and make a note of whether anything needed
        // to be changed.
        bool has_changed = false;
        Vector<const core::type::StructMember*, 4> new_members;
        new_members.Reserve(str->Members().Length());
        for (auto* member : str->Members()) {
            auto* new_member_type = RewriteType(member->Type());
            if (new_member_type != member->Type()) {
                has_changed = true;
            }

            // Create a struct member with the new type, taking all the layout properties of the
            // original member. No IO attributes should be present on host-shareable structures.
            new_members.Push(ty.Get<core::type::StructMember>(
                member->Name(), new_member_type, static_cast<uint32_t>(new_members.Length()),
                member->Offset(), member->Align(), member->Size(), core::IOAttributes{}));
        }

        // If no members were changed, just return the original struct.
        if (!has_changed) {
            return str;
        }

        // Create a new struct with the rewritten members.
        auto* new_str = ty.Get<core::type::Struct>(sym.New(str->Name().Name() + "_packed_vec3"),
                                                   std::move(new_members), str->Align(),
                                                   str->Size(), str->SizeNoPadding());

        // There are no struct flags that are valid for the MSL backend.
        TINT_ASSERT(str->StructFlags().Empty());

        return new_str;
    }

    /// Get (or create) a structure type that can be used for a packed vector of @p el elements when
    /// inside an array. The structure will have the alignment explicitly set to the required
    /// alignment of the original vec3 type.
    /// @param el the scalar element type
    /// @returns the packed array element type
    const core::type::Struct* GetPackedVec3ArrayElementStruct(const core::type::Type* el) {
        return packed_array_element_types.GetOrAdd(el, [&] {
            auto* packed = ty.packed_vec(el, 3);
            return ty.Struct(
                ir.symbols.New("tint_packed_vec3_" + el->FriendlyName() + "_array_element"),
                Vector{
                    ty.Get<core::type::StructMember>(
                        ir.symbols.New("packed"), packed, /* index */ 0u,
                        /* offset */ 0u, /* align */ 4 * el->Align(), /* size */ 3 * el->Size(),
                        core::IOAttributes{}),
                });
        });
    }

    /// @returns true if @p composite has an element type that will need a wrapper struct
    bool ElementTypeUsesWrapperStruct(const core::type::Type* composite) {
        return tint::Switch(
            composite,
            [&](const core::type::Matrix* mat) {  //
                return (mat->Rows() == 3);
            },
            [&](const core::type::Array* arr) {
                auto* vec = arr->ElemType()->As<core::type::Vector>();
                return (vec && vec->Width() == 3);
            },
            [](Default) {  //
                return false;
            });
    }

    /// Update a @p use of an @p unpacked_type type to work with a packed type instead.
    /// @param use the use
    /// @param unpacked_type the original unpacked type
    /// @param packed_type the packed type to use instead
    void UpdateUsage(core::ir::Usage& use,
                     const core::type::Type* unpacked_type,
                     const core::type::Type* packed_type) {
        if (packed_type == unpacked_type) {
            // This type does not contain vectors that need to be packed, so there's nothing to do.
            return;
        }

        tint::Switch(
            use.instruction,
            [&](core::ir::Access* access) {  //
                UpdateAccessUsage(access, unpacked_type);
            },
            [&](core::ir::CoreBuiltinCall* call) {
                // Assume this is only `arrayLength` until we find other cases.
                TINT_ASSERT(call->Func() == core::BuiltinFn::kArrayLength);
                // Nothing to do - the arrayLength builtin does not need to access the memory.
            },
            [&](core::ir::Let* let) {
                // Propagate the packed pointer through the `let` and update its usages.
                auto* unpacked_result_type = let->Result()->Type();
                auto* packed_result_type = RewriteType(unpacked_result_type);
                let->Result()->SetType(packed_result_type);
                let->Result()->ForEachUseSorted([&](core::ir::Usage let_use) {  //
                    UpdateUsage(let_use, unpacked_result_type, packed_result_type);
                });
            },
            [&](core::ir::Load* load) {
                b.InsertAfter(load, [&] {
                    auto* result = LoadPackedToUnpacked(unpacked_type->UnwrapPtr(), load->From());
                    load->Result()->ReplaceAllUsesWith(result);
                });
                load->Destroy();
            },
            [&](core::ir::LoadVectorElement*) {
                // Nothing to do - packed vectors support component access.
            },
            [&](core::ir::Store* store) {
                b.InsertBefore(store, [&] { StoreUnpackedToPacked(store->To(), store->From()); });
                store->Destroy();
            },
            [&](core::ir::StoreVectorElement*) {
                // Nothing to do - packed vectors support component access.
            },
            [&](core::ir::UserCall*) {
                // Nothing to do - pass the packed type to the function, which will be rewritten.
            },
            TINT_ICE_ON_NO_MATCH);
    }

    /// Update an access instruction that uses an unpacked_type type to use a packed type instead.
    /// @param access the access instruction
    /// @param unpacked_type the unpacked store type of the source pointer
    void UpdateAccessUsage(core::ir::Access* access, const core::type::Type* unpacked_type) {
        auto* unpacked_result_type = access->Result()->Type();
        auto* packed_result_type = RewriteType(unpacked_result_type);

        // Rebuild the indices of the access instruction.
        // Walk through the intermediate types that the access chain will be traversing, and
        // check for packed vectors that would be wrapped in structures.
        auto* obj_type = unpacked_type->UnwrapPtr();
        Vector<core::ir::Value*, 4> operands;
        operands.Push(access->Object());
        for (auto* idx : access->Indices()) {
            // Add the original index.
            operands.Push(idx);

            // If we are accessing into a composite that has an element type that will be
            // wrapped in a struct, we add an extra index to access into the first member of
            // that struct.
            if (ElementTypeUsesWrapperStruct(obj_type)) {
                operands.Push(b.Constant(u32(0)));
            }

            // Update the object type.
            if (auto* c = idx->As<core::ir::Constant>()) {
                obj_type = obj_type->Element(c->Value()->ValueAs<uint32_t>());
            } else {
                // Types that support dynamic indexing only have one element type.
                obj_type = obj_type->Elements().type;
            }
        }

        // Replace the access instruction's indices and update its usages.
        access->SetOperands(std::move(operands));
        access->Result()->SetType(packed_result_type);
        access->Result()->ForEachUseSorted([&](core::ir::Usage access_use) {  //
            UpdateUsage(access_use, unpacked_result_type, packed_result_type);
        });
    }

    /// Load a packed value from the pointer @p from and convert it to @p unpacked_type.
    ///
    /// Note: This function is called from within a builder insertion callback.
    ///
    /// @param unpacked_type the unpacked result type of the load
    /// @param from the packed pointer to load from
    /// @returns the unpacked result of the load
    core::ir::Value* LoadPackedToUnpacked(const core::type::Type* unpacked_type,
                                          core::ir::Value* from) {
        auto* packed_ptr = from->Type()->As<core::type::Pointer>();
        auto* packed_type = RewriteType(unpacked_type);
        if (unpacked_type == packed_type) {
            // There is no packed type inside `from`, so we can just load it directly.
            return b.Load(from)->Result();
        }

        return tint::Switch(
            unpacked_type,
            [&](const core::type::Array* arr) {
                return b.Call(LoadPackedArrayHelper(arr, packed_ptr), from)->Result();
            },
            [&](const core::type::Matrix* mat) {
                // Matrices are rewritten as arrays of structures, so pull the packed vectors out
                // of the structure for each column and then construct the matrix from them.
                auto* packed_col_type = RewriteType(mat->ColumnType());
                auto* packed_matrix = b.Load(from);
                Vector<core::ir::Value*, 4> columns;
                for (uint32_t col = 0; col < mat->Columns(); col++) {
                    auto* packed_col =
                        b.Access(packed_col_type, packed_matrix, u32(col), u32(0))->Result();
                    auto* unpacked_col = b.Call<msl::ir::BuiltinCall>(
                        mat->ColumnType(), msl::BuiltinFn::kConvert, packed_col);
                    columns.Push(unpacked_col->Result());
                }
                return b.Construct(unpacked_type, std::move(columns))->Result();
            },
            [&](const core::type::Struct* str) {
                return b.Call(LoadPackedStructHelper(str, packed_ptr), from)->Result();
            },
            [&](const core::type::Vector*) {
                // Load the packed vector and convert it to the unpacked equivalent.
                return b
                    .Call<msl::ir::BuiltinCall>(unpacked_type, msl::BuiltinFn::kConvert,
                                                b.Load(from))
                    ->Result();
            },
            TINT_ICE_ON_NO_MATCH);
    }

    /// Get (or create) a helper function that loads a packed array and converts it to the unpacked
    /// equivalent.
    /// @param unpacked_arr the unpacked array type
    /// @param packed_ptr_type the type of the pointer to the packed array
    /// @returns the helper function
    core::ir::Function* LoadPackedArrayHelper(const core::type::Array* unpacked_arr,
                                              const core::type::Pointer* packed_ptr_type) {
        return packed_load_helpers.GetOrAdd(packed_ptr_type, [&] {
            auto* func = b.Function(sym.New("tint_load_array_packed_vec3").Name(), unpacked_arr);
            auto* from = b.FunctionParam("from", packed_ptr_type);
            func->SetParams({from});

            auto* unpacked_el_type = unpacked_arr->ElemType();
            auto* packed_el_type = RewriteType(unpacked_el_type);
            auto* packed_el_ptr_type =
                ty.ptr(packed_ptr_type->AddressSpace(), packed_el_type, packed_ptr_type->Access());

            // Check if the element is a packed vector (which will be wrapped in a structure).
            bool packed_vec = false;
            if (auto* vec = packed_el_type->As<core::type::Vector>()) {
                packed_vec = vec->Packed();
            }

            b.Append(func->Block(), [&] {
                // Helper to load an array element at a given index.
                auto load_array_element = [&](core::ir::Value* index) {
                    auto* packed_el_ptr = b.Access(packed_el_ptr_type, from, index);
                    if (packed_vec) {
                        // If the element is a packed vector it will be wrapped in a structure, so
                        // load from the first member of that structure.
                        packed_el_ptr->AddIndex(b.Constant(u32(0)));
                    }
                    return LoadPackedToUnpacked(unpacked_el_type, packed_el_ptr->Result());
                };

                // Array elements that are packed vectors are wrapped in structures, so pull the
                // packed vectors out of each structure and then construct the array from them.
                TINT_ASSERT(unpacked_arr->ConstantCount());
                auto count = unpacked_arr->ConstantCount().value();
                if (count <= kMaxSeriallyUnpackedArraySize) {
                    Vector<core::ir::Value*, kMaxSeriallyUnpackedArraySize> elements;
                    for (uint32_t i = 0; i < count; i++) {
                        auto* unpacked_el = load_array_element(b.Constant(u32(i)));
                        elements.Push(unpacked_el);
                    }
                    b.Return(func, b.Construct(unpacked_arr, std::move(elements)));
                } else {
                    auto* result = b.Var(ty.ptr<function>(unpacked_arr));
                    b.LoopRange(ty, u32(0), u32(count), u32(1), [&](core::ir::Value* idx) {
                        auto* to =
                            b.Access(ty.ptr(function, unpacked_arr->ElemType()), result, idx);
                        auto* unpacked_el = load_array_element(idx);
                        b.Store(to, unpacked_el);
                    });
                    b.Return(func, b.Load(result));
                }
            });

            return func;
        });
    }

    /// Get (or create) a helper function that loads a packed struct and converts it to the unpacked
    /// equivalent.
    /// @param unpacked_str the unpacked struct type
    /// @param packed_ptr_type the type of the pointer to the packed struct
    /// @returns the helper function
    core::ir::Function* LoadPackedStructHelper(const core::type::Struct* unpacked_str,
                                               const core::type::Pointer* packed_ptr_type) {
        return packed_load_helpers.GetOrAdd(packed_ptr_type, [&] {
            auto* func = b.Function(sym.New("tint_load_struct_packed_vec3").Name(), unpacked_str);
            auto* from = b.FunctionParam("from", packed_ptr_type);
            func->SetParams({from});

            b.Append(func->Block(), [&] {
                Vector<core::ir::Value*, 4> members;
                members.Reserve(unpacked_str->Members().Length());
                for (auto* member : unpacked_str->Members()) {
                    auto* unpacked_member_type = member->Type();
                    auto* packed_member_type = RewriteType(unpacked_member_type);
                    auto* packed_member_ptr =
                        b.Access(ty.ptr(packed_ptr_type->AddressSpace(), packed_member_type,
                                        packed_ptr_type->Access()),
                                 from, u32(member->Index()));
                    auto* unpacked_member =
                        LoadPackedToUnpacked(unpacked_member_type, packed_member_ptr->Result());
                    members.Push(unpacked_member);
                }
                b.Return(func, b.Construct(unpacked_str, std::move(members))->Result());
            });

            return func;
        });
    }

    /// Store a @p value to a pointer @p to that contains packed vectors.
    ///
    /// Note: This function is called from within a builder insertion callback.
    ///
    /// @param to the packed pointer to store to
    /// @param value the unpacked value to store
    void StoreUnpackedToPacked(core::ir::Value* to, core::ir::Value* value) {
        auto* packed_ptr = to->Type()->As<core::type::Pointer>();
        auto* unpacked_type = value->Type();
        auto* packed_type = RewriteType(unpacked_type);
        if (unpacked_type == packed_type) {
            // There is no packed type inside `value`, so we can just store it directly.
            b.Store(to, value);
            return;
        }

        tint::Switch(
            unpacked_type,
            [&](const core::type::Array* arr) {
                b.Call(StorePackedArrayHelper(arr, packed_ptr), to, value)->Result();
            },
            [&](const core::type::Matrix* mat) {
                // Matrices are rewritten as arrays of structures, so store the packed vectors to
                // the first member of the that structure for each column.
                auto* packed_col_type = RewriteType(mat->ColumnType());
                auto* packed_col_ptr_type =
                    ty.ptr(packed_ptr->AddressSpace(), packed_col_type, packed_ptr->Access());
                for (uint32_t col = 0; col < mat->Columns(); col++) {
                    auto* packed_col_ptr = b.Access(packed_col_ptr_type, to, u32(col), u32(0));
                    auto* unpacked_col_val = b.Access(mat->ColumnType(), value, u32(col));
                    StoreUnpackedToPacked(packed_col_ptr->Result(), unpacked_col_val->Result());
                }
            },
            [&](const core::type::Struct* str) {
                b.Call(StorePackedStructHelper(str, packed_ptr), to, value)->Result();
            },
            [&](const core::type::Vector*) {  //
                // Convert the vector to the packed equivalent and store it.
                b.Store(to,
                        b.Call<msl::ir::BuiltinCall>(packed_type, msl::BuiltinFn::kConvert, value)
                            ->Result());
            },
            TINT_ICE_ON_NO_MATCH);
    }

    /// Get (or create) a helper function that stores an unpacked array to a packed pointer.
    /// @param unpacked_arr the unpacked array type
    /// @param packed_ptr_type the type of the pointer to the packed array
    /// @returns the helper function
    core::ir::Function* StorePackedArrayHelper(const core::type::Array* unpacked_arr,
                                               const core::type::Pointer* packed_ptr_type) {
        return packed_store_helpers.GetOrAdd(packed_ptr_type, [&] {
            auto* func = b.Function(sym.New("tint_store_array_packed_vec3").Name(), ty.void_());
            auto* to = b.FunctionParam("to", packed_ptr_type);
            auto* value = b.FunctionParam("value", unpacked_arr);
            func->SetParams({to, value});

            auto* unpacked_el_type = unpacked_arr->ElemType();
            auto* packed_el_type = RewriteType(unpacked_el_type);
            auto* packed_el_ptr_type =
                ty.ptr(packed_ptr_type->AddressSpace(), packed_el_type, packed_ptr_type->Access());

            // Check if the element is a packed vector (which will be wrapped in a structure).
            bool packed_vec = false;
            if (auto* vec = packed_el_type->As<core::type::Vector>()) {
                packed_vec = vec->Packed();
            }

            b.Append(func->Block(), [&] {
                // Helper to store an array element at a given index.
                auto store_array_element = [&](core::ir::Value* index) {
                    auto* unpacked_el = b.Access(unpacked_el_type, value, index);
                    auto* packed_el_ptr = b.Access(packed_el_ptr_type, to, index);
                    if (packed_vec) {
                        // If the element is a packed vector it will be wrapped in a structure, so
                        // store to the first member of that structure.
                        packed_el_ptr->AddIndex(b.Constant(u32(0)));
                    }
                    StoreUnpackedToPacked(packed_el_ptr->Result(), unpacked_el->Result());
                };

                // Store to each element of the array in a loop. If the element count is below a
                // threshold, unroll that loop in the shader.
                TINT_ASSERT(unpacked_arr->ConstantCount());
                auto count = unpacked_arr->ConstantCount().value();
                if (count <= kMaxSeriallyUnpackedArraySize) {
                    for (uint32_t i = 0; i < count; i++) {
                        store_array_element(b.Constant(u32(i)));
                    }
                } else {
                    b.LoopRange(ty, u32(0), u32(count), u32(1), [&](core::ir::Value* idx) {  //
                        store_array_element(idx);
                    });
                }
                b.Return(func);
            });

            return func;
        });
    }

    /// Get (or create) a helper function that stores an unpacked struct to a packed pointer.
    /// @param unpacked_str the unpacked struct type
    /// @param packed_ptr_type the type of the pointer to the packed struct
    /// @returns the helper function
    core::ir::Function* StorePackedStructHelper(const core::type::Struct* unpacked_str,
                                                const core::type::Pointer* packed_ptr_type) {
        return packed_store_helpers.GetOrAdd(packed_ptr_type, [&] {
            auto* func = b.Function(sym.New("tint_store_array_packed_vec3").Name(), ty.void_());
            auto* to = b.FunctionParam("to", packed_ptr_type);
            auto* value = b.FunctionParam("value", unpacked_str);
            func->SetParams({to, value});

            b.Append(func->Block(), [&] {
                // Store each member of the structure separately.
                for (auto* member : unpacked_str->Members()) {
                    auto* unpacked_member_type = member->Type();
                    auto* packed_member_type = RewriteType(unpacked_member_type);
                    auto* unpacked_member =
                        b.Access(unpacked_member_type, value, u32(member->Index()))->Result();
                    auto* packed_member_ptr =
                        b.Access(ty.ptr(packed_ptr_type->AddressSpace(), packed_member_type,
                                        packed_ptr_type->Access()),
                                 to, u32(member->Index()));
                    StoreUnpackedToPacked(packed_member_ptr->Result(), unpacked_member);
                }
                b.Return(func);
            });

            return func;
        });
    }
};

}  // namespace

Result<SuccessType> PackedVec3(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "msl.PackedVec3");
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::msl::writer::raise
