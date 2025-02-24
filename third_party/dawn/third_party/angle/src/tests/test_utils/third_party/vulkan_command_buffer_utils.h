/*
 * Vulkan Samples
 *
 * Copyright (C) 2015-2016 Valve Corporation
 * Copyright (C) 2015-2016 LunarG, Inc.
 * Copyright (C) 2015-2018 Google, Inc.
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

/*
VULKAN_SAMPLE_DESCRIPTION
samples utility functions
*/

#ifndef VULKAN_COMMAND_BUFFER_UTILS_H
#define VULKAN_COMMAND_BUFFER_UTILS_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "common/debug.h"

// glslang has issues with some specific warnings.
ANGLE_DISABLE_EXTRA_SEMI_WARNING

// Pull in upstream glslang header (http://anglebug.com/42266837)
#include "glslang/Public/ShaderLang.h"

ANGLE_REENABLE_EXTRA_SEMI_WARNING

#ifdef _WIN32
#    pragma comment(linker, "/subsystem:console")
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    ifndef NOMINMAX
#        define NOMINMAX /* Don't let Windows define min() or max() */
#    endif
#    define APP_NAME_STR_LEN 80
#elif defined(__ANDROID__)
// Include files for Android
#    include <android/log.h>
#    include <unistd.h>
#    include "util/OSWindow.h"
#    include "util/android/third_party/android_native_app_glue.h"
#else
#    include <unistd.h>
#    include "vulkan/vk_sdk_platform.h"
#endif

#include "common/vulkan/vk_headers.h"

/* Number of descriptor sets needs to be the same at alloc,       */
/* pipeline layout creation, and descriptor set layout creation   */
#define NUM_DESCRIPTOR_SETS 1

/* Number of samples needs to be the same at image creation,      */
/* renderpass creation and pipeline creation.                     */
#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

/* Number of viewports and number of scissors have to be the same */
/* at pipeline creation and in any call to set them dynamically   */
/* They also have to be the same as each other                    */
#define NUM_VIEWPORTS 1
#define NUM_SCISSORS NUM_VIEWPORTS

/* Amount of time, in nanoseconds, to wait for a command buffer to complete */
#define FENCE_TIMEOUT 100000000

