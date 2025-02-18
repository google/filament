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

#include <utility>

#include "src/tint/lang/hlsl/writer/ast_raise/pixel_local.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::hlsl::writer {
namespace {

struct ROVBinding {
    uint32_t field_index;
    uint32_t register_index;
    core::TexelFormat rov_format;
};

ast::transform::DataMap Bindings(std::initializer_list<ROVBinding> bindings,
                                 uint32_t groupIndex = 0) {
    PixelLocal::Config cfg;
    cfg.rov_group_index = groupIndex;
    for (auto& binding : bindings) {
        cfg.pls_member_to_rov_reg.Add(binding.field_index, binding.register_index);
        cfg.pls_member_to_rov_format.Add(binding.field_index, binding.rov_format);
    }
    ast::transform::DataMap data;
    data.Add<PixelLocal::Config>(std::move(cfg));
    return data;
}

using HLSLPixelLocalTest = ast::transform::TransformTest;

TEST_F(HLSLPixelLocalTest, EmptyModule) {
    auto* src = "";

    EXPECT_FALSE(ShouldRun<PixelLocal>(src, Bindings({})));
}

TEST_F(HLSLPixelLocalTest, MissingBindings) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn F() -> @location(0) vec4f {
  P.a += 42;
  return vec4f(1, 0, 0, 1);
}
)";

    auto* expect = R"(error: PixelLocal::Config::attachments missing entry for field 0)";
    ast::transform::DataMap data;
    data.Add<PixelLocal::Config>();
    auto got = Run<PixelLocal>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, UseInEntryPoint_NoPosition) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn F() -> @location(0) vec4f {
  P.a += 42;
  return vec4f(1, 0, 0, 1);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(0) @internal(rov) var pixel_local_a : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = textureLoad(pixel_local_a, rov_texcoord).x;
}

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4u(P.a));
}

struct F_res {
  @location(0)
  output_0 : vec4<f32>,
}

@fragment
fn F(@builtin(position) my_pos : vec4<f32>) -> F_res {
  let hlsl_sv_position = my_pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  let result = F_inner();
  store_into_pixel_local_storage(hlsl_sv_position);
  return F_res(result);
}

struct PixelLocal {
  a : u32,
}

var<private> P : PixelLocal;

fn F_inner() -> vec4f {
  P.a += 42;
  return vec4f(1, 0, 0, 1);
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Uint}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, UseInEntryPoint_SeparatePosition) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn F(@builtin(position) pos : vec4f) -> @location(0) vec4f {
  P.a += 42;
  return pos;
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(0) @internal(rov) var pixel_local_a : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = textureLoad(pixel_local_a, rov_texcoord).x;
}

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4u(P.a));
}

struct F_res {
  @location(0)
  output_0 : vec4<f32>,
}

@fragment
fn F(@builtin(position) pos : vec4f) -> F_res {
  let hlsl_sv_position = pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  let result = F_inner(pos);
  store_into_pixel_local_storage(hlsl_sv_position);
  return F_res(result);
}

struct PixelLocal {
  a : u32,
}

var<private> P : PixelLocal;

fn F_inner(pos : vec4f) -> vec4f {
  P.a += 42;
  return pos;
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Uint}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, UseInEntryPoint_PositionInStruct) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

struct FragmentInput {
  @location(0) input : vec4f,
  @builtin(position) pos : vec4f,
}

@fragment
fn F(fragmentInput : FragmentInput) -> @location(0) vec4f {
  P.a += 42;
  return fragmentInput.input + fragmentInput.pos;
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(0) @internal(rov) var pixel_local_a : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = textureLoad(pixel_local_a, rov_texcoord).x;
}

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4u(P.a));
}

struct F_res {
  @location(0)
  output_0 : vec4<f32>,
}

@fragment
fn F(fragmentInput : FragmentInput) -> F_res {
  let hlsl_sv_position = fragmentInput.pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  let result = F_inner(fragmentInput);
  store_into_pixel_local_storage(hlsl_sv_position);
  return F_res(result);
}

struct PixelLocal {
  a : u32,
}

var<private> P : PixelLocal;

struct FragmentInput {
  @location(0)
  input : vec4f,
  @builtin(position)
  pos : vec4f,
}

fn F_inner(fragmentInput : FragmentInput) -> vec4f {
  P.a += 42;
  return (fragmentInput.input + fragmentInput.pos);
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Uint}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, UseInEntryPoint_NonZeroROVGroupIndex) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

