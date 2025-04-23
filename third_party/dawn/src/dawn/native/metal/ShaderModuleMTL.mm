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

#include "dawn/native/metal/ShaderModuleMTL.h"

#include "dawn/common/MatchVariant.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/CacheRequest.h"
#include "dawn/native/Serializable.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/metal/BindGroupLayoutMTL.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/PipelineLayoutMTL.h"
#include "dawn/native/metal/RenderPipelineMTL.h"
#include "dawn/native/metal/UtilsMetal.h"
#include "dawn/native/stream/BlobSource.h"
#include "dawn/native/stream/ByteVectorSink.h"
#include "dawn/native/utils/WGPUHelpers.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/metrics/HistogramMacros.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include <tint/tint.h>

#include <sstream>

namespace dawn::native::metal {
namespace {

// The name to use when remapping entry points.
constexpr char kRemappedEntryPointName[] = "dawn_entry_point";

using OptionalVertexPullingTransformConfig = std::optional<tint::VertexPullingConfig>;

#define MSL_COMPILATION_REQUEST_MEMBERS(X)                                                       \
    X(SingleShaderStage, stage)                                                                  \
    X(const tint::Program*, inputProgram)                                                        \
    X(std::optional<tint::ast::transform::SubstituteOverride::Config>, substituteOverrideConfig) \
    X(LimitsForCompilationRequest, limits)                                                       \
    X(CacheKey::UnsafeUnkeyedValue<LimitsForCompilationRequest>, adapterSupportedLimits)         \
    X(uint32_t, maxSubgroupSize)                                                                 \
    X(std::string, entryPointName)                                                               \
    X(bool, usesSubgroupMatrix)                                                                  \
    X(bool, disableSymbolRenaming)                                                               \
    X(tint::msl::writer::Options, tintOptions)                                                   \
    X(CacheKey::UnsafeUnkeyedValue<dawn::platform::Platform*>, platform)

DAWN_MAKE_CACHE_REQUEST(MslCompilationRequest, MSL_COMPILATION_REQUEST_MEMBERS);
#undef MSL_COMPILATION_REQUEST_MEMBERS

using WorkgroupAllocations = std::vector<uint32_t>;

#define MSL_COMPILATION_MEMBERS(X)                \
    X(std::string, msl)                           \
    X(std::string, remappedEntryPointName)        \
    X(bool, needsStorageBufferLength)             \
    X(bool, hasInvariantAttribute)                \
    X(WorkgroupAllocations, workgroupAllocations) \
    X(Extent3D, localWorkgroupSize)

DAWN_SERIALIZABLE(struct, MslCompilation, MSL_COMPILATION_MEMBERS){};
#undef MSL_COMPILATION_MEMBERS

}  // namespace
}  // namespace dawn::native::metal

namespace dawn::native::metal {

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

ShaderModule::~ShaderModule() = default;

MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult,
                                    OwnedCompilationMessages* compilationMessages) {
    return InitializeBase(parseResult, compilationMessages);
}

