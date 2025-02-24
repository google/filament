//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "PreprocessorTest.h"
#include "compiler/preprocessor/Token.h"

namespace angle
{

class IfTest : public SimplePreprocessorTest
{};

TEST_F(IfTest, If_0)
{
    const char *str =
        "pass_1\n"
        "#if 0\n"
        "fail\n"
        "#endif\n"
        "pass_2\n";
    const char *expected =
        "pass_1\n"
        "\n"
        "\n"
        "\n"
        "pass_2\n";

    preprocess(str, expected);
}

TEST_F(IfTest, If_1)
{
    const char *str =
        "pass_1\n"
        "#if 1\n"
        "pass_2\n"
        "#endif\n"
        "pass_3\n";
    const char *expected =
        "pass_1\n"
        "\n"
        "pass_2\n"
        "\n"
        "pass_3\n";

    preprocess(str, expected);
}

TEST_F(IfTest, If_0_Else)
{
    const char *str =
        "pass_1\n"
        "#if 0\n"
        "fail\n"
        "#else\n"
        "pass_2\n"
        "#endif\n"
        "pass_3\n";
    const char *expected =
        "pass_1\n"
        "\n"
        "\n"
        "\n"
        "pass_2\n"
        "\n"
        "pass_3\n";

    preprocess(str, expected);
}

TEST_F(IfTest, If_1_Else)
{
    const char *str =
        "pass_1\n"
        "#if 1\n"
        "pass_2\n"
        "#else\n"
        "fail\n"
        "#endif\n"
        "pass_3\n";
    const char *expected =
        "pass_1\n"
        "\n"
        "pass_2\n"
        "\n"
        "\n"
        "\n"
        "pass_3\n";

    preprocess(str, expected);
}

TEST_F(IfTest, If_0_Elif)
{
    const char *str =
        "pass_1\n"
        "#if 0\n"
        "fail_1\n"
        "#elif 0\n"
        "fail_2\n"
        "#elif 1\n"
        "pass_2\n"
        "#elif 1\n"
        "fail_3\n"
        "#else\n"
        "fail_4\n"
        "#endif\n"
        "pass_3\n";
    const char *expected =
        "pass_1\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "pass_2\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "pass_3\n";

    preprocess(str, expected);
}

TEST_F(IfTest, If_1_Elif)
{
    const char *str =
        "pass_1\n"
        "#if 1\n"
        "pass_2\n"
        "#elif 0\n"
        "fail_1\n"
        "#elif 1\n"
        "fail_2\n"
        "#else\n"
        "fail_4\n"
        "#endif\n"
        "pass_3\n";
    const char *expected =
        "pass_1\n"
        "\n"
        "pass_2\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "pass_3\n";

    preprocess(str, expected);
}

TEST_F(IfTest, If_Elif_Else)
{
    const char *str =
        "pass_1\n"
        "#if 0\n"
        "fail_1\n"
        "#elif 0\n"
        "fail_2\n"
        "#elif 0\n"
        "fail_3\n"
        "#else\n"
        "pass_2\n"
        "#endif\n"
        "pass_3\n";
    const char *expected =
        "pass_1\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "pass_2\n"
        "\n"
        "pass_3\n";

    preprocess(str, expected);
}

TEST_F(IfTest, If_0_Nested)
{
    const char *str =
        "pass_1\n"
        "#if 0\n"
        "fail_1\n"
        "#if 1\n"
        "fail_2\n"
        "#else\n"
        "fail_3\n"
        "#endif\n"
        "#else\n"
        "pass_2\n"
        "#endif\n"
        "pass_3\n";
    const char *expected =
        "pass_1\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "pass_2\n"
        "\n"
        "pass_3\n";

    preprocess(str, expected);
}

TEST_F(IfTest, If_1_Nested)
{
    const char *str =
        "pass_1\n"
        "#if 1\n"
        "pass_2\n"
        "#if 1\n"
        "pass_3\n"
        "#else\n"
        "fail_1\n"
        "#endif\n"
        "#else\n"
        "fail_2\n"
        "#endif\n"
        "pass_4\n";
    const char *expected =
        "pass_1\n"
        "\n"
        "pass_2\n"
        "\n"
        "pass_3\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "pass_4\n";

    preprocess(str, expected);
}

TEST_F(IfTest, OperatorPrecedence)
{
    const char *str =
        "#if 1 + 2 * 3 + - (26 % 17 - + 4 / 2)\n"
        "fail_1\n"
        "#else\n"
        "pass_1\n"
        "#endif\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "pass_1\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, OperatorDefined)
{
    const char *str =
        "#if defined foo\n"
        "fail_1\n"
        "#else\n"
        "pass_1\n"
        "#endif\n"
        "#define foo\n"
        "#if defined(foo)\n"
        "pass_2\n"
        "#else\n"
        "fail_2\n"
        "#endif\n"
        "#undef foo\n"
        "#if defined ( foo ) \n"
        "fail_3\n"
        "#else\n"
        "pass_3\n"
        "#endif\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "pass_1\n"
        "\n"
        "\n"
        "\n"
        "pass_2\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "pass_3\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, OperatorEQ)
{
    const char *str =
        "#if 4 - 1 == 2 + 1\n"
        "pass\n"
        "#else\n"
        "fail\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n"
        "\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, OperatorNE)
{
    const char *str =
        "#if 1 != 2\n"
        "pass\n"
        "#else\n"
        "fail\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n"
        "\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, OperatorLess)
{
    const char *str =
        "#if 1 < 2\n"
        "pass\n"
        "#else\n"
        "fail\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n"
        "\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, OperatorGreater)
{
    const char *str =
        "#if 2 > 1\n"
        "pass\n"
        "#else\n"
        "fail\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n"
        "\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, OperatorLE)
{
    const char *str =
        "#if 1 <= 2\n"
        "pass_1\n"
        "#else\n"
        "fail_1\n"
        "#endif\n"
        "#if 2 <= 2\n"
        "pass_2\n"
        "#else\n"
        "fail_2\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass_1\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "pass_2\n"
        "\n"
        "\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, OperatorGE)
{
    const char *str =
        "#if 2 >= 1\n"
        "pass_1\n"
        "#else\n"
        "fail_1\n"
        "#endif\n"
        "#if 2 >= 2\n"
        "pass_2\n"
        "#else\n"
        "fail_2\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass_1\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "pass_2\n"
        "\n"
        "\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, OperatorBitwiseOR)
{
    const char *str =
        "#if (0xaaaaaaaa | 0x55555555) == 0xffffffff\n"
        "pass\n"
        "#else\n"
        "fail\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n"
        "\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, OperatorBitwiseAND)
{
    const char *str =
        "#if (0xaaaaaaa & 0x5555555) == 0\n"
        "pass\n"
        "#else\n"
        "fail\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n"
        "\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, OperatorBitwiseXOR)
{
    const char *str =
        "#if (0xaaaaaaa ^ 0x5555555) == 0xfffffff\n"
        "pass\n"
        "#else\n"
        "fail\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n"
        "\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, OperatorBitwiseComplement)
{
    const char *str =
        "#if (~ 0xdeadbeef) == -3735928560\n"
        "pass\n"
        "#else\n"
        "fail\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n"
        "\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, OperatorLeft)
{
    const char *str =
        "#if (1 << 12) == 4096\n"
        "pass\n"
        "#else\n"
        "fail\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n"
        "\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, OperatorRight)
{
    const char *str =
        "#if (31762 >> 8) == 124\n"
        "pass\n"
        "#else\n"
        "fail\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n"
        "\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, ExpressionWithMacros)
{
    const char *str =
        "#define one 1\n"
        "#define two 2\n"
        "#define three 3\n"
        "#if one + two == three\n"
        "pass\n"
        "#else\n"
        "fail\n"
        "#endif\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "\n"
        "pass\n"
        "\n"
        "\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, JunkInsideExcludedBlockIgnored)
{
    const char *str =
        "#if 0\n"
        "foo !@#$%^&* .1bar\n"
        "#foo\n"
        "#if bar\n"
        "fail\n"
        "#endif\n"
        "#else\n"
        "pass\n"
        "#endif\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "pass\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, Ifdef)
{
    const char *str =
        "#define foo\n"
        "#ifdef foo\n"
        "pass_1\n"
        "#else\n"
        "fail_1\n"
        "#endif\n"
        "#undef foo\n"
        "#ifdef foo\n"
        "fail_2\n"
        "#else\n"
        "pass_2\n"
        "#endif\n";
    const char *expected =
        "\n"
        "\n"
        "pass_1\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "pass_2\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, Ifndef)
{
    const char *str =
        "#define foo\n"
        "#ifndef foo\n"
        "fail_1\n"
        "#else\n"
        "pass_1\n"
        "#endif\n"
        "#undef foo\n"
        "#ifndef foo\n"
        "pass_2\n"
        "#else\n"
        "fail_2\n"
        "#endif\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "\n"
        "pass_1\n"
        "\n"
        "\n"
        "\n"
        "pass_2\n"
        "\n"
        "\n"
        "\n";

    preprocess(str, expected);
}

TEST_F(IfTest, MissingExpression)
{
    const char *str =
        "#if\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_INVALID_EXPRESSION,
                                    pp::SourceLocation(0, 1), "syntax error"));

    preprocess(str);
}

TEST_F(IfTest, DivisionByZero)
{
    const char *str =
        "#if 1 / (3 - (1 + 2))\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_DIVISION_BY_ZERO, pp::SourceLocation(0, 1), "1 / 0"));

    preprocess(str);
}

TEST_F(IfTest, ModuloByZero)
{
    const char *str =
        "#if 1 % (3 - (1 + 2))\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_DIVISION_BY_ZERO, pp::SourceLocation(0, 1), "1 % 0"));

    preprocess(str);
}

TEST_F(IfTest, DecIntegerOverflow)
{
    const char *str =
        "#if 4294967296\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_INTEGER_OVERFLOW, pp::SourceLocation(0, 1),
                                    "4294967296"));

    preprocess(str);
}

TEST_F(IfTest, OctIntegerOverflow)
{
    const char *str =
        "#if 077777777777\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_INTEGER_OVERFLOW, pp::SourceLocation(0, 1),
                                    "077777777777"));

    preprocess(str);
}

TEST_F(IfTest, HexIntegerOverflow)
{
    const char *str =
        "#if 0xfffffffff\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_INTEGER_OVERFLOW, pp::SourceLocation(0, 1),
                                    "0xfffffffff"));

    preprocess(str);
}

TEST_F(IfTest, UndefinedMacro)
{
    const char *str =
        "#if UNDEFINED\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_CONDITIONAL_UNEXPECTED_TOKEN,
                                    pp::SourceLocation(0, 1), "UNDEFINED"));

    preprocess(str);
}

TEST_F(IfTest, InvalidExpressionIgnoredForExcludedElif)
{
    const char *str =
        "#if 1\n"
        "pass\n"
        "#elif UNDEFINED\n"
        "fail\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n"
        "\n"
        "\n";

    // No error or warning.
    using testing::_;
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(str, expected);
}

TEST_F(IfTest, ElseWithoutIf)
{
    const char *str = "#else\n";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_CONDITIONAL_ELSE_WITHOUT_IF,
                                    pp::SourceLocation(0, 1), "else"));

    preprocess(str);
}

TEST_F(IfTest, ElifWithoutIf)
{
    const char *str = "#elif 1\n";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_CONDITIONAL_ELIF_WITHOUT_IF,
                                    pp::SourceLocation(0, 1), "elif"));

