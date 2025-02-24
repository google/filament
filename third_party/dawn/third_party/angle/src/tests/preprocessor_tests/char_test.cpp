//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <algorithm>
#include <climits>

#include "PreprocessorTest.h"
#include "compiler/preprocessor/Token.h"

namespace angle
{

class CharTest : public PreprocessorTest, public testing::WithParamInterface<int>
{
  public:
    CharTest() : PreprocessorTest(SH_GLES2_SPEC) {}
};

static const char kPunctuators[] = {'.', '+', '-', '/', '*', '%', '<', '>', '[', ']', '(', ')',
                                    '{', '}', '^', '|', '&', '~', '=', '!', ':', ';', ',', '?'};
static const int kNumPunctuators = sizeof(kPunctuators) / sizeof(kPunctuators[0]);

bool isPunctuator(char c)
{
    static const char *kPunctuatorBeg = kPunctuators;
    static const char *kPunctuatorEnd = kPunctuators + kNumPunctuators;
    return std::find(kPunctuatorBeg, kPunctuatorEnd, c) != kPunctuatorEnd;
}

static const char kWhitespaces[] = {' ', '\t', '\v', '\f', '\n', '\r'};
static const int kNumWhitespaces = sizeof(kWhitespaces) / sizeof(kWhitespaces[0]);

bool isWhitespace(char c)
{
    static const char *kWhitespaceBeg = kWhitespaces;
    static const char *kWhitespaceEnd = kWhitespaces + kNumWhitespaces;
    return std::find(kWhitespaceBeg, kWhitespaceEnd, c) != kWhitespaceEnd;
}

TEST_P(CharTest, Identified)
{
    std::string str(1, static_cast<char>(GetParam()));
    const char *cstr = str.c_str();
    int length       = 1;

    // Note that we pass the length param as well because the invalid
    // string may contain the null character.
    ASSERT_TRUE(mPreprocessor.init(1, &cstr, &length));

    int expectedType = pp::Token::LAST;
    std::string expectedValue;

    if (str[0] == '#')
    {
        // Lone '#' is ignored.
    }
    else if ((str[0] == '_') || ((str[0] >= 'a') && (str[0] <= 'z')) ||
             ((str[0] >= 'A') && (str[0] <= 'Z')))
    {
        expectedType  = pp::Token::IDENTIFIER;
        expectedValue = str;
    }
    else if (str[0] >= '0' && str[0] <= '9')
    {
        expectedType  = pp::Token::CONST_INT;
        expectedValue = str;
    }
    else if (isPunctuator(str[0]))
    {
        expectedType  = str[0];
        expectedValue = str;
    }
    else if (isWhitespace(str[0]))
    {
        // Whitespace is ignored.
    }
    else
    {
        // Everything else is invalid.
        using testing::_;
        EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_INVALID_CHARACTER, _, str));
    }

    pp::Token token;
    mPreprocessor.lex(&token);
    EXPECT_EQ(expectedType, token.type);
    EXPECT_EQ(expectedValue, token.text);
}

// Note +1 for the max-value in range. It is there because the max-value
// not included in the range.
INSTANTIATE_TEST_SUITE_P(All, CharTest, testing::Range(CHAR_MIN, CHAR_MAX + 1));

}  // namespace angle
