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

#include "src/tint/lang/core/ir/analysis/subgroup_matrix.h"

#include <algorithm>
#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::analysis {
namespace {

using IR_SubgroupMatrixAnalysis = IRTestHelper;

struct TypeInfo {
    std::string_view type_name;
    uint16_t M;
    uint16_t N;
    uint16_t K;
    SubgroupMatrixType result_type;
};
inline std::ostream& operator<<(std::ostream& out, TypeInfo data) {
    out << data.type_name;
    return out;
}

bool SortConfig(const SubgroupMatrixConfig& a, const SubgroupMatrixConfig& b) {
    if (a.type != b.type) {
        return a.type < b.type;
    }
    if (a.direction != b.direction) {
        return a.direction < b.direction;
    }
    if (a.M != b.M) {
        return a.M < b.M;
    }
    if (a.N != b.N) {
        return a.N < b.N;
    }
    return a.K < b.K;
}

using IR_SubgroupMatrixAnalysisTypeTest = IRTestParamHelper<TypeInfo>;
TEST_P(IR_SubgroupMatrixAnalysisTypeTest, Config_Type_Left) {
    auto p = GetParam();

    const core::type::Type* subtype = nullptr;
    if (p.type_name == "f16") {
        subtype = ty.f16();
    } else if (p.type_name == "f32") {
        subtype = ty.f32();
    } else if (p.type_name == "i8") {
        subtype = ty.i8();
    } else if (p.type_name == "u8") {
        subtype = ty.u8();
    } else if (p.type_name == "u32") {
        subtype = ty.u32();
    } else if (p.type_name == "i32") {
        subtype = ty.i32();
    } else {
        ASSERT_FALSE(true) << "invalid typename " << p.type_name;
    }

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Var("v", ty.ptr(function, ty.subgroup_matrix_left(subtype, p.K, p.M)));
        b.Return(func);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, subgroup_matrix_left<)" +
               std::string(p.type_name) + ", " + std::to_string(p.K) + ", " + std::to_string(p.M) +
               R"(>, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllow8BitIntegers}),
              Success);

    auto res = GatherSubgroupMatrixInfo(mod);
    EXPECT_TRUE(res.multiplies.empty());
    ASSERT_EQ(1u, res.configs.size());

    auto cfgs = std::vector(res.configs.begin(), res.configs.end());
    auto& cfg = cfgs[0];
    EXPECT_EQ(p.M, cfg.M);
    EXPECT_EQ(0u, cfg.N);
    EXPECT_EQ(p.K, cfg.K);
    EXPECT_EQ(p.result_type, cfg.type);
    EXPECT_EQ(SubgroupMatrixDirection::kLeft, cfg.direction);
}
TEST_P(IR_SubgroupMatrixAnalysisTypeTest, Config_Type_Right) {
    auto p = GetParam();

    const core::type::Type* subtype = nullptr;
    if (p.type_name == "f16") {
        subtype = ty.f16();
    } else if (p.type_name == "f32") {
        subtype = ty.f32();
    } else if (p.type_name == "i8") {
        subtype = ty.i8();
    } else if (p.type_name == "u8") {
        subtype = ty.u8();
    } else if (p.type_name == "u32") {
        subtype = ty.u32();
    } else if (p.type_name == "i32") {
        subtype = ty.i32();
    } else {
        ASSERT_FALSE(true) << "invalid typename " << p.type_name;
    }

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Var("v", ty.ptr(function, ty.subgroup_matrix_right(subtype, p.N, p.K)));
        b.Return(func);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, subgroup_matrix_right<)" +
               std::string(p.type_name) + ", " + std::to_string(p.N) + ", " + std::to_string(p.K) +
               R"(>, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllow8BitIntegers}),
              Success);

    auto res = GatherSubgroupMatrixInfo(mod);
    EXPECT_TRUE(res.multiplies.empty());
    ASSERT_EQ(1u, res.configs.size());

    auto cfgs = std::vector(res.configs.begin(), res.configs.end());
    auto& cfg = cfgs[0];
    EXPECT_EQ(0u, cfg.M);
    EXPECT_EQ(p.N, cfg.N);
    EXPECT_EQ(p.K, cfg.K);
    EXPECT_EQ(p.result_type, cfg.type);
    EXPECT_EQ(SubgroupMatrixDirection::kRight, cfg.direction);
}

