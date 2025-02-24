//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_GLSL_TRANSLATORGLSL_H_
#define COMPILER_TRANSLATOR_GLSL_TRANSLATORGLSL_H_

#include "compiler/translator/Compiler.h"

namespace sh
{

class TranslatorGLSL : public TCompiler
{
  public:
    TranslatorGLSL(sh::GLenum type, ShShaderSpec spec, ShShaderOutput output);

  protected:
    void initBuiltInFunctionEmulator(BuiltInFunctionEmulator *emu,
                                     const ShCompileOptions &compileOptions) override;

    [[nodiscard]] bool translate(TIntermBlock *root,
                                 const ShCompileOptions &compileOptions,
                                 PerformanceDiagnostics *perfDiagnostics) override;
    bool shouldFlattenPragmaStdglInvariantAll() override;

  private:
    void writeVersion(TIntermNode *root);
    void writeExtensionBehavior(TIntermNode *root, const ShCompileOptions &compileOptions);
    void conditionallyOutputInvariantDeclaration(const char *builtinVaryingName);
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_GLSL_TRANSLATORGLSL_H_
