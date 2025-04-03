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
#include <utility>

#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"

namespace tint::core::ir::analysis {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_IntegerRangeAnalysisTest : public IRTestHelper {};

TEST_F(IR_IntegerRangeAnalysisTest, LocalInvocationIndex_u32_XYZ) {
    auto* func = b.ComputeFunction("my_func", 4_u, 3_u, 2_u);
    auto* localInvocationIndex = b.FunctionParam("localInvocationIndex", mod.Types().u32());
    localInvocationIndex->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationIndex);
    func->SetParams({localInvocationIndex});

    b.Append(func->Block(), [&] {
        auto* dst = b.Var(ty.ptr<function, array<u32, 24u>>());
        auto* access_dst = b.Access(ty.ptr<function, u32>(), dst, localInvocationIndex);
        b.Store(access_dst, localInvocationIndex);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(4u, 3u, 2u) func(%localInvocationIndex:u32 [@local_invocation_index]):void {
  $B1: {
    %3:ptr<function, array<u32, 24>, read_write> = var undef
    %4:ptr<function, u32, read_write> = access %3, %localInvocationIndex
    store %4, %localInvocationIndex
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    auto* info = analysis.GetInfo(localInvocationIndex);
    ASSERT_NE(nullptr, info);
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info->range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info->range);
    EXPECT_EQ(0u, range.min_bound);
    EXPECT_EQ(23u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, LocalInvocationIndex_i32_XYZ) {
    auto* func = b.ComputeFunction("my_func", 5_i, 4_i, 3_i);
    auto* localInvocationIndex = b.FunctionParam("localInvocationIndex", mod.Types().u32());
    localInvocationIndex->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationIndex);
    func->SetParams({localInvocationIndex});

    b.Append(func->Block(), [&] {
        auto* dst = b.Var(ty.ptr<function, array<u32, 60u>>());
        auto* access_dst = b.Access(ty.ptr<function, u32>(), dst, localInvocationIndex);
        b.Store(access_dst, localInvocationIndex);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(5i, 4i, 3i) func(%localInvocationIndex:u32 [@local_invocation_index]):void {
  $B1: {
    %3:ptr<function, array<u32, 60>, read_write> = var undef
    %4:ptr<function, u32, read_write> = access %3, %localInvocationIndex
    store %4, %localInvocationIndex
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    auto* info = analysis.GetInfo(localInvocationIndex);
    ASSERT_NE(nullptr, info);
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info->range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info->range);
    EXPECT_EQ(0u, range.min_bound);
    EXPECT_EQ(59u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, LocalInvocationIndex_1_Y_1) {
    auto* func = b.ComputeFunction("my_func", 1_u, 8_u, 1_u);
    auto* localInvocationIndex = b.FunctionParam("localInvocationIndex", mod.Types().u32());
    localInvocationIndex->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationIndex);
    func->SetParams({localInvocationIndex});

    b.Append(func->Block(), [&] {
        auto* dst = b.Var(ty.ptr<function, array<u32, 8u>>());
        auto* access_dst = b.Access(ty.ptr<function, u32>(), dst, localInvocationIndex);
        b.Store(access_dst, localInvocationIndex);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(1u, 8u, 1u) func(%localInvocationIndex:u32 [@local_invocation_index]):void {
  $B1: {
    %3:ptr<function, array<u32, 8>, read_write> = var undef
    %4:ptr<function, u32, read_write> = access %3, %localInvocationIndex
    store %4, %localInvocationIndex
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    auto* info = analysis.GetInfo(localInvocationIndex);
    ASSERT_NE(nullptr, info);
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info->range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info->range);
    EXPECT_EQ(0u, range.min_bound);
    EXPECT_EQ(7u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, LocalInvocationIndex_1_1_Z) {
    auto* func = b.ComputeFunction("my_func", 1_u, 1_u, 16_u);
    auto* localInvocationIndex = b.FunctionParam("localInvocationIndex", mod.Types().u32());
    localInvocationIndex->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationIndex);
    func->SetParams({localInvocationIndex});

    b.Append(func->Block(), [&] {
        auto* dst = b.Var(ty.ptr<function, array<u32, 16u>>());
        auto* access_dst = b.Access(ty.ptr<function, u32>(), dst, localInvocationIndex);
        b.Store(access_dst, localInvocationIndex);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(1u, 1u, 16u) func(%localInvocationIndex:u32 [@local_invocation_index]):void {
  $B1: {
    %3:ptr<function, array<u32, 16>, read_write> = var undef
    %4:ptr<function, u32, read_write> = access %3, %localInvocationIndex
    store %4, %localInvocationIndex
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    auto* info = analysis.GetInfo(localInvocationIndex);
    ASSERT_NE(nullptr, info);
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info->range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info->range);
    EXPECT_EQ(0u, range.min_bound);
    EXPECT_EQ(15u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, LocalInvocationID_u32_XYZ) {
    auto* func = b.ComputeFunction("my_func", 4_u, 3_u, 2_u);
    auto* localInvocationId = b.FunctionParam("localInvocationId", mod.Types().vec3<u32>());
    localInvocationId->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationId);
    func->SetParams({localInvocationId});

    b.Append(func->Block(), [&] {
        auto* dst_x = b.Var(ty.ptr<function, array<u32, 4u>>());
        auto* access_src_x = b.Access(ty.u32(), localInvocationId, 0_u);
        auto* access_dst_x = b.Access(ty.ptr<function, u32>(), dst_x, access_src_x);
        b.Store(access_dst_x, access_src_x);
        auto* dst_y = b.Var(ty.ptr<function, array<u32, 3u>>());
        auto* access_src_y = b.Access(ty.u32(), localInvocationId, 1_u);
        auto* access_dst_y = b.Access(ty.ptr<function, u32>(), dst_y, access_src_y);
        b.Store(access_dst_y, access_src_y);
        auto* dst_z = b.Var(ty.ptr<function, array<u32, 2u>>());
        auto* access_src_z = b.Access(ty.u32(), localInvocationId, 2_u);
        auto* access_dst_z = b.Access(ty.ptr<function, u32>(), dst_z, access_src_z);
        b.Store(access_dst_z, access_src_z);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(4u, 3u, 2u) func(%localInvocationId:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %3:ptr<function, array<u32, 4>, read_write> = var undef
    %4:u32 = access %localInvocationId, 0u
    %5:ptr<function, u32, read_write> = access %3, %4
    store %5, %4
    %6:ptr<function, array<u32, 3>, read_write> = var undef
    %7:u32 = access %localInvocationId, 1u
    %8:ptr<function, u32, read_write> = access %6, %7
    store %8, %7
    %9:ptr<function, array<u32, 2>, read_write> = var undef
    %10:u32 = access %localInvocationId, 2u
    %11:ptr<function, u32, read_write> = access %9, %10
    store %11, %10
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);

    std::array<uint32_t, 3> expected_max_bounds = {3u, 2u, 1u};
    for (uint32_t i = 0; i < expected_max_bounds.size(); ++i) {
        auto* info = analysis.GetInfo(localInvocationId, i);

        ASSERT_NE(nullptr, info);
        ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info->range));

        const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info->range);
        EXPECT_EQ(0u, range.min_bound);
        EXPECT_EQ(expected_max_bounds[i], range.max_bound);
    }
}

TEST_F(IR_IntegerRangeAnalysisTest, LocalInvocationID_u32_1_Y_1) {
    auto* func = b.ComputeFunction("my_func", 1_u, 8_u, 1_u);
    auto* localInvocationId = b.FunctionParam("localInvocationId", mod.Types().vec3<u32>());
    localInvocationId->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationId);
    func->SetParams({localInvocationId});

    b.Append(func->Block(), [&] {
        auto* dst = b.Var(ty.ptr<function, array<u32, 8u>>());
        auto* access_src = b.Access(ty.u32(), localInvocationId, 1_u);
        auto* access_dst = b.Access(ty.ptr<function, u32>(), dst, access_src);
        b.Store(access_dst, access_src);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(1u, 8u, 1u) func(%localInvocationId:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %3:ptr<function, array<u32, 8>, read_write> = var undef
    %4:u32 = access %localInvocationId, 1u
    %5:ptr<function, u32, read_write> = access %3, %4
    store %5, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);

    std::array<uint32_t, 3> expected_max_bounds = {0u, 7u, 0u};
    for (uint32_t i = 0; i < expected_max_bounds.size(); ++i) {
        auto* info = analysis.GetInfo(localInvocationId, i);

        ASSERT_NE(nullptr, info);
        ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info->range));

        const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info->range);
        EXPECT_EQ(0u, range.min_bound);
        EXPECT_EQ(expected_max_bounds[i], range.max_bound);
    }
}

TEST_F(IR_IntegerRangeAnalysisTest, LocalInvocationID_u32_1_1_Z) {
    auto* func = b.ComputeFunction("my_func", 1_u, 1_u, 16_u);
    auto* localInvocationId = b.FunctionParam("localInvocationId", mod.Types().vec3<u32>());
    localInvocationId->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationId);
    func->SetParams({localInvocationId});

    b.Append(func->Block(), [&] {
        auto* dst = b.Var(ty.ptr<function, array<u32, 16u>>());
        auto* access_src = b.Access(ty.u32(), localInvocationId, 2_u);
        auto* access_dst = b.Access(ty.ptr<function, u32>(), dst, access_src);
        b.Store(access_dst, access_src);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(1u, 1u, 16u) func(%localInvocationId:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %3:ptr<function, array<u32, 16>, read_write> = var undef
    %4:u32 = access %localInvocationId, 2u
    %5:ptr<function, u32, read_write> = access %3, %4
    store %5, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);

    std::array<uint32_t, 3> expected_max_bounds = {0u, 0u, 15u};
    for (uint32_t i = 0; i < expected_max_bounds.size(); ++i) {
        auto* info = analysis.GetInfo(localInvocationId, i);

        ASSERT_NE(nullptr, info);
        ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info->range));

        const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info->range);
        EXPECT_EQ(0u, range.min_bound);
        EXPECT_EQ(expected_max_bounds[i], range.max_bound);
    }
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopInitializer_Success_sint) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 5_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 5i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopInitializer_Success_uint) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 3_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 3u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopInitializer_Failure_MissingInitializerBlock) {
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(nullptr, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopInitializer_Failure_MissingVarInitializer) {
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] { b.NextIteration(loop); });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(nullptr, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopInitializer_Failure_NotInitializedWithInteger) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 1_f);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, f32, read_write> = var 1.0f
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(nullptr, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopInitializer_Failure_NotInitializedWithConstValue) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* end = b.FunctionParam("end", ty.u32());
    auto* func = b.Function("func", ty.void_());
    func->AppendParam(end);
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", end);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%end:u32):void {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var %end
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(nullptr, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopInitializer_Failure_InitializerTooComplex) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            auto* load1 = b.Load(idx);
            auto* load2 = b.Load(idx);
            auto* x = b.Let("x", b.Add<u32>(load1, load2));
            b.Store(idx, x);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        %3:u32 = load %idx
        %4:u32 = load %idx
        %5:u32 = add %3, %4
        %x:u32 = let %5
        store %idx, %x
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(nullptr, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Success_AddOne_sint) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            binary = b.Add<i32>(b.Load(idx), 1_i);
            b.Store(idx, binary);
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %3:i32 = load %idx
        %4:i32 = add %3, 1i
        store %idx, %4
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Success_AddOne_uint) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            binary = b.Add<u32>(b.Load(idx), 1_u);
            b.Store(idx, binary);
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
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %3:u32 = load %idx
        %4:u32 = add %3, 1u
        store %idx, %4
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Success_OneAddLoopControlVariable_sint) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            binary = b.Add<i32>(1_i, b.Load(idx));
            b.Store(idx, binary);
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %3:i32 = load %idx
        %4:i32 = add 1i, %3
        store %idx, %4
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Success_OneAddLoopControlVariable_uint) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            binary = b.Add<u32>(1_u, b.Load(idx));
            b.Store(idx, binary);
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
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %3:u32 = load %idx
        %4:u32 = add 1u, %3
        store %idx, %4
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Success_MinusOne_sint) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            binary = b.Subtract<i32>(b.Load(idx), 1_i);
            b.Store(idx, binary);
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %3:i32 = load %idx
        %4:i32 = sub %3, 1i
        store %idx, %4
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Success_MinusOne_uint) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            binary = b.Subtract<u32>(b.Load(idx), 1_u);
            b.Store(idx, binary);
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
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %3:u32 = load %idx
        %4:u32 = sub %3, 1u
        store %idx, %4
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Failure_TooFewInstructions) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
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
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %3:u32 = load %idx
        store %idx, %3
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Failure_TooManyInstructions) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            auto* addx = b.Add<u32>(b.Load(idx), 1_u);
            auto* minusx = b.Subtract<u32>(addx, 1_u);
            b.Store(idx, minusx);
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
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %3:u32 = load %idx
        %4:u32 = add %3, 1u
        %5:u32 = sub %4, 1u
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Failure_FirstInstructionIsNotLoad) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            Var* idy = b.Var("idy", 1_u);
            b.Store(idx, b.Load(idy));
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
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %idy:ptr<function, u32, read_write> = var 1u
        %4:u32 = load %idy
        store %idx, %4
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Failure_NoLoadFromLoopControlVariable) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Var* idy = nullptr;
    auto* end = b.FunctionParam("end", ty.u32());
    auto* func = b.Function("func", ty.void_());
    func->AppendParam(end);
    b.Append(func->Block(), [&] {
        idy = b.Var("idy", end);
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            // idx = idy + 1
            b.Store(idx, b.Add<u32>(b.Load(idy), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%end:u32):void {
  $B1: {
    %idy:ptr<function, u32, read_write> = var %end
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %5:u32 = load %idy
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(nullptr,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Failure_SecondInstructionNotABinaryOp) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            // idx = bitcast<u32>(idx);
            b.Store(idx, b.Bitcast<u32>(b.Load(idx)));
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
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %3:u32 = load %idx
        %4:u32 = bitcast %3
        store %idx, %4
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Failure_AddTwo) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            // idx += 2u;
            b.Store(idx, b.Add<u32>(b.Load(idx), 2_u));
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
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %3:u32 = load %idx
        %4:u32 = add %3, 2u
        store %idx, %4
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Failure_MinusTwo) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            // idx -= 2u
            b.Store(idx, b.Subtract<u32>(b.Load(idx), 2_u));
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
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %3:u32 = load %idx
        %4:u32 = sub %3, 2u
        store %idx, %4
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Failure_OneMinusLoopControlVariable) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            // idx = 1u - idx
            b.Store(idx, b.Subtract<u32>(1_u, b.Load(idx)));
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
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %3:u32 = load %idx
        %4:u32 = sub 1u, %3
        store %idx, %4
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Failure_AddNonConstantValue) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* end = b.FunctionParam("end", ty.u32());
    auto* func = b.Function("func", ty.void_());
    func->AppendParam(end);
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            // idx += end
            b.Store(idx, b.Add<u32>(b.Load(idx), end));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%end:u32):void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %4:u32 = load %idx
        %5:u32 = add %4, %end
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Failure_MinusNonConstantValue) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* end = b.FunctionParam("end", ty.u32());
    auto* func = b.Function("func", ty.void_());
    func->AppendParam(end);
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            // idx -= end
            b.Store(idx, b.Subtract<u32>(b.Load(idx), end));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%end:u32):void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %4:u32 = load %idx
        %5:u32 = sub %4, %end
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Failure_NeitherAddNorMinus) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            // idx << 1ui;
            b.Store(idx, b.ShiftLeft<u32>(b.Load(idx), 1_u));
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
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %3:u32 = load %idx
        %4:u32 = shl %3, 1u
        store %idx, %4
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Failure_NoStoreFromLoopControlVariable) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Var* idy = nullptr;
    auto* end = b.FunctionParam("end", ty.u32());
    auto* func = b.Function("func", ty.void_());
    func->AppendParam(end);
    b.Append(func->Block(), [&] {
        idy = b.Var("idy", end);
        auto* loady = b.Load(idy);
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            b.Add<u32>(b.Load(idx), 1_u);
            // idx = idy
            b.Store(idx, loady);
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%end:u32):void {
  $B1: {
    %idy:ptr<function, u32, read_write> = var %end
    %4:u32 = load %idy
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %6:u32 = load %idx
        %7:u32 = add %6, 1u
        store %idx, %4
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopContinuing_Failure_NoStoreToLoopControlVariable) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Var* idy = nullptr;
    auto* end = b.FunctionParam("end", ty.u32());
    auto* func = b.Function("func", ty.void_());
    func->AppendParam(end);
    b.Append(func->Block(), [&] {
        idy = b.Var("idy", end);
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] {
            // idy = idx + 1
            b.Store(idy, b.Add<u32>(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%end:u32):void {
  $B1: {
    %idy:ptr<function, u32, read_write> = var %end
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        %6:u32 = add %5, 1u
        store %idy, %6
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_index_LessThan_Constant) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 10i
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
        %5:i32 = load %idx
        %6:i32 = add %5, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_Constant_LessThan_index) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 20_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 10 < idx
            binary = b.LessThan<bool>(10_i, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Subtract<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 20i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt 10i, %3
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
        %5:i32 = load %idx
        %6:i32 = sub %5, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_index_LessThan_Constant_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10u
            binary = b.LessThan<bool>(b.Load(idx), 10_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<u32>(b.Load(idx), 1_u));
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_Constant_LessThan_index_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 10u < idx
            binary = b.LessThan<bool>(10_u, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Subtract<u32>(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 20u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt 10u, %3
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_index_LessThanEqual_constant) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx <= 10
            binary = b.LessThanEqual<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lte %3, 10i
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
        %5:i32 = load %idx
        %6:i32 = add %5, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_constant_LessThanEqual_index) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 20_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 10 <= idx
            binary = b.LessThanEqual<bool>(10_i, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Subtract<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 20i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lte 10i, %3
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
        %5:i32 = load %idx
        %6:i32 = sub %5, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_index_GreaterThan_constant) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx > 20
            idx = b.Var("idx", 20_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            binary = b.GreaterThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Subtract<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 20i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = gt %3, 10i
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
        %5:i32 = load %idx
        %6:i32 = sub %5, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_constant_GreaterThan_index) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 10 > idx
            binary = b.GreaterThan<bool>(10_i, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = gt 10i, %3
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
        %5:i32 = load %idx
        %6:i32 = add %5, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_index_GreaterThanEqual_Constant) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 20_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx >= 10
            binary = b.GreaterThanEqual<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Subtract<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 20i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = gte %3, 10i
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
        %5:i32 = load %idx
        %6:i32 = sub %5, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_Constant_GreaterThanEqual_index) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 10 >= idx
            binary = b.GreaterThanEqual<bool>(10_i, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = gte 10i, %3
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
        %5:i32 = load %idx
        %6:i32 = add %5, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_InstructionsOtherThanLoadOnIndex) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* end = b.FunctionParam("end", ty.i32());
    auto* func = b.Function("func", ty.void_());
    func->AppendParam(end);
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 20_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            binary = b.GreaterThanEqual<bool>(10_i, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Store(idx, end);
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Subtract<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%end:i32):void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 20i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:i32 = load %idx
        %5:bool = gte 10i, %4
        if %5 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        store %idx, %end
        continue  # -> $B4
      }
      $B4: {  # continuing
        %6:i32 = load %idx
        %7:i32 = sub %6, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_FirstInstructionIsNotLoad) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // Initialize idy to 1
            Var* idy = b.Var("idy", 1_i);
            binary = b.LessThan<bool>(b.Load(idy), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %idy:ptr<function, i32, read_write> = var 1i
        %4:i32 = load %idy
        %5:bool = lt %4, 10i
        if %5 [t: $B5, f: $B6] {  # if_1
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
        %6:i32 = load %idx
        %7:i32 = add %6, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_NoLoadFromLoopControlVariable) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        Var* idy = b.Var("idy", 1_i);
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idy < 10
            binary = b.LessThan<bool>(b.Load(idy), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    %idy:ptr<function, i32, read_write> = var 1i
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:i32 = load %idy
        %5:bool = lt %4, 10i
        if %5 [t: $B5, f: $B6] {  # if_1
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
        %6:i32 = load %idx
        %7:i32 = add %6, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_StoreLoopControlVariableInInnerBlock) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        Var* idy = b.Var("idy", 1_i);
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            auto* ifelse_y = b.If(b.LessThan<bool>(b.Load(idy), 10_i));
            b.Append(ifelse_y->True(), [&] {
                b.Store(idx, 2_i);
                b.ExitIf(ifelse_y);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    %idy:ptr<function, i32, read_write> = var 1i
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:i32 = load %idx
        %5:bool = lt %4, 10i
        if %5 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %6:i32 = load %idy
        %7:bool = lt %6, 10i
        if %7 [t: $B7] {  # if_2
          $B7: {  # true
            store %idx, 2i
            exit_if  # if_2
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %8:i32 = load %idx
        %9:i32 = add %8, 1i
        store %idx, %9
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_SecondInstructionNotABinaryOp) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // bitcastX = bitcast<i32>(idx)
            auto* bitcastX = b.Bitcast<i32>(b.Load(idx));
            binary = b.LessThan<bool>(bitcastX, 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:i32 = bitcast %3
        %5:bool = lt %4, 10i
        if %5 [t: $B5, f: $B6] {  # if_1
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
        %6:i32 = load %idx
        %7:i32 = add %6, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_BinaryOpIsNotComparisonOp) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // shl = idx << 1
            auto* shl = b.ShiftLeft<i32>(b.Load(idx), 1_u);
            binary = b.LessThan<bool>(10_i, shl);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:i32 = shl %3, 1u
        %5:bool = lt 10i, %4
        if %5 [t: $B5, f: $B6] {  # if_1
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
        %6:i32 = load %idx
        %7:i32 = add %6, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_IndexCompareWithNonConst) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* end = b.FunctionParam("end", ty.i32());
    auto* func = b.Function("func", ty.void_());
    func->AppendParam(end);
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < end
            binary = b.LessThan<bool>(b.Load(idx), end);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%end:i32):void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:i32 = load %idx
        %5:bool = lt %4, %end
        if %5 [t: $B5, f: $B6] {  # if_1
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
        %6:i32 = load %idx
        %7:i32 = add %6, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_NonConstCompareWithIndex) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* end = b.FunctionParam("end", ty.i32());
    auto* func = b.Function("func", ty.void_());
    func->AppendParam(end);
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 20_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // end > idx
            binary = b.GreaterThan<bool>(end, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Subtract<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%end:i32):void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 20i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:i32 = load %idx
        %5:bool = gt %end, %4
        if %5 [t: $B5, f: $B6] {  # if_1
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
        %6:i32 = load %idx
        %7:i32 = sub %6, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_NonIndexCompareWithConst) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* end = b.FunctionParam("end", ty.i32());
    auto* func = b.Function("func", ty.void_());
    func->AppendParam(end);
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            b.Load(idx);
            // end < 10
            binary = b.LessThan<bool>(end, 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%end:i32):void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:i32 = load %idx
        %5:bool = lt %end, 10i
        if %5 [t: $B5, f: $B6] {  # if_1
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
        %6:i32 = load %idx
        %7:i32 = add %6, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_ConstCompareWithNonIndex) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* end = b.FunctionParam("end", ty.i32());
    auto* func = b.Function("func", ty.void_());
    func->AppendParam(end);
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 20_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            b.Load(idx);
            // 10 > end
            binary = b.GreaterThan<bool>(10_i, end);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Subtract<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%end:i32):void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 20i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:i32 = load %idx
        %5:bool = gt 10i, %end
        if %5 [t: $B5, f: $B6] {  # if_1
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
        %6:i32 = load %idx
        %7:i32 = sub %6, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_Index_LessThan_Zero_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 0u
            binary = b.LessThan<bool>(b.Load(idx), 0_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<u32>(b.Load(idx), 1_u));
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
        %4:bool = lt %3, 0u
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_Zero_GreaterThan_Index_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 0u > idx
            binary = b.GreaterThan<bool>(0_u, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<u32>(b.Load(idx), 1_u));
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
        %4:bool = gt 0u, %3
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_Index_GreaterThan_Max_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx > 4294967295u (maximum uint32_t value)
            binary = b.GreaterThan<bool>(b.Load(idx), u32(std::numeric_limits<uint32_t>::max()));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<u32>(b.Load(idx), 1_u));
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
        %4:bool = gt %3, 4294967295u
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_Max_u32_LessThan_Index) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 4294967295u (maximum uint32_t value) < idx
            binary = b.LessThan<bool>(u32(std::numeric_limits<uint32_t>::max()), b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<u32>(b.Load(idx), 1_u));
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
        %4:bool = lt 4294967295u, %3
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_Index_LessThan_Min_I32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < -2147483648 (minimum int32_t value)
            binary = b.LessThan<bool>(b.Load(idx), i32(std::numeric_limits<int32_t>::min()));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Subtract<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, -2147483648i
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
        %5:i32 = load %idx
        %6:i32 = sub %5, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_Min_I32_GreaterThan_Index) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // -2147483648 (minimum int32_t value) > idx
            binary = b.GreaterThan<bool>(i32(std::numeric_limits<int32_t>::min()), b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Subtract<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = gt -2147483648i, %3
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
        %5:i32 = load %idx
        %6:i32 = sub %5, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_Index_GreaterThan_Max_I32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx > 2147483647 (maximum int32_t value)
            binary = b.GreaterThan<bool>(b.Load(idx), i32(std::numeric_limits<int32_t>::max()));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Subtract<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = gt %3, 2147483647i
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
        %5:i32 = load %idx
        %6:i32 = sub %5, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_Max_I32_LessThan_Index) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 2147483647i (maximum int32_t value) < idx
            binary = b.LessThan<bool>(i32(std::numeric_limits<int32_t>::max()), b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Subtract<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt 2147483647i, %3
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
        %5:i32 = load %idx
        %6:i32 = sub %5, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_NotIfElseAfterCompareInstruction) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            binary = b.LessThan<bool>(b.Load(idx), 10_i);
            // Now idx can be 10 before the if-statement
            b.Add<i32>(b.Load(idx), 1_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 10i
        %5:i32 = load %idx
        %6:i32 = add %5, 1i
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
        %7:i32 = load %idx
        %8:i32 = add %7, 1i
        store %idx, %8
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_NotUseLastComparisonAsIfCondition) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* param = b.FunctionParam("param", ty.i32());
    auto* func = b.Function("func", ty.void_());
    func->AppendParam(param);
    b.Append(func->Block(), [&] {
        Binary* binaryOnParam = b.LessThan<bool>(param, 30_i);
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binaryOnParam);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    %3:bool = lt %param, 30i
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %5:i32 = load %idx
        %6:bool = lt %5, 10i
        if %3 [t: $B5, f: $B6] {  # if_1
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
        %7:i32 = load %idx
        %8:i32 = add %7, 1i
        store %idx, %8
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_MultipleInstructionsInTrueBlock) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] {
                // Now idx < 10
                b.Add<i32>(b.Load(idx), 30_i);
                b.ExitIf(ifelse);
            });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 10i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            %5:i32 = load %idx
            %6:i32 = add %5, 30i
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %7:i32 = load %idx
        %8:i32 = add %7, 1i
        store %idx, %8
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_ExitLoopInTrueBlock) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitLoop(loop); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 10i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:i32 = load %idx
        %6:i32 = add %5, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_TooManyInstructionsInFalseBlock) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] {
                // Now idx can be 10
                b.Load(idx);
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 10i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            %5:i32 = load %idx
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %6:i32 = load %idx
        %7:i32 = add %6, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_NoExitLoopInFalseBlock) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 10i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_if  # if_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:i32 = load %idx
        %6:i32 = add %5, 1i
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

    IntegerRangeAnalysis analysis(func);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

}  // namespace
}  // namespace tint::core::ir::analysis
