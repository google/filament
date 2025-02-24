//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "PreprocessorTest.h"
#include "compiler/preprocessor/Token.h"

namespace angle
{

class PragmaTest : public SimplePreprocessorTest
{};

TEST_F(PragmaTest, EmptyName)
{
    const char *str      = "#pragma\n";
    const char *expected = "\n";

    using testing::_;
    // No handlePragma calls.
    EXPECT_CALL(mDirectiveHandler, handlePragma(_, _, _, false)).Times(0);
    // No error or warning.
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(str, expected);
}

TEST_F(PragmaTest, EmptyValue)
{
    const char *str      = "#pragma foo\n";
    const char *expected = "\n";

    using testing::_;
    EXPECT_CALL(mDirectiveHandler, handlePragma(pp::SourceLocation(0, 1), "foo", "", false));
    // No error or warning.
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(str, expected);
}

TEST_F(PragmaTest, NameValue)
{
    const char *str      = "#pragma foo(bar)\n";
    const char *expected = "\n";

    using testing::_;
    EXPECT_CALL(mDirectiveHandler, handlePragma(pp::SourceLocation(0, 1), "foo", "bar", false));
    // No error or warning.
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(str, expected);
}

TEST_F(PragmaTest, STDGL)
{
    const char *str      = "#pragma STDGL\n";
    const char *expected = "\n";

    using testing::_;
    EXPECT_CALL(mDirectiveHandler, handlePragma(_, _, _, _)).Times(0);
    // No error or warning.
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(str, expected);
}

TEST_F(PragmaTest, STDGLInvariantAll)
{
    const char *str      = "#pragma STDGL invariant(all)\n";
    const char *expected = "\n";

    using testing::_;
    EXPECT_CALL(mDirectiveHandler,
                handlePragma(pp::SourceLocation(0, 1), "invariant", "all", true));
    // No error or warning.
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(str, expected);
}

TEST_F(PragmaTest, Comments)
{
    const char *str =
        "/*foo*/"
        "#"
        "/*foo*/"
        "pragma"
        "/*foo*/"
        "foo"
        "/*foo*/"
        "("
        "/*foo*/"
        "bar"
        "/*foo*/"
        ")"
        "/*foo*/"
        "//foo"
        "\n";
    const char *expected = "\n";

    using testing::_;
    EXPECT_CALL(mDirectiveHandler, handlePragma(pp::SourceLocation(0, 1), "foo", "bar", false));
    // No error or warning.
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(str, expected);
}

TEST_F(PragmaTest, MissingNewline)
{
    const char *str      = "#pragma foo(bar)";
    const char *expected = "";

    using testing::_;
    // Pragma successfully parsed.
    EXPECT_CALL(mDirectiveHandler, handlePragma(pp::SourceLocation(0, 1), "foo", "bar", false));
    // Error reported about EOF.
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_EOF_IN_DIRECTIVE, _, _));

    preprocess(str, expected);
}

class InvalidPragmaTest : public PragmaTest, public testing::WithParamInterface<const char *>
{};

TEST_P(InvalidPragmaTest, Identified)
{
    const char *str      = GetParam();
    const char *expected = "\n";

    using testing::_;
    // No handlePragma calls.
    EXPECT_CALL(mDirectiveHandler, handlePragma(_, _, _, false)).Times(0);
    // Unrecognized pragma warning.
    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_UNRECOGNIZED_PRAGMA, pp::SourceLocation(0, 1), _));

    preprocess(str, expected);
}

INSTANTIATE_TEST_SUITE_P(All,
                         InvalidPragmaTest,
                         testing::Values("#pragma 1\n",               // Invalid name.
                                         "#pragma foo()\n",           // Missing value.
                                         "#pragma foo bar)\n",        // Missing left paren,
                                         "#pragma foo(bar\n",         // Missing right paren.
                                         "#pragma foo bar\n",         // Missing parens.
                                         "#pragma foo(bar) baz\n"));  // Extra tokens.

}  // namespace angle
