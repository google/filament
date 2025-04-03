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

#include "dawn/native/vulkan/SharedTextureMemoryVk.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/SystemHandle.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "dawn/native/vulkan/SharedFenceVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/native/wgpu_structs_autogen.h"

#if DAWN_PLATFORM_IS(ANDROID)
#include <android/hardware_buffer.h>

#include "dawn/native/AHBFunctions.h"
#endif  // DAWN_PLATFORM_IS(ANDROID)

namespace dawn::native::vulkan {

namespace {

#if DAWN_PLATFORM_IS(LINUX)

// Encoding from <drm/drm_fourcc.h>
constexpr uint32_t DrmFourccCode(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    return a | (b << 8) | (c << 16) | (d << 24);
}

constexpr uint32_t DrmFourccCode(char a, char b, char c, char d) {
    return DrmFourccCode(static_cast<uint32_t>(a), static_cast<uint32_t>(b),
                         static_cast<uint32_t>(c), static_cast<uint32_t>(d));
}

constexpr auto kDrmFormatR8 = DrmFourccCode('R', '8', ' ', ' '); /* [7:0] R */
constexpr auto kDrmFormatGR88 =
    DrmFourccCode('G', 'R', '8', '8'); /* [15:0] G:R 8:8 little endian */
constexpr auto kDrmFormatXRGB8888 =
    DrmFourccCode('X', 'R', '2', '4'); /* [15:0] x:R:G:B 8:8:8:8 little endian */
constexpr auto kDrmFormatXBGR8888 =
    DrmFourccCode('X', 'B', '2', '4'); /* [15:0] x:B:G:R 8:8:8:8 little endian */
constexpr auto kDrmFormatARGB8888 =
    DrmFourccCode('A', 'R', '2', '4'); /* [31:0] A:R:G:B 8:8:8:8 little endian */
constexpr auto kDrmFormatABGR8888 =
    DrmFourccCode('A', 'B', '2', '4'); /* [31:0] A:B:G:R 8:8:8:8 little endian */
constexpr auto kDrmFormatABGR2101010 =
    DrmFourccCode('A', 'B', '3', '0'); /* [31:0] A:B:G:R 2:10:10:10 little endian */
constexpr auto kDrmFormatABGR16161616F =
    DrmFourccCode('A', 'B', '4', 'H'); /* [63:0] A:B:G:R 16:16:16:16 little endian */
constexpr auto kDrmFormatNV12 = DrmFourccCode('N', 'V', '1', '2'); /* 2x2 subsampled Cr:Cb plane */

ResultOrError<wgpu::TextureFormat> FormatFromDrmFormat(uint32_t drmFormat) {
    switch (drmFormat) {
        case kDrmFormatR8:
            return wgpu::TextureFormat::R8Unorm;
        case kDrmFormatGR88:
            return wgpu::TextureFormat::RG8Unorm;
        case kDrmFormatXRGB8888:
        case kDrmFormatARGB8888:
            return wgpu::TextureFormat::BGRA8Unorm;
        case kDrmFormatXBGR8888:
        case kDrmFormatABGR8888:
            return wgpu::TextureFormat::RGBA8Unorm;
        case kDrmFormatABGR2101010:
            return wgpu::TextureFormat::RGB10A2Unorm;
        case kDrmFormatABGR16161616F:
            return wgpu::TextureFormat::RGBA16Float;
        case kDrmFormatNV12:
            return wgpu::TextureFormat::R8BG8Biplanar420Unorm;
        default:
            return DAWN_VALIDATION_ERROR("Unsupported drm format %x.", drmFormat);
    }
}

#endif  // DAWN_PLATFORM_IS(LINUX)

// Creates a VkImage with VkExternalMemoryImageCreateInfo::handlesTypes set to
// `externalMemoryHandleTypeFlagBits`. The rest of the parameters are computed from `properties`
// and `imageFormatInfo`. Additional structs may be chained by passing them in the varardic
// parameter args.
template <typename... AdditionalChains>
ResultOrError<VkImage> CreateExternalVkImage(
    Device* device,
    const SharedTextureMemoryProperties& properties,
    const VkPhysicalDeviceImageFormatInfo2& imageFormatInfo,
    VkExternalMemoryHandleTypeFlagBits externalMemoryHandleTypeFlagBits,
    AdditionalChains*... additionalChains) {
    VkImageCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.flags = imageFormatInfo.flags;
    createInfo.imageType = imageFormatInfo.type;
    createInfo.format = imageFormatInfo.format;
    createInfo.extent = {properties.size.width, properties.size.height, 1};
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = properties.size.depthOrArrayLayers;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling = imageFormatInfo.tiling;
    createInfo.usage = imageFormatInfo.usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {};
    externalMemoryImageCreateInfo.handleTypes = externalMemoryHandleTypeFlagBits;

    PNextChainBuilder createInfoChain(&createInfo);
    createInfoChain.Add(&externalMemoryImageCreateInfo,
                        VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO);

    (createInfoChain.Add(additionalChains), ...);

    // Create the VkImage.
    VkImage vkImage;
    DAWN_TRY(CheckVkSuccess(
        device->fn.CreateImage(device->GetVkDevice(), &createInfo, nullptr, &*vkImage),
        "vkCreateImage"));
    return vkImage;
}

// Add `VkPhysicalDeviceExternalImageFormatInfo` and `additionalChains` to `imageFormatInfo` and
// check this memory type and format combination can be imported.
template <typename... AdditionalChains>
MaybeError CheckExternalImageFormatSupport(
    Device* device,
    const SharedTextureMemoryProperties& properties,
    VkPhysicalDeviceImageFormatInfo2* imageFormatInfo,
    VkExternalMemoryHandleTypeFlagBits externalMemoryHandleTypeFlagBits,
    AdditionalChains*... additionalChains) {
    VkPhysicalDeviceExternalImageFormatInfo externalImageFormatInfo = {};
    externalImageFormatInfo.handleType = externalMemoryHandleTypeFlagBits;

    PNextChainBuilder imageFormatInfoChain(imageFormatInfo);
    imageFormatInfoChain.Add(&externalImageFormatInfo,
                             VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO);
    (imageFormatInfoChain.Add(additionalChains), ...);

    VkImageFormatProperties2 imageFormatProps = {};
    imageFormatProps.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;

    VkExternalImageFormatProperties externalImageFormatProps = {};

    PNextChainBuilder imageFormatPropsChain(&imageFormatProps);
    imageFormatPropsChain.Add(&externalImageFormatProps,
                              VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES);

    DAWN_TRY_CONTEXT(
        CheckVkSuccess(device->fn.GetPhysicalDeviceImageFormatProperties2(
                           ToBackend(device->GetPhysicalDevice())->GetVkPhysicalDevice(),
                           imageFormatInfo, &imageFormatProps),
                       "vkGetPhysicalDeviceImageFormatProperties"),
        "checking import support for external memory type %x with %s %s\n",
        externalMemoryHandleTypeFlagBits, properties.format, properties.usage);

    VkExternalMemoryFeatureFlags featureFlags =
        externalImageFormatProps.externalMemoryProperties.externalMemoryFeatures;
    DAWN_INVALID_IF(!(featureFlags & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT),
                    "Vulkan memory is not importable.");

    return {};
}

// Add `additionalChains` to `memoryAllocateInfo` and call vkAllocateMemory.
template <typename... AdditionalChains>
ResultOrError<VkDeviceMemory> AllocateDeviceMemory(Device* device,
                                                   VkMemoryAllocateInfo* memoryAllocateInfo,
                                                   AdditionalChains*... additionalChains) {
    PNextChainBuilder memoryAllocateInfoChain(memoryAllocateInfo);

    (memoryAllocateInfoChain.Add(additionalChains), ...);

    VkDeviceMemory vkDeviceMemory;
    DAWN_TRY(CheckVkSuccess(device->fn.AllocateMemory(device->GetVkDevice(), memoryAllocateInfo,
                                                      nullptr, &*vkDeviceMemory),
                            "vkAllocateMemory"));
    return vkDeviceMemory;
}

}  // namespace

// static
ResultOrError<Ref<SharedTextureMemory>> SharedTextureMemory::Create(
    Device* device,
    StringView label,
    const SharedTextureMemoryDmaBufDescriptor* descriptor) {
#if DAWN_PLATFORM_IS(LINUX)
    VkDevice vkDevice = device->GetVkDevice();
    VkPhysicalDevice vkPhysicalDevice =
        ToBackend(device->GetPhysicalDevice())->GetVkPhysicalDevice();

    const VkExternalMemoryHandleTypeFlagBits handleType =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;

    DAWN_INVALID_IF(descriptor->size.depthOrArrayLayers != 1, "depthOrArrayLayers was not 1.");

    SharedTextureMemoryProperties properties;
    properties.size = {descriptor->size.width, descriptor->size.height,
                       descriptor->size.depthOrArrayLayers};

    DAWN_TRY_ASSIGN(properties.format, FormatFromDrmFormat(descriptor->drmFormat));

    properties.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                       wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding |
                       wgpu::TextureUsage::RenderAttachment;

    // Create the SharedTextureMemory object.
    Ref<SharedTextureMemory> sharedTextureMemory =
        SharedTextureMemory::Create(device, label, properties, VK_QUEUE_FAMILY_EXTERNAL_KHR);

    // Reflect properties to reify them.
    sharedTextureMemory->APIGetProperties(&properties);

    const Format* internalFormat = nullptr;
    DAWN_TRY_ASSIGN(internalFormat, device->GetInternalFormat(properties.format));

    const auto& compatibleViewFormats = device->GetCompatibleViewFormats(*internalFormat);

    VkFormat vkFormat = VulkanImageFormat(device, properties.format);

    // Usage flags to create the image with.
    VkImageUsageFlags vkUsageFlags = VulkanImageUsage(device, properties.usage, *internalFormat);

    // Number of memory planes in the image which will be queried from the DRM modifier.
    uint32_t memoryPlaneCount;

    // Info describing the image import. We will use this to check the import is valid, and then
    // perform the actual VkImage creation.
    VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {};
    // List of view formats the image can be created.
    std::array<VkFormat, 3> viewFormats;
    VkImageFormatListCreateInfo imageFormatListInfo = {};
    imageFormatListInfo.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;

    // The Vulkan spec seems to be too strict in that:
    // - use of DRM modifiers requires passing an image format list
    // - that image format list can't have sRGB formats if the usage has StorageBinding
    // In practice, it's "fine" to create a normal VkImage with StorageBinding and sRGB in the
    // format list, and the VVL here may be wrong. See also see
    // https://github.com/gpuweb/gpuweb/issues/4426. The support check may be unnecessarily strict.
    // TODO(crbug.com/dawn/2304): Follow up with the Vulkan spec and try to lift this.
    // Here, we set `addViewFormats` after checking for support to work around the strictness.
    bool addViewFormats = false;

    // Validate that the import is valid.
    {
        // Verify plane count for the modifier.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDrmFormatModifierPropertiesEXT.html#_description
        VkDrmFormatModifierPropertiesEXT drmModifierProps;
        DAWN_TRY_ASSIGN(drmModifierProps,
                        GetFormatModifierProps(device->fn, vkPhysicalDevice, vkFormat,
                                               descriptor->drmModifier));
        memoryPlaneCount = drmModifierProps.drmFormatModifierPlaneCount;
        if (drmModifierProps.drmFormatModifier == 0 /* DRM_FORMAT_MOD_LINEAR */) {
            uint32_t formatPlaneCount = GetAspectCount(internalFormat->aspects);
            DAWN_INVALID_IF(memoryPlaneCount != formatPlaneCount,
                            "DRM format plane count (%u) must match the format plane count (%u) if "
                            "drmModifier is DRM_FORMAT_MOD_LINEAR.",
                            memoryPlaneCount, formatPlaneCount);
        }
        DAWN_INVALID_IF(
            memoryPlaneCount != descriptor->planeCount,
            "Memory plane count (%x) for drm format (%u) and modifier (%u) specify a plane "
            "count of %u which "
            "does not match the provided plane count (%u)",
            vkFormat, descriptor->drmFormat, descriptor->drmModifier, memoryPlaneCount,
            descriptor->planeCount);
        DAWN_INVALID_IF(memoryPlaneCount == 0, "Memory plane count must not be 0");
        DAWN_INVALID_IF(memoryPlaneCount > kMaxPlanesPerFormat,
                        "Memory plane count (%u) must not exceed %u.", memoryPlaneCount,
                        kMaxPlanesPerFormat);

        // Verify that the format modifier of the external memory and the requested Vulkan format
        // are actually supported together in a dma-buf import.
        imageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
        imageFormatInfo.format = vkFormat;
        imageFormatInfo.type = VK_IMAGE_TYPE_2D;
        imageFormatInfo.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
        imageFormatInfo.usage = vkUsageFlags;
        imageFormatInfo.flags = 0;

        VkPhysicalDeviceImageDrmFormatModifierInfoEXT drmModifierInfo = {};
        drmModifierInfo.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT;
        drmModifierInfo.drmFormatModifier = descriptor->drmModifier;
        drmModifierInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        constexpr wgpu::TextureUsage kUsageRequiringView = wgpu::TextureUsage::RenderAttachment |
                                                           wgpu::TextureUsage::TextureBinding |
                                                           wgpu::TextureUsage::StorageBinding;
        const bool mayNeedView = (properties.usage & kUsageRequiringView) != 0;

        if (mayNeedView) {
            // Add the mutable format bit for view reinterpretation.
            imageFormatInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

            // Append the list of view formats the image must be compatible with.
            if (device->GetDeviceInfo().HasExt(DeviceExt::ImageFormatList)) {
                if (internalFormat->IsMultiPlanar()) {
                    viewFormats = {
                        VulkanImageFormat(device,
                                          internalFormat->GetAspectInfo(Aspect::Plane0).format),
                        VulkanImageFormat(device,
                                          internalFormat->GetAspectInfo(Aspect::Plane1).format)};
                    imageFormatListInfo.viewFormatCount = 2;
                } else {
                    // Pass the format as the one and only allowed view format.
                    // Use of VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT requires passing a non-zero
                    // list.
                    const bool needsBGRA8UnormStoragePolyfill =
                        properties.format == wgpu::TextureFormat::BGRA8Unorm &&
                        (properties.usage & wgpu::TextureUsage::StorageBinding);
                    if (compatibleViewFormats.empty()) {
                        DAWN_ASSERT(!needsBGRA8UnormStoragePolyfill);
                        viewFormats = {vkFormat};
                        imageFormatListInfo.viewFormatCount = 1;
                    } else {
                        viewFormats[imageFormatListInfo.viewFormatCount++] = vkFormat;
                        if (needsBGRA8UnormStoragePolyfill) {
                            viewFormats[imageFormatListInfo.viewFormatCount++] =
                                VK_FORMAT_R8G8B8A8_UNORM;
                        }
                        addViewFormats = true;
                    }
                }
                imageFormatListInfo.pViewFormats = viewFormats.data();
            }
        }

        if (imageFormatListInfo.viewFormatCount > 0) {
            DAWN_TRY_CONTEXT(
                CheckExternalImageFormatSupport(device, properties, &imageFormatInfo, handleType,
                                                &drmModifierInfo, &imageFormatListInfo),
                "checking import support for fd import of dma buf");
        } else {
            DAWN_TRY_CONTEXT(CheckExternalImageFormatSupport(device, properties, &imageFormatInfo,
                                                             handleType, &drmModifierInfo),
                             "checking import support for fd import of dma buf");
        }
    }

    // Validate that there is a single FD. If there is more than one FD, Dawn will need to validate
    // the format has VK_FORMAT_FEATURE_DISJOINT_BIT, create the VkImage with
    // VK_IMAGE_CREATE_DISJOINT_BIT, and separately bind the image planes to memory. Dawn doesn't
    // support use of VK_IMAGE_CREATE_DISJOINT_BIT currently. See crbug.com/42240514.
    int fd = descriptor->planes[0].fd;
    for (uint32_t i = 1; i < descriptor->planeCount; ++i) {
        DAWN_INVALID_IF(descriptor->planes[i].fd != fd,
                        "descriptor->planes[%u].fd (%i) does not match other plane fd (%i). All "
                        "fds must be the same.",
                        i, descriptor->planes[i].fd, fd);
    }

    // Don't add the view format if backend validation is enabled, otherwise most image creations
    // will fail with VVL. This view format is only needed for sRGB reinterpretation.
    // TODO(crbug.com/dawn/2304): Investigate if this is a bug in VVL.
    if (addViewFormats && !device->GetAdapter()->GetInstance()->IsBackendValidationEnabled()) {
        DAWN_ASSERT(compatibleViewFormats.size() == 1u);
        viewFormats[imageFormatListInfo.viewFormatCount++] =
            VulkanImageFormat(device, compatibleViewFormats[0]->format);
    }

    // Create the VkImage for the import.
    {
        std::array<VkSubresourceLayout, kMaxPlanesPerFormat> planeLayouts;
        for (uint32_t plane = 0u; plane < memoryPlaneCount; ++plane) {
            planeLayouts[plane].offset = descriptor->planes[plane].offset;
            planeLayouts[plane].size = 0;  // VK_EXT_image_drm_format_modifier mandates size = 0.
            planeLayouts[plane].rowPitch = descriptor->planes[plane].stride;
            planeLayouts[plane].arrayPitch = 0;  // Not an array texture
            planeLayouts[plane].depthPitch = 0;  // Not a depth texture
        }

        VkImageDrmFormatModifierExplicitCreateInfoEXT explicitCreateInfo = {};
        explicitCreateInfo.sType =
            VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT;
        explicitCreateInfo.drmFormatModifier = descriptor->drmModifier;
        explicitCreateInfo.drmFormatModifierPlaneCount = memoryPlaneCount;
        explicitCreateInfo.pPlaneLayouts = planeLayouts.data();

        VkImage vkImage;
        DAWN_TRY_ASSIGN(vkImage,
                        CreateExternalVkImage(device, properties, imageFormatInfo, handleType,
                                              &imageFormatListInfo, &explicitCreateInfo));

        sharedTextureMemory->mVkImage =
            AcquireRef(new RefCountedVkHandle<VkImage>(device, vkImage));
    }

    // Import the memory plane(s) as VkDeviceMemory and bind to the VkImage.
    VkMemoryFdPropertiesKHR fdProperties;
    fdProperties.sType = VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR;
    fdProperties.pNext = nullptr;

    // Get the valid memory types that the external memory can be imported as.
    DAWN_TRY(CheckVkSuccess(device->fn.GetMemoryFdPropertiesKHR(
                                vkDevice, handleType, descriptor->planes[0].fd, &fdProperties),
                            "vkGetMemoryFdPropertiesKHR"));

    // Get the valid memory types for the VkImage.
    VkMemoryRequirements memoryRequirements;
    device->fn.GetImageMemoryRequirements(vkDevice, sharedTextureMemory->mVkImage->Get(),
                                          &memoryRequirements);

    // Choose the best memory type that satisfies both the image's constraint and the
    // import's constraint.
    memoryRequirements.memoryTypeBits &= fdProperties.memoryTypeBits;
    int memoryTypeIndex = device->GetResourceMemoryAllocator()->FindBestTypeIndex(
        memoryRequirements, MemoryKind::DeviceLocal);
    DAWN_INVALID_IF(memoryTypeIndex == -1, "Unable to find an appropriate memory type for import.");

    SystemHandle memoryFD;
    DAWN_TRY_ASSIGN(memoryFD, SystemHandle::Duplicate(fd));

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

    VkImportMemoryFdInfoKHR importMemoryFdInfo;
    importMemoryFdInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR;
    importMemoryFdInfo.handleType = handleType;
    importMemoryFdInfo.fd = memoryFD.Get();

    // Import the fd as VkDeviceMemory
    VkDeviceMemory vkDeviceMemory;
    DAWN_TRY_ASSIGN(vkDeviceMemory,
                    AllocateDeviceMemory(device, &memoryAllocateInfo, &importMemoryFdInfo));

    memoryFD.Detach();  // Ownership transfered to the VkDeviceMemory.
    sharedTextureMemory->mVkDeviceMemory =
        AcquireRef(new RefCountedVkHandle<VkDeviceMemory>(device, vkDeviceMemory));

    // Bind the VkImage to the memory.
    DAWN_TRY(
        CheckVkSuccess(device->fn.BindImageMemory(vkDevice, sharedTextureMemory->mVkImage->Get(),
                                                  sharedTextureMemory->mVkDeviceMemory->Get(), 0),
                       "vkBindImageMemory"));
    return sharedTextureMemory;
#else
    DAWN_UNREACHABLE();
#endif  // DAWN_PLATFORM_IS(LINUX)
}

// static
ResultOrError<Ref<SharedTextureMemory>> SharedTextureMemory::Create(
    Device* device,
    StringView label,
    const SharedTextureMemoryAHardwareBufferDescriptor* descriptor) {
#if DAWN_PLATFORM_IS(ANDROID)
    const auto* ahbFunctions =
        ToBackend(device->GetAdapter()->GetPhysicalDevice())->GetOrLoadAHBFunctions();
    VkDevice vkDevice = device->GetVkDevice();

    auto* aHardwareBuffer = static_cast<struct AHardwareBuffer*>(descriptor->handle);

    bool useExternalFormat = descriptor->useExternalFormat;

    const VkExternalMemoryHandleTypeFlagBits handleType =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    // Reflect the properties of the AHardwareBuffer.
    SharedTextureMemoryProperties properties =
        GetAHBSharedTextureMemoryProperties(ahbFunctions, aHardwareBuffer);

    if (useExternalFormat) {
        // When using the external YUV texture format, only TextureBinding usage is valid.
        properties.usage &= wgpu::TextureUsage::TextureBinding;
    }

    VkFormat vkFormat;
    YCbCrVkDescriptor yCbCrAHBInfo;
    SampleTypeBit externalSampleType;
    VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
    };
    VkExternalFormatANDROID externalFormatAndroid = {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID,
    };

