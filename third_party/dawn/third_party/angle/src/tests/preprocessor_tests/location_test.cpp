//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "PreprocessorTest.h"
#include "compiler/preprocessor/Token.h"

namespace angle
{

class LocationTest : public PreprocessorTest
{
  protected:
    LocationTest() : PreprocessorTest(SH_GLES2_SPEC) {}

    void expectLocation(int count,
                        const char *const string[],
                        const int length[],
                        const pp::SourceLocation &location)
    {
        ASSERT_TRUE(mPreprocessor.init(count, string, length));

        pp::Token token;
        mPreprocessor.lex(&token);
        EXPECT_EQ(pp::Token::IDENTIFIER, token.type);
        EXPECT_EQ("foo", token.text);

        EXPECT_EQ(location.file, token.location.file);
        EXPECT_EQ(location.line, token.location.line);
    }
};

TEST_F(LocationTest, String0_Line1)
{
    const char *str = "foo";
    pp::SourceLocation loc(0, 1);

    SCOPED_TRACE("String0_Line1");
    expectLocation(1, &str, nullptr, loc);
}

TEST_F(LocationTest, String0_Line2)
{
    const char *str = "\nfoo";
    pp::SourceLocation loc(0, 2);

    SCOPED_TRACE("String0_Line2");
    expectLocation(1, &str, nullptr, loc);
}

TEST_F(LocationTest, String1_Line1)
{
    const char *const str[] = {"\n\n", "foo"};
    pp::SourceLocation loc(1, 1);

    SCOPED_TRACE("String1_Line1");
    expectLocation(2, str, nullptr, loc);
}

TEST_F(LocationTest, String1_Line2)
{
    const char *const str[] = {"\n\n", "\nfoo"};
    pp::SourceLocation loc(1, 2);

    SCOPED_TRACE("String1_Line2");
    expectLocation(2, str, nullptr, loc);
}

TEST_F(LocationTest, NewlineInsideCommentCounted)
{
    const char *str = "/*\n\n*/foo";
    pp::SourceLocation loc(0, 3);

    SCOPED_TRACE("NewlineInsideCommentCounted");
    expectLocation(1, &str, nullptr, loc);
}

TEST_F(LocationTest, ErrorLocationAfterComment)
{
    const char *str = "/*\n\n*/@";

    ASSERT_TRUE(mPreprocessor.init(1, &str, nullptr));
    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_INVALID_CHARACTER, pp::SourceLocation(0, 3), "@"));

    pp::Token token;
    mPreprocessor.lex(&token);
}

// The location of a token straddling two or more strings is that of the
// first character of the token.

TEST_F(LocationTest, TokenStraddlingTwoStrings)
{
    const char *const str[] = {"f", "oo"};
    pp::SourceLocation loc(0, 1);

    SCOPED_TRACE("TokenStraddlingTwoStrings");
    expectLocation(2, str, nullptr, loc);
}

TEST_F(LocationTest, TokenStraddlingThreeStrings)
{
    const char *const str[] = {"f", "o", "o"};
    pp::SourceLocation loc(0, 1);

    SCOPED_TRACE("TokenStraddlingThreeStrings");
    expectLocation(3, str, nullptr, loc);
}

TEST_F(LocationTest, EndOfFileWithoutNewline)
{
    const char *const str[] = {"foo"};
    ASSERT_TRUE(mPreprocessor.init(1, str, nullptr));

    pp::Token token;
    mPreprocessor.lex(&token);
    EXPECT_EQ(pp::Token::IDENTIFIER, token.type);
    EXPECT_EQ("foo", token.text);
    EXPECT_EQ(0, token.location.file);
    EXPECT_EQ(1, token.location.line);

    mPreprocessor.lex(&token);
    EXPECT_EQ(pp::Token::LAST, token.type);
    EXPECT_EQ(0, token.location.file);
    EXPECT_EQ(1, token.location.line);
}

TEST_F(LocationTest, EndOfFileAfterNewline)
{
    const char *const str[] = {"foo\n"};
    ASSERT_TRUE(mPreprocessor.init(1, str, nullptr));

    pp::Token token;
    mPreprocessor.lex(&token);
    EXPECT_EQ(pp::Token::IDENTIFIER, token.type);
    EXPECT_EQ("foo", token.text);
    EXPECT_EQ(0, token.location.file);
    EXPECT_EQ(1, token.location.line);

    mPreprocessor.lex(&token);
    EXPECT_EQ(pp::Token::LAST, token.type);
    EXPECT_EQ(0, token.location.file);
    EXPECT_EQ(2, token.location.line);
}

