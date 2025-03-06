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

#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/type/helper_test.h"

namespace tint::core::type {
namespace {

using ReferenceTest = TestHelper;

TEST_F(ReferenceTest, Creation) {
    auto* a =
        create<Reference>(core::AddressSpace::kStorage, create<I32>(), core::Access::kReadWrite);
    auto* b =
        create<Reference>(core::AddressSpace::kStorage, create<I32>(), core::Access::kReadWrite);
    auto* c =
        create<Reference>(core::AddressSpace::kStorage, create<F32>(), core::Access::kReadWrite);
    auto* d =
        create<Reference>(core::AddressSpace::kPrivate, create<I32>(), core::Access::kReadWrite);
    auto* e = create<Reference>(core::AddressSpace::kStorage, create<I32>(), core::Access::kRead);

    EXPECT_TRUE(a->StoreType()->Is<I32>());
    EXPECT_EQ(a->AddressSpace(), core::AddressSpace::kStorage);
    EXPECT_EQ(a->Access(), core::Access::kReadWrite);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
}

TEST_F(ReferenceTest, Hash) {
    auto* a =
        create<Reference>(core::AddressSpace::kStorage, create<I32>(), core::Access::kReadWrite);
    auto* b =
        create<Reference>(core::AddressSpace::kStorage, create<I32>(), core::Access::kReadWrite);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(ReferenceTest, Equals) {
    auto* a =
        create<Reference>(core::AddressSpace::kStorage, create<I32>(), core::Access::kReadWrite);
    auto* b =
        create<Reference>(core::AddressSpace::kStorage, create<I32>(), core::Access::kReadWrite);
    auto* c =
        create<Reference>(core::AddressSpace::kStorage, create<F32>(), core::Access::kReadWrite);
    auto* d =
        create<Reference>(core::AddressSpace::kPrivate, create<I32>(), core::Access::kReadWrite);
    auto* e = create<Reference>(core::AddressSpace::kStorage, create<I32>(), core::Access::kRead);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(*e));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(ReferenceTest, FriendlyName) {
    auto* r = create<Reference>(core::AddressSpace::kUndefined, create<I32>(), core::Access::kRead);
    EXPECT_EQ(r->FriendlyName(), "ref<i32, read>");
}

TEST_F(ReferenceTest, FriendlyNameWithAddressSpace) {
    auto* r = create<Reference>(core::AddressSpace::kWorkgroup, create<I32>(), core::Access::kRead);
    EXPECT_EQ(r->FriendlyName(), "ref<workgroup, i32, read>");
}

TEST_F(ReferenceTest, Clone) {
    auto* a =
        create<Reference>(core::AddressSpace::kStorage, create<I32>(), core::Access::kReadWrite);

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* ref = a->Clone(ctx);
    EXPECT_TRUE(ref->StoreType()->Is<I32>());
    EXPECT_EQ(ref->AddressSpace(), core::AddressSpace::kStorage);
    EXPECT_EQ(ref->Access(), core::Access::kReadWrite);
}

}  // namespace
}  // namespace tint::core::type
