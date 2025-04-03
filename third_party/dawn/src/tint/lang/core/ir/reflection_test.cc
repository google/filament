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
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/core/ir/reflection.h"

#include <string>

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/utils/result.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_ReflectionTest : public IRTestHelper {
  protected:
    /// @returns the module as a disassembled string
    std::string Disassemble() const { return "\n" + ir::Disassembler(mod).Plain(); }
};

TEST_F(IR_ReflectionTest, GetWorkgroupInfoBasic) {
    auto* var_a = mod.root_block->Append(b.Var<workgroup, u32>("a"));
    auto* foo = b.ComputeFunction("foo", 3_u, 5_u, 7_u);
    b.Append(foo->Block(), [&] {  //
        b.Load(var_a);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<workgroup, u32, read_write> = var undef
}

%foo = @compute @workgroup_size(3u, 5u, 7u) func():void {
  $B2: {
    %3:u32 = load %a
    ret
  }
}
)";
    EXPECT_EQ(src, Disassemble());

    auto res = GetWorkgroupInfo(mod);
    EXPECT_TRUE(res == tint::Success);
    EXPECT_EQ(res->x, 3u);
    EXPECT_EQ(res->y, 5u);
    EXPECT_EQ(res->z, 7u);
    EXPECT_EQ(res->storage_size, 16u);
}

TEST_F(IR_ReflectionTest, GetWorkgroupInfoMultiVar) {
    auto* var_a = mod.root_block->Append(b.Var<workgroup, u32>("a"));
    auto* var_b = mod.root_block->Append(b.Var<workgroup, u32>("b"));
    auto* var_c = mod.root_block->Append(b.Var<workgroup, u32>("c"));
    auto* var_d = mod.root_block->Append(b.Var<workgroup, u32>("d"));
    auto* foo = b.ComputeFunction("foo", 128_u, 1_u, 1_u);
    b.Append(foo->Block(), [&] {  //
        b.Load(var_a);
        b.Load(var_b);
        b.Load(var_c);
        b.Load(var_d);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<workgroup, u32, read_write> = var undef
  %b:ptr<workgroup, u32, read_write> = var undef
  %c:ptr<workgroup, u32, read_write> = var undef
  %d:ptr<workgroup, u32, read_write> = var undef
}

%foo = @compute @workgroup_size(128u, 1u, 1u) func():void {
  $B2: {
    %6:u32 = load %a
    %7:u32 = load %b
    %8:u32 = load %c
    %9:u32 = load %d
    ret
  }
}
)";
    EXPECT_EQ(src, Disassemble());

    auto res = GetWorkgroupInfo(mod);
    EXPECT_TRUE(res == tint::Success);
    EXPECT_EQ(res->x, 128u);
    EXPECT_EQ(res->y, 1u);
    EXPECT_EQ(res->z, 1u);
    EXPECT_EQ(res->storage_size, 64u);
}

TEST_F(IR_ReflectionTest, GetWorkgroupInfoNoVar) {
    auto* foo = b.ComputeFunction("foo", 128_u, 1_u, 1_u);
    b.Append(foo->Block(), [&] {  //
        b.Return(foo);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(128u, 1u, 1u) func():void {
  $B1: {
    ret
  }
}
)";
    EXPECT_EQ(src, Disassemble());

    auto res = GetWorkgroupInfo(mod);
    EXPECT_TRUE(res == tint::Success);
    EXPECT_EQ(res->x, 128u);
    EXPECT_EQ(res->y, 1u);
    EXPECT_EQ(res->z, 1u);
    EXPECT_EQ(res->storage_size, 0u);
}

TEST_F(IR_ReflectionTest, GetWorkgroupInfoFailNoWorkgroupSize) {
    // Referenced.
    auto* var_a = mod.root_block->Append(b.Var<workgroup, u32>("a"));
    auto* foo = b.Function("foo", ty.void_(), Function::PipelineStage::kCompute);
    b.Append(foo->Block(), [&] {  //
        b.Load(var_a);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<workgroup, u32, read_write> = var undef
}

%foo = @compute func():void {
  $B2: {
    %3:u32 = load %a
    ret
  }
}
)";
    EXPECT_EQ(src, Disassemble());

    auto res = GetWorkgroupInfo(mod);
    EXPECT_FALSE(res == tint::Success);

    auto* failure_msg = R"(IR GetWorkgroupInfo: Could not find workgroup size)";
    EXPECT_EQ(failure_msg, res.Failure().reason);
}

}  // namespace
}  // namespace tint::core::ir
