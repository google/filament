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

#include "src/tint/lang/wgsl/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/struct.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::resolver {
namespace {

using ResolverStructLayoutTest = ResolverTest;

TEST_F(ResolverStructLayoutTest, Scalars) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.f32()),
                                 Member("b", ty.u32()),
                                 Member("c", ty.i32()),
                             });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Size(), 12u);
    EXPECT_EQ(sem->SizeNoPadding(), 12u);
    EXPECT_EQ(sem->Align(), 4u);
    ASSERT_EQ(sem->Members().Length(), 3u);
    EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
    EXPECT_EQ(sem->Members()[0]->Align(), 4u);
    EXPECT_EQ(sem->Members()[0]->Size(), 4u);
    EXPECT_EQ(sem->Members()[1]->Offset(), 4u);
    EXPECT_EQ(sem->Members()[1]->Align(), 4u);
    EXPECT_EQ(sem->Members()[1]->Size(), 4u);
    EXPECT_EQ(sem->Members()[2]->Offset(), 8u);
    EXPECT_EQ(sem->Members()[2]->Align(), 4u);
    EXPECT_EQ(sem->Members()[2]->Size(), 4u);
    for (auto& m : sem->Members()) {
        EXPECT_EQ(m->Struct()->Declaration(), s);
    }
}

TEST_F(ResolverStructLayoutTest, ScalarsWithF16) {
    Enable(wgsl::Extension::kF16);

    auto* s = Structure("S", Vector{
                                 Member("a", ty.f32()),
                                 Member("b", ty.f16()),
                                 Member("c", ty.u32()),
                                 Member("d", ty.f16()),
                                 Member("e", ty.f16()),
                                 Member("f", ty.i32()),
                                 Member("g", ty.f16()),
                             });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Size(), 24u);
    EXPECT_EQ(sem->SizeNoPadding(), 22u);
    EXPECT_EQ(sem->Align(), 4u);
    ASSERT_EQ(sem->Members().Length(), 7u);
    // f32
    EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
    EXPECT_EQ(sem->Members()[0]->Align(), 4u);
    EXPECT_EQ(sem->Members()[0]->Size(), 4u);
    // f16
    EXPECT_EQ(sem->Members()[1]->Offset(), 4u);
    EXPECT_EQ(sem->Members()[1]->Align(), 2u);
    EXPECT_EQ(sem->Members()[1]->Size(), 2u);
    // u32
    EXPECT_EQ(sem->Members()[2]->Offset(), 8u);
    EXPECT_EQ(sem->Members()[2]->Align(), 4u);
    EXPECT_EQ(sem->Members()[2]->Size(), 4u);
    // f16
    EXPECT_EQ(sem->Members()[3]->Offset(), 12u);
    EXPECT_EQ(sem->Members()[3]->Align(), 2u);
    EXPECT_EQ(sem->Members()[3]->Size(), 2u);
    // f16
    EXPECT_EQ(sem->Members()[4]->Offset(), 14u);
    EXPECT_EQ(sem->Members()[4]->Align(), 2u);
    EXPECT_EQ(sem->Members()[4]->Size(), 2u);
    // i32
    EXPECT_EQ(sem->Members()[5]->Offset(), 16u);
    EXPECT_EQ(sem->Members()[5]->Align(), 4u);
    EXPECT_EQ(sem->Members()[5]->Size(), 4u);
    // f16
    EXPECT_EQ(sem->Members()[6]->Offset(), 20u);
    EXPECT_EQ(sem->Members()[6]->Align(), 2u);
    EXPECT_EQ(sem->Members()[6]->Size(), 2u);
}

