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

#include "src/tint/lang/msl/writer/raise/argument_buffers.h"

#include <string>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/msl/builtin_fn.h"
#include "src/tint/lang/msl/ir/builtin_call.h"

namespace tint::msl::writer::raise {
namespace {

using namespace tint::core::fluent_types;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The argument buffer configuration.
    const ArgumentBuffersConfig& config;

    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The type of the structures that will contain the argument buffers. One entry per binding
    /// group.
    Hashmap<uint32_t, const core::type::Pointer*, 4> arg_buffers{};

    /// The list of module-scope variables.
    Vector<core::ir::Var*, 8> module_vars{};

    /// Maps a binding argument buffer index to the function param
    std::unordered_map<uint32_t, core::ir::Value*> id_to_arg_buffer{};

    /// Maps from variable to the argument buffer index
    Hashmap<core::ir::Var*, uint32_t, 4> var_to_struct_idx{};

    /// Maps a global `var` to an entry point parameter argument buffer
    Hashmap<core::ir::Var*, Hashmap<core::ir::Function*, core::ir::FunctionParam*, 4>, 4>
        var_to_function_param{};

    /// A map from block to its containing function.
    Hashmap<core::ir::Block*, core::ir::Function*, 64> block_to_function{};

    /// Maps from a group number to the dynamic offset buffer
    Hashmap<uint32_t, core::ir::Value*, 8> group_to_dynamic_offset_buffer{};

    // The name of the argument buffer structures.
    static constexpr const char* kArgBufferName = "tint_arg_buffer_struct";
    static constexpr const char* kArgBufferParamName = "tint_arg_buffer";
    static constexpr const char* kDynamicOffsetParamName = "tint_dynamic_offset_buffer";

    /// Process the module.
    void Process() {
        // Seed the block-to-function map with the function entry blocks.
        // This is used to determine the owning function for any given instruction.
        for (auto& func : ir.functions) {
            block_to_function.Add(func->Block(), func);
        }

        CreateArgumentBuffers();

        for (auto func : ir.functions) {
            if (!func->IsEntryPoint()) {
                continue;
            }
            AddArgumentBuffersToEntryPoint(func);
        }

        // Replace uses of each module-scope variable.
        for (auto& var : module_vars) {
            if (!var->BindingPoint().has_value()) {
                continue;
            }

            // If we aren't replacing the group, there is nothing to do.
            auto iter = config.group_to_argument_buffer_info.find(var->BindingPoint()->group);
            if (iter == config.group_to_argument_buffer_info.end()) {
                continue;
            }

            Vector<core::ir::Instruction*, 16> to_destroy;
            auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
            var->Result()->ForEachUseUnsorted([&](core::ir::Usage use) {  //
                auto* extracted_variable = GetVariableFromStruct(var, use.instruction);

                // Everything but handles are just replaced with values from the structure.
                if (ptr->AddressSpace() != core::AddressSpace::kHandle) {
                    use.instruction->SetOperand(use.operand_index, extracted_variable);
                    return;
                }

                Switch(
                    use.instruction,
                    // Loads are replaced with a direct access to the variable.
                    [&](core::ir::Load* load) {
                        load->Result()->ReplaceAllUsesWith(extracted_variable);
                        to_destroy.Push(load);
                    },
                    // Accesses are replaced with accesses of the extracted variable.
                    [&](core::ir::Access* access) {
                        auto* ba = ptr->StoreType()->As<core::type::BindingArray>();
                        TINT_ASSERT(ba != nullptr);
                        auto* elem_type = ba->ElemType();

                        access->SetOperand(core::ir::Access::kObjectOperandOffset,
                                           extracted_variable);
                        access->Result()->SetType(elem_type);

                        // Accesses of the previously ptr<binding_array<T, N>> would return a ptr<T>
                        // but the new access returns a T. We need to modify all the previous load
                        // through the ptr<T> to direct accesses.
                        access->Result()->ForEachUseUnsorted([&](core::ir::Usage access_use) {
                            TINT_ASSERT(access_use.instruction->Is<core::ir::Load>());
                            access_use.instruction->Result()->ReplaceAllUsesWith(access->Result());
                            to_destroy.Push(access_use.instruction);
                        });
                    },
                    TINT_ICE_ON_NO_MATCH);
            });
            var->Destroy();

            // Clean up instructions that need to be removed.
            for (auto* inst : to_destroy) {
                inst->Destroy();
            }
        }
    }

