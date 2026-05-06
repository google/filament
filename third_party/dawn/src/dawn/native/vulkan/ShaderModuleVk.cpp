// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/vulkan/ShaderModuleVk.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/HashUtils.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/Ref.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/CacheRequest.h"
#include "dawn/native/ComputePipeline.h"
#include "dawn/native/Device.h"
#include "dawn/native/ImmediateConstantsLayout.h"
#include "dawn/native/Instance.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/ResourceTableDefaultResources.h"
#include "dawn/native/Serializable.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/utils/WGPUHelpers.h"
#include "dawn/native/vulkan/BindGroupLayoutVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/PipelineLayoutVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/native/wgpu_structs_autogen.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/metrics/HistogramMacros.h"
#include "dawn/platform/tracing/TraceEvent.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "tint/tint.h"

#ifdef DAWN_ENABLE_SPIRV_VALIDATION
#include "dawn/native/SpirvValidation.h"
#endif

namespace dawn::native::vulkan {

#define COMPILED_SPIRV_MEMBERS(X)                    \
    X(std::vector<uint32_t>, spirv)                  \
    X(std::optional<uint32_t>, explicitSubgroupSize) \
    X(Extent3D, workgroupSize)

// Represents the result and metadata for a SPIR-V compilation.
// clang-format off
DAWN_SERIALIZABLE(struct, CompiledSpirv, COMPILED_SPIRV_MEMBERS) {
    static ResultOrError<CompiledSpirv> FromValidatedBlob(Blob blob) {
        CompiledSpirv result;
        DAWN_TRY_ASSIGN(result, FromBlob(std::move(blob)));
        DAWN_INVALID_IF(result.spirv.empty(), "Cached CompiledSpirv result has no instructions");
        return result;
    }
};
// clang-format on
#undef COMPILED_SPIRV_MEMBERS

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

#if TINT_BUILD_SPV_WRITER

#define SPIRV_COMPILATION_REQUEST_MEMBERS(X)                                         \
    X(SingleShaderStage, stage)                                                      \
    X(ShaderModuleBase::ShaderModuleHash, shaderModuleHash)                          \
    X(UnsafeUnserializedValue<ShaderModuleBase::ScopedUseTintProgram>, inputProgram) \
    X(LimitsForCompilationRequest, limits)                                           \
    X(UnsafeUnserializedValue<LimitsForCompilationRequest>, adapterSupportedLimits)  \
    X(uint32_t, maxSubgroupSize)                                                     \
    X(uint32_t, minExplicitComputeSubgroupSize)                                      \
    X(uint32_t, maxExplicitComputeSubgroupSize)                                      \
    X(uint32_t, maxComputeWorkgroupSubgroups)                                        \
    X(bool, usesSubgroupMatrix)                                                      \
    X(std::vector<SubgroupMatrixConfig>, subgroupMatrixConfig)                       \
    X(tint::spirv::writer::Options, tintOptions)                                     \
    X(UnsafeUnserializedValue<dawn::platform::Platform*>, platform)

DAWN_MAKE_CACHE_REQUEST(SpirvCompilationRequest, SPIRV_COMPILATION_REQUEST_MEMBERS);
#undef SPIRV_COMPILATION_REQUEST_MEMBERS

#endif  // TINT_BUILD_SPV_WRITER

ResultOrError<ShaderModule::ModuleAndSpirv> ShaderModule::GetHandleAndSpirv(
    const CompileParameters& in) {
    TRACE_EVENT0(GetDevice()->GetPlatform(), General, "ShaderModuleVk::GetHandleAndSpirv");

#if TINT_BUILD_SPV_WRITER
    // Creation of module and spirv is deferred to this point when using tint generator

    // The first VkDescriptorSetLayout is the one for the resource table if needed and pushes the
    // bindings for all other bindgroups by 1.
    BindGroupIndex startOfBindGroups{0};
    std::optional<tint::ResourceTableConfig> resourceTableConfig = std::nullopt;
    if (in.layout->UsesResourceTable()) {
        startOfBindGroups = BindGroupIndex(1);

        auto bindingTypeOrder = ResourceTableDefaultResources::GetOrder();
        resourceTableConfig = tint::ResourceTableConfig{
            .resource_table_binding = tint::BindingPoint(0, 1),
            .storage_buffer_binding = tint::BindingPoint(0, 0),
            .default_binding_type_order = {bindingTypeOrder.begin(), bindingTypeOrder.end()},
        };
    }

    auto ToWGSLBindPoint = [](BindGroupIndex group, BindingNumber binding) -> tint::BindingPoint {
        return {
            .group = uint32_t{group},
            .binding = uint32_t{binding},
        };
    };
    auto ToSPIRVBindPoint = [&](BindGroupIndex group, BindingIndex index) -> tint::BindingPoint {
        return {
            .group = uint32_t{startOfBindGroups + group},
            .binding = uint32_t{index},
        };
    };
    tint::Bindings bindings =
        GenerateBindingRemapping(in.layout, in.stage->metadata->stage, ToSPIRVBindPoint);

    // Tint checks that bindings don't overlap and uses this map to allow-list some mappings.
    std::unordered_set<tint::BindingPoint> staticallyPairedTextureBindingPoints;

    // Remap YCbCr static sampler and texture pairs by remapping the texture to use the sampler's
    // binding point.
    for (BindGroupIndex group : in.layout->GetBindGroupLayoutsMask()) {
        const BindGroupLayout* bgl = ToBackend(in.layout->GetBindGroupLayout(group));

        // Post process the binding remapping to make statically paired texture point at the sampler
        // binding point instead.
        const auto& textureToStaticSampler = bgl->GetTextureToStaticSamplerMap();
        for (BindingIndex index : bgl->GetSampledTextureIndices()) {
            const auto& bindingInfo = bgl->GetBindingInfo(index);

            if (auto it = textureToStaticSampler.find(index); it != textureToStaticSampler.end()) {
                auto wgslBindingPoint = ToWGSLBindPoint(group, bindingInfo.binding);
                bindings.texture[wgslBindingPoint].binding = uint32_t{it->second};
                staticallyPairedTextureBindingPoints.insert(wgslBindingPoint);
            }
        }
    }

    // External textures also need special cases as they may be in one of three configurations:
    //  1. Not using a static sampler, nothing to do.
    //  2. Using the multiplanar path with a static sampler: the texture has been remapped to use
    //     its static sampler binding above, but we also need to update the
    //     ExternalMultiplanarTexture's information.
    //  3. Using the YCbCr path, in which case we need to replace the preexisting multiplanar
    //     ExternalTexture binding with the YCbCr one.
    for (BindGroupIndex group : in.layout->GetBindGroupLayoutsMask()) {
        const BindGroupLayout* bgl = ToBackend(in.layout->GetBindGroupLayout(group));

        for (APIBindingIndex index : bgl->GetExternalTextureIndices()) {
            const auto& bindingInfo = bgl->GetAPIBindingInfo(index);
            tint::BindingPoint etWGSLBindPoint = ToWGSLBindPoint(group, bindingInfo.binding);

            // Only modify external textures present in the remapping already.
            if (!bindings.external_texture.contains(etWGSLBindPoint)) {
                continue;
            }

            auto& etRemapping = bindings.external_texture[etWGSLBindPoint];
            const auto& etInfo = std::get<ExternalTextureBindingInfo>(bindingInfo.bindingLayout);

            if (in.ycbcrExternalTextures->contains({group, index})) {
                // Case 3. Replace with the YCbCr external texture binding.
                etRemapping = tint::ExternalYCBCRTexture{
                    .metadata = ToSPIRVBindPoint(group, etInfo.metadata),
                    .texture = ToSPIRVBindPoint(group, etInfo.staticSampler.value()),
                    .sampler = ToSPIRVBindPoint(group, etInfo.staticSampler.value())};
            } else if (etInfo.staticSampler.has_value()) {
                // Case 2. Update plane0 to use the static sampler's binding.
                std::get<tint::ExternalMultiplanarTexture>(etRemapping).plane0 =
                    ToSPIRVBindPoint(group, etInfo.staticSampler.value());
            } else {
                // Case 1. Nothing to do.
                DAWN_ASSERT(!GetDevice()->NeedsStaticSamplerForExternalTexture());
            }
        }
    }

    const bool hasInputAttachment = !bindings.input_attachment.empty();

    SpirvCompilationRequest req = {};
    req.stage = in.stage->metadata->stage;
    req.shaderModuleHash = GetHash();
    req.inputProgram = UnsafeUnserializedValue(UseTintProgram());
    req.platform = UnsafeUnserializedValue(GetDevice()->GetPlatform());
    req.usesSubgroupMatrix = in.stage->metadata->usesSubgroupMatrix;

    // TODO(464008240): Cleanup the exposing of `EnumerateSubgroupMatrixConfigs` when possible.
    req.subgroupMatrixConfig =
        ToBackend(GetDevice()->GetPhysicalDevice())
            ->EnumerateSubgroupMatrixConfigs(GetDevice()->GetAdapter()->GetTogglesState());

    req.tintOptions.entry_point_name = in.stage->entryPoint;
    req.tintOptions.remapped_entry_point_name = GetDevice()->GetIsolatedEntryPointName();
    req.tintOptions.strip_all_names = !GetDevice()->IsToggleEnabled(Toggle::DisableSymbolRenaming);

    req.tintOptions.statically_paired_texture_binding_points =
        std::move(staticallyPairedTextureBindingPoints);
    req.tintOptions.substitute_overrides_config = {
        .map = BuildSubstituteOverridesTransformConfig(*in.stage),
    };
    req.tintOptions.bindings = std::move(bindings);
    req.tintOptions.resource_table = std::move(resourceTableConfig);

    req.tintOptions.disable_robustness = !GetDevice()->IsRobustnessEnabled();
    req.tintOptions.disable_workgroup_init =
        GetDevice()->IsToggleEnabled(Toggle::DisableWorkgroupInit);

    req.tintOptions.workarounds.polyfill_unary_f32_negation =
        GetDevice()->IsToggleEnabled(Toggle::VulkanPolyfillF32Negation);

    // These polyfills all relate to incorrect backend optimization of fabs.
    // See: crbug.com/93692702
    if (GetDevice()->IsToggleEnabled(Toggle::VulkanPolyfillF32Abs)) {
        req.tintOptions.workarounds.polyfill_f32_abs = true;
        req.tintOptions.workarounds.polyfill_length_scalar_f32 = true;
        req.tintOptions.workarounds.polyfill_distance_scalar_f32 = true;
    }

    req.tintOptions.disable_polyfill_integer_div_mod =
        GetDevice()->IsToggleEnabled(Toggle::DisablePolyfillsOnIntegerDivisonAndModulo);

    req.tintOptions.emit_vertex_point_size = in.emitPointSize;
    req.tintOptions.polyfill_pixel_center = in.polyfillPixelCenter;
    req.tintOptions.multisampled_framebuffer_fetch = in.needsMultisampledFramebufferFetch;

    req.tintOptions.spirv_version = GetDevice()->IsToggleEnabled(Toggle::UseSpirv14)
                                        ? tint::spirv::writer::SpvVersion::kSpv14
                                        : tint::spirv::writer::SpvVersion::kSpv13;
    req.tintOptions.disable_integer_range_analysis =
        !GetDevice()->IsToggleEnabled(Toggle::EnableIntegerRangeAnalysisInRobustness);

    req.tintOptions.extensions.use_vulkan_memory_model =
        GetDevice()->IsToggleEnabled(Toggle::UseVulkanMemoryModel);
    // Currently we can disable index clamping on all runtime-sized arrays in Tint robustness
    // transform as unsized arrays can only be declared on storage address space.
    req.tintOptions.extensions.disable_runtime_sized_array_index_clamping =
        GetDevice()->IsToggleEnabled(Toggle::VulkanUseBufferRobustAccess2);
    req.tintOptions.extensions.disable_image_robustness =
        GetDevice()->IsToggleEnabled(Toggle::VulkanUseImageRobustAccess2);
    // The only possible alternative for the vulkan demote to helper extension is
    // "OpTerminateInvocation" which remains unimplemented in dawn/tint.
    req.tintOptions.extensions.use_demote_to_helper_invocation =
        GetDevice()->IsToggleEnabled(Toggle::VulkanUseDemoteToHelperInvocationExtension);
    req.tintOptions.extensions.use_storage_input_output_16 =
        GetDevice()->IsToggleEnabled(Toggle::VulkanUseStorageInputOutput16);
    req.tintOptions.extensions.dot_4x8_packed =
        GetDevice()->IsToggleEnabled(Toggle::PolyFillPacked4x8DotProduct);
    req.tintOptions.extensions.use_zero_initialize_workgroup_memory =
        GetDevice()->IsToggleEnabled(Toggle::VulkanUseZeroInitializeWorkgroupMemoryExtension);
    req.tintOptions.extensions.use_uniform_buffers =
        !GetDevice()->IsToggleEnabled(Toggle::DecomposeUniformBuffers);

    req.tintOptions.workarounds.subgroup_shuffle_clamped =
        GetDevice()->IsToggleEnabled(Toggle::SubgroupShuffleClamped);
    req.tintOptions.workarounds.texture_sample_compare_depth_cube_array =
        GetDevice()->IsToggleEnabled(Toggle::VulkanSampleCompareDepthCubeArrayWorkaround);
    req.tintOptions.workarounds.polyfill_pack_unpack_4x8_norm =
        GetDevice()->IsToggleEnabled(Toggle::PolyfillPackUnpack4x8Norm);
    req.tintOptions.workarounds.polyfill_case_switch =
        GetDevice()->IsToggleEnabled(Toggle::VulkanPolyfillSwitchWithIf);
    req.tintOptions.workarounds.scalarize_max_min_clamp =
        GetDevice()->IsToggleEnabled(Toggle::ScalarizeMaxMinClamp);

    req.tintOptions.workarounds.polyfill_saturate_as_min_max_f16 =
        GetDevice()->IsToggleEnabled(Toggle::SaturateAsMinMaxF16);

    req.tintOptions.workarounds.dva_transform_handle =
        GetDevice()->IsToggleEnabled(Toggle::VulkanDirectVariableAccessTransformHandle);
    req.tintOptions.workarounds.polyfill_subgroup_broadcast_f16 =
        GetDevice()->IsToggleEnabled(Toggle::EnableSubgroupsIntelGen9);
    req.tintOptions.workarounds.cooperative_matrix_stride_is_matrix_elements =
        GetDevice()->IsToggleEnabled(Toggle::VulkanCooperativeMatrixStrideIsMatrixElements);

    // Pass matrices to user functions by pointer on Qualcomm devices to workaround a known bug.
    // See crbug.com/tint/2045.
    if (ToBackend(GetDevice()->GetPhysicalDevice())->IsAndroidQualcomm()) {
        req.tintOptions.workarounds.pass_matrix_by_pointer = true;
    }

    // Set internal immediate constant offsets
    if (HasImmediateConstants(&RenderImmediateConstants::clampFragDepth, in.immediateMask)) {
        uint32_t offsetStartBytes = GetImmediateByteOffsetInPipeline(
            &RenderImmediateConstants::clampFragDepth, in.immediateMask);
        req.tintOptions.depth_range_offsets = {
            offsetStartBytes, offsetStartBytes + kImmediateConstantElementByteSize};
    }

    req.limits = LimitsForCompilationRequest::Create(GetDevice()->GetLimits().v1);
    req.adapterSupportedLimits = UnsafeUnserializedValue(
        LimitsForCompilationRequest::Create(GetDevice()->GetAdapter()->GetLimits().v1));
    req.maxSubgroupSize = GetDevice()->GetAdapter()->GetPhysicalDevice()->GetSubgroupMaxSize();
    if (GetDevice()->HasFeature(Feature::ChromiumExperimentalSubgroupSizeControl)) {
        req.minExplicitComputeSubgroupSize =
            GetDevice()->GetAdapter()->GetPhysicalDevice()->GetMinExplicitComputeSubgroupSize();
        req.maxExplicitComputeSubgroupSize =
            GetDevice()->GetAdapter()->GetPhysicalDevice()->GetMaxExplicitComputeSubgroupSize();
        req.maxComputeWorkgroupSubgroups =
            GetDevice()->GetAdapter()->GetPhysicalDevice()->GetMaxComputeWorkgroupSubgroups();
    }

    CacheResult<CompiledSpirv> compilation;
    DAWN_TRY_LOAD_OR_RUN(
        compilation, GetDevice(), std::move(req), CompiledSpirv::FromValidatedBlob,
        [](SpirvCompilationRequest r) -> ResultOrError<CompiledSpirv> {
            TRACE_EVENT0(r.platform.UnsafeGetValue(), General, "tint::spirv::writer::Generate()");

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
                    .dump_ir_when_validating = device->IsToggleEnabled(Toggle::DumpTintIR),
                    .enable_validation_asserts =
                        device->IsToggleEnabled(Toggle::EnableTintIRValidationAsserts),
                };
                ir = tint::wgsl::reader::ProgramToLoweredIR(*tintInputProgram, irOptions);
                DAWN_INVALID_IF(ir != tint::Success,
                                "An error occurred while generating Tint IR\n%s",
                                ir.Failure().reason);
            }

            tint::Result<tint::spirv::writer::Output> tintResult;
            {
                SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(r.platform.UnsafeGetValue(),
                                                   "ShaderModuleGenerateSPIRV");
                // Generate SPIR-V from Tint IR.
                tintResult = tint::spirv::writer::Generate(ir.Get(), r.tintOptions);
                DAWN_INVALID_IF(tintResult != tint::Success,
                                "An error occurred while generating SPIR-V\n%s",
                                tintResult.Failure().reason);
            }

            // Workgroup validation has to come after `Generate` because it may require
            // overrides to have been substituted.
            if (r.stage == SingleShaderStage::Compute) {
                Extent3D _;
                DAWN_TRY_ASSIGN(
                    _, ValidateComputeStageWorkgroupSize(
                           tintResult->workgroup_info, r.usesSubgroupMatrix, r.maxSubgroupSize,
                           r.limits, r.adapterSupportedLimits.UnsafeGetValue()));
                DAWN_TRY(ValidateExplicitComputeSubgroupSize(
                    tintResult->workgroup_info, r.minExplicitComputeSubgroupSize,
                    r.maxExplicitComputeSubgroupSize, r.maxComputeWorkgroupSubgroups));
            }

            DAWN_TRY(ValidateSubgroupMatrixConfiguration(tintResult->subgroup_matrix_info,
                                                         r.subgroupMatrixConfig));

            CompiledSpirv result;
            result.workgroupSize = {tintResult->workgroup_info.x, tintResult->workgroup_info.y,
                                    tintResult->workgroup_info.z};
            result.explicitSubgroupSize = tintResult->workgroup_info.subgroup_size;
            result.spirv = std::move(tintResult.Get().spirv);
            return result;
        },
        "Vulkan.CompileShaderToSPIRV");

