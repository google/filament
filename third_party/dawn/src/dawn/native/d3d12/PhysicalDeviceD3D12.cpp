// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/native/d3d12/PhysicalDeviceD3D12.h"

#include <algorithm>
#include <string>
#include <utility>

#include "dawn/common/Constants.h"
#include "dawn/common/Platform.h"
#include "dawn/common/WindowsUtils.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/BackendD3D12.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"
#include "dawn/platform/DawnPlatform.h"

namespace dawn::native::d3d12 {

// The D3D12_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT_SMALLRESOURCE(1380) was introduced
// in SDK 10.0.26100.0. Redefine the message name to allows it to be compiled on both
// SDK 10.0.22621.0 and SDK 10.0.26100.0.
#if D3D12_SDK_VERSION >= 612
const D3D12_MESSAGE_ID DAWN_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT_SMALLRESOURCE =
    D3D12_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT_SMALLRESOURCE;
#else
const D3D12_MESSAGE_ID DAWN_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT_SMALLRESOURCE =
    D3D12_MESSAGE_ID(1380);
#endif

PhysicalDevice::PhysicalDevice(Backend* backend, ComPtr<IDXGIAdapter3> hardwareAdapter)
    : Base(backend, std::move(hardwareAdapter), wgpu::BackendType::D3D12) {}

PhysicalDevice::~PhysicalDevice() {
    CleanUpDebugLayerFilters();
}

bool PhysicalDevice::SupportsExternalImages() const {
    // Via dawn::native::d3d::ExternalImageDXGI::Create
    return true;
}

bool PhysicalDevice::SupportsFeatureLevel(wgpu::FeatureLevel, InstanceBase* instance) const {
    return true;
}

uint32_t PhysicalDevice::GetAppliedShaderModelUnderToggles(const TogglesState& toggles) const {
    uint32_t appliedShaderModel = GetDeviceInfo().highestSupportedShaderModel;
    if ((appliedShaderModel >= 66) &&
        toggles.IsEnabled(Toggle::D3D12DontUseShaderModel66OrHigher)) {
        appliedShaderModel = 65;
    }
    return appliedShaderModel;
}

const D3D12DeviceInfo& PhysicalDevice::GetDeviceInfo() const {
    return mDeviceInfo;
}

Backend* PhysicalDevice::GetBackend() const {
    return static_cast<Backend*>(Base::GetBackend());
}

ComPtr<ID3D12Device> PhysicalDevice::GetDevice() const {
    return mD3d12Device;
}

MaybeError PhysicalDevice::InitializeImpl() {
    DAWN_TRY(Base::InitializeImpl());
    // D3D12 cannot check for feature support without a device.
    // Create the device to populate the adapter properties then reuse it when needed for actual
    // rendering.
    const PlatformFunctions* functions = GetBackend()->GetFunctions();
    if (FAILED(functions->d3d12CreateDevice(GetHardwareAdapter(), D3D_FEATURE_LEVEL_11_0,
                                            __uuidof(ID3D12Device), &mD3d12Device))) {
        return DAWN_INTERNAL_ERROR("D3D12CreateDevice failed");
    }

    DAWN_TRY(InitializeDebugLayerFilters());

    DAWN_TRY_ASSIGN(mDeviceInfo, GatherDeviceInfo(*this));

    // Base::InitializeImpl() cannot distinguish between discrete and integrated GPUs, so we need to
    // overwrite it here.
    if (mAdapterType == wgpu::AdapterType::DiscreteGPU && mDeviceInfo.isUMA) {
        mAdapterType = wgpu::AdapterType::IntegratedGPU;
    }

    mSubgroupMinSize = mDeviceInfo.waveLaneCountMin;
    // Currently the WaveLaneCountMax queried from D3D12 API is not reliable and the meaning is
    // unclear. Use 128 instead, which is the largest possible size. Reference:
    // https://github.com/Microsoft/DirectXShaderCompiler/wiki/Wave-Intrinsics#:~:text=UINT%20WaveLaneCountMax
    mSubgroupMaxSize = 128u;

    return {};
}

bool PhysicalDevice::AreTimestampQueriesSupported() const {
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ComPtr<ID3D12CommandQueue> d3d12CommandQueue;
    HRESULT hr = mD3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&d3d12CommandQueue));
    if (FAILED(hr)) {
        return false;
    }

    // GetTimestampFrequency returns an error HRESULT when there are bugs in Windows container
    // and vGPU implementations.
    uint64_t timeStampFrequency;
    hr = d3d12CommandQueue->GetTimestampFrequency(&timeStampFrequency);
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