    /// Create the argument buffers. Each bind group will have a separate structure.
    void CreateArgumentBuffers() {
        Vector<core::ir::Var*, 4> vars;
        for (auto* global : *ir.root_block) {
            auto* var = global->As<core::ir::Var>();
            if (!var) {
                continue;
            }

            // Only deal with vars which have binding points.
            auto bp = var->BindingPoint();
            if (!bp.has_value()) {
                continue;
            }

            if (config.skip_bindings.contains(*bp)) {
                continue;
            }

            vars.Push(var);
        }

        // Metal requires the argument buffer `id` entries to be in increasing order. Sort the Vars
        // such that when we create the struct we will create it in ascending order.
        vars.Sort([&](const auto* va, const auto* vb) {
            return va->BindingPoint() < vb->BindingPoint();
        });

        // Collect a list of struct members for the variable declarations.
        Hashmap<uint32_t, Vector<core::type::Manager::StructMemberDesc, 8>, 4> group_to_members;
        for (auto& var : vars) {
            auto bp = var->BindingPoint();
            auto* type = var->Result()->Type();

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

            auto& struct_members = group_to_members.GetOrAddZero(bp->group);
            var_to_struct_idx.Add(var, static_cast<uint32_t>(struct_members.Length()));

            struct_members.Push(core::type::Manager::StructMemberDesc{
                name, type,
                core::IOAttributes{
                    .binding_point = BindingPoint{bp->group, bp->binding},
                }});
        }

        // Sort the keys for deterministic struct generation
        auto keys = group_to_members.Keys().Sort();

        for (auto& k : keys) {
            // Create the structure.
            auto name = ir.symbols.New(std::string(kArgBufferName) + "_" + std::to_string(k));
            auto members = group_to_members.Get(k);

            auto* strct = ty.Struct(name, std::move(*members));
            strct->SetStructFlag(core::type::kExplicitLayout);

            auto* type = ty.ptr(uniform, strct, read);
            arg_buffers.Add(k, type);
        }
    }

    /// Add an argument buffer structure to an entry point function.
    /// @param func the entry point function to modify
    void AddArgumentBuffersToEntryPoint(core::ir::Function* func) {
        auto keys = arg_buffers.Keys().Sort();

        for (auto& buffer_id : keys) {
            auto iter = config.group_to_argument_buffer_info.find(buffer_id);
            if (iter == config.group_to_argument_buffer_info.end()) {
                continue;
            }

            auto name = std::string(kArgBufferParamName) + "_" + std::to_string(buffer_id);
            auto* param = b.FunctionParam(name, *arg_buffers.Get(buffer_id));

            param->SetBindingPoint(BindingPoint{0, iter->second.id});
            func->AppendParam(param);

            auto* ld = b.Load(param);
            func->Block()->Prepend(ld);

            id_to_arg_buffer.insert({buffer_id, ld->Result()});

            // If this buffer requires a dynamic offset buffer attached, create the function
            // parameter
            if (iter->second.dynamic_buffer_id.has_value()) {
                auto dynamic_buffer_name =
                    std::string(kDynamicOffsetParamName) + "_" + std::to_string(buffer_id);
                auto* dynamic_buffer_param =
                    b.FunctionParam(dynamic_buffer_name, ty.ptr(storage, ty.array<u32>(), read));

                dynamic_buffer_param->SetBindingPoint(
                    BindingPoint{0, iter->second.dynamic_buffer_id.value()});
                func->AppendParam(dynamic_buffer_param);

                group_to_dynamic_offset_buffer.Add(buffer_id, dynamic_buffer_param);
            }
        }
    }

