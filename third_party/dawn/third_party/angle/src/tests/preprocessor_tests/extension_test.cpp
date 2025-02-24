//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "PreprocessorTest.h"
#include "compiler/preprocessor/Token.h"

namespace angle
{

class ExtensionTest : public SimplePreprocessorTest
{};

TEST_F(ExtensionTest, Valid)
{
    const char *str      = "#extension foo : bar\n";
    const char *expected = "\n";

    using testing::_;
    EXPECT_CALL(mDirectiveHandler, handleExtension(pp::SourceLocation(0, 1), "foo", "bar"));
    // No error or warning.
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(str, expected);
}

TEST_F(ExtensionTest, Comments)
{
    const char *str =
        "/*foo*/"
        "#"
        "/*foo*/"
        "extension"
        "/*foo*/"
        "foo"
        "/*foo*/"
        ":"
        "/*foo*/"
        "bar"
        "/*foo*/"
        "//foo"
        "\n";
    const char *expected = "\n";

    using testing::_;
    EXPECT_CALL(mDirectiveHandler, handleExtension(pp::SourceLocation(0, 1), "foo", "bar"));
    // No error or warning.
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(str, expected);
}

TEST_F(ExtensionTest, MissingNewline)
{
    const char *str      = "#extension foo : bar";
    const char *expected = "";

    using testing::_;
    // Directive successfully parsed.
    EXPECT_CALL(mDirectiveHandler, handleExtension(pp::SourceLocation(0, 1), "foo", "bar"));
    // Error reported about EOF.
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_EOF_IN_DIRECTIVE, _, _));

    preprocess(str, expected);
}

TEST_F(ExtensionTest, ExtensionAfterNonPreProcessorTokenESSL1)
{
    const char *str =
        "int baz = 1;\n"
        "#extension foo : bar\n";
    const char *expected = "int baz = 1;\n\n";

    using testing::_;
#if defined(ANGLE_PLATFORM_CHROMEOS)
    // Directive successfully parsed.
    EXPECT_CALL(mDirectiveHandler, handleExtension(pp::SourceLocation(0, 2), "foo", "bar"));
#endif
    // Expect an error (chromeos warning) about extension pragmas after non-preprocessor tokens.
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_NON_PP_TOKEN_BEFORE_EXTENSION_ESSL1, _, _));

    preprocess(str, expected);
}

TEST_F(ExtensionTest, ExtensionAfterNonPreProcessorTokenESSL3)
{
    const char *str =
        "#version 300 es\n"
        "int baz = 1;\n"
        "#extension foo : bar\n";
    const char *expected = "\nint baz = 1;\n\n";

    using testing::_;
    // Directive successfully parsed.
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 300, SH_GLES2_SPEC, _));
    // Expect an error about extension pragmas after non-preprocessor tokens.
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_NON_PP_TOKEN_BEFORE_EXTENSION_ESSL3, _, _));

    preprocess(str, expected);
}

struct ExtensionTestParam
{
    const char *str;
    pp::Diagnostics::ID id;
};

using testing::WithParamInterface;
class InvalidExtensionTest : public ExtensionTest, public WithParamInterface<ExtensionTestParam>
{};

TEST_P(InvalidExtensionTest, Identified)
{
    ExtensionTestParam param = GetParam();
    const char *expected     = "\n";

    using testing::_;
    // No handleExtension call.
    EXPECT_CALL(mDirectiveHandler, handleExtension(_, _, _)).Times(0);
    // Invalid extension directive call.
    EXPECT_CALL(mDiagnostics, print(param.id, pp::SourceLocation(0, 1), _));

    preprocess(param.str, expected);
}

static const ExtensionTestParam kParams[] = {
    {"#extension\n", pp::Diagnostics::PP_INVALID_EXTENSION_DIRECTIVE},
    {"#extension 1\n", pp::Diagnostics::PP_INVALID_EXTENSION_NAME},
    {"#extension foo bar\n", pp::Diagnostics::PP_UNEXPECTED_TOKEN},
    {"#extension foo : \n", pp::Diagnostics::PP_INVALID_EXTENSION_DIRECTIVE},
    {"#extension foo : 1\n", pp::Diagnostics::PP_INVALID_EXTENSION_BEHAVIOR},
    {"#extension foo : bar baz\n", pp::Diagnostics::PP_UNEXPECTED_TOKEN}};
INSTANTIATE_TEST_SUITE_P(All, InvalidExtensionTest, testing::ValuesIn(kParams));

}  // namespace angle
