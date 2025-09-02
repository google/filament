// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/void.h"

namespace tint::core::type {
namespace {

using PointerTest = TestHelper;

TEST_F(PointerTest, Creation) {
    Manager ty;
    auto* a = ty.ptr(core::AddressSpace::kStorage, ty.i32(), core::Access::kReadWrite);
    auto* b = ty.ptr(core::AddressSpace::kStorage, ty.i32(), core::Access::kReadWrite);
    auto* c = ty.ptr(core::AddressSpace::kStorage, ty.f32(), core::Access::kReadWrite);
    auto* d = ty.ptr(core::AddressSpace::kPrivate, ty.i32(), core::Access::kReadWrite);
    auto* e = ty.ptr(core::AddressSpace::kStorage, ty.i32(), core::Access::kRead);

    EXPECT_TRUE(a->StoreType()->Is<I32>());
    EXPECT_EQ(a->AddressSpace(), core::AddressSpace::kStorage);
    EXPECT_EQ(a->Access(), core::Access::kReadWrite);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
}

TEST_F(PointerTest, Hash) {
    Manager ty;
    auto* a = ty.ptr(core::AddressSpace::kStorage, ty.i32(), core::Access::kReadWrite);
    auto* b = ty.ptr(core::AddressSpace::kStorage, ty.i32(), core::Access::kReadWrite);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(PointerTest, Equals) {
    Manager ty;
    auto* a = ty.ptr(core::AddressSpace::kStorage, ty.i32(), core::Access::kReadWrite);
    auto* b = ty.ptr(core::AddressSpace::kStorage, ty.i32(), core::Access::kReadWrite);
    auto* c = ty.ptr(core::AddressSpace::kStorage, ty.f32(), core::Access::kReadWrite);
    auto* d = ty.ptr(core::AddressSpace::kPrivate, ty.i32(), core::Access::kReadWrite);
    auto* e = ty.ptr(core::AddressSpace::kStorage, ty.i32(), core::Access::kRead);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(*e));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(PointerTest, FriendlyName) {
    Manager ty;
    auto* r = ty.ptr(core::AddressSpace::kUndefined, ty.i32(), core::Access::kRead);
    EXPECT_EQ(r->FriendlyName(), "ptr<i32, read>");
}

TEST_F(PointerTest, FriendlyNameWithAddressSpace) {
    Manager ty;
    auto* r = ty.ptr(core::AddressSpace::kWorkgroup, ty.i32(), core::Access::kRead);
    EXPECT_EQ(r->FriendlyName(), "ptr<workgroup, i32, read>");
}

TEST_F(PointerTest, Clone) {
    Manager ty;
    auto* a = ty.ptr(core::AddressSpace::kStorage, ty.i32(), core::Access::kReadWrite);

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* ptr = a->Clone(ctx);
    EXPECT_TRUE(ptr->StoreType()->Is<I32>());
    EXPECT_EQ(ptr->AddressSpace(), core::AddressSpace::kStorage);
    EXPECT_EQ(ptr->Access(), core::Access::kReadWrite);
}

}  // namespace
}  // namespace tint::core::type