    // Query the properties to find the appropriate VkFormat and memory type.
    {
        VkAndroidHardwareBufferFormatPropertiesANDROID bufferFormatProperties;
        PNextChainBuilder bufferPropertiesChain(&bufferProperties);
        bufferPropertiesChain.Add(
            &bufferFormatProperties,
            VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID);

        DAWN_TRY(CheckVkSuccess(device->fn.GetAndroidHardwareBufferPropertiesANDROID(
                                    vkDevice, aHardwareBuffer, &bufferProperties),
                                "vkGetAndroidHardwareBufferPropertiesANDROID"));

        // TODO(crbug.com/dawn/2476): Validate more as per
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageCreateInfo.html
        if (useExternalFormat) {
            DAWN_INVALID_IF(
                bufferFormatProperties.externalFormat == 0,
                "AHardwareBuffer with external sampler must have non-zero external format.");
            vkFormat = VK_FORMAT_UNDEFINED;
            externalFormatAndroid.externalFormat = bufferFormatProperties.externalFormat;
            properties.format = wgpu::TextureFormat::External;
        } else {
            vkFormat = bufferFormatProperties.format;
            externalFormatAndroid.externalFormat = 0;
            DAWN_TRY_ASSIGN(properties.format, FormatFromVkFormat(device, vkFormat));
        }

        // Populate the YCbCr info.
        yCbCrAHBInfo.externalFormat = externalFormatAndroid.externalFormat;
        yCbCrAHBInfo.vkFormat = vkFormat;
        yCbCrAHBInfo.vkYCbCrModel = bufferFormatProperties.suggestedYcbcrModel;
        yCbCrAHBInfo.vkYCbCrRange = bufferFormatProperties.suggestedYcbcrRange;
        yCbCrAHBInfo.vkComponentSwizzleRed =
            bufferFormatProperties.samplerYcbcrConversionComponents.r;
        yCbCrAHBInfo.vkComponentSwizzleGreen =
            bufferFormatProperties.samplerYcbcrConversionComponents.g;
        yCbCrAHBInfo.vkComponentSwizzleBlue =
            bufferFormatProperties.samplerYcbcrConversionComponents.b;
        yCbCrAHBInfo.vkComponentSwizzleAlpha =
            bufferFormatProperties.samplerYcbcrConversionComponents.a;
        yCbCrAHBInfo.vkXChromaOffset = bufferFormatProperties.suggestedXChromaOffset;
        yCbCrAHBInfo.vkYChromaOffset = bufferFormatProperties.suggestedYChromaOffset;

        uint32_t formatFeatures = bufferFormatProperties.formatFeatures;
        if (formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT) {
            yCbCrAHBInfo.vkChromaFilter = wgpu::FilterMode::Linear;
            externalSampleType = SampleTypeBit::UnfilterableFloat | SampleTypeBit::Float;
        } else {
            yCbCrAHBInfo.vkChromaFilter = wgpu::FilterMode::Nearest;
            externalSampleType = SampleTypeBit::UnfilterableFloat;
        }
        yCbCrAHBInfo.forceExplicitReconstruction =
            formatFeatures &
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT;
    }