    preprocess(str);
}

TEST_F(IfTest, EndifWithoutIf)
{
    const char *str = "#endif\n";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_CONDITIONAL_ENDIF_WITHOUT_IF,
                                    pp::SourceLocation(0, 1), "endif"));

    preprocess(str);
}

TEST_F(IfTest, ElseAfterElse)
{
    const char *str =
        "#if 1\n"
        "#else\n"
        "#else\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_CONDITIONAL_ELSE_AFTER_ELSE,
                                    pp::SourceLocation(0, 3), "else"));

    preprocess(str);
}

TEST_F(IfTest, ElifAfterElse)
{
    const char *str =
        "#if 1\n"
        "#else\n"
        "#elif 0\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_CONDITIONAL_ELIF_AFTER_ELSE,
                                    pp::SourceLocation(0, 3), "elif"));

    preprocess(str);
}

TEST_F(IfTest, UnterminatedIf)
{
    const char *str = "#if 1\n";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_CONDITIONAL_UNTERMINATED,
                                    pp::SourceLocation(0, 1), "if"));

    preprocess(str);
}

TEST_F(IfTest, UnterminatedIfdef)
{
    const char *str = "#ifdef foo\n";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_CONDITIONAL_UNTERMINATED,
                                    pp::SourceLocation(0, 1), "ifdef"));

    preprocess(str);
}

