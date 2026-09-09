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

#include "src/dawn/native/metal/ShaderModuleMTL.h"

#include <tint/tint.h>

#include <sstream>

#include "dawn/platform/DawnPlatform.h"
#include "src/dawn/common/MatchVariant.h"
#include "src/dawn/common/Range.h"
#include "src/dawn/native/Adapter.h"
#include "src/dawn/native/BindGroupLayout.h"
#include "src/dawn/native/CacheRequest.h"
#include "src/dawn/native/Serializable.h"
#include "src/dawn/native/TintUtils.h"
#include "src/dawn/native/metal/BindGroupLayoutMTL.h"
#include "src/dawn/native/metal/DeviceMTL.h"
#include "src/dawn/native/metal/ImmediatesLayoutMTL.h"
#include "src/dawn/native/metal/PipelineLayoutMTL.h"
#include "src/dawn/native/metal/RenderPipelineMTL.h"
#include "src/dawn/native/metal/UtilsMetal.h"
#include "src/dawn/native/stream/BlobSource.h"
#include "src/dawn/native/stream/ByteVectorSink.h"
#include "src/dawn/native/utils/WGPUHelpers.h"
#include "src/dawn/platform/metrics/HistogramMacros.h"
#include "src/dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::metal {
namespace {

using OptionalVertexPullingTransformConfig = std::optional<tint::VertexPullingConfig>;

#define MSL_COMPILATION_REQUEST_MEMBERS(X)                                           \
    X(SingleShaderStage, stage)                                                      \
    X(ShaderModuleBase::ShaderModuleHash, shaderModuleHash)                          \
    X(UnsafeUnserializedValue<ShaderModuleBase::ScopedUseTintProgram>, inputProgram) \
    X(LimitsForCompilationRequest, limits)                                           \
    X(UnsafeUnserializedValue<LimitsForCompilationRequest>, adapterSupportedLimits)  \
    X(uint32_t, minSubgroupSize)                                                     \
    X(uint32_t, maxSubgroupSize)                                                     \
    X(bool, usesSubgroupMatrix)                                                      \
    X(bool, useStrictMath)                                                           \
    X(bool, disableSymbolRenaming)                                                   \
    X(tint::msl::writer::Options, tintOptions)                                       \
    X(UnsafeUnserializedValue<dawn::platform::Platform*>, platform)

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

// clang-format off
DAWN_SERIALIZABLE(struct, MslCompilation, MSL_COMPILATION_MEMBERS) {
    static ResultOrError<MslCompilation> FromValidatedBlob(Blob blob) {
        MslCompilation result;
        DAWN_TRY_ASSIGN(result, FromBlob(std::move(blob)));
        DAWN_INVALID_IF(result.msl.empty() || result.remappedEntryPointName.empty(),
                        "Cached MslCompilation result has no MSL");
        return result;
    }
};
// clang-format on
#undef MSL_COMPILATION_MEMBERS

}  // namespace
}  // namespace dawn::native::metal

