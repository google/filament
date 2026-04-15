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

#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include <memory>

#include "src/tint/lang/wgsl/reader/reader.h"

namespace tint::resolver {

TestHelper::TestHelper()
    : resolver_(std::make_unique<Resolver>(this, wgsl::AllowedFeatures::Everything())) {}

TestHelper::~TestHelper() = default;

void TestHelper::ExpectError(std::string_view wgsl, std::string_view error) {
    wgsl::reader::Options options{
        .allowed_features = wgsl::AllowedFeatures::Everything(),
    };
    auto file = std::make_unique<Source::File>("input.wgsl", wgsl);
    auto result = wgsl::reader::Parse(file.get(), std::move(options));
    ASSERT_TRUE(result.Diagnostics().ContainsErrors());

    // Strip a single leading newline to allow expected errors to start with a newline.
    if (!error.empty() && error[0] == '\n') {
        error = error.substr(1);
    }
    EXPECT_EQ(result.Diagnostics().Str(), error);
}

void TestHelper::ExpectSuccess(std::string_view wgsl) {
    wgsl::reader::Options options{
        .allowed_features = wgsl::AllowedFeatures::Everything(),
    };
    auto file = std::make_unique<Source::File>("input.wgsl", wgsl);
    auto result = wgsl::reader::Parse(file.get(), std::move(options));
    ASSERT_TRUE(result.IsValid()) << result.Diagnostics().Str();
    ASSERT_FALSE(result.Diagnostics().ContainsErrors());
}

}  // namespace tint::resolver
