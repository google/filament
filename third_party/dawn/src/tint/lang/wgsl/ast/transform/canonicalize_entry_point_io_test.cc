// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/transform/canonicalize_entry_point_io.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"
#include "src/tint/lang/wgsl/ast/transform/unshadow.h"

namespace tint::ast::transform {
namespace {

using CanonicalizeEntryPointIOTest = TransformTest;

TEST_F(CanonicalizeEntryPointIOTest, Error_MissingTransformData) {
    auto* src = "";

    auto* expect =
        "error: missing transform data for tint::ast::transform::CanonicalizeEntryPointIO";

    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, NoShaderIO) {
    // Test that we do not introduce wrapper functions when there is no shader IO
    // to process.
    auto* src = R"(
@fragment
fn frag_main() {
}

@compute @workgroup_size(1)
fn comp_main() {
}
)";

    auto* expect = src;

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Parameters_Msl) {
    auto* src = R"(
enable chromium_experimental_framebuffer_fetch;

@fragment
fn frag_main(@location(1) loc1 : f32,
             @location(2) @interpolate(flat) loc2 : vec4<u32>,
             @builtin(position) coord : vec4<f32>,
             @color(3) color : vec4<f32>) {
  var col : f32 = (coord.x * loc1) + color.g;
}
)";

    auto* expect = R"(
enable chromium_experimental_framebuffer_fetch;

struct tint_symbol_1 {
  @color(3)
  color : vec4<f32>,
  @location(1)
  loc1 : f32,
  @location(2) @interpolate(flat)
  loc2 : vec4<u32>,
}

fn frag_main_inner(loc1 : f32, loc2 : vec4<u32>, coord : vec4<f32>, color : vec4<f32>) {
  var col : f32 = ((coord.x * loc1) + color.g);
}

@fragment
fn frag_main(@builtin(position) coord : vec4<f32>, tint_symbol : tint_symbol_1) {
  frag_main_inner(tint_symbol.loc1, tint_symbol.loc2, coord, tint_symbol.color);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Parameters_Hlsl) {
    auto* src = R"(
enable chromium_experimental_framebuffer_fetch;

@fragment
fn frag_main(@location(1) loc1 : f32,
             @location(2) @interpolate(flat) loc2 : vec4<u32>,
             @builtin(position) coord : vec4<f32>,
             @color(3) color : vec4<f32>) {
  var col : f32 = (coord.x * loc1) + color.g;
}
)";

    auto* expect = R"(
enable chromium_experimental_framebuffer_fetch;

struct tint_symbol_1 {
  @color(3)
  color : vec4<f32>,
  @location(1)
  loc1 : f32,
  @location(2) @interpolate(flat)
  loc2 : vec4<u32>,
  @builtin(position)
  coord : vec4<f32>,
}

fn frag_main_inner(loc1 : f32, loc2 : vec4<u32>, coord : vec4<f32>, color : vec4<f32>) {
  var col : f32 = ((coord.x * loc1) + color.g);
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) {
  frag_main_inner(tint_symbol.loc1, tint_symbol.loc2, vec4<f32>(tint_symbol.coord.xyz, (1 / tint_symbol.coord.w)), tint_symbol.color);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Parameter_TypeAlias) {
    auto* src = R"(
alias myf32 = f32;

@fragment
fn frag_main(@location(1) loc1 : myf32) {
  var x : myf32 = loc1;
}
)";

    auto* expect = R"(
alias myf32 = f32;

struct tint_symbol_1 {
  @location(1)
  loc1 : f32,
}

fn frag_main_inner(loc1 : myf32) {
  var x : myf32 = loc1;
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) {
  frag_main_inner(tint_symbol.loc1);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Parameter_TypeAlias_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main(@location(1) loc1 : myf32) {
  var x : myf32 = loc1;
}

alias myf32 = f32;
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(1)
  loc1 : f32,
}

fn frag_main_inner(loc1 : myf32) {
  var x : myf32 = loc1;
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) {
  frag_main_inner(tint_symbol.loc1);
}

alias myf32 = f32;
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_kMsl) {
    auto* src = R"(
enable chromium_experimental_framebuffer_fetch;

struct FragBuiltins {
  @builtin(position) coord : vec4<f32>,
};
struct FragLocations {
  @location(1) loc1 : f32,
  @location(2) @interpolate(flat) loc2 : vec4<u32>,
};
struct FragColors {
  @color(3) col3 : vec4<f32>,
  @color(1) col1 : vec4<u32>,
  @color(2) col2 : vec4<i32>,
};

@fragment
fn frag_main(@location(0) loc0 : f32,
             locations : FragLocations,
             builtins : FragBuiltins,
             colors : FragColors) {
  var col : f32 = (((builtins.coord.x * locations.loc1) + loc0) + colors.col3.g);
}
)";

    auto* expect = R"(
enable chromium_experimental_framebuffer_fetch;

struct FragBuiltins {
  coord : vec4<f32>,
}

struct FragLocations {
  loc1 : f32,
  loc2 : vec4<u32>,
}

struct FragColors {
  col3 : vec4<f32>,
  col1 : vec4<u32>,
  col2 : vec4<i32>,
}

struct tint_symbol_1 {
  @color(1)
  col1 : vec4<u32>,
  @color(2)
  col2 : vec4<i32>,
  @color(3)
  col3 : vec4<f32>,
  @location(0)
  loc0 : f32,
  @location(1)
  loc1 : f32,
  @location(2) @interpolate(flat)
  loc2 : vec4<u32>,
}

fn frag_main_inner(loc0 : f32, locations : FragLocations, builtins : FragBuiltins, colors : FragColors) {
  var col : f32 = (((builtins.coord.x * locations.loc1) + loc0) + colors.col3.g);
}

@fragment
fn frag_main(@builtin(position) coord : vec4<f32>, tint_symbol : tint_symbol_1) {
  frag_main_inner(tint_symbol.loc0, FragLocations(tint_symbol.loc1, tint_symbol.loc2), FragBuiltins(coord), FragColors(tint_symbol.col3, tint_symbol.col1, tint_symbol.col2));
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_kMsl_OutOfOrder) {
    auto* src = R"(
enable chromium_experimental_framebuffer_fetch;

@fragment
fn frag_main(@location(0) loc0 : f32,
             locations : FragLocations,
             builtins : FragBuiltins,
             colors : FragColors) {
  var col : f32 = (((builtins.coord.x * locations.loc1) + loc0) + colors.col3.g);
}

struct FragBuiltins {
  @builtin(position) coord : vec4<f32>,
};
struct FragLocations {
  @location(1) loc1 : f32,
  @location(2) @interpolate(flat) loc2 : vec4<u32>,
};
struct FragColors {
  @color(3) col3 : vec4<f32>,
  @color(1) col1 : vec4<u32>,
  @color(2) col2 : vec4<i32>,
};
)";

    auto* expect = R"(
enable chromium_experimental_framebuffer_fetch;

struct tint_symbol_1 {
  @color(1)
  col1 : vec4<u32>,
  @color(2)
  col2 : vec4<i32>,
  @color(3)
  col3 : vec4<f32>,
  @location(0)
  loc0 : f32,
  @location(1)
  loc1 : f32,
  @location(2) @interpolate(flat)
  loc2 : vec4<u32>,
}