struct FragmentInput {
  @location(0) input : vec4f,
  @builtin(position) pos : vec4f,
}

@fragment
fn F(fragmentInput : FragmentInput) -> @location(0) vec4f {
  P.a += 42;
  return fragmentInput.input + fragmentInput.pos;
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(3) @internal(rov) var pixel_local_a : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = textureLoad(pixel_local_a, rov_texcoord).x;
}

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4u(P.a));
}

struct F_res {
  @location(0)
  output_0 : vec4<f32>,
}

@fragment
fn F(fragmentInput : FragmentInput) -> F_res {
  let hlsl_sv_position = fragmentInput.pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  let result = F_inner(fragmentInput);
  store_into_pixel_local_storage(hlsl_sv_position);
  return F_res(result);
}

struct PixelLocal {
  a : u32,
}

var<private> P : PixelLocal;

struct FragmentInput {
  @location(0)
  input : vec4f,
  @builtin(position)
  pos : vec4f,
}

fn F_inner(fragmentInput : FragmentInput) -> vec4f {
  P.a += 42;
  return (fragmentInput.input + fragmentInput.pos);
}
)";

    constexpr uint32_t kROVGroupIndex = 3u;
    auto got =
        Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Uint}}, kROVGroupIndex));

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, UseInEntryPoint_Multiple_PixelLocal_Members) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
  d : u32,
}

var<pixel_local> P : PixelLocal;

struct FragmentInput {
  @location(0) input : vec4f,
  @builtin(position) pos : vec4f,
}

@fragment
fn F(fragmentInput : FragmentInput) -> @location(0) vec4f {
  P.a += 42;
  P.b -= 21;
  P.c += 12.5f;
  P.d -= 5;
  return fragmentInput.input + fragmentInput.pos;
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(0) @internal(rov) var pixel_local_a : texture_storage_2d<r32uint, read_write>;

@binding(2) @group(0) @internal(rov) var pixel_local_b : texture_storage_2d<r32sint, read_write>;

@binding(3) @group(0) @internal(rov) var pixel_local_c : texture_storage_2d<r32float, read_write>;

@binding(4) @group(0) @internal(rov) var pixel_local_d : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = textureLoad(pixel_local_a, rov_texcoord).x;
  P.b = textureLoad(pixel_local_b, rov_texcoord).x;
  P.c = textureLoad(pixel_local_c, rov_texcoord).x;
  P.d = textureLoad(pixel_local_d, rov_texcoord).x;
}

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4u(P.a));
  textureStore(pixel_local_b, rov_texcoord, vec4i(P.b));
  textureStore(pixel_local_c, rov_texcoord, vec4f(P.c));
  textureStore(pixel_local_d, rov_texcoord, vec4u(P.d));
}

struct F_res {
  @location(0)
  output_0 : vec4<f32>,
}

@fragment
fn F(fragmentInput : FragmentInput) -> F_res {
  let hlsl_sv_position = fragmentInput.pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  let result = F_inner(fragmentInput);
  store_into_pixel_local_storage(hlsl_sv_position);
  return F_res(result);
}

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
  d : u32,
}

var<private> P : PixelLocal;

struct FragmentInput {
  @location(0)
  input : vec4f,
  @builtin(position)
  pos : vec4f,
}

fn F_inner(fragmentInput : FragmentInput) -> vec4f {
  P.a += 42;
  P.b -= 21;
  P.c += 12.5f;
  P.d -= 5;
  return (fragmentInput.input + fragmentInput.pos);
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Uint},
                                              {1, 2, core::TexelFormat::kR32Sint},
                                              {2, 3, core::TexelFormat::kR32Float},
                                              {3, 4, core::TexelFormat::kR32Uint}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, UseInEntryPoint_Multiple_PixelLocal_Members_And_Fragment_Output) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
  d : u32,
}

var<pixel_local> P : PixelLocal;

struct FragmentInput {
  @location(0) input : vec4f,
  @builtin(position) pos : vec4f,
}

struct FragmentOutput {
  @location(0) color0 : vec4f,
  @location(1) color1 : vec4f,
}

