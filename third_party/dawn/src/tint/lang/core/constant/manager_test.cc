// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/constant/manager.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u64.h"
#include "src/tint/lang/core/type/u8.h"

namespace tint::core::constant {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

template <typename T>
size_t count(const T& range_loopable) {
    size_t n = 0;
    for (auto it : range_loopable) {
        (void)it;
        n++;
    }
    return n;
}

using ManagerTest = testing::Test;

TEST_F(ManagerTest, GetUnregistered) {
    constant::Manager cm;

    auto* c = cm.Get(1_i);
    static_assert(std::is_same_v<const Scalar<i32>*, decltype(c)>);
    ASSERT_NE(c, nullptr);
}

TEST_F(ManagerTest, GetSameConstantReturnsSamePtr) {
    constant::Manager cm;

    auto* c1 = cm.Get(1_i);
    static_assert(std::is_same_v<const Scalar<i32>*, decltype(c1)>);
    ASSERT_NE(c1, nullptr);

    auto* c2 = cm.Get(1_i);
    static_assert(std::is_same_v<const Scalar<i32>*, decltype(c2)>);
    EXPECT_EQ(c1, c2);
    EXPECT_EQ(c1->Type(), c2->Type());
}

TEST_F(ManagerTest, GetDifferentTypeReturnsDifferentPtr) {
    constant::Manager cm;

    auto* c1 = cm.Get(1_i);
    static_assert(std::is_same_v<const Scalar<i32>*, decltype(c1)>);
    ASSERT_NE(c1, nullptr);

    auto* c2 = cm.Get(1_u);
    static_assert(std::is_same_v<const Scalar<u32>*, decltype(c2)>);
    EXPECT_NE(static_cast<const Value*>(c1), static_cast<const Value*>(c2));
    EXPECT_NE(c1->Type(), c2->Type());
}

TEST_F(ManagerTest, GetDifferentValueReturnsDifferentPtr) {
    constant::Manager cm;

    auto* c1 = cm.Get(1_i);
    static_assert(std::is_same_v<const Scalar<i32>*, decltype(c1)>);
    ASSERT_NE(c1, nullptr);

    auto* c2 = cm.Get(2_i);
    static_assert(std::is_same_v<const Scalar<i32>*, decltype(c2)>);
    ASSERT_NE(c2, nullptr);
    EXPECT_NE(c1, c2);
    EXPECT_EQ(c1->Type(), c2->Type());
}

TEST_F(ManagerTest, Get_i32) {
    constant::Manager cm;

    auto* c = cm.Get(1_i);
    static_assert(std::is_same_v<const Scalar<i32>*, decltype(c)>);
    ASSERT_TRUE(Is<core::type::I32>(c->Type()));
    EXPECT_EQ(c->value, 1_i);
}

TEST_F(ManagerTest, Get_i8) {
    constant::Manager cm;

    auto* c = cm.Get(i8(1));
    static_assert(std::is_same_v<const Scalar<i8>*, decltype(c)>);
    ASSERT_TRUE(Is<core::type::I8>(c->Type()));
    EXPECT_EQ(c->value, i8(1));
}

TEST_F(ManagerTest, Get_u32) {
    constant::Manager cm;

    auto* c = cm.Get(1_u);
    static_assert(std::is_same_v<const Scalar<u32>*, decltype(c)>);
    ASSERT_TRUE(Is<core::type::U32>(c->Type()));
    EXPECT_EQ(c->value, 1_u);
}

TEST_F(ManagerTest, Get_u64) {
    constant::Manager cm;

    auto* c = cm.Get(u64(1));
    static_assert(std::is_same_v<const Scalar<u64>*, decltype(c)>);
    ASSERT_TRUE(Is<core::type::U64>(c->Type()));
    EXPECT_EQ(c->value, u64(1));
}

TEST_F(ManagerTest, Get_u8) {
    constant::Manager cm;

    auto* c = cm.Get(u8(1));
    static_assert(std::is_same_v<const Scalar<u8>*, decltype(c)>);
    ASSERT_TRUE(Is<core::type::U8>(c->Type()));
    EXPECT_EQ(c->value, u8(1));
}

TEST_F(ManagerTest, Get_f32) {
    constant::Manager cm;

    auto* c = cm.Get(1_f);
    static_assert(std::is_same_v<const Scalar<f32>*, decltype(c)>);
    ASSERT_TRUE(Is<core::type::F32>(c->Type()));
    EXPECT_EQ(c->value, 1_f);
}

TEST_F(ManagerTest, Get_f16) {
    constant::Manager cm;

    auto* c = cm.Get(1_h);
    static_assert(std::is_same_v<const Scalar<f16>*, decltype(c)>);
    ASSERT_TRUE(Is<core::type::F16>(c->Type()));
    EXPECT_EQ(c->value, 1_h);
}

TEST_F(ManagerTest, Get_bool) {
    constant::Manager cm;

    auto* c = cm.Get(true);
    static_assert(std::is_same_v<const Scalar<bool>*, decltype(c)>);
    ASSERT_TRUE(Is<core::type::Bool>(c->Type()));
    EXPECT_EQ(c->value, true);
}

TEST_F(ManagerTest, Get_AFloat) {
    constant::Manager cm;

    auto* c = cm.Get(1._a);
    static_assert(std::is_same_v<const Scalar<AFloat>*, decltype(c)>);
    ASSERT_TRUE(Is<core::type::AbstractFloat>(c->Type()));
    EXPECT_EQ(c->value, 1._a);
}

TEST_F(ManagerTest, Get_AInt) {
    constant::Manager cm;

    auto* c = cm.Get(1_a);
    static_assert(std::is_same_v<const Scalar<AInt>*, decltype(c)>);
    ASSERT_TRUE(Is<core::type::AbstractInt>(c->Type()));
    EXPECT_EQ(c->value, 1_a);
}

TEST_F(ManagerTest, WrapDoesntAffectInner_Constant) {
    Manager inner;
    Manager outer = Manager::Wrap(inner);

    inner.Get(1_i);

    EXPECT_EQ(count(inner), 1u);
    EXPECT_EQ(count(outer), 0u);

    outer.Get(1_i);

    EXPECT_EQ(count(inner), 1u);
    EXPECT_EQ(count(outer), 1u);
}

TEST_F(ManagerTest, WrapDoesntAffectInner_Types) {
    Manager inner;
    Manager outer = Manager::Wrap(inner);

    inner.types.Get<core::type::I32>();

    EXPECT_EQ(count(inner.types), 1u);
    EXPECT_EQ(count(outer.types), 0u);

    outer.types.Get<core::type::U32>();

    EXPECT_EQ(count(inner.types), 1u);
    EXPECT_EQ(count(outer.types), 1u);
}

}  // namespace
}  // namespace tint::core::constant