namespace {

tint::msl::writer::Bindings generateBindingInfo(
    SingleShaderStage stage,
    const PipelineLayout* layout,
    const BindingInfoArray& moduleBindingInfo,
    tint::msl::writer::ArrayLengthFromUniformOptions& arrayLengthFromUniform) {
    tint::msl::writer::Bindings bindings;

    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        const BindGroupLayout* bgl = ToBackend(layout->GetBindGroupLayout(group));

        for (const auto& currentModuleBindingInfo : moduleBindingInfo[group]) {
            // We cannot use structured binding here because lambda expressions can only capture
            // variables, while structured binding doesn't introduce variables.
            const auto& binding = currentModuleBindingInfo.first;
            const auto& shaderBindingInfo = currentModuleBindingInfo.second;

            tint::BindingPoint srcBindingPoint{static_cast<uint32_t>(group),
                                               static_cast<uint32_t>(binding)};

            BindingIndex bindingIndex = bgl->GetBindingIndex(binding);
            auto& bindingIndexInfo = layout->GetBindingIndexInfo(stage)[group];
            uint32_t shaderIndex = bindingIndexInfo[bindingIndex];

            tint::BindingPoint dstBindingPoint{0, shaderIndex};

            MatchVariant(
                shaderBindingInfo.bindingInfo,
                [&](const BufferBindingInfo& bindingInfo) {
                    switch (bindingInfo.type) {
                        case wgpu::BufferBindingType::Uniform:
                            bindings.uniform.emplace(
                                srcBindingPoint,
                                tint::msl::writer::binding::Uniform{dstBindingPoint.binding});
                            break;
                        case kInternalStorageBufferBinding:
                        case wgpu::BufferBindingType::Storage:
                        case wgpu::BufferBindingType::ReadOnlyStorage:
                        case kInternalReadOnlyStorageBufferBinding:
                            bindings.storage.emplace(
                                srcBindingPoint,
                                tint::msl::writer::binding::Storage{dstBindingPoint.binding});
                            // Use the ShaderIndex as the indices for the buffer size lookups in
                            // the array length uniform transform. This is used to compute the
                            // size of variable length arrays in storage buffers.
                            arrayLengthFromUniform.bindpoint_to_size_index.emplace(
                                srcBindingPoint, dstBindingPoint.binding);
                            break;
                        case wgpu::BufferBindingType::BindingNotUsed:
                        case wgpu::BufferBindingType::Undefined:
                            DAWN_UNREACHABLE();
                            break;
                    }
                },
                [&](const SamplerBindingInfo& bindingInfo) {
                    bindings.sampler.emplace(srcBindingPoint, tint::msl::writer::binding::Sampler{
                                                                  dstBindingPoint.binding});
                },
                [&](const TextureBindingInfo& bindingInfo) {
                    bindings.texture.emplace(srcBindingPoint, tint::msl::writer::binding::Texture{
                                                                  dstBindingPoint.binding});
                },
                [&](const StorageTextureBindingInfo& bindingInfo) {
                    bindings.storage_texture.emplace(
                        srcBindingPoint,
                        tint::msl::writer::binding::StorageTexture{dstBindingPoint.binding});
                },
                [&](const ExternalTextureBindingInfo& bindingInfo) {
                    const auto& etBindingMap = bgl->GetExternalTextureBindingExpansionMap();
                    const auto& expansion = etBindingMap.find(binding);
                    DAWN_ASSERT(expansion != etBindingMap.end());

                    const auto& bindingExpansion = expansion->second;
                    tint::msl::writer::binding::BindingInfo plane0{
                        bindingIndexInfo[bgl->GetBindingIndex(bindingExpansion.plane0)]};
                    tint::msl::writer::binding::BindingInfo plane1{
                        bindingIndexInfo[bgl->GetBindingIndex(bindingExpansion.plane1)]};
                    tint::msl::writer::binding::BindingInfo metadata{
                        bindingIndexInfo[bgl->GetBindingIndex(bindingExpansion.params)]};

                    bindings.external_texture.emplace(
                        srcBindingPoint,
                        tint::msl::writer::binding::ExternalTexture{metadata, plane0, plane1});
                },
                [](const InputAttachmentBindingInfo&) { DAWN_UNREACHABLE(); });
        }
    }
    return bindings;
}

