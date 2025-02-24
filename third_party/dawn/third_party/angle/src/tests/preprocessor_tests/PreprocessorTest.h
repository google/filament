//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gtest/gtest.h"

#include "MockDiagnostics.h"
#include "MockDirectiveHandler.h"
#include "compiler/preprocessor/Preprocessor.h"
#include "compiler/preprocessor/SourceLocation.h"

#ifndef PREPROCESSOR_TESTS_PREPROCESSOR_TEST_H_
#    define PREPROCESSOR_TESTS_PREPROCESSOR_TEST_H_

namespace angle
{

class PreprocessorTest : public testing::Test
{
  protected:
    PreprocessorTest(ShShaderSpec shaderSpec)
        : mPreprocessor(&mDiagnostics, &mDirectiveHandler, pp::PreprocessorSettings(shaderSpec))
    {}

    MockDiagnostics mDiagnostics;
    MockDirectiveHandler mDirectiveHandler;
    pp::Preprocessor mPreprocessor;
};

class SimplePreprocessorTest : public testing::Test
{
  protected:
    // Preprocesses the input string.
    void preprocess(const char *input);
    void preprocess(const char *input, const pp::PreprocessorSettings &settings);

    // Preprocesses the input string and verifies that it matches expected output.
    void preprocess(const char *input, const char *expected);
    void preprocess(const char *input, const char *expected, ShShaderSpec spec);

    // Lexes a single token from input and writes it to token.
    void lexSingleToken(const char *input, pp::Token *token);
    void lexSingleToken(size_t count, const char *const input[], pp::Token *token);

    MockDiagnostics mDiagnostics;
    MockDirectiveHandler mDirectiveHandler;

  private:
    void preprocess(const char *input, std::stringstream *output, pp::Preprocessor *preprocessor);
};

}  // namespace angle

inline std::ostream &operator<<(std::ostream &os, const angle::pp::SourceLocation &sourceLoc)
{
    return os << "(" << sourceLoc.file << ":" << sourceLoc.line << ")";
}

#endif  // PREPROCESSOR_TESTS_PREPROCESSOR_TEST_H_