fn frag_main_inner(loc0 : f32, locations : FragLocations, builtins : FragBuiltins, colors : FragColors) {
  var col : f32 = (((builtins.coord.x * locations.loc1) + loc0) + colors.col3.g);
}

@fragment
fn frag_main(@builtin(position) coord : vec4<f32>, tint_symbol : tint_symbol_1) {
  frag_main_inner(tint_symbol.loc0, FragLocations(tint_symbol.loc1, tint_symbol.loc2), FragBuiltins(coord), FragColors(tint_symbol.col3, tint_symbol.col1, tint_symbol.col2));
}

struct FragBuiltins {
  coord : vec4<f32>,
}

struct FragLocations {
  loc1 : f32,
  loc2 : vec4<u32>,
}

struct FragColors {
  col3 : vec4<f32>,
  col1 : vec4<u32>,
  col2 : vec4<i32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_Hlsl) {
    auto* src = R"(
enable chromium_experimental_framebuffer_fetch;

struct FragBuiltins {
  @builtin(position) coord : vec4<f32>,
};
struct FragLocations {
  @location(1) loc1 : f32,
  @location(2) @interpolate(flat) loc2 : vec4<u32>,
};
struct FragColors {
  @color(3) col3 : vec4<f32>,
  @color(1) col1 : vec4<u32>,
  @color(2) col2 : vec4<i32>,
};

@fragment
fn frag_main(@location(0) loc0 : f32,
             locations : FragLocations,
             builtins : FragBuiltins,
             colors : FragColors) {
  var col : f32 = (((builtins.coord.x * locations.loc1) + loc0) + colors.col3.g);
}
)";

    auto* expect = R"(
enable chromium_experimental_framebuffer_fetch;

struct FragBuiltins {
  coord : vec4<f32>,
}

struct FragLocations {
  loc1 : f32,
  loc2 : vec4<u32>,
}

struct FragColors {
  col3 : vec4<f32>,
  col1 : vec4<u32>,
  col2 : vec4<i32>,
}

struct tint_symbol_1 {
  @color(1)
  col1 : vec4<u32>,
  @color(2)
  col2 : vec4<i32>,
  @color(3)
  col3 : vec4<f32>,
  @location(0)
  loc0 : f32,
  @location(1)
  loc1 : f32,
  @location(2) @interpolate(flat)
  loc2 : vec4<u32>,
  @builtin(position)
  coord : vec4<f32>,
}

fn frag_main_inner(loc0 : f32, locations : FragLocations, builtins : FragBuiltins, colors : FragColors) {
  var col : f32 = (((builtins.coord.x * locations.loc1) + loc0) + colors.col3.g);
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) {
  frag_main_inner(tint_symbol.loc0, FragLocations(tint_symbol.loc1, tint_symbol.loc2), FragBuiltins(vec4<f32>(tint_symbol.coord.xyz, (1 / tint_symbol.coord.w))), FragColors(tint_symbol.col3, tint_symbol.col1, tint_symbol.col2));
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_Hlsl_OutOfOrder) {
    auto* src = R"(
enable chromium_experimental_framebuffer_fetch;

@fragment
fn frag_main(@location(0) loc0 : f32,
             locations : FragLocations,
             builtins : FragBuiltins,
             colors : FragColors) {
  var col : f32 = (((builtins.coord.x * locations.loc1) + loc0) + colors.col3.g);
}

struct FragBuiltins {
  @builtin(position) coord : vec4<f32>,
};
struct FragLocations {
  @location(1) loc1 : f32,
  @location(2) @interpolate(flat) loc2 : vec4<u32>,
};
struct FragColors {
  @color(3) col3 : vec4<f32>,
  @color(1) col1 : vec4<u32>,
  @color(2) col2 : vec4<i32>,
};
)";

    auto* expect = R"(
enable chromium_experimental_framebuffer_fetch;

struct tint_symbol_1 {
  @color(1)
  col1 : vec4<u32>,
  @color(2)
  col2 : vec4<i32>,
  @color(3)
  col3 : vec4<f32>,
  @location(0)
  loc0 : f32,
  @location(1)
  loc1 : f32,
  @location(2) @interpolate(flat)
  loc2 : vec4<u32>,
  @builtin(position)
  coord : vec4<f32>,
}

fn frag_main_inner(loc0 : f32, locations : FragLocations, builtins : FragBuiltins, colors : FragColors) {
  var col : f32 = (((builtins.coord.x * locations.loc1) + loc0) + colors.col3.g);
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) {
  frag_main_inner(tint_symbol.loc0, FragLocations(tint_symbol.loc1, tint_symbol.loc2), FragBuiltins(vec4<f32>(tint_symbol.coord.xyz, (1 / tint_symbol.coord.w))), FragColors(tint_symbol.col3, tint_symbol.col1, tint_symbol.col2));
}

struct FragBuiltins {
  coord : vec4<f32>,
}

struct FragLocations {
  loc1 : f32,
  loc2 : vec4<u32>,
}

struct FragColors {
  col3 : vec4<f32>,
  col1 : vec4<u32>,
  col2 : vec4<i32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_NonStruct_Msl) {
    auto* src = R"(
@fragment
fn frag_main() -> @builtin(frag_depth) f32 {
  return 1.0;
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(frag_depth)
  value : f32,
}

fn frag_main_inner() -> f32 {
  return 1.0;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_NonStruct_Hlsl) {
    auto* src = R"(
@fragment
fn frag_main() -> @builtin(frag_depth) f32 {
  return 1.0;
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(frag_depth)
  value : f32,
}

fn frag_main_inner() -> f32 {
  return 1.0;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_Struct_Msl) {
    auto* src = R"(
struct FragOutput {
  @location(0) color : vec4<f32>,
  @builtin(frag_depth) depth : f32,
  @builtin(sample_mask) mask : u32,
};

@fragment
fn frag_main() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}
)";

    auto* expect = R"(
struct FragOutput {
  color : vec4<f32>,
  depth : f32,
  mask : u32,
}

struct tint_symbol {
  @location(0)
  color : vec4<f32>,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  mask : u32,
}

fn frag_main_inner() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.color = inner_result.color;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.mask = inner_result.mask;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_Struct_Msl_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

struct FragOutput {
  @location(0) color : vec4<f32>,
  @builtin(frag_depth) depth : f32,
  @builtin(sample_mask) mask : u32,
};
)";

    auto* expect = R"(
struct tint_symbol {
  @location(0)
  color : vec4<f32>,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  mask : u32,
}

fn frag_main_inner() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.color = inner_result.color;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.mask = inner_result.mask;
  return wrapper_result;
}

struct FragOutput {
  color : vec4<f32>,
  depth : f32,
  mask : u32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_Struct_Hlsl) {
    auto* src = R"(
struct FragOutput {
  @location(0) color : vec4<f32>,
  @builtin(frag_depth) depth : f32,
  @builtin(sample_mask) mask : u32,
};

@fragment
fn frag_main() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}
)";

    auto* expect = R"(
struct FragOutput {
  color : vec4<f32>,
  depth : f32,
  mask : u32,
}

struct tint_symbol {
  @location(0)
  color : vec4<f32>,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  mask : u32,
}

