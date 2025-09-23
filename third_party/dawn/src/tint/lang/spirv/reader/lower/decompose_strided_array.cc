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

#include "src/tint/lang/spirv/reader/lower/decompose_strided_array.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/spirv/type/explicit_layout_array.h"

namespace tint::spirv::reader::lower {
namespace {

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
    Hashmap<const core::type::Type*, const core::type::Type*, 32> type_map{};

    /// A map from a constant to the rewritten constant.
    Hashmap<const core::constant::Value*, const core::constant::Value*, 8> constant_map{};

    /// All of the padded structure types that were created by this transform.
    Hashset<const core::type::Type*, 8> padded_structures{};

    /// Process the module.
    void Process() {
        Vector<core::ir::Access*, 8> access_worklist;
        Vector<core::ir::Construct*, 8> construct_worklist;
        for (auto* inst : ir.Instructions()) {
            // Replace all constant operands where the type will be changed, either due to being a
            // strided array, or a composite containing a strided array.
            for (uint32_t i = 0; i < inst->Operands().Length(); ++i) {
                auto* constant = As<core::ir::Constant>(inst->Operands()[i]);
                if (!constant) {
                    continue;
                }

                auto* new_constant = RewriteConstant(constant->Value());
                if (new_constant != constant->Value()) {
                    inst->SetOperand(i, b.Constant(new_constant));
                }
            }

            // Track all construct instructions that need to be updated later.
            if (auto* construct = inst->As<core::ir::Construct>()) {
                if (construct->Result(0)->Type()->Is<spirv::type::ExplicitLayoutArray>()) {
                    construct_worklist.Push(construct);
                }
            }

            // Update any instruction result that contains an array with a non-default stride.
            for (auto* result : inst->Results()) {
                result->SetType(RewriteType(result->Type()));
            }

            // Track all access instructions that may need to be updated later.
            if (auto* access = inst->As<core::ir::Access>()) {
                access_worklist.Push(access);
            }
        }

        // Update the types of any function parameters and function return types that contain arrays
        // with non-default strides.
        for (auto func : ir.functions) {
            for (auto* param : func->Params()) {
                param->SetType(RewriteType(param->Type()));
            }
            func->SetReturnType(RewriteType(func->ReturnType()));
        }

        // Wrap operands for construct instructions in padded structures.
        for (auto* construct : construct_worklist) {
            WrapConstructOperands(construct);
        }

        // Inject additional indices into access instructions where needed.
        // We do this last as it relies on us knowing whether the access chain indexes into padded
        // structures that we have added in the previous steps.
        for (auto* access : access_worklist) {
            UpdateAccessIndices(access);
        }
    }

    /// Rewrite a type to avoid using any arrays that have non-default strides.
    const core::type::Type* RewriteType(const core::type::Type* type) {
        return type_map.GetOrAdd(type, [&] {
            return tint::Switch(
                type,  //
                [&](const spirv::type::ExplicitLayoutArray* arr) {
                    auto* new_element_type = RewriteType(arr->ElemType());

                    if (!arr->IsStrideImplicit()) {
                        // The stride does not match the implicit element stride, so we need to wrap
                        // the element type in a structure that is padded to the target stride.
                        auto* element_struct =
                            ty.Struct(sym.New("tint_padded_array_element"),
                                      Vector{
                                          ty.Get<core::type::StructMember>(
                                              sym.New("tint_element"), new_element_type,
                                              /* index */ 0u,
                                              /* offset */ 0u,
                                              /* align */ new_element_type->Align(),
                                              /* size */ arr->Stride(), core::IOAttributes{}),
                                      });
                        new_element_type = element_struct;
                        padded_structures.Add(element_struct);
                    }

                    return ty.Get<core::type::Array>(new_element_type, arr->Count(), arr->Align(),
                                                     arr->Size(), arr->Stride(), arr->Stride());
                },
                [&](const core::type::Array* arr) {
                    auto* new_element_type = RewriteType(arr->ElemType());
                    TINT_ASSERT(arr->IsStrideImplicit());
                    return ty.Get<core::type::Array>(new_element_type, arr->Count(), arr->Align(),
                                                     arr->Size(), arr->Stride(), arr->Stride());
                },
                [&](const core::type::Struct* str) -> const core::type::Struct* {
                    // Rewrite members of the struct that contain arrays with non-default strides.
                    bool made_changes = false;
                    Vector<const core::type::StructMember*, 8> new_members;
                    new_members.Reserve(str->Members().Length());
                    for (auto* member : str->Members()) {
                        auto* new_member_type = RewriteType(member->Type());
                        if (new_member_type == member->Type()) {
                            new_members.Push(member);
                            continue;
                        }

                        new_members.Push(ty.Get<core::type::StructMember>(
                            member->Name(), new_member_type, member->Index(), member->Offset(),
                            member->Align(), member->Size(), member->Attributes()));
                        made_changes = true;
                    }
                    if (!made_changes) {
                        return str;
                    }

                    return ty.Struct(sym.New(str->Name().Name()), std::move(new_members));
                },
                [&](const core::type::Pointer* ptr) {
                    return ty.ptr(ptr->AddressSpace(), RewriteType(ptr->StoreType()),
                                  ptr->Access());
                },
                [&](Default) { return type; });
        });
    }