TEST_F(LocationTest, EndOfFileAfterEmptyString)
{
    const char *const str[] = {"foo\n", "\n", ""};
    ASSERT_TRUE(mPreprocessor.init(3, str, nullptr));

    pp::Token token;
    mPreprocessor.lex(&token);
    EXPECT_EQ(pp::Token::IDENTIFIER, token.type);
    EXPECT_EQ("foo", token.text);
    EXPECT_EQ(0, token.location.file);
    EXPECT_EQ(1, token.location.line);

    mPreprocessor.lex(&token);
    EXPECT_EQ(pp::Token::LAST, token.type);
    EXPECT_EQ(2, token.location.file);
    EXPECT_EQ(1, token.location.line);
}

TEST_F(LocationTest, ValidLineDirective1)
{
    const char *str =
        "#line 10\n"
        "foo";
    pp::SourceLocation loc(0, 10);

    SCOPED_TRACE("ValidLineDirective1");
    expectLocation(1, &str, nullptr, loc);
}

TEST_F(LocationTest, ValidLineDirective2)
{
    const char *str =
        "#line 10 20\n"
        "foo";
    pp::SourceLocation loc(20, 10);

    SCOPED_TRACE("ValidLineDirective2");
    expectLocation(1, &str, nullptr, loc);
}

TEST_F(LocationTest, LineDirectiveCommentsIgnored)
{
    const char *str =
        "/* bar */"
        "#"
        "/* bar */"
        "line"
        "/* bar */"
        "10"
        "/* bar */"
        "20"
        "/* bar */"
        "// bar   "
        "\n"
        "foo";
    pp::SourceLocation loc(20, 10);

    SCOPED_TRACE("LineDirectiveCommentsIgnored");
    expectLocation(1, &str, nullptr, loc);
}

TEST_F(LocationTest, LineDirectiveWithMacro1)
{
    const char *str =
        "#define L 10\n"
        "#define F(x) x\n"
        "#line L F(20)\n"
        "foo";
    pp::SourceLocation loc(20, 10);

    SCOPED_TRACE("LineDirectiveWithMacro1");
    expectLocation(1, &str, nullptr, loc);
}

TEST_F(LocationTest, LineDirectiveWithMacro2)
{
    const char *str =
        "#define LOC 10 20\n"
        "#line LOC\n"
        "foo";
    pp::SourceLocation loc(20, 10);

    SCOPED_TRACE("LineDirectiveWithMacro2");
    expectLocation(1, &str, nullptr, loc);
}

TEST_F(LocationTest, LineDirectiveWithPredefinedMacro)
{
    const char *str =
        "#line __LINE__ __FILE__\n"
        "foo";
    pp::SourceLocation loc(0, 1);

    SCOPED_TRACE("LineDirectiveWithMacro");
    expectLocation(1, &str, nullptr, loc);
}

TEST_F(LocationTest, LineDirectiveNewlineBeforeStringBreak)
{
    const char *const str[] = {"#line 10 20\n", "foo"};
    // String number is incremented after it is set by the line directive.
    // Also notice that line number is reset after the string break.
    pp::SourceLocation loc(21, 1);

    SCOPED_TRACE("LineDirectiveNewlineBeforeStringBreak");
    expectLocation(2, str, nullptr, loc);
}

TEST_F(LocationTest, LineDirectiveNewlineAfterStringBreak)
{
    const char *const str[] = {"#line 10 20", "\nfoo"};
    // String number is incremented before it is set by the line directive.
    pp::SourceLocation loc(20, 10);

    SCOPED_TRACE("LineDirectiveNewlineAfterStringBreak");
    expectLocation(2, str, nullptr, loc);
}

TEST_F(LocationTest, LineDirectiveMissingNewline)
{
    const char *str = "#line 10";
    ASSERT_TRUE(mPreprocessor.init(1, &str, nullptr));

    using testing::_;
    // Error reported about EOF.
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_EOF_IN_DIRECTIVE, _, _));

    pp::Token token;
    mPreprocessor.lex(&token);
}

// Test for an error being generated when the line number overflows - regular version
TEST_F(LocationTest, LineOverflowRegular)
{
    const char *str = "#line 0x7FFFFFFF\n\n";

    ASSERT_TRUE(mPreprocessor.init(1, &str, nullptr));

    using testing::_;
    // Error reported about EOF.
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_TOKENIZER_ERROR, _, _));

    pp::Token token;
    mPreprocessor.lex(&token);
}

