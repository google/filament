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

#include <concepts>

#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"

namespace tint::core::ir::analysis {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_LoopAnalysisTest : public IRTestHelper {};

enum class Type : uint8_t {
    kI32,
    kU32,
};
enum class Direction : uint8_t {
    kIndexOpBound,
    kBoundOpIndex,
};
enum class BoundKind : uint8_t {
    kConstant,
    kFunctionParam,
    kLet,
    kVar,
};
struct BoundDescriptor {
    Type type;
    Direction dir;
    BoundKind kind;
    int64_t const_value;
    BinaryOp op;
    bool is_finite;
};

// Empty structure used to describe the position of the index variable in an expression.
struct IndexValue {
} Index;

template <typename T>
concept I32OrU32 = std::same_as<T, i32> || std::same_as<T, u32>;

// Helper to declare a BoundDescriptor with a concise expression.
// e.g.
//   Bound<i32, Index, kLessThan, 10u>(true)
//   Bound<u32, kParameter, kLessThan, Index>(true)
//
template <I32OrU32 T, auto LHS, BinaryOp op, auto RHS>
BoundDescriptor Bound(bool is_infinite) {
    BoundDescriptor desc;
    desc.op = op;
    desc.is_finite = is_infinite;
    if constexpr (std::is_same_v<T, i32>) {
        desc.type = Type::kI32;
    } else if constexpr (std::is_same_v<T, u32>) {
        desc.type = Type::kU32;
    }

    if constexpr (std::is_same_v<decltype(LHS), IndexValue>) {
        // Index OP Bound
        desc.dir = Direction::kIndexOpBound;
        if constexpr (std::is_same_v<decltype(RHS), enum BoundKind>) {
            desc.kind = RHS;
        } else {
            desc.kind = BoundKind::kConstant;
            desc.const_value = RHS;
        }
    } else {
        // Bound OP Index
        desc.dir = Direction::kBoundOpIndex;
        if constexpr (std::is_same_v<decltype(LHS), enum BoundKind>) {
            desc.kind = LHS;
        } else {
            desc.kind = BoundKind::kConstant;
            desc.const_value = LHS;
        }
    }
    return desc;
}

void PrintKind(tint::StringStream& stream, const BoundDescriptor& bound) {
    switch (bound.kind) {
        case BoundKind::kConstant:
            if (bound.type == Type::kI32) {
                int64_t i = bound.const_value;
                if (i < 0) {
                    stream << "n";
                    i = -i;
                }
                stream << i << "i";
            } else if (bound.type == Type::kU32) {
                stream << bound.const_value << "u";
            }
            break;
        case BoundKind::kFunctionParam:
            stream << "param";
            break;
        case BoundKind::kLet:
            stream << "let";
            break;
        case BoundKind::kVar:
            stream << "var";
            break;
    }
}

// These parameterized tests cover the many different ways of constructing the loop exit condition:
//
//   if (LHS OP RHS) {
//     break;
//   }
//
// Either LHS or RHS will be the loop index.
// The other will be either a constant value or a selection from other possible value sources.
//
using IR_LoopAnalysisBoundTest = IRTestParamHelper<BoundDescriptor>;
TEST_P(IR_LoopAnalysisBoundTest, Bound) {
    auto params = GetParam();

    const core::type::Type* type = nullptr;
    core::ir::Constant* zero = nullptr;
    core::ir::Constant* one = nullptr;
    if (params.type == Type::kI32) {
        type = ty.i32();
        zero = b.Constant(0_i);
        one = b.Constant(1_i);
    } else {
        type = ty.u32();
        zero = b.Constant(0_u);
        one = b.Constant(1_u);
    }

    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* param = b.FunctionParam("param", type);
        func->AppendParam(param);
        auto* let = b.Let("let", param);
        auto* var = b.Var("var", param);

        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", zero);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            Instruction* cond = nullptr;

            core::ir::Value* bound = nullptr;
            switch (params.kind) {
                case BoundKind::kConstant:
                    if (params.type == Type::kI32) {
                        bound = b.Constant(i32(params.const_value));
                    } else {
                        bound = b.Constant(u32(params.const_value));
                    }
                    break;
                case BoundKind::kFunctionParam:
                    bound = param;
                    break;
                case BoundKind::kLet:
                    bound = let->Result();
                    break;
                case BoundKind::kVar:
                    bound = b.Load(var)->Result();
                    break;
            }

            switch (params.dir) {
                case Direction::kIndexOpBound:
                    cond = b.Binary(params.op, ty.bool_(), b.Load(idx), bound);
                    break;
                case Direction::kBoundOpIndex:
                    cond = b.Binary(params.op, ty.bool_(), bound, b.Load(idx));
                    break;
            }

            auto* ifelse = b.If(cond);
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), one));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);

    if (params.is_finite) {
        EXPECT_TRUE(info->IsFinite());
        EXPECT_EQ(info->index_var, idx);
    } else {
        EXPECT_FALSE(info->IsFinite());
        EXPECT_EQ(info->index_var, nullptr);
    }
}

