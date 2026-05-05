// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/reader/program_to_ir/ir_program_test.h"

namespace tint::wgsl::reader {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ProgramToIRSwizzleAssignmentTest = helpers::IRProgramTest;

TEST_F(ProgramToIRSwizzleAssignmentTest, Assignment_SingleElementSwizzle) {
    // var v : vec4<f32>;
    // v.y = 1.0;
    auto* v = Var("v", ty.vec4<f32>(), core::AddressSpace::kFunction);
    auto* assign = Assign(MemberAccessor(v, "y"), 1_f);
    WrapInFunction(v, assign);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()), R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    store_vector_element %v, 1u, 1.0f
    ret
  }
}
)");
}

TEST_F(ProgramToIRSwizzleAssignmentTest, Assignment_MultiElementSwizzle) {
    // var v : vec4<f32>;
    // v.ywx = vec3<f32>(1.0, 2.0, 3.0);
    auto* v = Var("v", ty.vec4<f32>(), core::AddressSpace::kFunction);
    auto* assign = Assign(MemberAccessor(v, "ywx"), Call<vec3<f32>>(1_f, 2_f, 3_f));
    WrapInFunction(v, assign);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()), R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %3:vec4<f32> = load %v
    %4:f32 = access vec3<f32>(1.0f, 2.0f, 3.0f), 0u
    %5:f32 = access vec3<f32>(1.0f, 2.0f, 3.0f), 1u
    %6:f32 = access vec3<f32>(1.0f, 2.0f, 3.0f), 2u
    %7:f32 = access %3, 2u
    %8:vec4<f32> = construct %6, %4, %7, %5
    store %v, %8
    ret
  }
}
)");
}

TEST_F(ProgramToIRSwizzleAssignmentTest, Assignment_ChainedSwizzle) {
    // var v : vec4<f32>;
    // v.zyx.x = 1.0;
    auto* v = Var("v", ty.vec4<f32>(), core::AddressSpace::kFunction);
    auto* assign = Assign(MemberAccessor(MemberAccessor(v, "zyx"), "x"), 1_f);
    WrapInFunction(v, assign);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()), R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    store_vector_element %v, 2u, 1.0f
    ret
  }
}
)");
}

TEST_F(ProgramToIRSwizzleAssignmentTest, Assignment_ChainedMultiElementSwizzle) {
    // var v : vec4<f32>;
    // v.zyx.xz = vec2<f32>(1.0, 2.0);
    auto* v = Var("v", ty.vec4<f32>(), core::AddressSpace::kFunction);
    auto* assign =
        Assign(MemberAccessor(MemberAccessor(v, "zyx"), "xz"), Call<vec2<f32>>(1_f, 2_f));
    WrapInFunction(v, assign);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()), R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %3:vec4<f32> = load %v
    %4:f32 = access vec2<f32>(1.0f, 2.0f), 0u
    %5:f32 = access vec2<f32>(1.0f, 2.0f), 1u
    %6:f32 = access %3, 1u
    %7:f32 = access %3, 3u
    %8:vec4<f32> = construct %5, %6, %4, %7
    store %v, %8
    ret
  }
}
)");
}

TEST_F(ProgramToIRSwizzleAssignmentTest, CompoundAssignment_SingleElementSwizzle) {
    // var v : vec4<f32>;
    // v.y += 1.0;
    auto* v = Var("v", ty.vec4<f32>(), core::AddressSpace::kFunction);
    auto* assign = CompoundAssign(MemberAccessor(v, "y"), 1_f, core::BinaryOp::kAdd);
    WrapInFunction(v, assign);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()), R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %3:f32 = load_vector_element %v, 1u
    %4:f32 = add %3, 1.0f
    store_vector_element %v, 1u, %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRSwizzleAssignmentTest, CompoundAssignment_MultiElementSwizzle) {
    // var v : vec4<f32>;
    // v.ywx *= vec3<f32>(1.0, 2.0, 3.0);
    auto* v = Var("v", ty.vec4<f32>(), core::AddressSpace::kFunction);
    auto* assign = CompoundAssign(MemberAccessor(v, "ywx"), Call<vec3<f32>>(1_f, 2_f, 3_f),
                                  core::BinaryOp::kMultiply);
    WrapInFunction(v, assign);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()), R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %3:vec4<f32> = load %v
    %4:vec3<f32> = swizzle %3, ywx
    %5:vec3<f32> = mul %4, vec3<f32>(1.0f, 2.0f, 3.0f)
    %6:vec4<f32> = load %v
    %7:f32 = access %5, 0u
    %8:f32 = access %5, 1u
    %9:f32 = access %5, 2u
    %10:f32 = access %6, 2u
    %11:vec4<f32> = construct %9, %7, %10, %8
    store %v, %11
    ret
  }
}
)");
}

TEST_F(ProgramToIRSwizzleAssignmentTest, CompoundAssignment_ChainedSwizzle) {
    // var v : vec4<f32>;
    // v.zyx.xz += vec2<f32>(1.0, 2.0);
    auto* v = Var("v", ty.vec4<f32>(), core::AddressSpace::kFunction);
    auto* assign = CompoundAssign(MemberAccessor(MemberAccessor(v, "zyx"), "xz"),
                                  Call<vec2<f32>>(1_f, 2_f), core::BinaryOp::kAdd);
    WrapInFunction(v, assign);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()), R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %3:vec4<f32> = load %v
    %4:vec2<f32> = swizzle %3, zx
    %5:vec2<f32> = add %4, vec2<f32>(1.0f, 2.0f)
    %6:vec4<f32> = load %v
    %7:f32 = access %5, 0u
    %8:f32 = access %5, 1u
    %9:f32 = access %6, 1u
    %10:f32 = access %6, 3u
    %11:vec4<f32> = construct %8, %9, %7, %10
    store %v, %11
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::wgsl::reader
