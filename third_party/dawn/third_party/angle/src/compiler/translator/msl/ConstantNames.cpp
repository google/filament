//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ConstantNames:
// Implementation of constant values used by the metal backend.

#include <stdio.h>

#include "GLSLANG/ShaderLang.h"

namespace sh
{

namespace mtl
{
/** extern */
const char kMultisampledRenderingConstName[]    = "ANGLEMultisampledRendering";
const char kRasterizerDiscardEnabledConstName[] = "ANGLERasterizerDisabled";
const char kDepthWriteEnabledConstName[]        = "ANGLEDepthWriteEnabled";
const char kEmulateAlphaToCoverageConstName[]   = "ANGLEEmulateAlphaToCoverage";
const char kWriteHelperSampleMaskConstName[]    = "ANGLEWriteHelperSampleMask";
const char kSampleMaskWriteEnabledConstName[]   = "ANGLESampleMaskWriteEnabled";
}  // namespace mtl

}  // namespace sh