@fragment
fn F(fragmentInput : FragmentInput) -> FragmentOutput {
  P.a += 42;
  P.b -= 21;
  P.c += 12.5f;
  P.d -= 5;
  return FragmentOutput(fragmentInput.input, fragmentInput.pos);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(0) @internal(rov) var pixel_local_a : texture_storage_2d<r32uint, read_write>;

@binding(2) @group(0) @internal(rov) var pixel_local_b : texture_storage_2d<r32sint, read_write>;

@binding(3) @group(0) @internal(rov) var pixel_local_c : texture_storage_2d<r32float, read_write>;

@binding(4) @group(0) @internal(rov) var pixel_local_d : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = textureLoad(pixel_local_a, rov_texcoord).x;
  P.b = textureLoad(pixel_local_b, rov_texcoord).x;
  P.c = textureLoad(pixel_local_c, rov_texcoord).x;
  P.d = textureLoad(pixel_local_d, rov_texcoord).x;
}

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4u(P.a));
  textureStore(pixel_local_b, rov_texcoord, vec4i(P.b));
  textureStore(pixel_local_c, rov_texcoord, vec4f(P.c));
  textureStore(pixel_local_d, rov_texcoord, vec4u(P.d));
}

struct F_res {
  @location(0)
  output_0 : vec4<f32>,
  @location(1)
  output_1 : vec4<f32>,
}

@fragment
fn F(fragmentInput : FragmentInput) -> F_res {
  let hlsl_sv_position = fragmentInput.pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  let result = F_inner(fragmentInput);
  store_into_pixel_local_storage(hlsl_sv_position);
  return F_res(result.color0, result.color1);
}

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
  d : u32,
}

var<private> P : PixelLocal;

struct FragmentInput {
  @location(0)
  input : vec4f,
  @builtin(position)
  pos : vec4f,
}

struct FragmentOutput {
  color0 : vec4f,
  color1 : vec4f,
}

fn F_inner(fragmentInput : FragmentInput) -> FragmentOutput {
  P.a += 42;
  P.b -= 21;
  P.c += 12.5f;
  P.d -= 5;
  return FragmentOutput(fragmentInput.input, fragmentInput.pos);
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Uint},
                                              {1, 2, core::TexelFormat::kR32Sint},
                                              {2, 3, core::TexelFormat::kR32Float},
                                              {3, 4, core::TexelFormat::kR32Uint}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, ROV_Format_Different_From_Pixel_Local_Member_Format) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
  d : u32,
}

var<pixel_local> P : PixelLocal;

struct FragmentInput {
  @location(0) input : vec4f,
  @builtin(position) pos : vec4f,
}

@fragment
fn F(fragmentInput : FragmentInput) -> @location(0) vec4f {
  P.a += 42;
  P.b -= 21;
  P.c += 12.5f;
  P.d -= 5;
  return fragmentInput.input + fragmentInput.pos;
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(0) @internal(rov) var pixel_local_a : texture_storage_2d<r32sint, read_write>;

@binding(2) @group(0) @internal(rov) var pixel_local_b : texture_storage_2d<r32float, read_write>;

@binding(3) @group(0) @internal(rov) var pixel_local_c : texture_storage_2d<r32uint, read_write>;

@binding(4) @group(0) @internal(rov) var pixel_local_d : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = bitcast<u32>(textureLoad(pixel_local_a, rov_texcoord).x);
  P.b = bitcast<i32>(textureLoad(pixel_local_b, rov_texcoord).x);
  P.c = bitcast<f32>(textureLoad(pixel_local_c, rov_texcoord).x);
  P.d = textureLoad(pixel_local_d, rov_texcoord).x;
}

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4i(bitcast<i32>(P.a)));
  textureStore(pixel_local_b, rov_texcoord, vec4f(bitcast<f32>(P.b)));
  textureStore(pixel_local_c, rov_texcoord, vec4u(bitcast<u32>(P.c)));
  textureStore(pixel_local_d, rov_texcoord, vec4u(P.d));
}

struct F_res {
  @location(0)
  output_0 : vec4<f32>,
}

@fragment
fn F(fragmentInput : FragmentInput) -> F_res {
  let hlsl_sv_position = fragmentInput.pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  let result = F_inner(fragmentInput);
  store_into_pixel_local_storage(hlsl_sv_position);
  return F_res(result);
}

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
  d : u32,
}

var<private> P : PixelLocal;

struct FragmentInput {
  @location(0)
  input : vec4f,
  @builtin(position)
  pos : vec4f,
}

fn F_inner(fragmentInput : FragmentInput) -> vec4f {
  P.a += 42;
  P.b -= 21;
  P.c += 12.5f;
  P.d -= 5;
  return (fragmentInput.input + fragmentInput.pos);
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Sint},
                                              {1, 2, core::TexelFormat::kR32Float},
                                              {2, 3, core::TexelFormat::kR32Uint},
                                              {3, 4, core::TexelFormat::kR32Uint}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, UseInCallee) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

