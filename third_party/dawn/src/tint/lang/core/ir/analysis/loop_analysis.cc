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

#include "src/tint/lang/core/ir/analysis/loop_analysis.h"

#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/bitcast.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/traverse.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::core::ir::analysis {

namespace {

/// Returns an instruction of the given kind if @p val is the result of such an instruction.
/// Otherwise returns nullptr.
template <typename InstClass>
InstClass* As(Value* val) {
    if (auto* instres = val->As<InstructionResult>()) {
        return instres->Instruction()->As<InstClass>();
    }
    return nullptr;
}

/// Returns the value, after unwrapping all bitcasts.
Value* UnwrapBitcast(Value* val) {
    while (auto* bitcast = As<Bitcast>(val)) {
        val = bitcast->Val();
    }
    return val;
}

/// Returns true if v is the integer constant 1.
bool IsOne(Value* v) {
    if (auto* cv = v->As<Constant>()) {
        return Switch(
            cv->Type(),
            [&](const core::type::I32*) { return cv->Value()->ValueAs<int32_t>() == 1; },
            [&](const core::type::U32*) { return cv->Value()->ValueAs<uint32_t>() == 1; },
            [&](const Default) -> bool { return false; });
    }
    return false;
}

/// Returns `true` if `val` is definitely `var +/- 1`.
bool IsIncrementOrDecrementOfVar(const Var& var, Value* val) {
    auto is_var_op_one = [&](Value* a, Value* b) {
        if (auto* a_load = As<Load>(a)) {
            return (a_load->From() == var.Result()) && IsOne(b);
        }
        return false;
    };
    if (auto* binary = As<Binary>(UnwrapBitcast(val))) {
        auto* lhs = UnwrapBitcast(binary->LHS());
        auto* rhs = UnwrapBitcast(binary->RHS());
        if (binary->Op() == BinaryOp::kAdd) {
            // Allow `var + 1` or `1 + var`.
            return is_var_op_one(lhs, rhs) || is_var_op_one(rhs, lhs);
        } else if (binary->Op() == BinaryOp::kSubtract) {
            // Only allow `var - 1`.
            return is_var_op_one(lhs, rhs);
        }
    }
    return false;
}

}  // anonymous namespace

/// PIMPL class that performs the analysis and holds the analysis cache.
struct LoopAnalysisImpl {
    explicit LoopAnalysisImpl(Function& func) {
        // Analyze all of the loops in the function.
        Traverse(func.Block(), [&](Loop* l) { AnalyzeLoop(*l); });
    }

    /// @returns the info for a loop
    const LoopInfo* GetInfo(const Loop& loop) const { return loop_info_map_.Get(&loop).value; }

  private:
    Hashmap<const Loop*, LoopInfo, 8> loop_info_map_;

    /// Analyze a loop.
    void AnalyzeLoop(Loop& loop) {
        if (auto* init_block = loop.Initializer()) {
            // Look for variables that could be used as iteration indices.
            for (auto* inst : *init_block) {
                auto* var = inst->As<Var>();
                if (!var) {
                    continue;
                }

                const auto* pty = var->Result()->Type()->As<core::type::Pointer>();
                if (!pty->StoreType()->IsIntegerScalar()) {
                    break;
                }

                // Check if the variable is an index that gives this loop a finite range.
                if (IsFiniteLoopIndex(loop, *var)) {
                    loop_info_map_.Add(&loop, LoopInfo{var});
                    return;
                }
            }
        }

        // We could not determine that this was a finite loop.
        loop_info_map_.Add(&loop, LoopInfo{});
    }

