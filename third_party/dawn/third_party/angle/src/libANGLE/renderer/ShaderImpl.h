//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderImpl.h: Defines the abstract rx::ShaderImpl class.

#ifndef LIBANGLE_RENDERER_SHADERIMPL_H_
#define LIBANGLE_RENDERER_SHADERIMPL_H_

#include <functional>

#include "common/CompiledShaderState.h"
#include "common/WorkerThread.h"
#include "common/angleutils.h"
#include "libANGLE/Shader.h"

namespace gl
{
class ShCompilerInstance;
}  // namespace gl

namespace rx
{
// The compile task is generally just a call to the translator.  However, different backends behave
// differently afterwards:
//
// - The Vulkan backend which generates binary (i.e. SPIR-V), does nothing more
// - The backends that generate text (HLSL, MSL, WGSL), do nothing at this stage, but modify the
//   text at link time before invoking the native compiler.  These expensive calls are handled in
//   link sub-tasks (see LinkSubTask in ProgramImpl.h).
// - The GL backend needs to invoke the native driver, which is problematic when done in another
//   thread (and is avoided).
//
// The call to the translator can thus be done in a separate thread or without holding the share
// group lock on all backends except GL.  On the GL backend, the translator call is done on the main
// thread followed by a call to the native driver.  If the driver supports
// GL_KHR_parallel_shader_compile, ANGLE still delays post-processing of the results to when
// compilation is done (just as if it was ANGLE itself that was doing the compilation in a thread).
class ShaderTranslateTask
{
  public:
    virtual ~ShaderTranslateTask() = default;

    // Used for compile()
    virtual bool translate(ShHandle compiler,
                           const ShCompileOptions &options,
                           const std::string &source);
    virtual void postTranslate(ShHandle compiler, const gl::CompiledShaderState &compiledState) {}

    // Used for load()
    virtual void load(const gl::CompiledShaderState &compiledState) {}

    // Used by the GL backend to query whether the driver is compiling in parallel internally.
    virtual bool isCompilingInternally() { return false; }
    // Used by the GL backend to finish internal compilation and return results.
    virtual angle::Result getResult(std::string &infoLog) { return angle::Result::Continue; }
};

class ShaderImpl : angle::NonCopyable
{
  public:
    ShaderImpl(const gl::ShaderState &state) : mState(state) {}
    virtual ~ShaderImpl() {}

    virtual void onDestroy(const gl::Context *context) {}

    virtual std::shared_ptr<ShaderTranslateTask> compile(const gl::Context *context,
                                                         ShCompileOptions *options)  = 0;
    virtual std::shared_ptr<ShaderTranslateTask> load(const gl::Context *context,
                                                      gl::BinaryInputStream *stream) = 0;

    virtual std::string getDebugInfo() const = 0;

    const gl::ShaderState &getState() const { return mState; }

    virtual angle::Result onLabelUpdate(const gl::Context *context);

  protected:
    const gl::ShaderState &mState;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_SHADERIMPL_H_