using enum BinaryOp;
using enum BoundKind;
INSTANTIATE_TEST_SUITE_P(,
                         IR_LoopAnalysisBoundTest,
                         testing::Values(
                             // Comparing the index to a constant that is not a limit is always OK.
                             Bound<i32, Index, kLessThan, 10>(true),
                             Bound<i32, 10, kLessThan, Index>(true),
                             Bound<u32, Index, kLessThan, 10>(true),
                             Bound<u32, 10, kLessThan, Index>(true),

                             Bound<i32, Index, kGreaterThan, 10>(true),
                             Bound<i32, 10, kGreaterThan, Index>(true),
                             Bound<u32, Index, kGreaterThan, 10>(true),
                             Bound<u32, 10, kGreaterThan, Index>(true),

                             Bound<i32, Index, kLessThanEqual, 10>(true),
                             Bound<i32, 10, kLessThanEqual, Index>(true),
                             Bound<u32, Index, kLessThanEqual, 10>(true),
                             Bound<u32, 10, kLessThanEqual, Index>(true),

                             Bound<i32, Index, kGreaterThanEqual, 10>(true),
                             Bound<i32, 10, kGreaterThanEqual, Index>(true),
                             Bound<u32, Index, kGreaterThanEqual, 10>(true),
                             Bound<u32, 10, kGreaterThanEqual, Index>(true),

                             // Comparing the index to a constant that is at a limit can result in
                             // an always-true or always-false result, which is not OK.
                             Bound<i32, Index, kLessThan, INT32_MIN>(false),
                             Bound<u32, Index, kLessThan, 0>(false),
                             Bound<i32, INT32_MAX, kLessThan, Index>(false),
                             Bound<u32, UINT32_MAX, kLessThan, Index>(false),

                             Bound<i32, Index, kGreaterThan, INT32_MAX>(false),
                             Bound<u32, Index, kGreaterThan, UINT32_MAX>(false),
                             Bound<i32, INT32_MIN, kGreaterThan, Index>(false),
                             Bound<u32, 0, kGreaterThan, Index>(false),

                             Bound<i32, Index, kLessThanEqual, INT32_MAX>(false),
                             Bound<u32, Index, kLessThanEqual, UINT32_MAX>(false),
                             Bound<i32, INT32_MIN, kLessThanEqual, Index>(false),
                             Bound<u32, 0u, kLessThanEqual, Index>(false),

                             Bound<i32, Index, kGreaterThanEqual, INT32_MIN>(false),
                             Bound<u32, Index, kGreaterThanEqual, 0u>(false),
                             Bound<i32, INT32_MAX, kGreaterThanEqual, Index>(false),
                             Bound<u32, UINT32_MAX, kGreaterThanEqual, Index>(false),

                             // Using other immutable values for the bound is not OK since that
                             // value could result in an always-true or always-false outcome.
                             Bound<i32, Index, kLessThan, kFunctionParam>(false),
                             Bound<i32, Index, kLessThan, kLet>(false),
                             Bound<i32, kFunctionParam, kLessThan, Index>(false),
                             Bound<i32, kLet, kLessThan, Index>(false),
                             Bound<u32, Index, kLessThan, kFunctionParam>(false),
                             Bound<u32, Index, kLessThan, kLet>(false),
                             Bound<u32, kFunctionParam, kLessThan, Index>(false),
                             Bound<u32, kLet, kLessThan, Index>(false),

                             Bound<i32, Index, kGreaterThan, kFunctionParam>(false),
                             Bound<i32, Index, kGreaterThan, kLet>(false),
                             Bound<i32, kFunctionParam, kGreaterThan, Index>(false),
                             Bound<i32, kLet, kGreaterThan, Index>(false),
                             Bound<u32, Index, kGreaterThan, kFunctionParam>(false),
                             Bound<u32, Index, kGreaterThan, kLet>(false),
                             Bound<u32, kFunctionParam, kGreaterThan, Index>(false),
                             Bound<u32, kLet, kGreaterThan, Index>(false),

                             Bound<i32, Index, kLessThanEqual, kFunctionParam>(false),
                             Bound<i32, Index, kLessThanEqual, kLet>(false),
                             Bound<i32, kFunctionParam, kLessThanEqual, Index>(false),
                             Bound<i32, kLet, kLessThanEqual, Index>(false),
                             Bound<u32, Index, kLessThanEqual, kFunctionParam>(false),
                             Bound<u32, Index, kLessThanEqual, kLet>(false),
                             Bound<u32, kFunctionParam, kLessThanEqual, Index>(false),
                             Bound<u32, kLet, kLessThanEqual, Index>(false),

                             Bound<i32, Index, kGreaterThanEqual, kFunctionParam>(false),
                             Bound<i32, Index, kGreaterThanEqual, kLet>(false),
                             Bound<i32, kFunctionParam, kGreaterThanEqual, Index>(false),
                             Bound<i32, kLet, kGreaterThanEqual, Index>(false),
                             Bound<u32, Index, kGreaterThanEqual, kFunctionParam>(false),
                             Bound<u32, Index, kGreaterThanEqual, kLet>(false),
                             Bound<u32, kFunctionParam, kGreaterThanEqual, Index>(false),
                             Bound<u32, kLet, kGreaterThanEqual, Index>(false),

                             // Using a var for the bound is never OK.
                             Bound<i32, Index, kLessThan, kVar>(false),
                             Bound<i32, kVar, kGreaterThan, Index>(false),
                             Bound<u32, Index, kLessThanEqual, kVar>(false),
                             Bound<u32, kVar, kGreaterThanEqual, Index>(false)

                             //
                             ),
                         [](const testing::TestParamInfo<BoundDescriptor>& desc) {
                             tint::StringStream name;

                             switch (desc.param.type) {
                                 case Type::kI32:
                                     name << "i32_";
                                     break;
                                 case Type::kU32:
                                     name << "u32_";
                                     break;
                             }

                             if (desc.param.dir == Direction::kIndexOpBound) {
                                 name << "index";
                             } else {
                                 PrintKind(name, desc.param);
                             }

                             switch (desc.param.op) {
                                 case kLessThan:
                                     name << "_lt_";
                                     break;
                                 case kGreaterThan:
                                     name << "_gt_";
                                     break;
                                 case kLessThanEqual:
                                     name << "_lte_";
                                     break;
                                 case kGreaterThanEqual:
                                     name << "_gte_";
                                     break;
                                 default:
                                     break;
                             }

                             if (desc.param.dir == Direction::kBoundOpIndex) {
                                 name << "index";
                             } else {
                                 PrintKind(name, desc.param);
                             }

                             return name.str();
                         });

