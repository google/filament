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

#include "dawn/native/d3d12/ShaderModuleD3D12.h"

#include <string>
#include <unordered_map>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Log.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/d3d/D3DCompilationRequest.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/BackendD3D12.h"
#include "dawn/native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/PhysicalDeviceD3D12.h"
#include "dawn/native/d3d12/PipelineLayoutD3D12.h"
#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/metrics/HistogramMacros.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include "tint/tint.h"

namespace dawn::native::d3d12 {

namespace {

void DumpDXCCompiledShader(Device* device,
                           const dawn::native::d3d::CompiledShader& compiledShader,
                           uint32_t compileFlags) {
    std::ostringstream dumpedMsg;
    DAWN_ASSERT(!compiledShader.hlslSource.empty());
    dumpedMsg << "/* Dumped generated HLSL */\n" << compiledShader.hlslSource << "\n";

    const Blob& shaderBlob = compiledShader.shaderBlob;
    DAWN_ASSERT(!shaderBlob.Empty());
    dumpedMsg << "/* DXC compile flags */\n"
              << dawn::native::d3d::CompileFlagsToString(compileFlags) << "\n";
    dumpedMsg << "/* Dumped disassembled DXIL */\n";
    DxcBuffer dxcBuffer;
    dxcBuffer.Encoding = DXC_CP_UTF8;
    dxcBuffer.Ptr = shaderBlob.Data();
    dxcBuffer.Size = shaderBlob.Size();

    ComPtr<IDxcResult> dxcResult;
    device->GetDxcCompiler()->Disassemble(&dxcBuffer, IID_PPV_ARGS(&dxcResult));

    ComPtr<IDxcBlobEncoding> disassembly;
    if (dxcResult && dxcResult->HasOutput(DXC_OUT_DISASSEMBLY) &&
        SUCCEEDED(dxcResult->GetOutput(DXC_OUT_DISASSEMBLY, IID_PPV_ARGS(&disassembly), nullptr))) {
        dumpedMsg << std::string_view(static_cast<const char*>(disassembly->GetBufferPointer()),
                                      disassembly->GetBufferSize());
    } else {
        dumpedMsg << "DXC disassemble failed\n";
        ComPtr<IDxcBlobEncoding> errors;
        if (dxcResult && dxcResult->HasOutput(DXC_OUT_ERRORS) &&
            SUCCEEDED(dxcResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr))) {
            dumpedMsg << std::string_view(static_cast<const char*>(errors->GetBufferPointer()),
                                          errors->GetBufferSize());
        }
    }

