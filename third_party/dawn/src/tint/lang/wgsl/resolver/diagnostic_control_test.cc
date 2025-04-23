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

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverDiagnosticControlTest = ResolverTest;

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_DefaultSeverity) {
    auto stmts = Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(warning: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_ErrorViaDirective) {
    DiagnosticDirective(wgsl::DiagnosticSeverity::kError, "chromium", "unreachable_code");

    auto stmts = Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_WarningViaDirective) {
    DiagnosticDirective(wgsl::DiagnosticSeverity::kWarning, "chromium", "unreachable_code");

    auto stmts = Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(warning: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_InfoViaDirective) {
    DiagnosticDirective(wgsl::DiagnosticSeverity::kInfo, "chromium", "unreachable_code");

    auto stmts = Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(note: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_OffViaDirective) {
    DiagnosticDirective(wgsl::DiagnosticSeverity::kOff, "chromium", "unreachable_code");

    auto stmts = Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(r()->error().empty());
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_ErrorViaAttribute) {
    auto* attr =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kError, "chromium", "unreachable_code");

    auto stmts = Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts, Vector{attr});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_WarningViaAttribute) {
    auto* attr =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kWarning, "chromium", "unreachable_code");

    auto stmts = Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts, Vector{attr});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(warning: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_InfoViaAttribute) {
    auto* attr =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kInfo, "chromium", "unreachable_code");

    auto stmts = Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts, Vector{attr});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(note: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_OffViaAttribute) {
    auto* attr =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "chromium", "unreachable_code");

    auto stmts = Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts, Vector{attr});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(r()->error().empty());
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_ErrorViaDirective_OverriddenViaAttribute) {
    // diagnostic(error, chromium.unreachable_code);
    //
    // @diagnostic(off, chromium.unreachable_code) fn foo() {
    //   return;
    //   return; // Should produce a warning
    // }
    DiagnosticDirective(wgsl::DiagnosticSeverity::kError, "chromium", "unreachable_code");
    auto* attr =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kWarning, "chromium", "unreachable_code");

    auto stmts = Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts, Vector{attr});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(warning: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, FunctionAttributeScope) {
    // @diagnostic(off, chromium.unreachable_code) fn foo() {
    //   return;
    //   return; // Should not produce a diagnostic
    // }
    //
    // fn zoo() {
    //   return;
    //   return; // Should produce a warning (default severity)
    // }
    //
    // @diagnostic(info, chromium.unreachable_code) fn bar() {
    //   return;
    //   return; // Should produce an info
    // }
    {
        auto* attr =
            DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "chromium", "unreachable_code");
        Func("foo", {}, ty.void_(),
             Vector{
                 Return(),
                 Return(Source{{12, 34}}),
             },
             Vector{attr});
    }
    {
        Func("bar", {}, ty.void_(),
             Vector{
                 Return(),
                 Return(Source{{45, 67}}),
             });
    }
    {
        auto* attr =
            DiagnosticAttribute(wgsl::DiagnosticSeverity::kInfo, "chromium", "unreachable_code");
        Func("zoo", {}, ty.void_(),
             Vector{
                 Return(),
                 Return(Source{{89, 10}}),
             },
             Vector{attr});
    }

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(45:67 warning: code is unreachable
89:10 note: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, BlockAttributeScope) {
    // fn foo() @diagnostic(off, chromium.unreachable_code) {
    //   {
    //     return;
    //     return; // Should not produce a diagnostic
    //   }
    //   @diagnostic(warning, chromium.unreachable_code) {
    //     if (true) @diagnostic(info, chromium.unreachable_code) {
    //       return;
    //       return; // Should produce an info
    //     } else {
    //       while (true) @diagnostic(off, chromium.unreachable_code) {
    //         return;
    //         return; // Should not produce a diagnostic
    //       }
    //       return;
    //       return; // Should produce an warning
    //     }
    //   }
    // }

    auto attr = [&](auto severity) {
        return Vector{DiagnosticAttribute(severity, "chromium", "unreachable_code")};
    };
    Func("foo", {}, ty.void_(),
         Vector{
             Return(),
             Return(Source{{12, 21}}),
             Block(Vector{
                 Block(
                     Vector{
                         If(Expr(true),
                            Block(
                                Vector{
                                    Return(),
                                    Return(Source{{34, 43}}),
                                },
                                attr(wgsl::DiagnosticSeverity::kInfo)),
                            Else(Block(Vector{
                                While(Expr(true), Block(
                                                      Vector{
                                                          Return(),
                                                          Return(Source{{56, 65}}),
                                                      },
                                                      attr(wgsl::DiagnosticSeverity::kOff))),
                                Return(),
                                Return(Source{{78, 87}}),
                            }))),
                     },
                     attr(wgsl::DiagnosticSeverity::kWarning)),
             }),
         },
         attr(wgsl::DiagnosticSeverity::kOff));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(34:43 note: code is unreachable
78:87 warning: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, UnrecognizedCoreRuleName_Directive) {
    DiagnosticDirective(wgsl::DiagnosticSeverity::kError,
                        DiagnosticRuleName(Source{{12, 34}}, "derivative_uniform"));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(12:34 warning: unrecognized diagnostic rule 'derivative_uniform'
Did you mean 'derivative_uniformity'?
Possible values: 'derivative_uniformity', 'subgroup_uniformity')");
}

TEST_F(ResolverDiagnosticControlTest, UnrecognizedCoreRuleName_Attribute) {
    auto* attr = DiagnosticAttribute(wgsl::DiagnosticSeverity::kError,
                                     DiagnosticRuleName(Source{{12, 34}}, "derivative_uniform"));
    Func("foo", {}, ty.void_(), {}, Vector{attr});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(12:34 warning: unrecognized diagnostic rule 'derivative_uniform'
Did you mean 'derivative_uniformity'?
Possible values: 'derivative_uniformity', 'subgroup_uniformity')");
}

TEST_F(ResolverDiagnosticControlTest, UnrecognizedChromiumRuleName_Directive) {
    DiagnosticDirective(wgsl::DiagnosticSeverity::kError,
                        DiagnosticRuleName(Source{{12, 34}}, "chromium", "unreachable_cod"));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(12:34 warning: unrecognized diagnostic rule 'chromium.unreachable_cod'
Did you mean 'chromium.unreachable_code'?
Possible values: 'chromium.subgroup_matrix_uniformity', 'chromium.unreachable_code')");
}

TEST_F(ResolverDiagnosticControlTest, UnrecognizedChromiumRuleName_Attribute) {
    auto* attr =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kError,
                            DiagnosticRuleName(Source{{12, 34}}, "chromium", "unreachable_cod"));
    Func("foo", {}, ty.void_(), {}, Vector{attr});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(12:34 warning: unrecognized diagnostic rule 'chromium.unreachable_cod'
Did you mean 'chromium.unreachable_code'?
Possible values: 'chromium.subgroup_matrix_uniformity', 'chromium.unreachable_code')");
}