#ifdef __ANDROID__
#    define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "VK-SAMPLE", __VA_ARGS__))
#    define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "VK-SAMPLE", __VA_ARGS__))
#    define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                               \
        {                                                                          \
            info.fp##entrypoint =                                                  \
                (PFN_vk##entrypoint)vkGetInstanceProcAddr(inst, "vk" #entrypoint); \
            if (info.fp##entrypoint == NULL)                                       \
            {                                                                      \
                std::cout << "vkGetDeviceProcAddr failed to find vk" #entrypoint;  \
                exit(-1);                                                          \
            }                                                                      \
        }

#endif

/*
 * Keep each of our swap chain buffers' image, command buffer and view in one
 * spot
 */
typedef struct _swap_chain_buffers
{
    VkImage image;
    VkImageView view;
    VkDeviceMemory mem;
} swap_chain_buffer;

/*
 * A layer can expose extensions, keep track of those
 * extensions here.
 */
typedef struct
{
    VkLayerProperties properties;
    std::vector<VkExtensionProperties> instance_extensions;
    std::vector<VkExtensionProperties> device_extensions;
} layer_properties;

/*
 * Structure for tracking information used / created / modified
 * by utility functions.
 */
struct sample_info
{
#ifdef _WIN32
#    define APP_NAME_STR_LEN 80
    HINSTANCE connection;         // hInstance - Windows Instance
    char name[APP_NAME_STR_LEN];  // Name to put on the window/icon
    HWND window;                  // hWnd - window handle
#elif defined(__ANDROID__)
    OSWindow *mOSWindow;
    PFN_vkCreateAndroidSurfaceKHR fpCreateAndroidSurfaceKHR;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    wl_display *display;
    wl_registry *registry;
    wl_compositor *compositor;
    wl_surface *window;
    wl_shell *shell;
    wl_shell_surface *shell_surface;
#else
    xcb_connection_t *connection;
    xcb_screen_t *screen;
    xcb_window_t window;
    xcb_intern_atom_reply_t *atom_wm_delete_window;
#endif  // _WIN32
    VkSurfaceKHR surface;
    bool prepared;
    bool use_staging_buffer;
    bool save_images;

    std::vector<const char *> instance_layer_names;
    std::vector<const char *> instance_extension_names;
    std::vector<layer_properties> instance_layer_properties;
    std::vector<VkExtensionProperties> instance_extension_properties;
    VkInstance inst;

    std::vector<const char *> device_extension_names;
    std::vector<VkExtensionProperties> device_extension_properties;
    std::vector<VkPhysicalDevice> gpus;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    uint32_t graphics_queue_family_index;
    uint32_t present_queue_family_index;
    VkPhysicalDeviceProperties gpu_props;
    std::vector<VkQueueFamilyProperties> queue_props;
    VkPhysicalDeviceMemoryProperties memory_properties;

    VkFramebuffer *framebuffers;
    int width, height;
    VkFormat format;

    uint32_t swapchainImageCount;
    VkSwapchainKHR swap_chain;
    std::vector<swap_chain_buffer> buffers;
    VkSemaphore imageAcquiredSemaphore;

    VkCommandPool cmd_pool;

    struct
    {
        VkFormat format;

        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    } depth;

    struct
    {
        VkBuffer buf;
        VkDeviceMemory mem;
        VkDescriptorBufferInfo buffer_info;
    } uniform_data;

    struct
    {
        VkBuffer buf;
        VkDeviceMemory mem;
        VkDescriptorBufferInfo buffer_info;
    } vertex_buffer;
    VkVertexInputBindingDescription vi_binding;
    VkVertexInputAttributeDescription vi_attribs[2];

    std::vector<float> MVP;

    VkCommandBuffer cmd;                 // Buffer for initialization commands
    std::vector<VkCommandBuffer> cmds;   // Place to hold a lot of buffers
    std::vector<VkCommandBuffer> cmd2s;  // Place to hold a lot of 2nd buffers
    VkPipelineLayout pipeline_layout;
    std::vector<VkDescriptorSetLayout> desc_layout;
    VkPipelineCache pipelineCache;
    VkRenderPass render_pass;
    VkPipeline pipeline;

    VkPipelineShaderStageCreateInfo shaderStages[2];

    VkDescriptorPool desc_pool;
    std::vector<VkDescriptorSet> desc_set;

    PFN_vkCreateDebugReportCallbackEXT dbgCreateDebugReportCallback;
    PFN_vkDestroyDebugReportCallbackEXT dbgDestroyDebugReportCallback;
    PFN_vkDebugReportMessageEXT dbgBreakCallback;
    std::vector<VkDebugReportCallbackEXT> debug_report_callbacks;

    uint32_t current_buffer;
    uint32_t queue_family_count;

    VkViewport viewport;
    VkRect2D scissor;
};

struct Vertex
{
    float posX, posY, posZ, posW;  // Position data
    float r, g, b, a;              // Color
};

struct VertexUV
{
    float posX, posY, posZ, posW;  // Position data
    float u, v;                    // texture u,v
};

#define XYZ1(_x_, _y_, _z_) (_x_), (_y_), (_z_), 1.f
#define UV(_u_, _v_) (_u_), (_v_)

static const Vertex g_vbData[] = {
    {XYZ1(-1, -1, -1), XYZ1(0.f, 0.f, 0.f)}, {XYZ1(1, -1, -1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f)},  {XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(1, -1, -1), XYZ1(1.f, 0.f, 0.f)},  {XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f)},

    {XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f)},  {XYZ1(-1, 1, 1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 1.f)},   {XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(-1, 1, 1), XYZ1(0.f, 1.f, 1.f)},   {XYZ1(1, 1, 1), XYZ1(1.f, 1.f, 1.f)},

    {XYZ1(1, 1, 1), XYZ1(1.f, 1.f, 1.f)},    {XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 1.f)},   {XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f)},   {XYZ1(1, -1, -1), XYZ1(1.f, 0.f, 0.f)},

    {XYZ1(-1, 1, 1), XYZ1(0.f, 1.f, 1.f)},   {XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f)},  {XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f)},  {XYZ1(-1, -1, -1), XYZ1(0.f, 0.f, 0.f)},

    {XYZ1(1, 1, 1), XYZ1(1.f, 1.f, 1.f)},    {XYZ1(-1, 1, 1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f)},   {XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(-1, 1, 1), XYZ1(0.f, 1.f, 1.f)},   {XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f)},

    {XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 1.f)},   {XYZ1(1, -1, -1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f)},  {XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(1, -1, -1), XYZ1(1.f, 0.f, 0.f)},  {XYZ1(-1, -1, -1), XYZ1(0.f, 0.f, 0.f)},
};

