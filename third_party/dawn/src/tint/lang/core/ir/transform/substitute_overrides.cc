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

#include <cstdint>
#include <functional>
#include <utility>

#include "src/tint/lang/core/binary_op.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/const_param_validator.h"
#include "src/tint/lang/core/ir/constexpr_if.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/evaluator.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/instruction_result.h"
#include "src/tint/lang/core/ir/override.h"
#include "src/tint/lang/core/ir/terminator.h"
#include "src/tint/lang/core/ir/traverse.h"
#include "src/tint/lang/core/ir/type/array_count.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/utils/numeric.h"

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

    /// Process the module.
    diag::Result<SuccessType> Process() {
        Vector<Instruction*, 8> to_remove;
        Vector<Constant*, 8> values_to_propagate;
        Vector<core::ir::Var*, 4> vars_with_value_array_count;
        Vector<core::ir::Override*, 16> override_complex_init;

        // Note, we don't `Destroy` the overrides when we substitute them. We need them to stay
        // alive because the `workgroup_size` and `array` usages aren't in the `Usages` list so
        // haven't been replaced yet.
        for (auto* inst : *ir.root_block) {
            if (auto* var = inst->As<core::ir::Var>()) {
                if (auto* ary = var->Result()->Type()->UnwrapPtr()->As<core::type::Array>()) {
                    if (ary->Count()->Is<core::ir::type::ValueArrayCount>()) {
                        vars_with_value_array_count.Push(var);
                    }
                }
            } else {
                // Gather all the non-var instructions which we'll remove
                to_remove.Push(inst);
            }

            auto* override = inst->As<core::ir::Override>();
            if (!override) {
                continue;
            }

            // Check if the user provided an override for the given ID. In the case of Dawn, all
            // overrides end up having an ID, so they will all be able to be queried here. If the
            // code came through the SPIR-V reader, and overrides are being applied on the top of
            // that IR tree, an OverrideId may not be set, but that also means in SPIR-V the
            // override could not be set anyway, so it can't have an override value applied.
            if (override->OverrideId().has_value()) {
                auto iter = cfg.map.find(override->OverrideId().value());
                if (iter != cfg.map.end()) {
                    bool substitution_representation_valid = tint::Switch(
                        override->Result()->Type(),  //
                        [&](const core::type::Bool*) { return true; },
                        [&](const core::type::I32*) {
                            return dawn::IsDoubleValueRepresentable<int32_t>(iter->second);
                        },
                        [&](const core::type::U32*) {
                            return dawn::IsDoubleValueRepresentable<uint32_t>(iter->second);
                        },
                        [&](const core::type::F32*) {
                            return dawn::IsDoubleValueRepresentable<float>(iter->second);
                        },
                        [&](const core::type::F16*) {
                            return dawn::IsDoubleValueRepresentableAsF16(iter->second);
                        },
                        TINT_ICE_ON_NO_MATCH);

                    if (!substitution_representation_valid) {
                        diag::Diagnostic error{};
                        error.severity = diag::Severity::Error;
                        error.source = ir.SourceOf(override);
                        error << "Pipeline overridable constant " << iter->first.value
                              << " with value (" << iter->second
                              << ")  is not representable in type ("
                              << override->Result()->Type()->FriendlyName() << ")";
                        return diag::Failure(error);
                    }

                    auto* replacement = CreateConstant(override->Result()->Type(), iter->second);
                    override->SetInitializer(replacement);
                }
            }

            if (override->Initializer() == nullptr) {
                diag::Diagnostic error{};
                error.severity = diag::Severity::Error;
                error.source = ir.SourceOf(override);
                error << "Initializer not provided for override, and override not overridden.";
                return diag::Failure(error);
            }

            if (auto* replacement = override->Initializer()->As<core::ir::Constant>()) {
                override->Result()->ReplaceAllUsesWith(replacement);
                values_to_propagate.Push(replacement);
            } else {
                // This override might depend on ConstExperIf block compile time evaluation.
                override_complex_init.Push(override);
            }
        }

        // When `overrides` are evaluated, only the `override` is checked, and any instructions back
        // up the block. This means, if we have a `constexpr-if` we may hit an override in the part
        // of the `constexpr-if` which should be ignored (because we had a `false && a_override / 0`
        // or something similar). If we evaluate `a_override` before we evaluate the `constexpr-if`
        // that represents the `&&` then we'll produce an incorrect compile error. Instead evaluate
        // the `constexpr-if` constructs early to remove them all and remove any blocks which should
        // not be evaluated.
        auto res = EvalConstExprIf();
        if (res != Success) {
            return res;
        }

        // Workgroup size MUST be evaluated prior to 'propagate' because workgroup size parameters
        // are not proper usages.
        for (auto func : ir.functions) {
            if (!func->IsCompute()) {
                continue;
            }

            auto wgs = func->WorkgroupSize();
            TINT_ASSERT(wgs.has_value());

            std::array<ir::Value*, 3> new_wg{};
            for (size_t i = 0; i < 3; ++i) {
                auto new_value = CalculateOverride(wgs.value()[i]);
                if (new_value != Success) {
                    return new_value.Failure();
                }
                new_wg[i] = new_value.Get();
            }
            func->SetWorkgroupSize(new_wg);
        }

        // Replace array types MUST be evaluate prior to 'propagate' because array count values are
        // not proper usages.
        for (auto var : vars_with_value_array_count) {
            auto* old_ptr = var->Result()->Type()->As<core::type::Pointer>();
            TINT_ASSERT(old_ptr);

            auto* old_ty = old_ptr->UnwrapPtr()->As<core::type::Array>();
            auto* cnt = old_ty->Count()->As<core::ir::type::ValueArrayCount>();
            TINT_ASSERT(cnt);

            auto new_value = CalculateOverride(cnt->value);
            if (new_value != Success) {
                return new_value.Failure();
            }

            // Pipeline creation error for zero or negative sized array. This is important as we do
            // not check constant evaluation access against zero size.
            int64_t cnt_size_check = new_value.Get()->Value()->ValueAs<AInt>();
            if (cnt_size_check < 1) {
                diag::Diagnostic error{};
                error.severity = diag::Severity::Error;
                error.source = ir.SourceOf(cnt->value);
                error << "array count (" << cnt_size_check << ") must be greater than 0";
                return diag::Failure(error);
            }

            uint32_t num_elements = new_value.Get()->Value()->ValueAs<uint32_t>();
            auto* new_cnt = ty.Get<core::type::ConstantArrayCount>(num_elements);
            auto* new_ty = ty.Get<core::type::Array>(old_ty->ElemType(), new_cnt, old_ty->Align(),
                                                     num_elements * old_ty->Stride(),
                                                     old_ty->Stride(), old_ty->ImplicitStride());

            auto* new_ptr = ty.ptr(old_ptr->AddressSpace(), new_ty, old_ptr->Access());
            var->Result()->SetType(new_ptr);

            // The `Var` type needs to propagate to certain usages.
            Vector<core::ir::Instruction*, 2> to_replace;
            to_replace.Push(var);

            while (!to_replace.IsEmpty()) {
                auto* inst = to_replace.Pop();
                for (auto usage : inst->Result()->UsagesUnsorted()) {
                    // This is an edge case where we have to specifically verify bounds access for
                    // these new arrays for all usages.
                    if (NeedsEval(usage->instruction)) {
                        auto r = eval::Eval(b, usage->instruction);
                        if (r != Success) {
                            return r.Failure();
                        }
                    }
                    if (!usage->instruction->Is<core::ir::Let>()) {
                        continue;
                    }

                    usage->instruction->Result()->SetType(new_ptr);
                    to_replace.Push(usage->instruction);
                }
            }
        }

        for (auto* override : override_complex_init) {
            auto res_const = CalculateOverride(override->Result());
            if (res_const != Success) {
                return res_const.Failure();
            }
            override->Result()->ReplaceAllUsesWith(res_const.Get());
            values_to_propagate.Push(res_const.Get());
        }

        // Propagate any replaced override instructions up their instruction chains
        res = Propagate(values_to_propagate);
        if (res != Success) {
            return res;
        }

        // Remove any non-var instruction in the root block
        for (auto* inst : to_remove) {
            // Some instructions can be destroyed by 'Propagate' or 'EvalConstExprIf'. This is
            // normal.
            if (inst->Alive()) {
                inst->Destroy();
            }
        }

        return Success;
    }

    diag::Result<SuccessType> EvalConstExprIf() {
        Vector<core::ir::ConstExprIf*, 32> ordered_constexpr_if;
        core::ir::Traverse(ir.root_block, [&ordered_constexpr_if](ConstExprIf* inst) {
            ordered_constexpr_if.Push(inst);
        });

        for (auto func : ir.functions) {
            core::ir::Traverse(func->Block(), [&ordered_constexpr_if](ConstExprIf* inst) {
                ordered_constexpr_if.Push(inst);
            });
        }

        for (auto* constexpr_if : ordered_constexpr_if) {
            // This very code can end up destroying other ConstExprIf instructions.
            if (!constexpr_if->Alive()) {
                continue;
            }

            auto res = eval::Eval(b, constexpr_if->Condition());
            if (res != Success) {
                return res.Failure();
            }

            TINT_ASSERT(res.Get());
            auto* inline_block =
                res.Get()->Value()->ValueAs<bool>() ? constexpr_if->True() : constexpr_if->False();
            TINT_ASSERT(inline_block->Terminator());
            for (;;) {
                auto block_inst = *inline_block->begin();
                if (block_inst->Is<core::ir::Terminator>()) {
                    break;
                }
                block_inst->Remove();
                block_inst->InsertBefore(constexpr_if);
            }
            // There will only be one arg since the return (of ConstExprIf) is a single
            // boolean.
            constexpr_if->Result()->ReplaceAllUsesWith(inline_block->Terminator()->Args()[0]);
            constexpr_if->Destroy();
        }

        return Success;
    }

    diag::Result<core::ir::Constant*> CalculateOverride(core::ir::Value* val) {
        auto r = eval::Eval(b, val);
        if (r != Success) {
            return r.Failure();
        }
        // Must be able to evaluate the constant.
        TINT_ASSERT(r.Get());

        return r;
    }

    diag::Result<SuccessType> Propagate(Vector<core::ir::Constant*, 8>& values_to_propagate) {
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

                usage.instruction->Result()->ReplaceAllUsesWith(replacement);
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
            [&](core::ir::Override*) { return true; },         //
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
    {
        auto result = ValidateAndDumpIfNeeded(ir, "core.SubstituteOverrides",
                                              kSubstituteOverridesCapabilities);
        if (result != Success) {
            return result.Failure();
        }
    }
    {
        auto result = State{ir, cfg}.Process();
        if (result != Success) {
            return Failure{result.Failure().reason.Str()};
        }
    }

    // TODO(crbug.com/382300469): This function should take in a constant module but it does not due
    // to missing constant functions.
    return tint::core::ir::ValidateConstParam(ir);
}

}  // namespace tint::core::ir::transform
