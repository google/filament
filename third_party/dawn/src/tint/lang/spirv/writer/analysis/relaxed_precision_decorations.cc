// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/analysis/relaxed_precision_decorations.h"

#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "src/tint/lang/spirv/type/image.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::spirv::writer::analysis {

namespace {

/// PIMPL state for the analysis.
struct State {
    /// The IR module.
    const core::ir::Module& ir;

    /// The set of values that can be decorated with RelaxedPrecision.
    RelaxedPrecisionDecorations& decorations;

    /// Process the module.
    void Process() {
        // Look for function parameters that might not need full f32 precision.
        // Do this in reverse-dependency order since the precision requirements may be derived from
        // calls to other user-defined functions.
        for (auto* func : ir.DependencyOrderedFunctions()) {
            for (auto* param : func->Params()) {
                ProcessRootValue(param);
            }
        }

        // Look for module-scope variables that might not need full f32 precision.
        for (auto* inst : *ir.root_block) {
            if (auto* var = inst->As<core::ir::Var>()) {
                ProcessRootValue(var->Result());
            }
        }
    }

    /// Process a value to see if it, and/or its usages, can be decorated with RelaxedPrecision.
    void ProcessRootValue(const core::ir::Value* value) {
        auto* image = value->Type()->UnwrapPtr()->As<spirv::type::Image>();
        if (!image) {
            // Only consider image types for now.
            return;
        }

        // Apply decorations to the operands and results of image builtins and record whether or not
        // the variable itself can be decorated based on its usages.
        const bool all_usages_support_relaxed_precision = WalkImageUsages(value);

        // Check the texel format to see if the image itself can be RelaxedPrecision.
        // For some formats this may depend on the usages.
        switch (image->GetTexelFormat()) {
            // 16-bit float formats and normalized formats can always use relaxed precision.
            case core::TexelFormat::kBgra8Unorm:
            case core::TexelFormat::kR16Float:
            case core::TexelFormat::kR16Snorm:
            case core::TexelFormat::kR16Unorm:
            case core::TexelFormat::kR8Snorm:
            case core::TexelFormat::kR8Unorm:
            case core::TexelFormat::kRg16Float:
            case core::TexelFormat::kRg16Snorm:
            case core::TexelFormat::kRg16Unorm:
            case core::TexelFormat::kRg8Snorm:
            case core::TexelFormat::kRg8Unorm:
            case core::TexelFormat::kRgb10A2Unorm:
            case core::TexelFormat::kRgba16Float:
            case core::TexelFormat::kRgba16Snorm:
            case core::TexelFormat::kRgba16Unorm:
            case core::TexelFormat::kRgba8Snorm:
            case core::TexelFormat::kRgba8Unorm:
                decorations.Add(value);
                break;

            // Integer formats never use relaxed precision.
            case core::TexelFormat::kR16Sint:
            case core::TexelFormat::kR16Uint:
            case core::TexelFormat::kR32Sint:
            case core::TexelFormat::kR32Uint:
            case core::TexelFormat::kR8Sint:
            case core::TexelFormat::kR8Uint:
            case core::TexelFormat::kRg16Sint:
            case core::TexelFormat::kRg16Uint:
            case core::TexelFormat::kRg32Sint:
            case core::TexelFormat::kRg32Uint:
            case core::TexelFormat::kRg8Sint:
            case core::TexelFormat::kRg8Uint:
            case core::TexelFormat::kRgb10A2Uint:
            case core::TexelFormat::kRgba16Sint:
            case core::TexelFormat::kRgba16Uint:
            case core::TexelFormat::kRgba32Sint:
            case core::TexelFormat::kRgba32Uint:
            case core::TexelFormat::kRgba8Sint:
            case core::TexelFormat::kRgba8Uint:
                break;

            // 32-bit float formats may still be able to use relaxed precision depending on their
            // usages.
            case core::TexelFormat::kR32Float:
            case core::TexelFormat::kRg32Float:
            case core::TexelFormat::kRgba32Float:
            case core::TexelFormat::kRg11B10Ufloat:
                if (all_usages_support_relaxed_precision) {
                    decorations.Add(value);
                }
                break;

            // Unknown formats can only use relaxed precision if their usages allow for it.
            // This covers sampled textures, where we can use relaxed precision if all sample and
            // fetch results are immediately converted to f16 types before use.
            case core::TexelFormat::kUndefined:
                if (all_usages_support_relaxed_precision) {
                    decorations.Add(value);
                }
                break;
        }
    }

