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

#ifndef SRC_DAWN_NATIVE_OPENGL_PIPELINEGL_H_
#define SRC_DAWN_NATIVE_OPENGL_PIPELINEGL_H_

#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/ityp_vector.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/PerStage.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/opengl/IntegerTypes.h"
#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native {
struct ProgrammableStage;
}  // namespace dawn::native

namespace dawn::native::opengl {

struct OpenGLFunctions;
class Buffer;
class PipelineLayout;
class Sampler;
class TextureView;

enum class EmulatedTextureMetadata : uint8_t {
    NumLevels,
    NumSamples,
};

// On OpenGL we need to emulate some texture queries in WGSL that are not available in GLSL:
//  - textureNumLevels on all single-sampled texture_2d/depth/...
//  - textureNumSamples on all multisampled texture_(depth_)multisampled_2d
enum class TextureQuery : uint8_t {
    NumLevels,
    NumSamples,
};
// Information for emulated queries are passed in a UBO so we need to know for each texture in the
// pipeline what data will be present and at which offset in the UBO.
struct EmulatedTextureBuiltin {
    // The index in the UBO of emulated builtin data.
    uint32_t index;
    TextureQuery query;
    // The group is needed to dirty bind groups when changing pipelines.
    // TODO(crbug.com/408065421): Remove the need for this by not dirtying the whole BingGroup in
    // this case.
    BindGroupIndex group;
};
using EmulatedTextureBuiltinInfo = absl::flat_hash_map<FlatBindingIndex, EmulatedTextureBuiltin>;

class PipelineGL {
  public:
    PipelineGL();
    ~PipelineGL();

    const std::vector<TextureUnit>& GetTextureUnitsForSampler(FlatBindingIndex index) const;
    const std::vector<TextureUnit>& GetTextureUnitsForTextureView(FlatBindingIndex index) const;
    GLuint GetProgramHandle() const;

    const EmulatedTextureBuiltinInfo& GetEmulatedTextureBuiltinInfo() const;
    bool NeedsTextureBuiltinUniformBuffer() const;

    bool NeedsSSBOLengthUniformBuffer() const;

  protected:
    MaybeError ApplyNow(const OpenGLFunctions& gl, const PipelineLayout* layout);
    MaybeError InitializeBase(const OpenGLFunctions& gl,
                              const PipelineLayout* layout,
                              const PerStage<ProgrammableStage>& stages,
                              bool usesVertexIndex,
                              bool usesInstanceIndex,
                              bool usesFragDepth,
                              VertexAttributeMask bgraSwizzleAttributes);
    void DeleteProgram(const OpenGLFunctions& gl);

  private:
    GLuint mProgram;
    ityp::vector<FlatBindingIndex, std::vector<TextureUnit>> mUnitsForSamplers;
    ityp::vector<FlatBindingIndex, std::vector<TextureUnit>> mUnitsForTextures;
    std::vector<TextureUnit> mPlaceholderSamplerUnits;
    // TODO(enga): This could live on the Device, or elsewhere, but currently it makes Device
    // destruction complex as it requires the sampler to be destroyed before the sampler cache.
    Ref<Sampler> mPlaceholderSampler;

    // Flag indicates if this pipeline has ssbo.length and need to use the array length from uniform
    // workaround.
    bool mNeedsSSBOLengthUniformBuffer = false;

    // Reflect info from tint: a map from texture binding point to extra data need to push into the
    // internal uniform buffer.
    EmulatedTextureBuiltinInfo mEmulatedTextureBuiltinInfo;
};

// Helper class used to allocate the emulated texture builtins in the UBO during the initialization
// of the pipeline and the compilation of shaders. It is necessary because the same metadata may be
// used by multiple shader stages and should be reused between them.
class EmulatedTextureBuiltinRegistrar {
  public:
    explicit EmulatedTextureBuiltinRegistrar(const PipelineLayout* layout);

    // Returns the index of the emulated builtin data in the UBO.
    uint32_t Register(BindGroupIndex group, BindingIndex binding, TextureQuery query);

    EmulatedTextureBuiltinInfo AcquireInfo();

  private:
    const PipelineLayout* mLayout;
    uint32_t mCurrentIndex = 0;
    EmulatedTextureBuiltinInfo mEmulatedTextureBuiltinInfo;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_PIPELINEGL_H_
