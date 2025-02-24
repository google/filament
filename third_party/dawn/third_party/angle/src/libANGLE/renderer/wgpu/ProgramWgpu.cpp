//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramWgpu.cpp:
//    Implements the class methods for ProgramWgpu.
//

#include "libANGLE/renderer/wgpu/ProgramWgpu.h"

#include "GLES2/gl2.h"
#include "common/PackedEnums.h"
#include "common/PackedGLEnums_autogen.h"
#include "common/debug.h"
#include "common/log_utils.h"
#include "libANGLE/Error.h"
#include "libANGLE/ProgramExecutable.h"
#include "libANGLE/renderer/wgpu/ProgramExecutableWgpu.h"
#include "libANGLE/renderer/wgpu/wgpu_utils.h"
#include "libANGLE/renderer/wgpu/wgpu_wgsl_util.h"
#include "libANGLE/trace.h"

#include <dawn/webgpu_cpp.h>

namespace rx
{
namespace
{
const bool kOutputFinalSource = false;

// Identical to Std140 encoder in all aspects, except it ignores opaque uniform types.
class WgpuDefaultBlockEncoder : public sh::Std140BlockEncoder
{
  public:
    void advanceOffset(GLenum type,
                       const std::vector<unsigned int> &arraySizes,
                       bool isRowMajorMatrix,
                       int arrayStride,
                       int matrixStride) override
    {
        if (gl::IsOpaqueType(type))
        {
            return;
        }

        sh::Std140BlockEncoder::advanceOffset(type, arraySizes, isRowMajorMatrix, arrayStride,
                                              matrixStride);
    }
};

void InitDefaultUniformBlock(const std::vector<sh::ShaderVariable> &uniforms,
                             sh::BlockLayoutMap *blockLayoutMapOut,
                             size_t *blockSizeOut)
{
    if (uniforms.empty())
    {
        *blockSizeOut = 0;
        return;
    }

    WgpuDefaultBlockEncoder blockEncoder;
    sh::GetActiveUniformBlockInfo(uniforms, "", &blockEncoder, blockLayoutMapOut);

    *blockSizeOut = blockEncoder.getCurrentOffset();
    return;
}

class CreateWGPUShaderModuleTask : public LinkSubTask
{
  public:
    CreateWGPUShaderModuleTask(wgpu::Instance instance,
                               wgpu::Device device,
                               const gl::SharedCompiledShaderState &compiledShaderState,
                               const gl::ProgramExecutable &executable,
                               gl::ProgramMergedVaryings mergedVaryings,
                               TranslatedWGPUShaderModule &resultShaderModule)
        : mInstance(instance),
          mDevice(device),
          mCompiledShaderState(compiledShaderState),
          mExecutable(executable),
          mMergedVaryings(std::move(mergedVaryings)),
          mShaderModule(resultShaderModule)
    {}

    angle::Result getResult(const gl::Context *context, gl::InfoLog &infoLog) override
    {
        infoLog << mLog.str();
        return mResult;
    }

    void operator()() override
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "CreateWGPUShaderModuleTask");

        gl::ShaderType shaderType = mCompiledShaderState->shaderType;

        ASSERT((mExecutable.getLinkedShaderStages() &
                ~gl::ShaderBitSet({gl::ShaderType::Vertex, gl::ShaderType::Fragment}))
                   .none());
        std::string finalShaderSource;
        if (shaderType == gl::ShaderType::Vertex)
        {
            finalShaderSource = webgpu::WgslAssignLocations(mCompiledShaderState->translatedSource,
                                                            mExecutable.getProgramInputs(),
                                                            mMergedVaryings, shaderType);
        }
        else if (shaderType == gl::ShaderType::Fragment)
        {
            finalShaderSource = webgpu::WgslAssignLocations(mCompiledShaderState->translatedSource,
                                                            mExecutable.getOutputVariables(),
                                                            mMergedVaryings, shaderType);
        }
        else
        {
            UNIMPLEMENTED();
        }
        if (kOutputFinalSource)
        {
            std::cout << finalShaderSource;
        }

        wgpu::ShaderModuleWGSLDescriptor shaderModuleWGSLDescriptor;
        shaderModuleWGSLDescriptor.code = finalShaderSource.c_str();

        wgpu::ShaderModuleDescriptor shaderModuleDescriptor;
        shaderModuleDescriptor.nextInChain = &shaderModuleWGSLDescriptor;

        mShaderModule.module = mDevice.CreateShaderModule(&shaderModuleDescriptor);

