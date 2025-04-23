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

#include "dawn/native/d3d11/PhysicalDeviceD3D11.h"

#include <algorithm>
#include <string>
#include <utility>

#include "dawn/common/Constants.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/BackendD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/PlatformFunctionsD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"
#include "dawn/platform/DawnPlatform.h"

namespace dawn::native::d3d11 {

PhysicalDevice::PhysicalDevice(Backend* backend,
                               ComPtr<IDXGIAdapter3> hardwareAdapter,
                               ComPtr<ID3D11Device> d3d11Device)
    : Base(backend, std::move(hardwareAdapter), wgpu::BackendType::D3D11),
      mIsSharedD3D11Device(!!d3d11Device),
      mD3D11Device(std::move(d3d11Device)) {}

PhysicalDevice::~PhysicalDevice() = default;

bool PhysicalDevice::SupportsExternalImages() const {
    return true;
}

bool PhysicalDevice::SupportsFeatureLevel(wgpu::FeatureLevel featureLevel,
                                          InstanceBase* instance) const {
    // TODO(dawn:1820): compare D3D11 feature levels with Dawn feature levels.
    switch (featureLevel) {
        case wgpu::FeatureLevel::Core: {
            return mFeatureLevel >= D3D_FEATURE_LEVEL_11_1;
        }
        case wgpu::FeatureLevel::Compatibility: {
            return mFeatureLevel >= D3D_FEATURE_LEVEL_11_0;
        }
        case wgpu::FeatureLevel::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

const DeviceInfo& PhysicalDevice::GetDeviceInfo() const {
    return mDeviceInfo;
}

ResultOrError<ComPtr<ID3D11Device>> PhysicalDevice::CreateD3D11Device(bool enableDebugLayer) {
    if (mIsSharedD3D11Device) {
        DAWN_ASSERT(mD3D11Device);
        return ComPtr<ID3D11Device>(mD3D11Device);
    }

    // If there mD3D11Device which is used for collecting GPU info is not null, try to use it.
    if (mD3D11Device) {
        // Backend validation level doesn't match, recreate the d3d11 device.
        if (enableDebugLayer == IsDebugLayerEnabled(mD3D11Device)) {
            return std::move(mD3D11Device);
        }
        mD3D11Device = nullptr;
    }

    const PlatformFunctions* functions = static_cast<Backend*>(GetBackend())->GetFunctions();
    const D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

    ComPtr<ID3D11Device> d3d11Device;

    if (enableDebugLayer) {
        // Try create d3d11 device with debug layer.
        HRESULT hr = functions->d3d11CreateDevice(
            GetHardwareAdapter(), D3D_DRIVER_TYPE_UNKNOWN,
            /*Software=*/nullptr, D3D11_CREATE_DEVICE_DEBUG, featureLevels,
            std::size(featureLevels), D3D11_SDK_VERSION, &d3d11Device,
            /*pFeatureLevel=*/nullptr, /*[out] ppImmediateContext=*/nullptr);

        if (SUCCEEDED(hr)) {
            DAWN_ASSERT(IsDebugLayerEnabled(d3d11Device));
            return d3d11Device;
        }
    }

    DAWN_TRY(CheckHRESULT(functions->d3d11CreateDevice(
                              GetHardwareAdapter(), D3D_DRIVER_TYPE_UNKNOWN,
                              /*Software=*/nullptr, /*Flags=*/0, featureLevels,
                              std::size(featureLevels), D3D11_SDK_VERSION, &d3d11Device,
                              /*pFeatureLevel=*/nullptr, /*[out] ppImmediateContext=*/nullptr),
                          "D3D11CreateDevice failed"));

    return d3d11Device;
}

MaybeError PhysicalDevice::InitializeImpl() {
    DAWN_TRY(Base::InitializeImpl());
    // D3D11 cannot check for feature support without a device.
    // Create the device to populate the adapter properties then reuse it when needed for actual
    // rendering.
    if (!mIsSharedD3D11Device) {
        DAWN_TRY_ASSIGN(mD3D11Device, CreateD3D11Device(/*enableDebugLayers=*/false));
    }

    mFeatureLevel = mD3D11Device->GetFeatureLevel();
    DAWN_TRY_ASSIGN(mDeviceInfo, GatherDeviceInfo(GetHardwareAdapter(), mD3D11Device));

    // Base::InitializeImpl() cannot distinguish between discrete and integrated GPUs, so we need to
    // overwrite it.
    if (mAdapterType == wgpu::AdapterType::DiscreteGPU && mDeviceInfo.isUMA) {
        mAdapterType = wgpu::AdapterType::IntegratedGPU;
    }

    return {};
}

void PhysicalDevice::InitializeSupportedFeaturesImpl() {
    EnableFeature(Feature::Depth32FloatStencil8);
    EnableFeature(Feature::DepthClipControl);
    EnableFeature(Feature::TextureCompressionBC);
    EnableFeature(Feature::D3D11MultithreadProtected);
    EnableFeature(Feature::Float32Filterable);
    EnableFeature(Feature::Float32Blendable);
    EnableFeature(Feature::DualSourceBlending);
    EnableFeature(Feature::ClipDistances);
    EnableFeature(Feature::Unorm16TextureFormats);
    EnableFeature(Feature::Snorm16TextureFormats);
    EnableFeature(Feature::Norm16TextureFormats);
    EnableFeature(Feature::AdapterPropertiesMemoryHeaps);
    EnableFeature(Feature::AdapterPropertiesD3D);
    EnableFeature(Feature::R8UnormStorage);
    EnableFeature(Feature::ShaderModuleCompilationOptions);
    EnableFeature(Feature::DawnLoadResolveTexture);
    EnableFeature(Feature::DawnPartialLoadResolveTexture);
    EnableFeature(Feature::RG11B10UfloatRenderable);
    if (mDeviceInfo.isUMA && mDeviceInfo.supportsMapNoOverwriteDynamicBuffers) {
        // With UMA we should allow mapping usages on more type of buffers.
        EnableFeature(Feature::BufferMapExtendedUsages);
    }

    // Multi planar formats are always supported since Feature Level 11.0
    // https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/format-support-for-direct3d-11-0-feature-level-hardware
    EnableFeature(Feature::DawnMultiPlanarFormats);
    EnableFeature(Feature::MultiPlanarFormatP010);
    EnableFeature(Feature::MultiPlanarRenderTargets);

    if (mDeviceInfo.supportsROV) {
        EnableFeature(Feature::PixelLocalStorageCoherent);
    }

    EnableFeature(Feature::SharedTextureMemoryD3D11Texture2D);
    EnableFeature(Feature::SharedTextureMemoryDXGISharedHandle);

    if (mDeviceInfo.supportsMonitoredFence || mDeviceInfo.supportsNonMonitoredFence) {
        EnableFeature(Feature::SharedFenceDXGISharedHandle);
    }

    UINT formatSupport = 0;
    HRESULT hr = mD3D11Device->CheckFormatSupport(DXGI_FORMAT_B8G8R8A8_UNORM, &formatSupport);
    DAWN_ASSERT(SUCCEEDED(hr));
    if (formatSupport & D3D11_FORMAT_SUPPORT_TYPED_UNORDERED_ACCESS_VIEW) {
        EnableFeature(Feature::BGRA8UnormStorage);
    }

    EnableFeature(Feature::DawnTexelCopyBufferRowAlignment);
    EnableFeature(Feature::FlexibleTextureViews);
}

MaybeError PhysicalDevice::InitializeSupportedLimitsImpl(CombinedLimits* limits) {
    GetDefaultLimitsForSupportedFeatureLevel(&limits->v1);

    // // https://docs.microsoft.com/en-us/windows/win32/direct3d12/hardware-feature-levels

    // Limits that are the same across D3D feature levels
    limits->v1.maxTextureDimension1D = D3D11_REQ_TEXTURE1D_U_DIMENSION;
    limits->v1.maxTextureDimension2D = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    limits->v1.maxTextureDimension3D = D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
    limits->v1.maxTextureArrayLayers = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
    // Slot values can be 0-15, inclusive:
    // https://docs.microsoft.com/en-ca/windows/win32/api/d3d11/ns-d3d11-d3d11_input_element_desc
    limits->v1.maxVertexBuffers = 16;
    // Both SV_VertexID and SV_InstanceID will consume vertex input slots.
    limits->v1.maxVertexAttributes = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - 2;

    uint32_t maxUAVsAllStages;
    uint32_t maxUAVsPerStage;

    if (mFeatureLevel >= D3D_FEATURE_LEVEL_11_1) {
        // In D3D 11.1, max UAV slots are shared between fragment & vertex stage so divide it by 2
        // to get per stage limit.
        maxUAVsAllStages = D3D11_1_UAV_SLOT_COUNT;
        maxUAVsPerStage = maxUAVsAllStages / 2;
    } else {
        // We don't support feature level < 11.0
        DAWN_INVALID_IF(mFeatureLevel < D3D_FEATURE_LEVEL_11_0, "Unsupported D3D feature level %u",
                        mFeatureLevel);
        // In D3D 11.0, only fragment and compute have UAVs. Vertex doesn't have UAV so we don't
        // need to divide the slot count between fragment & vertex.
        maxUAVsAllStages = D3D11_PS_CS_UAV_REGISTER_COUNT;
        maxUAVsPerStage = maxUAVsAllStages;
    }
    mUAVSlotCount = maxUAVsAllStages;

    // Reserve one slot for builtin constants.
    constexpr uint32_t kReservedCBVSlots = 1;
    limits->v1.maxUniformBuffersPerShaderStage =
        D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - kReservedCBVSlots;

    // Allocate half of the UAVs to storage buffers, and half to storage textures.
    limits->v1.maxStorageTexturesPerShaderStage = maxUAVsPerStage / 2;
    limits->v1.maxStorageBuffersPerShaderStage = maxUAVsPerStage / 2;
    limits->v1.maxStorageTexturesInFragmentStage = limits->v1.maxStorageTexturesPerShaderStage;
    limits->v1.maxStorageBuffersInFragmentStage = limits->v1.maxStorageBuffersPerShaderStage;
    // If the device only has feature level 11.0, technically, vertex stage doesn't have any UAV
    // slot (writable storage buffers). However, since Dawn spec requires that storage buffers must
    // be readonly in VS, it's safe to advertise that we have storage buffers in VS. Readonly
    // storage buffers will use SRV slots which are available in all stages.
    // The same for read-only storage textures in VS.
    limits->v1.maxStorageTexturesInVertexStage = limits->v1.maxStorageTexturesPerShaderStage;
    limits->v1.maxStorageBuffersInVertexStage = limits->v1.maxStorageBuffersPerShaderStage;
    limits->v1.maxSampledTexturesPerShaderStage = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
    limits->v1.maxSamplersPerShaderStage = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
    limits->v1.maxColorAttachments = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
    // This is maxColorAttachments times 16, the color format with the largest cost.
    limits->v1.maxColorAttachmentBytesPerSample = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT * 16;

    limits->v1.maxDynamicUniformBuffersPerPipelineLayout =
        limits->v1.maxUniformBuffersPerShaderStage;
    limits->v1.maxDynamicStorageBuffersPerPipelineLayout =
        limits->v1.maxStorageBuffersPerShaderStage;

    // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-attributes-numthreads
    limits->v1.maxComputeWorkgroupSizeX = D3D11_CS_THREAD_GROUP_MAX_X;
    limits->v1.maxComputeWorkgroupSizeY = D3D11_CS_THREAD_GROUP_MAX_Y;
    limits->v1.maxComputeWorkgroupSizeZ = D3D11_CS_THREAD_GROUP_MAX_Z;
    limits->v1.maxComputeInvocationsPerWorkgroup = D3D11_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;

    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-dispatch
    limits->v1.maxComputeWorkgroupsPerDimension = D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;

    // https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-downlevel-compute-shaders
    // Thread Group Shared Memory is limited to 16Kb on downlevel hardware. This is less than
    // the 32Kb that is available to Direct3D 11 hardware. D3D12 is also 32kb.
    limits->v1.maxComputeWorkgroupStorageSize = 32768;

    // Max number of "constants" where each constant is a 16-byte float4
    limits->v1.maxUniformBufferBindingSize = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;

    if (gpu_info::IsQualcomm_ACPI(GetVendorId())) {
        // limit of number of texels in a buffer == (1 << 27)
        // D3D11_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP
        // This limit doesn't apply to a raw buffer, but only applies to
        // typed, or structured buffer. so this could be a QC driver bug.
        limits->v1.maxStorageBufferBindingSize = uint64_t(1)
                                                 << D3D11_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP;
    } else {
        limits->v1.maxStorageBufferBindingSize = kAssumedMaxBufferSize;
    }

    // D3D11 has no documented limit on the buffer size.
    limits->v1.maxBufferSize = kAssumedMaxBufferSize;

    // 1 for SV_Position and 1 for (SV_IsFrontFace OR SV_SampleIndex).
    // See the discussions in https://github.com/gpuweb/gpuweb/issues/1962 for more details.
    limits->v1.maxInterStageShaderVariables = D3D11_PS_INPUT_REGISTER_COUNT - 2;

    // The BlitTextureToBuffer helper requires the alignment to be 4.
    limits->texelCopyBufferRowAlignmentLimits.minTexelCopyBufferRowAlignment = 4;

    return {};
}

FeatureValidationResult PhysicalDevice::ValidateFeatureSupportedWithTogglesImpl(
    wgpu::FeatureName feature,
    const TogglesState& toggles) const {
    return {};
}

void PhysicalDevice::SetupBackendAdapterToggles(dawn::platform::Platform* platform,
                                                TogglesState* adapterToggles) const {
    // D3D11 must use FXC, not DXC.
    adapterToggles->ForceSet(Toggle::UseDXC, false);
}

void PhysicalDevice::SetupBackendDeviceToggles(dawn::platform::Platform* platform,
                                               TogglesState* deviceToggles) const {
    // D3D11 can only clear RTV with float values.
    deviceToggles->Default(Toggle::ApplyClearBigIntegerColorValueWithDraw, true);
    deviceToggles->Default(Toggle::UseBlitForBufferToStencilTextureCopy, true);
    if (!mDeviceInfo.supportsMonitoredFence) {
        deviceToggles->Default(Toggle::D3D11UseUnmonitoredFence,
                               mDeviceInfo.supportsNonMonitoredFence);
        deviceToggles->ForceSet(Toggle::D3D11DisableFence, !mDeviceInfo.supportsNonMonitoredFence);
    }
    deviceToggles->Default(Toggle::UseBlitForT2B, true);
    deviceToggles->Default(Toggle::UseBlitForB2T, true);

    auto deviceId = GetDeviceId();
    auto vendorId = GetVendorId();
    // D3D11 ClearRenderTargetView() could be buggy with some old driver or GPUs. Intel Gen12+ GPUs
    // don't have the problem.
    // https://crbug.com/329702368
    //
    // The workaround still can't cover lazy clear,
    // TODO(crbug.com/364834368): Move handling of workaround at command submission time instead of
    // recording time.
    if (gpu_info::IsIntelGen11OrOlder(vendorId, deviceId)) {
        deviceToggles->Default(Toggle::ClearColorWithDraw, true);
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

// Resets the backend device and creates a new one. If any D3D11 objects belonging to the
// current ID3D11Device have not been destroyed, a non-zero value will be returned upon Reset()
// and the subequent call to CreateDevice will return a handle the existing device instead of
// creating a new one.
MaybeError PhysicalDevice::ResetInternalDeviceForTestingImpl() {
    [[maybe_unused]] auto refCount = mD3D11Device.Reset();
    DAWN_ASSERT(refCount == 0);
    DAWN_TRY(Initialize());

    return {};
}

void PhysicalDevice::PopulateBackendProperties(UnpackedPtr<AdapterInfo>& info) const {
    if (auto* memoryHeapProperties = info.Get<AdapterPropertiesMemoryHeaps>()) {
        // https://microsoft.github.io/DirectX-Specs/d3d/D3D12GPUUploadHeaps.html describes
        // the properties of D3D12 Default/Upload/Readback heaps. The assumption is that these are
        // roughly how D3D11 allocates memory has well.
        if (mDeviceInfo.isUMA) {
            auto* heapInfo = new MemoryHeapInfo[1];
            memoryHeapProperties->heapCount = 1;
            memoryHeapProperties->heapInfo = heapInfo;

            heapInfo[0].size =
                std::max(mDeviceInfo.dedicatedVideoMemory, mDeviceInfo.sharedSystemMemory);
            heapInfo[0].properties =
                wgpu::HeapProperty::DeviceLocal | wgpu::HeapProperty::HostVisible |
                wgpu::HeapProperty::HostUncached | wgpu::HeapProperty::HostCached;
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
        d3dProperties->shaderModel = GetDeviceInfo().shaderModel;
    }
}

}  // namespace dawn::native::d3d11