    const Format* internalFormat = nullptr;
    DAWN_TRY_ASSIGN(internalFormat, device->GetInternalFormat(properties.format));

    DAWN_INVALID_IF(internalFormat->IsMultiPlanar(),
                    "Multi-planar AHardwareBuffer not supported yet.");

    // Create the SharedTextureMemory object.
    Ref<SharedTextureMemory> sharedTextureMemory =
        SharedTextureMemory::Create(device, label, properties, VK_QUEUE_FAMILY_FOREIGN_EXT);

    sharedTextureMemory->mYCbCrAHBInfo = yCbCrAHBInfo;
    sharedTextureMemory->GetContents()->SetExternalFormatSupportedSampleTypes(externalSampleType);

    // Reflect properties to reify them.
    sharedTextureMemory->APIGetProperties(&properties);

    // Compute the Vulkan usage flags to create the image with.
    VkImageUsageFlags vkUsageFlags = VulkanImageUsage(device, properties.usage, *internalFormat);

    const auto& compatibleViewFormats = device->GetCompatibleViewFormats(*internalFormat);

    // Info describing the image import. We will use this to check the import is valid, and then
    // perform the actual VkImage creation.
    VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {};
    // List of view formats the image can be created.
    std::array<VkFormat, 2> viewFormats;
    VkImageFormatListCreateInfo imageFormatListInfo = {};
    imageFormatListInfo.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;

