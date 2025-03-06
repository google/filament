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

#include "src/tint/lang/msl/writer/raise/module_scope_vars.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
#include "src/tint/lang/core/ir/validator.h"

namespace tint::msl::writer::raise {
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

    /// The type of the structure that will contain all of the module-scope variables.
    const core::type::Struct* struct_type = nullptr;

    /// The list of module-scope variables.
    Vector<core::ir::Var*, 8> module_vars{};

    /// A map from a function to the value that contains the module-scope variable pointers.
    Hashmap<core::ir::Function*, core::ir::Value*, 8> function_to_struct_value{};

    /// A map from block to its containing function.
    Hashmap<core::ir::Block*, core::ir::Function*, 64> block_to_function{};

    /// The mapping from functions to their transitively referenced workgroup variables.
    core::ir::ReferencedModuleVars<core::ir::Module> referenced_module_vars{ir};

    // The name of the module-scope variables structure.
    static constexpr const char* kModuleVarsName = "tint_module_vars";

    /// Process the module.
    void Process() {
        // Seed the block-to-function map with the function entry blocks.
        // This is used to determine the owning function for any given instruction.
        for (auto& func : ir.functions) {
            block_to_function.Add(func->Block(), func);
        }

        // Create the structure to hold all module-scope variables.
        // This includes all variables declared in the module, even those that are unused by one or
        // more entry points.
        CreateStruct();

        // Process functions in reverse-dependency order (i.e. root to leaves).
        // This is so that when we update the callsites for a function to add the new argument, we
        // will have already added the necessary structure to the callers.
        auto functions = ir.DependencyOrderedFunctions();
        for (auto func = functions.rbegin(); func != functions.rend(); func++) {
            ProcessFunction(*func);
        }

        // Replace uses of each module-scope variable with values extracted from the structure.
        uint32_t index = 0;
        for (auto& var : module_vars) {
            Vector<core::ir::Instruction*, 16> to_destroy;
            auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();
            var->Result(0)->ForEachUseUnsorted([&](core::ir::Usage use) {  //
                auto* extracted_variable = GetVariableFromStruct(var, use.instruction, index);

                // We drop the pointer from handle variables and store them in the struct by value
                // instead, so remove any load instructions for the handle address space.
                if (use.instruction->Is<core::ir::Load>() &&
                    ptr->AddressSpace() == core::AddressSpace::kHandle) {
                    use.instruction->Result(0)->ReplaceAllUsesWith(extracted_variable);
                    to_destroy.Push(use.instruction);
                    return;
                }

                use.instruction->SetOperand(use.operand_index, extracted_variable);
            });
            var->Destroy();
            index++;

            // Clean up instructions that need to be removed.
            for (auto* inst : to_destroy) {
                inst->Destroy();
            }
        }
    }

    /// Create the structure type to hold all of the module-scope variables.
    void CreateStruct() {
        // Collect a list of struct members for the variable declarations.
        Vector<core::type::Manager::StructMemberDesc, 8> struct_members;
        for (auto* global : *ir.root_block) {
            if (auto* var = global->As<core::ir::Var>()) {
                auto* type = var->Result(0)->Type();

                // Handle types drop the pointer and are passed around by value.
                auto* ptr = type->As<core::type::Pointer>();
                if (ptr->AddressSpace() == core::AddressSpace::kHandle) {
                    type = ptr->StoreType();
                }

                auto name = ir.NameOf(var);
                if (!name) {
                    name = ir.symbols.New();
                }
                module_vars.Push(var);
                struct_members.Push(core::type::Manager::StructMemberDesc{name, type});
            }
        }
        if (struct_members.IsEmpty()) {
            return;
        }

        // Create the structure.
        auto name = ir.symbols.New("tint_module_vars_struct");
        struct_type = ty.Struct(name, std::move(struct_members));
    }

    /// Process a function.
    void ProcessFunction(core::ir::Function* func) {
        auto& refs = referenced_module_vars.TransitiveReferences(func);
        if (refs.IsEmpty()) {
            // No module-scope variables are referenced from this function, so no changes needed.
            return;
        }

        // Add the structure the holds the module-scope variable pointers to the function and record
        // it in the map. Entry points will create the structure, other functions will declare it as
        // a parameter.
        if (func->IsEntryPoint()) {
            function_to_struct_value.Add(func, AddModuleVarsToEntryPoint(func, refs));
        } else {
            function_to_struct_value.Add(func, AddModuleVarsToFunction(func));
        }
    }