void PhysicalDevice::InitializeSupportedFeaturesImpl() {
    EnableFeature(Feature::TextureCompressionBC);
    EnableFeature(Feature::DawnMultiPlanarFormats);
    EnableFeature(Feature::Depth32FloatStencil8);
    EnableFeature(Feature::IndirectFirstInstance);
    EnableFeature(Feature::RG11B10UfloatRenderable);
    EnableFeature(Feature::DepthClipControl);
    EnableFeature(Feature::Float32Filterable);
    EnableFeature(Feature::Float32Blendable);
    EnableFeature(Feature::DualSourceBlending);
    EnableFeature(Feature::Unorm16TextureFormats);
    EnableFeature(Feature::Snorm16TextureFormats);
    EnableFeature(Feature::Norm16TextureFormats);
    EnableFeature(Feature::AdapterPropertiesMemoryHeaps);
    EnableFeature(Feature::AdapterPropertiesD3D);
    EnableFeature(Feature::MultiPlanarRenderTargets);
    EnableFeature(Feature::R8UnormStorage);
    EnableFeature(Feature::SharedBufferMemoryD3D12Resource);
    EnableFeature(Feature::ShaderModuleCompilationOptions);
    EnableFeature(Feature::StaticSamplers);
    EnableFeature(Feature::MultiDrawIndirect);
    EnableFeature(Feature::ClipDistances);
    EnableFeature(Feature::FlexibleTextureViews);

    if (AreTimestampQueriesSupported()) {
        EnableFeature(Feature::TimestampQuery);
        EnableFeature(Feature::ChromiumExperimentalTimestampQueryInsidePasses);
    }

    // ShaderF16 features require DXC version being 1.4 or higher, shader model supporting 6.2 or
    // higher, and native supporting F16 shader ops.
    bool shaderF16Enabled = false;
    if (GetBackend()->IsDXCAvailableAndVersionAtLeast(1, 4, 1, 4) &&
        mDeviceInfo.highestSupportedShaderModel >= 62 && mDeviceInfo.supportsNative16BitShaderOps) {
        EnableFeature(Feature::ShaderF16);
        shaderF16Enabled = true;
    }

    // The function subgroupBroadcast(f16) fails for some edge cases on intel gen-9 devices.
    // See crbug.com/391680973
    const bool kForceDisableSubgroups = gpu_info::IsIntelGen9(GetVendorId(), GetDeviceId());
    // Subgroups feature requires SM >= 6.0 and capabilities flags.
    if (!kForceDisableSubgroups && GetBackend()->IsDXCAvailable() && mDeviceInfo.supportsWaveOps) {
        EnableFeature(Feature::Subgroups);
        // D3D12 devices that support both native f16 and wave ops can support subgroups-f16.
        // TODO(crbug.com/380244620): Remove when 'subgroups_f16' has been fully deprecated.
        if (shaderF16Enabled) {
            EnableFeature(Feature::SubgroupsF16);
        }
    }

    D3D12_FEATURE_DATA_FORMAT_SUPPORT bgra8unormFormatInfo = {};
    bgra8unormFormatInfo.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    HRESULT hr = mD3d12Device->CheckFeatureSupport(
        D3D12_FEATURE_FORMAT_SUPPORT, &bgra8unormFormatInfo, sizeof(bgra8unormFormatInfo));
    if (SUCCEEDED(hr) &&
        (bgra8unormFormatInfo.Support1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW)) {
        EnableFeature(Feature::BGRA8UnormStorage);
    }

    D3D12_FEATURE_DATA_EXISTING_HEAPS existingHeapInfo = {};
    hr = mD3d12Device->CheckFeatureSupport(D3D12_FEATURE_EXISTING_HEAPS, &existingHeapInfo,
                                           sizeof(existingHeapInfo));
    if (SUCCEEDED(hr) && existingHeapInfo.Supported) {
        EnableFeature(Feature::HostMappedPointer);
    }

    EnableFeature(Feature::SharedTextureMemoryDXGISharedHandle);
    EnableFeature(Feature::SharedFenceDXGISharedHandle);

    if (GetDeviceInfo().isUMA && GetDeviceInfo().isCacheCoherentUMA) {
        EnableFeature(Feature::BufferMapExtendedUsages);
    }
}

