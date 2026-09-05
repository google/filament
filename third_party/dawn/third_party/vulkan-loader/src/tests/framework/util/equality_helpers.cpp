/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and/or associated documentation files (the "Materials"), to
 * deal in the Materials without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Materials, and to permit persons to whom the Materials are
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE
 * USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 */

#include "equality_helpers.h"

bool operator==(const VkExtent3D& a, const VkExtent3D& b) {
    return a.width == b.width && a.height == b.height && a.depth == b.depth;
}
bool operator!=(const VkExtent3D& a, const VkExtent3D& b) { return !(a == b); }

bool operator==(const VkQueueFamilyProperties& a, const VkQueueFamilyProperties& b) {
    return a.minImageTransferGranularity == b.minImageTransferGranularity && a.queueCount == b.queueCount &&
           a.queueFlags == b.queueFlags && a.timestampValidBits == b.timestampValidBits;
}
bool operator!=(const VkQueueFamilyProperties& a, const VkQueueFamilyProperties& b) { return !(a == b); }

bool operator==(const VkQueueFamilyProperties& a, const VkQueueFamilyProperties2& b) { return a == b.queueFamilyProperties; }
bool operator!=(const VkQueueFamilyProperties& a, const VkQueueFamilyProperties2& b) { return a != b.queueFamilyProperties; }

bool operator==(const VkLayerProperties& a, const VkLayerProperties& b) {
    return string_eq(a.layerName, b.layerName, 256) && string_eq(a.description, b.description, 256) &&
           a.implementationVersion == b.implementationVersion && a.specVersion == b.specVersion;
}
bool operator!=(const VkLayerProperties& a, const VkLayerProperties& b) { return !(a == b); }

bool operator==(const VkExtensionProperties& a, const VkExtensionProperties& b) {
    return string_eq(a.extensionName, b.extensionName, 256) && a.specVersion == b.specVersion;
}
bool operator!=(const VkExtensionProperties& a, const VkExtensionProperties& b) { return !(a == b); }

bool operator==(const VkPhysicalDeviceFeatures& feats1, const VkPhysicalDeviceFeatures2& feats2) {
    return feats1.robustBufferAccess == feats2.features.robustBufferAccess &&
           feats1.fullDrawIndexUint32 == feats2.features.fullDrawIndexUint32 &&
           feats1.imageCubeArray == feats2.features.imageCubeArray && feats1.independentBlend == feats2.features.independentBlend &&
           feats1.geometryShader == feats2.features.geometryShader &&
           feats1.tessellationShader == feats2.features.tessellationShader &&
           feats1.sampleRateShading == feats2.features.sampleRateShading && feats1.dualSrcBlend == feats2.features.dualSrcBlend &&
           feats1.logicOp == feats2.features.logicOp && feats1.multiDrawIndirect == feats2.features.multiDrawIndirect &&
           feats1.drawIndirectFirstInstance == feats2.features.drawIndirectFirstInstance &&
           feats1.depthClamp == feats2.features.depthClamp && feats1.depthBiasClamp == feats2.features.depthBiasClamp &&
           feats1.fillModeNonSolid == feats2.features.fillModeNonSolid && feats1.depthBounds == feats2.features.depthBounds &&
           feats1.wideLines == feats2.features.wideLines && feats1.largePoints == feats2.features.largePoints &&
           feats1.alphaToOne == feats2.features.alphaToOne && feats1.multiViewport == feats2.features.multiViewport &&
           feats1.samplerAnisotropy == feats2.features.samplerAnisotropy &&
           feats1.textureCompressionETC2 == feats2.features.textureCompressionETC2 &&
           feats1.textureCompressionASTC_LDR == feats2.features.textureCompressionASTC_LDR &&
           feats1.textureCompressionBC == feats2.features.textureCompressionBC &&
           feats1.occlusionQueryPrecise == feats2.features.occlusionQueryPrecise &&
           feats1.pipelineStatisticsQuery == feats2.features.pipelineStatisticsQuery &&
           feats1.vertexPipelineStoresAndAtomics == feats2.features.vertexPipelineStoresAndAtomics &&
           feats1.fragmentStoresAndAtomics == feats2.features.fragmentStoresAndAtomics &&
           feats1.shaderTessellationAndGeometryPointSize == feats2.features.shaderTessellationAndGeometryPointSize &&
           feats1.shaderImageGatherExtended == feats2.features.shaderImageGatherExtended &&
           feats1.shaderStorageImageExtendedFormats == feats2.features.shaderStorageImageExtendedFormats &&
           feats1.shaderStorageImageMultisample == feats2.features.shaderStorageImageMultisample &&
           feats1.shaderStorageImageReadWithoutFormat == feats2.features.shaderStorageImageReadWithoutFormat &&
           feats1.shaderStorageImageWriteWithoutFormat == feats2.features.shaderStorageImageWriteWithoutFormat &&
           feats1.shaderUniformBufferArrayDynamicIndexing == feats2.features.shaderUniformBufferArrayDynamicIndexing &&
           feats1.shaderSampledImageArrayDynamicIndexing == feats2.features.shaderSampledImageArrayDynamicIndexing &&
           feats1.shaderStorageBufferArrayDynamicIndexing == feats2.features.shaderStorageBufferArrayDynamicIndexing &&
           feats1.shaderStorageImageArrayDynamicIndexing == feats2.features.shaderStorageImageArrayDynamicIndexing &&
           feats1.shaderClipDistance == feats2.features.shaderClipDistance &&
           feats1.shaderCullDistance == feats2.features.shaderCullDistance &&
           feats1.shaderFloat64 == feats2.features.shaderFloat64 && feats1.shaderInt64 == feats2.features.shaderInt64 &&
           feats1.shaderInt16 == feats2.features.shaderInt16 &&
           feats1.shaderResourceResidency == feats2.features.shaderResourceResidency &&
           feats1.shaderResourceMinLod == feats2.features.shaderResourceMinLod &&
           feats1.sparseBinding == feats2.features.sparseBinding &&
           feats1.sparseResidencyBuffer == feats2.features.sparseResidencyBuffer &&
           feats1.sparseResidencyImage2D == feats2.features.sparseResidencyImage2D &&
           feats1.sparseResidencyImage3D == feats2.features.sparseResidencyImage3D &&
           feats1.sparseResidency2Samples == feats2.features.sparseResidency2Samples &&
           feats1.sparseResidency4Samples == feats2.features.sparseResidency4Samples &&
           feats1.sparseResidency8Samples == feats2.features.sparseResidency8Samples &&
           feats1.sparseResidency16Samples == feats2.features.sparseResidency16Samples &&
           feats1.sparseResidencyAliased == feats2.features.sparseResidencyAliased &&
           feats1.variableMultisampleRate == feats2.features.variableMultisampleRate &&
           feats1.inheritedQueries == feats2.features.inheritedQueries;
}

