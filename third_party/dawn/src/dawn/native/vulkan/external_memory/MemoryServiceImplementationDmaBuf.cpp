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

#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/native/vulkan/BackendVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/native/vulkan/external_memory/MemoryServiceImplementation.h"
#include "dawn/native/vulkan/external_memory/MemoryServiceImplementationDmaBuf.h"

namespace dawn::native::vulkan::external_memory {

namespace {

bool GetFormatModifierProps(const VulkanFunctions& fn,
                            VkPhysicalDevice vkPhysicalDevice,
                            VkFormat format,
                            uint64_t modifier,
                            VkDrmFormatModifierPropertiesEXT* formatModifierProps) {
    std::vector<VkDrmFormatModifierPropertiesEXT> formatModifierPropsVector;
    VkFormatProperties2 formatProps = {};
    formatProps.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    VkDrmFormatModifierPropertiesListEXT formatModifierPropsList = {};
    formatModifierPropsList.drmFormatModifierCount = 0;
    formatModifierPropsList.pDrmFormatModifierProperties = nullptr;

    PNextChainBuilder formatPropsChain(&formatProps);
    formatPropsChain.Add(&formatModifierPropsList,
                         VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT);

    fn.GetPhysicalDeviceFormatProperties2(vkPhysicalDevice, format, &formatProps);

    uint32_t modifierCount = formatModifierPropsList.drmFormatModifierCount;
    formatModifierPropsVector.resize(modifierCount);
    formatModifierPropsList.pDrmFormatModifierProperties = formatModifierPropsVector.data();

    fn.GetPhysicalDeviceFormatProperties2(vkPhysicalDevice, format, &formatProps);
    for (const auto& props : formatModifierPropsVector) {
        if (props.drmFormatModifier == modifier) {
            *formatModifierProps = props;
            return true;
        }
    }
    return false;
}

// Some modifiers use multiple planes (for example, see the comment for
// I915_FORMAT_MOD_Y_TILED_CCS in drm/drm_fourcc.h).
ResultOrError<uint32_t> GetModifierPlaneCount(const VulkanFunctions& fn,
                                              VkPhysicalDevice vkPhysicalDevice,
                                              VkFormat format,
                                              uint64_t modifier) {
    VkDrmFormatModifierPropertiesEXT props;
    if (GetFormatModifierProps(fn, vkPhysicalDevice, format, modifier, &props)) {
        return static_cast<uint32_t>(props.drmFormatModifierPlaneCount);
    }
    return DAWN_VALIDATION_ERROR("DRM format modifier not supported.");
}

bool IsMultiPlanarVkFormat(VkFormat format) {
    switch (format) {
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
            return true;

        default:
            return false;
    }
}

bool SupportsDisjoint(const VulkanFunctions& fn,
                      VkPhysicalDevice vkPhysicalDevice,
                      VkFormat format,
                      uint64_t modifier) {
    if (IsMultiPlanarVkFormat(format)) {
        VkDrmFormatModifierPropertiesEXT props;
        return (GetFormatModifierProps(fn, vkPhysicalDevice, format, modifier, &props) &&
                (props.drmFormatModifierTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT));
    }
    return false;
}

}  // namespace

class ServiceImplementationDmaBuf : public ServiceImplementation {
  public:
    explicit ServiceImplementationDmaBuf(Device* device)
        : ServiceImplementation(device), mSupported(CheckSupport(device->GetDeviceInfo())) {}
    ~ServiceImplementationDmaBuf() override = default;

    static bool CheckSupport(const VulkanDeviceInfo& deviceInfo) {
        return deviceInfo.HasExt(DeviceExt::ExternalMemoryFD) &&
               deviceInfo.HasExt(DeviceExt::ImageDrmFormatModifier);
    }

    bool SupportsImportMemory(VkFormat format,
                              VkImageType type,
                              VkImageTiling tiling,
                              VkImageUsageFlags usage,
                              VkImageCreateFlags flags) override {
        return mSupported && (!IsMultiPlanarVkFormat(format) ||
                              (format == VK_FORMAT_G8_B8R8_2PLANE_420_UNORM &&
                               mDevice->GetDeviceInfo().HasExt(DeviceExt::ImageFormatList)));
    }