MaybeError PhysicalDevice::InitializeSupportedLimitsImpl(CombinedLimits* limits) {
    D3D12_FEATURE_DATA_D3D12_OPTIONS featureData = {};

    DAWN_TRY(CheckHRESULT(mD3d12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS,
                                                            &featureData, sizeof(featureData)),
                          "CheckFeatureSupport D3D12_FEATURE_D3D12_OPTIONS"));

    // Check if the device is at least D3D_FEATURE_LEVEL_11_1 or D3D_FEATURE_LEVEL_11_0
    const D3D_FEATURE_LEVEL levelsToQuery[]{D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

    D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevels;
    featureLevels.NumFeatureLevels = sizeof(levelsToQuery) / sizeof(D3D_FEATURE_LEVEL);
    featureLevels.pFeatureLevelsRequested = levelsToQuery;
    DAWN_TRY(CheckHRESULT(mD3d12Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS,
                                                            &featureLevels, sizeof(featureLevels)),
                          "CheckFeatureSupport D3D12_FEATURE_FEATURE_LEVELS"));

    if (featureLevels.MaxSupportedFeatureLevel == D3D_FEATURE_LEVEL_11_0 &&
        featureData.ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_2) {
        return DAWN_VALIDATION_ERROR(
            "At least Resource Binding Tier 2 is required for D3D12 Feature Level 11.0 "
            "devices.");
    }

    GetDefaultLimitsForSupportedFeatureLevel(&limits->v1);

    // https://docs.microsoft.com/en-us/windows/win32/direct3d12/hardware-feature-levels

    // Limits that are the same across D3D feature levels
    limits->v1.maxTextureDimension1D = D3D12_REQ_TEXTURE1D_U_DIMENSION;
    limits->v1.maxTextureDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    limits->v1.maxTextureDimension3D = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
    limits->v1.maxTextureArrayLayers = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
    // Slot values can be 0-15, inclusive:
    // https://docs.microsoft.com/en-ca/windows/win32/api/d3d12/ns-d3d12-d3d12_input_element_desc
    limits->v1.maxVertexBuffers = 16;
    // Both SV_VertexID and SV_InstanceID will consume vertex input slots.
    limits->v1.maxVertexAttributes = D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - 2;

    // Note: WebGPU requires FL11.1+
    // https://docs.microsoft.com/en-us/windows/win32/direct3d12/hardware-support
    // Resource Binding Tier:   1      2      3

    // Max(CBV+UAV+SRV)         1M    1M    1M+
    // Max CBV per stage        14    14   full
    // Max SRV per stage       128  full   full
    // Max UAV in all stages    64    64   full
    // Max Samplers per stage   16  2048   2048

    // https://docs.microsoft.com/en-us/windows-hardware/test/hlk/testref/efad06e8-51d1-40ce-ad5c-573a134b4bb6
    // "full" means the full heap can be used. This is tested
    // to work for 1 million descriptors, and 1.1M for tier 3.
    uint32_t maxCBVsPerStage;
    uint32_t maxSRVsPerStage;
    uint32_t maxUAVsAllStages;
    uint32_t maxSamplersPerStage;
    switch (featureData.ResourceBindingTier) {
        case D3D12_RESOURCE_BINDING_TIER_1:
            maxCBVsPerStage = 14;
            maxSRVsPerStage = 128;
            maxUAVsAllStages = 64;
            maxSamplersPerStage = 16;
            break;
        case D3D12_RESOURCE_BINDING_TIER_2:
            maxCBVsPerStage = 14;
            maxSRVsPerStage = 1'000'000;
            maxUAVsAllStages = 64;
            maxSamplersPerStage = 2048;
            break;
        case D3D12_RESOURCE_BINDING_TIER_3:
        default:
            maxCBVsPerStage = 1'100'000;
            maxSRVsPerStage = 1'100'000;
            maxUAVsAllStages = 1'100'000;
            maxSamplersPerStage = 2048;
            break;
    }

    DAWN_ASSERT(maxUAVsAllStages / 4 > limits->v1.maxStorageTexturesPerShaderStage);
    DAWN_ASSERT(maxUAVsAllStages / 4 > limits->v1.maxStorageBuffersPerShaderStage);
    uint32_t maxUAVsPerStage = maxUAVsAllStages / 2;

    limits->v1.maxUniformBuffersPerShaderStage = maxCBVsPerStage;
    // Allocate half of the UAVs to storage buffers, and half to storage textures.
    limits->v1.maxStorageTexturesPerShaderStage = maxUAVsPerStage / 2;
    limits->v1.maxStorageBuffersPerShaderStage = maxUAVsPerStage - maxUAVsPerStage / 2;
    limits->v1.maxStorageTexturesInFragmentStage = limits->v1.maxStorageTexturesPerShaderStage;
    limits->v1.maxStorageBuffersInFragmentStage = limits->v1.maxStorageBuffersPerShaderStage;
    limits->v1.maxStorageTexturesInVertexStage = limits->v1.maxStorageTexturesPerShaderStage;
    limits->v1.maxStorageBuffersInVertexStage = limits->v1.maxStorageBuffersPerShaderStage;
    limits->v1.maxSampledTexturesPerShaderStage = maxSRVsPerStage;
    limits->v1.maxSamplersPerShaderStage = maxSamplersPerStage;

    limits->v1.maxColorAttachments = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
    // This is maxColorAttachments times 16, the color format with the largest cost.
    limits->v1.maxColorAttachmentBytesPerSample = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT * 16;

    // https://docs.microsoft.com/en-us/windows/win32/direct3d12/root-signature-limits
    // In DWORDS. Descriptor tables cost 1, Root constants cost 1, Root descriptors cost 2.
    static constexpr uint32_t kMaxRootSignatureSize = 64u;
    // Dawn maps WebGPU's binding model by:
    //  - (maxBindGroups)
    //    CBVs/UAVs/SRVs for bind group are a root descriptor table
    //  - (maxBindGroups)
    //    Samplers for each bind group are a root descriptor table
    //  - dynamic uniform buffers - root descriptor
    //  - dynamic storage buffers - root descriptor plus a root constant for the size
    //  RESERVED:
    //  - 3 = max of:
    //    - 2 root constants for the baseVertex/baseInstance constants.
    //    - 3 root constants for num workgroups X, Y, Z
    static constexpr uint32_t kReservedSlots = 3;

    // Costs:
    //  - bind group: 2 = 1 cbv/uav/srv table + 1 sampler table
    //  - dynamic uniform buffer: 2 slots for a root descriptor
    //  - dynamic storage buffer: 3 slots for a root descriptor + root constant

    // Available slots after base limits considered.
    uint32_t availableRootSignatureSlots =
        kMaxRootSignatureSize - kReservedSlots - 2 * limits->v1.maxBindGroups -
        2 * limits->v1.maxDynamicUniformBuffersPerPipelineLayout -
        3 * limits->v1.maxDynamicStorageBuffersPerPipelineLayout;

    while (availableRootSignatureSlots >= 2) {
        // Start by incrementing maxDynamicStorageBuffersPerPipelineLayout since the
        // default is just 4 and developers likely want more. This scheme currently
        // gets us to 8.
        if (availableRootSignatureSlots >= 3) {
            limits->v1.maxDynamicStorageBuffersPerPipelineLayout += 1;
            availableRootSignatureSlots -= 3;
        }
        if (availableRootSignatureSlots >= 2) {
            limits->v1.maxBindGroups += 1;
            availableRootSignatureSlots -= 2;
        }
        if (availableRootSignatureSlots >= 2) {
            limits->v1.maxDynamicUniformBuffersPerPipelineLayout += 1;
            availableRootSignatureSlots -= 2;
        }
    }

    DAWN_ASSERT(2 * limits->v1.maxBindGroups +
                    2 * limits->v1.maxDynamicUniformBuffersPerPipelineLayout +
                    3 * limits->v1.maxDynamicStorageBuffersPerPipelineLayout <=
                kMaxRootSignatureSize - kReservedSlots);

    // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-attributes-numthreads
    limits->v1.maxComputeWorkgroupSizeX = D3D12_CS_THREAD_GROUP_MAX_X;
    limits->v1.maxComputeWorkgroupSizeY = D3D12_CS_THREAD_GROUP_MAX_Y;
    limits->v1.maxComputeWorkgroupSizeZ = D3D12_CS_THREAD_GROUP_MAX_Z;
    limits->v1.maxComputeInvocationsPerWorkgroup = D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;

    // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_dispatch_arguments
    limits->v1.maxComputeWorkgroupsPerDimension = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;

    // https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-downlevel-compute-shaders
    // Thread Group Shared Memory is limited to 16Kb on downlevel hardware. This is less than
    // the 32Kb that is available to Direct3D 11 hardware. D3D12 is also 32kb.
    limits->v1.maxComputeWorkgroupStorageSize = 32768;

    // Max number of "constants" where each constant is a 16-byte float4
    limits->v1.maxUniformBufferBindingSize = D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;

    // D3D12 has no documented limit on the buffer size.
    limits->v1.maxBufferSize = kAssumedMaxBufferSize;
    limits->v1.maxStorageBufferBindingSize = kAssumedMaxBufferSize;

    // 1 for SV_Position and 1 for (SV_IsFrontFace OR SV_SampleIndex).
    // See the discussions in https://github.com/gpuweb/gpuweb/issues/1962 for more details.
    limits->v1.maxInterStageShaderVariables = D3D12_PS_INPUT_REGISTER_COUNT - 2;

    // Using base limits for:
    // TODO(crbug.com/dawn/685):
    // - maxVertexBufferArrayStride

    if (gpu_info::IsQualcomm_ACPI(GetVendorId())) {
        // Due to a driver and hardware limitation, Raw Buffers can only address 2^27 WORDS instead
        // of the guaranteeed 2^31 bytes. Probably because it uses some form of texel buffer of
        // 32bit values to implement [RW]ByteAddressBuffer.
        limits->v1.maxStorageBufferBindingSize = sizeof(uint32_t)
                                                 << D3D12_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP;
    }

    return {};
}