TEST_P(IR_SubgroupMatrixAnalysisTypeTest, Config_Type_Result) {
    auto p = GetParam();

    const core::type::Type* subtype = nullptr;
    if (p.type_name == "f16") {
        subtype = ty.f16();
    } else if (p.type_name == "f32") {
        subtype = ty.f32();
    } else if (p.type_name == "i8") {
        subtype = ty.i8();
    } else if (p.type_name == "u8") {
        subtype = ty.u8();
    } else if (p.type_name == "u32") {
        subtype = ty.u32();
    } else if (p.type_name == "i32") {
        subtype = ty.i32();
    } else {
        ASSERT_FALSE(true) << "invalid typename " << p.type_name;
    }

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Var("v", ty.ptr(function, ty.subgroup_matrix_result(subtype, p.N, p.M)));
        b.Return(func);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, subgroup_matrix_result<)" +
               std::string(p.type_name) + ", " + std::to_string(p.N) + ", " + std::to_string(p.M) +
               R"(>, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllow8BitIntegers}),
              Success);

    auto res = GatherSubgroupMatrixInfo(mod);
    EXPECT_TRUE(res.multiplies.empty());
    ASSERT_EQ(1u, res.configs.size());

    auto cfgs = std::vector(res.configs.begin(), res.configs.end());
    auto& cfg = cfgs[0];
    EXPECT_EQ(p.M, cfg.M);
    EXPECT_EQ(p.N, cfg.N);
    EXPECT_EQ(0u, cfg.K);
    EXPECT_EQ(p.result_type, cfg.type);
    EXPECT_EQ(SubgroupMatrixDirection::kResult, cfg.direction);
}
INSTANTIATE_TEST_SUITE_P(,
                         IR_SubgroupMatrixAnalysisTypeTest,
                         testing::Values(
                             TypeInfo{
                                 .type_name = "f16",
                                 .M = 8,
                                 .N = 8,
                                 .K = 8,
                                 .result_type = SubgroupMatrixType::kF16,
                             },
                             TypeInfo{
                                 .type_name = "f32",
                                 .M = 2,
                                 .N = 2,
                                 .K = 2,
                                 .result_type = SubgroupMatrixType::kF32,
                             },
                             TypeInfo{
                                 .type_name = "i8",
                                 .M = 128,
                                 .N = 1024,
                                 .K = 512,
                                 .result_type = SubgroupMatrixType::kI8,
                             },
                             TypeInfo{
                                 .type_name = "u8",
                                 .M = 4,
                                 .N = 8,
                                 .K = 2,
                                 .result_type = SubgroupMatrixType::kU8,
                             },
                             TypeInfo{
                                 .type_name = "i32",
                                 .M = 8,
                                 .N = 4,
                                 .K = 2,
                                 .result_type = SubgroupMatrixType::kI32,
                             },
                             TypeInfo{
                                 .type_name = "u32",
                                 .M = 64,
                                 .N = 32242,
                                 .K = 2048,
                                 .result_type = SubgroupMatrixType::kU32,
                             }));

