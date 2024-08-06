#include <stdio.h>   // printf, fprintf
#include <stdlib.h>  // abort

#include "VizEngineAPIs.h"
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <Windows.h>

#define GLM_ENABLE_EXPERIMENTAL

#ifdef _WIN32
#include <vulkan/vulkan_win32.h>
#endif

#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to
// maximize ease of testing and compatibility with old VS compilers. To link
// with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project
// should not be affected, as you are likely to link with a newer binary of GLFW
// that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && \
    !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// #define APP_USE_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

// Data
static VkAllocationCallbacks* g_Allocator = nullptr;
static VkInstance g_Instance = VK_NULL_HANDLE;
static VkPhysicalDevice g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice g_Device = VK_NULL_HANDLE;
static uint32_t g_QueueFamily = (uint32_t)-1;
static VkQueue g_Queue = VK_NULL_HANDLE;
static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
static VkPipelineCache g_PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool g_DescriptorPool = VK_NULL_HANDLE;

static ImGui_ImplVulkanH_Window g_MainWindowData;
static int g_MinImageCount = 2;
static bool g_SwapChainRebuild = false;

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}
static void check_vk_result(VkResult err) {
  if (err == 0) return;
  fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
  if (err < 0) abort();
}

#ifdef APP_USE_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
             uint64_t object, size_t location, int32_t messageCode,
             const char* pLayerPrefix, const char* pMessage, void* pUserData) {
  (void)flags;
  (void)object;
  (void)location;
  (void)messageCode;
  (void)pUserData;
  (void)pLayerPrefix;  // Unused arguments
  fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n",
          objectType, pMessage);
  return VK_FALSE;
}
#endif  // APP_USE_VULKAN_DEBUG_REPORT

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties,
                        VkPhysicalDeviceMemoryProperties memProperties) {
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("Failed to find suitable memory type!");
}

void transitionImageLayout(VkDevice device, VkCommandPool commandPool,
                           VkQueue queue, VkImage image,
                           VkImageLayout oldLayout, VkImageLayout newLayout) {
  VkCommandBuffer commandBuffer;

  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(queue);

  vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format) {
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView imageView;
  if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
  }

  return imageView;
}

VkSampler createSampler(VkDevice device) {
  VkSamplerCreateInfo samplerInfo = {};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_FALSE;
  samplerInfo.maxAnisotropy = 1.0f;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = -1000;
  samplerInfo.maxLod = 1000;

  VkSampler sampler;
  if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture sampler!");
  }

  return sampler;
}

std::tuple<VkImage, VkDeviceMemory> importImageFromHandle(
    VkDevice currentDevice, VkPhysicalDevice physicalDevice, VkExtent2D extent,
    VkFormat format, void* handle) {
  bool const isDepth = false;

  VkImageUsageFlags const blittable =
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

  VkExternalMemoryImageCreateInfo externalImageCreateInfo = {};
  externalImageCreateInfo.sType =
      VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
#ifdef _WIN32
  externalImageCreateInfo.handleTypes =
      VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#elif __linux__
  externalImageCreateInfo.handleTypes =
      VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
  VkImageCreateInfo imageInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = &externalImageCreateInfo,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = format,
      .extent = {extent.width, extent.height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };

  VkImage image;
  VkResult result = vkCreateImage(currentDevice, &imageInfo, nullptr, &image);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Unable to create image");
  }

#ifdef _WIN32
  VkImportMemoryWin32HandleInfoKHR importMemoryInfo = {};
  importMemoryInfo.sType =
      VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR;
  importMemoryInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
  importMemoryInfo.handle = handle;
#elif __linux__
  VkImportMemoryFdInfoKHR importMemoryInfo = {};
  importMemoryInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR;
  importMemoryInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
  importMemoryInfo.fd = (int)handle;
#endif

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(currentDevice, image, &memRequirements);
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
  uint32_t memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memProperties);

  VkMemoryAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = &importMemoryInfo,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex = memoryTypeIndex,
  };

  VkDeviceMemory imageMemory;
  result = vkAllocateMemory(currentDevice, &allocInfo, nullptr, &imageMemory);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Unable to allocate image memory");
  }

  result = vkBindImageMemory(currentDevice, image, imageMemory, 0);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Unable to bind image memory");
  }

  return std::tuple(image, imageMemory);
}

static bool IsExtensionAvailable(
    const ImVector<VkExtensionProperties>& properties, const char* extension) {
  for (const VkExtensionProperties& p : properties)
    if (strcmp(p.extensionName, extension) == 0) return true;
  return false;
}

static VkPhysicalDevice SetupVulkan_SelectPhysicalDevice() {
  uint32_t gpu_count;
  VkResult err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, nullptr);
  check_vk_result(err);
  IM_ASSERT(gpu_count > 0);

  ImVector<VkPhysicalDevice> gpus;
  gpus.resize(gpu_count);
  err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, gpus.Data);
  check_vk_result(err);

  // If a number >1 of GPUs got reported, find discrete GPU if present, or use
  // first one available. This covers most common cases
  // (multi-gpu/integrated+dedicated graphics). Handling more complicated setups
  // (multiple dedicated GPUs) is out of scope of this sample.
  // for (VkPhysicalDevice& device : gpus) {
  //  VkPhysicalDeviceProperties properties;
  //  vkGetPhysicalDeviceProperties(device, &properties);
  //  if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
  //    return device;
  //}

  // Use first GPU (Integrated) is a Discrete one is not available.
  if (gpu_count > 0) return gpus[0];
  return VK_NULL_HANDLE;
}