TEST_F(ResolverStructLayoutTest, Alias) {
    auto* alias_a = Alias("a", ty.f32());
    auto* alias_b = Alias("b", ty.f32());

    auto* s = Structure("S", Vector{
                                 Member("a", ty.Of(alias_a)),
                                 Member("b", ty.Of(alias_b)),
                             });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Size(), 8u);
    EXPECT_EQ(sem->SizeNoPadding(), 8u);
    EXPECT_EQ(sem->Align(), 4u);
    ASSERT_EQ(sem->Members().Length(), 2u);
    EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
    EXPECT_EQ(sem->Members()[0]->Align(), 4u);
    EXPECT_EQ(sem->Members()[0]->Size(), 4u);
    EXPECT_EQ(sem->Members()[1]->Offset(), 4u);
    EXPECT_EQ(sem->Members()[1]->Align(), 4u);
    EXPECT_EQ(sem->Members()[1]->Size(), 4u);
    for (auto& m : sem->Members()) {
        EXPECT_EQ(m->Struct()->Declaration(), s);
    }
}

TEST_F(ResolverStructLayoutTest, ImplicitStrideArrayStaticSize) {
    Enable(wgsl::Extension::kF16);

    auto* s = Structure("S", Vector{
                                 Member("a", ty.array<i32, 3>()),
                                 Member("b", ty.array<f32, 5>()),
                                 Member("c", ty.array<f16, 7>()),
                                 Member("d", ty.array<f32, 1>()),
                             });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Size(), 52u);
    EXPECT_EQ(sem->SizeNoPadding(), 52u);
    EXPECT_EQ(sem->Align(), 4u);
    ASSERT_EQ(sem->Members().Length(), 4u);
    // array<i32, 3>
    EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
    EXPECT_EQ(sem->Members()[0]->Align(), 4u);
    EXPECT_EQ(sem->Members()[0]->Size(), 12u);
    // array<f32, 5>
    EXPECT_EQ(sem->Members()[1]->Offset(), 12u);
    EXPECT_EQ(sem->Members()[1]->Align(), 4u);
    EXPECT_EQ(sem->Members()[1]->Size(), 20u);
    // array<f16, 7>
    EXPECT_EQ(sem->Members()[2]->Offset(), 32u);
    EXPECT_EQ(sem->Members()[2]->Align(), 2u);
    EXPECT_EQ(sem->Members()[2]->Size(), 14u);
    // array<f32, 1>
    EXPECT_EQ(sem->Members()[3]->Offset(), 48u);
    EXPECT_EQ(sem->Members()[3]->Align(), 4u);
    EXPECT_EQ(sem->Members()[3]->Size(), 4u);

    for (auto& m : sem->Members()) {
        EXPECT_EQ(m->Struct()->Declaration(), s);
    }
}

TEST_F(ResolverStructLayoutTest, ExplicitStrideArrayStaticSize) {
    Enable(wgsl::Extension::kF16);

    auto* s = Structure("S", Vector{
                                 Member("a", ty.array<i32, 3>(Vector{Stride(8)})),
                                 Member("b", ty.array<f32, 5>(Vector{Stride(16)})),
                                 Member("c", ty.array<f16, 7>(Vector{Stride(4)})),
                                 Member("d", ty.array<f32, 1>(Vector{Stride(32)})),
                             });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Size(), 164u);
    EXPECT_EQ(sem->SizeNoPadding(), 164u);
    EXPECT_EQ(sem->Align(), 4u);
    ASSERT_EQ(sem->Members().Length(), 4u);
    // array<i32, 3>, stride = 8
    EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
    EXPECT_EQ(sem->Members()[0]->Align(), 4u);
    EXPECT_EQ(sem->Members()[0]->Size(), 24u);
    // array<f32, 5>, stride = 16
    EXPECT_EQ(sem->Members()[1]->Offset(), 24u);
    EXPECT_EQ(sem->Members()[1]->Align(), 4u);
    EXPECT_EQ(sem->Members()[1]->Size(), 80u);
    // array<f16, 7>, stride = 4
    EXPECT_EQ(sem->Members()[2]->Offset(), 104u);
    EXPECT_EQ(sem->Members()[2]->Align(), 2u);
    EXPECT_EQ(sem->Members()[2]->Size(), 28u);
    // array<f32, 1>, stride = 32
    EXPECT_EQ(sem->Members()[3]->Offset(), 132u);
    EXPECT_EQ(sem->Members()[3]->Align(), 4u);
    EXPECT_EQ(sem->Members()[3]->Size(), 32u);

    for (auto& m : sem->Members()) {
        EXPECT_EQ(m->Struct()->Declaration(), s);
    }
}