fn frag_main_inner() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.color = inner_result.color;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.mask = inner_result.mask;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_Struct_Hlsl_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

struct FragOutput {
  @location(0) color : vec4<f32>,
  @builtin(frag_depth) depth : f32,
  @builtin(sample_mask) mask : u32,
};
)";

    auto* expect = R"(
struct tint_symbol {
  @location(0)
  color : vec4<f32>,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  mask : u32,
}

fn frag_main_inner() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.color = inner_result.color;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.mask = inner_result.mask;
  return wrapper_result;
}

struct FragOutput {
  color : vec4<f32>,
  depth : f32,
  mask : u32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_SharedDeviceFunction_Msl) {
    auto* src = R"(
struct FragmentInput {
  @location(0) value : f32,
  @location(1) mul : f32,
};

fn foo(x : FragmentInput) -> f32 {
  return x.value * x.mul;
}

@fragment
fn frag_main1(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}
)";

    auto* expect = R"(
struct FragmentInput {
  value : f32,
  mul : f32,
}

fn foo(x : FragmentInput) -> f32 {
  return (x.value * x.mul);
}

struct tint_symbol_1 {
  @location(0)
  value : f32,
  @location(1)
  mul : f32,
}

fn frag_main1_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main1(tint_symbol : tint_symbol_1) {
  frag_main1_inner(FragmentInput(tint_symbol.value, tint_symbol.mul));
}

struct tint_symbol_3 {
  @location(0)
  value : f32,
  @location(1)
  mul : f32,
}

