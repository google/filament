/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2025 Valve Corporation
 * Copyright (c) 2018-2025 LunarG, Inc.
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

#include <cmath>
#include <cstring>
#include <string>
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__GNU__)
#include <unistd.h>
#endif
#include "gpuav/core/gpuav_constants.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "chassis/dispatch_object.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
namespace gpuav {

std::shared_ptr<vvl::Buffer> Validator::CreateBufferState(VkBuffer handle, const VkBufferCreateInfo *create_info) {
    return std::make_shared<Buffer>(*this, handle, create_info, *desc_heap_);
}

std::shared_ptr<vvl::BufferView> Validator::CreateBufferViewState(const std::shared_ptr<vvl::Buffer> &buffer, VkBufferView handle,
                                                                  const VkBufferViewCreateInfo *create_info,
                                                                  VkFormatFeatureFlags2 format_features) {
    return std::make_shared<BufferView>(buffer, handle, create_info, format_features, *desc_heap_);
}

std::shared_ptr<vvl::ImageView> Validator::CreateImageViewState(const std::shared_ptr<vvl::Image> &image_state, VkImageView handle,
                                                                const VkImageViewCreateInfo *create_info,
                                                                VkFormatFeatureFlags2 format_features,
                                                                const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props) {
    return std::make_shared<ImageView>(image_state, handle, create_info, format_features, cubic_props, *desc_heap_);
}

std::shared_ptr<vvl::Sampler> Validator::CreateSamplerState(VkSampler handle, const VkSamplerCreateInfo *create_info) {
    return std::make_shared<Sampler>(handle, create_info, *desc_heap_);
}

std::shared_ptr<vvl::AccelerationStructureKHR> Validator::CreateAccelerationStructureState(
    VkAccelerationStructureKHR handle, const VkAccelerationStructureCreateInfoKHR *create_info,
    std::shared_ptr<vvl::Buffer> &&buf_state) {
    return std::make_shared<AccelerationStructureKHR>(handle, create_info, std::move(buf_state), *desc_heap_);
}

std::shared_ptr<vvl::DescriptorSet> Validator::CreateDescriptorSet(VkDescriptorSet handle, vvl::DescriptorPool *pool,
                                                                   const std::shared_ptr<vvl::DescriptorSetLayout const> &layout,
                                                                   uint32_t variable_count) {
    return std::static_pointer_cast<vvl::DescriptorSet>(
        std::make_shared<DescriptorSet>(handle, pool, layout, variable_count, this));
}

std::shared_ptr<vvl::CommandBuffer> Validator::CreateCmdBufferState(VkCommandBuffer handle,
                                                                    const VkCommandBufferAllocateInfo *allocate_info,
                                                                    const vvl::CommandPool *pool) {
    return std::static_pointer_cast<vvl::CommandBuffer>(std::make_shared<CommandBuffer>(*this, handle, allocate_info, pool));
}

std::shared_ptr<vvl::Queue> Validator::CreateQueue(VkQueue handle, uint32_t family_index, uint32_t queue_index,
                                                   VkDeviceQueueCreateFlags flags,
                                                   const VkQueueFamilyProperties &queueFamilyProperties) {
    return std::static_pointer_cast<vvl::Queue>(
        std::make_shared<Queue>(*this, handle, family_index, queue_index, flags, queueFamilyProperties, timeline_khr_));
}

static std::vector<VkExtensionProperties> GetExtensions(VkPhysicalDevice physical_device) {
    VkResult err;
    uint32_t extension_count = 512;
    std::vector<VkExtensionProperties> extensions(extension_count);
    for (;;) {
        err = DispatchEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, extensions.data());
        if (err == VK_SUCCESS) {
            extensions.resize(extension_count);
            return extensions;
        } else if (err == VK_INCOMPLETE) {
            extension_count *= 2;  // wasn't enough space, increase it
            extensions.resize(extension_count);
        } else {
            return {};
        }
    }
}

static bool IsExtensionAvailable(const char *extension_name, const std::vector<VkExtensionProperties> &available_extensions) {
    for (const VkExtensionProperties &ext : available_extensions) {
        if (strncmp(extension_name, ext.extensionName, VK_MAX_EXTENSION_NAME_SIZE) == 0) {
            return true;
        }
    }

    return false;
}

