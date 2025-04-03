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

#include "src/tint/lang/core/ir/analysis/integer_range_analysis.h"

#include <limits>

#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/traverse.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::core::ir::analysis {

namespace {
/// Returns true if v is the integer constant 1.
bool IsOne(const Value* v) {
    if (auto* cv = v->As<Constant>()) {
        return Switch(
            cv->Type(),
            [&](const core::type::I32*) { return cv->Value()->ValueAs<int32_t>() == 1; },
            [&](const core::type::U32*) { return cv->Value()->ValueAs<uint32_t>() == 1; },
            [&](const Default) -> bool { return false; });
    }
    return false;
}

bool IsConstantInteger(const Value* v) {
    if (auto* cv = v->As<Constant>()) {
        return cv->Type()->IsIntegerScalar();
    }
    return false;
}
}  // namespace

IntegerRangeInfo::IntegerRangeInfo(int64_t min_bound, int64_t max_bound) {
    TINT_ASSERT(min_bound <= max_bound);
    range = SignedIntegerRange{min_bound, max_bound};
}

IntegerRangeInfo::IntegerRangeInfo(uint64_t min_bound, uint64_t max_bound) {
    TINT_ASSERT(min_bound <= max_bound);
    range = UnsignedIntegerRange{min_bound, max_bound};
}

struct IntegerRangeAnalysisImpl {
    explicit IntegerRangeAnalysisImpl(Function* func) : function_(func) {}

    const IntegerRangeInfo* GetInfo(const FunctionParam* param, uint32_t index) {
        if (!param->Type()->IsIntegerScalarOrVector()) {
            return nullptr;
        }

        const auto& info = integer_function_param_range_info_map_.GetOrAdd(
            param, [&]() -> Vector<IntegerRangeInfo, 3> {
                if (param->Builtin() == core::BuiltinValue::kLocalInvocationIndex) {
                    // We shouldn't be trying to use range analysis on a module that has
                    // non-constant workgroup sizes, since we will always have replaced pipeline
                    // overrides with constant values early in the pipeline.
                    TINT_ASSERT(function_->WorkgroupSizeAsConst().has_value());
                    std::array<uint32_t, 3> workgroup_size =
                        function_->WorkgroupSizeAsConst().value();
                    uint64_t max_bound =
                        workgroup_size[0] * workgroup_size[1] * workgroup_size[2] - 1u;
                    constexpr uint64_t kMinBound = 0;

                    return {IntegerRangeInfo(kMinBound, max_bound)};
                }

                if (param->Builtin() == core::BuiltinValue::kLocalInvocationId) {
                    TINT_ASSERT(function_->WorkgroupSizeAsConst().has_value());
                    std::array<uint32_t, 3> workgroup_size =
                        function_->WorkgroupSizeAsConst().value();

                    constexpr uint64_t kMinBound = 0;
                    Vector<IntegerRangeInfo, 3> integerRanges;
                    for (uint32_t size_x_y_z : workgroup_size) {
                        integerRanges.Push({kMinBound, size_x_y_z - 1u});
                    }
                    return integerRanges;
                }

                if (param->Type()->IsUnsignedIntegerScalar()) {
                    return {IntegerRangeInfo(0, std::numeric_limits<uint64_t>::max())};
                } else {
                    TINT_ASSERT(param->Type()->IsSignedIntegerScalar());
                    return {IntegerRangeInfo(std::numeric_limits<int64_t>::min(),
                                             std::numeric_limits<int64_t>::max())};
                }
            });

        TINT_ASSERT(info.Length() > index);
        return &info[index];
    }