// The preprocessor only allows one expression to follow an #if directive.
// Supplying two integer expressions should be an error.
TEST_F(IfTest, ExtraIntExpression)
{
    const char *str =
        "#if 1 1\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_CONDITIONAL_UNEXPECTED_TOKEN,
                                    pp::SourceLocation(0, 1), "1"));

    preprocess(str);
}

// The preprocessor only allows one expression to follow an #if directive.
// Supplying two expressions where one uses a preprocessor define should be an error.
TEST_F(IfTest, ExtraIdentifierExpression)
{
    const char *str =
        "#define one 1\n"
        "#if 1 one\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_CONDITIONAL_UNEXPECTED_TOKEN,
                                    pp::SourceLocation(0, 2), "1"));

    preprocess(str);
}

// Divide by zero that's not evaluated because of short-circuiting should not cause an error.
TEST_F(IfTest, ShortCircuitedDivideByZero)
{
    const char *str =
        "#if 1 || (2 / 0)\n"
        "pass\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n";

    preprocess(str, expected);
}

// Undefined identifier that's not evaluated because of short-circuiting should not cause an error.
TEST_F(IfTest, ShortCircuitedUndefined)
{
    const char *str =
        "#if 1 || UNDEFINED\n"
        "pass\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n";

    preprocess(str, expected);
}

