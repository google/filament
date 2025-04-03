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

#include "src/tint/lang/core/ir/transform/prepare_push_constants.h"

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_PreparePushConstantsTests : public TransformTest {
  public:
    Result<PushConstantLayout> Run(PreparePushConstantsConfig config) {
        // Run the transform.
        auto result = PreparePushConstants(mod, config);
        EXPECT_EQ(result, Success);
        if (result != Success) {
            return result.Failure();
        }

        // Validate the output IR.
        EXPECT_EQ(ir::Validate(mod, capabilities), Success);

        return result;
    }
};

TEST_F(IR_PreparePushConstantsTests, NoModify_NoInternalConstants) {
    auto* v = b.Var("v", ty.ptr<private_, i32>());
    mod.root_block->Append(v);

    auto* src = R"(
$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
}

)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    PreparePushConstantsConfig config;
    auto result = Run(config);
    if (result == Success) {
        EXPECT_EQ(result->var, nullptr);
        EXPECT_TRUE(result->offset_to_index.IsEmpty());
    }

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreparePushConstantsTests, NoModify_UserConstant_NoInternalConstants) {
    auto* v = b.Var("v", ty.ptr<push_constant, i32>());
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.i32());
    b.Append(foo->Block(), [&] {  //
        b.Return(foo, b.Load(v));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<push_constant, i32, read> = var undef
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

    PreparePushConstantsConfig config;
    auto result = Run(config);
    if (result == Success) {
        EXPECT_EQ(result->var, nullptr);
        EXPECT_TRUE(result->offset_to_index.IsEmpty());
    }

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreparePushConstantsTests, NoUserConstant_OneInternalConstant) {
    auto* v = b.Var("v", ty.ptr<private_, i32>());
    mod.root_block->Append(v);

    auto* src = R"(
$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_push_constant_struct = struct @align(4), @block {
  internal_constant_a:i32 @offset(0)
}

$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
  %tint_push_constants:ptr<push_constant, tint_push_constant_struct, read> = var undef
}

)";

    PreparePushConstantsConfig config;
    config.AddInternalConstant(0u, mod.symbols.New("internal_constant_a"), ty.i32());
    auto result = Run(config);
    if (result == Success) {
        EXPECT_NE(result->var, nullptr);
        EXPECT_EQ(result->offset_to_index.GetOr(0u, UINT32_MAX), 0u);
    }

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreparePushConstantsTests, NoUserConstant_MultipleInternalConstants) {
    auto* v = b.Var("v", ty.ptr<private_, i32>());
    mod.root_block->Append(v);

    auto* src = R"(
$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_push_constant_struct = struct @align(16), @block {
  internal_constant_a:i32 @offset(0)
  internal_constant_b:f32 @offset(4)
  internal_constant_c:vec4<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
  %tint_push_constants:ptr<push_constant, tint_push_constant_struct, read> = var undef
}

)";

    PreparePushConstantsConfig config;
    config.AddInternalConstant(0u, mod.symbols.New("internal_constant_a"), ty.i32());
    config.AddInternalConstant(4u, mod.symbols.New("internal_constant_b"), ty.f32());
    config.AddInternalConstant(16u, mod.symbols.New("internal_constant_c"), ty.vec4<f32>());
    auto result = Run(config);
    if (result == Success) {
        EXPECT_NE(result->var, nullptr);
        EXPECT_EQ(result->offset_to_index.GetOr(0u, UINT32_MAX), 0u);
        EXPECT_EQ(result->offset_to_index.GetOr(4u, UINT32_MAX), 1u);
        EXPECT_EQ(result->offset_to_index.GetOr(16u, UINT32_MAX), 2u);
    }

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreparePushConstantsTests, UserConstant_MultipleInternalConstants) {
    auto* v = b.Var("v", ty.ptr<push_constant, i32>());
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.i32());
    b.Append(foo->Block(), [&] {  //
        b.Return(foo, b.Load(v));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<push_constant, i32, read> = var undef
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
tint_push_constant_struct = struct @align(16), @block {
  user_constant:i32 @offset(0)
  internal_constant_a:i32 @offset(4)
  internal_constant_b:f32 @offset(8)
  internal_constant_c:vec4<f32> @offset(16)
}

$B1: {  # root
  %tint_push_constants:ptr<push_constant, tint_push_constant_struct, read> = var undef
}

%foo = func():i32 {
  $B2: {
    %3:ptr<push_constant, i32, read> = access %tint_push_constants, 0u
    %4:i32 = load %3
    ret %4
  }
}
)";

    PreparePushConstantsConfig config;
    config.AddInternalConstant(4u, mod.symbols.New("internal_constant_a"), ty.i32());
    config.AddInternalConstant(8u, mod.symbols.New("internal_constant_b"), ty.f32());
    config.AddInternalConstant(16u, mod.symbols.New("internal_constant_c"), ty.vec4<f32>());
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