    /// Walk usages of an image.
    /// Apply RelaxedPrecision decorations to operands and results where possible.
    /// @returns true if all usages allow for the image itself to be RelaxedPrecision
    bool WalkImageUsages(const core::ir::Value* image) {
        bool all_usages_allow_relaxed_precision = true;

        // Visit every use of the image object.
        Vector<const core::ir::Value*, 4> worklist{image};
        while (!worklist.IsEmpty()) {
            auto* value = worklist.Pop();
            for (auto use : value->UsagesSorted()) {
                all_usages_allow_relaxed_precision &= tint::Switch(
                    use.instruction,
                    [&](core::ir::Load* load) {  //
                        worklist.Push(load->Result());
                        return true;
                    },
                    [&](core::ir::UserCall* call) {
                        // If the function parameter does not have a RelaxedPrecision decoration,
                        // then the image passed as an argument cannot either.
                        auto param_idx = use.operand_index - core::ir::UserCall::kArgsOperandOffset;
                        auto* param = call->Target()->Params()[param_idx];
                        return decorations.Contains(param);
                    },
                    [&](ir::BuiltinCall* call) {  //
                        return ProcessImageBuiltin(call, worklist);
                    },
                    TINT_ICE_ON_NO_MATCH);
            }
        }

        return all_usages_allow_relaxed_precision;
    }

    /// Process an image builtin call, adding RelaxedPrecision decorations to texel values if
    /// possible, and updating @p worklist with any new values that need to be visited.
    /// @returns true if the builtin allows for the image to be decorated as RelaxedPrecision.
    bool ProcessImageBuiltin(ir::BuiltinCall* call, Vector<const core::ir::Value*, 4>& worklist) {
        switch (call->Func()) {
            // Functions that load or sample texel data can potentially have their result decorated
            // as RelaxedPrecision. Conversely, if they cannot be decorated as RelaxedPrecision,
            // then the originating variable cannot be either.
            case BuiltinFn::kImageDrefGather:
            case BuiltinFn::kImageFetch:
            case BuiltinFn::kImageGather:
            case BuiltinFn::kImageRead:
            case BuiltinFn::kImageSampleImplicitLod:
            case BuiltinFn::kImageSampleProjImplicitLod:
            case BuiltinFn::kImageSampleProjDrefImplicitLod:
            case BuiltinFn::kImageSampleExplicitLod:
            case BuiltinFn::kImageSampleProjExplicitLod:
            case BuiltinFn::kImageSampleProjDrefExplicitLod:
            case BuiltinFn::kImageSampleDrefImplicitLod:
            case BuiltinFn::kImageSampleDrefExplicitLod: {
                if (IsConvertedToF16(call->Result())) {
                    decorations.Add(call->Result());
                    return true;
                }
                return false;
            }

            // Functions that write texel data can potentially have the texel value decorated as
            // RelaxedPrecision. Conversely, if they cannot be decorated as RelaxedPrecision, then
            // the originating variable cannot be either.
            case BuiltinFn::kImageWrite: {
                auto* texel = call->Operand(2);
                if (IsConvertedFromF16(texel)) {
                    decorations.Add(texel);
                    return true;
                }
                return false;
            }

            // Walk through OpSampledImage to check the sampling operations.
            case BuiltinFn::kOpSampledImage:
                worklist.Push(call->Result());
                return true;

            // Query operations have no impact on allowable precision.
            case BuiltinFn::kImageQuerySize:
            case BuiltinFn::kImageQuerySizeLod:
            case BuiltinFn::kImageQueryLevels:
            case BuiltinFn::kImageQuerySamples:
                return true;

            default:
                TINT_IR_UNREACHABLE(ir) << "unhandled builtin call " << call->Func();
        }
    }

    /// @returns true if @p value is converted to an f16 type before any use
    bool IsConvertedToF16(core::ir::Value* value) {
        // Walk uses of the value.
        Vector<core::ir::Value*, 4> worklist{value};
        while (!worklist.IsEmpty()) {
            auto* next = worklist.Pop();
            for (auto use : next->UsagesUnsorted()) {
                auto* inst = use->instruction;
                if (inst->Is<core::ir::Convert>()) {
                    // Check if we are converting to F16.
                    if (!IsF16(inst->Result()->Type())) {
                        return false;
                    }
                } else if (inst->Is<core::ir::Let>()) {
                    // Walk through let instructions.
                    worklist.Push(inst->Result());
                } else {
                    // Any other instruction halts the analysis.
                    return false;
                }
            }
        }
        return true;
    }

    /// @returns true if @p value was converted from an f16 type
    bool IsConvertedFromF16(core::ir::Value* value) {
        // Walk backwards from the value.
        while (true) {
            auto* result = value->As<core::ir::InstructionResult>();
            if (!result) {
                return false;
            }

            auto* inst = result->Instruction();
            if (inst->IsAnyOf<core::ir::Convert>()) {
                // Check if we are converting from F16.
                return IsF16(inst->Operand(0)->Type());
            } else if (auto* let = inst->As<core::ir::Let>()) {
                // Walk through let instructions.
                value = let->Value();
                continue;
            } else {
                // Any other instructions halts the analysis.
                return false;
            }
        }
    }

    /// @returns true if @p type is an f16 type or a type whose elements are all f16
    bool IsF16(const core::type::Type* type) {
        return type->DeepestElement()->Is<core::type::F16>();
    }
};

}  // namespace

RelaxedPrecisionDecorations GetRelaxedPrecisionDecorations(const core::ir::Module& ir) {
    RelaxedPrecisionDecorations decorations;
    State{ir, decorations}.Process();
    return decorations;
}

}  // namespace tint::spirv::writer::analysis
