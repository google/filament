//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "PreprocessorTest.h"
#include "compiler/preprocessor/Token.h"

namespace angle
{

class VersionTest : public SimplePreprocessorTest
{};

TEST_F(VersionTest, NoDirectives)
{
    const char *str      = "foo\n";
    const char *expected = "foo\n";

    using testing::_;
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _));
    // No error or warning.
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(str, expected);
}

TEST_F(VersionTest, NoVersion)
{
    const char *str      = "#define foo\n";
    const char *expected = "\n";

    using testing::_;
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 100, SH_GLES2_SPEC, _));
    // No error or warning.
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(str, expected);
}

TEST_F(VersionTest, Valid)
{
    const char *str      = "#version 200\n";
    const char *expected = "\n";

    using testing::_;
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 200, SH_GLES2_SPEC, _));
    // No error or warning.
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(str, expected);
}

TEST_F(VersionTest, CommentsIgnored)
{
    const char *str =
        "/*foo*/"
        "#"
        "/*foo*/"
        "version"
        "/*foo*/"
        "200"
        "/*foo*/"
        "//foo"
        "\n";
    const char *expected = "\n";

    using testing::_;
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 200, SH_GLES2_SPEC, _));
    // No error or warning.
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(str, expected);
}

TEST_F(VersionTest, MissingNewline)
{
    const char *str      = "#version 200";
    const char *expected = "";

    using testing::_;
    // Directive successfully parsed.
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 1), 200, SH_GLES2_SPEC, _));
    // Error reported about EOF.
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_EOF_IN_DIRECTIVE, _, _));

    preprocess(str, expected);
}

TEST_F(VersionTest, AfterComments)
{
    const char *str =
        "/* block comment acceptable */\n"
        "// line comment acceptable\n"
        "#version 200\n";
    const char *expected = "\n\n\n";

    using testing::_;
    // Directive successfully parsed.
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 3), 200, SH_GLES2_SPEC, _));
    // No error or warning.
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(str, expected);
}

TEST_F(VersionTest, AfterWhitespace)
{
    const char *str =
        "\n"
        "\n"
        "#version 200\n";
    const char *expected = "\n\n\n";

    using testing::_;
    // Directive successfully parsed.
    EXPECT_CALL(mDirectiveHandler, handleVersion(pp::SourceLocation(0, 3), 200, SH_GLES2_SPEC, _));
    // No error or warning.
    EXPECT_CALL(mDiagnostics, print(_, _, _)).Times(0);

    preprocess(str, expected);
}

TEST_F(VersionTest, AfterValidToken)
{
    const char *str =
        "foo\n"
        "#version 200\n";

    using testing::_;
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_VERSION_NOT_FIRST_STATEMENT,
                                    pp::SourceLocation(0, 2), _));

    preprocess(str);
}

TEST_F(VersionTest, AfterInvalidToken)
{
    const char *str =
        "$\n"
        "#version 200\n";

    using testing::_;
    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_INVALID_CHARACTER, pp::SourceLocation(0, 1), "$"));
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_VERSION_NOT_FIRST_STATEMENT,
                                    pp::SourceLocation(0, 2), _));

    preprocess(str);
}

TEST_F(VersionTest, AfterValidDirective)
{
    const char *str =
        "#\n"
        "#version 200\n";

    using testing::_;
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_VERSION_NOT_FIRST_STATEMENT,
                                    pp::SourceLocation(0, 2), _));

    preprocess(str);
}

TEST_F(VersionTest, AfterInvalidDirective)
{
    const char *str =
        "#foo\n"
        "#version 200\n";

    using testing::_;
    EXPECT_CALL(mDiagnostics,
                print(pp::Diagnostics::PP_DIRECTIVE_INVALID_NAME, pp::SourceLocation(0, 1), "foo"));
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_VERSION_NOT_FIRST_STATEMENT,
                                    pp::SourceLocation(0, 2), _));

    preprocess(str);
}

TEST_F(VersionTest, AfterExcludedBlock)
{
    const char *str =
        "#if 0\n"
        "foo\n"
        "#endif\n"
        "#version 200\n";

    using testing::_;
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_VERSION_NOT_FIRST_STATEMENT,
                                    pp::SourceLocation(0, 4), _));

    preprocess(str);
}

struct VersionTestParam
{
    const char *str;
    pp::Diagnostics::ID id;
};

class InvalidVersionTest : public VersionTest, public testing::WithParamInterface<VersionTestParam>
{};

TEST_P(InvalidVersionTest, Identified)
{
    VersionTestParam param = GetParam();
    const char *expected   = "\n";

    using testing::_;
    // No handleVersion call.
    EXPECT_CALL(mDirectiveHandler, handleVersion(_, _, _, _)).Times(0);
    // Invalid version directive call.
    EXPECT_CALL(mDiagnostics, print(param.id, pp::SourceLocation(0, 1), _));

    preprocess(param.str, expected);
}

static const VersionTestParam kParams[] = {
    {"#version\n", pp::Diagnostics::PP_INVALID_VERSION_DIRECTIVE},
    {"#version foo\n", pp::Diagnostics::PP_INVALID_VERSION_NUMBER},
    {"#version 100 foo\n", pp::Diagnostics::PP_UNEXPECTED_TOKEN},
    {"#version 0xffffffff\n", pp::Diagnostics::PP_INTEGER_OVERFLOW}};

INSTANTIATE_TEST_SUITE_P(All, InvalidVersionTest, testing::ValuesIn(kParams));

}  // namespace angle