fn frag_main2_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(tint_symbol_2 : tint_symbol_3) {
  frag_main2_inner(FragmentInput(tint_symbol_2.value, tint_symbol_2.mul));
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_SharedDeviceFunction_Msl_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main1(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

fn foo(x : FragmentInput) -> f32 {
  return x.value * x.mul;
}

struct FragmentInput {
  @location(0) value : f32,
  @location(1) mul : f32,
};
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(0)
  value : f32,
  @location(1)
  mul : f32,
}

fn frag_main1_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main1(tint_symbol : tint_symbol_1) {
  frag_main1_inner(FragmentInput(tint_symbol.value, tint_symbol.mul));
}

struct tint_symbol_3 {
  @location(0)
  value : f32,
  @location(1)
  mul : f32,
}

fn frag_main2_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(tint_symbol_2 : tint_symbol_3) {
  frag_main2_inner(FragmentInput(tint_symbol_2.value, tint_symbol_2.mul));
}

fn foo(x : FragmentInput) -> f32 {
  return (x.value * x.mul);
}

struct FragmentInput {
  value : f32,
  mul : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_SharedDeviceFunction_Hlsl) {
    auto* src = R"(
struct FragmentInput {
  @location(0) value : f32,
  @location(1) mul : f32,
};

fn foo(x : FragmentInput) -> f32 {
  return x.value * x.mul;
}

@fragment
fn frag_main1(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}
)";

    auto* expect = R"(
struct FragmentInput {
  value : f32,
  mul : f32,
}

fn foo(x : FragmentInput) -> f32 {
  return (x.value * x.mul);
}

struct tint_symbol_1 {
  @location(0)
  value : f32,
  @location(1)
  mul : f32,
}

fn frag_main1_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main1(tint_symbol : tint_symbol_1) {
  frag_main1_inner(FragmentInput(tint_symbol.value, tint_symbol.mul));
}

struct tint_symbol_3 {
  @location(0)
  value : f32,
  @location(1)
  mul : f32,
}

fn frag_main2_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(tint_symbol_2 : tint_symbol_3) {
  frag_main2_inner(FragmentInput(tint_symbol_2.value, tint_symbol_2.mul));
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_SharedDeviceFunction_Hlsl_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main1(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

fn foo(x : FragmentInput) -> f32 {
  return x.value * x.mul;
}

struct FragmentInput {
  @location(0) value : f32,
  @location(1) mul : f32,
};
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(0)
  value : f32,
  @location(1)
  mul : f32,
}

fn frag_main1_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main1(tint_symbol : tint_symbol_1) {
  frag_main1_inner(FragmentInput(tint_symbol.value, tint_symbol.mul));
}

struct tint_symbol_3 {
  @location(0)
  value : f32,
  @location(1)
  mul : f32,
}

fn frag_main2_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(tint_symbol_2 : tint_symbol_3) {
  frag_main2_inner(FragmentInput(tint_symbol_2.value, tint_symbol_2.mul));
}

fn foo(x : FragmentInput) -> f32 {
  return (x.value * x.mul);
}

struct FragmentInput {
  value : f32,
  mul : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Struct_ModuleScopeVariable) {
    auto* src = R"(
struct FragmentInput {
  @location(0) col1 : f32,
  @location(1) col2 : f32,
};

var<private> global_inputs : FragmentInput;

fn foo() -> f32 {
  return global_inputs.col1 * 0.5;
}

fn bar() -> f32 {
  return global_inputs.col2 * 2.0;
}

@fragment
fn frag_main1(inputs : FragmentInput) {
 global_inputs = inputs;
 var r : f32 = foo();
 var g : f32 = bar();
}
)";

    auto* expect = R"(
struct FragmentInput {
  col1 : f32,
  col2 : f32,
}

var<private> global_inputs : FragmentInput;

fn foo() -> f32 {
  return (global_inputs.col1 * 0.5);
}

fn bar() -> f32 {
  return (global_inputs.col2 * 2.0);
}

struct tint_symbol_1 {
  @location(0)
  col1 : f32,
  @location(1)
  col2 : f32,
}

fn frag_main1_inner(inputs : FragmentInput) {
  global_inputs = inputs;
  var r : f32 = foo();
  var g : f32 = bar();
}

@fragment
fn frag_main1(tint_symbol : tint_symbol_1) {
  frag_main1_inner(FragmentInput(tint_symbol.col1, tint_symbol.col2));
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Struct_ModuleScopeVariable_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main1(inputs : FragmentInput) {
 global_inputs = inputs;
 var r : f32 = foo();
 var g : f32 = bar();
}

fn foo() -> f32 {
  return global_inputs.col1 * 0.5;
}

fn bar() -> f32 {
  return global_inputs.col2 * 2.0;
}

var<private> global_inputs : FragmentInput;

struct FragmentInput {
  @location(0) col1 : f32,
  @location(1) col2 : f32,
};
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(0)
  col1 : f32,
  @location(1)
  col2 : f32,
}

fn frag_main1_inner(inputs : FragmentInput) {
  global_inputs = inputs;
  var r : f32 = foo();
  var g : f32 = bar();
}

@fragment
fn frag_main1(tint_symbol : tint_symbol_1) {
  frag_main1_inner(FragmentInput(tint_symbol.col1, tint_symbol.col2));
}

fn foo() -> f32 {
  return (global_inputs.col1 * 0.5);
}

fn bar() -> f32 {
  return (global_inputs.col2 * 2.0);
}

var<private> global_inputs : FragmentInput;

struct FragmentInput {
  col1 : f32,
  col2 : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Struct_TypeAliases) {
    auto* src = R"(
alias myf32 = f32;

struct FragmentInput {
  @location(0) col1 : myf32,
  @location(1) col2 : myf32,
};

struct FragmentOutput {
  @location(0) col1 : myf32,
  @location(1) col2 : myf32,
};

alias MyFragmentInput = FragmentInput;

alias MyFragmentOutput = FragmentOutput;

fn foo(x : MyFragmentInput) -> myf32 {
  return x.col1;
}

@fragment
fn frag_main(inputs : MyFragmentInput) -> MyFragmentOutput {
  var x : myf32 = foo(inputs);
  return MyFragmentOutput(x, inputs.col2);
}
)";

    auto* expect = R"(
alias myf32 = f32;

struct FragmentInput {
  col1 : myf32,
  col2 : myf32,
}

struct FragmentOutput {
  col1 : myf32,
  col2 : myf32,
}

alias MyFragmentInput = FragmentInput;

alias MyFragmentOutput = FragmentOutput;

fn foo(x : MyFragmentInput) -> myf32 {
  return x.col1;
}

struct tint_symbol_1 {
  @location(0)
  col1 : f32,
  @location(1)
  col2 : f32,
}

struct tint_symbol_2 {
  @location(0)
  col1 : f32,
  @location(1)
  col2 : f32,
}

fn frag_main_inner(inputs : MyFragmentInput) -> MyFragmentOutput {
  var x : myf32 = foo(inputs);
  return MyFragmentOutput(x, inputs.col2);
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inner_result = frag_main_inner(MyFragmentInput(tint_symbol.col1, tint_symbol.col2));
  var wrapper_result : tint_symbol_2;
  wrapper_result.col1 = inner_result.col1;
  wrapper_result.col2 = inner_result.col2;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Struct_TypeAliases_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main(inputs : MyFragmentInput) -> MyFragmentOutput {
  var x : myf32 = foo(inputs);
  return MyFragmentOutput(x, inputs.col2);
}

alias MyFragmentInput = FragmentInput;

alias MyFragmentOutput = FragmentOutput;

fn foo(x : MyFragmentInput) -> myf32 {
  return x.col1;
}

struct FragmentInput {
  @location(0) col1 : myf32,
  @location(1) col2 : myf32,
};

struct FragmentOutput {
  @location(0) col1 : myf32,
  @location(1) col2 : myf32,
};

alias myf32 = f32;
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(0)
  col1 : f32,
  @location(1)
  col2 : f32,
}

struct tint_symbol_2 {
  @location(0)
  col1 : f32,
  @location(1)
  col2 : f32,
}

fn frag_main_inner(inputs : MyFragmentInput) -> MyFragmentOutput {
  var x : myf32 = foo(inputs);
  return MyFragmentOutput(x, inputs.col2);
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inner_result = frag_main_inner(MyFragmentInput(tint_symbol.col1, tint_symbol.col2));
  var wrapper_result : tint_symbol_2;
  wrapper_result.col1 = inner_result.col1;
  wrapper_result.col2 = inner_result.col2;
  return wrapper_result;
}

alias MyFragmentInput = FragmentInput;

alias MyFragmentOutput = FragmentOutput;

fn foo(x : MyFragmentInput) -> myf32 {
  return x.col1;
}

struct FragmentInput {
  col1 : myf32,
  col2 : myf32,
}

struct FragmentOutput {
  col1 : myf32,
  col2 : myf32,
}

alias myf32 = f32;
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, InterpolateAttributes) {
    auto* src = R"(
struct VertexOut {
  @builtin(position) pos : vec4<f32>,
  @location(1) @interpolate(flat) loc1 : f32,
  @location(2) @interpolate(linear, sample) loc2 : f32,
  @location(3) @interpolate(perspective, centroid) loc3 : f32,
};

struct FragmentIn {
  @location(1) @interpolate(flat) loc1 : f32,
  @location(2) @interpolate(linear, sample) loc2 : f32,
};

@vertex
fn vert_main() -> VertexOut {
  return VertexOut();
}

@fragment
fn frag_main(inputs : FragmentIn,
             @location(3) @interpolate(perspective, centroid) loc3 : f32) {
  let x = inputs.loc1 + inputs.loc2 + loc3;
}
)";

    auto* expect = R"(
struct VertexOut {
  pos : vec4<f32>,
  loc1 : f32,
  loc2 : f32,
  loc3 : f32,
}

struct FragmentIn {
  loc1 : f32,
  loc2 : f32,
}

struct tint_symbol {
  @location(1) @interpolate(flat)
  loc1 : f32,
  @location(2) @interpolate(linear, sample)
  loc2 : f32,
  @location(3) @interpolate(perspective, centroid)
  loc3 : f32,
  @builtin(position)
  pos : vec4<f32>,
}

fn vert_main_inner() -> VertexOut {
  return VertexOut();
}

@vertex
fn vert_main() -> tint_symbol {
  let inner_result = vert_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.loc1 = inner_result.loc1;
  wrapper_result.loc2 = inner_result.loc2;
  wrapper_result.loc3 = inner_result.loc3;
  return wrapper_result;
}

struct tint_symbol_2 {
  @location(1) @interpolate(flat)
  loc1 : f32,
  @location(2) @interpolate(linear, sample)
  loc2 : f32,
  @location(3) @interpolate(perspective, centroid)
  loc3 : f32,
}

fn frag_main_inner(inputs : FragmentIn, loc3 : f32) {
  let x = ((inputs.loc1 + inputs.loc2) + loc3);
}

@fragment
fn frag_main(tint_symbol_1 : tint_symbol_2) {
  frag_main_inner(FragmentIn(tint_symbol_1.loc1, tint_symbol_1.loc2), tint_symbol_1.loc3);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, InterpolateAttributes_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main(inputs : FragmentIn,
             @location(3) @interpolate(perspective, centroid) loc3 : f32) {
  let x = inputs.loc1 + inputs.loc2 + loc3;
}

@vertex
fn vert_main() -> VertexOut {
  return VertexOut();
}

struct VertexOut {
  @builtin(position) pos : vec4<f32>,
  @location(1) @interpolate(flat) loc1 : f32,
  @location(2) @interpolate(linear, sample) loc2 : f32,
  @location(3) @interpolate(perspective, centroid) loc3 : f32,
};

struct FragmentIn {
  @location(1) @interpolate(flat) loc1: f32,
  @location(2) @interpolate(linear, sample) loc2 : f32,
};
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(1) @interpolate(flat)
  loc1 : f32,
  @location(2) @interpolate(linear, sample)
  loc2 : f32,
  @location(3) @interpolate(perspective, centroid)
  loc3 : f32,
}

fn frag_main_inner(inputs : FragmentIn, loc3 : f32) {
  let x = ((inputs.loc1 + inputs.loc2) + loc3);
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) {
  frag_main_inner(FragmentIn(tint_symbol.loc1, tint_symbol.loc2), tint_symbol.loc3);
}

struct tint_symbol_2 {
  @location(1) @interpolate(flat)
  loc1 : f32,
  @location(2) @interpolate(linear, sample)
  loc2 : f32,
  @location(3) @interpolate(perspective, centroid)
  loc3 : f32,
  @builtin(position)
  pos : vec4<f32>,
}

fn vert_main_inner() -> VertexOut {
  return VertexOut();
}

@vertex
fn vert_main() -> tint_symbol_2 {
  let inner_result = vert_main_inner();
  var wrapper_result : tint_symbol_2;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.loc1 = inner_result.loc1;
  wrapper_result.loc2 = inner_result.loc2;
  wrapper_result.loc3 = inner_result.loc3;
  return wrapper_result;
}

struct VertexOut {
  pos : vec4<f32>,
  loc1 : f32,
  loc2 : f32,
  loc3 : f32,
}

struct FragmentIn {
  loc1 : f32,
  loc2 : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, InvariantAttributes) {
    auto* src = R"(
struct VertexOut {
  @builtin(position) @invariant pos : vec4<f32>,
};

@vertex
fn main1() -> VertexOut {
  return VertexOut();
}

@vertex
fn main2() -> @builtin(position) @invariant vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = R"(
struct VertexOut {
  pos : vec4<f32>,
}

struct tint_symbol {
  @builtin(position) @invariant
  pos : vec4<f32>,
}

fn main1_inner() -> VertexOut {
  return VertexOut();
}

@vertex
fn main1() -> tint_symbol {
  let inner_result = main1_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}

struct tint_symbol_1 {
  @builtin(position) @invariant
  value : vec4<f32>,
}

fn main2_inner() -> vec4<f32> {
  return vec4<f32>();
}

@vertex
fn main2() -> tint_symbol_1 {
  let inner_result_1 = main2_inner();
  var wrapper_result_1 : tint_symbol_1;
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, InvariantAttributes_OutOfOrder) {
    auto* src = R"(
@vertex
fn main1() -> VertexOut {
  return VertexOut();
}

@vertex
fn main2() -> @builtin(position) @invariant vec4<f32> {
  return vec4<f32>();
}

struct VertexOut {
  @builtin(position) @invariant pos : vec4<f32>,
};
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(position) @invariant
  pos : vec4<f32>,
}

fn main1_inner() -> VertexOut {
  return VertexOut();
}

@vertex
fn main1() -> tint_symbol {
  let inner_result = main1_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}

struct tint_symbol_1 {
  @builtin(position) @invariant
  value : vec4<f32>,
}

fn main2_inner() -> vec4<f32> {
  return vec4<f32>();
}

@vertex
fn main2() -> tint_symbol_1 {
  let inner_result_1 = main2_inner();
  var wrapper_result_1 : tint_symbol_1;
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}

struct VertexOut {
  pos : vec4<f32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Struct_LayoutAttributes) {
    auto* src = R"(
struct FragmentInput {
  @size(16) @location(1) value : f32,
  @builtin(position) @align(32) coord : vec4<f32>,
  @location(0) @interpolate(linear, sample) @align(128) loc0 : f32,
};

struct FragmentOutput {
  @size(16) @location(1) @interpolate(flat) value : f32,
};

@fragment
fn frag_main(inputs : FragmentInput) -> FragmentOutput {
  return FragmentOutput(inputs.coord.x * inputs.value + inputs.loc0);
}
)";

    auto* expect = R"(
struct FragmentInput {
  @size(16)
  value : f32,
  @align(32)
  coord : vec4<f32>,
  @align(128)
  loc0 : f32,
}

struct FragmentOutput {
  @size(16)
  value : f32,
}

struct tint_symbol_1 {
  @location(0) @interpolate(linear, sample)
  loc0 : f32,
  @location(1)
  value : f32,
  @builtin(position)
  coord : vec4<f32>,
}

struct tint_symbol_2 {
  @location(1)
  value : f32,
}

fn frag_main_inner(inputs : FragmentInput) -> FragmentOutput {
  return FragmentOutput(((inputs.coord.x * inputs.value) + inputs.loc0));
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inner_result = frag_main_inner(FragmentInput(tint_symbol.value, vec4<f32>(tint_symbol.coord.xyz, (1 / tint_symbol.coord.w)), tint_symbol.loc0));
  var wrapper_result : tint_symbol_2;
  wrapper_result.value = inner_result.value;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Struct_LayoutAttributes_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main(inputs : FragmentInput) -> FragmentOutput {
  return FragmentOutput(inputs.coord.x * inputs.value + inputs.loc0);
}

struct FragmentInput {
  @size(16) @location(1) value : f32,
  @builtin(position) @align(32) coord : vec4<f32>,
  @location(0) @interpolate(linear, sample) @align(128) loc0 : f32,
};

struct FragmentOutput {
  @size(16) @location(1) @interpolate(flat) value : f32,
};
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(0) @interpolate(linear, sample)
  loc0 : f32,
  @location(1)
  value : f32,
  @builtin(position)
  coord : vec4<f32>,
}

struct tint_symbol_2 {
  @location(1)
  value : f32,
}

fn frag_main_inner(inputs : FragmentInput) -> FragmentOutput {
  return FragmentOutput(((inputs.coord.x * inputs.value) + inputs.loc0));
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inner_result = frag_main_inner(FragmentInput(tint_symbol.value, vec4<f32>(tint_symbol.coord.xyz, (1 / tint_symbol.coord.w)), tint_symbol.loc0));
  var wrapper_result : tint_symbol_2;
  wrapper_result.value = inner_result.value;
  return wrapper_result;
}

struct FragmentInput {
  @size(16)
  value : f32,
  @align(32)
  coord : vec4<f32>,
  @align(128)
  loc0 : f32,
}

struct FragmentOutput {
  @size(16)
  value : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, SortedMembers) {
    auto* src = R"(
struct VertexOutput {
  @location(1) @interpolate(flat) b : u32,
  @builtin(position) pos : vec4<f32>,
  @location(3) @interpolate(flat) d : u32,
  @location(0) a : f32,
  @location(2) @interpolate(flat) c : i32,
};

struct FragmentInputExtra {
  @location(3) @interpolate(flat) d : u32,
  @builtin(position) pos : vec4<f32>,
  @location(0) a : f32,
};

@vertex
fn vert_main() -> VertexOutput {
  return VertexOutput();
}

@fragment
fn frag_main(@builtin(front_facing) ff : bool,
             @location(2) @interpolate(flat) c : i32,
             inputs : FragmentInputExtra,
             @location(1) @interpolate(flat) b : u32) {
}
)";

    auto* expect = R"(
struct VertexOutput {
  b : u32,
  pos : vec4<f32>,
  d : u32,
  a : f32,
  c : i32,
}

struct FragmentInputExtra {
  d : u32,
  pos : vec4<f32>,
  a : f32,
}

struct tint_symbol {
  @location(0)
  a : f32,
  @location(1) @interpolate(flat)
  b : u32,
  @location(2) @interpolate(flat)
  c : i32,
  @location(3) @interpolate(flat)
  d : u32,
  @builtin(position)
  pos : vec4<f32>,
}

fn vert_main_inner() -> VertexOutput {
  return VertexOutput();
}

@vertex
fn vert_main() -> tint_symbol {
  let inner_result = vert_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.b = inner_result.b;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.d = inner_result.d;
  wrapper_result.a = inner_result.a;
  wrapper_result.c = inner_result.c;
  return wrapper_result;
}

struct tint_symbol_2 {
  @location(0)
  a : f32,
  @location(1) @interpolate(flat)
  b : u32,
  @location(2) @interpolate(flat)
  c : i32,
  @location(3) @interpolate(flat)
  d : u32,
  @builtin(position)
  pos : vec4<f32>,
  @builtin(front_facing)
  ff : bool,
}

fn frag_main_inner(ff : bool, c : i32, inputs : FragmentInputExtra, b : u32) {
}

@fragment
fn frag_main(tint_symbol_1 : tint_symbol_2) {
  frag_main_inner(tint_symbol_1.ff, tint_symbol_1.c, FragmentInputExtra(tint_symbol_1.d, vec4<f32>(tint_symbol_1.pos.xyz, (1 / tint_symbol_1.pos.w)), tint_symbol_1.a), tint_symbol_1.b);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, SortedMembers_OutOfOrder) {
    auto* src = R"(
@vertex
fn vert_main() -> VertexOutput {
  return VertexOutput();
}

@fragment
fn frag_main(@builtin(front_facing) ff : bool,
             @location(2) @interpolate(flat) c : i32,
             inputs : FragmentInputExtra,
             @location(1) @interpolate(flat) b : u32) {
}

struct VertexOutput {
  @location(1) @interpolate(flat) b : u32,
  @builtin(position) pos : vec4<f32>,
  @location(3) @interpolate(flat) d : u32,
  @location(0) a : f32,
  @location(2) @interpolate(flat) c : i32,
};

struct FragmentInputExtra {
  @location(3) @interpolate(flat) d : u32,
  @builtin(position) pos : vec4<f32>,
  @location(0) a : f32,
};
)";

    auto* expect = R"(
struct tint_symbol {
  @location(0)
  a : f32,
  @location(1) @interpolate(flat)
  b : u32,
  @location(2) @interpolate(flat)
  c : i32,
  @location(3) @interpolate(flat)
  d : u32,
  @builtin(position)
  pos : vec4<f32>,
}

fn vert_main_inner() -> VertexOutput {
  return VertexOutput();
}

@vertex
fn vert_main() -> tint_symbol {
  let inner_result = vert_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.b = inner_result.b;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.d = inner_result.d;
  wrapper_result.a = inner_result.a;
  wrapper_result.c = inner_result.c;
  return wrapper_result;
}

struct tint_symbol_2 {
  @location(0)
  a : f32,
  @location(1) @interpolate(flat)
  b : u32,
  @location(2) @interpolate(flat)
  c : i32,
  @location(3) @interpolate(flat)
  d : u32,
  @builtin(position)
  pos : vec4<f32>,
  @builtin(front_facing)
  ff : bool,
}

fn frag_main_inner(ff : bool, c : i32, inputs : FragmentInputExtra, b : u32) {
}

@fragment
fn frag_main(tint_symbol_1 : tint_symbol_2) {
  frag_main_inner(tint_symbol_1.ff, tint_symbol_1.c, FragmentInputExtra(tint_symbol_1.d, vec4<f32>(tint_symbol_1.pos.xyz, (1 / tint_symbol_1.pos.w)), tint_symbol_1.a), tint_symbol_1.b);
}

struct VertexOutput {
  b : u32,
  pos : vec4<f32>,
  d : u32,
  a : f32,
  c : i32,
}

struct FragmentInputExtra {
  d : u32,
  pos : vec4<f32>,
  a : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, DontRenameSymbols) {
    auto* src = R"(
@fragment
fn tint_symbol_1(@location(0) col : f32) {
}
)";

    auto* expect = R"(
struct tint_symbol_2 {
  @location(0)
  col : f32,
}

fn tint_symbol_1_inner(col : f32) {
}

@fragment
fn tint_symbol_1(tint_symbol : tint_symbol_2) {
  tint_symbol_1_inner(tint_symbol.col);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_VoidNoReturn) {
    auto* src = R"(
@fragment
fn frag_main() {
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(sample_mask)
  fixed_sample_mask : u32,
}

fn frag_main_inner() {
}

@fragment
fn frag_main() -> tint_symbol {
  frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.fixed_sample_mask = 3u;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_VoidWithReturn) {
    auto* src = R"(
@fragment
fn frag_main() {
  return;
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(sample_mask)
  fixed_sample_mask : u32,
}

fn frag_main_inner() {
  return;
}

@fragment
fn frag_main() -> tint_symbol {
  frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.fixed_sample_mask = 3u;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_WithAuthoredMask) {
    auto* src = R"(
@fragment
fn frag_main() -> @builtin(sample_mask) u32 {
  return 7u;
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(sample_mask)
  value : u32,
}

fn frag_main_inner() -> u32 {
  return 7u;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.value = (inner_result & 3u);
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_WithoutAuthoredMask) {
    auto* src = R"(
@fragment
fn frag_main() -> @location(0) f32 {
  return 1.0;
}
)";

    auto* expect = R"(
struct tint_symbol {
  @location(0)
  value : f32,
  @builtin(sample_mask)
  fixed_sample_mask : u32,
}

fn frag_main_inner() -> f32 {
  return 1.0;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.value = inner_result;
  wrapper_result.fixed_sample_mask = 3u;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_StructWithAuthoredMask) {
    auto* src = R"(
struct Output {
  @builtin(frag_depth) depth : f32,
  @builtin(sample_mask) mask : u32,
  @location(0) value : f32,
};

@fragment
fn frag_main() -> Output {
  return Output(0.5, 7u, 1.0);
}
)";

    auto* expect = R"(
struct Output {
  depth : f32,
  mask : u32,
  value : f32,
}

struct tint_symbol {
  @location(0)
  value : f32,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  mask : u32,
}

fn frag_main_inner() -> Output {
  return Output(0.5, 7u, 1.0);
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.mask = (inner_result.mask & 3u);
  wrapper_result.value = inner_result.value;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_StructWithAuthoredMask_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main() -> Output {
  return Output(0.5, 7u, 1.0);
}

struct Output {
  @builtin(frag_depth) depth : f32,
  @builtin(sample_mask) mask : u32,
  @location(0) value : f32,
};
)";

    auto* expect = R"(
struct tint_symbol {
  @location(0)
  value : f32,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  mask : u32,
}

fn frag_main_inner() -> Output {
  return Output(0.5, 7u, 1.0);
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.mask = (inner_result.mask & 3u);
  wrapper_result.value = inner_result.value;
  return wrapper_result;
}

struct Output {
  depth : f32,
  mask : u32,
  value : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_StructWithoutAuthoredMask) {
    auto* src = R"(
struct Output {
  @builtin(frag_depth) depth : f32,
  @location(0) value : f32,
};

@fragment
fn frag_main() -> Output {
  return Output(0.5, 1.0);
}
)";

    auto* expect = R"(
struct Output {
  depth : f32,
  value : f32,
}

struct tint_symbol {
  @location(0)
  value : f32,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  fixed_sample_mask : u32,
}

fn frag_main_inner() -> Output {
  return Output(0.5, 1.0);
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.value = inner_result.value;
  wrapper_result.fixed_sample_mask = 3u;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_StructWithoutAuthoredMask_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main() -> Output {
  return Output(0.5, 1.0);
}

struct Output {
  @builtin(frag_depth) depth : f32,
  @location(0) value : f32,
};
)";

    auto* expect = R"(
struct tint_symbol {
  @location(0)
  value : f32,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  fixed_sample_mask : u32,
}

fn frag_main_inner() -> Output {
  return Output(0.5, 1.0);
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.value = inner_result.value;
  wrapper_result.fixed_sample_mask = 3u;
  return wrapper_result;
}

struct Output {
  depth : f32,
  value : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_MultipleShaders) {
    auto* src = R"(
@fragment
fn frag_main1() -> @builtin(sample_mask) u32 {
  return 7u;
}

@fragment
fn frag_main2() -> @location(0) f32 {
  return 1.0;
}

@vertex
fn vert_main1() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}

@compute @workgroup_size(1)
fn comp_main1() {
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(sample_mask)
  value : u32,
}

fn frag_main1_inner() -> u32 {
  return 7u;
}

@fragment
fn frag_main1() -> tint_symbol {
  let inner_result = frag_main1_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.value = (inner_result & 3u);
  return wrapper_result;
}

struct tint_symbol_1 {
  @location(0)
  value : f32,
  @builtin(sample_mask)
  fixed_sample_mask : u32,
}

fn frag_main2_inner() -> f32 {
  return 1.0;
}

@fragment
fn frag_main2() -> tint_symbol_1 {
  let inner_result_1 = frag_main2_inner();
  var wrapper_result_1 : tint_symbol_1;
  wrapper_result_1.value = inner_result_1;
  wrapper_result_1.fixed_sample_mask = 3u;
  return wrapper_result_1;
}

struct tint_symbol_2 {
  @builtin(position)
  value : vec4<f32>,
}

fn vert_main1_inner() -> vec4<f32> {
  return vec4<f32>();
}

@vertex
fn vert_main1() -> tint_symbol_2 {
  let inner_result_2 = vert_main1_inner();
  var wrapper_result_2 : tint_symbol_2;
  wrapper_result_2.value = inner_result_2;
  return wrapper_result_2;
}

@compute @workgroup_size(1)
fn comp_main1() {
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_AvoidNameClash) {
    auto* src = R"(
struct FragOut {
  @location(0) fixed_sample_mask : vec4<f32>,
  @location(1) fixed_sample_mask_1 : vec4<f32>,
};

@fragment
fn frag_main() -> FragOut {
  return FragOut();
}
)";

    auto* expect = R"(
struct FragOut {
  fixed_sample_mask : vec4<f32>,
  fixed_sample_mask_1 : vec4<f32>,
}

struct tint_symbol {
  @location(0)
  fixed_sample_mask : vec4<f32>,
  @location(1)
  fixed_sample_mask_1 : vec4<f32>,
  @builtin(sample_mask)
  fixed_sample_mask_2 : u32,
}

fn frag_main_inner() -> FragOut {
  return FragOut();
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.fixed_sample_mask = inner_result.fixed_sample_mask;
  wrapper_result.fixed_sample_mask_1 = inner_result.fixed_sample_mask_1;
  wrapper_result.fixed_sample_mask_2 = 3u;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_ReturnNonStruct_Msl) {
    auto* src = R"(
@vertex
fn vert_main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(position)
  value : vec4<f32>,
  @builtin(__point_size)
  vertex_point_size : f32,
}

fn vert_main_inner() -> vec4<f32> {
  return vec4<f32>();
}

@vertex
fn vert_main() -> tint_symbol {
  let inner_result = vert_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.value = inner_result;
  wrapper_result.vertex_point_size = 1.0f;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_ReturnStruct_Msl) {
    auto* src = R"(
struct VertOut {
  @builtin(position) pos : vec4<f32>,
};

@vertex
fn vert_main() -> VertOut {
  return VertOut();
}
)";

    auto* expect = R"(
struct VertOut {
  pos : vec4<f32>,
}

struct tint_symbol {
  @builtin(position)
  pos : vec4<f32>,
  @builtin(__point_size)
  vertex_point_size : f32,
}

fn vert_main_inner() -> VertOut {
  return VertOut();
}

@vertex
fn vert_main() -> tint_symbol {
  let inner_result = vert_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.vertex_point_size = 1.0f;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_ReturnStruct_Msl_OutOfOrder) {
    auto* src = R"(
@vertex
fn vert_main() -> VertOut {
  return VertOut();
}

struct VertOut {
  @builtin(position) pos : vec4<f32>,
};
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(position)
  pos : vec4<f32>,
  @builtin(__point_size)
  vertex_point_size : f32,
}

fn vert_main_inner() -> VertOut {
  return VertOut();
}

@vertex
fn vert_main() -> tint_symbol {
  let inner_result = vert_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.vertex_point_size = 1.0f;
  return wrapper_result;
}

struct VertOut {
  pos : vec4<f32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_AvoidNameClash_Msl) {
    auto* src = R"(
struct VertIn1 {
  @location(0) collide : f32,
};

struct VertIn2 {
  @location(1) collide : f32,
};

struct VertOut {
  @location(0) vertex_point_size : vec4<f32>,
  @builtin(position) vertex_point_size_1 : vec4<f32>,
};

@vertex
fn vert_main(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = collide.collide + collide_1.collide;
  return VertOut();
}
)";

    auto* expect = R"(
struct VertIn1 {
  collide : f32,
}

struct VertIn2 {
  collide : f32,
}

struct VertOut {
  vertex_point_size : vec4<f32>,
  vertex_point_size_1 : vec4<f32>,
}

struct tint_symbol_1 {
  @location(0)
  collide : f32,
  @location(1)
  collide_2 : f32,
}

struct tint_symbol_2 {
  @location(0)
  vertex_point_size : vec4<f32>,
  @builtin(position)
  vertex_point_size_1 : vec4<f32>,
  @builtin(__point_size)
  vertex_point_size_2 : f32,
}

fn vert_main_inner(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = (collide.collide + collide_1.collide);
  return VertOut();
}

@vertex
fn vert_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inner_result = vert_main_inner(VertIn1(tint_symbol.collide), VertIn2(tint_symbol.collide_2));
  var wrapper_result : tint_symbol_2;
  wrapper_result.vertex_point_size = inner_result.vertex_point_size;
  wrapper_result.vertex_point_size_1 = inner_result.vertex_point_size_1;
  wrapper_result.vertex_point_size_2 = 1.0f;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_AvoidNameClash_Msl_OutOfOrder) {
    auto* src = R"(
@vertex
fn vert_main(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = collide.collide + collide_1.collide;
  return VertOut();
}

struct VertIn1 {
  @location(0) collide : f32,
};

struct VertIn2 {
  @location(1) collide : f32,
};

struct VertOut {
  @location(0) vertex_point_size : vec4<f32>,
  @builtin(position) vertex_point_size_1 : vec4<f32>,
};
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(0)
  collide : f32,
  @location(1)
  collide_2 : f32,
}

struct tint_symbol_2 {
  @location(0)
  vertex_point_size : vec4<f32>,
  @builtin(position)
  vertex_point_size_1 : vec4<f32>,
  @builtin(__point_size)
  vertex_point_size_2 : f32,
}

fn vert_main_inner(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = (collide.collide + collide_1.collide);
  return VertOut();
}

@vertex
fn vert_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inner_result = vert_main_inner(VertIn1(tint_symbol.collide), VertIn2(tint_symbol.collide_2));
  var wrapper_result : tint_symbol_2;
  wrapper_result.vertex_point_size = inner_result.vertex_point_size;
  wrapper_result.vertex_point_size_1 = inner_result.vertex_point_size_1;
  wrapper_result.vertex_point_size_2 = 1.0f;
  return wrapper_result;
}

struct VertIn1 {
  collide : f32,
}

struct VertIn2 {
  collide : f32,
}

struct VertOut {
  vertex_point_size : vec4<f32>,
  vertex_point_size_1 : vec4<f32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_AvoidNameClash_Hlsl) {
    auto* src = R"(
struct VertIn1 {
  @location(0) collide : f32,
};

struct VertIn2 {
  @location(1) collide : f32,
};

struct VertOut {
  @location(0) vertex_point_size : vec4<f32>,
  @builtin(position) vertex_point_size_1 : vec4<f32>,
};

@vertex
fn vert_main(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = collide.collide + collide_1.collide;
  return VertOut();
}
)";

    auto* expect = R"(
struct VertIn1 {
  collide : f32,
}

struct VertIn2 {
  collide : f32,
}

struct VertOut {
  vertex_point_size : vec4<f32>,
  vertex_point_size_1 : vec4<f32>,
}

struct tint_symbol_1 {
  @location(0)
  collide : f32,
  @location(1)
  collide_2 : f32,
}

struct tint_symbol_2 {
  @location(0)
  vertex_point_size : vec4<f32>,
  @builtin(position)
  vertex_point_size_1 : vec4<f32>,
  @builtin(__point_size)
  vertex_point_size_2 : f32,
}

fn vert_main_inner(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = (collide.collide + collide_1.collide);
  return VertOut();
}

@vertex
fn vert_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inner_result = vert_main_inner(VertIn1(tint_symbol.collide), VertIn2(tint_symbol.collide_2));
  var wrapper_result : tint_symbol_2;
  wrapper_result.vertex_point_size = inner_result.vertex_point_size;
  wrapper_result.vertex_point_size_1 = inner_result.vertex_point_size_1;
  wrapper_result.vertex_point_size_2 = 1.0f;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_AvoidNameClash_Hlsl_OutOfOrder) {
    auto* src = R"(
@vertex
fn vert_main(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = collide.collide + collide_1.collide;
  return VertOut();
}

struct VertIn1 {
  @location(0) collide : f32,
};

struct VertIn2 {
  @location(1) collide : f32,
};

struct VertOut {
  @location(0) vertex_point_size : vec4<f32>,
  @builtin(position) vertex_point_size_1 : vec4<f32>,
};
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(0)
  collide : f32,
  @location(1)
  collide_2 : f32,
}

struct tint_symbol_2 {
  @location(0)
  vertex_point_size : vec4<f32>,
  @builtin(position)
  vertex_point_size_1 : vec4<f32>,
  @builtin(__point_size)
  vertex_point_size_2 : f32,
}

fn vert_main_inner(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = (collide.collide + collide_1.collide);
  return VertOut();
}

@vertex
fn vert_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inner_result = vert_main_inner(VertIn1(tint_symbol.collide), VertIn2(tint_symbol.collide_2));
  var wrapper_result : tint_symbol_2;
  wrapper_result.vertex_point_size = inner_result.vertex_point_size;
  wrapper_result.vertex_point_size_1 = inner_result.vertex_point_size_1;
  wrapper_result.vertex_point_size_2 = 1.0f;
  return wrapper_result;
}

struct VertIn1 {
  collide : f32,
}

struct VertIn2 {
  collide : f32,
}

struct VertOut {
  vertex_point_size : vec4<f32>,
  vertex_point_size_1 : vec4<f32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_Struct_Index_Attribute_Msl) {
    auto* src = R"(
enable dual_source_blending;

struct FragOutput {
  @location(0) @blend_src(0) color : vec4<f32>,
  @location(0) @blend_src(1) blend : vec4<f32>,
  @builtin(frag_depth) depth : f32,
  @builtin(sample_mask) mask : u32,
};

@fragment
fn frag_main() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  output.blend = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}
)";

    auto* expect = R"(
