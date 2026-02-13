// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/wgsl/sem/array_count.h"

namespace tint::sem {
namespace {

using SemArrayTest = core::type::TestHelper;

TEST_F(SemArrayTest, IsConstructable) {
    core::type::Manager ty;
    auto* named_override_sized = ty.Get<core::type::Array>(
        ty.u32(), ty.Get<NamedOverrideArrayCount>(nullptr), 4u, 8u, 32u, 16u);
    auto* unnamed_override_sized = ty.Get<core::type::Array>(
        ty.u32(), ty.Get<UnnamedOverrideArrayCount>(nullptr), 4u, 8u, 32u, 16u);

    EXPECT_FALSE(named_override_sized->IsConstructible());
    EXPECT_FALSE(unnamed_override_sized->IsConstructible());
}

TEST_F(SemArrayTest, HasCreationFixedFootprint) {
    core::type::Manager ty;
    auto* named_override_sized = ty.Get<core::type::Array>(
        ty.u32(), ty.Get<NamedOverrideArrayCount>(nullptr), 4u, 8u, 32u, 16u);
    auto* unnamed_override_sized = ty.Get<core::type::Array>(
        ty.u32(), ty.Get<UnnamedOverrideArrayCount>(nullptr), 4u, 8u, 32u, 16u);

    EXPECT_FALSE(named_override_sized->HasCreationFixedFootprint());
    EXPECT_FALSE(unnamed_override_sized->HasCreationFixedFootprint());
}

TEST_F(SemArrayTest, HasFixedFootprint) {
    core::type::Manager ty;
    auto* named_override_sized = ty.Get<core::type::Array>(
        ty.u32(), ty.Get<NamedOverrideArrayCount>(nullptr), 4u, 8u, 32u, 16u);
    auto* unnamed_override_sized = ty.Get<core::type::Array>(
        ty.u32(), ty.Get<UnnamedOverrideArrayCount>(nullptr), 4u, 8u, 32u, 16u);

    EXPECT_TRUE(named_override_sized->HasFixedFootprint());
    EXPECT_TRUE(unnamed_override_sized->HasFixedFootprint());
}

}  // namespace
}  // namespace tint::sem
