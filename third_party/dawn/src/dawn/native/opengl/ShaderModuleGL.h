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

#ifndef SRC_DAWN_NATIVE_OPENGL_SHADERMODULEGL_H_
#define SRC_DAWN_NATIVE_OPENGL_SHADERMODULEGL_H_

#include <string>
#include <vector>

#include "dawn/native/IntegerTypes.h"
#include "dawn/native/Serializable.h"
#include "dawn/native/ShaderModule.h"
#include "dawn/native/opengl/BindingPoint.h"
#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native {

struct ProgrammableStage;

namespace stream {
class Sink;
class Source;
}  // namespace stream

namespace opengl {

class Device;
class PipelineLayout;
struct OpenGLFunctions;

std::string GetBindingName(BindGroupIndex group, BindingNumber bindingNumber);

#define BINDING_LOCATION_MEMBERS(X) \
    X(BindGroupIndex, group)        \
    X(BindingNumber, binding)
DAWN_SERIALIZABLE(struct, BindingLocation, BINDING_LOCATION_MEMBERS){};
#undef BINDING_LOCATION_MEMBERS

bool operator<(const BindingLocation& a, const BindingLocation& b);

#define COMBINED_SAMPLER_MEMBERS(X)                                                         \
    X(BindingLocation, samplerLocation)                                                     \
    X(BindingLocation, textureLocation)                                                     \
    /* OpenGL requires a sampler with texelFetch. If this is true, the developer did not */ \
    /* provide one and Dawn should bind a placeholder non-filtering sampler;  */            \
    /* |samplerLocation| is unused. */                                                      \
    X(bool, usePlaceholderSampler)

DAWN_SERIALIZABLE(struct, CombinedSampler, COMBINED_SAMPLER_MEMBERS) {
    std::string GetName() const;
};
#undef COMBINED_SAMPLER_MEMBERS

bool operator<(const CombinedSampler& a, const CombinedSampler& b);

using CombinedSamplerInfo = std::vector<CombinedSampler>;

class ShaderModule final : public ShaderModuleBase {
  public:
    static ResultOrError<Ref<ShaderModule>> Create(
        Device* device,
        const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
        const std::vector<tint::wgsl::Extension>& internalExtensions,
        ShaderModuleParseResult* parseResult,
        OwnedCompilationMessages* compilationMessages);

    ResultOrError<GLuint> CompileShader(const OpenGLFunctions& gl,
                                        const ProgrammableStage& programmableStage,
                                        SingleShaderStage stage,
                                        bool usesVertexIndex,
                                        bool usesInstanceIndex,
                                        bool usesFragDepth,
                                        VertexAttributeMask bgraSwizzleAttributes,
                                        CombinedSamplerInfo* combinedSamplers,
                                        const PipelineLayout* layout,
                                        bool* needsPlaceholderSampler,
                                        bool* needsTextureBuiltinUniformBuffer,
                                        BindingPointToFunctionAndOffset* bindingPointToData);

  private:
    ShaderModule(Device* device,
                 const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                 std::vector<tint::wgsl::Extension> internalExtensions);
    ~ShaderModule() override = default;
    MaybeError Initialize(ShaderModuleParseResult* parseResult,
                          OwnedCompilationMessages* compilationMessages);
};

}  // namespace opengl
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_OPENGL_SHADERMODULEGL_H_