namespace dawn::native::metal {

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

ShaderModule::~ShaderModule() = default;

namespace {

tint::msl::writer::ArrayLengthOptions GenerateArrayLengthOptions(const PipelineLayout* layout,
                                                                 SingleShaderStage stage) {
    tint::msl::writer::ArrayLengthOptions arrayLength;

    // Use the ShaderIndex as the indices for the buffer size lookups in the array length uniform
    // transform. This is used to compute the size of variable length arrays in storage buffers.
    for (BindGroupIndex group : layout->GetBindGroupLayoutsMask()) {
        const BindGroupLayout* bgl = ToBackend(layout->GetBindGroupLayout(group));

        for (BindingIndex index : bgl->GetBufferIndices()) {
            const auto& bindingInfo = bgl->GetBindingInfo(index);
            if (!(bindingInfo.visibility & StageBit(stage))) {
                continue;
            }

            const auto& bufferInfo = std::get<BufferBindingInfo>(bindingInfo.bindingLayout);
            switch (bufferInfo.type) {
                case kInternalStorageBufferBinding:
                case wgpu::BufferBindingType::Storage:
                case wgpu::BufferBindingType::ReadOnlyStorage:
                case kInternalReadOnlyStorageBufferBinding:
                    arrayLength.bindpoint_to_size_index.emplace(
                        tint::BindingPoint{uint32_t{group}, uint32_t{bindingInfo.binding}},
                        layout->GetBindingIndexInfo(stage)[group][index]);
                    break;

                case wgpu::BufferBindingType::Uniform:
                    break;
                case wgpu::BufferBindingType::BindingNotUsed:
                case wgpu::BufferBindingType::Undefined:
                    DAWN_UNREACHABLE();
                    break;
            }
        }
    }
    return arrayLength;
}

std::unordered_map<uint32_t, tint::msl::writer::ArgumentBufferInfo> GenerateArgumentBufferInfo(
    SingleShaderStage stage,
    const PipelineLayout* layout,
    const BindingInfoArray& moduleBindingInfo,
    bool useArgumentBuffers) {
    if (!useArgumentBuffers) {
        return {};
    }

    std::unordered_map<uint32_t, tint::msl::writer::ArgumentBufferInfo> info = {};

    uint32_t curBufferIdx = kArgumentBufferSlotMax;
    for (BindGroupIndex group : layout->GetBindGroupLayoutsMask()) {
        const BindGroupLayout* bgl = ToBackend(layout->GetBindGroupLayout(group));

        // Note, both of these buffer index values need to match up to the value set in the
        // CommandBufferMTL #argument-buffer-and-dynamic-offsets-buffer-indices
        uint32_t argumentBufferIdx = curBufferIdx--;
        // TODO(crbug.com/363031535): The dynamic offsets should all be in a single grouping
        // which is in the immediates buffer.
        std::optional<uint32_t> dynamicOffsetsBufferIdx = std::nullopt;
        if (uint32_t(bgl->GetDynamicBufferCount()) > 0u) {
            dynamicOffsetsBufferIdx = curBufferIdx--;
        }

        tint::msl::writer::ArgumentBufferInfo argBufferInfo = {
            .id = argumentBufferIdx,
            .dynamic_buffer_id = dynamicOffsetsBufferIdx,
        };

        uint32_t curDynamicOffset = 0;

        for (BindingIndex bindingIndex : Range(bgl->GetBindingCount())) {
            const BindingInfo& bindingInfo = bgl->GetBindingInfo(bindingIndex);

            MatchVariant(
                bindingInfo.bindingLayout,  //
                [&](const BufferBindingInfo& binding) {
                    if (binding.hasDynamicOffset) {
                        argBufferInfo.binding_info_to_offset_index.insert(
                            {uint32_t{bindingIndex}, curDynamicOffset++});
                    }
                },
                [&](const SamplerBindingInfo& bindingInfo) {},
                [&](const StaticSamplerBindingInfo& bindingInfo) {},
                [&](const TextureBindingInfo& bindingInfo) {}, [](const TexelBufferBindingInfo&) {},
                [&](const StorageTextureBindingInfo& bindingInfo) {},
                [](const InputAttachmentBindingInfo&) { DAWN_UNREACHABLE(); },
                [](const ExternalTextureBindingInfo&) { DAWN_UNREACHABLE(); });
        }
        info.insert({static_cast<uint32_t>(group), argBufferInfo});
    }

    return info;
}

ResultOrError<CacheResult<MslCompilation>> TranslateToMSL(
    DeviceBase* device,
    const ProgrammableStage& programmableStage,
    SingleShaderStage stage,
    const PipelineLayout* layout,
    uint32_t sampleMask,
    const RenderPipeline* renderPipeline,
    const BindingInfoArray& moduleBindingInfo,
    bool useStrictMath,
    const ImmediateMask& pipelineImmediateMask) {
    std::ostringstream errorStream;
    errorStream << "Tint MSL failure:\n";

    bool useArgumentBuffers = device->IsToggleEnabled(Toggle::MetalUseArgumentBuffers);

    tint::Bindings bindings =
        GenerateBindingRemapping(layout, stage, [&](BindGroupIndex group, BindingIndex index) {
            if (useArgumentBuffers) {
                return tint::BindingPoint{
                    .group = uint32_t{group},
                    .binding = ToMTLArgumentBufferIndex(index),
                };
            } else {
                return tint::BindingPoint{
                    .group = 0,
                    .binding = layout->GetBindingIndexInfo(stage)[group][index],
                };
            }
        });

    tint::msl::writer::ArrayLengthOptions arrayLengthFromConstants =
        GenerateArrayLengthOptions(layout, stage);

    std::unordered_map<uint32_t, tint::msl::writer::ArgumentBufferInfo> argumentBufferInfo =
        GenerateArgumentBufferInfo(stage, layout, moduleBindingInfo, useArgumentBuffers);

    OptionalVertexPullingTransformConfig vertexPullingTransformConfig;
    if (stage == SingleShaderStage::Vertex &&
        device->IsToggleEnabled(Toggle::MetalEnableVertexPulling)) {
        vertexPullingTransformConfig =
            BuildVertexPullingTransformConfig(*renderPipeline, kPullingBufferBindingSet);

        for (VertexBufferSlot slot : renderPipeline->GetVertexBuffersUsed()) {
            uint32_t metalIndex = renderPipeline->GetMtlVertexBufferIndex(slot);

            // Tell Tint to map (kPullingBufferBindingSet, slot) to this MSL buffer index.
            tint::BindingPoint srcBindingPoint{
                .group = uint32_t{kPullingBufferBindingSet},
                .binding = uint8_t{slot},
            };
            tint::BindingPoint dstBindingPoint{
                .group = 0,
                .binding = metalIndex,
            };
            if (srcBindingPoint != dstBindingPoint) {
                bindings.storage.emplace(srcBindingPoint, dstBindingPoint);
            }

            // Use the ShaderIndex as the indices for the buffer size lookups in the array
            // length uniform transform.
            arrayLengthFromConstants.bindpoint_to_size_index.emplace(srcBindingPoint,
                                                                     dstBindingPoint.binding);
        }
    }

    if (!arrayLengthFromConstants.bindpoint_to_size_index.empty()) {
        arrayLengthFromConstants.buffer_sizes_offset =
            GetImmediateBufferSizesByteOffset(pipelineImmediateMask);
    }

    // Type should match src/tint/lang/msl/writer/common/options.h
    std::unordered_map<uint32_t, uint32_t> pixelLocalAttachments;
    if (stage == SingleShaderStage::Fragment && layout->HasPixelLocalStorage()) {
        const AttachmentState* attachmentState = renderPipeline->GetAttachmentState();
        const std::vector<wgpu::TextureFormat>& storageAttachmentSlots =
            attachmentState->GetStorageAttachmentSlots();
        std::vector<ColorAttachmentIndex> storageAttachmentPacking =
            attachmentState->ComputeStorageAttachmentPackingInColorAttachments();

        for (uint32_t i = 0; i < storageAttachmentSlots.size(); i++) {
            pixelLocalAttachments[i] = uint8_t{storageAttachmentPacking[i]};
        }
    }

    MslCompilationRequest req = {};
    req.stage = stage;
    req.shaderModuleHash = programmableStage.module->GetHash();
    req.inputProgram = UnsafeUnserializedValue(programmableStage.module->UseTintProgram());
    req.disableSymbolRenaming = device->IsToggleEnabled(Toggle::DisableSymbolRenaming);
    req.usesSubgroupMatrix = programmableStage.metadata->usesSubgroupMatrix;
    req.platform = UnsafeUnserializedValue(device->GetPlatform());
    req.useStrictMath = useStrictMath;

    req.tintOptions.substitute_overrides_config = {
        .map = BuildSubstituteOverridesTransformConfig(programmableStage),
    };
    req.tintOptions.strip_all_names = !req.disableSymbolRenaming;
    req.tintOptions.entry_point_name = programmableStage.entryPoint;
    req.tintOptions.remapped_entry_point_name = device->GetIsolatedEntryPointName();
    req.tintOptions.disable_robustness = !device->IsRobustnessEnabled();
    req.tintOptions.disable_workgroup_init = device->IsToggleEnabled(Toggle::DisableWorkgroupInit);
    req.tintOptions.disable_polyfill_integer_div_mod =
        device->IsToggleEnabled(Toggle::DisablePolyfillsOnIntegerDivisonAndModulo);
    req.tintOptions.disable_integer_range_analysis =
        !device->IsToggleEnabled(Toggle::EnableIntegerRangeAnalysisInRobustness);

    req.tintOptions.fixed_sample_mask = sampleMask;
    req.tintOptions.emit_vertex_point_size =
        stage == SingleShaderStage::Vertex &&
        renderPipeline->GetPrimitiveTopology() == wgpu::PrimitiveTopology::PointList;
    req.tintOptions.immediate_binding_point = tint::BindingPoint{0, kImmediateBlockBufferSlot};
    req.tintOptions.array_length_from_constants = std::move(arrayLengthFromConstants);
    req.tintOptions.pixel_local_attachments = std::move(pixelLocalAttachments);
    req.tintOptions.bindings = std::move(bindings);
    req.tintOptions.vertex_pulling_config = std::move(vertexPullingTransformConfig);

    // Set internal immediate offsets
    if (stage == SingleShaderStage::Fragment &&
        HasImmediates(&RenderImmediates::clampFragDepth, pipelineImmediateMask)) {
        uint32_t offsetStartBytes = GetImmediateByteOffsetInPipeline(
            &RenderImmediates::clampFragDepth, pipelineImmediateMask);
        req.tintOptions.depth_range_offsets = {offsetStartBytes,
                                               offsetStartBytes + kImmediateElementByteSize};
    }
    if (stage == SingleShaderStage::Compute) {
        req.tintOptions.non_constant_zero_offset = GetImmediateByteOffsetInPipeline(
            &ComputeImmediates::nonConstantZero, pipelineImmediateMask);
    } else {
        req.tintOptions.non_constant_zero_offset = GetImmediateByteOffsetInPipeline(
            &RenderImmediates::nonConstantZero, pipelineImmediateMask);
    }

    req.tintOptions.use_argument_buffers = useArgumentBuffers;
    req.tintOptions.group_to_argument_buffer_info = std::move(argumentBufferInfo);

    req.tintOptions.workarounds.scalarize_max_min_clamp =
        device->IsToggleEnabled(Toggle::ScalarizeMaxMinClamp);
    req.tintOptions.workarounds.disable_module_constant_f16 =
        device->IsToggleEnabled(Toggle::MetalDisableModuleConstantF16);
    req.tintOptions.workarounds.polyfill_subgroup_broadcast_f16 =
        device->IsToggleEnabled(Toggle::EnableSubgroupsIntelGen9);
    req.tintOptions.workarounds.polyfill_clamp_float =
        device->IsToggleEnabled(Toggle::MetalPolyfillClampFloat);
    req.tintOptions.workarounds.polyfill_unpack_2x16_snorm =
        device->IsToggleEnabled(Toggle::MetalPolyfillUnpack2x16snorm);
    req.tintOptions.workarounds.polyfill_unpack_2x16_unorm =
        device->IsToggleEnabled(Toggle::MetalPolyfillUnpack2x16unorm);
    req.tintOptions.workarounds.polyfill_tanh_f16 =
        device->IsToggleEnabled(Toggle::MetalPolyfillTanhF16);
    req.tintOptions.workarounds.replace_workgroup_bool_with_u32 =
        device->IsToggleEnabled(Toggle::MetalReplaceWorkgroupBoolWithU32);
    req.tintOptions.workarounds.collapse_subgroup_min_max =
        device->IsToggleEnabled(Toggle::CollapseSubgroupMinMax);
    req.tintOptions.workarounds.fix_u32_div_mod =
        device->IsToggleEnabled(Toggle::MetalFixU32DivMod);
    req.tintOptions.workarounds.polyfill_bool_vec_dynamic_store =
        device->IsToggleEnabled(Toggle::MetalPolyfillBoolVecDynamicStore);

    req.tintOptions.extensions.disable_demote_to_helper =
        device->IsToggleEnabled(Toggle::DisableDemoteToHelper);

    req.limits = LimitsForCompilationRequest::Create(device->GetLimits().v1);
    req.adapterSupportedLimits = UnsafeUnserializedValue(
        LimitsForCompilationRequest::Create(device->GetAdapter()->GetLimits().v1));
    req.minSubgroupSize = device->GetAdapter()->GetPhysicalDevice()->GetSubgroupMinSize();
    req.maxSubgroupSize = device->GetAdapter()->GetPhysicalDevice()->GetSubgroupMaxSize();

    CacheResult<MslCompilation> mslCompilation;
    DAWN_TRY_LOAD_OR_RUN(
        mslCompilation, device, std::move(req), MslCompilation::FromValidatedBlob,
        [](MslCompilationRequest r) -> ResultOrError<MslCompilation> {
            TRACE_EVENT(DAWN_TRACE_CATEGORY(), "tint::msl::writer::Generate");
            // Requires Tint Program here right before actual using.
            auto shaderModule = r.inputProgram.UnsafeGetValue();
            auto inputProgram = shaderModule->GetTintProgram();
            auto device = shaderModule->GetDevice();
            const tint::Program* tintInputProgram = &(inputProgram->program);
            // Convert the AST program to an IR module.
            tint::Result<tint::core::ir::Module> ir;
            {
                SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(r.platform.UnsafeGetValue(),
                                                   "ShaderModuleProgramToIR");
                tint::wgsl::reader::IROptions irOptions{
                    .ice_callback = device->GetTintInternalCompilerErrorCallback(),
                    .dump_ir_when_validating = device->IsToggleEnabled(Toggle::DumpTintIR),
                    .enable_validation_asserts =
                        device->IsToggleEnabled(Toggle::EnableTintIRValidationAsserts),
                };
                ir = tint::wgsl::reader::ProgramToLoweredIR(*tintInputProgram, irOptions);
                DAWN_INVALID_IF(ir != tint::Success,
                                "An error occurred while generating Tint IR\n%s",
                                ir.Failure().reason);
            }

            // Generate MSL.
            tint::Result<tint::msl::writer::Output> result;
            {
                SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(r.platform.UnsafeGetValue(),
                                                   "ShaderModuleGenerateMSL");
                result = tint::msl::writer::Generate(ir.Get(), r.tintOptions);
                DAWN_INVALID_IF(result != tint::Success,
                                "An error occurred while generating MSL:\n%s",
                                result.Failure().reason);
            }

            // Workgroup validation has to come after `Generate` because it may require
            // overrides to have been substituted.
            Extent3D localSize{0, 0, 0};
            if (r.stage == SingleShaderStage::Compute) {
                // Validate workgroup size and workgroup storage size.
                DAWN_TRY_ASSIGN(localSize,
                                ValidateComputeStageWorkgroupSize(
                                    result->workgroup_info, r.usesSubgroupMatrix, r.maxSubgroupSize,
                                    r.limits, r.adapterSupportedLimits.UnsafeGetValue()));

                if (!result->workgroup_allocations.empty()) {
                    DAWN_ASSERT(result->workgroup_allocations.size() == 1);

                    uint32_t maxComputeWorkgroupStorageSize =
                        r.limits.maxComputeWorkgroupStorageSize;
                    uint64_t size = result->workgroup_allocations.front();
                    DAWN_INTERNAL_ERROR_IF(
                        size > maxComputeWorkgroupStorageSize,
                        "The total combined workgroup storage (%u bytes) size with all workgroup "
                        "variables combined into a single structure is larger than the maximum "
                        "allowed (%u bytes).%s",
                        size, maxComputeWorkgroupStorageSize,
                        DAWN_INCREASE_LIMIT_MESSAGE(r.adapterSupportedLimits.UnsafeGetValue(),
                                                    maxComputeWorkgroupStorageSize, size));
                }

                if (result->workgroup_info.subgroup_size.has_value()) {
                    uint32_t explicitSubgroupSize = result->workgroup_info.subgroup_size.value();
                    DAWN_INVALID_IF(
                        explicitSubgroupSize < r.minSubgroupSize ||
                            explicitSubgroupSize > r.maxSubgroupSize,
                        "The subgroup_size attribute (%u) is not in the allowed range ([%u, %u]).",
                        explicitSubgroupSize, r.minSubgroupSize, r.maxSubgroupSize);
                }
            }

            auto msl = std::move(result->msl);
            // Metal supports math_mode as both compiler option and as a pragma. We add the
            // math_mode here as a string conditional on OSx version as the compiler option only
            // exists for MacOS after 15. See the Metal 4 spec for more information.
            // Note: this math_mode takes precedence over global flags provide to the compiler
            // (including the deprecated fastMathEnabled compiler option).
            std::string math_mode_heading;
            if (@available(macOS 15.0, iOS 18.0, *)) {
                math_mode_heading = "\n#pragma METAL fp math_mode(";
                math_mode_heading += r.useStrictMath ? "safe" : "relaxed";
                math_mode_heading += +")\n";
            }
            // Metal uses Clang to compile the shader as C++14. Disable everything in the -Wall
            // category. -Wunused-variable in particular comes up a lot in generated code, and
            // some (old?) Metal drivers accidentally treat it as a MTLLibraryErrorCompileError
            // instead of a warning.
            msl = R"(#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
)" + math_mode_heading +
                  msl;

            return MslCompilation{{
                std::move(msl),
                r.tintOptions.remapped_entry_point_name,
                result->needs_storage_buffer_sizes,
                result->has_invariant_attribute,
                std::move(result->workgroup_allocations),
                localSize,
            }};
        },
        "Metal.CompileShaderToMSL");