TEST_F(IR_SubgroupMatrixAnalysis, Config_Multiple) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Var("v1", ty.ptr(function, ty.subgroup_matrix_left(ty.f16(), 8u, 8u)));
        b.Var("v2", ty.ptr(function, ty.subgroup_matrix_result(ty.f16(), 32u, 64u)));
        b.Var("v3", ty.ptr(function, ty.subgroup_matrix_right(ty.f32(), 8u, 8u)));
        b.Var("v4", ty.ptr(function, ty.subgroup_matrix_right(ty.i8(), 8u, 8u)));

        b.Var("v1_dup", ty.ptr(function, ty.subgroup_matrix_left(ty.i8(), 2u, 8u)));
        b.Return(func);
    });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v1:ptr<function, subgroup_matrix_left<f16, 8, 8>, read_write> = var undef
    %v2:ptr<function, subgroup_matrix_result<f16, 32, 64>, read_write> = var undef
    %v3:ptr<function, subgroup_matrix_right<f32, 8, 8>, read_write> = var undef
    %v4:ptr<function, subgroup_matrix_right<i8, 8, 8>, read_write> = var undef
    %v1_dup:ptr<function, subgroup_matrix_left<i8, 2, 8>, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllow8BitIntegers}),
              Success);

    auto res = GatherSubgroupMatrixInfo(mod);
    EXPECT_TRUE(res.multiplies.empty());
    ASSERT_EQ(5u, res.configs.size());

    auto cfgs = std::vector(res.configs.begin(), res.configs.end());
    std::sort(cfgs.begin(), cfgs.end(), SortConfig);

    EXPECT_EQ(8u, cfgs[0].M);
    EXPECT_EQ(8u, cfgs[0].K);
    EXPECT_EQ(SubgroupMatrixType::kF16, cfgs[0].type);
    EXPECT_EQ(SubgroupMatrixDirection::kLeft, cfgs[0].direction);

    EXPECT_EQ(32u, cfgs[1].N);
    EXPECT_EQ(64u, cfgs[1].M);
    EXPECT_EQ(SubgroupMatrixType::kF16, cfgs[1].type);
    EXPECT_EQ(SubgroupMatrixDirection::kResult, cfgs[1].direction);

    EXPECT_EQ(8u, cfgs[2].N);
    EXPECT_EQ(8u, cfgs[2].K);
    EXPECT_EQ(SubgroupMatrixType::kF32, cfgs[2].type);
    EXPECT_EQ(SubgroupMatrixDirection::kRight, cfgs[2].direction);

    EXPECT_EQ(8u, cfgs[3].M);
    EXPECT_EQ(2u, cfgs[3].K);
    EXPECT_EQ(SubgroupMatrixType::kI8, cfgs[3].type);
    EXPECT_EQ(SubgroupMatrixDirection::kLeft, cfgs[3].direction);

    EXPECT_EQ(8u, cfgs[4].N);
    EXPECT_EQ(8u, cfgs[4].K);
    EXPECT_EQ(SubgroupMatrixType::kI8, cfgs[4].type);
    EXPECT_EQ(SubgroupMatrixDirection::kRight, cfgs[4].direction);
}

