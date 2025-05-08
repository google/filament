// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/transform/substitute_override.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

TEST(TintCheckAllFieldsReflected, WgslAstTransformSubstituteOverrideTest) {
    TINT_ASSERT_ALL_FIELDS_REFLECTED(SubstituteOverride::Config);
}

using SubstituteOverrideTest = TransformTest;

TEST_F(SubstituteOverrideTest, Error_NoData) {
    auto* src = R"(
override width: i32;
@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = "error: Missing override substitution data";

    DataMap data;
    auto got = Run<SubstituteOverride>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SubstituteOverrideTest, Error_NoOverrideValue) {
    auto* src = R"(
override width: i32;
@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = "error: Initializer not provided for override, and override not overridden.";

    SubstituteOverride::Config cfg;
    DataMap data;
    data.Add<SubstituteOverride::Config>(cfg);

    auto got = Run<SubstituteOverride>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SubstituteOverrideTest, Module_NoOverrides) {
    auto* src = R"(
@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = R"(
@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    SubstituteOverride::Config cfg;

    DataMap data;
    data.Add<SubstituteOverride::Config>(cfg);
    auto got = Run<SubstituteOverride>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SubstituteOverrideTest, OverrideNotInFile) {
    auto* src = R"(
@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = R"(
@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    SubstituteOverride::Config cfg;
    cfg.map.insert({OverrideId{99}, 42.0});

    DataMap data;
    data.Add<SubstituteOverride::Config>(cfg);
    auto got = Run<SubstituteOverride>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SubstituteOverrideTest, ImplicitId) {
    auto* src = R"(
enable f16;

override i_width: i32;
override i_height = 1i;

override f_width: f32;
override f_height = 1.f;

override h_width: f16;
override h_height = 1.h;

override b_width: bool;
override b_height = true;

override o_width = 2i;

@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = R"(
enable f16;

const i_width : i32 = 42i;

const i_height = 11i;

const f_width : f32 = 22.299999237060546875f;

const f_height = 12.3999996185302734375f;

const h_width : f16 = 9.3984375h;

const h_height = 3.3984375h;

const b_width : bool = true;

const b_height = false;

const o_width = 2i;

@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    SubstituteOverride::Config cfg;
    cfg.map.insert({OverrideId{0}, 42.0});
    cfg.map.insert({OverrideId{1}, 11.0});
    cfg.map.insert({OverrideId{2}, 22.3});
    cfg.map.insert({OverrideId{3}, 12.4});
    cfg.map.insert({OverrideId{4}, 9.4});
    cfg.map.insert({OverrideId{5}, 3.4});
    cfg.map.insert({OverrideId{6}, 1.0});
    cfg.map.insert({OverrideId{7}, 0.0});

    DataMap data;
    data.Add<SubstituteOverride::Config>(cfg);
    auto got = Run<SubstituteOverride>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SubstituteOverrideTest, ExplicitId) {
    auto* src = R"(
enable f16;

@id(0) override i_width: i32;
@id(10) override i_height = 1i;

@id(1) override f_width: f32;
@id(9) override f_height = 1.f;

@id(2) override h_width: f16;
@id(8) override h_height = 1.h;

@id(3) override b_width: bool;
@id(7) override b_height = true;

@id(5) override o_width = 2i;

@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = R"(
enable f16;

const i_width : i32 = 42i;

const i_height = 11i;

const f_width : f32 = 22.299999237060546875f;

const f_height = 12.3999996185302734375f;

const h_width : f16 = 9.3984375h;

const h_height = 3.3984375h;

const b_width : bool = true;

const b_height = false;

const o_width = 13i;

@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    SubstituteOverride::Config cfg;
    cfg.map.insert({OverrideId{0}, 42.0});
    cfg.map.insert({OverrideId{10}, 11.0});
    cfg.map.insert({OverrideId{1}, 22.3});
    cfg.map.insert({OverrideId{9}, 12.4});
    cfg.map.insert({OverrideId{2}, 9.4});
    cfg.map.insert({OverrideId{8}, 3.4});
    cfg.map.insert({OverrideId{3}, 1.0});
    cfg.map.insert({OverrideId{7}, 0.0});
    cfg.map.insert({OverrideId{5}, 13});

    DataMap data;
    data.Add<SubstituteOverride::Config>(cfg);
    auto got = Run<SubstituteOverride>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SubstituteOverrideTest, Identifier_Expression) {
    auto* src = R"(
override i_height = ~2i;

@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = R"(
const i_height = 11i;

@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    SubstituteOverride::Config cfg;
    cfg.map.insert({OverrideId{0}, 11.0});

    DataMap data;
    data.Add<SubstituteOverride::Config>(cfg);
    auto got = Run<SubstituteOverride>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SubstituteOverrideTest, IndexMaterialization) {
    auto* src = R"(
override O = 0; // Try switching to 'const'

fn f() {
  const smaller_than_any_f32 = 1e-50;
  const large_float = 1e27;
  // When O is an override, the outer index value is not constant, so the
  // value is not calculated at shader-creation time, and does not error.
  //
  // When O is a const, and 'smaller_than_any_f32' *is not* materialized, the
  // outer index value will evaluate to 10000, resulting in an out-of-bounds
  // error.
  //
  // When O is a const, and 'smaller_than_any_f32' *is* materialized, the
  // materialization of 'smaller_than_any_f32' to f32 will evaluate to zero,
  // and so the outer index value will be zero, and we get no error.
  _ = vec2(0)[i32(vec2(smaller_than_any_f32)[O]*large_float*large_float)];
}
)";

    auto* expect = R"(
const O = 0i;

fn f() {
  const smaller_than_any_f32 = 1e-50;
  const large_float = 1000000000000000013287555072.0;
  _ = __tint_materialize(vec2(0))[i32(((__tint_materialize(vec2(smaller_than_any_f32))[O] * large_float) * large_float))];
}
)";

    SubstituteOverride::Config cfg;
    cfg.map.insert({OverrideId{0}, 0.0});

    DataMap data;
    data.Add<SubstituteOverride::Config>(cfg);
    auto got = Run<SubstituteOverride>(src, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