TEST_F(IR_LoopAnalysisTest, Finite_IndexLessThanConstant_OnePlusIndex) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(1_u, b.Load(idx)));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        %6:u32 = add 1u, %5
        store %idx, %6
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_TRUE(info->IsFinite());
    EXPECT_EQ(info->index_var, idx);
}

TEST_F(IR_LoopAnalysisTest, Finite_IndexLessThanConstant_DecByOne) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Subtract(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        %6:u32 = sub %5, 1u
        store %idx, %6
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_TRUE(info->IsFinite());
    EXPECT_EQ(info->index_var, idx);
}

TEST_F(IR_LoopAnalysisTest, Finite_NonZeroInit) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 5_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 5u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        %6:u32 = add %5, 1u
        store %idx, %6
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_TRUE(info->IsFinite());
    EXPECT_EQ(info->index_var, idx);
}

TEST_F(IR_LoopAnalysisTest, Finite_BreakInFalse) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.GreaterThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitIf(ifelse);
            });
            b.Append(ifelse->False(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = gt %3, 10u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        %6:u32 = add %5, 1u
        store %idx, %6
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_TRUE(info->IsFinite());
    EXPECT_EQ(info->index_var, idx);
}

TEST_F(IR_LoopAnalysisTest, Finite_MultipleLoads) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            auto* load1 = b.Load(idx);
            auto* load2 = b.Load(idx);
            b.Let("x", b.Add(load1, load2));
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });

            auto* load1 = b.Load(idx);
            auto* load2 = b.Load(idx);
            b.Let("x", b.Add(load1, load2));
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            auto* load1 = b.Load(idx);
            auto* load2 = b.Load(idx);
            b.Let("x", b.Add(load1, load2));
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        %3:u32 = load %idx
        %4:u32 = load %idx
        %5:u32 = add %3, %4
        %x:u32 = let %5
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %7:u32 = load %idx
        %8:bool = lt %7, 10u
        if %8 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        %9:u32 = load %idx
        %10:u32 = load %idx
        %11:u32 = add %9, %10
        %x_1:u32 = let %11  # %x_1: 'x'
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:u32 = load %idx
        %14:u32 = load %idx
        %15:u32 = add %13, %14
        %x_2:u32 = let %15  # %x_2: 'x'
        %17:u32 = load %idx
        %18:u32 = add %17, 1u
        store %idx, %18
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_TRUE(info->IsFinite());
    EXPECT_EQ(info->index_var, idx);
}