TEST_F(IR_SubgroupMatrixAnalysis, Config_InControlFlow) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            b.Var("v", ty.ptr(function, ty.subgroup_matrix_result(ty.f16(), 8u, 8u)));
            b.Exit(if_);
        });
        b.Return(func);
    });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    if true [t: $B2] {  # if_1
      $B2: {  # true
        %v:ptr<function, subgroup_matrix_result<f16, 8, 8>, read_write> = var undef
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    auto res = GatherSubgroupMatrixInfo(mod);
    EXPECT_TRUE(res.multiplies.empty());
    ASSERT_EQ(1u, res.configs.size());

    auto cfgs = std::vector(res.configs.begin(), res.configs.end());
    auto& cfg = cfgs[0];
    EXPECT_EQ(8u, cfg.M);
    EXPECT_EQ(8u, cfg.N);
    EXPECT_EQ(SubgroupMatrixType::kF16, cfg.type);
    EXPECT_EQ(SubgroupMatrixDirection::kResult, cfg.direction);
}

TEST_F(IR_SubgroupMatrixAnalysis, Config_FunctionParam) {
    auto* f2 = b.Function("f", ty.void_());
    f2->AppendParam(b.FunctionParam("p", ty.subgroup_matrix_left(ty.f32(), 8u, 8u)));
    b.Append(f2->Block(), [&] {  //
        b.Return(f2);
    });

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        auto* v = b.Var("v", ty.ptr(function, ty.subgroup_matrix_left(ty.f32(), 8u, 8u)));
        b.Call(ty.void_(), f2, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
%f = func(%p:subgroup_matrix_left<f32, 8, 8>):void {
  $B1: {
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, subgroup_matrix_left<f32, 8, 8>, read_write> = var undef
    %5:subgroup_matrix_left<f32, 8, 8> = load %v
    %6:void = call %f, %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    EXPECT_EQ(Validate(mod), Success);

    auto res = GatherSubgroupMatrixInfo(mod);
    EXPECT_TRUE(res.multiplies.empty());
    ASSERT_EQ(1u, res.configs.size());

    auto cfgs = std::vector(res.configs.begin(), res.configs.end());
    auto& cfg = cfgs[0];
    EXPECT_EQ(8u, cfg.M);
    EXPECT_EQ(8u, cfg.K);
    EXPECT_EQ(SubgroupMatrixType::kF32, cfg.type);
    EXPECT_EQ(SubgroupMatrixDirection::kLeft, cfg.direction);
}

TEST_F(IR_SubgroupMatrixAnalysis, Config_FunctionReturn) {
    auto* f2 = b.Function("f", ty.subgroup_matrix_left(ty.f32(), 8u, 8u));
    b.Append(f2->Block(), [&] {  //
        b.Return(f2, b.Composite(ty.subgroup_matrix_left(ty.f32(), 8u, 8u), 5_f));
    });

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Let("v", b.Call(ty.subgroup_matrix_left(ty.f32(), 8u, 8u), f2));
        b.Return(func);
    });

    auto* src = R"(
%f = func():subgroup_matrix_left<f32, 8, 8> {
  $B1: {
    ret subgroup_matrix_left<f32, 8, 8>(5.0f)
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:subgroup_matrix_left<f32, 8, 8> = call %f
    %v:subgroup_matrix_left<f32, 8, 8> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    EXPECT_EQ(Validate(mod), Success);

    auto res = GatherSubgroupMatrixInfo(mod);
    EXPECT_TRUE(res.multiplies.empty());
    ASSERT_EQ(1u, res.configs.size());

    auto cfgs = std::vector(res.configs.begin(), res.configs.end());
    auto& cfg = cfgs[0];
    EXPECT_EQ(8u, cfg.M);
    EXPECT_EQ(8u, cfg.K);
    EXPECT_EQ(SubgroupMatrixType::kF32, cfg.type);
    EXPECT_EQ(SubgroupMatrixDirection::kLeft, cfg.direction);
}

TEST_F(IR_SubgroupMatrixAnalysis, Config_InStruct) {
    auto* s = ty.Struct(mod.symbols.New("S"),
                        {
                            {mod.symbols.New("a"), ty.subgroup_matrix_left(ty.f32(), 8u, 8u)},
                        });

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Var("v", ty.ptr(function, s));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:subgroup_matrix_left<f32, 8, 8> @offset(0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, S, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    EXPECT_EQ(Validate(mod), Success);

    auto res = GatherSubgroupMatrixInfo(mod);
    EXPECT_TRUE(res.multiplies.empty());
    ASSERT_EQ(1u, res.configs.size());

    auto cfgs = std::vector(res.configs.begin(), res.configs.end());
    auto& cfg = cfgs[0];
    EXPECT_EQ(8u, cfg.M);
    EXPECT_EQ(8u, cfg.K);
    EXPECT_EQ(SubgroupMatrixType::kF32, cfg.type);
    EXPECT_EQ(SubgroupMatrixDirection::kLeft, cfg.direction);
}

TEST_F(IR_SubgroupMatrixAnalysis, Config_InArray) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Var("v", ty.ptr(function, ty.array(ty.subgroup_matrix_left(ty.f32(), 8u, 8u), 4)));
        b.Return(func);
    });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, array<subgroup_matrix_left<f32, 8, 8>, 4>, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    EXPECT_EQ(Validate(mod), Success);

    auto res = GatherSubgroupMatrixInfo(mod);
    EXPECT_TRUE(res.multiplies.empty());
    ASSERT_EQ(1u, res.configs.size());

    auto cfgs = std::vector(res.configs.begin(), res.configs.end());
    auto& cfg = cfgs[0];
    EXPECT_EQ(8u, cfg.M);
    EXPECT_EQ(8u, cfg.K);
    EXPECT_EQ(SubgroupMatrixType::kF32, cfg.type);
    EXPECT_EQ(SubgroupMatrixDirection::kLeft, cfg.direction);
}

TEST_F(IR_SubgroupMatrixAnalysis, Multiply) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        auto* left =
            b.Load(b.Var("left", ty.ptr(function, ty.subgroup_matrix_left(ty.f32(), 2u, 8u))));
        auto* right =
            b.Load(b.Var("right", ty.ptr(function, ty.subgroup_matrix_right(ty.f32(), 8u, 2u))));

        b.Let("result", b.CallExplicit(ty.subgroup_matrix_result(ty.f32(), 8u, 8u),
                                       core::BuiltinFn::kSubgroupMatrixMultiply, Vector{ty.f32()},
                                       left, right));

        b.Return(func);
    });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %left:ptr<function, subgroup_matrix_left<f32, 2, 8>, read_write> = var undef
    %3:subgroup_matrix_left<f32, 2, 8> = load %left
    %right:ptr<function, subgroup_matrix_right<f32, 8, 2>, read_write> = var undef
    %5:subgroup_matrix_right<f32, 8, 2> = load %right
    %6:subgroup_matrix_result<f32, 8, 8> = subgroupMatrixMultiply<f32> %3, %5
    %result:subgroup_matrix_result<f32, 8, 8> = let %6
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    EXPECT_EQ(Validate(mod), Success);

    auto res = GatherSubgroupMatrixInfo(mod);
    ASSERT_EQ(3u, res.configs.size());

    auto cfgs = std::vector(res.configs.begin(), res.configs.end());
    std::sort(cfgs.begin(), cfgs.end(), SortConfig);

    EXPECT_EQ(2u, cfgs[0].K);
    EXPECT_EQ(8u, cfgs[0].M);
    EXPECT_EQ(SubgroupMatrixType::kF32, cfgs[0].type);
    EXPECT_EQ(SubgroupMatrixDirection::kLeft, cfgs[0].direction);

    EXPECT_EQ(8u, cfgs[1].N);
    EXPECT_EQ(2u, cfgs[1].K);
    EXPECT_EQ(SubgroupMatrixType::kF32, cfgs[1].type);
    EXPECT_EQ(SubgroupMatrixDirection::kRight, cfgs[1].direction);

    EXPECT_EQ(8u, cfgs[2].M);
    EXPECT_EQ(8u, cfgs[2].N);
    EXPECT_EQ(SubgroupMatrixType::kF32, cfgs[2].type);
    EXPECT_EQ(SubgroupMatrixDirection::kResult, cfgs[2].direction);

    ASSERT_EQ(1u, res.multiplies.size());

    auto mul = std::vector(res.multiplies.begin(), res.multiplies.end());
    auto& mult = mul[0];
    EXPECT_EQ(8u, mult.M);
    EXPECT_EQ(8u, mult.N);
    EXPECT_EQ(2u, mult.K);
    EXPECT_EQ(SubgroupMatrixType::kF32, mult.input_type);
    EXPECT_EQ(SubgroupMatrixType::kF32, mult.output_type);
}

