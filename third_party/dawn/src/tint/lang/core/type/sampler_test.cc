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

#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/void.h"

namespace tint::core::type {
namespace {

using SamplerTest = TestHelper;

TEST_F(SamplerTest, Creation) {
    Manager ty;
    auto* a = ty.sampler();
    auto* b = ty.sampler();
    auto* c = ty.comparison_sampler();

    EXPECT_EQ(a->Kind(), SamplerKind::kSampler);
    EXPECT_EQ(c->Kind(), SamplerKind::kComparisonSampler);

    EXPECT_FALSE(a->IsComparison());
    EXPECT_TRUE(c->IsComparison());

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST_F(SamplerTest, Hash) {
    Manager ty;
    auto* a = ty.sampler();
    auto* b = ty.sampler();

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(SamplerTest, Equals) {
    Manager ty;
    auto* a = ty.sampler();
    auto* b = ty.sampler();
    auto* c = ty.comparison_sampler();

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(SamplerTest, FriendlyNameSampler) {
    Sampler s{SamplerKind::kSampler};
    EXPECT_EQ(s.FriendlyName(), "sampler");
}

TEST_F(SamplerTest, FriendlyNameComparisonSampler) {
    Sampler s{SamplerKind::kComparisonSampler};
    EXPECT_EQ(s.FriendlyName(), "sampler_comparison");
}

TEST_F(SamplerTest, Clone) {
    Manager ty;
    auto* a = ty.sampler();

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* mt = a->Clone(ctx);
    EXPECT_EQ(mt->Kind(), SamplerKind::kSampler);
}

}  // namespace
}  // namespace tint::core::type
