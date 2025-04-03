// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/opengl/PipelineGL.h"

#include <algorithm>
#include <set>
#include <sstream>
#include <string>

#include "dawn/common/BitSetIterator.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/Device.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/opengl/BufferGL.h"
#include "dawn/native/opengl/Forward.h"
#include "dawn/native/opengl/OpenGLFunctions.h"
#include "dawn/native/opengl/PipelineLayoutGL.h"
#include "dawn/native/opengl/SamplerGL.h"
#include "dawn/native/opengl/ShaderModuleGL.h"
#include "dawn/native/opengl/TextureGL.h"
#include "dawn/native/opengl/UtilsGL.h"

namespace dawn::native::opengl {

PipelineGL::PipelineGL() : mProgram(0) {}

PipelineGL::~PipelineGL() = default;

MaybeError PipelineGL::InitializeBase(const OpenGLFunctions& gl,
                                      const PipelineLayout* layout,
                                      const PerStage<ProgrammableStage>& stages,
                                      bool usesVertexIndex,
                                      bool usesInstanceIndex,
                                      bool usesFragDepth,
                                      VertexAttributeMask bgraSwizzleAttributes) {
    mProgram = DAWN_GL_TRY(gl, CreateProgram());

    // Compute the set of active stages.
    wgpu::ShaderStage activeStages = wgpu::ShaderStage::None;
    for (SingleShaderStage stage : IterateStages(kAllStages)) {
        if (stages[stage].module != nullptr) {
            activeStages |= StageBit(stage);
        }
    }

    // Create an OpenGL shader for each stage and gather the list of combined samplers.
    PerStage<CombinedSamplerInfo> combinedSamplers;
    bool needsPlaceholderSampler = false;
    std::vector<GLuint> glShaders;
    for (SingleShaderStage stage : IterateStages(activeStages)) {
        ShaderModule* module = ToBackend(stages[stage].module.Get());
        GLuint shader;
        DAWN_TRY_ASSIGN(
            shader,
            module->CompileShader(
                gl, stages[stage], stage, usesVertexIndex, usesInstanceIndex, usesFragDepth,
                bgraSwizzleAttributes, &combinedSamplers[stage], layout, &needsPlaceholderSampler,
                &mNeedsTextureBuiltinUniformBuffer, &mBindingPointEmulatedBuiltins));
        // XXX transform to flip some attributes from RGBA to BGRA
        DAWN_GL_TRY(gl, AttachShader(mProgram, shader));
        glShaders.push_back(shader);
    }

    if (needsPlaceholderSampler) {
        SamplerDescriptor desc = {};
        DAWN_ASSERT(desc.minFilter == wgpu::FilterMode::Nearest);
        DAWN_ASSERT(desc.magFilter == wgpu::FilterMode::Nearest);
        DAWN_ASSERT(desc.mipmapFilter == wgpu::MipmapFilterMode::Nearest);
        Ref<SamplerBase> sampler;
        DAWN_TRY_ASSIGN(sampler, layout->GetDevice()->GetOrCreateSampler(&desc));
        mPlaceholderSampler = ToBackend(std::move(sampler));
    }

    if (!mBindingPointEmulatedBuiltins.empty()) {
        BufferDescriptor desc = {};
        desc.size = mBindingPointEmulatedBuiltins.size() * sizeof(uint32_t);
        desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        Ref<BufferBase> buffer;
        DAWN_TRY_ASSIGN(buffer, layout->GetDevice()->CreateBuffer(&desc));
        mTextureBuiltinsBuffer = ToBackend(std::move(buffer));
    }

    // Link all the shaders together.
    DAWN_GL_TRY(gl, LinkProgram(mProgram));

    GLint linkStatus = GL_FALSE;
    DAWN_GL_TRY(gl, GetProgramiv(mProgram, GL_LINK_STATUS, &linkStatus));
    if (linkStatus == GL_FALSE) {
        GLint infoLogLength = 0;
        DAWN_GL_TRY(gl, GetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &infoLogLength));

        if (infoLogLength > 1) {
            std::vector<char> buffer(infoLogLength);
            DAWN_GL_TRY(gl, GetProgramInfoLog(mProgram, infoLogLength, nullptr, &buffer[0]));
            return DAWN_VALIDATION_ERROR("Program link failed:\n%s", buffer.data());
        }
    }