TEST_F(IR_SubgroupMatrixAnalysis, MultiplyAccumulate) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        auto* left =
            b.Load(b.Var("left", ty.ptr(function, ty.subgroup_matrix_left(ty.f32(), 2u, 8u))));
        auto* right =
            b.Load(b.Var("right", ty.ptr(function, ty.subgroup_matrix_right(ty.f32(), 8u, 2u))));
        auto* acc =
            b.Load(b.Var("acc", ty.ptr(function, ty.subgroup_matrix_result(ty.f32(), 8u, 8u))));

        b.Let("result",
              b.Call(ty.subgroup_matrix_result(ty.f32(), 8u, 8u),
                     core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate, left, right, acc));

        b.Return(func);
    });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %left:ptr<function, subgroup_matrix_left<f32, 2, 8>, read_write> = var undef
    %3:subgroup_matrix_left<f32, 2, 8> = load %left
    %right:ptr<function, subgroup_matrix_right<f32, 8, 2>, read_write> = var undef
    %5:subgroup_matrix_right<f32, 8, 2> = load %right
    %acc:ptr<function, subgroup_matrix_result<f32, 8, 8>, read_write> = var undef
    %7:subgroup_matrix_result<f32, 8, 8> = load %acc
    %8:subgroup_matrix_result<f32, 8, 8> = subgroupMatrixMultiplyAccumulate %3, %5, %7
    %result:subgroup_matrix_result<f32, 8, 8> = let %8
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    EXPECT_EQ(Validate(mod), Success);

    auto res = GatherSubgroupMatrixInfo(mod);
    ASSERT_EQ(3u, res.configs.size());

    auto cfgs = std::vector(res.configs.begin(), res.configs.end());
    std::sort(cfgs.begin(), cfgs.end(), SortConfig);

    EXPECT_EQ(2u, cfgs[0].K);
    EXPECT_EQ(8u, cfgs[0].M);
    EXPECT_EQ(SubgroupMatrixType::kF32, cfgs[0].type);
    EXPECT_EQ(SubgroupMatrixDirection::kLeft, cfgs[0].direction);

    EXPECT_EQ(8u, cfgs[1].N);
    EXPECT_EQ(2u, cfgs[1].K);
    EXPECT_EQ(SubgroupMatrixType::kF32, cfgs[1].type);
    EXPECT_EQ(SubgroupMatrixDirection::kRight, cfgs[1].direction);

    EXPECT_EQ(8u, cfgs[2].M);
    EXPECT_EQ(8u, cfgs[2].N);
    EXPECT_EQ(SubgroupMatrixType::kF32, cfgs[2].type);
    EXPECT_EQ(SubgroupMatrixDirection::kResult, cfgs[2].direction);

    ASSERT_EQ(1u, res.multiplies.size());

    auto mul = std::vector(res.multiplies.begin(), res.multiplies.end());
    auto& mult = mul[0];
    EXPECT_EQ(8u, mult.M);
    EXPECT_EQ(8u, mult.N);
    EXPECT_EQ(2u, mult.K);
    EXPECT_EQ(SubgroupMatrixType::kF32, mult.input_type);
    EXPECT_EQ(SubgroupMatrixType::kF32, mult.output_type);
}

