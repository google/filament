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

#include "dawn/native/vulkan/UtilsVulkan.h"

#include "dawn/common/Assert.h"
#include "dawn/native/EnumMaskIterator.h"
#include "dawn/native/Format.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/ShaderModule.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/Forward.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/native/vulkan/VulkanFunctions.h"

namespace dawn::native::vulkan {

constexpr char kDeviceDebugPrefix[] = "DawnDbg=";
constexpr char kDeviceDebugSeparator[] = ";";

#define VK_OBJECT_TYPE_GETTER(object, objectType)         \
    template <>                                           \
    VkObjectType GetVkObjectType<object>(object handle) { \
        return objectType;                                \
    }

VK_OBJECT_TYPE_GETTER(VkBuffer, VK_OBJECT_TYPE_BUFFER)
VK_OBJECT_TYPE_GETTER(VkDescriptorSetLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT)
VK_OBJECT_TYPE_GETTER(VkDescriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET)
VK_OBJECT_TYPE_GETTER(VkPipeline, VK_OBJECT_TYPE_PIPELINE)
VK_OBJECT_TYPE_GETTER(VkPipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT)
VK_OBJECT_TYPE_GETTER(VkQueryPool, VK_OBJECT_TYPE_QUERY_POOL)
VK_OBJECT_TYPE_GETTER(VkSampler, VK_OBJECT_TYPE_SAMPLER)
VK_OBJECT_TYPE_GETTER(VkShaderModule, VK_OBJECT_TYPE_SHADER_MODULE)
VK_OBJECT_TYPE_GETTER(VkImage, VK_OBJECT_TYPE_IMAGE)
VK_OBJECT_TYPE_GETTER(VkImageView, VK_OBJECT_TYPE_IMAGE_VIEW)

#undef VK_OBJECT_TYPE_GETTER

VkCompareOp ToVulkanCompareOp(wgpu::CompareFunction op) {
    switch (op) {
        case wgpu::CompareFunction::Never:
            return VK_COMPARE_OP_NEVER;
        case wgpu::CompareFunction::Less:
            return VK_COMPARE_OP_LESS;
        case wgpu::CompareFunction::LessEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case wgpu::CompareFunction::Greater:
            return VK_COMPARE_OP_GREATER;
        case wgpu::CompareFunction::GreaterEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case wgpu::CompareFunction::Equal:
            return VK_COMPARE_OP_EQUAL;
        case wgpu::CompareFunction::NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case wgpu::CompareFunction::Always:
            return VK_COMPARE_OP_ALWAYS;

        case wgpu::CompareFunction::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

VkFilter ToVulkanSamplerFilter(wgpu::FilterMode filter) {
    switch (filter) {
        case wgpu::FilterMode::Linear:
            return VK_FILTER_LINEAR;
        case wgpu::FilterMode::Nearest:
            return VK_FILTER_NEAREST;
        case wgpu::FilterMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

// Convert Dawn texture aspects to  Vulkan texture aspect flags
VkImageAspectFlags VulkanAspectMask(const Aspect& aspects) {
    VkImageAspectFlags flags = 0;
    for (Aspect aspect : IterateEnumMask(aspects)) {
        switch (aspect) {
            case Aspect::Color:
                flags |= VK_IMAGE_ASPECT_COLOR_BIT;
                break;
            case Aspect::Depth:
                flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
                break;
            case Aspect::Stencil:
                flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
                break;

            case Aspect::CombinedDepthStencil:
                flags |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                break;

            case Aspect::Plane0:
                flags |= VK_IMAGE_ASPECT_PLANE_0_BIT;
                break;
            case Aspect::Plane1:
                flags |= VK_IMAGE_ASPECT_PLANE_1_BIT;
                break;
            case Aspect::Plane2:
                flags |= VK_IMAGE_ASPECT_PLANE_2_BIT;
                break;

            case Aspect::None:
                DAWN_UNREACHABLE();
        }
    }
    return flags;
}

// Vulkan SPEC requires the source/destination region specified by each element of
// pRegions must be a region that is contained within srcImage/dstImage. Here the size of
// the image refers to the virtual size, while Dawn validates texture copy extent with the
// physical size, so we need to re-calculate the texture copy extent to ensure it should fit
// in the virtual size of the subresource.
Extent3D ComputeTextureCopyExtent(const TextureCopy& textureCopy, const Extent3D& copySize) {
    Extent3D validTextureCopyExtent = copySize;
    const TextureBase* texture = textureCopy.texture.Get();
    Extent3D virtualSizeAtLevel =
        texture->GetMipLevelSingleSubresourceVirtualSize(textureCopy.mipLevel, textureCopy.aspect);
    DAWN_ASSERT(textureCopy.origin.x <= virtualSizeAtLevel.width);
    DAWN_ASSERT(textureCopy.origin.y <= virtualSizeAtLevel.height);
    if (copySize.width > virtualSizeAtLevel.width - textureCopy.origin.x) {
        DAWN_ASSERT(texture->GetFormat().isCompressed);
        validTextureCopyExtent.width = virtualSizeAtLevel.width - textureCopy.origin.x;
    }
    if (copySize.height > virtualSizeAtLevel.height - textureCopy.origin.y) {
        DAWN_ASSERT(texture->GetFormat().isCompressed);
        validTextureCopyExtent.height = virtualSizeAtLevel.height - textureCopy.origin.y;
    }

    return validTextureCopyExtent;
}

VkBufferImageCopy ComputeBufferImageCopyRegion(const BufferCopy& bufferCopy,
                                               const TextureCopy& textureCopy,
                                               const Extent3D& copySize) {
    TexelCopyBufferLayout passDataLayout;
    passDataLayout.offset = bufferCopy.offset;
    passDataLayout.rowsPerImage = bufferCopy.rowsPerImage;
    passDataLayout.bytesPerRow = bufferCopy.bytesPerRow;
    return ComputeBufferImageCopyRegion(passDataLayout, textureCopy, copySize);
}

VkBufferImageCopy ComputeBufferImageCopyRegion(const TexelCopyBufferLayout& dataLayout,
                                               const TextureCopy& textureCopy,
                                               const Extent3D& copySize) {
    const Texture* texture = ToBackend(textureCopy.texture.Get());

    VkBufferImageCopy region;

    region.bufferOffset = dataLayout.offset;
    // In Vulkan the row length is in texels while it is in bytes for Dawn
    const TexelBlockInfo& blockInfo = texture->GetFormat().GetAspectInfo(textureCopy.aspect).block;
    DAWN_ASSERT(dataLayout.bytesPerRow % blockInfo.byteSize == 0);
    region.bufferRowLength = dataLayout.bytesPerRow / blockInfo.byteSize * blockInfo.width;
    region.bufferImageHeight = dataLayout.rowsPerImage * blockInfo.height;

    region.imageSubresource.aspectMask = VulkanAspectMask(textureCopy.aspect);
    region.imageSubresource.mipLevel = textureCopy.mipLevel;

    switch (textureCopy.texture->GetDimension()) {
        case wgpu::TextureDimension::Undefined:
            DAWN_UNREACHABLE();
        case wgpu::TextureDimension::e1D:
            DAWN_ASSERT(textureCopy.origin.z == 0 && copySize.depthOrArrayLayers == 1);
            region.imageOffset.x = textureCopy.origin.x;
            region.imageOffset.y = 0;
            region.imageOffset.z = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            DAWN_ASSERT(!textureCopy.texture->GetFormat().isCompressed);
            region.imageExtent.width = copySize.width;
            region.imageExtent.height = 1;
            region.imageExtent.depth = 1;
            break;

        case wgpu::TextureDimension::e2D: {
            region.imageOffset.x = textureCopy.origin.x;
            region.imageOffset.y = textureCopy.origin.y;
            region.imageOffset.z = 0;
            region.imageSubresource.baseArrayLayer = textureCopy.origin.z;
            region.imageSubresource.layerCount = copySize.depthOrArrayLayers;

            Extent3D imageExtent = ComputeTextureCopyExtent(textureCopy, copySize);
            region.imageExtent.width = imageExtent.width;
            region.imageExtent.height = imageExtent.height;
            region.imageExtent.depth = 1;
            break;
        }

        case wgpu::TextureDimension::e3D: {
            region.imageOffset.x = textureCopy.origin.x;
            region.imageOffset.y = textureCopy.origin.y;
            region.imageOffset.z = textureCopy.origin.z;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            DAWN_ASSERT(!textureCopy.texture->GetFormat().isCompressed);
            region.imageExtent.width = copySize.width;
            region.imageExtent.height = copySize.height;
            region.imageExtent.depth = copySize.depthOrArrayLayers;
            break;
        }
    }

    return region;
}

void SetDebugNameInternal(Device* device,
                          VkObjectType objectType,
                          uint64_t objectHandle,
                          const char* prefix,
                          std::string_view label) {
    if (!device->IsToggleEnabled(Toggle::UseUserDefinedLabelsInBackend)) {
        return;
    }

    if (!objectHandle || !device->GetVkDevice()) {
        return;
    }

    if (device->GetGlobalInfo().HasExt(InstanceExt::DebugUtils)) {
        VkDebugUtilsObjectNameInfoEXT objectNameInfo;
        objectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        objectNameInfo.pNext = nullptr;
        objectNameInfo.objectType = objectType;
        objectNameInfo.objectHandle = objectHandle;

        std::ostringstream objectNameStream;
        // Prefix with the device's message ID so that if this label appears in a validation
        // message it can be parsed out and the message can be associated with the right device.
        objectNameStream << device->GetDebugPrefix() << kDeviceDebugSeparator << prefix;
        if (!label.empty()) {
            objectNameStream << "_" << label;
        }

        std::string objectName = objectNameStream.str();
        objectNameInfo.pObjectName = objectName.c_str();
        device->fn.SetDebugUtilsObjectNameEXT(device->GetVkDevice(), &objectNameInfo);
    }
}

std::string GetNextDeviceDebugPrefix() {
    static uint64_t nextDeviceDebugId = 1;
    std::ostringstream objectName;
    objectName << kDeviceDebugPrefix << nextDeviceDebugId++;
    return objectName.str();
}

std::string GetDeviceDebugPrefixFromDebugName(const char* debugName) {
    if (debugName == nullptr) {
        return {};
    }

    if (strncmp(debugName, kDeviceDebugPrefix, sizeof(kDeviceDebugPrefix) - 1) != 0) {
        return {};
    }

    const char* separator = strstr(debugName + sizeof(kDeviceDebugPrefix), kDeviceDebugSeparator);
    if (separator == nullptr) {
        return {};
    }

    size_t length = separator - debugName;
    return std::string(debugName, length);
}

std::vector<VkDrmFormatModifierPropertiesEXT> GetFormatModifierProps(
    const VulkanFunctions& fn,
    VkPhysicalDevice vkPhysicalDevice,
    VkFormat format) {
    VkFormatProperties2 formatProps = {};
    formatProps.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    PNextChainBuilder formatPropsChain(&formatProps);

    // Obtain the list of Linux DRM format modifiers compatible with a VkFormat
    VkDrmFormatModifierPropertiesListEXT formatModifierPropsList = {};
    formatModifierPropsList.drmFormatModifierCount = 0;
    formatModifierPropsList.pDrmFormatModifierProperties = nullptr;
    formatPropsChain.Add(&formatModifierPropsList,
                         VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT);

    fn.GetPhysicalDeviceFormatProperties2(vkPhysicalDevice, format, &formatProps);

    const uint32_t modifierCount = formatModifierPropsList.drmFormatModifierCount;

    std::vector<VkDrmFormatModifierPropertiesEXT> formatModifierPropsVector;
    formatModifierPropsVector.resize(modifierCount);
    formatModifierPropsList.pDrmFormatModifierProperties = formatModifierPropsVector.data();

    fn.GetPhysicalDeviceFormatProperties2(vkPhysicalDevice, format, &formatProps);
    return formatModifierPropsVector;
}

ResultOrError<VkDrmFormatModifierPropertiesEXT> GetFormatModifierProps(
    const VulkanFunctions& fn,
    VkPhysicalDevice vkPhysicalDevice,
    VkFormat format,
    uint64_t modifier) {
    std::vector<VkDrmFormatModifierPropertiesEXT> formatModifierPropsVector =
        GetFormatModifierProps(fn, vkPhysicalDevice, format);

    // Find the modifier props that match the modifier, and return them.
    for (const auto& props : formatModifierPropsVector) {
        if (props.drmFormatModifier == modifier) {
            return VkDrmFormatModifierPropertiesEXT{props};
        }
    }
    return DAWN_VALIDATION_ERROR("DRM format modifier %u not supported.", modifier);
}

ResultOrError<VkSamplerYcbcrConversion> CreateSamplerYCbCrConversionCreateInfo(
    YCbCrVkDescriptor yCbCrDescriptor,
    Device* device) {
    uint64_t externalFormat = yCbCrDescriptor.externalFormat;
    VkFormat vulkanFormat = static_cast<VkFormat>(yCbCrDescriptor.vkFormat);
    DAWN_INVALID_IF((externalFormat == 0 && vulkanFormat == VK_FORMAT_UNDEFINED),
                    "Both VkFormat and VkExternalFormatANDROID are undefined.");

    VkComponentMapping vulkanComponent;
    vulkanComponent.r = static_cast<VkComponentSwizzle>(yCbCrDescriptor.vkComponentSwizzleRed);
    vulkanComponent.g = static_cast<VkComponentSwizzle>(yCbCrDescriptor.vkComponentSwizzleGreen);
    vulkanComponent.b = static_cast<VkComponentSwizzle>(yCbCrDescriptor.vkComponentSwizzleBlue);
    vulkanComponent.a = static_cast<VkComponentSwizzle>(yCbCrDescriptor.vkComponentSwizzleAlpha);

    VkSamplerYcbcrConversionCreateInfo vulkanYCbCrCreateInfo;
    vulkanYCbCrCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    vulkanYCbCrCreateInfo.pNext = nullptr;
    vulkanYCbCrCreateInfo.format = vulkanFormat;
    vulkanYCbCrCreateInfo.ycbcrModel =
        static_cast<VkSamplerYcbcrModelConversion>(yCbCrDescriptor.vkYCbCrModel);
    vulkanYCbCrCreateInfo.ycbcrRange =
        static_cast<VkSamplerYcbcrRange>(yCbCrDescriptor.vkYCbCrRange);
    vulkanYCbCrCreateInfo.components = vulkanComponent;
    vulkanYCbCrCreateInfo.xChromaOffset =
        static_cast<VkChromaLocation>(yCbCrDescriptor.vkXChromaOffset);
    vulkanYCbCrCreateInfo.yChromaOffset =
        static_cast<VkChromaLocation>(yCbCrDescriptor.vkYChromaOffset);
    vulkanYCbCrCreateInfo.chromaFilter = ToVulkanSamplerFilter(yCbCrDescriptor.vkChromaFilter);
    vulkanYCbCrCreateInfo.forceExplicitReconstruction =
        static_cast<VkBool32>(yCbCrDescriptor.forceExplicitReconstruction);

#if DAWN_PLATFORM_IS(ANDROID)
    VkExternalFormatANDROID vulkanExternalFormat;
    // Chain VkExternalFormatANDROID only if needed.
    if (externalFormat != 0) {
        vulkanExternalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
        vulkanExternalFormat.pNext = nullptr;
        vulkanExternalFormat.externalFormat = externalFormat;

        vulkanYCbCrCreateInfo.pNext = &vulkanExternalFormat;
    }
#endif  // DAWN_PLATFORM_IS(ANDROID)

    VkSamplerYcbcrConversion samplerYCbCrConversion = VK_NULL_HANDLE;
    DAWN_TRY(CheckVkSuccess(
        device->fn.CreateSamplerYcbcrConversion(device->GetVkDevice(), &vulkanYCbCrCreateInfo,
                                                nullptr, &*samplerYCbCrConversion),
        "CreateSamplerYcbcrConversion"));

    return samplerYCbCrConversion;
}

}  // namespace dawn::native::vulkan
