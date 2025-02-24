//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompilerNULL.cpp:
//    Implements the class methods for CompilerNULL.
//

#include "libANGLE/renderer/null/CompilerNULL.h"

#include "common/debug.h"

namespace rx
{

CompilerNULL::CompilerNULL() : CompilerImpl() {}

CompilerNULL::~CompilerNULL() {}

ShShaderOutput CompilerNULL::getTranslatorOutputType() const
{
    return SH_NULL_OUTPUT;
}

}  // namespace rx
