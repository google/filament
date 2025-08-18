// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_SHADERMODULEPARSEREQUEST_H_
#define SRC_DAWN_NATIVE_SHADERMODULEPARSEREQUEST_H_

#include <unordered_set>
#include <vector>

#include "dawn/native/CacheRequest.h"
#include "dawn/native/Limits.h"
#include "dawn/native/Serializable.h"
#include "dawn/native/ShaderModule.h"

namespace dawn::native {

// Mapping tint::wgsl::AllowedFeatures
#define WGSL_ALLOWED_FEATURES_MEMBER(X)                      \
    X(std::unordered_set<tint::wgsl::Extension>, extensions) \
    X(std::unordered_set<tint::wgsl::LanguageFeature>, features)
// clang-format off
DAWN_SERIALIZABLE(struct, WGSLAllowedFeatures, WGSL_ALLOWED_FEATURES_MEMBER){
    static WGSLAllowedFeatures FromTint(tint::wgsl::AllowedFeatures allowedFeatures);
    tint::wgsl::AllowedFeatures ToTint() const;
};
// clang-format on
#undef WGSL_ALLOWED_FEATURES_MEMBER

#define SHADER_MODULE_PARSE_DEVICE_INFO_MEMBER(X) \
    /* Toggles, features, and limits */           \
    X(TogglesSet, toggles)                        \
    X(FeaturesSet, features)                      \
    X(LimitsForShaderModuleParseRequest, limits)  \
    X(WGSLAllowedFeatures, wgslAllowedFeatures)   \
    X(bool, isCompatibilityMode)
DAWN_SERIALIZABLE(struct, ShaderModuleParseDeviceInfo, SHADER_MODULE_PARSE_DEVICE_INFO_MEMBER){};
#undef SHADER_MODULE_PARSE_DEVICE_INFO_MEMBER

#define SHADER_MODULE_PARSE_SPIRV_DESCRIPTION_MEMBER(X)                            \
    /* Don't need to key the spirv code since it is hashed in shaderModuleHash. */ \
    X(UnsafeUnserializedValue<std::vector<uint32_t>>, spirvCode)                   \
    X(bool, allowNonUniformDerivatives)
DAWN_SERIALIZABLE(struct,
                  ShaderModuleParseSpirvDescription,
                  SHADER_MODULE_PARSE_SPIRV_DESCRIPTION_MEMBER){};
#undef SHADER_MODULE_PARSE_SPIRV_DESCRIPTION_MEMBER

#define SHADER_MODULE_PARSE_WGSL_DESCRIPTION_MEMBER(X)                           \
    /* Don't need to key the WGSL code and internal extensions since they are */ \
    /* hashed in shaderModuleHash. */                                            \
    X(UnsafeUnserializedValue<StringView>, wgsl)                                 \
    X(UnsafeUnserializedValue<std::vector<tint::wgsl::Extension>>, internalExtensions)
DAWN_SERIALIZABLE(struct,
                  ShaderModuleParseWGSLDescription,
                  SHADER_MODULE_PARSE_WGSL_DESCRIPTION_MEMBER){};
#undef SHADER_MODULE_PARSE_WGSL_DESCRIPTION_MEMBER

using ShaderModuleParseDescriptionVariant =
    std::variant<ShaderModuleParseSpirvDescription, ShaderModuleParseWGSLDescription>;

#define SHADER_MODULE_PARSE_REQUEST_MEMBER(X)                 \
    X(UnsafeUnserializedValue<LogEmitter*>, logEmitter)       \
    X(ShaderModuleParseDeviceInfo, deviceInfo)                \
    X(ShaderModuleBase::ShaderModuleHash, shaderModuleHash)   \
    X(ShaderModuleParseDescriptionVariant, shaderDescription) \
    X(bool, needReflection)

DAWN_MAKE_CACHE_REQUEST(ShaderModuleParseRequest, SHADER_MODULE_PARSE_REQUEST_MEMBER);
#undef SHADER_MODULE_PARSE_REQUEST_MEMBER

// Helper function to create a ShaderModuleParseRequest
ShaderModuleParseRequest BuildShaderModuleParseRequest(
    DeviceBase* device,
    ShaderModuleBase::ShaderModuleHash shaderModuleHash,
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions,
    bool needReflection);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SHADERMODULEPARSEREQUEST_H_
