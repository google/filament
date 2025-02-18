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

#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include "gmock/gmock.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

////////////////////////////////////////////////////////////////////////////////
// access
////////////////////////////////////////////////////////////////////////////////
using ResolverBuiltinStructs = ResolverTestWithParam<core::BuiltinType>;

TEST_P(ResolverBuiltinStructs, Resolve) {
    Enable(wgsl::Extension::kF16);

    // var<private> p : NAME;
    auto* var = GlobalVar("p", ty(GetParam()), core::AddressSpace::kPrivate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* str = As<core::type::Struct>(TypeOf(var)->UnwrapRef());
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Name().Name(), tint::ToString(GetParam()));
    EXPECT_FALSE(Is<sem::Struct>(str));
}

INSTANTIATE_TEST_SUITE_P(,
                         ResolverBuiltinStructs,
                         testing::Values(core::BuiltinType::kAtomicCompareExchangeResultI32,
                                         core::BuiltinType::kAtomicCompareExchangeResultU32,
                                         core::BuiltinType::kFrexpResultAbstract,
                                         core::BuiltinType::kFrexpResultF16,
                                         core::BuiltinType::kFrexpResultF32,
                                         core::BuiltinType::kFrexpResultVec2Abstract,
                                         core::BuiltinType::kFrexpResultVec2F16,
                                         core::BuiltinType::kFrexpResultVec2F32,
                                         core::BuiltinType::kFrexpResultVec3Abstract,
                                         core::BuiltinType::kFrexpResultVec3F16,
                                         core::BuiltinType::kFrexpResultVec3F32,
                                         core::BuiltinType::kFrexpResultVec4Abstract,
                                         core::BuiltinType::kFrexpResultVec4F16,
                                         core::BuiltinType::kFrexpResultVec4F32,
                                         core::BuiltinType::kModfResultAbstract,
                                         core::BuiltinType::kModfResultF16,
                                         core::BuiltinType::kModfResultF32,
                                         core::BuiltinType::kModfResultVec2Abstract,
                                         core::BuiltinType::kModfResultVec2F16,
                                         core::BuiltinType::kModfResultVec2F32,
                                         core::BuiltinType::kModfResultVec3Abstract,
                                         core::BuiltinType::kModfResultVec3F16,
                                         core::BuiltinType::kModfResultVec3F32,
                                         core::BuiltinType::kModfResultVec4Abstract,
                                         core::BuiltinType::kModfResultVec4F16,
                                         core::BuiltinType::kModfResultVec4F32));

}  // namespace
}  // namespace tint::resolver
