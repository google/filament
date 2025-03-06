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

#include "src/tint/lang/wgsl/ast/transform/binding_remapper.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using BindingRemapperTest = TransformTest;

TEST_F(BindingRemapperTest, ShouldRunEmptyRemappings) {
    auto* src = R"()";

    DataMap data;
    data.Add<BindingRemapper::Remappings>(BindingRemapper::BindingPoints{},
                                          BindingRemapper::AccessControls{},
                                          /* allow collisions */ false);

    EXPECT_FALSE(ShouldRun<BindingRemapper>(src, data));
}

TEST_F(BindingRemapperTest, ShouldRunEmptyRemappingsWithCollisions) {
    auto* src = R"()";

    DataMap data;
    data.Add<BindingRemapper::Remappings>(BindingRemapper::BindingPoints{},
                                          BindingRemapper::AccessControls{},
                                          /* allow collisions */ true);

    EXPECT_TRUE(ShouldRun<BindingRemapper>(src, data));
}

TEST_F(BindingRemapperTest, ShouldRunBindingPointRemappings) {
    auto* src = R"()";

    DataMap data;
    data.Add<BindingRemapper::Remappings>(
        BindingRemapper::BindingPoints{
            {{2, 1}, {1, 2}},
        },
        BindingRemapper::AccessControls{});

    EXPECT_TRUE(ShouldRun<BindingRemapper>(src, data));
}

TEST_F(BindingRemapperTest, ShouldRunAccessControlRemappings) {
    auto* src = R"()";

    DataMap data;
    data.Add<BindingRemapper::Remappings>(BindingRemapper::BindingPoints{},
                                          BindingRemapper::AccessControls{
                                              {{2, 1}, core::Access::kWrite},
                                          });

    EXPECT_TRUE(ShouldRun<BindingRemapper>(src, data));
}

TEST_F(BindingRemapperTest, NoRemappingsWithoutCollisions) {
    auto* src = R"(
struct S {
  a : f32,
}

@group(2) @binding(1) var<storage, read> a : S;

@group(3) @binding(2) var<storage, read> b : S;

@compute @workgroup_size(1)
fn f() {
}
)";

    auto* expect = src;

    DataMap data;
    data.Add<BindingRemapper::Remappings>(BindingRemapper::BindingPoints{},
                                          BindingRemapper::AccessControls{},
                                          /* allow_collisions */ false);
    auto got = Run<BindingRemapper>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(BindingRemapperTest, NoRemappingsWithCollisions) {
    auto* src = R"(
struct S {
  a : f32,
}

@group(2) @binding(1) var<storage, read> a : S;

@group(3) @binding(2) var<storage, read> b : S;

@compute @workgroup_size(1)
fn f() {
}
)";

    auto* expect = R"(
struct S {
  a : f32,
}

@internal(disable_validation__binding_point_collision) @group(2) @binding(1) var<storage, read> a : S;

@internal(disable_validation__binding_point_collision) @group(3) @binding(2) var<storage, read> b : S;

@compute @workgroup_size(1)
fn f() {
}
)";

    DataMap data;
    data.Add<BindingRemapper::Remappings>(BindingRemapper::BindingPoints{},
                                          BindingRemapper::AccessControls{},
                                          /* allow_collisions */ true);
    auto got = Run<BindingRemapper>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(BindingRemapperTest, RemapBindingPoints) {
    auto* src = R"(
struct S {
  a : f32,
};

@group(2) @binding(1) var<storage, read> a : S;

@group(3) @binding(2) var<storage, read> b : S;

@compute @workgroup_size(1)
fn f() {
}
)";

    auto* expect = R"(
struct S {
  a : f32,
}

@internal(disable_validation__binding_point_collision) @group(1) @binding(2) var<storage, read> a : S;

@internal(disable_validation__binding_point_collision) @group(3) @binding(2) var<storage, read> b : S;

@compute @workgroup_size(1)
fn f() {
}
)";

    DataMap data;
    data.Add<BindingRemapper::Remappings>(
        BindingRemapper::BindingPoints{
            {{2, 1}, {1, 2}},  // Remap
            {{4, 5}, {6, 7}},  // Not found
                               // Keep @group(3) @binding(2) as is
        },
        BindingRemapper::AccessControls{});
    auto got = Run<BindingRemapper>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(BindingRemapperTest, RemapAccessControls) {
    auto* src = R"(
struct S {
  a : f32,
};

@group(2) @binding(1) var<storage, read_write> a : S;

@group(3) @binding(2) var<storage, read_write> b : S;

@group(4) @binding(3) var<storage, read> c : S;

@compute @workgroup_size(1)
fn f() {
}
)";

    auto* expect = R"(
struct S {
  a : f32,
}

@internal(disable_validation__binding_point_collision) @group(2) @binding(1) var<storage, read_write> a : S;

@internal(disable_validation__binding_point_collision) @group(3) @binding(2) var<storage, read_write> b : S;

@internal(disable_validation__binding_point_collision) @group(4) @binding(3) var<storage, read> c : S;