    /// Add an entry to each function which uses a module-scoped variable to
    /// receive the variable as a parameter.
    /// @param func the function to modify
    /// @returns the function param
    core::ir::FunctionParam* AddModuleVarsToFunction(core::ir::Function* func, core::ir::Var* var) {
        auto& v = var_to_function_param.GetOrAddZero(var);
        return v.GetOrAdd(func, [&] {
            auto* type = var->Result()->Type();

            // Handle types drop the pointer and are passed around by value.
            auto* ptr = type->As<core::type::Pointer>();
            if (ptr->AddressSpace() == core::AddressSpace::kHandle) {
                type = ptr->StoreType();
            }

            // Add a new parameter to receive the variable parameter.
            core::ir::FunctionParam* param = nullptr;

            auto name = ir.NameOf(var).Name();
            if (name.empty()) {
                param = b.FunctionParam(type);
            } else {
                param = b.FunctionParam(ir.NameOf(var).Name(), type);
            }
            func->AppendParam(param);

            func->ForEachUseUnsorted([&](core::ir::Usage use) {
                if (auto* call = use.instruction->As<core::ir::UserCall>()) {
                    call->AppendArg(GetVariableFromStruct(var, call));
                }
            });

            return param;
        });
    }

    /// Get a variable from the argument buffer, inserting new access
    /// instructions before @p inst.
    /// @param var the variable to get the replacement for
    /// @param inst the instruction that uses the variable
    /// @returns the variable extracted from the structure
    core::ir::Value* GetVariableFromStruct(core::ir::Var* var, core::ir::Instruction* inst) {
        auto* func = ContainingFunction(inst);

        auto* type = var->Result()->Type();

        // Handle types drop the pointer and are passed around by value.
        auto* ptr = type->As<core::type::Pointer>();
        if (ptr->AddressSpace() == core::AddressSpace::kHandle) {
            type = ptr->StoreType();
        }

        if (func->IsEntryPoint()) {
            auto group = var->BindingPoint()->group;
            auto binding = var->BindingPoint()->binding;

            auto arg_buffer = id_to_arg_buffer.find(group);
            TINT_ASSERT(arg_buffer != id_to_arg_buffer.end());

            auto idx = *var_to_struct_idx.Get(var);

            auto* access = b.Access(type, arg_buffer->second, u32(idx));
            access->InsertBefore(inst);

            auto grp_iter = config.group_to_argument_buffer_info.find(group);
            if (grp_iter == config.group_to_argument_buffer_info.end()) {
                return access->Result();
            }

            auto binding_iter = grp_iter->second.binding_info_to_offset_index.find(binding);
            if (binding_iter == grp_iter->second.binding_info_to_offset_index.end()) {
                return access->Result();
            }

            TINT_ASSERT(group_to_dynamic_offset_buffer.Contains(group));

            core::ir::Value* offset_buffer = group_to_dynamic_offset_buffer.GetOr(group, nullptr);
            TINT_ASSERT(offset_buffer);

            core::ir::Value* result = nullptr;
            b.InsertAfter(access, [&] {
                auto* offset = b.Access(ty.ptr<storage, u32, read>(), offset_buffer,
                                        b.Constant(u32(binding_iter->second)));

                result = b.Call<msl::ir::BuiltinCall>(type, msl::BuiltinFn::kPointerOffset, access,
                                                      b.Load(offset))
                             ->Result();
            });

            return result;
        }

        return AddModuleVarsToFunction(func, var);
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

Result<SuccessType> ArgumentBuffers(core::ir::Module& ir, const ArgumentBuffersConfig& config) {
    auto result = ValidateAndDumpIfNeeded(
        ir, "msl.ArgumentBuffers",
        tint::core::ir::Capabilities{tint::core::ir::Capability::kAllowDuplicateBindings});
    if (result != Success) {
        return result.Failure();
    }

    State{config, ir}.Process();

    return Success;
}

}  // namespace tint::msl::writer::raise