    std::string logMessage = dumpedMsg.str();
    if (!logMessage.empty()) {
        device->EmitLog(WGPULoggingType_Info, logMessage.c_str());
    }
}
}  // namespace

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
    const std::optional<dawn::native::d3d::InterStageShaderVariablesMask>&
        usedInterstageVariables) {
    Device* device = ToBackend(GetDevice());
    TRACE_EVENT0(device->GetPlatform(), General, "ShaderModuleD3D12::Compile");
    DAWN_ASSERT(!IsError());

    ScopedTintICEHandler scopedICEHandler(device);
    const EntryPointMetadata& entryPoint = GetEntryPoint(programmableStage.entryPoint);
    const bool useTintIR = device->IsToggleEnabled(Toggle::UseTintIR);

    d3d::D3DCompilationRequest req = {};
    req.tracePlatform = UnsafeUnkeyedValue(device->GetPlatform());
    req.hlsl.shaderModel = ToBackend(device->GetPhysicalDevice())
                               ->GetAppliedShaderModelUnderToggles(device->GetTogglesState());
    req.hlsl.disableSymbolRenaming = device->IsToggleEnabled(Toggle::DisableSymbolRenaming);
    req.hlsl.dumpShaders = device->IsToggleEnabled(Toggle::DumpShaders);
    req.hlsl.useTintIR = useTintIR;

    req.bytecode.hasShaderF16Feature = device->HasFeature(Feature::ShaderF16);
    req.bytecode.compileFlags = compileFlags;

    if (device->IsToggleEnabled(Toggle::UseDXC)) {
        // If UseDXC toggle are not forced to be disable, DXC should have been validated to be
        // available.
        DAWN_ASSERT(ToBackend(device->GetPhysicalDevice())->GetBackend()->IsDXCAvailable());
        // We can get the DXC version information since IsDXCAvailable() is true.
        d3d12::DxcVersionInfo dxcVersionInfo =
            ToBackend(device->GetPhysicalDevice())->GetBackend()->GetDxcVersion();

        req.bytecode.compiler = d3d::Compiler::DXC;
        req.bytecode.dxcLibrary = device->GetDxcLibrary().Get();
        req.bytecode.dxcCompiler = device->GetDxcCompiler().Get();
        req.bytecode.compilerVersion = dxcVersionInfo.DxcCompilerVersion;
        req.bytecode.dxcShaderProfile = device->GetDxcShaderProfiles()[stage];
    } else {
        req.bytecode.compiler = d3d::Compiler::FXC;
        req.bytecode.d3dCompile = device->GetFunctions()->d3dCompile;
        req.bytecode.compilerVersion = D3D_COMPILER_VERSION;
        switch (stage) {
            case SingleShaderStage::Vertex:
                req.bytecode.fxcShaderProfile = "vs_5_1";
                break;
            case SingleShaderStage::Fragment:
                req.bytecode.fxcShaderProfile = "ps_5_1";
                break;
            case SingleShaderStage::Compute:
                req.bytecode.fxcShaderProfile = "cs_5_1";
                break;
        }
    }

    using tint::BindingPoint;

    tint::hlsl::writer::ArrayLengthFromUniformOptions arrayLengthFromUniform;
    arrayLengthFromUniform.ubo_binding = {layout->GetDynamicStorageBufferLengthsRegisterSpace(),
                                          layout->GetDynamicStorageBufferLengthsShaderRegister()};

    tint::hlsl::writer::Bindings bindings;

    const BindingInfoArray& moduleBindingInfo = entryPoint.bindings;
    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        const BindGroupLayout* bgl = ToBackend(layout->GetBindGroupLayout(group));
        const BindingGroupInfoMap& moduleGroupBindingInfo = moduleBindingInfo[group];

        // d3d12::BindGroupLayout packs the bindings per HLSL register-space. We modify
        // the Tint AST to make the "bindings" decoration match the offset chosen by
        // d3d12::BindGroupLayout so that Tint produces HLSL with the correct registers
        // assigned to each interface variable.
        for (const auto& [binding, shaderBindingInfo] : moduleGroupBindingInfo) {
            BindingIndex bindingIndex = bgl->GetBindingIndex(binding);
            BindingPoint srcBindingPoint{static_cast<uint32_t>(group),
                                         static_cast<uint32_t>(binding)};
            BindingPoint dstBindingPoint{static_cast<uint32_t>(group),
                                         bgl->GetShaderRegister(bindingIndex)};

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
                    static_cast<uint32_t>(group),
                    bgl->GetShaderRegister(bgl->GetBindingIndex(bindingExpansion.plane0))};
                tint::hlsl::writer::binding::BindingInfo plane1{
                    static_cast<uint32_t>(group),
                    bgl->GetShaderRegister(bgl->GetBindingIndex(bindingExpansion.plane1))};
                tint::hlsl::writer::binding::BindingInfo metadata{
                    static_cast<uint32_t>(group),
                    bgl->GetShaderRegister(bgl->GetBindingIndex(bindingExpansion.params))};
                bindings.external_texture.emplace(
                    srcBindingPoint,
                    tint::hlsl::writer::binding::ExternalTexture{metadata, plane0, plane1});
            }

            if (bufferBindingInfo) {
                const auto& bindingLayout =
                    std::get<BufferBindingInfo>(bgl->GetBindingInfo(bindingIndex).bindingLayout);

                // Declaring a read-only storage buffer in HLSL but specifying a storage
                // buffer in the BGL produces the wrong output. Force read-only storage
                // buffer bindings to be treated as UAV instead of SRV. Internal storage
                // buffer is a storage buffer used in the internal pipeline.
                const bool forceStorageBufferAsUAV =
                    (bufferBindingInfo->type == wgpu::BufferBindingType::ReadOnlyStorage &&
                     (bindingLayout.type == wgpu::BufferBindingType::Storage ||
                      bindingLayout.type == kInternalStorageBufferBinding));
                if (forceStorageBufferAsUAV) {
                    bindings.access_controls.emplace(srcBindingPoint,
                                                     tint::core::Access::kReadWrite);
                }

                // On D3D12 backend all storage buffers without Dynamic Buffer Offset will always be
                // bound to root descriptor tables, where D3D12 runtime can guarantee that OOB-read
                // will always return 0 and OOB-write will always take no action, so we don't need
                // to do robustness transform on them. Note that we still need to do robustness
                // transform on uniform buffers because only sized array is allowed in uniform
                // buffers, so FXC will report compilation error when the indexing to the array in a
                // cBuffer is out of bound and can be checked at compilation time. Storage buffers
                // are OK because they are always translated with RWByteAddressBuffers, which has no
                // such sized arrays.
                //
                // For example below WGSL shader will cause compilation error when we skip
                // robustness transform on uniform buffers:
                //
                // struct TestData {
                //     data: array<vec4<u32>, 3>,
                // };
                // @group(0) @binding(0) var<uniform> s: TestData;
                //
                // fn test() -> u32 {
                //     let index = 1000000u;
                //     if (s.data[index][0] != 0u) {    // error X3504: array index out of bounds
                //         return 0x1004u;
                //     }
                //     return 0u;
                // }
                if ((bufferBindingInfo->type == wgpu::BufferBindingType::Storage ||
                     bufferBindingInfo->type == wgpu::BufferBindingType::ReadOnlyStorage) &&
                    !bindingLayout.hasDynamicOffset) {
                    bindings.ignored_by_robustness_transform.emplace_back(srcBindingPoint);
                }
            }

            // Add arrayLengthFromUniform options
            {
                for (const auto& bindingAndRegisterOffset :
                     layout->GetDynamicStorageBufferLengthInfo()[group].bindingAndRegisterOffsets) {
                    BindingNumber bindingNum = bindingAndRegisterOffset.binding;
                    uint32_t registerOffset = bindingAndRegisterOffset.registerOffset;
                    BindingPoint bindingPoint{static_cast<uint32_t>(group),
                                              static_cast<uint32_t>(bindingNum)};
                    arrayLengthFromUniform.bindpoint_to_size_index.emplace(bindingPoint,
                                                                           registerOffset);
                }
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
    if (!useTintIR) {
        req.hlsl.firstIndexOffsetRegisterSpace = layout->GetFirstIndexOffsetRegisterSpace();
        req.hlsl.firstIndexOffsetShaderRegister = layout->GetFirstIndexOffsetShaderRegister();
    }
    req.hlsl.substituteOverrideConfig = std::move(substituteOverrideConfig);

    req.hlsl.tintOptions.disable_robustness = !device->IsRobustnessEnabled();
    req.hlsl.tintOptions.disable_workgroup_init =
        device->IsToggleEnabled(Toggle::DisableWorkgroupInit);
    req.hlsl.tintOptions.bindings = std::move(bindings);

    req.hlsl.tintOptions.compiler = req.bytecode.compiler == d3d::Compiler::FXC
                                        ? tint::hlsl::writer::Options::Compiler::kFXC
                                        : tint::hlsl::writer::Options::Compiler::kDXC;

    if (entryPoint.usesNumWorkgroups) {
        DAWN_ASSERT(stage == SingleShaderStage::Compute);
        req.hlsl.tintOptions.root_constant_binding_point = tint::BindingPoint{
            layout->GetNumWorkgroupsRegisterSpace(), layout->GetNumWorkgroupsShaderRegister()};
    } else if (useTintIR && stage == SingleShaderStage::Vertex) {
        // For vertex shaders, use root constant to add FirstIndexOffset, if needed
        req.hlsl.tintOptions.root_constant_binding_point =
            tint::BindingPoint{layout->GetFirstIndexOffsetRegisterSpace(),
                               layout->GetFirstIndexOffsetShaderRegister()};
    }

    // TODO(dawn:549): HLSL generation outputs the indices into the
    // array_length_from_uniform buffer that were actually used. When the blob cache can
    // store more than compiled shaders, we should reflect these used indices and store
    // them as well. This would allow us to only upload root constants that are actually
    // read by the shader.
    req.hlsl.tintOptions.array_length_from_uniform = std::move(arrayLengthFromUniform);

    if (stage == SingleShaderStage::Vertex) {
        // Now that only vertex shader can have interstage outputs.
        // Pass in the actually used interstage locations for tint to potentially truncate unused
        // outputs.
        if (usedInterstageVariables.has_value()) {
            req.hlsl.tintOptions.interstage_locations = *usedInterstageVariables;
        }

        req.hlsl.tintOptions.truncate_interstage_variables = true;
    }

    req.hlsl.tintOptions.polyfill_reflect_vec2_f32 =
        device->IsToggleEnabled(Toggle::D3D12PolyfillReflectVec2F32);
    req.hlsl.tintOptions.polyfill_dot_4x8_packed =
        device->IsToggleEnabled(Toggle::PolyFillPacked4x8DotProduct);
    req.hlsl.tintOptions.disable_polyfill_integer_div_mod =
        device->IsToggleEnabled(Toggle::DisablePolyfillsOnIntegerDivisonAndModulo);
    req.hlsl.tintOptions.polyfill_pack_unpack_4x8 =
        device->IsToggleEnabled(Toggle::D3D12PolyFillPackUnpack4x8);

    const CombinedLimits& limits = device->GetLimits();
    req.hlsl.limits = LimitsForCompilationRequest::Create(limits.v1);
    req.hlsl.adapter = UnsafeUnkeyedValue(static_cast<const AdapterBase*>(device->GetAdapter()));

    CacheResult<d3d::CompiledShader> compiledShader;
    DAWN_TRY_LOAD_OR_RUN(compiledShader, device, std::move(req), d3d::CompiledShader::FromBlob,
                         d3d::CompileShader, "D3D12.CompileShader");

    if (device->IsToggleEnabled(Toggle::DumpShaders)) {
        if (device->IsToggleEnabled(Toggle::UseDXC)) {
            DumpDXCCompiledShader(device, *compiledShader, compileFlags);
        } else {
            d3d::DumpFXCCompiledShader(device, *compiledShader, compileFlags);
        }
    }

    device->GetBlobCache()->EnsureStored(compiledShader);

    // Clear the hlslSource. It is only used for logging and should not be used
    // outside of the compilation.
    d3d::CompiledShader result = compiledShader.Acquire();
    result.hlslSource = "";
    return std::move(result);
}

}  // namespace dawn::native::d3d12