TEST_F(IR_SubgroupMatrixAnalysis, Multiply_DifferentResultType) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        auto* left =
            b.Load(b.Var("left", ty.ptr(function, ty.subgroup_matrix_left(ty.i8(), 2u, 8u))));
        auto* right =
            b.Load(b.Var("right", ty.ptr(function, ty.subgroup_matrix_right(ty.i8(), 8u, 2u))));

        b.Let("result", b.CallExplicit(ty.subgroup_matrix_result(ty.i32(), 8u, 8u),
                                       core::BuiltinFn::kSubgroupMatrixMultiply, Vector{ty.i32()},
                                       left, right));

        b.Return(func);
    });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %left:ptr<function, subgroup_matrix_left<i8, 2, 8>, read_write> = var undef
    %3:subgroup_matrix_left<i8, 2, 8> = load %left
    %right:ptr<function, subgroup_matrix_right<i8, 8, 2>, read_write> = var undef
    %5:subgroup_matrix_right<i8, 8, 2> = load %right
    %6:subgroup_matrix_result<i32, 8, 8> = subgroupMatrixMultiply<i32> %3, %5
    %result:subgroup_matrix_result<i32, 8, 8> = let %6
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    EXPECT_EQ(Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllow8BitIntegers}),
              Success);

    auto res = GatherSubgroupMatrixInfo(mod);
    ASSERT_EQ(3u, res.configs.size());

    auto cfgs = std::vector(res.configs.begin(), res.configs.end());
    std::sort(cfgs.begin(), cfgs.end(), SortConfig);

    EXPECT_EQ(2u, cfgs[0].K);
    EXPECT_EQ(8u, cfgs[0].M);
    EXPECT_EQ(SubgroupMatrixType::kI8, cfgs[0].type);
    EXPECT_EQ(SubgroupMatrixDirection::kLeft, cfgs[0].direction);

    EXPECT_EQ(8u, cfgs[1].N);
    EXPECT_EQ(2u, cfgs[1].K);
    EXPECT_EQ(SubgroupMatrixType::kI8, cfgs[1].type);
    EXPECT_EQ(SubgroupMatrixDirection::kRight, cfgs[1].direction);

    EXPECT_EQ(8u, cfgs[2].M);
    EXPECT_EQ(8u, cfgs[2].N);
    EXPECT_EQ(SubgroupMatrixType::kI32, cfgs[2].type);
    EXPECT_EQ(SubgroupMatrixDirection::kResult, cfgs[2].direction);

    ASSERT_EQ(1u, res.multiplies.size());

    auto mul = std::vector(res.multiplies.begin(), res.multiplies.end());
    auto& mult = mul[0];
    EXPECT_EQ(8u, mult.M);
    EXPECT_EQ(8u, mult.N);
    EXPECT_EQ(2u, mult.K);
    EXPECT_EQ(SubgroupMatrixType::kI8, mult.input_type);
    EXPECT_EQ(SubgroupMatrixType::kI32, mult.output_type);
}

