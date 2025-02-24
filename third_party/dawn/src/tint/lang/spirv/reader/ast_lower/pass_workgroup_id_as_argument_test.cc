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

#include "src/tint/lang/spirv/reader/ast_lower/pass_workgroup_id_as_argument.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::spirv::reader {
namespace {

using PassWorkgroupIdAsArgumentTest = ast::transform::TransformTest;

TEST_F(PassWorkgroupIdAsArgumentTest, Basic) {
    auto* src = R"(
enable chromium_disable_uniformity_analysis;

var<private> wgid : vec3u;

fn inner() {
  if (wgid.x == 0) {
    workgroupBarrier();
  }
}

@compute @workgroup_size(64)
fn main(@builtin(workgroup_id) wgid_param : vec3u) {
  wgid = wgid_param;
  inner();
}
)";

    auto* expect = R"(
enable chromium_disable_uniformity_analysis;

fn inner(tint_wgid : vec3u) {
  if ((tint_wgid.x == 0)) {
    workgroupBarrier();
  }
}

@compute @workgroup_size(64)
fn main(@builtin(workgroup_id) wgid_param : vec3u) {
  inner(wgid_param);
}
)";

    auto got = Run<PassWorkgroupIdAsArgument>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PassWorkgroupIdAsArgumentTest, MultipleUses) {
    auto* src = R"(
enable chromium_disable_uniformity_analysis;

var<private> wgid : vec3u;

fn inner() {
  if (wgid.x == 0) {
    workgroupBarrier();
  }
  if (wgid.y == 0) {
    workgroupBarrier();
  }
  if (wgid.z == 0) {
    workgroupBarrier();
  }
}

@compute @workgroup_size(64)
fn main(@builtin(workgroup_id) wgid_param : vec3u) {
  wgid = wgid_param;
  inner();
}
)";

    auto* expect = R"(
enable chromium_disable_uniformity_analysis;

fn inner(tint_wgid : vec3u) {
  if ((tint_wgid.x == 0)) {
    workgroupBarrier();
  }
  if ((tint_wgid.y == 0)) {
    workgroupBarrier();
  }
  if ((tint_wgid.z == 0)) {
    workgroupBarrier();
  }
}

@compute @workgroup_size(64)
fn main(@builtin(workgroup_id) wgid_param : vec3u) {
  inner(wgid_param);
}
)";

    auto got = Run<PassWorkgroupIdAsArgument>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PassWorkgroupIdAsArgumentTest, NestedCall) {
    auto* src = R"(
enable chromium_disable_uniformity_analysis;

var<private> wgid : vec3u;

fn inner_2() {
  if (wgid.x == 0) {
    workgroupBarrier();
  }
}

fn inner_1() {
  inner_2();
}

fn inner() {
  inner_1();
}

@compute @workgroup_size(64)
fn main(@builtin(workgroup_id) wgid_param : vec3u) {
  wgid = wgid_param;
  inner();
}
)";

    auto* expect = R"(
enable chromium_disable_uniformity_analysis;

fn inner_2(tint_wgid : vec3u) {
  if ((tint_wgid.x == 0)) {
    workgroupBarrier();
  }
}

fn inner_1(tint_wgid_1 : vec3u) {
  inner_2(tint_wgid_1);
}

fn inner(tint_wgid_2 : vec3u) {
  inner_1(tint_wgid_2);
}

@compute @workgroup_size(64)
fn main(@builtin(workgroup_id) wgid_param : vec3u) {
  inner(wgid_param);
}
)";

    auto got = Run<PassWorkgroupIdAsArgument>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PassWorkgroupIdAsArgumentTest, NestedCall_UsesAtEachLevel) {
    auto* src = R"(
enable chromium_disable_uniformity_analysis;

var<private> wgid : vec3u;

fn inner_2() {
  if (wgid.x == 0) {
    workgroupBarrier();
  }
}

fn inner_1() {
  inner_2();
  if (wgid.y == 0) {
    workgroupBarrier();
  }
}

fn inner() {
  inner_1();
  if (wgid.z == 0) {
    workgroupBarrier();
  }
}

@compute @workgroup_size(64)
fn main(@builtin(workgroup_id) wgid_param : vec3u) {
  wgid = wgid_param;
  inner();
}
)";

    auto* expect = R"(
enable chromium_disable_uniformity_analysis;

