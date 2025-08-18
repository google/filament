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

#include "src/tint/lang/spirv/writer/raise/fork_explicit_layout_types.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "src/tint/lang/spirv/ir/copy_logical.h"
#include "src/tint/lang/spirv/type/explicit_layout_array.h"
#include "src/tint/lang/spirv/writer/common/options.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer::raise {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The SPIR-V binary version.
    SpvVersion version;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The symbol table.
    SymbolTable& sym{ir.symbols};

    /// The array and structure types that must be emitted without explicit layout decorations.
    Hashset<const core::type::Type*, 16> must_emit_without_explicit_layout{};

    /// A map from original array/struct type to the explicitly laid out version.
    Hashmap<const core::type::Type*, const core::type::Type*, 16> explicit_type_map{};

    /// Helper functions for converting to and from explicitly laid out types.
    Hashmap<const core::type::Type*, core::ir::Function*, 8> conversion_helpers{};

    /// Process the module.
    void Process() {
        // Record arrays and structures that must not have explicit layout decorations, as well as
        // variables in address space that must have explicit layout decorations.
        Vector<core::ir::Var*, 16> vars_requiring_explicit_layout;
        for (auto* inst : ir.Instructions()) {
            auto* var = inst->As<core::ir::Var>();
            if (!var) {
                continue;
            }

            auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
            switch (ptr->AddressSpace()) {
                case core::AddressSpace::kImmediate:
                case core::AddressSpace::kStorage:
                case core::AddressSpace::kUniform:
                    vars_requiring_explicit_layout.Push(var);
                    break;

                case core::AddressSpace::kFunction:
                case core::AddressSpace::kPrivate:
                    // In SPIR-V 1.4 and earlier, Vulkan allowed explicit layout decorations.
                    if (version > SpvVersion::kSpv14) {
                        RecordTypesThatMustNotHaveExplicitLayout(ptr);
                    }
                    break;

                case core::AddressSpace::kWorkgroup:
                case core::AddressSpace::kHandle:
                case core::AddressSpace::kPixelLocal:
                case core::AddressSpace::kIn:
                case core::AddressSpace::kOut:
                    RecordTypesThatMustNotHaveExplicitLayout(ptr);
                    break;

                case core::AddressSpace::kUndefined:
                    break;
            }
        }

        // If a variable that requires explicit layout decorations is using types that will be
        // emitted without explicit layout decorations, we need to rewrite its store type and
        // introduce element-wise copies when loading and storing those types.
        for (auto* var : vars_requiring_explicit_layout) {
            UpdatePointerType(var->Result());
        }
    }

    /// Add all array and structure types nested in @p ptr to the set of types that must be emitted
    /// without explicit layout decorations.
    /// @param ptr the pointer type whose store type will be recorded
    void RecordTypesThatMustNotHaveExplicitLayout(const core::type::Pointer* ptr) {
        // Look for arrays and structures at any nesting depth of this type.
        Vector<const core::type::Type*, 8> type_queue;
        type_queue.Push(ptr->StoreType());
        while (!type_queue.IsEmpty()) {
            auto* next = type_queue.Pop();
            if (auto* str = next->As<core::type::Struct>()) {
                // Record this structure and then check its members if we haven't seen it before.
                if (must_emit_without_explicit_layout.Add(str)) {
                    for (auto* member : str->Members()) {
                        type_queue.Push(member->Type());
                    }
                }
            } else if (auto* arr = next->As<core::type::Array>()) {
                // Record this array and then check its element type if we haven't seen it before.
                if (must_emit_without_explicit_layout.Add(arr)) {
                    type_queue.Push(arr->ElemType());
                }
            }
        }
    }

    /// Recursively fork a type to produce an identical version that will have explicit layout
    /// decorations, if the original is going to be emitted without explicit layout decorations.
    /// @param type the original type to fork
    /// @returns the forked type, or `nullptr` if forking was not necessary
    const core::type::Type* GetForkedType(const core::type::Type* type) {
        return explicit_type_map.GetOrAdd(type, [&]() -> const core::type::Type* {
            if (auto* str = type->As<core::type::Struct>()) {
                return ForkStructIfNeeded(str);
            }
            if (auto* arr = type->As<core::type::Array>()) {
                return ForkArray(arr);
            }

            // Any other type is safe to use unchanged, as they do not have layout decorations.
            return nullptr;
        });
    }

    /// Recursively fork a structure type to produce an identical version that will have explicit
    /// layout decorations, if the original is going to be emitted without explicit layout
    /// decorations.
    /// @param original_struct the original structure type to fork
    /// @returns the forked struct type, or `nullptr` if forking was not necessary
    const core::type::Struct* ForkStructIfNeeded(const core::type::Struct* original_struct) {
        // Fork each member type as necessary.
        bool members_were_forked = false;
        Vector<const core::type::StructMember*, 8> new_members;
        for (auto* member : original_struct->Members()) {
            auto* new_member_type = GetForkedType(member->Type());
            if (!new_member_type) {
                // The member type was not forked, so just use the original member type.
                new_member_type = member->Type();
            } else {
                members_were_forked = true;
            }
            auto index = static_cast<uint32_t>(new_members.Length());
            new_members.Push(ty.Get<core::type::StructMember>(
                member->Name(), new_member_type, index, member->Offset(), member->Align(),
                member->Size(), core::IOAttributes{}));
        }

        // If no members were forked and the struct itself is not shared with other address spaces,
        // then the original struct can safely be reused.
        if (!must_emit_without_explicit_layout.Contains(original_struct) && !members_were_forked) {
            // TODO(crbug.com/tint/745): Remove the const_cast.
            const_cast<core::type::Struct*>(original_struct)
                ->SetStructFlag(core::type::kExplicitLayout);
            return nullptr;
        }

        // Create a new struct with the rewritten members.
        auto name = sym.New(original_struct->Name().Name() + "_tint_explicit_layout");
        auto* new_str = ty.Get<core::type::Struct>(name,                      //
                                                   std::move(new_members),    //
                                                   original_struct->Align(),  //
                                                   original_struct->Size(),   //
                                                   original_struct->SizeNoPadding());
        new_str->SetStructFlag(core::type::kExplicitLayout);
        for (auto flag : original_struct->StructFlags()) {
            new_str->SetStructFlag(flag);
        }
        return new_str;
    }

    /// Recursively fork an array type to produce an identical version that will have explicit
    /// layout decorations when emitted as SPIR-V.
    /// @param original_array the original array type to fork
    /// @returns the forked array type
    const type::ExplicitLayoutArray* ForkArray(const core::type::Array* original_array) {
        auto* new_element_type = GetForkedType(original_array->ElemType());
        if (!new_element_type) {
            // The element type was not forked, so just use the original element type.
            new_element_type = original_array->ElemType();
        }
        return ty.Get<type::ExplicitLayoutArray>(new_element_type,         //
                                                 original_array->Count(),  //
                                                 original_array->Align(),  //
                                                 original_array->Size(),   //
                                                 original_array->Stride());
    }

    /// Update the store type of an instruction result to use the forked version if needed.
    /// Replace any uses of the instruction to take the new type into account.
    /// @param result the instruction result to update
    void UpdatePointerType(core::ir::InstructionResult* result) {
        // Check if the store type needs to be forked.
        auto* ptr = result->Type()->As<core::type::Pointer>();
        auto* forked_type = GetForkedType(ptr->StoreType());
        if (!forked_type) {
            return;
        }

        // Update the store type to the forked type that will have an explicit layout.
        auto* new_ptr = ty.ptr(ptr->AddressSpace(), forked_type, ptr->Access());
        result->SetType(new_ptr);

        // Update any uses of the instruction to take the new type into account.
        // This may introduce manual copies to convert between the forked type and the original.
        result->ForEachUseSorted([&](core::ir::Usage use) {  //
            ReplaceForkedPointerUse(use);
        });
    }

    /// Replace a use of an instruction that produces a pointer to a type that has been forked.
    /// @param use the use of the forked pointer
    void ReplaceForkedPointerUse(core::ir::Usage use) {
        tint::Switch(
            use.instruction,  //
            [&](core::ir::Access* access) {
                // If the access produces a pointer to a type that has been forked, we need to
                // update the result type and then recurse into its uses.
                UpdatePointerType(access->Result());
            },
            [&](ir::BuiltinCall* call) {
                // The only builtin function that takes an array pointer is arrayLength().
                // No change is needed, as it will operate on the explicitly laid out array type.
                TINT_ASSERT(call->Func() == BuiltinFn::kArrayLength);
            },
            [&](core::ir::Let* let) {
                // A let usage will propagate the pointer to a type that has been forked, so we need
                // to update the result type and then recurse into its uses.
                UpdatePointerType(let->Result());
            },
            [&](core::ir::Load* load) {
                b.InsertAfter(load, [&] {
                    // Change the load instruction to produce the forked type, and then convert the
                    // result of the load to the original type.
                    auto* original_type = load->Result()->Type();
                    auto* forked_load = b.InstructionResult(load->From()->Type()->UnwrapPtr());
                    auto* converted = ConvertIfNeeded(original_type, forked_load);
                    load->Result()->ReplaceAllUsesWith(converted);
                    load->SetResult(forked_load);
                });
            },
            [&](core::ir::Store* store) {
                b.InsertBefore(store, [&] {
                    // Convert the `from` operand of the store instruction to the forked type.
                    auto* forked_type = store->To()->Type()->UnwrapPtr();
                    auto* converted = ConvertIfNeeded(forked_type, store->From());
                    store->SetOperand(core::ir::Store::kFromOperandOffset, converted);
                });
            },
            TINT_ICE_ON_NO_MATCH);
    }

    /// Convert a value to/from the explicitly laid out type, if necessary.
    /// @param src the value to convert
    /// @param dst_type the type to convert it to
    /// @returns the converted value
    core::ir::Value* ConvertIfNeeded(const core::type::Type* dst_type, core::ir::Value* src) {
        // If the source type is already the same as the destination type, there is nothing to do.
        auto* src_type = src->Type();
        if (src_type == dst_type) {
            return src;
        }

        // In SPIR-V 1.4 or later, use OpCopyLogical instead of member-wise copying.
        if (version >= SpvVersion::kSpv14) {
            auto* copy =
                ir.CreateInstruction<spirv::ir::CopyLogical>(b.InstructionResult(dst_type), src);
            return b.Append(copy)->Result();
        }
        // Create a helper function to do the conversion.
        auto* helper = conversion_helpers.GetOrAdd(src_type, [&] {
            auto* param = b.FunctionParam("tint_source", src_type);
            auto* func = b.Function("tint_convert_explicit_layout", dst_type);
            func->AppendParam(param);
            b.Append(func->Block(), [&] {
                if (auto* src_struct = src_type->As<core::type::Struct>()) {
                    auto* dst_struct = dst_type->As<core::type::Struct>();
                    b.Return(func, ConvertStruct(src_struct, dst_struct, param));
                } else if (auto* src_arr = src_type->As<core::type::Array>()) {
                    auto* dst_arr = dst_type->As<core::type::Array>();
                    b.Return(func, ConvertArray(src_arr, dst_arr, param));
                } else {
                    TINT_UNREACHABLE();
                }
            });
            return func;
        });
        return b.Call(helper, src)->Result();
    }

    /// Recursively convert a struct type to/from the explicitly laid out version.
    core::ir::Value* ConvertStruct(const core::type::Struct* src_struct,
                                   const core::type::Struct* dst_struct,
                                   core::ir::Value* input) {
        // Convert each member separately and the reconstruct the target struct.
        Vector<core::ir::Value*, 4> construct_args;
        for (uint32_t i = 0; i < dst_struct->Members().Length(); i++) {
            auto* src_member = src_struct->Members()[i];
            auto* dst_member = dst_struct->Members()[i];
            auto* extracted = b.Access(src_member->Type(), input, u32(i))->Result();
            auto* converted = ConvertIfNeeded(dst_member->Type(), extracted);
            construct_args.Push(converted);
        }
        return b.Construct(dst_struct, std::move(construct_args))->Result();
    }

    /// Recursively convert an array type to/from the explicitly laid out version.
    core::ir::Value* ConvertArray(const core::type::Array* src_array,
                                  const core::type::Array* dst_array,
                                  core::ir::Value* input) {
        // Runtime-sized arrays will never be converted as you cannot load/store them, and
        // pipeline-overrides will already have been substituted before this transform.
        auto* count = src_array->Count()->As<core::type::ConstantArrayCount>();
        TINT_ASSERT(count && count == dst_array->Count());

        // Create a local variable to hold the converted result.
        // Convert each element one at a time, writing into the local variable.
        auto* result = b.Var(ty.ptr<function>(dst_array));
        b.LoopRange(ty, 0_u, u32(count->value), 1_u, [&](core::ir::Value* idx) {
            auto* extracted = b.Access(src_array->ElemType(), input, idx)->Result();
            auto* converted = ConvertIfNeeded(dst_array->ElemType(), extracted);
            auto* dst_ptr = b.Access(ty.ptr(function, dst_array->ElemType()), result, idx);
            b.Store(dst_ptr, converted);
        });

        return b.Load(result)->Result();
    }
};

}  // namespace

Result<SuccessType> ForkExplicitLayoutTypes(core::ir::Module& ir, SpvVersion version) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.ForkExplicitLayoutTypes",
                                          kForkExplicitLayoutTypesCapabilities);
    if (result != Success) {
        return result;
    }

    State{ir, version}.Process();

    return Success;
}

}  // namespace tint::spirv::writer::raise
