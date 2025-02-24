//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// compiler_test.cpp:
//     utilities for compiler unit tests.

#include "tests/test_utils/compiler_test.h"

#include "angle_gl.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/FunctionLookup.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{
constexpr char kBinaryBlob[] = "<binary blob>";
bool IsBinaryBlob(const std::string &code)
{
    return code == kBinaryBlob;
}

ImmutableString GetSymbolTableMangledName(TIntermAggregate *node)
{
    ASSERT(!node->isConstructor());
    return TFunctionLookup::GetMangledName(node->getFunction()->name().data(),
                                           *node->getSequence());
}

class FunctionCallFinder : public TIntermTraverser
{
  public:
    FunctionCallFinder(const char *functionMangledName)
        : TIntermTraverser(true, false, false),
          mFunctionMangledName(functionMangledName),
          mNodeFound(nullptr)
    {}

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (!node->isConstructor() && GetSymbolTableMangledName(node) == mFunctionMangledName)
        {
            mNodeFound = node;
            return false;
        }
        return true;
    }

    bool isFound() const { return mNodeFound != nullptr; }
    const TIntermAggregate *getNode() const { return mNodeFound; }

  private:
    const char *mFunctionMangledName;
    TIntermAggregate *mNodeFound;
};

}  // anonymous namespace

bool compileTestShader(GLenum type,
                       ShShaderSpec spec,
                       ShShaderOutput output,
                       const std::string &shaderString,
                       ShBuiltInResources *resources,
                       const ShCompileOptions &compileOptions,
                       std::string *translatedCode,
                       std::string *infoLog)
{
    sh::TCompiler *translator = sh::ConstructCompiler(type, spec, output);
    if (!translator->Init(*resources))
    {
        SafeDelete(translator);
        return false;
    }

    const char *shaderStrings[] = {shaderString.c_str()};

    ShCompileOptions options = compileOptions;
    options.objectCode       = true;
    bool compilationSuccess  = translator->compile(shaderStrings, 1, options);
    TInfoSink &infoSink      = translator->getInfoSink();
    if (translatedCode)
    {
        *translatedCode = infoSink.obj.isBinary() ? kBinaryBlob : infoSink.obj.c_str();
    }
    if (infoLog)
    {
        *infoLog = infoSink.info.c_str();
    }
    SafeDelete(translator);
    return compilationSuccess;
}

bool compileTestShader(GLenum type,
                       ShShaderSpec spec,
                       ShShaderOutput output,
                       const std::string &shaderString,
                       const ShCompileOptions &compileOptions,
                       std::string *translatedCode,
                       std::string *infoLog)
{
    ShBuiltInResources resources;
    sh::InitBuiltInResources(&resources);
    resources.FragmentPrecisionHigh = 1;
    return compileTestShader(type, spec, output, shaderString, &resources, compileOptions,
                             translatedCode, infoLog);
}

MatchOutputCodeTest::MatchOutputCodeTest(GLenum shaderType, ShShaderOutput outputType)
    : mShaderType(shaderType), mDefaultCompileOptions{}
{
    sh::InitBuiltInResources(&mResources);
    mResources.FragmentPrecisionHigh = 1;
    mOutputCode[outputType]          = std::string();
}

void MatchOutputCodeTest::setDefaultCompileOptions(const ShCompileOptions &defaultCompileOptions)
{
    mDefaultCompileOptions = defaultCompileOptions;
}

void MatchOutputCodeTest::addOutputType(const ShShaderOutput outputType)
{
    mOutputCode[outputType] = std::string();
}

ShBuiltInResources *MatchOutputCodeTest::getResources()
{
    return &mResources;
}

void MatchOutputCodeTest::compile(const std::string &shaderString)
{
    compile(shaderString, mDefaultCompileOptions);
}

void MatchOutputCodeTest::compile(const std::string &shaderString,
                                  const ShCompileOptions &compileOptions)
{
    std::string infoLog;
    for (auto &code : mOutputCode)
    {
        bool compilationSuccess =
            compileWithSettings(code.first, shaderString, compileOptions, &code.second, &infoLog);
        if (!compilationSuccess)
        {
            FAIL() << "Shader compilation failed:\n" << infoLog;
        }
    }
}

bool MatchOutputCodeTest::compileWithSettings(ShShaderOutput output,
                                              const std::string &shaderString,
                                              const ShCompileOptions &compileOptions,
                                              std::string *translatedCode,
                                              std::string *infoLog)
{
    return compileTestShader(mShaderType, SH_GLES3_1_SPEC, output, shaderString, &mResources,
                             compileOptions, translatedCode, infoLog);
}

