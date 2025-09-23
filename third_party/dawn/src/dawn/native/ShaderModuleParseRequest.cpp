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

#include "dawn/native/ShaderModuleParseRequest.h"

#include <unordered_set>
#include <utility>
#include <vector>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/ShaderModule.h"

namespace dawn::native {

WGSLAllowedFeatures WGSLAllowedFeatures::FromTint(tint::wgsl::AllowedFeatures allowedFeatures) {
    return {{
        .extensions = std::move(allowedFeatures.extensions),
        .features = std::move(allowedFeatures.features),
    }};
}

tint::wgsl::AllowedFeatures WGSLAllowedFeatures::ToTint() const {
    return tint::wgsl::AllowedFeatures{
        .extensions = extensions,
        .features = features,
    };
}

ShaderModuleParseRequest BuildShaderModuleParseRequest(
    DeviceBase* device,
    ShaderModuleBase::ShaderModuleHash shaderModuleHash,
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions,
    bool needReflection) {
    ShaderModuleParseRequest req;
    req.logEmitter = UnsafeUnserializedValue<LogEmitter*>(device);
    req.deviceInfo = {
        {.toggles = device->GetTogglesState().GetEnabledToggles(),
         .features = device->GetEnabledFeatures(),
         .limits = LimitsForShaderModuleParseRequest::Create(device->GetLimits().v1),
         .wgslAllowedFeatures = WGSLAllowedFeatures::FromTint(device->GetWGSLAllowedFeatures()),
         .isCompatibilityMode = device->IsCompatibilityMode()}};
    req.shaderModuleHash = shaderModuleHash;
    req.needReflection = needReflection;

// Assuming the descriptor chain has already been validated.
#if TINT_BUILD_SPV_READER
    // Handling SPIR-V if enabled.
    if (const auto* spirvDesc = descriptor.Get<ShaderSourceSPIRV>()) {
        // SPIRV toggle and instance feature should have been validated before checking cache.
        DAWN_ASSERT(!device->IsToggleEnabled(Toggle::DisallowSpirv));
        DAWN_ASSERT(
            device->GetInstance()->HasFeature(wgpu::InstanceFeatureName::ShaderSourceSPIRV));
        // Descriptor should not contain WGSL part.
        DAWN_ASSERT(!descriptor.Has<ShaderSourceWGSL>());

        const auto* spirvOptions = descriptor.Get<DawnShaderModuleSPIRVOptionsDescriptor>();
        DAWN_ASSERT(spirvDesc != nullptr);

        ShaderModuleParseSpirvDescription spirv = {
            {// TODO(dawn:2033): Avoid unnecessary copies of the SPIR-V code.
             .spirvCode = UnsafeUnserializedValue(
                 std::vector<uint32_t>(spirvDesc->code, spirvDesc->code + spirvDesc->codeSize)),
             .allowNonUniformDerivatives =
                 spirvOptions ? static_cast<bool>(spirvOptions->allowNonUniformDerivatives)
                              : false}};

        req.shaderDescription = std::move(spirv);
        return req;
    }
#else   // TINT_BUILD_SPV_READER
    // SPIR-V is not enabled, so the descriptor should not contain it.
    DAWN_ASSERT(!descriptor.Has<ShaderSourceSPIRV>());
#endif  // TINT_BUILD_SPV_READER

    // Handling WGSL.
    const ShaderSourceWGSL* wgslDesc = descriptor.Get<ShaderSourceWGSL>();
    DAWN_ASSERT(wgslDesc != nullptr);

    req.shaderDescription = ShaderModuleParseWGSLDescription{
        {.wgsl = UnsafeUnserializedValue(wgslDesc->code),
         .internalExtensions = UnsafeUnserializedValue(internalExtensions)}};

    return req;
}
}  // namespace dawn::native
