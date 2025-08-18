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

#include "src/tint/lang/spirv/reader/lower/transpose_row_major.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/spirv/type/explicit_layout_array.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;  // NOLINT

constexpr std::string_view kTintLoadRowMajor = "tint_load_row_major_column";
constexpr std::string_view kTintTransposeRowMajorArray = "tint_transpose_row_major_array";
constexpr std::string_view kTintStoreRowMajor = "tint_store_row_major_column";

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The symbol manager.
    SymbolTable& sym{ir.symbols};

    // A map from a (type, is_row_major) pair to it's replacement (which maybe the same as the
    // original).
    struct TypeAndRowMajor {
        const core::type::Type* type;
        bool row_major;

        bool operator==(const TypeAndRowMajor& other) const {
            return type == other.type && row_major == other.row_major;
        }

        tint::HashCode HashCode() const { return Hash(type, row_major); }
    };
    Hashmap<TypeAndRowMajor, const core::type::Type*, 32> original_to_transposed{};

    /// A map from rewritten structs to original structs.
    Hashmap<const core::type::Struct*, const core::type::Struct*, 4> struct_to_original{};

    /// List of instruction results where the usages need to be updated
    Vector<core::ir::InstructionResult*, 32> results_to_update{};

    /// A hash of access instructions to the vector index they were accessing
    Hashmap<core::ir::Value*, core::ir::Value*, 8> access_to_vector_index{};

    /// A list of instructions to remove
    Vector<core::ir::Instruction*, 8> instructions_to_remove_if_unused{};

    /// A map of type to the load helper
    Hashmap<const core::type::Type*, core::ir::Function*, 4> load_functions{};

    /// A map of type to the store helper
    Hashmap<const core::type::Type*, core::ir::Function*, 4> store_functions{};

    /// Instructions which have been processed
    Hashset<core::ir::InstructionResult*, 32> processed_instructions{};

    /// Process the module.
    void Process() {
        Vector<core::ir::Instruction*, 4> instructions_to_process;
        for (auto* inst : ir.Instructions()) {
            // Replace all constant operands where the type will be changed due to it containing a
            // structure that uses a row-major attribute.
            for (uint32_t i = 0; i < inst->Operands().Length(); ++i) {
                if (auto* constant = As<core::ir::Constant>(inst->Operands()[i])) {
                    auto* new_constant = RewriteConstant(constant->Value(), false);
                    if (new_constant != constant->Value()) {
                        inst->SetOperand(i, b.Constant(new_constant));
                    }
                }
            }

            for (auto* res : inst->Results()) {
                auto* new_ty = RewriteType(res->Type(), false);
                if (new_ty != res->Type()) {
                    res->SetType(new_ty);
                    instructions_to_process.Push(inst);
                    results_to_update.Push(res);
                }
            }
        }
        for (auto inst : instructions_to_process) {
            ProcessInstruction(inst);
        }

        while (!results_to_update.IsEmpty()) {
            auto* result = results_to_update.Pop();

            if (!processed_instructions.Add(result)) {
                continue;
            }

            // It's possible we've already processed this instruction because we're working through
            // results, so if it isn't alive, then we've already replaced it and we can move on.
            if (!result->Alive()) {
                continue;
            }

            // Use sorted usages to force a copy, the replacements may create new usages of this
            // result.
            for (auto& usage : result->UsagesSorted()) {
                ProcessInstruction(usage.instruction);
            }
        }

        for (auto* inst : instructions_to_remove_if_unused) {
            // If we've detached, just remove the instruction
            if (inst->Results().IsEmpty()) {
                inst->Destroy();
                continue;
            }

            TINT_ASSERT(inst->Results().Length() == 1);
            if (!inst->Result()->IsUsed()) {
                inst->Destroy();
            }
        }
    }

    void ProcessInstruction(core::ir::Instruction* inst) {
        tint::Switch(
            inst,  //
            [&](core::ir::Var*) {
                // Nothing to do as we already substituted the type.
            },
            [&](core::ir::Let*) {
                // Nothing to do as we already substituted the type.
            },

            [&](core::ir::Load* ld) { ReplaceLoad(ld); },
            [&](core::ir::Store* store) { ReplaceStore(store); },
            [&](core::ir::Access* access) { ReplaceAccess(access); },
            [&](core::ir::Construct* construct) { ReplaceConstruct(construct); },
            [&](core::ir::LoadVectorElement* lve) { ReplaceLoadVectorElement(lve); },
            [&](core::ir::StoreVectorElement* sve) { ReplaceStoreVectorElement(sve); },
            TINT_ICE_ON_NO_MATCH);
    }

    void ReplaceConstruct(core::ir::Construct* construct) {
        auto* struct_type = construct->Result()->Type()->As<core::type::Struct>();
        if (!struct_type) {
            return;
        }

        b.InsertBefore(construct, [&] {
            Vector<core::ir::Value*, 8> new_operands;
            for (uint32_t i = 0; i < construct->Operands().Length(); i++) {
                auto* operand = construct->Operands()[i];
                auto* member_type = struct_type->Members()[i]->Type();
                if (member_type != operand->Type()) {
                    tint::Switch(
                        member_type,
                        [&](const core::type::Matrix*) {
                            new_operands.Push(
                                b.Call(member_type, core::BuiltinFn::kTranspose, operand)
                                    ->Result());
                        },
                        [&](const core::type::Array*) {
                            // TODO(437140112): Add support for arrays of matrices
                            TINT_UNIMPLEMENTED() << "handle construct of array of matrices";
                        },
                        TINT_ICE_ON_NO_MATCH);
                } else {
                    new_operands.Push(operand);
                }
            }
            construct->SetOperands(new_operands);
        });
    }

    // This is a store vector element that is going to a matrix which we've transposed the size of.
    // So, we need to swap the index on this store with the last index of the source access.
    void ReplaceStoreVectorElement(core::ir::StoreVectorElement* sve) {
        auto* src_to = sve->To()->As<core::ir::InstructionResult>();
        TINT_ASSERT(src_to);

        auto* src_access = src_to->Instruction()->As<core::ir::Access>();
        TINT_ASSERT(src_access);

        auto access_idx = access_to_vector_index.Get(sve->To());
        TINT_ASSERT(access_idx);

        core::ir::Access* new_access = nullptr;
        b.InsertAfter(src_access, [&] {
            auto* src_ty = src_to->Type()->As<core::type::Pointer>();
            TINT_ASSERT(src_ty);

            auto* src_mat = src_ty->StoreType()->As<core::type::Matrix>();
            TINT_ASSERT(src_mat);

            auto* new_ptr =
                ty.ptr(src_ty->AddressSpace(), ty.vec(src_mat->Type(), src_mat->Rows()));
            new_access = b.Access(new_ptr, src_access, Vector{sve->Index()});

            b.InsertAfter(sve,
                          [&] { b.StoreVectorElement(new_access, *access_idx, sve->Value()); });
            sve->Destroy();
        });

        instructions_to_remove_if_unused.Push(src_access);
    }

    // This is a load vector element that is coming from a matrix which we've transposed the size
    // of. So, we need to swap the index on this load with the last index of the source access.
    void ReplaceLoadVectorElement(core::ir::LoadVectorElement* lve) {
        auto* src_result = lve->From()->As<core::ir::InstructionResult>();
        TINT_ASSERT(src_result);

        auto* src_access = src_result->Instruction()->As<core::ir::Access>();
        TINT_ASSERT(src_access);

        auto access_idx = access_to_vector_index.Get(lve->From());
        TINT_ASSERT(access_idx);

        core::ir::Access* new_access = nullptr;
        b.InsertAfter(src_access, [&] {
            auto* src_ty = src_result->Type()->As<core::type::Pointer>();
            TINT_ASSERT(src_ty);

            auto* src_mat = src_ty->StoreType()->As<core::type::Matrix>();
            TINT_ASSERT(src_mat);

            auto* new_ptr =
                ty.ptr(src_ty->AddressSpace(), ty.vec(src_mat->Type(), src_mat->Rows()));
            new_access = b.Access(new_ptr, src_access, Vector{lve->Index()});

            b.InsertAfter(lve, [&] {
                b.LoadVectorElementWithResult(lve->DetachResult(), new_access, *access_idx);
            });
            lve->Destroy();
        });

        instructions_to_remove_if_unused.Push(src_access);
    }

    void ReplaceAccess(core::ir::Access* access) {
        bool indexed_through_row_major = false;

        auto* cur_ty = access->Object()->Type()->UnwrapPtr();
        const core::type::Type* parent_ty = nullptr;
        const core::type::Matrix* mat_ty = nullptr;
        auto indices = access->Indices();
        int32_t matrix_index = -1;
        for (uint32_t i = 0; i < indices.Length(); ++i) {
            auto* idx = indices[i];

            if (auto* struct_ty = cur_ty->As<core::type::Struct>()) {
                auto const_idx = idx->As<core::ir::Constant>()->Value()->ValueAs<uint32_t>();
                parent_ty = cur_ty;
                cur_ty = cur_ty->Element(const_idx);

                auto* orig_struct = struct_to_original.GetOr(struct_ty, nullptr);
                if (!orig_struct) {
                    // Structure didn't change, so doesn't contain a row-major member
                    continue;
                }

                auto* mem = orig_struct->Members()[const_idx];
                if (mem->RowMajor()) {
                    indexed_through_row_major = true;
                }

            } else {
                parent_ty = cur_ty;
                cur_ty = cur_ty->Elements().type;
            }

            // We do this at the end because we want the index that we load the matrix from, so the
            // next thing we look at would be the matrix.
            if (cur_ty->Is<core::type::Matrix>()) {
                TINT_ASSERT(matrix_index == -1);
                mat_ty = cur_ty->As<core::type::Matrix>();
                matrix_index = int32_t(i);
            }
        }

        // The matrix is in an array and we're dealing with the array. We'll need to handle the
        // array when we load/store it, so add the access to the results to update.
        if (cur_ty->Is<core::type::Array>()) {
            ReplacePointerAccess(access, parent_ty, cur_ty);
            results_to_update.Push(access->Result());
            return;
        }

        // The thing we're accessing has changed, so we need to change.
        if (!indexed_through_row_major && cur_ty != access->Result()->Type()->UnwrapPtr()) {
            ReplacePointerAccess(access, parent_ty, cur_ty);
            return;
        }

        // If we didn't find a matrix, then something is weird, we should have never tried to
        // replace the access.
        TINT_ASSERT(matrix_index != -1);

        // Not accessing through a matrix, nothing to do.
        if (!indexed_through_row_major) {
            return;
        }

        if (access->Object()->Type()->Is<core::type::Pointer>()) {
            ReplacePointerAccess(access, parent_ty, cur_ty);
            return;
        }

        // This isn't a pointer access, so we'll just split the access in half, transpose the
        // matrix itself and then access that matrix with the rest of the expression.

        Vector<core::ir::Value*, 4> mat_indices = indices.Truncate(size_t(matrix_index) + 1);

        b.InsertBefore(access, [&] {
            auto* m = b.Access(mat_ty, access->Object(), mat_indices);
            auto* t = b.Call(RewriteType(mat_ty, true), core::BuiltinFn::kTranspose, m)->Result();

            if (uint32_t(matrix_index) != indices.Length() - 1) {
                Vector<core::ir::Value*, 4> access_indices =
                    indices.Offset(size_t(matrix_index) + 1);
                b.AccessWithResult(access->DetachResult(), t, access_indices)->Result();
            } else {
                access->Result()->ReplaceAllUsesWith(t);
            }
        });
        access->Destroy();
    }

    void ReplacePointerAccess(core::ir::Access* access,
                              const core::type::Type* parent_ty,
                              const core::type::Type* cur_ty) {
        // We've accessed the matrix itself, or an array containing the matrix. We need to update
        // the access result type, and if changed, update the uses of this access
        if (cur_ty->Is<core::type::Matrix>() || cur_ty->Is<core::type::Array>()) {
            auto* new_access_ty = RewriteType(access->Result()->Type(), true);
            if (new_access_ty != access->Result()->Type()) {
                access->Result()->SetType(new_access_ty);
                results_to_update.Push(access->Result());
            }
            return;
        }

        // We're accessing a row of the vector. We need to replace this access with an access of
        // the parent matrix and then store away which row we're accessing so we can rebuild any
        // needed accesses later.
        if (cur_ty->Is<core::type::Vector>()) {
            TINT_ASSERT(parent_ty != nullptr);
            auto* idx = access->PopLastIndex();

            /// The access now returns the transposed matrix
            auto* new_access_ty = access->Result()->Type();
            if (auto* access_ptr = access->Result()->Type()->As<core::type::Pointer>()) {
                new_access_ty = ty.ptr(access_ptr->AddressSpace(), parent_ty, access_ptr->Access());
            }
            access->Result()->SetType(new_access_ty);

            access_to_vector_index.Add(access->Result(), idx);
            results_to_update.Push(access->Result());
            return;
        }

        TINT_UNREACHABLE() << "access of unknown type for row-major matrix";
    }

    void ReplaceLoad(core::ir::Load* ld) {
        auto* ld_ty = ld->Result()->Type();

        tint::Switch(
            ld_ty,  //
            [&](const core::type::Matrix*) {
                b.InsertAfter(ld, [&] {
                    // We're replacing the load, which means the source must have been a transposed
                    // matrix, so we need to get the load result as if it was row-major decorated.
                    auto* new_res = b.InstructionResult(RewriteType(ld->Result()->Type(), true));
                    b.CallWithResult(ld->DetachResult(), core::BuiltinFn::kTranspose, new_res);
                    ld->SetResult(new_res);
                });
            },
            [&](const core::type::Vector*) {
                auto idx = access_to_vector_index.Get(ld->From());
                TINT_ASSERT(idx);

                if (idx) {
                    // We're loading a vector from an access chain, we need to determine if this
                    // vector came from a row-major matrix
                    auto* load_fn = LoadColumnHelper(ld->From()->Type()->As<core::type::Pointer>());
                    b.InsertAfter(ld, [&] {
                        auto* v = *idx;
                        if (v->Type()->Is<core::type::I32>()) {
                            v = b.Convert(ty.u32(), v)->Result();
                        }
                        b.CallWithResult(ld->DetachResult(), load_fn, ld->From(), v);
                    });

                    // Do this after we're done so we don't end up modifying the usage list of the
                    // result we're iterating over.
                    instructions_to_remove_if_unused.Push(ld);

                } else {
                    TINT_UNREACHABLE() << "attempting to load a row-major vector?";
                }
            },
            [&](const core::type::Struct*) {
                // Handled elsewhere
            },
            [&](const core::type::Array*) {
                // We're replacing the load, which means the source must have been a transposed
                // array of matrix.
                auto* load_fn = TransposeArrayHelper(ld->From()->Type()->UnwrapPtr());
                b.InsertAfter(ld, [&] {
                    auto* new_ld = b.Load(ld->From());
                    b.CallWithResult(ld->DetachResult(), load_fn, new_ld);
                });

                // Do this after we're done so we don't end up modifying the usage list of the
                // result we're iterating over.
                instructions_to_remove_if_unused.Push(ld);
            },
            TINT_ICE_ON_NO_MATCH);
    }

    void ReplaceStore(core::ir::Store* store) {
        auto vec_idx = access_to_vector_index.Get(store->To());

        auto* to_ty = store->To()->Type()->UnwrapPtr();
        if (!vec_idx) {
            tint::Switch(
                to_ty,  //
                [&](const core::type::Matrix*) {
                    // Storing the full matrix
                    b.InsertBefore(store, [&] {
                        auto* from = b.Call(to_ty, core::BuiltinFn::kTranspose, store->From());
                        store->SetFrom(from->Result());
                    });
                },
                [&](const core::type::Array*) {
                    b.InsertBefore(store, [&] {
                        auto* from = store->From();
                        if (from->Type()->Is<core::type::Pointer>()) {
                            from = b.Load(from)->Result();
                        }
                        auto* fn = TransposeArrayHelper(from->Type());
                        store->SetFrom(b.Call(to_ty, fn, from)->Result());
                    });
                },
                [&](const core::type::Struct*) {
                    // Should already be fixed
                },
                TINT_ICE_ON_NO_MATCH);
            return;
        }

        // Storing a vector
        auto* store_fn = StoreColumnHelper(store->To()->Type()->As<core::type::Pointer>());
        b.InsertAfter(store, [&] {
            auto* v = *vec_idx;
            if (v->Type()->Is<core::type::I32>()) {
                v = b.Convert(ty.u32(), v)->Result();
            }

            b.Call(ty.void_(), store_fn, store->To(), v, store->From());
        });
        instructions_to_remove_if_unused.Push(store);
    }

    // Note, the type provided in the pointer has _already_ been transposed.
    core::ir::Function* StoreColumnHelper(const core::type::Pointer* ptr) {
        return store_functions.GetOrAdd(ptr, [&] {
            TINT_ASSERT(ptr);

            auto* row_major_ty = ptr->UnwrapPtr()->As<core::type::Matrix>();
            TINT_ASSERT(row_major_ty);

            auto* vec_ty = ty.vec(row_major_ty->Type(), row_major_ty->Columns());
            auto* col_ptr_ty =
                ty.ptr(ptr->AddressSpace(), ty.vec(row_major_ty->Type(), row_major_ty->Rows()),
                       ptr->Access());

            auto* fn = b.Function(kTintStoreRowMajor, ty.void_());
            auto* mat = b.FunctionParam(ptr);
            auto* row = b.FunctionParam(ty.u32());
            auto* col = b.FunctionParam(vec_ty);
            fn->SetParams({mat, row, col});

            b.Append(fn->Block(), [&] {
                for (uint32_t i = 0; i < row_major_ty->Columns(); ++i) {
                    auto* col_access = b.Access(vec_ty->DeepestElement(), col, u32(i));
                    b.StoreVectorElement(b.Access(col_ptr_ty, mat, u32(i)), row, col_access);
                }
                b.Return(fn);
            });
            return fn;
        });
    }

    // Note, the type provided in the pointer has _already_ been transposed.
    core::ir::Function* LoadColumnHelper(const core::type::Pointer* ptr) {
        return load_functions.GetOrAdd(ptr, [&] {
            TINT_ASSERT(ptr);

            auto* row_major_ty = ptr->UnwrapPtr()->As<core::type::Matrix>();
            TINT_ASSERT(row_major_ty);

            auto* vec_ty = ty.vec(row_major_ty->Type(), row_major_ty->Columns());
            auto* col_ptr_ty =
                ty.ptr(ptr->AddressSpace(), ty.vec(row_major_ty->Type(), row_major_ty->Rows()),
                       ptr->Access());

            auto* fn = b.Function(kTintLoadRowMajor, vec_ty);
            auto* mat = b.FunctionParam(ptr);
            auto* row = b.FunctionParam(ty.u32());
            fn->SetParams({mat, row});

            b.Append(fn->Block(), [&] {
                Vector<core::ir::Value*, 4> values;
                for (uint32_t i = 0; i < row_major_ty->Columns(); ++i) {
                    values.Push(
                        b.LoadVectorElement(b.Access(col_ptr_ty, mat, u32(i)), row)->Result());
                }
                b.Return(fn, b.Construct(vec_ty, values));
            });
            return fn;
        });
    }

    core::ir::Function* TransposeArrayHelper(const core::type::Type* in_type) {
        // The helper function will look like this:
        //   fn tint_transpose_array(from: array<mat3x2<f32>, 4>) -> array<mat2x3<f32>, 4> {
        //     var result : array<mat2x3<f32>, 4>;
        //     for (var i = 0; i < 4; i++) {
        //       result[i] = transpose(from[i]);
        //     }
        //     return result;
        //   }
        TINT_ASSERT(in_type);

        return load_functions.GetOrAdd(in_type, [&] {
            auto* outer_ty = in_type->As<core::type::Array>();
            TINT_ASSERT(outer_ty);

            auto* from_ty = outer_ty;
            auto* to_ty = RewriteType(in_type, true)->As<core::type::Array>();
            TINT_ASSERT(from_ty);

            auto* fn = b.Function(kTintTransposeRowMajorArray, to_ty);
            auto* in = b.FunctionParam(in_type);
            fn->SetParams({in});

            auto count = from_ty->ConstantCount();

            b.Append(fn->Block(), [&] {
                auto* res = b.Var(ty.ptr(function, to_ty));
                b.LoopRange(ty, u32(0), u32(*count), u32(1), [&](core::ir::Value* idx) {
                    core::ir::Value* transposed = nullptr;
                    auto* cur = b.Access(from_ty->ElemType(), in, idx);
                    if (auto* nested = outer_ty->ElemType()->As<core::type::Array>()) {
                        auto* inner_fn = TransposeArrayHelper(nested);
                        transposed = b.Call(to_ty->ElemType(), inner_fn, cur)->Result();
                    } else {
                        transposed =
                            b.Call(to_ty->ElemType(), core::BuiltinFn::kTranspose, cur)->Result();
                    }

                    auto* slot = b.Access(ty.ptr(function, to_ty->ElemType()), res, idx);
                    b.Store(slot, transposed);
                });
                auto* ld = b.Load(res);
                b.Return(fn, ld);
            });

            return fn;
        });
    }

    const core::type::Type* RewriteType(const core::type::Type* type, bool decorated_row_major) {
        return original_to_transposed.GetOrAdd(TypeAndRowMajor{type, decorated_row_major}, [&] {
            return tint::Switch(
                type,
                [&](const core::type::Array* arr) {
                    return RewriteArray(arr, decorated_row_major);
                },
                [&](const core::type::Struct* str) { return RewriteStruct(str); },
                [&](const core::type::Matrix* mat) {
                    if (!decorated_row_major) {
                        return mat;
                    }
                    return ty.mat(mat->Type(), mat->Rows(), mat->Columns());
                },
                [&](const core::type::Pointer* ptr) {
                    return ty.ptr(ptr->AddressSpace(),
                                  RewriteType(ptr->StoreType(), decorated_row_major),
                                  ptr->Access());
                },
                [&](Default) { return type; });
        });
    }

    const core::type::Type* RewriteArray(const core::type::Array* arr, bool decorated_row_major) {
        auto* elem_ty = RewriteType(arr->ElemType(), decorated_row_major);
        if (elem_ty == arr->ElemType()) {
            return arr;
        }

        // The element type is the only thing that will change. That does not affect the stride of
        // the array itself, which may either be the natural stride or an larger stride in the case
        // of an explicitly laid out array.
        if (arr->Is<spirv::type::ExplicitLayoutArray>()) {
            return ty.Get<spirv::type::ExplicitLayoutArray>(elem_ty, arr->Count(), arr->Align(),
                                                            arr->Size(), arr->Stride());
        }
        return ty.Get<core::type::Array>(elem_ty, arr->Count(), arr->Align(), arr->Size(),
                                         arr->Stride(), arr->Stride());
    }

    const core::type::Type* RewriteStruct(const core::type::Struct* old_struct) {
        bool made_changes = false;

        Vector<const core::type::StructMember*, 8> new_members;
        new_members.Reserve(old_struct->Members().Length());
        for (auto* member : old_struct->Members()) {
            auto* new_member_type = RewriteType(member->Type(), member->RowMajor());
            if (member->RowMajor() || new_member_type != member->Type()) {
                // Recreate the struct member without the row major attribute, using the new type.
                auto* new_member = ty.Get<core::type::StructMember>(
                    member->Name(), new_member_type, member->Index(), member->Offset(),
                    member->Align(), member->Size(), member->Attributes());
                if (member->HasMatrixStride()) {
                    new_member->SetMatrixStride(member->MatrixStride());
                }
                new_members.Push(new_member);
                made_changes = true;
            } else {
                new_members.Push(member);
            }
        }
        if (!made_changes) {
            return old_struct;
        }

        // Create the new struct and record the mapping to the old struct.
        auto* new_struct = ty.Struct(sym.New(old_struct->Name().Name()), std::move(new_members));
        struct_to_original.Add(new_struct, old_struct);

        return new_struct;
    }

    const core::constant::Value* RewriteConstant(const core::constant::Value* constant,
                                                 bool is_row_major) {
        auto* orig_type = constant->Type();
        auto* new_type = RewriteType(orig_type, is_row_major);
        if (new_type == orig_type) {
            return constant;
        }

        return tint::Switch(
            new_type,  //
            [&](const core::type::Matrix* mat) {
                if (!is_row_major) {
                    return constant;
                }

                auto* orig_mat = orig_type->As<core::type::Matrix>();
                TINT_ASSERT(orig_mat);
                TINT_ASSERT(constant->NumElements() == mat->Rows());

                Vector<const core::constant::Value*, 4> columns;
                for (size_t i = 0; i < mat->Columns(); ++i) {
                    auto* vec_ty = ty.vec(mat->Type(), mat->Rows());

                    Vector<const core::constant::Value*, 4> vec_elements;
                    for (uint32_t j = 0; j < mat->Rows(); ++j) {
                        auto* value = constant->Index(j);
                        TINT_ASSERT(value->NumElements() == mat->Columns());
                        vec_elements.Push(value->Index(i));
                    }
                    columns.Push(ir.constant_values.Composite(vec_ty, std::move(vec_elements)));
                }

                return ir.constant_values.Composite(new_type, std::move(columns));
            },
            [&](const core::type::Array*) {
                if (!is_row_major) {
                    return constant;
                }

                Vector<const core::constant::Value*, 16> elements;
                for (uint32_t i = 0; i < constant->NumElements(); i++) {
                    auto* value = constant->Index(i);
                    elements.Push(RewriteConstant(value, is_row_major));
                }
                return ir.constant_values.Composite(new_type, std::move(elements));
            },
            [&](const core::type::Struct* str) {
                TINT_ASSERT(constant->NumElements() == str->Members().Length());

                Vector<const core::constant::Value*, 16> elements;
                elements.Reserve(str->Members().Length());

                auto* orig_str = orig_type->As<core::type::Struct>();
                TINT_ASSERT(orig_str);

                for (size_t i = 0; i < orig_str->Members().Length(); ++i) {
                    auto& orig_mem = orig_str->Members()[i];
                    auto& new_mem = str->Members()[i];
                    auto* value = constant->Index(i);

                    auto* new_member_type = new_mem->Type();
                    if (new_member_type != value->Type()) {
                        elements.Push(RewriteConstant(value, orig_mem->RowMajor()));
                    } else {
                        elements.Push(value);
                    }
                }
                return ir.constant_values.Composite(new_type, std::move(elements));
            },
            [&](Default) { return constant; });
    }
};

}  // namespace

Result<SuccessType> TransposeRowMajor(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.TransposeRowMajor",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowMultipleEntryPoints,
                                              core::ir::Capability::kAllowStructMatrixDecorations,
                                              core::ir::Capability::kAllowNonCoreTypes,
                                              core::ir::Capability::kAllowOverrides,
                                          });
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::reader::lower