// Trampolines to make VMA call Dispatch for Vulkan calls
static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL gpuVkGetInstanceProcAddr(VkInstance inst, const char *name) {
    return DispatchGetInstanceProcAddr(inst, name);
}
static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL gpuVkGetDeviceProcAddr(VkDevice dev, const char *name) {
    return DispatchGetDeviceProcAddr(dev, name);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                                   VkPhysicalDeviceProperties *pProperties) {
    DispatchGetPhysicalDeviceProperties(physicalDevice, pProperties);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                                         VkPhysicalDeviceMemoryProperties *pMemoryProperties) {
    DispatchGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                                          const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory) {
    return DispatchAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks *pAllocator) {
    DispatchFreeMemory(device, memory, pAllocator);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                                     VkMemoryMapFlags flags, void **ppData) {
    return DispatchMapMemory(device, memory, offset, size, flags, ppData);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkUnmapMemory(VkDevice device, VkDeviceMemory memory) { DispatchUnmapMemory(device, memory); }
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                                   const VkMappedMemoryRange *pMemoryRanges) {
    return DispatchFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                                        const VkMappedMemoryRange *pMemoryRanges) {
    return DispatchInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                                            VkDeviceSize memoryOffset) {
    return DispatchBindBufferMemory(device, buffer, memory, memoryOffset);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory,
                                                           VkDeviceSize memoryOffset) {
    return DispatchBindImageMemory(device, image, memory, memoryOffset);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                                   VkMemoryRequirements *pMemoryRequirements) {
    DispatchGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                                  VkMemoryRequirements *pMemoryRequirements) {
    DispatchGetImageMemoryRequirements(device, image, pMemoryRequirements);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer) {
    return DispatchCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator) {
    return DispatchDestroyBuffer(device, buffer, pAllocator);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkImage *pImage) {
    return DispatchCreateImage(device, pCreateInfo, pAllocator, pImage);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) {
    DispatchDestroyImage(device, image, pAllocator);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                     uint32_t regionCount, const VkBufferCopy *pRegions) {
    DispatchCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

static VkResult UtilInitializeVma(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device,
                                  bool use_buffer_device_address, VmaAllocator *pAllocator) {
    VmaVulkanFunctions functions;
    VmaAllocatorCreateInfo allocator_info = {};
    allocator_info.instance = instance;
    allocator_info.device = device;
    allocator_info.physicalDevice = physical_device;

    if (use_buffer_device_address) {
        allocator_info.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    }

    functions.vkGetInstanceProcAddr = static_cast<PFN_vkGetInstanceProcAddr>(gpuVkGetInstanceProcAddr);
    functions.vkGetDeviceProcAddr = static_cast<PFN_vkGetDeviceProcAddr>(gpuVkGetDeviceProcAddr);
    functions.vkGetPhysicalDeviceProperties = static_cast<PFN_vkGetPhysicalDeviceProperties>(gpuVkGetPhysicalDeviceProperties);
    functions.vkGetPhysicalDeviceMemoryProperties =
        static_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(gpuVkGetPhysicalDeviceMemoryProperties);
    functions.vkAllocateMemory = static_cast<PFN_vkAllocateMemory>(gpuVkAllocateMemory);
    functions.vkFreeMemory = static_cast<PFN_vkFreeMemory>(gpuVkFreeMemory);
    functions.vkMapMemory = static_cast<PFN_vkMapMemory>(gpuVkMapMemory);
    functions.vkUnmapMemory = static_cast<PFN_vkUnmapMemory>(gpuVkUnmapMemory);
    functions.vkFlushMappedMemoryRanges = static_cast<PFN_vkFlushMappedMemoryRanges>(gpuVkFlushMappedMemoryRanges);
    functions.vkInvalidateMappedMemoryRanges = static_cast<PFN_vkInvalidateMappedMemoryRanges>(gpuVkInvalidateMappedMemoryRanges);
    functions.vkBindBufferMemory = static_cast<PFN_vkBindBufferMemory>(gpuVkBindBufferMemory);
    functions.vkBindImageMemory = static_cast<PFN_vkBindImageMemory>(gpuVkBindImageMemory);
    functions.vkGetBufferMemoryRequirements = static_cast<PFN_vkGetBufferMemoryRequirements>(gpuVkGetBufferMemoryRequirements);
    functions.vkGetImageMemoryRequirements = static_cast<PFN_vkGetImageMemoryRequirements>(gpuVkGetImageMemoryRequirements);
    functions.vkCreateBuffer = static_cast<PFN_vkCreateBuffer>(gpuVkCreateBuffer);
    functions.vkDestroyBuffer = static_cast<PFN_vkDestroyBuffer>(gpuVkDestroyBuffer);
    functions.vkCreateImage = static_cast<PFN_vkCreateImage>(gpuVkCreateImage);
    functions.vkDestroyImage = static_cast<PFN_vkDestroyImage>(gpuVkDestroyImage);
    functions.vkCmdCopyBuffer = static_cast<PFN_vkCmdCopyBuffer>(gpuVkCmdCopyBuffer);
    allocator_info.pVulkanFunctions = &functions;

    return vmaCreateAllocator(&allocator_info, pAllocator);
}

void Instance::PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                         const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, const RecordObject &record_obj,
                                         vku::safe_VkDeviceCreateInfo *modified_create_info) {
    BaseClass::PreCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, record_obj, modified_create_info);

    // GPU-AV requirements not met, exit early or future Vulkan calls may be invalid
    if (api_version < VK_API_VERSION_1_1) {
        return;
    }

    // In PreCallRecord this is all about trying to turn on as many feature/extension as possible on behalf of the app
    // If the features are not supported, can't internal error until post device creation
    VkPhysicalDeviceFeatures supported_features{};
    DispatchGetPhysicalDeviceFeatures(physicalDevice, &supported_features);

    VkPhysicalDeviceFeatures *enabled_features = const_cast<VkPhysicalDeviceFeatures *>(modified_create_info->pEnabledFeatures);
    if (!enabled_features) {
        // Look in a potential VkPhysicalDeviceFeatures2 feature struct
        if (auto *enabled_features_2 = const_cast<VkPhysicalDeviceFeatures2 *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFeatures2>(modified_create_info->pNext))) {
            enabled_features = &enabled_features_2->features;
        }
    }
    if (enabled_features) {
        if (supported_features.fragmentStoresAndAtomics && !enabled_features->fragmentStoresAndAtomics) {
            InternalWarning(instance, record_obj.location, "Forcing fragmentStoresAndAtomics to VK_TRUE");
            enabled_features->fragmentStoresAndAtomics = VK_TRUE;
        }
        if (supported_features.vertexPipelineStoresAndAtomics && !enabled_features->vertexPipelineStoresAndAtomics) {
            InternalWarning(instance, record_obj.location, "Forcing vertexPipelineStoresAndAtomics to VK_TRUE");
            enabled_features->vertexPipelineStoresAndAtomics = VK_TRUE;
        }
        if (supported_features.shaderInt64 && !enabled_features->shaderInt64) {
            InternalWarning(instance, record_obj.location, "Forcing shaderInt64 to VK_TRUE");
            enabled_features->shaderInt64 = VK_TRUE;
        }
    }

    std::vector<VkExtensionProperties> available_extensions = GetExtensions(physicalDevice);

    auto add_missing_features = [this, &record_obj, modified_create_info]() {
        // Add timeline semaphore feature - This is required as we use it to manage when command buffers are submitted at queue
        // submit time
        if (auto *ts_features = const_cast<VkPhysicalDeviceTimelineSemaphoreFeatures *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceTimelineSemaphoreFeatures>(modified_create_info))) {
            if (ts_features->timelineSemaphore == VK_FALSE) {
                InternalWarning(instance, record_obj.location,
                                "Forcing VkPhysicalDeviceTimelineSemaphoreFeatures::timelineSemaphore to VK_TRUE");
                ts_features->timelineSemaphore = VK_TRUE;
            }
        } else {
            InternalWarning(instance, record_obj.location,
                            "Adding a VkPhysicalDeviceTimelineSemaphoreFeatures to pNext with timelineSemaphore set to VK_TRUE");
            VkPhysicalDeviceTimelineSemaphoreFeatures new_ts_features = vku::InitStructHelper();
            new_ts_features.timelineSemaphore = VK_TRUE;
            vku::AddToPnext(*modified_create_info, new_ts_features);
        }

        // need vulkanMemoryModelDeviceScope feature to let us call atomicAdd to the output buffer
        if (auto *mm_features = const_cast<VkPhysicalDeviceVulkanMemoryModelFeatures *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVulkanMemoryModelFeatures>(modified_create_info))) {
            if (mm_features->vulkanMemoryModel == VK_FALSE) {
                InternalWarning(instance, record_obj.location,
                                "Forcing VkPhysicalDeviceVulkanMemoryModelFeatures::vulkanMemoryModel to VK_TRUE");
                mm_features->vulkanMemoryModel = VK_TRUE;
            }
            if (mm_features->vulkanMemoryModelDeviceScope == VK_FALSE) {
                InternalWarning(instance, record_obj.location,
                                "Forcing VkPhysicalDeviceVulkanMemoryModelFeatures::vulkanMemoryModelDeviceScope to VK_TRUE");
                mm_features->vulkanMemoryModelDeviceScope = VK_TRUE;
            }
        } else {
            InternalWarning(instance, record_obj.location,
                            "Adding a VkPhysicalDeviceVulkanMemoryModelFeatures to pNext with vulkanMemoryModel and "
                            "vulkanMemoryModelDeviceScope set to VK_TRUE");
            VkPhysicalDeviceVulkanMemoryModelFeatures new_mm_features = vku::InitStructHelper();
            new_mm_features.vulkanMemoryModel = VK_TRUE;
            new_mm_features.vulkanMemoryModelDeviceScope = VK_TRUE;
            vku::AddToPnext(*modified_create_info, new_mm_features);
        }
    };

    if (api_version > VK_API_VERSION_1_1) {
        if (auto *features12 = const_cast<VkPhysicalDeviceVulkan12Features *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(modified_create_info->pNext))) {
            if (features12->timelineSemaphore == VK_FALSE) {
                InternalWarning(instance, record_obj.location,
                                "Forcing VkPhysicalDeviceVulkan12Features::timelineSemaphore to VK_TRUE");
                features12->timelineSemaphore = VK_TRUE;
            }

            if (features12->vulkanMemoryModel == VK_FALSE) {
                InternalWarning(instance, record_obj.location,
                                "Forcing VkPhysicalDeviceVulkan12Features::vulkanMemoryModel to VK_TRUE");
                features12->vulkanMemoryModel = VK_TRUE;
            }
            if (features12->vulkanMemoryModelDeviceScope == VK_FALSE) {
                InternalWarning(instance, record_obj.location,
                                "Forcing VkPhysicalDeviceVulkan12Features::vulkanMemoryModelDeviceScope to VK_TRUE");
                features12->vulkanMemoryModelDeviceScope = VK_TRUE;
            }
        } else {
            add_missing_features();
        }
    } else if (api_version == VK_API_VERSION_1_1) {
        // Add our new extensions (will only add if found)
        const std::string_view ts_ext{"VK_KHR_timeline_semaphore"};
        vku::AddExtension(*modified_create_info, ts_ext.data());
        const std::string_view mm_ext{"VK_KHR_vulkan_memory_model"};
        vku::AddExtension(*modified_create_info, mm_ext.data());
        add_missing_features();
        timeline_khr_ = true;
    }

    // Force bufferDeviceAddress feature if available
    // ---
    auto add_bda_feature = [this, &record_obj, modified_create_info]() {
        // Add buffer device address feature
        if (auto *bda_features = const_cast<VkPhysicalDeviceBufferDeviceAddressFeatures *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceBufferDeviceAddressFeatures>(modified_create_info))) {
            if (!bda_features->bufferDeviceAddress) {
                InternalWarning(instance, record_obj.location,
                                "Forcing VkPhysicalDeviceBufferDeviceAddressFeatures::bufferDeviceAddress to VK_TRUE");
                bda_features->bufferDeviceAddress = VK_TRUE;
            }
        } else {
            InternalWarning(
                instance, record_obj.location,
                "Adding a VkPhysicalDeviceBufferDeviceAddressFeatures to pNext with bufferDeviceAddress set to VK_TRUE");
            VkPhysicalDeviceBufferDeviceAddressFeatures new_bda_features = vku::InitStructHelper();
            new_bda_features.bufferDeviceAddress = VK_TRUE;
            vku::AddToPnext(*modified_create_info, new_bda_features);
        }
    };

    if (api_version >= VK_API_VERSION_1_2) {
        if (auto *features12 = const_cast<VkPhysicalDeviceVulkan12Features *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(modified_create_info->pNext))) {
            if (!features12->bufferDeviceAddress) {
                InternalWarning(instance, record_obj.location,
                                "Forcing VkPhysicalDeviceVulkan12Features::bufferDeviceAddress to VK_TRUE");
                features12->bufferDeviceAddress = VK_TRUE;
            }
        } else {
            add_bda_feature();
        }
    } else if (IsExtensionAvailable(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, available_extensions)) {
        // Add our new extensions, only add if not found
        vku::AddExtension(*modified_create_info, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        add_bda_feature();
    }

    // Force uniformAndStorageBuffer8BitAccess feature if available and needed
    // ---
    if (gpuav_settings.validate_buffer_copies) {
        VkPhysicalDevice8BitStorageFeatures eight_bit_feature = vku::InitStructHelper();
        VkPhysicalDeviceFeatures2 features_2 = vku::InitStructHelper(&eight_bit_feature);
        DispatchGetPhysicalDeviceFeatures2(physicalDevice, &features_2);
        // uniformAndStorageBuffer8BitAccess is optional in 1.2. Only force on if available.
        if (eight_bit_feature.uniformAndStorageBuffer8BitAccess) {
            auto add_8bit_access_feature = [this, &record_obj, modified_create_info]() {
                // Add uniformAndStorageBuffer8BitAccess feature
                if (auto *eight_bit_access_feature = const_cast<VkPhysicalDevice8BitStorageFeatures *>(
                        vku::FindStructInPNextChain<VkPhysicalDevice8BitStorageFeatures>(modified_create_info))) {
                    if (!eight_bit_access_feature->uniformAndStorageBuffer8BitAccess) {
                        InternalWarning(
                            instance, record_obj.location,
                            "Forcing VkPhysicalDevice8BitStorageFeatures::uniformAndStorageBuffer8BitAccess to VK_TRUE");
                        eight_bit_access_feature->uniformAndStorageBuffer8BitAccess = VK_TRUE;
                    }
                } else {
                    InternalWarning(instance, record_obj.location,
                                    "Adding a VkPhysicalDevice8BitStorageFeatures to pNext with uniformAndStorageBuffer8BitAccess "
                                    "set to VK_TRUE");
                    VkPhysicalDevice8BitStorageFeatures new_bda_features = vku::InitStructHelper();
                    new_bda_features.uniformAndStorageBuffer8BitAccess = VK_TRUE;
                    vku::AddToPnext(*modified_create_info, new_bda_features);
                }
            };

            if (api_version >= VK_API_VERSION_1_2) {
                if (auto *features12 = const_cast<VkPhysicalDeviceVulkan12Features *>(
                        vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(modified_create_info->pNext))) {
                    if (!features12->uniformAndStorageBuffer8BitAccess) {
                        InternalWarning(instance, record_obj.location,
                                        "Forcing VkPhysicalDeviceVulkan12Features::uniformAndStorageBuffer8BitAccess to VK_TRUE");
                        features12->uniformAndStorageBuffer8BitAccess = VK_TRUE;
                    }
                } else {
                    add_8bit_access_feature();
                }
            } else if (IsExtensionAvailable(VK_KHR_8BIT_STORAGE_EXTENSION_NAME, available_extensions)) {
                // Add our new extensions, only if not found
                vku::AddExtension(*modified_create_info, VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
                add_8bit_access_feature();
            }
        }
    }

    // shaderInt64
    // ---
    VkPhysicalDeviceFeatures physical_device_features{};
    DispatchGetPhysicalDeviceFeatures(physicalDevice, &physical_device_features);
    if (physical_device_features.shaderInt64) {
        if (enabled_features) {
            enabled_features->shaderInt64 = VK_TRUE;
        } else if (auto *features = const_cast<VkPhysicalDeviceFeatures2 *>(
                       vku::FindStructInPNextChain<VkPhysicalDeviceFeatures2>(modified_create_info->pNext))) {
            features->features.shaderInt64 = VK_TRUE;
        } else {
            // Need to add a VkPhysicalDeviceFeatures pointer
            enabled_features = new VkPhysicalDeviceFeatures;
            memset(enabled_features, 0, sizeof(VkPhysicalDeviceFeatures));
            enabled_features->shaderInt64 = VK_TRUE;
            modified_create_info->pEnabledFeatures = enabled_features;
        }
    }
}

// Perform initializations that can be done at Create Device time.
void Validator::PostCreateDevice(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) {
    // GPU-AV not supported, exit early to prevent errors inside Validator::PostCallRecordCreateDevice
    if (api_version < VK_API_VERSION_1_1) {
        InternalError(device, loc, "GPU Shader Instrumentation requires Vulkan 1.1 or later.");
        return;
    }

    // Set up a stub implementation of the descriptor heap in case we abort.
    desc_heap_.emplace(*this, 0, loc);

    instrumentation_bindings_ = {
        // DebugPrintf Output buffer
        {glsl::kBindingInstDebugPrintf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Error output buffer
        {glsl::kBindingInstErrorBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Buffer holding output from GPU to do processing on the CPU
        {glsl::kBindingInstPostProcess, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Buffer holding input from CPU into the shader for descriptor indexing
        {glsl::kBindingInstDescriptorIndexingOOB, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Buffer holding buffer device addresses
        {glsl::kBindingInstBufferDeviceAddress, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Buffer holding action command index in command buffer
        {glsl::kBindingInstActionIndex, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Buffer holding a resource index from the per command buffer command resources list
        {glsl::kBindingInstCmdResourceIndex, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Commands errors counts buffer
        {glsl::kBindingInstCmdErrorsCount, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Vertex attribute fetch limits
        {glsl::kBindingInstVertexAttributeFetchLimits, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
    };

    // TODO - Now that GPU-AV and DebugPrintf are merged, we should just have a single PostCreateDevice if possible (or at least
    // better divide what belongs where as it is easy to mess)
    BaseClass::PostCreateDevice(pCreateInfo, loc);
    // We might fail in parent class device creation if global requirements are not met
    if (aborted_) return;

    // Need the device to be created before we can query features for settings
    InitSettings(loc);

    VkResult result = UtilInitializeVma(instance, physical_device, device, enabled_features.bufferDeviceAddress, &vma_allocator_);
    if (result != VK_SUCCESS) {
        InternalVmaError(device, loc, "Could not initialize VMA");
        return;
    }

    desc_set_manager_ =
        std::make_unique<vko::DescriptorSetManager>(device, static_cast<uint32_t>(instrumentation_bindings_.size()));

    // If api version 1.1 or later, SetDeviceLoaderData will be in the loader
    {
        auto chain_info = GetChainInfo(pCreateInfo, VK_LOADER_DATA_CALLBACK);
        assert(chain_info->u.pfnSetDeviceLoaderData);
        vk_set_device_loader_data_ = chain_info->u.pfnSetDeviceLoaderData;
    }

    if (gpuav_settings.shader_instrumentation.descriptor_checks) {
        VkPhysicalDeviceDescriptorIndexingProperties desc_indexing_props = vku::InitStructHelper();
        VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&desc_indexing_props);
        DispatchGetPhysicalDeviceProperties2Helper(api_version, physical_device, &props2);

        uint32_t num_descs = desc_indexing_props.maxUpdateAfterBindDescriptorsInAllPools;
        if (num_descs == 0 || num_descs > glsl::kDebugInputBindlessMaxDescriptors) {
            num_descs = glsl::kDebugInputBindlessMaxDescriptors;
        }

        desc_heap_.emplace(*this, num_descs, loc);
    }

    // Create error logging buffer allocation pool
    {
        VkBufferCreateInfo error_buffer_ci = vku::InitStructHelper();
        error_buffer_ci.size = glsl::kErrorBufferByteSize;
        error_buffer_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo error_buffer_alloc_ci = {};
        error_buffer_alloc_ci.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        error_buffer_alloc_ci.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        uint32_t mem_type_index;
        result = vmaFindMemoryTypeIndexForBufferInfo(vma_allocator_, &error_buffer_ci, &error_buffer_alloc_ci, &mem_type_index);
        if (result != VK_SUCCESS) {
            InternalVmaError(device, loc, "Unable to find memory type index.");
            return;
        }
        VmaPoolCreateInfo vma_pool_ci = {};
        vma_pool_ci.memoryTypeIndex = mem_type_index;
        vma_pool_ci.blockSize = 0;
        vma_pool_ci.maxBlockCount = 0;
        if (gpuav_settings.vma_linear_output) {
            vma_pool_ci.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
        }
        result = vmaCreatePool(vma_allocator_, &vma_pool_ci, &output_buffer_pool_);
        if (result != VK_SUCCESS) {
            InternalVmaError(device, loc, "Unable to create VMA memory pool.");
            return;
        }
    }

    // Create command indices buffer
    {
        indices_buffer_alignment_ = sizeof(uint32_t) * static_cast<uint32_t>(phys_dev_props.limits.minStorageBufferOffsetAlignment);
        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        buffer_info.size = cst::indices_count * indices_buffer_alignment_;
        VmaAllocationCreateInfo alloc_info = {};
        assert(output_buffer_pool_);
        alloc_info.pool = output_buffer_pool_;
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        alloc_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        const bool success = indices_buffer_.Create(loc, &buffer_info, &alloc_info);
        if (!success) {
            return;
        }

        auto indices_ptr = (uint32_t *)indices_buffer_.MapMemory(loc);

        for (uint32_t i = 0; i < buffer_info.size / sizeof(uint32_t); ++i) {
            indices_ptr[i] = i / (indices_buffer_alignment_ / sizeof(uint32_t));
        }

        indices_buffer_.UnmapMemory();
    }
}

// At this point extensions/features may have been turned on by us in PreCallRecord.
// Now that we have all the information, here is where we might disable GPU-AV settings that are missing requirements
void Validator::InitSettings(const Location &loc) {
    VkPhysicalDeviceFeatures supported_features{};
    DispatchGetPhysicalDeviceFeatures(physical_device, &supported_features);

    GpuAVSettings::ShaderInstrumentation &shader_instrumentation = gpuav_settings.shader_instrumentation;
    if (shader_instrumentation.descriptor_checks || shader_instrumentation.post_process_descriptor_index) {
        if (!enabled_features.bufferDeviceAddress) {
            shader_instrumentation.descriptor_checks = false;
            shader_instrumentation.post_process_descriptor_index = false;
            InternalWarning(device, loc,
                            "Descriptors Indexing Validation optin was enabled. but the bufferDeviceAddress was not supported "
                            "[Disabling gpuav_descriptor_checks and gpuav_post_process_descriptor_indexing]");
        }
    }

    if (shader_instrumentation.buffer_device_address) {
        const bool bda_validation_possible =
            ((IsExtEnabled(extensions.vk_ext_buffer_device_address) || IsExtEnabled(extensions.vk_khr_buffer_device_address)) &&
             enabled_features.shaderInt64 && enabled_features.bufferDeviceAddress);
        if (!bda_validation_possible) {
            shader_instrumentation.buffer_device_address = false;
            if (!enabled_features.shaderInt64) {
                InternalWarning(
                    device, loc,
                    "Buffer device address validation option was enabled, but the shaderInt64 feature is not supported. "
                    "[Disabling gpuav_buffer_address_oob].");
            } else {
                InternalWarning(device, loc,
                                "Buffer device address validation option was enabled, but required buffer device address extension "
                                "and/or features are not enabled. [Disabling gpuav_buffer_address_oob]");
            }
        }
    }

    if (shader_instrumentation.ray_query) {
        if (!enabled_features.rayQuery) {
            // TODO - Force on if possible, issue is we need to potentially enable all the dependency extensions
            shader_instrumentation.ray_query = false;
            InternalWarning(device, loc,
                            "Ray Query validation option was enabled, but the rayQuery feature is not enabled. "
                            "[Disabling gpuav_validate_ray_query]");
        }
    }

    // copy_buffer_to_image.comp relies on uint8_t buffers to perform validation
    if (gpuav_settings.validate_buffer_copies) {
        if (!enabled_features.uniformAndStorageBuffer8BitAccess) {
            gpuav_settings.validate_buffer_copies = false;
            InternalWarning(device, loc,
                            "Buffer copies option was enabled, but the uniformAndStorageBuffer8BitAccess feature is not supported. "
                            "[Disabling gpuav_buffer_copies]");
        }
    }

    if (IsExtEnabled(extensions.vk_ext_descriptor_buffer)) {
        InternalWarning(
            device, loc,
            "VK_EXT_descriptor_buffer is enabled, but GPU-AV does not currently support validation of descriptor buffers. "
            "[Disabling all shader instrumentation checks]");
        // Because of VUs like VUID-VkPipelineLayoutCreateInfo-pSetLayouts-08008 we currently would need to rework the entire shader
        // instrumentation logic
        gpuav_settings.DisableShaderInstrumentationAndOptions();
    }

    if (gpuav_settings.IsBufferValidationEnabled()) {
        if (phys_dev_props.limits.maxPushConstantsSize < 9 * sizeof(uint32_t)) {
            gpuav_settings.SetBufferValidationEnabled(false);
            InternalWarning(device, loc,
                            "Device does not support required minimum maxPushConstantsSize. Buffer validation needs at least 9 * "
                            "sizeof(uint32_t) bytes. No indirect buffer checking will be "
                            "attempted");
        } else if (!supported_features.shaderInt64) {
            InternalWarning(
                device, loc,
                "shaderInt64 feature not available, indirect trace rays validation and countBuffer (as seen in commands like "
                "vkCmdDrawIndexedIndirectCount) validation cannot be performed.");
        }
    }

    // If we have turned off all the possible things to instrument, turn off everything fully
    if (!gpuav_settings.IsShaderInstrumentationEnabled()) {
        gpuav_settings.DisableShaderInstrumentationAndOptions();
    }
}

void Validator::InternalVmaError(LogObjectList objlist, const Location &loc, const char *const specific_message) const {
    aborted_ = true;
    std::string error_message = specific_message;

    char *stats_string;
    vmaBuildStatsString(vma_allocator_, &stats_string, false);
    error_message += " VMA statistics = ";
    error_message += stats_string;
    vmaFreeStatsString(vma_allocator_, stats_string);

    char const *layer_name = gpuav_settings.debug_printf_only ? "DebugPrintf" : "GPU-AV";
    char const *vuid = gpuav_settings.debug_printf_only ? "UNASSIGNED-DEBUG-PRINTF" : "UNASSIGNED-GPU-Assisted-Validation";

    LogError(vuid, objlist, loc, "Internal VMA Error, %s is being disabled. Details:\n%s", layer_name, error_message.c_str());

    // Once we encounter an internal issue disconnect everything.
    // This prevents need to check "if (aborted)" (which is awful when we easily forget to check somewhere and the user gets spammed
    // with errors making it hard to see the first error with the real source of the problem).
    dispatch_device_->ReleaseValidationObject(LayerObjectTypeGpuAssisted);
}

VkDeviceAddress Validator::GetBufferDeviceAddressHelper(VkBuffer buffer) const {
    VkBufferDeviceAddressInfo address_info = vku::InitStructHelper();
    address_info.buffer = buffer;

    if (api_version >= VK_API_VERSION_1_2) {
        return DispatchGetBufferDeviceAddress(device, &address_info);
    } else {
        return DispatchGetBufferDeviceAddressKHR(device, &address_info);
    }
}

}  // namespace gpuav
