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

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
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

    /// A map from block to its containing function.
    Hashmap<core::ir::Block*, core::ir::Function*, 64> block_to_function{};

    /// A map from each function to a map from input variable to parameter.
    Hashmap<core::ir::Function*, Hashmap<core::ir::Var*, core::ir::Value*, 4>, 8>
        function_parameter_map{};

    /// The set of output variables that have been processed.
    Hashset<core::ir::Var*, 4> output_variables{};

    /// The mapping from functions to their transitively referenced output variables.
    core::ir::ReferencedModuleVars<core::ir::Module> referenced_output_vars{
        ir, [](const core::ir::Var* var) {
            auto* view = var->Result(0)->Type()->As<core::type::MemoryView>();
            return view && view->AddressSpace() == core::AddressSpace::kOut;
        }};

    /// Process the module.
    void Process() {
        // Process outputs first, as that may introduce new functions that input variables need to
        // be propagated through.
        ProcessOutputs();
        ProcessInputs();
    }

    /// Process output variables.
    /// Changes output variables to the `private` address space and wraps entry points that produce
    /// outputs with new functions that copy the outputs from the private variables to the return
    /// value.
    void ProcessOutputs() {
        // Update entry point functions to return their outputs, using a wrapper function.
        // Use a worklist as `ProcessEntryPointOutputs()` will add new functions.
        Vector<core::ir::Function*, 4> entry_points;
        for (auto& func : ir.functions) {
            if (func->IsEntryPoint()) {
                entry_points.Push(func);
            }
        }
        for (auto& ep : entry_points) {
            ProcessEntryPointOutputs(ep);
        }

        // Remove attributes from all of the original structs and module-scope output variables.
        // This is done last as we need to copy attributes during `ProcessEntryPointOutputs()`.
        for (auto& var : output_variables) {
            var->SetAttributes({});
            if (auto* str = var->Result(0)->Type()->UnwrapPtr()->As<core::type::Struct>()) {
                for (auto* member : str->Members()) {
                    // TODO(crbug.com/tint/745): Remove the const_cast.
                    const_cast<core::type::StructMember*>(member)->SetAttributes({});
                }
            }
        }
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
                auto addrspace = var->Result(0)->Type()->As<core::type::Pointer>()->AddressSpace();
                if (addrspace == core::AddressSpace::kIn) {
                    inputs.Push(var);
                }
            }
        }

        // Replace the input variables with function parameters.
        for (auto* var : inputs) {
            ReplaceInputPointerUses(var, var->Result(0));
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
            if (auto* access = use.instruction->As<core::ir::Access>()) {
                ReplaceOutputPointerAddressSpace(access->Result(0));
            } else if (!use.instruction->IsAnyOf<core::ir::Load, core::ir::LoadVectorElement,
                                                 core::ir::Store, core::ir::StoreVectorElement>()) {
                TINT_UNREACHABLE()
                    << "unexpected instruction: " << use.instruction->TypeInfo().name;
            }
        });
    }

    /// Process the outputs of an entry point function, adding a wrapper function to forward outputs
    /// through the return value.
    /// @param ep the entry point
    void ProcessEntryPointOutputs(core::ir::Function* ep) {
        const auto& referenced_outputs = referenced_output_vars.TransitiveReferences(ep);
        if (referenced_outputs.IsEmpty()) {
            return;
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
        auto add_output = [&](Symbol name, const core::type::Type* type,
                              core::IOAttributes attributes) {
            if (!name) {
                name = ir.symbols.New();
            }
            output_descriptors.Push(core::type::Manager::StructMemberDesc{name, type, attributes});
        };
        for (auto* var : referenced_outputs) {
            // Change the address space of the variable to private and update its uses, if we
            // haven't already seen this variable.
            if (output_variables.Add(var)) {
                ReplaceOutputPointerAddressSpace(var->Result(0));
            }

            // Copy the variable attributes to the struct member.
            auto var_attributes = var->Attributes();
            auto var_type = var->Result(0)->Type()->UnwrapPtr();
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
                    if (auto base_loc = var_attributes.location) {
                        // Location values increment from the base location value on the variable.
                        member_attributes.location = base_loc.value() + member->Index();
                    }
                    if (!member_attributes.interpolation) {
                        member_attributes.interpolation = var_attributes.interpolation;
                    }

                    add_output(member->Name(), member->Type(), std::move(member_attributes));

                    // Load the final result from the member of the original struct variable.
                    b.Append(wrapper->Block(), [&] {  //
                        auto* access =
                            b.Access(ty.ptr<private_>(member->Type()), var, u32(member->Index()));
                        results.Push(b.Load(access)->Result(0));
                    });
                }

                // If we skipped emission of any member, then we need to make sure the var is only
                // used through `access` instructions, otherwise the members may no longer match due
                // to the skipping.
                if (skipped_member_emission) {
                    for (auto& usage : var->Result(0)->UsagesUnsorted()) {
                        TINT_ASSERT(usage->instruction->Is<core::ir::Access>());
                    }
                }
            } else {
                // Load the final result from the original variable.
                b.Append(wrapper->Block(), [&] {
                    results.Push(b.Load(var)->Result(0));

                    // If we're dealing with sample_mask, extract the scalar from the array.
                    if (var_attributes.builtin == core::BuiltinValue::kSampleMask) {
                        var_type = ty.u32();
                        results.Back() = b.Access(ty.u32(), results.Back(), u32(0))->Result(0);
                    }
                });
                add_output(ir.NameOf(var), var_type, std::move(var_attributes));
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
            // TODO(dsinclair): Validate that all accesses of this member are then used only to
            // assign the value of 1.0.
            return true;
        }
        if (member_attributes.builtin == core::BuiltinValue::kCullDistance) {
            TINT_ASSERT(!IsIndexAccessed(var->Result(0), member->Index()));
            return true;
        }
        if (member_attributes.builtin == core::BuiltinValue::kClipDistances) {
            return !IsIndexAccessed(var->Result(0), member->Index());
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
                    l->Result(0)->ReplaceAllUsesWith(object);
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
                    a->Result(0)->SetType(a->Result(0)->Type()->UnwrapPtr());
                    ReplaceInputPointerUses(var, a->Result(0));
                },
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

    /// Get or create a function parameter to replace a module-scope variable.
    /// @param func the function
    /// @param var the module-scope variable
    /// @returns the function parameter
    core::ir::Value* GetParameter(core::ir::Function* func, core::ir::Var* var) {
        return function_parameter_map.GetOrAddZero(func).GetOrAdd(var, [&] {
            const bool entry_point = func->IsEntryPoint();
            auto* var_type = var->Result(0)->Type()->UnwrapPtr();

            // Use a scalar u32 for sample_mask builtins for entry point parameters.
            if (entry_point && var->Attributes().builtin == core::BuiltinValue::kSampleMask) {
                TINT_ASSERT(var_type->Is<core::type::Array>());
                TINT_ASSERT(var_type->As<core::type::Array>()->ConstantCount() == 1u);
                var_type = ty.u32();
            }

            // Create a new function parameter for the input.
            auto* param = b.FunctionParam(var_type);
            func->AppendParam(param);
            if (auto name = ir.NameOf(var)) {
                ir.SetName(param, name);
            }

            // Add attributes to the parameter if this is an entry point function.
            if (entry_point) {
                AddEntryPointParameterAttributes(param, var->Attributes());
            }

            // Update the callsites of this function.
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

            core::ir::Value* result = param;
            if (entry_point && var->Attributes().builtin == core::BuiltinValue::kSampleMask) {
                // Construct an array from the scalar sample_mask builtin value for entry points.
                auto* construct = b.Construct(var->Result(0)->Type()->UnwrapPtr(), param);
                func->Block()->Prepend(construct);
                result = construct->Result(0);
            }
            return result;
        });
    }

    /// Add attributes to an entry point function parameter.
    /// @param param the parameter
    /// @param attributes the attributes
    void AddEntryPointParameterAttributes(core::ir::FunctionParam* param,
                                          const core::IOAttributes& attributes) {
        if (auto* str = param->Type()->UnwrapPtr()->As<core::type::Struct>()) {
            for (auto* member : str->Members()) {
                // Use the base variable attributes if not specified directly on the member.
                auto member_attributes = member->Attributes();
                if (auto base_loc = attributes.location) {
                    // Location values increment from the base location value on the variable.
                    member_attributes.location = base_loc.value() + member->Index();
                }
                if (!member_attributes.interpolation) {
                    member_attributes.interpolation = attributes.interpolation;
                }
                // TODO(crbug.com/tint/745): Remove the const_cast.
                const_cast<core::type::StructMember*>(member)->SetAttributes(
                    std::move(member_attributes));
            }
        } else {
            // Set attributes directly on the function parameter.
            param->SetAttributes(attributes);
        }
    }
};

}  // namespace

Result<SuccessType> ShaderIO(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.ShaderIO");
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::reader::lower
