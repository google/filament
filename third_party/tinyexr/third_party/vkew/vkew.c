/*
 * vkew - Vulkan loader implementation (dlopen + vkGetInstanceProcAddr).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "vkew.h"

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
static void *vkew_dlopen(const char *p) { return (void *)LoadLibraryA(p); }
static void *vkew_dlsym(void *h, const char *s) {
    return (void *)GetProcAddress((HMODULE)h, s);
}
#else
#  include <dlfcn.h>
static void *vkew_dlopen(const char *p) { return dlopen(p, RTLD_NOW | RTLD_LOCAL); }
static void *vkew_dlsym(void *h, const char *s) { return dlsym(h, s); }
#endif

/* ---- definitions of the extern function pointers ------------------------ */
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = 0;
PFN_vkCreateInstance vkCreateInstance = 0;
PFN_vkDestroyInstance vkDestroyInstance = 0;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = 0;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = 0;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties = 0;
PFN_vkGetPhysicalDeviceQueueFamilyProperties
    vkGetPhysicalDeviceQueueFamilyProperties = 0;
PFN_vkCreateDevice vkCreateDevice = 0;
PFN_vkDestroyDevice vkDestroyDevice = 0;
PFN_vkGetDeviceQueue vkGetDeviceQueue = 0;
PFN_vkCreateBuffer vkCreateBuffer = 0;
PFN_vkDestroyBuffer vkDestroyBuffer = 0;
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements = 0;
PFN_vkAllocateMemory vkAllocateMemory = 0;
PFN_vkFreeMemory vkFreeMemory = 0;
PFN_vkBindBufferMemory vkBindBufferMemory = 0;
PFN_vkMapMemory vkMapMemory = 0;
PFN_vkUnmapMemory vkUnmapMemory = 0;
PFN_vkCreateShaderModule vkCreateShaderModule = 0;
PFN_vkDestroyShaderModule vkDestroyShaderModule = 0;
PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout = 0;
PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout = 0;
PFN_vkCreatePipelineLayout vkCreatePipelineLayout = 0;
PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout = 0;
PFN_vkCreateComputePipelines vkCreateComputePipelines = 0;
PFN_vkDestroyPipeline vkDestroyPipeline = 0;
PFN_vkCreateDescriptorPool vkCreateDescriptorPool = 0;
PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool = 0;
PFN_vkResetDescriptorPool vkResetDescriptorPool = 0;
PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets = 0;
PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets = 0;
PFN_vkCreateCommandPool vkCreateCommandPool = 0;
PFN_vkDestroyCommandPool vkDestroyCommandPool = 0;
PFN_vkResetCommandPool vkResetCommandPool = 0;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = 0;
PFN_vkFreeCommandBuffers vkFreeCommandBuffers = 0;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer = 0;
PFN_vkEndCommandBuffer vkEndCommandBuffer = 0;
PFN_vkCmdBindPipeline vkCmdBindPipeline = 0;
PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets = 0;
PFN_vkCmdPushConstants vkCmdPushConstants = 0;
PFN_vkCmdDispatch vkCmdDispatch = 0;
PFN_vkQueueSubmit vkQueueSubmit = 0;
PFN_vkQueueWaitIdle vkQueueWaitIdle = 0;
PFN_vkDeviceWaitIdle vkDeviceWaitIdle = 0;

static void *g_lib = 0;

