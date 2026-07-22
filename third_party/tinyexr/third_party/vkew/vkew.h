/*
 * vkew - a tiny Vulkan External Wrapper (loader) for headless compute.
 *
 * Mirrors the cuew approach: at runtime it dlopen()s the Vulkan loader
 * (libvulkan.so.1 / vulkan-1.dll / libvulkan.dylib) and resolves the function
 * pointers through vkGetInstanceProcAddr, so NO Vulkan SDK is required at build
 * time and we link only -ldl. Only the subset of the Vulkan 1.0 API needed for
 * headless compute (instance/device/queue/buffers/descriptors/compute
 * pipelines/command buffers) is declared here.
 *
 * Type layouts follow the Vulkan ABI on the LP64 platforms TinyEXR targets
 * (all handles are opaque pointers; VkDeviceSize is 64-bit). The large
 * VkPhysicalDeviceProperties tail (limits/sparse) is treated as an opaque,
 * oversized blob: the driver writes the whole struct, we only read the header
 * fields (deviceName/deviceType), so the blob just needs to be big enough.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VKEW_H_
#define VKEW_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#  define VKAPI_PTR __stdcall
#else
#  define VKAPI_PTR
#endif

/* ---- base types --------------------------------------------------------- */
typedef uint32_t VkFlags;
typedef uint32_t VkBool32;
typedef uint64_t VkDeviceSize;
typedef uint32_t VkSampleMask;

#define VK_NULL_HANDLE 0
#define VK_TRUE 1u
#define VK_FALSE 0u
#define VK_WHOLE_SIZE (~0ULL)
#define VK_QUEUE_FAMILY_IGNORED (~0u)
#define VK_MAX_PHYSICAL_DEVICE_NAME_SIZE 256
#define VK_UUID_SIZE 16
#define VK_MAX_MEMORY_TYPES 32
#define VK_MAX_MEMORY_HEAPS 16

/* All handles are opaque pointers on the LP64 platforms we support. */
#define VKEW_HANDLE(name) typedef struct name##_T *name
VKEW_HANDLE(VkInstance);
VKEW_HANDLE(VkPhysicalDevice);
VKEW_HANDLE(VkDevice);
VKEW_HANDLE(VkQueue);
VKEW_HANDLE(VkCommandBuffer);
VKEW_HANDLE(VkDeviceMemory);
VKEW_HANDLE(VkBuffer);
VKEW_HANDLE(VkShaderModule);
VKEW_HANDLE(VkDescriptorSetLayout);
VKEW_HANDLE(VkPipelineLayout);
VKEW_HANDLE(VkPipeline);
VKEW_HANDLE(VkPipelineCache);
VKEW_HANDLE(VkDescriptorPool);
VKEW_HANDLE(VkDescriptorSet);
VKEW_HANDLE(VkCommandPool);
VKEW_HANDLE(VkFence);
#undef VKEW_HANDLE

/* ---- enums (only the values we use) ------------------------------------- */
typedef int VkResult;
#define VK_SUCCESS 0
#define VK_NOT_READY 1
#define VK_TIMEOUT 2
#define VK_INCOMPLETE 5

typedef uint32_t VkStructureType;
#define VK_STRUCTURE_TYPE_APPLICATION_INFO 0
#define VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO 1
#define VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO 2
#define VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO 3
#define VK_STRUCTURE_TYPE_SUBMIT_INFO 4
#define VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO 5
#define VK_STRUCTURE_TYPE_FENCE_CREATE_INFO 8
#define VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO 12
#define VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO 16
#define VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO 18
#define VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO 29
#define VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO 30
#define VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO 32
#define VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO 33
#define VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO 34
#define VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET 35
#define VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO 39
#define VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO 40
#define VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO 42

typedef uint32_t VkPhysicalDeviceType;
#define VK_PHYSICAL_DEVICE_TYPE_OTHER 0
#define VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU 1
#define VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU 2
#define VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU 3
#define VK_PHYSICAL_DEVICE_TYPE_CPU 4

