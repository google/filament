// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

// VkInstanceFuncs.h lists the vulkan driver per-instance functions.
//
// TODO: Generate this list.

// VK_INSTANCE(<function name>, <return type>, <arguments>...)
VK_INSTANCE(vkAllocateCommandBuffers, VkResult, VkDevice, const VkCommandBufferAllocateInfo *, VkCommandBuffer *);
VK_INSTANCE(vkAllocateDescriptorSets, VkResult, VkDevice, const VkDescriptorSetAllocateInfo *, VkDescriptorSet *);
VK_INSTANCE(vkAllocateMemory, VkResult, VkDevice, const VkMemoryAllocateInfo *, const VkAllocationCallbacks *,
            VkDeviceMemory *);
VK_INSTANCE(vkBeginCommandBuffer, VkResult, VkCommandBuffer, const VkCommandBufferBeginInfo *);
VK_INSTANCE(vkBindBufferMemory, VkResult, VkDevice, VkBuffer, VkDeviceMemory, VkDeviceSize);
VK_INSTANCE(vkCmdBindDescriptorSets, void, VkCommandBuffer, VkPipelineBindPoint, VkPipelineLayout, uint32_t, uint32_t,
            const VkDescriptorSet *, uint32_t, const uint32_t *);
VK_INSTANCE(vkCmdBindPipeline, void, VkCommandBuffer, VkPipelineBindPoint, VkPipeline);
VK_INSTANCE(vkCmdDispatch, void, VkCommandBuffer, uint32_t, uint32_t, uint32_t);
VK_INSTANCE(vkCreateBuffer, VkResult, VkDevice, const VkBufferCreateInfo *, const VkAllocationCallbacks *, VkBuffer *);
VK_INSTANCE(vkCreateCommandPool, VkResult, VkDevice, const VkCommandPoolCreateInfo *, const VkAllocationCallbacks *,
            VkCommandPool *);
VK_INSTANCE(vkCreateComputePipelines, VkResult, VkDevice, VkPipelineCache, uint32_t, const VkComputePipelineCreateInfo *,
            const VkAllocationCallbacks *, VkPipeline *);
VK_INSTANCE(vkCreateDescriptorPool, VkResult, VkDevice, const VkDescriptorPoolCreateInfo *, const VkAllocationCallbacks *,
            VkDescriptorPool *);
VK_INSTANCE(vkCreateDescriptorSetLayout, VkResult, VkDevice, const VkDescriptorSetLayoutCreateInfo *,
            const VkAllocationCallbacks *, VkDescriptorSetLayout *);
VK_INSTANCE(vkCreateDevice, VkResult, VkPhysicalDevice, const VkDeviceCreateInfo *, const VkAllocationCallbacks *,
            VkDevice *);
VK_INSTANCE(vkCreatePipelineLayout, VkResult, VkDevice, const VkPipelineLayoutCreateInfo *, const VkAllocationCallbacks *,
            VkPipelineLayout *);
VK_INSTANCE(vkCreateShaderModule, VkResult, VkDevice, const VkShaderModuleCreateInfo *, const VkAllocationCallbacks *,
            VkShaderModule *);
VK_INSTANCE(vkDestroyBuffer, void, VkDevice, VkBuffer, const VkAllocationCallbacks *);
VK_INSTANCE(vkDestroyCommandPool, void, VkDevice, VkCommandPool, const VkAllocationCallbacks *);
VK_INSTANCE(vkDestroyDescriptorPool, void, VkDevice, VkDescriptorPool, const VkAllocationCallbacks *);
VK_INSTANCE(vkDestroyDescriptorSetLayout, void, VkDevice, VkDescriptorSetLayout, const VkAllocationCallbacks *);
VK_INSTANCE(vkDestroyDevice, VkResult, VkDevice, const VkAllocationCallbacks *);
VK_INSTANCE(vkDestroyInstance, void, VkInstance, const VkAllocationCallbacks *);
VK_INSTANCE(vkDestroyPipeline, void, VkDevice, VkPipeline, const VkAllocationCallbacks *);
VK_INSTANCE(vkDestroyPipelineLayout, void, VkDevice, VkPipelineLayout, const VkAllocationCallbacks *);
VK_INSTANCE(vkDestroyShaderModule, void, VkDevice, VkShaderModule, const VkAllocationCallbacks *);
VK_INSTANCE(vkEndCommandBuffer, VkResult, VkCommandBuffer);
VK_INSTANCE(vkEnumeratePhysicalDevices, VkResult, VkInstance, uint32_t *, VkPhysicalDevice *);
VK_INSTANCE(vkFreeCommandBuffers, void, VkDevice, VkCommandPool, uint32_t, const VkCommandBuffer *);
VK_INSTANCE(vkFreeMemory, void, VkDevice, VkDeviceMemory, const VkAllocationCallbacks *);
VK_INSTANCE(vkGetDeviceQueue, void, VkDevice, uint32_t, uint32_t, VkQueue *);
VK_INSTANCE(vkGetPhysicalDeviceMemoryProperties, void, VkPhysicalDevice, VkPhysicalDeviceMemoryProperties *);
VK_INSTANCE(vkGetPhysicalDeviceProperties, void, VkPhysicalDevice, VkPhysicalDeviceProperties *);
VK_INSTANCE(vkGetPhysicalDeviceProperties2, void, VkPhysicalDevice, VkPhysicalDeviceProperties2 *);
VK_INSTANCE(vkGetPhysicalDeviceQueueFamilyProperties, void, VkPhysicalDevice, uint32_t *, VkQueueFamilyProperties *);
VK_INSTANCE(vkMapMemory, VkResult, VkDevice, VkDeviceMemory, VkDeviceSize, VkDeviceSize, VkMemoryMapFlags, void **);
VK_INSTANCE(vkQueueSubmit, VkResult, VkQueue, uint32_t, const VkSubmitInfo *, VkFence);
VK_INSTANCE(vkQueueWaitIdle, VkResult, VkQueue);
VK_INSTANCE(vkUnmapMemory, void, VkDevice, VkDeviceMemory);
VK_INSTANCE(vkUpdateDescriptorSets, void, VkDevice, uint32_t, const VkWriteDescriptorSet *, uint32_t,
            const VkCopyDescriptorSet *);
VK_INSTANCE(vkDeviceWaitIdle, VkResult, VkDevice);