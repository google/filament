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

#include "src/tint/lang/wgsl/ast/enable.h"

#include "src/tint/lang/wgsl/ast/helper_test.h"

namespace tint::ast {
namespace {

using EnableTest = TestHelper;

TEST_F(EnableTest, Creation) {
    auto* ext = Enable(Source{{{20, 2}, {20, 5}}}, wgsl::Extension::kF16);
    EXPECT_EQ(ext->source.range.begin.line, 20u);
    EXPECT_EQ(ext->source.range.begin.column, 2u);
    EXPECT_EQ(ext->source.range.end.line, 20u);
    EXPECT_EQ(ext->source.range.end.column, 5u);
    ASSERT_EQ(ext->extensions.Length(), 1u);
    EXPECT_EQ(ext->extensions[0]->name, wgsl::Extension::kF16);
}

TEST_F(EnableTest, HasExtension) {
    auto* ext = Enable(Source{{{20, 2}, {20, 5}}}, wgsl::Extension::kF16);
    EXPECT_TRUE(ext->HasExtension(wgsl::Extension::kF16));
    EXPECT_FALSE(ext->HasExtension(wgsl::Extension::kChromiumDisableUniformityAnalysis));
}

}  // namespace
}  // namespace tint::ast