/* VkBufferUsageFlagBits */
#define VK_BUFFER_USAGE_TRANSFER_SRC_BIT 0x00000001
#define VK_BUFFER_USAGE_TRANSFER_DST_BIT 0x00000002
#define VK_BUFFER_USAGE_STORAGE_BUFFER_BIT 0x00000020

/* VkMemoryPropertyFlagBits */
#define VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT 0x00000001
#define VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 0x00000002
#define VK_MEMORY_PROPERTY_HOST_COHERENT_BIT 0x00000004

/* VkQueueFlagBits */
#define VK_QUEUE_GRAPHICS_BIT 0x00000001
#define VK_QUEUE_COMPUTE_BIT 0x00000002

/* VkSharingMode */
#define VK_SHARING_MODE_EXCLUSIVE 0

/* VkDescriptorType */
#define VK_DESCRIPTOR_TYPE_STORAGE_BUFFER 7

/* VkShaderStageFlagBits */
#define VK_SHADER_STAGE_COMPUTE_BIT 0x00000020

/* VkPipelineBindPoint */
#define VK_PIPELINE_BIND_POINT_COMPUTE 1

/* VkCommandPoolCreateFlagBits */
#define VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT 0x00000002

/* VkCommandBufferLevel */
#define VK_COMMAND_BUFFER_LEVEL_PRIMARY 0

/* VkCommandBufferUsageFlagBits */
#define VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT 0x00000001

/* VkDescriptorPoolCreateFlagBits */
#define VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT 0x00000001

/* ---- structs ------------------------------------------------------------ */
typedef struct VkApplicationInfo {
    VkStructureType sType;
    const void *pNext;
    const char *pApplicationName;
    uint32_t applicationVersion;
    const char *pEngineName;
    uint32_t engineVersion;
    uint32_t apiVersion;
} VkApplicationInfo;

typedef struct VkInstanceCreateInfo {
    VkStructureType sType;
    const void *pNext;
    VkFlags flags;
    const VkApplicationInfo *pApplicationInfo;
    uint32_t enabledLayerCount;
    const char *const *ppEnabledLayerNames;
    uint32_t enabledExtensionCount;
    const char *const *ppEnabledExtensionNames;
} VkInstanceCreateInfo;

typedef struct VkDeviceQueueCreateInfo {
    VkStructureType sType;
    const void *pNext;
    VkFlags flags;
    uint32_t queueFamilyIndex;
    uint32_t queueCount;
    const float *pQueuePriorities;
} VkDeviceQueueCreateInfo;

typedef struct VkDeviceCreateInfo {
    VkStructureType sType;
    const void *pNext;
    VkFlags flags;
    uint32_t queueCreateInfoCount;
    const VkDeviceQueueCreateInfo *pQueueCreateInfos;
    uint32_t enabledLayerCount;
    const char *const *ppEnabledLayerNames;
    uint32_t enabledExtensionCount;
    const char *const *ppEnabledExtensionNames;
    const void *pEnabledFeatures;
} VkDeviceCreateInfo;

/* Header fields are exact; the limits/sparse tail is an oversized opaque blob
 * (the driver fills the whole struct; we only read deviceType/deviceName). */
typedef struct VkPhysicalDeviceProperties {
    uint32_t apiVersion;
    uint32_t driverVersion;
    uint32_t vendorID;
    uint32_t deviceID;
    VkPhysicalDeviceType deviceType;
    char deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
    uint8_t pipelineCacheUUID[VK_UUID_SIZE];
    uint8_t limits_and_sparse[1024]; /* >= sizeof(VkPhysicalDeviceLimits)+sparse */
} VkPhysicalDeviceProperties;

typedef struct VkMemoryType {
    VkFlags propertyFlags;
    uint32_t heapIndex;
} VkMemoryType;

typedef struct VkMemoryHeap {
    VkDeviceSize size;
    VkFlags flags;
} VkMemoryHeap;

typedef struct VkPhysicalDeviceMemoryProperties {
    uint32_t memoryTypeCount;
    VkMemoryType memoryTypes[VK_MAX_MEMORY_TYPES];
    uint32_t memoryHeapCount;
    VkMemoryHeap memoryHeaps[VK_MAX_MEMORY_HEAPS];
} VkPhysicalDeviceMemoryProperties;

