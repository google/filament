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

#include "src/tint/lang/msl/writer/raise/fix_u32_div_mod.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/ir/transform/prepare_immediate_data.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer::raise {
namespace {

class MslWriter_FixU32DivModTest : public core::ir::transform::TransformTest {
  protected:
    void RunTransform() {
        // Set up an internal immediate data layout, including the non-constant zero value.
        constexpr uint32_t kNonConstantZeroOffset = 8;
        core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
        ASSERT_EQ(immediate_data_config.AddInternalImmediateData(0, mod.symbols.New("a"), ty.f32()),
                  Success);
        ASSERT_EQ(immediate_data_config.AddInternalImmediateData(
                      kNonConstantZeroOffset, mod.symbols.New("tint_non_constant_zero"), ty.u32()),
                  Success);
        ASSERT_EQ(
            immediate_data_config.AddInternalImmediateData(16, mod.symbols.New("b"), ty.f32()),
            Success);
        auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
        EXPECT_EQ(immediate_data, Success);

        FixU32DivModConfig config{
            .immediate_var = immediate_data->var,
            .non_constant_zero_index = immediate_data->IndexOf(kNonConstantZeroOffset),
        };
        Run(FixU32DivMod, config);
    }
};

TEST_F(MslWriter_FixU32DivModTest, NoModify_FDiv) {
    auto* lhs = b.FunctionParam<f32>("lhs");
    auto* rhs = b.FunctionParam<f32>("rhs");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.Divide(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:f32, %rhs:f32):f32 {
  $B1: {
    %4:f32 = div %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    FixU32DivModConfig config{};
    Run(FixU32DivMod, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_FixU32DivModTest, UMod) {
    auto* lhs = b.FunctionParam<u32>("lhs");
    auto* rhs = b.FunctionParam<u32>("rhs");
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.Modulo(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:u32, %rhs:u32):u32 {
  $B1: {
    %4:u32 = mod %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(4), @block {
  a:f32 @offset(0)
  tint_non_constant_zero:u32 @offset(8)
  b:f32 @offset(16)
}

$B1: {  # root
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = func(%lhs:u32, %rhs:u32):u32 {
  $B2: {
    %5:ptr<immediate, u32, read> = access %tint_immediate_data, 1u
    %6:u32 = load %5
    %7:u32 = add %lhs, %6
    %8:u32 = mod %7, %rhs
    ret %8
  }
}
)";

    RunTransform();

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_FixU32DivModTest, UDiv) {
    auto* lhs = b.FunctionParam<u32>("lhs");
    auto* rhs = b.FunctionParam<u32>("rhs");
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.Divide(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:u32, %rhs:u32):u32 {
  $B1: {
    %4:u32 = div %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(4), @block {
  a:f32 @offset(0)
  tint_non_constant_zero:u32 @offset(8)
  b:f32 @offset(16)
}

$B1: {  # root
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = func(%lhs:u32, %rhs:u32):u32 {
  $B2: {
    %5:ptr<immediate, u32, read> = access %tint_immediate_data, 1u
    %6:u32 = load %5
    %7:u32 = add %lhs, %6
    %8:u32 = div %7, %rhs
    ret %8
  }
}
)";

    RunTransform();

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_FixU32DivModTest, UDiv_Vector) {
    auto* lhs = b.FunctionParam<vec4u>("lhs");
    auto* rhs = b.FunctionParam<vec4u>("rhs");
    auto* func = b.Function("foo", ty.vec4u());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.Divide(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:vec4<u32>, %rhs:vec4<u32>):vec4<u32> {
  $B1: {
    %4:vec4<u32> = div %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(4), @block {
  a:f32 @offset(0)
  tint_non_constant_zero:u32 @offset(8)
  b:f32 @offset(16)
}

$B1: {  # root
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = func(%lhs:vec4<u32>, %rhs:vec4<u32>):vec4<u32> {
  $B2: {
    %5:ptr<immediate, u32, read> = access %tint_immediate_data, 1u
    %6:u32 = load %5
    %7:vec4<u32> = add %lhs, %6
    %8:vec4<u32> = div %7, %rhs
    ret %8
  }
}
)";

    RunTransform();

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::msl::writer::raise
