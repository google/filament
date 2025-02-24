//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <tuple>

#include "PreprocessorTest.h"
#include "compiler/preprocessor/Token.h"

namespace angle
{

#define CLOSED_RANGE(x, y) testing::Range(x, static_cast<char>((y) + 1))

class IdentifierTest : public SimplePreprocessorTest
{
  protected:
    void expectIdentifier(const std::string &str)
    {
        const char *cstr = str.c_str();

        pp::Token token;
        lexSingleToken(cstr, &token);
        EXPECT_EQ(pp::Token::IDENTIFIER, token.type);
        EXPECT_EQ(str, token.text);
    }
};

class SingleLetterIdentifierTest : public IdentifierTest, public testing::WithParamInterface<char>
{};

// This test covers identifier names of form [_a-zA-Z].
TEST_P(SingleLetterIdentifierTest, Identified)
{
    std::string str(1, GetParam());
    expectIdentifier(str);
}

// Test string: '_'
INSTANTIATE_TEST_SUITE_P(Underscore, SingleLetterIdentifierTest, testing::Values('_'));

// Test string: [a-z]
INSTANTIATE_TEST_SUITE_P(a_z, SingleLetterIdentifierTest, CLOSED_RANGE('a', 'z'));

// Test string: [A-Z]
INSTANTIATE_TEST_SUITE_P(A_Z, SingleLetterIdentifierTest, CLOSED_RANGE('A', 'Z'));

typedef std::tuple<char, char> IdentifierParams;
class DoubleLetterIdentifierTest : public IdentifierTest,
                                   public testing::WithParamInterface<IdentifierParams>
{};

// This test covers identifier names of form [_a-zA-Z][_a-zA-Z0-9].
TEST_P(DoubleLetterIdentifierTest, Identified)
{
    std::string str;
    str.push_back(std::get<0>(GetParam()));
    str.push_back(std::get<1>(GetParam()));

    expectIdentifier(str);
}

// Test string: "__"
INSTANTIATE_TEST_SUITE_P(Underscore_Underscore,
                         DoubleLetterIdentifierTest,
                         testing::Combine(testing::Values('_'), testing::Values('_')));

// Test string: "_"[a-z]
INSTANTIATE_TEST_SUITE_P(Underscore_a_z,
                         DoubleLetterIdentifierTest,
                         testing::Combine(testing::Values('_'), CLOSED_RANGE('a', 'z')));

// Test string: "_"[A-Z]
INSTANTIATE_TEST_SUITE_P(Underscore_A_Z,
                         DoubleLetterIdentifierTest,
                         testing::Combine(testing::Values('_'), CLOSED_RANGE('A', 'Z')));

// Test string: "_"[0-9]
INSTANTIATE_TEST_SUITE_P(Underscore_0_9,
                         DoubleLetterIdentifierTest,
                         testing::Combine(testing::Values('_'), CLOSED_RANGE('0', '9')));

// Test string: [a-z]"_"
INSTANTIATE_TEST_SUITE_P(a_z_Underscore,
                         DoubleLetterIdentifierTest,
                         testing::Combine(CLOSED_RANGE('a', 'z'), testing::Values('_')));

// Test string: [a-z][a-z]
INSTANTIATE_TEST_SUITE_P(a_z_a_z,
                         DoubleLetterIdentifierTest,
                         testing::Combine(CLOSED_RANGE('a', 'z'), CLOSED_RANGE('a', 'z')));

// Test string: [a-z][A-Z]
INSTANTIATE_TEST_SUITE_P(a_z_A_Z,
                         DoubleLetterIdentifierTest,
                         testing::Combine(CLOSED_RANGE('a', 'z'), CLOSED_RANGE('A', 'Z')));

// Test string: [a-z][0-9]
INSTANTIATE_TEST_SUITE_P(a_z_0_9,
                         DoubleLetterIdentifierTest,
                         testing::Combine(CLOSED_RANGE('a', 'z'), CLOSED_RANGE('0', '9')));

// Test string: [A-Z]"_"
INSTANTIATE_TEST_SUITE_P(A_Z_Underscore,
                         DoubleLetterIdentifierTest,
                         testing::Combine(CLOSED_RANGE('A', 'Z'), testing::Values('_')));

// Test string: [A-Z][a-z]
INSTANTIATE_TEST_SUITE_P(A_Z_a_z,
                         DoubleLetterIdentifierTest,
                         testing::Combine(CLOSED_RANGE('A', 'Z'), CLOSED_RANGE('a', 'z')));

// Test string: [A-Z][A-Z]
INSTANTIATE_TEST_SUITE_P(A_Z_A_Z,
                         DoubleLetterIdentifierTest,
                         testing::Combine(CLOSED_RANGE('A', 'Z'), CLOSED_RANGE('A', 'Z')));

// Test string: [A-Z][0-9]
INSTANTIATE_TEST_SUITE_P(A_Z_0_9,
                         DoubleLetterIdentifierTest,
                         testing::Combine(CLOSED_RANGE('A', 'Z'), CLOSED_RANGE('0', '9')));

// The tests above cover one-letter and various combinations of two-letter
// identifier names. This test covers all characters in a single string.
TEST_F(IdentifierTest, AllLetters)
{
    std::string str;
    for (char c = 'a'; c <= 'z'; ++c)
        str.push_back(c);

    str.push_back('_');

    for (char c = 'A'; c <= 'Z'; ++c)
        str.push_back(c);

    str.push_back('_');

    for (char c = '0'; c <= '9'; ++c)
        str.push_back(c);

    expectIdentifier(str);
}

}  // namespace angle
