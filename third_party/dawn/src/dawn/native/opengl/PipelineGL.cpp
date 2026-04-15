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

#include "dawn/common/Range.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/Device.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/opengl/BufferGL.h"
#include "dawn/native/opengl/DeviceGL.h"
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
                                      ImmediateConstantMask& pipelineImmediateMask,
                                      VertexAttributeMask bgraSwizzleAttributes,
                                      Extent3D* workgroupSize) {
    mProgram = DAWN_GL_TRY(gl, CreateProgram());

    // Compute the set of active stages.
    wgpu::ShaderStage activeStages = wgpu::ShaderStage::None;
    for (SingleShaderStage stage : IterateStages(kAllStages)) {
        if (stages[stage].module != nullptr) {
            activeStages |= StageBit(stage);
        }
    }

    // Create an OpenGL shader for each stage and gather the list of combined samplers.
    std::set<CombinedSampler> combinedSamplers;
    mNeedsSSBOLengthUniformBuffer = false;
    std::vector<GLuint> glShaders;
    EmulatedTextureBuiltinRegistrar emulatedTextureBuiltins(layout);
    for (SingleShaderStage stage : IterateStages(activeStages)) {
        ShaderModule* module = ToBackend(stages[stage].module.Get());
        bool needsSSBOLengthUniformBuffer = false;
        std::vector<CombinedSampler> stageCombinedSamplers;
        Extent3D localWorkgroupSize;
        GLuint shader;
        DAWN_TRY_ASSIGN(
            shader, module->CompileShader(gl, stages[stage], stage, pipelineImmediateMask,
                                          bgraSwizzleAttributes, &stageCombinedSamplers, layout,
                                          &emulatedTextureBuiltins, &needsSSBOLengthUniformBuffer,
                                          &localWorkgroupSize));
        if (stage == SingleShaderStage::Compute) {
            *workgroupSize = localWorkgroupSize;
        }

        mNeedsSSBOLengthUniformBuffer |= needsSSBOLengthUniformBuffer;
        combinedSamplers.insert(stageCombinedSamplers.begin(), stageCombinedSamplers.end());

        DAWN_GL_TRY(gl, AttachShader(mProgram, shader));
        glShaders.push_back(shader);
    }

    mEmulatedTextureBuiltinInfo = emulatedTextureBuiltins.AcquireInfo();

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

    mUnitsForSamplers.resize(layout->GetNumSamplers());
    mUnitsForTextures.resize(layout->GetNumSampledTextures());

    // Assign combined texture/samplers to GL texture units.
    TextureUnit textureUnit{0};
    for (const auto& combined : combinedSamplers) {
        // All the texture/samplers of a binding_array are set in a single glUniform1iv, gather them
        // all in this vector.
        absl::InlinedVector<GLint, 1> uniformsToSet;

        BindingIndex textureArrayStart = combined.textureLocation.index;
        for (auto textureArrayElement : Range(combined.textureLocation.shaderArraySize)) {
            FlatBindingIndex textureGLIndex =
                indices[combined.textureLocation.group][textureArrayStart + textureArrayElement];
            mUnitsForTextures[textureGLIndex].push_back(textureUnit);

            // Record that the placeholder sampler must be set for this texture unit if no sampler
            // is used in the shader.
            if (!combined.samplerLocation) {
                mPlaceholderSamplerUnits.push_back(textureUnit);
            } else {
                // Record that the sampler used in the shader must be set for this texture unit.
                BindingIndex samplerBindingIndex = combined.samplerLocation->index;
                FlatBindingIndex samplerGLIndex =
                    indices[combined.samplerLocation->group][samplerBindingIndex];
                mUnitsForSamplers[samplerGLIndex].push_back(textureUnit);
            }

            uniformsToSet.push_back(GLint(textureUnit));
            textureUnit++;
        }

        std::string name = combined.GetName();
        GLint location = DAWN_GL_TRY(gl, GetUniformLocation(mProgram, name.c_str()));
        // Non-arrayed GLSL variables cannot be set with glUniform1iv
        if (uniformsToSet.size() == 1) {
            DAWN_GL_TRY(gl, Uniform1i(location, uniformsToSet[0]));
        } else {
            DAWN_GL_TRY(gl, Uniform1iv(location, uniformsToSet.size(), uniformsToSet.data()));
        }
    }

    if (!mPlaceholderSamplerUnits.empty()) {
        Ref<SamplerBase> sampler;
        DAWN_TRY_ASSIGN(sampler, layout->GetDevice()->CreateSampler());
        mPlaceholderSampler = ToBackend(std::move(sampler));
    }

    // If the pipeline declares immediates but the GL driver determines that they are unused and
    // optimizes out the uniform variable, reset the mask. This prevents a GL_INVALID_VALUE error
    // when trying to update it via glUniform*().
    if (pipelineImmediateMask.any()) {
        auto location = DAWN_GL_TRY(gl, GetUniformLocation(mProgram, "tint_immediates"));
        if (location == -1) {
            pipelineImmediateMask.reset();
        }
    }

    for (GLuint glShader : glShaders) {
        DAWN_GL_TRY(gl, DetachShader(mProgram, glShader));
        DAWN_GL_TRY(gl, DeleteShader(glShader));
    }

    return {};
}