    if (device->IsToggleEnabled(Toggle::DumpShaders)) {
        std::ostringstream dumpedMsg;
        dumpedMsg << "/* Dumped generated MSL */\n" << mslCompilation->msl;
        device->EmitLog(wgpu::LoggingType::Info, dumpedMsg.str().c_str());
    }

    return mslCompilation;
}

}  // namespace

MaybeError ShaderModule::CreateFunction(SingleShaderStage stage,
                                        const ProgrammableStage& programmableStage,
                                        const PipelineLayout* layout,
                                        const ImmediateMask& pipelineImmediateMask,
                                        ShaderModule::MetalFunctionData* out,
                                        uint32_t sampleMask,
                                        const RenderPipeline* renderPipeline) {
    TRACE_EVENT(DAWN_TRACE_CATEGORY(), "metal::ShaderModule::CreateFunction", "label",
                utils::GetLabelForTrace(GetLabel()));

    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(out);

    const char* entryPointName = programmableStage.entryPoint.c_str();

    // Vertex stages must specify a renderPipeline
    if (stage == SingleShaderStage::Vertex) {
        DAWN_ASSERT(renderPipeline != nullptr);
    }

    CacheResult<MslCompilation> mslCompilation;
    DAWN_TRY_ASSIGN(mslCompilation,
                    TranslateToMSL(GetDevice(), programmableStage, stage, layout, sampleMask,
                                   renderPipeline, GetEntryPoint(entryPointName).bindings,
                                   GetStrictMath().value_or(false), pipelineImmediateMask));

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

    // TODO(433534277): Warn if the pipeline uses print but it is not available or enabled.
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 150000
    if (GetDevice()->IsToggleEnabled(Toggle::EnableShaderPrint)) {
        if (@available(macOS 15.0, iOS 18.0, *)) {
            (*compileOptions).enableLogging = true;
            (*compileOptions).languageVersion = MTLLanguageVersion3_2;
        }
    }
#endif

    // If possible we will use relaxed math as a pragma in the source rather than this fast math
    // global compiler option. See crbug.com/425650181
    if (@available(macOS 15.0, iOS 18.0, *)) {
        // This empty bock is intentional due to the mechanism to support @available.
        // (the @available must be in a plain 'if' statement)
        // Without this empty block one would get '-Wunsupported-availability-guard'
    } else {
// Silence the warning that fastMathEnabled is deprecated since we cannot remove it until the
// minimum support macOS version is macOS 15.0.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        (*compileOptions).fastMathEnabled = !GetStrictMath().value_or(false);
#pragma clang diagnostic pop
    }

    auto mtlDevice = ToBackend(GetDevice())->GetMTLDevice();
    NSError* error = nullptr;

    NSPRef<id<MTLLibrary>> library;
    platform::metrics::DawnHistogramTimer timer(GetDevice()->GetPlatform());
    {
        TRACE_EVENT(DAWN_TRACE_CATEGORY(), "MTLDevice::newLibraryWithSource");
        library = AcquireNSPRef([mtlDevice newLibraryWithSource:mslSource.Get()
                                                        options:compileOptions.Get()
                                                          error:&error]);
    }

    if (error != nullptr) {
        // clang-format formats the `mslCompilation->msl` below oddly as `mslCompilation -> msl`.
        // clang-format off
        DAWN_INVALID_IF(error.code != MTLLibraryErrorCompileWarning,
                        "ShaderModuleMTL: Unable to create library object: %s from "
                        "produced MSL shader below:\n\n%s",
                        [error.localizedDescription UTF8String], mslCompilation->msl);
        // clang-format on
    }
    DAWN_ASSERT(library != nil);
    timer.RecordMicroseconds("Metal.newLibraryWithSource.CacheMiss");

    NSRef<NSString> name = AcquireNSRef(
        [[NSString alloc] initWithUTF8String:mslCompilation->remappedEntryPointName.c_str()]);

    {
        TRACE_EVENT(DAWN_TRACE_CATEGORY(), "MTLLibrary::newFunctionWithName");
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

    // For emitting MSL in error message if render pipeline creation fails.
    out->msl = std::move(mslCompilation->msl);

    return {};
}
}  // namespace dawn::native::metal