int vkewInit(void) {
    static const char *names[] = {
#if defined(_WIN32)
        "vulkan-1.dll",
#elif defined(__APPLE__)
        "libvulkan.dylib", "libvulkan.1.dylib", "libMoltenVK.dylib",
#else
        "libvulkan.so.1", "libvulkan.so",
#endif
        0};
    int i;
    if (vkGetInstanceProcAddr) return VKEW_SUCCESS; /* already initialised */
    for (i = 0; names[i]; ++i) {
        g_lib = vkew_dlopen(names[i]);
        if (g_lib) break;
    }
    if (!g_lib) return VKEW_ERROR_OPEN_FAILED;
    vkGetInstanceProcAddr =
        (PFN_vkGetInstanceProcAddr)vkew_dlsym(g_lib, "vkGetInstanceProcAddr");
    if (!vkGetInstanceProcAddr) return VKEW_ERROR_INIT_FAILED;
    /* global commands: instance == NULL */
    vkCreateInstance =
        (PFN_vkCreateInstance)vkGetInstanceProcAddr(0, "vkCreateInstance");
    if (!vkCreateInstance) return VKEW_ERROR_INIT_FAILED;
    return VKEW_SUCCESS;
}

#define VKEW_LOAD(inst, name) \
    name = (PFN_##name)vkGetInstanceProcAddr((inst), #name)

int vkewLoadInstance(VkInstance instance) {
    VKEW_LOAD(instance, vkDestroyInstance);
    VKEW_LOAD(instance, vkEnumeratePhysicalDevices);
    VKEW_LOAD(instance, vkGetPhysicalDeviceProperties);
    VKEW_LOAD(instance, vkGetPhysicalDeviceMemoryProperties);
    VKEW_LOAD(instance, vkGetPhysicalDeviceQueueFamilyProperties);
    VKEW_LOAD(instance, vkCreateDevice);
    VKEW_LOAD(instance, vkDestroyDevice);
    VKEW_LOAD(instance, vkGetDeviceQueue);
    VKEW_LOAD(instance, vkCreateBuffer);
    VKEW_LOAD(instance, vkDestroyBuffer);
    VKEW_LOAD(instance, vkGetBufferMemoryRequirements);
    VKEW_LOAD(instance, vkAllocateMemory);
    VKEW_LOAD(instance, vkFreeMemory);
    VKEW_LOAD(instance, vkBindBufferMemory);
    VKEW_LOAD(instance, vkMapMemory);
    VKEW_LOAD(instance, vkUnmapMemory);
    VKEW_LOAD(instance, vkCreateShaderModule);
    VKEW_LOAD(instance, vkDestroyShaderModule);
    VKEW_LOAD(instance, vkCreateDescriptorSetLayout);
    VKEW_LOAD(instance, vkDestroyDescriptorSetLayout);
    VKEW_LOAD(instance, vkCreatePipelineLayout);
    VKEW_LOAD(instance, vkDestroyPipelineLayout);
    VKEW_LOAD(instance, vkCreateComputePipelines);
    VKEW_LOAD(instance, vkDestroyPipeline);
    VKEW_LOAD(instance, vkCreateDescriptorPool);
    VKEW_LOAD(instance, vkDestroyDescriptorPool);
    VKEW_LOAD(instance, vkResetDescriptorPool);
    VKEW_LOAD(instance, vkAllocateDescriptorSets);
    VKEW_LOAD(instance, vkUpdateDescriptorSets);
    VKEW_LOAD(instance, vkCreateCommandPool);
    VKEW_LOAD(instance, vkDestroyCommandPool);
    VKEW_LOAD(instance, vkResetCommandPool);
    VKEW_LOAD(instance, vkAllocateCommandBuffers);
    VKEW_LOAD(instance, vkFreeCommandBuffers);
    VKEW_LOAD(instance, vkBeginCommandBuffer);
    VKEW_LOAD(instance, vkEndCommandBuffer);
    VKEW_LOAD(instance, vkCmdBindPipeline);
    VKEW_LOAD(instance, vkCmdBindDescriptorSets);
    VKEW_LOAD(instance, vkCmdPushConstants);
    VKEW_LOAD(instance, vkCmdDispatch);
    VKEW_LOAD(instance, vkQueueSubmit);
    VKEW_LOAD(instance, vkQueueWaitIdle);
    VKEW_LOAD(instance, vkDeviceWaitIdle);
    return VKEW_SUCCESS;
}