// Defined operator produced by macro expansion has undefined behavior according to C++ spec,
// which the GLSL spec references (see C++14 draft spec section 16.1.4), but this behavior is
// needed for passing dEQP tests, which enforce stricter compatibility between implementations.
TEST_F(IfTest, DefinedOperatorValidAfterMacroExpansion)
{
    const char *str =
        "#define foo defined\n"
        "#if !foo bar\n"
        "pass\n"
        "#endif\n";

    preprocess(str, "\n\npass\n\n");
}

// Validate the defined operator is evaluated when the macro is called, not when defined.
TEST_F(IfTest, DefinedOperatorValidWhenUsed)
{
    constexpr char str[] = R"(#define BBB 1
#define AAA defined(BBB)
#undef BBB

#if !AAA
pass
#endif
)";

    preprocess(str, "\n\n\n\n\npass\n\n");
}

// Validate the defined operator is evaluated when the macro is called, not when defined.
TEST_F(IfTest, DefinedOperatorAfterMacro)
{
    constexpr char str[] = R"(#define AAA defined(BBB)
#define BBB 1

#if AAA
pass
#endif
)";

    preprocess(str, "\n\n\n\npass\n\n");
}

// Test generating "defined" by concatenation when a macro is called. This is not allowed.
TEST_F(IfTest, DefinedInMacroConcatenationNotAllowed)
{
    constexpr char str[] = R"(#define BBB 1
#define AAA(defi, ned) defi ## ned(BBB)

#if !AAA(defi, ned)
pass
#endif
)";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_CONDITIONAL_UNEXPECTED_TOKEN,
                                    pp::SourceLocation(0, 4), "defi"));
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_CONDITIONAL_UNEXPECTED_TOKEN,
                                    pp::SourceLocation(0, 4), "#"));

    preprocess(str);
}

// Test using defined in a macro parameter name. This is not allowed.
TEST_F(IfTest, DefinedAsParameterNameNotAllowed)
{
    constexpr char str[] = R"(#define BBB 1
#define AAA(defined) defined(BBB)

#if AAA(defined)
pass
#endif
)";

    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_UNEXPECTED_TOKEN, pp::SourceLocation(0, 0), ""));

    preprocess(str);
}

// This behavour is disabled in WebGL.
TEST_F(IfTest, DefinedOperatorInvalidAfterMacroExpansionInWebGL)
{
    const char *str =
        "#define foo defined\n"
        "#if !foo bar\n"
        "pass\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_CONDITIONAL_UNEXPECTED_TOKEN,
                                    pp::SourceLocation(0, 2), "defined"));
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_CONDITIONAL_UNEXPECTED_TOKEN,
                                    pp::SourceLocation(0, 2), "bar"));

    preprocess(str, pp::PreprocessorSettings(SH_WEBGL_SPEC));
}

