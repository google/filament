//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_HLSL_TRANSLATORHLSL_H_
#define COMPILER_TRANSLATOR_HLSL_TRANSLATORHLSL_H_

#include "compiler/translator/Compiler.h"

namespace sh
{

class TranslatorHLSL : public TCompiler
{
  public:
    TranslatorHLSL(sh::GLenum type, ShShaderSpec spec, ShShaderOutput output);
    TranslatorHLSL *getAsTranslatorHLSL() override { return this; }

    bool hasShaderStorageBlock(const std::string &interfaceBlockName) const;
    unsigned int getShaderStorageBlockRegister(const std::string &interfaceBlockName) const;

    bool hasUniformBlock(const std::string &interfaceBlockName) const;
    unsigned int getUniformBlockRegister(const std::string &interfaceBlockName) const;
    bool shouldUniformBlockUseStructuredBuffer(const std::string &uniformBlockName) const;
    const std::set<std::string> *getSlowCompilingUniformBlockSet() const;

    const std::map<std::string, unsigned int> *getUniformRegisterMap() const;
    unsigned int getReadonlyImage2DRegisterIndex() const;
    unsigned int getImage2DRegisterIndex() const;
    const std::set<std::string> *getUsedImage2DFunctionNames() const;

  protected:
    [[nodiscard]] bool translate(TIntermBlock *root,
                                 const ShCompileOptions &compileOptions,
                                 PerformanceDiagnostics *perfDiagnostics) override;
    bool shouldFlattenPragmaStdglInvariantAll() override;

    std::map<std::string, unsigned int> mShaderStorageBlockRegisterMap;
    std::map<std::string, unsigned int> mUniformBlockRegisterMap;
    std::map<std::string, bool> mUniformBlockUseStructuredBufferMap;
    std::map<std::string, unsigned int> mUniformRegisterMap;
    unsigned int mReadonlyImage2DRegisterIndex;
    unsigned int mImage2DRegisterIndex;
    std::set<std::string> mUsedImage2DFunctionNames;
    std::map<int, const TInterfaceBlock *> mUniformBlockOptimizedMap;
    std::set<std::string> mSlowCompilingUniformBlockSet;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_HLSL_TRANSLATORHLSL_H_
