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

using ResolverHostShareableValidationTest = ResolverTest;

TEST_F(ResolverHostShareableValidationTest, BoolMember) {
    auto* s = Structure("S", Vector{Member(Source{{56, 78}}, "x", ty.bool_(Source{{12, 34}}))});

    GlobalVar(Source{{90, 12}}, "g", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead,
              Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
56:78 note: while analyzing structure member S.x
90:12 note: while instantiating 'var' g)");
}

TEST_F(ResolverHostShareableValidationTest, BoolVectorMember) {
    auto* s =
        Structure("S", Vector{Member(Source{{56, 78}}, "x", ty.vec3<bool>(Source{{12, 34}}))});

    GlobalVar(Source{{90, 12}}, "g", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead,
              Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: type 'vec3<bool>' cannot be used in address space 'storage' as it is non-host-shareable
56:78 note: while analyzing structure member S.x
90:12 note: while instantiating 'var' g)");
}

TEST_F(ResolverHostShareableValidationTest, Aliases) {
    Alias("a1", ty.bool_());
    auto* s = Structure("S", Vector{Member(Source{{56, 78}}, "x", ty(Source{{12, 34}}, "a1"))});
    auto* a2 = Alias("a2", ty.Of(s));
    GlobalVar(Source{{90, 12}}, "g", ty.Of(a2), core::AddressSpace::kStorage, core::Access::kRead,
              Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
56:78 note: while analyzing structure member S.x
90:12 note: while instantiating 'var' g)");
}

TEST_F(ResolverHostShareableValidationTest, NestedStructures) {
    auto* i1 = Structure("I1", Vector{Member(Source{{1, 2}}, "x", ty.bool_())});
    auto* i2 = Structure("I2", Vector{Member(Source{{3, 4}}, "y", ty.Of(i1))});
    auto* i3 = Structure("I3", Vector{Member(Source{{5, 6}}, "z", ty.Of(i2))});

    auto* s = Structure("S", Vector{Member(Source{{7, 8}}, "m", ty.Of(i3))});

    GlobalVar(Source{{9, 10}}, "g", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead,
              Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(error: type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
1:2 note: while analyzing structure member I1.x
3:4 note: while analyzing structure member I2.y
5:6 note: while analyzing structure member I3.z
7:8 note: while analyzing structure member S.m
9:10 note: while instantiating 'var' g)");
}

TEST_F(ResolverHostShareableValidationTest, NoError) {
    Enable(wgsl::Extension::kF16);

    auto* i1 = Structure("I1", Vector{
                                   Member(Source{{1, 1}}, "w1", ty.f32()),
                                   Member(Source{{2, 1}}, "x1", ty.f32()),
                                   Member(Source{{3, 1}}, "y1", ty.vec3<f32>()),
                                   Member(Source{{4, 1}}, "z1", ty.array<i32, 4>()),
                               });
    auto* a1 = Alias("a1", ty.Of(i1));
    auto* i2 = Structure("I2", Vector{
                                   Member(Source{{5, 1}}, "x2", ty.mat2x2<f32>()),
                                   Member(Source{{6, 1}}, "w2", ty.mat3x4<f32>()),
                                   Member(Source{{7, 1}}, "z2", ty.Of(i1)),
                               });
    auto* a2 = Alias("a2", ty.Of(i2));
    auto* i3 = Structure("I3", Vector{
                                   Member(Source{{4, 1}}, "x3", ty.Of(a1)),
                                   Member(Source{{5, 1}}, "y3", ty.Of(i2)),
                                   Member(Source{{6, 1}}, "z3", ty.Of(a2)),
                               });

    auto* s = Structure("S", Vector{Member(Source{{7, 8}}, "m", ty.Of(i3))});

    GlobalVar(Source{{9, 10}}, "g", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead,
              Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace tint::resolver