TEST_F(IR_LoopAnalysisTest, Finite_Bitcasts) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Bitcast<u32>(b.Bitcast<i32>(b.Load(idx))), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            auto* lhs = b.Bitcast<i32>(b.Bitcast<u32>(b.Bitcast<i32>(b.Load(idx))));
            auto* rhs = b.Bitcast<i32>(b.Bitcast<u32>(1_u));
            b.Store(idx, b.Bitcast<u32>(b.Add(lhs, rhs)));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:i32 = bitcast<i32> %3
        %5:u32 = bitcast<u32> %4
        %6:bool = lt %5, 10u
        if %6 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %7:u32 = load %idx
        %8:i32 = bitcast<i32> %7
        %9:u32 = bitcast<u32> %8
        %10:i32 = bitcast<i32> %9
        %11:u32 = bitcast<u32> 1u
        %12:i32 = bitcast<i32> %11
        %13:i32 = add %10, %12
        %14:u32 = bitcast<u32> %13
        store %idx, %14
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_TRUE(info->IsFinite());
    EXPECT_EQ(info->index_var, idx);
}

TEST_F(IR_LoopAnalysisTest, Finite_MultipleVars) {
    Var* idx = nullptr;
    Var* v1 = nullptr;
    Var* v2 = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            v1 = b.Var("v1", 0_u);
            idx = b.Var("idx", 0_u);
            v2 = b.Var("v2", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            auto* ifelse_v1 = b.If(b.LessThanEqual(b.Load(v1), u32::Highest()));
            b.Append(ifelse_v1->True(), [&] {  //
                b.ExitLoop(loop);
            });
            auto* ifelse_v2 = b.If(b.LessThan(b.Load(v1), 10_u));
            b.Append(ifelse_v2->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(v1, b.Add(b.Load(v1), 1_u));
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.Store(v2, b.Add(b.Load(v2), 0_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %v1:ptr<function, u32, read_write> = var 0u
        %idx:ptr<function, u32, read_write> = var 0u
        %v2:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %5:u32 = load %idx
        %6:bool = lt %5, 10u
        if %6 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        %7:u32 = load %v1
        %8:bool = lte %7, 4294967295u
        if %8 [t: $B6] {  # if_2
          $B6: {  # true
            exit_loop  # loop_1
          }
        }
        %9:u32 = load %v1
        %10:bool = lt %9, 10u
        if %10 [t: $B7] {  # if_3
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %11:u32 = load %v1
        %12:u32 = add %11, 1u
        store %v1, %12
        %13:u32 = load %idx
        %14:u32 = add %13, 1u
        store %idx, %14
        %15:u32 = load %v2
        %16:u32 = add %15, 0u
        store %v2, %16
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_TRUE(info->IsFinite());
    EXPECT_EQ(info->index_var, idx);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_MissingInc) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Load(idx));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        store %idx, %5
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_IncByZero) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 0_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        %6:u32 = add %5, 0u
        store %idx, %6
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_IncByTwo) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 2_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        %6:u32 = add %5, 2u
        store %idx, %6
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_IncByNonConstant) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* inc = b.FunctionParam("inc", ty.u32());
    func->AppendParam(inc);
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), inc));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%inc:u32):void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:u32 = load %idx
        %5:bool = lt %4, 10u
        if %5 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %6:u32 = load %idx
        %7:u32 = add %6, %inc
        store %idx, %7
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_EndBoundIsLetInBody) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* load = b.Load(idx);
            auto* let = b.Let("end", load);
            auto* ifelse = b.If(b.LessThan(load, let));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %end:u32 = let %3
        %5:bool = lt %3, %end
        if %5 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %6:u32 = load %idx
        %7:u32 = add %6, 1u
        store %idx, %7
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_OneMinusIndex) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 10_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.GreaterThan(b.Load(idx), 0_i));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Subtract(1_i, b.Load(idx)));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 10i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = gt %3, 0i
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:i32 = load %idx
        %6:i32 = sub 1i, %5
        store %idx, %6
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_NotAddOrSub) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Multiply(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        %6:u32 = mul %5, 1u
        store %idx, %6
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_NonInteger) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_f);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_f));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 1_f));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, f32, read_write> = var 0.0f
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:f32 = load %idx
        %4:bool = lt %3, 10.0f
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:f32 = load %idx
        %6:f32 = add %5, 1.0f
        store %idx, %6
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_MissingStore) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Add(b.Load(idx), 1_u);
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        %6:u32 = add %5, 1u
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_ContinueBeforeBreak) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                auto* nested_if = b.If(true);
                b.Append(nested_if->True(), [&] {  //
                    b.Continue(loop);
                });
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            if true [t: $B6] {  # if_2
              $B6: {  # true
                continue  # -> $B4
              }
            }
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        %6:u32 = add %5, 1u
        store %idx, %6
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_IncThenDec) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Subtract(b.Add(b.Load(idx), 1_u), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        %6:u32 = add %5, 1u
        %7:u32 = sub %6, 1u
        store %idx, %7
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_AnotherStoreInBody) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Store(idx, 0_u);
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        store %idx, 0u
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        %6:u32 = add %5, 1u
        store %idx, %6
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_MultipleStoresInContinuing) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.Store(idx, 0_u);
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        %6:u32 = add %5, 1u
        store %idx, %6
        store %idx, 0u
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_ConditionalStore) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            auto* inc = b.Add(b.Load(idx), 1_u);
            auto* cond_store = b.If(false);
            b.Append(cond_store->True(), [&] {  //
                b.Store(idx, inc);
                b.ExitIf(cond_store);
            });
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        %6:u32 = add %5, 1u
        if false [t: $B6] {  # if_2
          $B6: {  # true
            store %idx, %6
            exit_if  # if_2
          }
        }
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_ConditionalAdd) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            auto* load = b.Load(idx);
            auto* inc = b.InstructionResult<u32>();
            auto* cond_add = b.If(false);
            cond_add->SetResult(inc);
            b.Append(cond_add->True(), [&] {  //
                b.ExitIf(cond_add, b.Add(load, 1_u));
            });
            b.Append(cond_add->False(), [&] {  //
                b.ExitIf(cond_add, load);
            });
            b.Store(idx, inc);
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        %6:u32 = if false [t: $B6, f: $B7] {  # if_2
          $B6: {  # true
            %7:u32 = add %5, 1u
            exit_if %7  # if_2
          }
          $B7: {  # false
            exit_if %5  # if_2
          }
        }
        store %idx, %6
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_PassVarPointerToFunction) {
    auto* helper = b.Function("func", ty.void_());
    auto* param = b.FunctionParam<ptr<function, u32>>("param");
    helper->AppendParam(param);
    b.Append(helper->Block(), [&] {
        b.Store(param, 0_u);
        b.Return(helper);
    });

    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Call(helper, idx);
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:ptr<function, u32, read_write>):void {
  $B1: {
    store %param, 0u
    ret
  }
}
%func_1 = func():void {  # %func_1: 'func'
  $B2: {
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B4
      }
      $B4: {  # body
        %5:u32 = load %idx
        %6:bool = lt %5, 10u
        if %6 [t: $B6] {  # if_1
          $B6: {  # true
            exit_loop  # loop_1
          }
        }
        %7:void = call %func, %idx
        continue  # -> $B5
      }
      $B5: {  # continuing
        %8:u32 = load %idx
        %9:u32 = add %8, 1u
        store %idx, %9
        next_iteration  # -> $B4
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MaybeInfinite_PassVarPointerToFunction_ViaLet) {
    auto* helper = b.Function("func", ty.void_());
    auto* param = b.FunctionParam<ptr<function, u32>>("param");
    helper->AppendParam(param);
    b.Append(helper->Block(), [&] {
        b.Store(param, 0_u);
        b.Return(helper);
    });

    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            auto* let = b.Let("p", idx);
            b.Call(helper, let);
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:ptr<function, u32, read_write>):void {
  $B1: {
    store %param, 0u
    ret
  }
}
%func_1 = func():void {  # %func_1: 'func'
  $B2: {
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B4
      }
      $B4: {  # body
        %5:u32 = load %idx
        %6:bool = lt %5, 10u
        if %6 [t: $B6] {  # if_1
          $B6: {  # true
            exit_loop  # loop_1
          }
        }
        %p:ptr<function, u32, read_write> = let %idx
        %8:void = call %func, %p
        continue  # -> $B5
      }
      $B5: {  # continuing
        %9:u32 = load %idx
        %10:u32 = add %9, 1u
        store %idx, %10
        next_iteration  # -> $B4
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

// Note: This is actually a finite, but we only scan for candidate vars in the initializer block.
TEST_F(IR_LoopAnalysisTest, MaybeInfinite_VarOutsideInit) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        idx = b.Var("idx", 0_u);
        loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(b.LessThan(b.Load(idx), 10_u));
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    %idx:ptr<function, u32, read_write> = var 0u
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B4] {  # if_1
          $B4: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %5:u32 = load %idx
        %6:u32 = add %5, 1u
        store %idx, %6
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);
    auto* info = analysis.GetInfo(*loop);
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->IsFinite());
    EXPECT_EQ(info->index_var, nullptr);
}