        wgpu::CompilationInfoCallbackInfo callbackInfo;
        callbackInfo.mode     = wgpu::CallbackMode::WaitAnyOnly;
        callbackInfo.callback = [](WGPUCompilationInfoRequestStatus status,
                                   struct WGPUCompilationInfo const *compilationInfo,
                                   void *userdata) {
            CreateWGPUShaderModuleTask *task = static_cast<CreateWGPUShaderModuleTask *>(userdata);

            if (status != WGPUCompilationInfoRequestStatus_Success)
            {
                task->mResult = angle::Result::Stop;
            }

            for (size_t msgIdx = 0; msgIdx < compilationInfo->messageCount; ++msgIdx)
            {
                const WGPUCompilationMessage &message = compilationInfo->messages[msgIdx];
                switch (message.type)
                {
                    case WGPUCompilationMessageType_Error:
                        task->mLog << "Error: ";
                        break;
                    case WGPUCompilationMessageType_Warning:
                        task->mLog << "Warning: ";
                        break;
                    case WGPUCompilationMessageType_Info:
                        task->mLog << "Info: ";
                        break;
                    default:
                        task->mLog << "Unknown: ";
                        break;
                }
                task->mLog << message.lineNum << ":" << message.linePos << ": " << message.message
                           << std::endl;
            }
        };
        callbackInfo.userdata = this;

        wgpu::FutureWaitInfo waitInfo;
        waitInfo.future = mShaderModule.module.GetCompilationInfo(callbackInfo);

        wgpu::WaitStatus waitStatus = mInstance.WaitAny(1, &waitInfo, -1);
        if (waitStatus != wgpu::WaitStatus::Success)
        {
            mResult = angle::Result::Stop;
        }
    }

  private:
    wgpu::Instance mInstance;
    wgpu::Device mDevice;
    gl::SharedCompiledShaderState mCompiledShaderState;
    const gl::ProgramExecutable &mExecutable;
    gl::ProgramMergedVaryings mMergedVaryings;

    TranslatedWGPUShaderModule &mShaderModule;

    std::ostringstream mLog;
    angle::Result mResult = angle::Result::Continue;
};

class LinkTaskWgpu : public LinkTask
{
  public:
    LinkTaskWgpu(wgpu::Instance instance, wgpu::Device device, ProgramWgpu *program)
        : mInstance(instance),
          mDevice(device),
          mProgram(program),
          mExecutable(&mProgram->getState().getExecutable())
    {}
    ~LinkTaskWgpu() override = default;

    void link(const gl::ProgramLinkedResources &resources,
              const gl::ProgramMergedVaryings &mergedVaryings,
              std::vector<std::shared_ptr<LinkSubTask>> *linkSubTasksOut,
              std::vector<std::shared_ptr<LinkSubTask>> *postLinkSubTasksOut) override
    {
        ASSERT(linkSubTasksOut && linkSubTasksOut->empty());
        ASSERT(postLinkSubTasksOut && postLinkSubTasksOut->empty());

        ProgramExecutableWgpu *executable =
            GetImplAs<ProgramExecutableWgpu>(&mProgram->getState().getExecutable());

        const gl::ShaderMap<gl::SharedCompiledShaderState> &shaders =
            mProgram->getState().getAttachedShaders();
        for (gl::ShaderType shaderType : gl::AllShaderTypes())
        {
            if (shaders[shaderType])
            {
                auto task = std::make_shared<CreateWGPUShaderModuleTask>(
                    mInstance, mDevice, shaders[shaderType], *executable->getExecutable(),
                    mergedVaryings, executable->getShaderModule(shaderType));
                linkSubTasksOut->push_back(task);
            }
        }

        // The default uniform block's CPU buffer needs to be allocated and the layout calculated,
        // now that the list of uniforms is known.
        angle::Result initUniformBlocksResult = initDefaultUniformBlocks();
        if (IsError(initUniformBlocksResult))
        {
            mLinkResult = initUniformBlocksResult;
            return;
        }

        mLinkResult = angle::Result::Continue;
    }

    angle::Result getResult(const gl::Context *context, gl::InfoLog &infoLog) override
    {
        return mLinkResult;
    }

