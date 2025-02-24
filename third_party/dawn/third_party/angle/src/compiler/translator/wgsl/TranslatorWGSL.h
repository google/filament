//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_WGSL_H_
#define COMPILER_TRANSLATOR_WGSL_H_

#include "compiler/translator/Compiler.h"

namespace sh
{
class TranslatorWGSL : public TCompiler
{
  public:
    TranslatorWGSL(sh::GLenum type, ShShaderSpec spec, ShShaderOutput output);

  protected:
    bool translate(TIntermBlock *root,
                   const ShCompileOptions &compileOptions,
                   PerformanceDiagnostics *perfDiagnostics) override;

    [[nodiscard]] bool shouldFlattenPragmaStdglInvariantAll() override;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_WGSL_H_