    const Var* GetLoopControlVariableFromConstantInitializer(const Loop* loop) {
        TINT_ASSERT(loop);

        auto* init_block = loop->Initializer();
        if (!init_block) {
            return nullptr;
        }

        // Currently we only support the loop initializer of a simple for-loop, which only has two
        // instructions
        // - The first instruction is to initialize the loop control variable
        //   with a constant integer (signed or unsigned) value.
        // - The second instruction is `next_iteration`
        // e.g. for (var i = 0; ...)
        if (init_block->Length() != 2u) {
            return nullptr;
        }

        auto* var = init_block->Front()->As<Var>();
        if (!var) {
            return nullptr;
        }

        if (!init_block->Back()->As<NextIteration>()) {
            return nullptr;
        }

        const auto* pointer = var->Result()->Type()->As<core::type::Pointer>();
        if (!pointer->StoreType()->IsIntegerScalar()) {
            return nullptr;
        }

        const auto* initializer = var->Initializer();
        if (!initializer) {
            return nullptr;
        }

        if (!initializer->As<Constant>()) {
            return nullptr;
        }

        return var;
    }

    // Currently we only support the loop continuing of a simple for-loop, which only has 4
    // instructions
    /// - The first instruction is to load the loop control variable into a temporary variable.
    /// - The second instruction is to add one or minus one to the temporary variable.
    /// - The third instruction is to store the value of the temporary variable into the loop
    ///   control variable.
    /// - The fourth instruction is `next_iteration`.
    const Binary* GetBinaryToUpdateLoopControlVariableInContinuingBlock(
        const Loop* loop,
        const Var* loop_control_variable) {
        TINT_ASSERT(loop);
        TINT_ASSERT(loop_control_variable);

        auto* continuing_block = loop->Continuing();
        if (!continuing_block) {
            return nullptr;
        }

        if (continuing_block->Length() != 4u) {
            return nullptr;
        }

        // 1st instruction:
        // %src = load %loop_control_variable
        const auto* load_from_loop_control_variable = continuing_block->Instructions()->As<Load>();
        if (!load_from_loop_control_variable) {
            return nullptr;
        }
        if (load_from_loop_control_variable->From() != loop_control_variable->Result()) {
            return nullptr;
        }

        // 2nd instruction:
        // %dst = add %src, 1
        // or %dst = add 1, %src
        // or %dst = sub %src, 1
        const auto* add_or_sub_from_loop_control_variable =
            load_from_loop_control_variable->next->As<Binary>();
        if (!add_or_sub_from_loop_control_variable) {
            return nullptr;
        }
        const auto* src = load_from_loop_control_variable->Result();
        const auto* lhs = add_or_sub_from_loop_control_variable->LHS();
        const auto* rhs = add_or_sub_from_loop_control_variable->RHS();
        switch (add_or_sub_from_loop_control_variable->Op()) {
            case BinaryOp::kAdd: {
                // %dst = add %src, 1
                if (lhs == src && IsOne(rhs)) {
                    break;
                }
                // %dst = add 1, %src
                if (rhs == src && IsOne(lhs)) {
                    break;
                }
                return nullptr;
            }
            case BinaryOp::kSubtract: {
                // %dst = sub %src, 1
                if (lhs == src && IsOne(rhs)) {
                    break;
                }
                return nullptr;
            }
            default:
                return nullptr;
        }

        // 3rd instruction:
        // store %loop_control_variable, %dst
        const auto* store_into_loop_control_variable =
            add_or_sub_from_loop_control_variable->next->As<Store>();
        if (!store_into_loop_control_variable) {
            return nullptr;
        }
        const auto* dst = add_or_sub_from_loop_control_variable->Result();
        if (store_into_loop_control_variable->From() != dst) {
            return nullptr;
        }
        if (store_into_loop_control_variable->To() != loop_control_variable->Result()) {
            return nullptr;
        }

        // 4th instruction:
        // next_iteration
        if (!store_into_loop_control_variable->next->As<NextIteration>()) {
            return nullptr;
        }

        return add_or_sub_from_loop_control_variable;
    }

