// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/d3d11/ShaderModuleD3D11.h"

#include <string>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Log.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/d3d/D3DCompilationRequest.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/BackendD3D11.h"
#include "dawn/native/d3d11/BindGroupLayoutD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/PhysicalDeviceD3D11.h"
#include "dawn/native/d3d11/PipelineLayoutD3D11.h"
#include "dawn/native/d3d11/PlatformFunctionsD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/metrics/HistogramMacros.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include "tint/tint.h"

namespace dawn::native::d3d11 {

// static
ResultOrError<Ref<ShaderModule>> ShaderModule::Create(
    Device* device,
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions,
    ShaderModuleParseResult* parseResult,
    OwnedCompilationMessages* compilationMessages) {
    Ref<ShaderModule> module = AcquireRef(new ShaderModule(device, descriptor, internalExtensions));
    DAWN_TRY(module->Initialize(parseResult, compilationMessages));
    return module;
}

ShaderModule::ShaderModule(Device* device,
                           const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                           std::vector<tint::wgsl::Extension> internalExtensions)
    : ShaderModuleBase(device, descriptor, std::move(internalExtensions)) {}

MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult,
                                    OwnedCompilationMessages* compilationMessages) {
    ScopedTintICEHandler scopedICEHandler(GetDevice());
    return InitializeBase(parseResult, compilationMessages);
}

