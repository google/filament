// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/transform/first_index_offset.h"

#include <memory>
#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using FirstIndexOffsetTest = TransformTest;

TEST_F(FirstIndexOffsetTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<FirstIndexOffset>(src));
}

TEST_F(FirstIndexOffsetTest, ShouldRunFragmentStage) {
    auto* src = R"(
@fragment
fn entry() {
  return;
}
)";

    EXPECT_FALSE(ShouldRun<FirstIndexOffset>(src));
}

TEST_F(FirstIndexOffsetTest, ShouldRunVertexStage) {
    auto* src = R"(
@vertex
fn entry() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    EXPECT_TRUE(ShouldRun<FirstIndexOffset>(src));
}

TEST_F(FirstIndexOffsetTest, EmptyModule) {
    auto* src = "";
    auto* expect = "";

    DataMap config;
    config.Add<FirstIndexOffset::BindingPoint>(0, 0);
    auto got = Run<FirstIndexOffset>(src, std::move(config));

    EXPECT_EQ(expect, str(got));

    auto* data = got.data.Get<FirstIndexOffset::Data>();

    EXPECT_EQ(data, nullptr);
}

TEST_F(FirstIndexOffsetTest, BasicVertexShader) {
    auto* src = R"(
@vertex
fn entry() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";
    auto* expect = src;

    DataMap config;
    config.Add<FirstIndexOffset::BindingPoint>(0, 0);
    auto got = Run<FirstIndexOffset>(src, std::move(config));

    EXPECT_EQ(expect, str(got));

    auto* data = got.data.Get<FirstIndexOffset::Data>();

    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->has_vertex_index, false);
    EXPECT_EQ(data->has_instance_index, false);
}

TEST_F(FirstIndexOffsetTest, BasicModuleVertexIndex) {
    auto* src = R"(
fn test(vert_idx : u32) -> u32 {
  return vert_idx;
}

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  test(vert_idx);
  return vec4<f32>();
}
)";

    auto* expect = R"(
struct tint_symbol {
  first_vertex_index : u32,
  first_instance_index : u32,
}

@binding(1) @group(2) var<uniform> tint_symbol_1 : tint_symbol;

fn test(vert_idx : u32) -> u32 {
  return vert_idx;
}

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  test((vert_idx + tint_symbol_1.first_vertex_index));
  return vec4<f32>();
}
)";

    DataMap config;
    config.Add<FirstIndexOffset::BindingPoint>(1, 2);
    auto got = Run<FirstIndexOffset>(src, std::move(config));

    EXPECT_EQ(expect, str(got));

    auto* data = got.data.Get<FirstIndexOffset::Data>();

    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->has_vertex_index, true);
    EXPECT_EQ(data->has_instance_index, false);
}

TEST_F(FirstIndexOffsetTest, BasicModuleVertexIndex_OutOfOrder) {
    auto* src = R"(
@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  test(vert_idx);
  return vec4<f32>();
}

fn test(vert_idx : u32) -> u32 {
  return vert_idx;
}
)";

    auto* expect = R"(
struct tint_symbol {
  first_vertex_index : u32,
  first_instance_index : u32,
}

@binding(1) @group(2) var<uniform> tint_symbol_1 : tint_symbol;

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  test((vert_idx + tint_symbol_1.first_vertex_index));
  return vec4<f32>();
}

fn test(vert_idx : u32) -> u32 {
  return vert_idx;
}
)";

    DataMap config;
    config.Add<FirstIndexOffset::BindingPoint>(1, 2);
    auto got = Run<FirstIndexOffset>(src, std::move(config));

    EXPECT_EQ(expect, str(got));

    auto* data = got.data.Get<FirstIndexOffset::Data>();

    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->has_vertex_index, true);
    EXPECT_EQ(data->has_instance_index, false);
}

TEST_F(FirstIndexOffsetTest, BasicModuleInstanceIndex) {
    auto* src = R"(
fn test(inst_idx : u32) -> u32 {
  return inst_idx;
}

@vertex
fn entry(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  test(inst_idx);
  return vec4<f32>();
}
)";

    auto* expect = R"(
struct tint_symbol {
  first_vertex_index : u32,
  first_instance_index : u32,
}

@binding(1) @group(7) var<uniform> tint_symbol_1 : tint_symbol;

fn test(inst_idx : u32) -> u32 {
  return inst_idx;
}

@vertex
fn entry(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  test((inst_idx + tint_symbol_1.first_instance_index));
  return vec4<f32>();
}
)";

    DataMap config;
    config.Add<FirstIndexOffset::BindingPoint>(1, 7);
    auto got = Run<FirstIndexOffset>(src, std::move(config));

    EXPECT_EQ(expect, str(got));

    auto* data = got.data.Get<FirstIndexOffset::Data>();

    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->has_vertex_index, false);
    EXPECT_EQ(data->has_instance_index, true);
}

TEST_F(FirstIndexOffsetTest, BasicModuleInstanceIndex_OutOfOrder) {
    auto* src = R"(
@vertex
fn entry(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  test(inst_idx);
  return vec4<f32>();
}

fn test(inst_idx : u32) -> u32 {
  return inst_idx;
}
)";

    auto* expect = R"(