static void SetupVulkan(ImVector<const char*> instance_extensions) {
  VkResult err;
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
  volkInitialize();
#endif

  // Create Vulkan Instance
  {
    uint32_t glfwExtensionCount = 0;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    // Enumerate available extensions
    uint32_t properties_count;
    ImVector<VkExtensionProperties> properties;
    vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
    properties.resize(properties_count);
    err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count,
                                                 properties.Data);
    check_vk_result(err);

    // Enable required extensions
    if (IsExtensionAvailable(
            properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
      instance_extensions.push_back(
          VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    if (IsExtensionAvailable(
            properties, VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME))
      instance_extensions.push_back(
          VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);

    if (IsExtensionAvailable(
            properties, VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME))
      instance_extensions.push_back(
          VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);

    if (IsExtensionAvailable(properties, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
      instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    if (IsExtensionAvailable(properties,
                             VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
      instance_extensions.push_back(
          VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
      create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

    // Enabling validation layers
#ifdef APP_USE_VULKAN_DEBUG_REPORT
    const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
    create_info.enabledLayerCount = 1;
    create_info.ppEnabledLayerNames = layers;
    instance_extensions.push_back("VK_EXT_debug_report");
#endif

    // Create Vulkan Instance
    create_info.enabledExtensionCount = (uint32_t)instance_extensions.Size;
    create_info.ppEnabledExtensionNames = instance_extensions.Data;
    err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
    check_vk_result(err);
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
    volkLoadInstance(g_Instance);
#endif

    // Setup the debug report callback
#ifdef APP_USE_VULKAN_DEBUG_REPORT
    auto f_vkCreateDebugReportCallbackEXT =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
            g_Instance, "vkCreateDebugReportCallbackEXT");
    IM_ASSERT(f_vkCreateDebugReportCallbackEXT != nullptr);
    VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
    debug_report_ci.sType =
        VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                            VK_DEBUG_REPORT_WARNING_BIT_EXT |
                            VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    debug_report_ci.pfnCallback = debug_report;
    debug_report_ci.pUserData = nullptr;
    err = f_vkCreateDebugReportCallbackEXT(g_Instance, &debug_report_ci,
                                           g_Allocator, &g_DebugReport);
    check_vk_result(err);
#endif
  }

  // Select Physical Device (GPU)
  g_PhysicalDevice = SetupVulkan_SelectPhysicalDevice();

  // Select graphics queue family
  {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, nullptr);
    VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(
        sizeof(VkQueueFamilyProperties) * count);
    vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, queues);
    for (uint32_t i = 0; i < count; i++)
      if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        g_QueueFamily = i;
        break;
      }
    free(queues);
    IM_ASSERT(g_QueueFamily != (uint32_t)-1);
  }

  // Create Logical Device (with 1 queue)
  {
    ImVector<const char*> device_extensions;
    device_extensions.push_back("VK_KHR_swapchain");
    device_extensions.push_back("VK_KHR_external_memory");
    device_extensions.push_back("VK_KHR_external_semaphore");
#ifdef _WIN32
    device_extensions.push_back("VK_KHR_external_memory_win32");
    device_extensions.push_back("VK_KHR_external_semaphore_win32");
#elif __linux__
    device_extensions.push_back("VK_KHR_external_memory_fd");
    device_extensions.push_back("VK_KHR_external_semaphore_fd");
#endif

    // Enumerate physical device extension
    uint32_t properties_count;
    ImVector<VkExtensionProperties> properties;
    vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr,
                                         &properties_count, nullptr);
    properties.resize(properties_count);
    vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr,
                                         &properties_count, properties.Data);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    if (IsExtensionAvailable(properties,
                             VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
      device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

    const float queue_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info[1] = {};
    queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex = g_QueueFamily;
    queue_info[0].queueCount = 1;
    queue_info[0].pQueuePriorities = queue_priority;
    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount =
        sizeof(queue_info) / sizeof(queue_info[0]);
    create_info.pQueueCreateInfos = queue_info;
    create_info.enabledExtensionCount = (uint32_t)device_extensions.Size;
    create_info.ppEnabledExtensionNames = device_extensions.Data;
    err =
        vkCreateDevice(g_PhysicalDevice, &create_info, g_Allocator, &g_Device);
    check_vk_result(err);
    vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
  }

  // Create Descriptor Pool
  // The example only requires a single combined image sampler descriptor for
  // the font image and only uses one descriptor set (for that) If you wish to
  // load e.g. additional textures you may need to alter pools sizes.
  {
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000}};

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    err = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator,
                                 &g_DescriptorPool);
    check_vk_result(err);
  }
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used
// by the demo. Your real engine/app may not use them.
static void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd,
                              VkSurfaceKHR surface, int width, int height) {
  wd->Surface = surface;

  // Check for WSI support
  VkBool32 res;
  vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily,
                                       wd->Surface, &res);
  if (res != VK_TRUE) {
    fprintf(stderr, "Error no WSI support on physical device 0\n");
    exit(-1);
  }

  // Select Surface Format
  const VkFormat requestSurfaceImageFormat[] = {
      VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
      VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
  const VkColorSpaceKHR requestSurfaceColorSpace =
      VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
      g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat,
      (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat),
      requestSurfaceColorSpace);

  // Select Present Mode
#ifdef APP_USE_UNLIMITED_FRAME_RATE
  VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR,
                                      VK_PRESENT_MODE_IMMEDIATE_KHR,
                                      VK_PRESENT_MODE_FIFO_KHR};
#else
  VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif
  wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
      g_PhysicalDevice, wd->Surface, &present_modes[0],
      IM_ARRAYSIZE(present_modes));
  // printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

  // Create SwapChain, RenderPass, Framebuffer, etc.
  IM_ASSERT(g_MinImageCount >= 2);
  ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device,
                                         wd, g_QueueFamily, g_Allocator, width,
                                         height, g_MinImageCount);
}

static void CleanupVulkan() {
  vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);

#ifdef APP_USE_VULKAN_DEBUG_REPORT
  // Remove the debug report callback
  auto f_vkDestroyDebugReportCallbackEXT =
      (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
          g_Instance, "vkDestroyDebugReportCallbackEXT");
  f_vkDestroyDebugReportCallbackEXT(g_Instance, g_DebugReport, g_Allocator);
#endif  // APP_USE_VULKAN_DEBUG_REPORT

  vkDestroyDevice(g_Device, g_Allocator);
  vkDestroyInstance(g_Instance, g_Allocator);
}

static void CleanupVulkanWindow() {
  ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, &g_MainWindowData,
                                  g_Allocator);
}

static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data) {
  VkResult err;

  VkSemaphore image_acquired_semaphore =
      wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
  VkSemaphore render_complete_semaphore =
      wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
  err = vkAcquireNextImageKHR(g_Device, wd->Swapchain, UINT64_MAX,
                              image_acquired_semaphore, VK_NULL_HANDLE,
                              &wd->FrameIndex);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    g_SwapChainRebuild = true;
    return;
  }
  check_vk_result(err);

  ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
  {
    err = vkWaitForFences(
        g_Device, 1, &fd->Fence, VK_TRUE,
        UINT64_MAX);  // wait indefinitely instead of periodically checking
    check_vk_result(err);

    err = vkResetFences(g_Device, 1, &fd->Fence);
    check_vk_result(err);
  }
  {
    err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
    check_vk_result(err);
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
    check_vk_result(err);
  }
  {
    VkRenderPassBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = wd->RenderPass;
    info.framebuffer = fd->Framebuffer;
    info.renderArea.extent.width = wd->Width;
    info.renderArea.extent.height = wd->Height;
    info.clearValueCount = 1;
    info.pClearValues = &wd->ClearValue;
    vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
  }

  // Record dear imgui primitives into command buffer
  ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

  // Submit command buffer
  vkCmdEndRenderPass(fd->CommandBuffer);
  {
    VkPipelineStageFlags wait_stage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &image_acquired_semaphore;
    info.pWaitDstStageMask = &wait_stage;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &fd->CommandBuffer;
    info.signalSemaphoreCount = 1;
    info.pSignalSemaphores = &render_complete_semaphore;

    err = vkEndCommandBuffer(fd->CommandBuffer);
    check_vk_result(err);
    err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
    check_vk_result(err);
  }
}

static void FramePresent(ImGui_ImplVulkanH_Window* wd) {
  if (g_SwapChainRebuild) return;
  VkSemaphore render_complete_semaphore =
      wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
  VkPresentInfoKHR info = {};
  info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  info.waitSemaphoreCount = 1;
  info.pWaitSemaphores = &render_complete_semaphore;
  info.swapchainCount = 1;
  info.pSwapchains = &wd->Swapchain;
  info.pImageIndices = &wd->FrameIndex;
  VkResult err = vkQueuePresentKHR(g_Queue, &info);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    g_SwapChainRebuild = true;
    return;
  }
  check_vk_result(err);
  wd->SemaphoreIndex =
      (wd->SemaphoreIndex + 1) %
      wd->SemaphoreCount;  // Now we can use the next set of semaphores
}

vzm::VzCamera::Controller* g_cc = nullptr;
vzm::VzRenderer* g_renderer;
vzm::VzScene* g_scene;

vzm::VzCamera* g_cam;
vzm::VzCamera* current_cam;
int current_cam_idx = 0;
std::vector<VID> cameras;

vzm::VzAsset* g_asset;
const int left_editUIWidth = 400;
const int right_editUIWidth = 400;
// workspace 공간의 크기
int workspace_width = 0;
int workspace_height = 0;
// 스왑버퍼(렌더타겟) 크기
int render_width = 1920;
int render_height = 1080;

VID currentVID = -1;

void resize(int width, int height) {
  g_cc->SetViewport(width, height);

  g_renderer->SetCanvas(width, height, 96.f, nullptr);
  float zNearP, zFarP, fovInDegree;
  current_cam->GetPerspectiveProjection(&zNearP, &zFarP, &fovInDegree, nullptr);
  current_cam->SetPerspectiveProjection(zNearP, zFarP, fovInDegree,
                                        (float)width / (float)height);
}

