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

    IntegerRangeAnalysis analysis(&mod);
    const IntegerRangeInfo& info = analysis.GetInfo(localInvocationIndex);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
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

    IntegerRangeAnalysis analysis(&mod);
    const IntegerRangeInfo& info = analysis.GetInfo(localInvocationIndex);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
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

    IntegerRangeAnalysis analysis(&mod);
    const IntegerRangeInfo& info = analysis.GetInfo(localInvocationIndex);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
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

    IntegerRangeAnalysis analysis(&mod);
    const IntegerRangeInfo& info = analysis.GetInfo(localInvocationIndex);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
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

    IntegerRangeAnalysis analysis(&mod);

    std::array<uint32_t, 3> expected_max_bounds = {3u, 2u, 1u};
    for (uint32_t i = 0; i < expected_max_bounds.size(); ++i) {
        const IntegerRangeInfo& info = analysis.GetInfo(localInvocationId, i);

        ASSERT_TRUE(info.IsValid());
        ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

        const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
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

    IntegerRangeAnalysis analysis(&mod);

    std::array<uint32_t, 3> expected_max_bounds = {0u, 7u, 0u};
    for (uint32_t i = 0; i < expected_max_bounds.size(); ++i) {
        const IntegerRangeInfo& info = analysis.GetInfo(localInvocationId, i);

        ASSERT_TRUE(info.IsValid());
        ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

        const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
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

    IntegerRangeAnalysis analysis(&mod);

    std::array<uint32_t, 3> expected_max_bounds = {0u, 0u, 15u};
    for (uint32_t i = 0; i < expected_max_bounds.size(); ++i) {
        const IntegerRangeInfo& info = analysis.GetInfo(localInvocationId, i);

        ASSERT_TRUE(info.IsValid());
        ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

        const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr,
              analysis.GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_index_LessThan_Constant_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
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
            // idx++
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(0, range.min_bound);
    EXPECT_EQ(9, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_Constant_LessThan_index_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(11, range.min_bound);
    EXPECT_EQ(20, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_index_LessThan_Constant_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 1u
            idx = b.Var("idx", 1_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 20u
            binary = b.LessThan<bool>(b.Load(idx), 20_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 1u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 20u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(1u, range.min_bound);
    EXPECT_EQ(19u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_Constant_LessThan_index_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(11u, range.min_bound);
    EXPECT_EQ(20u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_index_LessThanEqual_constant_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 1
            idx = b.Var("idx", 1_i);
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
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 1i
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(1, range.min_bound);
    EXPECT_EQ(10, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_index_LessThanEqual_constant_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 1
            idx = b.Var("idx", 1_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx <= 10
            binary = b.LessThanEqual<bool>(b.Load(idx), 10_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 1u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lte %3, 10u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(1u, range.min_bound);
    EXPECT_EQ(10u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_constant_LessThanEqual_index_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(10, range.min_bound);
    EXPECT_EQ(20, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_constant_LessThanEqual_index_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 10 <= idx
            binary = b.LessThanEqual<bool>(10_u, b.Load(idx));
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
        %4:bool = lte 10u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(10u, range.min_bound);
    EXPECT_EQ(20u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_index_GreaterThan_constant_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx > 10
            binary = b.GreaterThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(11, range.min_bound);
    EXPECT_EQ(20, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_index_GreaterThan_constant_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20u
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx > 10u
            binary = b.GreaterThan<bool>(b.Load(idx), 10_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(11u, range.min_bound);
    EXPECT_EQ(20u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_constant_GreaterThan_index_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
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
            // idx++
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(0, range.min_bound);
    EXPECT_EQ(9, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_constant_GreaterThan_index_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0u
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 10u > idx
            binary = b.GreaterThan<bool>(10_u, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %4:bool = gt 10u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(0u, range.min_bound);
    EXPECT_EQ(9u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_index_GreaterThanEqual_Constant_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
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
            // idx--
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(10, range.min_bound);
    EXPECT_EQ(20, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_index_GreaterThanEqual_Constant_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx >= 10
            binary = b.GreaterThanEqual<bool>(b.Load(idx), 10_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %4:bool = gte %3, 10u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(10u, range.min_bound);
    EXPECT_EQ(20u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_Constant_GreaterThanEqual_index_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
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
            // idx++
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(0, range.min_bound);
    EXPECT_EQ(10, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_Constant_GreaterThanEqual_index_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0u
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 10u >= idx
            binary = b.GreaterThanEqual<bool>(10_u, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %4:bool = gte 10u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(0u, range.min_bound);
    EXPECT_EQ(10u, range.max_bound);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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
            binary = b.GreaterThan<bool>(b.Load(idx), u32::Highest());
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

    IntegerRangeAnalysis analysis(&mod);
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
            binary = b.LessThan<bool>(u32::Highest(), b.Load(idx));
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

    IntegerRangeAnalysis analysis(&mod);
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
            binary = b.LessThan<bool>(b.Load(idx), i32::Lowest());
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

    IntegerRangeAnalysis analysis(&mod);
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
            binary = b.GreaterThan<bool>(i32::Lowest(), b.Load(idx));
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

    IntegerRangeAnalysis analysis(&mod);
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
            binary = b.GreaterThan<bool>(b.Load(idx), i32::Highest());
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

    IntegerRangeAnalysis analysis(&mod);
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
            binary = b.LessThan<bool>(i32::Highest(), b.Load(idx));
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_Index_LessThanEqual_Max_u32) {
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
            // idx <= 4294967295u (maximum uint32_t value)
            binary = b.LessThanEqual<bool>(b.Load(idx), u32::Highest());
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
        %4:bool = lte %3, 4294967295u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_Index_LessThanEqual_Max_i32) {
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
            // idx <= 2147483647 (maximum int32_t value)
            binary = b.LessThanEqual<bool>(b.Load(idx), i32::Highest());
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
        %4:bool = lte %3, 2147483647i
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_Min_i32_LessThanEqual_Index) {
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
            // -2147483648 (minimum int32_t value) <= idx
            binary = b.LessThanEqual<bool>(i32::Lowest(), b.Load(idx));
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
        %4:bool = lte -2147483648i, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_Min_u32_LessThanEqual_Index) {
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
            // 0 (minimum uint32_t value) <= idx
            binary = b.LessThanEqual<bool>(u32::Lowest(), b.Load(idx));
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
        %4:bool = lte 0u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_Index_GreaterThanEqual_Min_u32) {
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
            // idx >= 0u (minimum uint32_t value)
            binary = b.GreaterThanEqual<bool>(b.Load(idx), u32::Lowest());
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
        %4:bool = gte %3, 0u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_Index_GreaterThanEqual_Min_i32) {
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
            // idx >= -2147483648 (minimum int32_t value)
            binary = b.GreaterThanEqual<bool>(b.Load(idx), i32::Lowest());
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
        %4:bool = gte %3, -2147483648i
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_Max_i32_GreaterThanEqual_Index) {
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
            // 2147483647 (maximum int32_t value) >= idx
            binary = b.GreaterThanEqual<bool>(i32::Highest(), b.Load(idx));
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
        %4:bool = gte 2147483647i, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_Max_u32_GreaterThanEqual_Index) {
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
            // 4294967295 (maximum uint32_t value) >= idx
            binary = b.GreaterThanEqual<bool>(u32::Highest(), b.Load(idx));
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
        %4:bool = gte 4294967295u, %3
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_EmptyFalseBlock) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.ExitLoop(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
        }
        exit_loop  # loop_1
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(nullptr, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Success_LessThanEqual_init_equals_rhs_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 10
            idx = b.Var("idx", 10_i);
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
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 10i
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));

    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());

    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(10, range.min_bound);
    EXPECT_EQ(10, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Success_LessThanEqual_init_equals_rhs_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 10
            idx = b.Var("idx", 10_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx <= 10
            binary = b.LessThanEqual<bool>(b.Load(idx), 10_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 10u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lte %3, 10u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));

    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());

    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(10u, range.min_bound);
    EXPECT_EQ(10u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Failure_LessThanEqual_init_greater_than_rhs_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 10
            idx = b.Var("idx", 10_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx <= 1
            binary = b.LessThanEqual<bool>(b.Load(idx), 1_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 10i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lte %3, 1i
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));

    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Failure_LessThanEqual_init_greater_than_rhs_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 10
            idx = b.Var("idx", 10_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx <= 1
            binary = b.LessThanEqual<bool>(b.Load(idx), 1_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 10u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lte %3, 1u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));

    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Failure_index_LessThanEqual_constant_decreasing) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 1
            idx = b.Var("idx", 1_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx <= 10
            binary = b.LessThanEqual<bool>(b.Load(idx), 10_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %idx:ptr<function, u32, read_write> = var 1u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lte %3, 10u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));

    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Success_GreaterThanEqual_init_equals_rhs_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx >= 20
            binary = b.GreaterThanEqual<bool>(b.Load(idx), 20_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %4:bool = gte %3, 20i
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(20, range.min_bound);
    EXPECT_EQ(20, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Success_GreaterThanEqual_init_equals_rhs_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx >= 20
            binary = b.GreaterThanEqual<bool>(b.Load(idx), 20_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %4:bool = gte %3, 20u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(20u, range.min_bound);
    EXPECT_EQ(20u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Failure_GreaterThanEqual_init_less_than_rhs_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 10
            idx = b.Var("idx", 10_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx >= 20
            binary = b.GreaterThanEqual<bool>(b.Load(idx), 20_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %idx:ptr<function, i32, read_write> = var 10i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = gte %3, 20i
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Failure_GreaterThanEqual_init_less_than_rhs_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 10
            idx = b.Var("idx", 10_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx >= 20
            binary = b.GreaterThanEqual<bool>(b.Load(idx), 20_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %idx:ptr<function, u32, read_write> = var 10u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = gte %3, 20u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest,
       AnalyzeLoop_Failure_index_GreaterThanEqual_constant_increasing) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx >= 10
            binary = b.GreaterThanEqual<bool>(b.Load(idx), 10_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 20u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = gte %3, 10u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Success_index_LessThanEqual_max_u32_minus_1) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4294967294u (maximum uint32_t value - 1)
            idx = b.Var("idx", u32(u32::kHighestValue - 1u));
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx <= 4294967294u
            binary = b.LessThanEqual<bool>(b.Load(idx), u32(u32::kHighestValue - 1u));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 4294967294u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lte %3, 4294967294u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(u32::kHighestValue - 1u, range.min_bound);
    EXPECT_EQ(u32::kHighestValue - 1u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Success_index_LessThanEqual_max_i32_minus_1) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 2147483646 (maximum int32_t value - 1)
            idx = b.Var("idx", i32(i32::kHighestValue - 1));
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx <= 2147483646
            binary = b.LessThanEqual<bool>(b.Load(idx), i32(i32::kHighestValue - 1));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 2147483646i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lte %3, 2147483646i
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(i32::kHighestValue - 1, range.min_bound);
    EXPECT_EQ(i32::kHighestValue - 1, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Success_index_GreaterThanEqual_min_u32_add_1) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 1u (minimum uint32_t value + 1)
            idx = b.Var("idx", u32(u32::kLowestValue + 1u));
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx >= 1u
            binary = b.GreaterThanEqual<bool>(b.Load(idx), u32(u32::kLowestValue + 1u));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %idx:ptr<function, u32, read_write> = var 1u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = gte %3, 1u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(u32::kLowestValue + 1u, range.min_bound);
    EXPECT_EQ(u32::kLowestValue + 1u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Success_index_GreaterThanEqual_min_i32_add_1) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = -2147483647 (minimum int32_t value + 1)
            idx = b.Var("idx", i32(i32::kLowestValue + 1));
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx >= -2147483647
            binary = b.GreaterThanEqual<bool>(b.Load(idx), i32(i32::kLowestValue + 1));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %idx:ptr<function, i32, read_write> = var -2147483647i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = gte %3, -2147483647i
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(i32::kLowestValue + 1, range.min_bound);
    EXPECT_EQ(i32::kLowestValue + 1, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Failure_LessThan_init_greater_than_rhs_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 10
            idx = b.Var("idx", 10_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 1
            binary = b.LessThan<bool>(b.Load(idx), 1_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 10i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 1i
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));

    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Failure_LessThan_init_equals_rhs_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 10
            idx = b.Var("idx", 10_i);
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
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 10i
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));

    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Failure_LessThan_init_greater_than_rhs_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 10
            idx = b.Var("idx", 10_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 1
            binary = b.LessThan<bool>(b.Load(idx), 1_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 10u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 1u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));

    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Failure_LessThan_init_equals_rhs_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 10
            idx = b.Var("idx", 10_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            binary = b.LessThan<bool>(b.Load(idx), 10_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 10u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));

    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Failure_index_LessThan_constant_decreasing) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 1
            idx = b.Var("idx", 1_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            binary = b.LessThan<bool>(b.Load(idx), 10_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %idx:ptr<function, u32, read_write> = var 1u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));

    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Failure_GreaterThan_init_less_than_rhs_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 1
            idx = b.Var("idx", 1_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx > 10
            binary = b.GreaterThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 1i
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));

    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Failure_GreaterThan_init_equals_rhs_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 10
            idx = b.Var("idx", 10_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx > 10
            binary = b.GreaterThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 10i
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));

    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Failure_GreaterThan_init_less_than_rhs_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 1
            idx = b.Var("idx", 1_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx > 10
            binary = b.GreaterThan<bool>(b.Load(idx), 10_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 1u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));

    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Failure_GreaterThan_init_equals_rhs_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 10
            idx = b.Var("idx", 10_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx > 10
            binary = b.GreaterThan<bool>(b.Load(idx), 10_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 10u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));

    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Failure_index_GreaterThan_constant_increasing) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 10
            idx = b.Var("idx", 10_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx > 1
            binary = b.GreaterThan<bool>(b.Load(idx), 1_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 10u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = gt %3, 1u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));

    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

// const_lhs <= index
TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_LessThanEqual_init_equals_lhs_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 20 <= idx
            binary = b.LessThanEqual<bool>(20_i, b.Load(idx));
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
        %4:bool = lte 20i, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(20, range.min_bound);
    EXPECT_EQ(20, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_LessThanEqual_init_equals_lhs_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 20 <= idx
            binary = b.LessThanEqual<bool>(20_u, b.Load(idx));
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
        %4:bool = lte 20u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(20u, range.min_bound);
    EXPECT_EQ(20u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_LessThanEqual_init_LessThan_lhs_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 30 <= idx
            binary = b.LessThanEqual<bool>(30_i, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %4:bool = lte 30i, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_LessThanEqual_init_LessThan_lhs_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20u
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 30u <= idx
            binary = b.LessThanEqual<bool>(30_u, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %4:bool = lte 30u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest,
       AnalyzeLoopBody_Failure_constant_LessThanEqual_index_increasing) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20u
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 10u <= idx
            binary = b.LessThanEqual<bool>(10_u, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 20u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lte 10u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Success_min_u32_add_1_LessThanEqual_index) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 1u (minimum uint32_t value + 1)
            idx = b.Var("idx", u32(u32::kLowestValue + 1u));
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 1u >= idx
            binary = b.LessThanEqual<bool>(u32(u32::kLowestValue + 1u), b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %idx:ptr<function, u32, read_write> = var 1u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lte 1u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(u32::kLowestValue + 1u, range.min_bound);
    EXPECT_EQ(u32::kLowestValue + 1u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Success_min_i32_add_1_LessThanEqual_index) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = -2147483647 (minimum int32_t value + 1)
            idx = b.Var("idx", i32(i32::kLowestValue + 1));
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // -2147483647 >= idx
            binary = b.LessThanEqual<bool>(i32(i32::kLowestValue + 1), b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %idx:ptr<function, i32, read_write> = var -2147483647i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lte -2147483647i, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(i32::kLowestValue + 1, range.min_bound);
    EXPECT_EQ(i32::kLowestValue + 1, range.max_bound);
}

// const_lhs >= index
TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_GreaterThanEqual_init_equals_lhs_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 20 >= idx
            binary = b.GreaterThanEqual<bool>(20_i, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 20i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = gte 20i, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(20, range.min_bound);
    EXPECT_EQ(20, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_GreaterThanEqual_init_equals_lhs_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 20 >= idx
            binary = b.GreaterThanEqual<bool>(20_u, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 20u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = gte 20u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(20u, range.min_bound);
    EXPECT_EQ(20u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest,
       AnalyzeLoopBody_Failure_GreaterThanEqual_init_GreaterThan_lhs_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_i);
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
            // idx--
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest,
       AnalyzeLoopBody_Failure_GreaterThanEqual_init_GreaterThan_lhs_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20u
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 10u >= idx
            binary = b.GreaterThanEqual<bool>(10_u, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %4:bool = gte 10u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest,
       AnalyzeLoopBody_Failure_constant_GreaterThanEqual_index_decreasing) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20u
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 30u >= idx
            binary = b.GreaterThanEqual<bool>(30_u, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %4:bool = gte 30u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Success_max_u32_minus_1_GreaterThanEqual_index) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4294967294u (maximum uint32_t value - 1)
            idx = b.Var("idx", u32(u32::kHighestValue - 1u));
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 4294967294u >= idx
            binary = b.GreaterThanEqual<bool>(u32(u32::kHighestValue - 1u), b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 4294967294u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = gte 4294967294u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(u32::kHighestValue - 1u, range.min_bound);
    EXPECT_EQ(u32::kHighestValue - 1u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoop_Success_max_i32_minus_1_GreaterThanEqual_index) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 2147483646 (maximum int32_t value - 1)
            idx = b.Var("idx", i32(i32::kHighestValue - 1));
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 2147483646 >= idx
            binary = b.GreaterThanEqual<bool>(i32(i32::kHighestValue - 1), b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 2147483646i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = gte 2147483646i, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));
    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(i32::kHighestValue - 1, range.min_bound);
    EXPECT_EQ(i32::kHighestValue - 1, range.max_bound);
}

// const_lhs < index
TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_LessThan_init_equals_lhs_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 20 < idx
            binary = b.LessThan<bool>(20_i, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %4:bool = lt 20i, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_LessThan_init_equals_lhs_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 20 < idx
            binary = b.LessThan<bool>(20_u, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %4:bool = lt 20u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_LessThan_init_LessThan_lhs_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 30 < idx
            binary = b.LessThan<bool>(30_i, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %4:bool = lt 30i, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_LessThan_init_LessThan_lhs_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20u
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 30u < idx
            binary = b.LessThan<bool>(30_u, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %4:bool = lt 30u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_constant_LessThan_index_increasing) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20u
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
            // idx++
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

// lhs > index
TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_GreaterThan_init_equals_lhs_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 20 > idx
            binary = b.GreaterThan<bool>(20_i, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 20i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = gt 20i, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Success_GreaterThan_init_equals_lhs_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 20 > idx
            binary = b.GreaterThan<bool>(20_u, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 20u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = gt 20u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_GreaterThan_init_GreaterThan_lhs_i32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20
            idx = b.Var("idx", 20_i);
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
            // idx--
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_GreaterThan_init_GreaterThan_lhs_u32) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20u
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 10u > idx
            binary = b.GreaterThan<bool>(10_u, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %4:bool = gt 10u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AnalyzeLoopBody_Failure_constant_GreaterThan_index_decreasing) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20u
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 30u > idx
            binary = b.GreaterThan<bool>(30_u, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %4:bool = gt 30u, %3
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& info = analysis.GetInfo(idx);
    EXPECT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, LoadFromLoopControlVariableWithRange) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Load* load_idx = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            load_idx = b.Load(idx);
            b.Add<i32>(load_idx, 4_i);
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %5:i32 = load %idx
        %6:i32 = add %5, 4i
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

    IntegerRangeAnalysis analysis(&mod);

    const IntegerRangeInfo& idx_info = analysis.GetInfo(idx);
    EXPECT_TRUE(idx_info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(idx_info.range));

    const auto& range_idx = std::get<IntegerRangeInfo::SignedIntegerRange>(idx_info.range);
    EXPECT_EQ(0, range_idx.min_bound);
    EXPECT_EQ(9, range_idx.max_bound);

    const IntegerRangeInfo& load_idx_info = analysis.GetInfo(load_idx);
    EXPECT_TRUE(load_idx_info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(load_idx_info.range));

    const auto& range_load_idx =
        std::get<IntegerRangeInfo::SignedIntegerRange>(load_idx_info.range);
    EXPECT_EQ(0, range_load_idx.min_bound);
    EXPECT_EQ(9, range_load_idx.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, LoadFromLoopControlVariableWithoutRange) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    Binary* binary = nullptr;
    Load* load_idx = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 20u
            idx = b.Var("idx", 20_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // 30u > idx
            binary = b.GreaterThan<bool>(30_u, b.Load(idx));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            load_idx = b.Load(idx);
            b.Add<u32>(load_idx, 4_u);
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx--
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
        %4:bool = gt 30u, %3
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %5:u32 = load %idx
        %6:u32 = add %5, 4u
        continue  # -> $B4
      }
      $B4: {  # continuing
        %7:u32 = load %idx
        %8:u32 = sub %7, 1u
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

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_EQ(idx, analysis.GetLoopControlVariableFromConstantInitializerForTest(loop));
    EXPECT_EQ(binary, analysis.GetBinaryToCompareLoopControlVariableInLoopBodyForTest(loop, idx));

    const IntegerRangeInfo& idx_info = analysis.GetInfo(idx);
    EXPECT_FALSE(idx_info.IsValid());
    const IntegerRangeInfo& load_idx_info = analysis.GetInfo(idx);
    EXPECT_FALSE(load_idx_info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, LoadFromNonLoopControlVariable) {
    Load* load_a = nullptr;

    auto* var_a = mod.root_block->Append(b.Var<workgroup, u32>("a"));
    auto* func = b.ComputeFunction("foo", 3_u, 5_u, 7_u);
    b.Append(func->Block(), [&] {  //
        load_a = b.Load(var_a);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<workgroup, u32, read_write> = var undef
}

%foo = @compute @workgroup_size(3u, 5u, 7u) func():void {
  $B2: {
    %3:u32 = load %a
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);
    EXPECT_FALSE(analysis.GetInfo(load_a).IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AccessToLocalInvocationID) {
    auto* func = b.ComputeFunction("my_func", 4_u, 3_u, 2_u);
    auto* local_invocation_id = b.FunctionParam("localInvocationId", mod.Types().vec3<u32>());
    local_invocation_id->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationId);
    func->SetParams({local_invocation_id});

    Access* access_x = nullptr;
    Access* access_y = nullptr;
    Access* access_z = nullptr;
    b.Append(func->Block(), [&] {
        access_x = b.Access(ty.u32(), local_invocation_id, 0_u);
        access_y = b.Access(ty.u32(), local_invocation_id, 1_u);
        access_z = b.Access(ty.u32(), local_invocation_id, 2_u);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(4u, 3u, 2u) func(%localInvocationId:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %3:u32 = access %localInvocationId, 0u
    %4:u32 = access %localInvocationId, 1u
    %5:u32 = access %localInvocationId, 2u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    const auto& access_x_info = analysis.GetInfo(access_x);
    ASSERT_TRUE(access_x_info.IsValid());
    ASSERT_TRUE(
        std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(access_x_info.range));
    const auto& range_access_x =
        std::get<IntegerRangeInfo::UnsignedIntegerRange>(access_x_info.range);
    EXPECT_EQ(0u, range_access_x.min_bound);
    EXPECT_EQ(3u, range_access_x.max_bound);

    const auto& access_y_info = analysis.GetInfo(access_y);
    ASSERT_TRUE(access_x_info.IsValid());
    ASSERT_TRUE(
        std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(access_y_info.range));
    const auto& range_access_y =
        std::get<IntegerRangeInfo::UnsignedIntegerRange>(access_y_info.range);
    EXPECT_EQ(0u, range_access_y.min_bound);
    EXPECT_EQ(2u, range_access_y.max_bound);

    const auto& access_z_info = analysis.GetInfo(access_z);
    ASSERT_TRUE(access_x_info.IsValid());
    ASSERT_TRUE(
        std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(access_z_info.range));
    const auto& range_access_z =
        std::get<IntegerRangeInfo::UnsignedIntegerRange>(access_z_info.range);
    EXPECT_EQ(0u, range_access_z.min_bound);
    EXPECT_EQ(1u, range_access_z.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, NotAccessToFunctionParam) {
    auto* func = b.Function("func", ty.void_());
    Access* access = nullptr;
    b.Append(func->Block(), [&] {
        auto* dst = b.Var(ty.ptr<function, array<u32, 24u>>());
        access = b.Access(ty.ptr<function, u32>(), dst, 0_u);
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    %2:ptr<function, array<u32, 24>, read_write> = var undef
    %3:ptr<function, u32, read_write> = access %2, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(access);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AccessToFunctionParamNoRange) {
    auto* func = b.ComputeFunction("my_func", 4_u, 3_u, 2_u);
    auto* global_invocation_id = b.FunctionParam("globalId", mod.Types().vec3<u32>());
    global_invocation_id->SetBuiltin(tint::core::BuiltinValue::kGlobalInvocationId);
    func->SetParams({global_invocation_id});

    Access* access_x = nullptr;
    Access* access_y = nullptr;
    Access* access_z = nullptr;
    b.Append(func->Block(), [&] {
        access_x = b.Access(ty.u32(), global_invocation_id, 0_u);
        access_y = b.Access(ty.u32(), global_invocation_id, 1_u);
        access_z = b.Access(ty.u32(), global_invocation_id, 2_u);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(4u, 3u, 2u) func(%globalId:vec3<u32> [@global_invocation_id]):void {
  $B1: {
    %3:u32 = access %globalId, 0u
    %4:u32 = access %globalId, 1u
    %5:u32 = access %globalId, 2u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);
    ASSERT_FALSE(analysis.GetInfo(access_x).IsValid());
    ASSERT_FALSE(analysis.GetInfo(access_y).IsValid());
    ASSERT_FALSE(analysis.GetInfo(access_z).IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, AccessToNonIntegerFunctionParam) {
    auto* func = b.Function("func", ty.void_());
    Access* access = nullptr;
    auto* param = b.FunctionParam("param", ty.vec4<f32>());
    func->SetParams({param});
    b.Append(func->Block(), [&] {
        access = b.Access(ty.f32(), param, 0_u);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:vec4<f32>):void {
  $B1: {
    %3:f32 = access %param, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(access);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, NonConstantAccessIndex) {
    auto* func = b.ComputeFunction("my_func", 4_u, 3_u, 2_u);
    auto* local_invocation_id = b.FunctionParam("localInvocationId", mod.Types().vec3<u32>());
    local_invocation_id->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationId);
    func->SetParams({local_invocation_id});

    Access* access_x = nullptr;
    b.Append(func->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, u32>());
        auto* index = b.Load(var);
        access_x = b.Access(ty.u32(), local_invocation_id, index);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(4u, 3u, 2u) func(%localInvocationId:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %3:ptr<function, u32, read_write> = var undef
    %4:u32 = load %3
    %5:u32 = access %localInvocationId, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    const auto& access_x_info = analysis.GetInfo(access_x);
    ASSERT_FALSE(access_x_info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, SignedIntegerScalarConstant) {
    auto* func = b.Function("func", ty.void_());
    Constant* constant = nullptr;
    b.Append(func->Block(), [&] {
        constant = b.Constant(10_i);
        b.Var("a", constant);
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    %a:ptr<function, i32, read_write> = var 10i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(constant);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(10, range.min_bound);
    EXPECT_EQ(10, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, UnsignedIntegerScalarConstant) {
    auto* func = b.Function("func", ty.void_());
    Constant* constant = nullptr;
    b.Append(func->Block(), [&] {
        constant = b.Constant(20_u);
        b.Var("a", constant);
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    %a:ptr<function, u32, read_write> = var 20u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(constant);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(20u, range.min_bound);
    EXPECT_EQ(20u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, NonIntegerConstant) {
    auto* func = b.Function("func", ty.void_());
    Constant* constant = nullptr;
    b.Append(func->Block(), [&] {
        constant = b.Constant(1.0_f);
        b.Var("a", constant);
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    %a:ptr<function, f32, read_write> = var 1.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(constant);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, ValueAsScalarFunctionParameter) {
    Binary* add = nullptr;
    auto* func = b.ComputeFunction("my_func", 4_u, 3_u, 2_u);
    auto* localInvocationIndex = b.FunctionParam("localInvocationIndex", mod.Types().u32());
    localInvocationIndex->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationIndex);
    func->SetParams({localInvocationIndex});

    b.Append(func->Block(), [&] {
        // add = localInvocationIndex + 5
        add = b.Add<u32>(localInvocationIndex, 5_u);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(4u, 3u, 2u) func(%localInvocationIndex:u32 [@local_invocation_index]):void {
  $B1: {
    %3:u32 = add %localInvocationIndex, 5u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `add->LHS()` (`localInvocationIndex`)
    const auto& info = analysis.GetInfo(add->LHS());
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(0u, range.min_bound);
    EXPECT_EQ(23u, range.max_bound);

    // Range of `add` (`localInvocationIndex + 5`)
    const auto& info_add = analysis.GetInfo(add);
    ASSERT_TRUE(info_add.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info_add.range));
    const auto& range_add = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info_add.range);
    EXPECT_EQ(5u, range_add.min_bound);
    EXPECT_EQ(28u, range_add.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, ValueAsVectorFunctionParameter) {
    auto* func = b.ComputeFunction("my_func", 4_u, 3_u, 2_u);
    auto* local_invocation_id = b.FunctionParam("local_id", mod.Types().vec3<u32>());
    local_invocation_id->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationId);
    func->SetParams({local_invocation_id});

    Value* value = nullptr;
    b.Append(func->Block(), [&] {
        value = b.Value(local_invocation_id);
        b.Access(ty.u32(), local_invocation_id, 0_u);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(4u, 3u, 2u) func(%local_id:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %3:u32 = access %local_id, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    const auto& info = analysis.GetInfo(value);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, ValueAsAccess) {
    auto* func = b.ComputeFunction("my_func", 4_u, 3_u, 2_u);
    auto* local_invocation_id = b.FunctionParam("local_id", mod.Types().vec3<u32>());
    local_invocation_id->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationId);
    func->SetParams({local_invocation_id});

    Binary* add = nullptr;
    b.Append(func->Block(), [&] {
        auto* access_x = b.Access(ty.u32(), local_invocation_id, 0_u);
        auto* access_y = b.Access(ty.u32(), local_invocation_id, 1_u);
        // add = access_x + access_y
        add = b.Add<u32>(access_x, access_y);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(4u, 3u, 2u) func(%local_id:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %3:u32 = access %local_id, 0u
    %4:u32 = access %local_id, 1u
    %5:u32 = add %3, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `add->LHS()` (`local_id.x`)
    const auto& info_lhs = analysis.GetInfo(add->LHS());
    ASSERT_TRUE(info_lhs.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info_lhs.range));
    const auto& range_lhs = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info_lhs.range);
    EXPECT_EQ(0u, range_lhs.min_bound);
    EXPECT_EQ(3u, range_lhs.max_bound);

    // Range of `add->RHS()` (`local_id.y`)
    const auto& info_rhs = analysis.GetInfo(add->RHS());
    ASSERT_TRUE(info_rhs.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info_rhs.range));
    const auto& range_rhs = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info_rhs.range);
    EXPECT_EQ(0u, range_rhs.min_bound);
    EXPECT_EQ(2u, range_rhs.max_bound);

    // Range of `add` (`local_id.x + local_id.y`)
    const auto& info_add = analysis.GetInfo(add);
    ASSERT_TRUE(info_add.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info_add.range));
    const auto& range_add = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info_add.range);
    EXPECT_EQ(0u, range_add.min_bound);
    EXPECT_EQ(5u, range_add.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, ValueAsLoadAndConstant) {
    Var* idx = nullptr;
    Binary* add = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            // add = idx + 5
            add = b.Add<i32>(b.Load(idx), 5_i);
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %5:i32 = load %idx
        %6:i32 = add %5, 5i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `add->LHS()` (`idx`)
    const auto& info_lhs = analysis.GetInfo(add->LHS());
    ASSERT_TRUE(info_lhs.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info_lhs.range));
    const auto& range_lhs = std::get<IntegerRangeInfo::SignedIntegerRange>(info_lhs.range);
    EXPECT_EQ(0, range_lhs.min_bound);
    EXPECT_EQ(9, range_lhs.max_bound);

    // Range of `add->RHS()` (5)
    const auto& info_rhs = analysis.GetInfo(add->RHS());
    ASSERT_TRUE(info_rhs.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info_rhs.range));
    const auto& range_rhs = std::get<IntegerRangeInfo::SignedIntegerRange>(info_rhs.range);
    EXPECT_EQ(5, range_rhs.min_bound);
    EXPECT_EQ(5, range_rhs.max_bound);

    // Range of `add` (`idx + 5`)
    const auto& info_add = analysis.GetInfo(add);
    ASSERT_TRUE(info_add.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info_add.range));
    const auto& range_add = std::get<IntegerRangeInfo::SignedIntegerRange>(info_add.range);
    EXPECT_EQ(5, range_add.min_bound);
    EXPECT_EQ(14, range_add.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, ValueAsVar) {
    Var* idx = nullptr;
    Value* value = nullptr;
    Binary* add = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            value = b.Value(idx);
            // add = value + 5
            add = b.Add<i32>(b.Load(value), 5_i);
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %5:i32 = load %idx
        %6:i32 = add %5, 5i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `value`
    const auto& info = analysis.GetInfo(value);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(0, range.min_bound);
    EXPECT_EQ(9, range.max_bound);

    // Range of `add` (`value + 5`)
    const auto& info_add = analysis.GetInfo(add);
    ASSERT_TRUE(info_add.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info_add.range));
    const auto& range_add = std::get<IntegerRangeInfo::SignedIntegerRange>(info_add.range);
    EXPECT_EQ(5, range_add.min_bound);
    EXPECT_EQ(14, range_add.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, LetWithAccess) {
    auto* func = b.ComputeFunction("my_func", 4_u, 3_u, 2_u);
    auto* local_invocation_id = b.FunctionParam("local_id", mod.Types().vec3<u32>());
    local_invocation_id->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationId);
    func->SetParams({local_invocation_id});

    Let* let = nullptr;
    b.Append(func->Block(), [&] {
        // access_x: [0, 3]
        auto* access_x = b.Access(ty.u32(), local_invocation_id, 0_u);
        let = b.Let(access_x);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(4u, 3u, 2u) func(%local_id:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %3:u32 = access %local_id, 0u
    %4:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    const auto& info = analysis.GetInfo(let);
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(0u, range.min_bound);
    EXPECT_EQ(3u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, LetWithLoad) {
    Var* idx = nullptr;
    Let* let = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            auto* load = b.Load(idx);
            let = b.Let(load);
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %5:i32 = load %idx
        %6:i32 = let %5
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

    IntegerRangeAnalysis analysis(&mod);

    const auto& info = analysis.GetInfo(let);
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(0, range.min_bound);
    EXPECT_EQ(9, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, LetWithBinary) {
    auto* func = b.ComputeFunction("my_func", 4_u, 3_u, 2_u);
    auto* local_invocation_id = b.FunctionParam("local_id", mod.Types().vec3<u32>());
    local_invocation_id->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationId);
    func->SetParams({local_invocation_id});

    Let* let = nullptr;
    b.Append(func->Block(), [&] {
        // access_x: [0, 3]
        auto* access_x = b.Access(ty.u32(), local_invocation_id, 0_u);
        // access_y: [0, 2]
        auto* access_y = b.Access(ty.u32(), local_invocation_id, 1_u);
        auto* add = b.Add<u32>(access_x, access_y);
        let = b.Let(add);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(4u, 3u, 2u) func(%local_id:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %3:u32 = access %local_id, 0u
    %4:u32 = access %local_id, 1u
    %5:u32 = add %3, %4
    %6:u32 = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    const auto& info = analysis.GetInfo(let);
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(0u, range.min_bound);
    EXPECT_EQ(5u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, LetAsOperand) {
    auto* func = b.ComputeFunction("my_func", 4_u, 3_u, 2_u);
    auto* local_invocation_id = b.FunctionParam("local_id", mod.Types().vec3<u32>());
    local_invocation_id->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationId);
    func->SetParams({local_invocation_id});

    Let* let = nullptr;
    Binary* add = nullptr;
    b.Append(func->Block(), [&] {
        // let (access_x): [0, 3]
        let = b.Let(b.Access(ty.u32(), local_invocation_id, 0_u));
        // access_y: [0, 2]
        auto* access_y = b.Access(ty.u32(), local_invocation_id, 1_u);
        add = b.Add<u32>(let, access_y);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(4u, 3u, 2u) func(%local_id:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %3:u32 = access %local_id, 0u
    %4:u32 = let %3
    %5:u32 = access %local_id, 1u
    %6:u32 = add %4, %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    const auto& info_let = analysis.GetInfo(let);
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info_let.range));
    const auto& range_let = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info_let.range);
    EXPECT_EQ(0u, range_let.min_bound);
    EXPECT_EQ(3u, range_let.max_bound);

    const auto& info_add = analysis.GetInfo(add);
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info_add.range));
    const auto& range_add = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info_add.range);
    EXPECT_EQ(0u, range_add.min_bound);
    EXPECT_EQ(5u, range_add.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, MultipleBinaryAdds) {
    auto* func = b.ComputeFunction("my_func", 4_u, 3_u, 2_u);
    auto* local_invocation_id = b.FunctionParam("local_id", mod.Types().vec3<u32>());
    local_invocation_id->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationId);
    func->SetParams({local_invocation_id});

    Var* idx = nullptr;
    Binary* add = nullptr;
    b.Append(func->Block(), [&] {
        auto* access_x = b.Access(ty.u32(), local_invocation_id, 0_u);
        auto* access_y = b.Access(ty.u32(), local_invocation_id, 1_u);

        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            // add1 = local_id.x + local_id.y
            auto* add1 = b.Add<u32>(access_x, access_y);
            // add = idx + add1
            add = b.Add<u32>(b.Load(idx), add1);
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
            b.Store(idx, b.Add<u32>(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });

        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(4u, 3u, 2u) func(%local_id:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %3:u32 = access %local_id, 0u
    %4:u32 = access %local_id, 1u
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %6:u32 = load %idx
        %7:bool = lt %6, 10u
        if %7 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %8:u32 = add %3, %4
        %9:u32 = load %idx
        %10:u32 = add %9, %8
        continue  # -> $B4
      }
      $B4: {  # continuing
        %11:u32 = load %idx
        %12:u32 = add %11, 1u
        store %idx, %12
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `add` (`idx + (local_id.x + local_id.y)`)
    // access_x: [0, 3], access_y: [0, 2], idx: [0, 9]
    const auto& info_add = analysis.GetInfo(add);
    ASSERT_TRUE(info_add.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info_add.range));
    const auto& range_add = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info_add.range);
    EXPECT_EQ(0u, range_add.min_bound);
    EXPECT_EQ(14u, range_add.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryAdd_U32_Overflow) {
    auto* func = b.ComputeFunction("my_func", 8_u, 1_u, 1_u);
    auto* local_invocation_id = b.FunctionParam("local_id", mod.Types().vec3<u32>());
    local_invocation_id->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationId);
    func->SetParams({local_invocation_id});

    // kLargeValue = 4294967289u
    constexpr uint32_t kLargeValue = u32::kHighestValue - 6u;
    Binary* add = nullptr;
    b.Append(func->Block(), [&] {
        auto* access_x = b.Access(ty.u32(), local_invocation_id, 0_u);
        // add = local_id.x + kLargeValue
        add = b.Add<u32>(access_x, b.Constant(u32(kLargeValue)));
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(8u, 1u, 1u) func(%local_id:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %3:u32 = access %local_id, 0u
    %4:u32 = add %3, 4294967289u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `add` (`local_id.x + 4294967289`)
    // local_id.x: [0, 7], 4294967289 > u32::kHighestValue - 7
    const auto& info = analysis.GetInfo(add);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryAdd_I32_Overflow) {
    // kLargeValue = 2147483639
    constexpr int32_t kLargeValue = i32::kHighestValue - 8;

    Var* idx = nullptr;
    Binary* add = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            // add = idx + kLargeValue
            add = b.Add<i32>(b.Load(idx), b.Constant(i32(kLargeValue)));
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %5:i32 = load %idx
        %6:i32 = add %5, 2147483639i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `add` (`idx + 2147483639`)
    // idx: [0, 9], 2147483639 > i32::kHighestValue - 9
    const auto& info = analysis.GetInfo(add);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryAdd_I32_Underflow) {
    // kSmallValue = -2147483640
    constexpr int32_t kSmallValue = i32::kLowestValue + 8;

    Var* idx = nullptr;
    Binary* add = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = -9
            idx = b.Var("idx", -9_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 1
            auto* binary = b.LessThan<bool>(b.Load(idx), 1_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            // add = idx + kSmallValue
            add = b.Add<i32>(b.Load(idx), b.Constant(i32(kSmallValue)));
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var -9i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 1i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %5:i32 = load %idx
        %6:i32 = add %5, -2147483640i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `add` (`idx + (-2147483640)`)
    // idx: [-9, 0], -2147483640 < i32::kLowestValue + 9
    const auto& info = analysis.GetInfo(add);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinarySubtract_Success_U32) {
    auto* func = b.ComputeFunction("my_func", 4_u, 1_u, 1_u);
    auto* local_invocation_id = b.FunctionParam("local_id", mod.Types().vec3<u32>());
    local_invocation_id->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationId);
    func->SetParams({local_invocation_id});

    Var* idx = nullptr;
    Binary* subtract = nullptr;
    b.Append(func->Block(), [&] {
        auto* access_x = b.Access(ty.u32(), local_invocation_id, 0_u);

        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4
            idx = b.Var("idx", 4_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            // add = idx - local_id.x
            subtract = b.Subtract<u32>(b.Load(idx), access_x);
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
            b.Store(idx, b.Add<u32>(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });

        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(4u, 1u, 1u) func(%local_id:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %3:u32 = access %local_id, 0u
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 4u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %5:u32 = load %idx
        %6:bool = lt %5, 10u
        if %6 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %7:u32 = load %idx
        %8:u32 = sub %7, %3
        continue  # -> $B4
      }
      $B4: {  # continuing
        %9:u32 = load %idx
        %10:u32 = add %9, 1u
        store %idx, %10
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `subtract` (`idx - local_id.x`)
    // idx: [4, 9] local_id.x: [0, 3]
    const auto& info = analysis.GetInfo(subtract);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(1u, range.min_bound);
    EXPECT_EQ(9u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, BinarySubtract_Success_I32) {
    Var* idx = nullptr;
    Binary* subtract = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 5
            idx = b.Var("idx", 5_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_i);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 4
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 4_i);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                subtract = b.Subtract<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<i32>(b.Load(idy), 1_i));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 5i
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
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, i32, read_write> = var 1i
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:i32 = load %idy
            %7:bool = lt %6, 4i
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:i32 = load %idy
            %10:i32 = sub %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:i32 = load %idy
            %12:i32 = add %11, 1i
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `subtract` (`idx - idy`)
    // idx: [5, 9], idy: [1, 3]
    const auto& info = analysis.GetInfo(subtract);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(2, range.min_bound);
    EXPECT_EQ(8, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, BinarySubtract_Failure_Underflow_U32) {
    Var* idx = nullptr;
    Binary* subtract = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 5
            idx = b.Var("idx", 5_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 7
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 7_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                subtract = b.Subtract<u32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 5u
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
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 7u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:u32 = load %idx
            %9:u32 = load %idy
            %10:u32 = sub %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:u32 = load %idx
        %14:u32 = add %13, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `subtract` (`idx - idy`)
    // idx: [5, 9], idy: [1, 7], idx.min_bound < idy.max_bound
    const auto& info = analysis.GetInfo(subtract);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinarySubtract_Failure_Overflow_I32) {
    // kSmallValue = -2147483640
    constexpr int32_t kSmallValue = i32::kLowestValue + 8;

    Var* idx = nullptr;
    Binary* subtract = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            // subtract = idx - kSmallValue
            subtract = b.Subtract<i32>(b.Load(idx), b.Constant(i32(kSmallValue)));
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %4:bool = lt %3, 9i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %5:i32 = load %idx
        %6:i32 = sub %5, -2147483640i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `subtract` (`idx - (-2147483640)`)
    // idx: [0, 8], 2147483640 > i32::kHighestValue - 8
    const auto& info = analysis.GetInfo(subtract);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinarySubtract_Failure_Underflow_I32) {
    // kLargeValue = 2147483640
    constexpr int32_t kLargeValue = i32::kHighestValue - 7;

    Var* idx = nullptr;
    Binary* subtract = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = -9
            idx = b.Var("idx", -9_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 0
            auto* binary = b.LessThan<bool>(b.Load(idx), 0_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            // subtract = idx - kLargeValue
            subtract = b.Subtract<i32>(b.Load(idx), b.Constant(i32(kLargeValue)));
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var -9i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 0i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %5:i32 = load %idx
        %6:i32 = sub %5, 2147483640i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `subtract` (`idx - 2147483640`)
    // idx: [-9, 0], idx < i32::kLowestValue + 2147483640
    const auto& info = analysis.GetInfo(subtract);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryMultiply_Success_I32) {
    Var* idx = nullptr;
    Binary* multiply = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 5
            idx = b.Var("idx", 5_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_i);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 4
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 4_i);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                multiply = b.Multiply<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<i32>(b.Load(idy), 1_i));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 5i
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
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, i32, read_write> = var 1i
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:i32 = load %idy
            %7:bool = lt %6, 4i
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:i32 = load %idy
            %10:i32 = mul %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:i32 = load %idy
            %12:i32 = add %11, 1i
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `multiply` (`idx * idy`)
    // idx: [5, 9], idy: [1, 3]
    const auto& info = analysis.GetInfo(multiply);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(5, range.min_bound);
    EXPECT_EQ(27, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryMultiply_Success_U32) {
    Var* idx = nullptr;
    Binary* multiply = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 3
            idx = b.Var("idx", 3_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 11
            auto* binary = b.LessThan<bool>(b.Load(idx), 11_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 6
                idy = b.Var("idy", 6_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 20
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 20_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                multiply = b.Multiply<u32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 3u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 11u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 6u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 20u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:u32 = load %idx
            %9:u32 = load %idy
            %10:u32 = mul %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:u32 = load %idx
        %14:u32 = add %13, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `multiply` (`idx * idy`)
    // idx: [3, 10], idy: [6, 19]
    const auto& info = analysis.GetInfo(multiply);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(18u, range.min_bound);
    EXPECT_EQ(190u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryMultiply_Failure_negative_lhs) {
    Var* idx = nullptr;
    Binary* multiply = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = -5
            idx = b.Var("idx", -5_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_i);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 4
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 4_i);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                multiply = b.Multiply<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<i32>(b.Load(idy), 1_i));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var -5i
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
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, i32, read_write> = var 1i
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:i32 = load %idy
            %7:bool = lt %6, 4i
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:i32 = load %idy
            %10:i32 = mul %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:i32 = load %idy
            %12:i32 = add %11, 1i
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `multiply` (`idx * idy`)
    // idx: [-5, 9], idy: [1, 3]
    const auto& info = analysis.GetInfo(multiply);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryMultiply_Failure_negative_rhs) {
    Var* idx = nullptr;
    Binary* multiply = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 5
            idx = b.Var("idx", 5_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = -1
                idy = b.Var("idy", -1_i);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 4
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 4_i);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                multiply = b.Multiply<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<i32>(b.Load(idy), 1_i));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 5i
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
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, i32, read_write> = var -1i
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:i32 = load %idy
            %7:bool = lt %6, 4i
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:i32 = load %idy
            %10:i32 = mul %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:i32 = load %idy
            %12:i32 = add %11, 1i
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `multiply` (`idx * idy`)
    // idx: [5, 9], idy: [-1, 3]
    const auto& info = analysis.GetInfo(multiply);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryMultiply_Failure_Overflow_MaxBound_I32) {
    // kLargeValue = 268435456
    constexpr int32_t kLargeValue = i32::kHighestValue / 8 + 1;

    Var* idx = nullptr;
    Binary* multiply = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            // multiply = idx * kLargeValue
            multiply = b.Multiply<i32>(b.Load(idx), b.Constant(i32(kLargeValue)));
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %4:bool = lt %3, 9i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %5:i32 = load %idx
        %6:i32 = mul %5, 268435456i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `multiply` (`idx * 268435456`)
    // idx: [0, 8], 268435456 > i32::kHighestValue / 8
    const auto& info = analysis.GetInfo(multiply);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryMultiply_Failure_Overflow_MinBound_I32) {
    // kLargeValue = 134217728
    constexpr int32_t kLargeValue = i32::kHighestValue / 16 + 1;

    Var* idx = nullptr;
    Binary* multiply = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 16
            idx = b.Var("idx", 16_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 21
            auto* binary = b.LessThan<bool>(b.Load(idx), 21_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            // multiply = idx * kLargeValue
            multiply = b.Multiply<i32>(b.Load(idx), b.Constant(i32(kLargeValue)));
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 16i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 21i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %5:i32 = load %idx
        %6:i32 = mul %5, 134217728i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `multiply` (`idx * 134217728`)
    // idx: [16, 20], 134217728 > i32::kHighestValue / 16
    const auto& info = analysis.GetInfo(multiply);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryMultiply_Failure_Overflow_MaxBound_U32) {
    // kLargeValue = 536870912
    constexpr uint32_t kLargeValue = u32::kHighestValue / 8 + 1;

    Var* idx = nullptr;
    Binary* multiply = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            // multiply = idx * kLargeValue
            multiply = b.Multiply<u32>(b.Load(idx), b.Constant(u32(kLargeValue)));
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %4:bool = lt %3, 9u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %5:u32 = load %idx
        %6:u32 = mul %5, 536870912u
        continue  # -> $B4
      }
      $B4: {  # continuing
        %7:u32 = load %idx
        %8:u32 = add %7, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `multiply` (`idx * 536870912`)
    // idx: [0, 8], 536870912 > u32::kHighestValue / 8
    const auto& info = analysis.GetInfo(multiply);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryMultiply_Failure_Overflow_MinBound_U32) {
    // kLargeValue = 268435456
    constexpr uint32_t kLargeValue = u32::kHighestValue / 16 + 1;

    Var* idx = nullptr;
    Binary* multiply = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 16
            idx = b.Var("idx", 16_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 21
            auto* binary = b.LessThan<bool>(b.Load(idx), 21_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            // multiply = idx * kLargeValue
            multiply = b.Multiply<u32>(b.Load(idx), b.Constant(u32(kLargeValue)));
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 16u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 21u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %5:u32 = load %idx
        %6:u32 = mul %5, 268435456u
        continue  # -> $B4
      }
      $B4: {  # continuing
        %7:u32 = load %idx
        %8:u32 = add %7, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `multiply` (`idx * 268435456`)
    // idx: [16, 20], 268435456 > u32::kHighestValue / 16
    const auto& info = analysis.GetInfo(multiply);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Convert_Success_U32ToI32) {
    auto* func = b.ComputeFunction("my_func", 4_u, 3_u, 2_u);
    auto* localInvocationIndex = b.FunctionParam("localInvocationIndex", mod.Types().u32());
    localInvocationIndex->SetBuiltin(tint::core::BuiltinValue::kLocalInvocationIndex);
    func->SetParams({localInvocationIndex});

    Convert* convert = nullptr;
    b.Append(func->Block(), [&] {
        convert = b.Convert<i32>(localInvocationIndex);
        b.Return(func);
    });

    auto* src = R"(
%my_func = @compute @workgroup_size(4u, 3u, 2u) func(%localInvocationIndex:u32 [@local_invocation_index]):void {
  $B1: {
    %3:i32 = convert %localInvocationIndex
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(convert);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(0, range.min_bound);
    EXPECT_EQ(23, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Convert_Success_I32ToU32) {
    Var* idx = nullptr;
    Convert* convert = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 1
            idx = b.Var("idx", 1_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            convert = b.Convert<u32>(b.Load(idx));
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 1i
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
        %5:i32 = load %idx
        %6:u32 = convert %5
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

    IntegerRangeAnalysis analysis(&mod);

    const auto& info = analysis.GetInfo(convert);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));

    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(1u, range.min_bound);
    EXPECT_EQ(9u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Convert_Failure_NegativeI32ToU32) {
    Var* idx = nullptr;
    Convert* convert = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = -1
            idx = b.Var("idx", -1_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            convert = b.Convert<u32>(b.Load(idx));
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var -1i
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
        %5:i32 = load %idx
        %6:u32 = convert %5
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

    IntegerRangeAnalysis analysis(&mod);

    const auto& info = analysis.GetInfo(convert);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Convert_Failure_LargeU32ToI32) {
    Var* idx = nullptr;
    Convert* convert = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0u
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx <= u32(i32::kHighestValue) + 1u
            auto* binary = b.LessThanEqual<bool>(
                b.Load(idx), u32(static_cast<uint32_t>(i32::kHighestValue) + 1u));
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            convert = b.Convert<i32>(b.Load(idx));
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %4:bool = lte %3, 2147483648u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %5:u32 = load %idx
        %6:i32 = convert %5
        continue  # -> $B4
      }
      $B4: {  # continuing
        %7:u32 = load %idx
        %8:u32 = add %7, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    const auto& info = analysis.GetInfo(convert);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Convert_Failure_ConvertToNonInteger) {
    Var* idx = nullptr;
    Convert* convert = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = -1
            idx = b.Var("idx", -1_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });
            convert = b.Convert<f32>(b.Load(idx));
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var -1i
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
        %5:i32 = load %idx
        %6:f32 = convert %5
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

    IntegerRangeAnalysis analysis(&mod);

    const auto& info = analysis.GetInfo(convert);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Divide_Success_Divisible_I32) {
    Var* idx = nullptr;
    Binary* divide = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4
            idx = b.Var("idx", 4_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_i);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 3
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 3_i);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // divide = idx / idy
                divide = b.Divide<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<i32>(b.Load(idy), 1_i));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 4i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 9i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, i32, read_write> = var 1i
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:i32 = load %idy
            %7:bool = lt %6, 3i
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:i32 = load %idy
            %10:i32 = div %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:i32 = load %idy
            %12:i32 = add %11, 1i
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `divide` (`idx / idy`)
    // idx: [4, 8], idy: [1, 2]
    const auto& info = analysis.GetInfo(divide);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(2, range.min_bound);
    EXPECT_EQ(8, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Divide_Success_Divisible_U32) {
    Var* idx = nullptr;
    Binary* divide = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4
            idx = b.Var("idx", 4_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 3
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 3_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // divide = idx / idy
                divide = b.Divide<u32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 4u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 9u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 3u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:u32 = load %idx
            %9:u32 = load %idy
            %10:u32 = div %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:u32 = load %idx
        %14:u32 = add %13, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `divide` (`idx / idy`)
    // idx: [4u, 8u], idy: [1u, 2u]
    const auto& info = analysis.GetInfo(divide);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(2u, range.min_bound);
    EXPECT_EQ(8u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Divide_Success_Nondivisible_GreaterThanOne) {
    Var* idx = nullptr;
    Binary* divide = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 8
            idx = b.Var("idx", 8_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 17
            auto* binary = b.LessThan<bool>(b.Load(idx), 17_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 2
                idy = b.Var("idy", 2_i);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 4
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 4_i);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // divide = idx / idy
                divide = b.Divide<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<i32>(b.Load(idy), 1_i));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 8i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 17i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, i32, read_write> = var 2i
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:i32 = load %idy
            %7:bool = lt %6, 4i
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:i32 = load %idy
            %10:i32 = div %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:i32 = load %idy
            %12:i32 = add %11, 1i
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `divide` (`idx / idy`)
    // idx: [8, 16], idy: [2, 3]
    const auto& info = analysis.GetInfo(divide);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(2, range.min_bound);
    EXPECT_EQ(8, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Divide_Success_Nondivisible_LessThanOne) {
    Var* idx = nullptr;
    Binary* divide = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 8
            idx = b.Var("idx", 8_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 17
            auto* binary = b.LessThan<bool>(b.Load(idx), 17_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 6
                idy = b.Var("idy", 6_i);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 10
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 10_i);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // divide = idx / idy
                divide = b.Divide<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<i32>(b.Load(idy), 1_i));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 8i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 17i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, i32, read_write> = var 6i
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:i32 = load %idy
            %7:bool = lt %6, 10i
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:i32 = load %idy
            %10:i32 = div %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:i32 = load %idy
            %12:i32 = add %11, 1i
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `divide` (`idx / idy`)
    // idx: [8, 16], idy: [6, 9]
    const auto& info = analysis.GetInfo(divide);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(0, range.min_bound);
    EXPECT_EQ(2, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Divide_Success_ZeroLHS_I32) {
    Var* idx = nullptr;
    Binary* divide = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 17
            auto* binary = b.LessThan<bool>(b.Load(idx), 17_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 4
                idy = b.Var("idy", 4_i);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 9
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 9_i);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // divide = idx / idy
                divide = b.Divide<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<i32>(b.Load(idy), 1_i));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %4:bool = lt %3, 17i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, i32, read_write> = var 4i
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:i32 = load %idy
            %7:bool = lt %6, 9i
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:i32 = load %idy
            %10:i32 = div %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:i32 = load %idy
            %12:i32 = add %11, 1i
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `divide` (`idx / idy`)
    // idx: [0, 16], idy: [4, 8]
    const auto& info = analysis.GetInfo(divide);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(0, range.min_bound);
    EXPECT_EQ(4, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Divide_Success_ZeroLHS_U32) {
    Var* idx = nullptr;
    Binary* divide = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 17
            auto* binary = b.LessThan<bool>(b.Load(idx), 17_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 4
                idy = b.Var("idy", 4_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 9
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 9_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // divide = idx / idy
                divide = b.Divide<u32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %4:bool = lt %3, 17u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 4u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 9u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:u32 = load %idx
            %9:u32 = load %idy
            %10:u32 = div %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:u32 = load %idx
        %14:u32 = add %13, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `divide` (`idx / idy`)
    // idx: [0, 16u], idy: [4u, 8u]
    const auto& info = analysis.GetInfo(divide);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(0u, range.min_bound);
    EXPECT_EQ(4u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Divide_Failure_NegativeLHS) {
    Var* idx = nullptr;
    Binary* divide = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = -1
            idx = b.Var("idx", -1_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 17
            auto* binary = b.LessThan<bool>(b.Load(idx), 17_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 4
                idy = b.Var("idy", 4_i);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 9
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 9_i);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // divide = idx / idy
                divide = b.Divide<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<i32>(b.Load(idy), 1_i));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var -1i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 17i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, i32, read_write> = var 4i
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:i32 = load %idy
            %7:bool = lt %6, 9i
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:i32 = load %idy
            %10:i32 = div %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:i32 = load %idy
            %12:i32 = add %11, 1i
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `divide` (`idx / idy`)
    // idx: [-1, 16], idy: [4, 8]
    const auto& info = analysis.GetInfo(divide);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Divide_Failure_NegativeRHS) {
    Var* idx = nullptr;
    Binary* divide = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4
            idx = b.Var("idx", 4_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 17
            auto* binary = b.LessThan<bool>(b.Load(idx), 17_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = -4
                idy = b.Var("idy", -4_i);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 9
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 9_i);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // divide = idx / idy
                divide = b.Divide<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<i32>(b.Load(idy), 1_i));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 4i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 17i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, i32, read_write> = var -4i
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:i32 = load %idy
            %7:bool = lt %6, 9i
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:i32 = load %idy
            %10:i32 = div %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:i32 = load %idy
            %12:i32 = add %11, 1i
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `divide` (`idx / idy`)
    // idx: [0, 16], idy: [-4, 8]
    const auto& info = analysis.GetInfo(divide);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Divide_Failure_ZeroRHS_I32) {
    Var* idx = nullptr;
    Binary* divide = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4
            idx = b.Var("idx", 4_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 17
            auto* binary = b.LessThan<bool>(b.Load(idx), 17_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 0
                idy = b.Var("idy", 0_i);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 9
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 9_i);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // divide = idx / idy
                divide = b.Divide<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<i32>(b.Load(idy), 1_i));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 4i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 17i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, i32, read_write> = var 0i
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:i32 = load %idy
            %7:bool = lt %6, 9i
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:i32 = load %idy
            %10:i32 = div %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:i32 = load %idy
            %12:i32 = add %11, 1i
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `divide` (`idx / idy`)
    // idx: [4, 16], idy: [0, 8]
    const auto& info = analysis.GetInfo(divide);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Divide_Failure_ZeroRHS_U32) {
    Var* idx = nullptr;
    Binary* divide = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4
            idx = b.Var("idx", 4_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 0
                idy = b.Var("idy", 0_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 3
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 3_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // divide = idx / idy
                divide = b.Divide<u32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 4u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 9u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 0u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 3u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:u32 = load %idx
            %9:u32 = load %idy
            %10:u32 = div %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:u32 = load %idx
        %14:u32 = add %13, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `divide` (`idx / idy`)
    // idx: [4u, 8u], idy: [0u, 2u]
    const auto& info = analysis.GetInfo(divide);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftLeft_Success_U32) {
    Var* idx = nullptr;
    Binary* shiftLeft = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4
            idx = b.Var("idx", 4_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 3
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 3_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // shiftLeft = idx << idy
                shiftLeft = b.ShiftLeft<u32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 4u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 9u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 3u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:u32 = load %idx
            %9:u32 = load %idy
            %10:u32 = shl %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:u32 = load %idx
        %14:u32 = add %13, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftLeft` (`idx << idy`)
    // idx: [4u, 8u], idy: [1u, 2u]
    // shiftLeft: [4u << 1u, 8u << 2u] = [8u, 32u]
    const auto& info = analysis.GetInfo(shiftLeft);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(8u, range.min_bound);
    EXPECT_EQ(32u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftLeft_Success_I32_NonZero) {
    Var* idx = nullptr;
    Binary* shiftLeft = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4
            idx = b.Var("idx", 4_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 3
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 3_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // shiftLeft = idx << idy
                shiftLeft = b.ShiftLeft<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 4i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 9i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 3u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:u32 = load %idy
            %10:i32 = shl %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftLeft` (`idx << idy`)
    // idx: [4, 8], idy: [1u, 2u]
    // shiftLeft: [4 << 1u, 8 << 2u] = [8, 32]
    const auto& info = analysis.GetInfo(shiftLeft);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(8, range.min_bound);
    EXPECT_EQ(32, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftLeft_Success_I32_Zero) {
    Var* idx = nullptr;
    Binary* shiftLeft = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 3
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 3_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // shiftLeft = idx << idy
                shiftLeft = b.ShiftLeft<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %4:bool = lt %3, 9i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 3u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:u32 = load %idy
            %10:i32 = shl %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftLeft` (`idx << idy`)
    // idx: [0, 8], idy: [1u, 2u]
    // shiftLeft: [0 << 1u, 8 << 2u] = [0, 32]
    const auto& info = analysis.GetInfo(shiftLeft);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(0, range.min_bound);
    EXPECT_EQ(32, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftLeft_Failure_I32_Negative) {
    Var* idx = nullptr;
    Binary* shiftLeft = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = -1
            idx = b.Var("idx", -1_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 3
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 3_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // shiftLeft = idx << idy
                shiftLeft = b.ShiftLeft<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var -1i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 9i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 3u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:u32 = load %idy
            %10:i32 = shl %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftLeft` (`idx << idy`)
    // idx: [-1, 8], idy: [1u, 2u]
    const auto& info = analysis.GetInfo(shiftLeft);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftLeft_Failure_U32_NoLessThan32) {
    Var* idx = nullptr;
    Binary* shiftLeft = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4
            idx = b.Var("idx", 4_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 33
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 33_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // shiftLeft = idx << idy
                shiftLeft = b.ShiftLeft<u32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 4u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 9u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 33u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:u32 = load %idx
            %9:u32 = load %idy
            %10:u32 = shl %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:u32 = load %idx
        %14:u32 = add %13, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftLeft` (`idx << idy`)
    // idx: [4u, 8u], idy: [1u, 32u]
    const auto& info = analysis.GetInfo(shiftLeft);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftLeft_Failure_I32_NoLessThan32) {
    Var* idx = nullptr;
    Binary* shiftLeft = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 34
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 34_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // shiftLeft = idx << idy
                shiftLeft = b.ShiftLeft<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %4:bool = lt %3, 9i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 34u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:u32 = load %idy
            %10:i32 = shl %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftLeft` (`idx << idy`)
    // idx: [0, 8], idy: [1u, 33u]
    const auto& info = analysis.GetInfo(shiftLeft);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftLeft_Failure_U32_Overflow) {
    Var* idx = nullptr;
    Binary* shiftLeft = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 5
            auto* binary = b.LessThan<bool>(b.Load(idx), 5_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 31
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 31_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // shiftLeft = idx << idy
                shiftLeft = b.ShiftLeft<u32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %4:bool = lt %3, 5u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 31u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:u32 = load %idx
            %9:u32 = load %idy
            %10:u32 = shl %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:u32 = load %idx
        %14:u32 = add %13, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftLeft` (`idx << idy`)
    // idx: [0u, 4u], idy: [1u, 30u]
    // 4 << 30 = 4294967296L > u32::kHighestValue
    const auto& info = analysis.GetInfo(shiftLeft);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftLeft_Failure_U32_HighestValue_Overflow) {
    Var* idx = nullptr;
    Binary* shiftLeft = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 32
            auto* binary = b.LessThan<bool>(b.Load(idx), 32_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            auto* loadx = b.Load(idx);
            shiftLeft = b.ShiftLeft<u32>(u32::Highest(), loadx);
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %4:bool = lt %3, 32u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %5:u32 = load %idx
        %6:u32 = shl 4294967295u, %5
        continue  # -> $B4
      }
      $B4: {  # continuing
        %7:u32 = load %idx
        %8:u32 = add %7, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftLeft` (`u32::HighestValue << idx`)
    // idx: [0u, 31u]
    const auto& info = analysis.GetInfo(shiftLeft);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftLeft_Failure_I32_Overflow) {
    Var* idx = nullptr;
    Binary* shiftLeft = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 5
            auto* binary = b.LessThan<bool>(b.Load(idx), 5_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 30
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 30_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // shiftLeft = idx << idy
                shiftLeft = b.ShiftLeft<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %4:bool = lt %3, 5i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 30u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:u32 = load %idy
            %10:i32 = shl %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftLeft` (`idx << idy`)
    // idx: [0, 4], idy: [1u, 29u]
    // 4 << 29 = 2147483648 > i32::kHighestValue
    const auto& info = analysis.GetInfo(shiftLeft);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftLeft_Failure_I32_HighestValue_Overflow) {
    Var* idx = nullptr;
    Binary* shiftLeft = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 32
            auto* binary = b.LessThan<bool>(b.Load(idx), 32_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            auto* loadx = b.Load(idx);
            shiftLeft = b.ShiftLeft<i32>(i32::Highest(), loadx);
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %4:bool = lt %3, 32u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %5:u32 = load %idx
        %6:i32 = shl 2147483647i, %5
        continue  # -> $B4
      }
      $B4: {  # continuing
        %7:u32 = load %idx
        %8:u32 = add %7, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftLeft` (`i32::HighestValue << idx`)
    // idx: [0u, 31u]
    const auto& info = analysis.GetInfo(shiftLeft);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftRight_Success_U32_NonZero) {
    Var* idx = nullptr;
    Binary* shiftRight = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4
            idx = b.Var("idx", 4_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 3
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 3_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // shiftRight = idx >> idy
                shiftRight = b.ShiftRight<u32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 4u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 9u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 3u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:u32 = load %idx
            %9:u32 = load %idy
            %10:u32 = shr %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:u32 = load %idx
        %14:u32 = add %13, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftRight` (`idx >> idy`)
    // idx: [4u, 8u], idy: [1u, 2u]
    // shiftRight: [4u >> 2u, 8u >> 1u] = [1u, 4u]
    const auto& info = analysis.GetInfo(shiftRight);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(1u, range.min_bound);
    EXPECT_EQ(4u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftRight_Success_I32_NonZero) {
    Var* idx = nullptr;
    Binary* shiftRight = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4
            idx = b.Var("idx", 4_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 3
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 3_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // shiftRight = idx >> idy
                shiftRight = b.ShiftRight<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 4i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 9i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 3u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:u32 = load %idy
            %10:i32 = shr %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftRight` (`idx >> idy`)
    // idx: [4, 8], idy: [1u, 2u]
    // shiftRight: [4 >> 2u, 8 >> 1u] = [1, 4]
    const auto& info = analysis.GetInfo(shiftRight);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(1, range.min_bound);
    EXPECT_EQ(4, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftRight_Success_U32_Zero) {
    Var* idx = nullptr;
    Binary* shiftRight = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4
            idx = b.Var("idx", 4_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 5
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 5_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // shiftRight = idx >> idy
                shiftRight = b.ShiftRight<u32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 4u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 9u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 5u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:u32 = load %idx
            %9:u32 = load %idy
            %10:u32 = shr %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:u32 = load %idx
        %14:u32 = add %13, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftRight` (`idx >> idy`)
    // idx: [4u, 8u], idy: [1u, 4u]
    // shiftRight: [4u >> 4u, 8u >> 1u] = [0u, 4u]
    const auto& info = analysis.GetInfo(shiftRight);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(0u, range.min_bound);
    EXPECT_EQ(4u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftRight_Success_I32_Zero) {
    Var* idx = nullptr;
    Binary* shiftRight = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4
            idx = b.Var("idx", 4_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 4
                idy = b.Var("idy", 4_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 6
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 6_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // shiftRight = idx >> idy
                shiftRight = b.ShiftRight<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var 4i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 9i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 4u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 6u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:u32 = load %idy
            %10:i32 = shr %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftRight` (`idx >> idy`)
    // idx: [4, 8], idy: [4u, 5u]
    // shiftRight: [4 >> 5u, 8 >> 4u] = [0, 0]
    const auto& info = analysis.GetInfo(shiftRight);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(0, range.min_bound);
    EXPECT_EQ(0, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftRight_Failure_I32_Negative) {
    Var* idx = nullptr;
    Binary* shiftRight = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = -4
            idx = b.Var("idx", -4_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 4
                idy = b.Var("idy", 4_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 6
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 6_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // shiftRight = idx >> idy
                shiftRight = b.ShiftRight<i32>(loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var -4i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 9i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 4u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 6u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:u32 = load %idy
            %10:i32 = shr %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftRight` (`idx >> idy`)
    // idx: [-4, 8], idy: [4u, 5u]
    const auto& info = analysis.GetInfo(shiftRight);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftRight_Failure_U32_NoLessThan32) {
    Var* idx = nullptr;
    Binary* shiftRight = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4
            idx = b.Var("idx", 4_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 33
            auto* binary = b.LessThan<bool>(b.Load(idx), 33_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            auto* loadx = b.Load(idx);

            // shiftRight = u32::HighestValue >> idx
            shiftRight = b.ShiftRight<u32>(u32::Highest(), loadx);

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 4u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 33u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %5:u32 = load %idx
        %6:u32 = shr 4294967295u, %5
        continue  # -> $B4
      }
      $B4: {  # continuing
        %7:u32 = load %idx
        %8:u32 = add %7, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftRight` (`u32::kHighestValue >> idx`)
    // idx: [4u, 32u]
    const auto& info = analysis.GetInfo(shiftRight);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, ShiftRight_Failure_I32_NoLessThan32) {
    Var* idx = nullptr;
    Binary* shiftRight = nullptr;
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4
            idx = b.Var("idx", 4_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 33
            auto* binary = b.LessThan<bool>(b.Load(idx), 33_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            auto* loadx = b.Load(idx);

            // shiftRight = i32::Highest() >> idx
            shiftRight = b.ShiftRight<i32>(i32::Highest(), loadx);

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 4u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 33u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %5:u32 = load %idx
        %6:i32 = shr 2147483647i, %5
        continue  # -> $B4
      }
      $B4: {  # continuing
        %7:u32 = load %idx
        %8:u32 = add %7, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `shiftRight` (`i32::kHighestValue >> idx`)
    // idx: [4, 32]
    const auto& info = analysis.GetInfo(shiftRight);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Failure_F32) {
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().f32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        call_min = b.Call<f32>(BuiltinFn::kMin, param, 1.0_f);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:f32):void {
  $B1: {
    %3:f32 = min %param, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(param, 1.0f)`)
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Failure_Vector_I32) {
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().vec4<i32>());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* vec4_const = b.Construct(ty.vec4<i32>(), 1_i, 2_i, 3_i, 4_i);
        call_min = b.Call<vec4<i32>>(BuiltinFn::kMin, param, vec4_const);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:vec4<i32>):void {
  $B1: {
    %3:vec4<i32> = construct 1i, 2i, 3i, 4i
    %4:vec4<i32> = min %param, %3
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(param, vec4i(1, 2, 3, 4))`)
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Failure_Vector_U32) {
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().vec2<u32>());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* vec2_const = b.Construct(ty.vec2<u32>(), 1_u, 2_u);
        call_min = b.Call<vec2<u32>>(BuiltinFn::kMin, param, vec2_const);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:vec2<u32>):void {
  $B1: {
    %3:vec2<u32> = construct 1u, 2u
    %4:vec2<u32> = min %param, %3
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(param, vec2u(1, 2))`)
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Failure_BothInvalidRange_I32) {
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param1 = b.FunctionParam("param1", mod.Types().i32());
    auto* param2 = b.FunctionParam("param2", mod.Types().i32());
    func->AppendParam(param1);
    func->AppendParam(param2);

    b.Append(func->Block(), [&] {
        call_min = b.Call<i32>(BuiltinFn::kMin, param1, param2);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param1:i32, %param2:i32):void {
  $B1: {
    %4:i32 = min %param1, %param2
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(param1, param2)`)
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Failure_BothInvalidRange_U32) {
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param1 = b.FunctionParam("param1", mod.Types().u32());
    auto* param2 = b.FunctionParam("param2", mod.Types().u32());
    func->AppendParam(param1);
    func->AppendParam(param2);

    b.Append(func->Block(), [&] {
        call_min = b.Call<u32>(BuiltinFn::kMin, param1, param2);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param1:u32, %param2:u32):void {
  $B1: {
    %4:u32 = min %param1, %param2
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(param1, param2)`)
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Success_BothAreConstantValues_I32) {
    Var* idx = nullptr;
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = -4
            idx = b.Var("idx", -4_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < -3
            auto* binary = b.LessThan<bool>(b.Load(idx), -3_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 2
                idy = b.Var("idy", 2_i);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 3
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 3_i);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // call_min = min(idx, idy);
                call_min = b.Call<i32>(BuiltinFn::kMin, loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<i32>(b.Load(idy), 1_i));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var -4i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, -3i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, i32, read_write> = var 2i
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:i32 = load %idy
            %7:bool = lt %6, 3i
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:i32 = load %idy
            %10:i32 = min %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:i32 = load %idy
            %12:i32 = add %11, 1i
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(idx, idy)`)
    // idx: [-4, -4] idy: [2, 2]
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(-4, range.min_bound);
    EXPECT_EQ(-4, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Success_BothAreConstantValues_U32) {
    Var* idx = nullptr;
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 5u
            idx = b.Var("idx", 5_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 6u
            auto* binary = b.LessThan<bool>(b.Load(idx), 6_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1u
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 2u
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 2_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // call_min = min(idx, idy);
                call_min = b.Call<u32>(BuiltinFn::kMin, loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 5u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 6u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 2u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:u32 = load %idx
            %9:u32 = load %idy
            %10:u32 = min %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:u32 = load %idx
        %14:u32 = add %13, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(idx, idy)`)
    // idx: [5u, 5u] idy: [1u, 1u]
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(1u, range.min_bound);
    EXPECT_EQ(1u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Success_BothValidRange_I32) {
    Var* idx = nullptr;
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = -4
            idx = b.Var("idx", -4_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 4
                idy = b.Var("idy", 4_i);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 6
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 6_i);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // call_min = min(idx, idy);
                call_min = b.Call<i32>(BuiltinFn::kMin, loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<i32>(b.Load(idy), 1_i));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var -4i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 9i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, i32, read_write> = var 4i
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:i32 = load %idy
            %7:bool = lt %6, 6i
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:i32 = load %idy
            %10:i32 = min %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:i32 = load %idy
            %12:i32 = add %11, 1i
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(idx, idy)`)
    // idx: [-4, 8] idy: [4, 5]
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(-4, range.min_bound);
    EXPECT_EQ(5, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Success_BothValidRange_U32) {
    Var* idx = nullptr;
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4u
            idx = b.Var("idx", 4_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9u
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1u
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 8u
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 8_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // call_min = min(idx, idy);
                call_min = b.Call<u32>(BuiltinFn::kMin, loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 4u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 9u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 8u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:u32 = load %idx
            %9:u32 = load %idy
            %10:u32 = min %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:u32 = load %idx
        %14:u32 = add %13, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(idx, idy)`)
    // idx: [4u, 8u] idy: [1u, 7u]
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(1u, range.min_bound);
    EXPECT_EQ(7u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Failure_FirstIsInvalidRange_InvalidResult_I32) {
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* max_i32 = b.Constant(i32::Highest());
        call_min = b.Call<i32>(BuiltinFn::kMin, param, max_i32);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    %3:i32 = min %param, 2147483647i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(param, idx)`)
    // max_i32 = i32::kHighestValue, param: [i32::kLowestValue, i32::kHighestValue]
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Failure_FirstIsInvalidRange_InvalidResult_U32) {
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().u32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* max_u32 = b.Constant(u32::Highest());
        call_min = b.Call<u32>(BuiltinFn::kMin, param, max_u32);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:u32):void {
  $B1: {
    %3:u32 = min %param, 4294967295u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(param, idx)`)
    // max_u32 = u32::kHighestValue, param: [u32::kLowestValue, u32::kHighestValue]
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Failure_SecondIsInvalidRange_InvalidResult_I32) {
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* max_i32 = b.Constant(i32::Highest());
        call_min = b.Call<i32>(BuiltinFn::kMin, max_i32, param);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    %3:i32 = min 2147483647i, %param
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(idx, param)`)
    // max_i32 = i32::kHighestValue, param: [i32::kLowestValue, i32::kHighestValue]
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Failure_SecondIsInvalidRange_InvalidResult_U32) {
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().u32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* max_u32 = b.Constant(u32::Highest());
        call_min = b.Call<u32>(BuiltinFn::kMin, max_u32, param);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:u32):void {
  $B1: {
    %3:u32 = min 4294967295u, %param
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(idx, param)`)
    // max_u32 = u32::kHighestValue, param: [u32::kLowestValue, u32::kHighestValue]
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Success_FirstIsInvalidRange_I32) {
    Var* idx = nullptr;
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 1
            idx = b.Var("idx", 1_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            // call_min = min(param, idx);
            auto* loadx = b.Load(idx);
            call_min = b.Call<i32>(BuiltinFn::kMin, param, loadx);

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 1i
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
        %6:i32 = load %idx
        %7:i32 = min %param, %6
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(param, idx)`)
    // idx: [1, 9], param: [i32::kLowestValue, i32::kHighestValue]
    // call_min: [i32::kLowestValue, 9]
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(i32::kLowestValue, range.min_bound);
    EXPECT_EQ(9, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Success_FirstIsInvalidRange_U32) {
    Var* idx = nullptr;
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().u32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 1u
            idx = b.Var("idx", 1_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10u
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            // call_min = min(param, idx);
            auto* loadx = b.Load(idx);
            call_min = b.Call<u32>(BuiltinFn::kMin, param, loadx);

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
            b.Store(idx, b.Add<u32>(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:u32):void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 1u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:u32 = load %idx
        %5:bool = lt %4, 10u
        if %5 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %6:u32 = load %idx
        %7:u32 = min %param, %6
        continue  # -> $B4
      }
      $B4: {  # continuing
        %8:u32 = load %idx
        %9:u32 = add %8, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(param, idx)`)
    // idx: [1, 9], param: [u32::kLowestValue, u32::kHighestValue]
    // call_min: [u32::kLowestValue, 9]
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(u32::kLowestValue, range.min_bound);
    EXPECT_EQ(9u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Success_SecondIsInvalidRange_I32) {
    Var* idx = nullptr;
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = -10
            idx = b.Var("idx", -10_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < -2
            auto* binary = b.LessThan<bool>(b.Load(idx), -2_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            // call_min = min(idx, param);
            auto* loadx = b.Load(idx);
            call_min = b.Call<i32>(BuiltinFn::kMin, loadx, param);

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var -10i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:i32 = load %idx
        %5:bool = lt %4, -2i
        if %5 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %6:i32 = load %idx
        %7:i32 = min %6, %param
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(idx, param)`)
    // idx: [-10, -3], param: [i32::kLowestValue, i32::kHighestValue]
    // call_min: [i32::kLowestValue, -3]
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(i32::kLowestValue, range.min_bound);
    EXPECT_EQ(-3, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Success_SecondIsInvalidRange_U32) {
    Var* idx = nullptr;
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().u32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 10u
            idx = b.Var("idx", 10_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 20u
            auto* binary = b.LessThan<bool>(b.Load(idx), 20_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            // call_min = min(idx, param);
            auto* loadx = b.Load(idx);
            call_min = b.Call<u32>(BuiltinFn::kMin, loadx, param);

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
            b.Store(idx, b.Add<u32>(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:u32):void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 10u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:u32 = load %idx
        %5:bool = lt %4, 20u
        if %5 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %6:u32 = load %idx
        %7:u32 = min %6, %param
        continue  # -> $B4
      }
      $B4: {  # continuing
        %8:u32 = load %idx
        %9:u32 = add %8, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(idx, param)`)
    // idx: [10, 19], param: [u32::kLowestValue, u32::kHighestValue]
    // call_min: [u32::kLowestValue, 19]
    const auto& info = analysis.GetInfo(call_min);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(u32::kLowestValue, range.min_bound);
    EXPECT_EQ(19u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Failure_F32) {
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().f32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        call_max = b.Call<f32>(BuiltinFn::kMax, param, 1.0_f);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:f32):void {
  $B1: {
    %3:f32 = max %param, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(param, 1.0f)`)
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Failure_Vector_I32) {
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().vec4<i32>());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* vec4_const = b.Construct(ty.vec4<i32>(), 1_i, 2_i, 3_i, 4_i);
        call_max = b.Call<vec4<i32>>(BuiltinFn::kMax, param, vec4_const);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:vec4<i32>):void {
  $B1: {
    %3:vec4<i32> = construct 1i, 2i, 3i, 4i
    %4:vec4<i32> = max %param, %3
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(param, vec4i(1, 2, 3, 4))`)
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Failure_Vector_U32) {
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().vec2<u32>());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* vec2_const = b.Construct(ty.vec2<u32>(), 1_u, 2_u);
        call_max = b.Call<vec2<u32>>(BuiltinFn::kMax, param, vec2_const);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:vec2<u32>):void {
  $B1: {
    %3:vec2<u32> = construct 1u, 2u
    %4:vec2<u32> = max %param, %3
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(param, vec2u(1, 2))`)
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Failure_BothInvalidRange_I32) {
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param1 = b.FunctionParam("param1", mod.Types().i32());
    auto* param2 = b.FunctionParam("param2", mod.Types().i32());
    func->AppendParam(param1);
    func->AppendParam(param2);

    b.Append(func->Block(), [&] {
        call_max = b.Call<i32>(BuiltinFn::kMax, param1, param2);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param1:i32, %param2:i32):void {
  $B1: {
    %4:i32 = max %param1, %param2
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(param1, param2)`)
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Failure_BothInvalidRange_U32) {
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param1 = b.FunctionParam("param1", mod.Types().u32());
    auto* param2 = b.FunctionParam("param2", mod.Types().u32());
    func->AppendParam(param1);
    func->AppendParam(param2);

    b.Append(func->Block(), [&] {
        call_max = b.Call<u32>(BuiltinFn::kMax, param1, param2);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param1:u32, %param2:u32):void {
  $B1: {
    %4:u32 = max %param1, %param2
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(param1, param2)`)
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Success_BothAreConstantValues_I32) {
    Var* idx = nullptr;
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = -4
            idx = b.Var("idx", -4_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < -3
            auto* binary = b.LessThan<bool>(b.Load(idx), -3_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 2
                idy = b.Var("idy", 2_i);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 3
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 3_i);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // call_max = max(idx, idy);
                call_max = b.Call<i32>(BuiltinFn::kMax, loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<i32>(b.Load(idy), 1_i));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var -4i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, -3i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, i32, read_write> = var 2i
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:i32 = load %idy
            %7:bool = lt %6, 3i
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:i32 = load %idy
            %10:i32 = max %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:i32 = load %idy
            %12:i32 = add %11, 1i
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(idx, idy)`)
    // idx: [-4, -4] idy: [2, 2]
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(2, range.min_bound);
    EXPECT_EQ(2, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Success_BothAreConstantValues_U32) {
    Var* idx = nullptr;
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 5u
            idx = b.Var("idx", 5_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 6u
            auto* binary = b.LessThan<bool>(b.Load(idx), 6_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1u
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 2u
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 2_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // call_max = max(idx, idy);
                call_max = b.Call<u32>(BuiltinFn::kMax, loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 5u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 6u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 2u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:u32 = load %idx
            %9:u32 = load %idy
            %10:u32 = max %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:u32 = load %idx
        %14:u32 = add %13, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(idx, idy)`)
    // idx: [5u, 5u] idy: [1u, 1u]
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(5u, range.min_bound);
    EXPECT_EQ(5u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Success_BothValidRange_I32) {
    Var* idx = nullptr;
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = -4
            idx = b.Var("idx", -4_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 4
                idy = b.Var("idy", 4_i);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 6
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 6_i);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // call_max = max(idx, idy);
                call_max = b.Call<i32>(BuiltinFn::kMax, loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<i32>(b.Load(idy), 1_i));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, i32, read_write> = var -4i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %idx
        %4:bool = lt %3, 9i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, i32, read_write> = var 4i
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:i32 = load %idy
            %7:bool = lt %6, 6i
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:i32 = load %idx
            %9:i32 = load %idy
            %10:i32 = max %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:i32 = load %idy
            %12:i32 = add %11, 1i
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:i32 = load %idx
        %14:i32 = add %13, 1i
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(idx, idy)`)
    // idx: [-4, 8] idy: [4, 5]
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(4, range.min_bound);
    EXPECT_EQ(8, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Success_BothValidRange_U32) {
    Var* idx = nullptr;
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 4u
            idx = b.Var("idx", 4_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 9u
            auto* binary = b.LessThan<bool>(b.Load(idx), 9_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            Var* idy = nullptr;
            auto* loop2 = b.Loop();
            b.Append(loop2->Initializer(), [&] {
                // idy = 1u
                idy = b.Var("idy", 1_u);
                b.NextIteration(loop2);
            });
            b.Append(loop2->Body(), [&] {
                // idy < 8u
                auto* binary_inner = b.LessThan<bool>(b.Load(idy), 8_u);
                auto* ifelse_inner = b.If(binary_inner);
                b.Append(ifelse_inner->True(), [&] { b.ExitIf(ifelse_inner); });
                b.Append(ifelse_inner->False(), [&] { b.ExitLoop(loop2); });
                auto* loadx = b.Load(idx);
                auto* loady = b.Load(idy);
                // call_max = max(idx, idy);
                call_max = b.Call<u32>(BuiltinFn::kMax, loadx, loady);
                b.Continue(loop2);
            });
            b.Append(loop2->Continuing(), [&] {
                // idy++
                b.Store(idy, b.Add<u32>(b.Load(idy), 1_u));
                b.NextIteration(loop2);
            });

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
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
        %idx:ptr<function, u32, read_write> = var 4u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 9u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        loop [i: $B7, b: $B8, c: $B9] {  # loop_2
          $B7: {  # initializer
            %idy:ptr<function, u32, read_write> = var 1u
            next_iteration  # -> $B8
          }
          $B8: {  # body
            %6:u32 = load %idy
            %7:bool = lt %6, 8u
            if %7 [t: $B10, f: $B11] {  # if_2
              $B10: {  # true
                exit_if  # if_2
              }
              $B11: {  # false
                exit_loop  # loop_2
              }
            }
            %8:u32 = load %idx
            %9:u32 = load %idy
            %10:u32 = max %8, %9
            continue  # -> $B9
          }
          $B9: {  # continuing
            %11:u32 = load %idy
            %12:u32 = add %11, 1u
            store %idy, %12
            next_iteration  # -> $B8
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:u32 = load %idx
        %14:u32 = add %13, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(idx, idy)`)
    // idx: [4u, 8u] idy: [1u, 7u]
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(4u, range.min_bound);
    EXPECT_EQ(8u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Failure_FirstIsInvalidRange_InvalidResult_I32) {
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* min_i32 = b.Constant(i32::Lowest());
        call_max = b.Call<i32>(BuiltinFn::kMax, param, min_i32);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    %3:i32 = max %param, -2147483648i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(param, min_i32)`)
    // min_i32 = i32::kLowestValue, param: [i32::kLowestValue, i32::kHighestValue]
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Failure_FirstIsInvalidRange_InvalidResult_U32) {
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().u32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* min_u32 = b.Constant(u32::Lowest());
        call_max = b.Call<u32>(BuiltinFn::kMax, param, min_u32);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:u32):void {
  $B1: {
    %3:u32 = max %param, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(param, min_u32)`)
    // min_u32 = u32::kLowestValue, param: [u32::kLowestValue, u32::kHighestValue]
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Failure_SecondIsInvalidRange_InvalidResult_I32) {
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* min_i32 = b.Constant(i32::Lowest());
        call_max = b.Call<i32>(BuiltinFn::kMax, min_i32, param);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    %3:i32 = max -2147483648i, %param
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(min_i32, param)`)
    // min_i32 = i32::kLowestValue, param: [i32::kLowestValue, i32::kHighestValue]
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Failure_SecondIsInvalidRange_InvalidResult_U32) {
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().u32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* min_u32 = b.Constant(u32::Lowest());
        call_max = b.Call<u32>(BuiltinFn::kMax, min_u32, param);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:u32):void {
  $B1: {
    %3:u32 = max 0u, %param
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(min_u32, param)`)
    // min_u32 = u32::kHighestValue, param: [u32::kLowestValue, u32::kHighestValue]
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Success_FirstIsInvalidRange_I32) {
    Var* idx = nullptr;
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 1
            idx = b.Var("idx", 1_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            // call_max = max(param, idx);
            auto* loadx = b.Load(idx);
            call_max = b.Call<i32>(BuiltinFn::kMax, param, loadx);

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 1i
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
        %6:i32 = load %idx
        %7:i32 = max %param, %6
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(param, idx)`)
    // idx: [1, 9], param: [i32::kLowestValue, i32::kHighestValue]
    // call_max: [1, i32::kHighestValue]
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(1, range.min_bound);
    EXPECT_EQ(i32::kHighestValue, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Success_FirstIsInvalidRange_U32) {
    Var* idx = nullptr;
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().u32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 1u
            idx = b.Var("idx", 1_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 10u
            auto* binary = b.LessThan<bool>(b.Load(idx), 10_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            // call_max = max(param, idx);
            auto* loadx = b.Load(idx);
            call_max = b.Call<u32>(BuiltinFn::kMax, param, loadx);

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
            b.Store(idx, b.Add<u32>(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:u32):void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 1u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:u32 = load %idx
        %5:bool = lt %4, 10u
        if %5 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %6:u32 = load %idx
        %7:u32 = max %param, %6
        continue  # -> $B4
      }
      $B4: {  # continuing
        %8:u32 = load %idx
        %9:u32 = add %8, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(param, idx)`)
    // idx: [1, 9], param: [u32::kLowestValue, u32::kHighestValue]
    // call_max: [1, u32::kHighestValue]
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(1u, range.min_bound);
    EXPECT_EQ(u32::kHighestValue, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Success_SecondIsInvalidRange_I32) {
    Var* idx = nullptr;
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = -10
            idx = b.Var("idx", -10_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < -2
            auto* binary = b.LessThan<bool>(b.Load(idx), -2_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            // call_max = max(idx, param);
            auto* loadx = b.Load(idx);
            call_max = b.Call<i32>(BuiltinFn::kMax, loadx, param);

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var -10i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:i32 = load %idx
        %5:bool = lt %4, -2i
        if %5 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %6:i32 = load %idx
        %7:i32 = max %6, %param
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(idx, param)`)
    // idx: [-10, -3], param: [i32::kLowestValue, i32::kHighestValue]
    // call_max: [-10, i32::kHighestValue]
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(-10, range.min_bound);
    EXPECT_EQ(i32::kHighestValue, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Success_SecondIsInvalidRange_U32) {
    Var* idx = nullptr;
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().u32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 10u
            idx = b.Var("idx", 10_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 20u
            auto* binary = b.LessThan<bool>(b.Load(idx), 20_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            // call_max = max(idx, param);
            auto* loadx = b.Load(idx);
            call_max = b.Call<u32>(BuiltinFn::kMax, loadx, param);

            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
            b.Store(idx, b.Add<u32>(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:u32):void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 10u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:u32 = load %idx
        %5:bool = lt %4, 20u
        if %5 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %6:u32 = load %idx
        %7:u32 = max %6, %param
        continue  # -> $B4
      }
      $B4: {  # continuing
        %8:u32 = load %idx
        %9:u32 = add %8, 1u
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

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(idx, param)`)
    // idx: [10, 19], param: [u32::kLowestValue, u32::kHighestValue]
    // call_max: [10, u32::kHighestValue]
    const auto& info = analysis.GetInfo(call_max);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(10u, range.min_bound);
    EXPECT_EQ(u32::kHighestValue, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Builtin_Input_Success_I32) {
    CoreBuiltinCall* call_min = nullptr;
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* bound1 = b.Constant(-5_i);
        call_min = b.Call<i32>(BuiltinFn::kMin, bound1, param);
        auto* bound2 = b.Constant(3_i);
        call_max = b.Call<i32>(BuiltinFn::kMax, bound2, call_min);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    %3:i32 = min -5i, %param
    %4:i32 = max 3i, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(bound1, param)`)
    // bound1: [-5, -5] param: [i32::kLowestValue, i32::kHighestValue]
    // call_min: [i32::kLowestValue, -5]
    const auto& info_call_min = analysis.GetInfo(call_min);
    ASSERT_TRUE(info_call_min.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info_call_min.range));
    const auto& range_call_min =
        std::get<IntegerRangeInfo::SignedIntegerRange>(info_call_min.range);
    EXPECT_EQ(i32::kLowestValue, range_call_min.min_bound);
    EXPECT_EQ(-5, range_call_min.max_bound);

    // Range of `call_max` (`max(bound2, call_min)`)
    // bound2: [3, 3] call_min: [i32::kLowestValue, -5]
    // call_max: [3, 3]
    const auto& info_call_max = analysis.GetInfo(call_max);
    ASSERT_TRUE(info_call_max.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info_call_max.range));
    const auto& range_call_max =
        std::get<IntegerRangeInfo::SignedIntegerRange>(info_call_max.range);
    EXPECT_EQ(3, range_call_max.min_bound);
    EXPECT_EQ(3, range_call_max.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Max_Builtin_Input_Success_U32) {
    CoreBuiltinCall* call_min = nullptr;
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().u32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* bound1 = b.Constant(5_u);
        call_min = b.Call<u32>(BuiltinFn::kMin, bound1, param);
        auto* bound2 = b.Constant(3_u);
        call_max = b.Call<u32>(BuiltinFn::kMax, bound2, call_min);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:u32):void {
  $B1: {
    %3:u32 = min 5u, %param
    %4:u32 = max 3u, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(bound1, param)`)
    // bound1: [5u, 5u] param: [u32::kLowestValue, u32::kHighestValue]
    // call_min: [u32::kLowestValue, 5u]
    const auto& info_call_min = analysis.GetInfo(call_min);
    ASSERT_TRUE(info_call_min.IsValid());
    ASSERT_TRUE(
        std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info_call_min.range));
    const auto& range_call_min =
        std::get<IntegerRangeInfo::UnsignedIntegerRange>(info_call_min.range);
    EXPECT_EQ(u32::kLowestValue, range_call_min.min_bound);
    EXPECT_EQ(5u, range_call_min.max_bound);

    // Range of `call_max` (`max(bound2, call_min)`)
    // bound2: [3u, 3u] call_min: [u32::kLowestValue, 5u]
    // call_max: [3u, 5u]
    const auto& info_call_max = analysis.GetInfo(call_max);
    ASSERT_TRUE(info_call_max.IsValid());
    ASSERT_TRUE(
        std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info_call_max.range));
    const auto& range_call_max =
        std::get<IntegerRangeInfo::UnsignedIntegerRange>(info_call_max.range);
    EXPECT_EQ(3u, range_call_max.min_bound);
    EXPECT_EQ(5u, range_call_max.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Builtin_Input_Success_I32) {
    CoreBuiltinCall* call_min = nullptr;
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* bound1 = b.Constant(-3_i);
        call_max = b.Call<i32>(BuiltinFn::kMax, bound1, param);
        auto* bound2 = b.Constant(5_i);
        call_min = b.Call<i32>(BuiltinFn::kMin, bound2, call_max);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    %3:i32 = max -3i, %param
    %4:i32 = min 5i, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(bound1, param)`)
    // bound1: [-3, -3] param: [i32::kLowestValue, i32::kHighestValue]
    // call_max: [-3, i32::kHighestValue]
    const auto& info_call_max = analysis.GetInfo(call_max);
    ASSERT_TRUE(info_call_max.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info_call_max.range));
    const auto& range_call_max =
        std::get<IntegerRangeInfo::SignedIntegerRange>(info_call_max.range);
    EXPECT_EQ(-3, range_call_max.min_bound);
    EXPECT_EQ(i32::kHighestValue, range_call_max.max_bound);

    // Range of `call_min` (`min(bound2, call_max)`)
    // bound2: [5, 5] call_max: [-3, i32::kHighestValue]
    // call_min: [-3, 5]
    const auto& info_call_min = analysis.GetInfo(call_min);
    ASSERT_TRUE(info_call_min.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info_call_min.range));
    const auto& range_call_min =
        std::get<IntegerRangeInfo::SignedIntegerRange>(info_call_min.range);
    EXPECT_EQ(-3, range_call_min.min_bound);
    EXPECT_EQ(5, range_call_min.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, Builtin_Min_Builtin_Input_Success_U32) {
    CoreBuiltinCall* call_min = nullptr;
    CoreBuiltinCall* call_max = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().u32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* bound1 = b.Constant(5_u);
        call_max = b.Call<u32>(BuiltinFn::kMax, bound1, param);
        auto* bound2 = b.Constant(3_u);
        call_min = b.Call<u32>(BuiltinFn::kMin, bound2, call_max);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:u32):void {
  $B1: {
    %3:u32 = max 5u, %param
    %4:u32 = min 3u, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_max` (`max(bound1, param)`)
    // bound1: [5u, 5u] param: [u32::kLowestValue, u32::kHighestValue]
    // call_max: [5, u32::kHighestValue]
    const auto& info_call_max = analysis.GetInfo(call_max);
    ASSERT_TRUE(info_call_max.IsValid());
    ASSERT_TRUE(
        std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info_call_max.range));
    const auto& range_call_max =
        std::get<IntegerRangeInfo::UnsignedIntegerRange>(info_call_max.range);
    EXPECT_EQ(5u, range_call_max.min_bound);
    EXPECT_EQ(u32::kHighestValue, range_call_max.max_bound);

    // Range of `call_min` (`min(bound2, call_max)`)
    // bound2: [3u, 3u] call_max: [5u, u32::kHighestValue]
    // call_min: [3u, 3u]
    const auto& info_call_min = analysis.GetInfo(call_min);
    ASSERT_TRUE(info_call_min.IsValid());
    ASSERT_TRUE(
        std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info_call_min.range));
    const auto& range_call_min =
        std::get<IntegerRangeInfo::UnsignedIntegerRange>(info_call_min.range);
    EXPECT_EQ(3u, range_call_min.min_bound);
    EXPECT_EQ(3u, range_call_min.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryModulo_Failure_LHS_RHS_F32) {
    Binary* modulo = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().f32());
    func->AppendParam(param);
    b.Append(func->Block(), [&] {
        // modulo = param % 2.0f
        modulo = b.Modulo<f32>(param, 2_f);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:f32):void {
  $B1: {
    %3:f32 = mod %param, 2.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    // Range of `modulo` (`param % 2.0f`)
    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(modulo);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryModulo_Failure_LHS_RHS_Vec4I) {
    Binary* modulo = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().vec4<i32>());
    func->AppendParam(param);
    b.Append(func->Block(), [&] {
        auto* vec4_const = b.Construct(ty.vec4<i32>(), 1_i, 2_i, 3_i, 4_i);
        // modulo = param %
        modulo = b.Modulo<vec4<i32>>(param, vec4_const);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:vec4<i32>):void {
  $B1: {
    %3:vec4<i32> = construct 1i, 2i, 3i, 4i
    %4:vec4<i32> = mod %param, %3
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    // Range of `modulo` (`param % vec4i(1, 2, 3, 4)`)
    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(modulo);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryModulo_Failure_LHS_Negative_I32) {
    Binary* modulo = nullptr;
    CoreBuiltinCall* call_min = nullptr;

    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        // call_max = max(-3, param)
        auto* call_max = b.Call<i32>(BuiltinFn::kMax, b.Constant(-3_i), param);

        // call_min = min(5, call_max)
        // The range of call_min is [-3, 5]
        call_min = b.Call<i32>(BuiltinFn::kMin, b.Constant(5_i), call_max);

        // modulo = call_min % 2
        modulo = b.Modulo<i32>(call_min, 2_i);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    %3:i32 = max -3i, %param
    %4:i32 = min 5i, %3
    %5:i32 = mod %4, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min` (`min(5, max(-3, param))`)
    const auto& info_call_min = analysis.GetInfo(call_min);
    ASSERT_TRUE(info_call_min.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info_call_min.range));
    const auto& range_call_min =
        std::get<IntegerRangeInfo::SignedIntegerRange>(info_call_min.range);
    EXPECT_EQ(-3, range_call_min.min_bound);
    EXPECT_EQ(5, range_call_min.max_bound);

    // Range of `modulo` (`param % call_min`)
    const auto& info_mod = analysis.GetInfo(modulo);
    ASSERT_FALSE(info_mod.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryModulo_Failure_RHS_NonConstant_I32) {
    Binary* modulo = nullptr;
    CoreBuiltinCall* call_min_param1 = nullptr;
    CoreBuiltinCall* call_min_param2 = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param1 = b.FunctionParam("param1", mod.Types().i32());
    auto* param2 = b.FunctionParam("param2", mod.Types().i32());

    func->AppendParam(param1);
    func->AppendParam(param2);
    b.Append(func->Block(), [&] {
        // call_max_param1 = max(3, param1)
        auto* call_max_param1 = b.Call<i32>(BuiltinFn::kMax, b.Constant(3_i), param1);
        // call_min_param1 = min(6, call_max_param1)
        // The range of call_min_param1 is [3, 6]
        call_min_param1 = b.Call<i32>(BuiltinFn::kMin, b.Constant(6_i), call_max_param1);

        // call_max_param2 = max(2, param2)
        auto* call_max_param2 = b.Call<i32>(BuiltinFn::kMax, b.Constant(2_i), param2);
        // call_min_param2 = min(4, call_max_param2)
        // The range of call_min is [2, 4]
        call_min_param2 = b.Call<i32>(BuiltinFn::kMin, b.Constant(4_i), call_max_param2);

        // modulo = call_min_param1 % call_min_param1
        modulo = b.Modulo<i32>(call_min_param1, call_min_param2);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param1:i32, %param2:i32):void {
  $B1: {
    %4:i32 = max 3i, %param1
    %5:i32 = min 6i, %4
    %6:i32 = max 2i, %param2
    %7:i32 = min 4i, %6
    %8:i32 = mod %5, %7
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min_param2` (`min(4, max(2, param))`)
    const auto& info_call_min_param2 = analysis.GetInfo(call_min_param2);
    ASSERT_TRUE(info_call_min_param2.IsValid());
    ASSERT_TRUE(
        std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info_call_min_param2.range));
    const auto& range_call_min_param2 =
        std::get<IntegerRangeInfo::SignedIntegerRange>(info_call_min_param2.range);
    EXPECT_EQ(2, range_call_min_param2.min_bound);
    EXPECT_EQ(4, range_call_min_param2.max_bound);

    // Range of `modulo` (`param % call_min_param2`)
    const auto& info_mod = analysis.GetInfo(modulo);
    ASSERT_FALSE(info_mod.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryModulo_Failure_RHS_NonConstant_U32) {
    Binary* modulo = nullptr;
    CoreBuiltinCall* call_min_param1 = nullptr;
    CoreBuiltinCall* call_min_param2 = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param1 = b.FunctionParam("param1", mod.Types().u32());
    auto* param2 = b.FunctionParam("param2", mod.Types().u32());

    func->AppendParam(param1);
    func->AppendParam(param2);
    b.Append(func->Block(), [&] {
        // call_max_param1 = max(3, param1)
        auto* call_max_param1 = b.Call<u32>(BuiltinFn::kMax, b.Constant(3_u), param1);
        // call_min_param1 = min(6, call_max_param1)
        // The range of call_min_param1 is [3, 6]
        call_min_param1 = b.Call<u32>(BuiltinFn::kMin, b.Constant(6_u), call_max_param1);

        // call_max_param2 = max(2, param2)
        auto* call_max_param2 = b.Call<u32>(BuiltinFn::kMax, b.Constant(2_u), param2);
        // call_min_param2 = min(4, call_max_param2)
        // The range of call_min is [2, 4]
        call_min_param2 = b.Call<u32>(BuiltinFn::kMin, b.Constant(4_u), call_max_param2);

        // modulo = call_min_param1 % call_min_param2
        modulo = b.Modulo<u32>(call_min_param1, call_min_param2);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param1:u32, %param2:u32):void {
  $B1: {
    %4:u32 = max 3u, %param1
    %5:u32 = min 6u, %4
    %6:u32 = max 2u, %param2
    %7:u32 = min 4u, %6
    %8:u32 = mod %5, %7
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    IntegerRangeAnalysis analysis(&mod);

    // Range of `call_min_param2` (`min(4, max(2, param))`)
    const auto& info_call_min_param2 = analysis.GetInfo(call_min_param2);
    ASSERT_TRUE(info_call_min_param2.IsValid());
    ASSERT_TRUE(
        std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info_call_min_param2.range));
    const auto& range_call_min_param2 =
        std::get<IntegerRangeInfo::UnsignedIntegerRange>(info_call_min_param2.range);
    EXPECT_EQ(2u, range_call_min_param2.min_bound);
    EXPECT_EQ(4u, range_call_min_param2.max_bound);

    // Range of `modulo` (`param % call_min_param2`)
    const auto& info_mod = analysis.GetInfo(modulo);
    ASSERT_FALSE(info_mod.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryModulo_Failure_RHS_Negative_I32) {
    Binary* modulo = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);
    b.Append(func->Block(), [&] {
        // call_max = max(3, param)
        auto* call_max = b.Call<i32>(BuiltinFn::kMax, b.Constant(3_i), param);
        // call_min = min(6, call_max)
        // The range of call_min is [3, 6]
        auto* call_min = b.Call<i32>(BuiltinFn::kMin, b.Constant(6_i), call_max);

        // modulo = call_min % (-6)
        modulo = b.Modulo<i32>(call_min, b.Constant(-6_i));
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    %3:i32 = max 3i, %param
    %4:i32 = min 6i, %3
    %5:i32 = mod %4, -6i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    // Range of `modulo` (`call_min % (-6)`)
    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(modulo);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryModulo_Failure_RHS_Zero_I32) {
    Binary* modulo = nullptr;
    Var* idx = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);
    b.Append(func->Block(), [&] {
        // call_max = max(3, param)
        auto* call_max = b.Call<i32>(BuiltinFn::kMax, b.Constant(3_i), param);
        // call_min = min(6, call_max)
        // The range of call_min is [3, 6]
        auto* call_min = b.Call<i32>(BuiltinFn::kMin, b.Constant(6_i), call_max);

        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0
            idx = b.Var("idx", 0_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 1
            auto* binary = b.LessThan<bool>(b.Load(idx), 1_i);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            // modulo = call_min % idx
            modulo = b.Modulo<i32>(call_min, b.Load(idx));
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
            b.Store(idx, b.Add<i32>(b.Load(idx), 1_i));
            b.NextIteration(loop);
        });

        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    %3:i32 = max 3i, %param
    %4:i32 = min 6i, %3
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, i32, read_write> = var 0i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %6:i32 = load %idx
        %7:bool = lt %6, 1i
        if %7 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %8:i32 = load %idx
        %9:i32 = mod %4, %8
        continue  # -> $B4
      }
      $B4: {  # continuing
        %10:i32 = load %idx
        %11:i32 = add %10, 1i
        store %idx, %11
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    // The range of idx is [0, 0]
    IntegerRangeAnalysis analysis(&mod);
    const auto& info_idx = analysis.GetInfo(idx);
    ASSERT_TRUE(info_idx.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info_idx.range));
    const auto& range_idx = std::get<IntegerRangeInfo::SignedIntegerRange>(info_idx.range);
    EXPECT_EQ(0, range_idx.min_bound);
    EXPECT_EQ(0, range_idx.max_bound);

    // Range of `modulo` (`call_min % idx`)
    const auto& info_modulo = analysis.GetInfo(modulo);
    ASSERT_FALSE(info_modulo.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryModulo_Failure_RHS_Zero_U32) {
    Binary* modulo = nullptr;
    Var* idx = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().u32());
    func->AppendParam(param);
    b.Append(func->Block(), [&] {
        // call_max = max(3u, param)
        auto* call_max = b.Call<u32>(BuiltinFn::kMax, b.Constant(3_u), param);
        // call_min = min(6u, call_max)
        // The range of call_min is [3u, 6u]
        auto* call_min = b.Call<u32>(BuiltinFn::kMin, b.Constant(6_u), call_max);

        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            // idx = 0u
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            // idx < 1u
            auto* binary = b.LessThan<bool>(b.Load(idx), 1_u);
            auto* ifelse = b.If(binary);
            b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
            b.Append(ifelse->False(), [&] { b.ExitLoop(loop); });

            // modulo = call_min % idx
            modulo = b.Modulo<u32>(call_min, b.Load(idx));
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            // idx++
            b.Store(idx, b.Add<u32>(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });

        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:u32):void {
  $B1: {
    %3:u32 = max 3u, %param
    %4:u32 = min 6u, %3
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %6:u32 = load %idx
        %7:bool = lt %6, 1u
        if %7 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %8:u32 = load %idx
        %9:u32 = mod %4, %8
        continue  # -> $B4
      }
      $B4: {  # continuing
        %10:u32 = load %idx
        %11:u32 = add %10, 1u
        store %idx, %11
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    // The range of idx is [0u, 0u]
    IntegerRangeAnalysis analysis(&mod);
    const auto& info_idx = analysis.GetInfo(idx);
    ASSERT_TRUE(info_idx.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info_idx.range));
    const auto& range_idx = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info_idx.range);
    EXPECT_EQ(0u, range_idx.min_bound);
    EXPECT_EQ(0u, range_idx.max_bound);

    // Range of `modulo` (`call_min % idx`)
    const auto& info_modulo = analysis.GetInfo(modulo);
    ASSERT_FALSE(info_modulo.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryModulo_Failure_LHS_NoRange_I32) {
    Binary* modulo = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);
    b.Append(func->Block(), [&] {
        // modulo = param % 3
        modulo = b.Modulo<i32>(param, 3_i);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    %3:i32 = mod %param, 3i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    // Range of `param` is [i32::kLowestValue, i32::kHighestValue]
    // Range of `modulo` (`param % 3i`)
    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(modulo);
    ASSERT_FALSE(info.IsValid());
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryModulo_Success_LHS_NoRange_U32) {
    Binary* modulo = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().u32());
    func->AppendParam(param);
    b.Append(func->Block(), [&] {
        // modulo = param % 3u
        modulo = b.Modulo<u32>(param, 3_u);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:u32):void {
  $B1: {
    %3:u32 = mod %param, 3u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    // Range of `param` is [u32::kLowestValue, u32::kHighestValue]
    // Range of `modulo` (`param % 3u`) is [0u, 2u]
    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(modulo);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(0u, range.min_bound);
    EXPECT_EQ(2u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest,
       BinaryModulo_Success_MinLHS_Divide_RHS_LessThan_MaxLHS_Divide_RHS_I32) {
    Binary* modulo = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);
    b.Append(func->Block(), [&] {
        // call_max = max(6, param)
        auto* call_max = b.Call<i32>(BuiltinFn::kMax, b.Constant(6_i), param);
        // call_min = min(12, call_max)
        // The range of call_min is [6, 12]
        auto* call_min = b.Call<i32>(BuiltinFn::kMin, b.Constant(12_i), call_max);

        // modulo = call_min % 5
        modulo = b.Modulo<i32>(call_min, b.Constant(5_i));
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    %3:i32 = max 6i, %param
    %4:i32 = min 12i, %3
    %5:i32 = mod %4, 5i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    // Range of `call_min` is [6, 12]
    // Range of `modulo` (`call_min % 5`) is [0, 4] (10 % 5 == 0, and 9 % 5 == 4)
    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(modulo);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(0, range.min_bound);
    EXPECT_EQ(4, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest,
       BinaryModulo_Success_MinLHS_Divide_RHS_LessThan_MaxLHS_Divide_RHS_U32) {
    Binary* modulo = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().u32());
    func->AppendParam(param);
    b.Append(func->Block(), [&] {
        // call_max = max(6, param)
        auto* call_max = b.Call<u32>(BuiltinFn::kMax, b.Constant(6_u), param);
        // call_min = min(12, call_max)
        // The range of call_min is [6, 12]
        auto* call_min = b.Call<u32>(BuiltinFn::kMin, b.Constant(12_u), call_max);

        // modulo = call_min % 6
        modulo = b.Modulo<u32>(call_min, b.Constant(6_u));
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:u32):void {
  $B1: {
    %3:u32 = max 6u, %param
    %4:u32 = min 12u, %3
    %5:u32 = mod %4, 6u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    // Range of `call_min` is [6u, 12u]
    // Range of `modulo` (`call_min % 6u`) is [0u, 5u] (12 % 6 == 0, and 11 % 6 == 5)
    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(modulo);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(0u, range.min_bound);
    EXPECT_EQ(5u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest,
       BinaryModulo_Success_MinLHS_Divide_RHS_Equal_MaxLHS_Divide_RHS_I32) {
    Binary* modulo = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);
    b.Append(func->Block(), [&] {
        // call_max = max(8, param)
        auto* call_max = b.Call<i32>(BuiltinFn::kMax, b.Constant(8_i), param);
        // call_min = min(12, call_max)
        // The range of call_min is [8, 12]
        auto* call_min = b.Call<i32>(BuiltinFn::kMin, b.Constant(12_i), call_max);

        // modulo = call_min % 7
        modulo = b.Modulo<i32>(call_min, b.Constant(7_i));
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    %3:i32 = max 8i, %param
    %4:i32 = min 12i, %3
    %5:i32 = mod %4, 7i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    // Range of `call_min` is [8, 12]
    // Range of `modulo` (`call_min % 7`) is [1, 5] (8 % 7 == 1, 12 % 7 == 5)
    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(modulo);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(1, range.min_bound);
    EXPECT_EQ(5, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest,
       BinaryModulo_Success_MinLHS_Divide_RHS_Equal_MaxLHS_Divide_RHS_U32) {
    Binary* modulo = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().u32());
    func->AppendParam(param);
    b.Append(func->Block(), [&] {
        // call_max = max(8, param)
        auto* call_max = b.Call<u32>(BuiltinFn::kMax, b.Constant(8_u), param);
        // call_min = min(12, call_max)
        // The range of call_min is [8, 12]
        auto* call_min = b.Call<u32>(BuiltinFn::kMin, b.Constant(12_u), call_max);

        // modulo = call_min % 7
        modulo = b.Modulo<u32>(call_min, b.Constant(7_u));
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:u32):void {
  $B1: {
    %3:u32 = max 8u, %param
    %4:u32 = min 12u, %3
    %5:u32 = mod %4, 7u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    // Range of `call_min` is [8u, 12u]
    // Range of `modulo` (`call_min % 7u`) is [1u, 5u] (8 % 7 == 1, 12 % 7 == 5)
    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(modulo);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(1u, range.min_bound);
    EXPECT_EQ(5u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryModulo_Success_RHS_Is_One_I32) {
    Binary* modulo = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);
    b.Append(func->Block(), [&] {
        // call_max = max(8, param)
        auto* call_max = b.Call<i32>(BuiltinFn::kMax, b.Constant(8_i), param);
        // call_min = min(9, call_max)
        // The range of call_min is [8, 9]
        auto* call_min = b.Call<i32>(BuiltinFn::kMin, b.Constant(9_i), call_max);

        // modulo = call_min % 1
        modulo = b.Modulo<i32>(call_min, b.Constant(1_i));
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    %3:i32 = max 8i, %param
    %4:i32 = min 9i, %3
    %5:i32 = mod %4, 1i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    // Range of `call_min` is [8, 9]
    // Range of `modulo` (`call_min % 1`) is [0, 0]
    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(modulo);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(0, range.min_bound);
    EXPECT_EQ(0, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryModulo_Success_RHS_Is_One_U32) {
    Binary* modulo = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().u32());
    func->AppendParam(param);
    b.Append(func->Block(), [&] {
        // modulo = param % 1u
        modulo = b.Modulo<u32>(param, 1_u);
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:u32):void {
  $B1: {
    %3:u32 = mod %param, 1u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    // Range of `param` is [u32::kLowestValue, u32::kHighestValue]
    // Range of `modulo` (`param % 1u`) is [0u, 0u]
    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(modulo);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(0u, range.min_bound);
    EXPECT_EQ(0u, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryModulo_Success_RHS_Is_Highest_I32) {
    Binary* modulo = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().i32());
    func->AppendParam(param);
    b.Append(func->Block(), [&] {
        // call_max = max(0, param)
        auto* call_max = b.Call<i32>(BuiltinFn::kMax, b.Constant(0_i), param);

        // modulo = call_max % i32::kHighestValue
        modulo = b.Modulo<i32>(call_max, i32::Highest());
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:i32):void {
  $B1: {
    %3:i32 = max 0i, %param
    %4:i32 = mod %3, 2147483647i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    // Range of `call_max` is [0, i32::kHighestValue]
    // Range of `modulo` (`call_max % i32::kHighestValue`) is [0, i32::kHighestValue - 1]
    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(modulo);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::SignedIntegerRange>(info.range);
    EXPECT_EQ(0, range.min_bound);
    EXPECT_EQ(i32::kHighestValue - 1, range.max_bound);
}

TEST_F(IR_IntegerRangeAnalysisTest, BinaryModulo_Success_RHS_Is_Highest_U32) {
    Binary* modulo = nullptr;
    auto* func = b.Function("func", ty.void_());
    auto* param = b.FunctionParam("param", mod.Types().u32());
    func->AppendParam(param);
    b.Append(func->Block(), [&] {
        // modulo = param % u32::kHighestValue
        modulo = b.Modulo<u32>(param, u32::Highest());
        b.Return(func);
    });

    auto* src = R"(
%func = func(%param:u32):void {
  $B1: {
    %3:u32 = mod %param, 4294967295u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    // Range of `param` is [u32::kLowestValue, u32::kHighestValue]
    // Range of `modulo` (`param % u32::kHighestValue`) is [0u, u32::kHighestValue - 1u]
    IntegerRangeAnalysis analysis(&mod);
    const auto& info = analysis.GetInfo(modulo);
    ASSERT_TRUE(info.IsValid());
    ASSERT_TRUE(std::holds_alternative<IntegerRangeInfo::UnsignedIntegerRange>(info.range));
    const auto& range = std::get<IntegerRangeInfo::UnsignedIntegerRange>(info.range);
    EXPECT_EQ(0u, range.min_bound);
    EXPECT_EQ(u32::kHighestValue - 1u, range.max_bound);
}

}  // namespace
}  // namespace tint::core::ir::analysis
