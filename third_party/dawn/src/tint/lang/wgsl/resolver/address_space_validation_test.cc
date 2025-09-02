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
#include "src/tint/lang/wgsl/sem/struct.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ::testing::HasSubstr;

using ResolverAddressSpaceValidationTest = ResolverTest;

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_NoAddressSpace_Fail) {
    // var g : f32;
    GlobalVar(Source{{12, 34}}, "g", ty.f32());

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: module-scope 'var' declarations that are not of texture or sampler types must provide an address space)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_NoAddressSpace_Fail) {
    // type g = ptr<f32>;
    Alias("g", ty(Source{{12, 34}}, "ptr", ty.f32()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: 'ptr' requires at least 2 template arguments)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_FunctionAddressSpace_Fail) {
    // var<private> g : f32;
    GlobalVar(Source{{12, 34}}, "g", ty.f32(), core::AddressSpace::kFunction);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: module-scope 'var' must not use address space 'function'");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Private_RuntimeArray) {
    // var<private> v : array<i32>;
    GlobalVar(Source{{56, 78}}, "v", ty.array(Source{{12, 34}}, ty.i32()),
              core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
56:78 note: while instantiating 'var' v)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Private_RuntimeArray) {
    // type t : ptr<private, array<i32>>;
    Alias("t", ty.ptr<private_>(Source{{56, 78}}, ty.array(Source{{12, 34}}, ty.i32())));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
56:78 note: while instantiating ptr<private, array<i32>, read_write>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Private_RuntimeArrayInStruct) {
    // struct S { m : array<i32> };
    // var<private> v : S;
    Structure("S", Vector{Member(Source{{12, 34}}, "m", ty.array(ty.i32()))});
    GlobalVar(Source{{56, 78}}, "v", ty("S"), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: runtime-sized arrays can only be used in the <storage> address space
12:34 note: while analyzing structure member S.m
56:78 note: while instantiating 'var' v)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Private_RuntimeArrayInStruct) {
    // struct S { m : array<i32> };
    // type t = ptr<private, S>;
    Structure("S", Vector{Member(Source{{12, 34}}, "m", ty.array(ty.i32()))});
    Alias("t", ty.ptr<private_>(ty("S")));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: runtime-sized arrays can only be used in the <storage> address space
12:34 note: while analyzing structure member S.m
note: while instantiating ptr<private, S, read_write>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Workgroup_RuntimeArray) {
    // var<workgroup> v : array<i32>;
    GlobalVar(Source{{56, 78}}, "v", ty.array(Source{{12, 34}}, ty.i32()),
              core::AddressSpace::kWorkgroup);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
56:78 note: while instantiating 'var' v)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Workgroup_RuntimeArray) {
    // type t = ptr<workgroup, array<i32>>;
    Alias("t", ty.ptr<workgroup>(ty.array(Source{{12, 34}}, ty.i32())));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
note: while instantiating ptr<workgroup, array<i32>, read_write>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Workgroup_RuntimeArrayInStruct) {
    // struct S { m : array<i32> };
    // var<workgroup> v : S;
    Structure("S", Vector{Member(Source{{12, 34}}, "m", ty.array(ty.i32()))});
    GlobalVar(Source{{56, 78}}, "v", ty("S"), core::AddressSpace::kWorkgroup);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: runtime-sized arrays can only be used in the <storage> address space
12:34 note: while analyzing structure member S.m
56:78 note: while instantiating 'var' v)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Workgroup_RuntimeArrayInStruct) {
    // struct S { m : array<i32> };
    // type t = ptr<workgroup, S>;
    Structure("S", Vector{Member(Source{{12, 34}}, "m", ty.array(ty.i32()))});
    Alias(Source{{56, 78}}, "t", ty.ptr<workgroup>(ty("S")));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: runtime-sized arrays can only be used in the <storage> address space
12:34 note: while analyzing structure member S.m
note: while instantiating ptr<workgroup, S, read_write>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_Bool) {
    // var<storage> g : bool;
    GlobalVar(Source{{56, 78}}, "g", ty.bool_(Source{{12, 34}}), core::AddressSpace::kStorage,
              Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_Bool) {
    // type t = ptr<storage, bool>;
    Alias(Source{{56, 78}}, "t", ty.ptr<storage>(ty.bool_(Source{{12, 34}})));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
note: while instantiating ptr<storage, bool, read>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_BoolAlias) {
    // type a = bool;
    // @binding(0) @group(0) var<storage, read> g : a;
    Alias("a", ty.bool_());
    GlobalVar(Source{{56, 78}}, "g", ty(Source{{12, 34}}, "a"), core::AddressSpace::kStorage,
              Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_BoolAlias) {
    // type a = bool;
    // type t = ptr<storage, a>;
    Alias("a", ty.bool_());
    Alias(Source{{56, 78}}, "t", ty.ptr<storage>(ty(Source{{12, 34}}, "a")));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
note: while instantiating ptr<storage, bool, read>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_Pointer) {
    // var<storage> g : ptr<private, f32>;
    GlobalVar(Source{{56, 78}}, "g", ty.ptr<private_, f32>(Source{{12, 34}}),
              core::AddressSpace::kStorage, Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: type 'ptr<private, f32, read_write>' cannot be used in address space 'storage' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_IntScalar) {
    // var<storage> g : i32;
    GlobalVar("g", ty.i32(), core::AddressSpace::kStorage, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_IntScalar) {
    // type t = ptr<storage, i32;
    Alias("t", ty.ptr<storage, i32>());

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_F16) {
    // enable f16;
    // var<storage> g : f16;
    Enable(wgsl::Extension::kF16);

    GlobalVar("g", ty.f16(), core::AddressSpace::kStorage, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_F16) {
    // enable f16;
    // type t = ptr<storage, f16>;
    Enable(wgsl::Extension::kF16);

    Alias("t", ty.ptr<storage, f16>());

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_F16Alias) {
    // enable f16;
    // type a = f16;
    // var<storage, read> g : a;
    Enable(wgsl::Extension::kF16);

    Alias("a", ty.f16());
    GlobalVar("g", ty("a"), core::AddressSpace::kStorage, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_F16Alias) {
    // enable f16;
    // type a = f16;
    // type t = ptr<storage, a>;
    Enable(wgsl::Extension::kF16);

    Alias("a", ty.f16());
    Alias("t", ty.ptr<storage>(ty("a")));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_VectorF32) {
    // var<storage> g : vec4<f32>;
    GlobalVar("g", ty.vec4<f32>(), core::AddressSpace::kStorage, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_VectorF32) {
    // type t = ptr<storage, vec4<f32>>;
    Alias("t", ty.ptr<storage, vec4<f32>>());

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_VectorF16) {
    // var<storage> g : vec4<f16>;
    Enable(wgsl::Extension::kF16);
    GlobalVar("g", ty.vec(ty.f16(), 4u), core::AddressSpace::kStorage, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_VectorF16) {
    // type t = ptr<storage, vec4<f16>>;
    Enable(wgsl::Extension::kF16);
    Alias("t", ty.ptr<storage, vec4<f32>>());

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_ArrayF32) {
    // struct S{ a : f32 };
    // var<storage, read> g : array<S, 3u>;
    Structure("S", Vector{Member("a", ty.f32())});
    GlobalVar("g", ty.array(ty("S"), 3_u), core::AddressSpace::kStorage, core::Access::kRead,
              Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_ArrayF32) {
    // struct S{ a : f32 };
    // type t = ptr<storage, array<S, 3u>>;
    Structure("S", Vector{Member("a", ty.f32())});
    Alias("t", ty.ptr<storage>(ty.array(ty("S"), 3_u)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_ArrayF16) {
    // enable f16;
    // struct S{ a : f16 };
    // var<storage, read> g : array<S, 3u>;
    Enable(wgsl::Extension::kF16);

    Structure("S", Vector{Member("a", ty.f16())});
    GlobalVar("g", ty.array(ty("S"), 3_u), core::AddressSpace::kStorage, core::Access::kRead,
              Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_ArrayF16) {
    // enable f16;
    // struct S{ a : f16 };
    // type t = ptr<storage, array<S, 3u>, read>;
    Enable(wgsl::Extension::kF16);

    Structure("S", Vector{Member("a", ty.f16())});
    Alias("t", ty.ptr<storage, read>(ty.array(ty("S"), 3_u)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_StructI32) {
    // struct S { x : i32 };
    // var<storage, read> g : S;
    Structure("S", Vector{Member("x", ty.i32())});
    GlobalVar("g", ty("S"), core::AddressSpace::kStorage, core::Access::kRead, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_StructI32) {
    // struct S { x : i32 };
    // type t = ptr<storage, S, read>;
    Structure("S", Vector{Member("x", ty.i32())});
    Alias("t", ty.ptr<storage, read>(ty("S")));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_StructI32Aliases) {
    // struct S { x : i32 };
    // type a1 = S;
    // var<storage, read> g : a1;
    Structure("S", Vector{Member("x", ty.i32())});
    Alias("a1", ty("S"));
    Alias("a2", ty("a1"));
    GlobalVar("g", ty("a2"), core::AddressSpace::kStorage, core::Access::kRead, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_StructI32Aliases) {
    // struct S { x : i32 };
    // type a1 = S;
    // type t = ptr<storage, a1, read>;
    Structure("S", Vector{Member("x", ty.i32())});
    Alias("a1", ty("S"));
    Alias("a2", ty("a1"));
    Alias("t", ty.ptr<storage, read>(ty("a2")));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_StructF16) {
    // struct S { x : f16 };
    // var<storage, read> g : S;
    Enable(wgsl::Extension::kF16);

    Structure("S", Vector{Member("x", ty.f16())});
    GlobalVar("g", ty("S"), core::AddressSpace::kStorage, core::Access::kRead, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_StructF16) {
    // struct S { x : f16 };
    // type t = ptr<storage, S, read>;
    Enable(wgsl::Extension::kF16);

    Structure("S", Vector{Member("x", ty.f16())});
    Alias("t", ty.ptr<storage, read>(ty("S")));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_StructF16Aliases) {
    // struct S { x : f16 };
    // type a1 = S;
    // var<storage, read> g : a1;
    Enable(wgsl::Extension::kF16);

    Structure("S", Vector{Member("x", ty.f16())});
    Alias("a1", ty("S"));
    Alias("a2", ty("a1"));
    GlobalVar("g", ty("a2"), core::AddressSpace::kStorage, core::Access::kRead, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_StructF16Aliases) {
    // struct S { x : f16 };
    // type a1 = S;
    // type t = ptr<storage, a1, read>;
    Enable(wgsl::Extension::kF16);

    Structure("S", Vector{Member("x", ty.f16())});
    Alias("a1", ty("S"));
    Alias("a2", ty("a1"));
    Alias("g", ty.ptr<storage, read>(ty("a2")));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_NotStorage_AccessMode) {
    // var<private, read> g : a;
    GlobalVar(Source{{12, 34}}, "g", ty.i32(), core::AddressSpace::kPrivate, core::Access::kRead);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: only variables in <storage> address space may specify an access mode)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_NotStorage_AccessMode) {
    // type t = ptr<private, i32, read>;
    Alias("t", ty.ptr<private_, i32, read>(Source{{12, 34}}));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: only pointers in <storage> address space may specify an access mode)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_ReadAccessMode) {
    // @group(0) @binding(0) var<storage, read> a : i32;
    GlobalVar("a", ty.i32(), core::AddressSpace::kStorage, core::Access::kRead, Group(0_a),
              Binding(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_ReadAccessMode) {
    // type t = ptr<storage, i32, read>;
    Alias("t", ty.ptr<storage, i32, read>());

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_ReadWriteAccessMode) {
    // @group(0) @binding(0) var<storage, read_write> a : i32;
    GlobalVar("a", ty.i32(), core::AddressSpace::kStorage, core::Access::kReadWrite, Group(0_a),
              Binding(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_ReadWriteAccessMode) {
    // type t = ptr<storage, i32, read_write>;
    Alias("t", ty.ptr<storage, i32, read_write>());

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_WriteAccessMode) {
    // @group(0) @binding(0) var<storage, read_write> a : i32;
    GlobalVar(Source{{12, 34}}, "a", ty.i32(), core::AddressSpace::kStorage, core::Access::kWrite,
              Group(0_a), Binding(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(12:34 error: access mode 'write' is not valid for the 'storage' address space)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_WriteAccessMode) {
    // type t = ptr<storage, i32, write>;
    Alias("t", ty.ptr<storage, i32, write>(Source{{12, 34}}));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(12:34 error: access mode 'write' is not valid for the 'storage' address space)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBuffer_Struct_Runtime) {
    // struct S { m: array<f32>; };
    // @group(0) @binding(0) var<uniform> svar : S;

    Structure("S", Vector{Member(Source{{56, 78}}, "m", ty.array(Source{{12, 34}}, ty.i32()))});

    GlobalVar(Source{{90, 12}}, "svar", ty("S"), core::AddressSpace::kUniform, Binding(0_a),
              Group(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
56:78 note: while analyzing structure member S.m
90:12 note: while instantiating 'var' svar)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBuffer_Struct_Runtime) {
    // struct S { m: array<f32>; };
    // type t = ptr<uniform, S>;

    Structure("S", Vector{Member(Source{{56, 78}}, "m", ty.array(Source{{12, 34}}, ty.i32()))});

    Alias("t", ty.ptr<uniform>(Source{{90, 12}}, ty("S")));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: 'uniform' storage requires that array elements are aligned to 16 bytes, but array element of type 'i32' has a stride of 4 bytes. Consider using a vector or struct as the element type instead.
note: see layout of struct:
/*           align(4) size(4) */ struct S {
/* offset(0) align(4) size(4) */   m : array<i32>,
/*                            */ };
90:12 note: 'S' used in address space 'uniform' here)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferBool) {
    // var<uniform> g : bool;
    GlobalVar(Source{{56, 78}}, "g", ty.bool_(Source{{12, 34}}), core::AddressSpace::kUniform,
              Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: type 'bool' cannot be used in address space 'uniform' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferBool) {
    // type t = ptr<uniform, bool>;
    Alias("t", ty.ptr<uniform>(Source{{56, 78}}, ty.bool_(Source{{12, 34}})));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: type 'bool' cannot be used in address space 'uniform' as it is non-host-shareable
56:78 note: while instantiating ptr<uniform, bool, read>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferBoolAlias) {
    // type a = bool;
    // var<uniform> g : a;
    Alias("a", ty.bool_());
    GlobalVar(Source{{56, 78}}, "g", ty(Source{{12, 34}}, "a"), core::AddressSpace::kUniform,
              Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: type 'bool' cannot be used in address space 'uniform' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferBoolAlias) {
    // type a = bool;
    // type t = ptr<uniform, a>;
    Alias("a", ty.bool_());
    Alias("t", ty.ptr<uniform>(Source{{56, 78}}, ty(Source{{12, 34}}, "a")));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: type 'bool' cannot be used in address space 'uniform' as it is non-host-shareable
56:78 note: while instantiating ptr<uniform, bool, read>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformPointer) {
    // var<uniform> g : ptr<private, f32>;
    GlobalVar(Source{{56, 78}}, "g", ty.ptr<private_, f32>(Source{{12, 34}}),
              core::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: type 'ptr<private, f32, read_write>' cannot be used in address space 'uniform' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferIntScalar) {
    // var<uniform> g : i32;
    GlobalVar(Source{{56, 78}}, "g", ty.i32(), core::AddressSpace::kUniform, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferIntScalar) {
    // type t = ptr<uniform, i32>;
    Alias("t", ty.ptr<uniform, i32>());

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferF16) {
    // enable f16;
    // var<uniform> g : f16;
    Enable(wgsl::Extension::kF16);

    GlobalVar("g", ty.f16(), core::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferF16) {
    // enable f16;
    // type t = ptr<uniform, f16>;
    Enable(wgsl::Extension::kF16);

    Alias("t", ty.ptr<uniform, f16>());

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferVectorF32) {
    // var<uniform> g : vec4<f32>;
    GlobalVar("g", ty.vec4<f32>(), core::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferVectorF32) {
    // type t = ptr<uniform, vec4<f32>>;
    Alias("t", ty.ptr<uniform, vec4<f32>>());

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferVectorF16) {
    // enable f16;
    // var<uniform> g : vec4<f16>;
    Enable(wgsl::Extension::kF16);

    GlobalVar("g", ty.vec4<f16>(), core::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferVectorF16) {
    // enable f16;
    // type t = ptr<uniform, vec4<f16>>;
    Enable(wgsl::Extension::kF16);

    Alias("t", ty.ptr<uniform, f16>());

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferArrayF32) {
    // struct S {
    //   @size(16) f : f32;
    // }
    // var<uniform> g : array<S, 3u>;
    Structure("S", Vector{Member("a", ty.f32(), Vector{MemberSize(16_a)})});
    GlobalVar("g", ty.array(ty("S"), 3_u), core::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferArrayF32) {
    // struct S {
    //   @size(16) f : f32;
    // }
    // type t = ptr<uniform, array<S, 3u>>;
    Structure("S", Vector{Member("a", ty.f32(), Vector{MemberSize(16_a)})});
    Alias("t", ty.ptr<uniform>(ty.array(ty("S"), 3_u)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferArrayF16) {
    // enable f16;
    // struct S {
    //   @size(16) f : f16;
    // }
    // var<uniform> g : array<S, 3u>;
    Enable(wgsl::Extension::kF16);

    Structure("S", Vector{Member("a", ty.f16(), Vector{MemberSize(16_a)})});
    GlobalVar("g", ty.array(ty("S"), 3_u), core::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferArrayF16) {
    // enable f16;
    // struct S {
    //   @size(16) f : f16;
    // }
    // type t = ptr<uniform, array<S, 3u>>;
    Enable(wgsl::Extension::kF16);

    Structure("S", Vector{Member("a", ty.f16(), Vector{MemberSize(16_a)})});
    Alias("t", ty.ptr<uniform>(ty.array(ty("S"), 3_u)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferStructI32) {
    // struct S { x : i32 };
    // var<uniform> g : S;
    Structure("S", Vector{Member("x", ty.i32())});
    GlobalVar("g", ty("S"), core::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferStructI32) {
    // struct S { x : i32 };
    // type t = ptr<uniform, S>;
    Structure("S", Vector{Member("x", ty.i32())});
    Alias("t", ty.ptr<uniform>(ty("S")));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferStructI32Aliases) {
    // struct S { x : i32 };
    // type a1 = S;
    // var<uniform> g : a1;
    Structure("S", Vector{Member("x", ty.i32())});
    Alias("a1", ty("S"));
    GlobalVar("g", ty("a1"), core::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferStructI32Aliases) {
    // struct S { x : i32 };
    // type a1 = S;
    // type t = ptr<uniform, a1>;
    Structure("S", Vector{Member("x", ty.i32())});
    Alias("a1", ty("S"));
    Alias("t", ty.ptr<uniform>(ty("a1")));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferStructF16) {
    // enable f16;
    // struct S { x : f16 };
    // var<uniform> g : S;
    Enable(wgsl::Extension::kF16);

    Structure("S", Vector{Member("x", ty.f16())});
    GlobalVar("g", ty("S"), core::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferStructF16) {
    // enable f16;
    // struct S { x : f16 };
    // type t = ptr<uniform, S>;
    Enable(wgsl::Extension::kF16);

    Structure("S", Vector{Member("x", ty.f16())});
    Alias("t", ty.ptr<uniform>(ty("S")));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferStructF16Aliases) {
    // enable f16;
    // struct S { x : f16 };
    // type a1 = S;
    // var<uniform> g : a1;
    Enable(wgsl::Extension::kF16);

    Structure("S", Vector{Member("x", ty.f16())});
    Alias("a1", ty("S"));
    GlobalVar("g", ty("a1"), core::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferStructF16Aliases) {
    // enable f16;
    // struct S { x : f16 };
    // type a1 = S;
    // type t = ptr<uniform, a1>;
    Enable(wgsl::Extension::kF16);

    Structure("S", Vector{Member("x", ty.f16())});
    Alias("a1", ty("S"));
    Alias("t", ty.ptr<uniform>(ty("a1")));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_ImmediateBool) {
    // enable chromium_experimental_immediate;
    // var<immediate> g : bool;
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    GlobalVar(Source{{56, 78}}, "g", ty.bool_(Source{{12, 34}}), core::AddressSpace::kImmediate);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: type 'bool' cannot be used in address space 'immediate' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_ImmediateBool) {
    // enable chromium_experimental_immediate;
    // type t = ptr<immediate, bool>;
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    Alias(Source{{56, 78}}, "t", ty.ptr<immediate>(ty.bool_(Source{{12, 34}})));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: type 'bool' cannot be used in address space 'immediate' as it is non-host-shareable
note: while instantiating ptr<immediate, bool, read>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_ImmediateF16) {
    // enable f16;
    // enable chromium_experimental_immediate;
    // var<immediate> g : f16;
    Enable(wgsl::Extension::kF16);
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    GlobalVar("g", ty.f16(Source{{56, 78}}), core::AddressSpace::kImmediate);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error: using 'f16' in 'immediate' address space is not implemented yet");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_ImmediateF16) {
    // enable f16;
    // enable chromium_experimental_immediate;
    // type t = ptr<immediate, f16>;
    Enable(wgsl::Extension::kF16);
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    Alias("t", ty.ptr<immediate>(ty.f16(Source{{56, 78}})));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error: using 'f16' in 'immediate' address space is not implemented yet");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_ImmediatePointer) {
    // enable chromium_experimental_immediate;
    // var<immediate> g : ptr<private, f32>;
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    GlobalVar(Source{{56, 78}}, "g", ty.ptr<private_, f32>(Source{{12, 34}}),
              core::AddressSpace::kImmediate);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: type 'ptr<private, f32, read_write>' cannot be used in address space 'immediate' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_ImmediateIntScalar) {
    // enable chromium_experimental_immediate;
    // var<immediate> g : i32;
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    GlobalVar("g", ty.i32(), core::AddressSpace::kImmediate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_ImmediateIntScalar) {
    // enable chromium_experimental_immediate;
    // type t = ptr<immediate, i32>;
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    Alias("t", ty.ptr<immediate, i32>());

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_ImmediateVectorF32) {
    // enable chromium_experimental_immediate;
    // var<immediate> g : vec4<f32>;
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    GlobalVar("g", ty.vec4<f32>(), core::AddressSpace::kImmediate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_ImmediateVectorF32) {
    // enable chromium_experimental_immediate;
    // var<immediate> g : vec4<f32>;
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    Alias("t", ty.ptr<immediate, vec4<f32>>());

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_ImmediateArrayF32) {
    // enable chromium_experimental_immediate;
    // struct S { a : f32}
    // var<immediate> g : array<S, 3u>;
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    Structure("S", Vector{Member("a", ty.f32())});
    GlobalVar("g", ty.array(ty("S"), 3_u), core::AddressSpace::kImmediate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_ImmediateArrayF32) {
    // enable chromium_experimental_immediate;
    // struct S { a : f32}
    // type t = ptr<immediate, array<S, 3u>>;
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    Structure("S", Vector{Member("a", ty.f32())});
    Alias("t", ty.ptr<immediate>(ty.array(ty("S"), 3_u)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace tint::resolver
