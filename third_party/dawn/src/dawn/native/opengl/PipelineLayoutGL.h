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

#ifndef SRC_DAWN_NATIVE_OPENGL_PIPELINELAYOUTGL_H_
#define SRC_DAWN_NATIVE_OPENGL_PIPELINELAYOUTGL_H_

#include "dawn/native/PipelineLayout.h"

#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_vector.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/opengl/IntegerTypes.h"
#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native::opengl {

// According to gpuinfo.org, devices report GL_MAX_TEXTURE_IMAGE_UNITS <= 128
static constexpr size_t kGLMaxTextureImageUnitsReported = 128;

// According to gpuinfo.org, devices report GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS <= 96
static constexpr size_t kGLMaxShaderStorageBufferBindingsReported = 96;

class Device;

class PipelineLayout final : public PipelineLayoutBase {
  public:
    PipelineLayout(Device* device, const UnpackedPtr<PipelineLayoutDescriptor>& descriptor);

    // GL backend does not support separate bind group index
    // BindingIndexInfo is a map from BindingPoint(group, binding) to a flattened GLuint binding
    // number.
    using BindingIndexInfo = PerBindGroup<ityp::vector<BindingIndex, FlatBindingIndex>>;
    const BindingIndexInfo& GetBindingIndexInfo() const;

    FlatBindingIndex GetNumSamplers() const;
    FlatBindingIndex GetNumSampledTextures() const;
    FlatBindingIndex GetNumSSBO() const;

    FlatBindingIndex GetInternalTextureBuiltinsUniformBinding() const;
    FlatBindingIndex GetInternalArrayLengthUniformBinding() const;

    enum ImmediateLocation {
        FirstVertex = 0,
        FirstInstance = 1,
        MinDepth = 2,
        MaxDepth = 3,
    };

  private:
    ~PipelineLayout() override = default;
    BindingIndexInfo mIndexInfo;
    FlatBindingIndex mNumSamplers;
    FlatBindingIndex mNumSampledTextures;
    FlatBindingIndex mNumSSBO;

    FlatBindingIndex mInternalTextureBuiltinsUniformBinding;
    FlatBindingIndex mInternalArrayLengthUniformBinding;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_PIPELINELAYOUTGL_H_