TEST_F(ResolverDiagnosticControlTest, UnrecognizedOtherRuleName_Directive) {
    DiagnosticDirective(wgsl::DiagnosticSeverity::kError,
                        DiagnosticRuleName(Source{{12, 34}}, "unknown", "unreachable_cod"));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "");
}

TEST_F(ResolverDiagnosticControlTest, UnrecognizedOtherRuleName_Attribute) {
    auto* attr =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kError,
                            DiagnosticRuleName(Source{{12, 34}}, "unknown", "unreachable_cod"));
    Func("foo", {}, ty.void_(), {}, Vector{attr});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "");
}

TEST_F(ResolverDiagnosticControlTest, Conflict_SameNameSameSeverity_Directive) {
    DiagnosticDirective(wgsl::DiagnosticSeverity::kError,
                        DiagnosticRuleName(Source{{12, 34}}, "chromium", "unreachable_code"));
    DiagnosticDirective(wgsl::DiagnosticSeverity::kError,
                        DiagnosticRuleName(Source{{56, 78}}, "chromium", "unreachable_code"));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverDiagnosticControlTest, Conflict_SameNameDifferentSeverity_Directive) {
    DiagnosticDirective(wgsl::DiagnosticSeverity::kError,
                        DiagnosticRuleName(Source{{12, 34}}, "chromium", "unreachable_code"));
    DiagnosticDirective(wgsl::DiagnosticSeverity::kOff,
                        DiagnosticRuleName(Source{{56, 78}}, "chromium", "unreachable_code"));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: conflicting diagnostic directive
12:34 note: severity of 'chromium.unreachable_code' set to 'off' here)");
}

