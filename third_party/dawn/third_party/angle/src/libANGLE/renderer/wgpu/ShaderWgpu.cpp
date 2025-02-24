//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderWgpu.cpp:
//    Implements the class methods for ShaderWgpu.
//

#include "libANGLE/renderer/wgpu/ShaderWgpu.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/ContextImpl.h"
#include "libANGLE/trace.h"

namespace rx
{

namespace
{
class ShaderTranslateTaskWgpu final : public ShaderTranslateTask
{
    bool translate(ShHandle compiler,
                   const ShCompileOptions &options,
                   const std::string &source) override
    {
        ANGLE_TRACE_EVENT1("gpu.angle", "ShaderTranslateTaskWgpu::translate", "source", source);

        const char *srcStrings[] = {source.c_str()};
        return sh::Compile(compiler, srcStrings, ArraySize(srcStrings), options);
    }

    void postTranslate(ShHandle compiler, const gl::CompiledShaderState &compiledState) override {}
};
}  // namespace

ShaderWgpu::ShaderWgpu(const gl::ShaderState &data) : ShaderImpl(data) {}

ShaderWgpu::~ShaderWgpu() {}

std::shared_ptr<ShaderTranslateTask> ShaderWgpu::compile(const gl::Context *context,
                                                         ShCompileOptions *options)
{
    const gl::Extensions &extensions = context->getImplementation()->getExtensions();
    if (extensions.shaderPixelLocalStorageANGLE)
    {
        options->pls = context->getImplementation()->getNativePixelLocalStorageOptions();
    }

    options->validateAST = true;

    options->separateCompoundStructDeclarations = true;

    return std::shared_ptr<ShaderTranslateTask>(new ShaderTranslateTaskWgpu);
}

std::shared_ptr<ShaderTranslateTask> ShaderWgpu::load(const gl::Context *context,
                                                      gl::BinaryInputStream *stream)
{
    UNREACHABLE();
    return std::shared_ptr<ShaderTranslateTask>(new ShaderTranslateTask);
}

std::string ShaderWgpu::getDebugInfo() const
{
    return "";
}

}  // namespace rx