TEST_F(ResolverStructLayoutTest, ImplicitStrideArrayRuntimeSized) {
    auto* s = Structure("S", Vector{
                                 Member("c", ty.array<f32>()),
                             });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Size(), 4u);
    EXPECT_EQ(sem->SizeNoPadding(), 4u);
    EXPECT_EQ(sem->Align(), 4u);
    ASSERT_EQ(sem->Members().Length(), 1u);
    EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
    EXPECT_EQ(sem->Members()[0]->Align(), 4u);
    EXPECT_EQ(sem->Members()[0]->Size(), 4u);
    for (auto& m : sem->Members()) {
        EXPECT_EQ(m->Struct()->Declaration(), s);
    }
}

TEST_F(ResolverStructLayoutTest, ExplicitStrideArrayRuntimeSized) {
    auto* s = Structure("S", Vector{
                                 Member("c", ty.array<f32>(Vector{Stride(32)})),
                             });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Size(), 32u);
    EXPECT_EQ(sem->SizeNoPadding(), 32u);
    EXPECT_EQ(sem->Align(), 4u);
    ASSERT_EQ(sem->Members().Length(), 1u);
    EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
    EXPECT_EQ(sem->Members()[0]->Align(), 4u);
    EXPECT_EQ(sem->Members()[0]->Size(), 32u);
    for (auto& m : sem->Members()) {
        EXPECT_EQ(m->Struct()->Declaration(), s);
    }
}

TEST_F(ResolverStructLayoutTest, ImplicitStrideArrayOfExplicitStrideArray) {
    auto inner = ty.array<i32, 2>(Vector{Stride(16)});  // size: 32
    auto outer = ty.array(inner, 12_u);                 // size: 12 * 32
    auto* s = Structure("S", Vector{
                                 Member("c", outer),
                             });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Size(), 384u);
    EXPECT_EQ(sem->SizeNoPadding(), 384u);
    EXPECT_EQ(sem->Align(), 4u);
    ASSERT_EQ(sem->Members().Length(), 1u);
    EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
    EXPECT_EQ(sem->Members()[0]->Align(), 4u);
    EXPECT_EQ(sem->Members()[0]->Size(), 384u);
    for (auto& m : sem->Members()) {
        EXPECT_EQ(m->Struct()->Declaration(), s);
    }
}

TEST_F(ResolverStructLayoutTest, ImplicitStrideArrayOfStructure) {
    auto* inner = Structure("Inner", Vector{
                                         Member("a", ty.vec2<i32>()),
                                         Member("b", ty.vec3<i32>()),
                                         Member("c", ty.vec4<i32>()),
                                     });        // size: 48
    auto outer = ty.array(ty.Of(inner), 12_u);  // size: 12 * 48
    auto* s = Structure("S", Vector{
                                 Member("c", outer),
                             });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Size(), 576u);
    EXPECT_EQ(sem->SizeNoPadding(), 576u);
    EXPECT_EQ(sem->Align(), 16u);
    ASSERT_EQ(sem->Members().Length(), 1u);
    EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
    EXPECT_EQ(sem->Members()[0]->Align(), 16u);
    EXPECT_EQ(sem->Members()[0]->Size(), 576u);
    for (auto& m : sem->Members()) {
        EXPECT_EQ(m->Struct()->Declaration(), s);
    }
}