    // Validate that the import is valid
    {
        // Verify that the format modifier of the external memory and the requested Vulkan format
        // are actually supported together in an AHB import.
        imageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
        imageFormatInfo.format = vkFormat;
        imageFormatInfo.type = VK_IMAGE_TYPE_2D;
        imageFormatInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageFormatInfo.usage = vkUsageFlags;
        imageFormatInfo.flags = 0;

        if (!useExternalFormat) {
            constexpr wgpu::TextureUsage kUsageRequiringView =
                wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding |
                wgpu::TextureUsage::StorageBinding;
            const bool mayNeedViewReinterpretation =
                (properties.usage & kUsageRequiringView) != 0 && !compatibleViewFormats.empty();
            const bool needsBGRA8UnormStoragePolyfill =
                properties.format == wgpu::TextureFormat::BGRA8Unorm &&
                (properties.usage & wgpu::TextureUsage::StorageBinding);
            if (mayNeedViewReinterpretation || needsBGRA8UnormStoragePolyfill) {
                // Add the mutable format bit for view reinterpretation.
                imageFormatInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

                if (properties.usage & wgpu::TextureUsage::StorageBinding) {
                    // Don't use an image format list because it becomes impossible to make an
                    // rgba8unorm storage texture which may be reinterpreted as rgba8unorm-srgb,
                    // because the srgb format doesn't support storage. Creation with an explicit
                    // format list that includes srgb will fail.
                    // This is the same issue seen with the DMA buf import path which has a
                    // workaround to bypass the support check.
                    // TODO(crbug.com/dawn/2304): If the dma buf import is resolved in a better way,
                    // apply the same fix here.
                } else if (device->GetDeviceInfo().HasExt(DeviceExt::ImageFormatList)) {
                    // Set the list of view formats the image can be compatible with.
                    DAWN_ASSERT(compatibleViewFormats.size() == 1u);
                    viewFormats[0] = vkFormat;
                    viewFormats[1] = VulkanImageFormat(device, compatibleViewFormats[0]->format);
                    imageFormatListInfo.viewFormatCount = 2;
                    imageFormatListInfo.pViewFormats = viewFormats.data();
                }
            }

            if (imageFormatListInfo.viewFormatCount > 0) {
                DAWN_TRY_CONTEXT(
                    CheckExternalImageFormatSupport(device, properties, &imageFormatInfo,
                                                    handleType, &imageFormatListInfo),
                    "checking import support of AHardwareBuffer");
            } else {
                DAWN_TRY_CONTEXT(CheckExternalImageFormatSupport(device, properties,
                                                                 &imageFormatInfo, handleType),
                                 "checking import support of AHardwareBuffer");
            }
        }
    }

