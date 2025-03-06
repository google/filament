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

#include "src/tint/lang/core/ir/transform/substitute_overrides.h"

#include <functional>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/evaluator.h"
#include "src/tint/lang/core/ir/type/array_count.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/utils/result/result.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {
namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module& ir;

    /// The configuration
    const SubstituteOverridesConfig& cfg;

    /// The IR builder.
    Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Map of override id to value
    Hashmap<OverrideId, Constant*, 8> override_id_to_value_{};

    /// Process the module.
    Result<SuccessType> Process() {
        Vector<Instruction*, 8> to_remove;
        Vector<Constant*, 8> values_to_propagate;
        Vector<core::ir::Var*, 4> vars_with_value_array_count;

        // Note, we don't `Destroy` the overrides when we substitute them. We need them to stay
        // alive because the `workgroup_size` and `array` usages aren't in the `Usages` list so
        // haven't been replaced yet.

        // Find all overrides in the root block and replace them
        for (auto* inst : *ir.root_block) {
            auto* override = inst->As<core::ir::Override>();
            if (!override) {
                if (auto* var = inst->As<core::ir::Var>()) {
                    if (auto* ary = var->Result(0)->Type()->UnwrapPtr()->As<core::type::Array>()) {
                        if (ary->Count()->Is<core::ir::type::ValueArrayCount>()) {
                            vars_with_value_array_count.Push(var);
                        }
                    }
                } else {
                    // Gather all the non-var instructions which we'll remove
                    to_remove.Push(inst);
                }
                continue;
            }

            // Check if the user provided an override for the given ID.
            auto iter = cfg.map.find(override->OverrideId());
            if (iter != cfg.map.end()) {
                auto* replacement = CreateConstant(override->Result(0)->Type(), iter->second);
                ReplaceOverride(override, replacement);
                values_to_propagate.Push(replacement);
                to_remove.Push(override);
                continue;
            }

            if (override->Initializer() == nullptr) {
                diag::Diagnostic error{};
                error.severity = diag::Severity::Error;
                error.source = ir.SourceOf(override);
                error << "Initializer not provided for override, and override not overridden.";

                return Failure(error);
            }

            core::ir::Constant* replacement = override->Initializer()->As<core::ir::Constant>();
            if (replacement) {
                // Remove the initializer such that we don't find the override as a usage when we
                // try to propagate the replacement.
                override->SetInitializer(nullptr);
            } else {
                auto r = eval::Eval(b, override->Initializer());
                if (r != Success) {
                    return r.Failure();
                }
                replacement = r.Get();
            }
            ReplaceOverride(override, replacement);
            values_to_propagate.Push(replacement);
            to_remove.Push(override);
        }

        // Find any workgroup_sizes to replace
        for (auto func : ir.functions) {
            if (!func->IsCompute()) {
                continue;
            }

            auto wgs = func->WorkgroupSize();
            TINT_ASSERT(wgs.has_value());

            std::array<ir::Value*, 3> new_wg{};
            for (size_t i = 0; i < 3; ++i) {
                auto* val = wgs.value()[i];

                if (val->Is<core::ir::Constant>()) {
                    new_wg[i] = val;
                    continue;
                }

                auto new_value = CalculateOverride(val);
                if (!new_value.Get()) {
                    return new_value.Failure();
                }
                new_wg[i] = new_value.Get();
            }
            func->SetWorkgroupSize(new_wg);
        }

        // Replace array types using overrides
        for (auto var : vars_with_value_array_count) {
            auto* old_ptr = var->Result(0)->Type()->As<core::type::Pointer>();
            TINT_ASSERT(old_ptr);

            auto* old_ty = old_ptr->UnwrapPtr()->As<core::type::Array>();
            auto* cnt = old_ty->Count()->As<core::ir::type::ValueArrayCount>();
            TINT_ASSERT(cnt);

            auto new_value = CalculateOverride(cnt->value);
            if (!new_value.Get()) {
                return new_value.Failure();
            }

            uint32_t num_elements = new_value.Get()->Value()->ValueAs<uint32_t>();
            auto* new_cnt = ty.Get<core::type::ConstantArrayCount>(num_elements);
            auto* new_ty = ty.Get<core::type::Array>(old_ty->ElemType(), new_cnt, old_ty->Align(),
                                                     num_elements * old_ty->Stride(),
                                                     old_ty->Stride(), old_ty->ImplicitStride());

            auto* new_ptr = ty.ptr(old_ptr->AddressSpace(), new_ty, old_ptr->Access());
            var->Result(0)->SetType(new_ptr);

            // The `Var` type needs to propagate to certain usages.
            Vector<core::ir::Instruction*, 2> to_replace;
            to_replace.Push(var);

            while (!to_replace.IsEmpty()) {
                auto* inst = to_replace.Pop();

                for (auto usage : inst->Result(0)->UsagesUnsorted()) {
                    if (!usage->instruction->Is<core::ir::Let>()) {
                        continue;
                    }

                    usage->instruction->Result(0)->SetType(new_ptr);
                    to_replace.Push(usage->instruction);
                }
            }
        }

        // Remove any non-var instruction in the root block
        for (auto* inst : to_remove) {
            inst->Destroy();
        }

        {
            // Propagate any replaced override instructions up their instruction chains
            auto res = Propagate(values_to_propagate);
            if (res != Success) {
                return res;
            }
        }

        return Success;
    }

    Result<core::ir::Constant*> CalculateOverride(core::ir::Value* val) {
        auto* count_value = val->As<core::ir::InstructionResult>();
        TINT_ASSERT(count_value);

        if (auto* override = count_value->Instruction()->As<core::ir::Override>()) {
            auto replacement = override_id_to_value_.Get(override->OverrideId());
            TINT_ASSERT(replacement);
            return *replacement;
        }
        auto r = eval::Eval(b, count_value);
        if (r != Success) {
            return r.Failure();
        }
        // Must be able to evaluate the constant.
        TINT_ASSERT(r.Get());

        return r;
    }

    void ReplaceOverride(core::ir::Override* override, core::ir::Constant* replacement) {
        override_id_to_value_.Add(override->OverrideId(), replacement);
        override->Result(0)->ReplaceAllUsesWith(replacement);
    }

    Result<SuccessType> Propagate(Vector<core::ir::Constant*, 8>& values_to_propagate) {
        while (!values_to_propagate.IsEmpty()) {
            auto* value = values_to_propagate.Pop();
            for (auto usage : value->UsagesSorted()) {
                // If the instruction has no results, then it was destroyed already and we can just
                // skip it.
                if (!usage.instruction->Result(0)) {
                    continue;
                }

                if (!NeedsEval(usage.instruction)) {
                    continue;
                }

                auto r = eval::Eval(b, usage.instruction);
                if (r != Success) {
                    return r.Failure();
                }

                // The replacement can be a `nullptr` if we try to evaluate something like a `dpdx`
                // builtin which doesn't have a `@const` annotation.
                auto* replacement = r.Get();
                if (!replacement) {
                    continue;
                }

                usage.instruction->Result(0)->ReplaceAllUsesWith(replacement);
                values_to_propagate.Push(replacement);
                usage.instruction->Destroy();
            }
        }

        return Success;
    }

    bool NeedsEval(core::ir::Instruction* inst) {
        return tint::Switch(                                   //
            inst,                                              //
            [&](core::ir::Bitcast*) { return true; },          //
            [&](core::ir::Access*) { return true; },           //
            [&](core::ir::Construct*) { return true; },        //
            [&](core::ir::Convert*) { return true; },          //
            [&](core::ir::CoreBinary*) { return true; },       //
            [&](core::ir::CoreBuiltinCall*) { return true; },  //
            [&](core::ir::CoreUnary*) { return true; },        //
            [&](core::ir::Swizzle*) { return true; },          //
            [&](Default) { return false; });
    }

    Constant* CreateConstant(const core::type::Type* type, double val) {
        return tint::Switch(
            type,
            [&](const core::type::Bool*) { return b.Constant(!std::equal_to<double>()(val, 0.0)); },
            [&](const core::type::I32*) { return b.Constant(i32(val)); },
            [&](const core::type::U32*) { return b.Constant(u32(val)); },
            [&](const core::type::F32*) { return b.Constant(f32(val)); },
            [&](const core::type::F16*) { return b.Constant(f16(val)); },  //
            TINT_ICE_ON_NO_MATCH);
    }
};

}  // namespace

SubstituteOverridesConfig::SubstituteOverridesConfig() = default;

Result<SuccessType> SubstituteOverrides(Module& ir, const SubstituteOverridesConfig& cfg) {
    auto result =
        ValidateAndDumpIfNeeded(ir, "core.SubstituteOverrides", kSubstituteOverridesCapabilities);
    if (result != Success) {
        return result;
    }
    return State{ir, cfg}.Process();
}

}  // namespace tint::core::ir::transform
