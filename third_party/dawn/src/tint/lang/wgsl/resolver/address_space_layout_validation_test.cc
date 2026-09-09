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
    EXPECT_ERROR(R"(
struct S {
    @size(5) a : f32,
    @align(1) b : f32,
};
@group(0) @binding(0) var<storage> a : S;
)",
                 R"(
input.wgsl:4:12 error: alignment must be a multiple of '4' bytes
    @align(1) b : f32,
           ^
)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, StorageBuffer_UnalignedMember_SuggestedFix) {
    EXPECT_SUCCESS(R"(
struct S {
    @size(5) a : f32,
    @align(4) b : f32,
};
@group(0) @binding(0) var<storage> a : S;
)");
}

// Detect unaligned struct member for uniform buffers
TEST_F(ResolverAddressSpaceLayoutValidationTest, UniformBuffer_UnalignedMember_Struct) {
    EXPECT_ERROR(
        R"(
struct Inner {
    scalar : i32,
};

struct Outer {
    @size(5) scalar : f32,
    @align(1) inner : Inner,
};

@group(0) @binding(0) var<uniform> a : Outer;
)",
        R"(
input.wgsl:8:12 error: alignment must be a multiple of '4' bytes
    @align(1) inner : Inner,
           ^
)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest,
       UniformBuffer_UnalignedMember_Struct_SuggestedFix) {
    EXPECT_SUCCESS(R"(
struct Inner {
    scalar : i32,
};

struct Outer {
    @size(5) scalar : f32,
    @align(4) inner : Inner,
};

@group(0) @binding(0) var<uniform> a : Outer;)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest,
       UniformBuffer_MembersOffsetNotMultipleOf16_SuggestedFix) {
    EXPECT_SUCCESS(R"(
struct Inner {
    @align(4) @size(5) scalar : i32,
};

struct Outer {
    inner : Inner,
    @align(16) scalar : i32,
};

@group(0) @binding(0) var<uniform> a : Outer;)");
}

// Make sure that this doesn't fail validation because vec3's align is 16, but
// size is 12. 's' should be at offset 12, which is okay here.
TEST_F(ResolverAddressSpaceLayoutValidationTest, UniformBuffer_Vec3MemberOffset_NoFail) {
    EXPECT_SUCCESS(R"(
struct ScalarPackedAtEndOfVec3 {
    v : vec3<f32>,
    s : f32,
};
@group(0) @binding(0) var<uniform> a : ScalarPackedAtEndOfVec3;)");
}

// Make sure that this doesn't fail validation because vec3's align is 8, but
// size is 6. 's' should be at offset 6, which is okay here.
TEST_F(ResolverAddressSpaceLayoutValidationTest, UniformBuffer_Vec3F16MemberOffset_NoFail) {
    EXPECT_SUCCESS(R"(
enable f16;
struct ScalarPackedAtEndOfVec3 {
    v : vec3<f16>,
    s : f16,
};
@group(0) @binding(0) var<uniform> a : ScalarPackedAtEndOfVec3;)");
}

// Detect unaligned member for immediate data buffers
TEST_F(ResolverAddressSpaceLayoutValidationTest, Immediate_UnalignedMember) {
    EXPECT_ERROR(R"(
struct S {
    @size(5) a : f32,
    @align(1) b : f32,
};
var<immediate> a : S;
)",
                 R"(
input.wgsl:4:12 error: alignment must be a multiple of '4' bytes
    @align(1) b : f32,
           ^
)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, Immediate_Aligned) {
    EXPECT_SUCCESS(R"(
struct S {
    @size(5) a : f32,
    @align(4i) b : f32,
};
var<immediate> a : S;
)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, AlignAttributeTooSmall_Storage) {
    EXPECT_ERROR(R"(
struct S {
  @align(4) vector : vec4u,
  scalar : u32,
};

@group(0) @binding(0) var<storage, read_write> a : array<S>;
)",
                 R"(
input.wgsl:3:10 error: alignment must be a multiple of '16' bytes
  @align(4) vector : vec4u,
         ^
)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, AlignAttributeTooSmall_Workgroup) {
    EXPECT_ERROR(R"(
struct S {
  @align(4) vector : vec4u,
  scalar : u32,
};

var<workgroup> a : array<S, 4>;
)",
                 R"(
input.wgsl:3:10 error: alignment must be a multiple of '16' bytes
  @align(4) vector : vec4u,
         ^
)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, AlignAttributeTooSmall_Private) {
    EXPECT_ERROR(R"(
struct S {
  @align(4) vector : vec4u,
  scalar : u32,
};
var<private> a : array<S, 4>;
)",
                 R"(
input.wgsl:3:10 error: alignment must be a multiple of '16' bytes
  @align(4) vector : vec4u,
         ^
)");
}

TEST_F(ResolverAddressSpaceLayoutValidationTest, AlignAttributeTooSmall_Function) {
    EXPECT_ERROR(R"(
struct S {
  @align(4) vector : vec4u,
  scalar : u32,
};

fn foo() {
  var a : array<S, 4>;
}
)",
                 R"(
input.wgsl:3:10 error: alignment must be a multiple of '16' bytes
  @align(4) vector : vec4u,
         ^
)");
}

}  // namespace
}  // namespace tint::resolver
