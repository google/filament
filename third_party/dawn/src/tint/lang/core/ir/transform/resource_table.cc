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

#include "src/tint/lang/core/ir/transform/resource_table.h"

#include <unordered_map>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/resource_type.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The configuration.
    const std::optional<ResourceTableConfig>& config;

    /// The IR module.
    core::ir::Module& ir;

    /// The helper
    ResourceTableHelper* helper;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    Result<SuccessType> Process() {
        // Find calls to hasResource() and getResource().
        Vector<core::ir::BuiltinCall*, 8> has_resource_calls;
        Vector<core::ir::BuiltinCall*, 8> get_resource_calls;
        for (auto* inst : ir.Instructions()) {
            auto* call = inst->As<core::ir::CoreBuiltinCall>();
            if (call == nullptr) {
                continue;
            }
            if (call->Func() == core::BuiltinFn::kHasResource) {
                has_resource_calls.Push(call);
            }
            if (call->Func() == core::BuiltinFn::kGetResource) {
                get_resource_calls.Push(call);
            }
        }
        if (has_resource_calls.IsEmpty() && get_resource_calls.IsEmpty()) {
            return Success;
        }
        if (!config.has_value()) {
            return Failure{"hasResource and getResource require a resource table"};
        }

        Hashmap<const core::type::Type*, core::ir::Var*, 4> var_for_type;
        core::ir::Var* sb = nullptr;
        b.Append(ir.root_block, [&] {
            var_for_type = helper->GenerateVars(b, config->resource_table_binding,
                                                config->default_binding_type_order);
            sb = InjectStorageBuffer(config->storage_buffer_binding);
        });

        std::unordered_map<ResourceType, uint32_t> resource_type_to_idx;
        size_t def_size = config->default_binding_type_order.size();
        for (size_t i = 0; i < def_size; ++i) {
            auto res_ty = static_cast<ResourceType>(config->default_binding_type_order[i]);
            resource_type_to_idx.insert({res_ty, static_cast<uint32_t>(i)});
        }

        // Replace the calls to hasResource() and getResource() that we found earlier.
        for (auto* call : has_resource_calls) {
            b.InsertBefore(call, [&] {
                auto* binding_ty = call->ExplicitTemplateParams()[0];
                auto* idx = b.InsertConvertIfNeeded(ty.u32(), call->Args()[0]);
                GenHasResource(call->DetachResult(), binding_ty, idx, sb);
            });
            call->Destroy();
        }
        for (auto* call : get_resource_calls) {
            b.InsertBefore(call, [&] {
                auto* binding_ty = call->ExplicitTemplateParams()[0];
                auto* idx = b.InsertConvertIfNeeded(ty.u32(), call->Args()[0]);
                GenGetResource(call->DetachResult(), binding_ty, idx, sb, var_for_type,
                               resource_type_to_idx);
            });
            call->Destroy();
        }

        return Success;
    }

    // Note, assumes it's called inside a builder append block.
    void GenHasResource(core::ir::InstructionResult* result,
                        const core::type::Type* type,
                        core::ir::Value* idx,
                        core::ir::Var* storage_buffer) {
        auto* length = b.Access(ty.ptr<storage, u32, read>(), storage_buffer, 0_u);
        auto* len_check = b.LessThan(idx, b.Load(length));

        auto* has_check = b.If(len_check);
        has_check->SetResult(result);

        b.Append(has_check->True(), [&] {
            auto* type_val = b.Access(ty.ptr<storage, u32, read>(), storage_buffer, 1_u, idx);
            auto* v = b.Load(type_val);

            ResourceType resource_ty = core::type::TypeToResourceType(type);

            core::ir::Value* eq = nullptr;
            std::vector<ResourceType> conv = core::type::ConvertsFrom(type);
            if (!conv.empty()) {
                auto* conv_ty = ty.vec(ty.u32(), static_cast<uint32_t>(conv.size()) + 1);
                auto* lhs = b.Construct(conv_ty, v);

                Vector<Value*, 4> vals;
                vals.Push(b.Value(u32(resource_ty)));
                for (auto& r : conv) {
                    vals.Push(b.Value(u32(r)));
                }

                auto* rhs = b.Construct(conv_ty, vals);
                auto* cmp = b.Equal(lhs, rhs);
                eq = b.Call(ty.bool_(), core::BuiltinFn::kAny, cmp)->Result();
            } else {
                eq = b.Equal(v, u32(resource_ty))->Result();
            }
            b.ExitIf(has_check, eq);
        });

        b.Append(has_check->False(), [&] { b.ExitIf(has_check, b.Constant(false)); });
    }

    // Note, assumes it's called inside a builder append block.
    void GenGetResource(core::ir::InstructionResult* result,
                        const core::type::Type* type,
                        core::ir::Value* idx,
                        core::ir::Var* storage_buffer,
                        const Hashmap<const core::type::Type*, core::ir::Var*, 4>& alias_for_type,
                        const std::unordered_map<ResourceType, uint32_t>& resource_type_to_idx) {
        auto* has_result = b.InstructionResult(ty.bool_());
        GenHasResource(has_result, type, idx, storage_buffer);

        auto* get_check = b.If(has_result);
        auto* res = b.InstructionResult(ty.u32());
        get_check->SetResult(res);

        auto alias = alias_for_type.Get(type);
        TINT_IR_ASSERT(ir, alias);

        b.Append(get_check->True(), [&] { b.ExitIf(get_check, idx); });

        b.Append(get_check->False(), [&] {
            auto res_type = core::type::TypeToResourceType(type);
            auto idx_iter = resource_type_to_idx.find(res_type);
            TINT_IR_ASSERT(ir, idx_iter != resource_type_to_idx.end());

            auto* len_access = b.Access(ty.ptr<storage, u32, read>(), storage_buffer, 0_u);
            auto* num_elements = b.Load(len_access);

            auto* r = b.Add(u32(idx_iter->second), num_elements);

            b.ExitIf(get_check, r);
        });

        // TODO(439627523): Fix pointer access
        auto* ptr_ty = ty.ptr(handle, type, read);
        auto* access = b.Access(ptr_ty, (*alias)->Result(), res);
        b.LoadWithResult(result, access);
    }

    core::ir::Var* InjectStorageBuffer(const BindingPoint& bp) {
        auto* str = ty.Struct(ir.symbols.New("tint_resource_table_metadata_struct"),
                              Vector<core::type::Manager::StructMemberDesc, 2>{
                                  {ir.symbols.New("array_length"), ty.u32()},
                                  {ir.symbols.New("bindings"), ty.array<u32>()},
                              });

        auto* sb_ty = ty.ptr(storage, str, read);
        auto* sb = b.Var("tint_resource_table_metadata", sb_ty);
        sb->SetBindingPoint(bp.group, bp.binding);

        return sb;
    }
};

}  // namespace

ResourceTableHelper::~ResourceTableHelper() = default;

Result<SuccessType> ResourceTable(core::ir::Module& ir,
                                  const std::optional<ResourceTableConfig>& config,
                                  ResourceTableHelper* helper) {
    AssertValid(ir,
                core::ir::Capabilities{
                    core::ir::Capability::kAllowDuplicateBindings,
                    core::ir::Capability::kAllow8BitIntegers,
                    core::ir::Capability::kAllow16BitIntegers,
                },
                "before core.ResourceTable");

    return State{config, ir, helper}.Process();
}

}  // namespace tint::core::ir::transform