    // Create the VkImage for the import.
    {
        VkImage vkImage;
        DAWN_TRY_ASSIGN(vkImage,
                        CreateExternalVkImage(device, properties, imageFormatInfo, handleType,
                                              &imageFormatListInfo, &externalFormatAndroid));

        sharedTextureMemory->mVkImage =
            AcquireRef(new RefCountedVkHandle<VkImage>(device, vkImage));
    }

    // Import the memory as VkDeviceMemory and bind to the VkImage.
    {
        // Choose the best memory type that satisfies the import's constraint.
        VkMemoryRequirements memoryRequirements;
        memoryRequirements.memoryTypeBits = bufferProperties.memoryTypeBits;
        int memoryTypeIndex = device->GetResourceMemoryAllocator()->FindBestTypeIndex(
            memoryRequirements, MemoryKind::DeviceLocal);
        DAWN_INVALID_IF(memoryTypeIndex == -1,
                        "Unable to find an appropriate memory type for import.");

        VkMemoryAllocateInfo memoryAllocateInfo = {};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.allocationSize = bufferProperties.allocationSize;
        memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

        VkImportAndroidHardwareBufferInfoANDROID importMemoryAHBInfo = {};
        importMemoryAHBInfo.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
        importMemoryAHBInfo.buffer = aHardwareBuffer;

        // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#memory-external-android-hardware-buffer-image-resources
        // AHardwareBuffer imports *must* use dedicated allocations.
        VkMemoryDedicatedAllocateInfo dedicatedAllocateInfo;
        dedicatedAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
        dedicatedAllocateInfo.image = sharedTextureMemory->mVkImage->Get();
        dedicatedAllocateInfo.buffer = VkBuffer{};

        // Import the AHardwareBuffer as VkDeviceMemory
        VkDeviceMemory vkDeviceMemory;
        DAWN_TRY_ASSIGN(vkDeviceMemory,
                        AllocateDeviceMemory(device, &memoryAllocateInfo, &dedicatedAllocateInfo,
                                             &importMemoryAHBInfo));

        sharedTextureMemory->mVkDeviceMemory =
            AcquireRef(new RefCountedVkHandle<VkDeviceMemory>(device, vkDeviceMemory));

        // Bind the VkImage to the memory.
        DAWN_TRY(CheckVkSuccess(
            device->fn.BindImageMemory(vkDevice, sharedTextureMemory->mVkImage->Get(),
                                       sharedTextureMemory->mVkDeviceMemory->Get(), 0),
            "vkBindImageMemory"));

        // Verify the texture memory requirements fit within the constraints of the AHardwareBuffer.
        device->fn.GetImageMemoryRequirements(vkDevice, sharedTextureMemory->mVkImage->Get(),
                                              &memoryRequirements);

        DAWN_INVALID_IF((memoryRequirements.memoryTypeBits & bufferProperties.memoryTypeBits) == 0,
                        "Required memory type bits (%u) do not overlap with AHardwareBuffer memory "
                        "type bits (%u).",
                        memoryRequirements.memoryTypeBits, bufferProperties.memoryTypeBits);

        if (!device->IsToggleEnabled(Toggle::IgnoreImportedAHardwareBufferVulkanImageSize)) {
            DAWN_INVALID_IF(memoryRequirements.size > bufferProperties.allocationSize,
                            "Required texture memory size (%u) is larger than the AHardwareBuffer "
                            "allocation size (%u).",
                            memoryRequirements.size, bufferProperties.allocationSize);
        }
    }
    return sharedTextureMemory;
#else
    DAWN_UNREACHABLE();
#endif  // DAWN_PLATFORM_IS(ANDROID)
}

