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
// AND ANY EXPRESS OR- IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/core/ir/transform/remove_uniform_vector_component_loads.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_RemoveUniformVectorComponentLoadsTest = core::ir::transform::TransformTest;

TEST_F(IR_RemoveUniformVectorComponentLoadsTest, NoModify_NotUniform) {
    auto* buffer = b.Var("buffer", ty.ptr(storage, ty.vec4f(), read));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* load = b.LoadVectorElement(buffer, 2_u);
        b.Return(func, load);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, vec4<f32>, read> = var undef @binding_point(0, 0)
}

%foo = func():f32 {
  $B2: {
    %3:f32 = load_vector_element %buffer, 2u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(RemoveUniformVectorComponentLoads);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveUniformVectorComponentLoadsTest, Basic) {
    auto* buffer = b.Var("buffer", ty.ptr(uniform, ty.vec4f()));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* load = b.LoadVectorElement(buffer, 2_u);
        b.Return(func, load);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<uniform, vec4<f32>, read> = var undef @binding_point(0, 0)
}

%foo = func():f32 {
  $B2: {
    %3:f32 = load_vector_element %buffer, 2u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<uniform, vec4<f32>, read> = var undef @binding_point(0, 0)
}

%foo = func():f32 {
  $B2: {
    %3:vec4<f32> = load %buffer
    %4:vec4<f32> = let %3
    %5:f32 = access %4, 2u
    ret %5
  }
}
)";

    Run(RemoveUniformVectorComponentLoads);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveUniformVectorComponentLoadsTest, ViaLet) {
    auto* buffer = b.Var("buffer", ty.ptr(uniform, ty.vec4f()));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* p = b.Let(buffer);
        auto* load = b.LoadVectorElement(p, 2_u);
        b.Return(func, load);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<uniform, vec4<f32>, read> = var undef @binding_point(0, 0)
}

%foo = func():f32 {
  $B2: {
    %3:ptr<uniform, vec4<f32>, read> = let %buffer
    %4:f32 = load_vector_element %3, 2u
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<uniform, vec4<f32>, read> = var undef @binding_point(0, 0)
}

%foo = func():f32 {
  $B2: {
    %3:ptr<uniform, vec4<f32>, read> = let %buffer
    %4:vec4<f32> = load %3
    %5:vec4<f32> = let %4
    %6:f32 = access %5, 2u
    ret %6
  }
}
)";

    Run(RemoveUniformVectorComponentLoads);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveUniformVectorComponentLoadsTest, Multiple) {
    auto* buffer = b.Var("buffer", ty.ptr(uniform, ty.vec4f()));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.vec4f());
    b.Append(func->Block(), [&] {
        auto* x = b.LoadVectorElement(buffer, 0_u);
        auto* y = b.LoadVectorElement(buffer, 1_u);
        auto* z = b.LoadVectorElement(buffer, 2_u);
        auto* w = b.LoadVectorElement(buffer, 3_u);
        auto* c = b.Construct(ty.vec4f(), x, y, z, w);
        b.Return(func, c);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<uniform, vec4<f32>, read> = var undef @binding_point(0, 0)
}

%foo = func():vec4<f32> {
  $B2: {
    %3:f32 = load_vector_element %buffer, 0u
    %4:f32 = load_vector_element %buffer, 1u
    %5:f32 = load_vector_element %buffer, 2u
    %6:f32 = load_vector_element %buffer, 3u
    %7:vec4<f32> = construct %3, %4, %5, %6
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<uniform, vec4<f32>, read> = var undef @binding_point(0, 0)
}

%foo = func():vec4<f32> {
  $B2: {
    %3:vec4<f32> = load %buffer
    %4:vec4<f32> = let %3
    %5:f32 = access %4, 0u
    %6:vec4<f32> = load %buffer
    %7:vec4<f32> = let %6
    %8:f32 = access %7, 1u
    %9:vec4<f32> = load %buffer
    %10:vec4<f32> = let %9
    %11:f32 = access %10, 2u
    %12:vec4<f32> = load %buffer
    %13:vec4<f32> = let %12
    %14:f32 = access %13, 3u
    %15:vec4<f32> = construct %5, %8, %11, %14
    ret %15
  }
}
)";

    Run(RemoveUniformVectorComponentLoads);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
