// Copyright 2024 The Dawn & Tint Authors
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

#include "dawn/native/metal/PhysicalDeviceMTL.h"

#include "dawn/common/CoreFoundationRef.h"
#include "dawn/common/GPUInfo.h"
#include "dawn/common/Log.h"
#include "dawn/common/NSRef.h"
#include "dawn/common/Platform.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/MetalBackend.h"
#include "dawn/native/metal/BufferMTL.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/UtilsMetal.h"
#include "dawn/platform/DawnPlatform.h"

#if DAWN_PLATFORM_IS(MACOS)
#import <IOKit/IOKitLib.h>
#include "dawn/common/IOKitRef.h"
#endif

#include <string>
#include <vector>

namespace dawn::native::metal {

namespace {

struct PCIIDs {
    uint32_t vendorId;
    uint32_t deviceId;
};

struct Vendor {
    const char* trademark;
    uint32_t vendorId;
};

#if DAWN_PLATFORM_IS(MACOS)
const Vendor kVendors[] = {
    {"AMD", gpu_info::kVendorID_AMD},        {"Apple", gpu_info::kVendorID_Apple},
    {"Radeon", gpu_info::kVendorID_AMD},     {"Intel", gpu_info::kVendorID_Intel},
    {"Geforce", gpu_info::kVendorID_Nvidia}, {"Quadro", gpu_info::kVendorID_Nvidia}};

// Find vendor ID from MTLDevice name.
MaybeError GetVendorIdFromVendors(id<MTLDevice> device, PCIIDs* ids) {
    uint32_t vendorId = 0;
    const char* deviceName = [device.name UTF8String];
    for (const auto& it : kVendors) {
        if (strstr(deviceName, it.trademark) != nullptr) {
            vendorId = it.vendorId;
            break;
        }
    }

    if (vendorId == 0) {
        return DAWN_INTERNAL_ERROR("Failed to find vendor id with the device");
    }

    // Set vendor id with 0
    *ids = PCIIDs{vendorId, 0};
    return {};
}

// Extracts an integer property from a registry entry.
uint32_t GetEntryProperty(io_registry_entry_t entry, CFStringRef name) {
    uint32_t value = 0;

    // Recursively search registry entry and its parents for property name
    // The data should release with CFRelease
    CFRef<CFDataRef> data = AcquireCFRef(static_cast<CFDataRef>(IORegistryEntrySearchCFProperty(
        entry, kIOServicePlane, name, kCFAllocatorDefault,
        kIORegistryIterateRecursively | kIORegistryIterateParents)));

    if (data == nullptr) {
        return value;
    }

    // CFDataGetBytePtr() is guaranteed to return a read-only pointer
    value = *reinterpret_cast<const uint32_t*>(CFDataGetBytePtr(data.Get()));
    return value;
}

// Queries the IO Registry to find the PCI device and vendor IDs of the MTLDevice.
// The registry entry correponding to [device registryID] doesn't contain the exact PCI ids
// because it corresponds to a driver. However its parent entry corresponds to the device
// itself and has uint32_t "device-id" and "registry-id" keys. For example on a dual-GPU
// MacBook Pro 2017 the IORegistry explorer shows the following tree (simplified here):
//
//  - PCI0@0
//  | - AppleACPIPCI
//  | | - IGPU@2 (type IOPCIDevice)
//  | | | - IntelAccelerator (type IOGraphicsAccelerator2)
//  | | - PEG0@1
//  | | | - IOPP
//  | | | | - GFX0@0 (type IOPCIDevice)
//  | | | | | - AMDRadeonX4000_AMDBaffinGraphicsAccelerator (type IOGraphicsAccelerator2)
//
// [device registryID] is the ID for one of the IOGraphicsAccelerator2 and we can see that
// their parent always is an IOPCIDevice that has properties for the device and vendor IDs.
MaybeError GetDeviceIORegistryPCIInfo(id<MTLDevice> device, PCIIDs* ids) {
    // Get a matching dictionary for the IOGraphicsAccelerator2
    CFRef<CFMutableDictionaryRef> matchingDict =
        AcquireCFRef(IORegistryEntryIDMatching([device registryID]));
    if (matchingDict == nullptr) {
        return DAWN_INTERNAL_ERROR("Failed to create the matching dict for the device");
    }

    // Work around a breaking deprecation of kIOMasterPortDefault to kIOMainPortDefault. Both values
    // are equivalent with NULL (given mach_port_t is an unsigned int they probably mean 0) as noted
    // by the IOKitLib.h comments so use that directly.
    // TODO(chromium:1400252): Use kIOMainPortDefault once the minimum supported version includes
    // macOS 12.0
    constexpr mach_port_t kIOMainPort = 0;

    // IOServiceGetMatchingService will consume the reference on the matching dictionary,
    // so we don't need to release the dictionary.
    IORef<io_registry_entry_t> acceleratorEntry =
        AcquireIORef(IOServiceGetMatchingService(kIOMainPort, matchingDict.Detach()));

    if (acceleratorEntry == IO_OBJECT_NULL) {
        return DAWN_INTERNAL_ERROR("Failed to get the IO registry entry for the accelerator");
    }

    // Get the parent entry that will be the IOPCIDevice
    IORef<io_registry_entry_t> deviceEntry;
    if (IORegistryEntryGetParentEntry(acceleratorEntry.Get(), kIOServicePlane,
                                      deviceEntry.InitializeInto()) != kIOReturnSuccess) {
        return DAWN_INTERNAL_ERROR("Failed to get the IO registry entry for the device");
    }

    DAWN_ASSERT(deviceEntry != IO_OBJECT_NULL);

    uint32_t vendorId = GetEntryProperty(deviceEntry.Get(), CFSTR("vendor-id"));
    uint32_t deviceId = GetEntryProperty(deviceEntry.Get(), CFSTR("device-id"));

    *ids = PCIIDs{vendorId, deviceId};

    return {};
}

MaybeError GetDevicePCIInfo(id<MTLDevice> device, PCIIDs* ids) {
    auto result = GetDeviceIORegistryPCIInfo(device, ids);
    if (result.IsError()) {
        dawn::WarningLog() << "GetDeviceIORegistryPCIInfo failed: "
                           << result.AcquireError()->GetFormattedMessage();
    } else if (ids->vendorId != 0) {
        return result;
    }

    return GetVendorIdFromVendors(device, ids);
}

#elif DAWN_PLATFORM_IS(IOS)

MaybeError GetDevicePCIInfo(id<MTLDevice>, PCIIDs* ids) {
    *ids = PCIIDs{0, 0};
    return {};
}

#else
#error "Unsupported Apple platform."
#endif

bool IsGPUCounterSupported(id<MTLDevice> device,
                           MTLCommonCounterSet counterSetName,
                           std::vector<NSString*> counterNames) {
    id<MTLCounterSet> counterSet = nil;
    for (id<MTLCounterSet> set in [device counterSets]) {
        if ([set.name caseInsensitiveCompare:counterSetName] == NSOrderedSame) {
            counterSet = set;
            break;
        }
    }

    // The counter set is not supported.
    if (counterSet == nil) {
        return false;
    }

    NSArray<id<MTLCounter>>* countersInSet = [counterSet counters];
    // A GPU might support a counter set, but only support a subset of the counters in that
    // set, check if the counter set supports all specific counters we need. Return false
    // if there is a counter unsupported.
    for (MTLCommonCounter counterName : counterNames) {
        bool found = false;
        for (id<MTLCounter> counter in countersInSet) {
            if ([counter.name caseInsensitiveCompare:counterName] == NSOrderedSame) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }

    // Check whether it can read GPU counters at the specified command boundary or stage
    // boundary. Apple family GPUs do not support sampling between different Metal commands,
    // because they defer fragment processing until after the GPU processes all the primitives
    // in the render pass. GPU counters are only available if sampling at least one of the
    // command or stage boundaries is supported.
    if (!SupportCounterSamplingAtCommandBoundary(device) &&
        !SupportCounterSamplingAtStageBoundary(device)) {
        return false;
    }

    return true;
}

enum class MTLGPUFamily {
    Apple1,
    Apple2,
    Apple3,
    Apple4,
    Apple5,
    Apple6,
    Apple7,
    Mac1,
    Mac2,
};

ResultOrError<MTLGPUFamily> GetMTLGPUFamily(id<MTLDevice> device) {
    // https://developer.apple.com/documentation/metal/mtldevice/detecting_gpu_features_and_metal_software_versions?language=objc

#if !DAWN_PLATFORM_IS(IOS)
    if ([device supportsFamily:MTLGPUFamilyMac2]) {
        return MTLGPUFamily::Mac2;
    }
#endif
    if ([device supportsFamily:MTLGPUFamilyApple7]) {
        return MTLGPUFamily::Apple7;
    }
    if ([device supportsFamily:MTLGPUFamilyApple6]) {
        return MTLGPUFamily::Apple6;
    }
    if ([device supportsFamily:MTLGPUFamilyApple5]) {
        return MTLGPUFamily::Apple5;
    }
    if ([device supportsFamily:MTLGPUFamilyApple4]) {
        return MTLGPUFamily::Apple4;
    }
    if ([device supportsFamily:MTLGPUFamilyApple3]) {
        return MTLGPUFamily::Apple3;
    }
    if ([device supportsFamily:MTLGPUFamilyApple2]) {
        return MTLGPUFamily::Apple2;
    }
    if ([device supportsFamily:MTLGPUFamilyApple1]) {
        return MTLGPUFamily::Apple1;
    }

    // This family is no longer supported in the macOS 10.15 SDK but still exists so
    // default to it.
    return MTLGPUFamily::Mac1;
}

}  // anonymous namespace

PhysicalDevice::PhysicalDevice(InstanceBase* instance,
                               NSPRef<id<MTLDevice>> device,
                               bool metalValidationEnabled)
    : PhysicalDeviceBase(wgpu::BackendType::Metal),
      mDevice(std::move(device)),
      mMetalValidationEnabled(metalValidationEnabled) {
    mName = std::string([[*mDevice name] UTF8String]);

    PCIIDs ids;
    if (!instance->ConsumedError(GetDevicePCIInfo(*mDevice, &ids))) {
        mVendorId = ids.vendorId;
        mDeviceId = ids.deviceId;
    }

#if DAWN_PLATFORM_IS(IOS)
    mAdapterType = wgpu::AdapterType::IntegratedGPU;
    const char* systemName = "iOS ";
#elif DAWN_PLATFORM_IS(MACOS)
    if ([*mDevice hasUnifiedMemory]) {
        mAdapterType = wgpu::AdapterType::IntegratedGPU;
    } else {
        mAdapterType = wgpu::AdapterType::DiscreteGPU;
    }
    const char* systemName = "macOS ";
#else
#error "Unsupported Apple platform."
#endif

    NSString* osVersion = [[NSProcessInfo processInfo] operatingSystemVersionString];
    mDriverDescription = "Metal driver on " + std::string(systemName) + [osVersion UTF8String];

    mSubgroupMinSize = 4;   // The 4 comes from the minimum derivative group.
    mSubgroupMaxSize = 64;  // In MSL, a ballot is a uint64, so the max subgroup size is 64.
}

bool PhysicalDevice::IsMetalValidationEnabled() const {
    return mMetalValidationEnabled;
}

// PhysicalDeviceBase Implementation
bool PhysicalDevice::SupportsExternalImages() const {
    // SharedTextureMemory is the supported means of importing IOSurfaces.
    return false;
}

bool PhysicalDevice::SupportsFeatureLevel(wgpu::FeatureLevel, InstanceBase* instance) const {
    return true;
}

ResultOrError<PhysicalDeviceSurfaceCapabilities> PhysicalDevice::GetSurfaceCapabilities(
    InstanceBase* instance,
    const Surface*) const {
    PhysicalDeviceSurfaceCapabilities capabilities;

    capabilities.usages = wgpu::TextureUsage::RenderAttachment |
                          wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc |
                          wgpu::TextureUsage::CopyDst;

    capabilities.formats = {
        wgpu::TextureFormat::BGRA8Unorm,
        wgpu::TextureFormat::BGRA8UnormSrgb,
        wgpu::TextureFormat::RGBA16Float,
    };
#if DAWN_PLATFORM_IS(MACOS)
    capabilities.formats.push_back(wgpu::TextureFormat::RGB10A2Unorm);
#endif  // DAWN_PLATFORM_IS(MACOS)

    capabilities.presentModes = {
        wgpu::PresentMode::Fifo,
        wgpu::PresentMode::Immediate,
        wgpu::PresentMode::Mailbox,
    };

    capabilities.alphaModes = {
        wgpu::CompositeAlphaMode::Opaque,
        wgpu::CompositeAlphaMode::Premultiplied,
    };

    return capabilities;
}

ResultOrError<Ref<DeviceBase>> PhysicalDevice::CreateDeviceImpl(
    AdapterBase* adapter,
    const UnpackedPtr<DeviceDescriptor>& descriptor,
    const TogglesState& deviceToggles,
    Ref<DeviceBase::DeviceLostEvent>&& lostEvent) {
    return Device::Create(adapter, mDevice, descriptor, deviceToggles, std::move(lostEvent));
}

void PhysicalDevice::SetupBackendAdapterToggles(dawn::platform::Platform* platform,
                                                TogglesState* adapterToggles) const {}

void PhysicalDevice::SetupBackendDeviceToggles(dawn::platform::Platform* platform,
                                               TogglesState* deviceToggles) const {
    {
        bool haveStoreAndMSAAResolve = false;
#if DAWN_PLATFORM_IS(MACOS)
        haveStoreAndMSAAResolve = [*mDevice supportsFamily:MTLGPUFamilyCommon2];
#elif DAWN_PLATFORM_IS(IOS) && !DAWN_PLATFORM_IS(TVOS)
#if !defined(__IPHONE_16_0) || __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_16_0
        haveStoreAndMSAAResolve = [*mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v2];
#else
        // iOS 16 is A11 Bionic and later.
        haveStoreAndMSAAResolve = true;
#endif
#endif
        // On tvOS, we would need MTLFeatureSet_tvOS_GPUFamily2_v1.
        deviceToggles->Default(Toggle::EmulateStoreAndMSAAResolve, !haveStoreAndMSAAResolve);

        bool haveSamplerCompare = true;
#if DAWN_PLATFORM_IS(IOS) && !DAWN_PLATFORM_IS(TVOS) && \
    (!defined(__IPHONE_16_0) || __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_16_0)
        haveSamplerCompare = [*mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1];
#endif
        // TODO(crbug.com/dawn/342): Investigate emulation -- possibly expensive.
        deviceToggles->Default(Toggle::MetalDisableSamplerCompare, !haveSamplerCompare);

        bool haveBaseVertexBaseInstance = true;
#if DAWN_PLATFORM_IS(IOS) && !DAWN_PLATFORM_IS(TVOS) && \
    (!defined(__IPHONE_16_0) || __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_16_0)
        haveBaseVertexBaseInstance = [*mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1];
#endif
        // TODO(crbug.com/dawn/343): Investigate emulation.
        deviceToggles->Default(Toggle::DisableBaseVertex, !haveBaseVertexBaseInstance);
        deviceToggles->Default(Toggle::DisableBaseInstance, !haveBaseVertexBaseInstance);
    }

    // Vertex buffer robustness is implemented by using programmable vertex pulling. Enable
    // that code path if it isn't explicitly disabled.
    if (!deviceToggles->IsEnabled(Toggle::DisableRobustness)) {
        deviceToggles->Default(Toggle::MetalEnableVertexPulling, true);
    }

    // Shader `discard_fragment` changed semantics to be uniform in Metal 2.3+. See section 6.10.1.3
    // of the Metal Spec (v3.2).
    // TODO(crbug/390426577): Consider removing this toggle as it is no longer needs to be toggled
    // as it is (semantically correctly) supported for all Metal 2.3+.
    deviceToggles->Default(Toggle::DisableDemoteToHelper, true);

    // TODO(crbug.com/dawn/846): tighten this workaround when the driver bug is fixed.
    deviceToggles->Default(Toggle::AlwaysResolveIntoZeroLevelAndLayer, true);

    uint32_t deviceId = GetDeviceId();
    uint32_t vendorId = GetVendorId();

    // TODO(crbug.com/dawn/847): Use MTLStorageModeShared instead of MTLStorageModePrivate when
    // creating MTLCounterSampleBuffer in QuerySet on Intel platforms, otherwise it fails to
    // create the buffer. Change to use MTLStorageModePrivate when the bug is fixed.
    bool useSharedMode = gpu_info::IsIntel(vendorId);
    deviceToggles->Default(Toggle::MetalUseSharedModeForCounterSampleBuffer, useSharedMode);

    // Rendering R8Unorm and RG8Unorm to small mip doesn't work properly on Intel.
    // TODO(crbug.com/dawn/1071): Tighten the workaround when this issue is fixed.
    if (gpu_info::IsIntel(vendorId)) {
        deviceToggles->Default(Toggle::MetalRenderR8RG8UnormSmallMipToTempTexture, true);
    }

    // On some Intel GPUs vertex only render pipeline get wrong depth result if no fragment
    // shader provided. Create a placeholder fragment shader module to work around this issue.
    if (gpu_info::IsIntel(vendorId)) {
        bool usePlaceholderFragmentShader = true;
        if (gpu_info::IsSkylake(deviceId)) {
            usePlaceholderFragmentShader = false;
        }
        deviceToggles->Default(Toggle::UsePlaceholderFragmentInVertexOnlyPipeline,
                               usePlaceholderFragmentShader);
    }

    // On some Intel GPUs using big integer values as clear values in render pass doesn't work
    // correctly. Currently we have to add workaround for this issue by enabling the toggle
    // "apply_clear_big_integer_color_value_with_draw". See https://crbug.com/dawn/1109 and
    // https://crbug.com/dawn/1463 for more details.
    if (gpu_info::IsIntel(vendorId)) {
        deviceToggles->Default(Toggle::ApplyClearBigIntegerColorValueWithDraw, true);
    }

    // TODO(dawn:1473): Metal fails to store GPU counters to sampleBufferAttachments on empty
    // encoders on macOS 11.0+, we need to add mock blit command to blit encoder when encoding
    // writeTimestamp as workaround by enabling the toggle
    // "metal_use_mock_blit_encoder_for_write_timestamp".
        deviceToggles->Default(Toggle::MetalUseMockBlitEncoderForWriteTimestamp, true);

    // On macOS 15.0+, we can use sampleTimestamps:gpuTimestamp: from MTLDevice to capture CPU and
    // GPU timestamps to estimate GPU timestamp period at device creation, but this API call will
    // cause GPU overheating on Intel GPUs due to a driver bug keeping the GPU running at the
    // maximum clock. Disable timestamp sampling to avoid overheating user's devices.
    // See https://crbug.com/342701242 for more details.
        if (@available(macos 15.0, *)) {
            if (gpu_info::IsIntel(deviceId)) {
                deviceToggles->Default(Toggle::MetalDisableTimestampPeriodEstimation, true);
            }
        }

#if DAWN_PLATFORM_IS(MACOS)
    if (gpu_info::IsIntel(vendorId)) {
        deviceToggles->Default(
            Toggle::MetalUseBothDepthAndStencilAttachmentsForCombinedDepthStencilFormats, true);
        deviceToggles->Default(Toggle::UseBlitForBufferToStencilTextureCopy, true);
        deviceToggles->Default(Toggle::UseBlitForBufferToDepthTextureCopy, true);
        deviceToggles->Default(Toggle::UseBlitForDepthTextureToTextureCopyToNonzeroSubresource,
                               true);

        if ([NSProcessInfo.processInfo
                isOperatingSystemAtLeastVersion:NSOperatingSystemVersion{12, 0, 0}]) {
            deviceToggles->ForceSet(
                Toggle::NoWorkaroundSampleMaskBecomesZeroForAllButLastColorTarget, true);
        }
        if (gpu_info::IsIntelGen7(vendorId, deviceId) ||
            gpu_info::IsIntelGen8(vendorId, deviceId)) {
            deviceToggles->ForceSet(Toggle::NoWorkaroundIndirectBaseVertexNotApplied, true);
        }
    }
    if (gpu_info::IsAMD(vendorId) || gpu_info::IsIntel(vendorId)) {
        deviceToggles->Default(Toggle::MetalUseCombinedDepthStencilFormatForStencil8, true);
        deviceToggles->Default(Toggle::MetalKeepMultisubresourceDepthStencilTexturesInitialized,
                               true);
    }

    if (gpu_info::IsApple(vendorId)) {
        deviceToggles->Default(Toggle::MetalFillEmptyOcclusionQueriesWithZero, true);

        // TODO(crbug.com/372698905): Tighten the workaround when a fixed macOS version releases.

        // TODO(crbug.com/380316939): Replace the cast with MTLGPUFamilyApple8 when available.
        if ([*mDevice supportsFamily:static_cast<::MTLGPUFamily>(1008)]) {
            deviceToggles->Default(Toggle::MetalSerializeTimestampGenerationAndResolution, true);
        }
    }

    // Local testing shows the workaround is needed on AMD Radeon HD 8870M (gcn-1) MacOS 12.1;
    // not on AMD Radeon Pro 555 (gcn-4) MacOS 13.1.
    // Conservatively enable the workaround on AMD unless the system is MacOS 13.1+
    // with architecture at least AMD gcn-4.
    bool isLessThanAMDGN4OrMac13Dot1 = false;
    if (gpu_info::IsAMDGCN1(vendorId, deviceId) || gpu_info::IsAMDGCN2(vendorId, deviceId) ||
        gpu_info::IsAMDGCN3(vendorId, deviceId)) {
        isLessThanAMDGN4OrMac13Dot1 = true;
    } else if (gpu_info::IsAMD(vendorId)) {
        if (@available(macos 13.1, *)) {
        } else {
            isLessThanAMDGN4OrMac13Dot1 = true;
        }
    }
    if (isLessThanAMDGN4OrMac13Dot1) {
        deviceToggles->Default(
            Toggle::MetalUseBothDepthAndStencilAttachmentsForCombinedDepthStencilFormats, true);
    }
#endif
}

MaybeError PhysicalDevice::InitializeImpl() {
    return {};
}

void PhysicalDevice::InitializeSupportedFeaturesImpl() {
#if (defined(__MAC_11_0) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0) || \
    (defined(__IPHONE_14_0) && __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_14_0)
    if ([*mDevice supports32BitFloatFiltering]) {
        EnableFeature(Feature::Float32Filterable);
    }
#elif DAWN_PLATFORM_IS(MACOS)
    if ([*mDevice supportsFamily:MTLGPUFamilyMac2]) {
        EnableFeature(Feature::Float32Filterable);
    }
#endif

#if (defined(__MAC_11_0) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0) || \
    (defined(__IPHONE_16_4) && __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_16_4)
    if ([*mDevice supportsBCTextureCompression]) {
        EnableFeature(Feature::TextureCompressionBC);
    }
#elif DAWN_PLATFORM_IS(MACOS)
    EnableFeature(Feature::TextureCompressionBC);
#endif

#if DAWN_PLATFORM_IS(IOS) && !DAWN_PLATFORM_IS(TVOS) && \
    (!defined(__IPHONE_16_0) || __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_16_0)
    if ([*mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v1]) {
        EnableFeature(Feature::TextureCompressionETC2);
    }
    if ([*mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1]) {
        EnableFeature(Feature::TextureCompressionASTC);
    }
#endif

    // Check texture formats with MTLGPUFamily

    if ([*mDevice supportsFamily:MTLGPUFamilyApple2]) {
        EnableFeature(Feature::TextureCompressionETC2);
    }
    if ([*mDevice supportsFamily:MTLGPUFamilyApple3]) {
        EnableFeature(Feature::TextureCompressionASTC);
    }

    auto ShouldLeakCounterSets = [this] {
        // Intentionally leak counterSets to workaround an issue where the driver
        // over-releases the handle if it is accessed more than once. It becomes a zombie.
        // For more information, see crbug.com/1443658.
        // Appears to occur on non-Apple prior to MacOS 11, and continuing on Intel Gen 7,
        // Intel Gen 8, and Intel Gen 11 after that OS version.
        uint32_t vendorId = GetVendorId();
        uint32_t deviceId = GetDeviceId();
        if (gpu_info::IsIntelGen7(vendorId, deviceId) ||
            gpu_info::IsIntelGen8(vendorId, deviceId) ||
            gpu_info::IsIntelGen11(vendorId, deviceId)) {
            return true;
        }

        return false;
    };
    if (ShouldLeakCounterSets()) {
        [[*mDevice counterSets] retain];
    }

    if (IsGPUCounterSupported(*mDevice, MTLCommonCounterSetTimestamp,
                              {MTLCommonCounterTimestamp})) {
        EnableFeature(Feature::TimestampQuery);
        if (SupportCounterSamplingAtCommandBoundary(*mDevice)) {
            EnableFeature(Feature::ChromiumExperimentalTimestampQueryInsidePasses);
        }
    }

    EnableFeature(Feature::DepthClipControl);

    EnableFeature(Feature::Depth32FloatStencil8);

// TODO(dawn:2249): Enable on iOS. Some XCode or SDK versions seem to not match the docs.
#if DAWN_PLATFORM_IS(MACOS)

    EnableFeature(Feature::AdapterPropertiesMemoryHeaps);

#endif

    EnableFeature(Feature::DawnMultiPlanarFormats);
    EnableFeature(Feature::MultiPlanarFormatP010);
    EnableFeature(Feature::MultiPlanarRenderTargets);
    EnableFeature(Feature::MultiPlanarFormatExtendedUsages);

    EnableFeature(Feature::MultiPlanarFormatP210);
    EnableFeature(Feature::MultiPlanarFormatP410);

    EnableFeature(Feature::MultiPlanarFormatNv12a);

    EnableFeature(Feature::MultiPlanarFormatNv16);
    EnableFeature(Feature::MultiPlanarFormatNv24);

    // Memoryless storage mode and programmable blending are available only from the Apple2
    // family of GPUs on.
    if ([*mDevice supportsFamily:MTLGPUFamilyApple2]) {
        EnableFeature(Feature::FramebufferFetch);
        EnableFeature(Feature::TransientAttachments);
    }

    // TODO(crbug.com/356461286): Intel and AMD GPUs support the indirect command buffer and
    // argument buffer features which are required for multi draw. However, multi draw end2end tests
    // fail on non-Apple GPUs. Disable the feature for non-Apple GPUs. Apple3 family is the minimum
    // requirement and only includes Apple GPUs.

    // TODO(crbug.com/393183837) - Re-enable this feature when we use argument buffers.
    // if ([*mDevice supportsFamily:MTLGPUFamilyApple3]) {
    //     EnableFeature(Feature::MultiDrawIndirect);
    // }

    EnableFeature(Feature::IndirectFirstInstance);
    EnableFeature(Feature::ShaderF16);
    EnableFeature(Feature::RG11B10UfloatRenderable);
    EnableFeature(Feature::BGRA8UnormStorage);
    EnableFeature(Feature::DualSourceBlending);
    EnableFeature(Feature::R8UnormStorage);
    EnableFeature(Feature::ShaderModuleCompilationOptions);
    EnableFeature(Feature::DawnLoadResolveTexture);
    EnableFeature(Feature::ClipDistances);
    EnableFeature(Feature::Float32Blendable);
    EnableFeature(Feature::FlexibleTextureViews);

    // The function subgroupBroadcast(f16) fails for some edge cases on intel gen-9 devices.
    // See crbug.com/391680973
    const bool kForceDisableSubgroups = gpu_info::IsIntelGen9(GetVendorId(), GetDeviceId());

    // SIMD-scoped permute operations is supported by GPU family Metal3, Apple6, Apple7, Apple8,
    // and Mac2.
    // https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
    // Metal3 family is a superset of Apple7 and Apple8, and introduced in macOS 13.0+ or
    // iOS 16.0+. However when building with Chrome, mac_sdk_official_version in mac_sdk.gni
    // explicitly use Xcode 13.3 and MacOS 12.3 version 21E226, so does not support
    // MTLGPUFamilyMetal3.
    // Note that supportsFamily: method requires macOS 10.15+ or iOS 13.0+
    // TODO(380326541): Check that reduction operations are supported in Apple6. The support
    // table says Apple7.
    if (!kForceDisableSubgroups && ([*mDevice supportsFamily:MTLGPUFamilyApple6] ||
                                    [*mDevice supportsFamily:MTLGPUFamilyMac2])) {
        EnableFeature(Feature::Subgroups);
        // TODO(crbug.com/380244620) remove SubgroupsF16
        EnableFeature(Feature::SubgroupsF16);
    }

    if ([*mDevice supportsFamily:MTLGPUFamilyApple7]) {
        EnableFeature(Feature::ChromiumExperimentalSubgroupMatrix);
    }

    EnableFeature(Feature::SharedTextureMemoryIOSurface);

    EnableFeature(Feature::SharedFenceMTLSharedEvent);

    EnableFeature(Feature::Unorm16TextureFormats);
    EnableFeature(Feature::Snorm16TextureFormats);
    EnableFeature(Feature::Norm16TextureFormats);

    EnableFeature(Feature::HostMappedPointer);

#if DAWN_PLATFORM_IS(IOS)
    EnableFeature(Feature::BufferMapExtendedUsages);
#else
    if ([*mDevice hasUnifiedMemory]) {
        EnableFeature(Feature::BufferMapExtendedUsages);
    }
#endif
}

void PhysicalDevice::InitializeVendorArchitectureImpl() {
    // According to Apple's documentation:
    // https://developer.apple.com/documentation/metal/gpu_devices_and_work_submission/detecting_gpu_features_and_metal_software_versions
    // - "Use the Common family to create apps that target a range of GPUs on multiple
    //   platforms.""
    // - "A GPU can be a member of more than one family; in most cases, a GPU supports one
    //   of the Common families and then one or more families specific to the build target."
    // So we'll use the highest supported common family as the reported "architecture" on
    // devices where a deviceID isn't available.
    if (mDeviceId == 0) {
        if (@available(macOS 13.0, iOS 16.0, *)) {
            // TODO(crbug.com/380316939): Replace the cast with MTLGPUFamilyMetal3 when
            // available.
            if ([*mDevice supportsFamily:static_cast<::MTLGPUFamily>(5001)]) {
                mArchitectureName = "metal-3";
            }
        } else if ([*mDevice supportsFamily:MTLGPUFamilyCommon3]) {
            mArchitectureName = "common-3";
        } else if ([*mDevice supportsFamily:MTLGPUFamilyCommon2]) {
            mArchitectureName = "common-2";
        } else if ([*mDevice supportsFamily:MTLGPUFamilyCommon1]) {
            mArchitectureName = "common-1";
        }
    }

    mVendorName = gpu_info::GetVendorName(mVendorId);
    if (mDeviceId != 0) {
        mArchitectureName = gpu_info::GetArchitectureName(mVendorId, mDeviceId);
    }
}

MaybeError PhysicalDevice::InitializeSupportedLimitsImpl(CombinedLimits* limits) {
    struct MTLDeviceLimits {
        uint32_t maxVertexAttribsPerDescriptor;
        uint32_t maxBufferArgumentEntriesPerFunc;
        uint32_t maxTextureArgumentEntriesPerFunc;
        uint32_t maxSamplerStateArgumentEntriesPerFunc;
        uint32_t maxThreadsPerThreadgroup;
        uint32_t maxTotalThreadgroupMemory;
        uint32_t maxFragmentInputs;
        uint32_t maxFragmentInputComponents;
        uint32_t max1DTextureSize;
        uint32_t max2DTextureSize;
        uint32_t max3DTextureSize;
        uint32_t maxTextureArrayLayers;
        uint32_t minBufferOffsetAlignment;
        uint32_t maxColorRenderTargets;
        uint32_t maxTotalRenderTargetSize;
    };

    struct LimitsForFamily {
        uint32_t MTLDeviceLimits::*limit;
        ityp::array<MTLGPUFamily, uint32_t, 9> values;
    };

    // clang-format off
            // https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
            //                                                               Apple                                                      Mac
            //                                                                   1,      2,      3,      4,      5,      6,      7,       1,      2
            constexpr LimitsForFamily kMTLLimits[15] = {
                {&MTLDeviceLimits::maxVertexAttribsPerDescriptor,         {    31u,    31u,    31u,    31u,    31u,    31u,    31u,     31u,    31u }},
                {&MTLDeviceLimits::maxBufferArgumentEntriesPerFunc,       {    31u,    31u,    31u,    31u,    31u,    31u,    31u,     31u,    31u }},
                {&MTLDeviceLimits::maxTextureArgumentEntriesPerFunc,      {    31u,    31u,    31u,    96u,    96u,   128u,   128u,    128u,   128u }},
                {&MTLDeviceLimits::maxSamplerStateArgumentEntriesPerFunc, {    16u,    16u,    16u,    16u,    16u,    16u,    16u,     16u,    16u }},
                {&MTLDeviceLimits::maxThreadsPerThreadgroup,              {   512u,   512u,   512u,  1024u,  1024u,  1024u,  1024u,   1024u,  1024u }},
                {&MTLDeviceLimits::maxTotalThreadgroupMemory,             { 16352u, 16352u, 16384u, 32768u, 32768u, 32768u, 32768u,  32768u, 32768u }},
                {&MTLDeviceLimits::maxFragmentInputs,                     {    60u,    60u,    60u,   124u,   124u,   124u,   124u,     32u,    32u }},
                {&MTLDeviceLimits::maxFragmentInputComponents,            {    60u,    60u,    60u,   124u,   124u,   124u,   124u,    124u,   124u }},
                {&MTLDeviceLimits::max1DTextureSize,                      {  8192u,  8192u, 16384u, 16384u, 16384u, 16384u, 16384u,  16384u, 16384u }},
                {&MTLDeviceLimits::max2DTextureSize,                      {  8192u,  8192u, 16384u, 16384u, 16384u, 16384u, 16384u,  16384u, 16384u }},
                {&MTLDeviceLimits::max3DTextureSize,                      {  2048u,  2048u,  2048u,  2048u,  2048u,  2048u,  2048u,   2048u,  2048u }},
                {&MTLDeviceLimits::maxTextureArrayLayers,                 {  2048u,  2048u,  2048u,  2048u,  2048u,  2048u,  2048u,   2048u,  2048u }},
                {&MTLDeviceLimits::minBufferOffsetAlignment,              {     4u,     4u,     4u,     4u,     4u,     4u,     4u,    256u,   256u }},
                {&MTLDeviceLimits::maxColorRenderTargets,                 {     4u,     8u,     8u,     8u,     8u,     8u,     8u,      8u,     8u }},
                // Note: the feature set tables list No Limit for Mac 1 and Mac 2.
                // For these, we use maxColorRenderTargets * 16. 16 is the largest cost of any color format.
                {&MTLDeviceLimits::maxTotalRenderTargetSize,              {    16u,    32u,    32u,    64u,    64u,    64u,    64u,    128u,   128u }},
            };
    // clang-format on

    MTLGPUFamily mtlGPUFamily;
    DAWN_TRY_ASSIGN(mtlGPUFamily, GetMTLGPUFamily(*mDevice));

    MTLDeviceLimits mtlLimits;
    for (const auto& limitsForFamily : kMTLLimits) {
        mtlLimits.*limitsForFamily.limit = limitsForFamily.values[mtlGPUFamily];
    }

    GetDefaultLimitsForSupportedFeatureLevel(&limits->v1);

    limits->v1.maxTextureDimension1D = mtlLimits.max1DTextureSize;
    limits->v1.maxTextureDimension2D = mtlLimits.max2DTextureSize;
    limits->v1.maxTextureDimension3D = mtlLimits.max3DTextureSize;
    limits->v1.maxTextureArrayLayers = mtlLimits.maxTextureArrayLayers;
    limits->v1.maxColorAttachments = mtlLimits.maxColorRenderTargets;
    limits->v1.maxColorAttachmentBytesPerSample = mtlLimits.maxTotalRenderTargetSize;

    uint32_t maxBuffersPerStage = mtlLimits.maxBufferArgumentEntriesPerFunc;
    maxBuffersPerStage -= 1;  // One slot is reserved to store buffer lengths.

    uint32_t baseMaxBuffersPerStage = limits->v1.maxStorageBuffersPerShaderStage +
                                      limits->v1.maxUniformBuffersPerShaderStage +
                                      limits->v1.maxVertexBuffers;

    DAWN_ASSERT(maxBuffersPerStage >= baseMaxBuffersPerStage);
    {
        // Allocate all remaining buffers to maxStorageBuffersPerShaderStage.
        // TODO(crbug.com/2158): We can have more of all types of buffers when
        // using Metal argument buffers.
        uint32_t additional = maxBuffersPerStage - baseMaxBuffersPerStage;
        limits->v1.maxStorageBuffersPerShaderStage += additional;
    }

    uint32_t baseMaxTexturesPerStage =
        limits->v1.maxSampledTexturesPerShaderStage + limits->v1.maxStorageTexturesPerShaderStage;

    DAWN_ASSERT(mtlLimits.maxTextureArgumentEntriesPerFunc >= baseMaxTexturesPerStage);
    {
        uint32_t additional = mtlLimits.maxTextureArgumentEntriesPerFunc - baseMaxTexturesPerStage;
        limits->v1.maxSampledTexturesPerShaderStage += additional / 2;
        limits->v1.maxStorageTexturesPerShaderStage += (additional - additional / 2);
    }

    limits->v1.maxSamplersPerShaderStage = mtlLimits.maxSamplerStateArgumentEntriesPerFunc;

    // Metal limits are per-function, so the layout limits are the same as the stage
    // limits. Note: this should likely change if the implementation uses Metal argument
    // buffers. Non-dynamic buffers will probably be bound argument buffers, but dynamic
    // buffers may be set directly.
    //   Mac GPU families with tier 1 argument buffers support 64
    //   buffers, 128 textures, and 16 samplers. Mac GPU families
    //   with tier 2 argument buffers support 500000 buffers and
    //   textures, and 1024 unique samplers
    // Without argument buffers, we have slots [0 -> 29], inclusive, which is 30 total.
    // 8 are used by maxVertexBuffers.
    limits->v1.maxDynamicUniformBuffersPerPipelineLayout = 11u;
    limits->v1.maxDynamicStorageBuffersPerPipelineLayout = 11u;

    // The WebGPU limit is the limit across all vertex buffers, combined.
    limits->v1.maxVertexAttributes =
        limits->v1.maxVertexBuffers * mtlLimits.maxVertexAttribsPerDescriptor;

    // See https://github.com/gpuweb/gpuweb/issues/1962 for more details.
    uint32_t vendorId = GetVendorId();
    if (gpu_info::IsApple(vendorId)) {
        limits->v1.maxInterStageShaderVariables =
            std::min(mtlLimits.maxFragmentInputs, mtlLimits.maxFragmentInputComponents / 4);
    } else {
        // On non-Apple macOS each built-in consumes one individual inter-stage shader variable.
        limits->v1.maxInterStageShaderVariables = mtlLimits.maxFragmentInputs - 4;
    }

    limits->v1.maxComputeWorkgroupStorageSize = mtlLimits.maxTotalThreadgroupMemory;
    limits->v1.maxComputeInvocationsPerWorkgroup = mtlLimits.maxThreadsPerThreadgroup;
    limits->v1.maxComputeWorkgroupSizeX = mtlLimits.maxThreadsPerThreadgroup;
    limits->v1.maxComputeWorkgroupSizeY = mtlLimits.maxThreadsPerThreadgroup;
    limits->v1.maxComputeWorkgroupSizeZ = mtlLimits.maxThreadsPerThreadgroup;

    limits->v1.minUniformBufferOffsetAlignment = mtlLimits.minBufferOffsetAlignment;
    limits->v1.minStorageBufferOffsetAlignment = mtlLimits.minBufferOffsetAlignment;

    uint64_t maxBufferSize = Buffer::QueryMaxBufferLength(*mDevice);
    limits->v1.maxBufferSize = maxBufferSize;

    // Metal has no documented limit on the size of a binding. Use the maximum
    // buffer size.
    limits->v1.maxUniformBufferBindingSize = maxBufferSize;
    limits->v1.maxStorageBufferBindingSize = maxBufferSize;

    // Using base limits for:
    // TODO(crbug.com/dawn/685):
    // - maxBindGroups
    // - maxVertexBufferArrayStride

    limits->v1.maxStorageBuffersInFragmentStage = limits->v1.maxStorageBuffersPerShaderStage;
    limits->v1.maxStorageTexturesInFragmentStage = limits->v1.maxStorageTexturesPerShaderStage;
    limits->v1.maxStorageBuffersInVertexStage = limits->v1.maxStorageBuffersPerShaderStage;
    limits->v1.maxStorageTexturesInVertexStage = limits->v1.maxStorageTexturesPerShaderStage;

    return {};
}

FeatureValidationResult PhysicalDevice::ValidateFeatureSupportedWithTogglesImpl(
    wgpu::FeatureName feature,
    const TogglesState& toggles) const {
    return {};
}

void PhysicalDevice::PopulateBackendProperties(UnpackedPtr<AdapterInfo>& info) const {
    if (auto* subgroupProperties = info.Get<AdapterPropertiesSubgroups>()) {
        subgroupProperties->subgroupMinSize = 4;
        subgroupProperties->subgroupMaxSize = 64;
    }
    if (auto* memoryHeapProperties = info.Get<AdapterPropertiesMemoryHeaps>()) {
        if ([*mDevice hasUnifiedMemory]) {
            auto* heapInfo = new MemoryHeapInfo[1];
            memoryHeapProperties->heapCount = 1;
            memoryHeapProperties->heapInfo = heapInfo;

            heapInfo[0].properties =
                wgpu::HeapProperty::DeviceLocal | wgpu::HeapProperty::HostVisible |
                wgpu::HeapProperty::HostCoherent | wgpu::HeapProperty::HostCached;
// TODO(dawn:2249): Enable on iOS. Some XCode or SDK versions seem to not match the docs.
#if DAWN_PLATFORM_IS(MACOS)
            if (@available(iOS 16.0, *)) {
                heapInfo[0].size = [*mDevice recommendedMaxWorkingSetSize];
            } else
#endif
            {
                // Since AdapterPropertiesMemoryHeaps is already gated on the
                // availability and #ifdef above, we should never reach this case, however
                // excluding the conditional causes build errors.
                DAWN_UNREACHABLE();
            }
        } else {
#if DAWN_PLATFORM_IS(MACOS)
            auto* heapInfo = new MemoryHeapInfo[2];
            memoryHeapProperties->heapCount = 2;
            memoryHeapProperties->heapInfo = heapInfo;

            heapInfo[0].properties = wgpu::HeapProperty::DeviceLocal;
            heapInfo[0].size = [*mDevice recommendedMaxWorkingSetSize];

            mach_msg_type_number_t hostBasicInfoMsg = HOST_BASIC_INFO_COUNT;
            host_basic_info_data_t hostInfo{};
            DAWN_CHECK(host_info(mach_host_self(), HOST_BASIC_INFO,
                                 reinterpret_cast<host_info_t>(&hostInfo),
                                 &hostBasicInfoMsg) == KERN_SUCCESS);

            heapInfo[1].properties = wgpu::HeapProperty::HostVisible |
                                     wgpu::HeapProperty::HostCoherent |
                                     wgpu::HeapProperty::HostCached;
            heapInfo[1].size = hostInfo.max_mem;
#else
            DAWN_UNREACHABLE();
#endif
        }
    }
    if (auto* subgroupMatrixConfigs = info.Get<AdapterPropertiesSubgroupMatrixConfigs>()) {
        DAWN_ASSERT([*mDevice supportsFamily:MTLGPUFamilyApple7]);

        auto* configs = new SubgroupMatrixConfig[2];
        subgroupMatrixConfigs->configCount = 2;
        subgroupMatrixConfigs->configs = configs;

        configs[0].componentType = wgpu::SubgroupMatrixComponentType::F32;
        configs[0].resultComponentType = wgpu::SubgroupMatrixComponentType::F32;
        configs[0].M = 8;
        configs[0].N = 8;
        configs[0].K = 8;

        configs[1].componentType = wgpu::SubgroupMatrixComponentType::F16;
        configs[1].resultComponentType = wgpu::SubgroupMatrixComponentType::F16;
        configs[1].M = 8;
        configs[1].N = 8;
        configs[1].K = 8;
    }
}
}  // namespace dawn::native::metal
