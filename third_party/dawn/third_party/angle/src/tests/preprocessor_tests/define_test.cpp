//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <sstream>

#include "PreprocessorTest.h"
#include "compiler/preprocessor/Token.h"

namespace angle
{

using testing::_;

class DefineTest : public SimplePreprocessorTest
{};

TEST_F(DefineTest, NonIdentifier)
{
    const char *input =
        "#define 2 foo\n"
        "2\n";
    const char *expected =
        "\n"
        "2\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_UNEXPECTED_TOKEN, pp::SourceLocation(0, 1), "2"));

    preprocess(input, expected);
}

TEST_F(DefineTest, RedefinePredefined)
{
    const char *input =
        "#define __LINE__ 10\n"
        "__LINE__\n"
        "#define __FILE__ 20\n"
        "__FILE__\n"
        "#define __VERSION__ 200\n"
        "__VERSION__\n"
        "#define GL_ES 0\n"
        "GL_ES\n";
    const char *expected =
        "\n"
        "2\n"
        "\n"
        "0\n"
        "\n"
        "100\n"
        "\n"
        "1\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_MACRO_PREDEFINED_REDEFINED,
                                    pp::SourceLocation(0, 1), "__LINE__"));
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_MACRO_PREDEFINED_REDEFINED,
                                    pp::SourceLocation(0, 3), "__FILE__"));
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_MACRO_PREDEFINED_REDEFINED,
                                    pp::SourceLocation(0, 5), "__VERSION__"));
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_MACRO_PREDEFINED_REDEFINED,
                                    pp::SourceLocation(0, 7), "GL_ES"));

    preprocess(input, expected);
}

TEST_F(DefineTest, ReservedUnderScore1)
{
    const char *input =
        "#define __foo bar\n"
        "__foo\n";
    const char *expected =
        "\n"
        "bar\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_WARNING_MACRO_NAME_RESERVED,
                                    pp::SourceLocation(0, 1), "__foo"));

    preprocess(input, expected);
}

TEST_F(DefineTest, ReservedUnderScore2)
{
    const char *input =
        "#define foo__bar baz\n"
        "foo__bar\n";
    const char *expected =
        "\n"
        "baz\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_WARNING_MACRO_NAME_RESERVED,
                                    pp::SourceLocation(0, 1), "foo__bar"));

    preprocess(input, expected);
}

TEST_F(DefineTest, ReservedGL)
{
    const char *input =
        "#define GL_foo bar\n"
        "GL_foo\n";
    const char *expected =
        "\n"
        "GL_foo\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_MACRO_NAME_RESERVED, pp::SourceLocation(0, 1), "GL_foo"));

    preprocess(input, expected);
}

TEST_F(DefineTest, ObjRedefineValid)
{
    const char *input =
        "#define foo (1-1)\n"
        "#define foo /* whitespace */ (1-1) /* other */ \n"
        "foo\n";
    const char *expected =
        "\n"
        "\n"
        "(1-1)\n";
    // No error or warning.
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(input, expected);
}

TEST_F(DefineTest, ObjRedefineInvalid)
{
    const char *input =
        "#define foo (0)\n"
        "#define foo (1-1)\n"
        "foo\n";
    const char *expected =
        "\n"
        "\n"
        "(0)\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_MACRO_REDEFINED, pp::SourceLocation(0, 2), "foo"));

    preprocess(input, expected);
}

TEST_F(DefineTest, FuncRedefineValid)
{
    const char *input =
        "#define foo(a) ( a )\n"
        "#define foo( a )( /* whitespace */ a /* other */ )\n"
        "foo(b)\n";
    const char *expected =
        "\n"
        "\n"
        "( b )\n";
    // No error or warning.
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(input, expected);
}

TEST_F(DefineTest, FuncRedefineInvalid)
{
    const char *input =
        "#define foo(b) ( a )\n"
        "#define foo(b) ( b )\n"
        "foo(1)\n";
    const char *expected =
        "\n"
        "\n"
        "( a )\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_MACRO_REDEFINED, pp::SourceLocation(0, 2), "foo"));

    preprocess(input, expected);
}