TEST_F(ResolverStructLayoutTest, Vector) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.vec2<i32>()),
                                 Member("b", ty.vec3<i32>()),
                                 Member("c", ty.vec4<i32>()),
                             });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Size(), 48u);
    EXPECT_EQ(sem->SizeNoPadding(), 48u);
    EXPECT_EQ(sem->Align(), 16u);
    ASSERT_EQ(sem->Members().Length(), 3u);
    EXPECT_EQ(sem->Members()[0]->Offset(), 0u);  // vec2
    EXPECT_EQ(sem->Members()[0]->Align(), 8u);
    EXPECT_EQ(sem->Members()[0]->Size(), 8u);
    EXPECT_EQ(sem->Members()[1]->Offset(), 16u);  // vec3
    EXPECT_EQ(sem->Members()[1]->Align(), 16u);
    EXPECT_EQ(sem->Members()[1]->Size(), 12u);
    EXPECT_EQ(sem->Members()[2]->Offset(), 32u);  // vec4
    EXPECT_EQ(sem->Members()[2]->Align(), 16u);
    EXPECT_EQ(sem->Members()[2]->Size(), 16u);
    for (auto& m : sem->Members()) {
        EXPECT_EQ(m->Struct()->Declaration(), s);
    }
}

TEST_F(ResolverStructLayoutTest, Matrix) {
    Enable(wgsl::Extension::kF16);

    auto* s = Structure("S", Vector{
                                 Member("a_1", ty.mat2x2<f32>()),
                                 Member("a_2", ty.mat2x2<f16>()),
                                 Member("b_1", ty.mat2x3<f32>()),
                                 Member("b_2", ty.mat2x3<f16>()),
                                 Member("c_1", ty.mat2x4<f32>()),
                                 Member("c_2", ty.mat2x4<f16>()),
                                 Member("d_1", ty.mat3x2<f32>()),
                                 Member("d_2", ty.mat3x2<f16>()),
                                 Member("e_1", ty.mat3x3<f32>()),
                                 Member("e_2", ty.mat3x3<f16>()),
                                 Member("f_1", ty.mat3x4<f32>()),
                                 Member("f_2", ty.mat3x4<f16>()),
                                 Member("g_1", ty.mat4x2<f32>()),
                                 Member("g_2", ty.mat4x2<f16>()),
                                 Member("h_1", ty.mat4x3<f32>()),
                                 Member("h_2", ty.mat4x3<f16>()),
                                 Member("i_1", ty.mat4x4<f32>()),
                                 Member("i_2", ty.mat4x4<f16>()),
                             });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Size(), 576u);
    EXPECT_EQ(sem->SizeNoPadding(), 576u);
    EXPECT_EQ(sem->Align(), 16u);
    ASSERT_EQ(sem->Members().Length(), 18u);
    EXPECT_EQ(sem->Members()[0]->Offset(), 0u);  // mat2x2<f32>
    EXPECT_EQ(sem->Members()[0]->Align(), 8u);
    EXPECT_EQ(sem->Members()[0]->Size(), 16u);
    EXPECT_EQ(sem->Members()[1]->Offset(), 16u);  // mat2x2<f16>
    EXPECT_EQ(sem->Members()[1]->Align(), 4u);
    EXPECT_EQ(sem->Members()[1]->Size(), 8u);
    EXPECT_EQ(sem->Members()[2]->Offset(), 32u);  // mat2x3<f32>
    EXPECT_EQ(sem->Members()[2]->Align(), 16u);
    EXPECT_EQ(sem->Members()[2]->Size(), 32u);
    EXPECT_EQ(sem->Members()[3]->Offset(), 64u);  // mat2x3<f16>
    EXPECT_EQ(sem->Members()[3]->Align(), 8u);
    EXPECT_EQ(sem->Members()[3]->Size(), 16u);
    EXPECT_EQ(sem->Members()[4]->Offset(), 80u);  // mat2x4<f32>
    EXPECT_EQ(sem->Members()[4]->Align(), 16u);
    EXPECT_EQ(sem->Members()[4]->Size(), 32u);
    EXPECT_EQ(sem->Members()[5]->Offset(), 112u);  // mat2x4<f16>
    EXPECT_EQ(sem->Members()[5]->Align(), 8u);
    EXPECT_EQ(sem->Members()[5]->Size(), 16u);
    EXPECT_EQ(sem->Members()[6]->Offset(), 128u);  // mat3x2<f32>
    EXPECT_EQ(sem->Members()[6]->Align(), 8u);
    EXPECT_EQ(sem->Members()[6]->Size(), 24u);
    EXPECT_EQ(sem->Members()[7]->Offset(), 152u);  // mat3x2<f16>
    EXPECT_EQ(sem->Members()[7]->Align(), 4u);
    EXPECT_EQ(sem->Members()[7]->Size(), 12u);
    EXPECT_EQ(sem->Members()[8]->Offset(), 176u);  // mat3x3<f32>
    EXPECT_EQ(sem->Members()[8]->Align(), 16u);
    EXPECT_EQ(sem->Members()[8]->Size(), 48u);
    EXPECT_EQ(sem->Members()[9]->Offset(), 224u);  // mat3x3<f16>
    EXPECT_EQ(sem->Members()[9]->Align(), 8u);
    EXPECT_EQ(sem->Members()[9]->Size(), 24u);
    EXPECT_EQ(sem->Members()[10]->Offset(), 256u);  // mat3x4<f32>
    EXPECT_EQ(sem->Members()[10]->Align(), 16u);
    EXPECT_EQ(sem->Members()[10]->Size(), 48u);
    EXPECT_EQ(sem->Members()[11]->Offset(), 304u);  // mat3x4<f16>
    EXPECT_EQ(sem->Members()[11]->Align(), 8u);
    EXPECT_EQ(sem->Members()[11]->Size(), 24u);
    EXPECT_EQ(sem->Members()[12]->Offset(), 328u);  // mat4x2<f32>
    EXPECT_EQ(sem->Members()[12]->Align(), 8u);
    EXPECT_EQ(sem->Members()[12]->Size(), 32u);
    EXPECT_EQ(sem->Members()[13]->Offset(), 360u);  // mat4x2<f16>
    EXPECT_EQ(sem->Members()[13]->Align(), 4u);
    EXPECT_EQ(sem->Members()[13]->Size(), 16u);
    EXPECT_EQ(sem->Members()[14]->Offset(), 384u);  // mat4x3<f32>
    EXPECT_EQ(sem->Members()[14]->Align(), 16u);
    EXPECT_EQ(sem->Members()[14]->Size(), 64u);
    EXPECT_EQ(sem->Members()[15]->Offset(), 448u);  // mat4x3<f16>
    EXPECT_EQ(sem->Members()[15]->Align(), 8u);
    EXPECT_EQ(sem->Members()[15]->Size(), 32u);
    EXPECT_EQ(sem->Members()[16]->Offset(), 480u);  // mat4x4<f32>
    EXPECT_EQ(sem->Members()[16]->Align(), 16u);
    EXPECT_EQ(sem->Members()[16]->Size(), 64u);
    EXPECT_EQ(sem->Members()[17]->Offset(), 544u);  // mat4x4<f16>
    EXPECT_EQ(sem->Members()[17]->Align(), 8u);
    EXPECT_EQ(sem->Members()[17]->Size(), 32u);

    for (auto& m : sem->Members()) {
        EXPECT_EQ(m->Struct()->Declaration(), s);
    }
}