bool operator==(const VkPhysicalDeviceMemoryProperties& props1, const VkPhysicalDeviceMemoryProperties2& props2) {
    bool equal = true;
    equal = equal && props1.memoryTypeCount == props2.memoryProperties.memoryTypeCount;
    equal = equal && props1.memoryHeapCount == props2.memoryProperties.memoryHeapCount;
    for (uint32_t i = 0; i < props1.memoryHeapCount; ++i) {
        equal = equal && props1.memoryHeaps[i].size == props2.memoryProperties.memoryHeaps[i].size;
        equal = equal && props1.memoryHeaps[i].flags == props2.memoryProperties.memoryHeaps[i].flags;
    }
    for (uint32_t i = 0; i < props1.memoryTypeCount; ++i) {
        equal = equal && props1.memoryTypes[i].propertyFlags == props2.memoryProperties.memoryTypes[i].propertyFlags;
        equal = equal && props1.memoryTypes[i].heapIndex == props2.memoryProperties.memoryTypes[i].heapIndex;
    }
    return equal;
}
bool operator==(const VkSparseImageFormatProperties& props1, const VkSparseImageFormatProperties& props2) {
    return props1.aspectMask == props2.aspectMask && props1.imageGranularity.width == props2.imageGranularity.width &&
           props1.imageGranularity.height == props2.imageGranularity.height &&
           props1.imageGranularity.depth == props2.imageGranularity.depth && props1.flags == props2.flags;
}
bool operator==(const VkSparseImageFormatProperties& props1, const VkSparseImageFormatProperties2& props2) {
    return props1 == props2.properties;
}
bool operator==(const VkExternalMemoryProperties& props1, const VkExternalMemoryProperties& props2) {
    return props1.externalMemoryFeatures == props2.externalMemoryFeatures &&
           props1.exportFromImportedHandleTypes == props2.exportFromImportedHandleTypes &&
           props1.compatibleHandleTypes == props2.compatibleHandleTypes;
}
bool operator==(const VkExternalSemaphoreProperties& props1, const VkExternalSemaphoreProperties& props2) {
    return props1.externalSemaphoreFeatures == props2.externalSemaphoreFeatures &&
           props1.exportFromImportedHandleTypes == props2.exportFromImportedHandleTypes &&
           props1.compatibleHandleTypes == props2.compatibleHandleTypes;
}
bool operator==(const VkExternalFenceProperties& props1, const VkExternalFenceProperties& props2) {
    return props1.externalFenceFeatures == props2.externalFenceFeatures &&
           props1.exportFromImportedHandleTypes == props2.exportFromImportedHandleTypes &&
           props1.compatibleHandleTypes == props2.compatibleHandleTypes;
}
bool operator==(const VkSurfaceCapabilitiesKHR& props1, const VkSurfaceCapabilitiesKHR& props2) {
    return props1.minImageCount == props2.minImageCount && props1.maxImageCount == props2.maxImageCount &&
           props1.currentExtent.width == props2.currentExtent.width && props1.currentExtent.height == props2.currentExtent.height &&
           props1.minImageExtent.width == props2.minImageExtent.width &&
           props1.minImageExtent.height == props2.minImageExtent.height &&
           props1.maxImageExtent.width == props2.maxImageExtent.width &&
           props1.maxImageExtent.height == props2.maxImageExtent.height &&
           props1.maxImageArrayLayers == props2.maxImageArrayLayers && props1.supportedTransforms == props2.supportedTransforms &&
           props1.currentTransform == props2.currentTransform && props1.supportedCompositeAlpha == props2.supportedCompositeAlpha &&
           props1.supportedUsageFlags == props2.supportedUsageFlags;
}
bool operator==(const VkSurfacePresentScalingCapabilitiesEXT& caps1, const VkSurfacePresentScalingCapabilitiesEXT& caps2) {
    return caps1.supportedPresentScaling == caps2.supportedPresentScaling &&
           caps1.supportedPresentGravityX == caps2.supportedPresentGravityX &&
           caps1.supportedPresentGravityY == caps2.supportedPresentGravityY &&
           caps1.minScaledImageExtent.width == caps2.minScaledImageExtent.width &&
           caps1.minScaledImageExtent.height == caps2.minScaledImageExtent.height &&
           caps1.maxScaledImageExtent.width == caps2.maxScaledImageExtent.width &&
           caps1.maxScaledImageExtent.height == caps2.maxScaledImageExtent.height;
}
bool operator==(const VkSurfaceFormatKHR& format1, const VkSurfaceFormatKHR& format2) {
    return format1.format == format2.format && format1.colorSpace == format2.colorSpace;
}
bool operator==(const VkSurfaceFormatKHR& format1, const VkSurfaceFormat2KHR& format2) { return format1 == format2.surfaceFormat; }
bool operator==(const VkDisplayPropertiesKHR& props1, const VkDisplayPropertiesKHR& props2) {
    return props1.display == props2.display && props1.physicalDimensions.width == props2.physicalDimensions.width &&
           props1.physicalDimensions.height == props2.physicalDimensions.height &&
           props1.physicalResolution.width == props2.physicalResolution.width &&
           props1.physicalResolution.height == props2.physicalResolution.height &&
           props1.supportedTransforms == props2.supportedTransforms && props1.planeReorderPossible == props2.planeReorderPossible &&
           props1.persistentContent == props2.persistentContent;
}
bool operator==(const VkDisplayPropertiesKHR& props1, const VkDisplayProperties2KHR& props2) {
    return props1 == props2.displayProperties;
}
bool operator==(const VkDisplayModePropertiesKHR& disp1, const VkDisplayModePropertiesKHR& disp2) {
    return disp1.displayMode == disp2.displayMode && disp1.parameters.visibleRegion.width == disp2.parameters.visibleRegion.width &&
           disp1.parameters.visibleRegion.height == disp2.parameters.visibleRegion.height &&
           disp1.parameters.refreshRate == disp2.parameters.refreshRate;
}

