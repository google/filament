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

#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ResolverAddressSpaceValidationTest = ResolverTest;

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_NoAddressSpace_Fail) {
    EXPECT_ERROR("var g : f32;",
                 R"(
input.wgsl:1:1 error: module-scope 'var' declarations that are not of texture or sampler types must provide an address space
var g : f32;
^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_NoAddressSpace_Fail) {
    EXPECT_ERROR("alias g = ptr<f32>;",
                 R"(
input.wgsl:1:11 error: 'ptr' requires at least 2 template arguments
alias g = ptr<f32>;
          ^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_FunctionAddressSpace_Fail) {
    EXPECT_ERROR("var<function> g : f32;",
                 R"(
input.wgsl:1:1 error: module-scope 'var' must not use address space 'function'
var<function> g : f32;
^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Private_RuntimeArray) {
    EXPECT_ERROR("var<private> v : array<i32>;", R"(
input.wgsl:1:18 error: runtime-sized arrays cannot be used in the <private> address space
var<private> v : array<i32>;
                 ^^^^^^^^^^

input.wgsl:1:1 note: while instantiating 'var' v
var<private> v : array<i32>;
^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Private_RuntimeArray) {
    EXPECT_ERROR("alias t = ptr<private, array<i32>>;", R"(
input.wgsl:1:24 error: runtime-sized arrays cannot be used in the <private> address space
alias t = ptr<private, array<i32>>;
                       ^^^^^^^^^^

input.wgsl:1:11 note: while instantiating ptr<private, array<i32>, read_write>
alias t = ptr<private, array<i32>>;
          ^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Private_RuntimeArrayInStruct) {
    EXPECT_ERROR(R"(
struct S { m : array<i32> };
var<private> v : S;
)",
                 R"(
input.wgsl:2:16 error: runtime-sized arrays cannot be used in the <private> address space
struct S { m : array<i32> };
               ^^^^^^^^^^

input.wgsl:2:12 note: while analyzing structure member S.m
struct S { m : array<i32> };
           ^

input.wgsl:3:1 note: while instantiating 'var' v
var<private> v : S;
^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Private_RuntimeArrayInStruct) {
    EXPECT_ERROR(R"(
struct S { m : array<i32> };
alias t = ptr<private, S>;
)",
                 R"(
input.wgsl:2:16 error: runtime-sized arrays cannot be used in the <private> address space
struct S { m : array<i32> };
               ^^^^^^^^^^

input.wgsl:2:12 note: while analyzing structure member S.m
struct S { m : array<i32> };
           ^

input.wgsl:3:11 note: while instantiating ptr<private, S, read_write>
alias t = ptr<private, S>;
          ^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Workgroup_RuntimeArray) {
    EXPECT_ERROR(R"(
var<workgroup> v : array<i32>;
)",
                 R"(
input.wgsl:2:1 error: variables in 'workgroup' address space must have a fixed footprint
var<workgroup> v : array<i32>;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Workgroup_RuntimeArray) {
    EXPECT_SUCCESS(R"(
alias t = ptr<workgroup, array<i32>>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Workgroup_RuntimeArrayInStruct) {
    EXPECT_ERROR(R"(
struct S { m : array<i32> };
var<workgroup> v : S;
)",
                 R"(
input.wgsl:3:1 error: variables in 'workgroup' address space must have a fixed footprint
var<workgroup> v : S;
^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Workgroup_RuntimeArrayInStruct) {
    EXPECT_SUCCESS(R"(
struct S { m : array<i32> };
alias t = ptr<workgroup, S>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_Bool) {
    EXPECT_ERROR("@group(0) @binding(0) var<storage> g : bool;", R"(
input.wgsl:1:40 error: type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
@group(0) @binding(0) var<storage> g : bool;
                                       ^^^^

input.wgsl:1:23 note: while instantiating 'var' g
@group(0) @binding(0) var<storage> g : bool;
                      ^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_Bool) {
    EXPECT_ERROR("alias t = ptr<storage, bool>;", R"(
input.wgsl:1:24 error: type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
alias t = ptr<storage, bool>;
                       ^^^^

input.wgsl:1:11 note: while instantiating ptr<storage, bool, read>
alias t = ptr<storage, bool>;
          ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_BoolAlias) {
    EXPECT_ERROR(R"(
alias a = bool;
@binding(0) @group(0) var<storage, read> g : a;
)",
                 R"(
input.wgsl:3:46 error: type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
@binding(0) @group(0) var<storage, read> g : a;
                                             ^

input.wgsl:3:23 note: while instantiating 'var' g
@binding(0) @group(0) var<storage, read> g : a;
                      ^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_BoolAlias) {
    EXPECT_ERROR(R"(
alias a = bool;
alias t = ptr<storage, a>;
)",
                 R"(
input.wgsl:3:24 error: type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
alias t = ptr<storage, a>;
                       ^

input.wgsl:3:11 note: while instantiating ptr<storage, bool, read>
alias t = ptr<storage, a>;
          ^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_Pointer) {
    EXPECT_ERROR(R"(
@group(0) @binding(0) var<storage> g : ptr<private, f32>;
)",
                 R"(
input.wgsl:2:40 error: type 'ptr<private, f32, read_write>' cannot be used in address space 'storage' as it is non-host-shareable
@group(0) @binding(0) var<storage> g : ptr<private, f32>;
                                       ^^^^^^^^^^^^^^^^^

input.wgsl:2:23 note: while instantiating 'var' g
@group(0) @binding(0) var<storage> g : ptr<private, f32>;
                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_IntScalar) {
    EXPECT_SUCCESS("@group(0) @binding(0) var<storage> g : i32;");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_IntScalar) {
    EXPECT_SUCCESS("alias t = ptr<storage, i32>;");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_F16) {
    EXPECT_SUCCESS(R"(
enable f16;
@group(0) @binding(0) var<storage> g : f16;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_F16) {
    EXPECT_SUCCESS(R"(
enable f16;
alias t = ptr<storage, f16>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_F16Alias) {
    EXPECT_SUCCESS(R"(
enable f16;
alias a = f16;
@group(0) @binding(0) var<storage, read> g : a;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_F16Alias) {
    EXPECT_SUCCESS(R"(
enable f16;
alias a = f16;
alias t = ptr<storage, a>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_VectorF32) {
    EXPECT_SUCCESS("@group(0) @binding(0) var<storage> g : vec4<f32>;");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_VectorF32) {
    EXPECT_SUCCESS("alias t = ptr<storage, vec4<f32>>;");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_VectorF16) {
    EXPECT_SUCCESS(R"(
enable f16;
@group(0) @binding(0) var<storage> g : vec4<f16>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_VectorF16) {
    EXPECT_SUCCESS(R"(
enable f16;
alias t = ptr<storage, vec4<f16>>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_ArrayF32) {
    EXPECT_SUCCESS(R"(
struct S{ a : f32 };
@group(0) @binding(0) var<storage, read> g : array<S, 3u>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_ArrayF32) {
    EXPECT_SUCCESS(R"(
struct S{ a : f32 };
alias t = ptr<storage, array<S, 3u>>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_ArrayF16) {
    EXPECT_SUCCESS(R"(
enable f16;

struct S {
  a : f16,
};
@group(0) @binding(0) var<storage, read> g : array<S, 3u>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_ArrayF16) {
    EXPECT_SUCCESS(R"(
enable f16;

struct S {
  a : f16
};
alias t = ptr<storage, array<S, 3u>, read>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_StructI32) {
    EXPECT_SUCCESS(R"(
struct S {
  x : i32,
};
@group(0) @binding(0) var<storage, read> g : S;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_StructI32) {
    EXPECT_SUCCESS(R"(
struct S {
  x: i32,
};
alias t = ptr<storage, S, read_write>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_StructI32Aliases) {
    EXPECT_SUCCESS(R"(
struct S {
  x : i32
};
alias a1 = S;
alias a2 = a1;
@group(0) @binding(0) var<storage, read> g : a2;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_StructI32Aliases) {
    EXPECT_SUCCESS(R"(
struct S {
  x : i32
};
alias a1 = S;
alias a2 = a1;
alias t = ptr<storage, a2, read>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_StructF16) {
    EXPECT_SUCCESS(R"(
enable f16;

struct S {
  x : f16
};
@group(0) @binding(0) var<storage, read> g : S;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_StructF16) {
    EXPECT_SUCCESS(R"(
enable f16;

struct S {
  x : f16
};
alias t = ptr<storage, S, read>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_StructF16Aliases) {
    EXPECT_SUCCESS(R"(
enable f16;

struct S {
  x : f16
};
alias a1 = S;
alias a2 = a1;
@group(0) @binding(0) var<storage, read> g : a2;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_StructF16Aliases) {
    EXPECT_SUCCESS(R"(
enable f16;

struct S {
  x : f16
};
alias a1 = S;
alias a2 = a1;
alias t = ptr<storage, a2, read>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_NotStorage_AccessMode) {
    EXPECT_ERROR(R"(
var<private, read> g : i32;
)",
                 R"(
input.wgsl:2:1 error: only variables in <storage> address space may specify an access mode
var<private, read> g : i32;
^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_NotStorage_AccessMode) {
    EXPECT_ERROR(R"(
alias t = ptr<private, i32, read>;
)",
                 R"(
input.wgsl:2:11 error: only pointers in <storage> address space may specify an access mode
alias t = ptr<private, i32, read>;
          ^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_ReadAccessMode) {
    EXPECT_SUCCESS("@group(0) @binding(0) var<storage, read> a : i32;");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_ReadAccessMode) {
    EXPECT_SUCCESS("alias t = ptr<storage, i32, read>;");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_ReadWriteAccessMode) {
    EXPECT_SUCCESS("@group(0) @binding(0) var<storage, read_write> a : i32;");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_ReadWriteAccessMode) {
    EXPECT_SUCCESS("alias t = ptr<storage, i32, read_write>;");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_WriteAccessMode) {
    EXPECT_ERROR(R"(
@group(0) @binding(0) var<storage, write> a : i32;
)",
                 R"(
input.wgsl:2:23 error: access mode 'write' is not valid for the 'storage' address space
@group(0) @binding(0) var<storage, write> a : i32;
                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_WriteAccessMode) {
    EXPECT_ERROR(R"(
alias t = ptr<storage, i32, write>;
)",
                 R"(
input.wgsl:2:11 error: access mode 'write' is not valid for the 'storage' address space
alias t = ptr<storage, i32, write>;
          ^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBuffer_Struct_Runtime) {
    EXPECT_ERROR(R"(
struct S {
  m: array<i32>,
};
@group(0) @binding(0) var<uniform> svar : S;
)",
                 R"(
input.wgsl:5:23 error: variables in 'uniform' address space must have a fixed footprint
@group(0) @binding(0) var<uniform> svar : S;
                      ^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBuffer_Struct_Runtime) {
    EXPECT_SUCCESS(R"(
struct S {
  m: array<i32>,
};
alias t = ptr<uniform, S>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferBool) {
    EXPECT_ERROR(R"(
@group(0) @binding(0) var<uniform> g : bool;
)",
                 R"(
input.wgsl:2:40 error: type 'bool' cannot be used in address space 'uniform' as it is non-host-shareable
@group(0) @binding(0) var<uniform> g : bool;
                                       ^^^^

input.wgsl:2:23 note: while instantiating 'var' g
@group(0) @binding(0) var<uniform> g : bool;
                      ^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferBool) {
    EXPECT_ERROR(R"(
alias t = ptr<uniform, bool>;
)",
                 R"(
input.wgsl:2:24 error: type 'bool' cannot be used in address space 'uniform' as it is non-host-shareable
alias t = ptr<uniform, bool>;
                       ^^^^

input.wgsl:2:11 note: while instantiating ptr<uniform, bool, read>
alias t = ptr<uniform, bool>;
          ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferBoolAlias) {
    EXPECT_ERROR(R"(
alias a = bool;
@group(0) @binding(0) var<uniform> g : a;
)",
                 R"(
input.wgsl:3:40 error: type 'bool' cannot be used in address space 'uniform' as it is non-host-shareable
@group(0) @binding(0) var<uniform> g : a;
                                       ^

input.wgsl:3:23 note: while instantiating 'var' g
@group(0) @binding(0) var<uniform> g : a;
                      ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferBoolAlias) {
    EXPECT_ERROR(R"(
alias a = bool;
alias t = ptr<uniform, a>;
)",
                 R"(
input.wgsl:3:24 error: type 'bool' cannot be used in address space 'uniform' as it is non-host-shareable
alias t = ptr<uniform, a>;
                       ^

input.wgsl:3:11 note: while instantiating ptr<uniform, bool, read>
alias t = ptr<uniform, a>;
          ^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformPointer) {
    EXPECT_ERROR(R"(
@group(0) @binding(0) var<uniform> g : ptr<private, f32>;
)",
                 R"(
input.wgsl:2:40 error: type 'ptr<private, f32, read_write>' cannot be used in address space 'uniform' as it is non-host-shareable
@group(0) @binding(0) var<uniform> g : ptr<private, f32>;
                                       ^^^^^^^^^^^^^^^^^

input.wgsl:2:23 note: while instantiating 'var' g
@group(0) @binding(0) var<uniform> g : ptr<private, f32>;
                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferIntScalar) {
    EXPECT_SUCCESS("@group(0) @binding(0) var<uniform> g : i32;");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferIntScalar) {
    EXPECT_SUCCESS("alias t = ptr<uniform, i32>;");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferF16) {
    EXPECT_SUCCESS(R"(
enable f16;

@group(0) @binding(0) var<uniform> g : f16;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferF16) {
    EXPECT_SUCCESS(R"(
enable f16;

alias t = ptr<uniform, f16>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferVectorF32) {
    EXPECT_SUCCESS("@group(0) @binding(0) var<uniform> g : vec4<f32>;");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferVectorF32) {
    EXPECT_SUCCESS("alias t = ptr<uniform, vec4<f32>>;");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferVectorF16) {
    EXPECT_SUCCESS(R"(
enable f16;

@group(0) @binding(0) var<uniform> g : vec4<f16>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferVectorF16) {
    EXPECT_SUCCESS(R"(
enable f16;

alias t = ptr<uniform, f16>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferArrayF32) {
    EXPECT_SUCCESS(R"(
struct S {
   @size(16) f : f32,
}
@group(0) @binding(0) var<uniform> g : array<S, 3u>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferArrayF32) {
    EXPECT_SUCCESS(R"(
struct S {
  @size(16) f : f32,
}
alias t = ptr<uniform, array<S, 3u>>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferArrayF16) {
    EXPECT_SUCCESS(R"(
enable f16;

struct S {
   @size(16) f : f16,
}
@group(0) @binding(0) var<uniform> g : array<S, 3u>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferArrayF16) {
    EXPECT_SUCCESS(R"(
enable f16;

struct S {
  @size(16) f : f16,
}
alias t = ptr<uniform, array<S, 3u>>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferStructI32) {
    EXPECT_SUCCESS(R"(
struct S {
  x : i32
};
@group(0) @binding(0) var<uniform> g : S;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferStructI32) {
    EXPECT_SUCCESS(R"(
struct S {
  x : i32,
};
alias t = ptr<uniform, S>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferStructI32Aliases) {
    EXPECT_SUCCESS(R"(
struct S {
  x : i32
};
alias a1 = S;
@group(0) @binding(0) var<uniform> g : a1;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferStructI32Aliases) {
    EXPECT_SUCCESS(R"(
struct S {
  x : i32
};
alias a1 = S;
alias t = ptr<uniform, a1>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferStructF16) {
    EXPECT_SUCCESS(R"(
enable f16;

struct S {
  x : f16
};
@group(0) @binding(0) var<uniform> g : S;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferStructF16) {
    EXPECT_SUCCESS(R"(
enable f16;

struct S {
  x : f16
};
alias t = ptr<uniform, S>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferStructF16Aliases) {
    EXPECT_SUCCESS(R"(
enable f16;

struct S {
  x : f16
};
alias a1 = S;
@group(0) @binding(0) var<uniform> g : a1;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferStructF16Aliases) {
    EXPECT_SUCCESS(R"(
enable f16;

struct S {
  x : f16
};
alias a1 = S;
alias t = ptr<uniform, a1>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_ImmediateBool) {
    EXPECT_ERROR(R"(
var<immediate> g : bool;
)",
                 R"(
input.wgsl:2:20 error: type 'bool' cannot be used in address space 'immediate' as it is non-host-shareable
var<immediate> g : bool;
                   ^^^^

input.wgsl:2:1 note: while instantiating 'var' g
var<immediate> g : bool;
^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_ImmediateBool) {
    EXPECT_ERROR(R"(
alias t = ptr<immediate, bool>;
)",
                 R"(
input.wgsl:2:26 error: type 'bool' cannot be used in address space 'immediate' as it is non-host-shareable
alias t = ptr<immediate, bool>;
                         ^^^^

input.wgsl:2:11 note: while instantiating ptr<immediate, bool, read>
alias t = ptr<immediate, bool>;
          ^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_ImmediateF16) {
    EXPECT_SUCCESS(R"(
enable f16;

var<immediate> g : f16;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_ImmediateF16) {
    EXPECT_SUCCESS(R"(
enable f16;

alias t = ptr<immediate, f16>;
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_ImmediatePointer) {
    EXPECT_ERROR(R"(
var<immediate> g : ptr<private, f32>;
)",
                 R"(
input.wgsl:2:20 error: type 'ptr<private, f32, read_write>' cannot be used in address space 'immediate' as it is non-host-shareable
var<immediate> g : ptr<private, f32>;
                   ^^^^^^^^^^^^^^^^^

input.wgsl:2:1 note: while instantiating 'var' g
var<immediate> g : ptr<private, f32>;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_ImmediateIntScalar) {
    EXPECT_SUCCESS("var<immediate> g : i32;");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_ImmediateIntScalar) {
    EXPECT_SUCCESS("alias t = ptr<immediate, i32>;");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_ImmediateVectorF32) {
    EXPECT_SUCCESS("var<immediate> g : vec4<f32>;");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_ImmediateVectorF32) {
    EXPECT_SUCCESS("var<immediate> g : vec4<f32>;");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_ImmediateArrayF32) {
    EXPECT_ERROR(R"(
struct S {
  a : f32
}
var<immediate> g : array<S, 3u>;
)",
                 R"(input.wgsl:5:20 error: arrays cannot be used in the <immediate> address space
var<immediate> g : array<S, 3u>;
                   ^^^^^^^^^^^^

input.wgsl:5:1 note: while instantiating 'var' g
var<immediate> g : array<S, 3u>;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_ImmediateArrayF32) {
    EXPECT_ERROR(R"(
struct S {
  a : f32
}
alias t = ptr<immediate, array<S, 3u>>;
)",
                 R"(input.wgsl:5:26 error: arrays cannot be used in the <immediate> address space
alias t = ptr<immediate, array<S, 3u>>;
                         ^^^^^^^^^^^^

input.wgsl:5:11 note: while instantiating ptr<immediate, array<S, 3>, read>
alias t = ptr<immediate, array<S, 3u>>;
          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

}  // namespace
}  // namespace tint::resolver