fn inner_2(tint_wgid : vec3u) {
  if ((tint_wgid.x == 0)) {
    workgroupBarrier();
  }
}

fn inner_1(tint_wgid_1 : vec3u) {
  inner_2(tint_wgid_1);
  if ((tint_wgid_1.y == 0)) {
    workgroupBarrier();
  }
}

fn inner(tint_wgid_2 : vec3u) {
  inner_1(tint_wgid_2);
  if ((tint_wgid_2.z == 0)) {
    workgroupBarrier();
  }
}

@compute @workgroup_size(64)
fn main(@builtin(workgroup_id) wgid_param : vec3u) {
  inner(wgid_param);
}
)";

    auto got = Run<PassWorkgroupIdAsArgument>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PassWorkgroupIdAsArgumentTest, NestedCall_MultipleCallsites) {
    auto* src = R"(
enable chromium_disable_uniformity_analysis;

var<private> wgid : vec3u;

fn inner_2() {
  if (wgid.x == 0) {
    workgroupBarrier();
  }
}

fn inner_1() {
  inner_2();
  inner_2();
  inner_2();
}

fn inner() {
  inner_1();
  inner_2();
  inner_1();
}

@compute @workgroup_size(64)
fn main(@builtin(workgroup_id) wgid_param : vec3u) {
  wgid = wgid_param;
  inner();
}
)";

    auto* expect = R"(
enable chromium_disable_uniformity_analysis;

fn inner_2(tint_wgid : vec3u) {
  if ((tint_wgid.x == 0)) {
    workgroupBarrier();
  }
}

fn inner_1(tint_wgid_1 : vec3u) {
  inner_2(tint_wgid_1);
  inner_2(tint_wgid_1);
  inner_2(tint_wgid_1);
}

fn inner(tint_wgid_2 : vec3u) {
  inner_1(tint_wgid_2);
  inner_2(tint_wgid_2);
  inner_1(tint_wgid_2);
}

@compute @workgroup_size(64)
fn main(@builtin(workgroup_id) wgid_param : vec3u) {
  inner(wgid_param);
}
)";

    auto got = Run<PassWorkgroupIdAsArgument>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PassWorkgroupIdAsArgumentTest, NestedCall_OtherParameters) {
    auto* src = R"(
enable chromium_disable_uniformity_analysis;

var<private> wgid : vec3u;

fn inner_2(a : u32, b : u32) {
  if (wgid.x + a == b) {
    workgroupBarrier();
  }
}

fn inner_1(a : u32) {
  inner_2(a, 1);
}

fn inner() {
  inner_1(2);
}

@compute @workgroup_size(64)
fn main(@builtin(workgroup_id) wgid_param : vec3u) {
  wgid = wgid_param;
  inner();
}
)";

    auto* expect = R"(
enable chromium_disable_uniformity_analysis;

fn inner_2(a : u32, b : u32, tint_wgid : vec3u) {
  if (((tint_wgid.x + a) == b)) {
    workgroupBarrier();
  }
}

fn inner_1(a : u32, tint_wgid_1 : vec3u) {
  inner_2(a, 1, tint_wgid_1);
}

fn inner(tint_wgid_2 : vec3u) {
  inner_1(2, tint_wgid_2);
}

@compute @workgroup_size(64)
fn main(@builtin(workgroup_id) wgid_param : vec3u) {
  inner(wgid_param);
}
)";

    auto got = Run<PassWorkgroupIdAsArgument>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PassWorkgroupIdAsArgumentTest, BitcastToI32) {
    auto* src = R"(
enable chromium_disable_uniformity_analysis;

var<private> wgid : vec3i;

fn inner() {
  if (wgid.x == 0i) {
    workgroupBarrier();
  }
}

@compute @workgroup_size(64)
fn main(@builtin(workgroup_id) wgid_param : vec3u) {
  wgid = bitcast<vec3i>(wgid_param);
  inner();
}
)";

    auto* expect = R"(
enable chromium_disable_uniformity_analysis;

fn inner(tint_wgid : vec3i) {
  if ((tint_wgid.x == 0i)) {
    workgroupBarrier();
  }
}

@compute @workgroup_size(64)
fn main(@builtin(workgroup_id) wgid_param : vec3u) {
  let tint_wgid_bitcast = bitcast<vec3i>(wgid_param);
  inner(tint_wgid_bitcast);
}
)";

    auto got = Run<PassWorkgroupIdAsArgument>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::spirv::reader
