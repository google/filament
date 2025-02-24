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

class InvalidNumberTest : public SimplePreprocessorTest,
                          public testing::WithParamInterface<const char *>
{};

TEST_P(InvalidNumberTest, InvalidNumberIdentified)
{
    const char *str = GetParam();

    using testing::_;
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_INVALID_NUMBER, _, str));

    preprocess(str);
}

INSTANTIATE_TEST_SUITE_P(InvalidIntegers, InvalidNumberTest, testing::Values("1a", "08", "0xG"));

INSTANTIATE_TEST_SUITE_P(InvalidFloats,
                         InvalidNumberTest,
                         testing::Values("1eg", "0.a", "0.1.2", ".0a", ".0.1"));

typedef std::tuple<const char *, char> IntegerParams;
class IntegerTest : public SimplePreprocessorTest, public testing::WithParamInterface<IntegerParams>
{};

TEST_P(IntegerTest, Identified)
{
    std::string str(std::get<0>(GetParam()));  // prefix.
    str.push_back(std::get<1>(GetParam()));    // digit.
    const char *cstr = str.c_str();

    pp::Token token;
    lexSingleToken(cstr, &token);
    EXPECT_EQ(pp::Token::CONST_INT, token.type);
    EXPECT_EQ(str, token.text);
}

INSTANTIATE_TEST_SUITE_P(DecimalInteger,
                         IntegerTest,
                         testing::Combine(testing::Values(""), CLOSED_RANGE('0', '9')));

INSTANTIATE_TEST_SUITE_P(OctalInteger,
                         IntegerTest,
                         testing::Combine(testing::Values("0"), CLOSED_RANGE('0', '7')));

INSTANTIATE_TEST_SUITE_P(HexadecimalInteger_0_9,
                         IntegerTest,
                         testing::Combine(testing::Values("0x", "0X"), CLOSED_RANGE('0', '9')));

INSTANTIATE_TEST_SUITE_P(HexadecimalInteger_a_f,
                         IntegerTest,
                         testing::Combine(testing::Values("0x", "0X"), CLOSED_RANGE('a', 'f')));

INSTANTIATE_TEST_SUITE_P(HexadecimalInteger_A_F,
                         IntegerTest,
                         testing::Combine(testing::Values("0x", "0X"), CLOSED_RANGE('A', 'F')));

class FloatTest : public SimplePreprocessorTest
{
  protected:
    void expectFloat(const std::string &str)
    {
        const char *cstr = str.c_str();

        pp::Token token;
        lexSingleToken(cstr, &token);
        EXPECT_EQ(pp::Token::CONST_FLOAT, token.type);
        EXPECT_EQ(str, token.text);
    }
};

typedef std::tuple<char, char, const char *, char> FloatScientificParams;
class FloatScientificTest : public FloatTest,
                            public testing::WithParamInterface<FloatScientificParams>
{};

// This test covers floating point numbers of form [0-9][eE][+-]?[0-9].
TEST_P(FloatScientificTest, FloatIdentified)
{
    std::string str;
    str.push_back(std::get<0>(GetParam()));  // significand [0-9].
    str.push_back(std::get<1>(GetParam()));  // separator [eE].
    str.append(std::get<2>(GetParam()));     // sign [" " "+" "-"].
    str.push_back(std::get<3>(GetParam()));  // exponent [0-9].

    SCOPED_TRACE("FloatScientificTest");
    expectFloat(str);
}

INSTANTIATE_TEST_SUITE_P(FloatScientific,
                         FloatScientificTest,
                         testing::Combine(CLOSED_RANGE('0', '9'),
                                          testing::Values('e', 'E'),
                                          testing::Values("", "+", "-"),
                                          CLOSED_RANGE('0', '9')));

typedef std::tuple<char, char> FloatFractionParams;
class FloatFractionTest : public FloatTest, public testing::WithParamInterface<FloatFractionParams>
{};

// This test covers floating point numbers of form [0-9]"." and [0-9]?"."[0-9].
TEST_P(FloatFractionTest, FloatIdentified)
{
    std::string str;

    char significand = std::get<0>(GetParam());
    if (significand != '\0')
        str.push_back(significand);

    str.push_back('.');

    char fraction = std::get<1>(GetParam());
    if (fraction != '\0')
        str.push_back(fraction);

    SCOPED_TRACE("FloatFractionTest");
    expectFloat(str);
}

INSTANTIATE_TEST_SUITE_P(FloatFraction_X_X,
                         FloatFractionTest,
                         testing::Combine(CLOSED_RANGE('0', '9'), CLOSED_RANGE('0', '9')));

INSTANTIATE_TEST_SUITE_P(FloatFraction_0_X,
                         FloatFractionTest,
                         testing::Combine(testing::Values('\0'), CLOSED_RANGE('0', '9')));

INSTANTIATE_TEST_SUITE_P(FloatFraction_X_0,
                         FloatFractionTest,
                         testing::Combine(CLOSED_RANGE('0', '9'), testing::Values('\0')));

// In the tests above we have tested individual parts of a float separately.
// This test has all parts of a float.
TEST_F(FloatTest, FractionScientific)
{
    SCOPED_TRACE("FractionScientific");
    expectFloat("0.1e+2");
}

}  // namespace angle