struct tint_symbol {
  first_vertex_index : u32,
  first_instance_index : u32,
}

@binding(1) @group(7) var<uniform> tint_symbol_1 : tint_symbol;

@vertex
fn entry(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  test((inst_idx + tint_symbol_1.first_instance_index));
  return vec4<f32>();
}

fn test(inst_idx : u32) -> u32 {
  return inst_idx;
}
)";

    DataMap config;
    config.Add<FirstIndexOffset::BindingPoint>(1, 7);
    auto got = Run<FirstIndexOffset>(src, std::move(config));

    EXPECT_EQ(expect, str(got));

    auto* data = got.data.Get<FirstIndexOffset::Data>();

    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->has_vertex_index, false);
    EXPECT_EQ(data->has_instance_index, true);
}

TEST_F(FirstIndexOffsetTest, BasicModuleBothIndex) {
    auto* src = R"(
fn test(instance_idx : u32, vert_idx : u32) -> u32 {
  return instance_idx + vert_idx;
}

struct Inputs {
  @builtin(instance_index) instance_idx : u32,
  @builtin(vertex_index) vert_idx : u32,
};

@vertex
fn entry(inputs : Inputs) -> @builtin(position) vec4<f32> {
  test(inputs.instance_idx, inputs.vert_idx);
  return vec4<f32>();
}
)";

    auto* expect = R"(
struct tint_symbol {
  first_vertex_index : u32,
  first_instance_index : u32,
}

@binding(1) @group(2) var<uniform> tint_symbol_1 : tint_symbol;

fn test(instance_idx : u32, vert_idx : u32) -> u32 {
  return (instance_idx + vert_idx);
}

struct Inputs {
  @builtin(instance_index)
  instance_idx : u32,
  @builtin(vertex_index)
  vert_idx : u32,
}

@vertex
fn entry(inputs : Inputs) -> @builtin(position) vec4<f32> {
  test((inputs.instance_idx + tint_symbol_1.first_instance_index), (inputs.vert_idx + tint_symbol_1.first_vertex_index));
  return vec4<f32>();
}
)";

    DataMap config;
    config.Add<FirstIndexOffset::BindingPoint>(1, 2);
    auto got = Run<FirstIndexOffset>(src, std::move(config));

    EXPECT_EQ(expect, str(got));

    auto* data = got.data.Get<FirstIndexOffset::Data>();

    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->has_vertex_index, true);
    EXPECT_EQ(data->has_instance_index, true);
}

TEST_F(FirstIndexOffsetTest, BasicModuleBothIndex_OutOfOrder) {
    auto* src = R"(
@vertex
fn entry(inputs : Inputs) -> @builtin(position) vec4<f32> {
  test(inputs.instance_idx, inputs.vert_idx);
  return vec4<f32>();
}

struct Inputs {
  @builtin(instance_index) instance_idx : u32,
  @builtin(vertex_index) vert_idx : u32,
};

fn test(instance_idx : u32, vert_idx : u32) -> u32 {
  return instance_idx + vert_idx;
}
)";

    auto* expect = R"(
struct tint_symbol {
  first_vertex_index : u32,
  first_instance_index : u32,
}

@binding(1) @group(2) var<uniform> tint_symbol_1 : tint_symbol;

@vertex
fn entry(inputs : Inputs) -> @builtin(position) vec4<f32> {
  test((inputs.instance_idx + tint_symbol_1.first_instance_index), (inputs.vert_idx + tint_symbol_1.first_vertex_index));
  return vec4<f32>();
}

struct Inputs {
  @builtin(instance_index)
  instance_idx : u32,
  @builtin(vertex_index)
  vert_idx : u32,
}

fn test(instance_idx : u32, vert_idx : u32) -> u32 {
  return (instance_idx + vert_idx);
}
)";

    DataMap config;
    config.Add<FirstIndexOffset::BindingPoint>(1, 2);
    auto got = Run<FirstIndexOffset>(src, std::move(config));

    EXPECT_EQ(expect, str(got));

    auto* data = got.data.Get<FirstIndexOffset::Data>();

    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->has_vertex_index, true);
    EXPECT_EQ(data->has_instance_index, true);
}

TEST_F(FirstIndexOffsetTest, NestedCalls) {
    auto* src = R"(
fn func1(vert_idx : u32) -> u32 {
  return vert_idx;
}

fn func2(vert_idx : u32) -> u32 {
  return func1(vert_idx);
}

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  func2(vert_idx);
  return vec4<f32>();
}
)";

    auto* expect = R"(
struct tint_symbol {
  first_vertex_index : u32,
  first_instance_index : u32,
}

@binding(1) @group(2) var<uniform> tint_symbol_1 : tint_symbol;

fn func1(vert_idx : u32) -> u32 {
  return vert_idx;
}