FeatureValidationResult PhysicalDevice::ValidateFeatureSupportedWithTogglesImpl(
    wgpu::FeatureName feature,
    const TogglesState& toggles) const {
    // Toggle states of adapters and devices can change whether DXC is used and which shader model
    // version is applied. Validate features that requires DXC and/or specific shader model
    // version here.
    if (!toggles.IsEnabled(Toggle::UseDXC)) {
        // Disable features that require DXC. Note that required DXC version for each feature is
        // checked in InitializeSupportedFeaturesImpl.
        switch (feature) {
            case wgpu::FeatureName::ShaderF16:
            case wgpu::FeatureName::Subgroups:
            case wgpu::FeatureName::SubgroupsF16:
                return FeatureValidationResult(
                    absl::StrFormat("Feature %s requires DXC for D3D12.", feature));
            default:
                break;
        }
    }
    // Validate applied shader version.
    switch (feature) {
        // The feature `shader-f16` and `subgroups-f16` requires using shader model 6.2 or higher.
        case wgpu::FeatureName::ShaderF16:
        case wgpu::FeatureName::SubgroupsF16: {
            if (!(GetAppliedShaderModelUnderToggles(toggles) >= 62)) {
                return FeatureValidationResult(absl::StrFormat(
                    "Feature %s requires shader model 6.2 or higher for D3D12.", feature));
            }
            break;
        }
        default:
            break;
    }

    return {};
}

MaybeError PhysicalDevice::InitializeDebugLayerFilters() {
    // If the debug layer is not installed, return immediately.
    ComPtr<ID3D12InfoQueue> infoQueue;
    if (FAILED(mD3d12Device.As(&infoQueue))) {
        return {};
    }

    D3D12_MESSAGE_ID denyIds[] = {
        //
        // Permanent IDs: list of warnings that are not applicable
        //

        // Resource sub-allocation partially maps pre-allocated heaps. This means the
        // entire physical addresses space may have no resources or have many resources
        // assigned the same heap.
        D3D12_MESSAGE_ID_HEAP_ADDRESS_RANGE_HAS_NO_RESOURCE,
        D3D12_MESSAGE_ID_HEAP_ADDRESS_RANGE_INTERSECTS_MULTIPLE_BUFFERS,

        // The debug layer validates pipeline objects when they are created. Dawn validates
        // them when them when they are set. Therefore, since the issue is caught at a later
        // time, we can silence this warnings.
        D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_RENDERTARGETVIEW_NOT_SET,

        // Adding a clear color during resource creation would require heuristics or delayed
        // creation.
        // https://crbug.com/dawn/418
        D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
        D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,

        // Dawn enforces proper Unmaps at a later time.
        // https://crbug.com/dawn/422
        D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_GPU_WRITTEN_READBACK_RESOURCE_MAPPED,

        // WebGPU allows empty scissors without empty viewports.
        D3D12_MESSAGE_ID_DRAW_EMPTY_SCISSOR_RECTANGLE,

        // Backend textures can be reused across different frontend textures,
        // which can result in changes to the label of the backend texture if
        // the user has assigned distinct labels to the different frontend textures.
        D3D12_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,

        //
        // Temporary IDs: list of warnings that should be fixed or promoted
        //

        // For small placed resource alignment, we first request the small alignment, which may
        // get rejected and generate a debug error. Then, we request 0 to get the allowed
        // allowed alignment.
        D3D12_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT,
        // This message id is added for invalid small resource alignment.
        DAWN_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT_SMALLRESOURCE,

        // WebGPU allows OOB vertex buffer access and relies on D3D12's robust buffer access
        // behavior.
        D3D12_MESSAGE_ID_COMMAND_LIST_DRAW_VERTEX_BUFFER_TOO_SMALL,

        // WebGPU allows setVertexBuffer with offset that equals to the whole vertex buffer
        // size.
        // Even this means that no vertex buffer view has been set in D3D12 backend.
        // https://crbug.com/dawn/1255
        D3D12_MESSAGE_ID_COMMAND_LIST_DRAW_VERTEX_BUFFER_NOT_SET,

        // When using f16 in vertex attributes the debug layer may report float16_t as type
        // `unknown`, resulting in a CREATEINPUTLAYOUT_TYPE_MISMATCH warning.
        // https://crbug.com/tint/1473
        D3D12_MESSAGE_ID_CREATEINPUTLAYOUT_TYPE_MISMATCH,
    };

    // Create a retrieval filter with a deny list to suppress messages.
    // Any messages remaining will be converted to Dawn errors.
    D3D12_INFO_QUEUE_FILTER filter{};
    // Filter out info/message and only create errors from warnings or worse.
    D3D12_MESSAGE_SEVERITY severities[] = {
        D3D12_MESSAGE_SEVERITY_INFO,
        D3D12_MESSAGE_SEVERITY_MESSAGE,
    };
    filter.DenyList.NumSeverities = ARRAYSIZE(severities);
    filter.DenyList.pSeverityList = severities;
    filter.DenyList.NumIDs = ARRAYSIZE(denyIds);
    filter.DenyList.pIDList = denyIds;

    // To avoid flooding the console, a storage-filter is also used to
    // prevent messages from getting logged.
    DAWN_TRY(
        CheckHRESULT(infoQueue->PushStorageFilter(&filter), "ID3D12InfoQueue::PushStorageFilter"));

    DAWN_TRY(CheckHRESULT(infoQueue->PushRetrievalFilter(&filter),
                          "ID3D12InfoQueue::PushRetrievalFilter"));

    return {};
}

