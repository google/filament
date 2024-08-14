#include "VizEngineAPIs.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "settings.hpp"

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

vzm::VzRenderer* g_renderer;
vzm::VzScene* g_scene;

vzm::VzCamera* g_cam;
vzm::VzCamera* current_cam;
int current_cam_idx = 0;
std::vector<VID> cameras;

bool g_lightEnabled = false;
vzm::VzLight* g_light;

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
  if (current_cam == g_cam) {
    g_cam->GetController()->SetViewport(width, height);
  }

  g_renderer->SetCanvas(width, height, 96.f, nullptr);
  float zNearP, zFarP, fovInDegree;
  current_cam->GetPerspectiveProjection(&zNearP, &zFarP, &fovInDegree, nullptr);
  current_cam->SetPerspectiveProjection(zNearP, zFarP, fovInDegree,
                                        (float)width / (float)height);
}

void setMouseButton(GLFWwindow* window, int button, int state,
                    int modifier_key) {
  if (!g_cam || g_cam != current_cam) {
    return;
  }

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
        if (button == 0) {
          g_cam->GetController()->GrabBegin(xPos, yPos, false);
        } else if (button == 1) {
          g_cam->GetController()->GrabBegin(xPos, yPos, true);
        }
      }
      break;
    case GLFW_RELEASE:
      g_cam->GetController()->GrabEnd();
      break;
    default:
      break;
  }
}
void setCursorPos(GLFWwindow* window, double x, double y) {
  int width;
  int height;

  if (!g_cam || g_cam != current_cam) {
    return;
  }
  glfwGetWindowSize(window, &width, &height);

  int xPos = static_cast<int>(x - left_editUIWidth);
  int yPos = height - static_cast<int>(y);

  if (glfwGetMouseButton(window, 0) == GLFW_PRESS ||
      glfwGetMouseButton(window, 1) == GLFW_PRESS) {
    g_cam->GetController()->GrabDrag(xPos, yPos);
  }
}
void setMouseScroll(GLFWwindow* window, double xOffset, double yOffset) {
  if (!g_cam || g_cam != current_cam) {
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
    g_cam->GetController()->Scroll(static_cast<int>(x - left_editUIWidth),
                                   height - static_cast<int>(y),
                                   -5.0f * (float)yOffset);
  }
}
void onFrameBufferResize(GLFWwindow* window, int width, int height) {
  if (g_renderer && current_cam) {
    workspace_width = width - left_editUIWidth - right_editUIWidth;
    workspace_height = height;

    resize(render_width, render_height);
  }
}

