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

#include "src/tint/lang/spirv/reader/lower/shader_io.h"

#include <string>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
#include "src/tint/lang/core/ir/validator.h"

namespace tint::spirv::reader::lower {

namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// A map from block to its containing function.
    Hashmap<core::ir::Block*, core::ir::Function*, 64> block_to_function{};

    /// A map from each function to a map from input variable to parameter.
    Hashmap<core::ir::Function*, Hashmap<core::ir::Var*, core::ir::Value*, 4>, 8>
        function_parameter_map{};

    /// The set of output variables that have been processed.
    Hashset<core::ir::Var*, 4> output_variables{};

    /// The set of structs used as input variable types
    Hashset<const core::type::Struct*, 4> input_structs{};

    /// The mapping from functions to their transitively referenced output variables.
    core::ir::ReferencedModuleVars<core::ir::Module> referenced_output_vars{
        ir, [](const core::ir::Var* var) {
            auto* view = var->Result()->Type()->As<core::type::MemoryView>();
            return view && view->AddressSpace() == core::AddressSpace::kOut;
        }};

    /// Process the module.
    Result<SuccessType> Process() {
        // Process outputs first, as that may introduce new functions that input variables need to
        // be propagated through.
        if (auto result = ProcessOutputs(); result != Success) {
            return result;
        }
        ProcessInputs();

        auto clean_members = [](const core::type::Struct* strct) {
            for (auto* member : strct->Members()) {
                // TODO(crbug.com/tint/745): Remove the const_cast.
                const_cast<core::type::StructMember*>(member)->ResetAttributes();
            }
        };

        // Remove attributes from all of the original structs and module-scope output variables.
        // This is done last as we need to copy attributes during `ProcessEntryPointOutputs()` and
        // we need access to any struct locations during processing of inputs.
        for (auto& var : output_variables) {
            var->ResetAttributes();
            if (auto* str = var->Result()->Type()->UnwrapPtr()->As<core::type::Struct>()) {
                clean_members(str);
            }
        }

        // All input structs have been reduced to parameters on the entry point so remove any
        // annotations from the structure members.
        for (auto& strct : input_structs) {
            clean_members(strct);
        }
        return Success;
    }

    /// Process output variables.
    /// Changes output variables to the `private` address space and wraps entry points that produce
    /// outputs with new functions that copy the outputs from the private variables to the return
    /// value.
    Result<SuccessType> ProcessOutputs() {
        // Update entry point functions to return their outputs, using a wrapper function.
        // Use a worklist as `ProcessEntryPointOutputs()` will add new functions.
        Vector<core::ir::Function*, 4> entry_points;
        for (auto& func : ir.functions) {
            if (func->IsEntryPoint()) {
                entry_points.Push(func);
            }
        }
        for (auto& ep : entry_points) {
            if (auto result = ProcessEntryPointOutputs(ep); result != Success) {
                return result;
            }
        }
        return Success;
    }

    /// Process input variables.
    /// Pass inputs down the call stack as parameters to any functions that need them.
    void ProcessInputs() {
        // Seed the block-to-function map with the function entry blocks.
        for (auto& func : ir.functions) {
            block_to_function.Add(func->Block(), func);
        }

        // Gather the list of all module-scope input variables.
        Vector<core::ir::Var*, 4> inputs;
        for (auto* global : *ir.root_block) {
            if (auto* var = global->As<core::ir::Var>()) {
                auto addrspace = var->Result()->Type()->As<core::type::Pointer>()->AddressSpace();
                if (addrspace == core::AddressSpace::kIn) {
                    inputs.Push(var);
                }
            }
        }

        // Replace the input variables with function parameters.
        for (auto* var : inputs) {
            ReplaceInputPointerUses(var, var->Result());
            var->Destroy();
        }
    }

    /// Replace an output pointer address space to make it `private`.
    /// @param value the output variable
    void ReplaceOutputPointerAddressSpace(core::ir::InstructionResult* value) {
        // Change the address space to `private`.
        auto* old_ptr_type = value->Type();
        auto* new_ptr_type = ty.ptr(core::AddressSpace::kPrivate, old_ptr_type->UnwrapPtr());
        value->SetType(new_ptr_type);

        // Update all uses of the module-scope variable.
        value->ForEachUseUnsorted([&](core::ir::Usage use) {
            if (use.instruction->IsAnyOf<core::ir::Access, core::ir::Let>()) {
                ReplaceOutputPointerAddressSpace(use.instruction->Result());
            } else if (use.instruction->Is<core::ir::Phony>()) {
                use.instruction->Destroy();
            } else if (!use.instruction->IsAnyOf<core::ir::Load, core::ir::LoadVectorElement,
                                                 core::ir::Store, core::ir::StoreVectorElement>()) {
                TINT_UNREACHABLE()
                    << "unexpected instruction: " << use.instruction->TypeInfo().name;
            }
        });
    }