ResultOrError<d3d::CompiledShader> ShaderModule::Compile(
    const ProgrammableStage& programmableStage,
    SingleShaderStage stage,
    const PipelineLayout* layout,
    uint32_t compileFlags,
    const std::optional<dawn::native::d3d::InterStageShaderVariablesMask>& usedInterstageVariables,
    const std::optional<tint::hlsl::writer::PixelLocalOptions>& pixelLocalOptions) {
    Device* device = ToBackend(GetDevice());
    TRACE_EVENT0(device->GetPlatform(), General, "ShaderModuleD3D11::Compile");
    DAWN_ASSERT(!IsError());

    ScopedTintICEHandler scopedICEHandler(device);
    const EntryPointMetadata& entryPoint = GetEntryPoint(programmableStage.entryPoint);
    const bool useTintIR = device->IsToggleEnabled(Toggle::UseTintIR);

    d3d::D3DCompilationRequest req = {};
    req.tracePlatform = UnsafeUnkeyedValue(device->GetPlatform());
    req.hlsl.shaderModel = 50;
    req.hlsl.disableSymbolRenaming = device->IsToggleEnabled(Toggle::DisableSymbolRenaming);
    req.hlsl.dumpShaders = device->IsToggleEnabled(Toggle::DumpShaders);
    req.hlsl.useTintIR = useTintIR;

    req.bytecode.hasShaderF16Feature = false;
    req.bytecode.compileFlags = compileFlags;

    // D3D11 only supports FXC.
    req.bytecode.compiler = d3d::Compiler::FXC;
    req.bytecode.d3dCompile = device->GetFunctions()->d3dCompile;
    req.bytecode.compilerVersion = D3D_COMPILER_VERSION;
    DAWN_ASSERT(device->GetDeviceInfo().shaderModel == 50);
    switch (stage) {
        case SingleShaderStage::Vertex:
            req.bytecode.fxcShaderProfile = "vs_5_0";
            break;
        case SingleShaderStage::Fragment:
            req.bytecode.fxcShaderProfile = "ps_5_0";
            break;
        case SingleShaderStage::Compute:
            req.bytecode.fxcShaderProfile = "cs_5_0";
            break;
    }

    const BindingInfoArray& moduleBindingInfo = entryPoint.bindings;

    tint::hlsl::writer::Bindings bindings;

    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        const BindGroupLayout* bgl = ToBackend(layout->GetBindGroupLayout(group));
        const auto& indices = layout->GetBindingIndexInfo()[group];
        const BindingGroupInfoMap& moduleGroupBindingInfo = moduleBindingInfo[group];

        for (const auto& [binding, shaderBindingInfo] : moduleGroupBindingInfo) {
            BindingIndex bindingIndex = bgl->GetBindingIndex(binding);
            tint::BindingPoint srcBindingPoint{static_cast<uint32_t>(group),
                                               static_cast<uint32_t>(binding)};
            tint::BindingPoint dstBindingPoint{0u, indices[bindingIndex]};
            auto* const bufferBindingInfo =
                std::get_if<BufferBindingInfo>(&shaderBindingInfo.bindingInfo);

            if (bufferBindingInfo) {
                switch (bufferBindingInfo->type) {
                    case wgpu::BufferBindingType::Uniform:
                        bindings.uniform.emplace(
                            srcBindingPoint, tint::hlsl::writer::binding::Uniform{
                                                 dstBindingPoint.group, dstBindingPoint.binding});
                        break;
                    case kInternalStorageBufferBinding:
                    case wgpu::BufferBindingType::Storage:
                    case wgpu::BufferBindingType::ReadOnlyStorage:
                        bindings.storage.emplace(
                            srcBindingPoint, tint::hlsl::writer::binding::Storage{
                                                 dstBindingPoint.group, dstBindingPoint.binding});
                        break;
                    case wgpu::BufferBindingType::BindingNotUsed:
                    case wgpu::BufferBindingType::Undefined:
                        DAWN_UNREACHABLE();
                        break;
                }
            } else if (std::holds_alternative<SamplerBindingInfo>(shaderBindingInfo.bindingInfo)) {
                bindings.sampler.emplace(
                    srcBindingPoint, tint::hlsl::writer::binding::Sampler{dstBindingPoint.group,
                                                                          dstBindingPoint.binding});
            } else if (std::holds_alternative<TextureBindingInfo>(shaderBindingInfo.bindingInfo)) {
                bindings.texture.emplace(
                    srcBindingPoint, tint::hlsl::writer::binding::Texture{dstBindingPoint.group,
                                                                          dstBindingPoint.binding});
            } else if (std::holds_alternative<StorageTextureBindingInfo>(
                           shaderBindingInfo.bindingInfo)) {
                bindings.storage_texture.emplace(
                    srcBindingPoint, tint::hlsl::writer::binding::StorageTexture{
                                         dstBindingPoint.group, dstBindingPoint.binding});
            } else if (std::holds_alternative<ExternalTextureBindingInfo>(
                           shaderBindingInfo.bindingInfo)) {
                const auto& etBindingMap = bgl->GetExternalTextureBindingExpansionMap();
                const auto& expansion = etBindingMap.find(binding);
                DAWN_ASSERT(expansion != etBindingMap.end());

                const auto& bindingExpansion = expansion->second;
                tint::hlsl::writer::binding::BindingInfo plane0{
                    0u, indices[bgl->GetBindingIndex(bindingExpansion.plane0)]};
                tint::hlsl::writer::binding::BindingInfo plane1{
                    0u, indices[bgl->GetBindingIndex(bindingExpansion.plane1)]};
                tint::hlsl::writer::binding::BindingInfo metadata{
                    0u, indices[bgl->GetBindingIndex(bindingExpansion.params)]};
                bindings.external_texture.emplace(
                    srcBindingPoint,
                    tint::hlsl::writer::binding::ExternalTexture{metadata, plane0, plane1});
            }
        }
    }

    std::optional<tint::ast::transform::SubstituteOverride::Config> substituteOverrideConfig;
    if (!programmableStage.metadata->overrides.empty()) {
        substituteOverrideConfig = BuildSubstituteOverridesTransformConfig(programmableStage);
    }

    auto tintProgram = GetTintProgram();
    req.hlsl.inputProgram = &(tintProgram->program);
    req.hlsl.entryPointName = programmableStage.entryPoint.c_str();
    req.hlsl.stage = stage;
    // Put the firstIndex into the internally reserved group and binding to avoid conflicting with
    // any existing bindings.
    if (!useTintIR) {
        req.hlsl.firstIndexOffsetRegisterSpace = PipelineLayout::kReservedConstantsBindGroupIndex;
        req.hlsl.firstIndexOffsetShaderRegister = PipelineLayout::kFirstIndexOffsetBindingNumber;
        // Remap to the desired space and binding, [0, kFirstIndexOffsetConstantBufferSlot].
        {
            tint::BindingPoint srcBindingPoint{req.hlsl.firstIndexOffsetRegisterSpace,
                                               req.hlsl.firstIndexOffsetShaderRegister};
            // D3D11 (HLSL SM5.0) doesn't support spaces, so we have to put the firstIndex in the
            // default space(0)
            tint::BindingPoint dstBindingPoint{0u,
                                               PipelineLayout::kFirstIndexOffsetConstantBufferSlot};

            bindings.uniform.emplace(srcBindingPoint,
                                     tint::hlsl::writer::binding::Uniform{dstBindingPoint.group,
                                                                          dstBindingPoint.binding});
        }
    }

    req.hlsl.substituteOverrideConfig = std::move(substituteOverrideConfig);

    const CombinedLimits& limits = device->GetLimits();
    req.hlsl.limits = LimitsForCompilationRequest::Create(limits.v1);
    req.hlsl.adapter = UnsafeUnkeyedValue(static_cast<const AdapterBase*>(device->GetAdapter()));

    req.hlsl.tintOptions.disable_robustness = !device->IsRobustnessEnabled();
    req.hlsl.tintOptions.disable_workgroup_init =
        device->IsToggleEnabled(Toggle::DisableWorkgroupInit);
    req.hlsl.tintOptions.bindings = std::move(bindings);

    if (entryPoint.usesNumWorkgroups) {
        DAWN_ASSERT(stage == SingleShaderStage::Compute);
        req.hlsl.tintOptions.root_constant_binding_point =
            tint::BindingPoint{0, PipelineLayout::kNumWorkgroupsConstantBufferSlot};
    } else if (useTintIR && stage == SingleShaderStage::Vertex) {
        // For vertex shaders, use root constant to add FirstIndexOffset, if needed
        req.hlsl.tintOptions.root_constant_binding_point =
            tint::BindingPoint{0, PipelineLayout::kFirstIndexOffsetConstantBufferSlot};
    }

    if (stage == SingleShaderStage::Vertex) {
        // Now that only vertex shader can have interstage outputs.
        // Pass in the actually used interstage locations for tint to potentially truncate unused
        // outputs.
        if (usedInterstageVariables.has_value()) {
            req.hlsl.tintOptions.interstage_locations = *usedInterstageVariables;
        }
        req.hlsl.tintOptions.truncate_interstage_variables = true;
    } else if (stage == SingleShaderStage::Fragment) {
        if (pixelLocalOptions.has_value()) {
            req.hlsl.tintOptions.pixel_local = *pixelLocalOptions;
        }
    }

    // D3D11 only supports FXC
    req.hlsl.tintOptions.compiler = tint::hlsl::writer::Options::Compiler::kFXC;

    // TODO(dawn:1705): do we need to support it?
    req.hlsl.tintOptions.polyfill_reflect_vec2_f32 = false;

    // D3D11 doesn't support shader model 6+ features
    req.hlsl.tintOptions.polyfill_dot_4x8_packed = true;
    req.hlsl.tintOptions.polyfill_pack_unpack_4x8 = true;

    CacheResult<d3d::CompiledShader> compiledShader;
    DAWN_TRY_LOAD_OR_RUN(compiledShader, device, std::move(req), d3d::CompiledShader::FromBlob,
                         d3d::CompileShader, "D3D11.CompileShader");

    if (device->IsToggleEnabled(Toggle::DumpShaders)) {
        d3d::DumpFXCCompiledShader(device, *compiledShader, compileFlags);
    }

    device->GetBlobCache()->EnsureStored(compiledShader);

    // Clear the hlslSource. It is only used for logging and should not be used
    // outside of the compilation.
    d3d::CompiledShader result = compiledShader.Acquire();
    result.hlslSource = std::string();

    return result;
}

}  // namespace dawn::native::d3d11
