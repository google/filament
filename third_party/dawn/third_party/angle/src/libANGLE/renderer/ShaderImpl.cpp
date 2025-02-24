//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderImpl.cpp: Implementation methods of ShaderImpl

#include "libANGLE/renderer/ShaderImpl.h"

#include "libANGLE/Context.h"
#include "libANGLE/trace.h"

namespace rx
{
bool ShaderTranslateTask::translate(ShHandle compiler,
                                    const ShCompileOptions &options,
                                    const std::string &source)
{
    ANGLE_TRACE_EVENT1("gpu.angle", "ShaderTranslateTask::run", "source", source);
    const char *src = source.c_str();
    return sh::Compile(compiler, &src, 1, options);
}

angle::Result ShaderImpl::onLabelUpdate(const gl::Context *context)
{
    return angle::Result::Continue;
}

}  // namespace rx