bool operator==(const VkDisplayModePropertiesKHR& disp1, const VkDisplayModeProperties2KHR& disp2) {
    return disp1 == disp2.displayModeProperties;
}
bool operator==(const VkDisplayPlaneCapabilitiesKHR& caps1, const VkDisplayPlaneCapabilitiesKHR& caps2) {
    return caps1.supportedAlpha == caps2.supportedAlpha && caps1.minSrcPosition.x == caps2.minSrcPosition.x &&
           caps1.minSrcPosition.y == caps2.minSrcPosition.y && caps1.maxSrcPosition.x == caps2.maxSrcPosition.x &&
           caps1.maxSrcPosition.y == caps2.maxSrcPosition.y && caps1.minSrcExtent.width == caps2.minSrcExtent.width &&
           caps1.minSrcExtent.height == caps2.minSrcExtent.height && caps1.maxSrcExtent.width == caps2.maxSrcExtent.width &&
           caps1.maxSrcExtent.height == caps2.maxSrcExtent.height && caps1.minDstPosition.x == caps2.minDstPosition.x &&
           caps1.minDstPosition.y == caps2.minDstPosition.y && caps1.maxDstPosition.x == caps2.maxDstPosition.x &&
           caps1.maxDstPosition.y == caps2.maxDstPosition.y && caps1.minDstExtent.width == caps2.minDstExtent.width &&
           caps1.minDstExtent.height == caps2.minDstExtent.height && caps1.maxDstExtent.width == caps2.maxDstExtent.width &&
           caps1.maxDstExtent.height == caps2.maxDstExtent.height;
}

bool operator==(const VkDisplayPlaneCapabilitiesKHR& caps1, const VkDisplayPlaneCapabilities2KHR& caps2) {
    return caps1 == caps2.capabilities;
}
bool operator==(const VkDisplayPlanePropertiesKHR& props1, const VkDisplayPlanePropertiesKHR& props2) {
    return props1.currentDisplay == props2.currentDisplay && props1.currentStackIndex == props2.currentStackIndex;
}
bool operator==(const VkDisplayPlanePropertiesKHR& props1, const VkDisplayPlaneProperties2KHR& props2) {
    return props1 == props2.displayPlaneProperties;
}
bool operator==(const VkExtent2D& ext1, const VkExtent2D& ext2) { return ext1.height == ext2.height && ext1.width == ext2.width; }