void treeNode(VID id) {
  vzm::VzSceneComp* component = (vzm::VzSceneComp*)vzm::GetVzComponent(id);
  std::string sName = component->GetName();

  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
  if (currentVID == id) {
    flags |= ImGuiTreeNodeFlags_Selected;
  }
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
  //g_scene->LoadIBL("lightroom_14b");
  g_cam = (vzm::VzCamera*)vzm::NewSceneComponent(
      vzm::SCENE_COMPONENT_TYPE::CAMERA, "UserCamera");
  glm::fvec3 p(0, 0, 10);
  glm::fvec3 at(0, 0, -4);
  glm::fvec3 u(0, 1, 0);
  g_cam->SetWorldPose((float*)&p, (float*)&at, (float*)&u);
  g_cam->SetPerspectiveProjection(0.1f, 1000.f, 45.f,
                                  (float)render_width / render_height);
  vzm::VzCamera::Controller* cc = g_cam->GetController();
  *(glm::fvec3*)cc->orbitHomePosition = p;
  cc->UpdateControllerSettings();
  cc->SetViewport(render_width, render_height);

  g_light = (vzm::VzLight*)vzm::NewSceneComponent(
      vzm::SCENE_COMPONENT_TYPE::LIGHT, "sunlight");
  vzm::AppendSceneCompTo(g_light, g_scene);
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
      glfwCreateWindow(1280, 720, "Grapicar Viewer", nullptr, nullptr);
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
  void* swapHandles[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};

  float any = 0.0f;
  int iAny = 0;
  bool bAny = 0.0f;
  float anyVec[3] = {
      0,
  };

  int tabIdx = 0;
  int camTabIdx = 0;

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
          vzm::VzBaseComp* comp = vzm::GetVzComponent(cameras[current_cam_idx]);
          std::string previewName = comp->GetName();
          if (ImGui::BeginCombo("Current Camera", previewName.c_str())) {
            for (int n = 0; n < cameras.size(); n++) {
              bool is_selected = (current_cam_idx == n);
              std::string camName = vzm::GetVzComponent(cameras[n])->GetName();
              if (ImGui::Selectable(camName.c_str(), is_selected)) {
                current_cam_idx = n;
                current_cam = (vzm::VzCamera*)vzm::GetVzComponent(cameras[n]);
                resize(render_width, render_height);
              }
              if (is_selected) {
                ImGui::SetItemDefaultFocus();
              }
            }
            ImGui::EndCombo();
          }

          ImGui::Text("set projection from");
          {
            ImGui::BeginTabBar("camTab");
            if (ImGui::BeginTabItem("fov")) {
              camTabIdx = 0;
              ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("focal length")) {
              camTabIdx = 1;
              ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
          }
          switch (camTabIdx) {
            case 0: {
              float zNearP, zFarP, fovInDegree;
              current_cam->GetPerspectiveProjection(&zNearP, &zFarP,
                                                    &fovInDegree, nullptr);
              if (ImGui::SliderFloat("Near", &zNearP, 0.001f, 1.0f)) {
                current_cam->SetPerspectiveProjection(
                    zNearP, zFarP, fovInDegree, (float)width / (float)height);
              }
              if (ImGui::SliderFloat("Far", &zFarP, 1.0f, 10000.0f)) {
                current_cam->SetPerspectiveProjection(
                    zNearP, zFarP, fovInDegree, (float)width / (float)height);
              }
              if (ImGui::InputFloat("fov", &fovInDegree)) {
                current_cam->SetPerspectiveProjection(
                    zNearP, zFarP, fovInDegree, (float)width / (float)height);
              }
              break;
            }
            case 1: {
              float zNearP = current_cam->GetNear();
              float zFarP = current_cam->GetCullingFar();
              float focalLength = current_cam->GetFocalLength();
              if (ImGui::SliderFloat("Near", &zNearP, 0.001f, 1.0f)) {
                current_cam->SetLensProjection(
                    focalLength, (float)width / (float)height, zNearP, zFarP);
              }
              if (ImGui::SliderFloat("Far", &zFarP, 1.0f, 10000.0f)) {
                current_cam->SetLensProjection(
                    focalLength, (float)width / (float)height, zNearP, zFarP);
              }
              if (ImGui::SliderFloat("Focal length (mm)", &focalLength, 16.0f,
                                     90.0f)) {
                current_cam->SetLensProjection(
                    focalLength, (float)width / (float)height, zNearP, zFarP);
              }
              break;
            }
          }

          float aperture = current_cam->GetAperture();
          float shutterSpeed = 1.0f / current_cam->GetShutterSpeed();
          float sensitivity = current_cam->GetSensitivity();
          float focusDistance = current_cam->GetFocusDistance();

          if (ImGui::SliderFloat("Aperture", &aperture, 1.0f, 32.0f)) {
            current_cam->SetExposure(aperture, 1.0f / shutterSpeed,
                                     sensitivity);
          }
          if (ImGui::SliderFloat("Speed (1/s)", &shutterSpeed, 1000.0f, 1.0f)) {
            current_cam->SetExposure(aperture, 1.0f / shutterSpeed,
                                     sensitivity);
          }
          if (ImGui::SliderFloat("ISO", &sensitivity, 25.0f, 6400.0f)) {
            current_cam->SetExposure(aperture, 1.0f / shutterSpeed,
                                     sensitivity);
          }
          if (ImGui::SliderFloat("Focus distance", &focusDistance, 0.0f,
                                 30.0f)) {
            current_cam->SetFocusDistance(focusDistance);
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
                std::string postLabel = "##";
                postLabel += mi->GetVID();

                if (ImGui::CollapsingHeader(mname.c_str())) {
                  ImGui::Indent();
                  VID maid = mi->GetMaterial();
                  vzm::VzMaterial* ma =
                      (vzm::VzMaterial*)vzm::GetVzComponent(maid);
                  std::map<std::string, vzm::VzMaterial::ParameterInfo> pram;
                  // std::map<std::string, vzm::UniformType> pram;
                  ma->GetAllowedParameters(pram);

                  //int lightModel = (int)ma->GetLightingModel();
                  //std::string lmLabelName = "LightModel";
                  //lmLabelName += postLabel;
                  //if (ImGui::Combo(
                  //        lmLabelName.c_str(), &lightModel,
                  //        "UNLIT\0LIT\0SUBSURFACE\0CLOTH\0SPECULAR_GLOSSINESS\0\0")) {
                  //  ma->SetLightingModel(
                  //      (vzm::VzMaterial::LightingModel)lightModel);
                  //}

                  bool doubleSided = mi->IsDoubleSided();
                  std::string dsLabelName = "DoubleSided";
                  dsLabelName += postLabel;
                  if (ImGui::Checkbox(dsLabelName.c_str(), &doubleSided)) {
                    mi->SetDoubleSided(doubleSided);
                  }

                  ImGui::PushItemWidth(-1);
                  int transparencyMode = (int)mi->GetTransparencyMode();
                  std::string tmLabelName = "TransparencyMode";
                  tmLabelName += postLabel;
                  ImGui::Text("Transparency Mode");
                  if (ImGui::Combo(
                          tmLabelName.c_str(), &transparencyMode,
                          "DEFAULT\0TWO_PASSES_ONE_SIDE\0TWO_PASSES_TWO_"
                          "SIDES\0\0")) {
                    mi->SetTransparencyMode(
                        (vzm::VzMI::TransparencyMode)transparencyMode);
                  }
                  ImGui::PopItemWidth();

                  for (auto iter = pram.begin(); iter != pram.end(); iter++) {
                    std::vector<float> v;
                    vzm::VzMaterial::ParameterInfo& paramInfo = iter->second;
                    std::string pname = paramInfo.name;
                    std::string param_label = pname + postLabel;

                    //_ 붙은 파라미터 제외
                    if (pname.find('_') != std::string::npos) {
                      continue;
                    }

                    ImGui::BeginGroup();
                    ImGui::Text("%s", pname.c_str());

                    ImGui::PushItemWidth(-1);

                    switch (paramInfo.type) {
                      case vzm::UniformType::BOOL:
                        if (paramInfo.isSampler) {
                          std::string label = "Upload Texture";
                          label += postLabel;
                          if (ImGui::Button(label.c_str())) {
                            vzm::VzTexture* texture =
                                (vzm::VzTexture*)vzm::NewResComponent(
                                    vzm::RES_COMPONENT_TYPE::TEXTURE,
                                    "my image");
                            std::wstring filePath = OpenFileDialog();

                            if (filePath.size() > 0) {
                              std::string str_path;
                              str_path.assign(filePath.begin(), filePath.end());
                              texture->ReadImage(str_path);
                              mi->SetTexture(pname, texture->GetVID());
                            }
                          }
                        }
                        break;
                      case vzm::UniformType::FLOAT:
                        v.resize(1);
                        mi->GetParameter(paramInfo.name, paramInfo.type,
                                         (void*)v.data());
                        if (ImGui::InputFloat(param_label.c_str(), &v[0])) {
                          mi->SetParameter(paramInfo.name, paramInfo.type,
                                           (void*)v.data());
                        }
                        break;
                      case vzm::UniformType::FLOAT3:
                        v.resize(3);
                        mi->GetParameter(paramInfo.name, paramInfo.type,
                                         (void*)v.data());
                        if (ImGui::ColorEdit3(param_label.c_str(), &v[0]),
                            ImGuiColorEditFlags_DefaultOptions_) {
                          mi->SetParameter(paramInfo.name, paramInfo.type,
                                           (void*)v.data());
                        }
                        break;
                      case vzm::UniformType::FLOAT4:
                        v.resize(4);
                        mi->GetParameter(paramInfo.name, paramInfo.type,
                                         (void*)v.data());
                        if (ImGui::ColorEdit4(param_label.c_str(), &v[0]),
                            ImGuiColorEditFlags_DefaultOptions_) {
                          mi->SetParameter(paramInfo.name, paramInfo.type,
                                           (void*)v.data());
                          break;
                        }
                    }
                    ImGui::PopItemWidth();
                    ImGui::EndGroup();
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

              int lightType = (int)lightComponent->GetType();
              float intensity = lightComponent->GetIntensity();
              float color[3];
              lightComponent->GetColor(color);
              float falloff = lightComponent->GetFalloff();

              if (ImGui::Combo(
                      "Light Type", &lightType,
                      "SUN\0DIRECTIONAL\0POINT\0FOCUSED_SPOT\0SPOT\0\0")) {
                lightComponent->SetType((vzm::VzLight::Type)lightType);
              }

              if (ImGui::SliderFloat("intensity", &intensity, 0.0f,
                                     150000.0f)) {
                lightComponent->SetIntensity(intensity);
              }

              if (ImGui::ColorEdit3("Color", color)) {
                lightComponent->SetColor(color);
              }
              if (ImGui::InputFloat("Falloff", &falloff)) {
                lightComponent->SetFalloff(falloff);
              }

              switch ((vzm::VzLight::Type)lightType) {
                case vzm::VzLight::Type::SUN: {
                  float haloSize = lightComponent->GetSunHaloSize();
                  float haloFallOff = lightComponent->GetSunHaloFalloff();
                  float sunRadius = lightComponent->GetSunAngularRadius();

                  if (ImGui::SliderFloat("Halo size", &haloSize, 1.01f,
                                         40.0f)) {
                    lightComponent->SetSunHaloSize(haloSize);
                  }
                  if (ImGui::SliderFloat("Halo falloff", &haloFallOff, 4.0f,
                                         1024.0f)) {
                    lightComponent->SetSunHaloFalloff(haloFallOff);
                  }
                  if (ImGui::SliderFloat("Sun radius", &sunRadius, 0.1f,
                                         10.0f)) {
                    lightComponent->SetSunAngularRadius(sunRadius);
                  }
                  break;
                }
                case vzm::VzLight::Type::DIRECTIONAL: {
                  break;
                }
                case vzm::VzLight::Type::POINT: {
                  break;
                }
                case vzm::VzLight::Type::FOCUSED_SPOT: {
                  break;
                }
                case vzm::VzLight::Type::SPOT: {
                  float spotLightInnerCone =
                      lightComponent->GetSpotLightInnerCone();
                  float spotLightOuterCone =
                      lightComponent->GetSpotLightOuterCone();

                  if (ImGui::InputFloat("Spot Light Inner Cone",
                                        &spotLightInnerCone)) {
                    lightComponent->SetSpotLightCone(spotLightInnerCone,
                                                     spotLightOuterCone);
                  }
                  if (ImGui::InputFloat("Spot Light Outer Cone",
                                        &spotLightOuterCone)) {
                    lightComponent->SetSpotLightCone(spotLightInnerCone,
                                                     spotLightOuterCone);
                  }
                  break;
                }
              }

              vzm::VzLight::ShadowOptions sOpts =
                  lightComponent->GetShadowOptions();

              ImGui::Indent();
              if (ImGui::CollapsingHeader("Shadow")) {
                bool shadowEnabled = lightComponent->IsShadowCaster();
                int mapSize = sOpts.mapSize;
                bool stableShadows = sOpts.stable;
                bool enableLiSPSM = sOpts.lispsm;
                float shadowFar = sOpts.shadowFar;

                if (ImGui::Checkbox("Enable shadow", &shadowEnabled)) {
                  lightComponent->SetShadowCaster(shadowEnabled);
                }
                if (ImGui::SliderInt("Shadow map size", &mapSize, 32, 1024)) {
                  sOpts.mapSize = mapSize;
                  lightComponent->SetShadowOptions(sOpts);
                }
                if (ImGui::Checkbox("Stable Shadows", &stableShadows)) {
                  sOpts.stable = stableShadows;
                  lightComponent->SetShadowOptions(sOpts);
                }
                if (ImGui::Checkbox("Enable LiSPSM", &enableLiSPSM)) {
                  sOpts.lispsm = enableLiSPSM;
                  lightComponent->SetShadowOptions(sOpts);
                }
                if (ImGui::SliderFloat("Shadow Far", &shadowFar, 0.0f, 10.0f)) {
                  sOpts.shadowFar = shadowFar;
                  lightComponent->SetShadowOptions(sOpts);
                }
                // if (ImGui::CollapsingHeader("Shadow direction")) {
                float fSunLightDirection[3];
                lightComponent->GetDirection(fSunLightDirection);
                if (ImGui::InputFloat3("Light Direction", fSunLightDirection)) {
                  lightComponent->SetDirection(fSunLightDirection);
                }
                //}
                if (g_renderer->GetShadowType() ==
                    vzm::VzRenderer::ShadowType::VSM) {
                  bool elvsm = sOpts.vsm.elvsm;
                  float vsmBlur = sOpts.vsm.blurWidth;

                  if (ImGui::Checkbox("ELVSM", &elvsm)) {
                    sOpts.vsm.elvsm = elvsm;
                    lightComponent->SetShadowOptions(sOpts);
                  }
                  if (ImGui::SliderFloat("VSM blur", &vsmBlur, 0.0f, 125.0f)) {
                    sOpts.vsm.blurWidth = vsmBlur;
                    lightComponent->SetShadowOptions(sOpts);
                  }
                }

                int shadowCascades = sOpts.shadowCascades;
                bool enableContactShadows = sOpts.screenSpaceContactShadows;
                float splitPos0 = sOpts.cascadeSplitPositions[0];
                float splitPos1 = sOpts.cascadeSplitPositions[1];
                float splitPos2 = sOpts.cascadeSplitPositions[2];

                if (ImGui::SliderInt("Cascades", &shadowCascades, 1, 4)) {
                  sOpts.shadowCascades = shadowCascades;
                  lightComponent->SetShadowOptions(sOpts);
                }
                if (ImGui::Checkbox("Enable contact shadows",
                                    &enableContactShadows)) {
                  sOpts.screenSpaceContactShadows = enableContactShadows;
                  lightComponent->SetShadowOptions(sOpts);
                }
                if (ImGui::SliderFloat("Split pos 0", &splitPos0, 0.0f, 1.0f)) {
                  sOpts.cascadeSplitPositions[0] = splitPos0;
                  lightComponent->SetShadowOptions(sOpts);
                }
                if (ImGui::SliderFloat("Split pos 1", &splitPos1, 0.0f, 1.0f)) {
                  sOpts.cascadeSplitPositions[1] = splitPos1;
                  lightComponent->SetShadowOptions(sOpts);
                }
                if (ImGui::SliderFloat("Split pos 2", &splitPos2, 0.0f, 1.0f)) {
                  sOpts.cascadeSplitPositions[2] = splitPos2;
                  lightComponent->SetShadowOptions(sOpts);
                }
              }
            }
            ImGui::Unindent();
          }

          break;
        }
        case 1:
          if (ImGui::Button("Import", ImVec2(right_editUIWidth, 40))) {
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
          if (ImGui::Button("Import Settings", ImVec2(right_editUIWidth, 40))) {
            std::wstring filePath = OpenFileDialog();
            if (filePath.size() > 0) {
              std::string str_path;
              str_path.assign(filePath.begin(), filePath.end());
              std::vector<VID> root_vids = g_asset->GetGLTFRoots();
              if (root_vids.size() > 0) {
                settingsIO::importSettings(root_vids[0], str_path);
              } else {
                std::cerr << "asset 없음" << std::endl;
              }
            }
          }
          if (ImGui::Button("Export", ImVec2(right_editUIWidth, 40))) {
            vzm::ExportAssetToGlb(g_asset, "export.glb");
          }
          if (ImGui::Button("Export Settings", ImVec2(right_editUIWidth, 40))) {
            std::vector<VID> root_vids = g_asset->GetGLTFRoots();
            if (root_vids.size() > 0) {
              settingsIO::exportSettings(root_vids[0]);
            } else {
              std::cerr << "asset 없음" << std::endl;
            }
          }
          //if (ImGui::CollapsingHeader("Automation")) {
          //  // ImGui::Indent();
          //  //// if (true) {
          //  ////   ImGui::TextColored(yellow, "Test case %zu / %zu", 0, 0);
          //  //// } else {
          //  ////   ImGui::TextColored(yellow, "%zu test cases", 0);
          //  //// }
          //  //// ImGui::PushItemWidth(150);
          //  //// ImGui::SliderFloat("Sleep (seconds)", &any, 0.0, 5.0);
          //  //// ImGui::PopItemWidth();
          //  //// ImGui::Checkbox("Export screenshot for each test", &bAny);
          //  //// ImGui::Checkbox("Export settings JSON for each test", &bAny);
          //  //// if (false) {
          //  ////   if (ImGui::Button("Stop batch test")) {
          //  ////   }
          //  //// } else if (ImGui::Button("Run batch test")) {
          //  //// }
          //  // if (ImGui::Button("Export view settings")) {
          //  //   ImGui::OpenPopup("MessageBox");
          //  // }
          //  // ImGui::Unindent();
          //}
          //if (ImGui::CollapsingHeader("Stats")) {
          //  // ImGui::Indent();
          //  // ImGui::Text("%zu entities in the asset", 0);
          //  // ImGui::Text("%zu renderables (excluding UI)", 0);
          //  // ImGui::Text("%zu skipped frames", 0);
          //  // ImGui::Unindent();
          //}
          //if (ImGui::CollapsingHeader("Debug")) {
          //  //            if (ImGui::Button("Capture frame")) {
          //  //            }
          //  //            ImGui::Checkbox("Disable buffer padding", &bAny);
          //  //            ImGui::Checkbox("Disable sub-passes", &bAny);
          //  //            ImGui::Checkbox("Camera at origin", &bAny);
          //  //            ImGui::Checkbox("Far Origin", &bAny);
          //  //            ImGui::SliderFloat("Origin", &any, 0, 1);
          //  //            ImGui::Checkbox("Far uses shadow casters", &bAny);
          //  //            ImGui::Checkbox("Focus shadow casters", &bAny);
          //  //
          //  //            bool debugDirectionalShadowmap;
          //  //            if (true) {
          //  //              ImGui::Checkbox("Debug DIR shadowmap", &bAny);
          //  //            }
          //  //
          //  //            ImGui::Checkbox("Display Shadow Texture", &bAny);
          //  //            if (true) {
          //  //              int layerCount;
          //  //              int levelCount;
          //  //              ImGui::Indent();
          //  //              ImGui::SliderFloat("scale", &any, 0.0f, 8.0f);
          //  //              ImGui::SliderFloat("contrast", &any, 0.0f, 8.0f);
          //  //              ImGui::SliderInt("layer", &iAny, 0, 10);
          //  //              ImGui::SliderInt("level", &iAny, 0, 10);
          //  //              ImGui::SliderInt("channel", &iAny, 0, 10);
          //  //              ImGui::Unindent();
          //  //            }
          //  //            bool debugFroxelVisualization;
          //  //            if (true) {
          //  //              ImGui::Checkbox("Froxel Visualization", &bAny);
          //  //            }
          //  //
          //  // #ifndef NDEBUG
          //  //            ImGui::SliderFloat("Kp", &any, 0, 2);
          //  //            ImGui::SliderFloat("Ki", &any, 0, 2);
          //  //            ImGui::SliderFloat("Kd", &any, 0, 2);
          //  // #endif
          //  //            ImGui::BeginDisabled(bAny);  // overdrawDisabled);
          //  //            ImGui::Checkbox(!bAny        // overdrawDisabled
          //  //                                ? "Visualize overdraw"
          //  //                                : "Visualize overdraw (disabled
          //  //                                for Vulkan)",
          //  //                            &bAny);
          //  //            ImGui::EndDisabled();
          //}

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
            bool bScreenSpaceReflectionsEnabled =
                g_renderer->IsScreenSpaceReflectionsEnabled();
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
                                &bScreenSpaceReflectionsEnabled)) {
              g_renderer->SetScreenSpaceReflectionsEnabled(
                  bScreenSpaceReflectionsEnabled);
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

          if (ImGui::CollapsingHeader("TAA Options")) {
            bool taaUpscaling = g_renderer->IsTaaUpscaling();
            bool isTaaHistoryReprojection =
                g_renderer->IsTaaHistoryReprojection();
            float feedback = g_renderer->GetTaaFeedback();
            bool filterHistory = g_renderer->IsTaaFilterHistory();
            bool filterInput = g_renderer->IsTaaFilterInput();
            float filterWidth = g_renderer->GetTaaFilterWidth();
            float lodBias = g_renderer->GetTaaLodBias();
            bool useYCoCg = g_renderer->IsTaaUseYCoCg();
            bool preventFlickering = g_renderer->IsTaaPreventFlickering();
            int taaJlitterPattern = (int)g_renderer->GetTaaJitterPattern();
            int taaBoxClipping = (int)g_renderer->GetTaaBoxClipping();
            int taaBoxType = (int)g_renderer->GetTaaBoxType();
            float varianceGamma = g_renderer->GetTaaVarianceGamma();
            float rcas = g_renderer->GetTaaSharpness();

            if (ImGui::Checkbox("Upscaling", &taaUpscaling)) {
              g_renderer->SetTaaUpscaling(taaUpscaling);
            }
            if (ImGui::Checkbox("History Reprojection",
                                &isTaaHistoryReprojection)) {
              g_renderer->SetTaaHistoryReprojection(isTaaHistoryReprojection);
            }
            if (ImGui::SliderFloat("Feedback", &feedback, 0.0f, 1.0f)) {
              g_renderer->SetTaaFeedback(feedback);
            }
            if (ImGui::Checkbox("Filter History", &filterHistory)) {
              g_renderer->SetTaaFilterHistory(filterHistory);
            }
            if (ImGui::Checkbox("Filter Input", &filterInput)) {
              g_renderer->SetTaaFilterInput(filterInput);
            }
            if (ImGui::SliderFloat("FilterWidth", &filterWidth, 0.2f, 2.0f)) {
              g_renderer->SetTaaFilterWidth(filterWidth);
            }
            if (ImGui::SliderFloat("LOD bias", &lodBias, -8.0f, 0.0f)) {
              g_renderer->SetTaaLodBias(lodBias);
            }
            if (ImGui::Checkbox("Use YCoCg", &useYCoCg)) {
              g_renderer->SetTaaUseYCoCg(useYCoCg);
            }
            if (ImGui::Checkbox("Prevent Flickering", &preventFlickering)) {
              g_renderer->SetTaaPreventFlickering(preventFlickering);
            }
            if (ImGui::Combo("Jitter Pattern", &taaJlitterPattern,
                             "RGSS x4\0Uniform Helix x4\0Halton x8\0Halton "
                             "x16\0Halton x32\0\0")) {
              g_renderer->SetTaaJitterPattern(
                  (vzm::VzRenderer::JitterPattern)taaJlitterPattern);
            }
            if (ImGui::Combo("Box Clipping", &taaBoxClipping,
                             "Accurate\0Clamp\0None\0\0")) {
              g_renderer->SetTaaBoxClipping(
                  (vzm::VzRenderer::BoxClipping)taaBoxClipping);
            }
            if (ImGui::Combo("Box Type", &taaBoxType,
                             "AABB\0Variance\0Both\0\0")) {
              g_renderer->SetTaaBoxType((vzm::VzRenderer::BoxType)taaBoxType);
            }
            if (ImGui::SliderFloat("Variance Gamma", &varianceGamma, 0.75f,
                                   1.25f)) {
              g_renderer->SetTaaVarianceGamma(varianceGamma);
            }
            if (ImGui::SliderFloat("RCAS", &rcas, 0.0f, 1.0f)) {
              g_renderer->SetTaaSharpness(rcas);
            }
          }

          if (ImGui::CollapsingHeader("SSAO Options")) {
            int quality = g_renderer->GetSsaoQuality();
            int lowPass = g_renderer->GetSsaoLowPassFilter();
            bool bentNormals = g_renderer->IsSsaoBentNormals();
            bool isUpsampling = g_renderer->IsSsaoUpsampling();
            float minHorizonAngle = g_renderer->GetSsaoMinHorizonAngleRad();
            float bilateralThreshold = g_renderer->GetSsaoBilateralThreshold();
            bool halfResolution = g_renderer->IsSsaoHalfResolution();

            if (ImGui::SliderInt("Quality", &quality, 0, 3)) {
              g_renderer->SetSsaoQuality(quality);
            }
            if (ImGui::SliderInt("Low Pass", &lowPass, 0, 2)) {
              g_renderer->SetSsaoLowPassFilter(lowPass);
            }
            if (ImGui::Checkbox("Bent Normals", &bentNormals)) {
              g_renderer->SetSsaoBentNormals(bentNormals);
            }
            if (ImGui::Checkbox("High quality upsampling", &isUpsampling)) {
              g_renderer->SetSsaoUpsampling(isUpsampling);
            }
            if (ImGui::SliderFloat("Min Horizon angle", &minHorizonAngle, 0.0f,
                                   (float)1.0)) {
              g_renderer->SetSsaoMinHorizonAngleRad(minHorizonAngle);
            }
            if (ImGui::SliderFloat("Bilateral Threshold", &bilateralThreshold,
                                   0.0f, 0.1f)) {
              g_renderer->SetSsaoBilateralThreshold(bilateralThreshold);
            }
            if (ImGui::Checkbox("Half resolution", &halfResolution)) {
              g_renderer->SetSsaoHalfResolution(halfResolution);
            }

            ImGui::Indent();
            if (ImGui::CollapsingHeader(
                    "Dominant Light Shadows (experimental)")) {
              bool ssctEnabled = g_renderer->IsSsaoSsctEnabled();
              float ssctLightConeRad = g_renderer->GetSsaoSsctLightConeRad();
              float shadowDist = g_renderer->GetSsaoSsctShadowDistance();
              float contactDistMax =
                  g_renderer->GetSsaoSsctContactDistanceMax();
              float ssctIntensity = g_renderer->GetSsaoSsctIntensity();
              float ssctDepthBias = g_renderer->GetSsaoSsctDepthBias();
              float ssctDepthSlopeBias =
                  g_renderer->GetSsaoSsctDepthSlopeBias();
              int sampleCount = g_renderer->GetSsaoSsctSampleCount();
              float lightDirection[3];
              g_renderer->GetSsaoSsctLightDirection(lightDirection);

              if (ImGui::Checkbox("Enabled##dls", &ssctEnabled)) {
                g_renderer->SetSsaoSsctEnabled(ssctEnabled);
              }
              if (ImGui::SliderFloat("Cone angle", &ssctLightConeRad, 0.0f,
                                     1.0f)) {
                g_renderer->SetSsaoSsctLightConeRad(ssctLightConeRad);
              }
              if (ImGui::SliderFloat("Shadow Distance", &shadowDist, 0.0f,
                                     10.0f)) {
                g_renderer->SetSsaoSsctShadowDistance(shadowDist);
              }
              if (ImGui::SliderFloat("Contact dist max", &contactDistMax, 0.0f,
                                     100.0f)) {
                g_renderer->SetSsaoSsctContactDistanceMax(contactDistMax);
              }
              if (ImGui::SliderFloat("Intensity##dls", &ssctIntensity, 0.0f,
                                     10.0f)) {
                g_renderer->SetSsaoSsctIntensity(ssctIntensity);
              }
              if (ImGui::SliderFloat("Depth bias", &ssctDepthBias, 0.0f,
                                     1.0f)) {
                g_renderer->SetSsaoSsctDepthBias(ssctDepthBias);
              }
              if (ImGui::SliderFloat("Depth slope bias", &ssctDepthSlopeBias,
                                     0.0f, 1.0f)) {
                g_renderer->SetSsaoSsctDepthSlopeBias(ssctDepthSlopeBias);
              }
              if (ImGui::SliderInt("Sample Count", &sampleCount, 1, 32)) {
                g_renderer->SetSsaoSsctSampleCount(sampleCount);
              }
              // ImGuiExt::DirectionWidget("Direction##dls", lightDirection);
              if (ImGui::InputFloat3("Direction##dls", lightDirection)) {
                g_renderer->SetSsaoSsctLightDirection(lightDirection);
              }
            }
            ImGui::Unindent();
          }

          if (ImGui::CollapsingHeader("Screen-space reflections Options")) {
            float rayThickness =
                g_renderer->GetScreenSpaceReflectionsThickness();
            float bias = g_renderer->GetScreenSpaceReflectionsBias();
            float maxDist = g_renderer->GetScreenSpaceReflectionsMaxDistance();
            float stride = g_renderer->GetScreenSpaceReflectionsStride();

            if (ImGui::SliderFloat("Ray thickness", &rayThickness, 0.001f,
                                   0.2f)) {
              g_renderer->SetScreenSpaceReflectionsThickness(rayThickness);
            }
            if (ImGui::SliderFloat("Bias", &bias, 0.001f, 0.5f)) {
              g_renderer->SetScreenSpaceReflectionsBias(bias);
            }
            if (ImGui::SliderFloat("Max distance", &maxDist, 0.1f, 10.0f)) {
              g_renderer->SetScreenSpaceReflectionsMaxDistance(maxDist);
            }
            if (ImGui::SliderFloat("Stride", &stride, 1.0, 10.0f)) {
              g_renderer->SetScreenSpaceReflectionsStride(stride);
            }
          }

          if (ImGui::CollapsingHeader("Dynamic Resolution")) {
            bool enabled = g_renderer->IsDynamicResoultionEnabled();
            bool homogeneous =
                g_renderer->IsDynamicResoultionHomogeneousScaling();
            float minScale = g_renderer->GetDynamicResoultionMinScale();
            float maxScale = g_renderer->GetDynamicResoultionMaxScale();
            int quality = g_renderer->GetDynamicResoultionQuality();
            float sharpness = g_renderer->GetDynamicResoultionSharpness();

            if (ImGui::Checkbox("enabled", &enabled)) {
              g_renderer->SetDynamicResoultionEnabled(enabled);
            }
            if (ImGui::Checkbox("homogeneous", &homogeneous)) {
              g_renderer->SetDynamicResoultionHomogeneousScaling(homogeneous);
            }
            if (ImGui::SliderFloat("min. scale", &minScale, 0.25f, 1.0f)) {
              g_renderer->SetDynamicResoultionMinScale(minScale);
            }
            if (ImGui::SliderFloat("max. scale", &maxScale, 0.25f, 1.0f)) {
              g_renderer->SetDynamicResoultionMaxScale(maxScale);
            }
            if (ImGui::SliderInt("quality", &quality, 0, 3)) {
              g_renderer->SetDynamicResoultionQuality(quality);
            }
            if (ImGui::SliderFloat("sharpness", &sharpness, 0.0f, 1.0f)) {
              g_renderer->SetDynamicResoultionSharpness(sharpness);
            }
          }

          if (ImGui::CollapsingHeader("Light Settings")) {
            ImGui::Indent();
            if (ImGui::CollapsingHeader("Indirect light")) {
              float iblIntensity = g_scene->GetIBLIntensity();
              float iblRotation = g_scene->GetIBLRotation();
              if (ImGui::Button("Select IBL")) {
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
            if (ImGui::CollapsingHeader("Sunlight")) {
              vzm::VzLight::ShadowOptions sOpts = g_light->GetShadowOptions();
              float intensity = g_light->GetIntensity();
              float haloSize = g_light->GetSunHaloSize();
              float haloFallOff = g_light->GetSunHaloFalloff();
              float sunRadius = g_light->GetSunAngularRadius();
              float shadowFar = sOpts.shadowFar;

              if (ImGui::Checkbox("Enable sunlight", &g_lightEnabled)) {
              }

              if (ImGui::SliderFloat("Sun intensity", &intensity, 0.0f,
                                     150000.0f)) {
                g_light->SetIntensity(intensity);
              }
              if (ImGui::SliderFloat("Halo size", &haloSize, 1.01f, 40.0f)) {
                g_light->SetSunHaloSize(haloSize);
              }
              if (ImGui::SliderFloat("Halo falloff", &haloFallOff, 4.0f,
                                     1024.0f)) {
                g_light->SetSunHaloFalloff(haloFallOff);
              }
              if (ImGui::SliderFloat("Sun radius", &sunRadius, 0.1f, 10.0f)) {
                g_light->SetSunAngularRadius(sunRadius);
              }
              ImGui::Indent();
              if (ImGui::CollapsingHeader("Shadow")) {
                bool shadowEnabled = g_light->IsShadowCaster();
                int mapSize = sOpts.mapSize;
                bool stableShadows = sOpts.stable;
                bool enableLiSPSM = sOpts.lispsm;
                if (ImGui::Checkbox("Enable shadow", &shadowEnabled)) {
                  g_light->SetShadowCaster(shadowEnabled);
                }
                if (ImGui::SliderInt("Shadow map size", &mapSize, 32, 1024)) {
                  sOpts.mapSize = mapSize;
                  g_light->SetShadowOptions(sOpts);
                }
                if (ImGui::Checkbox("Stable Shadows", &stableShadows)) {
                  sOpts.stable = stableShadows;
                  g_light->SetShadowOptions(sOpts);
                }
                if (ImGui::Checkbox("Enable LiSPSM", &enableLiSPSM)) {
                  sOpts.lispsm = enableLiSPSM;
                  g_light->SetShadowOptions(sOpts);
                }

                if (ImGui::SliderFloat("Shadow Far", &shadowFar, 0.0f, 10.0f)) {
                  sOpts.shadowFar = shadowFar;
                  g_light->SetShadowOptions(sOpts);
                }

                // if (ImGui::CollapsingHeader("Shadow direction")) {
                float fSunLightDirection[3];
                g_light->GetDirection(fSunLightDirection);
                if (ImGui::InputFloat3("Light Direction", fSunLightDirection)) {
                  g_light->SetDirection(fSunLightDirection);
                }
                // glm::vec3 sunLightDirection(fSunLightDirection[0],
                //                             fSunLightDirection[1],
                //                             fSunLightDirection[2]);
                // glm::vec4 shadowTransform(
                //     sOpts.transform[0], sOpts.transform[1],
                //     sOpts.transform[2], sOpts.transform[3]);
                // glm::vec3 shadowDirection =
                //     shadowTransform * glm::vec4(sunLightDirection, 0.0f);
                //// ImGuiExt::DirectionWidget("Shadow direction",
                //// shadowDirection.v);
                // if (ImGui::InputFloat3("Shadow Direction",
                //                        (float*)&shadowDirection[0])) {
                //   shadowTransform = glm::normalize(glm::vec4(
                //       glm::cross(sunLightDirection, shadowDirection),
                //       glm::sqrt(length2(sunLightDirection) *
                //                 length2(shadowDirection)) +
                //           glm::dot(sunLightDirection, shadowDirection)));
                //   sOpts.transform[0] = shadowTransform[0];
                //   sOpts.transform[1] = shadowTransform[1];
                //   sOpts.transform[2] = shadowTransform[2];
                //   sOpts.transform[3] = shadowTransform[3];

                //  g_light->SetShadowOptions(sOpts);
                //};
                //}

                if (g_renderer->GetShadowType() ==
                    vzm::VzRenderer::ShadowType::VSM) {
                  bool elvsm = sOpts.vsm.elvsm;
                  float vsmBlur = sOpts.vsm.blurWidth;

                  if (ImGui::Checkbox("ELVSM", &elvsm)) {
                    sOpts.vsm.elvsm = elvsm;
                    g_light->SetShadowOptions(sOpts);
                  }
                  if (ImGui::SliderFloat("VSM blur", &vsmBlur, 0.0f, 125.0f)) {
                    sOpts.vsm.blurWidth = vsmBlur;
                    g_light->SetShadowOptions(sOpts);
                  }
                }

                int shadowCascades = sOpts.shadowCascades;
                bool enableContactShadows = sOpts.screenSpaceContactShadows;
                float splitPos0 = sOpts.cascadeSplitPositions[0];
                float splitPos1 = sOpts.cascadeSplitPositions[1];
                float splitPos2 = sOpts.cascadeSplitPositions[2];

                if (ImGui::SliderInt("Cascades", &shadowCascades, 1, 4)) {
                  sOpts.shadowCascades = shadowCascades;
                  g_light->SetShadowOptions(sOpts);
                }
                // ImGui::Checkbox("Debug cascades",
                //                 debug.getPropertyAddress<bool>(
                //                     "d.shadowmap.visualize_cascades"));
                if (ImGui::Checkbox("Enable contact shadows",
                                    &enableContactShadows)) {
                  sOpts.screenSpaceContactShadows = enableContactShadows;
                  g_light->SetShadowOptions(sOpts);
                }
                if (ImGui::SliderFloat("Split pos 0", &splitPos0, 0.0f, 1.0f)) {
                  sOpts.cascadeSplitPositions[0] = splitPos0;
                  g_light->SetShadowOptions(sOpts);
                }
                if (ImGui::SliderFloat("Split pos 1", &splitPos1, 0.0f, 1.0f)) {
                  sOpts.cascadeSplitPositions[1] = splitPos1;
                  g_light->SetShadowOptions(sOpts);
                }
                if (ImGui::SliderFloat("Split pos 2", &splitPos2, 0.0f, 1.0f)) {
                  sOpts.cascadeSplitPositions[2] = splitPos2;
                  g_light->SetShadowOptions(sOpts);
                }
              }

              ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("Shadow Settings")) {
              int shadowType = (int)g_renderer->GetShadowType();

              if (ImGui::Combo("Shadow type", &shadowType,
                               "PCF\0VSM\0DPCF\0PCSS\0PCFd\0\0")) {
                g_renderer->SetShadowType(
                    (vzm::VzRenderer::ShadowType)shadowType);
              }

              if (shadowType == (int)vzm::VzRenderer::ShadowType::VSM) {
                bool highPrecision = g_renderer->IsVsmHighPrecision();

                int vsmMSAASamples = g_renderer->GetVsmMsaaSamples();
                int vsmAnisotropy = g_renderer->GetVsmAnisotropy();
                bool vsmMipmapping = g_renderer->IsVsmMipmapping();

                ImGui::Checkbox("High precision", &highPrecision);
                ImGui::SliderInt("VSM MSAA samples", &vsmMSAASamples, 0, 3);
                ImGui::SliderInt("VSM anisotropy", &vsmAnisotropy, 0, 3);
                ImGui::Checkbox("VSM mipmapping", &vsmMipmapping);

              } else if (shadowType == (int)vzm::VzRenderer::ShadowType::DPCF ||
                         shadowType == (int)vzm::VzRenderer::ShadowType::PCSS) {
                float softShadowPenumbraScale =
                    g_renderer->GetSoftShadowPenumbraScale();
                float softShadowPenumbraRatioScale =
                    g_renderer->GetSoftShadowPenumbraRatioScale();

                if (ImGui::SliderFloat("Penumbra scale",
                                       &softShadowPenumbraScale, 0.0f,
                                       100.0f)) {
                  g_renderer->SetSoftShadowPenumbraScale(
                      softShadowPenumbraScale);
                }
                if (ImGui::SliderFloat("Penumbra Ratio scale",
                                       &softShadowPenumbraRatioScale, 1.0f,
                                       100.0f)) {
                  g_renderer->SetSoftShadowPenumbraRatioScale(
                      softShadowPenumbraRatioScale);
                }
              }
            }
            ImGui::Unindent();
          }

          if (ImGui::CollapsingHeader("Fog")) {
            ImGui::Indent();
            bool isFogEnabled = g_renderer->IsFogEnabled();
            float fogDist = g_renderer->GetFogDistance();
            float fogDensity = g_renderer->GetFogDensity();
            float fogHeight = g_renderer->GetFogHeight();
            float fogHeightFalloff = g_renderer->GetFogHeightFalloff();
            float fogInScatteringStart = g_renderer->GetFogInScatteringStart();
            float fogInScatteringSize = g_renderer->GetFogInScatteringSize();
            bool isFogExcludeSkybox = g_renderer->IsFogExcludeSkybox();
            int fogColorSource = (int)g_renderer->GetFogColorSource();
            float fogColor[3];
            g_renderer->GetFogColor(fogColor);

            if (ImGui::Checkbox("Enable large-scale fog", &isFogEnabled)) {
              g_renderer->SetFogEnabled(isFogEnabled);
            }
            if (ImGui::SliderFloat("Start [m]", &fogDist, 0.0f, 100.0f)) {
              g_renderer->SetFogDistance(fogDist);
            }
            if (ImGui::SliderFloat("Extinction [1/m]", &fogDensity, 0.0f,
                                   1.0f)) {
              g_renderer->SetFogDensity(fogDensity);
            }
            if (ImGui::SliderFloat("Floor [m]", &fogHeight, 0.0f, 100.0f)) {
              g_renderer->SetFogHeight(fogHeight);
            }
            if (ImGui::SliderFloat("Height falloff [1/m]", &fogHeightFalloff,
                                   0.0f, 4.0f)) {
              g_renderer->SetFogHeightFalloff(fogHeightFalloff);
            }
            if (ImGui::SliderFloat("Sun Scattering start [m]",
                                   &fogInScatteringStart, 0.0f, 100.0f)) {
              g_renderer->SetFogInScatteringStart(fogInScatteringStart);
            }
            if (ImGui::SliderFloat("Sun Scattering size", &fogInScatteringSize,
                                   0.1f, 100.0f)) {
              g_renderer->SetFogInScatteringSize(fogInScatteringSize);
            }
            if (ImGui::Checkbox("Exclude Skybox", &isFogExcludeSkybox)) {
              g_renderer->SetFogExcludeSkybox(isFogExcludeSkybox);
            }
            if (ImGui::Combo("Color##fogColor", &fogColorSource,
                             "Constant\0IBL\0Skybox\0\0")) {
              g_renderer->SetFogColorSource(
                  (vzm::VzRenderer::FogColorSource)fogColorSource);
            }
            if (ImGui::ColorPicker3("Color", fogColor)) {
              g_renderer->SetFogColor(fogColor);
            }
            ImGui::Unindent();
          }

          //if (ImGui::CollapsingHeader("Scene")) {
          //   ImGui::Indent();
          //   vzm::VzRenderer::ClearOptions clearOptions;
          //   g_renderer->GetClearOptions(clearOptions);
          //   if (ImGui::Checkbox("Scale to unit cube", &bAny)) {
          //   }
          //   ImGui::Checkbox("Automatic instancing", &bAny);
          //   ImGui::Checkbox("Show skybox", &bAny);
          //   ImGui::ColorEdit3("Background color", anyVec);
          //  // We do not yet support ground shadow or scene selection in remote mode.
          //   if (true) {
          //     ImGui::Checkbox("Ground shadow", &bAny);
          //     ImGui::Indent();
          //     ImGui::SliderFloat("Strength", &any, 0.0f, 1.0f);
          //     ImGui::Unindent();
          //     
          //    // if (mAsset->getSceneCount() > 1) {
          //    //   ImGui::Separator();
          //    //   sceneSelectionUI();
          //    // }
          //  }
          //   g_renderer->SetClearOptions(clearOptions);
          //   ImGui::Unindent();
          //}

          if (ImGui::CollapsingHeader("Camera")) {
             ImGui::Indent();
             if (ImGui::CollapsingHeader("DoF")) {
               bool dofEnabled = g_renderer->IsDofEnabled();
               float dofCocScale = g_renderer->GetDofCocScale();
               float dofCocAspectRatio = g_renderer->GetDofCocAspectRatio();
               int dofRingCount = g_renderer->GetDofRingCount();
               int dofMaxCoc = g_renderer->GetDofMaxCoc();
               bool isDofNativeResolution = g_renderer->IsDofNativeResolution();
               bool isDofMedian = g_renderer->IsDofMedian();

               if (ImGui::Checkbox("Enabled##dofEnabled", &dofEnabled)) {
                 g_renderer->SetDofEnabled(dofEnabled);
               }
               //if (ImGui::SliderFloat("Focus distance", &any, 0.0f,
               //                       30.0f)) {
               //}
               if (ImGui::SliderFloat("Blur scale", &dofCocScale, 0.1f,
                                      10.0f)) {
                 g_renderer->SetDofCocScale(dofCocScale);
               }
               if (ImGui::SliderFloat("CoC aspect-ratio", &dofCocAspectRatio,
                                      0.25f, 4.0f)) {
                 g_renderer->SetDofCocAspectRatio(dofCocAspectRatio);
               }
               if (ImGui::SliderInt("Ring count", &dofRingCount, 1, 17)) {
                 g_renderer->SetDofRingCount(dofRingCount);
               }
               if (ImGui::SliderInt("Max CoC", &dofMaxCoc, 1, 32)) {
                 g_renderer->SetDofMaxCoc(dofMaxCoc);
               }
               if (ImGui::Checkbox("Native Resolution",
                                   &isDofNativeResolution)) {
                 g_renderer->SetDofNativeResolution(isDofNativeResolution);
               }
               if (ImGui::Checkbox("Median Filter", &isDofMedian)) {
                 g_renderer->SetDofMedian(isDofMedian);
               }
             }
             if (ImGui::CollapsingHeader("Vignette")) {
               bool vignetteEnabled = g_renderer->IsVignetteEnabled();
               float midPoint = g_renderer->GetVignetteMidPoint();
               float roundness = g_renderer->GetVignetteRoundness();
               float feather = g_renderer->GetVignetteFeather();
               float color[3];
               g_renderer->GetVignetteColor(color);

               if (ImGui::Checkbox("Enabled##vignetteEnabled",
                                   &vignetteEnabled)) {
                 g_renderer->SetVignetteEnabled(vignetteEnabled);
               }
               if (ImGui::SliderFloat("Mid point", &midPoint, 0.0f, 1.0f)) {
                 g_renderer->SetVignetteMidPoint(midPoint);
               }
               if (ImGui::SliderFloat("Roundness", &roundness, 0.0f, 1.0f)) {
                 g_renderer->SetVignetteRoundness(roundness);
               }
               if (ImGui::SliderFloat("Feather", &feather, 0.0f, 1.0f)) {
                 g_renderer->SetVignetteFeather(feather);
               }
               if (ImGui::ColorEdit3("Color##vignetteColor", color)) {
                 g_renderer->SetVignetteColor(color);
               }
             }

             ImGui::Unindent();
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