enable dual_source_blending;

struct FragOutput {
  color : vec4<f32>,
  blend : vec4<f32>,
  depth : f32,
  mask : u32,
}

struct tint_symbol {
  @location(0) @blend_src(0)
  color : vec4<f32>,
  @location(0) @blend_src(1)
  blend : vec4<f32>,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  mask : u32,
}

fn frag_main_inner() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  output.blend = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.color = inner_result.color;
  wrapper_result.blend = inner_result.blend;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.mask = inner_result.mask;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_Struct_Index_Attribute_Hlsl) {
    auto* src = R"(
enable dual_source_blending;

struct FragOutput {
  @location(0) @blend_src(0) color : vec4<f32>,
  @location(0) @blend_src(1) blend : vec4<f32>,
  @builtin(frag_depth) depth : f32,
  @builtin(sample_mask) mask : u32,
};

@fragment
fn frag_main() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  output.blend = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}
)";

    auto* expect = R"(
enable dual_source_blending;

struct FragOutput {
  color : vec4<f32>,
  blend : vec4<f32>,
  depth : f32,
  mask : u32,
}

struct tint_symbol {
  @location(0) @blend_src(0)
  color : vec4<f32>,
  @location(0) @blend_src(1)
  blend : vec4<f32>,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  mask : u32,
}