    /// Add a module-scope variables structure to an entry point function.
    /// @param func the entry point function to modify
    /// @param referenced_vars the set of variables transitively referenced by the entry point
    /// @returns the structure that holds the module-scope variables
    core::ir::Value* AddModuleVarsToEntryPoint(
        core::ir::Function* func,
        const core::ir::ReferencedModuleVars<core::ir::Module>::VarSet& referenced_vars) {
        core::ir::Value* module_var_struct = nullptr;
        core::ir::FunctionParam* workgroup_allocation_param = nullptr;
        Vector<core::type::Manager::StructMemberDesc, 4> workgroup_struct_members;

        // Add parameters and insert instruction at the top of the entry point to set up the
        // module-scope variables structure.
        b.InsertBefore(func->Block()->Front(), [&] {  //
            Vector<core::ir::Value*, 8> construct_args;
            for (auto var : module_vars) {
                if (!referenced_vars.Contains(var)) {
                    // The variable isn't used by this entry point, so set the member to unused.
                    construct_args.Push(b.Unused());
                    continue;
                }

                // Create a new declaration in the entry point to replace the module-scope variable.
                // Use either a parameter or a local variable, depending on the address space.
                core::ir::Value* decl = nullptr;
                auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();
                switch (ptr->AddressSpace()) {
                    case core::AddressSpace::kPrivate: {
                        // Private variables become function-scope variables.
                        auto* local_var = b.Var(ptr);
                        local_var->SetInitializer(var->Initializer());
                        decl = local_var->Result(0);
                        break;
                    }
                    case core::AddressSpace::kStorage:
                    case core::AddressSpace::kUniform: {
                        // Storage and uniform buffers become function parameters.
                        auto* param = b.FunctionParam(ptr);
                        param->SetBindingPoint(var->BindingPoint());
                        func->AppendParam(param);
                        decl = param;
                        break;
                    }
                    case core::AddressSpace::kWorkgroup: {
                        // Workgroup variables are received as a function parameter (to workaround
                        // an MSL compiler bug with threadgroup matrices), and we aggregate all
                        // workgroup variables into a structure to avoid hitting MSL's limit for
                        // threadgroup memory arguments.
                        if (!workgroup_allocation_param) {
                            workgroup_allocation_param = b.FunctionParam(nullptr);
                            func->AppendParam(workgroup_allocation_param);
                        }
                        decl = b.Access(ptr, workgroup_allocation_param,
                                        u32(workgroup_struct_members.Length()))
                                   ->Result(0);
                        workgroup_struct_members.Push(core::type::Manager::StructMemberDesc{
                            ir.symbols.New(),
                            ptr->StoreType(),
                        });
                        break;
                    }
                    case core::AddressSpace::kHandle: {
                        // Handle types become function parameters and drop the pointer.
                        auto* param = b.FunctionParam(ptr->UnwrapPtr());
                        param->SetBindingPoint(var->BindingPoint());
                        func->AppendParam(param);
                        decl = param;
                        break;
                    }
                    default:
                        TINT_UNREACHABLE() << "unhandled address space: " << ptr->AddressSpace();
                }

                // Copy an existing name over to the new declaration if present.
                if (auto name = ir.NameOf(var)) {
                    ir.SetName(decl, name);
                }
                construct_args.Push(decl);
            }

            // Construct the structure value and name it with a `let` instruction.
            // The `let` prevents the printer from inlining the constructor, which aids readability.
            auto* construct = b.Construct(struct_type, std::move(construct_args));
            module_var_struct = b.Let(kModuleVarsName, construct)->Result(0);
        });

        // Create the workgroup variable structure if needed.
        if (!workgroup_struct_members.IsEmpty()) {
            auto* workgroup_struct =
                ty.Struct(ir.symbols.New(), std::move(workgroup_struct_members));
            workgroup_allocation_param->SetType(ty.ptr<workgroup>(workgroup_struct));
        }

        return module_var_struct;
    }

    /// Add a module-scope variables structure to a non-entry-point function.
    /// @param func the function to modify
    /// @returns the parameter that holds the module-scope variables structure
    core::ir::Value* AddModuleVarsToFunction(core::ir::Function* func) {
        // Add a new parameter to receive the module-scope variables structure.
        auto* param = b.FunctionParam(kModuleVarsName, struct_type);
        func->AppendParam(param);

        // Update all callsites to pass the module-scope variables structure as an argument.
        func->ForEachUseUnsorted([&](core::ir::Usage use) {
            if (auto* call = use.instruction->As<core::ir::UserCall>()) {
                call->AppendArg(*function_to_struct_value.Get(ContainingFunction(call)));
            }
        });

        return param;
    }

    /// Get a variable from the module-scope variable replacement structure, inserting new access
    /// instructions before @p inst.
    /// @param var the variable to get the replacement for
    /// @param inst the instruction that uses the variable
    /// @param index the index of the variable in the structure member list
    /// @returns the variable extracted from the structure
    core::ir::Value* GetVariableFromStruct(core::ir::Var* var,
                                           core::ir::Instruction* inst,
                                           uint32_t index) {
        auto* func = ContainingFunction(inst);
        auto* struct_value = function_to_struct_value.GetOr(func, nullptr);
        auto* type = var->Result(0)->Type();

        // Handle types drop the pointer and are passed around by value.
        auto* ptr = type->As<core::type::Pointer>();
        if (ptr->AddressSpace() == core::AddressSpace::kHandle) {
            type = ptr->StoreType();
        }

        auto* access = b.Access(type, struct_value, u32(index));
        access->InsertBefore(inst);
        return access->Result(0);
    }

    /// Get the function that contains an instruction.
    /// @param inst the instruction
    /// @returns the function
    core::ir::Function* ContainingFunction(core::ir::Instruction* inst) {
        return block_to_function.GetOrAdd(inst->Block(), [&] {  //
            return ContainingFunction(inst->Block()->Parent());
        });
    }
};

}  // namespace

Result<SuccessType> ModuleScopeVars(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "msl.ModuleScopeVars");
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::msl::writer::raise
