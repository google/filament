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

#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

#include "src/tint/lang/wgsl/ast/diagnostic_control.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, DiagnosticAttribute_Name) {
    auto p = parser("diagnostic(off, foo)");
    auto a = p->attribute();
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(a.matched);
    auto* d = a.value->As<ast::DiagnosticAttribute>();
    ASSERT_NE(d, nullptr);
    EXPECT_EQ(d->control.severity, wgsl::DiagnosticSeverity::kOff);
    auto* r = d->control.rule_name;
    ASSERT_NE(r, nullptr);
    EXPECT_EQ(r->category, nullptr);
    ast::CheckIdentifier(r->name, "foo");
}

TEST_F(WGSLParserTest, DiagnosticAttribute_CategoryName) {
    auto p = parser("diagnostic(off, foo.bar)");
    auto a = p->attribute();
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(a.matched);
    auto* d = a.value->As<ast::DiagnosticAttribute>();
    ASSERT_NE(d, nullptr);
    EXPECT_EQ(d->control.severity, wgsl::DiagnosticSeverity::kOff);
    auto* r = d->control.rule_name;
    ASSERT_NE(r, nullptr);
    ast::CheckIdentifier(r->category, "foo");
    ast::CheckIdentifier(r->name, "bar");
}

}  // namespace
}  // namespace tint::wgsl::reader
