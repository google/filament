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

#include "src/tint/lang/core/ir/const_param_validator.h"

#include <cstdint>
#include <string>
#include <utility>

#include "src/tint/lang/core/builtin_fn.h"
#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/core_binary.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/instruction_result.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/type/array_count.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/internal_limits.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::core::ir {

namespace {

/// The core IR const param validator.
class ConstParamValidator {
  public:
    /// Create a core const param validator
    /// @param mod the module to be validated
    explicit ConstParamValidator(Module& mod);

    /// Destructor
    ~ConstParamValidator();

    /// Runs the const param validator over the module provided during construction
    /// @returns success or failure
    Result<SuccessType> Run();

    void CheckSubgroupCall(const CoreBuiltinCall* call);
    void CheckBuiltinCall(const BuiltinCall* call);
    void CheckCoreBinaryCall(const CoreBinary* inst);
    void CheckExtractBitsCall(const CoreBuiltinCall* call);
    void CheckInsertBitsCall(const CoreBuiltinCall* call);
    void CheckLdexpCall(const CoreBuiltinCall* call);
    void CheckQuantizeToF16(const CoreBuiltinCall* call);
    void CheckClampCall(const CoreBuiltinCall* call);
    void CheckPack2x16float(const CoreBuiltinCall* call);
    void CheckSmoothstepCall(const CoreBuiltinCall* call);
    void CheckBinaryDivModCall(const CoreBinary* call);
    void CheckBinaryShiftCall(const CoreBinary* call);

    diag::Diagnostic& AddError(const Instruction& inst);

  private:
    Module& mod_;
    constant::Eval const_eval_;
    diag::List diagnostics_;
};

ConstParamValidator::ConstParamValidator(Module& mod)
    : mod_(mod), const_eval_(mod.constant_values, diagnostics_) {}

ConstParamValidator::~ConstParamValidator() = default;

diag::Diagnostic& ConstParamValidator::AddError(const Instruction& inst) {
    auto src = mod_.SourceOf(&inst);
    return diagnostics_.AddError(src);
}

const constant::Value* GetConstArg(const CoreBuiltinCall* call, uint32_t param_index) {
    if (call->Args().Length() <= param_index) {
        return nullptr;
    }
    if (call->Args()[param_index] == nullptr) {
        return nullptr;
    }
    if (!call->Args()[param_index]->Is<ir::Constant>()) {
        return nullptr;
    }
    return call->Args()[param_index]->As<ir::Constant>()->Value();
}

void ConstParamValidator::CheckExtractBitsCall(const CoreBuiltinCall* call) {
    // This can be u32/i32 or vector of those types.
    auto* param0 = call->Args()[0];
    auto* const_val_offset = GetConstArg(call, 1);
    auto* const_val_count = GetConstArg(call, 2);
    if (const_val_count && const_val_offset) {
        auto* zero = const_eval_.Zero(param0->Type(), {}, Source{}).Get();
        auto fakeArgs = Vector{zero, const_val_offset, const_val_count};
        [[maybe_unused]] auto result =
            const_eval_.extractBits(param0->Type(), fakeArgs, mod_.SourceOf(call));
    }
}

void ConstParamValidator::CheckInsertBitsCall(const CoreBuiltinCall* call) {
    // This can be u32/i32 or vector of those types.
    auto* param0 = call->Args()[0];
    auto* const_val_offset = GetConstArg(call, 2);
    auto* const_val_count = GetConstArg(call, 3);
    if (const_val_count && const_val_offset) {
        auto* zero = const_eval_.Zero(param0->Type(), {}, Source{}).Get();
        auto fakeArgs = Vector{zero, zero, const_val_offset, const_val_count};
        [[maybe_unused]] auto result =
            const_eval_.insertBits(param0->Type(), fakeArgs, mod_.SourceOf(call));
    }
}

void ConstParamValidator::CheckLdexpCall(const CoreBuiltinCall* call) {
    auto* param0 = call->Args()[0];
    if (auto const_val = GetConstArg(call, 1)) {
        auto* zero = const_eval_.Zero(param0->Type(), {}, Source{}).Get();
        auto fakeArgs = Vector{zero, const_val};
        [[maybe_unused]] auto result =
            const_eval_.ldexp(param0->Type(), fakeArgs, mod_.SourceOf(call));
    }
}

void ConstParamValidator::CheckQuantizeToF16(const CoreBuiltinCall* call) {
    if (auto const_val = GetConstArg(call, 0)) {
        [[maybe_unused]] auto result = const_eval_.quantizeToF16(
            call->Result()->Type(), Vector{const_val}, mod_.SourceOf(call));
    }
}

void ConstParamValidator::CheckPack2x16float(const CoreBuiltinCall* call) {
    if (auto const_val = GetConstArg(call, 0)) {
        [[maybe_unused]] auto result = const_eval_.pack2x16float(
            call->Result()->Type(), Vector{const_val}, mod_.SourceOf(call));
    }
}

void ConstParamValidator::CheckSubgroupCall(const CoreBuiltinCall* call) {
    if (auto const_val = GetConstArg(call, 1)) {
        auto as_aint = const_val->ValueAs<AInt>();
        // User friendly param name.
        std::string paramName = "sourceLaneIndex";
        switch (call->Func()) {
            case core::BuiltinFn::kSubgroupShuffleXor:
                paramName = "mask";
                break;
            case core::BuiltinFn::kSubgroupShuffleUp:
            case core::BuiltinFn::kSubgroupShuffleDown:
                paramName = "delta";
                break;
            default:
                break;
        }

        if (as_aint >= tint::internal_limits::kMaxSubgroupSize) {
            AddError(*call) << "The " << paramName << " argument of " << call->FriendlyName()
                            << " must be less than " << tint::internal_limits::kMaxSubgroupSize;
        } else if (as_aint < 0) {
            AddError(*call) << "The " << paramName << " argument of " << call->FriendlyName()
                            << " must be greater than or equal to zero";
        }
    }
}

void ConstParamValidator::CheckClampCall(const CoreBuiltinCall* call) {
    auto* const_val_low = GetConstArg(call, 1);
    auto* const_val_high = GetConstArg(call, 2);
    if (const_val_low && const_val_high) {
        auto fakeArgs = Vector{const_val_low, const_val_low, const_val_high};
        [[maybe_unused]] auto result =
            const_eval_.clamp(call->Result()->Type(), fakeArgs, mod_.SourceOf(call));
    }
}

void ConstParamValidator::CheckSmoothstepCall(const CoreBuiltinCall* call) {
    auto* const_val_low = GetConstArg(call, 0);
    auto* const_val_high = GetConstArg(call, 1);
    if (const_val_low && const_val_high) {
        auto fakeArgs = Vector{const_val_low, const_val_high, const_val_high};
        [[maybe_unused]] auto result =
            const_eval_.smoothstep(call->Result()->Type(), fakeArgs, mod_.SourceOf(call));
    }
}

void ConstParamValidator::CheckBuiltinCall(const BuiltinCall* inst) {
    if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
        switch (call->Func()) {
            case core::BuiltinFn::kSubgroupShuffle:
            case core::BuiltinFn::kSubgroupShuffleXor:
            case core::BuiltinFn::kSubgroupShuffleUp:
            case core::BuiltinFn::kSubgroupShuffleDown:
                CheckSubgroupCall(call);
                break;
            case core::BuiltinFn::kExtractBits:
                CheckExtractBitsCall(call);
                break;
            case core::BuiltinFn::kInsertBits:
                CheckInsertBitsCall(call);
                break;
            case core::BuiltinFn::kLdexp:
                CheckLdexpCall(call);
                break;
            case core::BuiltinFn::kClamp:
                CheckClampCall(call);
                break;
            case core::BuiltinFn::kSmoothstep:
                CheckSmoothstepCall(call);
                break;
            case core::BuiltinFn::kQuantizeToF16:
                CheckQuantizeToF16(call);
                break;
            case core::BuiltinFn::kPack2X16Float:
                CheckPack2x16float(call);
                break;
            default:
                break;
        }
    }
}