    // Compute links between stages for combined samplers, then bind them to texture units
    DAWN_GL_TRY(gl, UseProgram(mProgram));
    const auto& indices = layout->GetBindingIndexInfo();

    std::set<CombinedSampler> combinedSamplersSet;
    for (SingleShaderStage stage : IterateStages(activeStages)) {
        for (const CombinedSampler& combined : combinedSamplers[stage]) {
            combinedSamplersSet.insert(combined);
        }
    }

    mUnitsForSamplers.resize(layout->GetNumSamplers());
    mUnitsForTextures.resize(layout->GetNumSampledTextures());

    GLuint textureUnit = layout->GetTextureUnitsUsed();
    for (const auto& combined : combinedSamplersSet) {
        const std::string& name = combined.GetName();
        GLint location = DAWN_GL_TRY(gl, GetUniformLocation(mProgram, name.c_str()));

        if (location == -1) {
            continue;
        }

        DAWN_GL_TRY(gl, Uniform1i(location, textureUnit));

        bool shouldUseFiltering;
        {
            const BindGroupLayoutInternalBase* bgl =
                layout->GetBindGroupLayout(combined.textureLocation.group);
            BindingIndex bindingIndex = bgl->GetBindingIndex(combined.textureLocation.binding);

            GLuint textureIndex = indices[combined.textureLocation.group][bindingIndex];
            mUnitsForTextures[textureIndex].push_back(textureUnit);

            const auto& bindingLayout = bgl->GetBindingInfo(bindingIndex).bindingLayout;
            auto sampleType = std::get<TextureBindingInfo>(bindingLayout).sampleType;
            shouldUseFiltering = sampleType == wgpu::TextureSampleType::Float ||
                                 sampleType == wgpu::TextureSampleType::Depth;
        }
        {
            if (combined.usePlaceholderSampler) {
                mPlaceholderSamplerUnits.push_back(textureUnit);
            } else {
                const BindGroupLayoutInternalBase* bgl =
                    layout->GetBindGroupLayout(combined.samplerLocation.group);
                BindingIndex bindingIndex = bgl->GetBindingIndex(combined.samplerLocation.binding);

                GLuint samplerIndex = indices[combined.samplerLocation.group][bindingIndex];
                mUnitsForSamplers[samplerIndex].push_back({textureUnit, shouldUseFiltering});
            }
        }

        textureUnit++;
    }

    for (GLuint glShader : glShaders) {
        DAWN_GL_TRY(gl, DetachShader(mProgram, glShader));
        DAWN_GL_TRY(gl, DeleteShader(glShader));
    }

    mInternalUniformBufferBinding = layout->GetInternalUniformBinding();

    return {};
}

void PipelineGL::DeleteProgram(const OpenGLFunctions& gl) {
    DAWN_GL_TRY_IGNORE_ERRORS(gl, DeleteProgram(mProgram));
}

const std::vector<PipelineGL::SamplerUnit>& PipelineGL::GetTextureUnitsForSampler(
    GLuint index) const {
    DAWN_ASSERT(index < mUnitsForSamplers.size());
    return mUnitsForSamplers[index];
}

const std::vector<GLuint>& PipelineGL::GetTextureUnitsForTextureView(GLuint index) const {
    DAWN_ASSERT(index < mUnitsForTextures.size());
    return mUnitsForTextures[index];
}

GLuint PipelineGL::GetProgramHandle() const {
    return mProgram;
}

MaybeError PipelineGL::ApplyNow(const OpenGLFunctions& gl) {
    DAWN_GL_TRY(gl, UseProgram(mProgram));
    for (GLuint unit : mPlaceholderSamplerUnits) {
        DAWN_ASSERT(mPlaceholderSampler.Get() != nullptr);
        DAWN_GL_TRY(gl, BindSampler(unit, mPlaceholderSampler->GetNonFilteringHandle()));
    }

    if (mTextureBuiltinsBuffer.Get() != nullptr) {
        DAWN_GL_TRY(gl, BindBufferBase(GL_UNIFORM_BUFFER, mInternalUniformBufferBinding,
                                       mTextureBuiltinsBuffer->GetHandle()));
    }
    return {};
}

const Buffer* PipelineGL::GetInternalUniformBuffer() const {
    return mTextureBuiltinsBuffer.Get();
}

const BindingPointToFunctionAndOffset& PipelineGL::GetBindingPointBuiltinDataInfo() const {
    return mBindingPointEmulatedBuiltins;
}

}  // namespace dawn::native::opengl