ResultOrError<CacheResult<MslCompilation>> TranslateToMSL(
    DeviceBase* device,
    const ProgrammableStage& programmableStage,
    SingleShaderStage stage,
    const PipelineLayout* layout,
    ShaderModule::MetalFunctionData* out,
    uint32_t sampleMask,
    const RenderPipeline* renderPipeline,
    const BindingInfoArray& moduleBindingInfo) {
    std::ostringstream errorStream;
    errorStream << "Tint MSL failure:\n";

    tint::msl::writer::ArrayLengthFromUniformOptions arrayLengthFromUniform;
    arrayLengthFromUniform.ubo_binding = kBufferLengthBufferSlot;

    tint::msl::writer::Bindings bindings =
        generateBindingInfo(stage, layout, moduleBindingInfo, arrayLengthFromUniform);

    OptionalVertexPullingTransformConfig vertexPullingTransformConfig;
    if (stage == SingleShaderStage::Vertex &&
        device->IsToggleEnabled(Toggle::MetalEnableVertexPulling)) {
        vertexPullingTransformConfig =
            BuildVertexPullingTransformConfig(*renderPipeline, kPullingBufferBindingSet);

        for (VertexBufferSlot slot : IterateBitSet(renderPipeline->GetVertexBuffersUsed())) {
            uint32_t metalIndex = renderPipeline->GetMtlVertexBufferIndex(slot);

            // Tell Tint to map (kPullingBufferBindingSet, slot) to this MSL buffer index.
            tint::BindingPoint srcBindingPoint{static_cast<uint32_t>(kPullingBufferBindingSet),
                                               static_cast<uint8_t>(slot)};
            tint::BindingPoint dstBindingPoint{0, metalIndex};
            if (srcBindingPoint != dstBindingPoint) {
                bindings.storage.emplace(
                    srcBindingPoint, tint::msl::writer::binding::Storage{dstBindingPoint.binding});
            }

            // Use the ShaderIndex as the indices for the buffer size lookups in the array
            // length uniform transform.
            arrayLengthFromUniform.bindpoint_to_size_index.emplace(srcBindingPoint,
                                                                   dstBindingPoint.binding);
        }
    }

    std::optional<tint::ast::transform::SubstituteOverride::Config> substituteOverrideConfig;
    if (!programmableStage.metadata->overrides.empty()) {
        substituteOverrideConfig = BuildSubstituteOverridesTransformConfig(programmableStage);
    }

    std::unordered_map<uint32_t, uint32_t> pixelLocalAttachments;
    if (stage == SingleShaderStage::Fragment && layout->HasPixelLocalStorage()) {
        const AttachmentState* attachmentState = renderPipeline->GetAttachmentState();
        const std::vector<wgpu::TextureFormat>& storageAttachmentSlots =
            attachmentState->GetStorageAttachmentSlots();
        std::vector<ColorAttachmentIndex> storageAttachmentPacking =
            attachmentState->ComputeStorageAttachmentPackingInColorAttachments();

        for (size_t i = 0; i < storageAttachmentSlots.size(); i++) {
            pixelLocalAttachments[i] = uint8_t(storageAttachmentPacking[i]);
        }
    }

    MslCompilationRequest req = {};
    req.stage = stage;
    auto tintProgram = programmableStage.module->GetTintProgram();
    req.inputProgram = &(tintProgram->program);
    req.substituteOverrideConfig = std::move(substituteOverrideConfig);
    req.entryPointName = programmableStage.entryPoint.c_str();
    req.disableSymbolRenaming = device->IsToggleEnabled(Toggle::DisableSymbolRenaming);
    req.usesSubgroupMatrix = programmableStage.metadata->usesSubgroupMatrix;
    req.platform = UnsafeUnkeyedValue(device->GetPlatform());

    req.tintOptions.strip_all_names = !req.disableSymbolRenaming;
    req.tintOptions.remapped_entry_point_name = kRemappedEntryPointName;
    req.tintOptions.disable_robustness = !device->IsRobustnessEnabled();
    req.tintOptions.buffer_size_ubo_index = kBufferLengthBufferSlot;
    req.tintOptions.fixed_sample_mask = sampleMask;
    req.tintOptions.disable_workgroup_init = false;
    req.tintOptions.disable_demote_to_helper =
        device->IsToggleEnabled(Toggle::DisableDemoteToHelper);
    req.tintOptions.emit_vertex_point_size =
        stage == SingleShaderStage::Vertex &&
        renderPipeline->GetPrimitiveTopology() == wgpu::PrimitiveTopology::PointList;
    req.tintOptions.array_length_from_uniform = std::move(arrayLengthFromUniform);
    req.tintOptions.pixel_local_attachments = std::move(pixelLocalAttachments);
    req.tintOptions.bindings = std::move(bindings);
    req.tintOptions.disable_polyfill_integer_div_mod =
        device->IsToggleEnabled(Toggle::DisablePolyfillsOnIntegerDivisonAndModulo);
    req.tintOptions.vertex_pulling_config = std::move(vertexPullingTransformConfig);

    req.limits = LimitsForCompilationRequest::Create(device->GetLimits().v1);
    req.adapterSupportedLimits =
        LimitsForCompilationRequest::Create(device->GetAdapter()->GetLimits().v1);
    req.maxSubgroupSize = device->GetAdapter()->GetPhysicalDevice()->GetSubgroupMaxSize();

    CacheResult<MslCompilation> mslCompilation;
    {
        ScopedTintICEHandler scopedICEHandler(device);
        DAWN_TRY_LOAD_OR_RUN(
            mslCompilation, device, std::move(req), MslCompilation::FromBlob,
            [](MslCompilationRequest r) -> ResultOrError<MslCompilation> {
                TRACE_EVENT0(r.platform.UnsafeGetValue(), General, "tint::msl::writer::Generate");
                // Convert the AST program to an IR module.
                auto ir = tint::wgsl::reader::ProgramToLoweredIR(*r.inputProgram);
                DAWN_INVALID_IF(ir != tint::Success,
                                "An error occurred while generating Tint IR\n%s",
                                ir.Failure().reason);

                auto singleEntryPointResult =
                    tint::core::ir::transform::SingleEntryPoint(ir.Get(), r.entryPointName);
                DAWN_INVALID_IF(singleEntryPointResult != tint::Success,
                                "Pipeline single entry point (IR) failed:\n%s",
                                singleEntryPointResult.Failure().reason);

                if (r.substituteOverrideConfig) {
                    // this needs to run after SingleEntryPoint transform which removes unused
                    // overrides for the current entry point.
                    tint::core::ir::transform::SubstituteOverridesConfig cfg;
                    cfg.map = r.substituteOverrideConfig->map;
                    auto substituteOverridesResult =
                        tint::core::ir::transform::SubstituteOverrides(ir.Get(), cfg);
                    DAWN_INVALID_IF(substituteOverridesResult != tint::Success,
                                    "Pipeline override substitution (IR) failed:\n%s",
                                    substituteOverridesResult.Failure().reason);
                }

                // Generate MSL.
                auto result = tint::msl::writer::Generate(ir.Get(), r.tintOptions);
                DAWN_INVALID_IF(result != tint::Success,
                                "An error occurred while generating MSL:\n%s",
                                result.Failure().reason);

                // Workgroup validation has to come after `Generate` because it may require
                // overrides to have been substituted.
                Extent3D localSize{0, 0, 0};
                if (r.stage == SingleShaderStage::Compute) {
                    // Validate workgroup size and workgroup storage size.
                    DAWN_TRY_ASSIGN(
                        localSize,
                        ValidateComputeStageWorkgroupSize(
                            result->workgroup_info.x, result->workgroup_info.y,
                            result->workgroup_info.z, result->workgroup_info.storage_size,
                            r.usesSubgroupMatrix, r.maxSubgroupSize, r.limits,
                            r.adapterSupportedLimits.UnsafeGetValue()));
                }

                // Metal uses Clang to compile the shader as C++14. Disable everything in the -Wall
                // category. -Wunused-variable in particular comes up a lot in generated code, and
                // some (old?) Metal drivers accidentally treat it as a MTLLibraryErrorCompileError
                // instead of a warning.
                auto msl = std::move(result->msl);
                msl = R"(
                    #ifdef __clang__
                    #pragma clang diagnostic ignored "-Wall"
                    #endif
                )" + msl;

                auto workgroupAllocations =
                    std::move(result->workgroup_info.allocations.at(kRemappedEntryPointName));
                return MslCompilation{{
                    std::move(msl),
                    std::move(kRemappedEntryPointName),
                    result->needs_storage_buffer_sizes,
                    result->has_invariant_attribute,
                    std::move(workgroupAllocations),
                    localSize,
                }};
            },
            "Metal.CompileShaderToMSL");
    }

    if (device->IsToggleEnabled(Toggle::DumpShaders)) {
        std::ostringstream dumpedMsg;
        dumpedMsg << "/* Dumped generated MSL */\n" << mslCompilation->msl;
        device->EmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
    }

    return mslCompilation;
}

}  // namespace