// Test for an error being generated when the line number overflows - inside /* */ comment version
TEST_F(LocationTest, LineOverflowInComment)
{
    const char *str = "#line 0x7FFFFFFF\n/*\n*/";

    ASSERT_TRUE(mPreprocessor.init(1, &str, nullptr));

    using testing::_;
    // Error reported about EOF.
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_TOKENIZER_ERROR, _, _));

    pp::Token token;
    mPreprocessor.lex(&token);
}

// Test for an error being generated when the line number overflows - inside \n continuation
// version
TEST_F(LocationTest, LineOverflowInContinuationN)
{
    const char *str = "#line 0x7FFFFFFF\n \\\n\n";

    ASSERT_TRUE(mPreprocessor.init(1, &str, nullptr));

    using testing::_;
    // Error reported about EOF.
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_TOKENIZER_ERROR, _, _));

    pp::Token token;
    mPreprocessor.lex(&token);
}

// Test for an error being generated when the line number overflows - inside \r\n continuation
// version
TEST_F(LocationTest, LineOverflowInContinuationRN)
{
    const char *str = "#line 0x7FFFFFFF\n \\\r\n\n";

    ASSERT_TRUE(mPreprocessor.init(1, &str, nullptr));

    using testing::_;
    // Error reported about EOF.
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_TOKENIZER_ERROR, _, _));

    pp::Token token;
    mPreprocessor.lex(&token);
}

struct LineTestParam
{
    const char *str;
    pp::Diagnostics::ID id;
};

class InvalidLineTest : public LocationTest, public testing::WithParamInterface<LineTestParam>
{};

TEST_P(InvalidLineTest, Identified)
{
    LineTestParam param = GetParam();
    ASSERT_TRUE(mPreprocessor.init(1, &param.str, nullptr));

    using testing::_;
    // Invalid line directive call.
    EXPECT_CALL(mDiagnostics, print(param.id, pp::SourceLocation(0, 1), _));

    pp::Token token;
    mPreprocessor.lex(&token);
}

static const LineTestParam kParams[] = {
    {"#line\n", pp::Diagnostics::PP_INVALID_LINE_DIRECTIVE},
    {"#line foo\n", pp::Diagnostics::PP_INVALID_LINE_NUMBER},
    {"#line defined(foo)\n", pp::Diagnostics::PP_INVALID_LINE_NUMBER},
    {"#line 10 foo\n", pp::Diagnostics::PP_INVALID_FILE_NUMBER},
    {"#line 10 20 foo\n", pp::Diagnostics::PP_UNEXPECTED_TOKEN},
    {"#line 0xffffffff\n", pp::Diagnostics::PP_INTEGER_OVERFLOW},
    {"#line 10 0xffffffff\n", pp::Diagnostics::PP_INTEGER_OVERFLOW}};

INSTANTIATE_TEST_SUITE_P(All, InvalidLineTest, testing::ValuesIn(kParams));

struct LineExpressionTestParam
{
    const char *expression;
    int expectedLine;
};

class LineExpressionTest : public LocationTest,
                           public testing::WithParamInterface<LineExpressionTestParam>
{};

TEST_P(LineExpressionTest, ExpressionEvaluation)
{
    LineExpressionTestParam param = GetParam();
    const char *strs[3]           = {"#line ", param.expression, "\nfoo"};

    pp::SourceLocation loc(2, param.expectedLine);

    expectLocation(3, strs, nullptr, loc);
}

static const LineExpressionTestParam kParamsLineExpressionTest[] = {
    {"1 + 2", 3},  {"5 - 3", 2},        {"7 * 11", 77},  {"20 / 10", 2},  {"10 % 5", 0},
    {"7 && 3", 1}, {"7 || 0", 1},       {"11 == 11", 1}, {"11 != 11", 0}, {"11 > 7", 1},
    {"11 < 7", 0}, {"11 >= 7", 1},      {"11 <= 7", 0},  {"!11", 0},      {"-1", -1},
    {"+9", 9},     {"(1 + 2) * 4", 12}, {"3 | 5", 7},    {"3 ^ 5", 6},    {"3 & 5", 1},
    {"~5", ~5},    {"2 << 3", 16},      {"16 >> 2", 4}};

INSTANTIATE_TEST_SUITE_P(All, LineExpressionTest, testing::ValuesIn(kParamsLineExpressionTest));

}  // namespace angle