void PhysicalDevice::CleanUpDebugLayerFilters() {
    // The device may not exist if this adapter failed to initialize.
    if (mD3d12Device == nullptr) {
        return;
    }

    // If the debug layer is not installed, return immediately to avoid crashing the process.
    ComPtr<ID3D12InfoQueue> infoQueue;
    if (FAILED(mD3d12Device.As(&infoQueue))) {
        return;
    }

    infoQueue->PopRetrievalFilter();
    infoQueue->PopStorageFilter();
}

void PhysicalDevice::SetupBackendAdapterToggles(dawn::platform::Platform* platform,
                                                TogglesState* adapterToggles) const {
    // Check for use_dxc toggle
#ifdef DAWN_USE_BUILT_DXC
    // Default to using DXC. If shader model < 6.0, though, we must use FXC.
    if (GetDeviceInfo().highestSupportedShaderModel < 60) {
        adapterToggles->ForceSet(Toggle::UseDXC, false);
    }

    bool useDxc = platform->IsFeatureEnabled(dawn::platform::Features::kWebGPUUseDXC);
    adapterToggles->Default(Toggle::UseDXC, useDxc);
#else
    // Default to using FXC
    if (!GetBackend()->IsDXCAvailable()) {
        adapterToggles->ForceSet(Toggle::UseDXC, false);
    }
    adapterToggles->Default(Toggle::UseDXC, false);
#endif

    uint32_t deviceId = GetDeviceId();
    uint32_t vendorId = GetVendorId();

    // On Intel Gen12 D3D driver < 32.0.101.5762, using shader model 6.6 will cause unexpected
    // result when adding/subtracting I32/U32 vector/scalar with vector/scalar in constant
    // initialized array. See https://crbug.com/tint/2189 and https://crbug.com/dawn/2470 for more
    // information.
    if (gpu_info::IsIntelGen12HP(vendorId, deviceId) ||
        gpu_info::IsIntelGen12LP(vendorId, deviceId)) {
        if (gpu_info::CompareWindowsDriverVersion(vendorId, GetDriverVersion(),
                                                  {32, 0, 101, 5762}) == -1) {
            adapterToggles->Default(Toggle::D3D12DontUseShaderModel66OrHigher, true);
        }
    }

    // Workaround for textureDimensions() produces incorrect results with shader model 6.6 on Intel
    // D3D driver > 27.20.100.8935 and < 27.20.100.9684 on Intel Gen9 and Gen9.5 GPUs.
    // See https://crbug.com/dawn/2448 for more information.
    if (gpu_info::IsIntelGen9(vendorId, deviceId)) {
        if (gpu_info::CompareWindowsDriverVersion(vendorId, GetDriverVersion(),
                                                  {27, 20, 100, 8935}) == 1 &&
            gpu_info::CompareWindowsDriverVersion(vendorId, GetDriverVersion(),
                                                  {27, 20, 100, 9684}) == -1) {
            adapterToggles->ForceSet(Toggle::D3D12DontUseShaderModel66OrHigher, true);
        }
    }

    // On Intel Gen11 D3D12 GPUs using shader model 6.6 causes many unexpected issues.
    // See https://crbug.com/374606634 for more information.
    if (gpu_info::IsIntelGen11(vendorId, deviceId)) {
        adapterToggles->ForceSet(Toggle::D3D12DontUseShaderModel66OrHigher, true);
    }

    // On Intel Gen12LP fragment shader is possible to run with wave lane count of 8
    // while driver reporting WaveLaneCountMin being 16.
    if (gpu_info::IsIntelGen12LP(vendorId, deviceId)) {
        adapterToggles->Default(Toggle::D3D12RelaxMinSubgroupSizeTo8, true);
    }
}