void setMouseButton(GLFWwindow* window, int button, int state,
                    int modifier_key) {
  if (!g_cc) {
    return;
  }
  if (button == 0) {
    double x;
    double y;
    int width;
    int height;

    glfwGetWindowSize(window, &width, &height);
    glfwGetCursorPos(window, &x, &y);

    int xPos = static_cast<int>(x - left_editUIWidth);
    int yPos = height - static_cast<int>(y);
    switch (state) {
      case GLFW_PRESS:
        if (x > left_editUIWidth && x < left_editUIWidth + workspace_width &&
            y < workspace_height) {
          g_cc->GrabBegin(xPos, yPos, false);
        }
        break;
      case GLFW_RELEASE:
        g_cc->GrabEnd();
        break;
      default:
        break;
    }
  }
}
void setCursorPos(GLFWwindow* window, double x, double y) {
  int state;
  int width;
  int height;

  if (!g_cc) {
    return;
  }
  state = glfwGetMouseButton(window, 0);
  glfwGetWindowSize(window, &width, &height);

  int xPos = static_cast<int>(x - left_editUIWidth);
  int yPos = height - static_cast<int>(y);

  if (state == GLFW_PRESS) {
    g_cc->GrabDrag(xPos, yPos);
  }
}
void setMouseScroll(GLFWwindow* window, double xOffset, double yOffset) {
  if (!g_cc) {
    return;
  }
  double x;
  double y;
  int width;
  int height;

  glfwGetWindowSize(window, &width, &height);
  glfwGetCursorPos(window, &x, &y);

  if (x > left_editUIWidth && x < left_editUIWidth + workspace_width &&
      y < workspace_height) {
    g_cc->Scroll(static_cast<int>(x - left_editUIWidth),
                 height - static_cast<int>(y), 5.0f * (float)yOffset);
  }
}
void onFrameBufferResize(GLFWwindow* window, int width, int height) {
  if (g_cc && g_renderer && current_cam) {
    workspace_width = width - left_editUIWidth - right_editUIWidth;
    workspace_height = height;

    resize(render_width, render_height);
  }
}

void treeNode(VID id) {
  vzm::VzSceneComp* component = (vzm::VzSceneComp*)vzm::GetVzComponent(id);
  std::string sName = component->GetName();

  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
  if (component->GetChildren().size() == 0) {
    flags |= ImGuiTreeNodeFlags_Leaf;
  }
  const char* name = sName.length() > 0 ? sName.c_str() : "Node";
  if (ImGui::TreeNodeEx((const void*)id, flags, "%s", name)) {
    if (ImGui::IsItemClicked()) {
      currentVID = id;
    }
    std::vector<VID> children = component->GetChildren();
    for (auto ce : children) {
      treeNode(ce);
    }
    ImGui::TreePop();
  } else {
    if (ImGui::IsItemClicked()) {
      currentVID = id;
    }
  }
};

std::wstring OpenFileDialog() {
  OPENFILENAME ofn;
  wchar_t szFile[260] = {0};
  HWND hwnd = NULL;

  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = nullptr;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = L"All\0*.*\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  wchar_t originCurrentDirectory[260];
  GetCurrentDirectory(260, originCurrentDirectory);

  if (GetOpenFileName(&ofn) == TRUE) {
    SetCurrentDirectory(originCurrentDirectory);
    return std::wstring(ofn.lpstrFile);
  }

  return L"";
}