    /// Analyzes @p var as a candidate loop index variable for the given @p loop to see if it is
    /// used in a way that guarantees the loop will be iterating over a finite range.
    /// @returns true if @p var is an index variable for a finite ranged loop
    bool IsFiniteLoopIndex(Loop& loop, Var& index) {
        // Look for a store to the index in the continuing block.
        // Make sure there is only one, and make sure that the only other uses are loads.
        Store* single_store_in_continue_block = nullptr;
        const auto& uses = index.Result()->UsagesUnsorted();
        for (auto& use : uses) {
            if (auto* store = use->instruction->As<Store>()) {
                if (store->Block() != loop.Continuing()) {
                    // Not in the continuing block, so we cannot easily prove that it will be
                    // executed.
                    return false;
                }
                if (single_store_in_continue_block) {
                    // Found more than one store, so we cannot easily analyze this candidate.
                    return false;
                }
                single_store_in_continue_block = store;
            } else if (!use->instruction->Is<Load>()) {
                // Not a store or a load, so reject this candidate.
                return false;
            }
        }
        if (!single_store_in_continue_block) {
            return false;
        }

        // Check that the store we found is either incrementing or decrementing the index by `1`.
        if (!IsIncrementOrDecrementOfVar(index, single_store_in_continue_block->From())) {
            return false;
        }

        // We have proven that the variable increments exactly once per iteration.
        // Now check that the loop body begins with a break-if construct that will is guaranteed to
        // exit the loop if the index reaches the loop bound.
        bool has_break_if = false;
        for (auto* inst : *loop.Body()) {
            // The Switch returns `true` if more instructions should be checked, otherwise `false`.
            bool keep_going = Switch(
                inst,                            //
                [&](Load*) { return true; },     //
                [&](Bitcast*) { return true; },  //
                [&](Binary*) { return true; },   //
                [&](If* i) {
                    if (IsBreakIfOnIndex(loop, i, index)) {
                        // The loop is finite.
                        has_break_if = true;
                    }
                    // Only look at the first 'if' we find.
                    return false;
                },
                [&](Default) { return false; });
            if (!keep_going) {
                break;
            }
        }
        return has_break_if;
    }

    /// @returns `true` if @p is a break-if construct that exits the loop based on @p index.
    bool IsBreakIfOnIndex(const Loop& loop, If* i, Var& index) {
        // Returns `true` if the given value is a load of the index variable.
        auto is_index = [&index](Value* v) {
            if (auto* load = As<Load>(UnwrapBitcast(v))) {
                return load->From() == index.Result();
            }
            return false;
        };
        // Returns `true` if the given value an immutable value declared before the loop body.
        auto is_immutable_before_body = [&loop](Value* v) {
            return tint::Switch(
                UnwrapBitcast(v),                         //
                [](ir::Constant*) { return true; },       //
                [](ir::FunctionParam*) { return true; },  //
                [&](ir::InstructionResult* r) {
                    auto* let = r->Instruction()->As<Let>();
                    return let && let->Block() != loop.Body();
                }  //
            );
        };

        // Check if the condition matches (%idx < %bound) or (%idx > %bound).
        // The value %bound can be any immutable value that was declared before the body.
        auto* binary = As<Binary>(i->Condition());
        if (!binary) {
            return false;
        }
        if (binary->Op() != BinaryOp::kLessThan && binary->Op() != BinaryOp::kGreaterThan) {
            return false;
        }
        if ((is_index(binary->LHS()) && is_immutable_before_body(binary->RHS())) ||
            (is_index(binary->RHS()) && is_immutable_before_body(binary->LHS()))) {
            // The condition matches, so now make sure that one of the branches will exit from the
            // loop and do nothing else.
            auto is_simple_loop_exit = [](Block* b) {
                return b && !b->IsEmpty() && (b->Front()->Is<ExitLoop>());
            };
            return is_simple_loop_exit(i->True()) || is_simple_loop_exit(i->False());
        }
        return false;
    }
};

LoopAnalysis::LoopAnalysis(Function& func) : impl_(new LoopAnalysisImpl(func)) {}
LoopAnalysis::~LoopAnalysis() = default;

const LoopInfo* LoopAnalysis::GetInfo(const Loop& loop) const {
    return impl_->GetInfo(loop);
}

}  // namespace tint::core::ir::analysis
