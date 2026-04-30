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

#include "gmock/gmock.h"
#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace {

using AtomicVec2uMinMaxExtensionTest = ResolverTest;

TEST_F(AtomicVec2uMinMaxExtensionTest, AtomicVec2uWithoutExtension) {
    GlobalVar("a", ty.atomic(ty.vec2(Source{{12, 34}}, ty.u32())), core::AddressSpace::kStorage);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error:  The type 'atomic<vec2u>' cannot be used without the extension "
              "'atomic_vec2u_min_max'");
}

TEST_F(AtomicVec2uMinMaxExtensionTest, AtomicVec2uInWorkgroup) {
    Enable(wgsl::Extension::kAtomicVec2UMinMax);

    GlobalVar(Source{{14, 35}}, "a", ty.atomic(ty.vec2(Source{{12, 34}}, ty.u32())),
              core::AddressSpace::kWorkgroup);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: atomic variables of type 'vec2<u32>' can only be in 'storage' address space
14:35 note: while instantiating 'var' a)");
}

TEST_F(AtomicVec2uMinMaxExtensionTest, AtomicVec2uStorage) {
    Enable(wgsl::Extension::kAtomicVec2UMinMax);

    GlobalVar(Source{{14, 35}}, "a", ty.atomic(ty.vec2(Source{{12, 34}}, ty.u32())),
              core::AddressSpace::kStorage, core::Access::kReadWrite, Group(0_a), Binding(0_a));

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(AtomicVec2uMinMaxExtensionTest, AtomicVec2uInWorkgroupArray) {
    Enable(wgsl::Extension::kAtomicVec2UMinMax);

    GlobalVar(Source{{14, 35}}, "a", ty.array(ty.atomic(ty.vec2(Source{{12, 34}}, ty.u32())), 4_u),
              core::AddressSpace::kWorkgroup);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: atomic variables of type 'vec2<u32>' can only be in 'storage' address space
14:35 note: while instantiating 'var' a)");
}

