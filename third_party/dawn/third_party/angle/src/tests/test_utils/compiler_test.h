//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// compiler_test.h:
//     utilities for compiler unit tests.

#ifndef TESTS_TEST_UTILS_COMPILER_TEST_H_
#define TESTS_TEST_UTILS_COMPILER_TEST_H_

#include <map>
#include <regex>
#include <vector>

#include "gtest/gtest.h"

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "compiler/translator/glsl/TranslatorESSL.h"
#include "compiler/translator/tree_util/FindSymbolNode.h"

namespace sh
{

bool compileTestShader(GLenum type,
                       ShShaderSpec spec,
                       ShShaderOutput output,
                       const std::string &shaderString,
                       ShBuiltInResources *resources,
                       const ShCompileOptions &compileOptions,
                       std::string *translatedCode,
                       std::string *infoLog);

bool compileTestShader(GLenum type,
                       ShShaderSpec spec,
                       ShShaderOutput output,
                       const std::string &shaderString,
                       const ShCompileOptions &compileOptions,
                       std::string *translatedCode,
                       std::string *infoLog);

class MatchOutputCodeTest : public testing::Test
{
  protected:
    MatchOutputCodeTest(GLenum shaderType, ShShaderOutput outputType);

    void setDefaultCompileOptions(const ShCompileOptions &defaultCompileOptions);
    void addOutputType(const ShShaderOutput outputType);

    ShBuiltInResources *getResources();

    // Compile functions clear any results from earlier calls to them.
    void compile(const std::string &shaderString);
    void compile(const std::string &shaderString, const ShCompileOptions &compileOptions);

    bool foundInESSLCode(const char *stringToFind) const
    {
        return foundInCode(SH_ESSL_OUTPUT, stringToFind);
    }

    bool foundInGLSLCode(const char *stringToFind) const
    {
        return foundInCode(SH_GLSL_COMPATIBILITY_OUTPUT, stringToFind);
    }

    bool foundInCode(ShShaderOutput output, const char *stringToFind) const;
    bool foundInCodeRegex(ShShaderOutput output,
                          const std::regex &regexToFind,
                          std::smatch *match = nullptr) const;

    // Test that the strings are found in the specified output in the specified order.
    bool foundInCodeInOrder(ShShaderOutput output, std::vector<const char *> stringsToFind);

    // Test that the string occurs for exactly expectedOccurrences times
    bool foundInCode(ShShaderOutput output,
                     const char *stringToFind,
                     const int expectedOccurrences) const;

    // Test that the string is found in all outputs
    bool foundInCode(const char *stringToFind) const;
    bool foundInCodeRegex(const std::regex &regexToFind, std::smatch *match = nullptr) const;

    // Test that the string occurs for exactly expectedOccurrences times in all outputs
    bool foundInCode(const char *stringToFind, const int expectedOccurrences) const;

    // Test that the strings are found in all outputs in the specified order.
    bool foundInCodeInOrder(std::vector<const char *> stringsToFind);

    // Test that the string is found in none of the outputs
    bool notFoundInCode(const char *stringToFind) const;

    std::string outputCode(ShShaderOutput output) const;

  private:
    bool compileWithSettings(ShShaderOutput output,
                             const std::string &shaderString,
                             const ShCompileOptions &compileOptions,
                             std::string *translatedCode,
                             std::string *infoLog);

    GLenum mShaderType;
    ShCompileOptions mDefaultCompileOptions;
    ShBuiltInResources mResources;

    std::map<ShShaderOutput, std::string> mOutputCode;
};

// Returns a pointer to a function call node with a mangled name functionName.
const TIntermAggregate *FindFunctionCallNode(TIntermNode *root, const TString &functionName);
}  // namespace sh

#endif  // TESTS_TEST_UTILS_COMPILER_TEST_H_