void PhysicalDevice::SetupBackendDeviceToggles(dawn::platform::Platform* platform,
                                               TogglesState* deviceToggles) const {
    const bool useResourceHeapTier2 = (GetDeviceInfo().resourceHeapTier >= 2);
    deviceToggles->Default(Toggle::UseD3D12ResourceHeapTier2, useResourceHeapTier2);
    deviceToggles->Default(Toggle::UseD3D12RenderPass, GetDeviceInfo().supportsRenderPass);
    deviceToggles->Default(Toggle::UseD3D12ResidencyManagement, true);
    deviceToggles->Default(Toggle::D3D12AlwaysUseTypelessFormatsForCastableTexture,
                           !GetDeviceInfo().supportsCastingFullyTypedFormat);
    deviceToggles->Default(Toggle::ApplyClearBigIntegerColorValueWithDraw, true);

    // The restriction on the source box specifying a portion of the depth stencil texture in
    // CopyTextureRegion() is only available on the D3D12 platforms which doesn't support
    // programmable sample positions.
    deviceToggles->Default(
        Toggle::D3D12UseTempBufferInDepthStencilTextureAndBufferCopyWithNonZeroBufferOffset,
        GetDeviceInfo().programmableSamplePositionsTier == 0);

    // Disable optimizations when using FXC
    // See https://crbug.com/dawn/1203
    deviceToggles->Default(Toggle::FxcOptimizations, false);

    // By default use the maximum shader-visible heap size allowed.
    deviceToggles->Default(Toggle::UseD3D12SmallShaderVisibleHeapForTesting, false);

    // By default use D3D12 Root Signature Version 1.1 when possible, otherwise we should force
    // disable this toggle.
    // Additionally, DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS was only added in the
    // Windows 10 2018 Spring Creator's Update. Force disable the toggle if we do not have
    // at least WWDM 2.4.
    // https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html#flags-added-in-root-signature-version-11
    if (!GetDeviceInfo().supportsRootSignatureVersion1_1 || GetDriverVersion()[0] < 24) {
        deviceToggles->ForceSet(Toggle::D3D12UseRootSignatureVersion1_1, false);
    } else {
        deviceToggles->Default(Toggle::D3D12UseRootSignatureVersion1_1,
                               GetDeviceInfo().supportsRootSignatureVersion1_1);
    }

    // By default create MSAA textures with 64KB (D3D12_SMALL_MSAA_RESOURCE_PLACEMENT_ALIGNMENT)
    // alignment when possible, otherwise we should never enable this toggle.
    if (!GetDeviceInfo().use64KBAlignedMSAATexture) {
        deviceToggles->ForceSet(Toggle::D3D12Use64KBAlignedMSAATexture, false);
    }
    deviceToggles->Default(Toggle::D3D12Use64KBAlignedMSAATexture,
                           GetDeviceInfo().use64KBAlignedMSAATexture);

    // By default use D3D12_HEAP_FLAG_CREATE_NOT_ZEROED when possible, otherwise we should never
    // enable this toggle.
    if (!GetDeviceInfo().supportsHeapFlagCreateNotZeroed) {
        deviceToggles->ForceSet(Toggle::D3D12CreateNotZeroedHeap, false);
    }
    deviceToggles->Default(Toggle::D3D12CreateNotZeroedHeap,
                           GetDeviceInfo().supportsHeapFlagCreateNotZeroed);

    // By default allow relaxed row pitch and offset in buffer-texture copies when possible,
    // otherwise we should never enable this toggle.
    if (!GetDeviceInfo().supportsUnrestrictedBufferTextureCopyPitch) {
        deviceToggles->ForceSet(Toggle::D3D12RelaxBufferTextureCopyPitchAndOffsetAlignment, false);
    }
    deviceToggles->Default(Toggle::D3D12RelaxBufferTextureCopyPitchAndOffsetAlignment,
                           GetDeviceInfo().supportsUnrestrictedBufferTextureCopyPitch);

    // Native support of packed 4x8 integer dot product required shader model 6.4 or higher, and
    // DXC 1.4 or higher.
    if (!(GetAppliedShaderModelUnderToggles(*deviceToggles) >= 64) ||
        !deviceToggles->IsEnabled(Toggle::UseDXC) ||
        !GetBackend()->IsDXCAvailableAndVersionAtLeast(1, 4, 1, 4)) {
        deviceToggles->ForceSet(Toggle::PolyFillPacked4x8DotProduct, true);
    }

    // Native support of pack/unpack 4x8 intrinsics required shader model 6.6 or higher, and
    // DXC 1.4 or higher.
    if (!(GetAppliedShaderModelUnderToggles(*deviceToggles) >= 66) ||
        !deviceToggles->IsEnabled(Toggle::UseDXC) ||
        !GetBackend()->IsDXCAvailableAndVersionAtLeast(1, 6, 1, 6)) {
        deviceToggles->ForceSet(Toggle::D3D12PolyFillPackUnpack4x8, true);
    }

    uint32_t deviceId = GetDeviceId();
    uint32_t vendorId = GetVendorId();

    // Currently this workaround is only needed on Intel Gen9, Gen9.5 and Gen11 GPUs.
    // See http://crbug.com/1161355 for more information.
    if (gpu_info::IsIntelGen9(vendorId, deviceId) || gpu_info::IsIntelGen11(vendorId, deviceId)) {
        const gpu_info::DriverVersion kFixedDriverVersion = {31, 0, 101, 2114};
        if (gpu_info::CompareWindowsDriverVersion(vendorId, GetDriverVersion(),
                                                  kFixedDriverVersion) < 0) {
            deviceToggles->Default(
                Toggle::UseTempBufferInSmallFormatTextureToTextureCopyFromGreaterToLessMipLevel,
                true);
        }
    }

    // Currently this workaround is only needed on Intel Gen9, Gen9.5, Gen12 and Xe GPUs.
    // See http://crbug.com/dawn/1487 for more information.
    if (gpu_info::IsIntelGen9(vendorId, deviceId) || gpu_info::IsIntelGen12LP(vendorId, deviceId) ||
        gpu_info::IsIntelGen12HP(vendorId, deviceId) ||
        gpu_info::IsIntelXeLPG(vendorId, deviceId)) {
        deviceToggles->Default(Toggle::D3D12ForceClearCopyableDepthStencilTextureOnCreation, true);
    }

    // Currently this workaround is only needed on Intel Gen12 and Xe GPUs.
    // See http://crbug.com/dawn/1487 for more information.
    if (gpu_info::IsIntelGen12LP(vendorId, deviceId) ||
        gpu_info::IsIntelGen12HP(vendorId, deviceId) ||
        gpu_info::IsIntelXeLPG(vendorId, deviceId)) {
        deviceToggles->Default(Toggle::D3D12DontSetClearValueOnDepthTextureCreation, true);
    }

    // This workaround is only needed on Intel Gen12LP with driver prior to 30.0.101.1692.
    // See http://crbug.com/dawn/949 for more information.
    if (gpu_info::IsIntelGen12LP(vendorId, deviceId)) {
        const gpu_info::DriverVersion kFixedDriverVersion = {30, 0, 101, 1692};
        if (gpu_info::CompareWindowsDriverVersion(vendorId, GetDriverVersion(),
                                                  kFixedDriverVersion) == -1) {
            deviceToggles->Default(Toggle::D3D12AllocateExtraMemoryFor2DArrayColorTexture, true);
        }
    }

    // This workaround is needed on Intel Gen9 GPUs with driver >= 31.0.101.2121 and Gen12LP GPUs
    // with driver >= 31.0.101.4091. See http://crbug.com/dawn/1083 for more information.
    bool useBlitForT2T = false;
    if (gpu_info::IsIntelGen9(vendorId, deviceId)) {
        useBlitForT2T = gpu_info::CompareWindowsDriverVersion(vendorId, GetDriverVersion(),
                                                              {31, 0, 101, 2121}) != -1;
    } else if (gpu_info::IsIntelGen12LP(vendorId, deviceId)) {
        useBlitForT2T = gpu_info::CompareWindowsDriverVersion(vendorId, GetDriverVersion(),
                                                              {31, 0, 101, 4091}) != -1;
    }
    if (useBlitForT2T) {
        deviceToggles->Default(Toggle::UseBlitForDepthTextureToTextureCopyToNonzeroSubresource,
                               true);
    }

    // D3D driver has a bug resolving overlapping queries to a same buffer on Intel Gen12 GPUs. This
    // workaround is needed on the driver version >= 30.0.101.3413.
    // TODO(crbug.com/dawn/1546): Remove the workaround when the bug is fixed in D3D driver.
    if (gpu_info::IsIntelGen12LP(vendorId, deviceId) ||
        gpu_info::IsIntelGen12HP(vendorId, deviceId)) {
        const gpu_info::DriverVersion kDriverVersion = {30, 0, 101, 3413};
        if (gpu_info::CompareWindowsDriverVersion(vendorId, GetDriverVersion(), kDriverVersion) !=
            -1) {
            deviceToggles->Default(Toggle::ClearBufferBeforeResolveQueries, true);
        }
    }

    // B2T copy failed with stencil8 format on Intel ACM/MTL/ARL GPUs. See
    // https://issues.chromium.org/issues/368085621 for more information.
    if (gpu_info::IsIntelGen12HP(vendorId, deviceId) ||
        gpu_info::IsIntelXeLPG(vendorId, deviceId)) {
        deviceToggles->Default(Toggle::UseBlitForBufferToStencilTextureCopy, true);
    }

    // Currently these workarounds are needed on Intel Gen9.5 and Gen11 GPUs, as well as
    // AMD GPUS.
    // See http://crbug.com/1237175, http://crbug.com/dawn/1628, and http://crbug.com/dawn/2032
    // for more information.
    if ((gpu_info::IsIntelGen9(vendorId, deviceId) && !gpu_info::IsSkylake(deviceId)) ||
        gpu_info::IsIntelGen11(vendorId, deviceId) || gpu_info::IsAMD(vendorId)) {
        deviceToggles->Default(
            Toggle::DisableSubAllocationFor2DTextureWithCopyDstOrRenderAttachment, true);
        // Now we don't need to force clearing depth stencil textures with CopyDst as all the depth
        // stencil textures (can only be 2D textures) will be created with CreateCommittedResource()
        // instead of CreatePlacedResource().
        deviceToggles->Default(Toggle::D3D12ForceClearCopyableDepthStencilTextureOnCreation, false);
    }

    // Currently this toggle is only needed on Intel Gen9 and Gen9.5 GPUs.
    // See http://crbug.com/dawn/1579 for more information.
    if (gpu_info::IsIntelGen9(vendorId, deviceId)) {
        const gpu_info::DriverVersion kFixedDriverVersion = {31, 0, 101, 2121};
        if (gpu_info::CompareWindowsDriverVersion(vendorId, GetDriverVersion(),
                                                  kFixedDriverVersion) < 0) {
            // We can add workaround when the blending operation is "Add", DstFactor is "Zero" and
            // SrcFactor is "DstAlpha".
            deviceToggles->ForceSet(
                Toggle::D3D12ReplaceAddWithMinusWhenDstFactorIsZeroAndSrcFactorIsDstAlpha, true);

            // Unfortunately we cannot add workaround for other cases.
            deviceToggles->ForceSet(
                Toggle::NoWorkaroundDstAlphaAsSrcBlendFactorForBothColorAndAlphaDoesNotWork, true);
        }
    }

    // Currently these workarounds are only needed on Intel Gen9 and Gen11 GPUs.
    // See http://crbug.com/dawn/484 for more information.
    if (gpu_info::IsIntelGen9(vendorId, deviceId) || gpu_info::IsIntelGen11(vendorId, deviceId)) {
        deviceToggles->Default(Toggle::D3D12DontUseNotZeroedHeapFlagOnTexturesAsCommitedResources,
                               true);
    }

    if (!mDeviceInfo.supportsTextureCopyBetweenDimensions) {
        deviceToggles->ForceSet(
            Toggle::D3D12UseTempBufferInTextureToTextureCopyBetweenDifferentDimensions, true);
    }

    // Polyfill reflect builtin for vec2<f32> on Intel device if using FXC.
    // See https://crbug.com/tint/1798 for more information.
    if (gpu_info::IsIntel(vendorId) && !deviceToggles->IsEnabled(Toggle::UseDXC)) {
        deviceToggles->Default(Toggle::D3D12PolyfillReflectVec2F32, true);
    }

    // Currently this workaround is needed on old Intel drivers and newer version of Windows 11.
    // See http://crbug.com/dawn/2308 for more information.
    if (gpu_info::IsIntel(vendorId)) {
        constexpr uint64_t kAffectedMinimumWindowsBuildNumber = 25957u;
        const gpu_info::DriverVersion kAffectedMaximumDriverVersion = {27, 20, 100, 9664};
        if (GetBackend()->GetFunctions()->GetWindowsBuildNumber() >=
                kAffectedMinimumWindowsBuildNumber &&
            gpu_info::CompareWindowsDriverVersion(vendorId, GetDriverVersion(),
                                                  kAffectedMaximumDriverVersion) <= 0) {
            deviceToggles->Default(Toggle::DisableResourceSuballocation, true);
        }
    }

    if (gpu_info::IsNvidia(vendorId)) {
        deviceToggles->Default(Toggle::D3D12ForceStencilComponentReplicateSwizzle, true);
        deviceToggles->Default(Toggle::D3D12ExpandShaderResourceStateTransitionsToCopySource, true);
    }

    // Use the Tint IR backend by default if the corresponding platform feature is enabled.
    deviceToggles->Default(Toggle::UseTintIR,
                           platform->IsFeatureEnabled(platform::Features::kWebGPUUseTintIR));
}