const std::vector<TextureUnit>& PipelineGL::GetTextureUnitsForSampler(
    FlatBindingIndex index) const {
    DAWN_ASSERT(index < mUnitsForSamplers.size());
    return mUnitsForSamplers[index];
}

const std::vector<TextureUnit>& PipelineGL::GetTextureUnitsForTextureView(
    FlatBindingIndex index) const {
    DAWN_ASSERT(index < mUnitsForTextures.size());
    return mUnitsForTextures[index];
}

MaybeError PipelineGL::ApplyNow(const OpenGLFunctions& gl, const PipelineLayout* layout) {
    DAWN_GL_TRY(gl, UseProgram(mProgram));
    for (TextureUnit unit : mPlaceholderSamplerUnits) {
        DAWN_ASSERT(mPlaceholderSampler.Get() != nullptr);
        DAWN_GL_TRY(gl, BindSampler(GLuint(unit), mPlaceholderSampler->GetHandle()));
    }

    return {};
}

const EmulatedTextureBuiltinInfo& PipelineGL::GetEmulatedTextureBuiltinInfo() const {
    return mEmulatedTextureBuiltinInfo;
}

bool PipelineGL::NeedsTextureBuiltinUniformBuffer() const {
    return !mEmulatedTextureBuiltinInfo.empty();
}

bool PipelineGL::NeedsSSBOLengthUniformBuffer() const {
    return mNeedsSSBOLengthUniformBuffer;
}

// EmulatedTextureBuiltinRegistrar

EmulatedTextureBuiltinRegistrar::EmulatedTextureBuiltinRegistrar(const PipelineLayout* layout)
    : mLayout(layout) {}

uint32_t EmulatedTextureBuiltinRegistrar::Register(BindGroupIndex group,
                                                   BindingIndex binding,
                                                   TextureQuery query) {
    FlatBindingIndex firstTextureIndex = mLayout->GetBindingIndexInfo()[group][binding];

    if (!mEmulatedTextureBuiltinInfo.contains(firstTextureIndex)) {
        // Register the metadata to add for each element of the binding_array (if there is one).
        BindingIndex arraySize =
            mLayout->GetBindGroupLayout(group)->GetBindingInfo(binding).arraySize;
        for (BindingIndex arrayElement : Range(arraySize)) {
            FlatBindingIndex textureIndex =
                mLayout->GetBindingIndexInfo()[group][binding + arrayElement];

            mEmulatedTextureBuiltinInfo.emplace(
                textureIndex,
                EmulatedTextureBuiltin{.index = mCurrentIndex, .query = query, .group = group});
            mCurrentIndex++;
        }
    }

    return mEmulatedTextureBuiltinInfo[firstTextureIndex].index;
}

EmulatedTextureBuiltinInfo EmulatedTextureBuiltinRegistrar::AcquireInfo() {
    return mEmulatedTextureBuiltinInfo;
}

}  // namespace dawn::native::opengl
