//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_HLSL_BUILTINFUNCTIONEMULATORHLSL_H_
#define COMPILER_TRANSLATOR_HLSL_BUILTINFUNCTIONEMULATORHLSL_H_

#include "GLSLANG/ShaderLang.h"

namespace sh
{

class BuiltInFunctionEmulator;

void InitBuiltInFunctionEmulatorForHLSL(BuiltInFunctionEmulator *emu);

//
// This works around isnan() bug on some Intel drivers.
//
void InitBuiltInIsnanFunctionEmulatorForHLSLWorkarounds(BuiltInFunctionEmulator *emu,
                                                        int targetGLSLVersion);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_HLSL_BUILTINFUNCTIONEMULATORHLSL_H_