TEST_F(ResolverStructLayoutTest, NestedStruct) {
    auto* inner = Structure("Inner", Vector{
                                         Member("a", ty.mat3x3<f32>()),
                                     });
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.Of(inner)),
                                 Member("c", ty.i32()),
                             });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Size(), 80u);
    EXPECT_EQ(sem->SizeNoPadding(), 68u);
    EXPECT_EQ(sem->Align(), 16u);
    ASSERT_EQ(sem->Members().Length(), 3u);
    EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
    EXPECT_EQ(sem->Members()[0]->Align(), 4u);
    EXPECT_EQ(sem->Members()[0]->Size(), 4u);
    EXPECT_EQ(sem->Members()[1]->Offset(), 16u);
    EXPECT_EQ(sem->Members()[1]->Align(), 16u);
    EXPECT_EQ(sem->Members()[1]->Size(), 48u);
    EXPECT_EQ(sem->Members()[2]->Offset(), 64u);
    EXPECT_EQ(sem->Members()[2]->Align(), 4u);
    EXPECT_EQ(sem->Members()[2]->Size(), 4u);
    for (auto& m : sem->Members()) {
        EXPECT_EQ(m->Struct()->Declaration(), s);
    }
}

TEST_F(ResolverStructLayoutTest, SizeAttributes) {
    auto* inner = Structure("Inner", Vector{
                                         Member("a", ty.f32(), Vector{MemberSize(8_a)}),
                                         Member("b", ty.f32(), Vector{MemberSize(16_a)}),
                                         Member("c", ty.f32(), Vector{MemberSize(8_a)}),
                                     });
    auto* s = Structure("S", Vector{
                                 Member("a", ty.f32(), Vector{MemberSize(4_a)}),
                                 Member("b", ty.u32(), Vector{MemberSize(8_a)}),
                                 Member("c", ty.Of(inner)),
                                 Member("d", ty.i32(), Vector{MemberSize(32_a)}),
                             });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Size(), 76u);
    EXPECT_EQ(sem->SizeNoPadding(), 76u);
    EXPECT_EQ(sem->Align(), 4u);
    ASSERT_EQ(sem->Members().Length(), 4u);
    EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
    EXPECT_EQ(sem->Members()[0]->Align(), 4u);
    EXPECT_EQ(sem->Members()[0]->Size(), 4u);
    EXPECT_EQ(sem->Members()[1]->Offset(), 4u);
    EXPECT_EQ(sem->Members()[1]->Align(), 4u);
    EXPECT_EQ(sem->Members()[1]->Size(), 8u);
    EXPECT_EQ(sem->Members()[2]->Offset(), 12u);
    EXPECT_EQ(sem->Members()[2]->Align(), 4u);
    EXPECT_EQ(sem->Members()[2]->Size(), 32u);
    EXPECT_EQ(sem->Members()[3]->Offset(), 44u);
    EXPECT_EQ(sem->Members()[3]->Align(), 4u);
    EXPECT_EQ(sem->Members()[3]->Size(), 32u);
    for (auto& m : sem->Members()) {
        EXPECT_EQ(m->Struct()->Declaration(), s);
    }
}

