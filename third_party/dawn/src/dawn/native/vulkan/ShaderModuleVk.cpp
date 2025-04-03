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

#define COMPILED_SPIRV_MEMBERS(X) X(std::vector<uint32_t>, spirv)

// Represents the result and metadata for a SPIR-V compilation.
DAWN_SERIALIZABLE(struct, CompiledSpirv, COMPILED_SPIRV_MEMBERS){};
#undef COMPILED_SPIRV_MEMBERS

bool TransformedShaderModuleCacheKey::operator==(
    const TransformedShaderModuleCacheKey& other) const {
    if (layoutPtr != other.layoutPtr || entryPoint != other.entryPoint ||
        constants.size() != other.constants.size()) {
        return false;
    }
    if (!std::equal(constants.begin(), constants.end(), other.constants.begin())) {
        return false;
    }
    if (emitPointSize != other.emitPointSize) {
        return false;
    }
    return true;
}

size_t TransformedShaderModuleCacheKeyHashFunc::operator()(
    const TransformedShaderModuleCacheKey& key) const {
    size_t hash = 0;
    HashCombine(&hash, key.layoutPtr, key.entryPoint, key.emitPointSize);
    for (const auto& entry : key.constants) {
        HashCombine(&hash, entry.first, entry.second);
    }
    return hash;
}

class ShaderModule::ConcurrentTransformedShaderModuleCache {
  public:
    explicit ConcurrentTransformedShaderModuleCache(Device* device) : mDevice(device) {}

    ~ConcurrentTransformedShaderModuleCache() {
        std::lock_guard<std::mutex> lock(mMutex);

        for (const auto& [_, moduleAndSpirv] : mTransformedShaderModuleCache) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(moduleAndSpirv.vkModule);
        }
    }

    std::optional<ModuleAndSpirv> Find(const TransformedShaderModuleCacheKey& key) {
        std::lock_guard<std::mutex> lock(mMutex);

        auto iter = mTransformedShaderModuleCache.find(key);
        if (iter != mTransformedShaderModuleCache.end()) {
            return iter->second.AsRefs();
        }
        return {};
    }
    ModuleAndSpirv AddOrGet(const TransformedShaderModuleCacheKey& key,
                            VkShaderModule module,
                            CompiledSpirv compilation,
                            bool hasInputAttachment) {
        DAWN_ASSERT(module != VK_NULL_HANDLE);
        std::lock_guard<std::mutex> lock(mMutex);

        auto iter = mTransformedShaderModuleCache.find(key);
        if (iter == mTransformedShaderModuleCache.end()) {
            bool added = false;
            std::tie(iter, added) = mTransformedShaderModuleCache.emplace(
                key, Entry{module, std::move(compilation.spirv), hasInputAttachment});
            DAWN_ASSERT(added);
        } else {
            // No need to use FencedDeleter since this shader module was just created and does
            // not need to wait for queue operations to complete.
            // Also, use of fenced deleter here is not thread safe.
            mDevice->fn.DestroyShaderModule(mDevice->GetVkDevice(), module, nullptr);
        }
        return iter->second.AsRefs();
    }

  private:
    struct Entry {
        VkShaderModule vkModule;
        std::vector<uint32_t> spirv;
        bool hasInputAttachment;

        ModuleAndSpirv AsRefs() const {
            return {
                vkModule,
                spirv.data(),
                spirv.size(),
                hasInputAttachment,
            };
        }
    };

    raw_ptr<Device> mDevice;
    std::mutex mMutex;
    absl::flat_hash_map<TransformedShaderModuleCacheKey,
                        Entry,
                        TransformedShaderModuleCacheKeyHashFunc>
        mTransformedShaderModuleCache;
};

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
    : ShaderModuleBase(device, descriptor, std::move(internalExtensions)),
      mTransformedShaderModuleCache(
          std::make_unique<ConcurrentTransformedShaderModuleCache>(device)) {}

MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult,
                                    OwnedCompilationMessages* compilationMessages) {
    return InitializeBase(parseResult, compilationMessages);
}

void ShaderModule::DestroyImpl() {
    ShaderModuleBase::DestroyImpl();
    // Remove reference to internal cache to trigger cleanup.
    mTransformedShaderModuleCache = nullptr;
}

ShaderModule::~ShaderModule() = default;

#if TINT_BUILD_SPV_WRITER

#define SPIRV_COMPILATION_REQUEST_MEMBERS(X)                                                     \
    X(SingleShaderStage, stage)                                                                  \
    X(const tint::Program*, inputProgram)                                                        \
    X(std::optional<tint::ast::transform::SubstituteOverride::Config>, substituteOverrideConfig) \
    X(LimitsForCompilationRequest, limits)                                                       \
    X(CacheKey::UnsafeUnkeyedValue<LimitsForCompilationRequest>, adapterSupportedLimits)         \
    X(uint32_t, maxSubgroupSize)                                                                 \
    X(std::string_view, entryPointName)                                                          \
    X(bool, usesSubgroupMatrix)                                                                  \
    X(tint::spirv::writer::Options, tintOptions)                                                 \
    X(CacheKey::UnsafeUnkeyedValue<dawn::platform::Platform*>, platform)

