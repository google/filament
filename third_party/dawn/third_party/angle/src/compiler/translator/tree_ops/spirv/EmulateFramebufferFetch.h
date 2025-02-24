//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateFramebufferFetch.h: Replace inout, gl_LastFragData, gl_LastFragColorARM,
// gl_LastFragDepthARM and gl_LastFragStencilARM with usages of input attachments.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_SPIRV_EMULATEFRAMEBUFFERFETCH_H_
#define COMPILER_TRANSLATOR_TREEOPS_SPIRV_EMULATEFRAMEBUFFERFETCH_H_

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/spirv/TranslatorSPIRV.h"

namespace sh
{

class TIntermBlock;

// Emulate framebuffer fetch through the use of input attachments.
[[nodiscard]] bool EmulateFramebufferFetch(TCompiler *compiler,
                                           TIntermBlock *root,
                                           InputAttachmentMap *inputAttachmentMapOut);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_SPIRV_REPLACEFORSHADERFRAMEBUFFERFETCH_H_