TEST_F(ResolverStructLayoutTest, AlignAttributes) {
    auto* inner = Structure("Inner", Vector{
                                         Member("a", ty.f32(), Vector{MemberAlign(8_i)}),
                                         Member("b", ty.f32(), Vector{MemberAlign(16_i)}),
                                         Member("c", ty.f32(), Vector{MemberAlign(4_i)}),
                                     });
    auto* s = Structure("S", Vector{
                                 Member("a", ty.f32(), Vector{MemberAlign(4_i)}),
                                 Member("b", ty.u32(), Vector{MemberAlign(8_i)}),
                                 Member("c", ty.Of(inner)),
                                 Member("d", ty.i32(), Vector{MemberAlign(32_i)}),
                             });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Size(), 96u);
    EXPECT_EQ(sem->SizeNoPadding(), 68u);
    EXPECT_EQ(sem->Align(), 32u);
    ASSERT_EQ(sem->Members().Length(), 4u);
    EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
    EXPECT_EQ(sem->Members()[0]->Align(), 4u);
    EXPECT_EQ(sem->Members()[0]->Size(), 4u);
    EXPECT_EQ(sem->Members()[1]->Offset(), 8u);
    EXPECT_EQ(sem->Members()[1]->Align(), 8u);
    EXPECT_EQ(sem->Members()[1]->Size(), 4u);
    EXPECT_EQ(sem->Members()[2]->Offset(), 16u);
    EXPECT_EQ(sem->Members()[2]->Align(), 16u);
    EXPECT_EQ(sem->Members()[2]->Size(), 32u);
    EXPECT_EQ(sem->Members()[3]->Offset(), 64u);
    EXPECT_EQ(sem->Members()[3]->Align(), 32u);
    EXPECT_EQ(sem->Members()[3]->Size(), 4u);
    for (auto& m : sem->Members()) {
        EXPECT_EQ(m->Struct()->Declaration(), s);
    }
}