fn frag_main_inner() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  output.blend = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.color = inner_result.color;
  wrapper_result.blend = inner_result.blend;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.mask = inner_result.mask;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, SubgroupBuiltins_Hlsl) {
    auto* src = R"(
enable subgroups;

@compute @workgroup_size(64)
fn frag_main(@builtin(subgroup_invocation_id) id : u32,
             @builtin(subgroup_size) size : u32) {
  let x = size - id;
}
)";

    auto* expect = R"(
enable subgroups;

@internal(intrinsic_wave_get_lane_index) @internal(disable_validation__function_has_no_body)
fn __WaveGetLaneIndex() -> u32

@internal(intrinsic_wave_get_lane_count) @internal(disable_validation__function_has_no_body)
fn __WaveGetLaneCount() -> u32

fn frag_main_inner(id : u32, size : u32) {
  let x = (size - id);
}

@compute @workgroup_size(64)
fn frag_main() {
  frag_main_inner(__WaveGetLaneIndex(), __WaveGetLaneCount());
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, SubgroupBuiltinsStruct_Hlsl) {
    auto* src = R"(
enable subgroups;

struct Inputs {
  @builtin(subgroup_invocation_id) id : u32,
  @builtin(subgroup_size) size : u32,
}

@compute @workgroup_size(64)
fn frag_main(inputs : Inputs) {
  let x = inputs.size - inputs.id;
}
)";

    auto* expect = R"(
enable subgroups;

@internal(intrinsic_wave_get_lane_index) @internal(disable_validation__function_has_no_body)
fn __WaveGetLaneIndex() -> u32

@internal(intrinsic_wave_get_lane_count) @internal(disable_validation__function_has_no_body)
fn __WaveGetLaneCount() -> u32

struct Inputs {
  id : u32,
  size : u32,
}

fn frag_main_inner(inputs : Inputs) {
  let x = (inputs.size - inputs.id);
}

@compute @workgroup_size(64)
fn frag_main() {
  frag_main_inner(Inputs(__WaveGetLaneIndex(), __WaveGetLaneCount()));
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
