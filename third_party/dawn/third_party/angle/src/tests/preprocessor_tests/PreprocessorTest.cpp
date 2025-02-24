//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "PreprocessorTest.h"
#include "compiler/preprocessor/Token.h"

namespace angle
{

void SimplePreprocessorTest::preprocess(const char *input,
                                        std::stringstream *output,
                                        pp::Preprocessor *preprocessor)
{
    ASSERT_TRUE(preprocessor->init(1, &input, nullptr));

    int line = 1;
    pp::Token token;
    do
    {
        preprocessor->lex(&token);
        if (output)
        {
            for (; line < token.location.line; ++line)
            {
                *output << "\n";
            }
            *output << token;
        }
    } while (token.type != pp::Token::LAST);
}

void SimplePreprocessorTest::preprocess(const char *input, const pp::PreprocessorSettings &settings)
{
    pp::Preprocessor preprocessor(&mDiagnostics, &mDirectiveHandler, settings);
    preprocess(input, nullptr, &preprocessor);
}

void SimplePreprocessorTest::preprocess(const char *input)
{
    preprocess(input, pp::PreprocessorSettings(SH_GLES2_SPEC));
}

void SimplePreprocessorTest::preprocess(const char *input, const char *expected)
{
    preprocess(input, expected, SH_GLES2_SPEC);
}

void SimplePreprocessorTest::preprocess(const char *input, const char *expected, ShShaderSpec spec)
{
    pp::Preprocessor preprocessor(&mDiagnostics, &mDirectiveHandler,
                                  pp::PreprocessorSettings(spec));
    std::stringstream output;
    preprocess(input, &output, &preprocessor);

    std::string actual = output.str();
    EXPECT_STREQ(expected, actual.c_str());
}

void SimplePreprocessorTest::lexSingleToken(const char *input, pp::Token *token)
{
    pp::Preprocessor preprocessor(&mDiagnostics, &mDirectiveHandler,
                                  pp::PreprocessorSettings(SH_GLES2_SPEC));
    ASSERT_TRUE(preprocessor.init(1, &input, nullptr));
    preprocessor.lex(token);
}

void SimplePreprocessorTest::lexSingleToken(size_t count,
                                            const char *const input[],
                                            pp::Token *token)
{
    pp::Preprocessor preprocessor(&mDiagnostics, &mDirectiveHandler,
                                  pp::PreprocessorSettings(SH_GLES2_SPEC));
    ASSERT_TRUE(preprocessor.init(count, input, nullptr));
    preprocessor.lex(token);
}

}  // namespace angle
