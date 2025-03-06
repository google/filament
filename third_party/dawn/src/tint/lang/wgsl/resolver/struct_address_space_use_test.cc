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

using ::testing::UnorderedElementsAre;

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverAddressSpaceUseTest = ResolverTest;

TEST_F(ResolverAddressSpaceUseTest, UnreachableStruct) {
    auto* s = Structure("S", Vector{Member("a", ty.f32())});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->AddressSpaceUsage().IsEmpty());
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableFromParameter) {
    auto* s = Structure("S", Vector{Member("a", ty.f32())});

    Func("f", Vector{Param("param", ty.Of(s))}, ty.void_(), tint::Empty, tint::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(core::AddressSpace::kUndefined));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableFromReturnType) {
    auto* s = Structure("S", Vector{Member("a", ty.f32())});

    Func("f", tint::Empty, ty.Of(s), Vector{Return(Call(ty.Of(s)))}, tint::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(core::AddressSpace::kUndefined));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableFromGlobal) {
    auto* s = Structure("S", Vector{Member("a", ty.f32())});

    GlobalVar("g", ty.Of(s), core::AddressSpace::kPrivate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(core::AddressSpace::kPrivate));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableViaGlobalAlias) {
    auto* s = Structure("S", Vector{Member("a", ty.f32())});
    auto* a = Alias("A", ty.Of(s));
    GlobalVar("g", ty.Of(a), core::AddressSpace::kPrivate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(core::AddressSpace::kPrivate));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableViaGlobalStruct) {
    auto* s = Structure("S", Vector{Member("a", ty.f32())});
    auto* o = Structure("O", Vector{Member("a", ty.Of(s))});
    GlobalVar("g", ty.Of(o), core::AddressSpace::kPrivate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(core::AddressSpace::kPrivate));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableViaGlobalArray) {
    auto* s = Structure("S", Vector{Member("a", ty.f32())});
    auto a = ty.array(ty.Of(s), 3_u);
    GlobalVar("g", a, core::AddressSpace::kPrivate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(core::AddressSpace::kPrivate));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableFromLocal) {
    auto* s = Structure("S", Vector{Member("a", ty.f32())});

    WrapInFunction(Var("g", ty.Of(s)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(core::AddressSpace::kFunction));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableViaLocalAlias) {
    auto* s = Structure("S", Vector{Member("a", ty.f32())});
    auto* a = Alias("A", ty.Of(s));
    WrapInFunction(Var("g", ty.Of(a)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(core::AddressSpace::kFunction));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableViaLocalStruct) {
    auto* s = Structure("S", Vector{Member("a", ty.f32())});
    auto* o = Structure("O", Vector{Member("a", ty.Of(s))});
    WrapInFunction(Var("g", ty.Of(o)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(core::AddressSpace::kFunction));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableViaLocalArray) {
    auto* s = Structure("S", Vector{Member("a", ty.f32())});
    auto a = ty.array(ty.Of(s), 3_u);
    WrapInFunction(Var("g", a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(core::AddressSpace::kFunction));
}

TEST_F(ResolverAddressSpaceUseTest, StructMultipleAddressSpaceUses) {
    auto* s = Structure("S", Vector{Member("a", ty.f32())});
    GlobalVar("x", ty.Of(s), core::AddressSpace::kUniform, Binding(0_a), Group(0_a));
    GlobalVar("y", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead, Binding(1_a),
              Group(0_a));
    WrapInFunction(Var("g", ty.Of(s)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(),
                UnorderedElementsAre(core::AddressSpace::kUniform, core::AddressSpace::kStorage,
                                     core::AddressSpace::kFunction));
}

}  // namespace
}  // namespace tint::resolver
