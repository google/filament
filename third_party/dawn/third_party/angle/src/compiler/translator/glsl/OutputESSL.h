//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_GLSL_OUTPUTESSL_H_
#define COMPILER_TRANSLATOR_GLSL_OUTPUTESSL_H_

#include "compiler/translator/glsl/OutputGLSLBase.h"

namespace sh
{

class TOutputESSL : public TOutputGLSLBase
{
  public:
    TOutputESSL(TCompiler *compiler,
                TInfoSinkBase &objSink,
                const ShCompileOptions &compileOptions);

  protected:
    bool writeVariablePrecision(TPrecision precision) override;
    ImmutableString translateTextureFunction(const ImmutableString &name,
                                             const ShCompileOptions &option) override;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_GLSL_OUTPUTESSL_H_
