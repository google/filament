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

#include "gmock/gmock.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ResolverAddressSpaceLayoutValidationTest = ResolverTest;

// Detect unaligned member for storage buffers
TEST_F(ResolverAddressSpaceLayoutValidationTest, StorageBuffer_UnalignedMember) {
    // struct S {
    //     @size(5) a : f32;
    //     @align(1) b : f32;
    // };
    // @group(0) @binding(0)
    // var<storage> a : S;

    Structure(Ident(Source{{12, 34}}, "S"),
              Vector{
                  Member("a", ty.f32(), Vector{MemberSize(5_a)}),
                  Member(Source{{34, 56}}, "b", ty.f32(), Vector{MemberAlign(1_i)}),
              });

    GlobalVar(Source{{78, 90}}, "a", ty.AsType("S"), core::AddressSpace::kStorage, Group(0_a),
              Binding(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(34:56 error: the offset of a struct member of type 'f32' in address space 'storage' must be a multiple of 4 bytes, but 'b' is currently at offset 5. Consider setting '@align(4)' on this member
12:34 note: see layout of struct:
/*           align(4) size(12) */ struct S {
/* offset(0) align(4) size( 5) */   a : f32,
/* offset(5) align(1) size( 4) */   b : f32,
/* offset(9) align(1) size( 3) */   // -- implicit struct size padding --
/*                             */ };
78:90 note: 'S' used in address space 'storage' here)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, StorageBuffer_UnalignedMember_SuggestedFix) {
    // struct S {
    //     @size(5) a : f32;
    //     @align(4) b : f32;
    // };
    // @group(0) @binding(0)
    // var<storage> a : S;

    Structure("S", Vector{
                       Member("a", ty.f32(), Vector{MemberSize(5_a)}),
                       Member("b", ty.f32(), Vector{MemberAlign(4_i)}),
                   });

    GlobalVar("a", ty.AsType("S"), core::AddressSpace::kStorage, Group(0_a), Binding(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

// Detect unaligned struct member for uniform buffers
TEST_F(ResolverAddressSpaceLayoutValidationTest, UniformBuffer_UnalignedMember_Struct) {
    // struct Inner {
    //   scalar : i32;
    // };
    //
    // struct Outer {
    //   @size(5) scalar : f32;
    //   @align(1) inner : Inner;
    // };
    //
    // @group(0) @binding(0)
    // var<uniform> a : Outer;

    Structure(Ident(Source{{12, 34}}, "Inner"), Vector{
                                                    Member("scalar", ty.i32()),
                                                });

    Structure(Ident(Source{{34, 56}}, "Outer"),
              Vector{
                  Member("scalar", ty.f32(), Vector{MemberSize(5_a)}),
                  Member(Source{{56, 78}}, "inner", ty.AsType("Inner"), Vector{MemberAlign(1_a)}),
              });

    GlobalVar(Source{{78, 90}}, "a", ty.AsType("Outer"), core::AddressSpace::kUniform, Group(0_a),
              Binding(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: the offset of a struct member of type 'Inner' in address space 'uniform' must be a multiple of 4 bytes, but 'inner' is currently at offset 5. Consider setting '@align(4)' on this member
34:56 note: see layout of struct:
/*           align(4) size(12) */ struct Outer {
/* offset(0) align(4) size( 5) */   scalar : f32,
/* offset(5) align(1) size( 4) */   inner : Inner,
/* offset(9) align(1) size( 3) */   // -- implicit struct size padding --
/*                             */ };
12:34 note: and layout of struct member:
/*           align(4) size(4) */ struct Inner {
/* offset(0) align(4) size(4) */   scalar : i32,
/*                            */ };
78:90 note: 'Outer' used in address space 'uniform' here)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest,
       UniformBuffer_UnalignedMember_Struct_SuggestedFix) {
    // struct Inner {
    //   scalar : i32;
    // };
    //
    // struct Outer {
    //   @size(5) scalar : f32;
    //   @align(4) inner : Inner;
    // };
    //
    // @group(0) @binding(0)
    // var<uniform> a : Outer;

    Structure("Inner", Vector{
                           Member("scalar", ty.i32()),
                       });

    Structure("Outer", Vector{
                           Member("scalar", ty.f32(), Vector{MemberSize(5_a)}),
                           Member("inner", ty.AsType("Inner"), Vector{MemberAlign(4_i)}),
                       });

    GlobalVar("a", ty.AsType("Outer"), core::AddressSpace::kUniform, Group(0_a), Binding(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceLayoutValidationTest,
       UniformBuffer_MembersOffsetNotMultipleOf16_SuggestedFix) {
    // struct Inner {
    //   @align(4) @size(5) scalar : i32;
    // };
    //
    // struct Outer {
    //   @align(16) inner : Inner;
    //   scalar : i32;
    // };
    //
    // @group(0) @binding(0)
    // var<uniform> a : Outer;

    Structure("Inner", Vector{
                           Member("scalar", ty.i32(), Vector{MemberAlign(4_i), MemberSize(5_a)}),
                       });

    Structure("Outer", Vector{
                           Member("inner", ty.AsType("Inner")),
                           Member("scalar", ty.i32(), Vector{MemberAlign(16_i)}),
                       });

    GlobalVar("a", ty.AsType("Outer"), core::AddressSpace::kUniform, Group(0_a), Binding(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

// Make sure that this doesn't fail validation because vec3's align is 16, but
// size is 12. 's' should be at offset 12, which is okay here.
TEST_F(ResolverAddressSpaceLayoutValidationTest, UniformBuffer_Vec3MemberOffset_NoFail) {
    // struct ScalarPackedAtEndOfVec3 {
    //     v : vec3<f32>;
    //     s : f32;
    // };
    // @group(0) @binding(0)
    // var<uniform> a : ScalarPackedAtEndOfVec3;

    Structure("ScalarPackedAtEndOfVec3", Vector{
                                             Member("v", ty.vec3(ty.f32())),
                                             Member("s", ty.f32()),
                                         });

    GlobalVar("a", ty.AsType("ScalarPackedAtEndOfVec3"), core::AddressSpace::kUniform, Group(0_a),
              Binding(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

// Make sure that this doesn't fail validation because vec3's align is 8, but
// size is 6. 's' should be at offset 6, which is okay here.
TEST_F(ResolverAddressSpaceLayoutValidationTest, UniformBuffer_Vec3F16MemberOffset_NoFail) {
    // struct ScalarPackedAtEndOfVec3 {
    //     v : vec3<f16>;
    //     s : f16;
    // };
    // @group(0) @binding(0)
    // var<uniform> a : ScalarPackedAtEndOfVec3;

    Enable(wgsl::Extension::kF16);

    Structure("ScalarPackedAtEndOfVec3", Vector{
                                             Member("v", ty.vec3(ty.f16())),
                                             Member("s", ty.f16()),
                                         });

    GlobalVar("a", ty.AsType("ScalarPackedAtEndOfVec3"), core::AddressSpace::kUniform, Group(0_a),
              Binding(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

// Detect unaligned member for immediate data buffers
TEST_F(ResolverAddressSpaceLayoutValidationTest, Immediate_UnalignedMember) {
    // struct S {
    //     @size(5) a : f32;
    //     @align(1) b : f32;
    // };
    // var<immediate> a : S;
    Structure(Ident(Source{{12, 34}}, "S"),
              Vector{Member("a", ty.f32(), Vector{MemberSize(5_a)}),
                     Member(Source{{34, 56}}, "b", ty.f32(), Vector{MemberAlign(1_i)})});
    GlobalVar(Source{{78, 90}}, "a", ty.AsType("S"), core::AddressSpace::kImmediate);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(34:56 error: the offset of a struct member of type 'f32' in address space 'immediate' must be a multiple of 4 bytes, but 'b' is currently at offset 5. Consider setting '@align(4)' on this member
12:34 note: see layout of struct:
/*           align(4) size(12) */ struct S {
/* offset(0) align(4) size( 5) */   a : f32,
/* offset(5) align(1) size( 4) */   b : f32,
/* offset(9) align(1) size( 3) */   // -- implicit struct size padding --
/*                             */ };
78:90 note: 'S' used in address space 'immediate' here)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, Immediate_Aligned) {
    // struct S {
    //     @size(5) a : f32;
    //     @align(4) b : f32;
    // };
    // var<immediate> a : S;
    Structure("S", Vector{Member("a", ty.f32(), Vector{MemberSize(5_a)}),
                          Member("b", ty.f32(), Vector{MemberAlign(4_i)})});
    GlobalVar("a", ty.AsType("S"), core::AddressSpace::kImmediate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, AlignAttributeTooSmall_Storage) {
    // struct S {
    //   @align(4) vector : vec4u;
    //   scalar : u32;
    // };
    //
    // @group(0) @binding(0)
    // var<storage, read_write> a : array<S>;
    Structure(
        "S", Vector{
                 Member("vector", ty.vec4<u32>(), Vector{MemberAlign(Expr(Source{{12, 34}}, 4_a))}),
                 Member("scalar", ty.u32()),
             });

    GlobalVar(Source{{56, 78}}, "a", ty.AsType("S"), core::AddressSpace::kStorage,
              core::Access::kReadWrite, Group(0_a), Binding(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: alignment must be a multiple of '16' bytes for the 'storage' address space
56:78 note: 'S' used in address space 'storage' here)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, AlignAttributeTooSmall_Workgroup) {
    // struct S {
    //   @align(4) vector : vec4u;
    //   scalar : u32;
    // };
    //
    // var<workgroup> a : array<S, 4>;
    Structure(
        "S", Vector{
                 Member("vector", ty.vec4<u32>(), Vector{MemberAlign(Expr(Source{{12, 34}}, 4_a))}),
                 Member("scalar", ty.u32()),
             });

    GlobalVar(Source{{56, 78}}, "a", ty.AsType("S"), core::AddressSpace::kWorkgroup, Group(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: alignment must be a multiple of '16' bytes for the 'workgroup' address space
56:78 note: 'S' used in address space 'workgroup' here)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, AlignAttributeTooSmall_Private) {
    // struct S {
    //   @align(4) vector : vec4u;
    //   scalar : u32;
    // };
    //
    // var<private> a : array<S, 4>;
    Structure(
        "S", Vector{
                 Member("vector", ty.vec4<u32>(), Vector{MemberAlign(Expr(Source{{12, 34}}, 4_a))}),
                 Member("scalar", ty.u32()),
             });

    GlobalVar(Source{{56, 78}}, "a", ty.AsType("S"), core::AddressSpace::kPrivate, Group(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: alignment must be a multiple of '16' bytes for the 'private' address space
56:78 note: 'S' used in address space 'private' here)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, AlignAttributeTooSmall_Function) {
    // struct S {
    //   @align(4) vector : vec4u;
    //   scalar : u32;
    // };
    //
    // fn foo() {
    //   var a : array<S, 4>;
    // }
    Structure(
        "S", Vector{
                 Member("vector", ty.vec4<u32>(), Vector{MemberAlign(Expr(Source{{12, 34}}, 4_a))}),
                 Member("scalar", ty.u32()),
             });

    GlobalVar(Source{{56, 78}}, "a", ty.AsType("S"), core::AddressSpace::kFunction, Group(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: alignment must be a multiple of '16' bytes for the 'function' address space
56:78 note: 'S' used in address space 'function' here)");
}

}  // namespace
}  // namespace tint::resolver