DAWN_MAKE_CACHE_REQUEST(SpirvCompilationRequest, SPIRV_COMPILATION_REQUEST_MEMBERS);
#undef SPIRV_COMPILATION_REQUEST_MEMBERS

#endif  // TINT_BUILD_SPV_WRITER

ResultOrError<ShaderModule::ModuleAndSpirv> ShaderModule::GetHandleAndSpirv(
    SingleShaderStage stage,
    const ProgrammableStage& programmableStage,
    const PipelineLayout* layout,
    bool emitPointSize,
    const ImmediateConstantMask& pipelineImmediateMask) {
    TRACE_EVENT0(GetDevice()->GetPlatform(), General, "ShaderModuleVk::GetHandleAndSpirv");

    // Check to see if we have the handle and spirv cached already
    // TODO(chromium:345359083): Improve the computation of the cache key. For example, it isn't
    // ideal to use `reinterpret_cast<uintptr_t>(layout)` as the layout may be freed and
    // reallocated during the runtime.
    auto cacheKey = TransformedShaderModuleCacheKey{reinterpret_cast<uintptr_t>(layout),
                                                    programmableStage.entryPoint.c_str(),
                                                    programmableStage.constants, emitPointSize};
    auto handleAndSpirv = mTransformedShaderModuleCache->Find(cacheKey);
    if (handleAndSpirv.has_value()) {
        return std::move(*handleAndSpirv);
    }

#if TINT_BUILD_SPV_WRITER
    // Creation of module and spirv is deferred to this point when using tint generator

    tint::spirv::writer::Bindings bindings;
    std::unordered_set<tint::BindingPoint> statically_paired_texture_binding_points;

    const BindingInfoArray& moduleBindingInfo =
        GetEntryPoint(programmableStage.entryPoint.c_str()).bindings;

    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        const BindGroupLayout* bgl = ToBackend(layout->GetBindGroupLayout(group));

        for (const auto& currentModuleBindingInfo : moduleBindingInfo[group]) {
            // We cannot use structured binding here because lambda expressions can only capture
            // variables, while structured binding doesn't introduce variables.
            const auto& binding = currentModuleBindingInfo.first;
            const auto& shaderBindingInfo = currentModuleBindingInfo.second;

            tint::BindingPoint srcBindingPoint{static_cast<uint32_t>(group),
                                               static_cast<uint32_t>(binding)};

            tint::BindingPoint dstBindingPoint{
                static_cast<uint32_t>(group), static_cast<uint32_t>(bgl->GetBindingIndex(binding))};

            MatchVariant(
                shaderBindingInfo.bindingInfo,
                [&](const BufferBindingInfo& bindingInfo) {
                    switch (bindingInfo.type) {
                        case wgpu::BufferBindingType::Uniform:
                            bindings.uniform.emplace(
                                srcBindingPoint,
                                tint::spirv::writer::binding::Uniform{dstBindingPoint.group,
                                                                      dstBindingPoint.binding});
                            break;
                        case kInternalStorageBufferBinding:
                        case wgpu::BufferBindingType::Storage:
                        case wgpu::BufferBindingType::ReadOnlyStorage:
                        case kInternalReadOnlyStorageBufferBinding:
                            bindings.storage.emplace(
                                srcBindingPoint,
                                tint::spirv::writer::binding::Storage{dstBindingPoint.group,
                                                                      dstBindingPoint.binding});
                            break;
                        case wgpu::BufferBindingType::BindingNotUsed:
                        case wgpu::BufferBindingType::Undefined:
                            DAWN_UNREACHABLE();
                            break;
                    }
                },
                [&](const SamplerBindingInfo& bindingInfo) {
                    bindings.sampler.emplace(srcBindingPoint,
                                             tint::spirv::writer::binding::Sampler{
                                                 dstBindingPoint.group, dstBindingPoint.binding});
                },
                [&](const TextureBindingInfo& bindingInfo) {
                    if (auto samplerIndex = bgl->GetStaticSamplerIndexForTexture(
                            BindingIndex{dstBindingPoint.binding})) {
                        dstBindingPoint.binding = static_cast<uint32_t>(samplerIndex.value());
                        statically_paired_texture_binding_points.insert(srcBindingPoint);
                    }
                    bindings.texture.emplace(srcBindingPoint,
                                             tint::spirv::writer::binding::Texture{
                                                 dstBindingPoint.group, dstBindingPoint.binding});
                },
                [&](const StorageTextureBindingInfo& bindingInfo) {
                    bindings.storage_texture.emplace(
                        srcBindingPoint, tint::spirv::writer::binding::StorageTexture{
                                             dstBindingPoint.group, dstBindingPoint.binding});
                },
                [&](const ExternalTextureBindingInfo& bindingInfo) {
                    const auto& bindingMap = bgl->GetExternalTextureBindingExpansionMap();
                    const auto& expansion = bindingMap.find(binding);
                    DAWN_ASSERT(expansion != bindingMap.end());

                    const auto& bindingExpansion = expansion->second;
                    tint::spirv::writer::binding::BindingInfo plane0{
                        static_cast<uint32_t>(group),
                        static_cast<uint32_t>(bgl->GetBindingIndex(bindingExpansion.plane0))};
                    tint::spirv::writer::binding::BindingInfo plane1{
                        static_cast<uint32_t>(group),
                        static_cast<uint32_t>(bgl->GetBindingIndex(bindingExpansion.plane1))};
                    tint::spirv::writer::binding::BindingInfo metadata{
                        static_cast<uint32_t>(group),
                        static_cast<uint32_t>(bgl->GetBindingIndex(bindingExpansion.params))};

                    bindings.external_texture.emplace(
                        srcBindingPoint,
                        tint::spirv::writer::binding::ExternalTexture{metadata, plane0, plane1});
                },
                [&](const InputAttachmentBindingInfo& bindingInfo) {
                    bindings.input_attachment.emplace(
                        srcBindingPoint, tint::spirv::writer::binding::InputAttachment{
                                             dstBindingPoint.group, dstBindingPoint.binding});
                });
        }
    }

    const bool hasInputAttachment = !bindings.input_attachment.empty();

    std::optional<tint::ast::transform::SubstituteOverride::Config> substituteOverrideConfig;
    if (!programmableStage.metadata->overrides.empty()) {
        substituteOverrideConfig = BuildSubstituteOverridesTransformConfig(programmableStage);
    }

    SpirvCompilationRequest req = {};
    req.stage = stage;
    auto tintProgram = GetTintProgram();
    req.inputProgram = &(tintProgram->program);
    req.entryPointName = programmableStage.entryPoint;
    req.platform = UnsafeUnkeyedValue(GetDevice()->GetPlatform());
    req.substituteOverrideConfig = std::move(substituteOverrideConfig);
    req.usesSubgroupMatrix = programmableStage.metadata->usesSubgroupMatrix;

    req.tintOptions.remapped_entry_point_name = kRemappedEntryPointName;
    req.tintOptions.strip_all_names = !GetDevice()->IsToggleEnabled(Toggle::DisableSymbolRenaming);

    req.tintOptions.statically_paired_texture_binding_points =
        std::move(statically_paired_texture_binding_points);
    req.tintOptions.disable_robustness = !GetDevice()->IsRobustnessEnabled();
    req.tintOptions.emit_vertex_point_size = emitPointSize;

    req.tintOptions.disable_workgroup_init =
        GetDevice()->IsToggleEnabled(Toggle::DisableWorkgroupInit);
    // The only possible alternative for the vulkan demote to helper extension is
    // "OpTerminateInvocation" which remains unimplemented in dawn/tint.
    req.tintOptions.use_demote_to_helper_invocation_extensions =
        GetDevice()->IsToggleEnabled(Toggle::VulkanUseDemoteToHelperInvocationExtension);

    req.tintOptions.use_zero_initialize_workgroup_memory_extension =
        GetDevice()->IsToggleEnabled(Toggle::VulkanUseZeroInitializeWorkgroupMemoryExtension);
    req.tintOptions.use_storage_input_output_16 =
        GetDevice()->IsToggleEnabled(Toggle::VulkanUseStorageInputOutput16);
    req.tintOptions.bindings = std::move(bindings);
    req.tintOptions.disable_image_robustness =
        GetDevice()->IsToggleEnabled(Toggle::VulkanUseImageRobustAccess2);
    // Currently we can disable index clamping on all runtime-sized arrays in Tint robustness
    // transform as unsized arrays can only be declared on storage address space.
    req.tintOptions.disable_runtime_sized_array_index_clamping =
        GetDevice()->IsToggleEnabled(Toggle::VulkanUseBufferRobustAccess2);
    req.tintOptions.polyfill_dot_4x8_packed =
        GetDevice()->IsToggleEnabled(Toggle::PolyFillPacked4x8DotProduct);
    req.tintOptions.polyfill_pack_unpack_4x8_norm =
        GetDevice()->IsToggleEnabled(Toggle::PolyfillPackUnpack4x8Norm);
    req.tintOptions.disable_polyfill_integer_div_mod =
        GetDevice()->IsToggleEnabled(Toggle::DisablePolyfillsOnIntegerDivisonAndModulo);
    req.tintOptions.use_vulkan_memory_model =
        GetDevice()->IsToggleEnabled(Toggle::UseVulkanMemoryModel);

    // Pass matrices to user functions by pointer on Qualcomm devices to workaround a known bug.
    // See crbug.com/tint/2045.
    if (ToBackend(GetDevice()->GetPhysicalDevice())->IsAndroidQualcomm()) {
        req.tintOptions.pass_matrix_by_pointer = true;
    }

    // Set internal immediate constant offsets
    if (HasImmediateConstants(&RenderImmediateConstants::clampFragDepth, pipelineImmediateMask)) {
        uint32_t offsetStartBytes = GetImmediateByteOffsetInPipeline(
            &RenderImmediateConstants::clampFragDepth, pipelineImmediateMask);
        req.tintOptions.depth_range_offsets = {
            offsetStartBytes, offsetStartBytes + kImmediateConstantElementByteSize};
    }

    req.limits = LimitsForCompilationRequest::Create(GetDevice()->GetLimits().v1);
    req.adapterSupportedLimits =
        LimitsForCompilationRequest::Create(GetDevice()->GetAdapter()->GetLimits().v1);
    req.maxSubgroupSize = GetDevice()->GetAdapter()->GetPhysicalDevice()->GetSubgroupMaxSize();

    CacheResult<CompiledSpirv> compilation;
    {
        ScopedTintICEHandler scopedICEHandler(GetDevice());
        DAWN_TRY_LOAD_OR_RUN(
            compilation, GetDevice(), std::move(req), CompiledSpirv::FromBlob,
            [](SpirvCompilationRequest r) -> ResultOrError<CompiledSpirv> {
                TRACE_EVENT0(r.platform.UnsafeGetValue(), General,
                             "tint::spirv::writer::Generate()");

                // Convert the AST program to an IR module.
                tint::Result<tint::core::ir::Module> ir;
                {
                    SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(r.platform.UnsafeGetValue(),
                                                       "ShaderModuleProgramToIR");
                    ir = tint::wgsl::reader::ProgramToLoweredIR(*r.inputProgram);
                    DAWN_INVALID_IF(ir != tint::Success,
                                    "An error occurred while generating Tint IR\n%s",
                                    ir.Failure().reason);
                }

                {
                    SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(r.platform.UnsafeGetValue(),
                                                       "ShaderModuleSingleEntryPoint");
                    // Many Vulkan drivers can't handle multi-entrypoint shader modules.
                    auto singleEntryPointResult =
                        tint::core::ir::transform::SingleEntryPoint(ir.Get(), r.entryPointName);
                    DAWN_INVALID_IF(singleEntryPointResult != tint::Success,
                                    "Pipeline single entry point (IR) failed:\n%s",
                                    singleEntryPointResult.Failure().reason);
                }

                if (r.substituteOverrideConfig) {
                    SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(r.platform.UnsafeGetValue(),
                                                       "ShaderModuleSubstituteOverrides");
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
                        _,
                        ValidateComputeStageWorkgroupSize(
                            tintResult->workgroup_info.x, tintResult->workgroup_info.y,
                            tintResult->workgroup_info.z, tintResult->workgroup_info.storage_size,
                            r.usesSubgroupMatrix, r.maxSubgroupSize, r.limits,
                            r.adapterSupportedLimits.UnsafeGetValue()));
                }

                CompiledSpirv result;
                result.spirv = std::move(tintResult.Get().spirv);
                return result;
            },
            "Vulkan.CompileShaderToSPIRV");
    }

#ifdef DAWN_ENABLE_SPIRV_VALIDATION
    DAWN_TRY(ValidateSpirv(GetDevice(), compilation->spirv.data(), compilation->spirv.size(),
                           GetDevice()->IsToggleEnabled(Toggle::DumpShaders)));
#endif

    VkShaderModuleCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
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

    ModuleAndSpirv moduleAndSpirv;
    if (newHandle != VK_NULL_HANDLE) {
        device->GetBlobCache()->EnsureStored(compilation);

        // Set the label on `newHandle` now, and not on `moduleAndSpirv.module` later
        // since `moduleAndSpirv.module` may be in use by multiple threads.
        SetDebugName(ToBackend(GetDevice()), newHandle, "Dawn_ShaderModule", GetLabel());
        moduleAndSpirv = mTransformedShaderModuleCache->AddOrGet(
            cacheKey, newHandle, compilation.Acquire(), hasInputAttachment);
    }

    return std::move(moduleAndSpirv);
#else
    return DAWN_INTERNAL_ERROR("TINT_BUILD_SPV_WRITER is not defined.");
#endif
}

}  // namespace dawn::native::vulkan
