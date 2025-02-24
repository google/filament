//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompilerGL:
//   Implementation of the GL compiler methods.
//

#include "libANGLE/renderer/gl/CompilerGL.h"

#include "libANGLE/gles_extensions_autogen.h"
#include "libANGLE/renderer/gl/ContextGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "platform/autogen/FeaturesGL_autogen.h"

namespace rx
{
CompilerGL::CompilerGL(const ContextGL *context)
    : mTranslatorOutputType(GetShaderOutputType(context->getFunctions()))
{}

ShShaderOutput CompilerGL::getTranslatorOutputType() const
{
    return mTranslatorOutputType;
}

}  // namespace rx
