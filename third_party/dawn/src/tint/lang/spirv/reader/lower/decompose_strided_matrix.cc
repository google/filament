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

#include "src/tint/lang/spirv/reader/lower/decompose_strided_matrix.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/traverse.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/spirv/type/explicit_layout_array.h"

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

    /// The symbol manager.
    SymbolTable& sym{ir.symbols};

    /// A map from a type to its replacement type (which may be the same as the original).
    struct TypeAndStride {
        const core::type::Type* type;
        uint32_t stride;

        bool operator==(const TypeAndStride& other) const {
            return type == other.type && stride == other.stride;
        }

        tint::HashCode HashCode() const { return Hash(type, stride); }
    };
    Hashmap<TypeAndStride, const core::type::Type*, 32> type_map{};

    /// A signature representation for caching specialized function clones.
    struct FunctionSpec {
        core::ir::Function* func;
        Vector<const core::type::Type*, 4> param_types;

        bool operator==(const FunctionSpec& other) const {
            return func == other.func && param_types == other.param_types;
        }

        tint::HashCode HashCode() const {
            auto hash = Hash(func);
            for (auto* type : param_types) {
                hash = HashCombine(hash, type);
            }
            return hash;
        }
    };
    Hashmap<FunctionSpec, core::ir::Function*, 8> cloned_functions{};

    /// A map from rewritten structs to original structs.
    Hashmap<const core::type::Struct*, const core::type::Struct*, 4> struct_to_original{};

    /// Worklist of access instructions that need to be updated.
    Vector<core::ir::Access*, 32> access_worklist{};

    /// Worklist of construct instructions that need to be updated.
    Vector<core::ir::Construct*, 32> construct_worklist{};

    /// Worklist of user calls that need to be updated.
    Vector<core::ir::UserCall*, 32> user_call_worklist{};

    /// Process the module.
    void Process() {
        if (ir.root_block) {
            ProcessBlock(ir.root_block);
        }

        // Update the types of any function parameters and function return types that contain
        // matrices with non-default strides.
        size_t processed_functions = 0;
        while (processed_functions < ir.functions.Length()) {
            // Process all functions not already processed.
            // Note that new functions might have been added when processing
            // user calls. The outer while-loop ensures those functions are
            // processed as well.
            for (; processed_functions < ir.functions.Length(); processed_functions++) {
                auto* func = ir.functions[processed_functions];
                for (auto* param : func->Params()) {
                    param->SetType(RewriteType(param->Type()));
                }
                func->SetReturnType(RewriteType(func->ReturnType()));

                ProcessBlock(func->Block());
            }

            // Update any access instructions that produce strided matrices.
            for (auto* access : access_worklist) {
                UpdateAccessInstruction(access, /* source_is_strided */ false);
            }
            access_worklist.Clear();

            // Convert strided matrix operands for construct instructions.
            for (auto* construct : construct_worklist) {
                ConvertConstructOperands(construct);
            }
            construct_worklist.Clear();

            // Process user calls. This might append new functions to ir.functions.
            for (auto* call : user_call_worklist) {
                ProcessUserCall(call);
            }
            user_call_worklist.Clear();
        }
    }

    /// Process a block iteratively.
    void ProcessBlock(core::ir::Block* block) {
        core::ir::Traverse(block, [&](core::ir::Instruction* inst) {
            // Replace all constant operands where the type will be changed due to it containing a
            // structure that uses a matrix stride attribute.
            for (uint32_t i = 0; i < inst->Operands().Length(); ++i) {
                if (auto* constant = As<core::ir::Constant>(inst->Operands()[i])) {
                    auto* new_constant = RewriteConstant(constant->Value());
                    if (new_constant != constant->Value()) {
                        inst->SetOperand(i, b.Constant(new_constant));
                    }
                }
            }

            // Update any instruction result that contains a matrix stride attribute.
            for (auto* result : inst->Results()) {
                result->SetType(RewriteType(result->Type()));
            }

            // Track instructions that may need to be updated later.
            if (auto* access = inst->As<core::ir::Access>()) {
                access_worklist.Push(access);
            } else if (auto* construct = inst->As<core::ir::Construct>()) {
                construct_worklist.Push(construct);
            } else if (auto* call = inst->As<core::ir::UserCall>()) {
                user_call_worklist.Push(call);
            }
        });
    }

    /// Processes a user call instruction.
    /// If the types of the arguments passed to the call differ from the parameter types of the
    /// target function (due to matrix pointer arguments having been rewritten to array pointers),
    /// this function redirects the call to a specialized clone of the target function that matches
    /// the new argument types.
    void ProcessUserCall(core::ir::UserCall* call) {
        auto* target = call->Target();
        if (target->Params().IsEmpty()) {
            return;
        }

        bool needs_specialization = false;
        Vector<const core::type::Type*, 4> new_param_types;
        new_param_types.Reserve(call->Args().size());

        for (size_t i = 0; i < call->Args().size(); ++i) {
            auto* arg = call->Args()[i];
            auto* param = target->Params()[i];

            if (arg->Type() != param->Type()) {
                needs_specialization = true;
                new_param_types.Push(arg->Type());
            } else {
                new_param_types.Push(param->Type());
            }
        }

        if (!needs_specialization) {
            return;
        }

        auto* specialized_target = GetClonedFunction(target, new_param_types);
        call->SetTarget(specialized_target);
    }

    /// Gets or creates a clone of `func` specialized for the parameter types in `new_param_types`.
    /// This function assumes that the only parameters whose types change are pointers to strided
    /// matrices, and they are replaced by pointers to their decomposed strided arrays.
    core::ir::Function* GetClonedFunction(
        core::ir::Function* func,
        const Vector<const core::type::Type*, 4>& new_param_types) {
        FunctionSpec spec{func, new_param_types};
        return cloned_functions.GetOrAdd(spec, [&] {
            core::ir::CloneContext ctx(ir);

            // Clone the function and its body using the CloneContext.
            auto* new_func = func->Clone(ctx);

            ir.functions.Push(new_func);

            for (size_t i = 0; i < func->Params().Length(); ++i) {
                auto* old_param = func->Params()[i];
                auto* new_param = new_func->Params()[i];
                if (old_param->Type() != new_param_types[i]) {
                    auto* old_ptr = old_param->Type()->As<core::type::Pointer>();
                    auto* new_ptr = new_param_types[i]->As<core::type::Pointer>();
                    TINT_ASSERT(old_ptr && new_ptr);
                    ReplaceMatrixPointerWithArrayPointer(old_ptr, new_ptr->StoreType(), new_param);
                }
            }

            return new_func;
        });
    }

    /// Rewrite a type to replace structure members that have matrix strides.
    const core::type::Type* RewriteType(const core::type::Type* type, uint32_t stride = 0) {
        return type_map.GetOrAdd(TypeAndStride{type, stride}, [&] {
            return tint::Switch(
                type,  //
                [&](const core::type::Matrix* mat) -> const core::type::Type* {
                    if (stride == 0 || stride == mat->ColumnStride()) {
                        return mat;
                    }
                    // Replace the matrix with a strided array of column vectors.
                    TINT_ASSERT(stride % mat->ColumnStride() == 0);
                    return ty.Get<spirv::type::ExplicitLayoutArray>(
                        mat->ColumnType(), ty.Get<core::type::ConstantArrayCount>(mat->Columns()),
                        stride * mat->Columns(), stride);
                },
                [&](const core::type::Array* arr) { return RewriteArray(arr, stride); },
                [&](const core::type::Struct* str) { return RewriteStruct(str); },
                [&](const core::type::Pointer* ptr) {
                    return ty.ptr(ptr->AddressSpace(), RewriteType(ptr->StoreType()),
                                  ptr->Access());
                },
                [&](Default) { return type; });
        });
    }

    /// Rewrite an array type if necessary.
    const core::type::Array* RewriteArray(const core::type::Array* arr, uint32_t matrix_stride) {
        auto* new_element_type = RewriteType(arr->ElemType(), matrix_stride);
        if (new_element_type == arr->ElemType()) {
            return arr;
        }

        // The element type is the only thing that will change. That does not affect the stride of
        // the array itself, which may either be the natural stride or an larger stride in the case
        // of an explicitly laid out array.
        if (auto* ex = arr->As<spirv::type::ExplicitLayoutArray>()) {
            return ty.Get<spirv::type::ExplicitLayoutArray>(new_element_type, arr->Count(),
                                                            arr->Size(), ex->Stride());
        }
        return ty.Get<core::type::Array>(new_element_type, arr->Count(), arr->Size());
    }

    /// Rewrite a structure type to replace structure members that have matrix stride attributes.
    const core::type::Struct* RewriteStruct(const core::type::Struct* old_struct) {
        bool made_changes = false;

        Vector<const core::type::StructMember*, 8> new_members;
        new_members.Reserve(old_struct->Members().Length());
        for (auto* member : old_struct->Members()) {
            auto* new_member_type = RewriteType(member->Type(), member->MatrixStride());
            if (member->HasMatrixStride() || new_member_type != member->Type()) {
                // Recreate the struct member without the stride attribute, and using the new type.
                new_members.Push(ty.Get<core::type::StructMember>(
                    member->Name(), new_member_type, member->Index(), member->Offset(),
                    member->Align(), member->Size(), member->Attributes()));
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

    /// Rewrite a constant to replace strided matrix constants with the equivalent strided array
    /// of column vector constants.
    const core::constant::Value* RewriteConstant(const core::constant::Value* constant,
                                                 uint32_t stride = 0) {
        auto* new_type = RewriteType(constant->Type(), stride);
        if (new_type == constant->Type()) {
            return constant;
        }

        Vector<const core::constant::Value*, 16> elements;
        for (uint32_t i = 0; i < constant->NumElements(); i++) {
            auto* value = constant->Index(i);

            // If this is a struct member, we need to check if the type has changed.
            if (auto* new_struct_type = new_type->As<core::type::Struct>()) {
                auto* new_member_type = new_struct_type->Members()[i]->Type();
                if (new_member_type != value->Type()) {
                    // Create a new constant using the strided array type.
                    // If the type changed, it must have had a MatrixStride decoration and will have
                    // been rewritten as an array type (or it already was an array).
                    auto* array = new_member_type->As<core::type::Array>();
                    TINT_ASSERT(array);

                    auto* old_struct_type = constant->Type()->As<core::type::Struct>();
                    auto member_stride = old_struct_type->Members()[i]->MatrixStride();

                    Vector<const core::constant::Value*, 4> new_elements;
                    for (uint32_t j = 0; j < array->ConstantCount().value(); j++) {
                        new_elements.Push(RewriteConstant(value->Index(j), member_stride));
                    }
                    value = ir.constant_values.Composite(array, std::move(new_elements));
                }
            }

            elements.Push(RewriteConstant(value, stride));
        }
        return ir.constant_values.Composite(new_type, std::move(elements));
    }

    /// Convert strided matrix operands to strided arrays for a construct instruction.
    void ConvertConstructOperands(core::ir::Construct* construct) {
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
                    new_operands.Push(Convert(member_type, operand));
                } else {
                    new_operands.Push(operand);
                }
            }
            construct->SetOperands(new_operands);
        });
    }

    /// Update the result type of an access instruction if needed, and the uses of that result.
    void UpdateAccessInstruction(core::ir::Access* access, bool source_is_strided) {
        // Determine the result type based on the potentially modified object type.
        bool indexed_through_strided_member = source_is_strided;
        auto* current_type = access->Object()->Type()->UnwrapPtr();
        for (auto* idx : access->Indices()) {
            if (auto* struct_type = current_type->As<core::type::Struct>()) {
                auto const_idx = idx->As<core::ir::Constant>()->Value()->ValueAs<uint32_t>();
                current_type = current_type->Element(const_idx);

                // Check if we are indexing into a member that has a non-natural matrix stride.
                auto* original_struct = struct_to_original.GetOr(struct_type, nullptr);
                if (!original_struct) {
                    // The structure type has not changed so cannot have any matrix strides.
                    continue;
                }
                auto* member = original_struct->Members()[const_idx];
                if (member->HasMatrixStride() && current_type != member->Type()) {
                    indexed_through_strided_member = true;
                }
            } else {
                current_type = current_type->Elements().type;
            }
        }
        if (!indexed_through_strided_member || current_type->Is<core::type::Vector>()) {
            return;
        }

        if (auto* ptr = access->Result()->Type()->As<core::type::Pointer>()) {
            ReplaceMatrixPointerWithArrayPointer(ptr, current_type, access->Result());
        } else {
            // We were extracting a strided matrix from a structure, so we need to convert the
            // strided array back to that matrix type.
            b.InsertAfter(access, [&] {
                auto* extracted_array = b.InstructionResult(current_type);
                access->Result()->ReplaceAllUsesWith(
                    Convert(access->Result()->Type(), extracted_array));
                access->SetResult(extracted_array);
            });
        }
    }

    /// Change the type of a pointer value that contains a strided matrix, and then
    /// update any instructions that use that value.
    void ReplaceMatrixPointerWithArrayPointer(const core::type::Pointer* old_ptr,
                                              const core::type::Type* new_store_type,
                                              core::ir::Value* value) {
        auto* old_store_type = old_ptr->StoreType();
        auto* new_ptr = ty.ptr(old_ptr->AddressSpace(), new_store_type, old_ptr->Access());

        value->SetType(new_ptr);

        Vector<core::ir::Value*, 8> worklist{value};
        while (!worklist.IsEmpty()) {
            auto* val = worklist.Pop();
            val->ForEachUseUnsorted([&](const core::ir::Usage& use) {
                tint::Switch(
                    use.instruction,  //
                    [&](core::ir::Access* access) {
                        UpdateAccessInstruction(access, /* source_is_strided */ true);
                    },
                    [&](core::ir::Let* let) {
                        let->Result()->SetType(new_ptr);
                        worklist.Push(let->Result());
                    },
                    [&](core::ir::Load* load) {
                        // Convert the value to the original type.
                        b.InsertAfter(load, [&] {
                            auto* new_load_result = b.InstructionResult(new_store_type);
                            auto* converted = Convert(old_store_type, new_load_result);
                            load->Result()->ReplaceAllUsesWith(converted);
                            load->SetResult(new_load_result);
                        });
                    },
                    [&](core::ir::Store* store) {
                        // Convert the value to the new type.
                        b.InsertBefore(store, [&] {  //
                            store->SetFrom(Convert(new_store_type, store->From()));
                        });
                    },
                    [&](core::ir::UserCall*) {
                        // The argument's type is already updated because it is the value whose type
                        // we changed. The main traversal will see the type mismatch and clone the
                        // target function.
                    },
                    TINT_ICE_ON_NO_MATCH);
            });
        }
    }

    /// Convert a value between an [array of] strided matrix and an [array of] strided array.
    core::ir::Value* Convert(const core::type::Type* dst, core::ir::Value* src) {
        auto dst_elements = dst->Elements();
        auto src_elements = src->Type()->Elements();
        TINT_ASSERT(dst_elements.count == src_elements.count);
        Vector<core::ir::Value*, 8> elements;
        elements.Reserve(dst_elements.count);
        for (uint32_t i = 0; i < dst_elements.count; i++) {
            // Extract the element from the source value.
            core::ir::Value* el = nullptr;
            if (auto* constant = src->As<core::ir::Constant>()) {
                el = b.Constant(constant->Value()->Index(i));
            } else {
                el = b.Access(src_elements.type, src, u32(i))->Result();
            }

            // Recurse to convert strided matrices nested in arrays if needed.
            if (src_elements.type != dst_elements.type) {
                el = Convert(dst_elements.type, el);
            }

            elements.Push(el);
        }
        return b.Construct(dst, std::move(elements))->Result();
    }
};

}  // namespace

Result<SuccessType> DecomposeStridedMatrix(core::ir::Module& ir) {
    AssertValid(ir, "before spirv.DecomposeStridedMatrix");

    State{ir}.Process();

    ir.properties.Remove(core::ir::Property::kAllowStructMatrixDecorations);

    return Success;
}

}  // namespace tint::spirv::reader::lower
