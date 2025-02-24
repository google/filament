// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See device_features_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2023-2025 Google Inc.
 * Copyright (c) 2023-2025 LunarG, Inc.
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
 ****************************************************************************/

// NOLINTBEGIN

#include "generated/device_features.h"
#include "generated/vk_api_version.h"
#include "generated/vk_extension_helper.h"
#include <vulkan/utility/vk_struct_helper.hpp>

void GetEnabledDeviceFeatures(const VkDeviceCreateInfo *pCreateInfo, DeviceFeatures *features, const APIVersion &api_version) {
    // Initialize all to false
    *features = {};

    // handle VkPhysicalDeviceFeatures specially as it is not part of the pNext chain,
    // and when it is (through VkPhysicalDeviceFeatures2), it requires an extra indirection.
    const VkPhysicalDeviceFeatures *core_features = pCreateInfo->pEnabledFeatures;
    if (core_features == nullptr) {
        const VkPhysicalDeviceFeatures2 *features2 = vku::FindStructInPNextChain<VkPhysicalDeviceFeatures2>(pCreateInfo->pNext);
        if (features2 != nullptr) {
            core_features = &features2->features;
        }
    }
    if (core_features != nullptr) {
        features->robustBufferAccess = core_features->robustBufferAccess == VK_TRUE;
        features->fullDrawIndexUint32 = core_features->fullDrawIndexUint32 == VK_TRUE;
        features->imageCubeArray = core_features->imageCubeArray == VK_TRUE;
        features->independentBlend = core_features->independentBlend == VK_TRUE;
        features->geometryShader = core_features->geometryShader == VK_TRUE;
        features->tessellationShader = core_features->tessellationShader == VK_TRUE;
        features->sampleRateShading = core_features->sampleRateShading == VK_TRUE;
        features->dualSrcBlend = core_features->dualSrcBlend == VK_TRUE;
        features->logicOp = core_features->logicOp == VK_TRUE;
        features->multiDrawIndirect = core_features->multiDrawIndirect == VK_TRUE;
        features->drawIndirectFirstInstance = core_features->drawIndirectFirstInstance == VK_TRUE;
        features->depthClamp = core_features->depthClamp == VK_TRUE;
        features->depthBiasClamp = core_features->depthBiasClamp == VK_TRUE;
        features->fillModeNonSolid = core_features->fillModeNonSolid == VK_TRUE;
        features->depthBounds = core_features->depthBounds == VK_TRUE;
        features->wideLines = core_features->wideLines == VK_TRUE;
        features->largePoints = core_features->largePoints == VK_TRUE;
        features->alphaToOne = core_features->alphaToOne == VK_TRUE;
        features->multiViewport = core_features->multiViewport == VK_TRUE;
        features->samplerAnisotropy = core_features->samplerAnisotropy == VK_TRUE;
        features->textureCompressionETC2 = core_features->textureCompressionETC2 == VK_TRUE;
        features->textureCompressionASTC_LDR = core_features->textureCompressionASTC_LDR == VK_TRUE;
        features->textureCompressionBC = core_features->textureCompressionBC == VK_TRUE;
        features->occlusionQueryPrecise = core_features->occlusionQueryPrecise == VK_TRUE;
        features->pipelineStatisticsQuery = core_features->pipelineStatisticsQuery == VK_TRUE;
        features->vertexPipelineStoresAndAtomics = core_features->vertexPipelineStoresAndAtomics == VK_TRUE;
        features->fragmentStoresAndAtomics = core_features->fragmentStoresAndAtomics == VK_TRUE;
        features->shaderTessellationAndGeometryPointSize = core_features->shaderTessellationAndGeometryPointSize == VK_TRUE;
        features->shaderImageGatherExtended = core_features->shaderImageGatherExtended == VK_TRUE;
        features->shaderStorageImageExtendedFormats = core_features->shaderStorageImageExtendedFormats == VK_TRUE;
        features->shaderStorageImageMultisample = core_features->shaderStorageImageMultisample == VK_TRUE;
        features->shaderStorageImageReadWithoutFormat = core_features->shaderStorageImageReadWithoutFormat == VK_TRUE;
        features->shaderStorageImageWriteWithoutFormat = core_features->shaderStorageImageWriteWithoutFormat == VK_TRUE;
        features->shaderUniformBufferArrayDynamicIndexing = core_features->shaderUniformBufferArrayDynamicIndexing == VK_TRUE;
        features->shaderSampledImageArrayDynamicIndexing = core_features->shaderSampledImageArrayDynamicIndexing == VK_TRUE;
        features->shaderStorageBufferArrayDynamicIndexing = core_features->shaderStorageBufferArrayDynamicIndexing == VK_TRUE;
        features->shaderStorageImageArrayDynamicIndexing = core_features->shaderStorageImageArrayDynamicIndexing == VK_TRUE;
        features->shaderClipDistance = core_features->shaderClipDistance == VK_TRUE;
        features->shaderCullDistance = core_features->shaderCullDistance == VK_TRUE;
        features->shaderFloat64 = core_features->shaderFloat64 == VK_TRUE;
        features->shaderInt64 = core_features->shaderInt64 == VK_TRUE;
        features->shaderInt16 = core_features->shaderInt16 == VK_TRUE;
        features->shaderResourceResidency = core_features->shaderResourceResidency == VK_TRUE;
        features->shaderResourceMinLod = core_features->shaderResourceMinLod == VK_TRUE;
        features->sparseBinding = core_features->sparseBinding == VK_TRUE;
        features->sparseResidencyBuffer = core_features->sparseResidencyBuffer == VK_TRUE;
        features->sparseResidencyImage2D = core_features->sparseResidencyImage2D == VK_TRUE;
        features->sparseResidencyImage3D = core_features->sparseResidencyImage3D == VK_TRUE;
        features->sparseResidency2Samples = core_features->sparseResidency2Samples == VK_TRUE;
        features->sparseResidency4Samples = core_features->sparseResidency4Samples == VK_TRUE;
        features->sparseResidency8Samples = core_features->sparseResidency8Samples == VK_TRUE;
        features->sparseResidency16Samples = core_features->sparseResidency16Samples == VK_TRUE;
        features->sparseResidencyAliased = core_features->sparseResidencyAliased == VK_TRUE;
        features->variableMultisampleRate = core_features->variableMultisampleRate == VK_TRUE;
        features->inheritedQueries = core_features->inheritedQueries == VK_TRUE;
    }

    for (const VkBaseInStructure *pNext = reinterpret_cast<const VkBaseInStructure *>(pCreateInfo->pNext); pNext != nullptr;
         pNext = pNext->pNext) {
        switch (pNext->sType) {
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES: {
                const VkPhysicalDevice16BitStorageFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDevice16BitStorageFeatures *>(pNext);
                features->storageBuffer16BitAccess |= enabled->storageBuffer16BitAccess == VK_TRUE;
                features->uniformAndStorageBuffer16BitAccess |= enabled->uniformAndStorageBuffer16BitAccess == VK_TRUE;
                features->storagePushConstant16 |= enabled->storagePushConstant16 == VK_TRUE;
                features->storageInputOutput16 |= enabled->storageInputOutput16 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES: {
                const VkPhysicalDeviceMultiviewFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceMultiviewFeatures *>(pNext);
                features->multiview |= enabled->multiview == VK_TRUE;
                features->multiviewGeometryShader |= enabled->multiviewGeometryShader == VK_TRUE;
                features->multiviewTessellationShader |= enabled->multiviewTessellationShader == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES: {
                const VkPhysicalDeviceVariablePointersFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceVariablePointersFeatures *>(pNext);
                features->variablePointersStorageBuffer |= enabled->variablePointersStorageBuffer == VK_TRUE;
                features->variablePointers |= enabled->variablePointers == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES: {
                const VkPhysicalDeviceProtectedMemoryFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceProtectedMemoryFeatures *>(pNext);
                features->protectedMemory |= enabled->protectedMemory == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES: {
                const VkPhysicalDeviceSamplerYcbcrConversionFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceSamplerYcbcrConversionFeatures *>(pNext);
                features->samplerYcbcrConversion |= enabled->samplerYcbcrConversion == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES: {
                const VkPhysicalDeviceShaderDrawParametersFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderDrawParametersFeatures *>(pNext);
                features->shaderDrawParameters |= enabled->shaderDrawParameters == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES: {
                const VkPhysicalDeviceVulkan11Features *enabled = reinterpret_cast<const VkPhysicalDeviceVulkan11Features *>(pNext);
                features->storageBuffer16BitAccess |= enabled->storageBuffer16BitAccess == VK_TRUE;
                features->uniformAndStorageBuffer16BitAccess |= enabled->uniformAndStorageBuffer16BitAccess == VK_TRUE;
                features->storagePushConstant16 |= enabled->storagePushConstant16 == VK_TRUE;
                features->storageInputOutput16 |= enabled->storageInputOutput16 == VK_TRUE;
                features->multiview |= enabled->multiview == VK_TRUE;
                features->multiviewGeometryShader |= enabled->multiviewGeometryShader == VK_TRUE;
                features->multiviewTessellationShader |= enabled->multiviewTessellationShader == VK_TRUE;
                features->variablePointersStorageBuffer |= enabled->variablePointersStorageBuffer == VK_TRUE;
                features->variablePointers |= enabled->variablePointers == VK_TRUE;
                features->protectedMemory |= enabled->protectedMemory == VK_TRUE;
                features->samplerYcbcrConversion |= enabled->samplerYcbcrConversion == VK_TRUE;
                features->shaderDrawParameters |= enabled->shaderDrawParameters == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES: {
                const VkPhysicalDeviceVulkan12Features *enabled = reinterpret_cast<const VkPhysicalDeviceVulkan12Features *>(pNext);
                features->samplerMirrorClampToEdge |= enabled->samplerMirrorClampToEdge == VK_TRUE;
                features->drawIndirectCount |= enabled->drawIndirectCount == VK_TRUE;
                features->storageBuffer8BitAccess |= enabled->storageBuffer8BitAccess == VK_TRUE;
                features->uniformAndStorageBuffer8BitAccess |= enabled->uniformAndStorageBuffer8BitAccess == VK_TRUE;
                features->storagePushConstant8 |= enabled->storagePushConstant8 == VK_TRUE;
                features->shaderBufferInt64Atomics |= enabled->shaderBufferInt64Atomics == VK_TRUE;
                features->shaderSharedInt64Atomics |= enabled->shaderSharedInt64Atomics == VK_TRUE;
                features->shaderFloat16 |= enabled->shaderFloat16 == VK_TRUE;
                features->shaderInt8 |= enabled->shaderInt8 == VK_TRUE;
                features->descriptorIndexing |= enabled->descriptorIndexing == VK_TRUE;
                features->shaderInputAttachmentArrayDynamicIndexing |=
                    enabled->shaderInputAttachmentArrayDynamicIndexing == VK_TRUE;
                features->shaderUniformTexelBufferArrayDynamicIndexing |=
                    enabled->shaderUniformTexelBufferArrayDynamicIndexing == VK_TRUE;
                features->shaderStorageTexelBufferArrayDynamicIndexing |=
                    enabled->shaderStorageTexelBufferArrayDynamicIndexing == VK_TRUE;
                features->shaderUniformBufferArrayNonUniformIndexing |=
                    enabled->shaderUniformBufferArrayNonUniformIndexing == VK_TRUE;
                features->shaderSampledImageArrayNonUniformIndexing |=
                    enabled->shaderSampledImageArrayNonUniformIndexing == VK_TRUE;
                features->shaderStorageBufferArrayNonUniformIndexing |=
                    enabled->shaderStorageBufferArrayNonUniformIndexing == VK_TRUE;
                features->shaderStorageImageArrayNonUniformIndexing |=
                    enabled->shaderStorageImageArrayNonUniformIndexing == VK_TRUE;
                features->shaderInputAttachmentArrayNonUniformIndexing |=
                    enabled->shaderInputAttachmentArrayNonUniformIndexing == VK_TRUE;
                features->shaderUniformTexelBufferArrayNonUniformIndexing |=
                    enabled->shaderUniformTexelBufferArrayNonUniformIndexing == VK_TRUE;
                features->shaderStorageTexelBufferArrayNonUniformIndexing |=
                    enabled->shaderStorageTexelBufferArrayNonUniformIndexing == VK_TRUE;
                features->descriptorBindingUniformBufferUpdateAfterBind |=
                    enabled->descriptorBindingUniformBufferUpdateAfterBind == VK_TRUE;
                features->descriptorBindingSampledImageUpdateAfterBind |=
                    enabled->descriptorBindingSampledImageUpdateAfterBind == VK_TRUE;
                features->descriptorBindingStorageImageUpdateAfterBind |=
                    enabled->descriptorBindingStorageImageUpdateAfterBind == VK_TRUE;
                features->descriptorBindingStorageBufferUpdateAfterBind |=
                    enabled->descriptorBindingStorageBufferUpdateAfterBind == VK_TRUE;
                features->descriptorBindingUniformTexelBufferUpdateAfterBind |=
                    enabled->descriptorBindingUniformTexelBufferUpdateAfterBind == VK_TRUE;
                features->descriptorBindingStorageTexelBufferUpdateAfterBind |=
                    enabled->descriptorBindingStorageTexelBufferUpdateAfterBind == VK_TRUE;
                features->descriptorBindingUpdateUnusedWhilePending |=
                    enabled->descriptorBindingUpdateUnusedWhilePending == VK_TRUE;
                features->descriptorBindingPartiallyBound |= enabled->descriptorBindingPartiallyBound == VK_TRUE;
                features->descriptorBindingVariableDescriptorCount |= enabled->descriptorBindingVariableDescriptorCount == VK_TRUE;
                features->runtimeDescriptorArray |= enabled->runtimeDescriptorArray == VK_TRUE;
                features->samplerFilterMinmax |= enabled->samplerFilterMinmax == VK_TRUE;
                features->scalarBlockLayout |= enabled->scalarBlockLayout == VK_TRUE;
                features->imagelessFramebuffer |= enabled->imagelessFramebuffer == VK_TRUE;
                features->uniformBufferStandardLayout |= enabled->uniformBufferStandardLayout == VK_TRUE;
                features->shaderSubgroupExtendedTypes |= enabled->shaderSubgroupExtendedTypes == VK_TRUE;
                features->separateDepthStencilLayouts |= enabled->separateDepthStencilLayouts == VK_TRUE;
                features->hostQueryReset |= enabled->hostQueryReset == VK_TRUE;
                features->timelineSemaphore |= enabled->timelineSemaphore == VK_TRUE;
                features->bufferDeviceAddress |= enabled->bufferDeviceAddress == VK_TRUE;
                features->bufferDeviceAddressCaptureReplay |= enabled->bufferDeviceAddressCaptureReplay == VK_TRUE;
                features->bufferDeviceAddressMultiDevice |= enabled->bufferDeviceAddressMultiDevice == VK_TRUE;
                features->vulkanMemoryModel |= enabled->vulkanMemoryModel == VK_TRUE;
                features->vulkanMemoryModelDeviceScope |= enabled->vulkanMemoryModelDeviceScope == VK_TRUE;
                features->vulkanMemoryModelAvailabilityVisibilityChains |=
                    enabled->vulkanMemoryModelAvailabilityVisibilityChains == VK_TRUE;
                features->shaderOutputViewportIndex |= enabled->shaderOutputViewportIndex == VK_TRUE;
                features->shaderOutputLayer |= enabled->shaderOutputLayer == VK_TRUE;
                features->subgroupBroadcastDynamicId |= enabled->subgroupBroadcastDynamicId == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES: {
                const VkPhysicalDevice8BitStorageFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDevice8BitStorageFeatures *>(pNext);
                features->storageBuffer8BitAccess |= enabled->storageBuffer8BitAccess == VK_TRUE;
                features->uniformAndStorageBuffer8BitAccess |= enabled->uniformAndStorageBuffer8BitAccess == VK_TRUE;
                features->storagePushConstant8 |= enabled->storagePushConstant8 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES: {
                const VkPhysicalDeviceShaderAtomicInt64Features *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderAtomicInt64Features *>(pNext);
                features->shaderBufferInt64Atomics |= enabled->shaderBufferInt64Atomics == VK_TRUE;
                features->shaderSharedInt64Atomics |= enabled->shaderSharedInt64Atomics == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES: {
                const VkPhysicalDeviceShaderFloat16Int8Features *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderFloat16Int8Features *>(pNext);
                features->shaderFloat16 |= enabled->shaderFloat16 == VK_TRUE;
                features->shaderInt8 |= enabled->shaderInt8 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES: {
                const VkPhysicalDeviceDescriptorIndexingFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDescriptorIndexingFeatures *>(pNext);
                features->shaderInputAttachmentArrayDynamicIndexing |=
                    enabled->shaderInputAttachmentArrayDynamicIndexing == VK_TRUE;
                features->shaderUniformTexelBufferArrayDynamicIndexing |=
                    enabled->shaderUniformTexelBufferArrayDynamicIndexing == VK_TRUE;
                features->shaderStorageTexelBufferArrayDynamicIndexing |=
                    enabled->shaderStorageTexelBufferArrayDynamicIndexing == VK_TRUE;
                features->shaderUniformBufferArrayNonUniformIndexing |=
                    enabled->shaderUniformBufferArrayNonUniformIndexing == VK_TRUE;
                features->shaderSampledImageArrayNonUniformIndexing |=
                    enabled->shaderSampledImageArrayNonUniformIndexing == VK_TRUE;
                features->shaderStorageBufferArrayNonUniformIndexing |=
                    enabled->shaderStorageBufferArrayNonUniformIndexing == VK_TRUE;
                features->shaderStorageImageArrayNonUniformIndexing |=
                    enabled->shaderStorageImageArrayNonUniformIndexing == VK_TRUE;
                features->shaderInputAttachmentArrayNonUniformIndexing |=
                    enabled->shaderInputAttachmentArrayNonUniformIndexing == VK_TRUE;
                features->shaderUniformTexelBufferArrayNonUniformIndexing |=
                    enabled->shaderUniformTexelBufferArrayNonUniformIndexing == VK_TRUE;
                features->shaderStorageTexelBufferArrayNonUniformIndexing |=
                    enabled->shaderStorageTexelBufferArrayNonUniformIndexing == VK_TRUE;
                features->descriptorBindingUniformBufferUpdateAfterBind |=
                    enabled->descriptorBindingUniformBufferUpdateAfterBind == VK_TRUE;
                features->descriptorBindingSampledImageUpdateAfterBind |=
                    enabled->descriptorBindingSampledImageUpdateAfterBind == VK_TRUE;
                features->descriptorBindingStorageImageUpdateAfterBind |=
                    enabled->descriptorBindingStorageImageUpdateAfterBind == VK_TRUE;
                features->descriptorBindingStorageBufferUpdateAfterBind |=
                    enabled->descriptorBindingStorageBufferUpdateAfterBind == VK_TRUE;
                features->descriptorBindingUniformTexelBufferUpdateAfterBind |=
                    enabled->descriptorBindingUniformTexelBufferUpdateAfterBind == VK_TRUE;
                features->descriptorBindingStorageTexelBufferUpdateAfterBind |=
                    enabled->descriptorBindingStorageTexelBufferUpdateAfterBind == VK_TRUE;
                features->descriptorBindingUpdateUnusedWhilePending |=
                    enabled->descriptorBindingUpdateUnusedWhilePending == VK_TRUE;
                features->descriptorBindingPartiallyBound |= enabled->descriptorBindingPartiallyBound == VK_TRUE;
                features->descriptorBindingVariableDescriptorCount |= enabled->descriptorBindingVariableDescriptorCount == VK_TRUE;
                features->runtimeDescriptorArray |= enabled->runtimeDescriptorArray == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES: {
                const VkPhysicalDeviceScalarBlockLayoutFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceScalarBlockLayoutFeatures *>(pNext);
                features->scalarBlockLayout |= enabled->scalarBlockLayout == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES: {
                const VkPhysicalDeviceVulkanMemoryModelFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceVulkanMemoryModelFeatures *>(pNext);
                features->vulkanMemoryModel |= enabled->vulkanMemoryModel == VK_TRUE;
                features->vulkanMemoryModelDeviceScope |= enabled->vulkanMemoryModelDeviceScope == VK_TRUE;
                features->vulkanMemoryModelAvailabilityVisibilityChains |=
                    enabled->vulkanMemoryModelAvailabilityVisibilityChains == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES: {
                const VkPhysicalDeviceImagelessFramebufferFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceImagelessFramebufferFeatures *>(pNext);
                features->imagelessFramebuffer |= enabled->imagelessFramebuffer == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES: {
                const VkPhysicalDeviceUniformBufferStandardLayoutFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceUniformBufferStandardLayoutFeatures *>(pNext);
                features->uniformBufferStandardLayout |= enabled->uniformBufferStandardLayout == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES: {
                const VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures *>(pNext);
                features->shaderSubgroupExtendedTypes |= enabled->shaderSubgroupExtendedTypes == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES: {
                const VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures *>(pNext);
                features->separateDepthStencilLayouts |= enabled->separateDepthStencilLayouts == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES: {
                const VkPhysicalDeviceHostQueryResetFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceHostQueryResetFeatures *>(pNext);
                features->hostQueryReset |= enabled->hostQueryReset == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES: {
                const VkPhysicalDeviceTimelineSemaphoreFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceTimelineSemaphoreFeatures *>(pNext);
                features->timelineSemaphore |= enabled->timelineSemaphore == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES: {
                const VkPhysicalDeviceBufferDeviceAddressFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeatures *>(pNext);
                features->bufferDeviceAddress |= enabled->bufferDeviceAddress == VK_TRUE;
                features->bufferDeviceAddressCaptureReplay |= enabled->bufferDeviceAddressCaptureReplay == VK_TRUE;
                features->bufferDeviceAddressMultiDevice |= enabled->bufferDeviceAddressMultiDevice == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES: {
                const VkPhysicalDeviceVulkan13Features *enabled = reinterpret_cast<const VkPhysicalDeviceVulkan13Features *>(pNext);
                features->robustImageAccess |= enabled->robustImageAccess == VK_TRUE;
                features->inlineUniformBlock |= enabled->inlineUniformBlock == VK_TRUE;
                features->descriptorBindingInlineUniformBlockUpdateAfterBind |=
                    enabled->descriptorBindingInlineUniformBlockUpdateAfterBind == VK_TRUE;
                features->pipelineCreationCacheControl |= enabled->pipelineCreationCacheControl == VK_TRUE;
                features->privateData |= enabled->privateData == VK_TRUE;
                features->shaderDemoteToHelperInvocation |= enabled->shaderDemoteToHelperInvocation == VK_TRUE;
                features->shaderTerminateInvocation |= enabled->shaderTerminateInvocation == VK_TRUE;
                features->subgroupSizeControl |= enabled->subgroupSizeControl == VK_TRUE;
                features->computeFullSubgroups |= enabled->computeFullSubgroups == VK_TRUE;
                features->synchronization2 |= enabled->synchronization2 == VK_TRUE;
                features->textureCompressionASTC_HDR |= enabled->textureCompressionASTC_HDR == VK_TRUE;
                features->shaderZeroInitializeWorkgroupMemory |= enabled->shaderZeroInitializeWorkgroupMemory == VK_TRUE;
                features->dynamicRendering |= enabled->dynamicRendering == VK_TRUE;
                features->shaderIntegerDotProduct |= enabled->shaderIntegerDotProduct == VK_TRUE;
                features->maintenance4 |= enabled->maintenance4 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES: {
                const VkPhysicalDeviceShaderTerminateInvocationFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderTerminateInvocationFeatures *>(pNext);
                features->shaderTerminateInvocation |= enabled->shaderTerminateInvocation == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES: {
                const VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures *>(pNext);
                features->shaderDemoteToHelperInvocation |= enabled->shaderDemoteToHelperInvocation == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES: {
                const VkPhysicalDevicePrivateDataFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDevicePrivateDataFeatures *>(pNext);
                features->privateData |= enabled->privateData == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES: {
                const VkPhysicalDevicePipelineCreationCacheControlFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDevicePipelineCreationCacheControlFeatures *>(pNext);
                features->pipelineCreationCacheControl |= enabled->pipelineCreationCacheControl == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES: {
                const VkPhysicalDeviceSynchronization2Features *enabled =
                    reinterpret_cast<const VkPhysicalDeviceSynchronization2Features *>(pNext);
                features->synchronization2 |= enabled->synchronization2 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES: {
                const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures *>(pNext);
                features->shaderZeroInitializeWorkgroupMemory |= enabled->shaderZeroInitializeWorkgroupMemory == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES: {
                const VkPhysicalDeviceImageRobustnessFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceImageRobustnessFeatures *>(pNext);
                features->robustImageAccess |= enabled->robustImageAccess == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES: {
                const VkPhysicalDeviceSubgroupSizeControlFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceSubgroupSizeControlFeatures *>(pNext);
                features->subgroupSizeControl |= enabled->subgroupSizeControl == VK_TRUE;
                features->computeFullSubgroups |= enabled->computeFullSubgroups == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES: {
                const VkPhysicalDeviceInlineUniformBlockFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceInlineUniformBlockFeatures *>(pNext);
                features->inlineUniformBlock |= enabled->inlineUniformBlock == VK_TRUE;
                features->descriptorBindingInlineUniformBlockUpdateAfterBind |=
                    enabled->descriptorBindingInlineUniformBlockUpdateAfterBind == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES: {
                const VkPhysicalDeviceTextureCompressionASTCHDRFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceTextureCompressionASTCHDRFeatures *>(pNext);
                features->textureCompressionASTC_HDR |= enabled->textureCompressionASTC_HDR == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES: {
                const VkPhysicalDeviceDynamicRenderingFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDynamicRenderingFeatures *>(pNext);
                features->dynamicRendering |= enabled->dynamicRendering == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES: {
                const VkPhysicalDeviceShaderIntegerDotProductFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderIntegerDotProductFeatures *>(pNext);
                features->shaderIntegerDotProduct |= enabled->shaderIntegerDotProduct == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES: {
                const VkPhysicalDeviceMaintenance4Features *enabled =
                    reinterpret_cast<const VkPhysicalDeviceMaintenance4Features *>(pNext);
                features->maintenance4 |= enabled->maintenance4 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES: {
                const VkPhysicalDeviceVulkan14Features *enabled = reinterpret_cast<const VkPhysicalDeviceVulkan14Features *>(pNext);
                features->globalPriorityQuery |= enabled->globalPriorityQuery == VK_TRUE;
                features->shaderSubgroupRotate |= enabled->shaderSubgroupRotate == VK_TRUE;
                features->shaderSubgroupRotateClustered |= enabled->shaderSubgroupRotateClustered == VK_TRUE;
                features->shaderFloatControls2 |= enabled->shaderFloatControls2 == VK_TRUE;
                features->shaderExpectAssume |= enabled->shaderExpectAssume == VK_TRUE;
                features->rectangularLines |= enabled->rectangularLines == VK_TRUE;
                features->bresenhamLines |= enabled->bresenhamLines == VK_TRUE;
                features->smoothLines |= enabled->smoothLines == VK_TRUE;
                features->stippledRectangularLines |= enabled->stippledRectangularLines == VK_TRUE;
                features->stippledBresenhamLines |= enabled->stippledBresenhamLines == VK_TRUE;
                features->stippledSmoothLines |= enabled->stippledSmoothLines == VK_TRUE;
                features->vertexAttributeInstanceRateDivisor |= enabled->vertexAttributeInstanceRateDivisor == VK_TRUE;
                features->vertexAttributeInstanceRateZeroDivisor |= enabled->vertexAttributeInstanceRateZeroDivisor == VK_TRUE;
                features->indexTypeUint8 |= enabled->indexTypeUint8 == VK_TRUE;
                features->dynamicRenderingLocalRead |= enabled->dynamicRenderingLocalRead == VK_TRUE;
                features->maintenance5 |= enabled->maintenance5 == VK_TRUE;
                features->maintenance6 |= enabled->maintenance6 == VK_TRUE;
                features->pipelineProtectedAccess |= enabled->pipelineProtectedAccess == VK_TRUE;
                features->pipelineRobustness |= enabled->pipelineRobustness == VK_TRUE;
                features->hostImageCopy |= enabled->hostImageCopy == VK_TRUE;
                features->pushDescriptor |= enabled->pushDescriptor == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES: {
                const VkPhysicalDeviceGlobalPriorityQueryFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceGlobalPriorityQueryFeatures *>(pNext);
                features->globalPriorityQuery |= enabled->globalPriorityQuery == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES: {
                const VkPhysicalDeviceShaderSubgroupRotateFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderSubgroupRotateFeatures *>(pNext);
                features->shaderSubgroupRotate |= enabled->shaderSubgroupRotate == VK_TRUE;
                features->shaderSubgroupRotateClustered |= enabled->shaderSubgroupRotateClustered == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES: {
                const VkPhysicalDeviceShaderFloatControls2Features *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderFloatControls2Features *>(pNext);
                features->shaderFloatControls2 |= enabled->shaderFloatControls2 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES: {
                const VkPhysicalDeviceShaderExpectAssumeFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderExpectAssumeFeatures *>(pNext);
                features->shaderExpectAssume |= enabled->shaderExpectAssume == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES: {
                const VkPhysicalDeviceLineRasterizationFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceLineRasterizationFeatures *>(pNext);
                features->rectangularLines |= enabled->rectangularLines == VK_TRUE;
                features->bresenhamLines |= enabled->bresenhamLines == VK_TRUE;
                features->smoothLines |= enabled->smoothLines == VK_TRUE;
                features->stippledRectangularLines |= enabled->stippledRectangularLines == VK_TRUE;
                features->stippledBresenhamLines |= enabled->stippledBresenhamLines == VK_TRUE;
                features->stippledSmoothLines |= enabled->stippledSmoothLines == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES: {
                const VkPhysicalDeviceVertexAttributeDivisorFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorFeatures *>(pNext);
                features->vertexAttributeInstanceRateDivisor |= enabled->vertexAttributeInstanceRateDivisor == VK_TRUE;
                features->vertexAttributeInstanceRateZeroDivisor |= enabled->vertexAttributeInstanceRateZeroDivisor == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES: {
                const VkPhysicalDeviceIndexTypeUint8Features *enabled =
                    reinterpret_cast<const VkPhysicalDeviceIndexTypeUint8Features *>(pNext);
                features->indexTypeUint8 |= enabled->indexTypeUint8 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES: {
                const VkPhysicalDeviceMaintenance5Features *enabled =
                    reinterpret_cast<const VkPhysicalDeviceMaintenance5Features *>(pNext);
                features->maintenance5 |= enabled->maintenance5 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES: {
                const VkPhysicalDeviceDynamicRenderingLocalReadFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDynamicRenderingLocalReadFeatures *>(pNext);
                features->dynamicRenderingLocalRead |= enabled->dynamicRenderingLocalRead == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES: {
                const VkPhysicalDeviceMaintenance6Features *enabled =
                    reinterpret_cast<const VkPhysicalDeviceMaintenance6Features *>(pNext);
                features->maintenance6 |= enabled->maintenance6 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES: {
                const VkPhysicalDevicePipelineProtectedAccessFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDevicePipelineProtectedAccessFeatures *>(pNext);
                features->pipelineProtectedAccess |= enabled->pipelineProtectedAccess == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES: {
                const VkPhysicalDevicePipelineRobustnessFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDevicePipelineRobustnessFeatures *>(pNext);
                features->pipelineRobustness |= enabled->pipelineRobustness == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES: {
                const VkPhysicalDeviceHostImageCopyFeatures *enabled =
                    reinterpret_cast<const VkPhysicalDeviceHostImageCopyFeatures *>(pNext);
                features->hostImageCopy |= enabled->hostImageCopy == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR: {
                const VkPhysicalDevicePerformanceQueryFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDevicePerformanceQueryFeaturesKHR *>(pNext);
                features->performanceCounterQueryPools |= enabled->performanceCounterQueryPools == VK_TRUE;
                features->performanceCounterMultipleQueryPools |= enabled->performanceCounterMultipleQueryPools == VK_TRUE;
                break;
            }
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR: {
                const VkPhysicalDevicePortabilitySubsetFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(pNext);
                features->constantAlphaColorBlendFactors |= enabled->constantAlphaColorBlendFactors == VK_TRUE;
                features->events |= enabled->events == VK_TRUE;
                features->imageViewFormatReinterpretation |= enabled->imageViewFormatReinterpretation == VK_TRUE;
                features->imageViewFormatSwizzle |= enabled->imageViewFormatSwizzle == VK_TRUE;
                features->imageView2DOn3DImage |= enabled->imageView2DOn3DImage == VK_TRUE;
                features->multisampleArrayImage |= enabled->multisampleArrayImage == VK_TRUE;
                features->mutableComparisonSamplers |= enabled->mutableComparisonSamplers == VK_TRUE;
                features->pointPolygons |= enabled->pointPolygons == VK_TRUE;
                features->samplerMipLodBias |= enabled->samplerMipLodBias == VK_TRUE;
                features->separateStencilMaskRef |= enabled->separateStencilMaskRef == VK_TRUE;
                features->shaderSampleRateInterpolationFunctions |= enabled->shaderSampleRateInterpolationFunctions == VK_TRUE;
                features->tessellationIsolines |= enabled->tessellationIsolines == VK_TRUE;
                features->tessellationPointMode |= enabled->tessellationPointMode == VK_TRUE;
                features->triangleFans |= enabled->triangleFans == VK_TRUE;
                features->vertexAttributeAccessBeyondStride |= enabled->vertexAttributeAccessBeyondStride == VK_TRUE;
                break;
            }
#endif  // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR: {
                const VkPhysicalDeviceShaderClockFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderClockFeaturesKHR *>(pNext);
                features->shaderSubgroupClock |= enabled->shaderSubgroupClock == VK_TRUE;
                features->shaderDeviceClock |= enabled->shaderDeviceClock == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR: {
                const VkPhysicalDeviceFragmentShadingRateFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateFeaturesKHR *>(pNext);
                features->pipelineFragmentShadingRate |= enabled->pipelineFragmentShadingRate == VK_TRUE;
                features->primitiveFragmentShadingRate |= enabled->primitiveFragmentShadingRate == VK_TRUE;
                features->attachmentFragmentShadingRate |= enabled->attachmentFragmentShadingRate == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_QUAD_CONTROL_FEATURES_KHR: {
                const VkPhysicalDeviceShaderQuadControlFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderQuadControlFeaturesKHR *>(pNext);
                features->shaderQuadControl |= enabled->shaderQuadControl == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR: {
                const VkPhysicalDevicePresentWaitFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDevicePresentWaitFeaturesKHR *>(pNext);
                features->presentWait |= enabled->presentWait == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR: {
                const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR *>(pNext);
                features->pipelineExecutableInfo |= enabled->pipelineExecutableInfo == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR: {
                const VkPhysicalDevicePresentIdFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDevicePresentIdFeaturesKHR *>(pNext);
                features->presentId |= enabled->presentId == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR: {
                const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR *>(pNext);
                features->fragmentShaderBarycentric |= enabled->fragmentShaderBarycentric == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR: {
                const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR *>(pNext);
                features->shaderSubgroupUniformControlFlow |= enabled->shaderSubgroupUniformControlFlow == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR: {
                const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR *>(pNext);
                features->workgroupMemoryExplicitLayout |= enabled->workgroupMemoryExplicitLayout == VK_TRUE;
                features->workgroupMemoryExplicitLayoutScalarBlockLayout |=
                    enabled->workgroupMemoryExplicitLayoutScalarBlockLayout == VK_TRUE;
                features->workgroupMemoryExplicitLayout8BitAccess |= enabled->workgroupMemoryExplicitLayout8BitAccess == VK_TRUE;
                features->workgroupMemoryExplicitLayout16BitAccess |= enabled->workgroupMemoryExplicitLayout16BitAccess == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR: {
                const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR *>(pNext);
                features->rayTracingMaintenance1 |= enabled->rayTracingMaintenance1 == VK_TRUE;
                features->rayTracingPipelineTraceRaysIndirect2 |= enabled->rayTracingPipelineTraceRaysIndirect2 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR: {
                const VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR *>(pNext);
                features->shaderMaximalReconvergence |= enabled->shaderMaximalReconvergence == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR: {
                const VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR *>(pNext);
                features->rayTracingPositionFetch |= enabled->rayTracingPositionFetch == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_FEATURES_KHR: {
                const VkPhysicalDevicePipelineBinaryFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDevicePipelineBinaryFeaturesKHR *>(pNext);
                features->pipelineBinaries |= enabled->pipelineBinaries == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR: {
                const VkPhysicalDeviceCooperativeMatrixFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixFeaturesKHR *>(pNext);
                features->cooperativeMatrix |= enabled->cooperativeMatrix == VK_TRUE;
                features->cooperativeMatrixRobustBufferAccess |= enabled->cooperativeMatrixRobustBufferAccess == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR: {
                const VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR *>(pNext);
                features->computeDerivativeGroupQuads |= enabled->computeDerivativeGroupQuads == VK_TRUE;
                features->computeDerivativeGroupLinear |= enabled->computeDerivativeGroupLinear == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_AV1_FEATURES_KHR: {
                const VkPhysicalDeviceVideoEncodeAV1FeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceVideoEncodeAV1FeaturesKHR *>(pNext);
                features->videoEncodeAV1 |= enabled->videoEncodeAV1 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_1_FEATURES_KHR: {
                const VkPhysicalDeviceVideoMaintenance1FeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceVideoMaintenance1FeaturesKHR *>(pNext);
                features->videoMaintenance1 |= enabled->videoMaintenance1 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUANTIZATION_MAP_FEATURES_KHR: {
                const VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR *>(pNext);
                features->videoEncodeQuantizationMap |= enabled->videoEncodeQuantizationMap == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_RELAXED_EXTENDED_INSTRUCTION_FEATURES_KHR: {
                const VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR *>(pNext);
                features->shaderRelaxedExtendedInstruction |= enabled->shaderRelaxedExtendedInstruction == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR: {
                const VkPhysicalDeviceMaintenance7FeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceMaintenance7FeaturesKHR *>(pNext);
                features->maintenance7 |= enabled->maintenance7 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR: {
                const VkPhysicalDeviceMaintenance8FeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceMaintenance8FeaturesKHR *>(pNext);
                features->maintenance8 |= enabled->maintenance8 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_2_FEATURES_KHR: {
                const VkPhysicalDeviceVideoMaintenance2FeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceVideoMaintenance2FeaturesKHR *>(pNext);
                features->videoMaintenance2 |= enabled->videoMaintenance2 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_KHR: {
                const VkPhysicalDeviceDepthClampZeroOneFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDepthClampZeroOneFeaturesKHR *>(pNext);
                features->depthClampZeroOne |= enabled->depthClampZeroOne == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT: {
                const VkPhysicalDeviceTransformFeedbackFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceTransformFeedbackFeaturesEXT *>(pNext);
                features->transformFeedback |= enabled->transformFeedback == VK_TRUE;
                features->geometryStreams |= enabled->geometryStreams == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV: {
                const VkPhysicalDeviceCornerSampledImageFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceCornerSampledImageFeaturesNV *>(pNext);
                features->cornerSampledImage |= enabled->cornerSampledImage == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT: {
                const VkPhysicalDeviceASTCDecodeFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceASTCDecodeFeaturesEXT *>(pNext);
                features->decodeModeSharedExponent |= enabled->decodeModeSharedExponent == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT: {
                const VkPhysicalDeviceConditionalRenderingFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceConditionalRenderingFeaturesEXT *>(pNext);
                features->conditionalRendering |= enabled->conditionalRendering == VK_TRUE;
                features->inheritedConditionalRendering |= enabled->inheritedConditionalRendering == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT: {
                const VkPhysicalDeviceDepthClipEnableFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDepthClipEnableFeaturesEXT *>(pNext);
                features->depthClipEnable |= enabled->depthClipEnable == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RELAXED_LINE_RASTERIZATION_FEATURES_IMG: {
                const VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG *enabled =
                    reinterpret_cast<const VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG *>(pNext);
                features->relaxedLineRasterization |= enabled->relaxedLineRasterization == VK_TRUE;
                break;
            }
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_FEATURES_AMDX: {
                const VkPhysicalDeviceShaderEnqueueFeaturesAMDX *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderEnqueueFeaturesAMDX *>(pNext);
                features->shaderEnqueue |= enabled->shaderEnqueue == VK_TRUE;
                features->shaderMeshEnqueue |= enabled->shaderMeshEnqueue == VK_TRUE;
                break;
            }
#endif  // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT: {
                const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *>(pNext);
                features->advancedBlendCoherentOperations |= enabled->advancedBlendCoherentOperations == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV: {
                const VkPhysicalDeviceShaderSMBuiltinsFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderSMBuiltinsFeaturesNV *>(pNext);
                features->shaderSMBuiltins |= enabled->shaderSMBuiltins == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV: {
                const VkPhysicalDeviceShadingRateImageFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShadingRateImageFeaturesNV *>(pNext);
                features->shadingRateImage |= enabled->shadingRateImage == VK_TRUE;
                features->shadingRateCoarseSampleOrder |= enabled->shadingRateCoarseSampleOrder == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV: {
                const VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV *>(pNext);
                features->representativeFragmentTest |= enabled->representativeFragmentTest == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV: {
                const VkPhysicalDeviceMeshShaderFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesNV *>(pNext);
                features->taskShader |= enabled->taskShader == VK_TRUE;
                features->meshShader |= enabled->meshShader == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV: {
                const VkPhysicalDeviceShaderImageFootprintFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderImageFootprintFeaturesNV *>(pNext);
                features->imageFootprint |= enabled->imageFootprint == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV: {
                const VkPhysicalDeviceExclusiveScissorFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceExclusiveScissorFeaturesNV *>(pNext);
                features->exclusiveScissor |= enabled->exclusiveScissor == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL: {
                const VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL *>(pNext);
                features->shaderIntegerFunctions2 |= enabled->shaderIntegerFunctions2 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT: {
                const VkPhysicalDeviceFragmentDensityMapFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapFeaturesEXT *>(pNext);
                features->fragmentDensityMap |= enabled->fragmentDensityMap == VK_TRUE;
                features->fragmentDensityMapDynamic |= enabled->fragmentDensityMapDynamic == VK_TRUE;
                features->fragmentDensityMapNonSubsampledImages |= enabled->fragmentDensityMapNonSubsampledImages == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD: {
                const VkPhysicalDeviceCoherentMemoryFeaturesAMD *enabled =
                    reinterpret_cast<const VkPhysicalDeviceCoherentMemoryFeaturesAMD *>(pNext);
                features->deviceCoherentMemory |= enabled->deviceCoherentMemory == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT: {
                const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT *>(pNext);
                features->shaderImageInt64Atomics |= enabled->shaderImageInt64Atomics == VK_TRUE;
                features->sparseImageInt64Atomics |= enabled->sparseImageInt64Atomics == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT: {
                const VkPhysicalDeviceMemoryPriorityFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceMemoryPriorityFeaturesEXT *>(pNext);
                features->memoryPriority |= enabled->memoryPriority == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV: {
                const VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV *>(pNext);
                features->dedicatedAllocationImageAliasing |= enabled->dedicatedAllocationImageAliasing == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT: {
                const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT *>(pNext);
                features->bufferDeviceAddressEXT |= enabled->bufferDeviceAddress == VK_TRUE;
                features->bufferDeviceAddressCaptureReplayEXT |= enabled->bufferDeviceAddressCaptureReplay == VK_TRUE;
                features->bufferDeviceAddressMultiDeviceEXT |= enabled->bufferDeviceAddressMultiDevice == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV: {
                const VkPhysicalDeviceCooperativeMatrixFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixFeaturesNV *>(pNext);
                features->cooperativeMatrix |= enabled->cooperativeMatrix == VK_TRUE;
                features->cooperativeMatrixRobustBufferAccess |= enabled->cooperativeMatrixRobustBufferAccess == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV: {
                const VkPhysicalDeviceCoverageReductionModeFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceCoverageReductionModeFeaturesNV *>(pNext);
                features->coverageReductionMode |= enabled->coverageReductionMode == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT: {
                const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *>(pNext);
                features->fragmentShaderSampleInterlock |= enabled->fragmentShaderSampleInterlock == VK_TRUE;
                features->fragmentShaderPixelInterlock |= enabled->fragmentShaderPixelInterlock == VK_TRUE;
                features->fragmentShaderShadingRateInterlock |= enabled->fragmentShaderShadingRateInterlock == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT: {
                const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT *>(pNext);
                features->ycbcrImageArrays |= enabled->ycbcrImageArrays == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT: {
                const VkPhysicalDeviceProvokingVertexFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceProvokingVertexFeaturesEXT *>(pNext);
                features->provokingVertexLast |= enabled->provokingVertexLast == VK_TRUE;
                features->transformFeedbackPreservesProvokingVertex |=
                    enabled->transformFeedbackPreservesProvokingVertex == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT: {
                const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(pNext);
                features->shaderBufferFloat32Atomics |= enabled->shaderBufferFloat32Atomics == VK_TRUE;
                features->shaderBufferFloat32AtomicAdd |= enabled->shaderBufferFloat32AtomicAdd == VK_TRUE;
                features->shaderBufferFloat64Atomics |= enabled->shaderBufferFloat64Atomics == VK_TRUE;
                features->shaderBufferFloat64AtomicAdd |= enabled->shaderBufferFloat64AtomicAdd == VK_TRUE;
                features->shaderSharedFloat32Atomics |= enabled->shaderSharedFloat32Atomics == VK_TRUE;
                features->shaderSharedFloat32AtomicAdd |= enabled->shaderSharedFloat32AtomicAdd == VK_TRUE;
                features->shaderSharedFloat64Atomics |= enabled->shaderSharedFloat64Atomics == VK_TRUE;
                features->shaderSharedFloat64AtomicAdd |= enabled->shaderSharedFloat64AtomicAdd == VK_TRUE;
                features->shaderImageFloat32Atomics |= enabled->shaderImageFloat32Atomics == VK_TRUE;
                features->shaderImageFloat32AtomicAdd |= enabled->shaderImageFloat32AtomicAdd == VK_TRUE;
                features->sparseImageFloat32Atomics |= enabled->sparseImageFloat32Atomics == VK_TRUE;
                features->sparseImageFloat32AtomicAdd |= enabled->sparseImageFloat32AtomicAdd == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT: {
                const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *>(pNext);
                features->extendedDynamicState |= enabled->extendedDynamicState == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_FEATURES_EXT: {
                const VkPhysicalDeviceMapMemoryPlacedFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceMapMemoryPlacedFeaturesEXT *>(pNext);
                features->memoryMapPlaced |= enabled->memoryMapPlaced == VK_TRUE;
                features->memoryMapRangePlaced |= enabled->memoryMapRangePlaced == VK_TRUE;
                features->memoryUnmapReserve |= enabled->memoryUnmapReserve == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT: {
                const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(pNext);
                features->shaderBufferFloat16Atomics |= enabled->shaderBufferFloat16Atomics == VK_TRUE;
                features->shaderBufferFloat16AtomicAdd |= enabled->shaderBufferFloat16AtomicAdd == VK_TRUE;
                features->shaderBufferFloat16AtomicMinMax |= enabled->shaderBufferFloat16AtomicMinMax == VK_TRUE;
                features->shaderBufferFloat32AtomicMinMax |= enabled->shaderBufferFloat32AtomicMinMax == VK_TRUE;
                features->shaderBufferFloat64AtomicMinMax |= enabled->shaderBufferFloat64AtomicMinMax == VK_TRUE;
                features->shaderSharedFloat16Atomics |= enabled->shaderSharedFloat16Atomics == VK_TRUE;
                features->shaderSharedFloat16AtomicAdd |= enabled->shaderSharedFloat16AtomicAdd == VK_TRUE;
                features->shaderSharedFloat16AtomicMinMax |= enabled->shaderSharedFloat16AtomicMinMax == VK_TRUE;
                features->shaderSharedFloat32AtomicMinMax |= enabled->shaderSharedFloat32AtomicMinMax == VK_TRUE;
                features->shaderSharedFloat64AtomicMinMax |= enabled->shaderSharedFloat64AtomicMinMax == VK_TRUE;
                features->shaderImageFloat32AtomicMinMax |= enabled->shaderImageFloat32AtomicMinMax == VK_TRUE;
                features->sparseImageFloat32AtomicMinMax |= enabled->sparseImageFloat32AtomicMinMax == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT: {
                const VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT *>(pNext);
                features->swapchainMaintenance1 |= enabled->swapchainMaintenance1 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV: {
                const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV *>(pNext);
                features->deviceGeneratedCommandsNV |= enabled->deviceGeneratedCommands == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INHERITED_VIEWPORT_SCISSOR_FEATURES_NV: {
                const VkPhysicalDeviceInheritedViewportScissorFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceInheritedViewportScissorFeaturesNV *>(pNext);
                features->inheritedViewportScissor2D |= enabled->inheritedViewportScissor2D == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT: {
                const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT *>(pNext);
                features->texelBufferAlignment |= enabled->texelBufferAlignment == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_BIAS_CONTROL_FEATURES_EXT: {
                const VkPhysicalDeviceDepthBiasControlFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDepthBiasControlFeaturesEXT *>(pNext);
                features->depthBiasControl |= enabled->depthBiasControl == VK_TRUE;
                features->leastRepresentableValueForceUnormRepresentation |=
                    enabled->leastRepresentableValueForceUnormRepresentation == VK_TRUE;
                features->floatRepresentation |= enabled->floatRepresentation == VK_TRUE;
                features->depthBiasExact |= enabled->depthBiasExact == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT: {
                const VkPhysicalDeviceDeviceMemoryReportFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDeviceMemoryReportFeaturesEXT *>(pNext);
                features->deviceMemoryReport |= enabled->deviceMemoryReport == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT: {
                const VkPhysicalDeviceRobustness2FeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceRobustness2FeaturesEXT *>(pNext);
                features->robustBufferAccess2 |= enabled->robustBufferAccess2 == VK_TRUE;
                features->robustImageAccess2 |= enabled->robustImageAccess2 == VK_TRUE;
                features->nullDescriptor |= enabled->nullDescriptor == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT: {
                const VkPhysicalDeviceCustomBorderColorFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceCustomBorderColorFeaturesEXT *>(pNext);
                features->customBorderColors |= enabled->customBorderColors == VK_TRUE;
                features->customBorderColorWithoutFormat |= enabled->customBorderColorWithoutFormat == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_BARRIER_FEATURES_NV: {
                const VkPhysicalDevicePresentBarrierFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDevicePresentBarrierFeaturesNV *>(pNext);
                features->presentBarrier |= enabled->presentBarrier == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DIAGNOSTICS_CONFIG_FEATURES_NV: {
                const VkPhysicalDeviceDiagnosticsConfigFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDiagnosticsConfigFeaturesNV *>(pNext);
                features->diagnosticsConfig |= enabled->diagnosticsConfig == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_FEATURES_NV: {
                const VkPhysicalDeviceCudaKernelLaunchFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceCudaKernelLaunchFeaturesNV *>(pNext);
                features->cudaKernelLaunchFeatures |= enabled->cudaKernelLaunchFeatures == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT: {
                const VkPhysicalDeviceDescriptorBufferFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDescriptorBufferFeaturesEXT *>(pNext);
                features->descriptorBuffer |= enabled->descriptorBuffer == VK_TRUE;
                features->descriptorBufferCaptureReplay |= enabled->descriptorBufferCaptureReplay == VK_TRUE;
                features->descriptorBufferImageLayoutIgnored |= enabled->descriptorBufferImageLayoutIgnored == VK_TRUE;
                features->descriptorBufferPushDescriptors |= enabled->descriptorBufferPushDescriptors == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT: {
                const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT *>(pNext);
                features->graphicsPipelineLibrary |= enabled->graphicsPipelineLibrary == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_FEATURES_AMD: {
                const VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD *>(pNext);
                features->shaderEarlyAndLateFragmentTests |= enabled->shaderEarlyAndLateFragmentTests == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_FEATURES_NV: {
                const VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV *>(pNext);
                features->fragmentShadingRateEnums |= enabled->fragmentShadingRateEnums == VK_TRUE;
                features->supersampleFragmentShadingRates |= enabled->supersampleFragmentShadingRates == VK_TRUE;
                features->noInvocationFragmentShadingRates |= enabled->noInvocationFragmentShadingRates == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MOTION_BLUR_FEATURES_NV: {
                const VkPhysicalDeviceRayTracingMotionBlurFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceRayTracingMotionBlurFeaturesNV *>(pNext);
                features->rayTracingMotionBlur |= enabled->rayTracingMotionBlur == VK_TRUE;
                features->rayTracingMotionBlurPipelineTraceRaysIndirect |=
                    enabled->rayTracingMotionBlurPipelineTraceRaysIndirect == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT: {
                const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT *>(pNext);
                features->ycbcr2plane444Formats |= enabled->ycbcr2plane444Formats == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_FEATURES_EXT: {
                const VkPhysicalDeviceFragmentDensityMap2FeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceFragmentDensityMap2FeaturesEXT *>(pNext);
                features->fragmentDensityMapDeferred |= enabled->fragmentDensityMapDeferred == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT: {
                const VkPhysicalDeviceImageCompressionControlFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceImageCompressionControlFeaturesEXT *>(pNext);
                features->imageCompressionControl |= enabled->imageCompressionControl == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_FEATURES_EXT: {
                const VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT *>(pNext);
                features->attachmentFeedbackLoopLayout |= enabled->attachmentFeedbackLoopLayout == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT: {
                const VkPhysicalDevice4444FormatsFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDevice4444FormatsFeaturesEXT *>(pNext);
                features->formatA4R4G4B4 |= enabled->formatA4R4G4B4 == VK_TRUE;
                features->formatA4B4G4R4 |= enabled->formatA4B4G4R4 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT: {
                const VkPhysicalDeviceFaultFeaturesEXT *enabled = reinterpret_cast<const VkPhysicalDeviceFaultFeaturesEXT *>(pNext);
                features->deviceFault |= enabled->deviceFault == VK_TRUE;
                features->deviceFaultVendorBinary |= enabled->deviceFaultVendorBinary == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT: {
                const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT *>(pNext);
                features->rasterizationOrderColorAttachmentAccess |= enabled->rasterizationOrderColorAttachmentAccess == VK_TRUE;
                features->rasterizationOrderDepthAttachmentAccess |= enabled->rasterizationOrderDepthAttachmentAccess == VK_TRUE;
                features->rasterizationOrderStencilAttachmentAccess |=
                    enabled->rasterizationOrderStencilAttachmentAccess == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT: {
                const VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT *>(pNext);
                features->formatRgba10x6WithoutYCbCrSampler |= enabled->formatRgba10x6WithoutYCbCrSampler == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT: {
                const VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT *>(pNext);
                features->mutableDescriptorType |= enabled->mutableDescriptorType == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT: {
                const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *>(pNext);
                features->vertexInputDynamicState |= enabled->vertexInputDynamicState == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT: {
                const VkPhysicalDeviceAddressBindingReportFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceAddressBindingReportFeaturesEXT *>(pNext);
                features->reportAddressBinding |= enabled->reportAddressBinding == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT: {
                const VkPhysicalDeviceDepthClipControlFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDepthClipControlFeaturesEXT *>(pNext);
                features->depthClipControl |= enabled->depthClipControl == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT: {
                const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT *>(pNext);
                features->primitiveTopologyListRestart |= enabled->primitiveTopologyListRestart == VK_TRUE;
                features->primitiveTopologyPatchListRestart |= enabled->primitiveTopologyPatchListRestart == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_EXT: {
                const VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT *>(pNext);
                features->presentModeFifoLatestReady |= enabled->presentModeFifoLatestReady == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_FEATURES_HUAWEI: {
                const VkPhysicalDeviceSubpassShadingFeaturesHUAWEI *enabled =
                    reinterpret_cast<const VkPhysicalDeviceSubpassShadingFeaturesHUAWEI *>(pNext);
                features->subpassShading |= enabled->subpassShading == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INVOCATION_MASK_FEATURES_HUAWEI: {
                const VkPhysicalDeviceInvocationMaskFeaturesHUAWEI *enabled =
                    reinterpret_cast<const VkPhysicalDeviceInvocationMaskFeaturesHUAWEI *>(pNext);
                features->invocationMask |= enabled->invocationMask == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_RDMA_FEATURES_NV: {
                const VkPhysicalDeviceExternalMemoryRDMAFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceExternalMemoryRDMAFeaturesNV *>(pNext);
                features->externalMemoryRDMA |= enabled->externalMemoryRDMA == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROPERTIES_FEATURES_EXT: {
                const VkPhysicalDevicePipelinePropertiesFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDevicePipelinePropertiesFeaturesEXT *>(pNext);
                features->pipelinePropertiesIdentifier |= enabled->pipelinePropertiesIdentifier == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT: {
                const VkPhysicalDeviceFrameBoundaryFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceFrameBoundaryFeaturesEXT *>(pNext);
                features->frameBoundary |= enabled->frameBoundary == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT: {
                const VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT *>(pNext);
                features->multisampledRenderToSingleSampled |= enabled->multisampledRenderToSingleSampled == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT: {
                const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *>(pNext);
                features->extendedDynamicState2 |= enabled->extendedDynamicState2 == VK_TRUE;
                features->extendedDynamicState2LogicOp |= enabled->extendedDynamicState2LogicOp == VK_TRUE;
                features->extendedDynamicState2PatchControlPoints |= enabled->extendedDynamicState2PatchControlPoints == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT: {
                const VkPhysicalDeviceColorWriteEnableFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceColorWriteEnableFeaturesEXT *>(pNext);
                features->colorWriteEnable |= enabled->colorWriteEnable == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT: {
                const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT *>(pNext);
                features->primitivesGeneratedQuery |= enabled->primitivesGeneratedQuery == VK_TRUE;
                features->primitivesGeneratedQueryWithRasterizerDiscard |=
                    enabled->primitivesGeneratedQueryWithRasterizerDiscard == VK_TRUE;
                features->primitivesGeneratedQueryWithNonZeroStreams |=
                    enabled->primitivesGeneratedQueryWithNonZeroStreams == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT: {
                const VkPhysicalDeviceImageViewMinLodFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceImageViewMinLodFeaturesEXT *>(pNext);
                features->minLod |= enabled->minLod == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT: {
                const VkPhysicalDeviceMultiDrawFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceMultiDrawFeaturesEXT *>(pNext);
                features->multiDraw |= enabled->multiDraw == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT: {
                const VkPhysicalDeviceImage2DViewOf3DFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceImage2DViewOf3DFeaturesEXT *>(pNext);
                features->image2DViewOf3D |= enabled->image2DViewOf3D == VK_TRUE;
                features->sampler2DViewOf3D |= enabled->sampler2DViewOf3D == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_FEATURES_EXT: {
                const VkPhysicalDeviceShaderTileImageFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderTileImageFeaturesEXT *>(pNext);
                features->shaderTileImageColorReadAccess |= enabled->shaderTileImageColorReadAccess == VK_TRUE;
                features->shaderTileImageDepthReadAccess |= enabled->shaderTileImageDepthReadAccess == VK_TRUE;
                features->shaderTileImageStencilReadAccess |= enabled->shaderTileImageStencilReadAccess == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT: {
                const VkPhysicalDeviceOpacityMicromapFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceOpacityMicromapFeaturesEXT *>(pNext);
                features->micromap |= enabled->micromap == VK_TRUE;
                features->micromapCaptureReplay |= enabled->micromapCaptureReplay == VK_TRUE;
                features->micromapHostCommands |= enabled->micromapHostCommands == VK_TRUE;
                break;
            }
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_FEATURES_NV: {
                const VkPhysicalDeviceDisplacementMicromapFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDisplacementMicromapFeaturesNV *>(pNext);
                features->displacementMicromap |= enabled->displacementMicromap == VK_TRUE;
                break;
            }
#endif  // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_FEATURES_HUAWEI: {
                const VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI *enabled =
                    reinterpret_cast<const VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI *>(pNext);
                features->clustercullingShader |= enabled->clustercullingShader == VK_TRUE;
                features->multiviewClusterCullingShader |= enabled->multiviewClusterCullingShader == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_VRS_FEATURES_HUAWEI: {
                const VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI *enabled =
                    reinterpret_cast<const VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI *>(pNext);
                features->clusterShadingRate |= enabled->clusterShadingRate == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BORDER_COLOR_SWIZZLE_FEATURES_EXT: {
                const VkPhysicalDeviceBorderColorSwizzleFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceBorderColorSwizzleFeaturesEXT *>(pNext);
                features->borderColorSwizzle |= enabled->borderColorSwizzle == VK_TRUE;
                features->borderColorSwizzleFromImage |= enabled->borderColorSwizzleFromImage == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT: {
                const VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT *>(pNext);
                features->pageableDeviceLocalMemory |= enabled->pageableDeviceLocalMemory == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_FEATURES_ARM: {
                const VkPhysicalDeviceSchedulingControlsFeaturesARM *enabled =
                    reinterpret_cast<const VkPhysicalDeviceSchedulingControlsFeaturesARM *>(pNext);
                features->schedulingControls |= enabled->schedulingControls == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT: {
                const VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT *>(pNext);
                features->imageSlicedViewOf3D |= enabled->imageSlicedViewOf3D == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_SET_HOST_MAPPING_FEATURES_VALVE: {
                const VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE *>(pNext);
                features->descriptorSetHostMapping |= enabled->descriptorSetHostMapping == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NON_SEAMLESS_CUBE_MAP_FEATURES_EXT: {
                const VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT *>(pNext);
                features->nonSeamlessCubeMap |= enabled->nonSeamlessCubeMap == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_FEATURES_ARM: {
                const VkPhysicalDeviceRenderPassStripedFeaturesARM *enabled =
                    reinterpret_cast<const VkPhysicalDeviceRenderPassStripedFeaturesARM *>(pNext);
                features->renderPassStriped |= enabled->renderPassStriped == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_QCOM: {
                const VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM *enabled =
                    reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM *>(pNext);
                features->fragmentDensityMapOffset |= enabled->fragmentDensityMapOffset == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_FEATURES_NV: {
                const VkPhysicalDeviceCopyMemoryIndirectFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceCopyMemoryIndirectFeaturesNV *>(pNext);
                features->indirectCopy |= enabled->indirectCopy == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV: {
                const VkPhysicalDeviceMemoryDecompressionFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceMemoryDecompressionFeaturesNV *>(pNext);
                features->memoryDecompression |= enabled->memoryDecompression == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_COMPUTE_FEATURES_NV: {
                const VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV *>(pNext);
                features->deviceGeneratedCompute |= enabled->deviceGeneratedCompute == VK_TRUE;
                features->deviceGeneratedComputePipelines |= enabled->deviceGeneratedComputePipelines == VK_TRUE;
                features->deviceGeneratedComputeCaptureReplay |= enabled->deviceGeneratedComputeCaptureReplay == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_LINEAR_SWEPT_SPHERES_FEATURES_NV: {
                const VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV *>(pNext);
                features->spheres |= enabled->spheres == VK_TRUE;
                features->linearSweptSpheres |= enabled->linearSweptSpheres == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINEAR_COLOR_ATTACHMENT_FEATURES_NV: {
                const VkPhysicalDeviceLinearColorAttachmentFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceLinearColorAttachmentFeaturesNV *>(pNext);
                features->linearColorAttachment |= enabled->linearColorAttachment == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT: {
                const VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT *>(pNext);
                features->imageCompressionControlSwapchain |= enabled->imageCompressionControlSwapchain == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_FEATURES_QCOM: {
                const VkPhysicalDeviceImageProcessingFeaturesQCOM *enabled =
                    reinterpret_cast<const VkPhysicalDeviceImageProcessingFeaturesQCOM *>(pNext);
                features->textureSampleWeighted |= enabled->textureSampleWeighted == VK_TRUE;
                features->textureBoxFilter |= enabled->textureBoxFilter == VK_TRUE;
                features->textureBlockMatch |= enabled->textureBlockMatch == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT: {
                const VkPhysicalDeviceNestedCommandBufferFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceNestedCommandBufferFeaturesEXT *>(pNext);
                features->nestedCommandBuffer |= enabled->nestedCommandBuffer == VK_TRUE;
                features->nestedCommandBufferRendering |= enabled->nestedCommandBufferRendering == VK_TRUE;
                features->nestedCommandBufferSimultaneousUse |= enabled->nestedCommandBufferSimultaneousUse == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT: {
                const VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(pNext);
                features->extendedDynamicState3TessellationDomainOrigin |=
                    enabled->extendedDynamicState3TessellationDomainOrigin == VK_TRUE;
                features->extendedDynamicState3DepthClampEnable |= enabled->extendedDynamicState3DepthClampEnable == VK_TRUE;
                features->extendedDynamicState3PolygonMode |= enabled->extendedDynamicState3PolygonMode == VK_TRUE;
                features->extendedDynamicState3RasterizationSamples |=
                    enabled->extendedDynamicState3RasterizationSamples == VK_TRUE;
                features->extendedDynamicState3SampleMask |= enabled->extendedDynamicState3SampleMask == VK_TRUE;
                features->extendedDynamicState3AlphaToCoverageEnable |=
                    enabled->extendedDynamicState3AlphaToCoverageEnable == VK_TRUE;
                features->extendedDynamicState3AlphaToOneEnable |= enabled->extendedDynamicState3AlphaToOneEnable == VK_TRUE;
                features->extendedDynamicState3LogicOpEnable |= enabled->extendedDynamicState3LogicOpEnable == VK_TRUE;
                features->extendedDynamicState3ColorBlendEnable |= enabled->extendedDynamicState3ColorBlendEnable == VK_TRUE;
                features->extendedDynamicState3ColorBlendEquation |= enabled->extendedDynamicState3ColorBlendEquation == VK_TRUE;
                features->extendedDynamicState3ColorWriteMask |= enabled->extendedDynamicState3ColorWriteMask == VK_TRUE;
                features->extendedDynamicState3RasterizationStream |= enabled->extendedDynamicState3RasterizationStream == VK_TRUE;
                features->extendedDynamicState3ConservativeRasterizationMode |=
                    enabled->extendedDynamicState3ConservativeRasterizationMode == VK_TRUE;
                features->extendedDynamicState3ExtraPrimitiveOverestimationSize |=
                    enabled->extendedDynamicState3ExtraPrimitiveOverestimationSize == VK_TRUE;
                features->extendedDynamicState3DepthClipEnable |= enabled->extendedDynamicState3DepthClipEnable == VK_TRUE;
                features->extendedDynamicState3SampleLocationsEnable |=
                    enabled->extendedDynamicState3SampleLocationsEnable == VK_TRUE;
                features->extendedDynamicState3ColorBlendAdvanced |= enabled->extendedDynamicState3ColorBlendAdvanced == VK_TRUE;
                features->extendedDynamicState3ProvokingVertexMode |= enabled->extendedDynamicState3ProvokingVertexMode == VK_TRUE;
                features->extendedDynamicState3LineRasterizationMode |=
                    enabled->extendedDynamicState3LineRasterizationMode == VK_TRUE;
                features->extendedDynamicState3LineStippleEnable |= enabled->extendedDynamicState3LineStippleEnable == VK_TRUE;
                features->extendedDynamicState3DepthClipNegativeOneToOne |=
                    enabled->extendedDynamicState3DepthClipNegativeOneToOne == VK_TRUE;
                features->extendedDynamicState3ViewportWScalingEnable |=
                    enabled->extendedDynamicState3ViewportWScalingEnable == VK_TRUE;
                features->extendedDynamicState3ViewportSwizzle |= enabled->extendedDynamicState3ViewportSwizzle == VK_TRUE;
                features->extendedDynamicState3CoverageToColorEnable |=
                    enabled->extendedDynamicState3CoverageToColorEnable == VK_TRUE;
                features->extendedDynamicState3CoverageToColorLocation |=
                    enabled->extendedDynamicState3CoverageToColorLocation == VK_TRUE;
                features->extendedDynamicState3CoverageModulationMode |=
                    enabled->extendedDynamicState3CoverageModulationMode == VK_TRUE;
                features->extendedDynamicState3CoverageModulationTableEnable |=
                    enabled->extendedDynamicState3CoverageModulationTableEnable == VK_TRUE;
                features->extendedDynamicState3CoverageModulationTable |=
                    enabled->extendedDynamicState3CoverageModulationTable == VK_TRUE;
                features->extendedDynamicState3CoverageReductionMode |=
                    enabled->extendedDynamicState3CoverageReductionMode == VK_TRUE;
                features->extendedDynamicState3RepresentativeFragmentTestEnable |=
                    enabled->extendedDynamicState3RepresentativeFragmentTestEnable == VK_TRUE;
                features->extendedDynamicState3ShadingRateImageEnable |=
                    enabled->extendedDynamicState3ShadingRateImageEnable == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_MERGE_FEEDBACK_FEATURES_EXT: {
                const VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT *>(pNext);
                features->subpassMergeFeedback |= enabled->subpassMergeFeedback == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_FEATURES_EXT: {
                const VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT *>(pNext);
                features->shaderModuleIdentifier |= enabled->shaderModuleIdentifier == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_FEATURES_NV: {
                const VkPhysicalDeviceOpticalFlowFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceOpticalFlowFeaturesNV *>(pNext);
                features->opticalFlow |= enabled->opticalFlow == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT: {
                const VkPhysicalDeviceLegacyDitheringFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceLegacyDitheringFeaturesEXT *>(pNext);
                features->legacyDithering |= enabled->legacyDithering == VK_TRUE;
                break;
            }
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_FEATURES_ANDROID: {
                const VkPhysicalDeviceExternalFormatResolveFeaturesANDROID *enabled =
                    reinterpret_cast<const VkPhysicalDeviceExternalFormatResolveFeaturesANDROID *>(pNext);
                features->externalFormatResolve |= enabled->externalFormatResolve == VK_TRUE;
                break;
            }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ANTI_LAG_FEATURES_AMD: {
                const VkPhysicalDeviceAntiLagFeaturesAMD *enabled =
                    reinterpret_cast<const VkPhysicalDeviceAntiLagFeaturesAMD *>(pNext);
                features->antiLag |= enabled->antiLag == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT: {
                const VkPhysicalDeviceShaderObjectFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderObjectFeaturesEXT *>(pNext);
                features->shaderObject |= enabled->shaderObject == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_PROPERTIES_FEATURES_QCOM: {
                const VkPhysicalDeviceTilePropertiesFeaturesQCOM *enabled =
                    reinterpret_cast<const VkPhysicalDeviceTilePropertiesFeaturesQCOM *>(pNext);
                features->tileProperties |= enabled->tileProperties == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_AMIGO_PROFILING_FEATURES_SEC: {
                const VkPhysicalDeviceAmigoProfilingFeaturesSEC *enabled =
                    reinterpret_cast<const VkPhysicalDeviceAmigoProfilingFeaturesSEC *>(pNext);
                features->amigoProfiling |= enabled->amigoProfiling == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_VIEWPORTS_FEATURES_QCOM: {
                const VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM *enabled =
                    reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM *>(pNext);
                features->multiviewPerViewViewports |= enabled->multiviewPerViewViewports == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_FEATURES_NV: {
                const VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV *>(pNext);
                features->rayTracingInvocationReorder |= enabled->rayTracingInvocationReorder == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_FEATURES_NV: {
                const VkPhysicalDeviceCooperativeVectorFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceCooperativeVectorFeaturesNV *>(pNext);
                features->cooperativeVector |= enabled->cooperativeVector == VK_TRUE;
                features->cooperativeVectorTraining |= enabled->cooperativeVectorTraining == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_FEATURES_NV: {
                const VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV *>(pNext);
                features->extendedSparseAddressSpace |= enabled->extendedSparseAddressSpace == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_FEATURES_EXT: {
                const VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT *>(pNext);
                features->legacyVertexAttributes |= enabled->legacyVertexAttributes == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_FEATURES_ARM: {
                const VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM *>(pNext);
                features->shaderCoreBuiltins |= enabled->shaderCoreBuiltins == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_LIBRARY_GROUP_HANDLES_FEATURES_EXT: {
                const VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT *>(pNext);
                features->pipelineLibraryGroupHandles |= enabled->pipelineLibraryGroupHandles == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT: {
                const VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT *>(pNext);
                features->dynamicRenderingUnusedAttachments |= enabled->dynamicRenderingUnusedAttachments == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_RENDER_AREAS_FEATURES_QCOM: {
                const VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM *enabled =
                    reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM *>(pNext);
                features->multiviewPerViewRenderAreas |= enabled->multiviewPerViewRenderAreas == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PER_STAGE_DESCRIPTOR_SET_FEATURES_NV: {
                const VkPhysicalDevicePerStageDescriptorSetFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDevicePerStageDescriptorSetFeaturesNV *>(pNext);
                features->perStageDescriptorSet |= enabled->perStageDescriptorSet == VK_TRUE;
                features->dynamicPipelineLayout |= enabled->dynamicPipelineLayout == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_FEATURES_QCOM: {
                const VkPhysicalDeviceImageProcessing2FeaturesQCOM *enabled =
                    reinterpret_cast<const VkPhysicalDeviceImageProcessing2FeaturesQCOM *>(pNext);
                features->textureBlockMatch2 |= enabled->textureBlockMatch2 == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_WEIGHTS_FEATURES_QCOM: {
                const VkPhysicalDeviceCubicWeightsFeaturesQCOM *enabled =
                    reinterpret_cast<const VkPhysicalDeviceCubicWeightsFeaturesQCOM *>(pNext);
                features->selectableCubicWeights |= enabled->selectableCubicWeights == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_DEGAMMA_FEATURES_QCOM: {
                const VkPhysicalDeviceYcbcrDegammaFeaturesQCOM *enabled =
                    reinterpret_cast<const VkPhysicalDeviceYcbcrDegammaFeaturesQCOM *>(pNext);
                features->ycbcrDegamma |= enabled->ycbcrDegamma == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_CLAMP_FEATURES_QCOM: {
                const VkPhysicalDeviceCubicClampFeaturesQCOM *enabled =
                    reinterpret_cast<const VkPhysicalDeviceCubicClampFeaturesQCOM *>(pNext);
                features->cubicRangeClamp |= enabled->cubicRangeClamp == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_FEATURES_EXT: {
                const VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT *>(pNext);
                features->attachmentFeedbackLoopDynamicState |= enabled->attachmentFeedbackLoopDynamicState == VK_TRUE;
                break;
            }
#ifdef VK_USE_PLATFORM_SCREEN_QNX
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_SCREEN_BUFFER_FEATURES_QNX: {
                const VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX *enabled =
                    reinterpret_cast<const VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX *>(pNext);
                features->screenBufferImport |= enabled->screenBufferImport == VK_TRUE;
                break;
            }
#endif  // VK_USE_PLATFORM_SCREEN_QNX
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_POOL_OVERALLOCATION_FEATURES_NV: {
                const VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV *>(pNext);
                features->descriptorPoolOverallocation |= enabled->descriptorPoolOverallocation == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAW_ACCESS_CHAINS_FEATURES_NV: {
                const VkPhysicalDeviceRawAccessChainsFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceRawAccessChainsFeaturesNV *>(pNext);
                features->shaderRawAccessChains |= enabled->shaderRawAccessChains == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMMAND_BUFFER_INHERITANCE_FEATURES_NV: {
                const VkPhysicalDeviceCommandBufferInheritanceFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceCommandBufferInheritanceFeaturesNV *>(pNext);
                features->commandBufferInheritance |= enabled->commandBufferInheritance == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT16_VECTOR_FEATURES_NV: {
                const VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV *>(pNext);
                features->shaderFloat16VectorAtomics |= enabled->shaderFloat16VectorAtomics == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_REPLICATED_COMPOSITES_FEATURES_EXT: {
                const VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT *>(pNext);
                features->shaderReplicatedComposites |= enabled->shaderReplicatedComposites == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_VALIDATION_FEATURES_NV: {
                const VkPhysicalDeviceRayTracingValidationFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceRayTracingValidationFeaturesNV *>(pNext);
                features->rayTracingValidation |= enabled->rayTracingValidation == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_FEATURES_NV: {
                const VkPhysicalDeviceClusterAccelerationStructureFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceClusterAccelerationStructureFeaturesNV *>(pNext);
                features->clusterAccelerationStructure |= enabled->clusterAccelerationStructure == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_FEATURES_NV: {
                const VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV *>(pNext);
                features->partitionedAccelerationStructure |= enabled->partitionedAccelerationStructure == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT: {
                const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT *>(pNext);
                features->deviceGeneratedCommands |= enabled->deviceGeneratedCommands == VK_TRUE;
                features->dynamicGeneratedPipelineLayout |= enabled->dynamicGeneratedPipelineLayout == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_FEATURES_MESA: {
                const VkPhysicalDeviceImageAlignmentControlFeaturesMESA *enabled =
                    reinterpret_cast<const VkPhysicalDeviceImageAlignmentControlFeaturesMESA *>(pNext);
                features->imageAlignmentControl |= enabled->imageAlignmentControl == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_CONTROL_FEATURES_EXT: {
                const VkPhysicalDeviceDepthClampControlFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceDepthClampControlFeaturesEXT *>(pNext);
                features->depthClampControl |= enabled->depthClampControl == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HDR_VIVID_FEATURES_HUAWEI: {
                const VkPhysicalDeviceHdrVividFeaturesHUAWEI *enabled =
                    reinterpret_cast<const VkPhysicalDeviceHdrVividFeaturesHUAWEI *>(pNext);
                features->hdrVivid |= enabled->hdrVivid == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_FEATURES_NV: {
                const VkPhysicalDeviceCooperativeMatrix2FeaturesNV *enabled =
                    reinterpret_cast<const VkPhysicalDeviceCooperativeMatrix2FeaturesNV *>(pNext);
                features->cooperativeMatrixWorkgroupScope |= enabled->cooperativeMatrixWorkgroupScope == VK_TRUE;
                features->cooperativeMatrixFlexibleDimensions |= enabled->cooperativeMatrixFlexibleDimensions == VK_TRUE;
                features->cooperativeMatrixReductions |= enabled->cooperativeMatrixReductions == VK_TRUE;
                features->cooperativeMatrixConversions |= enabled->cooperativeMatrixConversions == VK_TRUE;
                features->cooperativeMatrixPerElementOperations |= enabled->cooperativeMatrixPerElementOperations == VK_TRUE;
                features->cooperativeMatrixTensorAddressing |= enabled->cooperativeMatrixTensorAddressing == VK_TRUE;
                features->cooperativeMatrixBlockLoads |= enabled->cooperativeMatrixBlockLoads == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_OPACITY_MICROMAP_FEATURES_ARM: {
                const VkPhysicalDevicePipelineOpacityMicromapFeaturesARM *enabled =
                    reinterpret_cast<const VkPhysicalDevicePipelineOpacityMicromapFeaturesARM *>(pNext);
                features->pipelineOpacityMicromap |= enabled->pipelineOpacityMicromap == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT: {
                const VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT *>(pNext);
                features->vertexAttributeRobustness |= enabled->vertexAttributeRobustness == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR: {
                const VkPhysicalDeviceAccelerationStructureFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceAccelerationStructureFeaturesKHR *>(pNext);
                features->accelerationStructure |= enabled->accelerationStructure == VK_TRUE;
                features->accelerationStructureCaptureReplay |= enabled->accelerationStructureCaptureReplay == VK_TRUE;
                features->accelerationStructureIndirectBuild |= enabled->accelerationStructureIndirectBuild == VK_TRUE;
                features->accelerationStructureHostCommands |= enabled->accelerationStructureHostCommands == VK_TRUE;
                features->descriptorBindingAccelerationStructureUpdateAfterBind |=
                    enabled->descriptorBindingAccelerationStructureUpdateAfterBind == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR: {
                const VkPhysicalDeviceRayTracingPipelineFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceRayTracingPipelineFeaturesKHR *>(pNext);
                features->rayTracingPipeline |= enabled->rayTracingPipeline == VK_TRUE;
                features->rayTracingPipelineShaderGroupHandleCaptureReplay |=
                    enabled->rayTracingPipelineShaderGroupHandleCaptureReplay == VK_TRUE;
                features->rayTracingPipelineShaderGroupHandleCaptureReplayMixed |=
                    enabled->rayTracingPipelineShaderGroupHandleCaptureReplayMixed == VK_TRUE;
                features->rayTracingPipelineTraceRaysIndirect |= enabled->rayTracingPipelineTraceRaysIndirect == VK_TRUE;
                features->rayTraversalPrimitiveCulling |= enabled->rayTraversalPrimitiveCulling == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR: {
                const VkPhysicalDeviceRayQueryFeaturesKHR *enabled =
                    reinterpret_cast<const VkPhysicalDeviceRayQueryFeaturesKHR *>(pNext);
                features->rayQuery |= enabled->rayQuery == VK_TRUE;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT: {
                const VkPhysicalDeviceMeshShaderFeaturesEXT *enabled =
                    reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesEXT *>(pNext);
                features->taskShader |= enabled->taskShader == VK_TRUE;
                features->meshShader |= enabled->meshShader == VK_TRUE;
                features->multiviewMeshShader |= enabled->multiviewMeshShader == VK_TRUE;
                features->primitiveFragmentShadingRateMeshShader |= enabled->primitiveFragmentShadingRateMeshShader == VK_TRUE;
                features->meshShaderQueries |= enabled->meshShaderQueries == VK_TRUE;
                break;
            }

            default:
                break;
        }
    }

    // Some older extensions were made without features, but equivalent features were
    // added to the core spec when they were promoted.  When those extensions are
    // enabled, treat validation rules as if the corresponding feature is enabled.
    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        vvl::Extension extension = GetExtension(pCreateInfo->ppEnabledExtensionNames[i]);
        if (extension == vvl::Extension::_VK_EXT_descriptor_indexing) {
            features->descriptorIndexing = true;
        }
        if (extension == vvl::Extension::_VK_EXT_sampler_filter_minmax) {
            features->samplerFilterMinmax = true;
        }
        if (extension == vvl::Extension::_VK_EXT_shader_viewport_index_layer) {
            features->shaderOutputViewportIndex = true;
            features->shaderOutputLayer = true;
        }
        if (extension == vvl::Extension::_VK_KHR_draw_indirect_count) {
            features->drawIndirectCount = true;
        }
        if (extension == vvl::Extension::_VK_KHR_sampler_mirror_clamp_to_edge) {
            features->samplerMirrorClampToEdge = true;
        }
        if (extension == vvl::Extension::_VK_KHR_shader_draw_parameters) {
            features->shaderDrawParameters = true;
        }
    }

    // texelBufferAlignment was not promoted to VkPhysicalDeviceVulkan13Features
    // but the feature is automatically enabled.
    // Setting the feature explicitly to 'false' doesn't change that
    if (api_version >= VK_API_VERSION_1_3) {
        features->texelBufferAlignment = true;
    }
}

// NOLINTEND