// static
ResultOrError<Ref<SharedTextureMemory>> SharedTextureMemory::Create(
    Device* device,
    StringView label,
    const SharedTextureMemoryOpaqueFDDescriptor* descriptor) {
#if DAWN_PLATFORM_IS(POSIX)
    VkDevice vkDevice = device->GetVkDevice();

    const VkExternalMemoryHandleTypeFlagBits handleType =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    const VkImageCreateInfo* createInfo =
        static_cast<const VkImageCreateInfo*>(descriptor->vkImageCreateInfo);
    DAWN_INVALID_IF(
        createInfo->sType != VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        "descriptor->vkImageCreateInfo.sType was not VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO.");

    // Validate the createInfo chain.
    const VkExternalMemoryImageCreateInfo* externalMemoryImageCreateInfo = nullptr;
    const VkImageFormatListCreateInfo* imageFormatListInfo = nullptr;
    {
        const VkBaseInStructure* current = static_cast<const VkBaseInStructure*>(createInfo->pNext);
        while (current != nullptr) {
            switch (current->sType) {
                case VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO:
                    // TODO(crbug.com/dawn/1745): Use this to inform supported types of WebGPU
                    // format reinterpretation (srgb).
                    imageFormatListInfo =
                        reinterpret_cast<const VkImageFormatListCreateInfo*>(current);
                    break;
                case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO:
                    externalMemoryImageCreateInfo =
                        reinterpret_cast<const VkExternalMemoryImageCreateInfo*>(current);
                    DAWN_INVALID_IF((externalMemoryImageCreateInfo->handleTypes & handleType) == 0,
                                    "VkExternalMemoryImageCreateInfo::handleTypes (chained on "
                                    "descriptor->vkImageCreateInfo) did not have "
                                    "VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT.");
                    break;
                default:
                    return DAWN_VALIDATION_ERROR(
                        "Unsupported descriptor->vkImageCreateInfo chain with sType 0x%x",
                        current->sType);
            }
            current = current->pNext;
        }
    }

    DAWN_INVALID_IF(
        externalMemoryImageCreateInfo == nullptr,
        "descriptor->vkImageCreateInfo did not have chain with VkExternalMemoryImageCreateInfo");

    DAWN_INVALID_IF(
        (createInfo->usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0,
        "descriptor->vkImageCreateInfo.usage did not have VK_IMAGE_USAGE_TRANSFER_DST_BIT");

    const bool isBGRA8UnormStorage = createInfo->format == VK_FORMAT_B8G8R8A8_UNORM &&
                                     (createInfo->usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0;
    DAWN_INVALID_IF(
        isBGRA8UnormStorage && (createInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) == 0,
        "descriptor->vkImageCreateInfo.flags did not have VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT when "
        "usage has VK_IMAGE_USAGE_STORAGE_BIT and format is VK_FORMAT_B8G8R8A8_UNORM");

    // Populate the properties from the VkImageCreateInfo
    SharedTextureMemoryProperties properties{};
    properties.size = {
        createInfo->extent.width,
        createInfo->extent.height,
        std::max(createInfo->arrayLayers, createInfo->extent.depth),
    };
    DAWN_TRY_ASSIGN(properties.format, FormatFromVkFormat(device, createInfo->format));
    if (createInfo->usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        properties.usage |= wgpu::TextureUsage::CopySrc;
    }
    if (createInfo->usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        properties.usage |= wgpu::TextureUsage::CopyDst;
    }
    if (createInfo->usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
        properties.usage |= wgpu::TextureUsage::TextureBinding;
    }
    if (createInfo->usage & VK_IMAGE_USAGE_STORAGE_BIT) {
        properties.usage |= wgpu::TextureUsage::StorageBinding;
    }
    if (createInfo->usage &
        (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
        properties.usage |= wgpu::TextureUsage::RenderAttachment;
    }

    const Format* internalFormat;
    DAWN_TRY_ASSIGN(internalFormat, device->GetInternalFormat(properties.format));

    const auto& compatibleViewFormats = device->GetCompatibleViewFormats(*internalFormat);

    // Create the SharedTextureMemory object.
    Ref<SharedTextureMemory> sharedTextureMemory =
        SharedTextureMemory::Create(device, label, properties, VK_QUEUE_FAMILY_EXTERNAL_KHR);

    // Reflect properties to reify them.
    sharedTextureMemory->APIGetProperties(&properties);

    constexpr wgpu::TextureUsage kUsageRequiringView = wgpu::TextureUsage::RenderAttachment |
                                                       wgpu::TextureUsage::TextureBinding |
                                                       wgpu::TextureUsage::StorageBinding;
    const bool mayNeedViewReinterpretation =
        (properties.usage & kUsageRequiringView) != 0 && !compatibleViewFormats.empty();

    DAWN_INVALID_IF(
        mayNeedViewReinterpretation && !(createInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT),
        "VkImageCreateInfo::flags did not have VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT "
        "which is required for view format reinterpretation.");

    if (imageFormatListInfo && mayNeedViewReinterpretation) {
        auto viewFormatsBegin = imageFormatListInfo->pViewFormats;
        auto viewFormatsEnd =
            imageFormatListInfo->pViewFormats + imageFormatListInfo->viewFormatCount;
        VkFormat baseVkFormat = VulkanImageFormat(device, properties.format);

        DAWN_INVALID_IF(
            std::find(viewFormatsBegin, viewFormatsEnd, baseVkFormat) == viewFormatsEnd,
            "VkImageFormatCreateInfo did not contain VkFormat 0x%x which may be required to "
            "create a texture view with %s.",
            baseVkFormat, properties.format);

        for (const auto* f : compatibleViewFormats) {
            VkFormat vkFormat = VulkanImageFormat(device, f->format);
            DAWN_INVALID_IF(std::find(viewFormatsBegin, viewFormatsEnd, vkFormat) == viewFormatsEnd,
                            "VkImageFormatCreateInfo did not contain VkFormat 0x%x which may be "
                            "required to create a texture view with %s.",
                            vkFormat, f->format);
        }
    }

    // Validate that an OpaqueFD import with this createInfo is valid.
    {
        VkPhysicalDeviceImageFormatInfo2 imageFormatInfo;
        imageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2_KHR;
        imageFormatInfo.pNext = nullptr;
        imageFormatInfo.format = createInfo->format;
        imageFormatInfo.type = createInfo->imageType;
        imageFormatInfo.tiling = createInfo->tiling;
        imageFormatInfo.usage = createInfo->usage;
        imageFormatInfo.flags = createInfo->flags;

        if (imageFormatListInfo) {
            VkImageFormatListCreateInfo imageFormatListInfoCopy;
            imageFormatListInfoCopy = *imageFormatListInfo;
            DAWN_TRY_CONTEXT(CheckExternalImageFormatSupport(device, properties, &imageFormatInfo,
                                                             handleType, &imageFormatListInfoCopy),
                             "checking import support for opaque fd import");
        } else {
            DAWN_TRY_CONTEXT(
                CheckExternalImageFormatSupport(device, properties, &imageFormatInfo, handleType),
                "checking import support for opaque fd import");
        }
    }

    // Create the VkImage
    {
        VkImage vkImage;
        DAWN_TRY(CheckVkSuccess(device->fn.CreateImage(vkDevice, createInfo, nullptr, &*vkImage),
                                "vkCreateImage"));
        sharedTextureMemory->mVkImage =
            AcquireRef(new RefCountedVkHandle<VkImage>(device, vkImage));
    }

    // Import the memoryFD as VkDeviceMemory and bind to the VkImage.
    {
        VkMemoryRequirements requirements;
        device->fn.GetImageMemoryRequirements(device->GetVkDevice(),
                                              sharedTextureMemory->mVkImage->Get(), &requirements);
        DAWN_INVALID_IF(requirements.size > descriptor->allocationSize,
                        "Required texture memory size (%u) is larger than the memory fd "
                        "allocation size (%u).",
                        requirements.size, descriptor->allocationSize);

        SystemHandle memoryFD;
        DAWN_TRY_ASSIGN(memoryFD, SystemHandle::Duplicate(descriptor->memoryFD));

        VkMemoryDedicatedAllocateInfo dedicatedAllocateInfo{};
        dedicatedAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
        dedicatedAllocateInfo.image = sharedTextureMemory->mVkImage->Get();

        VkImportMemoryFdInfoKHR importMemoryFdInfo{};
        importMemoryFdInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR;
        importMemoryFdInfo.handleType = handleType;
        importMemoryFdInfo.fd = memoryFD.Get();

        VkMemoryAllocateInfo memoryAllocateInfo = {};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.allocationSize = descriptor->allocationSize;
        memoryAllocateInfo.memoryTypeIndex = descriptor->memoryTypeIndex;

        // Import as VkDeviceMemory
        VkDeviceMemory vkDeviceMemory;
        if (descriptor->dedicatedAllocation) {
            DAWN_TRY_ASSIGN(vkDeviceMemory,
                            AllocateDeviceMemory(device, &memoryAllocateInfo, &importMemoryFdInfo,
                                                 &dedicatedAllocateInfo));
        } else {
            DAWN_TRY_ASSIGN(vkDeviceMemory,
                            AllocateDeviceMemory(device, &memoryAllocateInfo, &importMemoryFdInfo));
        }

        memoryFD.Detach();  // Ownership transfered to the VkDeviceMemory.
        sharedTextureMemory->mVkDeviceMemory =
            AcquireRef(new RefCountedVkHandle<VkDeviceMemory>(device, vkDeviceMemory));

        // Bind the VkImage to the memory.
        DAWN_TRY(CheckVkSuccess(
            device->fn.BindImageMemory(vkDevice, sharedTextureMemory->mVkImage->Get(),
                                       sharedTextureMemory->mVkDeviceMemory->Get(), 0),
            "vkBindImageMemory"));
    }
    return sharedTextureMemory;
#else
    DAWN_UNREACHABLE();
#endif  // DAWN_PLATFORM_IS(POSIX)
}

// static
Ref<SharedTextureMemory> SharedTextureMemory::Create(
    Device* device,
    StringView label,
    const SharedTextureMemoryProperties& properties,
    uint32_t queueFamilyIndex) {
    Ref<SharedTextureMemory> sharedTextureMemory =
        AcquireRef(new SharedTextureMemory(device, label, properties, queueFamilyIndex));
    sharedTextureMemory->Initialize();
    return sharedTextureMemory;
}

SharedTextureMemory::SharedTextureMemory(Device* device,
                                         StringView label,
                                         const SharedTextureMemoryProperties& properties,
                                         uint32_t queueFamilyIndex)
    : SharedTextureMemoryBase(device, label, properties), mQueueFamilyIndex(queueFamilyIndex) {}

RefCountedVkHandle<VkDeviceMemory>* SharedTextureMemory::GetVkDeviceMemory() const {
    return mVkDeviceMemory.Get();
}

RefCountedVkHandle<VkImage>* SharedTextureMemory::GetVkImage() const {
    return mVkImage.Get();
}

uint32_t SharedTextureMemory::GetQueueFamilyIndex() const {
    return mQueueFamilyIndex;
}

void SharedTextureMemory::DestroyImpl() {
    mVkImage = nullptr;
    mVkDeviceMemory = nullptr;
}

ResultOrError<Ref<TextureBase>> SharedTextureMemory::CreateTextureImpl(
    const UnpackedPtr<TextureDescriptor>& descriptor) {
    return SharedTexture::Create(this, descriptor);
}

MaybeError SharedTextureMemory::BeginAccessImpl(
    TextureBase* texture,
    const UnpackedPtr<BeginAccessDescriptor>& descriptor) {
    // TODO(dawn/2276): support concurrent read access.
    DAWN_INVALID_IF(descriptor->concurrentRead, "Vulkan backend doesn't support concurrent read.");
    DAWN_INVALID_IF(
        texture->GetFormat().format == wgpu::TextureFormat::External && !descriptor->initialized,
        "BeginAccess with Texture format (%s) must be initialized", texture->GetFormat().format);

    wgpu::SType type;
    DAWN_TRY_ASSIGN(
        type, (descriptor.ValidateBranches<Branch<SharedTextureMemoryVkImageLayoutBeginState>>()));
    DAWN_ASSERT(type == wgpu::SType::SharedTextureMemoryVkImageLayoutBeginState);

    auto vkLayoutBeginState = descriptor.Get<SharedTextureMemoryVkImageLayoutBeginState>();
    DAWN_ASSERT(vkLayoutBeginState != nullptr);

    for (size_t i = 0; i < descriptor->fenceCount; ++i) {
        // All fences are backed by binary semaphores.
        DAWN_INVALID_IF(descriptor->signaledValues[i] != 1, "%s signaled value (%u) was not 1.",
                        descriptor->fences[i], descriptor->signaledValues[i]);
    }
    static_cast<SharedTexture*>(texture)->SetPendingAcquire(
        static_cast<VkImageLayout>(vkLayoutBeginState->oldLayout),
        static_cast<VkImageLayout>(vkLayoutBeginState->newLayout));
    return {};
}

#if DAWN_PLATFORM_IS(FUCHSIA) || DAWN_PLATFORM_IS(LINUX)
ResultOrError<FenceAndSignalValue> SharedTextureMemory::EndAccessImpl(
    TextureBase* texture,
    ExecutionSerial lastUsageSerial,
    UnpackedPtr<EndAccessState>& state) {
    wgpu::SType type;
    DAWN_TRY_ASSIGN(type,
                    (state.ValidateBranches<Branch<SharedTextureMemoryVkImageLayoutEndState>>()));
    DAWN_ASSERT(type == wgpu::SType::SharedTextureMemoryVkImageLayoutEndState);

    auto vkLayoutEndState = state.Get<SharedTextureMemoryVkImageLayoutEndState>();
    DAWN_ASSERT(vkLayoutEndState != nullptr);

#if DAWN_PLATFORM_IS(FUCHSIA)
    DAWN_INVALID_IF(!GetDevice()->HasFeature(Feature::SharedFenceVkSemaphoreZirconHandle),
                    "Required feature (%s) for %s is missing.",
                    wgpu::FeatureName::SharedFenceVkSemaphoreZirconHandle,
                    wgpu::SharedFenceType::VkSemaphoreZirconHandle);
#elif DAWN_PLATFORM_IS(LINUX)
    DAWN_INVALID_IF(!GetDevice()->HasFeature(Feature::SharedFenceSyncFD) &&
                        !GetDevice()->HasFeature(Feature::SharedFenceVkSemaphoreOpaqueFD),
                    "Required feature (%s or %s) for %s or %s is missing.",
                    wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD,
                    wgpu::FeatureName::SharedFenceSyncFD,
                    wgpu::SharedFenceType::VkSemaphoreOpaqueFD, wgpu::SharedFenceType::SyncFD);
#endif

    SystemHandle handle;
    {
        ExternalSemaphoreHandle semaphoreHandle;
        VkImageLayout releasedOldLayout;
        VkImageLayout releasedNewLayout;
        DAWN_TRY(static_cast<SharedTexture*>(texture)->EndAccess(
            &semaphoreHandle, &releasedOldLayout, &releasedNewLayout));
        // Handle is acquired from the texture so we need to make sure to close it.
        // TODO(dawn:1745): Consider using one event per submit that is tracked by the
        // CommandRecordingContext so that we don't need to create one handle per texture,
        // and so we don't need to acquire it here to close it.
        handle = SystemHandle::Acquire(semaphoreHandle);
        vkLayoutEndState->oldLayout = releasedOldLayout;
        vkLayoutEndState->newLayout = releasedNewLayout;
    }

    Ref<SharedFence> fence;

#if DAWN_PLATFORM_IS(FUCHSIA)
    SharedFenceVkSemaphoreZirconHandleDescriptor desc;
    desc.handle = handle.Get();

    DAWN_TRY_ASSIGN(fence,
                    SharedFence::Create(ToBackend(GetDevice()), "Internal VkSemaphore", &desc));
#elif DAWN_PLATFORM_IS(LINUX)
    if (GetDevice()->HasFeature(Feature::SharedFenceSyncFD)) {
        SharedFenceSyncFDDescriptor desc;
        desc.handle = handle.Get();

        DAWN_TRY_ASSIGN(fence,
                        SharedFence::Create(ToBackend(GetDevice()), "Internal VkSemaphore", &desc));
    } else {
        SharedFenceVkSemaphoreOpaqueFDDescriptor desc;
        desc.handle = handle.Get();

        DAWN_TRY_ASSIGN(fence,
                        SharedFence::Create(ToBackend(GetDevice()), "Internal VkSemaphore", &desc));
    }
#endif
    // All semaphores are binary semaphores.
    return FenceAndSignalValue{std::move(fence), 1};
}

#else  // DAWN_PLATFORM_IS(FUCHSIA) || DAWN_PLATFORM_IS(LINUX)

ResultOrError<FenceAndSignalValue> SharedTextureMemory::EndAccessImpl(
    TextureBase* texture,
    ExecutionSerial lastUsageSerial,
    UnpackedPtr<EndAccessState>& state) {
    return DAWN_VALIDATION_ERROR("No shared fence features supported.");
}

#endif  // DAWN_PLATFORM_IS(FUCHSIA) || DAWN_PLATFORM_IS(LINUX)

MaybeError SharedTextureMemory::GetChainedProperties(
    UnpackedPtr<SharedTextureMemoryProperties>& properties) const {
    auto ahbProperties = properties.Get<SharedTextureMemoryAHardwareBufferProperties>();

    if (!ahbProperties) {
        return {};
    }

    if (ahbProperties->yCbCrInfo.nextInChain) {
        return DAWN_VALIDATION_ERROR(
            "yCBCrInfo field of SharedTextureMemoryAHardwareBufferProperties has a chained "
            "struct.");
    }

    ahbProperties->yCbCrInfo = mYCbCrAHBInfo;

    return {};
}

}  // namespace dawn::native::vulkan