TEST_F(ResolverDiagnosticControlTest, Conflict_SameUnknownNameDifferentSeverity_Directive) {
    DiagnosticDirective(wgsl::DiagnosticSeverity::kError,
                        DiagnosticRuleName(Source{{12, 34}}, "chromium", "unreachable_codes"));
    DiagnosticDirective(wgsl::DiagnosticSeverity::kOff,
                        DiagnosticRuleName(Source{{56, 78}}, "chromium", "unreachable_codes"));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 warning: unrecognized diagnostic rule 'chromium.unreachable_codes'
Did you mean 'chromium.unreachable_code'?
Possible values: 'chromium.subgroup_matrix_uniformity', 'chromium.unreachable_code'
56:78 warning: unrecognized diagnostic rule 'chromium.unreachable_codes'
Did you mean 'chromium.unreachable_code'?
Possible values: 'chromium.subgroup_matrix_uniformity', 'chromium.unreachable_code'
56:78 error: conflicting diagnostic directive
12:34 note: severity of 'chromium.unreachable_codes' set to 'off' here)");
}

TEST_F(ResolverDiagnosticControlTest, Conflict_DifferentUnknownNameDifferentSeverity_Directive) {
    DiagnosticDirective(wgsl::DiagnosticSeverity::kError, "chromium", "unreachable_codes");
    DiagnosticDirective(wgsl::DiagnosticSeverity::kOff, "chromium", "unreachable_codex");
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverDiagnosticControlTest, Conflict_SameNameSameSeverity_Attribute) {
    auto* attr1 =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kError,
                            DiagnosticRuleName(Source{{12, 34}}, "chromium", "unreachable_code"));
    auto* attr2 =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kError,
                            DiagnosticRuleName(Source{{56, 78}}, "chromium", "unreachable_code"));
    Func("foo", {}, ty.void_(), {}, Vector{attr1, attr2});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate diagnostic attribute
12:34 note: first attribute declared here)");
}

TEST_F(ResolverDiagnosticControlTest, Conflict_SameNameDifferentSeverity_Attribute) {
    auto* attr1 =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kError,
                            DiagnosticRuleName(Source{{12, 34}}, "chromium", "unreachable_code"));
    auto* attr2 =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff,
                            DiagnosticRuleName(Source{{56, 78}}, "chromium", "unreachable_code"));
    Func("foo", {}, ty.void_(), {}, Vector{attr1, attr2});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: conflicting diagnostic attribute
12:34 note: severity of 'chromium.unreachable_code' set to 'off' here)");
}

TEST_F(ResolverDiagnosticControlTest, Conflict_SameUnknownNameDifferentSeverity_Attribute) {
    auto* attr1 =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kError,
                            DiagnosticRuleName(Source{{12, 34}}, "chromium", "unreachable_codes"));
    auto* attr2 =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff,
                            DiagnosticRuleName(Source{{56, 78}}, "chromium", "unreachable_codes"));
    Func("foo", {}, ty.void_(), {}, Vector{attr1, attr2});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 warning: unrecognized diagnostic rule 'chromium.unreachable_codes'
Did you mean 'chromium.unreachable_code'?
Possible values: 'chromium.subgroup_matrix_uniformity', 'chromium.unreachable_code'
56:78 warning: unrecognized diagnostic rule 'chromium.unreachable_codes'
Did you mean 'chromium.unreachable_code'?
Possible values: 'chromium.subgroup_matrix_uniformity', 'chromium.unreachable_code'
56:78 error: conflicting diagnostic attribute
12:34 note: severity of 'chromium.unreachable_codes' set to 'off' here)");
}

TEST_F(ResolverDiagnosticControlTest, Conflict_DifferentUnknownNameDifferentSeverity_Attribute) {
    auto* attr1 =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kError, "chromium", "unreachable_codes");
    auto* attr2 =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "chromium", "unreachable_codex");
    Func("foo", {}, ty.void_(), {}, Vector{attr1, attr2});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverDiagnosticControlTest, Conflict_SameNameSameInfoSeverity_Attribute) {
    auto* attr1 =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kInfo,
                            DiagnosticRuleName(Source{{12, 34}}, "chromium", "unreachable_code"));
    auto* attr2 =
        DiagnosticAttribute(wgsl::DiagnosticSeverity::kInfo,
                            DiagnosticRuleName(Source{{56, 78}}, "chromium", "unreachable_code"));
    Func("foo", {}, ty.void_(), {}, Vector{attr1, attr2});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate diagnostic attribute
12:34 note: first attribute declared here)");
}

}  // namespace
}  // namespace tint::resolver
