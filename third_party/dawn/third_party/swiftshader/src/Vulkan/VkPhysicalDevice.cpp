// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "VkPhysicalDevice.hpp"

#include "VkConfig.hpp"
#include "VkStringify.hpp"
#include "Pipeline/SpirvShader.hpp"  // sw::SIMD::Width
#include "Reactor/Reactor.hpp"

#include <cstring>
#include <limits>

#ifdef __ANDROID__
#	include <android/hardware_buffer.h>
#endif

namespace vk {

PhysicalDevice::PhysicalDevice(const void *, void *mem)
{
}

const VkPhysicalDeviceFeatures &PhysicalDevice::getFeatures() const
{
	static const VkPhysicalDeviceFeatures features{
		VK_TRUE,   // robustBufferAccess
		VK_TRUE,   // fullDrawIndexUint32
		VK_TRUE,   // imageCubeArray
		VK_TRUE,   // independentBlend
		VK_FALSE,  // geometryShader
		VK_FALSE,  // tessellationShader
		VK_TRUE,   // sampleRateShading
		VK_FALSE,  // dualSrcBlend
		VK_FALSE,  // logicOp
		VK_TRUE,   // multiDrawIndirect
		VK_TRUE,   // drawIndirectFirstInstance
		VK_TRUE,   // depthClamp
		VK_TRUE,   // depthBiasClamp
		VK_TRUE,   // fillModeNonSolid
		VK_TRUE,   // depthBounds
		VK_FALSE,  // wideLines
		VK_TRUE,   // largePoints
		VK_FALSE,  // alphaToOne
		VK_FALSE,  // multiViewport
		VK_TRUE,   // samplerAnisotropy
		VK_TRUE,   // textureCompressionETC2
#ifdef SWIFTSHADER_ENABLE_ASTC
		VK_TRUE,  // textureCompressionASTC_LDR
#else
		VK_FALSE,  // textureCompressionASTC_LDR
#endif
		VK_TRUE,   // textureCompressionBC
		VK_TRUE,   // occlusionQueryPrecise
		VK_FALSE,  // pipelineStatisticsQuery
		VK_TRUE,   // vertexPipelineStoresAndAtomics
		VK_TRUE,   // fragmentStoresAndAtomics
		VK_FALSE,  // shaderTessellationAndGeometryPointSize
		VK_FALSE,  // shaderImageGatherExtended
		VK_TRUE,   // shaderStorageImageExtendedFormats
		VK_TRUE,   // shaderStorageImageMultisample
		VK_FALSE,  // shaderStorageImageReadWithoutFormat
		VK_TRUE,   // shaderStorageImageWriteWithoutFormat
		VK_TRUE,   // shaderUniformBufferArrayDynamicIndexing
		VK_TRUE,   // shaderSampledImageArrayDynamicIndexing
		VK_TRUE,   // shaderStorageBufferArrayDynamicIndexing
		VK_TRUE,   // shaderStorageImageArrayDynamicIndexing
		VK_TRUE,   // shaderClipDistance
		VK_TRUE,   // shaderCullDistance
		VK_FALSE,  // shaderFloat64
		VK_FALSE,  // shaderInt64
		VK_FALSE,  // shaderInt16
		VK_FALSE,  // shaderResourceResidency
		VK_FALSE,  // shaderResourceMinLod
		VK_FALSE,  // sparseBinding
		VK_FALSE,  // sparseResidencyBuffer
		VK_FALSE,  // sparseResidencyImage2D
		VK_FALSE,  // sparseResidencyImage3D
		VK_FALSE,  // sparseResidency2Samples
		VK_FALSE,  // sparseResidency4Samples
		VK_FALSE,  // sparseResidency8Samples
		VK_FALSE,  // sparseResidency16Samples
		VK_FALSE,  // sparseResidencyAliased
		VK_TRUE,   // variableMultisampleRate
		VK_FALSE,  // inheritedQueries
	};

	return features;
}

template<typename T>
static void getPhysicalDeviceSamplerYcbcrConversionFeatures(T *features)
{
	features->samplerYcbcrConversion = VK_TRUE;
}

template<typename T>
static void getPhysicalDevice16BitStorageFeatures(T *features)
{
	features->storageBuffer16BitAccess = VK_FALSE;
	features->storageInputOutput16 = VK_FALSE;
	features->storagePushConstant16 = VK_FALSE;
	features->uniformAndStorageBuffer16BitAccess = VK_FALSE;
}

template<typename T>
static void getPhysicalDeviceVariablePointersFeatures(T *features)
{
	features->variablePointersStorageBuffer = VK_FALSE;
	features->variablePointers = VK_FALSE;
}

template<typename T>
static void getPhysicalDevice8BitStorageFeaturesKHR(T *features)
{
	features->storageBuffer8BitAccess = VK_FALSE;
	features->uniformAndStorageBuffer8BitAccess = VK_FALSE;
	features->storagePushConstant8 = VK_FALSE;
}

template<typename T>
static void getPhysicalDeviceMultiviewFeatures(T *features)
{
	features->multiview = VK_TRUE;
	features->multiviewGeometryShader = VK_FALSE;
	features->multiviewTessellationShader = VK_FALSE;
}

template<typename T>
static void getPhysicalDeviceProtectedMemoryFeatures(T *features)
{
	features->protectedMemory = VK_FALSE;
}

template<typename T>
static void getPhysicalDeviceShaderDrawParameterFeatures(T *features)
{
	features->shaderDrawParameters = VK_FALSE;
}

template<typename T>
static void getPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR(T *features)
{
	features->separateDepthStencilLayouts = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceLineRasterizationFeaturesEXT(T *features)
{
	features->rectangularLines = VK_TRUE;
	features->bresenhamLines = VK_TRUE;
	features->smoothLines = VK_FALSE;
	features->stippledRectangularLines = VK_FALSE;
	features->stippledBresenhamLines = VK_FALSE;
	features->stippledSmoothLines = VK_FALSE;
}

template<typename T>
static void getPhysicalDeviceProvokingVertexFeaturesEXT(T *features)
{
	features->provokingVertexLast = VK_TRUE;
	features->transformFeedbackPreservesProvokingVertex = VK_FALSE;
}

template<typename T>
static void getPhysicalDeviceHostQueryResetFeatures(T *features)
{
	features->hostQueryReset = VK_TRUE;
}

template<typename T>
static void getPhysicalDevicePipelineCreationCacheControlFeatures(T *features)
{
	features->pipelineCreationCacheControl = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceImageRobustnessFeatures(T *features)
{
	features->robustImageAccess = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceShaderDrawParametersFeatures(T *features)
{
	features->shaderDrawParameters = VK_FALSE;
}

template<typename T>
static void getPhysicalDeviceVulkan11Features(T *features)
{
	getPhysicalDevice16BitStorageFeatures(features);
	getPhysicalDeviceMultiviewFeatures(features);
	getPhysicalDeviceVariablePointersFeatures(features);
	getPhysicalDeviceProtectedMemoryFeatures(features);
	getPhysicalDeviceSamplerYcbcrConversionFeatures(features);
	getPhysicalDeviceShaderDrawParametersFeatures(features);
}

template<typename T>
static void getPhysicalDeviceImagelessFramebufferFeatures(T *features)
{
	features->imagelessFramebuffer = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceShaderSubgroupExtendedTypesFeatures(T *features)
{
	features->shaderSubgroupExtendedTypes = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceScalarBlockLayoutFeatures(T *features)
{
	features->scalarBlockLayout = VK_TRUE;
}

#ifdef SWIFTSHADER_DEVICE_MEMORY_REPORT
template<typename T>
static void getPhysicalDeviceDeviceMemoryReportFeaturesEXT(T *features)
{
	features->deviceMemoryReport = VK_TRUE;
}
#endif  // SWIFTSHADER_DEVICE_MEMORY_REPORT

template<typename T>
static void getPhysicalDeviceUniformBufferStandardLayoutFeatures(T *features)
{
	features->uniformBufferStandardLayout = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceDescriptorIndexingFeatures(T *features)
{
	features->shaderInputAttachmentArrayDynamicIndexing = VK_FALSE;
	features->shaderUniformTexelBufferArrayDynamicIndexing = VK_TRUE;
	features->shaderStorageTexelBufferArrayDynamicIndexing = VK_TRUE;
	features->shaderUniformBufferArrayNonUniformIndexing = VK_TRUE;
	features->shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	features->shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
	features->shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
	features->shaderInputAttachmentArrayNonUniformIndexing = VK_FALSE;
	features->shaderUniformTexelBufferArrayNonUniformIndexing = VK_TRUE;
	features->shaderStorageTexelBufferArrayNonUniformIndexing = VK_TRUE;
	features->descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE;
	features->descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
	features->descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
	features->descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
	features->descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE;
	features->descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE;
	features->descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
	features->descriptorBindingPartiallyBound = VK_TRUE;
	features->descriptorBindingVariableDescriptorCount = VK_TRUE;
	features->runtimeDescriptorArray = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceVulkanMemoryModelFeatures(T *features)
{
	features->vulkanMemoryModel = VK_TRUE;
	features->vulkanMemoryModelDeviceScope = VK_TRUE;
	features->vulkanMemoryModelAvailabilityVisibilityChains = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceTimelineSemaphoreFeatures(T *features)
{
	features->timelineSemaphore = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceShaderAtomicInt64Features(T *features)
{
	features->shaderBufferInt64Atomics = VK_FALSE;
	features->shaderSharedInt64Atomics = VK_FALSE;
}

template<typename T>
static void getPhysicalDeviceShaderFloat16Int8Features(T *features)
{
	features->shaderFloat16 = VK_FALSE;
	features->shaderInt8 = VK_FALSE;
}

template<typename T>
static void getPhysicalDeviceBufferDeviceAddressFeatures(T *features)
{
	features->bufferDeviceAddress = VK_TRUE;
	features->bufferDeviceAddressCaptureReplay = VK_FALSE;
	features->bufferDeviceAddressMultiDevice = VK_FALSE;
}

template<typename T>
static void getPhysicalDeviceDynamicRenderingFeatures(T *features)
{
	features->dynamicRendering = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceDynamicRenderingLocalReadFeatures(T *features)
{
	features->dynamicRenderingLocalRead = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceInlineUniformBlockFeatures(T *features)
{
	features->inlineUniformBlock = VK_TRUE;
	features->descriptorBindingInlineUniformBlockUpdateAfterBind = VK_TRUE;
}

template<typename T>
static void getPhysicalDevicePrivateDataFeatures(T *features)
{
	features->privateData = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceTextureCompressionASTCHDRFeatures(T *features)
{
	features->textureCompressionASTC_HDR = VK_FALSE;
}

template<typename T>
static void getPhysicalDeviceShaderDemoteToHelperInvocationFeatures(T *features)
{
	features->shaderDemoteToHelperInvocation = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceShaderTerminateInvocationFeatures(T *features)
{
	features->shaderTerminateInvocation = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceSubgroupSizeControlFeatures(T *features)
{
	features->subgroupSizeControl = VK_TRUE;
	features->computeFullSubgroups = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceSynchronization2Features(T *features)
{
	features->synchronization2 = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceShaderIntegerDotProductFeatures(T *features)
{
	features->shaderIntegerDotProduct = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures(T *features)
{
	features->shaderZeroInitializeWorkgroupMemory = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceMaintenance4Features(T *features)
{
	features->maintenance4 = VK_TRUE;
}

template<typename T>
static void getPhysicalDevicePrimitiveTopologyListRestartFeatures(T *features)
{
	features->primitiveTopologyListRestart = VK_TRUE;
	features->primitiveTopologyPatchListRestart = VK_FALSE;
}

template<typename T>
static void getPhysicalDevicePipelineRobustnessFeatures(T *features)
{
	features->pipelineRobustness = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceGraphicsPipelineLibraryFeatures(T *features)
{
	features->graphicsPipelineLibrary = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceGlobalPriorityQueryFeatures(T *features)
{
	features->globalPriorityQuery = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceSwapchainMaintenance1FeaturesKHR(T *features)
{
	features->swapchainMaintenance1 = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceHostImageCopyFeatures(T *features)
{
	features->hostImageCopy = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceIndexTypeUint8Features(T *features)
{
	features->indexTypeUint8 = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceVulkan12Features(T *features)
{
	features->samplerMirrorClampToEdge = VK_TRUE;
	features->drawIndirectCount = VK_FALSE;
	getPhysicalDevice8BitStorageFeaturesKHR(features);
	getPhysicalDeviceShaderAtomicInt64Features(features);
	getPhysicalDeviceShaderFloat16Int8Features(features);
	features->descriptorIndexing = VK_TRUE;
	getPhysicalDeviceDescriptorIndexingFeatures(features);
	features->samplerFilterMinmax = VK_FALSE;
	getPhysicalDeviceScalarBlockLayoutFeatures(features);
	getPhysicalDeviceImagelessFramebufferFeatures(features);
	getPhysicalDeviceUniformBufferStandardLayoutFeatures(features);
	getPhysicalDeviceShaderSubgroupExtendedTypesFeatures(features);
	getPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR(features);
	getPhysicalDeviceHostQueryResetFeatures(features);
	getPhysicalDeviceTimelineSemaphoreFeatures(features);
	getPhysicalDeviceBufferDeviceAddressFeatures(features);
	getPhysicalDeviceVulkanMemoryModelFeatures(features);
	features->shaderOutputViewportIndex = VK_FALSE;
	features->shaderOutputLayer = VK_FALSE;
	features->subgroupBroadcastDynamicId = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceDepthClipEnableFeaturesEXT(T *features)
{
	features->depthClipEnable = VK_TRUE;
}

template<typename T>
static void getPhysicalDeviceVulkan13Features(T *features)
{
	getPhysicalDeviceImageRobustnessFeatures(features);
	getPhysicalDeviceInlineUniformBlockFeatures(features);
	getPhysicalDevicePipelineCreationCacheControlFeatures(features);
	getPhysicalDevicePrivateDataFeatures(features);
	getPhysicalDeviceShaderDemoteToHelperInvocationFeatures(features);
	getPhysicalDeviceShaderTerminateInvocationFeatures(features);
	getPhysicalDeviceSubgroupSizeControlFeatures(features);
	getPhysicalDeviceSynchronization2Features(features);
	getPhysicalDeviceTextureCompressionASTCHDRFeatures(features);
	getPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures(features);
	getPhysicalDeviceDynamicRenderingFeatures(features);
	getPhysicalDeviceShaderIntegerDotProductFeatures(features);
	getPhysicalDeviceMaintenance4Features(features);
}

static void getPhysicalDeviceCustomBorderColorFeaturesEXT(VkPhysicalDeviceCustomBorderColorFeaturesEXT *features)
{
	features->customBorderColors = VK_TRUE;
	features->customBorderColorWithoutFormat = VK_TRUE;
}

static void getPhysicalDeviceBlendOperationAdvancedFeaturesEXT(VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *features)
{
	features->advancedBlendCoherentOperations = VK_FALSE;
}

static void getPhysicalDeviceExtendedDynamicStateFeaturesEXT(VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *features)
{
	features->extendedDynamicState = VK_TRUE;
}

static void getPhysicalDeviceVertexInputDynamicStateFeaturesEXT(VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *features)
{
	features->vertexInputDynamicState = VK_TRUE;
}

static void getPhysicalDevice4444FormatsFeaturesEXT(VkPhysicalDevice4444FormatsFeaturesEXT *features)
{
	features->formatA4R4G4B4 = VK_TRUE;
	features->formatA4B4G4R4 = VK_TRUE;
}

static void getPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT(VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT *features)
{
	features->rasterizationOrderColorAttachmentAccess = VK_TRUE;
	features->rasterizationOrderDepthAttachmentAccess = VK_TRUE;
	features->rasterizationOrderStencilAttachmentAccess = VK_TRUE;
}

static void getPhysicalDeviceDepthClipControlFeaturesExt(VkPhysicalDeviceDepthClipControlFeaturesEXT *features)
{
	features->depthClipControl = VK_TRUE;
}

void PhysicalDevice::getFeatures2(VkPhysicalDeviceFeatures2 *features) const
{
	features->features = getFeatures();
	VkBaseOutStructure *curExtension = reinterpret_cast<VkBaseOutStructure *>(features->pNext);
	while(curExtension != nullptr)
	{
		switch(curExtension->sType)
		{
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES:
			getPhysicalDeviceVulkan11Features(reinterpret_cast<VkPhysicalDeviceVulkan11Features *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES:
			getPhysicalDeviceVulkan12Features(reinterpret_cast<VkPhysicalDeviceVulkan12Features *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES:
			getPhysicalDeviceVulkan13Features(reinterpret_cast<VkPhysicalDeviceVulkan13Features *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES:
			getPhysicalDeviceMultiviewFeatures(reinterpret_cast<VkPhysicalDeviceMultiviewFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES:
			getPhysicalDeviceVariablePointersFeatures(reinterpret_cast<VkPhysicalDeviceVariablePointersFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES:
			getPhysicalDevice16BitStorageFeatures(reinterpret_cast<VkPhysicalDevice16BitStorageFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES:
			getPhysicalDeviceSamplerYcbcrConversionFeatures(reinterpret_cast<VkPhysicalDeviceSamplerYcbcrConversionFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES:
			getPhysicalDeviceProtectedMemoryFeatures(reinterpret_cast<VkPhysicalDeviceProtectedMemoryFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES:
			getPhysicalDeviceShaderDrawParameterFeatures(reinterpret_cast<VkPhysicalDeviceShaderDrawParameterFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES:
			getPhysicalDeviceHostQueryResetFeatures(reinterpret_cast<VkPhysicalDeviceHostQueryResetFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES:
			getPhysicalDevicePipelineCreationCacheControlFeatures(reinterpret_cast<VkPhysicalDevicePipelineCreationCacheControlFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES:
			getPhysicalDeviceImageRobustnessFeatures(reinterpret_cast<VkPhysicalDeviceImageRobustnessFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT:
			getPhysicalDeviceLineRasterizationFeaturesEXT(reinterpret_cast<VkPhysicalDeviceLineRasterizationFeaturesEXT *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES:
			getPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR(reinterpret_cast<VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES:
			getPhysicalDevice8BitStorageFeaturesKHR(reinterpret_cast<VkPhysicalDevice8BitStorageFeaturesKHR *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT:
			getPhysicalDeviceProvokingVertexFeaturesEXT(reinterpret_cast<VkPhysicalDeviceProvokingVertexFeaturesEXT *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES:
			getPhysicalDeviceImagelessFramebufferFeatures(reinterpret_cast<VkPhysicalDeviceImagelessFramebufferFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES:
			getPhysicalDeviceShaderSubgroupExtendedTypesFeatures(reinterpret_cast<VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES:
			getPhysicalDeviceScalarBlockLayoutFeatures(reinterpret_cast<VkPhysicalDeviceScalarBlockLayoutFeatures *>(curExtension));
			break;
#ifdef SWIFTSHADER_DEVICE_MEMORY_REPORT
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT:
			getPhysicalDeviceDeviceMemoryReportFeaturesEXT(reinterpret_cast<VkPhysicalDeviceDeviceMemoryReportFeaturesEXT *>(curExtension));
			break;
#endif  // SWIFTSHADER_DEVICE_MEMORY_REPORT
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES:
			getPhysicalDeviceUniformBufferStandardLayoutFeatures(reinterpret_cast<VkPhysicalDeviceUniformBufferStandardLayoutFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES:
			getPhysicalDeviceVulkanMemoryModelFeatures(reinterpret_cast<VkPhysicalDeviceVulkanMemoryModelFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES:
			getPhysicalDeviceTimelineSemaphoreFeatures(reinterpret_cast<VkPhysicalDeviceTimelineSemaphoreFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES:
			getPhysicalDeviceShaderAtomicInt64Features(reinterpret_cast<VkPhysicalDeviceShaderAtomicInt64Features *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES:
			getPhysicalDeviceShaderFloat16Int8Features(reinterpret_cast<VkPhysicalDeviceShaderFloat16Int8Features *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES:
			getPhysicalDeviceBufferDeviceAddressFeatures(reinterpret_cast<VkPhysicalDeviceBufferDeviceAddressFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES:
			getPhysicalDeviceDynamicRenderingFeatures(reinterpret_cast<VkPhysicalDeviceDynamicRenderingFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR:
			getPhysicalDeviceDynamicRenderingLocalReadFeatures(reinterpret_cast<VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES:
			getPhysicalDeviceDescriptorIndexingFeatures(reinterpret_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT:
			getPhysicalDeviceDepthClipEnableFeaturesEXT(reinterpret_cast<VkPhysicalDeviceDepthClipEnableFeaturesEXT *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES:
			getPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures(reinterpret_cast<VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT:
			getPhysicalDeviceCustomBorderColorFeaturesEXT(reinterpret_cast<VkPhysicalDeviceCustomBorderColorFeaturesEXT *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT:
			getPhysicalDeviceBlendOperationAdvancedFeaturesEXT(reinterpret_cast<VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT:
			getPhysicalDeviceExtendedDynamicStateFeaturesEXT(reinterpret_cast<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT:
			getPhysicalDeviceVertexInputDynamicStateFeaturesEXT(reinterpret_cast<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES:
			getPhysicalDevicePrivateDataFeatures(reinterpret_cast<VkPhysicalDevicePrivateDataFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES:
			getPhysicalDeviceTextureCompressionASTCHDRFeatures(reinterpret_cast<VkPhysicalDeviceTextureCompressionASTCHDRFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES:
			getPhysicalDeviceShaderDemoteToHelperInvocationFeatures(reinterpret_cast<VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES:
			getPhysicalDeviceShaderTerminateInvocationFeatures(reinterpret_cast<VkPhysicalDeviceShaderTerminateInvocationFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES:
			getPhysicalDeviceSubgroupSizeControlFeatures(reinterpret_cast<VkPhysicalDeviceSubgroupSizeControlFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES:
			getPhysicalDeviceInlineUniformBlockFeatures(reinterpret_cast<VkPhysicalDeviceInlineUniformBlockFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT:
			getPhysicalDevice4444FormatsFeaturesEXT(reinterpret_cast<struct VkPhysicalDevice4444FormatsFeaturesEXT *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES:
			getPhysicalDeviceSynchronization2Features(reinterpret_cast<struct VkPhysicalDeviceSynchronization2Features *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES:
			getPhysicalDeviceShaderIntegerDotProductFeatures(reinterpret_cast<struct VkPhysicalDeviceShaderIntegerDotProductFeatures *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES:
			getPhysicalDeviceMaintenance4Features(reinterpret_cast<struct VkPhysicalDeviceMaintenance4Features *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT:
			getPhysicalDevicePrimitiveTopologyListRestartFeatures(reinterpret_cast<struct VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES_EXT:
			getPhysicalDevicePipelineRobustnessFeatures(reinterpret_cast<struct VkPhysicalDevicePipelineRobustnessFeaturesEXT *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT:
			getPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT(reinterpret_cast<struct VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES_KHR:
			getPhysicalDeviceGlobalPriorityQueryFeatures(reinterpret_cast<struct VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT:
			getPhysicalDeviceDepthClipControlFeaturesExt(reinterpret_cast<struct VkPhysicalDeviceDepthClipControlFeaturesEXT *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT:
			getPhysicalDeviceGraphicsPipelineLibraryFeatures(reinterpret_cast<struct VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT:
			getPhysicalDeviceSwapchainMaintenance1FeaturesKHR(reinterpret_cast<struct VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES_EXT:
			getPhysicalDeviceHostImageCopyFeatures(reinterpret_cast<struct VkPhysicalDeviceHostImageCopyFeaturesEXT *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT:
			getPhysicalDeviceIndexTypeUint8Features(reinterpret_cast<VkPhysicalDeviceIndexTypeUint8FeaturesEXT *>(curExtension));
			break;
		case VK_STRUCTURE_TYPE_MAX_ENUM:  // TODO(b/176893525): This may not be legal. dEQP tests that this value is ignored.
			break;
		default:
			UNSUPPORTED("curExtension->sType: %s", vk::Stringify(curExtension->sType).c_str());
			break;
		}
		curExtension = reinterpret_cast<VkBaseOutStructure *>(curExtension->pNext);
	}
}

VkSampleCountFlags PhysicalDevice::getSampleCounts()
{
	return VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT;
}

const VkPhysicalDeviceLimits &PhysicalDevice::getLimits()
{
	VkSampleCountFlags sampleCounts = getSampleCounts();

	static const VkPhysicalDeviceLimits limits = {
		1 << (vk::MAX_IMAGE_LEVELS_1D - 1),          // maxImageDimension1D
		1 << (vk::MAX_IMAGE_LEVELS_2D - 1),          // maxImageDimension2D
		1 << (vk::MAX_IMAGE_LEVELS_3D - 1),          // maxImageDimension3D
		1 << (vk::MAX_IMAGE_LEVELS_CUBE - 1),        // maxImageDimensionCube
		vk::MAX_IMAGE_ARRAY_LAYERS,                  // maxImageArrayLayers
		65536,                                       // maxTexelBufferElements
		65536,                                       // maxUniformBufferRange
		vk::MAX_MEMORY_ALLOCATION_SIZE,              // maxStorageBufferRange
		vk::MAX_PUSH_CONSTANT_SIZE,                  // maxPushConstantsSize
		4096,                                        // maxMemoryAllocationCount
		vk::MAX_SAMPLER_ALLOCATION_COUNT,            // maxSamplerAllocationCount
		4096,                                        // bufferImageGranularity
		0,                                           // sparseAddressSpaceSize (unsupported)
		MAX_BOUND_DESCRIPTOR_SETS,                   // maxBoundDescriptorSets
		64,                                          // maxPerStageDescriptorSamplers
		15,                                          // maxPerStageDescriptorUniformBuffers
		30,                                          // maxPerStageDescriptorStorageBuffers
		200,                                         // maxPerStageDescriptorSampledImages
		16,                                          // maxPerStageDescriptorStorageImages
		sw::MAX_COLOR_BUFFERS,                       // maxPerStageDescriptorInputAttachments
		200,                                         // maxPerStageResources
		576,                                         // maxDescriptorSetSamplers
		90,                                          // maxDescriptorSetUniformBuffers
		MAX_DESCRIPTOR_SET_UNIFORM_BUFFERS_DYNAMIC,  // maxDescriptorSetUniformBuffersDynamic
		96,                                          // maxDescriptorSetStorageBuffers
		MAX_DESCRIPTOR_SET_STORAGE_BUFFERS_DYNAMIC,  // maxDescriptorSetStorageBuffersDynamic
		1800,                                        // maxDescriptorSetSampledImages
		144,                                         // maxDescriptorSetStorageImages
		sw::MAX_COLOR_BUFFERS,                       // maxDescriptorSetInputAttachments
		16,                                          // maxVertexInputAttributes
		vk::MAX_VERTEX_INPUT_BINDINGS,               // maxVertexInputBindings
		2047,                                        // maxVertexInputAttributeOffset
		2048,                                        // maxVertexInputBindingStride
		sw::MAX_INTERFACE_COMPONENTS,                // maxVertexOutputComponents
		0,                                           // maxTessellationGenerationLevel (unsupported)
		0,                                           // maxTessellationPatchSize (unsupported)
		0,                                           // maxTessellationControlPerVertexInputComponents (unsupported)
		0,                                           // maxTessellationControlPerVertexOutputComponents (unsupported)
		0,                                           // maxTessellationControlPerPatchOutputComponents (unsupported)
		0,                                           // maxTessellationControlTotalOutputComponents (unsupported)
		0,                                           // maxTessellationEvaluationInputComponents (unsupported)
		0,                                           // maxTessellationEvaluationOutputComponents (unsupported)
		0,                                           // maxGeometryShaderInvocations (unsupported)
		0,                                           // maxGeometryInputComponents (unsupported)
		0,                                           // maxGeometryOutputComponents (unsupported)
		0,                                           // maxGeometryOutputVertices (unsupported)
		0,                                           // maxGeometryTotalOutputComponents (unsupported)
		sw::MAX_INTERFACE_COMPONENTS,                // maxFragmentInputComponents
		sw::MAX_COLOR_BUFFERS,                       // maxFragmentOutputAttachments
		1,                                           // maxFragmentDualSrcAttachments
		28,                                          // maxFragmentCombinedOutputResources
		32768,                                       // maxComputeSharedMemorySize
		{ 65535, 65535, 65535 },                     // maxComputeWorkGroupCount[3]
		vk::MAX_COMPUTE_WORKGROUP_INVOCATIONS,       // maxComputeWorkGroupInvocations
		{ 256, 256, 64 },                            // maxComputeWorkGroupSize[3]
		vk::SUBPIXEL_PRECISION_BITS,                 // subPixelPrecisionBits
		8,                                           // subTexelPrecisionBits
		6,                                           // mipmapPrecisionBits
		UINT32_MAX,                                  // maxDrawIndexedIndexValue
		UINT32_MAX,                                  // maxDrawIndirectCount
		vk::MAX_SAMPLER_LOD_BIAS,                    // maxSamplerLodBias
		16,                                          // maxSamplerAnisotropy
		MAX_VIEWPORTS,                               // maxViewports
		{ sw::MAX_VIEWPORT_DIM,
		  sw::MAX_VIEWPORT_DIM },  // maxViewportDimensions[2]
		{ -2 * sw::MAX_VIEWPORT_DIM,
		  2 * sw::MAX_VIEWPORT_DIM - 1 },                 // viewportBoundsRange[2]
		0,                                                // viewportSubPixelBits
		vk::MIN_MEMORY_MAP_ALIGNMENT,                     // minMemoryMapAlignment
		vk::MIN_TEXEL_BUFFER_OFFSET_ALIGNMENT,            // minTexelBufferOffsetAlignment
		vk::MIN_UNIFORM_BUFFER_OFFSET_ALIGNMENT,          // minUniformBufferOffsetAlignment
		vk::MIN_STORAGE_BUFFER_OFFSET_ALIGNMENT,          // minStorageBufferOffsetAlignment
		sw::MIN_TEXEL_OFFSET,                             // minTexelOffset
		sw::MAX_TEXEL_OFFSET,                             // maxTexelOffset
		sw::MIN_TEXEL_OFFSET,                             // minTexelGatherOffset
		sw::MAX_TEXEL_OFFSET,                             // maxTexelGatherOffset
		-0.5,                                             // minInterpolationOffset
		0.5,                                              // maxInterpolationOffset
		4,                                                // subPixelInterpolationOffsetBits
		sw::MAX_FRAMEBUFFER_DIM,                          // maxFramebufferWidth
		sw::MAX_FRAMEBUFFER_DIM,                          // maxFramebufferHeight
		256,                                              // maxFramebufferLayers
		sampleCounts,                                     // framebufferColorSampleCounts
		sampleCounts,                                     // framebufferDepthSampleCounts
		sampleCounts,                                     // framebufferStencilSampleCounts
		sampleCounts,                                     // framebufferNoAttachmentsSampleCounts
		sw::MAX_COLOR_BUFFERS,                            // maxColorAttachments
		sampleCounts,                                     // sampledImageColorSampleCounts
		sampleCounts,                                     // sampledImageIntegerSampleCounts
		sampleCounts,                                     // sampledImageDepthSampleCounts
		sampleCounts,                                     // sampledImageStencilSampleCounts
		sampleCounts,                                     // storageImageSampleCounts
		1,                                                // maxSampleMaskWords
		VK_TRUE,                                          // timestampComputeAndGraphics
		1,                                                // timestampPeriod
		sw::MAX_CLIP_DISTANCES,                           // maxClipDistances
		sw::MAX_CULL_DISTANCES,                           // maxCullDistances
		sw::MAX_CLIP_DISTANCES + sw::MAX_CULL_DISTANCES,  // maxCombinedClipAndCullDistances
		2,                                                // discreteQueuePriorities
		{ 1.0, vk::MAX_POINT_SIZE },                      // pointSizeRange[2]
		{ 1.0, 1.0 },                                     // lineWidthRange[2] (unsupported)
		0.0,                                              // pointSizeGranularity (unsupported)
		0.0,                                              // lineWidthGranularity (unsupported)
		VK_TRUE,                                          // strictLines
		VK_TRUE,                                          // standardSampleLocations
		64,                                               // optimalBufferCopyOffsetAlignment
		64,                                               // optimalBufferCopyRowPitchAlignment
		256,                                              // nonCoherentAtomSize
	};

	return limits;
}

const VkPhysicalDeviceProperties &PhysicalDevice::getProperties() const
{
	auto getProperties = [&]() -> VkPhysicalDeviceProperties {
		VkPhysicalDeviceProperties properties = {
			API_VERSION,
			DRIVER_VERSION,
			VENDOR_ID,
			DEVICE_ID,
			VK_PHYSICAL_DEVICE_TYPE_CPU,  // deviceType
			"",                           // deviceName
			SWIFTSHADER_UUID,             // pipelineCacheUUID
			getLimits(),                  // limits
			{}                            // sparseProperties
		};

		// Append Reactor JIT backend name and version
		snprintf(properties.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE,
		         "%s (%s)", SWIFTSHADER_DEVICE_NAME, rr::Caps::backendName().c_str());

		return properties;
	};

	static const VkPhysicalDeviceProperties properties = getProperties();
	return properties;
}

template<typename T>
static void getIdProperties(T *properties)
{
	memset(properties->deviceUUID, 0, VK_UUID_SIZE);
	memset(properties->driverUUID, 0, VK_UUID_SIZE);
	memset(properties->deviceLUID, 0, VK_LUID_SIZE);

	memcpy(properties->deviceUUID, SWIFTSHADER_UUID, VK_UUID_SIZE);
	*((uint64_t *)properties->driverUUID) = DRIVER_VERSION;

	properties->deviceNodeMask = 0;
	properties->deviceLUIDValid = VK_FALSE;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceIDProperties *properties) const
{
	getIdProperties(properties);
}

template<typename T>
static void getMaintenance3Properties(T *properties)
{
	properties->maxMemoryAllocationSize = MAX_MEMORY_ALLOCATION_SIZE;
	properties->maxPerSetDescriptors = 1024;
}

template<typename T>
static void getMaintenance4Properties(T *properties)
{
	properties->maxBufferSize = MAX_MEMORY_ALLOCATION_SIZE;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceMaintenance3Properties *properties) const
{
	getMaintenance3Properties(properties);
}

void PhysicalDevice::getProperties(VkPhysicalDeviceMaintenance4Properties *properties) const
{
	getMaintenance4Properties(properties);
}

template<typename T>
static void getMultiviewProperties(T *properties)
{
	properties->maxMultiviewViewCount = 6;
	properties->maxMultiviewInstanceIndex = 1u << 27;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceMultiviewProperties *properties) const
{
	getMultiviewProperties(properties);
}

template<typename T>
static void getPointClippingProperties(T *properties)
{
	properties->pointClippingBehavior = VK_POINT_CLIPPING_BEHAVIOR_ALL_CLIP_PLANES;
}

void PhysicalDevice::getProperties(VkPhysicalDevicePointClippingProperties *properties) const
{
	getPointClippingProperties(properties);
}

template<typename T>
static void getProtectedMemoryProperties(T *properties)
{
	properties->protectedNoFault = VK_FALSE;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceProtectedMemoryProperties *properties) const
{
	getProtectedMemoryProperties(properties);
}

template<typename T>
static void getSubgroupProperties(T *properties)
{
	properties->subgroupSize = sw::SIMD::Width;
	properties->supportedStages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
	properties->supportedOperations =
	    VK_SUBGROUP_FEATURE_BASIC_BIT |
	    VK_SUBGROUP_FEATURE_VOTE_BIT |
	    VK_SUBGROUP_FEATURE_ARITHMETIC_BIT |
	    VK_SUBGROUP_FEATURE_BALLOT_BIT |
	    VK_SUBGROUP_FEATURE_SHUFFLE_BIT |
	    VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT |
	    VK_SUBGROUP_FEATURE_QUAD_BIT;
	properties->quadOperationsInAllStages = VK_FALSE;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceSubgroupProperties *properties) const
{
	getSubgroupProperties(properties);
}

void PhysicalDevice::getProperties(VkPhysicalDeviceVulkan11Properties *properties) const
{
	getIdProperties(properties);

	// We can't use templated functions for Vulkan11 & subgroup properties. The names of the
	// variables in VkPhysicalDeviceSubgroupProperties differ from the names in the Vulkan11
	// struct.
	VkPhysicalDeviceSubgroupProperties subgroupProperties = {};
	getProperties(&subgroupProperties);
	properties->subgroupSize = subgroupProperties.subgroupSize;
	properties->subgroupSupportedStages = subgroupProperties.supportedStages;
	properties->subgroupSupportedOperations = subgroupProperties.supportedOperations;
	properties->subgroupQuadOperationsInAllStages = subgroupProperties.quadOperationsInAllStages;

	getPointClippingProperties(properties);
	getMultiviewProperties(properties);
	getProtectedMemoryProperties(properties);
	getMaintenance3Properties(properties);
}

void PhysicalDevice::getProperties(const VkExternalMemoryHandleTypeFlagBits *handleType, VkExternalImageFormatProperties *properties) const
{
	VkExternalMemoryProperties *extMemProperties = &properties->externalMemoryProperties;
#if SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD
	if(*handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT)
	{
		extMemProperties->compatibleHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
		extMemProperties->exportFromImportedHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
		extMemProperties->externalMemoryFeatures = VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT;
		return;
	}
#endif
#if SWIFTSHADER_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER
	if(*handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)
	{
		extMemProperties->compatibleHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
		extMemProperties->exportFromImportedHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
		extMemProperties->externalMemoryFeatures = VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT;
		return;
	}
#endif
#if VK_USE_PLATFORM_FUCHSIA
	if(*handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA)
	{
		extMemProperties->compatibleHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA;
		extMemProperties->exportFromImportedHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA;
		extMemProperties->externalMemoryFeatures = VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT;
		return;
	}
#endif
	extMemProperties->compatibleHandleTypes = 0;
	extMemProperties->exportFromImportedHandleTypes = 0;
	extMemProperties->externalMemoryFeatures = 0;
}

void PhysicalDevice::getProperties(const VkExternalMemoryHandleTypeFlagBits *handleType, VkExternalBufferProperties *properties) const
{
	VkExternalMemoryProperties *extMemProperties = &properties->externalMemoryProperties;
#if SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD
	if(*handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT)
	{
		extMemProperties->compatibleHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
		extMemProperties->exportFromImportedHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
		extMemProperties->externalMemoryFeatures = VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT;
		return;
	}
#endif
#if SWIFTSHADER_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER
	if(*handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)
	{
		extMemProperties->compatibleHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
		extMemProperties->exportFromImportedHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
		extMemProperties->externalMemoryFeatures = VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT;
		return;
	}
#endif
#if VK_USE_PLATFORM_FUCHSIA
	if(*handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA)
	{
		extMemProperties->compatibleHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA;
		extMemProperties->exportFromImportedHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA;
		extMemProperties->externalMemoryFeatures = VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT;
		return;
	}
#endif
	extMemProperties->compatibleHandleTypes = 0;
	extMemProperties->exportFromImportedHandleTypes = 0;
	extMemProperties->externalMemoryFeatures = 0;
}

void PhysicalDevice::getProperties(VkSamplerYcbcrConversionImageFormatProperties *properties) const
{
	properties->combinedImageSamplerDescriptorCount = 1;  // Need only one descriptor for YCbCr sampling.
}

#ifdef __ANDROID__
void PhysicalDevice::getProperties(VkPhysicalDevicePresentationPropertiesANDROID *properties) const
{
	properties->sharedImage = VK_FALSE;
}

void PhysicalDevice::getProperties(const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo, VkAndroidHardwareBufferUsageANDROID *ahbProperties) const
{
	// Maps VkImageUsageFlags to AHB usage flags using this table from the Vulkan spec
	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#memory-external-android-hardware-buffer-usage

	// VK_IMAGE_CREATE_PROTECTED_BIT not currently supported.
	ASSERT((pImageFormatInfo->flags & VK_IMAGE_CREATE_PROTECTED_BIT) == 0);

	// "It must include at least one GPU usage flag (AHARDWAREBUFFER_USAGE_GPU_*), even if none of the corresponding Vulkan usages or flags are requested."
	uint64_t ahbUsage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;

	// Already covered by the default GPU usage flag above.
	//
	// if ((vkUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) || (vkUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT))
	// {
	// 	 ahbUsage |= AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
	// }

	if((pImageFormatInfo->usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) || (pImageFormatInfo->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
	{
		ahbUsage |= AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER;
	}

	if(pImageFormatInfo->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
	{
		ahbUsage |= AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP;
	}

	if(pImageFormatInfo->flags & VK_IMAGE_CREATE_PROTECTED_BIT)
	{
		ahbUsage |= AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;
	}

	ahbProperties->androidHardwareBufferUsage = ahbUsage;
}
#endif

void PhysicalDevice::getProperties(const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo, VkExternalBufferProperties *pExternalBufferProperties) const
{
	VkExternalMemoryProperties *properties = &pExternalBufferProperties->externalMemoryProperties;

#if SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD || SWIFTSHADER_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER
	const VkExternalMemoryHandleTypeFlagBits *handleType = &pExternalBufferInfo->handleType;
#endif

#if SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD
	if(*handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT)
	{
		properties->compatibleHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
		properties->exportFromImportedHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
		properties->externalMemoryFeatures = VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT;
		return;
	}
#endif
#if SWIFTSHADER_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER
	if(*handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)
	{
		properties->compatibleHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
		properties->exportFromImportedHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
		properties->externalMemoryFeatures = VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT;
		return;
	}
#endif
	properties->compatibleHandleTypes = 0;
	properties->exportFromImportedHandleTypes = 0;
	properties->externalMemoryFeatures = 0;
}

void PhysicalDevice::getProperties(const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo, VkExternalFenceProperties *pExternalFenceProperties) const
{
	pExternalFenceProperties->compatibleHandleTypes = 0;
	pExternalFenceProperties->exportFromImportedHandleTypes = 0;
	pExternalFenceProperties->externalFenceFeatures = 0;
}

void PhysicalDevice::getProperties(const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo, VkExternalSemaphoreProperties *pExternalSemaphoreProperties) const
{
	for(const auto *nextInfo = reinterpret_cast<const VkBaseInStructure *>(pExternalSemaphoreInfo->pNext);
	    nextInfo != nullptr; nextInfo = nextInfo->pNext)
	{
		switch(nextInfo->sType)
		{
		case VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO:
			{
				const auto *tlsInfo = reinterpret_cast<const VkSemaphoreTypeCreateInfo *>(nextInfo);
				// Timeline Semaphore does not support external semaphore
				if(tlsInfo->semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE)
				{
					pExternalSemaphoreProperties->compatibleHandleTypes = 0;
					pExternalSemaphoreProperties->exportFromImportedHandleTypes = 0;
					pExternalSemaphoreProperties->externalSemaphoreFeatures = 0;
					return;
				}
			}
			break;
		default:
			WARN("nextInfo->sType = %s", vk::Stringify(nextInfo->sType).c_str());
			break;
		}
	}

#if SWIFTSHADER_EXTERNAL_SEMAPHORE_OPAQUE_FD
	if(pExternalSemaphoreInfo->handleType == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT)
	{
		pExternalSemaphoreProperties->compatibleHandleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
		pExternalSemaphoreProperties->exportFromImportedHandleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
		pExternalSemaphoreProperties->externalSemaphoreFeatures = VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT;
		return;
	}
#endif
#if VK_USE_PLATFORM_FUCHSIA
	if(pExternalSemaphoreInfo->handleType == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA)
	{
		pExternalSemaphoreProperties->compatibleHandleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA;
		pExternalSemaphoreProperties->exportFromImportedHandleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA;
		pExternalSemaphoreProperties->externalSemaphoreFeatures = VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT;
		return;
	}
#endif
	pExternalSemaphoreProperties->compatibleHandleTypes = 0;
	pExternalSemaphoreProperties->exportFromImportedHandleTypes = 0;
	pExternalSemaphoreProperties->externalSemaphoreFeatures = 0;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceExternalMemoryHostPropertiesEXT *properties) const
{
	properties->minImportedHostPointerAlignment = vk::MIN_IMPORTED_HOST_POINTER_ALIGNMENT;
}

template<typename T>
static void getDriverProperties(T *properties)
{
	properties->driverID = VK_DRIVER_ID_GOOGLE_SWIFTSHADER_KHR;
	strcpy(properties->driverName, "SwiftShader driver");
	strcpy(properties->driverInfo, "");
	properties->conformanceVersion = { 1, 3, 3, 1 };
}

void PhysicalDevice::getProperties(VkPhysicalDeviceDriverProperties *properties) const
{
	getDriverProperties(properties);
}

void PhysicalDevice::getProperties(VkPhysicalDeviceLineRasterizationPropertiesEXT *properties) const
{
	properties->lineSubPixelPrecisionBits = vk::SUBPIXEL_PRECISION_BITS;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceProvokingVertexPropertiesEXT *properties) const
{
	properties->provokingVertexModePerPipeline = VK_TRUE;
}

template<typename T>
static void getFloatControlsProperties(T *properties)
{
	// The spec states:
	// shaderSignedZeroInfNanPreserveFloat32 is a boolean value indicating whether
	// sign of a zero, Nans and +/-infinity can be preserved in 32-bit floating-point
	// computations. It also indicates whether the SignedZeroInfNanPreserve execution
	// mode can be used for 32-bit floating-point types.
	//
	// There are similar clauses for all the shader* bools present here.
	//
	// It does not state that an implementation must report its default behavior using
	// these variables. At this time SwiftShader does not expose any preserve, denormal,
	// or rounding controls.
	properties->denormBehaviorIndependence = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE;
	properties->roundingModeIndependence = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE;
	properties->shaderSignedZeroInfNanPreserveFloat16 = VK_TRUE;
	properties->shaderSignedZeroInfNanPreserveFloat32 = VK_TRUE;
	properties->shaderSignedZeroInfNanPreserveFloat64 = VK_TRUE;
	properties->shaderDenormPreserveFloat16 = VK_FALSE;
	properties->shaderDenormPreserveFloat32 = VK_FALSE;
	properties->shaderDenormPreserveFloat64 = VK_FALSE;
	properties->shaderDenormFlushToZeroFloat16 = VK_FALSE;
	properties->shaderDenormFlushToZeroFloat32 = VK_FALSE;
	properties->shaderDenormFlushToZeroFloat64 = VK_FALSE;
	properties->shaderRoundingModeRTZFloat16 = VK_FALSE;
	properties->shaderRoundingModeRTZFloat32 = VK_FALSE;
	properties->shaderRoundingModeRTZFloat64 = VK_FALSE;
	properties->shaderRoundingModeRTEFloat16 = VK_FALSE;
	properties->shaderRoundingModeRTEFloat32 = VK_FALSE;
	properties->shaderRoundingModeRTEFloat64 = VK_FALSE;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceFloatControlsProperties *properties) const
{
	getFloatControlsProperties(properties);
}

template<typename T>
static void getDescriptorIndexingProperties(T *properties)
{
	// "The UpdateAfterBind descriptor limits must each be greater than or equal to
	//  the corresponding non-UpdateAfterBind limit."
	const VkPhysicalDeviceLimits &limits = PhysicalDevice::getLimits();

	// Limits from:
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#limits-minmax
	// Table 53. Required Limits
	properties->maxUpdateAfterBindDescriptorsInAllPools = vk::MAX_UPDATE_AFTER_BIND_DESCRIPTORS;
	properties->shaderUniformBufferArrayNonUniformIndexingNative = VK_FALSE;
	properties->shaderSampledImageArrayNonUniformIndexingNative = VK_FALSE;
	properties->shaderStorageBufferArrayNonUniformIndexingNative = VK_FALSE;
	properties->shaderStorageImageArrayNonUniformIndexingNative = VK_FALSE;
	properties->shaderInputAttachmentArrayNonUniformIndexingNative = VK_FALSE;
	properties->robustBufferAccessUpdateAfterBind = VK_FALSE;
	properties->quadDivergentImplicitLod = VK_FALSE;
	properties->maxPerStageDescriptorUpdateAfterBindSamplers = vk::MAX_UPDATE_AFTER_BIND_DESCRIPTORS;
	properties->maxPerStageDescriptorUpdateAfterBindUniformBuffers = limits.maxPerStageDescriptorUniformBuffers;
	properties->maxPerStageDescriptorUpdateAfterBindStorageBuffers = vk::MAX_UPDATE_AFTER_BIND_DESCRIPTORS;
	properties->maxPerStageDescriptorUpdateAfterBindSampledImages = vk::MAX_UPDATE_AFTER_BIND_DESCRIPTORS;
	properties->maxPerStageDescriptorUpdateAfterBindStorageImages = vk::MAX_UPDATE_AFTER_BIND_DESCRIPTORS;
	properties->maxPerStageDescriptorUpdateAfterBindInputAttachments = limits.maxPerStageDescriptorInputAttachments;
	properties->maxPerStageUpdateAfterBindResources = vk::MAX_UPDATE_AFTER_BIND_DESCRIPTORS;
	properties->maxDescriptorSetUpdateAfterBindSamplers = vk::MAX_UPDATE_AFTER_BIND_DESCRIPTORS;
	properties->maxDescriptorSetUpdateAfterBindUniformBuffers = limits.maxDescriptorSetUniformBuffers;
	properties->maxDescriptorSetUpdateAfterBindUniformBuffersDynamic = limits.maxDescriptorSetUniformBuffersDynamic;
	properties->maxDescriptorSetUpdateAfterBindStorageBuffers = vk::MAX_UPDATE_AFTER_BIND_DESCRIPTORS;
	properties->maxDescriptorSetUpdateAfterBindStorageBuffersDynamic = limits.maxDescriptorSetStorageBuffersDynamic;
	properties->maxDescriptorSetUpdateAfterBindSampledImages = vk::MAX_UPDATE_AFTER_BIND_DESCRIPTORS;
	properties->maxDescriptorSetUpdateAfterBindStorageImages = vk::MAX_UPDATE_AFTER_BIND_DESCRIPTORS;
	properties->maxDescriptorSetUpdateAfterBindInputAttachments = limits.maxDescriptorSetInputAttachments;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceDescriptorIndexingProperties *properties) const
{
	getDescriptorIndexingProperties(properties);
}

template<typename T>
static void getDepthStencilResolveProperties(T *properties)
{
	properties->supportedDepthResolveModes = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT | VK_RESOLVE_MODE_NONE;
	properties->supportedStencilResolveModes = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT | VK_RESOLVE_MODE_NONE;
	properties->independentResolveNone = VK_TRUE;
	properties->independentResolve = VK_TRUE;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceDepthStencilResolveProperties *properties) const
{
	getDepthStencilResolveProperties(properties);
}

void PhysicalDevice::getProperties(VkPhysicalDeviceCustomBorderColorPropertiesEXT *properties) const
{
	properties->maxCustomBorderColorSamplers = MAX_SAMPLER_ALLOCATION_COUNT;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT *properties) const
{
	properties->advancedBlendMaxColorAttachments = sw::MAX_COLOR_BUFFERS;
	properties->advancedBlendIndependentBlend = VK_FALSE;
	properties->advancedBlendNonPremultipliedSrcColor = VK_FALSE;
	properties->advancedBlendNonPremultipliedDstColor = VK_FALSE;
	properties->advancedBlendCorrelatedOverlap = VK_FALSE;
	properties->advancedBlendAllOperations = VK_FALSE;
}

template<typename T>
static void getSubgroupSizeControlProperties(T *properties)
{
	VkPhysicalDeviceSubgroupProperties subgroupProperties = {};
	getSubgroupProperties(&subgroupProperties);
	properties->minSubgroupSize = subgroupProperties.subgroupSize;
	properties->maxSubgroupSize = subgroupProperties.subgroupSize;
	properties->maxComputeWorkgroupSubgroups = vk::MAX_COMPUTE_WORKGROUP_INVOCATIONS /
	                                           properties->minSubgroupSize;
	properties->requiredSubgroupSizeStages = subgroupProperties.supportedStages;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceSubgroupSizeControlProperties *properties) const
{
	getSubgroupSizeControlProperties(properties);
}

template<typename T>
static void getInlineUniformBlockProperties(T *properties)
{
	properties->maxInlineUniformBlockSize = MAX_INLINE_UNIFORM_BLOCK_SIZE;
	properties->maxPerStageDescriptorInlineUniformBlocks = 4;
	properties->maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks = 4;
	properties->maxDescriptorSetInlineUniformBlocks = 4;
	properties->maxDescriptorSetUpdateAfterBindInlineUniformBlocks = 4;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceInlineUniformBlockProperties *properties) const
{
	getInlineUniformBlockProperties(properties);
}

template<typename T>
static void getTexelBufferAlignmentProperties(T *properties)
{
	properties->storageTexelBufferOffsetAlignmentBytes = vk::MIN_TEXEL_BUFFER_OFFSET_ALIGNMENT;
	properties->storageTexelBufferOffsetSingleTexelAlignment = VK_FALSE;
	properties->uniformTexelBufferOffsetAlignmentBytes = vk::MIN_TEXEL_BUFFER_OFFSET_ALIGNMENT;
	properties->uniformTexelBufferOffsetSingleTexelAlignment = VK_FALSE;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceTexelBufferAlignmentProperties *properties) const
{
	getTexelBufferAlignmentProperties(properties);
}

template<typename T>
static void getShaderIntegerDotProductProperties(T *properties)
{
	properties->integerDotProduct8BitUnsignedAccelerated = VK_FALSE;
	properties->integerDotProduct8BitSignedAccelerated = VK_FALSE;
	properties->integerDotProduct8BitMixedSignednessAccelerated = VK_FALSE;
	properties->integerDotProduct4x8BitPackedUnsignedAccelerated = VK_FALSE;
	properties->integerDotProduct4x8BitPackedSignedAccelerated = VK_FALSE;
	properties->integerDotProduct4x8BitPackedMixedSignednessAccelerated = VK_FALSE;
	properties->integerDotProduct16BitUnsignedAccelerated = VK_FALSE;
	properties->integerDotProduct16BitSignedAccelerated = VK_FALSE;
	properties->integerDotProduct16BitMixedSignednessAccelerated = VK_FALSE;
	properties->integerDotProduct32BitUnsignedAccelerated = VK_FALSE;
	properties->integerDotProduct32BitSignedAccelerated = VK_FALSE;
	properties->integerDotProduct32BitMixedSignednessAccelerated = VK_FALSE;
	properties->integerDotProduct64BitUnsignedAccelerated = VK_FALSE;
	properties->integerDotProduct64BitSignedAccelerated = VK_FALSE;
	properties->integerDotProduct64BitMixedSignednessAccelerated = VK_FALSE;
	properties->integerDotProductAccumulatingSaturating8BitUnsignedAccelerated = VK_FALSE;
	properties->integerDotProductAccumulatingSaturating8BitSignedAccelerated = VK_FALSE;
	properties->integerDotProductAccumulatingSaturating8BitMixedSignednessAccelerated = VK_FALSE;
	properties->integerDotProductAccumulatingSaturating4x8BitPackedUnsignedAccelerated = VK_FALSE;
	properties->integerDotProductAccumulatingSaturating4x8BitPackedSignedAccelerated = VK_FALSE;
	properties->integerDotProductAccumulatingSaturating4x8BitPackedMixedSignednessAccelerated = VK_FALSE;
	properties->integerDotProductAccumulatingSaturating16BitUnsignedAccelerated = VK_FALSE;
	properties->integerDotProductAccumulatingSaturating16BitSignedAccelerated = VK_FALSE;
	properties->integerDotProductAccumulatingSaturating16BitMixedSignednessAccelerated = VK_FALSE;
	properties->integerDotProductAccumulatingSaturating32BitUnsignedAccelerated = VK_FALSE;
	properties->integerDotProductAccumulatingSaturating32BitSignedAccelerated = VK_FALSE;
	properties->integerDotProductAccumulatingSaturating32BitMixedSignednessAccelerated = VK_FALSE;
	properties->integerDotProductAccumulatingSaturating64BitUnsignedAccelerated = VK_FALSE;
	properties->integerDotProductAccumulatingSaturating64BitSignedAccelerated = VK_FALSE;
	properties->integerDotProductAccumulatingSaturating64BitMixedSignednessAccelerated = VK_FALSE;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceShaderIntegerDotProductProperties *properties) const
{
	getShaderIntegerDotProductProperties(properties);
}

template<typename T>
static void getGraphicsPipelineLibraryProperties(T *properties)
{
	// Library linking is currently fast in SwiftShader, because all the pipeline creation cost
	// is actually paid at draw time.
	properties->graphicsPipelineLibraryFastLinking = VK_TRUE;
	// TODO: check this
	properties->graphicsPipelineLibraryIndependentInterpolationDecoration = VK_FALSE;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT *properties) const
{
	getGraphicsPipelineLibraryProperties(properties);
}

template<typename T>
static void getSamplerFilterMinmaxProperties(T *properties)
{
	properties->filterMinmaxSingleComponentFormats = VK_FALSE;
	properties->filterMinmaxImageComponentMapping = VK_FALSE;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceSamplerFilterMinmaxProperties *properties) const
{
	getSamplerFilterMinmaxProperties(properties);
}

template<typename T>
static void getTimelineSemaphoreProperties(T *properties)
{
	// Our implementation of Timeline Semaphores allows the timeline to advance to any value from any value.
	properties->maxTimelineSemaphoreValueDifference = (uint64_t)-1;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceTimelineSemaphoreProperties *properties) const
{
	getTimelineSemaphoreProperties(properties);
}

template<typename T>
static void getPipelineRobustnessProperties(T *properties)
{
	// Buffer access is not robust by default.
	properties->defaultRobustnessStorageBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED_EXT;
	properties->defaultRobustnessUniformBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED_EXT;
	properties->defaultRobustnessVertexInputs = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED_EXT;
	// SwiftShader currently provides robustImageAccess robustness unconditionally.
	// robustImageAccess2 is not supported.
	// TODO(b/162327166): Only provide robustness when requested.
	properties->defaultRobustnessImages = VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_EXT;
}

void PhysicalDevice::getProperties(VkPhysicalDevicePipelineRobustnessPropertiesEXT *properties) const
{
	getPipelineRobustnessProperties(properties);
}

template<typename T>
static void getHostImageCopyProperties(T *properties)
{
	// There are no image layouts in SwiftShader, so support all layouts for host image copy
	constexpr VkImageLayout kAllLayouts[] = {
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR,
	};
	constexpr uint32_t kAllLayoutsCount = std::size(kAllLayouts);

	if(properties->pCopySrcLayouts == nullptr)
	{
		properties->copySrcLayoutCount = kAllLayoutsCount;
	}
	else
	{
		properties->copySrcLayoutCount = std::min(properties->copySrcLayoutCount, kAllLayoutsCount);
		memcpy(properties->pCopySrcLayouts, kAllLayouts, properties->copySrcLayoutCount * sizeof(*properties->pCopySrcLayouts));
	}

	if(properties->pCopyDstLayouts == nullptr)
	{
		properties->copyDstLayoutCount = kAllLayoutsCount;
	}
	else
	{
		properties->copyDstLayoutCount = std::min(properties->copyDstLayoutCount, kAllLayoutsCount);
		memcpy(properties->pCopyDstLayouts, kAllLayouts, properties->copyDstLayoutCount * sizeof(*properties->pCopyDstLayouts));
	}

	memcpy(properties->optimalTilingLayoutUUID, SWIFTSHADER_UUID, VK_UUID_SIZE);
	properties->identicalMemoryTypeRequirements = VK_TRUE;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceHostImageCopyPropertiesEXT *properties) const
{
	getHostImageCopyProperties(properties);
}

void PhysicalDevice::getProperties(VkPhysicalDeviceVulkan12Properties *properties) const
{
	getDriverProperties(properties);
	getFloatControlsProperties(properties);
	getDescriptorIndexingProperties(properties);
	getDepthStencilResolveProperties(properties);
	getSamplerFilterMinmaxProperties(properties);
	getTimelineSemaphoreProperties(properties);
	properties->framebufferIntegerColorSampleCounts = VK_SAMPLE_COUNT_1_BIT;
}

void PhysicalDevice::getProperties(VkPhysicalDeviceVulkan13Properties *properties) const
{
	getSubgroupSizeControlProperties(properties);
	getInlineUniformBlockProperties(properties);
	properties->maxInlineUniformTotalSize = properties->maxInlineUniformBlockSize *
	                                        properties->maxDescriptorSetInlineUniformBlocks;
	getShaderIntegerDotProductProperties(properties);
	getTexelBufferAlignmentProperties(properties);
	getMaintenance4Properties(properties);
}

bool PhysicalDevice::hasFeatures(const VkPhysicalDeviceFeatures &requestedFeatures) const
{
	const VkPhysicalDeviceFeatures &supportedFeatures = getFeatures();
	const VkBool32 *supportedFeature = reinterpret_cast<const VkBool32 *>(&supportedFeatures);
	const VkBool32 *requestedFeature = reinterpret_cast<const VkBool32 *>(&requestedFeatures);
	constexpr auto featureCount = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);

	for(unsigned int i = 0; i < featureCount; i++)
	{
		if((requestedFeature[i] != VK_FALSE) && (supportedFeature[i] == VK_FALSE))
		{
			return false;
		}
	}

	return true;
}

// CheckFeature returns false if requested is asking for a feature that is not supported
#define CheckFeature(requested, supported, feature) (requested->feature == VK_FALSE || supported.feature == VK_TRUE)

template<typename T>
T PhysicalDevice::getSupportedFeatures(const T *requested) const
{
	VkPhysicalDeviceFeatures2 features;
	features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	T supported;
	supported.sType = requested->sType;
	supported.pNext = nullptr;
	features.pNext = &supported;
	getFeatures2(&features);
	return supported;
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceLineRasterizationFeaturesEXT *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, rectangularLines) &&
	       CheckFeature(requested, supported, bresenhamLines) &&
	       CheckFeature(requested, supported, smoothLines) &&
	       CheckFeature(requested, supported, stippledRectangularLines) &&
	       CheckFeature(requested, supported, stippledBresenhamLines) &&
	       CheckFeature(requested, supported, stippledSmoothLines);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceProvokingVertexFeaturesEXT *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, provokingVertexLast) &&
	       CheckFeature(requested, supported, transformFeedbackPreservesProvokingVertex);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceVulkan11Features *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, storageBuffer16BitAccess) &&
	       CheckFeature(requested, supported, uniformAndStorageBuffer16BitAccess) &&
	       CheckFeature(requested, supported, storagePushConstant16) &&
	       CheckFeature(requested, supported, storageInputOutput16) &&
	       CheckFeature(requested, supported, multiview) &&
	       CheckFeature(requested, supported, multiviewGeometryShader) &&
	       CheckFeature(requested, supported, multiviewTessellationShader) &&
	       CheckFeature(requested, supported, variablePointersStorageBuffer) &&
	       CheckFeature(requested, supported, variablePointers) &&
	       CheckFeature(requested, supported, protectedMemory) &&
	       CheckFeature(requested, supported, samplerYcbcrConversion) &&
	       CheckFeature(requested, supported, shaderDrawParameters);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceVulkan12Features *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, samplerMirrorClampToEdge) &&
	       CheckFeature(requested, supported, drawIndirectCount) &&
	       CheckFeature(requested, supported, storageBuffer8BitAccess) &&
	       CheckFeature(requested, supported, uniformAndStorageBuffer8BitAccess) &&
	       CheckFeature(requested, supported, storagePushConstant8) &&
	       CheckFeature(requested, supported, shaderBufferInt64Atomics) &&
	       CheckFeature(requested, supported, shaderSharedInt64Atomics) &&
	       CheckFeature(requested, supported, shaderFloat16) &&
	       CheckFeature(requested, supported, shaderInt8) &&
	       CheckFeature(requested, supported, descriptorIndexing) &&
	       CheckFeature(requested, supported, shaderInputAttachmentArrayDynamicIndexing) &&
	       CheckFeature(requested, supported, shaderUniformTexelBufferArrayDynamicIndexing) &&
	       CheckFeature(requested, supported, shaderStorageTexelBufferArrayDynamicIndexing) &&
	       CheckFeature(requested, supported, shaderUniformBufferArrayNonUniformIndexing) &&
	       CheckFeature(requested, supported, shaderSampledImageArrayNonUniformIndexing) &&
	       CheckFeature(requested, supported, shaderStorageBufferArrayNonUniformIndexing) &&
	       CheckFeature(requested, supported, shaderStorageImageArrayNonUniformIndexing) &&
	       CheckFeature(requested, supported, shaderInputAttachmentArrayNonUniformIndexing) &&
	       CheckFeature(requested, supported, shaderUniformTexelBufferArrayNonUniformIndexing) &&
	       CheckFeature(requested, supported, shaderStorageTexelBufferArrayNonUniformIndexing) &&
	       CheckFeature(requested, supported, descriptorBindingUniformBufferUpdateAfterBind) &&
	       CheckFeature(requested, supported, descriptorBindingSampledImageUpdateAfterBind) &&
	       CheckFeature(requested, supported, descriptorBindingStorageImageUpdateAfterBind) &&
	       CheckFeature(requested, supported, descriptorBindingStorageBufferUpdateAfterBind) &&
	       CheckFeature(requested, supported, descriptorBindingUniformTexelBufferUpdateAfterBind) &&
	       CheckFeature(requested, supported, descriptorBindingStorageTexelBufferUpdateAfterBind) &&
	       CheckFeature(requested, supported, descriptorBindingUpdateUnusedWhilePending) &&
	       CheckFeature(requested, supported, descriptorBindingPartiallyBound) &&
	       CheckFeature(requested, supported, descriptorBindingVariableDescriptorCount) &&
	       CheckFeature(requested, supported, runtimeDescriptorArray) &&
	       CheckFeature(requested, supported, samplerFilterMinmax) &&
	       CheckFeature(requested, supported, scalarBlockLayout) &&
	       CheckFeature(requested, supported, imagelessFramebuffer) &&
	       CheckFeature(requested, supported, uniformBufferStandardLayout) &&
	       CheckFeature(requested, supported, shaderSubgroupExtendedTypes) &&
	       CheckFeature(requested, supported, separateDepthStencilLayouts) &&
	       CheckFeature(requested, supported, hostQueryReset) &&
	       CheckFeature(requested, supported, timelineSemaphore) &&
	       CheckFeature(requested, supported, bufferDeviceAddress) &&
	       CheckFeature(requested, supported, bufferDeviceAddressCaptureReplay) &&
	       CheckFeature(requested, supported, bufferDeviceAddressMultiDevice) &&
	       CheckFeature(requested, supported, vulkanMemoryModel) &&
	       CheckFeature(requested, supported, vulkanMemoryModelDeviceScope) &&
	       CheckFeature(requested, supported, vulkanMemoryModelAvailabilityVisibilityChains) &&
	       CheckFeature(requested, supported, shaderOutputViewportIndex) &&
	       CheckFeature(requested, supported, shaderOutputLayer) &&
	       CheckFeature(requested, supported, subgroupBroadcastDynamicId);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceVulkan13Features *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, robustImageAccess) &&
	       CheckFeature(requested, supported, inlineUniformBlock) &&
	       CheckFeature(requested, supported, descriptorBindingInlineUniformBlockUpdateAfterBind) &&
	       CheckFeature(requested, supported, pipelineCreationCacheControl) &&
	       CheckFeature(requested, supported, privateData) &&
	       CheckFeature(requested, supported, shaderDemoteToHelperInvocation) &&
	       CheckFeature(requested, supported, shaderTerminateInvocation) &&
	       CheckFeature(requested, supported, subgroupSizeControl) &&
	       CheckFeature(requested, supported, computeFullSubgroups) &&
	       CheckFeature(requested, supported, synchronization2) &&
	       CheckFeature(requested, supported, textureCompressionASTC_HDR) &&
	       CheckFeature(requested, supported, shaderZeroInitializeWorkgroupMemory) &&
	       CheckFeature(requested, supported, dynamicRendering) &&
	       CheckFeature(requested, supported, shaderIntegerDotProduct) &&
	       CheckFeature(requested, supported, maintenance4);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceDepthClipEnableFeaturesEXT *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, depthClipEnable);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, advancedBlendCoherentOperations);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceInlineUniformBlockFeatures *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, inlineUniformBlock) &&
	       CheckFeature(requested, supported, descriptorBindingInlineUniformBlockUpdateAfterBind);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceShaderIntegerDotProductFeatures *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, shaderIntegerDotProduct);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, extendedDynamicState);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, vertexInputDynamicState);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDevicePrivateDataFeatures *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, privateData);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceTextureCompressionASTCHDRFeatures *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, textureCompressionASTC_HDR);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, shaderDemoteToHelperInvocation);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceShaderTerminateInvocationFeatures *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, shaderTerminateInvocation);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceSubgroupSizeControlFeatures *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, subgroupSizeControl) &&
	       CheckFeature(requested, supported, computeFullSubgroups);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, shaderZeroInitializeWorkgroupMemory);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, primitiveTopologyListRestart) &&
	       CheckFeature(requested, supported, primitiveTopologyPatchListRestart);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, graphicsPipelineLibrary);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, swapchainMaintenance1);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceHostImageCopyFeaturesEXT *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, hostImageCopy);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceIndexTypeUint8FeaturesEXT *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, indexTypeUint8);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceDescriptorIndexingFeatures *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, shaderInputAttachmentArrayDynamicIndexing) &&
	       CheckFeature(requested, supported, shaderUniformTexelBufferArrayDynamicIndexing) &&
	       CheckFeature(requested, supported, shaderStorageTexelBufferArrayDynamicIndexing) &&
	       CheckFeature(requested, supported, shaderUniformBufferArrayNonUniformIndexing) &&
	       CheckFeature(requested, supported, shaderSampledImageArrayNonUniformIndexing) &&
	       CheckFeature(requested, supported, shaderStorageBufferArrayNonUniformIndexing) &&
	       CheckFeature(requested, supported, shaderStorageImageArrayNonUniformIndexing) &&
	       CheckFeature(requested, supported, shaderInputAttachmentArrayNonUniformIndexing) &&
	       CheckFeature(requested, supported, shaderUniformTexelBufferArrayNonUniformIndexing) &&
	       CheckFeature(requested, supported, shaderStorageTexelBufferArrayNonUniformIndexing) &&
	       CheckFeature(requested, supported, descriptorBindingUniformBufferUpdateAfterBind) &&
	       CheckFeature(requested, supported, descriptorBindingSampledImageUpdateAfterBind) &&
	       CheckFeature(requested, supported, descriptorBindingStorageImageUpdateAfterBind) &&
	       CheckFeature(requested, supported, descriptorBindingStorageBufferUpdateAfterBind) &&
	       CheckFeature(requested, supported, descriptorBindingUniformTexelBufferUpdateAfterBind) &&
	       CheckFeature(requested, supported, descriptorBindingStorageTexelBufferUpdateAfterBind) &&
	       CheckFeature(requested, supported, descriptorBindingUpdateUnusedWhilePending) &&
	       CheckFeature(requested, supported, descriptorBindingPartiallyBound) &&
	       CheckFeature(requested, supported, descriptorBindingVariableDescriptorCount) &&
	       CheckFeature(requested, supported, runtimeDescriptorArray);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDevicePipelineRobustnessFeaturesEXT *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, pipelineRobustness);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceProtectedMemoryFeatures *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, protectedMemory);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceBufferDeviceAddressFeatures *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, bufferDeviceAddress) &&
	       CheckFeature(requested, supported, bufferDeviceAddressCaptureReplay) &&
	       CheckFeature(requested, supported, bufferDeviceAddressMultiDevice);
}

bool PhysicalDevice::hasExtendedFeatures(const VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR *requested) const
{
	auto supported = getSupportedFeatures(requested);

	return CheckFeature(requested, supported, globalPriorityQuery);
}
#undef CheckFeature

static bool checkFormatUsage(VkImageUsageFlags usage, VkFormatFeatureFlags2KHR features)
{
	// Check for usage conflict with features
	if((usage & VK_IMAGE_USAGE_SAMPLED_BIT) && !(features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
	{
		return false;
	}

	if((usage & VK_IMAGE_USAGE_STORAGE_BIT) && !(features & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
	{
		return false;
	}

	if((usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) && !(features & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT))
	{
		return false;
	}

	if((usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) && !(features & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
	{
		return false;
	}

	if((usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) && !(features & (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)))
	{
		return false;
	}

	if((usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) && !(features & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT))
	{
		return false;
	}

	if((usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) && !(features & VK_FORMAT_FEATURE_TRANSFER_DST_BIT))
	{
		return false;
	}

	if((usage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT) && !(features & VK_FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT_EXT))
	{
		return false;
	}

	return true;
}

bool vk::PhysicalDevice::isFormatSupported(vk::Format format, VkImageType type, VkImageTiling tiling,
                                           VkImageUsageFlags usage, VkImageUsageFlags stencilUsage, VkImageCreateFlags flags)
{
	VkFormatProperties3 properties = {};
	vk::PhysicalDevice::GetFormatProperties(format, &properties);

	if(flags & VK_IMAGE_CREATE_EXTENDED_USAGE_BIT)
	{
		for(vk::Format f : format.getCompatibleFormats())
		{
			VkFormatProperties extendedProperties = {};
			vk::PhysicalDevice::GetFormatProperties(f, &extendedProperties);
			properties.linearTilingFeatures |= extendedProperties.linearTilingFeatures;
			properties.optimalTilingFeatures |= extendedProperties.optimalTilingFeatures;
			properties.bufferFeatures |= extendedProperties.bufferFeatures;
		}
	}

	VkFormatFeatureFlags2KHR features;
	switch(tiling)
	{
	case VK_IMAGE_TILING_LINEAR:
		features = properties.linearTilingFeatures;
		break;

	case VK_IMAGE_TILING_OPTIMAL:
		features = properties.optimalTilingFeatures;
		break;

	default:
		UNSUPPORTED("VkImageTiling %d", int(tiling));
		features = 0;
	}

	if(features == 0)
	{
		return false;
	}

	// Reject any usage or separate stencil usage that is not compatible with the specified format.
	if(!checkFormatUsage(usage, features))
	{
		return false;
	}
	// If stencilUsage is 0 then no separate usage was provided and it takes on the same value as usage,
	// which has already been checked. So only check non-zero stencilUsage.
	if(stencilUsage != 0 && !checkFormatUsage(stencilUsage, features))
	{
		return false;
	}

	auto allRecognizedUsageBits = VK_IMAGE_USAGE_SAMPLED_BIT |
	                              VK_IMAGE_USAGE_STORAGE_BIT |
	                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
	                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
	                              VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
	                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
	                              VK_IMAGE_USAGE_TRANSFER_DST_BIT |
	                              VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
	                              VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT;
	ASSERT(!(usage & ~(allRecognizedUsageBits)));

	if(usage & VK_IMAGE_USAGE_SAMPLED_BIT)
	{
		if(tiling == VK_IMAGE_TILING_LINEAR)
		{
			// TODO(b/171299814): Compressed formats and cube maps are not supported for sampling using VK_IMAGE_TILING_LINEAR; otherwise, sampling
			// in linear tiling is always supported as long as it can be sampled when using VK_IMAGE_TILING_OPTIMAL.
			if(!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) ||
			   vk::Format(format).isCompressed() ||
			   (flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT))
			{
				return false;
			}
		}
		else if(!(features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
		{
			return false;
		}
	}

	// "Images created with tiling equal to VK_IMAGE_TILING_LINEAR have further restrictions on their limits and capabilities
	//  compared to images created with tiling equal to VK_IMAGE_TILING_OPTIMAL."
	if(tiling == VK_IMAGE_TILING_LINEAR)
	{
		if(type != VK_IMAGE_TYPE_2D)
		{
			return false;
		}

		if(vk::Format(format).isDepth() || vk::Format(format).isStencil())
		{
			return false;
		}
	}

	// "Images created with a format from one of those listed in Formats requiring sampler Y'CBCR conversion for VK_IMAGE_ASPECT_COLOR_BIT image views
	//  have further restrictions on their limits and capabilities compared to images created with other formats."
	if(vk::Format(format).isYcbcrFormat())
	{
		if(type != VK_IMAGE_TYPE_2D)
		{
			return false;
		}
	}

	return true;
}

void PhysicalDevice::GetFormatProperties(Format format, VkFormatProperties *pFormatProperties)
{
	VkFormatProperties3 formatProperties3 = {};
	GetFormatProperties(format, &formatProperties3);

	// VkFormatFeatureFlags2KHR is a 64-bit extension of the 32-bit VkFormatFeatureFlags,
	// so when querying the legacy flags just return the lower 32-bit portion.
	pFormatProperties->linearTilingFeatures = static_cast<VkFormatFeatureFlags>(formatProperties3.linearTilingFeatures);
	pFormatProperties->optimalTilingFeatures = static_cast<VkFormatFeatureFlags>(formatProperties3.optimalTilingFeatures);
	pFormatProperties->bufferFeatures = static_cast<VkFormatFeatureFlags>(formatProperties3.bufferFeatures);
}

void PhysicalDevice::GetFormatProperties(Format format, VkFormatProperties3 *pFormatProperties)
{
	pFormatProperties->linearTilingFeatures = 0;   // Unsupported format
	pFormatProperties->optimalTilingFeatures = 0;  // Unsupported format
	pFormatProperties->bufferFeatures = 0;         // Unsupported format

	switch(format)
	{
	// Formats which can be sampled *and* filtered
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
	case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
	case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
	case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
	case VK_FORMAT_B5G6R5_UNORM_PACK16:
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
	case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
	case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_R8_SRGB:
	case VK_FORMAT_R8_SNORM:
	case VK_FORMAT_R8G8_UNORM:
	case VK_FORMAT_R8G8_SRGB:
	case VK_FORMAT_R8G8_SNORM:
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SNORM:
	case VK_FORMAT_R8G8B8A8_SRGB:
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_B8G8R8A8_SRGB:
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
	case VK_FORMAT_R16_UNORM:
	case VK_FORMAT_R16_SNORM:
	case VK_FORMAT_R16_SFLOAT:
	case VK_FORMAT_R16G16_UNORM:
	case VK_FORMAT_R16G16_SNORM:
	case VK_FORMAT_R16G16_SFLOAT:
	case VK_FORMAT_R16G16B16A16_UNORM:
	case VK_FORMAT_R16G16B16A16_SNORM:
	case VK_FORMAT_R16G16B16A16_SFLOAT:
	case VK_FORMAT_R32_SFLOAT:
	case VK_FORMAT_R32G32_SFLOAT:
	case VK_FORMAT_R32G32B32A32_SFLOAT:
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
	case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
	case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
	case VK_FORMAT_BC2_UNORM_BLOCK:
	case VK_FORMAT_BC2_SRGB_BLOCK:
	case VK_FORMAT_BC3_UNORM_BLOCK:
	case VK_FORMAT_BC3_SRGB_BLOCK:
	case VK_FORMAT_BC4_UNORM_BLOCK:
	case VK_FORMAT_BC4_SNORM_BLOCK:
	case VK_FORMAT_BC5_UNORM_BLOCK:
	case VK_FORMAT_BC5_SNORM_BLOCK:
	case VK_FORMAT_BC6H_UFLOAT_BLOCK:
	case VK_FORMAT_BC6H_SFLOAT_BLOCK:
	case VK_FORMAT_BC7_UNORM_BLOCK:
	case VK_FORMAT_BC7_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
	case VK_FORMAT_EAC_R11_UNORM_BLOCK:
	case VK_FORMAT_EAC_R11_SNORM_BLOCK:
	case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
	case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
#ifdef SWIFTSHADER_ENABLE_ASTC
	case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
	case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
	case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
	case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
	case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
	case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
	case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
	case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
	case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
	case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
	case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
	case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
#endif
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_D32_SFLOAT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		pFormatProperties->optimalTilingFeatures |=
		    VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
		// [[fallthrough]]

	// Formats which can be sampled, but don't support filtering
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
	case VK_FORMAT_R16_UINT:
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16B16A16_UINT:
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R32_UINT:
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32G32_UINT:
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32B32A32_UINT:
	case VK_FORMAT_R32G32B32A32_SINT:
	case VK_FORMAT_S8_UINT:
		pFormatProperties->optimalTilingFeatures |=
		    VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
		    VK_FORMAT_FEATURE_BLIT_SRC_BIT |
		    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
		    VK_FORMAT_FEATURE_TRANSFER_DST_BIT |
		    VK_FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT_EXT;
		break;

	// YCbCr formats:
	case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
	case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
	case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
		pFormatProperties->optimalTilingFeatures |=
		    VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
		    VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
		    VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT |
		    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
		    VK_FORMAT_FEATURE_TRANSFER_DST_BIT |
		    VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT |
		    VK_FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT_EXT;
		break;
	default:
		break;
	}

	switch(format)
	{
	// Vulkan 1.0 mandatory storage image formats supporting atomic operations
	case VK_FORMAT_R32_UINT:
	case VK_FORMAT_R32_SINT:
		pFormatProperties->optimalTilingFeatures |=
		    VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT;
		pFormatProperties->bufferFeatures |=
		    VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT;
		// [[fallthrough]]
	// Vulkan 1.0 mandatory storage image formats
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SNORM:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_R16G16B16A16_UINT:
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R16G16B16A16_SFLOAT:
	case VK_FORMAT_R32_SFLOAT:
	case VK_FORMAT_R32G32_UINT:
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32_SFLOAT:
	case VK_FORMAT_R32G32B32A32_UINT:
	case VK_FORMAT_R32G32B32A32_SINT:
	case VK_FORMAT_R32G32B32A32_SFLOAT:
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
	// Vulkan 1.0 shaderStorageImageExtendedFormats
	case VK_FORMAT_R16G16_SFLOAT:
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
	case VK_FORMAT_R16_SFLOAT:
	case VK_FORMAT_R16G16B16A16_UNORM:
	case VK_FORMAT_R16G16_UNORM:
	case VK_FORMAT_R8G8_UNORM:
	case VK_FORMAT_R16_UNORM:
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_R16G16B16A16_SNORM:
	case VK_FORMAT_R16G16_SNORM:
	case VK_FORMAT_R8G8_SNORM:
	case VK_FORMAT_R16_SNORM:
	case VK_FORMAT_R8_SNORM:
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R16_UINT:
	case VK_FORMAT_R8_UINT:
	// Additional formats not listed under "Formats without shader storage format"
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_B8G8R8A8_SRGB:
		pFormatProperties->optimalTilingFeatures |=
		    VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
		    VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT;
		pFormatProperties->bufferFeatures |=
		    VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT;
		break;
	default:
		break;
	}

	switch(format)
	{
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
	case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
	case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
	case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
	case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
	case VK_FORMAT_B5G6R5_UNORM_PACK16:
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
	case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_R8G8_UNORM:
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SRGB:
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_B8G8R8A8_SRGB:
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
	case VK_FORMAT_R16_SFLOAT:
	case VK_FORMAT_R16G16_SFLOAT:
	case VK_FORMAT_R16G16B16A16_SFLOAT:
	case VK_FORMAT_R32_SFLOAT:
	case VK_FORMAT_R32G32_SFLOAT:
	case VK_FORMAT_R32G32B32A32_SFLOAT:
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
	case VK_FORMAT_R16_UNORM:
	case VK_FORMAT_R16_UINT:
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16G16_UNORM:
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16B16A16_UNORM:
	case VK_FORMAT_R16G16B16A16_UINT:
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R32_UINT:
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32G32_UINT:
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32B32A32_UINT:
	case VK_FORMAT_R32G32B32A32_SINT:
		pFormatProperties->optimalTilingFeatures |=
		    VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
		    VK_FORMAT_FEATURE_BLIT_DST_BIT;
		break;
	case VK_FORMAT_S8_UINT:
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_D32_SFLOAT:          // Note: either VK_FORMAT_D32_SFLOAT or VK_FORMAT_X8_D24_UNORM_PACK32 must be supported
	case VK_FORMAT_D32_SFLOAT_S8_UINT:  // Note: either VK_FORMAT_D24_UNORM_S8_UINT or VK_FORMAT_D32_SFLOAT_S8_UINT must be supported
		pFormatProperties->optimalTilingFeatures |=
		    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
		break;
	default:
		break;
	}

	switch(format)
	{
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_D32_SFLOAT:          // Note: either VK_FORMAT_D32_SFLOAT or VK_FORMAT_X8_D24_UNORM_PACK32 must be supported
	case VK_FORMAT_D32_SFLOAT_S8_UINT:  // Note: either VK_FORMAT_D24_UNORM_S8_UINT or VK_FORMAT_D32_SFLOAT_S8_UINT must be supported
		pFormatProperties->optimalTilingFeatures |=
		    VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT_KHR;
		break;
	default:
		break;
	}

	if(format.supportsColorAttachmentBlend())
	{
		pFormatProperties->optimalTilingFeatures |=
		    VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
	}

	switch(format)
	{
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_R8_SNORM:
	case VK_FORMAT_R8_USCALED:
	case VK_FORMAT_R8_SSCALED:
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R8G8_UNORM:
	case VK_FORMAT_R8G8_SNORM:
	case VK_FORMAT_R8G8_USCALED:
	case VK_FORMAT_R8G8_SSCALED:
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SNORM:
	case VK_FORMAT_R8G8B8A8_USCALED:
	case VK_FORMAT_R8G8B8A8_SSCALED:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
	case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
	case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
	case VK_FORMAT_A2R10G10B10_SINT_PACK32:
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
	case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
	case VK_FORMAT_A2B10G10R10_SINT_PACK32:
	case VK_FORMAT_R16_UNORM:
	case VK_FORMAT_R16_SNORM:
	case VK_FORMAT_R16_USCALED:
	case VK_FORMAT_R16_SSCALED:
	case VK_FORMAT_R16_UINT:
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16_SFLOAT:
	case VK_FORMAT_R16G16_UNORM:
	case VK_FORMAT_R16G16_SNORM:
	case VK_FORMAT_R16G16_USCALED:
	case VK_FORMAT_R16G16_SSCALED:
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16_SFLOAT:
	case VK_FORMAT_R16G16B16A16_UNORM:
	case VK_FORMAT_R16G16B16A16_SNORM:
	case VK_FORMAT_R16G16B16A16_USCALED:
	case VK_FORMAT_R16G16B16A16_SSCALED:
	case VK_FORMAT_R16G16B16A16_UINT:
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R16G16B16A16_SFLOAT:
	case VK_FORMAT_R32_UINT:
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32_SFLOAT:
	case VK_FORMAT_R32G32_UINT:
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32_SFLOAT:
	case VK_FORMAT_R32G32B32_UINT:
	case VK_FORMAT_R32G32B32_SINT:
	case VK_FORMAT_R32G32B32_SFLOAT:
	case VK_FORMAT_R32G32B32A32_UINT:
	case VK_FORMAT_R32G32B32A32_SINT:
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		pFormatProperties->bufferFeatures |=
		    VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT;
		break;
	default:
		break;
	}

	switch(format)
	{
	// Vulkan 1.1 mandatory
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_R8_SNORM:
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R8G8_UNORM:
	case VK_FORMAT_R8G8_SNORM:
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SNORM:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
	case VK_FORMAT_R16_UINT:
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16_SFLOAT:
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16_SFLOAT:
	case VK_FORMAT_R16G16B16A16_UINT:
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R16G16B16A16_SFLOAT:
	case VK_FORMAT_R32_UINT:
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32_SFLOAT:
	case VK_FORMAT_R32G32_UINT:
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32_SFLOAT:
	case VK_FORMAT_R32G32B32A32_UINT:
	case VK_FORMAT_R32G32B32A32_SINT:
	case VK_FORMAT_R32G32B32A32_SFLOAT:
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
	// Optional
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
		pFormatProperties->bufferFeatures |=
		    VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT;
		break;
	default:
		break;
	}

	if(pFormatProperties->optimalTilingFeatures)
	{
		// "Formats that are required to support VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT must also support
		//  VK_FORMAT_FEATURE_TRANSFER_SRC_BIT and VK_FORMAT_FEATURE_TRANSFER_DST_BIT."
		//
		//  Additionally:
		//  "If VK_EXT_host_image_copy is supported and VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT is supported
		//  in optimalTilingFeatures or linearTilingFeatures for a color format,
		//  VK_FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT_EXT must also be supported in optimalTilingFeatures
		//  or linearTilingFeatures respectively."

		pFormatProperties->linearTilingFeatures |= VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
		                                           VK_FORMAT_FEATURE_TRANSFER_DST_BIT |
		                                           VK_FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT_EXT;

		if(!format.isCompressed())
		{
			VkFormatFeatureFlagBits2KHR transferableFeatureBits = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
			                                                      VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
			                                                      VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT_KHR;

			pFormatProperties->linearTilingFeatures |= (pFormatProperties->optimalTilingFeatures & transferableFeatureBits);
		}
	}
}

void PhysicalDevice::getImageFormatProperties(Format format, VkImageType type, VkImageTiling tiling,
                                              VkImageUsageFlags usage, VkImageCreateFlags flags,
                                              VkImageFormatProperties *pImageFormatProperties) const
{
	pImageFormatProperties->sampleCounts = VK_SAMPLE_COUNT_1_BIT;
	pImageFormatProperties->maxArrayLayers = vk::MAX_IMAGE_ARRAY_LAYERS;
	pImageFormatProperties->maxExtent.depth = 1;

	switch(type)
	{
	case VK_IMAGE_TYPE_1D:
		pImageFormatProperties->maxMipLevels = vk::MAX_IMAGE_LEVELS_1D;
		pImageFormatProperties->maxExtent.width = 1 << (vk::MAX_IMAGE_LEVELS_1D - 1);
		pImageFormatProperties->maxExtent.height = 1;
		break;
	case VK_IMAGE_TYPE_2D:
		if(flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
		{
			pImageFormatProperties->maxMipLevels = vk::MAX_IMAGE_LEVELS_CUBE;
			pImageFormatProperties->maxExtent.width = 1 << (vk::MAX_IMAGE_LEVELS_CUBE - 1);
			pImageFormatProperties->maxExtent.height = 1 << (vk::MAX_IMAGE_LEVELS_CUBE - 1);
		}
		else
		{
			pImageFormatProperties->maxMipLevels = vk::MAX_IMAGE_LEVELS_2D;
			pImageFormatProperties->maxExtent.width = 1 << (vk::MAX_IMAGE_LEVELS_2D - 1);
			pImageFormatProperties->maxExtent.height = 1 << (vk::MAX_IMAGE_LEVELS_2D - 1);

			VkFormatProperties props;
			GetFormatProperties(format, &props);
			auto features = tiling == VK_IMAGE_TILING_LINEAR ? props.linearTilingFeatures : props.optimalTilingFeatures;
			if(features & (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
			{
				// Only renderable formats make sense for multisample
				pImageFormatProperties->sampleCounts = getSampleCounts();
			}
		}
		break;
	case VK_IMAGE_TYPE_3D:
		pImageFormatProperties->maxMipLevels = vk::MAX_IMAGE_LEVELS_3D;
		pImageFormatProperties->maxExtent.width = 1 << (vk::MAX_IMAGE_LEVELS_3D - 1);
		pImageFormatProperties->maxExtent.height = 1 << (vk::MAX_IMAGE_LEVELS_3D - 1);
		pImageFormatProperties->maxExtent.depth = 1 << (vk::MAX_IMAGE_LEVELS_3D - 1);
		pImageFormatProperties->maxArrayLayers = 1;  // no 3D + layers
		break;
	default:
		UNREACHABLE("VkImageType: %d", int(type));
		break;
	}

	pImageFormatProperties->maxResourceSize = 1u << 31;  // Minimum value for maxResourceSize

	// "Images created with tiling equal to VK_IMAGE_TILING_LINEAR have further restrictions on their limits and capabilities
	//  compared to images created with tiling equal to VK_IMAGE_TILING_OPTIMAL."
	if(tiling == VK_IMAGE_TILING_LINEAR)
	{
		pImageFormatProperties->maxMipLevels = 1;
		pImageFormatProperties->maxArrayLayers = 1;
		pImageFormatProperties->sampleCounts = VK_SAMPLE_COUNT_1_BIT;
	}

	// "Images created with a format from one of those listed in Formats requiring sampler Y'CbCr conversion for VK_IMAGE_ASPECT_COLOR_BIT image views
	//  have further restrictions on their limits and capabilities compared to images created with other formats."
	if(format.isYcbcrFormat())
	{
		pImageFormatProperties->maxMipLevels = 1;  // TODO(b/151263485): This is relied on by the sampler to disable mipmapping for Y'CbCr image sampling.
		pImageFormatProperties->maxArrayLayers = 1;
		pImageFormatProperties->sampleCounts = VK_SAMPLE_COUNT_1_BIT;
	}
}

uint32_t PhysicalDevice::getQueueFamilyPropertyCount() const
{
	return 1;
}

VkQueueFamilyProperties PhysicalDevice::getQueueFamilyProperties() const
{
	VkQueueFamilyProperties properties = {};
	properties.minImageTransferGranularity.width = 1;
	properties.minImageTransferGranularity.height = 1;
	properties.minImageTransferGranularity.depth = 1;
	properties.queueCount = 1;
	properties.queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
	properties.timestampValidBits = 64;

	return properties;
}

void PhysicalDevice::getQueueFamilyProperties(uint32_t pQueueFamilyPropertyCount,
                                              VkQueueFamilyProperties *pQueueFamilyProperties) const
{
	for(uint32_t i = 0; i < pQueueFamilyPropertyCount; i++)
	{
		pQueueFamilyProperties[i] = getQueueFamilyProperties();
	}
}

void PhysicalDevice::getQueueFamilyGlobalPriorityProperties(VkQueueFamilyGlobalPriorityPropertiesKHR *pQueueFamilyGlobalPriorityProperties) const
{
	pQueueFamilyGlobalPriorityProperties->priorityCount = 1;
	pQueueFamilyGlobalPriorityProperties->priorities[0] = VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR;
}

bool PhysicalDevice::validateQueueGlobalPriority(VkQueueGlobalPriorityKHR queueGlobalPriority) const
{
	VkQueueFamilyGlobalPriorityPropertiesKHR queueFamilyGlobalPriorityProperties;
	getQueueFamilyGlobalPriorityProperties(&queueFamilyGlobalPriorityProperties);

	for(uint32_t i = 0; i < queueFamilyGlobalPriorityProperties.priorityCount; ++i)
	{
		if(queueGlobalPriority == queueFamilyGlobalPriorityProperties.priorities[i])
		{
			return true;
		}
	}

	return false;
}

void PhysicalDevice::getQueueFamilyProperties(uint32_t pQueueFamilyPropertyCount,
                                              VkQueueFamilyProperties2 *pQueueFamilyProperties) const
{
	for(uint32_t i = 0; i < pQueueFamilyPropertyCount; i++)
	{
		pQueueFamilyProperties[i].queueFamilyProperties = getQueueFamilyProperties();

		VkBaseOutStructure *extInfo = reinterpret_cast<VkBaseOutStructure *>(pQueueFamilyProperties[i].pNext);
		while(extInfo)
		{
			switch(extInfo->sType)
			{
			case VK_STRUCTURE_TYPE_QUEUE_FAMILY_GLOBAL_PRIORITY_PROPERTIES_KHR:
				getQueueFamilyGlobalPriorityProperties(reinterpret_cast<VkQueueFamilyGlobalPriorityPropertiesKHR *>(extInfo));
				break;
			default:
				UNSUPPORTED("pQueueFamilyProperties->pNext sType = %s", vk::Stringify(extInfo->sType).c_str());
				break;
			}

			extInfo = extInfo->pNext;
		}
	}
}

const VkPhysicalDeviceMemoryProperties &PhysicalDevice::GetMemoryProperties()
{
	static const VkPhysicalDeviceMemoryProperties properties{
		1,  // memoryTypeCount
		{
		    // vk::MEMORY_TYPE_GENERIC_BIT
		    {
		        (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
		         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
		         VK_MEMORY_PROPERTY_HOST_CACHED_BIT),  // propertyFlags
		        0                                      // heapIndex
		    },
		},
		1,  // memoryHeapCount
		{
		    {
		        vk::PHYSICAL_DEVICE_HEAP_SIZE,   // size
		        VK_MEMORY_HEAP_DEVICE_LOCAL_BIT  // flags
		    },
		}
	};

	return properties;
}

}  // namespace vk