@compute @workgroup_size(1)
fn f() {
}
)";

    DataMap data;
    data.Add<BindingRemapper::Remappings>(
        BindingRemapper::BindingPoints{},
        BindingRemapper::AccessControls{
            {{2, 1}, core::Access::kReadWrite},  // Modify access control
            // Keep @group(3) @binding(2) as is
            {{4, 3}, core::Access::kRead},  // Add access control
        });
    auto got = Run<BindingRemapper>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(BindingRemapperTest, RemapAll) {
    auto* src = R"(
struct S {
  a : f32,
};

@group(2) @binding(1) var<storage, read> a : S;

@group(3) @binding(2) var<storage, read> b : S;

@compute @workgroup_size(1)
fn f() {
}
)";

    auto* expect = R"(
struct S {
  a : f32,
}

@internal(disable_validation__binding_point_collision) @group(4) @binding(5) var<storage, read_write> a : S;

@internal(disable_validation__binding_point_collision) @group(6) @binding(7) var<storage, read_write> b : S;

@compute @workgroup_size(1)
fn f() {
}
)";

    DataMap data;
    data.Add<BindingRemapper::Remappings>(
        BindingRemapper::BindingPoints{
            {{2, 1}, {4, 5}},
            {{3, 2}, {6, 7}},
        },
        BindingRemapper::AccessControls{
            {{2, 1}, core::Access::kReadWrite},
            {{3, 2}, core::Access::kReadWrite},
        });
    auto got = Run<BindingRemapper>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(BindingRemapperTest, BindingCollisionsSameEntryPoint) {
    auto* src = R"(
struct S {
  i : i32,
};

@group(2) @binding(1) var<storage, read> a : S;

@group(3) @binding(2) var<storage, read> b : S;

@group(4) @binding(3) var<storage, read> c : S;

@group(5) @binding(4) var<storage, read> d : S;

@compute @workgroup_size(1)
fn f() {
  let x : i32 = (((a.i + b.i) + c.i) + d.i);
}
)";

    auto* expect = R"(
struct S {
  i : i32,
}

@internal(disable_validation__binding_point_collision) @group(1) @binding(1) var<storage, read> a : S;

@internal(disable_validation__binding_point_collision) @group(1) @binding(1) var<storage, read> b : S;

@internal(disable_validation__binding_point_collision) @group(5) @binding(4) var<storage, read> c : S;

@internal(disable_validation__binding_point_collision) @group(5) @binding(4) var<storage, read> d : S;

@compute @workgroup_size(1)
fn f() {
  let x : i32 = (((a.i + b.i) + c.i) + d.i);
}
)";

    DataMap data;
    data.Add<BindingRemapper::Remappings>(
        BindingRemapper::BindingPoints{
            {{2, 1}, {1, 1}},
            {{3, 2}, {1, 1}},
            {{4, 3}, {5, 4}},
        },
        BindingRemapper::AccessControls{}, true);
    auto got = Run<BindingRemapper>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(BindingRemapperTest, BindingCollisionsDifferentEntryPoints) {
    auto* src = R"(
struct S {
  i : i32,
};

@group(2) @binding(1) var<storage, read> a : S;

@group(3) @binding(2) var<storage, read> b : S;

@group(4) @binding(3) var<storage, read> c : S;

@group(5) @binding(4) var<storage, read> d : S;

@compute @workgroup_size(1)
fn f1() {
  let x : i32 = (a.i + c.i);
}

@compute @workgroup_size(1)
fn f2() {
  let x : i32 = (b.i + d.i);
}
)";

    auto* expect = R"(
struct S {
  i : i32,
}

@internal(disable_validation__binding_point_collision) @group(1) @binding(1) var<storage, read> a : S;

@internal(disable_validation__binding_point_collision) @group(1) @binding(1) var<storage, read> b : S;

@internal(disable_validation__binding_point_collision) @group(5) @binding(4) var<storage, read> c : S;

@internal(disable_validation__binding_point_collision) @group(5) @binding(4) var<storage, read> d : S;

@compute @workgroup_size(1)
fn f1() {
  let x : i32 = (a.i + c.i);
}

@compute @workgroup_size(1)
fn f2() {
  let x : i32 = (b.i + d.i);
}
)";

    DataMap data;
    data.Add<BindingRemapper::Remappings>(
        BindingRemapper::BindingPoints{
            {{2, 1}, {1, 1}},
            {{3, 2}, {1, 1}},
            {{4, 3}, {5, 4}},
        },
        BindingRemapper::AccessControls{}, true);
    auto got = Run<BindingRemapper>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(BindingRemapperTest, NoData) {
    auto* src = R"(
struct S {
  a : f32,
}

@group(2) @binding(1) var<storage, read> a : S;

@group(3) @binding(2) var<storage, read> b : S;

@compute @workgroup_size(1)
fn f() {
}
)";

    auto* expect = R"(error: missing transform data for tint::ast::transform::BindingRemapper)";

    auto got = Run<BindingRemapper>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
