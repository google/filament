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

#include "src/tint/lang/wgsl/ast/transform/disable_uniformity_analysis.h"

#include <string>
#include <utility>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using DisableUniformityAnalysisTest = TransformTest;

TEST_F(DisableUniformityAnalysisTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_TRUE(ShouldRun<DisableUniformityAnalysis>(src));
}

TEST_F(DisableUniformityAnalysisTest, ShouldRunExtensionAlreadyPresent) {
    auto* src = R"(
enable chromium_disable_uniformity_analysis;
)";

    EXPECT_FALSE(ShouldRun<DisableUniformityAnalysis>(src));
}

TEST_F(DisableUniformityAnalysisTest, EmptyModule) {
    auto* src = R"()";

    auto* expect = R"(
enable chromium_disable_uniformity_analysis;
)";

    auto got = Run<DisableUniformityAnalysis>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DisableUniformityAnalysisTest, NonEmptyModule) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read> global : i32;

@compute @workgroup_size(64)
fn main() {
  if ((global == 42)) {
    workgroupBarrier();
  }
}
)";

    auto expect = "\nenable chromium_disable_uniformity_analysis;\n" + std::string(src);

    auto got = Run<DisableUniformityAnalysis>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