TEST_F(IR_LoopAnalysisTest, MultipleLoops) {
    // We will create two pairs of nested loops:
    // loop outer_1 {   <- finite
    //   loop inner_1 { <- infinite
    //   }
    // }
    // loop outer_2 {   <- infinite
    //   loop inner_2 { <- finite
    //   }
    // }
    Var* idx_outer_1 = nullptr;
    Var* idx_inner_1 = nullptr;
    Var* idx_outer_2 = nullptr;
    Var* idx_inner_2 = nullptr;
    Loop* loop_outer_1 = nullptr;
    Loop* loop_inner_1 = nullptr;
    Loop* loop_outer_2 = nullptr;
    Loop* loop_inner_2 = nullptr;

    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop_outer_1 = b.Loop();
        b.Append(loop_outer_1->Initializer(), [&] {  //
            idx_outer_1 = b.Var("idx", 0_u);
            b.NextIteration(loop_outer_1);
        });
        b.Append(loop_outer_1->Body(), [&] {
            auto* ifelse_outer = b.If(b.LessThan(b.Load(idx_outer_1), 10_u));
            b.Append(ifelse_outer->True(), [&] {  //
                b.ExitLoop(loop_outer_1);
            });

            loop_inner_1 = b.Loop();
            b.Append(loop_inner_1->Initializer(), [&] {  //
                idx_inner_1 = b.Var("idx", 0_u);
                b.NextIteration(loop_inner_1);
            });
            b.Append(loop_inner_1->Body(), [&] {
                auto* ifelse_inner = b.If(b.LessThan(b.Load(idx_inner_1), 10_u));
                b.Append(ifelse_inner->True(), [&] {  //
                    b.ExitLoop(loop_inner_1);
                });
                b.Continue(loop_inner_1);
            });
            b.Append(loop_inner_1->Continuing(), [&] {  //
                b.NextIteration(loop_inner_1);
            });

            b.Continue(loop_outer_1);
        });
        b.Append(loop_outer_1->Continuing(), [&] {  //
            b.Store(idx_outer_1, b.Add(b.Load(idx_outer_1), 1_u));
            b.NextIteration(loop_outer_1);
        });

        loop_outer_2 = b.Loop();
        b.Append(loop_outer_2->Initializer(), [&] {  //
            idx_outer_2 = b.Var("idx", 0_u);
            b.NextIteration(loop_outer_2);
        });
        b.Append(loop_outer_2->Body(), [&] {
            auto* ifelse_outer = b.If(b.LessThan(b.Load(idx_outer_2), 10_u));
            b.Append(ifelse_outer->True(), [&] {  //
                b.ExitLoop(loop_outer_2);
            });

            loop_inner_2 = b.Loop();
            b.Append(loop_inner_2->Initializer(), [&] {  //
                idx_inner_2 = b.Var("idx", 0_u);
                b.NextIteration(loop_inner_2);
            });
            b.Append(loop_inner_2->Body(), [&] {
                auto* ifelse_inner = b.If(b.LessThan(b.Load(idx_inner_2), 10_u));
                b.Append(ifelse_inner->True(), [&] {  //
                    b.ExitLoop(loop_inner_2);
                });
                b.Continue(loop_inner_2);
            });
            b.Append(loop_inner_2->Continuing(), [&] {  //
                b.Store(idx_inner_2, b.Add(b.Load(idx_inner_2), 1_u));
                b.NextIteration(loop_inner_2);
            });

            b.Continue(loop_outer_2);
        });
        b.Append(loop_outer_2->Continuing(), [&] {  //
            b.NextIteration(loop_outer_2);
        });

        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        loop [i: $B6, b: $B7, c: $B8] {  # loop_2
          $B6: {  # initializer
            %idx_1:ptr<function, u32, read_write> = var 0u  # %idx_1: 'idx'
            next_iteration  # -> $B7
          }
          $B7: {  # body
            %6:u32 = load %idx_1
            %7:bool = lt %6, 10u
            if %7 [t: $B9] {  # if_2
              $B9: {  # true
                exit_loop  # loop_2
              }
            }
            continue  # -> $B8
          }
          $B8: {  # continuing
            next_iteration  # -> $B7
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %8:u32 = load %idx
        %9:u32 = add %8, 1u
        store %idx, %9
        next_iteration  # -> $B3
      }
    }
    loop [i: $B10, b: $B11, c: $B12] {  # loop_3
      $B10: {  # initializer
        %idx_2:ptr<function, u32, read_write> = var 0u  # %idx_2: 'idx'
        next_iteration  # -> $B11
      }
      $B11: {  # body
        %11:u32 = load %idx_2
        %12:bool = lt %11, 10u
        if %12 [t: $B13] {  # if_3
          $B13: {  # true
            exit_loop  # loop_3
          }
        }
        loop [i: $B14, b: $B15, c: $B16] {  # loop_4
          $B14: {  # initializer
            %idx_3:ptr<function, u32, read_write> = var 0u  # %idx_3: 'idx'
            next_iteration  # -> $B15
          }
          $B15: {  # body
            %14:u32 = load %idx_3
            %15:bool = lt %14, 10u
            if %15 [t: $B17] {  # if_4
              $B17: {  # true
                exit_loop  # loop_4
              }
            }
            continue  # -> $B16
          }
          $B16: {  # continuing
            %16:u32 = load %idx_3
            %17:u32 = add %16, 1u
            store %idx_3, %17
            next_iteration  # -> $B15
          }
        }
        continue  # -> $B12
      }
      $B12: {  # continuing
        next_iteration  # -> $B11
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    LoopAnalysis analysis(*func);

    auto* info_outer_1 = analysis.GetInfo(*loop_outer_1);
    ASSERT_NE(info_outer_1, nullptr);
    EXPECT_TRUE(info_outer_1->IsFinite());
    EXPECT_EQ(info_outer_1->index_var, idx_outer_1);

    auto* info_inner_1 = analysis.GetInfo(*loop_inner_1);
    ASSERT_NE(info_inner_1, nullptr);
    EXPECT_FALSE(info_inner_1->IsFinite());
    EXPECT_EQ(info_inner_1->index_var, nullptr);

    auto* info_outer_2 = analysis.GetInfo(*loop_outer_2);
    ASSERT_NE(info_outer_2, nullptr);
    EXPECT_FALSE(info_outer_2->IsFinite());
    EXPECT_EQ(info_outer_2->index_var, nullptr);

    auto* info_inner_2 = analysis.GetInfo(*loop_inner_2);
    ASSERT_NE(info_inner_2, nullptr);
    EXPECT_TRUE(info_inner_2->IsFinite());
    EXPECT_EQ(info_inner_2->index_var, idx_inner_2);
}

}  // namespace
}  // namespace tint::core::ir::analysis
