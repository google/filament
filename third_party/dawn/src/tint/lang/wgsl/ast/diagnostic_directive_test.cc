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

#include "src/tint/lang/wgsl/ast/diagnostic_directive.h"

#include "src/tint/lang/wgsl/ast/helper_test.h"

namespace tint::ast {
namespace {

using DiagnosticDirectiveTest = TestHelper;

TEST_F(DiagnosticDirectiveTest, Name) {
    auto* d =
        DiagnosticDirective(Source{{{10, 5}, {10, 15}}}, wgsl::DiagnosticSeverity::kWarning, "foo");
    EXPECT_EQ(d->source.range.begin.line, 10u);
    EXPECT_EQ(d->source.range.begin.column, 5u);
    EXPECT_EQ(d->source.range.end.line, 10u);
    EXPECT_EQ(d->source.range.end.column, 15u);
    EXPECT_EQ(d->control.severity, wgsl::DiagnosticSeverity::kWarning);
    EXPECT_EQ(d->control.rule_name->category, nullptr);
    CheckIdentifier(d->control.rule_name->name, "foo");
}

TEST_F(DiagnosticDirectiveTest, CategoryAndName) {
    auto* d = DiagnosticDirective(Source{{{10, 5}, {10, 15}}}, wgsl::DiagnosticSeverity::kWarning,
                                  "foo", "bar");
    EXPECT_EQ(d->source.range.begin.line, 10u);
    EXPECT_EQ(d->source.range.begin.column, 5u);
    EXPECT_EQ(d->source.range.end.line, 10u);
    EXPECT_EQ(d->source.range.end.column, 15u);
    EXPECT_EQ(d->control.severity, wgsl::DiagnosticSeverity::kWarning);
    CheckIdentifier(d->control.rule_name->category, "foo");
    CheckIdentifier(d->control.rule_name->name, "bar");
}

}  // namespace
}  // namespace tint::ast
