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
#include "dawn/common/MatchVariant.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/ResourceTableDefaultResources.h"
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
        device->EmitLog(wgpu::LoggingType::Info, logMessage.c_str());
    }
}
}  // namespace

// static
ResultOrError<Ref<ShaderModule>> ShaderModule::Create(
    Device* device,
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions) {
    Ref<ShaderModule> shader = AcquireRef(new ShaderModule(device, descriptor, internalExtensions));
    shader->Initialize();
    return shader;
}

ShaderModule::ShaderModule(Device* device,
                           const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                           std::vector<tint::wgsl::Extension> internalExtensions)
    : ShaderModuleBase(device, descriptor, std::move(internalExtensions)) {}

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

    const EntryPointMetadata& entryPoint = GetEntryPoint(programmableStage.entryPoint);

    d3d::D3DCompilationRequest req = {};
    req.tracePlatform = UnsafeUnserializedValue(device->GetPlatform());
    req.hlsl.shaderModel = ToBackend(device->GetPhysicalDevice())
                               ->GetAppliedShaderModelUnderToggles(device->GetTogglesState());
    req.hlsl.disableSymbolRenaming = device->IsToggleEnabled(Toggle::DisableSymbolRenaming);
    req.hlsl.dumpShaders = device->IsToggleEnabled(Toggle::DumpShaders);
    req.hlsl.dumpShadersOnFailure = device->IsToggleEnabled(Toggle::DumpShadersOnFailure);
    req.hlsl.tintOptions.entry_point_name = programmableStage.entryPoint;
    req.hlsl.tintOptions.remapped_entry_point_name = device->GetIsolatedEntryPointName();

    req.bytecode.hasShaderF16Feature = device->HasFeature(Feature::ShaderF16);
    req.bytecode.compileFlags = compileFlags;

    if (device->IsToggleEnabled(Toggle::UseDXC)) {
        req.bytecode.compiler = d3d::Compiler::DXC;
        req.bytecode.dxcLibrary = UnsafeUnserializedValue(device->GetDxcLibrary().Get());
        req.bytecode.dxcCompiler = UnsafeUnserializedValue(device->GetDxcCompiler().Get());
        req.bytecode.dxcShaderProfile = device->GetDxcShaderProfiles()[stage];
    } else {
        req.bytecode.compiler = d3d::Compiler::FXC;
        req.bytecode.d3dCompile =
            UnsafeUnserializedValue(pD3DCompile{device->GetFunctions()->d3dCompile});
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

    tint::hlsl::writer::ArrayLengthFromUniformOptions arrayLengthFromUniform;
    arrayLengthFromUniform.ubo_binding = {layout->GetDynamicStorageBufferLengthsRegisterSpace(),
                                          layout->GetDynamicStorageBufferLengthsShaderRegister()};

    tint::hlsl::writer::ArrayOffsetFromUniformOptions arrayOffsetFromUniform;
    arrayOffsetFromUniform.ubo_binding = {layout->GetDynamicStorageBufferOffsetsRegisterSpace(),
                                          layout->GetDynamicStorageBufferOffsetsShaderRegister()};

    std::optional<tint::ResourceTableConfig> resourceTableConfig = std::nullopt;
    if (layout->UsesResourceTable()) {
        auto bindingTypeOrder = ResourceTableDefaultResources::GetOrder();
        uint32_t baseGroup = layout->GetBaseResourceTableRegisterSpace();
        resourceTableConfig = tint::ResourceTableConfig{
            // For HLSL, Tint emits multiple unbounded arrays per type, each in its own group
            // (stage)
            // starting at baseGroup + 1, and monotonically increasing.
            // Note that all tables and metadata buffer are in binding (register) 0, since
            // they are in the same descriptor table.
            .resource_table_binding = tint::BindingPoint(baseGroup + 1, 0),
            .storage_buffer_binding = tint::BindingPoint(baseGroup, 0),
            .default_binding_type_order = {bindingTypeOrder.begin(), bindingTypeOrder.end()},
        };
    }

    tint::Bindings bindings =
        GenerateBindingRemapping(layout, stage, [&](BindGroupIndex group, BindingIndex index) {
            const BindGroupLayout* bgl = ToBackend(layout->GetBindGroupLayout(group));
            return tint::BindingPoint{
                .group = uint32_t(group),
                .binding = bgl->GetShaderRegister(index),
            };
        });

    std::vector<tint::BindingPoint> ignored_by_robustness;
    for (BindGroupIndex group : layout->GetBindGroupLayoutsMask()) {
        const BindGroupLayout* bgl = ToBackend(layout->GetBindGroupLayout(group));

        // On D3D12 backend all storage buffers, including dynamic ones, are bound to root
        // descriptor tables. D3D12 runtime guarantees that OOB-read returns 0 and
        // OOB-write is a no-op, so we should be able to disable robustness on these bindings.
        // However, for dynamic storage buffers, we bind the buffer from the base offset to the
        // end of the buffer, allowing dynamic indexing at shader-time, so we must enable
        // robustness to ensure we don't perform reads/writes outside the base offset + dynamic
        // offset + binding.size.
        //
        // Note that we need to enable robustness on uniform buffers despite the fact that only
        // fixed-size arrays are allowed for them. This is because FXC will fail compilation when
        // detecting an OOB access on a fixed-size array in a cBuffer; but for WebGPU, OOB access is
        // well defined so we want it to compile and run correctly. Storage buffers don't have this
        // issue because they are always compiled to RWByteAddressBuffers which has no fixed-size
        // array.
        //
        // For example below WGSL shader will cause compilation error when we skip robustness
        // transform on uniform buffers:
        //
        // struct TestData {
        //     data: array<vec4<u32>, 3>,
        // };
        // @group(0) @binding(0) var<uniform> s: TestData;
        //
        // fn test() -> u32 {
        //     let index = 1000000u;
        //     if (s.data[index][0] != 0u) { // error X3504: array index out of bounds
        //         return 0x1004u;
        //     }
        //     return 0u;
        // }
        for (BindingIndex index : bgl->GetBufferIndices()) {
            const auto& bindingInfo = bgl->GetBindingInfo(index);
            const auto& bufferInfo = std::get<BufferBindingInfo>(bindingInfo.bindingLayout);
            if ((bufferInfo.type == wgpu::BufferBindingType::Storage ||
                 bufferInfo.type == wgpu::BufferBindingType::ReadOnlyStorage) &&
                !bufferInfo.hasDynamicOffset) {
                ignored_by_robustness.emplace_back(tint::BindingPoint{
                    .group = uint32_t(group), .binding = uint32_t(bindingInfo.binding)});
            }
        }

        // Add per-group arrayLengthFromUniform and arrayOffsetFromUniform options
        for (const auto& bindingAndRegisterOffset :
             layout->GetDynamicStorageBufferInfo()[group].bindingAndRegisterOffsets) {
            // The bindpoint to index mapping is the same for both lengths and offsets,
            // the difference is the uniform buffer object binding
            // (arrayLengthFromUniform.ubo_binding and arrayOffsetFromUniform.ubo_binding).
            BindingNumber bindingNum = bindingAndRegisterOffset.binding;

            // Skip bindings not present for the stage because GenerateBindingRemapping doesn't
            // provide a remapping for them, which could lead to collisions between used mappings
            // and unused mappings.
            APIBindingIndex apiBindingIndex = bgl->GetAPIBindingIndex(bindingNum);
            if (!(bgl->GetAPIBindingInfo(apiBindingIndex).visibility & StageBit(stage))) {
                continue;
            }

            uint32_t registerOffset = bindingAndRegisterOffset.registerOffset;
            tint::BindingPoint bindingPoint{static_cast<uint32_t>(group),
                                            static_cast<uint32_t>(bindingNum)};
            arrayLengthFromUniform.bindpoint_to_size_index.emplace(bindingPoint, registerOffset);
            arrayOffsetFromUniform.bindpoint_to_offset_index.emplace(bindingPoint, registerOffset);
        }
    }

    req.hlsl.shaderModuleHash = GetHash();
    req.hlsl.inputProgram = UnsafeUnserializedValue(UseTintProgram());
    req.hlsl.stage = stage;
    req.hlsl.tintOptions.substitute_overrides_config = {
        .map = BuildSubstituteOverridesTransformConfig(programmableStage),
    };
    req.hlsl.tintOptions.disable_robustness = !device->IsRobustnessEnabled();
    req.hlsl.tintOptions.disable_workgroup_init =
        device->IsToggleEnabled(Toggle::DisableWorkgroupInit);
    req.hlsl.tintOptions.disable_polyfill_integer_div_mod =
        device->IsToggleEnabled(Toggle::DisablePolyfillsOnIntegerDivisonAndModulo);
    req.hlsl.tintOptions.disable_integer_range_analysis =
        !device->IsToggleEnabled(Toggle::EnableIntegerRangeAnalysisInRobustness);

    req.hlsl.tintOptions.bindings = std::move(bindings);
    req.hlsl.tintOptions.resource_table = std::move(resourceTableConfig);
    req.hlsl.tintOptions.ignored_by_robustness_transform = std::move(ignored_by_robustness);

    req.hlsl.tintOptions.compiler = req.bytecode.compiler == d3d::Compiler::FXC
                                        ? tint::hlsl::writer::Options::Compiler::kFXC
                                        : tint::hlsl::writer::Options::Compiler::kDXC;

    if (entryPoint.usesNumWorkgroups) {
        DAWN_ASSERT(stage == SingleShaderStage::Compute);
        req.hlsl.tintOptions.root_constant_binding_point = tint::BindingPoint{
            layout->GetNumWorkgroupsRegisterSpace(), layout->GetNumWorkgroupsShaderRegister()};
    } else if (stage == SingleShaderStage::Vertex) {
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
    req.hlsl.tintOptions.array_offset_from_uniform = std::move(arrayOffsetFromUniform);

    req.hlsl.tintOptions.immediate_binding_point = tint::BindingPoint{
        layout->GetImmediatesRegisterSpace(), layout->GetImmediatesShaderRegister()};

    if (stage == SingleShaderStage::Vertex) {
        // Now that only vertex shader can have interstage outputs.
        // Pass in the actually used interstage locations for tint to potentially truncate unused
        // outputs.
        if (usedInterstageVariables.has_value()) {
            req.hlsl.tintOptions.interstage_locations = *usedInterstageVariables;
        }

        req.hlsl.tintOptions.truncate_interstage_variables = true;
    }

    req.hlsl.tintOptions.workarounds.scalarize_max_min_clamp =
        device->IsToggleEnabled(Toggle::ScalarizeMaxMinClamp);
    req.hlsl.tintOptions.workarounds.polyfill_reflect_vec2_f32 =
        device->IsToggleEnabled(Toggle::D3D12PolyfillReflectVec2F32);
    req.hlsl.tintOptions.workarounds.polyfill_subgroup_broadcast_f16 =
        device->IsToggleEnabled(Toggle::EnableSubgroupsIntelGen9);

    req.hlsl.tintOptions.extensions.polyfill_dot_4x8_packed =
        device->IsToggleEnabled(Toggle::PolyFillPacked4x8DotProduct);
    req.hlsl.tintOptions.extensions.polyfill_pack_unpack_4x8 =
        device->IsToggleEnabled(Toggle::D3D12PolyFillPackUnpack4x8);

    req.hlsl.limits = LimitsForCompilationRequest::Create(device->GetLimits().v1);
    req.hlsl.adapterSupportedLimits = UnsafeUnserializedValue(
        LimitsForCompilationRequest::Create(device->GetAdapter()->GetLimits().v1));
    req.hlsl.maxSubgroupSize = device->GetAdapter()->GetPhysicalDevice()->GetSubgroupMaxSize();

    if (device->HasFeature(Feature::ChromiumExperimentalSubgroupSizeControl)) {
        req.hlsl.minExplicitComputeSubgroupSize =
            device->GetAdapter()->GetPhysicalDevice()->GetMinExplicitComputeSubgroupSize();
        req.hlsl.maxExplicitComputeSubgroupSize =
            device->GetAdapter()->GetPhysicalDevice()->GetMaxExplicitComputeSubgroupSize();
        req.hlsl.maxComputeWorkgroupSubgroups =
            device->GetAdapter()->GetPhysicalDevice()->GetMaxComputeWorkgroupSubgroups();
    }

    CacheResult<d3d::CompiledShader> compiledShader;
    DAWN_TRY_LOAD_OR_RUN(compiledShader, device, std::move(req),
                         d3d::CompiledShader::FromValidatedBlob, d3d::CompileShader,
                         "D3D12.CompileShader");

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
