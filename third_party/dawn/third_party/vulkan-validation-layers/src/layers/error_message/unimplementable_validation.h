/*
 * Copyright (c) 2023-2025 LunarG, Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
// clang-format off

// This file list all VUID that are not possible to validate.
// This file should never be included, but here for searchability and statistics

const char* unimplementable_validation[] = {
    // sparseAddressSpaceSize can't be tracked in a layer
    // https://gitlab.khronos.org/vulkan/vulkan/-/issues/2403
    "VUID-vkCreateBuffer-flags-00911",

    // Some of the early extensions were not created with a feature bit. This means if the extension is used, we considered it
    // "enabled". This becomes a problem as some coniditional VUIDs depend on the Extension to be enabled, this means we are left
    // with 2 variations of the VUIDs, but only one is not possible to ever get to.
    // The following are a list of these:
    "VUID-VkSubpassDescription2-multisampledRenderToSingleSampled-06869",  // VUID-VkSubpassDescription2-multisampledRenderToSingleSampled-06872

    // This VUID cannot be validated at vkCmdEndDebugUtilsLabelEXT time. Needs spec clarification.
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5671
    "VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-01912",

    // These VUIDs cannot be validated beyond making sure the pointer is not null
    "VUID-VkMemoryToImageCopy-pHostPointer-09061", "VUID-VkImageToMemoryCopy-pHostPointer-09066"

    // these are already taken care in spirv-val for 08737
    "VUID-VkShaderModuleCreateInfo-pCode-08736", "VUID-VkShaderCreateInfoEXT-pCode-08736",
    "VUID-VkShaderModuleCreateInfo-pCode-08738", "VUID-VkShaderCreateInfoEXT-pCode-08738",

    // We can't detect what user does in their callback
    "VUID-PFN_vkDebugUtilsMessengerCallbackEXT-None-04769",
    "VUID-VkDeviceMemoryReportCallbackDataEXT-pNext-pNext",
    "VUID-VkDeviceMemoryReportCallbackDataEXT-sType-sType",

    // We are not going to package glslang inside the VVL layer just to validate VK_NV_glsl_shader
    "VUID-VkShaderModuleCreateInfo-pCode-01379",

    // These are checked already in VUID-vkGetPrivateData-objectType-04018 and VUID-vkSetPrivateData-objectHandle-04016
    "VUID-vkGetPrivateData-device-parameter",
    "VUID-vkSetPrivateData-device-parameter",

    // These ask if pData is a certain size, but no way to validate a pointer to memory is a certain size.
    // There is already another implicit VU checking if pData is not null.
    "VUID-vkGetBufferOpaqueCaptureDescriptorDataEXT-pData-08073",
    "VUID-vkGetImageOpaqueCaptureDescriptorDataEXT-pData-08077",
    "VUID-vkGetImageViewOpaqueCaptureDescriptorDataEXT-pData-08081",
    "VUID-vkGetSamplerOpaqueCaptureDescriptorDataEXT-pData-08085",
    "VUID-vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT-pData-08089",

    // These would need to be checked by the loader as it uses these to call into the layers/drivers
    "VUID-vkEnumerateInstanceVersion-pApiVersion-parameter",
    "VUID-vkEnumerateDeviceExtensionProperties-pPropertyCount-parameter",
    "VUID-vkEnumerateDeviceLayerProperties-pPropertyCount-parameter",
    "VUID-vkEnumerateInstanceLayerProperties-pPropertyCount-parameter",
    "VUID-vkEnumerateInstanceExtensionProperties-pPropertyCount-parameter",
    // These are implemented, but can't test as the loader will fail out first
    "VUID-VkInstanceCreateInfo-ppEnabledLayerNames-parameter",
    "VUID-VkInstanceCreateInfo-ppEnabledExtensionNames-parameter"

    // Caches are called between application runs so there is no way for a layer to track this information
    "VUID-VkPipelineCacheCreateInfo-initialDataSize-00768",
    "VUID-VkPipelineCacheCreateInfo-initialDataSize-00769",
    "VUID-VkValidationCacheCreateInfoEXT-initialDataSize-01534",
    "VUID-VkValidationCacheCreateInfoEXT-initialDataSize-01535",
    // The header data returned from vkGetPipelineCacheData is the driver's responsibility to make correct.
    // There is CTS for this, and not within the scope of the Validation Layers to check
    "VUID-VkPipelineCacheHeaderVersionOne-headerSize-04967",
    "VUID-VkPipelineCacheHeaderVersionOne-headerVersion-04968",
    "VUID-VkPipelineCacheHeaderVersionOne-headerSize-08990",
    "VUID-VkPipelineCacheHeaderVersionOne-headerVersion-parameter",
    // Same as above, struct is returned by the driver
    "VUID-VkDeviceFaultVendorBinaryHeaderVersionOneEXT-headerSize-07340",
    "VUID-VkDeviceFaultVendorBinaryHeaderVersionOneEXT-headerVersion-07341",
    "VUID-VkDeviceFaultVendorBinaryHeaderVersionOneEXT-headerVersion-parameter",

    // Extension has redundant implicit VUs
    "VUID-VkBufferConstraintsInfoFUCHSIA-createInfo-parameter",
    "VUID-VkBufferConstraintsInfoFUCHSIA-bufferCollectionConstraints-parameter",
    "VUID-VkImageConstraintsInfoFUCHSIA-bufferCollectionConstraints-parameter",
    "VUID-VkImageFormatConstraintsInfoFUCHSIA-imageCreateInfo-parameter",

    // https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6639#note_468463
    "VUID-VkIndirectCommandsVertexBufferTokenEXT-vertexBindingUnit-11134",

    // Unlike things like VkIndirectCommandsTokenDataEXT/VkDescriptorDataEXT this is
    // an union of pNext structs that have no way to hit without hitting other things first
    "VUID-VkAccelerationStructureGeometryKHR-triangles-parameter",
    "VUID-VkAccelerationStructureGeometryKHR-instances-parameter",
    "VUID-VkAccelerationStructureGeometryKHR-aabbs-parameter",

    // These implicit VUs ask to check for a valid structure that has no sType,
    // there is nothing that can actually be validated
    //
    // VkImageSubresourceLayers
    "VUID-VkImageBlit-dstSubresource-parameter",
    "VUID-VkImageBlit-srcSubresource-parameter",
    "VUID-VkImageBlit2-dstSubresource-parameter",
    "VUID-VkImageBlit2-srcSubresource-parameter",
    "VUID-VkImageCopy-dstSubresource-parameter",
    "VUID-VkImageCopy-srcSubresource-parameter",
    "VUID-VkImageCopy2-dstSubresource-parameter",
    "VUID-VkImageCopy2-srcSubresource-parameter",
    "VUID-VkImageResolve-dstSubresource-parameter",
    "VUID-VkImageResolve-srcSubresource-parameter",
    "VUID-VkImageResolve2-dstSubresource-parameter",
    "VUID-VkImageResolve2-srcSubresource-parameter",
    "VUID-VkBufferImageCopy-imageSubresource-parameter",
    "VUID-VkBufferImageCopy2-imageSubresource-parameter",
    "VUID-VkMemoryToImageCopy-imageSubresource-parameter",
    "VUID-VkImageToMemoryCopy-imageSubresource-parameter",
    "VUID-VkCopyMemoryToImageIndirectCommandNV-imageSubresource-parameter",
    // VkImageSubresourceRange
    "VUID-VkImageMemoryBarrier-subresourceRange-parameter",
    "VUID-VkImageMemoryBarrier2-subresourceRange-parameter",
    "VUID-VkHostImageLayoutTransitionInfo-subresourceRange-parameter",
    "VUID-VkImageViewCreateInfo-subresourceRange-parameter",
    // VkImageSubresource
    "VUID-VkImageSubresource2-imageSubresource-parameter",
    "VUID-VkSparseImageMemoryBind-subresource-parameter",
    // VkStencilOpState
    "VUID-VkPipelineDepthStencilStateCreateInfo-front-parameter",
    "VUID-VkPipelineDepthStencilStateCreateInfo-back-parameter",
    // VkClearValue
    "VUID-VkRenderingAttachmentInfo-clearValue-parameter",
    // VkComponentMapping
    "VUID-VkImageViewCreateInfo-components-parameter",
    "VUID-VkSamplerYcbcrConversionCreateInfo-components-parameter",
    "VUID-VkSamplerBorderColorComponentMappingCreateInfoEXT-components-parameter",
    // VkAttachmentReference
    "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-parameter",
    // VkVideoEncodeH264QpKHR and VkVideoEncodeH264FrameSizeKHR
    "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-maxFrameSize-parameter",
    "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-maxQp-parameter",
    "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-minQp-parameter",
    // VkVideoEncodeH265QpKHR and VkVideoEncodeH265FrameSizeKHR
    "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-maxFrameSize-parameter",
    "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-maxQp-parameter",
    "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-minQp-parameter",
    // VkVideoPictureResourceInfoKHR
    "VUID-VkVideoDecodeInfoKHR-dstPictureResource-parameter",
    "VUID-VkVideoEncodeInfoKHR-srcPictureResource-parameter",
    // VkPushConstantRange
    "VUID-VkIndirectCommandsPushConstantTokenEXT-updateRange-parameter",
    // Video
    "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-maxFrameSize-parameter",
    "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-maxQIndex-parameter",
    "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-minQIndex-parameter",

    // These structs are never called anywhere explicitly
    "VUID-VkAccelerationStructureInstanceKHR-flags-parameter",
    "VUID-VkAccelerationStructureMatrixMotionInstanceNV-flags-parameter",
    "VUID-VkAccelerationStructureSRTMotionInstanceNV-flags-parameter",

    // When:
    //   Struct A has a pointer field to Struct B
    //   Struct B has a non-pointer field to Struct C
    // you get a situation where Struct B has a VU that is not hit because we validate it in Struct C
    "VUID-VkAttachmentSampleLocationsEXT-sampleLocationsInfo-parameter",              // VUID-VkSampleLocationsInfoEXT-sType-sType
    "VUID-VkSubpassSampleLocationsEXT-sampleLocationsInfo-parameter",                 // VUID-VkSampleLocationsInfoEXT-sType-sType
    "VUID-VkPipelineSampleLocationsStateCreateInfoEXT-sampleLocationsInfo-parameter", // VUID-VkSampleLocationsInfoEXT-sType-sType
    "VUID-VkComputePipelineCreateInfo-stage-parameter", // VUID-VkPipelineShaderStageCreateInfo-sType-sType

    // Not possible as described in https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6324
    "VUID-VkGraphicsPipelineCreateInfo-pTessellationState-09023",
    "VUID-VkGraphicsPipelineCreateInfo-pViewportState-09025",
    "VUID-VkGraphicsPipelineCreateInfo-pMultisampleState-09027",
    "VUID-VkGraphicsPipelineCreateInfo-pDepthStencilState-09029",
    "VUID-VkGraphicsPipelineCreateInfo-pInputAssemblyState-09032",
    "VUID-VkGraphicsPipelineCreateInfo-pDepthStencilState-09034",
    "VUID-VkGraphicsPipelineCreateInfo-pDepthStencilState-09036",
    "VUID-VkGraphicsPipelineCreateInfo-pColorBlendState-09038",
    "VUID-VkGraphicsPipelineCreateInfo-pRasterizationState-09039",
    "VUID-VkGraphicsPipelineCreateInfo-pRasterizationState-09040",
    // another variation of it
    "VUID-vkGetDeviceFaultInfoEXT-pFaultCounts-07337",
    "VUID-vkGetDeviceFaultInfoEXT-pFaultCounts-07338",
    "VUID-vkGetDeviceFaultInfoEXT-pFaultCounts-07339",
    "VUID-VkRenderingInputAttachmentIndexInfo-pDepthInputAttachmentIndex-parameter",
    "VUID-VkRenderingInputAttachmentIndexInfo-pStencilInputAttachmentIndex-parameter"

    // These VUs have "is not NULL it must be a pointer to a valid pointer to valid structure" language
    // There is no actual way to validate thsese
    // https://gitlab.khronos.org/vulkan/vulkan/-/issues/3718
    "VUID-VkDescriptorGetInfoEXT-pUniformTexelBuffer-parameter",
    "VUID-VkDescriptorGetInfoEXT-pStorageTexelBuffer-parameter",
    "VUID-VkDescriptorGetInfoEXT-pUniformBuffer-parameter",
    "VUID-VkDescriptorGetInfoEXT-pStorageBuffer-parameter",
    // These occur in stateless validation when a pointer member is optional and the length member is also optional
    "VUID-VkPipelineColorBlendStateCreateInfo-pAttachments-parameter",
    "VUID-VkSubpassDescription-pResolveAttachments-parameter",
    "VUID-VkTimelineSemaphoreSubmitInfo-pWaitSemaphoreValues-parameter",
    "VUID-VkTimelineSemaphoreSubmitInfo-pSignalSemaphoreValues-parameter",
    "VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-pStdSPSs-parameter",
    "VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-pStdPPSs-parameter",
    "VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-pStdVPSs-parameter",
    "VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-pStdSPSs-parameter",
    "VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-pStdPPSs-parameter",
    "VUID-VkD3D12FenceSubmitInfoKHR-pWaitSemaphoreValues-parameter",
    "VUID-VkD3D12FenceSubmitInfoKHR-pSignalSemaphoreValues-parameter",
    "VUID-VkPresentRegionKHR-pRectangles-parameter",
    "VUID-VkBindDescriptorSetsInfo-pDynamicOffsets-parameter",
    "VUID-VkPhysicalDeviceHostImageCopyProperties-pCopySrcLayouts-parameter",
    "VUID-VkPhysicalDeviceHostImageCopyProperties-pCopyDstLayouts-parameter",
    "VUID-VkSurfacePresentModeCompatibilityEXT-pPresentModes-parameter",
    "VUID-VkFrameBoundaryEXT-pImages-parameter",
    "VUID-VkFrameBoundaryEXT-pBuffers-parameter",
    "VUID-VkFrameBoundaryEXT-pTag-parameter",
    "VUID-VkMicromapBuildInfoEXT-pUsageCounts-parameter",
    "VUID-VkMicromapBuildInfoEXT-ppUsageCounts-parameter",
    "VUID-VkAccelerationStructureTrianglesOpacityMicromapEXT-pUsageCounts-parameter",
    "VUID-VkAccelerationStructureTrianglesOpacityMicromapEXT-ppUsageCounts-parameter",
    "VUID-VkAccelerationStructureTrianglesDisplacementMicromapNV-pUsageCounts-parameter",
    "VUID-VkAccelerationStructureTrianglesDisplacementMicromapNV-ppUsageCounts-parameter",
    "VUID-VkShaderCreateInfoEXT-pSetLayouts-parameter",
    "VUID-VkShaderCreateInfoEXT-pPushConstantRanges-parameter",
    "VUID-VkLatencySurfaceCapabilitiesNV-pPresentModes-parameter",
    "VUID-vkCmdBeginTransformFeedbackEXT-pCounterBufferOffsets-parameter",
    "VUID-vkCmdEndTransformFeedbackEXT-pCounterBufferOffsets-parameter",
    "VUID-vkCmdBindVertexBuffers2-pSizes-parameter",
    "VUID-vkCmdBindVertexBuffers2-pStrides-parameter",
    "VUID-VkDescriptorGetInfoEXT-pSampledImage-parameter",
    "VUID-VkDescriptorGetInfoEXT-pSampler-parameter",
    "VUID-VkDescriptorGetInfoEXT-pStorageImage-parameter",
    "VUID-vkGetAccelerationStructureBuildSizesKHR-pMaxPrimitiveCounts-parameter",
    "VUID-vkCmdDrawMultiIndexedEXT-pVertexOffset-parameter",
    "VUID-VkDisplayModeCreateInfoKHR-parameters-parameter",
    "VUID-VkPipelineBinaryHandlesInfoKHR-pPipelineBinaries-parameter",
    "VUID-VkSubpassDescription2-pResolveAttachments-parameter",
    "VUID-VkPhysicalDeviceLayeredApiPropertiesListKHR-pLayeredApis-parameter",
    "VUID-VkExecutionGraphPipelineCreateInfoAMDX-pStages-parameter",
    "VUID-VkGetLatencyMarkerInfoNV-pTimings-parameter",
    "VUID-VkIndirectExecutionSetShaderInfoEXT-pSetLayoutInfos-parameter",
    "VUID-VkAccelerationStructureBuildGeometryInfoKHR-pGeometries-parameter",
    "VUID-vkEnumeratePhysicalDeviceGroups-pPhysicalDeviceGroupProperties-parameter",
    "VUID-vkGetImageSparseMemoryRequirements2-pSparseMemoryRequirements-parameter",
    "VUID-vkGetPhysicalDeviceQueueFamilyProperties2-pQueueFamilyProperties-parameter",
    "VUID-vkGetPhysicalDeviceSparseImageFormatProperties2-pProperties-parameter",
    "VUID-vkGetPhysicalDeviceToolProperties-pToolProperties-parameter",
    "VUID-vkGetDeviceImageSparseMemoryRequirements-pSparseMemoryRequirements-parameter",
    "VUID-vkGetPhysicalDeviceVideoFormatPropertiesKHR-pVideoFormatProperties-parameter",
    "VUID-vkGetVideoSessionMemoryRequirementsKHR-pMemoryRequirements-parameter",
    "VUID-vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR-pCounters-parameter",
    "VUID-vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR-pCounterDescriptions-parameter",
    "VUID-vkGetPhysicalDeviceSurfaceFormats2KHR-pSurfaceFormats-parameter",
    "VUID-vkGetPhysicalDeviceDisplayProperties2KHR-pProperties-parameter",
    "VUID-vkGetPhysicalDeviceDisplayPlaneProperties2KHR-pProperties-parameter",
    "VUID-vkGetDisplayModeProperties2KHR-pProperties-parameter",
    "VUID-vkGetPhysicalDeviceFragmentShadingRatesKHR-pFragmentShadingRates-parameter",
    "VUID-vkGetPipelineExecutablePropertiesKHR-pProperties-parameter",
    "VUID-vkGetPipelineExecutableStatisticsKHR-pStatistics-parameter",
    "VUID-vkGetPipelineExecutableInternalRepresentationsKHR-pInternalRepresentations-parameter",
    "VUID-vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR-pProperties-parameter",
    "VUID-vkGetQueueCheckpointDataNV-pCheckpointData-parameter",
    "VUID-vkGetQueueCheckpointData2NV-pCheckpointData-parameter",
    "VUID-vkGetPhysicalDeviceCooperativeMatrixPropertiesNV-pProperties-parameter",
    "VUID-vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV-pCombinations-parameter",
    "VUID-vkGetPhysicalDeviceOpticalFlowImageFormatsNV-pImageFormatProperties-parameter",
    "VUID-vkGetFramebufferTilePropertiesQCOM-pProperties-parameter",
    "VUID-vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV-pProperties-parameter",
    "VUID-VkAccelerationStructureBuildGeometryInfoKHR-ppGeometries-parameter",
    "VUID-VkPipelineBinaryCreateInfoKHR-pKeysAndDataInfo-parameter",
    "VUID-vkCmdSetDepthClampRangeEXT-pDepthClampRange-parameter",
    "VUID-VkRenderingInputAttachmentIndexInfo-pColorAttachmentInputIndices-parameter",
    "VUID-VkPipelineViewportDepthClampControlCreateInfoEXT-pDepthClampRange-parameter",
    // These occur in stateless validation when a pointer member is optional and the length member is null
    "VUID-VkDeviceCreateInfo-pEnabledFeatures-parameter",
    "VUID-VkInstanceCreateInfo-pApplicationInfo-parameter",
    "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-parameter",
    "VUID-VkSubpassDescription-pDepthStencilAttachment-parameter",
    "VUID-VkShaderCreateInfoEXT-pSpecializationInfo-parameter",
    "VUID-VkExportFenceWin32HandleInfoKHR-pAttributes-parameter",
    "VUID-VkExportSemaphoreWin32HandleInfoKHR-pAttributes-parameter",
    "VUID-VkExportMemoryWin32HandleInfoKHR-pAttributes-parameter",
    "VUID-VkExportMemoryWin32HandleInfoNV-pAttributes-parameter",
    "VUID-vkEnumerateDeviceExtensionProperties-pProperties-parameter",
    "VUID-vkEnumerateDeviceLayerProperties-pProperties-parameter",
    "VUID-vkEnumerateInstanceExtensionProperties-pProperties-parameter",
    "VUID-vkEnumerateInstanceLayerProperties-pProperties-parameter",
    // Checking for null-terminated UTF-8 string
    "VUID-VkApplicationInfo-pApplicationName-parameter",
    "VUID-VkApplicationInfo-pEngineName-parameter",
    "VUID-VkDebugUtilsObjectNameInfoEXT-pObjectName-parameter",
    "VUID-VkDebugUtilsMessengerCallbackDataEXT-pMessageIdName-parameter",
    "VUID-VkDebugUtilsMessengerCallbackDataEXT-pMessage-parameter",
    "VUID-VkPipelineShaderStageNodeCreateInfoAMDX-pName-parameter",
    "VUID-VkShaderCreateInfoEXT-pName-parameter",
    "VUID-vkGetDeviceProcAddr-pName-parameter",
    "VUID-vkGetInstanceProcAddr-pName-parameter",
    "VUID-vkEnumerateDeviceExtensionProperties-pLayerName-parameter",
    "VUID-vkEnumerateInstanceExtensionProperties-pLayerName-parameter",
    // Can't validate a VkAllocationCallbacks structure
    "VUID-vkCreateAccelerationStructureKHR-pAllocator-parameter",
    "VUID-vkCreateAccelerationStructureNV-pAllocator-parameter",
    "VUID-vkCreateAndroidSurfaceKHR-pAllocator-parameter",
    "VUID-vkCreateBuffer-pAllocator-parameter",
    "VUID-vkCreateBufferCollectionFUCHSIA-pAllocator-parameter",
    "VUID-vkCreateBufferView-pAllocator-parameter",
    "VUID-vkCreateCommandPool-pAllocator-parameter",
    "VUID-vkCreateComputePipelines-pAllocator-parameter",
    "VUID-vkCreateCuFunctionNVX-pAllocator-parameter",
    "VUID-vkCreateCuModuleNVX-pAllocator-parameter",
    "VUID-vkCreateCudaFunctionNV-pAllocator-parameter",
    "VUID-vkCreateCudaModuleNV-pAllocator-parameter",
    "VUID-vkCreateDebugReportCallbackEXT-pAllocator-parameter",
    "VUID-vkCreateDebugUtilsMessengerEXT-pAllocator-parameter",
    "VUID-vkCreateDeferredOperationKHR-pAllocator-parameter",
    "VUID-vkCreateDescriptorPool-pAllocator-parameter",
    "VUID-vkCreateDescriptorSetLayout-pAllocator-parameter",
    "VUID-vkCreateDescriptorUpdateTemplate-pAllocator-parameter",
    "VUID-vkCreateDevice-pAllocator-parameter",
    "VUID-vkCreateDirectFBSurfaceEXT-pAllocator-parameter",
    "VUID-vkCreateDisplayModeKHR-pAllocator-parameter",
    "VUID-vkCreateDisplayPlaneSurfaceKHR-pAllocator-parameter",
    "VUID-vkCreateEvent-pAllocator-parameter",
    "VUID-vkCreateExecutionGraphPipelinesAMDX-pAllocator-parameter",
    "VUID-vkCreateFence-pAllocator-parameter",
    "VUID-vkCreateFramebuffer-pAllocator-parameter",
    "VUID-vkCreateGraphicsPipelines-pAllocator-parameter",
    "VUID-vkCreateHeadlessSurfaceEXT-pAllocator-parameter",
    "VUID-vkCreateIOSSurfaceMVK-pAllocator-parameter",
    "VUID-vkCreateImage-pAllocator-parameter",
    "VUID-vkCreateImagePipeSurfaceFUCHSIA-pAllocator-parameter",
    "VUID-vkCreateImageView-pAllocator-parameter",
    "VUID-vkCreateIndirectCommandsLayoutNV-pAllocator-parameter",
    "VUID-vkCreateInstance-pAllocator-parameter",
    "VUID-vkCreateMacOSSurfaceMVK-pAllocator-parameter",
    "VUID-vkCreateMetalSurfaceEXT-pAllocator-parameter",
    "VUID-vkCreateMicromapEXT-pAllocator-parameter",
    "VUID-vkCreateOpticalFlowSessionNV-pAllocator-parameter",
    "VUID-vkCreatePipelineCache-pAllocator-parameter",
    "VUID-vkCreatePipelineLayout-pAllocator-parameter",
    "VUID-vkCreatePrivateDataSlot-pAllocator-parameter",
    "VUID-vkCreateQueryPool-pAllocator-parameter",
    "VUID-vkCreateRayTracingPipelinesKHR-pAllocator-parameter",
    "VUID-vkCreateRayTracingPipelinesNV-pAllocator-parameter",
    "VUID-vkCreateRenderPass-pAllocator-parameter",
    "VUID-vkCreateRenderPass2-pAllocator-parameter",
    "VUID-vkCreateSampler-pAllocator-parameter",
    "VUID-vkCreateSamplerYcbcrConversion-pAllocator-parameter",
    "VUID-vkCreateScreenSurfaceQNX-pAllocator-parameter",
    "VUID-vkCreateSemaphore-pAllocator-parameter",
    "VUID-vkCreateShaderModule-pAllocator-parameter",
    "VUID-vkCreateShadersEXT-pAllocator-parameter",
    "VUID-vkCreateSharedSwapchainsKHR-pAllocator-parameter",
    "VUID-vkCreateStreamDescriptorSurfaceGGP-pAllocator-parameter",
    "VUID-vkCreateSwapchainKHR-pAllocator-parameter",
    "VUID-vkCreateValidationCacheEXT-pAllocator-parameter",
    "VUID-vkCreateViSurfaceNN-pAllocator-parameter",
    "VUID-vkCreateVideoSessionKHR-pAllocator-parameter",
    "VUID-vkCreateVideoSessionParametersKHR-pAllocator-parameter",
    "VUID-vkCreateWaylandSurfaceKHR-pAllocator-parameter",
    "VUID-vkCreateWin32SurfaceKHR-pAllocator-parameter",
    "VUID-vkCreateXcbSurfaceKHR-pAllocator-parameter",
    "VUID-vkCreateXlibSurfaceKHR-pAllocator-parameter",
    "VUID-vkCreateIndirectCommandsLayoutEXT-pAllocator-parameter",
    "VUID-vkCreateIndirectExecutionSetEXT-pAllocator-parameter",
    "VUID-vkCreatePipelineBinariesKHR-pAllocator-parameter",
    "VUID-vkDestroyAccelerationStructureKHR-pAllocator-parameter",
    "VUID-vkDestroyAccelerationStructureNV-pAllocator-parameter",
    "VUID-vkDestroyBuffer-pAllocator-parameter",
    "VUID-vkDestroyBufferCollectionFUCHSIA-pAllocator-parameter",
    "VUID-vkDestroyBufferView-pAllocator-parameter",
    "VUID-vkDestroyCommandPool-pAllocator-parameter",
    "VUID-vkDestroyCuFunctionNVX-pAllocator-parameter",
    "VUID-vkDestroyCuModuleNVX-pAllocator-parameter",
    "VUID-vkDestroyCudaFunctionNV-pAllocator-parameter",
    "VUID-vkDestroyCudaModuleNV-pAllocator-parameter",
    "VUID-vkDestroyDebugReportCallbackEXT-pAllocator-parameter",
    "VUID-vkDestroyDebugUtilsMessengerEXT-pAllocator-parameter",
    "VUID-vkDestroyDeferredOperationKHR-pAllocator-parameter",
    "VUID-vkDestroyDescriptorPool-pAllocator-parameter",
    "VUID-vkDestroyDescriptorSetLayout-pAllocator-parameter",
    "VUID-vkDestroyDescriptorUpdateTemplate-pAllocator-parameter",
    "VUID-vkDestroyDevice-pAllocator-parameter",
    "VUID-vkDestroyEvent-pAllocator-parameter",
    "VUID-vkDestroyFence-pAllocator-parameter",
    "VUID-vkDestroyFramebuffer-pAllocator-parameter",
    "VUID-vkDestroyImage-pAllocator-parameter",
    "VUID-vkDestroyImageView-pAllocator-parameter",
    "VUID-vkDestroyIndirectCommandsLayoutNV-pAllocator-parameter",
    "VUID-vkDestroyInstance-pAllocator-parameter",
    "VUID-vkDestroyMicromapEXT-pAllocator-parameter",
    "VUID-vkDestroyOpticalFlowSessionNV-pAllocator-parameter",
    "VUID-vkDestroyPipeline-pAllocator-parameter",
    "VUID-vkDestroyPipelineCache-pAllocator-parameter",
    "VUID-vkDestroyPipelineLayout-pAllocator-parameter",
    "VUID-vkDestroyPrivateDataSlot-pAllocator-parameter",
    "VUID-vkDestroyQueryPool-pAllocator-parameter",
    "VUID-vkDestroyRenderPass-pAllocator-parameter",
    "VUID-vkDestroySampler-pAllocator-parameter",
    "VUID-vkDestroySamplerYcbcrConversion-pAllocator-parameter",
    "VUID-vkDestroySemaphore-pAllocator-parameter",
    "VUID-vkDestroyShaderEXT-pAllocator-parameter",
    "VUID-vkDestroyShaderModule-pAllocator-parameter",
    "VUID-vkDestroySurfaceKHR-pAllocator-parameter",
    "VUID-vkDestroySwapchainKHR-pAllocator-parameter",
    "VUID-vkDestroyValidationCacheEXT-pAllocator-parameter",
    "VUID-vkDestroyVideoSessionKHR-pAllocator-parameter",
    "VUID-vkDestroyVideoSessionParametersKHR-pAllocator-parameter",
    "VUID-vkDestroyIndirectCommandsLayoutEXT-pAllocator-parameter",
    "VUID-vkDestroyIndirectExecutionSetEXT-pAllocator-parameter",
    "VUID-vkDestroyPipelineBinaryKHR-pAllocator-parameter",
    "VUID-vkReleaseCapturedPipelineDataKHR-pAllocator-parameter",
    "VUID-vkFreeMemory-pAllocator-parameter",
    "VUID-vkRegisterDeviceEventEXT-pAllocator-parameter",
    "VUID-vkRegisterDisplayEventEXT-pAllocator-parameter",
    "VUID-vkAllocateMemory-pAllocator-parameter",

    // Removed in https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/9302
    // Found these are not invalid actually
    "VUID-VkPhysicalDeviceAccelerationStructurePropertiesKHR-sType-sType",
    "VUID-VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI-sType-sType",
    "VUID-VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR-sType-sType",
    "VUID-VkPhysicalDeviceConservativeRasterizationPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceCooperativeMatrix2PropertiesNV-sType-sType",
    "VUID-VkPhysicalDeviceCooperativeMatrixPropertiesKHR-sType-sType",
    "VUID-VkPhysicalDeviceCooperativeMatrixPropertiesNV-sType-sType",
    "VUID-VkPhysicalDeviceCopyMemoryIndirectPropertiesNV-sType-sType",
    "VUID-VkPhysicalDeviceCudaKernelLaunchPropertiesNV-sType-sType",
    "VUID-VkPhysicalDeviceCustomBorderColorPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceDepthStencilResolveProperties-sType-sType",
    "VUID-VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceDescriptorBufferPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceDescriptorIndexingProperties-sType-sType",
    "VUID-VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV-sType-sType",
    "VUID-VkPhysicalDeviceDiscardRectanglePropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceDisplacementMicromapPropertiesNV-sType-sType",
    "VUID-VkPhysicalDeviceDriverProperties-sType-sType",
    "VUID-VkPhysicalDeviceDrmPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceExtendedDynamicState3PropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV-sType-sType",
    "VUID-VkPhysicalDeviceExternalFormatResolvePropertiesANDROID-sType-sType",
    "VUID-VkPhysicalDeviceExternalMemoryHostPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceFloatControlsProperties-sType-sType",
    "VUID-VkPhysicalDeviceFragmentDensityMap2PropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM-sType-sType",
    "VUID-VkPhysicalDeviceFragmentDensityMapPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR-sType-sType",
    "VUID-VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV-sType-sType",
    "VUID-VkPhysicalDeviceFragmentShadingRatePropertiesKHR-sType-sType",
    "VUID-VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceHostImageCopyProperties-sType-sType",
    "VUID-VkPhysicalDeviceIDProperties-sType-sType",
    "VUID-VkPhysicalDeviceImageAlignmentControlPropertiesMESA-sType-sType",
    "VUID-VkPhysicalDeviceImageProcessing2PropertiesQCOM-sType-sType",
    "VUID-VkPhysicalDeviceImageProcessingPropertiesQCOM-sType-sType",
    "VUID-VkPhysicalDeviceInlineUniformBlockProperties-sType-sType",
    "VUID-VkPhysicalDeviceLayeredApiPropertiesKHR-pNext-pNext",
    "VUID-VkPhysicalDeviceLayeredApiPropertiesKHR-sType-sType",
    "VUID-VkPhysicalDeviceLayeredApiPropertiesKHR-sType-unique",
    "VUID-VkPhysicalDeviceLayeredApiPropertiesListKHR-sType-sType",
    "VUID-VkPhysicalDeviceLayeredDriverPropertiesMSFT-sType-sType",
    "VUID-VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceLineRasterizationProperties-sType-sType",
    "VUID-VkPhysicalDeviceMaintenance3Properties-sType-sType",
    "VUID-VkPhysicalDeviceMaintenance4Properties-sType-sType",
    "VUID-VkPhysicalDeviceMaintenance5Properties-sType-sType",
    "VUID-VkPhysicalDeviceMaintenance6Properties-sType-sType",
    "VUID-VkPhysicalDeviceMaintenance7PropertiesKHR-sType-sType",
    "VUID-VkPhysicalDeviceMapMemoryPlacedPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceMemoryDecompressionPropertiesNV-sType-sType",
    "VUID-VkPhysicalDeviceMeshShaderPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceMeshShaderPropertiesNV-sType-sType",
    "VUID-VkPhysicalDeviceMultiDrawPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX-sType-sType",
    "VUID-VkPhysicalDeviceMultiviewProperties-sType-sType",
    "VUID-VkPhysicalDeviceNestedCommandBufferPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceOpacityMicromapPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceOpticalFlowPropertiesNV-sType-sType",
    "VUID-VkPhysicalDevicePCIBusInfoPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDevicePerformanceQueryPropertiesKHR-sType-sType",
    "VUID-VkPhysicalDevicePipelineBinaryPropertiesKHR-sType-sType",
    "VUID-VkPhysicalDevicePipelineRobustnessProperties-sType-sType",
    "VUID-VkPhysicalDevicePointClippingProperties-sType-sType",
    "VUID-VkPhysicalDevicePortabilitySubsetPropertiesKHR-sType-sType",
    "VUID-VkPhysicalDeviceProtectedMemoryProperties-sType-sType",
    "VUID-VkPhysicalDeviceProvokingVertexPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDevicePushDescriptorProperties-sType-sType",
    "VUID-VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV-sType-sType",
    "VUID-VkPhysicalDeviceRayTracingPipelinePropertiesKHR-sType-sType",
    "VUID-VkPhysicalDeviceRayTracingPropertiesNV-sType-sType",
    "VUID-VkPhysicalDeviceRenderPassStripedPropertiesARM-sType-sType",
    "VUID-VkPhysicalDeviceRobustness2PropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceSampleLocationsPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceSamplerFilterMinmaxProperties-sType-sType",
    "VUID-VkPhysicalDeviceSchedulingControlsPropertiesARM-sType-sType",
    "VUID-VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM-sType-sType",
    "VUID-VkPhysicalDeviceShaderCoreProperties2AMD-sType-sType",
    "VUID-VkPhysicalDeviceShaderCorePropertiesAMD-sType-sType",
    "VUID-VkPhysicalDeviceShaderCorePropertiesARM-sType-sType",
    "VUID-VkPhysicalDeviceShaderEnqueuePropertiesAMDX-sType-sType",
    "VUID-VkPhysicalDeviceShaderIntegerDotProductProperties-sType-sType",
    "VUID-VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceShaderObjectPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceShaderSMBuiltinsPropertiesNV-sType-sType",
    "VUID-VkPhysicalDeviceShaderTileImagePropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceShadingRateImagePropertiesNV-sType-sType",
    "VUID-VkPhysicalDeviceSubgroupProperties-sType-sType",
    "VUID-VkPhysicalDeviceSubgroupSizeControlProperties-sType-sType",
    "VUID-VkPhysicalDeviceSubpassShadingPropertiesHUAWEI-sType-sType",
    "VUID-VkPhysicalDeviceTexelBufferAlignmentProperties-sType-sType",
    "VUID-VkPhysicalDeviceTimelineSemaphoreProperties-sType-sType",
    "VUID-VkPhysicalDeviceTransformFeedbackPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceVertexAttributeDivisorProperties-sType-sType",
    "VUID-VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT-sType-sType",
    "VUID-VkPhysicalDeviceVulkan11Properties-sType-sType",
    "VUID-VkPhysicalDeviceVulkan12Properties-sType-sType",
    "VUID-VkPhysicalDeviceVulkan13Properties-sType-sType",
    "VUID-VkPhysicalDeviceVulkan14Properties-sType-sType",

    // Needs to be correct for VVL to even know about the struct
    "VUID-VkLayerSettingsCreateInfoEXT-sType-sType"

    // Points to Video struts not defined
    "VUID-VkVideoDecodeAV1InlineSessionParametersInfoKHR-pStdSequenceHeader-parameter",
    "VUID-VkVideoDecodeH264InlineSessionParametersInfoKHR-pStdPPS-parameter",
    "VUID-VkVideoDecodeH264InlineSessionParametersInfoKHR-pStdSPS-parameter",
    "VUID-VkVideoDecodeH265InlineSessionParametersInfoKHR-pStdPPS-parameter",
    "VUID-VkVideoDecodeH265InlineSessionParametersInfoKHR-pStdSPS-parameter",
    "VUID-VkVideoDecodeH265InlineSessionParametersInfoKHR-pStdVPS-parameter",
    "VUID-VkVideoEncodeAV1SessionParametersCreateInfoKHR-pStdDecoderModelInfo-parameter",
    "VUID-VkVideoEncodeAV1SessionParametersCreateInfoKHR-pStdOperatingPoints-parameter",

    // Acceleration structure replay related, 
    // but VVL has no way of tracking needed info (typically stored offline)
    "VUID-VkAccelerationStructureCreateInfoKHR-deviceAddress-09488"
    "VUID-VkAccelerationStructureCreateInfoKHR-deviceAddress-09489"
    "VUID-VkAccelerationStructureCreateInfoKHR-deviceAddress-09490"
    "VUID-VkAccelerationStructureCreateInfoKHR-deviceAddress-10393"
};

// VUs from deprecated extensions that would require complex codegen to get working
const char* deprecated_validation[] = {
    "VUID-VkAccelerationStructureCreateInfoNV-info-parameter",
    "VUID-VkAccelerationStructureInfoNV-type-parameter",
    "VUID-VkAccelerationStructureMotionInstanceNV-flags-zerobitmask",
    "VUID-VkAccelerationStructureMotionInstanceNV-matrixMotionInstance-parameter",
    "VUID-VkAccelerationStructureMotionInstanceNV-staticInstance-parameter",
    "VUID-VkAccelerationStructureMotionInstanceNV-type-parameter",
    "VUID-VkGeometryDataNV-aabbs-parameter",
    "VUID-VkGeometryDataNV-triangles-parameter",
    "VUID-VkGeometryNV-geometry-parameter",
    "VUID-VkAccelerationStructureTrianglesDisplacementMicromapNV-micromap-parameter",
    "VUID-VkBindIndexBufferIndirectCommandNV-indexType-parameter",
    "VUID-VkAccelerationStructureMotionInstanceNV-srtMotionInstance-parameter",
};

// clang-format on