typedef struct VkExtent3D {
    uint32_t width, height, depth;
} VkExtent3D;

typedef struct VkQueueFamilyProperties {
    VkFlags queueFlags;
    uint32_t queueCount;
    uint32_t timestampValidBits;
    VkExtent3D minImageTransferGranularity;
} VkQueueFamilyProperties;

typedef struct VkBufferCreateInfo {
    VkStructureType sType;
    const void *pNext;
    VkFlags flags;
    VkDeviceSize size;
    VkFlags usage;
    uint32_t sharingMode;
    uint32_t queueFamilyIndexCount;
    const uint32_t *pQueueFamilyIndices;
} VkBufferCreateInfo;

typedef struct VkMemoryRequirements {
    VkDeviceSize size;
    VkDeviceSize alignment;
    uint32_t memoryTypeBits;
} VkMemoryRequirements;

typedef struct VkMemoryAllocateInfo {
    VkStructureType sType;
    const void *pNext;
    VkDeviceSize allocationSize;
    uint32_t memoryTypeIndex;
} VkMemoryAllocateInfo;

typedef struct VkDescriptorSetLayoutBinding {
    uint32_t binding;
    uint32_t descriptorType;
    uint32_t descriptorCount;
    VkFlags stageFlags;
    const void *pImmutableSamplers;
} VkDescriptorSetLayoutBinding;

typedef struct VkDescriptorSetLayoutCreateInfo {
    VkStructureType sType;
    const void *pNext;
    VkFlags flags;
    uint32_t bindingCount;
    const VkDescriptorSetLayoutBinding *pBindings;
} VkDescriptorSetLayoutCreateInfo;

typedef struct VkPushConstantRange {
    VkFlags stageFlags;
    uint32_t offset;
    uint32_t size;
} VkPushConstantRange;

typedef struct VkPipelineLayoutCreateInfo {
    VkStructureType sType;
    const void *pNext;
    VkFlags flags;
    uint32_t setLayoutCount;
    const VkDescriptorSetLayout *pSetLayouts;
    uint32_t pushConstantRangeCount;
    const VkPushConstantRange *pPushConstantRanges;
} VkPipelineLayoutCreateInfo;

typedef struct VkShaderModuleCreateInfo {
    VkStructureType sType;
    const void *pNext;
    VkFlags flags;
    size_t codeSize;
    const uint32_t *pCode;
} VkShaderModuleCreateInfo;

typedef struct VkSpecializationInfo VkSpecializationInfo;
typedef struct VkPipelineShaderStageCreateInfo {
    VkStructureType sType;
    const void *pNext;
    VkFlags flags;
    VkFlags stage;
    VkShaderModule module;
    const char *pName;
    const VkSpecializationInfo *pSpecializationInfo;
} VkPipelineShaderStageCreateInfo;

typedef struct VkComputePipelineCreateInfo {
    VkStructureType sType;
    const void *pNext;
    VkFlags flags;
    VkPipelineShaderStageCreateInfo stage;
    VkPipelineLayout layout;
    VkPipeline basePipelineHandle;
    int32_t basePipelineIndex;
} VkComputePipelineCreateInfo;

typedef struct VkDescriptorPoolSize {
    uint32_t type;
    uint32_t descriptorCount;
} VkDescriptorPoolSize;

typedef struct VkDescriptorPoolCreateInfo {
    VkStructureType sType;
    const void *pNext;
    VkFlags flags;
    uint32_t maxSets;
    uint32_t poolSizeCount;
    const VkDescriptorPoolSize *pPoolSizes;
} VkDescriptorPoolCreateInfo;

typedef struct VkDescriptorSetAllocateInfo {
    VkStructureType sType;
    const void *pNext;
    VkDescriptorPool descriptorPool;
    uint32_t descriptorSetCount;
    const VkDescriptorSetLayout *pSetLayouts;
} VkDescriptorSetAllocateInfo;

typedef struct VkDescriptorBufferInfo {
    VkBuffer buffer;
    VkDeviceSize offset;
    VkDeviceSize range;
} VkDescriptorBufferInfo;