    /// Rewrite a constant to avoid using any arrays that have non-default strides.
    const core::constant::Value* RewriteConstant(const core::constant::Value* constant) {
        auto* new_type = RewriteType(constant->Type());
        if (new_type == constant->Type()) {
            return constant;
        }

        // Check if the constant is an array of padded structures.
        // A padded structure will only appear as the element type of an array, so we only need to
        // check the first element type.
        const core::type::Type* padded_struct_type = nullptr;
        if (padded_structures.Contains(new_type->Element(0))) {
            padded_struct_type = new_type->Element(0);
        }

        Vector<const core::constant::Value*, 16> elements;
        for (uint32_t i = 0; i < constant->NumElements(); i++) {
            auto* new_element = RewriteConstant(constant->Index(i));

            // If we are rewriting an array of padded structures, then we need to wrap the original
            // element in a constant structure.
            if (padded_struct_type) {
                new_element = ir.constant_values.Composite(padded_struct_type, Vector{new_element});
            }

            elements.Push(new_element);
        }
        return ir.constant_values.Composite(new_type, std::move(elements));
    }

    /// Wrap all operands for a construct instruction that produces an array with padded elements.
    void WrapConstructOperands(core::ir::Construct* construct) {
        auto* padded_struct_type = construct->Result(0)->Type()->Element(0);
        TINT_ASSERT(padded_struct_type && padded_structures.Contains(padded_struct_type));
        b.InsertBefore(construct, [&] {
            Vector<core::ir::Value*, 8> new_operands;
            for (auto* operand : construct->Operands()) {
                new_operands.Push(b.Construct(padded_struct_type, operand)->Result(0));
            }
            construct->SetOperands(new_operands);
        });
    }

    /// Inject additional `0u` indices into access instructions that index into any padded structure
    /// types that we have created.
    void UpdateAccessIndices(core::ir::Access* access) {
        Vector<core::ir::Value*, 8> new_operands{access->Object()};

        auto old_indices = access->Indices();
        auto* current_type = access->Object()->Type()->UnwrapPtr();
        for (auto* idx : old_indices) {
            new_operands.Push(idx);

            auto* const_idx = idx->As<core::ir::Constant>();
            current_type = const_idx
                               ? current_type->Element(const_idx->Value()->ValueAs<uint32_t>())
                               : current_type->Elements().type;

            if (padded_structures.Contains(current_type)) {
                new_operands.Push(b.Zero<core::u32>());
                current_type = current_type->Element(0);
            }
        }
        access->SetOperands(std::move(new_operands));
    }
};

}  // namespace

Result<SuccessType> DecomposeStridedArray(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.DecomposeStridedArray",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowMultipleEntryPoints,
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