    void AddOutput(Vector<core::type::Manager::StructMemberDesc, 4>& output_descriptors,
                   Vector<core::ir::Value*, 4>& results,
                   core::ir::Value* from,
                   Symbol name,
                   const core::type::Type* type,
                   core::IOAttributes& attributes) {
        if (!name) {
            name = ir.symbols.New();
        }

        if ((type->Is<core::type::Vector>() || type->IsScalar()) ||
            attributes.builtin == core::BuiltinValue::kClipDistances) {
            output_descriptors.Push(core::type::Manager::StructMemberDesc{name, type, attributes});
            if (attributes.location.has_value()) {
                attributes.location = {attributes.location.value() + 1};
            }
            results.Push(from);
            return;
        }

        tint::Switch(
            type,
            [&](const core::type::Struct* strct) {
                const auto& members = strct->Members();
                auto len = members.Length();
                for (size_t i = 0; i < len; ++i) {
                    auto& mem = members[i];

                    auto mem_attrs = mem->Attributes();
                    if (!mem_attrs.location.has_value()) {
                        mem_attrs.location = attributes.location;
                    }
                    if (!mem_attrs.interpolation) {
                        mem_attrs.interpolation = attributes.interpolation;
                    }

                    auto* a = b.Access(mem->Type(), from, u32(mem->Index()));

                    AddOutput(output_descriptors, results, a->Result(), Symbol{}, mem->Type(),
                              mem_attrs);
                    attributes.location = mem_attrs.location;
                }
            },
            [&](const core::type::Array* ary) {
                auto cnt = ary->ConstantCount();
                TINT_ASSERT(cnt.has_value());

                auto* ary_ty = ary->ElemType();
                for (size_t i = 0; i < cnt; ++i) {
                    auto* a = b.Access(ary_ty, from, u32(i));
                    AddOutput(output_descriptors, results, a->Result(), Symbol{}, ary_ty,
                              attributes);
                }
            },
            [&](const core::type::Matrix* mat) {
                auto* row_ty = ty.vec(mat->DeepestElement(), mat->Rows());
                for (size_t i = 0; i < mat->Columns(); ++i) {
                    auto* a = b.Access(row_ty, from, u32(i));
                    AddOutput(output_descriptors, results, a->Result(), Symbol{}, row_ty,
                              attributes);
                }
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Process the outputs of an entry point function, adding a wrapper function to forward outputs
    /// through the return value.
    /// @param ep the entry point
    Result<SuccessType> ProcessEntryPointOutputs(core::ir::Function* ep) {
        const auto& referenced_outputs = referenced_output_vars.TransitiveReferences(ep);
        if (referenced_outputs.IsEmpty()) {
            return Success;
        }

        // Add a wrapper function to return either a single value or a struct.
        auto* wrapper = b.Function(ty.void_(), ep->Stage());
        if (auto name = ir.NameOf(ep)) {
            ir.SetName(ep, name.Name() + "_inner");
            ir.SetName(wrapper, name);
        }

        // Call the original entry point and make it a regular function.
        ep->SetStage(core::ir::Function::PipelineStage::kUndefined);
        b.Append(wrapper->Block(), [&] {  //
            b.Call(ep);
        });

        // Collect all outputs into a list of struct member declarations.
        // Also add instructions to load their final values in the wrapper function.
        Vector<core::ir::Value*, 4> results;
        Vector<core::type::Manager::StructMemberDesc, 4> output_descriptors;

        for (auto* var : referenced_outputs) {
            // Change the address space of the variable to private and update its uses, if we
            // haven't already seen this variable.
            if (output_variables.Add(var)) {
                ReplaceOutputPointerAddressSpace(var->Result());
            }

            // Copy the variable attributes to the struct member.
            auto var_attributes = var->Attributes();
            auto var_type = var->Result()->Type()->UnwrapPtr();
            Result<SuccessType> result = Success;
            b.Append(wrapper->Block(), [&] {
                if (auto* str = var_type->As<core::type::Struct>()) {
                    bool skipped_member_emission = false;

                    // Add an output for each member of the struct.
                    for (auto* member : str->Members()) {
                        if (ShouldSkipMemberEmission(var, member)) {
                            skipped_member_emission = true;
                            continue;
                        }

                        // Use the base variable attributes if not specified directly on the member.
                        auto member_attributes = member->Attributes();
                        if (!member_attributes.location.has_value()) {
                            member_attributes.location = var_attributes.location;
                        }
                        if (!member_attributes.interpolation) {
                            member_attributes.interpolation = var_attributes.interpolation;
                        }

                        // Load the final result from the member of the original struct variable.
                        auto* access =
                            b.Access(ty.ptr<private_>(member->Type()), var, u32(member->Index()));

                        AddOutput(output_descriptors, results, b.Load(access)->Result(),
                                  member->Name(), member->Type(), member_attributes);
                        var_attributes.location = member_attributes.location;
                    }

                    // If we skipped emission of any member, then we need to make sure the var is
                    // only used through `access` instructions, otherwise the members may no longer
                    // match due to the skipping.
                    if (skipped_member_emission) {
                        for (auto& usage : var->Result()->UsagesUnsorted()) {
                            auto* access = usage->instruction->As<core::ir::Access>();
                            TINT_ASSERT(access);
                            auto* const_idx = access->Indices()[0]->As<core::ir::Constant>();
                            TINT_ASSERT(const_idx);

                            // Check that pointsize members are only assigned values of 1.0.
                            auto* member = str->Members()[const_idx->Value()->ValueAs<uint32_t>()];
                            if (member->Attributes().builtin == core::BuiltinValue::kPointSize) {
                                result = ValidatePointSizeStore(access);
                                return;
                            }
                        }
                    }
                } else {
                    // Don't want to emit point size as it doesn't exist in WGSL.
                    if (var->Attributes().builtin == core::BuiltinValue::kPointSize) {
                        var->SetInitializer(b.Constant(1.0_f));
                        result = ValidatePointSizeStore(var);
                        return;
                    }

                    // Load the final result from the original variable.
                    auto* ld = b.Load(var);

                    core::ir::Value* from = nullptr;
                    // If we're dealing with sample_mask, extract the scalar from the array.
                    if (var_attributes.builtin == core::BuiltinValue::kSampleMask) {
                        // The SPIR-V mask can be either i32 or u32, but WGSL is only u32. So,
                        // convert if necessary.
                        auto* access =
                            b.Access(ld->Result()->Type()->DeepestElement(), ld, u32(0))->Result();
                        if (access->Type()->IsSignedIntegerScalar()) {
                            access = b.Convert(ty.u32(), access)->Result();
                        }
                        from = access;
                        var_type = ty.u32();
                    } else {
                        from = ld->Result();
                    }

                    AddOutput(output_descriptors, results, from, ir.NameOf(var), var_type,
                              var_attributes);
                }
            });
            if (result != Success) {
                return result;
            }
        }

        if (output_descriptors.Length() == 1) {
            // Copy the output attributes to the function return.
            wrapper->SetReturnAttributes(output_descriptors[0].attributes);

            // Return the output from the wrapper function.
            wrapper->SetReturnType(output_descriptors[0].type);
            b.Append(wrapper->Block(), [&] {  //
                b.Return(wrapper, results[0]);
            });
        } else {
            // Create a struct to hold all of the output values.
            auto* str = ty.Struct(ir.symbols.New(), std::move(output_descriptors));
            wrapper->SetReturnType(str);

            // Collect the output values and return them from the wrapper function.
            b.Append(wrapper->Block(), [&] {  //
                b.Return(wrapper, b.Construct(str, std::move(results)));
            });
        }
        return Success;
    }

    /// Validates that a `point_size` builtin is only ever stored to with the constant value `1.0`,
    /// or is loaded from.
    /// @param point_size the `point_size` pointer
    /// @returns success if the validation passes, otherwise a failure.
    Result<SuccessType> ValidatePointSizeStore(core::ir::Instruction* point_size) {
        Vector<core::ir::Instruction*, 4> worklist;
        point_size->Result()->ForEachUseUnsorted([&](core::ir::Usage use) {  //
            worklist.Push(use.instruction);
        });

        while (!worklist.IsEmpty()) {
            auto* inst = worklist.Pop();
            if (auto* store = inst->As<core::ir::Store>()) {
                auto* constant = store->From()->As<core::ir::Constant>();
                if (!constant) {
                    return Failure{"store to point_size is not a constant"};
                }
                TINT_ASSERT(constant->Type()->Is<core::type::F32>());
                if (constant->Value()->ValueAs<f32>() != 1.0f) {
                    return Failure{"store to point_size is not 1.0"};
                }
            } else if (inst->Is<core::ir::Load>()) {
                // Load instructions are OK.
            } else if (auto* let = inst->As<core::ir::Let>()) {
                // Check all uses of the let instruction.
                let->Result()->ForEachUseUnsorted([&](core::ir::Usage use) {  //
                    worklist.Push(use.instruction);
                });
            } else {
                return Failure{"unhandled use of a point_size variable: " +
                               std::string(inst->TypeInfo().name)};
            }
        }
        return Success;
    }

    /// Returns true if the struct member should be skipped on emission
    /// @param var the var which references the structure
    /// @param member the member to check
    /// @returns true if the member should be skipped.
    bool ShouldSkipMemberEmission(core::ir::Var* var, const core::type::StructMember* member) {
        auto var_attributes = var->Attributes();
        auto member_attributes = member->Attributes();

        // If neither the var, nor the member has attributes, then skip
        if (!var_attributes.builtin.has_value() && !var_attributes.color.has_value() &&
            !var_attributes.location.has_value()) {
            if (!member_attributes.builtin.has_value() && !member_attributes.color.has_value() &&
                !member_attributes.location.has_value()) {
                return true;
            }
        }

        // The `gl_PerVertex` structure always gets emitted by glslang, but it may only be used by
        // the `gl_Position` variable. The structure will also contain the `gl_PointSize`,
        // `gl_ClipDistance` and `gl_CullDistance`.

        if (member_attributes.builtin == core::BuiltinValue::kPointSize) {
            return true;
        }
        if (member_attributes.builtin == core::BuiltinValue::kCullDistance) {
            TINT_ASSERT(!IsIndexAccessed(var->Result(), member->Index()));
            return true;
        }
        if (member_attributes.builtin == core::BuiltinValue::kClipDistances) {
            return !IsIndexAccessed(var->Result(), member->Index());
        }
        return false;
    }

    /// Returns true if the `idx` member of `val` is accessed. The `val` must be of type
    /// `Structure` which contains the given member index.
    /// @param val the value to check
    /// @param idx the index to look for
    /// @returns true if `idx` is accessed.
    bool IsIndexAccessed(core::ir::Value* val, uint32_t idx) {
        for (auto& usage : val->UsagesUnsorted()) {
            // Only care about access chains
            auto* chain = usage->instruction->As<core::ir::Access>();
            if (!chain) {
                continue;
            }
            TINT_ASSERT(chain->Indices().Length() >= 1);

            // A member access has to be a constant index
            auto* cnst = chain->Indices()[0]->As<core::ir::Constant>();
            if (!cnst) {
                continue;
            }

            uint32_t v = cnst->Value()->ValueAs<uint32_t>();
            if (v == idx) {
                return true;
            }
        }
        return false;
    }

    /// Replace a use of an input pointer value.
    /// @param var the originating input variable
    /// @param value the input pointer value
    void ReplaceInputPointerUses(core::ir::Var* var, core::ir::Value* value) {
        Vector<core::ir::Instruction*, 8> to_destroy;
        value->ForEachUseUnsorted([&](core::ir::Usage use) {
            auto* object = value;
            if (object->Type()->Is<core::type::Pointer>()) {
                // Get (or create) the function parameter that will replace the variable.
                auto* func = ContainingFunction(use.instruction);
                object = GetParameter(func, var);
            }

            Switch(
                use.instruction,
                [&](core::ir::Load* l) {
                    // Fold the load away and replace its uses with the new parameter.
                    l->Result()->ReplaceAllUsesWith(object);
                    to_destroy.Push(l);
                },
                [&](core::ir::LoadVectorElement* lve) {
                    // Replace the vector element load with an access instruction.
                    auto* access = b.AccessWithResult(lve->DetachResult(), object, lve->Index());
                    access->InsertBefore(lve);
                    to_destroy.Push(lve);
                },
                [&](core::ir::Access* a) {
                    // Remove the pointer from the source and destination type.
                    a->SetOperand(core::ir::Access::kObjectOperandOffset, object);
                    a->Result()->SetType(a->Result()->Type()->UnwrapPtr());
                    ReplaceInputPointerUses(var, a->Result());
                },
                [&](core::ir::Let* l) {
                    // Fold away
                    ReplaceInputPointerUses(var, l->Result());
                    to_destroy.Push(l);
                },
                [&](core::ir::Phony* p) { to_destroy.Push(p); },  //
                TINT_ICE_ON_NO_MATCH);
        });

        // Clean up orphaned instructions.
        for (auto* inst : to_destroy) {
            inst->Destroy();
        }
    }

    /// Get the function that contains an instruction.
    /// @param inst the instruction
    /// @returns the function
    core::ir::Function* ContainingFunction(core::ir::Instruction* inst) {
        return block_to_function.GetOrAdd(inst->Block(), [&] {  //
            return ContainingFunction(inst->Block()->Parent());
        });
    }

    core::ir::Value* GetEntryPointParameter(core::ir::Function* func, core::ir::Var* var) {
        auto* var_type = var->Result()->Type()->UnwrapPtr();

        // The SPIR_V type may not match the required WGSL entry point type, swap them as
        // needed.
        if (var->Attributes().builtin.has_value()) {
            switch (var->Attributes().builtin.value()) {
                case core::BuiltinValue::kSampleMask: {
                    TINT_ASSERT(var_type->Is<core::type::Array>());
                    TINT_ASSERT(var_type->As<core::type::Array>()->ConstantCount() == 1u);
                    var_type = ty.u32();
                    break;
                }
                case core::BuiltinValue::kInstanceIndex:
                case core::BuiltinValue::kVertexIndex:
                case core::BuiltinValue::kLocalInvocationIndex:
                case core::BuiltinValue::kSubgroupInvocationId:
                case core::BuiltinValue::kSubgroupSize:
                case core::BuiltinValue::kSampleIndex: {
                    var_type = ty.u32();
                    break;
                }
                case core::BuiltinValue::kLocalInvocationId:
                case core::BuiltinValue::kGlobalInvocationId:
                case core::BuiltinValue::kWorkgroupId:
                case core::BuiltinValue::kNumWorkgroups: {
                    var_type = ty.vec3<u32>();
                    break;
                }
                default: {
                    break;
                }
            }
        }

        // Create a new function parameter for the input.
        core::ir::Value* param = nullptr;
        b.InsertBefore(func->Block()->Front(),
                       [&] { param = CreateParam(func, var_type, var->Attributes()); });

        if (auto name = ir.NameOf(var)) {
            ir.SetName(param, name);
        }

        core::ir::Value* result = param;
        if (var->Attributes().builtin.has_value()) {
            switch (var->Attributes().builtin.value()) {
                case core::BuiltinValue::kSampleMask: {
                    // Construct an array from the scalar sample_mask builtin value for entry
                    // points.

                    auto* mask_ty = var->Result()->Type()->UnwrapPtr()->As<core::type::Array>();
                    TINT_ASSERT(mask_ty);

                    // If the SPIR-V mask was an i32, need to convert from the u32 provided by
                    // WGSL.
                    if (mask_ty->ElemType()->IsSignedIntegerScalar()) {
                        auto* conv = b.Convert(ty.i32(), result);
                        func->Block()->Prepend(conv);

                        auto* construct = b.Construct(mask_ty, conv);
                        construct->InsertAfter(conv);
                        result = construct->Result();
                    } else {
                        auto* construct = b.Construct(mask_ty, result);
                        func->Block()->Prepend(construct);
                        result = construct->Result();
                    }
                    break;
                }
                case core::BuiltinValue::kInstanceIndex:
                case core::BuiltinValue::kVertexIndex:
                case core::BuiltinValue::kLocalInvocationIndex:
                case core::BuiltinValue::kSubgroupInvocationId:
                case core::BuiltinValue::kSubgroupSize:
                case core::BuiltinValue::kSampleIndex: {
                    auto* idx_ty = var->Result()->Type()->UnwrapPtr();
                    if (idx_ty->IsSignedIntegerScalar()) {
                        auto* conv = b.Convert(ty.i32(), result);
                        func->Block()->Prepend(conv);
                        result = conv->Result();
                    }
                    break;
                }
                case core::BuiltinValue::kLocalInvocationId:
                case core::BuiltinValue::kGlobalInvocationId:
                case core::BuiltinValue::kWorkgroupId:
                case core::BuiltinValue::kNumWorkgroups: {
                    auto* idx_ty = var->Result()->Type()->UnwrapPtr();
                    auto* elem_ty = idx_ty->DeepestElement();
                    if (elem_ty->IsSignedIntegerScalar()) {
                        auto* conv = b.Convert(ty.MatchWidth(ty.i32(), idx_ty), result);
                        func->Block()->Prepend(conv);
                        result = conv->Result();
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }
        return result;
    }

    /// Get or create a function parameter to replace a module-scope variable.
    /// @param func the function
    /// @param var the module-scope variable
    /// @returns the function parameter
    core::ir::Value* GetParameter(core::ir::Function* func, core::ir::Var* var) {
        return function_parameter_map.GetOrAddZero(func).GetOrAdd(var, [&]() -> core::ir::Value* {
            if (func->IsEntryPoint()) {
                return GetEntryPointParameter(func, var);
            }

            // Create a new function parameter for the input.
            auto* param = b.FunctionParam(var->Result()->Type()->UnwrapPtr());
            func->AppendParam(param);
            if (auto name = ir.NameOf(var)) {
                ir.SetName(param, name);
            }

            // Update the call sites of this function.
            func->ForEachUseUnsorted([&](core::ir::Usage use) {
                if (auto* call = use.instruction->As<core::ir::UserCall>()) {
                    // Recurse into the calling function.
                    auto* caller = ContainingFunction(call);
                    call->AppendArg(GetParameter(caller, var));
                } else if (!use.instruction->Is<core::ir::Return>()) {
                    TINT_UNREACHABLE()
                        << "unexpected instruction: " << use.instruction->TypeInfo().name;
                }
            });

            return param;
        });
    }

    core::ir::Value* CreateParam(core::ir::Function* func,
                                 const core::type::Type* type,
                                 core::IOAttributes& attributes) {
        if (type->IsScalar() || type->Is<core::type::Vector>()) {
            auto* fp = b.FunctionParam(type);
            fp->SetAttributes(attributes);

            if (attributes.location.has_value()) {
                attributes.location = {attributes.location.value() + 1};
            }

            func->AppendParam(fp);
            return fp;
        }

        Vector<core::ir::Value*, 4> params;
        tint::Switch(
            type,  //
            [&](const core::type::Array* ary) {
                auto cnt = ary->ConstantCount();
                TINT_ASSERT(cnt.has_value());

                params.Reserve(cnt.value());

                auto* ary_ty = ary->ElemType();
                for (size_t i = 0; i < cnt; ++i) {
                    params.Push(CreateParam(func, ary_ty, attributes));
                }
            },
            [&](const core::type::Matrix* mat) {
                params.Reserve(mat->Columns());

                auto* row_ty = ty.vec(mat->DeepestElement(), mat->Rows());
                for (size_t i = 0; i < mat->Columns(); ++i) {
                    params.Push(CreateParam(func, row_ty, attributes));
                }
            },
            [&](const core::type::Struct* strct) {
                params.Reserve(strct->Members().Length());

                input_structs.Add(strct);

                const auto& members = strct->Members();
                auto len = members.Length();
                for (size_t i = 0; i < len; ++i) {
                    auto& mem = members[i];

                    auto mem_attrs = mem->Attributes();
                    if (attributes.location.has_value()) {
                        mem_attrs.location = attributes.location.value();
                    }
                    if (!mem_attrs.interpolation) {
                        mem_attrs.interpolation = attributes.interpolation;
                    }

                    params.Push(CreateParam(func, mem->Type(), mem_attrs));
                    attributes.location = mem_attrs.location;
                }
            },  //
            TINT_ICE_ON_NO_MATCH);

        return b.Construct(type, params)->Result();
    }
};

}  // namespace

Result<SuccessType> ShaderIO(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.ShaderIO",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowMultipleEntryPoints,
                                              core::ir::Capability::kAllowOverrides,
                                              core::ir::Capability::kAllowPhonyInstructions,
                                              core::ir::Capability::kAllowNonCoreTypes,
                                              core::ir::Capability::kAllowStructMatrixDecorations,
                                          });
    if (result != Success) {
        return result.Failure();
    }

    return State{ir}.Process();
}

}  // namespace tint::spirv::reader::lower