typedef struct VkWriteDescriptorSet {
    VkStructureType sType;
    const void *pNext;
    VkDescriptorSet dstSet;
    uint32_t dstBinding;
    uint32_t dstArrayElement;
    uint32_t descriptorCount;
    uint32_t descriptorType;
    const void *pImageInfo;
    const VkDescriptorBufferInfo *pBufferInfo;
    const void *pTexelBufferView;
} VkWriteDescriptorSet;

typedef struct VkCommandPoolCreateInfo {
    VkStructureType sType;
    const void *pNext;
    VkFlags flags;
    uint32_t queueFamilyIndex;
} VkCommandPoolCreateInfo;

typedef struct VkCommandBufferAllocateInfo {
    VkStructureType sType;
    const void *pNext;
    VkCommandPool commandPool;
    uint32_t level;
    uint32_t commandBufferCount;
} VkCommandBufferAllocateInfo;

typedef struct VkCommandBufferBeginInfo {
    VkStructureType sType;
    const void *pNext;
    VkFlags flags;
    const void *pInheritanceInfo;
} VkCommandBufferBeginInfo;

typedef struct VkSubmitInfo {
    VkStructureType sType;
    const void *pNext;
    uint32_t waitSemaphoreCount;
    const void *pWaitSemaphores;
    const VkFlags *pWaitDstStageMask;
    uint32_t commandBufferCount;
    const VkCommandBuffer *pCommandBuffers;
    uint32_t signalSemaphoreCount;
    const void *pSignalSemaphores;
} VkSubmitInfo;

/* ---- function pointer typedefs ------------------------------------------ */
typedef void(VKAPI_PTR *PFN_vkVoidFunction)(void);
typedef PFN_vkVoidFunction(VKAPI_PTR *PFN_vkGetInstanceProcAddr)(VkInstance,
                                                                 const char *);
typedef PFN_vkVoidFunction(VKAPI_PTR *PFN_vkGetDeviceProcAddr)(VkDevice,
                                                               const char *);
typedef VkResult(VKAPI_PTR *PFN_vkCreateInstance)(const VkInstanceCreateInfo *,
                                                  const void *, VkInstance *);
typedef void(VKAPI_PTR *PFN_vkDestroyInstance)(VkInstance, const void *);
typedef VkResult(VKAPI_PTR *PFN_vkEnumeratePhysicalDevices)(VkInstance,
                                                            uint32_t *,
                                                            VkPhysicalDevice *);
typedef void(VKAPI_PTR *PFN_vkGetPhysicalDeviceProperties)(
    VkPhysicalDevice, VkPhysicalDeviceProperties *);
typedef void(VKAPI_PTR *PFN_vkGetPhysicalDeviceMemoryProperties)(
    VkPhysicalDevice, VkPhysicalDeviceMemoryProperties *);
typedef void(VKAPI_PTR *PFN_vkGetPhysicalDeviceQueueFamilyProperties)(
    VkPhysicalDevice, uint32_t *, VkQueueFamilyProperties *);
typedef VkResult(VKAPI_PTR *PFN_vkCreateDevice)(VkPhysicalDevice,
                                                const VkDeviceCreateInfo *,
                                                const void *, VkDevice *);
typedef void(VKAPI_PTR *PFN_vkDestroyDevice)(VkDevice, const void *);
typedef void(VKAPI_PTR *PFN_vkGetDeviceQueue)(VkDevice, uint32_t, uint32_t,
                                              VkQueue *);
typedef VkResult(VKAPI_PTR *PFN_vkCreateBuffer)(VkDevice,
                                                const VkBufferCreateInfo *,
                                                const void *, VkBuffer *);
typedef void(VKAPI_PTR *PFN_vkDestroyBuffer)(VkDevice, VkBuffer, const void *);
typedef void(VKAPI_PTR *PFN_vkGetBufferMemoryRequirements)(
    VkDevice, VkBuffer, VkMemoryRequirements *);
typedef VkResult(VKAPI_PTR *PFN_vkAllocateMemory)(VkDevice,
                                                  const VkMemoryAllocateInfo *,
                                                  const void *, VkDeviceMemory *);
typedef void(VKAPI_PTR *PFN_vkFreeMemory)(VkDevice, VkDeviceMemory,
                                          const void *);