static const Vertex g_vb_solid_face_colors_Data[] = {
    // red face
    {XYZ1(-1, -1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(1, 1, 1), XYZ1(1.f, 0.f, 0.f)},
    // green face
    {XYZ1(-1, -1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(1, 1, -1), XYZ1(0.f, 1.f, 0.f)},
    // blue face
    {XYZ1(-1, 1, 1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, -1, -1), XYZ1(0.f, 0.f, 1.f)},
    // yellow face
    {XYZ1(1, 1, 1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, -1, -1), XYZ1(1.f, 1.f, 0.f)},
    // magenta face
    {XYZ1(1, 1, 1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(-1, 1, -1), XYZ1(1.f, 0.f, 1.f)},
    // cyan face
    {XYZ1(1, -1, 1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(-1, -1, -1), XYZ1(0.f, 1.f, 1.f)},
};

static const VertexUV g_vb_texture_Data[] = {
    // left face
    {XYZ1(-1, -1, -1), UV(1.f, 0.f)},  // lft-top-front
    {XYZ1(-1, 1, 1), UV(0.f, 1.f)},    // lft-btm-back
    {XYZ1(-1, -1, 1), UV(0.f, 0.f)},   // lft-top-back
    {XYZ1(-1, 1, 1), UV(0.f, 1.f)},    // lft-btm-back
    {XYZ1(-1, -1, -1), UV(1.f, 0.f)},  // lft-top-front
    {XYZ1(-1, 1, -1), UV(1.f, 1.f)},   // lft-btm-front
    // front face
    {XYZ1(-1, -1, -1), UV(0.f, 0.f)},  // lft-top-front
    {XYZ1(1, -1, -1), UV(1.f, 0.f)},   // rgt-top-front
    {XYZ1(1, 1, -1), UV(1.f, 1.f)},    // rgt-btm-front
    {XYZ1(-1, -1, -1), UV(0.f, 0.f)},  // lft-top-front
    {XYZ1(1, 1, -1), UV(1.f, 1.f)},    // rgt-btm-front
    {XYZ1(-1, 1, -1), UV(0.f, 1.f)},   // lft-btm-front
    // top face
    {XYZ1(-1, -1, -1), UV(0.f, 1.f)},  // lft-top-front
    {XYZ1(1, -1, 1), UV(1.f, 0.f)},    // rgt-top-back
    {XYZ1(1, -1, -1), UV(1.f, 1.f)},   // rgt-top-front
    {XYZ1(-1, -1, -1), UV(0.f, 1.f)},  // lft-top-front
    {XYZ1(-1, -1, 1), UV(0.f, 0.f)},   // lft-top-back
    {XYZ1(1, -1, 1), UV(1.f, 0.f)},    // rgt-top-back
    // bottom face
    {XYZ1(-1, 1, -1), UV(0.f, 0.f)},  // lft-btm-front
    {XYZ1(1, 1, 1), UV(1.f, 1.f)},    // rgt-btm-back
    {XYZ1(-1, 1, 1), UV(0.f, 1.f)},   // lft-btm-back
    {XYZ1(-1, 1, -1), UV(0.f, 0.f)},  // lft-btm-front
    {XYZ1(1, 1, -1), UV(1.f, 0.f)},   // rgt-btm-front
    {XYZ1(1, 1, 1), UV(1.f, 1.f)},    // rgt-btm-back
    // right face
    {XYZ1(1, 1, -1), UV(0.f, 1.f)},   // rgt-btm-front
    {XYZ1(1, -1, 1), UV(1.f, 0.f)},   // rgt-top-back
    {XYZ1(1, 1, 1), UV(1.f, 1.f)},    // rgt-btm-back
    {XYZ1(1, -1, 1), UV(1.f, 0.f)},   // rgt-top-back
    {XYZ1(1, 1, -1), UV(0.f, 1.f)},   // rgt-btm-front
    {XYZ1(1, -1, -1), UV(0.f, 0.f)},  // rgt-top-front
    // back face
    {XYZ1(-1, 1, 1), UV(1.f, 1.f)},   // lft-btm-back
    {XYZ1(1, 1, 1), UV(0.f, 1.f)},    // rgt-btm-back
    {XYZ1(-1, -1, 1), UV(1.f, 0.f)},  // lft-top-back
    {XYZ1(-1, -1, 1), UV(1.f, 0.f)},  // lft-top-back
    {XYZ1(1, 1, 1), UV(0.f, 1.f)},    // rgt-btm-back
    {XYZ1(1, -1, 1), UV(0.f, 0.f)},   // rgt-top-back
};

inline void init_resources(TBuiltInResource &Resources)
{
    Resources.maxLights                                   = 32;
    Resources.maxClipPlanes                               = 6;
    Resources.maxTextureUnits                             = 32;
    Resources.maxTextureCoords                            = 32;
    Resources.maxVertexAttribs                            = 64;
    Resources.maxVertexUniformComponents                  = 4096;
    Resources.maxVaryingFloats                            = 64;
    Resources.maxVertexTextureImageUnits                  = 32;
    Resources.maxCombinedTextureImageUnits                = 80;
    Resources.maxTextureImageUnits                        = 32;
    Resources.maxFragmentUniformComponents                = 4096;
    Resources.maxDrawBuffers                              = 32;
    Resources.maxVertexUniformVectors                     = 128;
    Resources.maxVaryingVectors                           = 8;
    Resources.maxFragmentUniformVectors                   = 16;
    Resources.maxVertexOutputVectors                      = 16;
    Resources.maxFragmentInputVectors                     = 15;
    Resources.minProgramTexelOffset                       = -8;
    Resources.maxProgramTexelOffset                       = 7;
    Resources.maxClipDistances                            = 8;
    Resources.maxComputeWorkGroupCountX                   = 65535;
    Resources.maxComputeWorkGroupCountY                   = 65535;
    Resources.maxComputeWorkGroupCountZ                   = 65535;
    Resources.maxComputeWorkGroupSizeX                    = 1024;
    Resources.maxComputeWorkGroupSizeY                    = 1024;
    Resources.maxComputeWorkGroupSizeZ                    = 64;
    Resources.maxComputeUniformComponents                 = 1024;
    Resources.maxComputeTextureImageUnits                 = 16;
    Resources.maxComputeImageUniforms                     = 8;
    Resources.maxComputeAtomicCounters                    = 8;
    Resources.maxComputeAtomicCounterBuffers              = 1;
    Resources.maxVaryingComponents                        = 60;
    Resources.maxVertexOutputComponents                   = 64;
    Resources.maxGeometryInputComponents                  = 64;
    Resources.maxGeometryOutputComponents                 = 128;
    Resources.maxFragmentInputComponents                  = 128;
    Resources.maxImageUnits                               = 8;
    Resources.maxCombinedImageUnitsAndFragmentOutputs     = 8;
    Resources.maxCombinedShaderOutputResources            = 8;
    Resources.maxImageSamples                             = 0;
    Resources.maxVertexImageUniforms                      = 0;
    Resources.maxTessControlImageUniforms                 = 0;
    Resources.maxTessEvaluationImageUniforms              = 0;
    Resources.maxGeometryImageUniforms                    = 0;
    Resources.maxFragmentImageUniforms                    = 8;
    Resources.maxCombinedImageUniforms                    = 8;
    Resources.maxGeometryTextureImageUnits                = 16;
    Resources.maxGeometryOutputVertices                   = 256;
    Resources.maxGeometryTotalOutputComponents            = 1024;
    Resources.maxGeometryUniformComponents                = 1024;
    Resources.maxGeometryVaryingComponents                = 64;
    Resources.maxTessControlInputComponents               = 128;
    Resources.maxTessControlOutputComponents              = 128;
    Resources.maxTessControlTextureImageUnits             = 16;
    Resources.maxTessControlUniformComponents             = 1024;
    Resources.maxTessControlTotalOutputComponents         = 4096;
    Resources.maxTessEvaluationInputComponents            = 128;
    Resources.maxTessEvaluationOutputComponents           = 128;
    Resources.maxTessEvaluationTextureImageUnits          = 16;
    Resources.maxTessEvaluationUniformComponents          = 1024;
    Resources.maxTessPatchComponents                      = 120;
    Resources.maxPatchVertices                            = 32;
    Resources.maxTessGenLevel                             = 64;
    Resources.maxViewports                                = 16;
    Resources.maxVertexAtomicCounters                     = 0;
    Resources.maxTessControlAtomicCounters                = 0;
    Resources.maxTessEvaluationAtomicCounters             = 0;
    Resources.maxGeometryAtomicCounters                   = 0;
    Resources.maxFragmentAtomicCounters                   = 8;
    Resources.maxCombinedAtomicCounters                   = 8;
    Resources.maxAtomicCounterBindings                    = 1;
    Resources.maxVertexAtomicCounterBuffers               = 0;
    Resources.maxTessControlAtomicCounterBuffers          = 0;
    Resources.maxTessEvaluationAtomicCounterBuffers       = 0;
    Resources.maxGeometryAtomicCounterBuffers             = 0;
    Resources.maxFragmentAtomicCounterBuffers             = 1;
    Resources.maxCombinedAtomicCounterBuffers             = 1;
    Resources.maxAtomicCounterBufferSize                  = 16384;
    Resources.maxTransformFeedbackBuffers                 = 4;
    Resources.maxTransformFeedbackInterleavedComponents   = 64;
    Resources.maxCullDistances                            = 8;
    Resources.maxCombinedClipAndCullDistances             = 8;
    Resources.maxSamples                                  = 4;
    Resources.limits.nonInductiveForLoops                 = 1;
    Resources.limits.whileLoops                           = 1;
    Resources.limits.doWhileLoops                         = 1;
    Resources.limits.generalUniformIndexing               = 1;
    Resources.limits.generalAttributeMatrixVectorIndexing = 1;
    Resources.limits.generalVaryingIndexing               = 1;
    Resources.limits.generalSamplerIndexing               = 1;
    Resources.limits.generalVariableIndexing              = 1;
    Resources.limits.generalConstantMatrixVectorIndexing  = 1;
}

inline EShLanguage FindLanguage(const VkShaderStageFlagBits shader_type)
{
    switch (shader_type)
    {
        case VK_SHADER_STAGE_VERTEX_BIT:
            return EShLangVertex;

        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            return EShLangTessControl;

        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            return EShLangTessEvaluation;

        case VK_SHADER_STAGE_GEOMETRY_BIT:
            return EShLangGeometry;

        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return EShLangFragment;

        case VK_SHADER_STAGE_COMPUTE_BIT:
            return EShLangCompute;

        default:
            return EShLangVertex;
    }
}

VkResult init_global_extension_properties(layer_properties &layer_props);

VkResult init_global_layer_properties(sample_info &info);

VkResult init_device_extension_properties(struct sample_info &info, layer_properties &layer_props);

void init_instance_extension_names(struct sample_info &info);
VkResult init_instance(struct sample_info &info, char const *const app_short_name);
void init_device_extension_names(struct sample_info &info);
VkResult init_device(struct sample_info &info);
VkResult init_enumerate_device(struct sample_info &info, uint32_t gpu_count = 1);
VkBool32 demo_check_layers(const std::vector<layer_properties> &layer_props,
                           const std::vector<const char *> &layer_names);
void init_connection(struct sample_info &info);
void init_window(struct sample_info &info);
void init_swapchain_extension(struct sample_info &info);
void init_command_pool(struct sample_info &info, VkCommandPoolCreateFlags cmd_pool_create_flags);
void init_command_buffer(struct sample_info &info);
void init_command_buffer_array(struct sample_info &info, int numBuffers);
void init_command_buffer2_array(struct sample_info &info, int numBuffers);
void execute_begin_command_buffer(struct sample_info &info);
void init_device_queue(struct sample_info &info);
void init_swap_chain(struct sample_info &info,
                     VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
void init_depth_buffer(struct sample_info &info);
void init_uniform_buffer(struct sample_info &info);
void init_descriptor_and_pipeline_layouts(
    struct sample_info &info,
    bool use_texture,
    VkDescriptorSetLayoutCreateFlags descSetLayoutCreateFlags = 0);
void init_renderpass(struct sample_info &info,
                     bool include_depth,
                     bool clear                = true,
                     VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
void init_vertex_buffer(struct sample_info &info,
                        const void *vertexData,
                        uint32_t dataSize,
                        uint32_t dataStride,
                        bool use_texture);
void init_framebuffers(struct sample_info &info, bool include_depth);
void init_descriptor_pool(struct sample_info &info, bool use_texture);
void init_descriptor_set(struct sample_info &info);
void init_shaders(struct sample_info &info, const char *vertShaderText, const char *fragShaderText);
void init_pipeline_cache(struct sample_info &info);
void init_pipeline(struct sample_info &info, VkBool32 include_depth, VkBool32 include_vi = true);
void init_sampler(struct sample_info &info, VkSampler &sampler);
void init_viewports(struct sample_info &info);
void init_viewports_array(struct sample_info &info, int index);
void init_viewports2_array(struct sample_info &info, int index);
void init_scissors(struct sample_info &info);
void init_scissors_array(struct sample_info &info, int index);
void init_scissors2_array(struct sample_info &info, int index);
void init_window_size(struct sample_info &info, int32_t default_width, int32_t default_height);
void destroy_pipeline(struct sample_info &info);
void destroy_pipeline_cache(struct sample_info &info);
void destroy_descriptor_pool(struct sample_info &info);
void destroy_vertex_buffer(struct sample_info &info);
void destroy_framebuffers(struct sample_info &info);
void destroy_shaders(struct sample_info &info);
void destroy_renderpass(struct sample_info &info);
void destroy_descriptor_and_pipeline_layouts(struct sample_info &info);
void destroy_uniform_buffer(struct sample_info &info);
void destroy_depth_buffer(struct sample_info &info);
void destroy_swap_chain(struct sample_info &info);
void destroy_command_buffer(struct sample_info &info);
void destroy_command_buffer_array(struct sample_info &info, int numBuffers);
void destroy_command_buffer2_array(struct sample_info &info, int numBuffers);
void reset_command_buffer2_array(struct sample_info &info,
                                 VkCommandBufferResetFlags cmd_buffer_reset_flags);
void reset_command_pool(struct sample_info &info, VkCommandPoolResetFlags cmd_pool_reset_flags);
void destroy_command_pool(struct sample_info &info);
void destroy_device(struct sample_info &info);
void destroy_instance(struct sample_info &info);
void destroy_window(struct sample_info &info);

#endif