bool MatchOutputCodeTest::foundInCodeRegex(ShShaderOutput output,
                                           const std::regex &regexToFind,
                                           std::smatch *match) const
{
    const auto code = mOutputCode.find(output);
    EXPECT_NE(mOutputCode.end(), code);
    if (code == mOutputCode.end())
    {
        return std::string::npos;
    }

    // No meaningful check for binary blobs
    if (IsBinaryBlob(code->second))
    {
        return true;
    }

    if (match)
    {
        return std::regex_search(code->second, *match, regexToFind);
    }
    else
    {
        return std::regex_search(code->second, regexToFind);
    }
}

bool MatchOutputCodeTest::foundInCode(ShShaderOutput output, const char *stringToFind) const
{
    const auto code = mOutputCode.find(output);
    EXPECT_NE(mOutputCode.end(), code);
    if (code == mOutputCode.end())
    {
        return std::string::npos;
    }

    // No meaningful check for binary blobs
    if (IsBinaryBlob(code->second))
    {
        return true;
    }

    return code->second.find(stringToFind) != std::string::npos;
}

bool MatchOutputCodeTest::foundInCodeInOrder(ShShaderOutput output,
                                             std::vector<const char *> stringsToFind)
{
    const auto code = mOutputCode.find(output);
    EXPECT_NE(mOutputCode.end(), code);
    if (code == mOutputCode.end())
    {
        return false;
    }

    // No meaningful check for binary blobs
    if (IsBinaryBlob(code->second))
    {
        return true;
    }

    size_t currentPos = 0;
    for (const char *stringToFind : stringsToFind)
    {
        auto position = code->second.find(stringToFind, currentPos);
        if (position == std::string::npos)
        {
            return false;
        }
        currentPos = position + strlen(stringToFind);
    }
    return true;
}

bool MatchOutputCodeTest::foundInCode(ShShaderOutput output,
                                      const char *stringToFind,
                                      const int expectedOccurrences) const
{
    const auto code = mOutputCode.find(output);
    EXPECT_NE(mOutputCode.end(), code);
    if (code == mOutputCode.end())
    {
        return false;
    }

    // No meaningful check for binary blobs
    if (IsBinaryBlob(code->second))
    {
        return true;
    }

    size_t currentPos  = 0;
    int occurencesLeft = expectedOccurrences;

    const size_t searchStringLength = strlen(stringToFind);

    while (occurencesLeft-- > 0)
    {
        auto position = code->second.find(stringToFind, currentPos);
        if (position == std::string::npos)
        {
            return false;
        }
        // Search strings should not overlap.
        currentPos = position + searchStringLength;
    }
    // Make sure that there aren't extra occurrences.
    return code->second.find(stringToFind, currentPos) == std::string::npos;
}

bool MatchOutputCodeTest::foundInCode(const char *stringToFind) const
{
    for (auto &code : mOutputCode)
    {
        if (!foundInCode(code.first, stringToFind))
        {
            return false;
        }
    }
    return true;
}

bool MatchOutputCodeTest::foundInCodeRegex(const std::regex &regexToFind, std::smatch *match) const
{
    for (auto &code : mOutputCode)
    {
        if (!foundInCodeRegex(code.first, regexToFind, match))
        {
            return false;
        }
    }
    return true;
}

bool MatchOutputCodeTest::foundInCode(const char *stringToFind, const int expectedOccurrences) const
{
    for (auto &code : mOutputCode)
    {
        if (!foundInCode(code.first, stringToFind, expectedOccurrences))
        {
            return false;
        }
    }
    return true;
}

bool MatchOutputCodeTest::foundInCodeInOrder(std::vector<const char *> stringsToFind)
{
    for (auto &code : mOutputCode)
    {
        if (!foundInCodeInOrder(code.first, stringsToFind))
        {
            return false;
        }
    }
    return true;
}

bool MatchOutputCodeTest::notFoundInCode(const char *stringToFind) const
{
    for (auto &code : mOutputCode)
    {
        // No meaningful check for binary blobs
        if (IsBinaryBlob(code.second))
        {
            continue;
        }

        if (foundInCode(code.first, stringToFind))
        {
            return false;
        }
    }
    return true;
}

std::string MatchOutputCodeTest::outputCode(ShShaderOutput output) const
{
    const auto code = mOutputCode.find(output);
    EXPECT_NE(mOutputCode.end(), code);
    if (code == mOutputCode.end())
    {
        return {};
    }

    // No meaningful check for binary blobs
    if (IsBinaryBlob(code->second))
    {
        return {};
    }

    return code->second;
}

const TIntermAggregate *FindFunctionCallNode(TIntermNode *root, const TString &functionMangledName)
{
    FunctionCallFinder finder(functionMangledName.c_str());
    root->traverse(&finder);
    return finder.getNode();
}

}  // namespace sh