TEST_F(AtomicVec2uMinMaxExtensionTest, AtomicVec2uStorageArray) {
    Enable(wgsl::Extension::kAtomicVec2UMinMax);

    GlobalVar("a", ty.array(ty.atomic(ty.vec2(ty.u32())), 4_u), core::AddressSpace::kStorage,
              core::Access::kReadWrite, Group(0_a), Binding(0_a));

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(AtomicVec2uMinMaxExtensionTest, AtomicMin_WithoutExtension) {
    GlobalVar("a", ty.atomic(ty.vec2(ty.u32())), core::AddressSpace::kStorage,
              core::Access::kReadWrite, Group(0_a), Binding(0_a));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             CallStmt(
                 Call(core::BuiltinFn::kAtomicStoreMin, AddressOf("a"), Call<vec2<u32>>(1_u, 2_u))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error:  The type 'atomic<vec2u>' cannot be used without the extension "
              "'atomic_vec2u_min_max'");
}

TEST_F(AtomicVec2uMinMaxExtensionTest, AtomicMin_WithExtension) {
    Enable(wgsl::Extension::kAtomicVec2UMinMax);
    GlobalVar("a", ty.atomic(ty.vec2(ty.u32())), core::AddressSpace::kStorage,
              core::Access::kReadWrite, Group(0_a), Binding(0_a));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             CallStmt(
                 Call(core::BuiltinFn::kAtomicStoreMin, AddressOf("a"), Call<vec2<u32>>(1_u, 2_u))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(AtomicVec2uMinMaxExtensionTest, AtomicMax_WithoutExtension) {
    GlobalVar("a", ty.atomic(ty.vec2(ty.u32())), core::AddressSpace::kStorage,
              core::Access::kReadWrite, Group(0_a), Binding(0_a));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             CallStmt(
                 Call(core::BuiltinFn::kAtomicStoreMax, AddressOf("a"), Call<vec2<u32>>(1_u, 2_u))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error:  The type 'atomic<vec2u>' cannot be used without the extension "
              "'atomic_vec2u_min_max'");
}

TEST_F(AtomicVec2uMinMaxExtensionTest, AtomicMax_WithExtension) {
    Enable(wgsl::Extension::kAtomicVec2UMinMax);
    GlobalVar("a", ty.atomic(ty.vec2(ty.u32())), core::AddressSpace::kStorage,
              core::Access::kReadWrite, Group(0_a), Binding(0_a));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             CallStmt(
                 Call(core::BuiltinFn::kAtomicStoreMax, AddressOf("a"), Call<vec2<u32>>(1_u, 2_u))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(AtomicVec2uMinMaxExtensionTest, AtomicVec2u_FeatureReq) {
    ExpectError(
        R"(

@group(0) @binding(0) var<storage, read_write> a : atomic<vec2u>;
fn f() {
  atomicStoreMax(&a, vec2u(1u, 2u));
}
)",
        R"(
input.wgsl:3:59 error:  The type 'atomic<vec2u>' cannot be used without the extension 'atomic_vec2u_min_max'
@group(0) @binding(0) var<storage, read_write> a : atomic<vec2u>;
                                                          ^^^^^
)");
}

TEST_F(AtomicVec2uMinMaxExtensionTest, AtomicVec2u_StorageBasic) {
    ExpectSuccess(
        R"(
enable atomic_vec2u_min_max;
@group(0) @binding(0) var<storage, read_write> a : atomic<vec2u>;
fn f() {
  atomicStoreMax(&a, vec2u(1u, 2u));
}
)");
}

TEST_F(AtomicVec2uMinMaxExtensionTest, AtomicVec2u_StorageArray) {
    ExpectSuccess(
        R"(
enable atomic_vec2u_min_max;
@group(0) @binding(0) var<storage, read_write> a : array<atomic<vec2u>>;
fn f() {
  atomicStoreMin(&a[0], vec2u(1u, 2u));
}
)");
}

TEST_F(AtomicVec2uMinMaxExtensionTest, AtomicVec2u_StorageStructArray) {
    ExpectSuccess(
        R"(
enable atomic_vec2u_min_max;
struct Data
{
   a:u32,
   b:atomic<vec2u>
}

@group(0) @binding(0) var<storage, read_write> a : array<Data>;
fn f() {
  atomicStoreMin(&a[0].b, vec2u(1u, 2u));
}
)");
}

TEST_F(AtomicVec2uMinMaxExtensionTest, AtomicVec2u_WorkgroupBasic) {
    ExpectError(
        R"(
enable atomic_vec2u_min_max;
@group(0) @binding(0) var<workgroup, read_write> a : array<atomic<vec2u>, 10>;
fn f() {
  atomicStoreMax(&a[0], vec2u(1u, 2u));
}
)",
        R"(
input.wgsl:3:54 error: atomic variables of type 'vec2<u32>' can only be in 'storage' address space
@group(0) @binding(0) var<workgroup, read_write> a : array<atomic<vec2u>, 10>;
                                                     ^^^^^^^^^^^^^^^^^^^^^^^^

input.wgsl:3:23 note: while instantiating 'var' a
@group(0) @binding(0) var<workgroup, read_write> a : array<atomic<vec2u>, 10>;
                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(AtomicVec2uMinMaxExtensionTest, AtomicVec2u_WrongParam) {
    ExpectError(
        R"(
enable atomic_vec2u_min_max;
@group(0) @binding(0) var<storage, read_write> a : array<atomic<vec2u>, 10>;
fn f() {
  atomicStoreMax(&a[0], 1u);
}
)",
        R"(input.wgsl:5:3 error: no matching call to 'atomicStoreMax(ptr<storage, atomic<vec2<u32>>, read_write>, u32)'

1 candidate function:
 • 'atomicStoreMax(ptr<storage, atomic<vec2<u32>>, read_write>  ✓ , vec2<u32>  ✗ )'

  atomicStoreMax(&a[0], 1u);
  ^^^^^^^^^^^^^^
)");
}

}  // namespace
}  // namespace tint::resolver
