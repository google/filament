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

#include "src/tint/lang/core/ir/transform/prepare_immediate_data.h"

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_PrepareImmediateDataTests : public TransformTest {
  public:
    Result<ImmediateDataLayout> Run(PrepareImmediateDataConfig config) {
        // Run the transform.
        auto result = PrepareImmediateData(mod, config);
        EXPECT_EQ(result, Success);
        if (result != Success) {
            return result.Failure();
        }

        // Validate the output IR.
        EXPECT_EQ(ir::Validate(mod, capabilities), Success);

        return result;
    }
};

TEST_F(IR_PrepareImmediateDataTests, NoModify_NoInternalImmediateData) {
    auto* v = b.Var("v", ty.ptr<private_, i32>());
    mod.root_block->Append(v);

    auto* src = R"(
$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
}

)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    PrepareImmediateDataConfig config;
    auto result = Run(config);
    if (result == Success) {
        EXPECT_EQ(result->var, nullptr);
        EXPECT_TRUE(result->offset_to_index.IsEmpty());
    }

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PrepareImmediateDataTests, NoModify_UserImmediateData_NoInternalImmediateData) {
    auto* v = b.Var("v", ty.ptr<immediate, i32>());
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.i32());
    b.Append(foo->Block(), [&] {  //
        b.Return(foo, b.Load(v));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<immediate, i32, read> = var undef
}

%foo = func():i32 {
  $B2: {
    %3:i32 = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    PrepareImmediateDataConfig config;
    auto result = Run(config);
    if (result == Success) {
        EXPECT_EQ(result->var, nullptr);
        EXPECT_TRUE(result->offset_to_index.IsEmpty());
    }

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PrepareImmediateDataTests, NoUserImmedaiteData_OneInternalImmediateData) {
    auto* v = b.Var("v", ty.ptr<private_, i32>());
    mod.root_block->Append(v);

    auto* src = R"(
$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(4), @block {
  internal_constant_a:i32 @offset(0)
}

$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

)";

    PrepareImmediateDataConfig config;
    config.AddInternalImmediateData(0u, mod.symbols.New("internal_constant_a"), ty.i32());
    auto result = Run(config);
    if (result == Success) {
        EXPECT_NE(result->var, nullptr);
        EXPECT_EQ(result->offset_to_index.GetOr(0u, UINT32_MAX), 0u);
    }

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PrepareImmediateDataTests, NoUserImmediateData_MultipleInternalImmediateData) {
    auto* v = b.Var("v", ty.ptr<private_, i32>());
    mod.root_block->Append(v);

    auto* src = R"(
$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  internal_constant_a:i32 @offset(0)
  internal_constant_b:f32 @offset(4)
  internal_constant_c:vec4<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

)";

    PrepareImmediateDataConfig config;
    config.AddInternalImmediateData(0u, mod.symbols.New("internal_constant_a"), ty.i32());
    config.AddInternalImmediateData(4u, mod.symbols.New("internal_constant_b"), ty.f32());
    config.AddInternalImmediateData(16u, mod.symbols.New("internal_constant_c"), ty.vec4<f32>());
    auto result = Run(config);
    if (result == Success) {
        EXPECT_NE(result->var, nullptr);
        EXPECT_EQ(result->offset_to_index.GetOr(0u, UINT32_MAX), 0u);
        EXPECT_EQ(result->offset_to_index.GetOr(4u, UINT32_MAX), 1u);
        EXPECT_EQ(result->offset_to_index.GetOr(16u, UINT32_MAX), 2u);
    }

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PrepareImmediateDataTests, UserImmediateData_MultipleInternalImmediateData) {
    auto* v = b.Var("v", ty.ptr<immediate, i32>());
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.i32());
    b.Append(foo->Block(), [&] {  //
        b.Return(foo, b.Load(v));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<immediate, i32, read> = var undef
}

%foo = func():i32 {
  $B2: {
    %3:i32 = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  user_immediate_data:i32 @offset(0)
  internal_constant_a:i32 @offset(4)
  internal_constant_b:f32 @offset(8)
  internal_constant_c:vec4<f32> @offset(16)
}

$B1: {  # root
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = func():i32 {
  $B2: {
    %3:ptr<immediate, i32, read> = access %tint_immediate_data, 0u
    %4:i32 = load %3
    ret %4
  }
}
)";

    PrepareImmediateDataConfig config;
    config.AddInternalImmediateData(4u, mod.symbols.New("internal_constant_a"), ty.i32());
    config.AddInternalImmediateData(8u, mod.symbols.New("internal_constant_b"), ty.f32());
    config.AddInternalImmediateData(16u, mod.symbols.New("internal_constant_c"), ty.vec4<f32>());
    auto result = Run(config);
    if (result == Success) {
        EXPECT_NE(result->var, nullptr);
        EXPECT_EQ(result->offset_to_index.GetOr(4u, UINT32_MAX), 1u);
        EXPECT_EQ(result->offset_to_index.GetOr(8u, UINT32_MAX), 2u);
        EXPECT_EQ(result->offset_to_index.GetOr(16u, UINT32_MAX), 3u);
    }

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