void initViewer() {
  g_scene = vzm::NewScene("my scene");
  g_scene->LoadIBL("../../../VisualStudio/samples/assets/ibl/lightroom_14b");
  // g_scene->LoadIBL("lightroom_14b");
  g_cam = (vzm::VzCamera*)vzm::NewSceneComponent(
      vzm::SCENE_COMPONENT_TYPE::CAMERA, "UserCamera");
  glm::fvec3 p(0, 0, 10);
  glm::fvec3 at(0, 0, -4);
  glm::fvec3 u(0, 1, 0);
  g_cam->SetWorldPose((float*)&p, (float*)&at, (float*)&u);
  g_cam->SetPerspectiveProjection(0.1f, 1000.f, 45.f,
                                  (float)render_width / render_height);
  g_cc = g_cam->GetController();
  *(glm::fvec3*)g_cc->orbitHomePosition = p;
  g_cc->UpdateControllerSettings();
  g_cc->SetViewport(render_width, render_height);

  // vzm::VzLight* g_light = (vzm::VzLight*)vzm::NewSceneComponent(
  //     vzm::SCENE_COMPONENT_TYPE::LIGHT, "sunlight");
  // vzm::AppendSceneCompTo(light, scene);
  vzm::AppendSceneCompTo(g_cam, g_scene);
  current_cam = g_cam;

  cameras.clear();
}
void deinitViewer() {
  vzm::RemoveComponent(g_cam->GetVID());
  vzm::RemoveComponent(g_scene->GetVID());
}
int main(int, char**) {
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) return 1;

  // Create window with Vulkan context
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow* window =
      glfwCreateWindow(1280, 720, "Grapicar Filament Viewer", nullptr, nullptr);
  if (!glfwVulkanSupported()) {
    printf("GLFW: Vulkan Not Supported\n");
    return 1;
  }

  ImVector<const char*> extensions;
  uint32_t extensions_count = 0;
  const char** glfw_extensions =
      glfwGetRequiredInstanceExtensions(&extensions_count);
  for (uint32_t i = 0; i < extensions_count; i++)
    extensions.push_back(glfw_extensions[i]);
  SetupVulkan(extensions);

  // Create Window Surface
  VkSurfaceKHR surface;
  VkResult err =
      glfwCreateWindowSurface(g_Instance, window, g_Allocator, &surface);
  check_vk_result(err);

  glfwSetMouseButtonCallback(window, setMouseButton);
  glfwSetCursorPosCallback(window, setCursorPos);
  glfwSetScrollCallback(window, setMouseScroll);
  glfwSetFramebufferSizeCallback(window, onFrameBufferResize);

  // Create Framebuffers
  int w, h;
  glfwGetFramebufferSize(window, &w, &h);
  ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
  SetupVulkanWindow(wd, surface, w, h);

  vzm::ParamMap<std::string> arguments;
  arguments.SetParam("api", std::string("vulkan"));
  arguments.SetParam("vulkan-gpu-hint", std::string("0"));
  if (VZ_OK != vzm::InitEngineLib(arguments)) {
    std::cerr << "Failed to initialize engine library." << std::endl;
    return -1;
  }

  workspace_width = w - left_editUIWidth - right_editUIWidth;
  workspace_height = h;

  //
  g_renderer = vzm::NewRenderer("my renderer");
  g_renderer->SetCanvas(render_width, render_height, 96.f, nullptr);
  g_renderer->SetVisibleLayerMask(0x4, 0x4);
  initViewer();

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForVulkan(window, true);
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = g_Instance;
  init_info.PhysicalDevice = g_PhysicalDevice;
  init_info.Device = g_Device;
  init_info.QueueFamily = g_QueueFamily;
  init_info.Queue = g_Queue;
  init_info.PipelineCache = g_PipelineCache;
  init_info.DescriptorPool = g_DescriptorPool;
  init_info.RenderPass = wd->RenderPass;
  init_info.Subpass = 0;
  init_info.MinImageCount = g_MinImageCount;
  init_info.ImageCount = wd->ImageCount;
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = g_Allocator;
  init_info.CheckVkResultFn = check_vk_result;
  ImGui_ImplVulkan_Init(&init_info);

  // Our state
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  /*external memory handle로 image, imageview, DescriptorSet 생성*/
  VkSampler sampler = createSampler(g_Device);

  VkDescriptorSet swapTextures[2] = {
      0,
  };
  VkImage swapImages[2] = {
      0,
  };
  VkImageView swapImageViews[2] = {
      0,
  };
  VkDeviceMemory swapMemories[2] = {
      0,
  };
  void* swapHandles[2] = {
      INVALID_HANDLE_VALUE,
  };

  float any = 0.0f;
  int iAny = 0;
  bool bAny = 0.0f;
  float anyVec[3] = {
      0,
  };

  int tabIdx = 0;

  std::vector<bool> animActiveVec;
  float currentAnimPlayTime = 0.0f;
  float currentAnimTotalTime = 0.0f;
  bool isPlay = false;
  clock_t prevTime = clock();
  clock_t currentTime = clock();
  // Main loop
  while (!glfwWindowShouldClose(window)) {
    g_renderer->Render(g_scene, current_cam);
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
    // tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
    // your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
    // data to your main application, or clear/overwrite your copy of the
    // keyboard data. Generally you may always pass all inputs to dear imgui,
    // and hide them from your application based on those two flags.
    glfwPollEvents();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (g_SwapChainRebuild) {
      if (width > 0 && height > 0) {
        ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
        ImGui_ImplVulkanH_CreateOrResizeWindow(
            g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData,
            g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
        g_MainWindowData.FrameIndex = 0;
        g_SwapChainRebuild = false;
      }
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // left ui
    {
      ImGui::GetStyle().WindowRounding = 0;
      ImGui::SetNextWindowPos(ImVec2(0, 0));

      ImGui::SetNextWindowSize(ImVec2(left_editUIWidth, height),
                               ImGuiCond_Once);
      ImGui::SetNextWindowSizeConstraints(ImVec2(left_editUIWidth, height),
                                          ImVec2(left_editUIWidth, height));

      ImGui::Begin("left-ui", nullptr, ImGuiWindowFlags_NoTitleBar);

      if (ImGui::CollapsingHeader(
              "Resolution",
              ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        int width = render_width;
        int height = render_height;
        ImGui::Text("Type number and press Enter key.");
        if (ImGui::InputInt("width", &width, 1, 100,
                            ImGuiInputTextFlags_EnterReturnsTrue)) {
          if (width > 0 && width < 10000) {
            render_width = width;
            resize(render_width, render_height);
          } else {
            width = render_width;
          }
        }
        if (ImGui::InputInt("height", &height, 1, 100,
                            ImGuiInputTextFlags_EnterReturnsTrue)) {
          if (height > 0 && height < 10000) {
            render_height = height;
            resize(render_width, render_height);
          } else {
            height = render_height;
          }
        }
        ImGui::Unindent();
      }

      if (ImGui::CollapsingHeader(
              "Camera", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen)) {
        if (cameras.size() > 0) {
          float zNearP, zFarP, fovInDegree;
          current_cam->GetPerspectiveProjection(&zNearP, &zFarP, &fovInDegree,
                                                nullptr);
          vzm::VzBaseComp* comp = vzm::GetVzComponent(cameras[current_cam_idx]);
          std::string previewName = comp->GetName();
          if (ImGui::BeginCombo("Current Camera", previewName.c_str())) {
            for (int n = 0; n < cameras.size(); n++) {
              bool is_selected = (current_cam_idx == n);
              std::string camName = vzm::GetVzComponent(cameras[n])->GetName();
              if (ImGui::Selectable(camName.c_str(), is_selected)) {
                current_cam_idx = n;
                current_cam = (vzm::VzCamera*)vzm::GetVzComponent(cameras[n]);
              }
              if (is_selected) {
                ImGui::SetItemDefaultFocus();
              }
            }
            ImGui::EndCombo();
          }
          if (ImGui::InputFloat("near", &zNearP)) {
            current_cam->SetPerspectiveProjection(zNearP, zFarP, fovInDegree,
                                                  (float)width / (float)height);
          }
          if (ImGui::InputFloat("far", &zFarP)) {
            current_cam->SetPerspectiveProjection(zNearP, zFarP, fovInDegree,
                                                  (float)width / (float)height);
          }
          if (ImGui::InputFloat("fov", &fovInDegree)) {
            current_cam->SetPerspectiveProjection(zNearP, zFarP, fovInDegree,
                                                  (float)width / (float)height);
          }
        }
      }

      if (ImGui::CollapsingHeader(
              "Hierarchy",
              ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        // std::vector<VID> scene_children = scene->GetSceneCompChildren();
        // for (int i = 0; i < scene_children.size(); i++) {
        //   treeNode(scene_children[i]);
        // }
        if (g_asset) {
          std::vector<VID> root_vids = g_asset->GetGLTFRoots();
          for (int i = 0; i < root_vids.size(); i++) {
            treeNode(root_vids[i]);
          }
        }
        ImGui::Unindent();
      }

      if (ImGui::CollapsingHeader(
              "Animation",
              ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        currentTime = clock();
        if (g_asset) {
          vzm::VzAsset::Animator* animator = g_asset->GetAnimator();
          const size_t animationCount = animator->GetAnimationCount();
          // int inputAnimIdx = currentAnimIdx;

          // delta time 만큼 시간 진행
          if (isPlay) {
            currentAnimPlayTime += (currentTime - prevTime) / 1000.0f;
            if (currentAnimPlayTime > currentAnimTotalTime) {
              currentAnimPlayTime -= currentAnimTotalTime;
            }

            for (int i = 0; i < animationCount; i++) {
              if (animActiveVec[i]) {
                animator->ApplyAnimationTimeAt(i, currentAnimPlayTime);
              }
            }
            animator->UpdateBoneMatrices();
          }

          // UI
          if (ImGui::Button("Play / Pause")) {
            isPlay = !isPlay;
          }

          if (ImGui::Button("Stop")) {
            isPlay = false;
            currentAnimPlayTime = 0.0f;

            for (int i = 0; i < animationCount; i++) {
              if (animActiveVec[i]) {
                animator->ApplyAnimationTimeAt(i, currentAnimPlayTime);
              }
            }
          }

          if (ImGui::SliderFloat("Time", &currentAnimPlayTime, 0.0f,
                                 currentAnimTotalTime, "%4.2f seconds",
                                 ImGuiSliderFlags_AlwaysClamp)) {
            for (int i = 0; i < animationCount; i++) {
              if (animActiveVec[i]) {
                animator->ApplyAnimationTimeAt(i, currentAnimPlayTime);
              }
            }
            animator->UpdateBoneMatrices();
          }

          if (ImGui::Button("Select All")) {
            currentAnimTotalTime = 0.0f;
            for (int i = 0; i < animationCount; i++) {
              animActiveVec[i] = true;
              currentAnimTotalTime = std::max(
                  currentAnimTotalTime, animator->GetAnimationPlayTime(i));
            }
          }
          if (ImGui::Button("Deselect All")) {
            currentAnimTotalTime = 0.0f;
            for (int i = 0; i < animationCount; i++) {
              animator->ApplyAnimationTimeAt(i, 0.0f);
              animActiveVec[i] = false;
            }
          }
          for (size_t i = 0; i < animationCount; ++i) {
            std::string label = " " + animator->GetAnimationLabel(i);
            if (label.empty()) {
              label = "Unnamed " + std::to_string(i);
            }
            bool isActive = animActiveVec[i];
            if (ImGui::Checkbox(label.c_str(), &isActive)) {
              animActiveVec[i] = isActive;

              for (int j = 0; j < animationCount; j++) {
                if (animActiveVec[j]) {
                  animator->ApplyAnimationTimeAt(j, 0.0f);
                }
              }
              if (animActiveVec[i]) {
                animator->ActivateAnimation(i);
              } else {
                animator->ApplyAnimationTimeAt(i, 0.0f);
                animator->DeactivateAnimation(i);
              }
              currentAnimPlayTime = 0.0f;
              currentAnimTotalTime = 0.0f;
              for (int j = 0; j < animationCount; j++) {
                if (animActiveVec[j]) {
                  currentAnimTotalTime = std::max(
                      currentAnimTotalTime, animator->GetAnimationPlayTime(j));
                }
              }
            }
          }
        }
        prevTime = currentTime;
        ImGui::Unindent();
      }

      // left_editUIWidth = ImGui::GetWindowWidth();
      ImGui::End();
    }

    // swap buffer image
    {
      void* swapHandle = vzm::GetGraphicsSharedRenderTarget();
      if (swapHandle != INVALID_HANDLE_VALUE) {
        int index = 0;
        if (swapHandles[0] != swapHandle && swapHandles[1] != swapHandle) {
          if (swapHandles[0] != INVALID_HANDLE_VALUE &&
              swapHandles[1] != INVALID_HANDLE_VALUE) {
            swapHandles[0] = INVALID_HANDLE_VALUE;
            swapHandles[1] = INVALID_HANDLE_VALUE;
          }
          if (swapHandles[0] == INVALID_HANDLE_VALUE)
            index = 0;
          else
            index = 1;

          auto [importedImage, importedMemory] = importImageFromHandle(
              g_Device, g_PhysicalDevice,
              {(uint32_t)render_width, (uint32_t)render_height},
              VK_FORMAT_R8G8B8A8_UNORM, swapHandle);
          VkImageView importedImageView = createImageView(
              g_Device, importedImage, VK_FORMAT_R8G8B8A8_UNORM);

          swapHandles[index] = swapHandle;
          swapTextures[index] = ImGui_ImplVulkan_AddTexture(
              sampler, importedImageView,
              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          swapImages[index] = importedImage;
          swapImageViews[index] = importedImageView;
          swapMemories[index] = importedMemory;

        } else {
          if (swapHandles[0] == swapHandle)
            index = 0;
          else if (swapHandles[1] == swapHandle)
            index = 1;
        }

        ImGui::GetStyle().WindowRounding = 0;
        ImGui::GetStyle().WindowPadding = ImVec2(0, 0);

        ImGui::SetNextWindowPos(ImVec2(left_editUIWidth, 0));

        const int window_width = (int)ImGui::GetIO().DisplaySize.x;
        const int window_height = (int)ImGui::GetIO().DisplaySize.y;
        workspace_width = window_width - left_editUIWidth - right_editUIWidth;
        workspace_height = window_height;

        ImGui::SetNextWindowSize(ImVec2(workspace_width, workspace_height),
                                 ImGuiCond_Once);
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(workspace_width, workspace_height),
            ImVec2(workspace_width, workspace_height));
        ImGui::Begin("swapchain", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);

        float image_width = 0.0f;
        float image_height = 0.0f;
        if ((float)render_width / render_height >
            (float)workspace_width / workspace_height) {
          image_width = (float)workspace_width;
          image_height = (float)image_width * render_height / render_width;
        } else {
          image_height = (float)workspace_height;
          image_width = (float)image_height * render_width / render_height;
        }
        ImGui::Image((ImTextureID)swapTextures[index],
                     ImVec2(image_width, image_height));
        ImGui::End();
        // ImGui::GetBackgroundDrawList()->AddImage((ImTextureID)swapTextures[index],
        //                                          ImVec2(0, 0),
        //                                          ImVec2(width, height));
        transitionImageLayout(g_Device, wd->Frames[wd->FrameIndex].CommandPool,
                              g_Queue, swapImages[index],
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // Rendering
      }
    }

    // right ui
    {
      ImGui::GetStyle().WindowRounding = 0;
      ImGui::SetNextWindowPos(ImVec2(width - right_editUIWidth, 0));

      // const float width = ImGui::GetIO().DisplaySize.x;
      // const float height = ImGui::GetIO().DisplaySize.y;

      ImGui::SetNextWindowSize(ImVec2(right_editUIWidth, height),
                               ImGuiCond_Once);
      ImGui::SetNextWindowSizeConstraints(ImVec2(right_editUIWidth, height),
                                          ImVec2(right_editUIWidth, height));

      ImGui::Begin("right-ui", nullptr, ImGuiWindowFlags_NoTitleBar);

      const ImVec4 yellow(1.0f, 1.0f, 0.0f, 1.0f);
      {
        ImGui::BeginTabBar("tab");

        if (ImGui::BeginTabItem("Node")) {
          tabIdx = 0;
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Settings")) {
          tabIdx = 1;
          ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
      }
      switch (tabIdx) {
        case 0: {
          if (currentVID == -1) {
            break;
          }
          vzm::VzSceneComp* component =
              (vzm::VzSceneComp*)vzm::GetVzComponent(currentVID);
          vzm::SCENE_COMPONENT_TYPE type = component->GetSceneCompType();
          ImGui::Text(component->GetName().c_str());
          if (ImGui::CollapsingHeader(
                  "Transform",
                  ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent();
            float position[3];
            float rotation[3];
            float scale[3];
            component->GetPosition(position);
            component->GetRotation(rotation);
            rotation[0] = glm::degrees(rotation[0]);
            rotation[1] = glm::degrees(rotation[1]);
            rotation[2] = glm::degrees(rotation[2]);
            component->GetScale(scale);

            if (ImGui::InputFloat3("Position", position)) {
              component->SetPosition(position);
            }
            if (ImGui::InputFloat3("Rotation", rotation)) {
              rotation[0] = glm::radians(rotation[0]);
              rotation[1] = glm::radians(rotation[1]);
              rotation[2] = glm::radians(rotation[2]);
              component->SetRotation(rotation);
            }
            if (ImGui::InputFloat3("Scale", scale)) {
              component->SetScale(scale);
            }
            ImGui::Unindent();
          }
          if (ImGui::CollapsingHeader(
                  "Material",
                  ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent();
            if (type == vzm::SCENE_COMPONENT_TYPE::ACTOR) {
              vzm::VzActor* actor = (vzm::VzActor*)component;
              std::vector<VID> mis = actor->GetMIs();
              for (size_t prim = 0; prim < mis.size(); ++prim) {
                vzm::VzMI* mi = (vzm::VzMI*)vzm::GetVzComponent(mis[prim]);
                std::string mname = mi->GetName();
                if (ImGui::CollapsingHeader(mname.c_str())) {
                  ImGui::Indent();
                  VID maid = mi->GetMaterial();
                  vzm::VzMaterial* ma =
                      (vzm::VzMaterial*)vzm::GetVzComponent(maid);
                  std::map<std::string, vzm::UniformType> pram;
                  ma->GetAllowedParameters(pram);
                  auto it = pram.begin();
                  auto end_it = pram.end();
                  while (it != end_it) {
                    std::vector<float> v;
                    ImGui::Text("%s", it->first.c_str());
                    int size = (int)it->second - 3;
                    if (size > 0) {
                      v.resize(size);
                      mi->GetParameter(it->first, it->second, (void*)v.data());
                      std::string pname = it->first + std::to_string(prim);
                      ImGui::PushItemWidth(-1);
                      switch (it->second) {
                        case vzm::UniformType::BOOL:
                          if (ImGui::Checkbox(pname.c_str(), (bool*)&v[0])) {
                            mi->SetParameter(it->first, it->second,
                                             (void*)v.data());
                          }
                          break;
                        case vzm::UniformType::FLOAT:
                          if (ImGui::InputFloat(pname.c_str(), &v[0])) {
                            mi->SetParameter(it->first, it->second,
                                             (void*)v.data());
                          }
                          break;
                        case vzm::UniformType::FLOAT3:
                          if (ImGui::ColorEdit3(pname.c_str(), &v[0]),
                              ImGuiColorEditFlags_DefaultOptions_) {
                            mi->SetParameter(it->first, it->second,
                                             (void*)v.data());
                          }
                          break;
                        case vzm::UniformType::FLOAT4:
                          if (ImGui::ColorEdit4(pname.c_str(), &v[0]),
                              ImGuiColorEditFlags_DefaultOptions_) {
                            mi->SetParameter(it->first, it->second,
                                             (void*)v.data());
                            break;
                          }
                      }
                      ImGui::PopItemWidth();
                    }
                    it++;
                  }
                  ImGui::Unindent();
                }
              }
            }
            ImGui::Unindent();
          }
          if (ImGui::CollapsingHeader(
                  "Light",
                  ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent();

            if (type == vzm::SCENE_COMPONENT_TYPE::LIGHT) {
              vzm::VzLight* lightComponent = (vzm::VzLight*)component;
              float intensity = lightComponent->GetIntensity();
              float color[3];
              lightComponent->GetColor(color);
              float range = lightComponent->GetRange();
              float innerCone = lightComponent->getInnerCone();
              float outerCone = lightComponent->getOuterCone();

              if (ImGui::InputFloat("Intensity", &intensity)) {
                lightComponent->SetIntensity(intensity);
              }
              if (ImGui::ColorEdit3("Color", color)) {
                lightComponent->SetColor(color);
              }
              if (ImGui::InputFloat("Range", &range)) {
                lightComponent->SetRange(range);
              }
              if (ImGui::InputFloat("Inner Cone", &innerCone)) {
                lightComponent->SetCone(innerCone, outerCone);
              }
              if (ImGui::InputFloat("Outer Cone", &outerCone)) {
                lightComponent->SetCone(innerCone, outerCone);
              }
            }
            ImGui::Unindent();
          }

          break;
        }
        case 1:
          if (ImGui::Button("Import", ImVec2(right_editUIWidth, 50))) {
            std::wstring filePath = OpenFileDialog();
            if (filePath.size() > 0) {
              std::string str_path;
              str_path.assign(filePath.begin(), filePath.end());

              if (g_asset) {
                deinitViewer();
                initViewer();
              }
              g_asset = vzm::LoadFileIntoAsset(str_path, "my gltf asset");

              // animation
              g_asset->GetAnimator()->AddPlayScene(g_scene->GetVID());
              g_asset->GetAnimator()->SetPlayMode(
                  vzm::VzAsset::Animator::PlayMode::PAUSE);
              int animCount = g_asset->GetAnimator()->GetAnimationCount();
              animActiveVec = std::vector<bool>(animCount);
              currentAnimTotalTime = 0.0f;

              std::vector<VID> root_vids = g_asset->GetGLTFRoots();
              if (root_vids.size() > 0) {
                vzm::AppendSceneCompVidTo(root_vids[0], g_scene->GetVID());
              }
              // camera
              vzm::GetSceneCompoenentVids(vzm::SCENE_COMPONENT_TYPE::CAMERA,
                                          g_scene->GetVID(), cameras);
              for (int i = 0; i < cameras.size(); i++) {
                if (g_cam == (vzm::VzCamera*)vzm::GetVzComponent(cameras[i])) {
                  current_cam_idx = i;
                }
              }
            }
          }
          if (ImGui::Button("Export", ImVec2(right_editUIWidth, 50))) {
            vzm::ExportAssetToGlb(g_asset, "export.glb");
          }
          if (ImGui::CollapsingHeader("Automation")) {
            // ImGui::Indent();

            //// if (true) {
            ////   ImGui::TextColored(yellow, "Test case %zu / %zu", 0, 0);
            //// } else {
            ////   ImGui::TextColored(yellow, "%zu test cases", 0);
            //// }
            //// ImGui::PushItemWidth(150);
            //// ImGui::SliderFloat("Sleep (seconds)", &any, 0.0, 5.0);
            //// ImGui::PopItemWidth();
            //// ImGui::Checkbox("Export screenshot for each test", &bAny);
            //// ImGui::Checkbox("Export settings JSON for each test", &bAny);
            //// if (false) {
            ////   if (ImGui::Button("Stop batch test")) {
            ////   }
            //// } else if (ImGui::Button("Run batch test")) {
            //// }

            // if (ImGui::Button("Export view settings")) {
            //   ImGui::OpenPopup("MessageBox");
            // }
            // ImGui::Unindent();
          }

          if (ImGui::CollapsingHeader("Stats")) {
            // ImGui::Indent();
            // ImGui::Text("%zu entities in the asset", 0);
            // ImGui::Text("%zu renderables (excluding UI)", 0);
            // ImGui::Text("%zu skipped frames", 0);
            // ImGui::Unindent();
          }

          if (ImGui::CollapsingHeader("Debug")) {
            //            if (ImGui::Button("Capture frame")) {
            //            }
            //            ImGui::Checkbox("Disable buffer padding", &bAny);
            //            ImGui::Checkbox("Disable sub-passes", &bAny);
            //            ImGui::Checkbox("Camera at origin", &bAny);
            //            ImGui::Checkbox("Far Origin", &bAny);
            //            ImGui::SliderFloat("Origin", &any, 0, 1);
            //            ImGui::Checkbox("Far uses shadow casters", &bAny);
            //            ImGui::Checkbox("Focus shadow casters", &bAny);
            //
            //            bool debugDirectionalShadowmap;
            //            if (true) {
            //              ImGui::Checkbox("Debug DIR shadowmap", &bAny);
            //            }
            //
            //            ImGui::Checkbox("Display Shadow Texture", &bAny);
            //            if (true) {
            //              int layerCount;
            //              int levelCount;
            //              ImGui::Indent();
            //              ImGui::SliderFloat("scale", &any, 0.0f, 8.0f);
            //              ImGui::SliderFloat("contrast", &any, 0.0f, 8.0f);
            //              ImGui::SliderInt("layer", &iAny, 0, 10);
            //              ImGui::SliderInt("level", &iAny, 0, 10);
            //              ImGui::SliderInt("channel", &iAny, 0, 10);
            //              ImGui::Unindent();
            //            }
            //            bool debugFroxelVisualization;
            //            if (true) {
            //              ImGui::Checkbox("Froxel Visualization", &bAny);
            //            }
            //
            // #ifndef NDEBUG
            //            ImGui::SliderFloat("Kp", &any, 0, 2);
            //            ImGui::SliderFloat("Ki", &any, 0, 2);
            //            ImGui::SliderFloat("Kd", &any, 0, 2);
            // #endif
            //            ImGui::BeginDisabled(bAny);  // overdrawDisabled);
            //            ImGui::Checkbox(!bAny        // overdrawDisabled
            //                                ? "Visualize overdraw"
            //                                : "Visualize overdraw (disabled
            //                                for Vulkan)",
            //                            &bAny);
            //            ImGui::EndDisabled();
          }

          if (ImGui::BeginPopupModal("MessageBox", nullptr,
                                     ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s", "testtest");  // app.messageBoxText.c_str());
            if (ImGui::Button("OK", ImVec2(120, 0))) {
              ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
          }

          if (ImGui::CollapsingHeader("View")) {
            bool bPostProcessEnabled = g_renderer->IsPostProcessingEnabled();
            bool bDitheringEnabled = g_renderer->IsDitheringEnabled();
            bool bBloomEnabled = g_renderer->IsBloomEnabled();
            bool bTaaEnabled = g_renderer->IsTaaEnabled();
            bool bFxaaEnabled = g_renderer->IsFxaaEnabled();
            bool bMsaaEnabled = g_renderer->IsMsaaEnabled();
            bool bSsaoEnabled = g_renderer->IsSsaoEnabled();
            bool bScreenSpaceReflectionEnabled =
                g_renderer->IsScreenSpaceReflectionEnabled();
            bool bGuardBandEnabled = g_renderer->IsGuardBandEnabled();

            ImGui::Indent();

            if (ImGui::Checkbox("Post-processing", &bPostProcessEnabled)) {
              g_renderer->SetPostProcessingEnabled(bPostProcessEnabled);
            }
            ImGui::Indent();
            if (ImGui::Checkbox("Dithering", &bDitheringEnabled)) {
              g_renderer->SetDitheringEnabled(bDitheringEnabled);
            }
            if (ImGui::Checkbox("Bloom", &bBloomEnabled)) {
              g_renderer->SetBloomEnabled(bBloomEnabled);
            }
            if (ImGui::Checkbox("TAA", &bTaaEnabled)) {
              g_renderer->SetTaaEnabled(bTaaEnabled);
            }

            if (ImGui::Checkbox("FXAA", &bFxaaEnabled)) {
              g_renderer->SetFxaaEnabled(bFxaaEnabled);
            }
            ImGui::Unindent();

            if (ImGui::Checkbox("MSAA 4x", &bMsaaEnabled)) {
              g_renderer->SetMsaaEnabled(bMsaaEnabled);
            }
            // ImGui::Indent();
            // ImGui::Checkbox("Custom resolve", &bAny);
            // ImGui::Unindent();

            if (ImGui::Checkbox("SSAO", &bSsaoEnabled)) {
              g_renderer->SetSsaoEnabled(bSsaoEnabled);
            }

            if (ImGui::Checkbox("Screen-space reflections",
                                &bScreenSpaceReflectionEnabled)) {
              g_renderer->SetScreenSpaceReflectionEnabled(
                  bScreenSpaceReflectionEnabled);
            }
            ImGui::Unindent();

            if (ImGui::Checkbox("Screen-space Guard Band",
                                &bGuardBandEnabled)) {
              g_renderer->SetGuardBandEnabled(bGuardBandEnabled);
            }
          }

          if (ImGui::CollapsingHeader("Bloom Options")) {
            float bloomStrength = g_renderer->GetBloomStrength();
            bool isBloomThreshold = g_renderer->IsBloomThreshold();
            int bloomLevels = g_renderer->GetBloomLevels();
            int bloomQuality = g_renderer->GetBloomQuality();
            bool isBloomLensFlare = g_renderer->IsBloomLensFlare();

            if (ImGui::SliderFloat("Strength", &bloomStrength, 0.0f, 1.0f)) {
              g_renderer->SetBloomStrength(bloomStrength);
            }
            if (ImGui::Checkbox("Threshold", &isBloomThreshold)) {
              g_renderer->SetBloomThreshold(isBloomThreshold);
            }
            if (ImGui::SliderInt("Levels", &bloomLevels, 3, 11)) {
              g_renderer->SetBloomLevels(bloomLevels);
            }
            if (ImGui::SliderInt("Bloom Quality", &bloomQuality, 0, 3)) {
              g_renderer->SetBloomQuality(bloomQuality);
            }
            if (ImGui::Checkbox("Lens Flare", &isBloomLensFlare)) {
              g_renderer->SetBloomLensFlare(isBloomLensFlare);
            }
          }

          ///////////////////////////////////
          if (ImGui::CollapsingHeader("TAA Options")) {
            /*ImGui::Checkbox("Upscaling", &bAny);
            ImGui::Checkbox("History Reprojection", &bAny);
            ImGui::SliderFloat("Feedback", &any, 0.0f, 1.0f);
            ImGui::Checkbox("Filter History", &bAny);
            ImGui::Checkbox("Filter Input", &bAny);
            ImGui::SliderFloat("FilterWidth", &any, 0.2f, 2.0f);
            ImGui::SliderFloat("LOD bias", &any, -8.0f, 0.0f);
            ImGui::Checkbox("Use YCoCg", &bAny);
            ImGui::Checkbox("Prevent Flickering", &bAny);
            ImGui::Combo("Jitter Pattern", &iAny,
                         "RGSS x4\0Uniform Helix x4\0Halton x8\0Halton "
                         "x16\0Halton x32\0\0");
            ImGui::Combo("Box Clipping", &iAny, "Accurate\0Clamp\0None\0\0");
            ImGui::Combo("Box Type", &iAny, "AABB\0Variance\0Both\0\0");
            ImGui::SliderFloat("Variance Gamma", &any, 0.75f, 1.25f);
            ImGui::SliderFloat("RCAS", &any, 0.0f, 1.0f);*/
          }

          if (ImGui::CollapsingHeader("SSAO Options")) {
            /*   ImGui::SliderInt("Quality", &iAny, 0, 3);
               ImGui::SliderInt("Low Pass", &iAny, 0, 2);
               ImGui::Checkbox("Bent Normals", &bAny);
               ImGui::Checkbox("High quality upsampling", &bAny);
               ImGui::SliderFloat("Min Horizon angle", &any, 0.0f, (float)1.0);
               ImGui::SliderFloat("Bilateral Threshold", &any, 0.0f, 0.1f);
               ImGui::Checkbox("Half resolution", &bAny);
               if (ImGui::CollapsingHeader(
                       "Dominant Light Shadows (experimental)")) {
                 ImGui::Checkbox("Enabled##dls", &bAny);
                 ImGui::SliderFloat("Cone angle", &any, 0.0f, 1.0f);
                 ImGui::SliderFloat("Shadow Distance", &any, 0.0f, 10.0f);
                 ImGui::SliderFloat("Contact dist max", &any, 0.0f, 100.0f);
                 ImGui::SliderFloat("Intensity##dls", &any, 0.0f, 10.0f);
                 ImGui::SliderFloat("Depth bias", &any, 0.0f, 1.0f);
                 ImGui::SliderFloat("Depth slope bias", &any, 0.0f, 1.0f);
                 ImGui::SliderInt("Sample Count", &iAny, 1, 32);
               }*/
          }

          if (ImGui::CollapsingHeader("Screen-space reflections Options")) {
            /*       ImGui::SliderFloat("Ray thickness", &any, 0.001f, 0.2f);
                   ImGui::SliderFloat("Bias", &any, 0.001f, 0.5f);
                   ImGui::SliderFloat("Max distance", &any, 0.1f, 10.0f);
                   ImGui::SliderFloat("Stride", &any, 1.0, 10.0f);*/
          }

          if (ImGui::CollapsingHeader("Dynamic Resolution")) {
            // ImGui::Checkbox("enabled", &bAny);
            // ImGui::Checkbox("homogeneous", &bAny);
            // ImGui::SliderFloat("min. scale", &any, 0.25f, 1.0f);
            // ImGui::SliderFloat("max. scale", &any, 0.25f, 1.0f);
            // ImGui::SliderInt("quality", &iAny, 0, 3);
            // ImGui::SliderFloat("sharpness", &any, 0.0f, 1.0f);
          }
          if (ImGui::CollapsingHeader("Light")) {
            ImGui::Indent();
            if (ImGui::CollapsingHeader("Indirect light")) {
              float iblIntensity = g_scene->GetIBLIntensity();
              float iblRotation = g_scene->GetIBLRotation();
              if (ImGui::Button("Select IBL", ImVec2(right_editUIWidth, 50))) {
                std::wstring filePath = OpenFileDialog();
                if (filePath.size() != 0) {
                  std::string str_path;
                  str_path.assign(filePath.begin(), filePath.end());
                  g_scene->LoadIBL(str_path);
                }
              }
              if (ImGui::InputFloat("IBL intensity", &iblIntensity)) {
                g_scene->SetIBLIntensity(iblIntensity);
              }
              if (ImGui::SliderAngle("IBL rotation", &iblRotation)) {
                g_scene->SetIBLRotation(iblRotation);
              }
            }
            // if (ImGui::CollapsingHeader("Sunlight")) {
            //   ImGui::Checkbox("Enable sunlight", &bAny);
            //   ImGui::SliderFloat("Sun intensity", &any, 0.0f, 150000.0f);
            //   ImGui::SliderFloat("Halo size", &any, 1.01f, 40.0f);
            //   ImGui::SliderFloat("Halo falloff", &any, 4.0f, 1024.0f);
            //   ImGui::SliderFloat("Sun radius", &any, 0.1f, 10.0f);
            //   ImGui::SliderFloat("Shadow Far", &any, 0.0f, 10.0f);
            //   //                             mSettings.viewer.cameraFar);

            //  if (ImGui::CollapsingHeader("Shadow direction")) {
            //    // ImGuiExt::DirectionWidget("Shadow direction",
            //    // shadowDirection.v);
            //  }
            //}
            // if (ImGui::CollapsingHeader("Shadows")) {
            //  ImGui::Checkbox("Enable shadows", &bAny);
            //  ImGui::SliderInt("Shadow map size", &iAny, 32, 1024);
            //  ImGui::Checkbox("Stable Shadows", &bAny);
            //  ImGui::Checkbox("Enable LiSPSM", &bAny);

            //  ImGui::Combo("Shadow type", &iAny,
            //               "PCF\0VSM\0DPCF\0PCSS\0PCFd\0\0");

            //  if (true) {
            //    ImGui::Checkbox("High precision", &bAny);
            //    ImGui::Checkbox("ELVSM", &bAny);
            //    ImGui::SliderInt("VSM MSAA samples", &iAny, 0, 3);
            //    ImGui::SliderInt("VSM anisotropy", &iAny, 0, 3);
            //    ImGui::Checkbox("VSM mipmapping", &bAny);
            //    ImGui::SliderFloat("VSM blur", &any, 0.0f, 125.0f);
            //  } else if (false) {
            //    ImGui::SliderFloat("Penumbra scale", &any, 0.0f, 100.0f);
            //    ImGui::SliderFloat("Penumbra Ratio scale", &any, 1.0f,
            //    100.0f);
            //  }

            //  ImGui::SliderInt("Cascades", &iAny, 1, 4);
            //  ImGui::Checkbox("Debug cascades", &bAny);
            //  ImGui::Checkbox("Enable contact shadows", &bAny);
            //  ImGui::SliderFloat("Split pos 0", &any, 0.0f, 1.0f);
            //  ImGui::SliderFloat("Split pos 1", &any, 0.0f, 1.0f);
            //  ImGui::SliderFloat("Split pos 2", &any, 0.0f, 1.0f);
            //}
            ImGui::Unindent();
          }

          if (ImGui::CollapsingHeader("Fog")) {
            /* ImGui::Indent();
             ImGui::Checkbox("Enable large-scale fog", &bAny);
             ImGui::SliderFloat("Start [m]", &any, 0.0f, 100.0f);
             ImGui::SliderFloat("Extinction [1/m]", &any, 0.0f, 1.0f);
             ImGui::SliderFloat("Floor [m]", &any, 0.0f, 100.0f);
             ImGui::SliderFloat("Height falloff [1/m]", &any, 0.0f, 4.0f);
             ImGui::SliderFloat("Sun Scattering start [m]", &any, 0.0f, 100.0f);
             ImGui::SliderFloat("Sun Scattering size", &any, 0.1f, 100.0f);
             ImGui::Checkbox("Exclude Skybox", &bAny);
             ImGui::Combo("Color##fogColor", &iAny,
             "Constant\0IBL\0Skybox\0\0"); ImGui::ColorPicker3("Color", anyVec);
             ImGui::Unindent();*/
          }
          if (ImGui::CollapsingHeader("Scene")) {
            // ImGui::Indent();

            // if (ImGui::Checkbox("Scale to unit cube", &bAny)) {
            // }

            // ImGui::Checkbox("Automatic instancing", &bAny);

            // ImGui::Checkbox("Show skybox", &bAny);
            // ImGui::ColorEdit3("Background color", anyVec);

            //// We do not yet support ground shadow or scene selection in
            /// remote / mode.
            // if (true) {
            //   ImGui::Checkbox("Ground shadow", &bAny);
            //   ImGui::Indent();
            //   ImGui::SliderFloat("Strength", &any, 0.0f, 1.0f);
            //   ImGui::Unindent();

            //  // if (mAsset->getSceneCount() > 1) {
            //  //   ImGui::Separator();
            //  //   sceneSelectionUI();
            //  // }
            //}

            // ImGui::Unindent();
          }

          if (ImGui::CollapsingHeader("Camera")) {
            // ImGui::Indent();
            // float zNearP, zFarP, fovInDegree, aspectRatio;
            // g_cam->GetPerspectiveProjection(&zNearP, &zFarP, &fovInDegree,
            //                                 &aspectRatio);
            // if (ImGui::SliderFloat("Near", &zNearP, 0.001f, 1.0f)) {
            //   g_cam->SetPerspectiveProjection(zNearP, zFarP, fovInDegree,
            //                                   aspectRatio);
            // }
            // if (ImGui::SliderFloat("Far", &zFarP, 1.0f, 10000.0f)) {
            //   g_cam->SetPerspectiveProjection(zNearP, zFarP, fovInDegree,
            //                                   aspectRatio);
            // }
            // if (ImGui::InputFloat("Fov", &fovInDegree)) {
            //   g_cam->SetPerspectiveProjection(zNearP, zFarP, fovInDegree,
            //                                   aspectRatio);
            // }
            // if (ImGui::InputFloat("AspectRatio", &aspectRatio)) {
            //   g_cam->SetPerspectiveProjection(zNearP, zFarP, fovInDegree,
            //                                   aspectRatio);
            // }

            // ImGui::SliderFloat("Focal length (mm)", &any, 16.0f, 90.0f);
            // ImGui::SliderFloat("Aperture", &any, 1.0f, 32.0f);
            // ImGui::SliderFloat("Speed (1/s)", &any, 1000.0f, 1.0f);
            // ImGui::SliderFloat("ISO", &any, 25.0f, 6400.0f);
            // if (ImGui::CollapsingHeader("DoF")) {
            //   ImGui::Checkbox("Enabled##dofEnabled", &bAny);
            //   ImGui::SliderFloat("Focus distance", &any, 0.0f, 30.0f);
            //   ImGui::SliderFloat("Blur scale", &any, 0.1f, 10.0f);
            //   ImGui::SliderFloat("CoC aspect-ratio", &any, 0.25f, 4.0f);
            //   ImGui::SliderInt("Ring count", &iAny, 1, 17);
            //   ImGui::SliderInt("Max CoC", &iAny, 1, 32);
            //   ImGui::Checkbox("Native Resolution", &bAny);
            //   ImGui::Checkbox("Median Filter", &bAny);
            // }

            // if (ImGui::CollapsingHeader("Vignette")) {
            //   ImGui::Checkbox("Enabled##vignetteEnabled", &bAny);
            //   ImGui::SliderFloat("Mid point", &any, 0.0f, 1.0f);
            //   ImGui::SliderFloat("Roundness", &any, 0.0f, 1.0f);
            //   ImGui::SliderFloat("Feather", &any, 0.0f, 1.0f);
            //   ImGui::ColorEdit3("Color##vignetteColor", anyVec);
            // }

            //// We do not yet support camera selection in the remote UI. To
            //// support this feature, we would need to send a message from
            //// DebugServer to the WebSockets client.
            ///*       if (true) {
            //         ImGui::ListBox("Cameras", &mCurrentCamera,
            //         cstrings.data(),
            //                        cstrings.size());
            //       }*/

            // ImGui::SliderFloat("Ocular distance", &any, 0.0f, 1.0f);

            // float toeInDegrees = any;
            //// mSettings.viewer.cameraEyeToeIn / f::PI * 180.0f;
            // ImGui::SliderFloat("Toe in", &toeInDegrees, 0.0f, 30.0, "%.3f°");
            //// mSettings.viewer.cameraEyeToeIn = toeInDegrees / 180.0f *
            /// f::PI;

            // ImGui::Unindent();
          }
          break;
      }

      // right_editUIWidth = ImGui::GetWindowWidth();
      ImGui::End();
    }

    // render
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    const bool is_minimized =
        (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
    if (!is_minimized) {
      wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
      wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
      wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
      wd->ClearValue.color.float32[3] = clear_color.w;
      FrameRender(wd, draw_data);
      FramePresent(wd);
    }
  }
  vzm::DeinitEngineLib();

  // Cleanup
  err = vkDeviceWaitIdle(g_Device);
  check_vk_result(err);
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  vkFreeDescriptorSets(g_Device, g_DescriptorPool, 2, swapTextures);
  vkDestroyImageView(g_Device, swapImageViews[0], nullptr);
  vkDestroyImageView(g_Device, swapImageViews[1], nullptr);
  vkDestroyImage(g_Device, swapImages[0], nullptr);
  vkDestroyImage(g_Device, swapImages[1], nullptr);
  vkFreeMemory(g_Device, swapMemories[0], nullptr);
  vkFreeMemory(g_Device, swapMemories[1], nullptr);
  vkDestroySampler(g_Device, sampler, nullptr);

  CleanupVulkanWindow();
  CleanupVulkan();

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}