    bool SupportsCreateImage(const ExternalImageDescriptor* descriptor,
                             VkFormat format,
                             VkImageUsageFlags usage,
                             bool* supportsDisjoint) override {
        *supportsDisjoint = false;
        // Early out before we try using extension functions
        if (!mSupported) {
            return false;
        }
        if (descriptor->GetType() != ExternalImageType::DmaBuf) {
            return false;
        }
        const ExternalImageDescriptorDmaBuf* dmaBufDescriptor =
            static_cast<const ExternalImageDescriptorDmaBuf*>(descriptor);

        // Verify plane count for the modifier.
        VkPhysicalDevice vkPhysicalDevice =
            ToBackend(mDevice->GetPhysicalDevice())->GetVkPhysicalDevice();
        uint32_t planeCount = 0;
        if (mDevice->ConsumedError(GetModifierPlaneCount(mDevice->fn, vkPhysicalDevice, format,
                                                         dmaBufDescriptor->drmModifier),
                                   &planeCount)) {
            return false;
        }
        if (planeCount == 0) {
            return false;
        }
        // Only support the NV12 multi-planar format for now.
        if (planeCount > 1 && format != VK_FORMAT_G8_B8R8_2PLANE_420_UNORM) {
            return false;
        }
        *supportsDisjoint =
            SupportsDisjoint(mDevice->fn, vkPhysicalDevice, format, dmaBufDescriptor->drmModifier);

        // Verify that the format modifier of the external memory and the requested Vulkan format
        // are actually supported together in a dma-buf import.
        VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {};
        imageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
        imageFormatInfo.format = format;
        imageFormatInfo.type = VK_IMAGE_TYPE_2D;
        imageFormatInfo.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
        imageFormatInfo.usage = usage;
        imageFormatInfo.flags = 0;
        PNextChainBuilder imageFormatInfoChain(&imageFormatInfo);

        VkPhysicalDeviceExternalImageFormatInfo externalImageFormatInfo = {};
        externalImageFormatInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
        imageFormatInfoChain.Add(&externalImageFormatInfo,
                                 VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO);

        VkPhysicalDeviceImageDrmFormatModifierInfoEXT drmModifierInfo = {};
        drmModifierInfo.drmFormatModifier = dmaBufDescriptor->drmModifier;
        drmModifierInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageFormatInfoChain.Add(
            &drmModifierInfo, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT);

        // For mutable vkimage of multi-planar format, we also need to make sure the each
        // plane's view format can be supported.
        std::array<VkFormat, 2> viewFormats;
        VkImageFormatListCreateInfo imageFormatListInfo = {};

        if (planeCount > 1) {
            DAWN_ASSERT(format == VK_FORMAT_G8_B8R8_2PLANE_420_UNORM);
            viewFormats = {VK_FORMAT_R8_UNORM, VK_FORMAT_R8G8_UNORM};
            imageFormatListInfo.viewFormatCount = 2;
            imageFormatListInfo.pViewFormats = viewFormats.data();
            imageFormatInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
            imageFormatInfoChain.Add(&imageFormatListInfo,
                                     VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO);
        }

        VkImageFormatProperties2 imageFormatProps = {};
        imageFormatProps.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
        PNextChainBuilder imageFormatPropsChain(&imageFormatProps);

        VkExternalImageFormatProperties externalImageFormatProps = {};
        imageFormatPropsChain.Add(&externalImageFormatProps,
                                  VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES);

        VkResult result = VkResult::WrapUnsafe(mDevice->fn.GetPhysicalDeviceImageFormatProperties2(
            vkPhysicalDevice, &imageFormatInfo, &imageFormatProps));
        if (result != VK_SUCCESS) {
            return false;
        }
        VkExternalMemoryFeatureFlags featureFlags =
            externalImageFormatProps.externalMemoryProperties.externalMemoryFeatures;
        return featureFlags & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT;
    }

    ResultOrError<MemoryImportParams> GetMemoryImportParams(
        const ExternalImageDescriptor* descriptor,
        VkImage image) override {
        DAWN_INVALID_IF(descriptor->GetType() != ExternalImageType::DmaBuf,
                        "ExternalImageDescriptor is not a ExternalImageDescriptorDmaBuf.");

        const ExternalImageDescriptorDmaBuf* dmaBufDescriptor =
            static_cast<const ExternalImageDescriptorDmaBuf*>(descriptor);
        VkDevice device = mDevice->GetVkDevice();

        // Get the valid memory types for the VkImage.
        VkMemoryRequirements memoryRequirements;
        mDevice->fn.GetImageMemoryRequirements(device, image, &memoryRequirements);

        VkMemoryFdPropertiesKHR fdProperties;
        fdProperties.sType = VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR;
        fdProperties.pNext = nullptr;

        // Get the valid memory types that the external memory can be imported as.
        mDevice->fn.GetMemoryFdPropertiesKHR(device, VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
                                             dmaBufDescriptor->memoryFD, &fdProperties);
        // Choose the best memory type that satisfies both the image's constraint and the
        // import's constraint.
        memoryRequirements.memoryTypeBits &= fdProperties.memoryTypeBits;
        int memoryTypeIndex = mDevice->GetResourceMemoryAllocator()->FindBestTypeIndex(
            memoryRequirements, MemoryKind::Opaque);
        DAWN_INVALID_IF(memoryTypeIndex == -1,
                        "Unable to find an appropriate memory type for import.");

        MemoryImportParams params;
        params.allocationSize = memoryRequirements.size;
        params.memoryTypeIndex = static_cast<uint32_t>(memoryTypeIndex);
        params.dedicatedAllocation = RequiresDedicatedAllocation(dmaBufDescriptor, image);
        return params;
    }