  private:
    angle::Result initDefaultUniformBlocks()
    {
        ProgramExecutableWgpu *executableWgpu = webgpu::GetImpl(mExecutable);

        // Process vertex and fragment uniforms into std140 packing.
        gl::ShaderMap<sh::BlockLayoutMap> layoutMap;
        gl::ShaderMap<size_t> requiredBufferSize;
        requiredBufferSize.fill(0);

        generateUniformLayoutMapping(&layoutMap, &requiredBufferSize);
        initDefaultUniformLayoutMapping(&layoutMap);

        // All uniform initializations are complete, now resize the buffers accordingly and return
        ANGLE_TRY(executableWgpu->resizeUniformBlockMemory(requiredBufferSize));

        executableWgpu->markDefaultUniformsDirty();

        return angle::Result::Continue;
    }

    void generateUniformLayoutMapping(gl::ShaderMap<sh::BlockLayoutMap> *layoutMapOut,
                                      gl::ShaderMap<size_t> *requiredBufferSizeOut)
    {
        for (const gl::ShaderType shaderType : mExecutable->getLinkedShaderStages())
        {
            const gl::SharedCompiledShaderState &shader =
                mProgram->getState().getAttachedShader(shaderType);

            if (shader)
            {
                const std::vector<sh::ShaderVariable> &uniforms = shader->uniforms;
                InitDefaultUniformBlock(uniforms, &(*layoutMapOut)[shaderType],
                                        &(*requiredBufferSizeOut)[shaderType]);
            }
        }
    }

    void initDefaultUniformLayoutMapping(gl::ShaderMap<sh::BlockLayoutMap> *layoutMapOut)
    {
        // Init the default block layout info.
        ProgramExecutableWgpu *executableWgpu = webgpu::GetImpl(mExecutable);
        const auto &uniforms                  = mExecutable->getUniforms();

        for (const gl::VariableLocation &location : mExecutable->getUniformLocations())
        {
            gl::ShaderMap<sh::BlockMemberInfo> layoutInfo;

            if (location.used() && !location.ignored)
            {
                const auto &uniform = uniforms[location.index];
                if (uniform.isInDefaultBlock() && !uniform.isSampler() && !uniform.isImage() &&
                    !uniform.isFragmentInOut())
                {
                    std::string uniformName = mExecutable->getUniformNameByIndex(location.index);
                    if (uniform.isArray())
                    {
                        // Gets the uniform name without the [0] at the end.
                        uniformName = gl::StripLastArrayIndex(uniformName);
                        ASSERT(uniformName.size() !=
                               mExecutable->getUniformNameByIndex(location.index).size());
                    }

                    bool found = false;

                    for (const gl::ShaderType shaderType : mExecutable->getLinkedShaderStages())
                    {
                        auto it = (*layoutMapOut)[shaderType].find(uniformName);
                        if (it != (*layoutMapOut)[shaderType].end())
                        {
                            found                  = true;
                            layoutInfo[shaderType] = it->second;
                        }
                    }

                    ASSERT(found);
                }
            }

            for (const gl::ShaderType shaderType : mExecutable->getLinkedShaderStages())
            {
                executableWgpu->getSharedDefaultUniformBlock(shaderType)
                    ->uniformLayout.push_back(layoutInfo[shaderType]);
            }
        }
    }

    wgpu::Instance mInstance;
    wgpu::Device mDevice;
    ProgramWgpu *mProgram = nullptr;
    const gl::ProgramExecutable *mExecutable;
    angle::Result mLinkResult = angle::Result::Stop;
};
}  // anonymous namespace

ProgramWgpu::ProgramWgpu(const gl::ProgramState &state) : ProgramImpl(state) {}

ProgramWgpu::~ProgramWgpu() {}

angle::Result ProgramWgpu::load(const gl::Context *context,
                                gl::BinaryInputStream *stream,
                                std::shared_ptr<LinkTask> *loadTaskOut,
                                egl::CacheGetResult *resultOut)
{
    *loadTaskOut = {};
    *resultOut   = egl::CacheGetResult::Success;
    return angle::Result::Continue;
}

void ProgramWgpu::save(const gl::Context *context, gl::BinaryOutputStream *stream) {}

void ProgramWgpu::setBinaryRetrievableHint(bool retrievable) {}

void ProgramWgpu::setSeparable(bool separable) {}

angle::Result ProgramWgpu::link(const gl::Context *context, std::shared_ptr<LinkTask> *linkTaskOut)
{
    wgpu::Device device     = webgpu::GetDevice(context);
    wgpu::Instance instance = webgpu::GetInstance(context);

    *linkTaskOut = std::shared_ptr<LinkTask>(new LinkTaskWgpu(instance, device, this));
    return angle::Result::Continue;
}

GLboolean ProgramWgpu::validate(const gl::Caps &caps)
{
    return GL_TRUE;
}

}  // namespace rx