fn func2(vert_idx : u32) -> u32 {
  return func1(vert_idx);
}

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  func2((vert_idx + tint_symbol_1.first_vertex_index));
  return vec4<f32>();
}
)";

    DataMap config;
    config.Add<FirstIndexOffset::BindingPoint>(1, 2);
    auto got = Run<FirstIndexOffset>(src, std::move(config));

    EXPECT_EQ(expect, str(got));

    auto* data = got.data.Get<FirstIndexOffset::Data>();

    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->has_vertex_index, true);
    EXPECT_EQ(data->has_instance_index, false);
}

TEST_F(FirstIndexOffsetTest, NestedCalls_OutOfOrder) {
    auto* src = R"(
@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  func2(vert_idx);
  return vec4<f32>();
}

fn func2(vert_idx : u32) -> u32 {
  return func1(vert_idx);
}

fn func1(vert_idx : u32) -> u32 {
  return vert_idx;
}
)";

    auto* expect = R"(
struct tint_symbol {
  first_vertex_index : u32,
  first_instance_index : u32,
}

@binding(1) @group(2) var<uniform> tint_symbol_1 : tint_symbol;

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  func2((vert_idx + tint_symbol_1.first_vertex_index));
  return vec4<f32>();
}

fn func2(vert_idx : u32) -> u32 {
  return func1(vert_idx);
}

fn func1(vert_idx : u32) -> u32 {
  return vert_idx;
}
)";

    DataMap config;
    config.Add<FirstIndexOffset::BindingPoint>(1, 2);
    auto got = Run<FirstIndexOffset>(src, std::move(config));

    EXPECT_EQ(expect, str(got));

    auto* data = got.data.Get<FirstIndexOffset::Data>();

    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->has_vertex_index, true);
    EXPECT_EQ(data->has_instance_index, false);
}

TEST_F(FirstIndexOffsetTest, MultipleEntryPoints) {
    auto* src = R"(
fn func(i : u32) -> u32 {
  return i;
}

@vertex
fn entry_a(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  func(vert_idx);
  return vec4<f32>();
}

@vertex
fn entry_b(@builtin(vertex_index) vert_idx : u32, @builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  func(vert_idx + inst_idx);
  return vec4<f32>();
}

@vertex
fn entry_c(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  func(inst_idx);
  return vec4<f32>();
}
)";

    auto* expect = R"(
struct tint_symbol {
  first_vertex_index : u32,
  first_instance_index : u32,
}

@binding(1) @group(2) var<uniform> tint_symbol_1 : tint_symbol;

fn func(i : u32) -> u32 {
  return i;
}

@vertex
fn entry_a(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  func((vert_idx + tint_symbol_1.first_vertex_index));
  return vec4<f32>();
}

@vertex
fn entry_b(@builtin(vertex_index) vert_idx : u32, @builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  func(((vert_idx + tint_symbol_1.first_vertex_index) + (inst_idx + tint_symbol_1.first_instance_index)));
  return vec4<f32>();
}

@vertex
fn entry_c(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  func((inst_idx + tint_symbol_1.first_instance_index));
  return vec4<f32>();
}
)";

    DataMap config;
    config.Add<FirstIndexOffset::BindingPoint>(1, 2);
    auto got = Run<FirstIndexOffset>(src, std::move(config));

    EXPECT_EQ(expect, str(got));

    auto* data = got.data.Get<FirstIndexOffset::Data>();

    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->has_vertex_index, true);
    EXPECT_EQ(data->has_instance_index, true);
}

TEST_F(FirstIndexOffsetTest, MultipleEntryPoints_OutOfOrder) {
    auto* src = R"(
@vertex
fn entry_a(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  func(vert_idx);
  return vec4<f32>();
}

@vertex
fn entry_b(@builtin(vertex_index) vert_idx : u32, @builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  func(vert_idx + inst_idx);
  return vec4<f32>();
}

@vertex
fn entry_c(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  func(inst_idx);
  return vec4<f32>();
}

fn func(i : u32) -> u32 {
  return i;
}
)";

    auto* expect = R"(
struct tint_symbol {
  first_vertex_index : u32,
  first_instance_index : u32,
}

@binding(1) @group(2) var<uniform> tint_symbol_1 : tint_symbol;

@vertex
fn entry_a(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  func((vert_idx + tint_symbol_1.first_vertex_index));
  return vec4<f32>();
}

@vertex
fn entry_b(@builtin(vertex_index) vert_idx : u32, @builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  func(((vert_idx + tint_symbol_1.first_vertex_index) + (inst_idx + tint_symbol_1.first_instance_index)));
  return vec4<f32>();
}

@vertex
fn entry_c(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  func((inst_idx + tint_symbol_1.first_instance_index));
  return vec4<f32>();
}

fn func(i : u32) -> u32 {
  return i;
}
)";

    DataMap config;
    config.Add<FirstIndexOffset::BindingPoint>(1, 2);
    auto got = Run<FirstIndexOffset>(src, std::move(config));

    EXPECT_EQ(expect, str(got));

    auto* data = got.data.Get<FirstIndexOffset::Data>();

    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->has_vertex_index, true);
    EXPECT_EQ(data->has_instance_index, true);
}

}  // namespace
}  // namespace tint::ast::transform
