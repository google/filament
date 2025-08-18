// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/transform/binding_remapper.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_BindingRemapperTest : public TransformTest {
  public:
    IR_BindingRemapperTest() { capabilities = kBindingRemapperCapabilities; }
};

TEST_F(IR_BindingRemapperTest, NoModify_NoRemappings) {
    auto* buffer = b.Var("buffer", ty.ptr<uniform, i32>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<uniform, i32, read> = var undef @binding_point(0, 0)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    std::unordered_map<tint::BindingPoint, tint::BindingPoint> binding_points;
    Run(BindingRemapper, binding_points);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BindingRemapperTest, NoModify_RemappingDifferentBindingPoint) {
    auto* buffer = b.Var("buffer", ty.ptr<uniform, i32>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<uniform, i32, read> = var undef @binding_point(0, 0)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    std::unordered_map<tint::BindingPoint, tint::BindingPoint> binding_points;
    binding_points[tint::BindingPoint{0u, 1u}] = tint::BindingPoint{1u, 0u};
    Run(BindingRemapper, binding_points);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BindingRemapperTest, RemappingGroup) {
    auto* buffer = b.Var("buffer", ty.ptr<uniform, i32>());
    buffer->SetBindingPoint(1, 2);
    mod.root_block->Append(buffer);

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<uniform, i32, read> = var undef @binding_point(1, 2)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<uniform, i32, read> = var undef @binding_point(3, 2)
}

)";

    std::unordered_map<tint::BindingPoint, tint::BindingPoint> binding_points;
    binding_points[tint::BindingPoint{1u, 2u}] = tint::BindingPoint{3u, 2u};
    Run(BindingRemapper, binding_points);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BindingRemapperTest, RemappingBindingIndex) {
    auto* buffer = b.Var("buffer", ty.ptr<uniform, i32>());
    buffer->SetBindingPoint(1, 2);
    mod.root_block->Append(buffer);

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<uniform, i32, read> = var undef @binding_point(1, 2)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<uniform, i32, read> = var undef @binding_point(1, 3)
}

)";

    std::unordered_map<tint::BindingPoint, tint::BindingPoint> binding_points;
    binding_points[tint::BindingPoint{1u, 2u}] = tint::BindingPoint{1u, 3u};
    Run(BindingRemapper, binding_points);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BindingRemapperTest, RemappingGroupAndBindingIndex) {
    auto* buffer = b.Var("buffer", ty.ptr<uniform, i32>());
    buffer->SetBindingPoint(1, 2);
    mod.root_block->Append(buffer);

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<uniform, i32, read> = var undef @binding_point(1, 2)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<uniform, i32, read> = var undef @binding_point(3, 4)
}

)";

    std::unordered_map<tint::BindingPoint, tint::BindingPoint> binding_points;
    binding_points[tint::BindingPoint{1u, 2u}] = tint::BindingPoint{3u, 4u};
    Run(BindingRemapper, binding_points);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BindingRemapperTest, SwapTwoBindingPoints) {
    auto* buffer_a = b.Var("buffer_a", ty.ptr<uniform, i32>());
    buffer_a->SetBindingPoint(1, 2);
    mod.root_block->Append(buffer_a);
    auto* buffer_b = b.Var("buffer_b", ty.ptr<uniform, i32>());
    buffer_b->SetBindingPoint(3, 4);
    mod.root_block->Append(buffer_b);

    auto* src = R"(
$B1: {  # root
  %buffer_a:ptr<uniform, i32, read> = var undef @binding_point(1, 2)
  %buffer_b:ptr<uniform, i32, read> = var undef @binding_point(3, 4)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer_a:ptr<uniform, i32, read> = var undef @binding_point(3, 4)
  %buffer_b:ptr<uniform, i32, read> = var undef @binding_point(1, 2)
}

)";

    std::unordered_map<tint::BindingPoint, tint::BindingPoint> binding_points;
    binding_points[tint::BindingPoint{1u, 2u}] = tint::BindingPoint{3u, 4u};
    binding_points[tint::BindingPoint{3u, 4u}] = tint::BindingPoint{1u, 2u};
    Run(BindingRemapper, binding_points);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BindingRemapperTest, BindingPointCollisionSameEntryPoint) {
    auto* buffer_a = b.Var("buffer_a", ty.ptr<uniform, i32>());
    buffer_a->SetBindingPoint(1, 2);
    mod.root_block->Append(buffer_a);
    auto* buffer_b = b.Var("buffer_b", ty.ptr<uniform, i32>());
    buffer_b->SetBindingPoint(3, 4);
    mod.root_block->Append(buffer_b);

    auto* ep = b.Function("main", mod.Types().void_(), Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        b.Load(buffer_a);
        b.Load(buffer_b);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %buffer_a:ptr<uniform, i32, read> = var undef @binding_point(1, 2)
  %buffer_b:ptr<uniform, i32, read> = var undef @binding_point(3, 4)
}

%main = @fragment func():void {
  $B2: {
    %4:i32 = load %buffer_a
    %5:i32 = load %buffer_b
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer_a:ptr<uniform, i32, read> = var undef @binding_point(0, 1)
  %buffer_b:ptr<uniform, i32, read> = var undef @binding_point(0, 1)
}

%main = @fragment func():void {
  $B2: {
    %4:i32 = load %buffer_a
    %5:i32 = load %buffer_b
    ret
  }
}
)";

    std::unordered_map<tint::BindingPoint, tint::BindingPoint> binding_points;
    binding_points[tint::BindingPoint{1u, 2u}] = tint::BindingPoint{0u, 1u};
    binding_points[tint::BindingPoint{3u, 4u}] = tint::BindingPoint{0u, 1u};
    Run(BindingRemapper, binding_points);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