// Defined operator produced by macro expansion has undefined behavior according to C++ spec,
// which the GLSL spec references (see C++14 draft spec section 16.1.4). Some edge case
// behaviours with defined are not portable between implementations and thus are not required
// to pass dEQP Tests.
TEST_F(IfTest, UnterminatedDefinedInMacro)
{
    const char *str =
        "#define foo defined(\n"
        "#if foo\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_UNEXPECTED_TOKEN, pp::SourceLocation(0, 2), "\n"));
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_INVALID_EXPRESSION,
                                    pp::SourceLocation(0, 2), "syntax error"));

    preprocess(str);
}

// Defined operator produced by macro expansion has undefined behavior according to C++ spec,
// which the GLSL spec references (see C++14 draft spec section 16.1.4). Some edge case
// behaviours with defined are not portable between implementations and thus are not required
// to pass dEQP Tests.
TEST_F(IfTest, UnterminatedDefinedInMacro2)
{
    const char *str =
        "#define foo defined(bar\n"
        "#if foo\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_UNEXPECTED_TOKEN, pp::SourceLocation(0, 2), "\n"));
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_INVALID_EXPRESSION,
                                    pp::SourceLocation(0, 2), "syntax error"));

    preprocess(str);
}

// Undefined shift: negative shift offset.
TEST_F(IfTest, BitShiftLeftOperatorNegativeOffset)
{
    const char *str =
        "#if 2 << -1 == 1\n"
        "foo\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_UNDEFINED_SHIFT, pp::SourceLocation(0, 1), "2 << -1"));

    preprocess(str);
}

// Undefined shift: shift offset is out of range.
TEST_F(IfTest, BitShiftLeftOperatorOffset32)
{
    const char *str =
        "#if 2 << 32 == 1\n"
        "foo\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_UNDEFINED_SHIFT, pp::SourceLocation(0, 1), "2 << 32"));

    preprocess(str);
}

// Left hand side of shift is negative.
TEST_F(IfTest, BitShiftLeftOperatorNegativeLHS)
{
    const char *str =
        "#if (-2) << 1 == -4\n"
        "pass\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n";

    preprocess(str, expected);
}

// Left shift overflows. Note that the intended result is not explicitly specified, but we assume it
// to do the same operation on the 2's complement bit representation as unsigned shift in C++.
TEST_F(IfTest, BitShiftLeftOverflow)
{
    const char *str =
        "#if (0x10000 + 0x1) << 28 == 0x10000000\n"
        "pass\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n";

    preprocess(str, expected);
}

// Left shift of a negative number overflows. Note that the intended result is not explicitly
// specified, but we assume it to do the same operation on the 2's complement bit representation as
// unsigned shift in C++.
TEST_F(IfTest, BitShiftLeftNegativeOverflow)
{
    // The bit representation of -5 is 11111111 11111111 11111111 11111011.
    // Shifting by 30 leaves:          11000000 00000000 00000000 00000000.
    const char *str =
        "#if (-5) << 30 == -1073741824\n"
        "pass\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n";

    preprocess(str, expected);
}

// Undefined shift: shift offset is out of range.
TEST_F(IfTest, BitShiftRightOperatorNegativeOffset)
{
    const char *str =
        "#if 2 >> -1 == 4\n"
        "foo\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_UNDEFINED_SHIFT, pp::SourceLocation(0, 1), "2 >> -1"));

    preprocess(str);
}

// Undefined shift: shift offset is out of range.
TEST_F(IfTest, BitShiftRightOperatorOffset32)
{
    const char *str =
        "#if 2 >> 32 == 0\n"
        "foo\n"
        "#endif\n";

    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_UNDEFINED_SHIFT, pp::SourceLocation(0, 1), "2 >> 32"));

    preprocess(str);
}

// Left hand side of shift is negative.
TEST_F(IfTest, BitShiftRightOperatorNegativeLHS)
{
    const char *str =
        "#if (-2) >> 1 == 0x7fffffff\n"
        "pass\n"
        "#endif\n";
    const char *expected =
        "\n"
        "pass\n"
        "\n";

    preprocess(str, expected);
}

}  // namespace angle