typedef VkResult(VKAPI_PTR *PFN_vkBindBufferMemory)(VkDevice, VkBuffer,
                                                    VkDeviceMemory, VkDeviceSize);
typedef VkResult(VKAPI_PTR *PFN_vkMapMemory)(VkDevice, VkDeviceMemory,
                                             VkDeviceSize, VkDeviceSize, VkFlags,
                                             void **);
typedef void(VKAPI_PTR *PFN_vkUnmapMemory)(VkDevice, VkDeviceMemory);
typedef VkResult(VKAPI_PTR *PFN_vkCreateShaderModule)(
    VkDevice, const VkShaderModuleCreateInfo *, const void *, VkShaderModule *);
typedef void(VKAPI_PTR *PFN_vkDestroyShaderModule)(VkDevice, VkShaderModule,
                                                   const void *);
typedef VkResult(VKAPI_PTR *PFN_vkCreateDescriptorSetLayout)(
    VkDevice, const VkDescriptorSetLayoutCreateInfo *, const void *,
    VkDescriptorSetLayout *);
typedef void(VKAPI_PTR *PFN_vkDestroyDescriptorSetLayout)(
    VkDevice, VkDescriptorSetLayout, const void *);
typedef VkResult(VKAPI_PTR *PFN_vkCreatePipelineLayout)(
    VkDevice, const VkPipelineLayoutCreateInfo *, const void *,
    VkPipelineLayout *);
typedef void(VKAPI_PTR *PFN_vkDestroyPipelineLayout)(VkDevice, VkPipelineLayout,
                                                     const void *);
typedef VkResult(VKAPI_PTR *PFN_vkCreateComputePipelines)(
    VkDevice, VkPipelineCache, uint32_t, const VkComputePipelineCreateInfo *,
    const void *, VkPipeline *);
typedef void(VKAPI_PTR *PFN_vkDestroyPipeline)(VkDevice, VkPipeline,
                                               const void *);
typedef VkResult(VKAPI_PTR *PFN_vkCreateDescriptorPool)(
    VkDevice, const VkDescriptorPoolCreateInfo *, const void *,
    VkDescriptorPool *);
typedef void(VKAPI_PTR *PFN_vkDestroyDescriptorPool)(VkDevice, VkDescriptorPool,
                                                     const void *);
typedef VkResult(VKAPI_PTR *PFN_vkResetDescriptorPool)(VkDevice, VkDescriptorPool,
                                                       VkFlags);
typedef VkResult(VKAPI_PTR *PFN_vkAllocateDescriptorSets)(
    VkDevice, const VkDescriptorSetAllocateInfo *, VkDescriptorSet *);
typedef void(VKAPI_PTR *PFN_vkUpdateDescriptorSets)(VkDevice, uint32_t,
                                                    const VkWriteDescriptorSet *,
                                                    uint32_t, const void *);
typedef VkResult(VKAPI_PTR *PFN_vkCreateCommandPool)(
    VkDevice, const VkCommandPoolCreateInfo *, const void *, VkCommandPool *);
typedef void(VKAPI_PTR *PFN_vkDestroyCommandPool)(VkDevice, VkCommandPool,
                                                  const void *);
typedef VkResult(VKAPI_PTR *PFN_vkResetCommandPool)(VkDevice, VkCommandPool,
                                                    VkFlags);
typedef VkResult(VKAPI_PTR *PFN_vkAllocateCommandBuffers)(
    VkDevice, const VkCommandBufferAllocateInfo *, VkCommandBuffer *);
typedef void(VKAPI_PTR *PFN_vkFreeCommandBuffers)(VkDevice, VkCommandPool,
                                                  uint32_t,
                                                  const VkCommandBuffer *);
typedef VkResult(VKAPI_PTR *PFN_vkBeginCommandBuffer)(
    VkCommandBuffer, const VkCommandBufferBeginInfo *);
typedef VkResult(VKAPI_PTR *PFN_vkEndCommandBuffer)(VkCommandBuffer);
typedef void(VKAPI_PTR *PFN_vkCmdBindPipeline)(VkCommandBuffer, uint32_t,
                                               VkPipeline);