    // Currently we only support the loop continuing of a simple for-loop which meets all the below
    // requirements:
    // - The loop control variable is only used as the parameter of the load instruction.
    // - The first instruction is to load the loop control variable into a temporary variable.
    // - The second instruction is to compare the temporary variable with a constant value and save
    //   the result to a boolean variable.
    // - The second instruction cannot be a comparison that will never return true.
    // - The third instruction is an `ifelse` expression that uses the boolean variable got in the
    //   second instruction as the condition.
    // - The true block of the above `ifelse` expression doesn't contain `exit_loop`.
    // - The false block of the above `ifelse` expression only contains `exit_loop`.
    const Binary* GetBinaryToCompareLoopControlVariableInLoopBody(
        const Loop* loop,
        const Var* loop_control_variable) {
        TINT_ASSERT(loop);
        TINT_ASSERT(loop_control_variable);

        auto* body_block = loop->Body();

        // Reject any non-load instructions unless it is a store in the continuing block
        const auto& uses = loop_control_variable->Result(0)->UsagesUnsorted();
        for (auto& use : uses) {
            if (use->instruction->Is<Load>()) {
                continue;
            }
            if (use->instruction->Is<Store>() && use->instruction->Block() == loop->Continuing()) {
                continue;
            }
            return nullptr;
        }

        // 1st instruction:
        // %src = load %loop_control_variable
        const auto* load_from_loop_control_variable = body_block->Instructions()->As<Load>();
        if (!load_from_loop_control_variable) {
            return nullptr;
        }
        if (load_from_loop_control_variable->From() != loop_control_variable->Result(0)) {
            return nullptr;
        }

        // 2nd instruction:
        // %condition:bool = lt(gt, lte, gte) %src, constant_value
        // or %condition:bool = lt(gt, lte, gte) constant_value, %src
        const auto* exit_condition_on_loop_control_variable =
            load_from_loop_control_variable->next->As<Binary>();
        if (!exit_condition_on_loop_control_variable) {
            return nullptr;
        }
        BinaryOp op = exit_condition_on_loop_control_variable->Op();
        auto* lhs = exit_condition_on_loop_control_variable->LHS();
        auto* rhs = exit_condition_on_loop_control_variable->RHS();
        switch (op) {
            case BinaryOp::kLessThan:
            case BinaryOp::kGreaterThan:
            case BinaryOp::kLessThanEqual:
            case BinaryOp::kGreaterThanEqual: {
                if (IsConstantInteger(rhs) && lhs == load_from_loop_control_variable->Result(0)) {
                    break;
                }
                if (IsConstantInteger(lhs) && rhs == load_from_loop_control_variable->Result(0)) {
                    break;
                }
                return nullptr;
            }
            default:
                return nullptr;
        }

        // Early-return when the comparison will never return true.
        if (op == BinaryOp::kLessThan) {
            if (IsConstantInteger(lhs)) {
                // std::numeric_limits<uint32_t>::max() < idx
                if (lhs->As<Constant>()->Value()->Type()->IsUnsignedIntegerScalar() &&
                    lhs->As<Constant>()->Value()->ValueAs<uint32_t>() ==
                        std::numeric_limits<uint32_t>::max()) {
                    return nullptr;
                }
                // std::numeric_limits<int32_t>::max() < idx
                if (lhs->As<Constant>()->Value()->Type()->IsSignedIntegerScalar() &&
                    lhs->As<Constant>()->Value()->ValueAs<int32_t>() ==
                        std::numeric_limits<int32_t>::max()) {
                    return nullptr;
                }
            } else {
                TINT_ASSERT(IsConstantInteger(rhs));
                // idx < 0u
                if (rhs->As<Constant>()->Value()->Type()->IsUnsignedIntegerScalar() &&
                    rhs->As<Constant>()->Value()->ValueAs<uint32_t>() ==
                        std::numeric_limits<uint32_t>::min()) {
                    return nullptr;
                }
                // idx < std::numeric_limits<int32_t>::min()
                if (rhs->As<Constant>()->Value()->Type()->IsSignedIntegerScalar() &&
                    rhs->As<Constant>()->Value()->ValueAs<int32_t>() ==
                        std::numeric_limits<int32_t>::min()) {
                    return nullptr;
                }
            }
        } else if (op == BinaryOp::kGreaterThan) {
            if (IsConstantInteger(lhs)) {
                // std::numeric_limits<uint32_t>::min() > idx
                if (lhs->As<Constant>()->Value()->Type()->IsUnsignedIntegerScalar() &&
                    lhs->As<Constant>()->Value()->ValueAs<uint32_t>() ==
                        std::numeric_limits<uint32_t>::min()) {
                    return nullptr;
                }
                // std::numeric_limits<int32_t>::min() > idx
                if (lhs->As<Constant>()->Value()->Type()->IsSignedIntegerScalar() &&
                    lhs->As<Constant>()->Value()->ValueAs<int32_t>() ==
                        std::numeric_limits<int32_t>::min()) {
                    return nullptr;
                }
            } else {
                TINT_ASSERT(IsConstantInteger(rhs));
                // idx > std::numeric_limits<uint32_t>::max()
                if (rhs->As<Constant>()->Value()->Type()->IsUnsignedIntegerScalar() &&
                    rhs->As<Constant>()->Value()->ValueAs<uint32_t>() ==
                        std::numeric_limits<uint32_t>::max()) {
                    return nullptr;
                }
                // idx > std::numeric_limits<int32_t>::max()
                if (rhs->As<Constant>()->Value()->Type()->IsSignedIntegerScalar() &&
                    rhs->As<Constant>()->Value()->ValueAs<int32_t>() ==
                        std::numeric_limits<int32_t>::max()) {
                    return nullptr;
                }
            }
        }

        // 3rd instruction:
        // if %condition [t: $true, f: $false] {
        //   $true: {
        //     // Maybe some other instructions
        //     exit_if
        //   }
        //   $false: { exit_loop }
        // }
        const auto* if_on_exit_condition = exit_condition_on_loop_control_variable->next->As<If>();
        if (!if_on_exit_condition) {
            return nullptr;
        }
        if (if_on_exit_condition->Condition() !=
            exit_condition_on_loop_control_variable->Result(0)) {
            return nullptr;
        }
        if (!if_on_exit_condition->True()->Terminator()->As<ExitIf>()) {
            return nullptr;
        }
        if (!if_on_exit_condition->False()->Front()->As<ExitLoop>()) {
            return nullptr;
        }

        return exit_condition_on_loop_control_variable;
    }