#ifdef DAWN_ENABLE_SPIRV_VALIDATION
    if (GetDevice()->IsToggleEnabled(Toggle::DumpShaders)) {
        DumpSpirv(GetDevice(), compilation->spirv.data(), compilation->spirv.size());
    }

    if (GetDevice()->IsToggleEnabled(Toggle::EnableSpirvValidation)) {
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(GetDevice()->GetPlatform(), "Vulkan.ValidateSpirv");

        // Validate and if required dump the compiled SPIR-V code.
        const bool spv14 = GetDevice()->IsToggleEnabled(Toggle::UseSpirv14);
        DAWN_TRY(ValidateSpirv(GetDevice(), compilation->spirv.data(), compilation->spirv.size(),
                               spv14));
    }
#endif

    VkShaderModuleCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0u;
    createInfo.codeSize = compilation->spirv.size() * sizeof(uint32_t);
    createInfo.pCode = compilation->spirv.data();

    Device* device = ToBackend(GetDevice());

    VkShaderModule newHandle = VK_NULL_HANDLE;
    {
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(GetDevice()->GetPlatform(), "Vulkan.CreateShaderModule");
        TRACE_EVENT0(GetDevice()->GetPlatform(), General, "vkCreateShaderModule");
        DAWN_TRY(CheckVkSuccess(
            device->fn.CreateShaderModule(device->GetVkDevice(), &createInfo, nullptr, &*newHandle),
            "CreateShaderModule"));
    }
    DAWN_CHECK(newHandle != VK_NULL_HANDLE);

    device->GetBlobCache()->EnsureStored(compilation);
    SetDebugName(ToBackend(GetDevice()), newHandle, "Dawn_ShaderModule", GetLabel());

    return ModuleAndSpirv{.module = newHandle,
                          .spirv = std::move(compilation->spirv),
                          .hasInputAttachment = hasInputAttachment,
                          .workgroupSize = compilation->workgroupSize,
                          .explicitSubgroupSize = compilation->explicitSubgroupSize};
#else
    return DAWN_INTERNAL_ERROR("TINT_BUILD_SPV_WRITER is not defined.");
#endif
}

}  // namespace dawn::native::vulkan