TEST_F(IR_SubgroupMatrixAnalysis, Multiply_Multiple) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        auto* left =
            b.Load(b.Var("left", ty.ptr(function, ty.subgroup_matrix_left(ty.f32(), 2u, 8u))));
        auto* right =
            b.Load(b.Var("right", ty.ptr(function, ty.subgroup_matrix_right(ty.f32(), 8u, 2u))));

        b.Let("result", b.CallExplicit(ty.subgroup_matrix_result(ty.f32(), 8u, 8u),
                                       core::BuiltinFn::kSubgroupMatrixMultiply, Vector{ty.f32()},
                                       left, right));

        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            auto* left2 =
                b.Load(b.Var("left2", ty.ptr(function, ty.subgroup_matrix_left(ty.u32(), 2u, 8u))));
            auto* right2 = b.Load(
                b.Var("right2", ty.ptr(function, ty.subgroup_matrix_right(ty.u32(), 8u, 2u))));

            b.Let("result2", b.CallExplicit(ty.subgroup_matrix_result(ty.u32(), 8u, 8u),
                                            core::BuiltinFn::kSubgroupMatrixMultiply,
                                            Vector{ty.u32()}, left2, right2));

            b.Exit(if_);
        });

        b.Return(func);
    });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %left:ptr<function, subgroup_matrix_left<f32, 2, 8>, read_write> = var undef
    %3:subgroup_matrix_left<f32, 2, 8> = load %left
    %right:ptr<function, subgroup_matrix_right<f32, 8, 2>, read_write> = var undef
    %5:subgroup_matrix_right<f32, 8, 2> = load %right
    %6:subgroup_matrix_result<f32, 8, 8> = subgroupMatrixMultiply<f32> %3, %5
    %result:subgroup_matrix_result<f32, 8, 8> = let %6
    if true [t: $B2] {  # if_1
      $B2: {  # true
        %left2:ptr<function, subgroup_matrix_left<u32, 2, 8>, read_write> = var undef
        %9:subgroup_matrix_left<u32, 2, 8> = load %left2
        %right2:ptr<function, subgroup_matrix_right<u32, 8, 2>, read_write> = var undef
        %11:subgroup_matrix_right<u32, 8, 2> = load %right2
        %12:subgroup_matrix_result<u32, 8, 8> = subgroupMatrixMultiply<u32> %9, %11
        %result2:subgroup_matrix_result<u32, 8, 8> = let %12
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    EXPECT_EQ(Validate(mod), Success);

    auto res = GatherSubgroupMatrixInfo(mod);
    ASSERT_EQ(6u, res.configs.size());

    auto cfgs = std::vector(res.configs.begin(), res.configs.end());
    std::sort(cfgs.begin(), cfgs.end(), SortConfig);

    EXPECT_EQ(2u, cfgs[0].K);
    EXPECT_EQ(8u, cfgs[0].M);
    EXPECT_EQ(SubgroupMatrixType::kF32, cfgs[0].type);
    EXPECT_EQ(SubgroupMatrixDirection::kLeft, cfgs[0].direction);

    EXPECT_EQ(8u, cfgs[1].N);
    EXPECT_EQ(2u, cfgs[1].K);
    EXPECT_EQ(SubgroupMatrixType::kF32, cfgs[1].type);
    EXPECT_EQ(SubgroupMatrixDirection::kRight, cfgs[1].direction);

    EXPECT_EQ(8u, cfgs[2].M);
    EXPECT_EQ(8u, cfgs[2].N);
    EXPECT_EQ(SubgroupMatrixType::kF32, cfgs[2].type);
    EXPECT_EQ(SubgroupMatrixDirection::kResult, cfgs[2].direction);

    EXPECT_EQ(2u, cfgs[3].K);
    EXPECT_EQ(8u, cfgs[3].M);
    EXPECT_EQ(SubgroupMatrixType::kU32, cfgs[3].type);
    EXPECT_EQ(SubgroupMatrixDirection::kLeft, cfgs[3].direction);

    EXPECT_EQ(8u, cfgs[4].N);
    EXPECT_EQ(2u, cfgs[4].K);
    EXPECT_EQ(SubgroupMatrixType::kU32, cfgs[4].type);
    EXPECT_EQ(SubgroupMatrixDirection::kRight, cfgs[4].direction);

    EXPECT_EQ(8u, cfgs[5].M);
    EXPECT_EQ(8u, cfgs[5].N);
    EXPECT_EQ(SubgroupMatrixType::kU32, cfgs[5].type);
    EXPECT_EQ(SubgroupMatrixDirection::kResult, cfgs[5].direction);

    ASSERT_EQ(2u, res.multiplies.size());

    auto mul = std::vector(res.multiplies.begin(), res.multiplies.end());
    std::sort(mul.begin(), mul.end(),
              [&](const SubgroupMatrixMultiply& a, const SubgroupMatrixMultiply& b) {
                  return a.input_type < b.input_type;
              });

    EXPECT_EQ(8u, mul[0].M);
    EXPECT_EQ(8u, mul[0].N);
    EXPECT_EQ(2u, mul[0].K);
    EXPECT_EQ(SubgroupMatrixType::kF32, mul[0].input_type);
    EXPECT_EQ(SubgroupMatrixType::kF32, mul[0].output_type);

    EXPECT_EQ(8u, mul[1].M);
    EXPECT_EQ(8u, mul[1].N);
    EXPECT_EQ(2u, mul[1].K);
    EXPECT_EQ(SubgroupMatrixType::kU32, mul[1].input_type);
    EXPECT_EQ(SubgroupMatrixType::kU32, mul[1].output_type);
}

}  // namespace
}  // namespace tint::core::ir::analysis