TEST_F(DefineTest, ObjBasic)
{
    const char *input =
        "#define foo 1\n"
        "foo\n";
    const char *expected =
        "\n"
        "1\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ObjEmpty)
{
    const char *input =
        "#define foo\n"
        "foo\n";
    const char *expected =
        "\n"
        "\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ObjChain)
{
    const char *input =
        "#define foo 1\n"
        "#define bar foo\n"
        "bar\n";
    const char *expected =
        "\n"
        "\n"
        "1\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ObjChainReverse)
{
    const char *input =
        "#define bar foo\n"
        "#define foo 1\n"
        "bar\n";
    const char *expected =
        "\n"
        "\n"
        "1\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ObjRecursive)
{
    const char *input =
        "#define foo bar\n"
        "#define bar baz\n"
        "#define baz foo\n"
        "foo\n"
        "bar\n"
        "baz\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "foo\n"
        "bar\n"
        "baz\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ObjCompositeChain)
{
    const char *input =
        "#define foo 1\n"
        "#define bar a foo\n"
        "bar\n";
    const char *expected =
        "\n"
        "\n"
        "a 1\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ObjCompositeChainReverse)
{
    const char *input =
        "#define bar a foo\n"
        "#define foo 1\n"
        "bar\n";
    const char *expected =
        "\n"
        "\n"
        "a 1\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ObjCompositeRecursive)
{
    const char *input =
        "#define foo a bar\n"
        "#define bar b baz\n"
        "#define baz c foo\n"
        "foo\n"
        "bar\n"
        "baz\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "a b c foo\n"
        "b c a bar\n"
        "c a b baz\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ObjChainSelfRecursive)
{
    const char *input =
        "#define foo foo\n"
        "#define bar foo\n"
        "bar\n";
    const char *expected =
        "\n"
        "\n"
        "foo\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ObjectLikeWithParens)
{
    const char *input =
        "#define foo ()1\n"
        "foo()\n"
        "#define bar ()2\n"
        "bar()\n";
    const char *expected =
        "\n"
        "()1()\n"
        "\n"
        "()2()\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncEmpty)
{
    const char *input =
        "#define foo()\n"
        "foo()\n";
    const char *expected =
        "\n"
        "\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncNoArgs)
{
    const char *input =
        "#define foo() bar\n"
        "foo()\n";
    const char *expected =
        "\n"
        "bar\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncOneArgUnused)
{
    const char *input =
        "#define foo(x) 1\n"
        "foo(bar)\n";
    const char *expected =
        "\n"
        "1\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncTwoArgsUnused)
{
    const char *input =
        "#define foo(x,y) 1\n"
        "foo(bar,baz)\n";
    const char *expected =
        "\n"
        "1\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncOneArg)
{
    const char *input =
        "#define foo(x) ((x)+1)\n"
        "foo(bar)\n";
    const char *expected =
        "\n"
        "((bar)+1)\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncTwoArgs)
{
    const char *input =
        "#define foo(x,y) ((x)*(y))\n"
        "foo(bar,baz)\n";
    const char *expected =
        "\n"
        "((bar)*(baz))\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncEmptyArgs)
{
    const char *input =
        "#define zero() pass\n"
        "#define one(x) pass\n"
        "#define two(x,y) pass\n"
        "zero()\n"
        "one()\n"
        "two(,)\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "pass\n"
        "pass\n"
        "pass\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncMacroAsParam)
{
    const char *input =
        "#define x 0\n"
        "#define foo(x) x\n"
        "foo(1)\n";
    const char *expected =
        "\n"
        "\n"
        "1\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncOneArgMulti)
{
    const char *input =
        "#define foo(x) (x)\n"
        "foo(this is a multi-word argument)\n";
    const char *expected =
        "\n"
        "(this is a multi-word argument)\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncTwoArgsMulti)
{
    const char *input =
        "#define foo(x,y) x,two fish,red fish,y\n"
        "foo(one fish, blue fish)\n";
    const char *expected =
        "\n"
        "one fish,two fish,red fish,blue fish\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncCompose)
{
    const char *input =
        "#define bar(x) (1+(x))\n"
        "#define foo(y) (2*(y))\n"
        "foo(bar(3))\n";
    const char *expected =
        "\n"
        "\n"
        "(2*((1+(3))))\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncArgWithParens)
{
    const char *input =
        "#define foo(x) (x)\n"
        "foo(argument(with parens) FTW)\n";
    const char *expected =
        "\n"
        "(argument(with parens) FTW)\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncMacroAsNonMacro)
{
    const char *input =
        "#define foo(bar) bar\n"
        "foo bar\n";
    const char *expected =
        "\n"
        "foo bar\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncExtraNewlines)
{
    const char *input =
        "#define foo(a) (a)\n"
        "foo\n"
        "(\n"
        "1\n"
        ")\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "\n"
        "(1)\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ChainObjToFunc)
{
    const char *input =
        "#define foo() pass\n"
        "#define bar foo()\n"
        "bar\n";
    const char *expected =
        "\n"
        "\n"
        "pass\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ChainObjToNonFunc)
{
    const char *input =
        "#define pass() fail\n"
        "#define bar pass\n"
        "bar\n";
    const char *expected =
        "\n"
        "\n"
        "pass\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ChainObjToFuncWithArgs)
{
    const char *input =
        "#define foo(fail) fail\n"
        "#define bar foo(pass)\n"
        "bar\n";
    const char *expected =
        "\n"
        "\n"
        "pass\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ChainObjToFuncCompose)
{
    const char *input =
        "#define baz(fail) fail\n"
        "#define bar(fail) fail\n"
        "#define foo bar(baz(pass))\n"
        "foo\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "pass\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ChainObjToFuncParensInText1)
{
    const char *input =
        "#define fail() pass\n"
        "#define foo fail\n"
        "foo()\n";
    const char *expected =
        "\n"
        "\n"
        "pass\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ChainObjToFuncParensInText2)
{
    const char *input =
        "#define bar with,embedded,commas\n"
        "#define func(x) pass\n"
        "#define foo func\n"
        "foo(bar)\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "pass\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ChainObjToFuncMultiLevel)
{
    const char *input =
        "#define foo(x) pass\n"
        "#define bar foo\n"
        "#define baz bar\n"
        "#define joe baz\n"
        "joe (fail)\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "\n"
        "pass\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ObjToFuncRecursive)
{
    const char *input =
        "#define A(a,b) B(a,b)\n"
        "#define C A(0,C)\n"
        "C\n";
    const char *expected =
        "\n"
        "\n"
        "B(0,C)\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, ChainFuncToFuncCompose)
{
    const char *input =
        "#define baz(fail) fail\n"
        "#define bar(fail) fail\n"
        "#define foo() bar(baz(pass))\n"
        "foo()\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "pass\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncSelfRecursive)
{
    const char *input =
        "#define foo(a) foo(2*(a))\n"
        "foo(3)\n";
    const char *expected =
        "\n"
        "foo(2*(3))\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncSelfCompose)
{
    const char *input =
        "#define foo(a) foo(2*(a))\n"
        "foo(foo(3))\n";
    const char *expected =
        "\n"
        "foo(2*(foo(2*(3))))\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncSelfComposeNonFunc)
{
    const char *input =
        "#define foo(bar) bar\n"
        "foo(foo)\n";
    const char *expected =
        "\n"
        "foo\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncSelfComposeNonFuncMultiTokenArg)
{
    const char *input =
        "#define foo(bar) bar\n"
        "foo(1+foo)\n";
    const char *expected =
        "\n"
        "1+foo\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FinalizeUnexpandedMacro)
{
    const char *input =
        "#define expand(x) expand(x once)\n"
        "#define foo(x) x\n"
        "foo(expand(just))\n";
    const char *expected =
        "\n"
        "\n"
        "expand(just once)\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncArgWithCommas)
{
    const char *input =
        "#define foo(x) pass\n"
        "foo(argument (with,embedded, commas) -- baz)\n";
    const char *expected =
        "\n"
        "pass\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncArgObjMaroWithComma)
{
    const char *input =
        "#define foo(a) (a)\n"
        "#define bar two,words\n"
        "foo(bar)\n";
    const char *expected =
        "\n"
        "\n"
        "(two,words)\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncLeftParenInMacroRightParenInText)
{
    const char *input =
        "#define bar(a) a*2\n"
        "#define foo bar(\n"
        "foo b)\n";
    const char *expected =
        "\n"
        "\n"
        "b*2\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, RepeatedArg)
{
    const char *input =
        "#define double(x) x x\n"
        "double(1)\n";
    const char *expected =
        "\n"
        "1 1\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, FuncMissingRightParen)
{
    const char *input =
        "#define foo(x) (2*(x))\n"
        "foo(3\n";
    const char *expected =
        "\n"
        "\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_MACRO_UNTERMINATED_INVOCATION,
                                    pp::SourceLocation(0, 2), "foo"));

    preprocess(input, expected);
}

TEST_F(DefineTest, FuncIncorrectArgCount)
{
    const char *input =
        "#define foo(x,y) ((x)+(y))\n"
        "foo()\n"
        "foo(1)\n"
        "foo(1,2,3)\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_MACRO_TOO_FEW_ARGS, pp::SourceLocation(0, 2), "foo"));
    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_MACRO_TOO_FEW_ARGS, pp::SourceLocation(0, 3), "foo"));
    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_MACRO_TOO_MANY_ARGS, pp::SourceLocation(0, 4), "foo"));

    preprocess(input, expected);
}

TEST_F(DefineTest, Undef)
{
    const char *input =
        "#define foo 1\n"
        "foo\n"
        "#undef foo\n"
        "foo\n";
    const char *expected =
        "\n"
        "1\n"
        "\n"
        "foo\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

TEST_F(DefineTest, UndefPredefined)
{
    const char *input =
        "#undef __LINE__\n"
        "__LINE__\n"
        "#undef __FILE__\n"
        "__FILE__\n"
        "#undef __VERSION__\n"
        "__VERSION__\n"
        "#undef GL_ES\n"
        "GL_ES\n";
    const char *expected =
        "\n"
        "2\n"
        "\n"
        "0\n"
        "\n"
        "100\n"
        "\n"
        "1\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_MACRO_PREDEFINED_UNDEFINED,
                                    pp::SourceLocation(0, 1), "__LINE__"));
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_MACRO_PREDEFINED_UNDEFINED,
                                    pp::SourceLocation(0, 3), "__FILE__"));
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_MACRO_PREDEFINED_UNDEFINED,
                                    pp::SourceLocation(0, 5), "__VERSION__"));
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_MACRO_PREDEFINED_UNDEFINED,
                                    pp::SourceLocation(0, 7), "GL_ES"));

    preprocess(input, expected);
}

TEST_F(DefineTest, UndefRedefine)
{
    const char *input =
        "#define foo 1\n"
        "foo\n"
        "#undef foo\n"
        "foo\n"
        "#define foo 2\n"
        "foo\n";
    const char *expected =
        "\n"
        "1\n"
        "\n"
        "foo\n"
        "\n"
        "2\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);

    preprocess(input, expected);
}

// Example from C99 standard section 6.10.3.5 Scope of macro definitions
TEST_F(DefineTest, C99Example)
{
    const char *input =
        "#define x    3          \n"
        "#define f(a) f(x * (a)) \n"
        "#undef  x               \n"
        "#define x    2          \n"
        "#define g    f          \n"
        "#define z    z[0]       \n"
        "#define h    g(~        \n"
        "#define m(a) a(w)       \n"
        "#define w    0,1        \n"
        "#define t(a) a          \n"
        "#define p()  int        \n"
        "#define q(x) x          \n"
        "                        \n"
        "f(y+1) + f(f(z)) % t(t(g)(0) + t)(1);\n"
        "g(x+(3,4)-w) | h 5) & m\n"
        "    (f)^m(m);\n"
        "p() i[q()] = { q(1), 23, 4, 5, };\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "f(2 * (y+1)) + f(2 * (f(2 * (z[0])))) % f(2 * (0)) + t(1);\n"
        "f(2 * (2+(3,4)-0,1)) | f(2 * (~ 5)) &\n"
        " f(2 * (0,1))^m(0,1);\n"
        "int i[] = { 1, 23, 4, 5, };\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);

    preprocess(input, expected);
}

TEST_F(DefineTest, Predefined_GL_ES)
{
    const char *input    = "GL_ES\n";
    const char *expected = "1\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);

    preprocess(input, expected);
}

TEST_F(DefineTest, Predefined_VERSION)
{
    const char *input    = "__VERSION__\n";
    const char *expected = "100\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);

    preprocess(input, expected);
}

TEST_F(DefineTest, Predefined_LINE1)
{
    const char *str = "\n\n__LINE__";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 3), 100, SH_GLES2_SPEC, _))
        .Times(1);

    pp::Token token;
    lexSingleToken(str, &token);
    EXPECT_EQ(pp::Token::CONST_INT, token.type);
    EXPECT_EQ("3", token.text);
}

TEST_F(DefineTest, Predefined_LINE2)
{
    const char *str =
        "#line 10\n"
        "__LINE__\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);

    pp::Token token;
    lexSingleToken(str, &token);
    EXPECT_EQ(pp::Token::CONST_INT, token.type);
    EXPECT_EQ("10", token.text);
}

TEST_F(DefineTest, Predefined_FILE1)
{
    const char *const str[] = {"", "", "__FILE__"};
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(2, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);

    pp::Token token;
    lexSingleToken(3, str, &token);
    EXPECT_EQ(pp::Token::CONST_INT, token.type);
    EXPECT_EQ("2", token.text);
}

TEST_F(DefineTest, Predefined_FILE2)
{
    const char *const str[] = {"#line 10 20\n", "__FILE__"};
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);

    pp::Token token;
    lexSingleToken(2, str, &token);
    EXPECT_EQ(pp::Token::CONST_INT, token.type);
    EXPECT_EQ("21", token.text);
}

// Defined operator produced by macro expansion should be parsed inside #if directives
TEST_F(DefineTest, ExpandedDefinedParsedInsideIf)
{
    const char *input =
        "#define bar 1\n"
        "#define foo defined(bar)\n"
        "#if foo\n"
        "bar\n"
        "#endif\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "1\n"
        "\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

// Defined operator produced by macro expansion should not be parsed outside #if directives
TEST_F(DefineTest, ExpandedDefinedNotParsedOutsideIf)
{
    const char *input =
        "#define foo defined(bar)\n"
        "foo\n";
    const char *expected =
        "\n"
        "defined(bar)\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

// Test that line directive expressions give errors on negative or undefined shifts.
TEST_F(DefineTest, NegativeShiftInLineDirective)
{
    const char *input =
        "#line 1 << -1\n"
        "#line 1 >> -1\n"
        "#line 1 << x\n"
        "#line 1 >> x\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_UNDEFINED_SHIFT, _, _)).Times(4);
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_INVALID_LINE_NUMBER, _, _)).Times(2);
    preprocess(input, expected);
}

// Undefining a macro in its invocation parameters produces and error
TEST_F(DefineTest, UndefineInInvocation)
{
    const char *input =
        "#define G(a, b) a b\n"
        "G(\n"
        "#undef G\n"
        "1, 2)\n";
    const char *expected = "\n\n\n1 2\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_MACRO_UNDEFINED_WHILE_INVOKED,
                                    pp::SourceLocation(0, 3), _));

    preprocess(input, expected);
}

// Undefining a macro before its invocation parameters produces and error
TEST_F(DefineTest, UndefineInInvocationPreLParen)
{
    const char *input =
        "#define G(a, b) a b\n"
        "G\n"
        "#undef G\n"
        "(1, 2)\n";
    const char *expected = "\n\n\n1 2\n";

    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_MACRO_UNDEFINED_WHILE_INVOKED,
                                    pp::SourceLocation(0, 3), _));

    preprocess(input, expected);
}

// The name of the macro "a" is inside an incomplete macro invocation of macro "m()" in its own
// expansion. This should not result in infinite recursion.
TEST_F(DefineTest, RecursiveMacroNameInsideIncompleteMacroInvocationInMacroExpansion)
{
    const char *input =
        "#define m(a)\n"
        "#define a m((a)\n"
        "a)\n";
    const char *expected =
        "\n"
        "\n"
        "\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

// The name of the macro "a" is inside an incomplete macro invocation of macro "m()" in its own
// expansion. Then the macro "a" is undef'd. This is a regression test for a memory management bug
// where macro "a" would be freed on undef even though cleaning up the recursive macro invocation
// would still need to refer to macro "a".
TEST_F(DefineTest, UndefInsideRecursiveMacroInvocation)
{
    const char *input =
        "#define m(a)\n"
        "#define a m((a)\n"
        "a\n"
        "#undef a\n"
        ")\n";
    const char *expected =
        "\n"
        "\n"
        "\n"
        "\n"
        "\n";
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(input, expected);
}

// The macro invocations form a long chain. The macro expander should protect against stack overflow
// and generate an error in this case.
TEST_F(DefineTest, LongMacroInvocationChain)
{
    std::stringstream inputStream;
    std::stringstream expectedStream;

    inputStream << "#define b(x) x\n";
    inputStream << "#define a0(x) foo x\n";
    for (int i = 1; i < 20; ++i)
    {
        inputStream << "#define a" << i << "(x) b(a" << (i - 1) << "(x))\n";
    }
    inputStream << "a19(y)\n";

    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_MACRO_INVOCATION_CHAIN_TOO_DEEP,
                                    pp::SourceLocation(0, 22), _));

    pp::PreprocessorSettings settings(SH_GLES2_SPEC);
    settings.maxMacroExpansionDepth = 19;
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _))
        .Times(1);

    preprocess(inputStream.str().c_str(), settings);
}

// Tests what happens when a line directive is between macro name and parenthesis and unterminated
// argument list. Mainly to explain
// LineDirectiveInvalidNumberWithParenthesisFromFunctionInvocationInMiddleOfUnterminatedFunctionInvocation
TEST_F(DefineTest, LineDirectiveInMiddleOfUnterminatedFunctionInvocation)
{
    const char kInput[]    = R"(
#define f() a
f
#line 6
(x)";
    const char kExpected[] = R"(




)";
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_MACRO_UNTERMINATED_INVOCATION,
                                    pp::SourceLocation(0, 3), "f"))
        .Times(1);
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 2), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(kInput, kExpected);
}

// Tests what happens when an invalid line directive is between macro name and parenthesis and
// unterminated argument list. Mainly to explain
// LineDirectiveInvalidNumberWithParenthesisFromFunctionInvocationInMiddleOfUnterminatedFunctionInvocation
TEST_F(DefineTest, LineDirectiveInvalidNumberInMiddleOfUnterminatedFunctionInvocation)
{
    const char kInput[]    = R"(
#define f() a
f
#line a
(x)";
    const char kExpected[] = R"(



)";
    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_INVALID_LINE_NUMBER, pp::SourceLocation(0, 4), "a"))
        .Times(1);
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_MACRO_UNTERMINATED_INVOCATION,
                                    pp::SourceLocation(0, 3), "f"))
        .Times(1);
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 2), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(kInput, kExpected);
}

// Tests what happens when an invalid line directive with number coming from a function invocation
// is between macro name and parenthesis and unterminated argument list. Mainly to explain
// LineDirectiveInvalidNumberWithParenthesisFromFunctionInvocationInMiddleOfUnterminatedFunctionInvocation
TEST_F(DefineTest,
       LineDirectiveInvalidNumberFromFunctionInvocationInMiddleOfUnterminatedFunctionInvocation)
{
    const char kInput[]    = R"(
#define f() a
f
#line f()
(x)";
    const char kExpected[] = R"(



)";
    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_INVALID_LINE_NUMBER, pp::SourceLocation(0, 4), "a"))
        .Times(1);
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_MACRO_UNTERMINATED_INVOCATION,
                                    pp::SourceLocation(0, 3), "f"))
        .Times(1);
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 2), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(kInput, kExpected);
}

// Tests what happens when an invalid line directive with number that has a parenthesis coming from
// a function invocation is between macro name and parenthesis and unterminated argument list. This
// used to assert.
TEST_F(
    DefineTest,
    LineDirectiveInvalidNumberWithParenthesisFromFunctionInvocationInMiddleOfUnterminatedFunctionInvocation)
{
    const char kInput[]    = R"(
#define f() a(
f
#line f()
(x)";
    const char kExpected[] = R"(



)";
    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_INVALID_LINE_NUMBER, pp::SourceLocation(0, 4), "a"))
        .Times(1);
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_MACRO_UNTERMINATED_INVOCATION,
                                    pp::SourceLocation(0, 3), "f"))
        .Times(1);
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 2), 100, SH_GLES2_SPEC, _))
        .Times(1);
    preprocess(kInput, kExpected);
}

}  // namespace angle