typedef void(VKAPI_PTR *PFN_vkCmdBindDescriptorSets)(
    VkCommandBuffer, uint32_t, VkPipelineLayout, uint32_t, uint32_t,
    const VkDescriptorSet *, uint32_t, const uint32_t *);
typedef void(VKAPI_PTR *PFN_vkCmdPushConstants)(VkCommandBuffer, VkPipelineLayout,
                                                VkFlags, uint32_t, uint32_t,
                                                const void *);
typedef void(VKAPI_PTR *PFN_vkCmdDispatch)(VkCommandBuffer, uint32_t, uint32_t,
                                           uint32_t);
typedef VkResult(VKAPI_PTR *PFN_vkQueueSubmit)(VkQueue, uint32_t,
                                               const VkSubmitInfo *, VkFence);
typedef VkResult(VKAPI_PTR *PFN_vkQueueWaitIdle)(VkQueue);
typedef VkResult(VKAPI_PTR *PFN_vkDeviceWaitIdle)(VkDevice);

/* ---- resolved function pointers (extern; defined in vkew.c) ------------- */
extern PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
extern PFN_vkCreateInstance vkCreateInstance;
extern PFN_vkDestroyInstance vkDestroyInstance;
extern PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
extern PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
extern PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
extern PFN_vkGetPhysicalDeviceQueueFamilyProperties
    vkGetPhysicalDeviceQueueFamilyProperties;
extern PFN_vkCreateDevice vkCreateDevice;
extern PFN_vkDestroyDevice vkDestroyDevice;
extern PFN_vkGetDeviceQueue vkGetDeviceQueue;
extern PFN_vkCreateBuffer vkCreateBuffer;
extern PFN_vkDestroyBuffer vkDestroyBuffer;
extern PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
extern PFN_vkAllocateMemory vkAllocateMemory;
extern PFN_vkFreeMemory vkFreeMemory;
extern PFN_vkBindBufferMemory vkBindBufferMemory;
extern PFN_vkMapMemory vkMapMemory;
extern PFN_vkUnmapMemory vkUnmapMemory;
extern PFN_vkCreateShaderModule vkCreateShaderModule;
extern PFN_vkDestroyShaderModule vkDestroyShaderModule;
extern PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
extern PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
extern PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
extern PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
extern PFN_vkCreateComputePipelines vkCreateComputePipelines;
extern PFN_vkDestroyPipeline vkDestroyPipeline;
extern PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
extern PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
extern PFN_vkResetDescriptorPool vkResetDescriptorPool;
extern PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
extern PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
extern PFN_vkCreateCommandPool vkCreateCommandPool;
extern PFN_vkDestroyCommandPool vkDestroyCommandPool;
extern PFN_vkResetCommandPool vkResetCommandPool;
extern PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
extern PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
extern PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
extern PFN_vkEndCommandBuffer vkEndCommandBuffer;
extern PFN_vkCmdBindPipeline vkCmdBindPipeline;
extern PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
extern PFN_vkCmdPushConstants vkCmdPushConstants;
extern PFN_vkCmdDispatch vkCmdDispatch;
extern PFN_vkQueueSubmit vkQueueSubmit;
extern PFN_vkQueueWaitIdle vkQueueWaitIdle;
extern PFN_vkDeviceWaitIdle vkDeviceWaitIdle;

/* ---- loader ------------------------------------------------------------- */
enum {
    VKEW_SUCCESS = 0,
    VKEW_ERROR_OPEN_FAILED = -1, /* could not dlopen the Vulkan loader */
    VKEW_ERROR_INIT_FAILED = -2  /* vkGetInstanceProcAddr not found */
};

/* dlopen the Vulkan loader and resolve vkGetInstanceProcAddr + the global
 * entry points (vkCreateInstance). Returns VKEW_SUCCESS or an error code. */
int vkewInit(void);

/* Resolve all instance/device-level entry points against `instance` (call once
 * after vkCreateInstance succeeds). Returns VKEW_SUCCESS. */
int vkewLoadInstance(VkInstance instance);

#ifdef __cplusplus
}
#endif

#endif /* VKEW_H_ */