void ConstParamValidator::CheckBinaryDivModCall(const CoreBinary* call) {
    // Integer division by zero should be checked for the partial evaluation case (only rhs
    // is const). FP division by zero is only invalid when the whole expression is
    // constant-evaluated.
    if (call->RHS()->Type()->IsIntegerScalarOrVector()) {
        auto rhs_constant = call->RHS()->As<ir::Constant>();
        if (rhs_constant && rhs_constant->Value()->AnyZero()) {
            AddError(*call) << "integer division by zero is invalid";
        }
    }
}

void ConstParamValidator::CheckBinaryShiftCall(const CoreBinary* call) {
    // If lhs value is a concrete type, and rhs is a const-expression greater than or equal
    // to the bit width of lhs, then it is a shader-creation error.
    const auto* elem_type = call->LHS()->Type()->DeepestElement();
    const uint32_t bit_width = elem_type->Size() * 8;
    if (auto* rhs_val_as_const = call->RHS()->As<ir::Constant>()) {
        auto* rhs_as_value = rhs_val_as_const->Value();
        for (size_t i = 0, n = rhs_as_value->NumElements(); i < n; i++) {
            auto* shift_val = n == 1 ? rhs_as_value : rhs_as_value->Index(i);
            if (shift_val->ValueAs<u32>() >= bit_width) {
                AddError(*call) << "shift "
                                << (call->Op() == core::BinaryOp::kShiftLeft ? "left" : "right")
                                << " value must be less than the bit width of the lhs, which is "
                                << bit_width;
                break;
            }
        }
    }
}

void ConstParamValidator::CheckCoreBinaryCall(const CoreBinary* call) {
    switch (call->Op()) {
        case core::BinaryOp::kDivide:
        case core::BinaryOp::kModulo:
            CheckBinaryDivModCall(call);
            break;
        case core::BinaryOp::kShiftLeft:
        case core::BinaryOp::kShiftRight:
            CheckBinaryShiftCall(call);
            break;
        default:
            break;
    }
}

Result<SuccessType> ConstParamValidator::Run() {
    auto instructions = this->mod_.Instructions();

    for (auto inst : instructions) {
        tint::Switch(
            inst,                                                  //
            [&](const BuiltinCall* c) { CheckBuiltinCall(c); },    //
            [&](const CoreBinary* c) { CheckCoreBinaryCall(c); },  //
            [&](Default) {});
    }

    if (diagnostics_.ContainsErrors()) {
        return Failure{diagnostics_.Str()};
    }

    return Success;
}

}  // namespace

Result<SuccessType> ValidateConstParam(Module& mod) {
    ConstParamValidator v(mod);
    return v.Run();
}

}  // namespace tint::core::ir
