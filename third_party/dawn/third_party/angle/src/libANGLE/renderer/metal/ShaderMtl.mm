//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderMtl.mm:
//    Implements the class methods for ShaderMtl.
//

#include "libANGLE/renderer/metal/ShaderMtl.h"

#include "common/WorkerThread.h"
#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/Shader.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"
#include "libANGLE/trace.h"

namespace rx
{
namespace
{
class ShaderTranslateTaskMtl final : public ShaderTranslateTask
{
  public:
    ShaderTranslateTaskMtl(const SharedCompiledShaderStateMtl &shader) : mShader(shader) {}
    ~ShaderTranslateTaskMtl() override = default;

    void postTranslate(ShHandle compiler, const gl::CompiledShaderState &compiledState) override
    {
        sh::TranslatorMSL *translatorMSL =
            static_cast<sh::TShHandleBase *>(compiler)->getAsTranslatorMSL();
        if (translatorMSL != nullptr)
        {
            // Copy reflection data from translation
            mShader->translatorMetalReflection = *translatorMSL->getTranslatorMetalReflection();
            translatorMSL->getTranslatorMetalReflection()->reset();
        }
    }

  private:
    SharedCompiledShaderStateMtl mShader;
};
}  // anonymous namespace

ShaderMtl::ShaderMtl(const gl::ShaderState &state) : ShaderImpl(state) {}

ShaderMtl::~ShaderMtl() {}

std::shared_ptr<ShaderTranslateTask> ShaderMtl::compile(const gl::Context *context,
                                                        ShCompileOptions *options)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    DisplayMtl *displayMtl = contextMtl->getDisplay();

    // Create a new compiled shader state.  Currently running program link jobs will use the
    // previous state.
    mCompiledState = std::make_shared<CompiledShaderStateMtl>();

    // TODO(jcunningham): Remove this workaround once correct fix to move validation to the very end
    // is in place. https://bugs.webkit.org/show_bug.cgi?id=224991
    options->validateAST = false;

    options->simplifyLoopConditions = true;

    options->initializeUninitializedLocals = true;

    options->separateCompoundStructDeclarations = true;

    if (context->isWebGL() && mState.getShaderType() != gl::ShaderType::Compute)
    {
        options->initOutputVariables = true;
    }

    options->metal.generateShareableShaders =
        displayMtl->getFeatures().generateShareableShaders.enabled;

    if (displayMtl->getFeatures().intelExplicitBoolCastWorkaround.enabled ||
        options->metal.generateShareableShaders)
    {
        options->addExplicitBoolCasts = true;
    }

    options->clampPointSize = true;
#if TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
    options->clampFragDepth = true;
#endif

    if (displayMtl->getFeatures().emulateAlphaToCoverage.enabled)
    {
        options->emulateAlphaToCoverage = true;
    }

    // Constants:
    options->metal.driverUniformsBindingIndex    = mtl::kDriverUniformsBindingIndex;
    options->metal.defaultUniformsBindingIndex   = mtl::kDefaultUniformsBindingIndex;
    options->metal.UBOArgumentBufferBindingIndex = mtl::kUBOArgumentBufferBindingIndex;

    // GL_ANGLE_shader_pixel_local_storage.
    if (displayMtl->getNativeExtensions().shaderPixelLocalStorageANGLE)
    {
        options->pls = displayMtl->getNativePixelLocalStorageOptions();
    }

    options->preTransformTextureCubeGradDerivatives =
        displayMtl->getFeatures().preTransformTextureCubeGradDerivatives.enabled;

    options->rescopeGlobalVariables = displayMtl->getFeatures().rescopeGlobalVariables.enabled;

    if (displayMtl->getFeatures().injectAsmStatementIntoLoopBodies.enabled)
    {
        options->metal.injectAsmStatementIntoLoopBodies = true;
    }

    return std::shared_ptr<ShaderTranslateTask>(new ShaderTranslateTaskMtl(mCompiledState));
}

std::shared_ptr<ShaderTranslateTask> ShaderMtl::load(const gl::Context *context,
                                                     gl::BinaryInputStream *stream)
{
    UNREACHABLE();
    return std::shared_ptr<ShaderTranslateTask>(new ShaderTranslateTask);
}

std::string ShaderMtl::getDebugInfo() const
{
    return mState.getCompiledState()->translatedSource;
}

}  // namespace rx
