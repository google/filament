// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/core/constant/invalid.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/clone_context.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::core::constant {
namespace {

using ConstantTest_Invalid = testing::Test;

TEST_F(ConstantTest_Invalid, AllZero) {
    Manager constants;
    auto* invalid = constants.Invalid();
    EXPECT_FALSE(invalid->AllZero());
}

TEST_F(ConstantTest_Invalid, AnyZero) {
    Manager constants;
    auto* invalid = constants.Invalid();
    EXPECT_FALSE(invalid->AnyZero());
}

TEST_F(ConstantTest_Invalid, Index) {
    Manager constants;
    auto* invalid = constants.Invalid();
    EXPECT_EQ(invalid->Index(0), nullptr);
    EXPECT_EQ(invalid->Index(1), nullptr);
    EXPECT_EQ(invalid->Index(2), nullptr);
}

TEST_F(ConstantTest_Invalid, Clone) {
    Manager constants;
    auto* invalid = constants.Invalid();

    constant::Manager mgr;
    constant::CloneContext ctx{core::type::CloneContext{{nullptr}, {nullptr, &mgr.types}}, mgr};

    auto* cloned = invalid->Clone(ctx);
    EXPECT_NE(cloned, invalid);
    ASSERT_NE(cloned, nullptr);
    EXPECT_TRUE(cloned->type->Is<core::type::Invalid>());
    EXPECT_TRUE(cloned->Is<core::constant::Invalid>());
}

}  // namespace
}  // namespace tint::core::constant