fn X() {
  P.a += 42;
}

@fragment
fn F() {
  X();
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(0) @internal(rov) var pixel_local_a : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = textureLoad(pixel_local_a, rov_texcoord).x;
}

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4u(P.a));
}

@fragment
fn F(@builtin(position) my_pos : vec4<f32>) {
  let hlsl_sv_position = my_pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  F_inner();
  store_into_pixel_local_storage(hlsl_sv_position);
}

struct PixelLocal {
  a : u32,
}

var<private> P : PixelLocal;

fn X() {
  P.a += 42;
}

fn F_inner() {
  X();
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Uint}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, ROVLoadFunctionNameUsed) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  P.a += u32(my_input.x);
}

@fragment
fn F() {
  load_from_pixel_local_storage(vec4f(1.0, 0.0, 0.0, 1.0));
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(0) @internal(rov) var pixel_local_a : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage_1(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = textureLoad(pixel_local_a, rov_texcoord).x;
}

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4u(P.a));
}

@fragment
fn F(@builtin(position) my_pos : vec4<f32>) {
  let hlsl_sv_position = my_pos;
  load_from_pixel_local_storage_1(hlsl_sv_position);
  F_inner();
  store_into_pixel_local_storage(hlsl_sv_position);
}

struct PixelLocal {
  a : u32,
}

var<private> P : PixelLocal;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  P.a += u32(my_input.x);
}

fn F_inner() {
  load_from_pixel_local_storage(vec4f(1.0, 0.0, 0.0, 1.0));
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Uint}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, ROVStoreFunctionNameUsed) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  P.a += u32(my_input.x);
}

@fragment
fn F() {
  store_into_pixel_local_storage(vec4f(1.0, 0.0, 0.0, 1.0));
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(0) @internal(rov) var pixel_local_a : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = textureLoad(pixel_local_a, rov_texcoord).x;
}

fn store_into_pixel_local_storage_1(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4u(P.a));
}

@fragment
fn F(@builtin(position) my_pos : vec4<f32>) {
  let hlsl_sv_position = my_pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  F_inner();
  store_into_pixel_local_storage_1(hlsl_sv_position);
}

struct PixelLocal {
  a : u32,
}

var<private> P : PixelLocal;

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  P.a += u32(my_input.x);
}

fn F_inner() {
  store_into_pixel_local_storage(vec4f(1.0, 0.0, 0.0, 1.0));
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Uint}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, NewBuiltinPositionVariableNamesUsed) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn F(@location(0) my_pos : vec4f) {
  let hlsl_sv_position = my_pos * 2;
  P.a += u32(hlsl_sv_position.y);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(0) @internal(rov) var pixel_local_a : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = textureLoad(pixel_local_a, rov_texcoord).x;
}

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4u(P.a));
}

@fragment
fn F(@location(0) my_pos : vec4f, @builtin(position) my_pos_1 : vec4<f32>) {
  let hlsl_sv_position_1 = my_pos_1;
  load_from_pixel_local_storage(hlsl_sv_position_1);
  F_inner(my_pos);
  store_into_pixel_local_storage(hlsl_sv_position_1);
}

struct PixelLocal {
  a : u32,
}

var<private> P : PixelLocal;

fn F_inner(my_pos : vec4f) {
  let hlsl_sv_position = (my_pos * 2);
  P.a += u32(hlsl_sv_position.y);
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Uint}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, WithInvariantBuiltinInputParameter) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn F(@invariant @builtin(position) pos : vec4f) {
  P.a += u32(pos.x);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(0) @internal(rov) var pixel_local_a : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = textureLoad(pixel_local_a, rov_texcoord).x;
}

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4u(P.a));
}

@fragment
fn F(@invariant @builtin(position) pos : vec4f) {
  let hlsl_sv_position = pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  F_inner(pos);
  store_into_pixel_local_storage(hlsl_sv_position);
}

struct PixelLocal {
  a : u32,
}

var<private> P : PixelLocal;

fn F_inner(pos : vec4f) {
  P.a += u32(pos.x);
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Uint}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, WithInvariantBuiltinInputStructParameter) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

struct In {
  @invariant @builtin(position) pos : vec4f,
}

