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

#include "src/tint/lang/wgsl/ast/transform/add_empty_entry_point.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using AddEmptyEntryPointTest = TransformTest;

TEST_F(AddEmptyEntryPointTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_TRUE(ShouldRun<AddEmptyEntryPoint>(src));
}

TEST_F(AddEmptyEntryPointTest, ShouldRunExistingEntryPoint) {
    auto* src = R"(
@compute @workgroup_size(1)
fn existing() {}
)";

    EXPECT_FALSE(ShouldRun<AddEmptyEntryPoint>(src));
}

TEST_F(AddEmptyEntryPointTest, EmptyModule) {
    auto* src = R"()";

    auto* expect = R"(
@compute @workgroup_size(1i)
fn unused_entry_point() {
}
)";

    auto got = Run<AddEmptyEntryPoint>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddEmptyEntryPointTest, ExistingEntryPoint) {
    auto* src = R"(
@fragment
fn main() {
}
)";

    auto* expect = src;

    auto got = Run<AddEmptyEntryPoint>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddEmptyEntryPointTest, NameClash) {
    auto* src = R"(var<private> unused_entry_point : f32;)";

    auto* expect = R"(
@compute @workgroup_size(1i)
fn unused_entry_point_1() {
}

var<private> unused_entry_point : f32;
)";

    auto got = Run<AddEmptyEntryPoint>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
