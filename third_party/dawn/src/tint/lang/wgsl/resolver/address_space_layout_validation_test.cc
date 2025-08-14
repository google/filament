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

    GlobalVar(Source{{78, 90}}, "a", ty("S"), core::AddressSpace::kStorage, Group(0_a),
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

    GlobalVar("a", ty("S"), core::AddressSpace::kStorage, Group(0_a), Binding(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

// Detect unaligned struct member for uniform buffers
TEST_F(ResolverAddressSpaceLayoutValidationTest, UniformBuffer_UnalignedMember_Struct) {
    // struct Inner {
    //   scalar : i32;
    // };
    //
    // struct Outer {
    //   scalar : f32;
    //   inner : Inner;
    // };
    //
    // @group(0) @binding(0)
    // var<uniform> a : Outer;

    Structure(Ident(Source{{12, 34}}, "Inner"), Vector{
                                                    Member("scalar", ty.i32()),
                                                });

    Structure(Ident(Source{{34, 56}}, "Outer"), Vector{
                                                    Member("scalar", ty.f32()),
                                                    Member(Source{{56, 78}}, "inner", ty("Inner")),
                                                });

    GlobalVar(Source{{78, 90}}, "a", ty("Outer"), core::AddressSpace::kUniform, Group(0_a),
              Binding(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: the offset of a struct member of type 'Inner' in address space 'uniform' must be a multiple of 16 bytes, but 'inner' is currently at offset 4. Consider setting '@align(16)' on this member
34:56 note: see layout of struct:
/*           align(4) size(8) */ struct Outer {
/* offset(0) align(4) size(4) */   scalar : f32,
/* offset(4) align(4) size(4) */   inner : Inner,
/*                            */ };
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
    //   scalar : f32;
    //   @align(16) inner : Inner;
    // };
    //
    // @group(0) @binding(0)
    // var<uniform> a : Outer;

    Structure("Inner", Vector{
                           Member("scalar", ty.i32()),
                       });

    Structure("Outer", Vector{
                           Member("scalar", ty.f32()),
                           Member("inner", ty("Inner"), Vector{MemberAlign(16_i)}),
                       });

    GlobalVar("a", ty("Outer"), core::AddressSpace::kUniform, Group(0_a), Binding(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

// Detect unaligned array member for uniform buffers
TEST_F(ResolverAddressSpaceLayoutValidationTest, UniformBuffer_UnalignedMember_Array) {
    // type Inner = @stride(16) array<f32, 10u>;
    //
    // struct Outer {
    //   scalar : f32;
    //   inner : Inner;
    // };
    //
    // @group(0) @binding(0)
    // var<uniform> a : Outer;
    Alias("Inner", ty.array<f32, 10>(Vector{Stride(16)}));

    Structure(Ident(Source{{12, 34}}, "Outer"), Vector{
                                                    Member("scalar", ty.f32()),
                                                    Member(Source{{56, 78}}, "inner", ty("Inner")),
                                                });

    GlobalVar(Source{{78, 90}}, "a", ty("Outer"), core::AddressSpace::kUniform, Group(0_a),
              Binding(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: the offset of a struct member of type '@stride(16) array<f32, 10>' in address space 'uniform' must be a multiple of 16 bytes, but 'inner' is currently at offset 4. Consider setting '@align(16)' on this member
12:34 note: see layout of struct:
/*             align(4) size(164) */ struct Outer {
/* offset(  0) align(4) size(  4) */   scalar : f32,
/* offset(  4) align(4) size(160) */   inner : @stride(16) array<f32, 10>,
/*                                */ };
78:90 note: 'Outer' used in address space 'uniform' here)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, UniformBuffer_UnalignedMember_Array_SuggestedFix) {
    // type Inner = @stride(16) array<f32, 10u>;
    //
    // struct Outer {
    //   scalar : f32;
    //   @align(16) inner : Inner;
    // };
    //
    // @group(0) @binding(0)
    // var<uniform> a : Outer;
    Alias("Inner", ty.array<f32, 10>(Vector{Stride(16)}));

    Structure("Outer", Vector{
                           Member("scalar", ty.f32()),
                           Member("inner", ty("Inner"), Vector{MemberAlign(16_i)}),
                       });

    GlobalVar("a", ty("Outer"), core::AddressSpace::kUniform, Group(0_a), Binding(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

// Detect uniform buffers with byte offset between 2 members that is not a
// multiple of 16 bytes
TEST_F(ResolverAddressSpaceLayoutValidationTest, UniformBuffer_MembersOffsetNotMultipleOf16) {
    // struct Inner {
    //   @align(4) @size(5) scalar : i32;
    // };
    //
    // struct Outer {
    //   inner : Inner;
    //   scalar : i32;
    // };
    //
    // @group(0) @binding(0)
    // var<uniform> a : Outer;

    Structure(Ident(Source{{12, 34}}, "Inner"),
              Vector{
                  Member("scalar", ty.i32(), Vector{MemberAlign(4_i), MemberSize(5_a)}),
              });

    Structure(Source{{34, 56}}, "Outer",
              Vector{
                  Member(Source{{56, 78}}, "inner", ty("Inner")),
                  Member(Source{{78, 90}}, "scalar", ty.i32()),
              });

    GlobalVar(Source{{22, 24}}, "a", ty("Outer"), core::AddressSpace::kUniform, Group(0_a),
              Binding(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(78:90 error: 'uniform' storage requires that the number of bytes between the start of the previous member of type struct and the current member be a multiple of 16 bytes, but there are currently 8 bytes between 'inner' and 'scalar'. Consider setting '@align(16)' on this member
note: see layout of struct:
/*            align(4) size(12) */ struct Outer {
/* offset( 0) align(4) size( 8) */   inner : Inner,
/* offset( 8) align(4) size( 4) */   scalar : i32,
/*                              */ };
12:34 note: and layout of previous member struct:
/*           align(4) size(8) */ struct Inner {
/* offset(0) align(4) size(5) */   scalar : i32,
/* offset(5) align(1) size(3) */   // -- implicit struct size padding --
/*                            */ };
22:24 note: 'Outer' used in address space 'uniform' here)");
}

// See https://crbug.com/tint/1344
TEST_F(ResolverAddressSpaceLayoutValidationTest,
       UniformBuffer_MembersOffsetNotMultipleOf16_InnerMoreMembersThanOuter) {
    // struct Inner {
    //   a : i32;
    //   b : i32;
    //   c : i32;
    //   @align(4) @size(5) scalar : i32;
    // };
    //
    // struct Outer {
    //   inner : Inner;
    //   scalar : i32;
    // };
    //
    // @group(0) @binding(0)
    // var<uniform> a : Outer;

    Structure(Ident(Source{{12, 34}}, "Inner"),
              Vector{
                  Member("a", ty.i32()),
                  Member("b", ty.i32()),
                  Member("c", ty.i32()),
                  Member("scalar", ty.i32(), Vector{MemberAlign(4_i), MemberSize(5_a)}),
              });

    Structure(Source{{34, 56}}, "Outer",
              Vector{
                  Member(Source{{56, 78}}, "inner", ty("Inner")),
                  Member(Source{{78, 90}}, "scalar", ty.i32()),
              });

    GlobalVar(Source{{22, 24}}, "a", ty("Outer"), core::AddressSpace::kUniform, Group(0_a),
              Binding(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(78:90 error: 'uniform' storage requires that the number of bytes between the start of the previous member of type struct and the current member be a multiple of 16 bytes, but there are currently 20 bytes between 'inner' and 'scalar'. Consider setting '@align(16)' on this member
note: see layout of struct:
/*            align(4) size(24) */ struct Outer {
/* offset( 0) align(4) size(20) */   inner : Inner,
/* offset(20) align(4) size( 4) */   scalar : i32,
/*                              */ };
12:34 note: and layout of previous member struct:
/*            align(4) size(20) */ struct Inner {
/* offset( 0) align(4) size( 4) */   a : i32,
/* offset( 4) align(4) size( 4) */   b : i32,
/* offset( 8) align(4) size( 4) */   c : i32,
/* offset(12) align(4) size( 5) */   scalar : i32,
/* offset(17) align(1) size( 3) */   // -- implicit struct size padding --
/*                              */ };
22:24 note: 'Outer' used in address space 'uniform' here)");
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
                           Member("inner", ty("Inner")),
                           Member("scalar", ty.i32(), Vector{MemberAlign(16_i)}),
                       });

    GlobalVar("a", ty("Outer"), core::AddressSpace::kUniform, Group(0_a), Binding(0_a));

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

    GlobalVar("a", ty("ScalarPackedAtEndOfVec3"), core::AddressSpace::kUniform, Group(0_a),
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

    GlobalVar("a", ty("ScalarPackedAtEndOfVec3"), core::AddressSpace::kUniform, Group(0_a),
              Binding(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

// Detect array stride must be a multiple of 16 bytes for uniform buffers
TEST_F(ResolverAddressSpaceLayoutValidationTest, UniformBuffer_InvalidArrayStride_Scalar) {
    // type Inner = array<f32, 10u>;
    //
    // struct Outer {
    //   inner : Inner;
    //   scalar : i32;
    // };
    //
    // @group(0) @binding(0)
    // var<uniform> a : Outer;

    Alias("Inner", ty.array<f32, 10>());

    Structure(Ident(Source{{12, 34}}, "Outer"), Vector{
                                                    Member("inner", ty(Source{{34, 56}}, "Inner")),
                                                    Member("scalar", ty.i32()),
                                                });

    GlobalVar(Source{{78, 90}}, "a", ty("Outer"), core::AddressSpace::kUniform, Group(0_a),
              Binding(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(34:56 error: 'uniform' storage requires that array elements are aligned to 16 bytes, but array element of type 'f32' has a stride of 4 bytes. Consider using a vector or struct as the element type instead.
12:34 note: see layout of struct:
/*            align(4) size(44) */ struct Outer {
/* offset( 0) align(4) size(40) */   inner : array<f32, 10>,
/* offset(40) align(4) size( 4) */   scalar : i32,
/*                              */ };
78:90 note: 'Outer' used in address space 'uniform' here)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, UniformBuffer_InvalidArrayStride_Vector) {
    // type Inner = array<vec2<f32>, 10u>;
    //
    // struct Outer {
    //   inner : Inner;
    //   scalar : i32;
    // };
    //
    // @group(0) @binding(0)
    // var<uniform> a : Outer;

    Alias("Inner", ty.array<vec2<f32>, 10>());

    Structure(Ident(Source{{12, 34}}, "Outer"), Vector{
                                                    Member("inner", ty(Source{{34, 56}}, "Inner")),
                                                    Member("scalar", ty.i32()),
                                                });

    GlobalVar(Source{{78, 90}}, "a", ty("Outer"), core::AddressSpace::kUniform, Group(0_a),
              Binding(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(34:56 error: 'uniform' storage requires that array elements are aligned to 16 bytes, but array element of type 'vec2<f32>' has a stride of 8 bytes. Consider using a vec4 instead.
12:34 note: see layout of struct:
/*            align(8) size(88) */ struct Outer {
/* offset( 0) align(8) size(80) */   inner : array<vec2<f32>, 10>,
/* offset(80) align(4) size( 4) */   scalar : i32,
/* offset(84) align(1) size( 4) */   // -- implicit struct size padding --
/*                              */ };
78:90 note: 'Outer' used in address space 'uniform' here)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, UniformBuffer_InvalidArrayStride_Struct) {
    // struct ArrayElem {
    //   a : f32;
    //   b : i32;
    // }
    // type Inner = array<ArrayElem, 10u>;
    //
    // struct Outer {
    //   inner : Inner;
    //   scalar : i32;
    // };
    //
    // @group(0) @binding(0)
    // var<uniform> a : Outer;

    auto* array_elem = Structure("ArrayElem", Vector{
                                                  Member("a", ty.f32()),
                                                  Member("b", ty.i32()),
                                              });
    Alias("Inner", ty.array(ty.Of(array_elem), 10_u));

    Structure(Ident(Source{{12, 34}}, "Outer"), Vector{
                                                    Member("inner", ty(Source{{34, 56}}, "Inner")),
                                                    Member("scalar", ty.i32()),
                                                });

    GlobalVar(Source{{78, 90}}, "a", ty("Outer"), core::AddressSpace::kUniform, Group(0_a),
              Binding(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(34:56 error: 'uniform' storage requires that array elements are aligned to 16 bytes, but array element of type 'ArrayElem' has a stride of 8 bytes. Consider using the '@size' attribute on the last struct member.
12:34 note: see layout of struct:
/*            align(4) size(84) */ struct Outer {
/* offset( 0) align(4) size(80) */   inner : array<ArrayElem, 10>,
/* offset(80) align(4) size( 4) */   scalar : i32,
/*                              */ };
78:90 note: 'Outer' used in address space 'uniform' here)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, UniformBuffer_InvalidArrayStride_TopLevelArray) {
    // @group(0) @binding(0)
    // var<uniform> a : array<f32, 4u>;
    GlobalVar("a", ty.array(ty.f32(), 4_u), core::AddressSpace::kUniform, Group(0_a), Binding(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: 'uniform' storage requires that array elements are aligned to 16 bytes, but array element of type 'f32' has a stride of 4 bytes. Consider using a vector or struct as the element type instead.)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, UniformBuffer_InvalidArrayStride_NestedArray) {
    // struct Outer {
    //   inner : array<array<f32, 4u>, 4u>
    // };
    //
    // @group(0) @binding(0)
    // var<uniform> a : array<Outer, 4u>;

    Structure(Ident(Source{{12, 34}}, "Outer"),
              Vector{
                  Member("inner", ty.array(Source{{34, 56}}, ty.array<f32, 4>(), 4_u)),
              });

    GlobalVar(Source{{78, 90}}, "a", ty("Outer"), core::AddressSpace::kUniform, Group(0_a),
              Binding(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(34:56 error: 'uniform' storage requires that array elements are aligned to 16 bytes, but array element of type 'f32' has a stride of 4 bytes. Consider using a vector or struct as the element type instead.
12:34 note: see layout of struct:
/*            align(4) size(64) */ struct Outer {
/* offset( 0) align(4) size(64) */   inner : array<array<f32, 4>, 4>,
/*                              */ };
78:90 note: 'Outer' used in address space 'uniform' here)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, UniformBuffer_InvalidArrayStride_SuggestedFix) {
    // type Inner = @stride(16) array<f32, 10u>;
    //
    // struct Outer {
    //   inner : Inner;
    //   scalar : i32;
    // };
    //
    // @group(0) @binding(0)
    // var<uniform> a : Outer;

    Alias("Inner", ty.array<f32, 10>(Vector{Stride(16)}));

    Structure("Outer", Vector{
                           Member("inner", ty("Inner")),
                           Member("scalar", ty.i32()),
                       });

    GlobalVar("a", ty("Outer"), core::AddressSpace::kUniform, Group(0_a), Binding(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

// Detect unaligned member for immediate data buffers
TEST_F(ResolverAddressSpaceLayoutValidationTest, Immediate_UnalignedMember) {
    // enable chromium_experimental_immediate;
    // struct S {
    //     @size(5) a : f32;
    //     @align(1) b : f32;
    // };
    // var<immediate> a : S;
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    Structure(Ident(Source{{12, 34}}, "S"),
              Vector{Member("a", ty.f32(), Vector{MemberSize(5_a)}),
                     Member(Source{{34, 56}}, "b", ty.f32(), Vector{MemberAlign(1_i)})});
    GlobalVar(Source{{78, 90}}, "a", ty("S"), core::AddressSpace::kImmediate);

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
    // enable chromium_experimental_immediate;
    // struct S {
    //     @size(5) a : f32;
    //     @align(4) b : f32;
    // };
    // var<immediate> a : S;
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    Structure("S", Vector{Member("a", ty.f32(), Vector{MemberSize(5_a)}),
                          Member("b", ty.f32(), Vector{MemberAlign(4_i)})});
    GlobalVar("a", ty("S"), core::AddressSpace::kImmediate);

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

    GlobalVar(Source{{56, 78}}, "a", ty("S"), core::AddressSpace::kStorage,
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

    GlobalVar(Source{{56, 78}}, "a", ty("S"), core::AddressSpace::kWorkgroup, Group(0_a));

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

    GlobalVar(Source{{56, 78}}, "a", ty("S"), core::AddressSpace::kPrivate, Group(0_a));

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

    GlobalVar(Source{{56, 78}}, "a", ty("S"), core::AddressSpace::kFunction, Group(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: alignment must be a multiple of '16' bytes for the 'function' address space
56:78 note: 'S' used in address space 'function' here)");
}

}  // namespace
}  // namespace tint::resolver
