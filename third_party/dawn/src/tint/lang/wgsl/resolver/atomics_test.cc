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

#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

struct ResolverAtomicTest : public resolver::TestHelper, public testing::Test {};

TEST_F(ResolverAtomicTest, GlobalWorkgroupI32) {
    auto* g = GlobalVar("a", ty.atomic(Source{{12, 34}}, ty.i32()), core::AddressSpace::kWorkgroup);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_TRUE(TypeOf(g)->Is<core::type::Reference>());
    auto* atomic = TypeOf(g)->UnwrapRef()->As<core::type::Atomic>();
    ASSERT_NE(atomic, nullptr);
    EXPECT_TRUE(atomic->Type()->Is<core::type::I32>());
}

TEST_F(ResolverAtomicTest, GlobalWorkgroupU32) {
    auto* g = GlobalVar("a", ty.atomic(Source{{12, 34}}, ty.u32()), core::AddressSpace::kWorkgroup);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_TRUE(TypeOf(g)->Is<core::type::Reference>());
    auto* atomic = TypeOf(g)->UnwrapRef()->As<core::type::Atomic>();
    ASSERT_NE(atomic, nullptr);
    EXPECT_TRUE(atomic->Type()->Is<core::type::U32>());
}

TEST_F(ResolverAtomicTest, GlobalStorageStruct) {
    auto* s = Structure("s", Vector{Member("a", ty.atomic(Source{{12, 34}}, ty.i32()))});
    auto* g = GlobalVar("g", ty.Of(s), core::AddressSpace::kStorage, core::Access::kReadWrite,
                        Binding(0_a), Group(0_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_TRUE(TypeOf(g)->Is<core::type::Reference>());
    auto* str = TypeOf(g)->UnwrapRef()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    ASSERT_EQ(str->Members().Length(), 1u);
    auto* atomic = str->Members()[0]->Type()->As<core::type::Atomic>();
    ASSERT_NE(atomic, nullptr);
    ASSERT_TRUE(atomic->Type()->Is<core::type::I32>());
}

}  // namespace
}  // namespace tint::resolver