MaybeError ShaderModule::CreateFunction(SingleShaderStage stage,
                                        const ProgrammableStage& programmableStage,
                                        const PipelineLayout* layout,
                                        ShaderModule::MetalFunctionData* out,
                                        uint32_t sampleMask,
                                        const RenderPipeline* renderPipeline) {
    TRACE_EVENT1(GetDevice()->GetPlatform(), General, "metal::ShaderModule::CreateFunction",
                 "label", utils::GetLabelForTrace(GetLabel()));

    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(out);

    const char* entryPointName = programmableStage.entryPoint.c_str();

    // Vertex stages must specify a renderPipeline
    if (stage == SingleShaderStage::Vertex) {
        DAWN_ASSERT(renderPipeline != nullptr);
    }

    CacheResult<MslCompilation> mslCompilation;
    DAWN_TRY_ASSIGN(mslCompilation,
                    TranslateToMSL(GetDevice(), programmableStage, stage, layout, out, sampleMask,
                                   renderPipeline, GetEntryPoint(entryPointName).bindings));

    out->needsStorageBufferLength = mslCompilation->needsStorageBufferLength;
    out->workgroupAllocations = std::move(mslCompilation->workgroupAllocations);
    out->localWorkgroupSize = MTLSizeMake(mslCompilation->localWorkgroupSize.width,
                                          mslCompilation->localWorkgroupSize.height,
                                          mslCompilation->localWorkgroupSize.depthOrArrayLayers);

    NSRef<NSString> mslSource =
        AcquireNSRef([[NSString alloc] initWithUTF8String:mslCompilation->msl.c_str()]);

    NSRef<MTLCompileOptions> compileOptions = AcquireNSRef([[MTLCompileOptions alloc] init]);
    if (mslCompilation->hasInvariantAttribute) {
        (*compileOptions).preserveInvariance = true;
    }

    (*compileOptions).fastMathEnabled = !GetStrictMath().value_or(false);

    auto mtlDevice = ToBackend(GetDevice())->GetMTLDevice();
    NSError* error = nullptr;

    NSPRef<id<MTLLibrary>> library;
    platform::metrics::DawnHistogramTimer timer(GetDevice()->GetPlatform());
    {
        TRACE_EVENT0(GetDevice()->GetPlatform(), General, "MTLDevice::newLibraryWithSource");
        library = AcquireNSPRef([mtlDevice newLibraryWithSource:mslSource.Get()
                                                        options:compileOptions.Get()
                                                          error:&error]);
    }

    if (error != nullptr) {
        DAWN_INVALID_IF(error.code != MTLLibraryErrorCompileWarning,
                        "Unable to create library object: %s.",
                        [error.localizedDescription UTF8String]);
    }
    DAWN_ASSERT(library != nil);
    timer.RecordMicroseconds("Metal.newLibraryWithSource.CacheMiss");

    NSRef<NSString> name = AcquireNSRef(
        [[NSString alloc] initWithUTF8String:mslCompilation->remappedEntryPointName.c_str()]);

    {
        TRACE_EVENT0(GetDevice()->GetPlatform(), General, "MTLLibrary::newFunctionWithName");
        out->function = AcquireNSPRef([*library newFunctionWithName:name.Get()]);
        // TODO(372181030): Remove this unnecessary check when we understand why the MTLFunction
        // might be nil here.
        if (out->function == nil) {
            std::string availableFunctions;
            for (NSString* fn : [*library functionNames]) {
                availableFunctions += "\n - \"";
                availableFunctions += [fn UTF8String];
            }

            return DAWN_FORMAT_INTERNAL_ERROR(
                "ShaderModuleMTL: failed to get the MTLFunction \'%s\' from produced MSL "
                "shader below:\n\n%s\n\nAvailable functions are:%s",
                mslCompilation->remappedEntryPointName, mslCompilation->msl, availableFunctions);
        }
    }

    std::ostringstream labelStream;
    labelStream << GetLabel() << "::" << entryPointName;
    SetDebugName(GetDevice(), out->function.Get(), "Dawn_ShaderModule", labelStream.str());
    GetDevice()->GetBlobCache()->EnsureStored(mslCompilation);

    if (GetDevice()->IsToggleEnabled(Toggle::MetalEnableVertexPulling) &&
        GetEntryPoint(entryPointName).usedVertexInputs.any()) {
        out->needsStorageBufferLength = true;
    }

    return {};
}
}  // namespace dawn::native::metal