TEST_F(ResolverStructLayoutTest, StructWithLotsOfPadding) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32(), Vector{MemberAlign(1024_i)}),
                             });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Size(), 1024u);
    EXPECT_EQ(sem->SizeNoPadding(), 4u);
    EXPECT_EQ(sem->Align(), 1024u);
    ASSERT_EQ(sem->Members().Length(), 1u);
    EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
    EXPECT_EQ(sem->Members()[0]->Align(), 1024u);
    EXPECT_EQ(sem->Members()[0]->Size(), 4u);
    for (auto& m : sem->Members()) {
        EXPECT_EQ(m->Struct()->Declaration(), s);
    }
}

TEST_F(ResolverStructLayoutTest, OffsetAttributes) {
    auto* inner = Structure("Inner", Vector{
                                         Member("a", ty.f32(), Vector{MemberOffset(8_i)}),
                                         Member("b", ty.f32(), Vector{MemberOffset(16_i)}),
                                         Member("c", ty.f32(), Vector{MemberOffset(32_i)}),
                                     });
    auto* s = Structure("S", Vector{
                                 Member("a", ty.f32(), Vector{MemberOffset(4_i)}),
                                 Member("b", ty.u32(), Vector{MemberOffset(8_i)}),
                                 Member("c", ty.Of(inner), Vector{MemberOffset(32_i)}),
                                 Member("d", ty.i32()),
                                 Member("e", ty.i32(), Vector{MemberOffset(128_i)}),
                             });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Size(), 132u);
    EXPECT_EQ(sem->SizeNoPadding(), 132u);
    EXPECT_EQ(sem->Align(), 4u);
    ASSERT_EQ(sem->Members().Length(), 5u);
    EXPECT_EQ(sem->Members()[0]->Offset(), 4u);
    EXPECT_EQ(sem->Members()[0]->Align(), 4u);
    EXPECT_EQ(sem->Members()[0]->Size(), 4u);
    EXPECT_EQ(sem->Members()[1]->Offset(), 8u);
    EXPECT_EQ(sem->Members()[1]->Align(), 4u);
    EXPECT_EQ(sem->Members()[1]->Size(), 4u);
    EXPECT_EQ(sem->Members()[2]->Offset(), 32u);
    EXPECT_EQ(sem->Members()[2]->Align(), 4u);
    EXPECT_EQ(sem->Members()[2]->Size(), 36u);
    EXPECT_EQ(sem->Members()[3]->Offset(), 68u);
    EXPECT_EQ(sem->Members()[3]->Align(), 4u);
    EXPECT_EQ(sem->Members()[3]->Size(), 4u);
    EXPECT_EQ(sem->Members()[4]->Offset(), 128u);
    EXPECT_EQ(sem->Members()[4]->Align(), 4u);
    EXPECT_EQ(sem->Members()[4]->Size(), 4u);
    for (auto& m : sem->Members()) {
        EXPECT_EQ(m->Struct()->Declaration(), s);
    }
}

}  // namespace
}  // namespace tint::resolver