    uint32_t GetQueueFamilyIndex() override { return VK_QUEUE_FAMILY_EXTERNAL_KHR; }

    ResultOrError<VkDeviceMemory> ImportMemory(ExternalMemoryHandle handle,
                                               const MemoryImportParams& importParams,
                                               VkImage image) override {
        DAWN_INVALID_IF(handle < 0, "Importing memory with an invalid handle.");

        VkMemoryAllocateInfo memoryAllocateInfo = {};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.allocationSize = importParams.allocationSize;
        memoryAllocateInfo.memoryTypeIndex = importParams.memoryTypeIndex;
        PNextChainBuilder memoryAllocateInfoChain(&memoryAllocateInfo);

        VkImportMemoryFdInfoKHR importMemoryFdInfo;
        importMemoryFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
        importMemoryFdInfo.fd = handle;
        memoryAllocateInfoChain.Add(&importMemoryFdInfo,
                                    VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR);

        VkMemoryDedicatedAllocateInfo memoryDedicatedAllocateInfo;
        if (importParams.dedicatedAllocation) {
            memoryDedicatedAllocateInfo.image = image;
            memoryDedicatedAllocateInfo.buffer = VkBuffer{};
            memoryAllocateInfoChain.Add(&memoryDedicatedAllocateInfo,
                                        VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO);
        }

        VkDeviceMemory allocatedMemory = VK_NULL_HANDLE;
        DAWN_TRY(
            CheckVkSuccess(mDevice->fn.AllocateMemory(mDevice->GetVkDevice(), &memoryAllocateInfo,
                                                      nullptr, &*allocatedMemory),
                           "vkAllocateMemory"));
        return allocatedMemory;
    }

    ResultOrError<VkImage> CreateImage(const ExternalImageDescriptor* descriptor,
                                       const VkImageCreateInfo& baseCreateInfo) override {
        DAWN_INVALID_IF(descriptor->GetType() != ExternalImageType::DmaBuf,
                        "ExternalImageDescriptor is not a dma-buf descriptor.");

        const ExternalImageDescriptorDmaBuf* dmaBufDescriptor =
            static_cast<const ExternalImageDescriptorDmaBuf*>(descriptor);
        VkPhysicalDevice vkPhysicalDevice =
            ToBackend(mDevice->GetPhysicalDevice())->GetVkPhysicalDevice();
        VkDevice device = mDevice->GetVkDevice();

        uint32_t planeCount;
        DAWN_TRY_ASSIGN(planeCount,
                        GetModifierPlaneCount(mDevice->fn, vkPhysicalDevice, baseCreateInfo.format,
                                              dmaBufDescriptor->drmModifier));

        VkImageCreateInfo createInfo = baseCreateInfo;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        createInfo.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;

        PNextChainBuilder createInfoChain(&createInfo);

        VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {};
        externalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
        createInfoChain.Add(&externalMemoryImageCreateInfo,
                            VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO);

        VkSubresourceLayout planeLayouts[ExternalImageDescriptorDmaBuf::kMaxPlanes];
        for (uint32_t plane = 0u; plane < planeCount; ++plane) {
            planeLayouts[plane].offset = dmaBufDescriptor->planeLayouts[plane].offset;
            planeLayouts[plane].size = 0;  // VK_EXT_image_drm_format_modifier mandates size = 0.
            planeLayouts[plane].rowPitch = dmaBufDescriptor->planeLayouts[plane].stride;
            planeLayouts[plane].arrayPitch = 0;  // Not an array texture
            planeLayouts[plane].depthPitch = 0;  // Not a depth texture
        }

        VkImageDrmFormatModifierExplicitCreateInfoEXT explicitCreateInfo = {};
        explicitCreateInfo.drmFormatModifier = dmaBufDescriptor->drmModifier;
        explicitCreateInfo.drmFormatModifierPlaneCount = planeCount;
        explicitCreateInfo.pPlaneLayouts = &planeLayouts[0];

        if (planeCount > 1) {
            // For multi-planar formats, VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT specifies that a
            // VkImageView can be plane's format which might differ from the image's format.
            createInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
        }
        createInfoChain.Add(&explicitCreateInfo,
                            VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT);

        // Create a new VkImage with tiling equal to the DRM format modifier.
        VkImage image;
        DAWN_TRY(CheckVkSuccess(mDevice->fn.CreateImage(device, &createInfo, nullptr, &*image),
                                "CreateImage"));
        return image;
    }

    bool Supported() const override { return mSupported; }

  private:
    bool mSupported = false;
};

bool CheckDmaBufSupport(const VulkanDeviceInfo& deviceInfo) {
    return ServiceImplementationDmaBuf::CheckSupport(deviceInfo);
}

std::unique_ptr<ServiceImplementation> CreateDmaBufService(Device* device) {
    return std::make_unique<ServiceImplementationDmaBuf>(device);
}

}  // namespace dawn::native::vulkan::external_memory