  private:
    Function* function_;
    Hashmap<const FunctionParam*, Vector<IntegerRangeInfo, 3>, 4>
        integer_function_param_range_info_map_;
};

IntegerRangeAnalysis::IntegerRangeAnalysis(Function* func)
    : impl_(new IntegerRangeAnalysisImpl(func)) {}
IntegerRangeAnalysis::~IntegerRangeAnalysis() = default;

const IntegerRangeInfo* IntegerRangeAnalysis::GetInfo(const FunctionParam* param, uint32_t index) {
    return impl_->GetInfo(param, index);
}

const Var* IntegerRangeAnalysis::GetLoopControlVariableFromConstantInitializerForTest(
    const Loop* loop) {
    return impl_->GetLoopControlVariableFromConstantInitializer(loop);
}

const Binary* IntegerRangeAnalysis::GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(
    const Loop* loop,
    const Var* loop_control_variable) {
    return impl_->GetBinaryToUpdateLoopControlVariableInContinuingBlock(loop,
                                                                        loop_control_variable);
}

const Binary* IntegerRangeAnalysis::GetBinaryToCompareLoopControlVariableInLoopBodyForTest(
    const Loop* loop,
    const Var* loop_control_variable) {
    return impl_->GetBinaryToCompareLoopControlVariableInLoopBody(loop, loop_control_variable);
}

}  // namespace tint::core::ir::analysis