ResultOrError<Ref<DeviceBase>> PhysicalDevice::CreateDeviceImpl(
    AdapterBase* adapter,
    const UnpackedPtr<DeviceDescriptor>& descriptor,
    const TogglesState& deviceToggles,
    Ref<DeviceBase::DeviceLostEvent>&& lostEvent) {
    return Device::Create(adapter, descriptor, deviceToggles, std::move(lostEvent));
}

// Resets the backend device and creates a new one. If any D3D12 objects belonging to the
// current ID3D12Device have not been destroyed, a non-zero value will be returned upon Reset()
// and the subsequent call to CreateDevice will return a handle the existing device instead of
// creating a new one.
MaybeError PhysicalDevice::ResetInternalDeviceForTestingImpl() {
    [[maybe_unused]] auto refCount = mD3d12Device.Reset();
    DAWN_ASSERT(refCount == 0);
    DAWN_TRY(Initialize());

    return {};
}

void PhysicalDevice::PopulateBackendProperties(UnpackedPtr<AdapterInfo>& info) const {
    if (auto* subgroupProperties = info.Get<AdapterPropertiesSubgroups>()) {
        subgroupProperties->subgroupMinSize = mDeviceInfo.waveLaneCountMin;
        // Currently the WaveLaneCountMax queried from D3D12 API is not reliable and the meaning is
        // unclear. Use 128 instead, which is the largest possible size. Reference:
        // https://github.com/Microsoft/DirectXShaderCompiler/wiki/Wave-Intrinsics#:~:text=UINT%20WaveLaneCountMax
        subgroupProperties->subgroupMaxSize = 128u;
    }
    if (auto* memoryHeapProperties = info.Get<AdapterPropertiesMemoryHeaps>()) {
        // https://microsoft.github.io/DirectX-Specs/d3d/D3D12GPUUploadHeaps.html describes
        // the properties of D3D12 Default/Upload/Readback heaps.
        if (mDeviceInfo.isUMA) {
            auto* heapInfo = new MemoryHeapInfo[1];
            memoryHeapProperties->heapCount = 1;
            memoryHeapProperties->heapInfo = heapInfo;

            heapInfo[0].size =
                std::max(mDeviceInfo.dedicatedVideoMemory, mDeviceInfo.sharedSystemMemory);

            if (mDeviceInfo.isCacheCoherentUMA) {
                heapInfo[0].properties =
                    wgpu::HeapProperty::DeviceLocal | wgpu::HeapProperty::HostVisible |
                    wgpu::HeapProperty::HostCoherent | wgpu::HeapProperty::HostCached;
            } else {
                heapInfo[0].properties =
                    wgpu::HeapProperty::DeviceLocal | wgpu::HeapProperty::HostVisible |
                    wgpu::HeapProperty::HostUncached | wgpu::HeapProperty::HostCached;
            }
        } else {
            auto* heapInfo = new MemoryHeapInfo[2];
            memoryHeapProperties->heapCount = 2;
            memoryHeapProperties->heapInfo = heapInfo;

            heapInfo[0].size = mDeviceInfo.dedicatedVideoMemory;
            heapInfo[0].properties = wgpu::HeapProperty::DeviceLocal;

            heapInfo[1].size = mDeviceInfo.sharedSystemMemory;
            heapInfo[1].properties =
                wgpu::HeapProperty::HostVisible | wgpu::HeapProperty::HostCoherent |
                wgpu::HeapProperty::HostUncached | wgpu::HeapProperty::HostCached;
        }
    }
    if (auto* d3dProperties = info.Get<AdapterPropertiesD3D>()) {
        // Report highest supported shader model version, instead of actual applied version.
        d3dProperties->shaderModel = GetDeviceInfo().highestSupportedShaderModel;
    }
}

}  // namespace dawn::native::d3d12
