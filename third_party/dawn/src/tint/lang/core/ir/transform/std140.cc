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

#include "src/tint/lang/core/ir/transform/std140.h"

#include <cstdint>
#include <utility>

#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/text/string_stream.h"

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

    /// The symbol table.
    SymbolTable& sym{ir.symbols};

    /// Map from original type to a new type with decomposed matrices.
    Hashmap<const core::type::Type*, const core::type::Type*, 4> rewritten_types{};

    /// Map from struct member to its new index.
    Hashmap<const core::type::StructMember*, uint32_t, 4> member_index_map{};

    /// Map from a type to a helper function that will convert its rewritten form back to it.
    Hashmap<const core::type::Struct*, Function*, 4> convert_helpers{};

    /// Process the module.
    void Process() {
        if (ir.root_block->IsEmpty()) {
            return;
        }

        // Find uniform buffers that contain matrices that need to be decomposed.
        Vector<std::pair<Var*, const core::type::Type*>, 8> buffer_variables;
        for (auto inst : *ir.root_block) {
            if (auto* var = inst->As<Var>()) {
                auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
                if (!ptr || ptr->AddressSpace() != core::AddressSpace::kUniform) {
                    continue;
                }
                auto* store_type = RewriteType(ptr->StoreType());
                if (store_type != ptr->StoreType()) {
                    buffer_variables.Push(std::make_pair(var, store_type));
                }
            }
        }

        // Now process the buffer variables, replacing them with new variables that have decomposed
        // matrices and updating all usages of the variables.
        for (auto var_and_ty : buffer_variables) {
            // Create a new variable with the modified store type.
            auto* old_var = var_and_ty.first;
            auto* new_var = b.Var(ty.ptr(uniform, var_and_ty.second));
            const auto& bp = old_var->BindingPoint();
            new_var->SetBindingPoint(bp->group, bp->binding);
            if (auto name = ir.NameOf(old_var)) {
                ir.SetName(new_var->Result(), name);
            }

            // Transform instructions that accessed the variable to use the decomposed var.
            old_var->Result()->ForEachUseSorted(
                [&](Usage use) { Replace(use.instruction, new_var->Result()); });

            // Replace the original variable with the new variable.
            old_var->ReplaceWith(new_var);
            old_var->Destroy();
        }
    }

    /// @param type the type to check
    /// @returns the matrix if @p type is a matrix that needs to be decomposed
    static const core::type::Matrix* NeedsDecomposing(const core::type::Type* type) {
        if (auto* mat = type->As<core::type::Matrix>(); mat && NeedsDecomposing(mat)) {
            return mat;
        }
        return nullptr;
    }

    /// @param mat the matrix type to check
    /// @returns true if @p mat needs to be decomposed
    static bool NeedsDecomposing(const core::type::Matrix* mat) {
        // Std140 layout rules only require us to do this transform for matrices whose column
        // strides are not a multiple of 16 bytes.
        //
        // Due to a bug on Qualcomm devices, we also do this when the *size* of the column vector is
        // not a multiple of 16 bytes (e.g. matCx3 types). See crbug.com/tint/2074.
        return mat->ColumnType()->Size() & 15;
    }

    /// Rewrite a type if necessary, decomposing contained matrices.
    /// @param type the type to rewrite
    /// @returns the new type
    const core::type::Type* RewriteType(const core::type::Type* type) {
        return rewritten_types.GetOrAdd(type, [&] {
            return tint::Switch(
                type,
                [&](const core::type::Array* arr) {
                    // Create a new array with element type potentially rewritten.
                    return ty.array(RewriteType(arr->ElemType()), arr->ConstantCount().value());
                },
                [&](const core::type::Struct* str) -> const core::type::Type* {
                    bool needs_rewrite = false;
                    uint32_t member_index = 0;
                    Vector<const core::type::StructMember*, 4> new_members;
                    for (auto* member : str->Members()) {
                        if (auto* mat = NeedsDecomposing(member->Type())) {
                            // Decompose these matrices into a separate member for each column.
                            member_index_map.Add(member, member_index);
                            auto* col = mat->ColumnType();
                            uint32_t offset = member->Offset();
                            for (uint32_t i = 0; i < mat->Columns(); i++) {
                                StringStream ss;
                                ss << member->Name().Name() << "_col" << std::to_string(i);
                                new_members.Push(ty.Get<core::type::StructMember>(
                                    sym.New(ss.str()), col, member_index, offset, col->Align(),
                                    col->Size(), core::IOAttributes{}));
                                offset += col->Align();
                                member_index++;
                            }
                            needs_rewrite = true;
                        } else {
                            // For all other types, recursively rewrite them as necessary.
                            auto* new_member_ty = RewriteType(member->Type());
                            new_members.Push(ty.Get<core::type::StructMember>(
                                member->Name(), new_member_ty, member_index, member->Offset(),
                                member->Align(), member->Size(), core::IOAttributes{}));
                            member_index_map.Add(member, member_index);
                            member_index++;
                            if (new_member_ty != member->Type()) {
                                needs_rewrite = true;
                            }
                        }
                    }

                    // If no members needed to be rewritten, just return the original struct.
                    if (!needs_rewrite) {
                        return str;
                    }

                    // Create a new struct with the rewritten members.
                    auto* new_str = ty.Get<core::type::Struct>(
                        sym.New(str->Name().Name() + "_std140"), std::move(new_members),
                        str->Align(), str->Size(), str->SizeNoPadding());
                    for (auto flag : str->StructFlags()) {
                        new_str->SetStructFlag(flag);
                    }
                    return new_str;
                },
                [&](const core::type::Matrix* mat) -> const core::type::Type* {
                    if (!NeedsDecomposing(mat)) {
                        return mat;
                    }
                    StringStream name;
                    name << "mat" << mat->Columns() << "x" << mat->Rows() << "_"
                         << mat->ColumnType()->Type()->FriendlyName() << "_std140";
                    Vector<core::type::StructMember*, 4> members;
                    // Decompose these matrices into a separate member for each column.
                    auto* col = mat->ColumnType();
                    uint32_t offset = 0;
                    for (uint32_t i = 0; i < mat->Columns(); i++) {
                        StringStream ss;
                        ss << "col" << std::to_string(i);
                        members.Push(ty.Get<core::type::StructMember>(
                            sym.New(ss.str()), col, i, offset, col->Align(), col->Size(),
                            core::IOAttributes{}));
                        offset += col->Align();
                    }

                    // Create a new struct with the rewritten members.
                    return ty.Get<core::type::Struct>(
                        sym.New(name.str()), std::move(members), col->Align(),
                        col->Align() * mat->Columns(),
                        (col->Align() * (mat->Columns() - 1)) + col->Size());
                },
                [&](Default) {
                    // This type cannot contain a matrix, so no changes needed.
                    return type;
                });
        });
    }

    /// Reconstructs a column-decomposed matrix.
    /// @param mat the matrix type
    /// @param root the root value being accessed into
    /// @param indices the access indices that index the first column of the matrix.
    /// @returns the loaded matrix
    Value* RebuildMatrix(const core::type::Matrix* mat, Value* root, VectorRef<Value*> indices) {
        // Recombine each column vector from the struct and reconstruct the original matrix type.
        bool is_ptr = root->Type()->Is<core::type::Pointer>();
        auto first_column = indices.Back()->As<Constant>()->Value()->ValueAs<uint32_t>();
        Vector<Value*, 4> column_indices(std::move(indices));
        Vector<Value*, 4> args;
        for (uint32_t i = 0; i < mat->Columns(); i++) {
            column_indices.Back() = b.Constant(u32(first_column + i));
            if (is_ptr) {
                auto* access = b.Access(ty.ptr(uniform, mat->ColumnType()), root, column_indices);
                args.Push(b.Load(access)->Result());
            } else {
                auto* access = b.Access(mat->ColumnType(), root, column_indices);
                args.Push(access->Result());
            }
        }
        return b.Construct(mat, std::move(args))->Result();
    }

    /// Convert a value that may contain decomposed matrices to a value with the original type.
    /// @param source the value to convert
    /// @param orig_ty the original type to convert type
    /// @returns the converted value
    Value* Convert(Value* source, const core::type::Type* orig_ty) {
        if (source->Type() == orig_ty) {
            // The type was not rewritten, so just return the source value.
            return source;
        }
        return tint::Switch(
            orig_ty,  //
            [&](const core::type::Struct* str) -> Value* {
                // Create a helper function that converts the struct to the original type.
                auto* helper = convert_helpers.GetOrAdd(str, [&] {
                    auto* input_str = source->Type()->As<core::type::Struct>();
                    auto* func = b.Function("tint_convert_" + str->FriendlyName(), str);
                    auto* input = b.FunctionParam("tint_input", input_str);
                    func->SetParams({input});
                    b.Append(func->Block(), [&] {
                        uint32_t index = 0;
                        Vector<Value*, 4> args;
                        for (auto* member : str->Members()) {
                            if (auto* mat = NeedsDecomposing(member->Type())) {
                                args.Push(
                                    RebuildMatrix(mat, input, Vector{b.Constant(u32(index))}));
                                index += mat->Columns();
                            } else {
                                // Extract and convert the member.
                                auto* type = input_str->Element(index);
                                auto* extract = b.Access(type, input, u32(index));
                                args.Push(Convert(extract->Result(), member->Type()));
                                index++;
                            }
                        }

                        // Construct and return the original struct.
                        b.Return(func, b.Construct(str, std::move(args)));
                    });
                    return func;
                });

                // Call the helper function to convert the struct.
                return b.Call(str, helper, source)->Result();
            },
            [&](const core::type::Array* arr) -> Value* {
                // Create a loop that copies and converts each element of the array.
                auto* el_ty = source->Type()->Elements().type;
                auto* new_arr = b.Var(ty.ptr(function, arr));
                b.LoopRange(ty, 0_u, u32(arr->ConstantCount().value()), 1_u, [&](Value* idx) {
                    // Convert arr[idx] and store to new_arr[idx];
                    auto* to = b.Access(ty.ptr(function, arr->ElemType()), new_arr, idx);
                    auto* from = b.Access(el_ty, source, idx)->Result();
                    b.Store(to, Convert(from, arr->ElemType()));
                });
                return b.Load(new_arr)->Result();
            },
            [&](const core::type::Matrix* mat) -> Value* {
                if (!NeedsDecomposing(mat)) {
                    return source;
                }
                return RebuildMatrix(mat, source, Vector{b.Constant(u32(0))});
            },
            [&](Default) { return source; });
    }

    /// Replace a use of a value that contains or was derived from a decomposed matrix.
    /// @param inst the instruction to replace
    /// @param replacement the replacement value
    void Replace(Instruction* inst, Value* replacement) {
        b.InsertBefore(inst, [&] {
            tint::Switch(
                inst,  //
                [&](Access* access) {
                    auto* object_ty = access->Object()->Type()->As<core::type::MemoryView>();
                    if (!object_ty || object_ty->AddressSpace() != core::AddressSpace::kUniform) {
                        // Access to non-uniform memory views does not require transformation.
                        return;
                    }

                    if (!replacement->Type()->Is<core::type::MemoryView>()) {
                        // The replacement is a value, in which case the decomposed matrix has
                        // already been reconstructed. In this situation the access only needs its
                        // return type updating, and downstream instructions need updating.
                        access->SetOperand(Access::kObjectOperandOffset, replacement);
                        auto* result = access->Result();
                        result->SetType(result->Type()->UnwrapPtrOrRef());
                        result->ForEachUseSorted(
                            [&](Usage use) { Replace(use.instruction, result); });
                        return;
                    }

                    // Modify the access indices to take decomposed matrices into account.
                    auto* current_type = object_ty->StoreType();
                    Vector<Value*, 4> indices;

                    if (NeedsDecomposing(current_type)) {
                        // Decomposed matrices are indexed using their first column vector
                        indices.Push(b.Constant(0_u));
                    }

                    for (size_t i = 0, n = access->Indices().Length(); i < n; i++) {
                        auto* idx = access->Indices()[i];

                        if (auto* mat = NeedsDecomposing(current_type)) {
                            // Access chain passes through decomposed matrix.
                            if (auto* const_idx = idx->As<Constant>()) {
                                // Column vector index is a constant.
                                // Instead of loading the whole matrix, fold the access of the
                                // matrix and the constant column index into an single access of
                                // column vector member.
                                auto* base_idx = indices.Back()->As<Constant>();
                                indices.Back() =
                                    b.Constant(u32(base_idx->Value()->ValueAs<uint32_t>() +
                                                   const_idx->Value()->ValueAs<uint32_t>()));
                                current_type = mat->ColumnType();
                                i++;  // We've already consumed the column access
                            } else {
                                // Column vector index is dynamic.
                                // Reconstruct the whole matrix and index that.
                                replacement = RebuildMatrix(mat, replacement, std::move(indices));
                                indices.Clear();
                                indices.Push(idx);
                                current_type = mat->ColumnType();
                            }
                        } else if (auto* str = current_type->As<core::type::Struct>()) {
                            // Remap member index
                            uint32_t old_index = idx->As<Constant>()->Value()->ValueAs<uint32_t>();
                            uint32_t new_index = *member_index_map.Get(str->Members()[old_index]);
                            current_type = str->Element(old_index);
                            indices.Push(b.Constant(u32(new_index)));
                        } else {
                            indices.Push(idx);
                            current_type = current_type->Elements().type;
                            if (NeedsDecomposing(current_type)) {
                                // Decomposed matrices are indexed using their first column vector
                                indices.Push(b.Constant(0_u));
                            }
                        }
                    }

                    if (auto* mat = NeedsDecomposing(current_type)) {
                        replacement = RebuildMatrix(mat, replacement, std::move(indices));
                        indices.Clear();
                    }

                    if (!indices.IsEmpty()) {
                        // Emit the access with the modified indices.
                        if (replacement->Type()->Is<core::type::Pointer>()) {
                            current_type = ty.ptr(uniform, RewriteType(current_type));
                        }
                        auto* new_access = b.Access(current_type, replacement, std::move(indices));
                        replacement = new_access->Result();
                    }

                    // Replace every instruction that uses the original access instruction.
                    access->Result()->ForEachUseSorted(
                        [&](Usage use) { Replace(use.instruction, replacement); });
                    access->Destroy();
                },
                [&](Load* load) {
                    if (!replacement->Type()->Is<core::type::Pointer>()) {
                        // We have already loaded to a value type, so this load just folds away.
                        load->Result()->ReplaceAllUsesWith(replacement);
                    } else {
                        // Load the decomposed value and then convert it to the original type.
                        auto* decomposed = b.Load(replacement);
                        auto* converted = Convert(decomposed->Result(), load->Result()->Type());
                        load->Result()->ReplaceAllUsesWith(converted);
                    }
                    load->Destroy();
                },
                [&](LoadVectorElement* load) {
                    if (!replacement->Type()->Is<core::type::Pointer>()) {
                        // We have loaded a decomposed matrix and reconstructed it, so this is now
                        // extracting from a value type.
                        b.AccessWithResult(load->DetachResult(), replacement, load->Index());
                        load->Destroy();
                    } else {
                        // There was no decomposed matrix on the path to this instruction so just
                        // update the source operand.
                        load->SetOperand(LoadVectorElement::kFromOperandOffset, replacement);
                    }
                },
                [&](Let* let) {
                    // Let instructions just fold away.
                    let->Result()->ForEachUseSorted(
                        [&](Usage use) { Replace(use.instruction, replacement); });
                    let->Destroy();
                });
        });
    }
};

}  // namespace

Result<SuccessType> Std140(Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.Std140", kStd140Capabilities);
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