@fragment
fn F(in : In) {
  P.a += u32(in.pos.x);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(0) @internal(rov) var pixel_local_a : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = textureLoad(pixel_local_a, rov_texcoord).x;
}

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4u(P.a));
}

@fragment
fn F(in : In) {
  let hlsl_sv_position = in.pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  F_inner(in);
  store_into_pixel_local_storage(hlsl_sv_position);
}

struct PixelLocal {
  a : u32,
}

var<private> P : PixelLocal;

struct In {
  @invariant @builtin(position)
  pos : vec4f,
}

fn F_inner(in : In) {
  P.a += u32(in.pos.x);
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Uint}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, WithLocationInputParameter) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn F(@location(0) a : vec4f, @interpolate(flat) @location(1) b : vec4f) {
  P.a += u32(a.x) + u32(b.y);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(0) @internal(rov) var pixel_local_a : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = textureLoad(pixel_local_a, rov_texcoord).x;
}

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4u(P.a));
}

@fragment
fn F(@location(0) a : vec4f, @interpolate(flat) @location(1) b : vec4f, @builtin(position) my_pos : vec4<f32>) {
  let hlsl_sv_position = my_pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  F_inner(a, b);
  store_into_pixel_local_storage(hlsl_sv_position);
}

struct PixelLocal {
  a : u32,
}

var<private> P : PixelLocal;

fn F_inner(a : vec4f, b : vec4f) {
  P.a += (u32(a.x) + u32(b.y));
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Uint}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, WithLocationInputStructParameter) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

struct In {
  @location(0) a : vec4f,
  @interpolate(flat) @location(1) b : vec4f,
}

@fragment
fn F(in : In) {
  P.a += u32(in.a.x) + u32(in.b.y);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(0) @internal(rov) var pixel_local_a : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = textureLoad(pixel_local_a, rov_texcoord).x;
}

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4u(P.a));
}

@fragment
fn F(in : In, @builtin(position) my_pos : vec4<f32>) {
  let hlsl_sv_position = my_pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  F_inner(in);
  store_into_pixel_local_storage(hlsl_sv_position);
}

struct PixelLocal {
  a : u32,
}

var<private> P : PixelLocal;

struct In {
  @location(0)
  a : vec4f,
  @interpolate(flat) @location(1)
  b : vec4f,
}

fn F_inner(in : In) {
  P.a += (u32(in.a.x) + u32(in.b.y));
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Uint}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, WithBuiltinAndLocationInputParameter) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn F(@builtin(position) pos : vec4f, @location(0) uv : vec4f) {
  P.a += u32(pos.x) + u32(uv.x);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(0) @internal(rov) var pixel_local_a : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = textureLoad(pixel_local_a, rov_texcoord).x;
}

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4u(P.a));
}

@fragment
fn F(@builtin(position) pos : vec4f, @location(0) uv : vec4f) {
  let hlsl_sv_position = pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  F_inner(pos, uv);
  store_into_pixel_local_storage(hlsl_sv_position);
}

struct PixelLocal {
  a : u32,
}

var<private> P : PixelLocal;

fn F_inner(pos : vec4f, uv : vec4f) {
  P.a += (u32(pos.x) + u32(uv.x));
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Uint}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(HLSLPixelLocalTest, WithBuiltinAndLocationInputStructParameter) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

struct In {
  @builtin(position) pos : vec4f,
  @location(0) uv : vec4f,
}

@fragment
fn F(in : In) {
  P.a += u32(in.pos.x) + u32(in.uv.x);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

@binding(1) @group(0) @internal(rov) var pixel_local_a : texture_storage_2d<r32uint, read_write>;

fn load_from_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  P.a = textureLoad(pixel_local_a, rov_texcoord).x;
}

fn store_into_pixel_local_storage(my_input : vec4<f32>) {
  let rov_texcoord = vec2u(my_input.xy);
  textureStore(pixel_local_a, rov_texcoord, vec4u(P.a));
}

@fragment
fn F(in : In) {
  let hlsl_sv_position = in.pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  F_inner(in);
  store_into_pixel_local_storage(hlsl_sv_position);
}

struct PixelLocal {
  a : u32,
}

var<private> P : PixelLocal;

struct In {
  @builtin(position)
  pos : vec4f,
  @location(0)
  uv : vec4f,
}

fn F_inner(in : In) {
  P.a += (u32(in.pos.x) + u32(in.uv.x));
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1, core::TexelFormat::kR32Uint}}));

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::hlsl::writer
