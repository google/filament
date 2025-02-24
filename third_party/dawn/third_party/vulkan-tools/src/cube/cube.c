/*
 * Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
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
 *
 * Author: Chia-I Wu <olv@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Ian Elliott <ian@LunarG.com>
 * Author: Ian Elliott <ianelliott@google.com>
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Gwan-gyeong Mun <elongbug@gmail.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Bill Hollings <bill.hollings@brenwill.com>
 */

#define _GNU_SOURCE
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <signal.h>
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
#include <errno.h>
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
#include "xlib_loader.h"
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
#include "xcb_loader.h"
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include <linux/input.h>
#include "wayland_loader.h"
#endif

#ifdef _WIN32
#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows")
#endif  // MSVC
#define APP_NAME_STR_LEN 80
#endif  // _WIN32

// Volk requires VK_NO_PROTOTYPES before including vulkan.h
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#define VOLK_IMPLEMENTATION
#include "volk.h"

#include "linmath.h"
#include "object_type_string_helper.h"

#include "gettime.h"
#include "inttypes.h"
#define MILLION 1000000L
#define BILLION 1000000000L

#define DEMO_TEXTURE_COUNT 1
#define APP_SHORT_NAME "vkcube"
#define APP_LONG_NAME "Vulkan Cube"

// Allow a maximum of two outstanding presentation operations.
#define FRAME_LAG 2

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#if defined(NDEBUG) && defined(__GNUC__)
#define U_ASSERT_ONLY __attribute__((unused))
#else
#define U_ASSERT_ONLY
#endif

#if defined(__GNUC__)
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#ifdef _WIN32
bool in_callback = false;
#define ERR_EXIT(err_msg, err_class)                                             \
    do {                                                                         \
        if (!demo->suppress_popups) MessageBox(NULL, err_msg, err_class, MB_OK); \
        exit(1);                                                                 \
    } while (0)
void DbgMsg(char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vprintf(fmt, va);
    va_end(va);
    fflush(stdout);
}

#elif defined __ANDROID__
#include <android/log.h>
#define ERR_EXIT(err_msg, err_class)                                           \
    do {                                                                       \
        ((void)__android_log_print(ANDROID_LOG_INFO, "Vulkan Cube", err_msg)); \
        exit(1);                                                               \
    } while (0)
#ifdef VARARGS_WORKS_ON_ANDROID
void DbgMsg(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    __android_log_print(ANDROID_LOG_INFO, "Vulkan Cube", fmt, va);
    va_end(va);
}
#else  // VARARGS_WORKS_ON_ANDROID
#define DbgMsg(fmt, ...)                                                                  \
    do {                                                                                  \
        ((void)__android_log_print(ANDROID_LOG_INFO, "Vulkan Cube", fmt, ##__VA_ARGS__)); \
    } while (0)
#endif  // VARARGS_WORKS_ON_ANDROID
#else
#define ERR_EXIT(err_msg, err_class) \
    do {                             \
        printf("%s\n", err_msg);     \
        fflush(stdout);              \
        exit(1);                     \
    } while (0)
void DbgMsg(char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vprintf(fmt, va);
    va_end(va);
    fflush(stdout);
}
#endif

/*
 * structure to track all objects related to a texture.
 */
struct texture_object {
    VkSampler sampler;

    VkImage image;
    VkBuffer buffer;
    VkImageLayout imageLayout;

    VkMemoryAllocateInfo mem_alloc;
    VkDeviceMemory mem;
    VkImageView view;
    int32_t tex_width, tex_height;
};

static char *tex_files[] = {"lunarg.ppm"};

static int validation_error = 0;

struct vktexcube_vs_uniform {
    // Must start with MVP
    float mvp[4][4];
    float position[12 * 3][4];
    float attr[12 * 3][4];
};

//--------------------------------------------------------------------------------------
// Mesh and VertexFormat Data
//--------------------------------------------------------------------------------------
// clang-format off
static const float g_vertex_buffer_data[] = {
    -1.0f,-1.0f,-1.0f,  // -X side
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,

    -1.0f,-1.0f,-1.0f,  // -Z side
     1.0f, 1.0f,-1.0f,
     1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
     1.0f, 1.0f,-1.0f,

    -1.0f,-1.0f,-1.0f,  // -Y side
     1.0f,-1.0f,-1.0f,
     1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
     1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,

    -1.0f, 1.0f,-1.0f,  // +Y side
    -1.0f, 1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
     1.0f, 1.0f, 1.0f,
     1.0f, 1.0f,-1.0f,

     1.0f, 1.0f,-1.0f,  // +X side
     1.0f, 1.0f, 1.0f,
     1.0f,-1.0f, 1.0f,
     1.0f,-1.0f, 1.0f,
     1.0f,-1.0f,-1.0f,
     1.0f, 1.0f,-1.0f,

    -1.0f, 1.0f, 1.0f,  // +Z side
    -1.0f,-1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
     1.0f,-1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
};

static const float g_uv_buffer_data[] = {
    0.0f, 1.0f,  // -X side
    1.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,

    1.0f, 1.0f,  // -Z side
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,  // -Y side
    1.0f, 1.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,  // +Y side
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,

    1.0f, 0.0f,  // +X side
    0.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,

    0.0f, 0.0f,  // +Z side
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
};
// clang-format on

void dumpMatrix(const char *note, mat4x4 MVP) {
    int i;

    printf("%s: \n", note);
    for (i = 0; i < 4; i++) {
        printf("%f, %f, %f, %f\n", MVP[i][0], MVP[i][1], MVP[i][2], MVP[i][3]);
    }
    printf("\n");
    fflush(stdout);
}

void dumpVec4(const char *note, vec4 vector) {
    printf("%s: \n", note);
    printf("%f, %f, %f, %f\n", vector[0], vector[1], vector[2], vector[3]);
    printf("\n");
    fflush(stdout);
}

char const *to_string(VkPhysicalDeviceType const type) {
    switch (type) {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            return "Other";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return "IntegratedGpu";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return "DiscreteGpu";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return "VirtualGpu";
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return "Cpu";
        default:
            return "Unknown";
    }
}

typedef enum WSI_PLATFORM {
    WSI_PLATFORM_AUTO = 0,
    WSI_PLATFORM_WIN32,
    WSI_PLATFORM_METAL,
    WSI_PLATFORM_ANDROID,
    WSI_PLATFORM_QNX,
    WSI_PLATFORM_XCB,
    WSI_PLATFORM_XLIB,
    WSI_PLATFORM_WAYLAND,
    WSI_PLATFORM_DIRECTFB,
    WSI_PLATFORM_DISPLAY,
    WSI_PLATFORM_INVALID,  // Sentinel just to indicate invalid user input
} WSI_PLATFORM;

WSI_PLATFORM wsi_from_string(const char *str) {
    if (strcmp(str, "auto") == 0) return WSI_PLATFORM_AUTO;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (strcmp(str, "win32") == 0) return WSI_PLATFORM_WIN32;
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (strcmp(str, "metal") == 0) return WSI_PLATFORM_METAL;
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (strcmp(str, "android") == 0) return WSI_PLATFORM_ANDROID;
#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    if (strcmp(str, "qnx") == 0) return WSI_PLATFORM_QNX;
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (strcmp(str, "xcb") == 0) return WSI_PLATFORM_XCB;
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (strcmp(str, "xlib") == 0) return WSI_PLATFORM_XLIB;
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    if (strcmp(str, "wayland") == 0) return WSI_PLATFORM_WAYLAND;
#endif
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    if (strcmp(str, "directfb") == 0) return WSI_PLATFORM_DIRECTFB;
#endif
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
    if (strcmp(str, "display") == 0) return WSI_PLATFORM_DISPLAY;
#endif
    return WSI_PLATFORM_INVALID;
};

const char *wsi_to_string(WSI_PLATFORM wsi_platform) {
    switch (wsi_platform) {
        case (WSI_PLATFORM_AUTO):
            return "auto";
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        case (WSI_PLATFORM_WIN32):
            return "win32";
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
        case (WSI_PLATFORM_METAL):
            return "metal";
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        case (WSI_PLATFORM_ANDROID):
            return "android";
#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
        case (WSI_PLATFORM_QNX):
            return "qnx";
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
        case (WSI_PLATFORM_XCB):
            return "xcb";
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        case (WSI_PLATFORM_XLIB):
            return "xlib";
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
        case (WSI_PLATFORM_WAYLAND):
            return "wayland";
#endif
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
        case (WSI_PLATFORM_DIRECTFB):
            return "directfb";
#endif
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
        case (WSI_PLATFORM_DISPLAY):
            return "display";
#endif
        default:
            return "unknown";
    }
};

typedef struct {
    VkImage image;
    VkCommandBuffer cmd;
    VkCommandBuffer graphics_to_present_cmd;
    VkImageView view;
    VkBuffer uniform_buffer;
    VkDeviceMemory uniform_memory;
    void *uniform_memory_ptr;
    VkFramebuffer framebuffer;
    VkDescriptorSet descriptor_set;
} SwapchainImageResources;

struct demo {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
#define APP_NAME_STR_LEN 80
    HINSTANCE connection;         // hInstance - Windows Instance
    char name[APP_NAME_STR_LEN];  // Name to put on the window/icon
    HWND window;                  // hWnd - window handle
    POINT minsize;                // minimum window size
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    void *xlib_library;
    Display *xlib_display;
    Window xlib_window;
    Atom xlib_wm_delete_window;
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    void *xcb_library;
    Display *xcb_display;
    xcb_connection_t *connection;
    xcb_screen_t *screen;
    xcb_window_t xcb_window;
    xcb_intern_atom_reply_t *atom_wm_delete_window;
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    void *wayland_library;  // Dynamic library for wayland
    struct wl_display *wayland_display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_surface *window;
    struct xdg_wm_base *xdg_wm_base;
    struct zxdg_decoration_manager_v1 *xdg_decoration_mgr;
    struct zxdg_toplevel_decoration_v1 *toplevel_decoration;
    struct xdg_surface *xdg_surface;
    int xdg_surface_has_been_configured;
    struct xdg_toplevel *xdg_toplevel;
    struct wl_seat *seat;
    struct wl_pointer *pointer;
    struct wl_keyboard *keyboard;
#endif
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    IDirectFB *dfb;
    IDirectFBSurface *directfb_window;
    IDirectFBEventBuffer *event_buffer;
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    struct ANativeWindow *window;
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
    void *caMetalLayer;
#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    screen_context_t screen_context;
    screen_window_t screen_window;
    screen_event_t screen_event;
#endif
    WSI_PLATFORM wsi_platform;
    VkSurfaceKHR surface;
    bool prepared;
    bool use_staging_buffer;
    bool separate_present_queue;
    bool is_minimized;
    bool invalid_gpu_selection;
    int32_t gpu_number;

    bool VK_KHR_incremental_present_enabled;

    bool VK_GOOGLE_display_timing_enabled;
    bool syncd_with_actual_presents;
    uint64_t refresh_duration;
    uint64_t refresh_duration_multiplier;
    uint64_t target_IPD;  // image present duration (inverse of frame rate)
    uint64_t prev_desired_present_time;
    uint32_t next_present_id;
    uint32_t last_early_id;  // 0 if no early images
    uint32_t last_late_id;   // 0 if no late images

    VkInstance inst;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    uint32_t graphics_queue_family_index;
    uint32_t present_queue_family_index;
    VkSemaphore image_acquired_semaphores[FRAME_LAG];
    VkSemaphore draw_complete_semaphores[FRAME_LAG];
    VkSemaphore image_ownership_semaphores[FRAME_LAG];
    VkPhysicalDeviceProperties gpu_props;
    VkQueueFamilyProperties *queue_props;
    VkPhysicalDeviceMemoryProperties memory_properties;

    uint32_t enabled_extension_count;
    uint32_t enabled_layer_count;
    char *extension_names[64];
    char *enabled_layers[64];

    int width, height;
    VkFormat format;
    VkColorSpaceKHR color_space;

    uint32_t swapchainImageCount;
    VkSwapchainKHR swapchain;
    SwapchainImageResources *swapchain_image_resources;
    VkPresentModeKHR presentMode;
    VkFence fences[FRAME_LAG];
    int frame_index;
    bool first_swapchain_frame;

    VkCommandPool cmd_pool;
    VkCommandPool present_cmd_pool;

    struct {
        VkFormat format;

        VkImage image;
        VkMemoryAllocateInfo mem_alloc;
        VkDeviceMemory mem;
        VkImageView view;
    } depth;

    struct texture_object textures[DEMO_TEXTURE_COUNT];
    struct texture_object staging_texture;

    VkCommandBuffer cmd;  // Buffer for initialization commands
    VkPipelineLayout pipeline_layout;
    VkDescriptorSetLayout desc_layout;
    VkPipelineCache pipelineCache;
    VkRenderPass render_pass;
    VkPipeline pipeline;

    mat4x4 projection_matrix;
    mat4x4 view_matrix;
    mat4x4 model_matrix;

    float spin_angle;
    float spin_increment;
    bool pause;

    VkShaderModule vert_shader_module;
    VkShaderModule frag_shader_module;

    VkDescriptorPool desc_pool;

    bool quit;
    int32_t curFrame;
    int32_t frameCount;
    bool validate;
    bool use_break;
    bool suppress_popups;
    bool force_errors;

    VkDebugUtilsMessengerEXT dbg_messenger;

    uint32_t current_buffer;
    uint32_t queue_family_count;
};

VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData) {
    char prefix[64] = "";
    char *message = (char *)malloc(strlen(pCallbackData->pMessage) + 5000);
    assert(message);
    struct demo *demo = (struct demo *)pUserData;

    if (demo->use_break) {
#ifndef WIN32
        raise(SIGTRAP);
#else
        DebugBreak();
#endif
    }

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        strcat(prefix, "VERBOSE : ");
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        strcat(prefix, "INFO : ");
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        strcat(prefix, "WARNING : ");
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        strcat(prefix, "ERROR : ");
    }

    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        strcat(prefix, "GENERAL");
    } else {
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
            strcat(prefix, "VALIDATION");
            validation_error = 1;
        }
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
            if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
                strcat(prefix, "|");
            }
            strcat(prefix, "PERFORMANCE");
        }
    }

    sprintf(message, "%s - Message Id Number: %d | Message Id Name: %s\n\t%s\n", prefix, pCallbackData->messageIdNumber,
            pCallbackData->pMessageIdName == NULL ? "" : pCallbackData->pMessageIdName, pCallbackData->pMessage);
    if (pCallbackData->objectCount > 0) {
        char tmp_message[500];
        sprintf(tmp_message, "\n\tObjects - %d\n", pCallbackData->objectCount);
        strcat(message, tmp_message);
        for (uint32_t object = 0; object < pCallbackData->objectCount; ++object) {
            sprintf(tmp_message, "\t\tObject[%d] - %s", object, string_VkObjectType(pCallbackData->pObjects[object].objectType));
            strcat(message, tmp_message);

            VkObjectType t = pCallbackData->pObjects[object].objectType;
            if (t == VK_OBJECT_TYPE_INSTANCE || t == VK_OBJECT_TYPE_PHYSICAL_DEVICE || t == VK_OBJECT_TYPE_DEVICE ||
                t == VK_OBJECT_TYPE_COMMAND_BUFFER || t == VK_OBJECT_TYPE_QUEUE) {
                sprintf(tmp_message, ", Handle %p", (void *)(uintptr_t)(pCallbackData->pObjects[object].objectHandle));
                strcat(message, tmp_message);
            } else {
                sprintf(tmp_message, ", Handle Ox%" PRIx64, (pCallbackData->pObjects[object].objectHandle));
                strcat(message, tmp_message);
            }

            if (NULL != pCallbackData->pObjects[object].pObjectName && strlen(pCallbackData->pObjects[object].pObjectName) > 0) {
                sprintf(tmp_message, ", Name \"%s\"", pCallbackData->pObjects[object].pObjectName);
                strcat(message, tmp_message);
            }
            sprintf(tmp_message, "\n");
            strcat(message, tmp_message);
        }
    }
    if (pCallbackData->cmdBufLabelCount > 0) {
        char tmp_message[500];
        sprintf(tmp_message, "\n\tCommand Buffer Labels - %d\n", pCallbackData->cmdBufLabelCount);
        strcat(message, tmp_message);
        for (uint32_t cmd_buf_label = 0; cmd_buf_label < pCallbackData->cmdBufLabelCount; ++cmd_buf_label) {
            sprintf(tmp_message, "\t\tLabel[%d] - %s { %f, %f, %f, %f}\n", cmd_buf_label,
                    pCallbackData->pCmdBufLabels[cmd_buf_label].pLabelName, pCallbackData->pCmdBufLabels[cmd_buf_label].color[0],
                    pCallbackData->pCmdBufLabels[cmd_buf_label].color[1], pCallbackData->pCmdBufLabels[cmd_buf_label].color[2],
                    pCallbackData->pCmdBufLabels[cmd_buf_label].color[3]);
            strcat(message, tmp_message);
        }
    }

#ifdef _WIN32

    in_callback = true;
    if (!demo->suppress_popups) MessageBox(NULL, message, "Alert", MB_OK);
    in_callback = false;

#elif defined(ANDROID)

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        __android_log_print(ANDROID_LOG_INFO, APP_SHORT_NAME, "%s", message);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        __android_log_print(ANDROID_LOG_WARN, APP_SHORT_NAME, "%s", message);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        __android_log_print(ANDROID_LOG_ERROR, APP_SHORT_NAME, "%s", message);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        __android_log_print(ANDROID_LOG_VERBOSE, APP_SHORT_NAME, "%s", message);
    } else {
        __android_log_print(ANDROID_LOG_INFO, APP_SHORT_NAME, "%s", message);
    }

#else

    printf("%s\n", message);
    fflush(stdout);

#endif

    free(message);

    // Don't bail out, but keep going.
    return false;
}

bool ActualTimeLate(uint64_t desired, uint64_t actual, uint64_t rdur) {
    // The desired time was the earliest time that the present should have
    // occured.  In almost every case, the actual time should be later than the
    // desired time.  We should only consider the actual time "late" if it is
    // after "desired + rdur".
    if (actual <= desired) {
        // The actual time was before or equal to the desired time.  This will
        // probably never happen, but in case it does, return false since the
        // present was obviously NOT late.
        return false;
    }
    uint64_t deadline = desired + rdur;
    if (actual > deadline) {
        return true;
    } else {
        return false;
    }
}
bool CanPresentEarlier(uint64_t earliest, uint64_t actual, uint64_t margin, uint64_t rdur) {
    if (earliest < actual) {
        // Consider whether this present could have occured earlier.  Make sure
        // that earliest time was at least 2msec earlier than actual time, and
        // that the margin was at least 2msec:
        uint64_t diff = actual - earliest;
        if ((diff >= (2 * MILLION)) && (margin >= (2 * MILLION))) {
            // This present could have occured earlier because both: 1) the
            // earliest time was at least 2 msec before actual time, and 2) the
            // margin was at least 2msec.
            return true;
        }
    }
    return false;
}

// Forward declarations:
static void demo_resize(struct demo *demo);
static void demo_create_surface(struct demo *demo);

#if defined(__GNUC__) || defined(__clang__)
#define DECORATE_PRINTF(_fmt_argnum, _first_param_num) __attribute__((format(printf, _fmt_argnum, _first_param_num)))
#else
#define DECORATE_PRINTF(_fmt_num, _first_param_num)
#endif

DECORATE_PRINTF(4, 5)
static void demo_name_object(struct demo *demo, VkObjectType object_type, uint64_t vulkan_handle, const char *format, ...) {
    if (!demo->validate) {
        return;
    }
    VkResult U_ASSERT_ONLY err;
    char name[1024];
    va_list argptr;
    va_start(argptr, format);
    vsnprintf(name, sizeof(name), format, argptr);
    va_end(argptr);
    name[sizeof(name) - 1] = '\0';

    VkDebugUtilsObjectNameInfoEXT obj_name = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext = NULL,
        .objectType = object_type,
        .objectHandle = vulkan_handle,
        .pObjectName = name,
    };
    err = vkSetDebugUtilsObjectNameEXT(demo->device, &obj_name);
    assert(!err);
}

DECORATE_PRINTF(4, 5)
static void demo_push_cb_label(struct demo *demo, VkCommandBuffer cb, const float *color, const char *format, ...) {
    if (!demo->validate) {
        return;
    }
    char name[1024];
    va_list argptr;
    va_start(argptr, format);
    vsnprintf(name, sizeof(name), format, argptr);
    va_end(argptr);
    name[sizeof(name) - 1] = '\0';

    VkDebugUtilsLabelEXT label = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
        .pNext = NULL,
        .pLabelName = name,
    };
    if (color) {
        memcpy(label.color, color, sizeof(label.color));
    }

    vkCmdBeginDebugUtilsLabelEXT(cb, &label);
}

static void demo_pop_cb_label(struct demo *demo, VkCommandBuffer cb) {
    if (!demo->validate) {
        return;
    }
    vkCmdEndDebugUtilsLabelEXT(cb);
}

static bool memory_type_from_properties(struct demo *demo, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
        if ((typeBits & 1) == 1) {
            // Type is available, does it match user properties?
            if ((demo->memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    // No memory types matched, return failure
    return false;
}

static void demo_flush_init_cmd(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;

    // This function could get called twice if the texture uses a staging buffer
    // In that case the second call should be ignored
    if (demo->cmd == VK_NULL_HANDLE) return;

    err = vkEndCommandBuffer(demo->cmd);
    assert(!err);

    VkFence fence;
    VkFenceCreateInfo fence_ci = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext = NULL, .flags = 0};
    if (demo->force_errors) {
        // Remove sType to intentionally force validation layer errors.
        fence_ci.sType = 0;
    }
    err = vkCreateFence(demo->device, &fence_ci, NULL, &fence);
    assert(!err);
    demo_name_object(demo, VK_OBJECT_TYPE_FENCE, (uint64_t)fence, "InitFence");

    const VkCommandBuffer cmd_bufs[] = {demo->cmd};
    VkSubmitInfo submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                .pNext = NULL,
                                .waitSemaphoreCount = 0,
                                .pWaitSemaphores = NULL,
                                .pWaitDstStageMask = NULL,
                                .commandBufferCount = 1,
                                .pCommandBuffers = cmd_bufs,
                                .signalSemaphoreCount = 0,
                                .pSignalSemaphores = NULL};

    err = vkQueueSubmit(demo->graphics_queue, 1, &submit_info, fence);
    assert(!err);

    err = vkWaitForFences(demo->device, 1, &fence, VK_TRUE, UINT64_MAX);
    assert(!err);

    vkFreeCommandBuffers(demo->device, demo->cmd_pool, 1, cmd_bufs);
    vkDestroyFence(demo->device, fence, NULL);
    demo->cmd = VK_NULL_HANDLE;
}

static void demo_set_image_layout(struct demo *demo, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout,
                                  VkImageLayout new_image_layout, VkAccessFlagBits srcAccessMask, VkPipelineStageFlags src_stages,
                                  VkPipelineStageFlags dest_stages) {
    assert(demo->cmd);

    VkImageMemoryBarrier image_memory_barrier = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                                 .pNext = NULL,
                                                 .srcAccessMask = srcAccessMask,
                                                 .dstAccessMask = 0,
                                                 .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                                 .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                                 .oldLayout = old_image_layout,
                                                 .newLayout = new_image_layout,
                                                 .image = image,
                                                 .subresourceRange = {aspectMask, 0, 1, 0, 1}};

    switch (new_image_layout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            /* Make sure anything that was copying from this image has completed */
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            image_memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            break;

        default:
            image_memory_barrier.dstAccessMask = 0;
            break;
    }

    VkImageMemoryBarrier *pmemory_barrier = &image_memory_barrier;

    vkCmdPipelineBarrier(demo->cmd, src_stages, dest_stages, 0, 0, NULL, 0, NULL, 1, pmemory_barrier);
}

static void demo_draw_build_cmd(struct demo *demo, VkCommandBuffer cmd_buf) {
    const VkCommandBufferBeginInfo cmd_buf_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        .pInheritanceInfo = NULL,
    };
    const VkClearValue clear_values[2] = {
        [0] = {.color.float32 = {0.2f, 0.2f, 0.2f, 0.2f}},
        [1] = {.depthStencil = {1.0f, 0}},
    };
    const VkRenderPassBeginInfo rp_begin = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = NULL,
        .renderPass = demo->render_pass,
        .framebuffer = demo->swapchain_image_resources[demo->current_buffer].framebuffer,
        .renderArea.offset.x = 0,
        .renderArea.offset.y = 0,
        .renderArea.extent.width = demo->width,
        .renderArea.extent.height = demo->height,
        .clearValueCount = 2,
        .pClearValues = clear_values,
    };
    VkResult U_ASSERT_ONLY err;

    err = vkBeginCommandBuffer(cmd_buf, &cmd_buf_info);

    demo_name_object(demo, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)cmd_buf, "CubeDrawCommandBuf");

    const float begin_color[4] = {0.4f, 0.3f, 0.2f, 0.1f};
    demo_push_cb_label(demo, cmd_buf, begin_color, "DrawBegin");

    assert(!err);
    vkCmdBeginRenderPass(cmd_buf, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

    const float renderpass_color[4] = {8.4f, 7.3f, 6.2f, 7.1f};
    demo_push_cb_label(demo, cmd_buf, renderpass_color, "InsideRenderPass");

    vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, demo->pipeline);
    vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, demo->pipeline_layout, 0, 1,
                            &demo->swapchain_image_resources[demo->current_buffer].descriptor_set, 0, NULL);
    VkViewport viewport;
    memset(&viewport, 0, sizeof(viewport));
    float viewport_dimension;
    if (demo->width < demo->height) {
        viewport_dimension = (float)demo->width;
        viewport.y = (demo->height - demo->width) / 2.0f;
    } else {
        viewport_dimension = (float)demo->height;
        viewport.x = (demo->width - demo->height) / 2.0f;
    }
    viewport.height = viewport_dimension;
    viewport.width = viewport_dimension;
    viewport.minDepth = (float)0.0f;
    viewport.maxDepth = (float)1.0f;
    vkCmdSetViewport(cmd_buf, 0, 1, &viewport);

    VkRect2D scissor;
    memset(&scissor, 0, sizeof(scissor));
    scissor.extent.width = demo->width;
    scissor.extent.height = demo->height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(cmd_buf, 0, 1, &scissor);

    const float draw_color[4] = {-0.4f, -0.3f, -0.2f, -0.1f};
    demo_push_cb_label(demo, cmd_buf, draw_color, "ActualDraw");
    vkCmdDraw(cmd_buf, 12 * 3, 1, 0, 0);
    demo_pop_cb_label(demo, cmd_buf);

    // Note that ending the renderpass changes the image's layout from
    // COLOR_ATTACHMENT_OPTIMAL to PRESENT_SRC_KHR
    vkCmdEndRenderPass(cmd_buf);
    demo_pop_cb_label(demo, cmd_buf);

    if (demo->separate_present_queue) {
        // We have to transfer ownership from the graphics queue family to the
        // present queue family to be able to present.  Note that we don't have
        // to transfer from present queue family back to graphics queue family at
        // the start of the next frame because we don't care about the image's
        // contents at that point.
        VkImageMemoryBarrier image_ownership_barrier = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                                        .pNext = NULL,
                                                        .srcAccessMask = 0,
                                                        .dstAccessMask = 0,
                                                        .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                        .srcQueueFamilyIndex = demo->graphics_queue_family_index,
                                                        .dstQueueFamilyIndex = demo->present_queue_family_index,
                                                        .image = demo->swapchain_image_resources[demo->current_buffer].image,
                                                        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

        vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0,
                             NULL, 1, &image_ownership_barrier);
    }
    demo_pop_cb_label(demo, cmd_buf);
    err = vkEndCommandBuffer(cmd_buf);
    assert(!err);
}

void demo_build_image_ownership_cmd(struct demo *demo, int i) {
    VkResult U_ASSERT_ONLY err;

    const VkCommandBufferBeginInfo cmd_buf_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        .pInheritanceInfo = NULL,
    };
    err = vkBeginCommandBuffer(demo->swapchain_image_resources[i].graphics_to_present_cmd, &cmd_buf_info);
    assert(!err);

    VkImageMemoryBarrier image_ownership_barrier = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                                    .pNext = NULL,
                                                    .srcAccessMask = 0,
                                                    .dstAccessMask = 0,
                                                    .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                    .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                    .srcQueueFamilyIndex = demo->graphics_queue_family_index,
                                                    .dstQueueFamilyIndex = demo->present_queue_family_index,
                                                    .image = demo->swapchain_image_resources[i].image,
                                                    .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

    vkCmdPipelineBarrier(demo->swapchain_image_resources[i].graphics_to_present_cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &image_ownership_barrier);
    err = vkEndCommandBuffer(demo->swapchain_image_resources[i].graphics_to_present_cmd);
    assert(!err);
}

void demo_update_data_buffer(struct demo *demo) {
    mat4x4 MVP, Model, VP;
    int matrixSize = sizeof(MVP);

    mat4x4_mul(VP, demo->projection_matrix, demo->view_matrix);

    // Rotate around the Y axis
    mat4x4_dup(Model, demo->model_matrix);
    mat4x4_rotate_Y(demo->model_matrix, Model, (float)degreesToRadians(demo->spin_angle));
    mat4x4_orthonormalize(demo->model_matrix, demo->model_matrix);
    mat4x4_mul(MVP, VP, demo->model_matrix);

    memcpy(demo->swapchain_image_resources[demo->current_buffer].uniform_memory_ptr, (const void *)&MVP[0][0], matrixSize);
}

void DemoUpdateTargetIPD(struct demo *demo) {
    // Look at what happened to previous presents, and make appropriate
    // adjustments in timing:
    VkResult U_ASSERT_ONLY err;
    VkPastPresentationTimingGOOGLE *past = NULL;
    uint32_t count = 0;

    err = vkGetPastPresentationTimingGOOGLE(demo->device, demo->swapchain, &count, NULL);
    assert(!err);
    if (count) {
        past = (VkPastPresentationTimingGOOGLE *)malloc(sizeof(VkPastPresentationTimingGOOGLE) * count);
        assert(past);
        err = vkGetPastPresentationTimingGOOGLE(demo->device, demo->swapchain, &count, past);
        assert(!err);

        bool early = false;
        bool late = false;
        bool calibrate_next = false;
        for (uint32_t i = 0; i < count; i++) {
            if (!demo->syncd_with_actual_presents) {
                // This is the first time that we've received an
                // actualPresentTime for this swapchain.  In order to not
                // perceive these early frames as "late", we need to sync-up
                // our future desiredPresentTime's with the
                // actualPresentTime(s) that we're receiving now.
                calibrate_next = true;

                // So that we don't suspect any pending presents as late,
                // record them all as suspected-late presents:
                demo->last_late_id = demo->next_present_id - 1;
                demo->last_early_id = 0;
                demo->syncd_with_actual_presents = true;
                break;
            } else if (CanPresentEarlier(past[i].earliestPresentTime, past[i].actualPresentTime, past[i].presentMargin,
                                         demo->refresh_duration)) {
                // This image could have been presented earlier.  We don't want
                // to decrease the target_IPD until we've seen early presents
                // for at least two seconds.
                if (demo->last_early_id == past[i].presentID) {
                    // We've now seen two seconds worth of early presents.
                    // Flag it as such, and reset the counter:
                    early = true;
                    demo->last_early_id = 0;
                } else if (demo->last_early_id == 0) {
                    // This is the first early present we've seen.
                    // Calculate the presentID for two seconds from now.
                    uint64_t lastEarlyTime = past[i].actualPresentTime + (2 * BILLION);
                    uint32_t howManyPresents = (uint32_t)((lastEarlyTime - past[i].actualPresentTime) / demo->target_IPD);
                    demo->last_early_id = past[i].presentID + howManyPresents;
                } else {
                    // We are in the midst of a set of early images,
                    // and so we won't do anything.
                }
                late = false;
                demo->last_late_id = 0;
            } else if (ActualTimeLate(past[i].desiredPresentTime, past[i].actualPresentTime, demo->refresh_duration)) {
                // This image was presented after its desired time.  Since
                // there's a delay between calling vkQueuePresentKHR and when
                // we get the timing data, several presents may have been late.
                // Thus, we need to threat all of the outstanding presents as
                // being likely late, so that we only increase the target_IPD
                // once for all of those presents.
                if ((demo->last_late_id == 0) || (demo->last_late_id < past[i].presentID)) {
                    late = true;
                    // Record the last suspected-late present:
                    demo->last_late_id = demo->next_present_id - 1;
                } else {
                    // We are in the midst of a set of likely-late images,
                    // and so we won't do anything.
                }
                early = false;
                demo->last_early_id = 0;
            } else {
                // Since this image was not presented early or late, reset
                // any sets of early or late presentIDs:
                early = false;
                late = false;
                calibrate_next = true;
                demo->last_early_id = 0;
                demo->last_late_id = 0;
            }
        }

        if (early) {
            // Since we've seen at least two-seconds worth of presnts that
            // could have occured earlier than desired, let's decrease the
            // target_IPD (i.e. increase the frame rate):
            //
            // TODO(ianelliott): Try to calculate a better target_IPD based
            // on the most recently-seen present (this is overly-simplistic).
            demo->refresh_duration_multiplier--;
            if (demo->refresh_duration_multiplier == 0) {
                // This should never happen, but in case it does, don't
                // try to go faster.
                demo->refresh_duration_multiplier = 1;
            }
            demo->target_IPD = demo->refresh_duration * demo->refresh_duration_multiplier;
        }
        if (late) {
            // Since we found a new instance of a late present, we want to
            // increase the target_IPD (i.e. decrease the frame rate):
            //
            // TODO(ianelliott): Try to calculate a better target_IPD based
            // on the most recently-seen present (this is overly-simplistic).
            demo->refresh_duration_multiplier++;
            demo->target_IPD = demo->refresh_duration * demo->refresh_duration_multiplier;
        }

        if (calibrate_next) {
            int64_t multiple = demo->next_present_id - past[count - 1].presentID;
            demo->prev_desired_present_time = (past[count - 1].actualPresentTime + (multiple * demo->target_IPD));
        }
        free(past);
    }
}

static void demo_draw(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;

    // Ensure no more than FRAME_LAG renderings are outstanding
    vkWaitForFences(demo->device, 1, &demo->fences[demo->frame_index], VK_TRUE, UINT64_MAX);
    vkResetFences(demo->device, 1, &demo->fences[demo->frame_index]);

    do {
        // Get the index of the next available swapchain image:
        err = vkAcquireNextImageKHR(demo->device, demo->swapchain, UINT64_MAX, demo->image_acquired_semaphores[demo->frame_index],
                                    VK_NULL_HANDLE, &demo->current_buffer);

        if (err == VK_ERROR_OUT_OF_DATE_KHR) {
            // demo->swapchain is out of date (e.g. the window was resized) and
            // must be recreated:
            demo_resize(demo);
        } else if (err == VK_SUBOPTIMAL_KHR) {
            // demo->swapchain is not as optimal as it could be, but the platform's
            // presentation engine will still present the image correctly.
            break;
        } else if (err == VK_ERROR_SURFACE_LOST_KHR) {
            vkDestroySurfaceKHR(demo->inst, demo->surface, NULL);
            demo_create_surface(demo);
            demo_resize(demo);
        } else {
            assert(!err);
        }
    } while (err != VK_SUCCESS);

    demo_update_data_buffer(demo);

    if (demo->VK_GOOGLE_display_timing_enabled) {
        // Look at what happened to previous presents, and make appropriate
        // adjustments in timing:
        DemoUpdateTargetIPD(demo);

        // Note: a real application would position its geometry to that it's in
        // the correct locatoin for when the next image is presented.  It might
        // also wait, so that there's less latency between any input and when
        // the next image is rendered/presented.  This demo program is so
        // simple that it doesn't do either of those.
    }

    // Wait for the image acquired semaphore to be signaled to ensure
    // that the image won't be rendered to until the presentation
    // engine has fully released ownership to the application, and it is
    // okay to render to the image.
    VkPipelineStageFlags pipe_stage_flags;
    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submit_info.pWaitDstStageMask = &pipe_stage_flags;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &demo->image_acquired_semaphores[demo->frame_index];
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &demo->swapchain_image_resources[demo->current_buffer].cmd;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &demo->draw_complete_semaphores[demo->frame_index];
    err = vkQueueSubmit(demo->graphics_queue, 1, &submit_info, demo->fences[demo->frame_index]);
    assert(!err);

    if (demo->separate_present_queue) {
        // If we are using separate queues, change image ownership to the
        // present queue before presenting, waiting for the draw complete
        // semaphore and signalling the ownership released semaphore when finished
        VkFence nullFence = VK_NULL_HANDLE;
        pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &demo->draw_complete_semaphores[demo->frame_index];
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &demo->swapchain_image_resources[demo->current_buffer].graphics_to_present_cmd;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &demo->image_ownership_semaphores[demo->frame_index];
        err = vkQueueSubmit(demo->present_queue, 1, &submit_info, nullFence);
        assert(!err);
    }

    // If we are using separate queues we have to wait for image ownership,
    // otherwise wait for draw complete
    VkPresentInfoKHR present = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = NULL,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = (demo->separate_present_queue) ? &demo->image_ownership_semaphores[demo->frame_index]
                                                          : &demo->draw_complete_semaphores[demo->frame_index],
        .swapchainCount = 1,
        .pSwapchains = &demo->swapchain,
        .pImageIndices = &demo->current_buffer,
    };

    VkRectLayerKHR rect;
    VkPresentRegionKHR region;
    VkPresentRegionsKHR regions;
    if (demo->VK_KHR_incremental_present_enabled) {
        // If using VK_KHR_incremental_present, we provide a hint of the region
        // that contains changed content relative to the previously-presented
        // image.  The implementation can use this hint in order to save
        // work/power (by only copying the region in the hint).  The
        // implementation is free to ignore the hint though, and so we must
        // ensure that the entire image has the correctly-drawn content.
        uint32_t eighthOfWidth = demo->width / 8;
        uint32_t eighthOfHeight = demo->height / 8;

        if (demo->first_swapchain_frame) {
            rect.offset.x = 0;
            rect.offset.y = 0;
            rect.extent.width = demo->width;
            rect.extent.height = demo->height;
        } else {
            rect.offset.x = eighthOfWidth;
            rect.offset.y = eighthOfHeight;
            rect.extent.width = eighthOfWidth * 6;
            rect.extent.height = eighthOfHeight * 6;
        }
        rect.layer = 0;

        region.rectangleCount = 1;
        region.pRectangles = &rect;

        regions.sType = VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR;
        regions.pNext = present.pNext;
        regions.swapchainCount = present.swapchainCount;
        regions.pRegions = &region;
        present.pNext = &regions;
    }

    if (demo->VK_GOOGLE_display_timing_enabled) {
        VkPresentTimeGOOGLE ptime;
        if (demo->prev_desired_present_time == 0) {
            // This must be the first present for this swapchain.
            //
            // We don't know where we are relative to the presentation engine's
            // display's refresh cycle.  We also don't know how long rendering
            // takes.  Let's make a grossly-simplified assumption that the
            // desiredPresentTime should be half way between now and
            // now+target_IPD.  We will adjust over time.
            uint64_t curtime = getTimeInNanoseconds();
            if (curtime == 0) {
                // Since we didn't find out the current time, don't give a
                // desiredPresentTime:
                ptime.desiredPresentTime = 0;
            } else {
                ptime.desiredPresentTime = curtime + (demo->target_IPD >> 1);
            }
        } else {
            ptime.desiredPresentTime = (demo->prev_desired_present_time + demo->target_IPD);
        }
        ptime.presentID = demo->next_present_id++;
        demo->prev_desired_present_time = ptime.desiredPresentTime;

        VkPresentTimesInfoGOOGLE present_time = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE,
            .pNext = present.pNext,
            .swapchainCount = present.swapchainCount,
            .pTimes = &ptime,
        };
        if (demo->VK_GOOGLE_display_timing_enabled) {
            present.pNext = &present_time;
        }
    }

    err = vkQueuePresentKHR(demo->present_queue, &present);
    demo->frame_index += 1;
    demo->frame_index %= FRAME_LAG;
    demo->first_swapchain_frame = false;

    if (err == VK_ERROR_OUT_OF_DATE_KHR) {
        // demo->swapchain is out of date (e.g. the window was resized) and
        // must be recreated:
        demo_resize(demo);
    } else if (err == VK_SUBOPTIMAL_KHR) {
        // SUBOPTIMAL could be due to a resize
        VkSurfaceCapabilitiesKHR surfCapabilities;
        err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(demo->gpu, demo->surface, &surfCapabilities);
        assert(!err);
        if (surfCapabilities.currentExtent.width != (uint32_t)demo->width ||
            surfCapabilities.currentExtent.height != (uint32_t)demo->height) {
            demo_resize(demo);
        }
    } else if (err == VK_ERROR_SURFACE_LOST_KHR) {
        vkDestroySurfaceKHR(demo->inst, demo->surface, NULL);
        demo_create_surface(demo);
        demo_resize(demo);
    } else {
        assert(!err);
    }
}

static void demo_prepare_buffers(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;
    VkSwapchainKHR oldSwapchain = demo->swapchain;

    // Check the surface capabilities and formats
    VkSurfaceCapabilitiesKHR surfCapabilities;
    err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(demo->gpu, demo->surface, &surfCapabilities);
    assert(!err);

    uint32_t presentModeCount;
    err = vkGetPhysicalDeviceSurfacePresentModesKHR(demo->gpu, demo->surface, &presentModeCount, NULL);
    assert(!err);
    VkPresentModeKHR *presentModes = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
    assert(presentModes);
    err = vkGetPhysicalDeviceSurfacePresentModesKHR(demo->gpu, demo->surface, &presentModeCount, presentModes);
    assert(!err);

    VkExtent2D swapchainExtent;
    // width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
    if (surfCapabilities.currentExtent.width == 0xFFFFFFFF) {
        // If the surface size is undefined, the size is set to the size
        // of the images requested, which must fit within the minimum and
        // maximum values.
        swapchainExtent.width = demo->width;
        swapchainExtent.height = demo->height;

        if (swapchainExtent.width < surfCapabilities.minImageExtent.width) {
            swapchainExtent.width = surfCapabilities.minImageExtent.width;
        } else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width) {
            swapchainExtent.width = surfCapabilities.maxImageExtent.width;
        }

        if (swapchainExtent.height < surfCapabilities.minImageExtent.height) {
            swapchainExtent.height = surfCapabilities.minImageExtent.height;
        } else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height) {
            swapchainExtent.height = surfCapabilities.maxImageExtent.height;
        }
    } else {
        // If the surface size is defined, the swap chain size must match
        swapchainExtent = surfCapabilities.currentExtent;
        demo->width = surfCapabilities.currentExtent.width;
        demo->height = surfCapabilities.currentExtent.height;
    }

    if (surfCapabilities.maxImageExtent.width == 0 || surfCapabilities.maxImageExtent.height == 0) {
        demo->is_minimized = true;
        return;
    } else {
        demo->is_minimized = false;
    }

    // The FIFO present mode is guaranteed by the spec to be supported
    // and to have no tearing.  It's a great default present mode to use.
    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    //  There are times when you may wish to use another present mode.  The
    //  following code shows how to select them, and the comments provide some
    //  reasons you may wish to use them.
    //
    // It should be noted that Vulkan 1.0 doesn't provide a method for
    // synchronizing rendering with the presentation engine's display.  There
    // is a method provided for throttling rendering with the display, but
    // there are some presentation engines for which this method will not work.
    // If an application doesn't throttle its rendering, and if it renders much
    // faster than the refresh rate of the display, this can waste power on
    // mobile devices.  That is because power is being spent rendering images
    // that may never be seen.

    // VK_PRESENT_MODE_IMMEDIATE_KHR is for applications that don't care about
    // tearing, or have some way of synchronizing their rendering with the
    // display.
    // VK_PRESENT_MODE_MAILBOX_KHR may be useful for applications that
    // generally render a new presentable image every refresh cycle, but are
    // occasionally early.  In this case, the application wants the new image
    // to be displayed instead of the previously-queued-for-presentation image
    // that has not yet been displayed.
    // VK_PRESENT_MODE_FIFO_RELAXED_KHR is for applications that generally
    // render a new presentable image every refresh cycle, but are occasionally
    // late.  In this case (perhaps because of stuttering/latency concerns),
    // the application wants the late image to be immediately displayed, even
    // though that may mean some tearing.

    if (demo->presentMode != swapchainPresentMode) {
        for (size_t i = 0; i < presentModeCount; ++i) {
            if (presentModes[i] == demo->presentMode) {
                swapchainPresentMode = demo->presentMode;
                break;
            }
        }
    }
    if (swapchainPresentMode != demo->presentMode) {
        ERR_EXIT("Present mode specified is not supported\n", "Present mode unsupported");
    }

    // Determine the number of VkImages to use in the swap chain.
    // Application desires to acquire 3 images at a time for triple
    // buffering
    uint32_t desiredNumOfSwapchainImages = 3;
    if (desiredNumOfSwapchainImages < surfCapabilities.minImageCount) {
        desiredNumOfSwapchainImages = surfCapabilities.minImageCount;
    }
    // If maxImageCount is 0, we can ask for as many images as we want;
    // otherwise we're limited to maxImageCount
    if ((surfCapabilities.maxImageCount > 0) && (desiredNumOfSwapchainImages > surfCapabilities.maxImageCount)) {
        // Application must settle for fewer images than desired:
        desiredNumOfSwapchainImages = surfCapabilities.maxImageCount;
    }

    VkSurfaceTransformFlagsKHR preTransform;
    if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        preTransform = surfCapabilities.currentTransform;
    }

    // Find a supported composite alpha mode - one of these is guaranteed to be set
    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (uint32_t i = 0; i < ARRAY_SIZE(compositeAlphaFlags); i++) {
        if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i]) {
            compositeAlpha = compositeAlphaFlags[i];
            break;
        }
    }

    VkSwapchainCreateInfoKHR swapchain_ci = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .surface = demo->surface,
        .minImageCount = desiredNumOfSwapchainImages,
        .imageFormat = demo->format,
        .imageColorSpace = demo->color_space,
        .imageExtent =
            {
                .width = swapchainExtent.width,
                .height = swapchainExtent.height,
            },
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = preTransform,
        .compositeAlpha = compositeAlpha,
        .imageArrayLayers = 1,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL,
        .presentMode = swapchainPresentMode,
        .oldSwapchain = oldSwapchain,
        .clipped = true,
    };
    uint32_t i;
    err = vkCreateSwapchainKHR(demo->device, &swapchain_ci, NULL, &demo->swapchain);
    assert(!err);

    // If we just re-created an existing swapchain, we should destroy the old
    // swapchain at this point.
    // Note: destroying the swapchain also cleans up all its associated
    // presentable images once the platform is done with them.
    if (oldSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(demo->device, oldSwapchain, NULL);
    }

    err = vkGetSwapchainImagesKHR(demo->device, demo->swapchain, &demo->swapchainImageCount, NULL);
    assert(!err);

    VkImage *swapchainImages = (VkImage *)malloc(demo->swapchainImageCount * sizeof(VkImage));
    assert(swapchainImages);
    err = vkGetSwapchainImagesKHR(demo->device, demo->swapchain, &demo->swapchainImageCount, swapchainImages);
    assert(!err);

    demo->swapchain_image_resources =
        (SwapchainImageResources *)malloc(sizeof(SwapchainImageResources) * demo->swapchainImageCount);
    assert(demo->swapchain_image_resources);

    for (i = 0; i < demo->swapchainImageCount; i++) {
        demo_name_object(demo, VK_OBJECT_TYPE_IMAGE, (uint64_t)swapchainImages[i], "SwapchainImage(%u)", i);
    }
    for (i = 0; i < demo->swapchainImageCount; i++) {
        VkImageViewCreateInfo color_image_view = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .format = demo->format,
            .components =
                {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
            .subresourceRange =
                {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .flags = 0,
        };

        demo->swapchain_image_resources[i].image = swapchainImages[i];

        color_image_view.image = demo->swapchain_image_resources[i].image;

        err = vkCreateImageView(demo->device, &color_image_view, NULL, &demo->swapchain_image_resources[i].view);
        assert(!err);
        demo_name_object(demo, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)demo->swapchain_image_resources[i].view, "SwapchainView(%u)",
                         i);
    }

    if (demo->VK_GOOGLE_display_timing_enabled) {
        VkRefreshCycleDurationGOOGLE rc_dur;
        err = vkGetRefreshCycleDurationGOOGLE(demo->device, demo->swapchain, &rc_dur);
        assert(!err);
        demo->refresh_duration = rc_dur.refreshDuration;

        demo->syncd_with_actual_presents = false;
        // Initially target 1X the refresh duration:
        demo->target_IPD = demo->refresh_duration;
        demo->refresh_duration_multiplier = 1;
        demo->prev_desired_present_time = 0;
        demo->next_present_id = 1;
    }

    if (NULL != swapchainImages) {
        free(swapchainImages);
    }

    if (NULL != presentModes) {
        free(presentModes);
    }
}

static void demo_prepare_depth(struct demo *demo) {
    const VkFormat depth_format = VK_FORMAT_D16_UNORM;
    const VkImageCreateInfo image = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = NULL,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = depth_format,
        .extent = {demo->width, demo->height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .flags = 0,
    };

    VkImageViewCreateInfo view = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = NULL,
        .image = VK_NULL_HANDLE,
        .format = depth_format,
        .subresourceRange =
            {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
        .flags = 0,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
    };

    if (demo->force_errors) {
        // Intentionally force a bad pNext value to generate a validation layer error
        view.pNext = &image;
    }

    VkMemoryRequirements mem_reqs;
    VkResult U_ASSERT_ONLY err;
    bool U_ASSERT_ONLY pass;

    demo->depth.format = depth_format;

    /* create image */
    err = vkCreateImage(demo->device, &image, NULL, &demo->depth.image);
    assert(!err);
    demo_name_object(demo, VK_OBJECT_TYPE_IMAGE, (uint64_t)demo->depth.image, "DepthImage");

    vkGetImageMemoryRequirements(demo->device, demo->depth.image, &mem_reqs);
    assert(!err);

    demo->depth.mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    demo->depth.mem_alloc.pNext = NULL;
    demo->depth.mem_alloc.allocationSize = mem_reqs.size;
    demo->depth.mem_alloc.memoryTypeIndex = 0;

    pass = memory_type_from_properties(demo, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                       &demo->depth.mem_alloc.memoryTypeIndex);
    assert(pass);

    /* allocate memory */
    err = vkAllocateMemory(demo->device, &demo->depth.mem_alloc, NULL, &demo->depth.mem);
    assert(!err);
    demo_name_object(demo, VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)demo->depth.mem, "DepthMem");

    /* bind memory */
    err = vkBindImageMemory(demo->device, demo->depth.image, demo->depth.mem, 0);
    assert(!err);

    /* create image view */
    view.image = demo->depth.image;
    err = vkCreateImageView(demo->device, &view, NULL, &demo->depth.view);
    assert(!err);
    demo_name_object(demo, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)demo->depth.view, "DepthView");
}

/* Convert ppm image data from header file into RGBA texture image */
#include "lunarg.ppm.h"
bool loadTexture(const char *filename, uint8_t *rgba_data, VkSubresourceLayout *layout, int32_t *width, int32_t *height) {
    (void)filename;
    char *cPtr;
    cPtr = (char *)lunarg_ppm;
    if ((unsigned char *)cPtr >= (lunarg_ppm + lunarg_ppm_len) || strncmp(cPtr, "P6\n", 3)) {
        return false;
    }
    while (strncmp(cPtr++, "\n", 1))
        ;
    sscanf(cPtr, "%u %u", width, height);
    if (rgba_data == NULL) {
        return true;
    }
    while (strncmp(cPtr++, "\n", 1))
        ;
    if ((unsigned char *)cPtr >= (lunarg_ppm + lunarg_ppm_len) || strncmp(cPtr, "255\n", 4)) {
        return false;
    }
    while (strncmp(cPtr++, "\n", 1))
        ;
    for (int y = 0; y < *height; y++) {
        uint8_t *rowPtr = rgba_data;
        for (int x = 0; x < *width; x++) {
            memcpy(rowPtr, cPtr, 3);
            rowPtr[3] = 255; /* Alpha of 1 */
            rowPtr += 4;
            cPtr += 3;
        }
        rgba_data += layout->rowPitch;
    }
    return true;
}

static void demo_prepare_texture_buffer(struct demo *demo, const char *filename, struct texture_object *tex_obj) {
    int32_t tex_width;
    int32_t tex_height;
    VkResult U_ASSERT_ONLY err;
    bool U_ASSERT_ONLY pass;

    if (!loadTexture(filename, NULL, NULL, &tex_width, &tex_height)) {
        ERR_EXIT("Failed to load textures", "Load Texture Failure");
    }

    tex_obj->tex_width = tex_width;
    tex_obj->tex_height = tex_height;

    const VkBufferCreateInfo buffer_create_info = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                                   .pNext = NULL,
                                                   .flags = 0,
                                                   .size = tex_width * tex_height * 4,
                                                   .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                   .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                                   .queueFamilyIndexCount = 0,
                                                   .pQueueFamilyIndices = NULL};

    err = vkCreateBuffer(demo->device, &buffer_create_info, NULL, &tex_obj->buffer);
    assert(!err);
    demo_name_object(demo, VK_OBJECT_TYPE_BUFFER, (uint64_t)tex_obj->buffer, "TexBuffer(%s)", filename);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(demo->device, tex_obj->buffer, &mem_reqs);

    tex_obj->mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    tex_obj->mem_alloc.pNext = NULL;
    tex_obj->mem_alloc.allocationSize = mem_reqs.size;
    tex_obj->mem_alloc.memoryTypeIndex = 0;

    VkFlags requirements = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    pass = memory_type_from_properties(demo, mem_reqs.memoryTypeBits, requirements, &tex_obj->mem_alloc.memoryTypeIndex);
    assert(pass);

    err = vkAllocateMemory(demo->device, &tex_obj->mem_alloc, NULL, &(tex_obj->mem));
    assert(!err);
    demo_name_object(demo, VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)tex_obj->mem, "TexBufMemory(%s)", filename);

    /* bind memory */
    err = vkBindBufferMemory(demo->device, tex_obj->buffer, tex_obj->mem, 0);
    assert(!err);

    VkSubresourceLayout layout;
    memset(&layout, 0, sizeof(layout));
    layout.rowPitch = tex_width * 4;

    void *data;
    err = vkMapMemory(demo->device, tex_obj->mem, 0, tex_obj->mem_alloc.allocationSize, 0, &data);
    assert(!err);

    if (!loadTexture(filename, data, &layout, &tex_width, &tex_height)) {
        fprintf(stderr, "Error loading texture: %s\n", filename);
    }

    vkUnmapMemory(demo->device, tex_obj->mem);
}

static void demo_prepare_texture_image(struct demo *demo, const char *filename, struct texture_object *tex_obj,
                                       VkImageTiling tiling, VkImageUsageFlags usage, VkFlags required_props) {
    const VkFormat tex_format = VK_FORMAT_R8G8B8A8_SRGB;
    int32_t tex_width;
    int32_t tex_height;
    VkResult U_ASSERT_ONLY err;
    bool U_ASSERT_ONLY pass;

    if (!loadTexture(filename, NULL, NULL, &tex_width, &tex_height)) {
        ERR_EXIT("Failed to load textures", "Load Texture Failure");
    }

    tex_obj->tex_width = tex_width;
    tex_obj->tex_height = tex_height;

    const VkImageCreateInfo image_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = NULL,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = tex_format,
        .extent = {tex_width, tex_height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = tiling,
        .usage = usage,
        .flags = 0,
        .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
    };

    VkMemoryRequirements mem_reqs;

    err = vkCreateImage(demo->device, &image_create_info, NULL, &tex_obj->image);
    assert(!err);
    demo_name_object(demo, VK_OBJECT_TYPE_IMAGE, (uint64_t)tex_obj->image, "TexImage(%s)", filename);

    vkGetImageMemoryRequirements(demo->device, tex_obj->image, &mem_reqs);

    tex_obj->mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    tex_obj->mem_alloc.pNext = NULL;
    tex_obj->mem_alloc.allocationSize = mem_reqs.size;
    tex_obj->mem_alloc.memoryTypeIndex = 0;

    pass = memory_type_from_properties(demo, mem_reqs.memoryTypeBits, required_props, &tex_obj->mem_alloc.memoryTypeIndex);
    assert(pass);

    /* allocate memory */
    err = vkAllocateMemory(demo->device, &tex_obj->mem_alloc, NULL, &(tex_obj->mem));
    assert(!err);
    demo_name_object(demo, VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)tex_obj->mem, "TexImageMem(%s)", filename);

    /* bind memory */
    err = vkBindImageMemory(demo->device, tex_obj->image, tex_obj->mem, 0);
    assert(!err);

    if (required_props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        const VkImageSubresource subres = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .arrayLayer = 0,
        };
        VkSubresourceLayout layout;
        void *data;

        vkGetImageSubresourceLayout(demo->device, tex_obj->image, &subres, &layout);

        err = vkMapMemory(demo->device, tex_obj->mem, 0, tex_obj->mem_alloc.allocationSize, 0, &data);
        assert(!err);

        if (!loadTexture(filename, data, &layout, &tex_width, &tex_height)) {
            fprintf(stderr, "Error loading texture: %s\n", filename);
        }

        vkUnmapMemory(demo->device, tex_obj->mem);
    }

    tex_obj->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

static void demo_destroy_texture(struct demo *demo, struct texture_object *tex_objs) {
    /* clean up staging resources */
    vkFreeMemory(demo->device, tex_objs->mem, NULL);
    if (tex_objs->image) vkDestroyImage(demo->device, tex_objs->image, NULL);
    if (tex_objs->buffer) vkDestroyBuffer(demo->device, tex_objs->buffer, NULL);
}

static void demo_prepare_textures(struct demo *demo) {
    const VkFormat tex_format = VK_FORMAT_R8G8B8A8_SRGB;
    VkFormatProperties props;
    uint32_t i;

    vkGetPhysicalDeviceFormatProperties(demo->gpu, tex_format, &props);

    for (i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        VkResult U_ASSERT_ONLY err;

        if ((props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) && !demo->use_staging_buffer) {
            demo_push_cb_label(demo, demo->cmd, NULL, "DirectTexture(%u)", i);
            /* Device can texture using linear textures */
            demo_prepare_texture_image(demo, tex_files[i], &demo->textures[i], VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT,
                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            // Nothing in the pipeline needs to be complete to start, and don't allow fragment
            // shader to run until layout transition completes
            demo_set_image_layout(demo, demo->textures[i].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED,
                                  demo->textures[i].imageLayout, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            demo->staging_texture.image = 0;
            demo_pop_cb_label(demo, demo->cmd);  // "DirectTexture"
        } else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
            /* Must use staging buffer to copy linear texture to optimized */
            demo_push_cb_label(demo, demo->cmd, NULL, "StagingTexture(%u)", i);

            memset(&demo->staging_texture, 0, sizeof(demo->staging_texture));
            demo_prepare_texture_buffer(demo, tex_files[i], &demo->staging_texture);

            demo_prepare_texture_image(demo, tex_files[i], &demo->textures[i], VK_IMAGE_TILING_OPTIMAL,
                                       (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            demo_set_image_layout(demo, demo->textures[i].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT);

            demo_push_cb_label(demo, demo->cmd, NULL, "StagingBufferCopy(%u)", i);

            VkBufferImageCopy copy_region = {
                .bufferOffset = 0,
                .bufferRowLength = demo->staging_texture.tex_width,
                .bufferImageHeight = demo->staging_texture.tex_height,
                .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                .imageOffset = {0, 0, 0},
                .imageExtent = {demo->staging_texture.tex_width, demo->staging_texture.tex_height, 1},
            };

            vkCmdCopyBufferToImage(demo->cmd, demo->staging_texture.buffer, demo->textures[i].image,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
            demo_pop_cb_label(demo, demo->cmd);  // "StagingBufferCopy"

            demo_set_image_layout(demo, demo->textures[i].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  demo->textures[i].imageLayout, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            demo_pop_cb_label(demo, demo->cmd);  // "StagingTexture"

        } else {
            /* Can't support VK_FORMAT_R8G8B8A8_SRGB !? */
            assert(!"No support for R8G8B8A8_SRGB as texture image format");
        }

        const VkSamplerCreateInfo sampler = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = NULL,
            .magFilter = VK_FILTER_NEAREST,
            .minFilter = VK_FILTER_NEAREST,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_FALSE,
            .maxAnisotropy = 1,
            .compareOp = VK_COMPARE_OP_NEVER,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
            .unnormalizedCoordinates = VK_FALSE,
        };

        VkImageViewCreateInfo view = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .image = VK_NULL_HANDLE,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = tex_format,
            .components =
                {
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                },
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
            .flags = 0,
        };

        /* create sampler */
        err = vkCreateSampler(demo->device, &sampler, NULL, &demo->textures[i].sampler);
        assert(!err);
        demo_name_object(demo, VK_OBJECT_TYPE_SAMPLER, (uint64_t)demo->textures[i].sampler, "Sampler(%u)", i);

        /* create image view */
        view.image = demo->textures[i].image;
        err = vkCreateImageView(demo->device, &view, NULL, &demo->textures[i].view);
        demo_name_object(demo, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)demo->textures[i].view, "TexImageView(%u)", i);
        assert(!err);
    }
}

void demo_prepare_cube_data_buffers(struct demo *demo) {
    VkBufferCreateInfo buf_info;
    VkMemoryRequirements mem_reqs;
    VkMemoryAllocateInfo mem_alloc;
    mat4x4 MVP, VP;
    VkResult U_ASSERT_ONLY err;
    bool U_ASSERT_ONLY pass;
    struct vktexcube_vs_uniform data;

    mat4x4_mul(VP, demo->projection_matrix, demo->view_matrix);
    mat4x4_mul(MVP, VP, demo->model_matrix);
    memcpy(data.mvp, MVP, sizeof(MVP));
    //    dumpMatrix("MVP", MVP);

    for (unsigned int i = 0; i < 12 * 3; i++) {
        data.position[i][0] = g_vertex_buffer_data[i * 3];
        data.position[i][1] = g_vertex_buffer_data[i * 3 + 1];
        data.position[i][2] = g_vertex_buffer_data[i * 3 + 2];
        data.position[i][3] = 1.0f;
        data.attr[i][0] = g_uv_buffer_data[2 * i];
        data.attr[i][1] = g_uv_buffer_data[2 * i + 1];
        data.attr[i][2] = 0;
        data.attr[i][3] = 0;
    }

    memset(&buf_info, 0, sizeof(buf_info));
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buf_info.size = sizeof(data);

    for (unsigned int i = 0; i < demo->swapchainImageCount; i++) {
        err = vkCreateBuffer(demo->device, &buf_info, NULL, &demo->swapchain_image_resources[i].uniform_buffer);
        assert(!err);
        demo_name_object(demo, VK_OBJECT_TYPE_BUFFER, (uint64_t)demo->swapchain_image_resources[i].uniform_buffer,
                         "SwapchainUniformBuf(%u)", i);

        vkGetBufferMemoryRequirements(demo->device, demo->swapchain_image_resources[i].uniform_buffer, &mem_reqs);

        mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem_alloc.pNext = NULL;
        mem_alloc.allocationSize = mem_reqs.size;
        mem_alloc.memoryTypeIndex = 0;

        pass = memory_type_from_properties(demo, mem_reqs.memoryTypeBits,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                           &mem_alloc.memoryTypeIndex);
        assert(pass);

        err = vkAllocateMemory(demo->device, &mem_alloc, NULL, &demo->swapchain_image_resources[i].uniform_memory);
        assert(!err);
        demo_name_object(demo, VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)demo->swapchain_image_resources[i].uniform_memory,
                         "SwapchainUniformMem(%u)", i);

        err = vkMapMemory(demo->device, demo->swapchain_image_resources[i].uniform_memory, 0, VK_WHOLE_SIZE, 0,
                          &demo->swapchain_image_resources[i].uniform_memory_ptr);
        assert(!err);

        memcpy(demo->swapchain_image_resources[i].uniform_memory_ptr, &data, sizeof data);

        err = vkBindBufferMemory(demo->device, demo->swapchain_image_resources[i].uniform_buffer,
                                 demo->swapchain_image_resources[i].uniform_memory, 0);
        assert(!err);
    }
}

static void demo_prepare_descriptor_layout(struct demo *demo) {
    const VkDescriptorSetLayoutBinding layout_bindings[2] = {
        [0] =
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = NULL,
            },
        [1] =
            {
                .binding = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = DEMO_TEXTURE_COUNT,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = NULL,
            },
    };
    const VkDescriptorSetLayoutCreateInfo descriptor_layout = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .bindingCount = 2,
        .pBindings = layout_bindings,
    };
    VkResult U_ASSERT_ONLY err;

    err = vkCreateDescriptorSetLayout(demo->device, &descriptor_layout, NULL, &demo->desc_layout);
    assert(!err);

    const VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .setLayoutCount = 1,
        .pSetLayouts = &demo->desc_layout,
    };

    err = vkCreatePipelineLayout(demo->device, &pPipelineLayoutCreateInfo, NULL, &demo->pipeline_layout);
    assert(!err);
}

static void demo_prepare_render_pass(struct demo *demo) {
    // The initial layout for the color and depth attachments will be LAYOUT_UNDEFINED
    // because at the start of the renderpass, we don't care about their contents.
    // At the start of the subpass, the color attachment's layout will be transitioned
    // to LAYOUT_COLOR_ATTACHMENT_OPTIMAL and the depth stencil attachment's layout
    // will be transitioned to LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL.  At the end of
    // the renderpass, the color attachment's layout will be transitioned to
    // LAYOUT_PRESENT_SRC_KHR to be ready to present.  This is all done as part of
    // the renderpass, no barriers are necessary.
    const VkAttachmentDescription attachments[2] = {
        [0] =
            {
                .format = demo->format,
                .flags = 0,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            },
        [1] =
            {
                .format = demo->depth.format,
                .flags = 0,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            },
    };
    const VkAttachmentReference color_reference = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    const VkAttachmentReference depth_reference = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };
    const VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .flags = 0,
        .inputAttachmentCount = 0,
        .pInputAttachments = NULL,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_reference,
        .pResolveAttachments = NULL,
        .pDepthStencilAttachment = &depth_reference,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = NULL,
    };

    VkSubpassDependency attachmentDependencies[2] = {
        [0] =
            {
                // Depth buffer is shared between swapchain images
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .dependencyFlags = 0,
            },
        [1] =
            {
                // Image Layout Transition
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                .dependencyFlags = 0,
            },
    };

    const VkRenderPassCreateInfo rp_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .attachmentCount = 2,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 2,
        .pDependencies = attachmentDependencies,
    };
    VkResult U_ASSERT_ONLY err;

    err = vkCreateRenderPass(demo->device, &rp_info, NULL, &demo->render_pass);
    assert(!err);
}

static VkShaderModule demo_prepare_shader_module(const char *name, struct demo *demo, const uint32_t *code, size_t size) {
    VkShaderModule module;
    VkShaderModuleCreateInfo moduleCreateInfo;
    VkResult U_ASSERT_ONLY err;

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = size;
    moduleCreateInfo.pCode = code;

    err = vkCreateShaderModule(demo->device, &moduleCreateInfo, NULL, &module);
    assert(!err);
    demo_name_object(demo, VK_OBJECT_TYPE_SHADER_MODULE, (uint64_t)module, "%s", name);

    return module;
}

static void demo_prepare_vs(struct demo *demo) {
    const uint32_t vs_code[] = {
#include "cube.vert.inc"
    };
    demo->vert_shader_module = demo_prepare_shader_module("cube.vert", demo, vs_code, sizeof(vs_code));
}

static void demo_prepare_fs(struct demo *demo) {
    const uint32_t fs_code[] = {
#include "cube.frag.inc"
    };
    demo->frag_shader_module = demo_prepare_shader_module("cube.frag", demo, fs_code, sizeof(fs_code));
}

static void demo_prepare_pipeline(struct demo *demo) {
#define NUM_DYNAMIC_STATES 2 /*Viewport + Scissor*/

    VkGraphicsPipelineCreateInfo pipeline;
    VkPipelineCacheCreateInfo pipelineCache;
    VkPipelineVertexInputStateCreateInfo vi;
    VkPipelineInputAssemblyStateCreateInfo ia;
    VkPipelineRasterizationStateCreateInfo rs;
    VkPipelineColorBlendStateCreateInfo cb;
    VkPipelineDepthStencilStateCreateInfo ds;
    VkPipelineViewportStateCreateInfo vp;
    VkPipelineMultisampleStateCreateInfo ms;
    VkDynamicState dynamicStateEnables[NUM_DYNAMIC_STATES];
    VkPipelineDynamicStateCreateInfo dynamicState;
    VkResult U_ASSERT_ONLY err;

    memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
    memset(&dynamicState, 0, sizeof dynamicState);
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables;

    memset(&pipeline, 0, sizeof(pipeline));
    pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline.layout = demo->pipeline_layout;

    memset(&vi, 0, sizeof(vi));
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    memset(&ia, 0, sizeof(ia));
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    memset(&rs, 0, sizeof(rs));
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.depthClampEnable = VK_FALSE;
    rs.rasterizerDiscardEnable = VK_FALSE;
    rs.depthBiasEnable = VK_FALSE;
    rs.lineWidth = 1.0f;

    memset(&cb, 0, sizeof(cb));
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    VkPipelineColorBlendAttachmentState att_state[1];
    memset(att_state, 0, sizeof(att_state));
    att_state[0].colorWriteMask = 0xf;
    att_state[0].blendEnable = VK_FALSE;
    cb.attachmentCount = 1;
    cb.pAttachments = att_state;

    memset(&vp, 0, sizeof(vp));
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    vp.scissorCount = 1;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

    memset(&ds, 0, sizeof(ds));
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    ds.depthBoundsTestEnable = VK_FALSE;
    ds.back.failOp = VK_STENCIL_OP_KEEP;
    ds.back.passOp = VK_STENCIL_OP_KEEP;
    ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
    ds.stencilTestEnable = VK_FALSE;
    ds.front = ds.back;

    memset(&ms, 0, sizeof(ms));
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.pSampleMask = NULL;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    demo_prepare_vs(demo);
    demo_prepare_fs(demo);

    // Two stages: vs and fs
    VkPipelineShaderStageCreateInfo shaderStages[2];
    memset(&shaderStages, 0, 2 * sizeof(VkPipelineShaderStageCreateInfo));

    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = demo->vert_shader_module;
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = demo->frag_shader_module;
    shaderStages[1].pName = "main";

    memset(&pipelineCache, 0, sizeof(pipelineCache));
    pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    err = vkCreatePipelineCache(demo->device, &pipelineCache, NULL, &demo->pipelineCache);
    assert(!err);

    pipeline.pVertexInputState = &vi;
    pipeline.pInputAssemblyState = &ia;
    pipeline.pRasterizationState = &rs;
    pipeline.pColorBlendState = &cb;
    pipeline.pMultisampleState = &ms;
    pipeline.pViewportState = &vp;
    pipeline.pDepthStencilState = &ds;
    pipeline.stageCount = ARRAY_SIZE(shaderStages);
    pipeline.pStages = shaderStages;
    pipeline.renderPass = demo->render_pass;
    pipeline.pDynamicState = &dynamicState;

    err = vkCreateGraphicsPipelines(demo->device, demo->pipelineCache, 1, &pipeline, NULL, &demo->pipeline);
    assert(!err);

    vkDestroyShaderModule(demo->device, demo->frag_shader_module, NULL);
    vkDestroyShaderModule(demo->device, demo->vert_shader_module, NULL);
}

static void demo_prepare_descriptor_pool(struct demo *demo) {
    const VkDescriptorPoolSize type_counts[2] = {
        [0] =
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = demo->swapchainImageCount,
            },
        [1] =
            {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = demo->swapchainImageCount * DEMO_TEXTURE_COUNT,
            },
    };
    const VkDescriptorPoolCreateInfo descriptor_pool = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .maxSets = demo->swapchainImageCount,
        .poolSizeCount = 2,
        .pPoolSizes = type_counts,
    };
    VkResult U_ASSERT_ONLY err;

    err = vkCreateDescriptorPool(demo->device, &descriptor_pool, NULL, &demo->desc_pool);
    assert(!err);
}

static void demo_prepare_descriptor_set(struct demo *demo) {
    VkDescriptorImageInfo tex_descs[DEMO_TEXTURE_COUNT];
    VkWriteDescriptorSet writes[2];
    VkResult U_ASSERT_ONLY err;

    VkDescriptorSetAllocateInfo alloc_info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                              .pNext = NULL,
                                              .descriptorPool = demo->desc_pool,
                                              .descriptorSetCount = 1,
                                              .pSetLayouts = &demo->desc_layout};

    VkDescriptorBufferInfo buffer_info;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(struct vktexcube_vs_uniform);

    memset(&tex_descs, 0, sizeof(tex_descs));
    for (unsigned int i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        tex_descs[i].sampler = demo->textures[i].sampler;
        tex_descs[i].imageView = demo->textures[i].view;
        tex_descs[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    memset(&writes, 0, sizeof(writes));

    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].pBufferInfo = &buffer_info;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstBinding = 1;
    writes[1].descriptorCount = DEMO_TEXTURE_COUNT;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].pImageInfo = tex_descs;

    for (unsigned int i = 0; i < demo->swapchainImageCount; i++) {
        err = vkAllocateDescriptorSets(demo->device, &alloc_info, &demo->swapchain_image_resources[i].descriptor_set);
        assert(!err);
        buffer_info.buffer = demo->swapchain_image_resources[i].uniform_buffer;
        writes[0].dstSet = demo->swapchain_image_resources[i].descriptor_set;
        writes[1].dstSet = demo->swapchain_image_resources[i].descriptor_set;
        vkUpdateDescriptorSets(demo->device, 2, writes, 0, NULL);
    }
}

static void demo_prepare_framebuffers(struct demo *demo) {
    VkImageView attachments[2];
    attachments[1] = demo->depth.view;

    const VkFramebufferCreateInfo fb_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = NULL,
        .renderPass = demo->render_pass,
        .attachmentCount = 2,
        .pAttachments = attachments,
        .width = demo->width,
        .height = demo->height,
        .layers = 1,
    };
    VkResult U_ASSERT_ONLY err;
    uint32_t i;

    for (i = 0; i < demo->swapchainImageCount; i++) {
        attachments[0] = demo->swapchain_image_resources[i].view;
        err = vkCreateFramebuffer(demo->device, &fb_info, NULL, &demo->swapchain_image_resources[i].framebuffer);
        assert(!err);
        demo_name_object(demo, VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)demo->swapchain_image_resources[i].framebuffer,
                         "Framebuffer(%u)", i);
    }
}

static void demo_prepare(struct demo *demo) {
    demo_prepare_buffers(demo);

    if (demo->is_minimized) {
        demo->prepared = false;
        return;
    }

    VkResult U_ASSERT_ONLY err;
    if (demo->cmd_pool == VK_NULL_HANDLE) {
        const VkCommandPoolCreateInfo cmd_pool_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = NULL,
            .queueFamilyIndex = demo->graphics_queue_family_index,
            .flags = 0,
        };
        err = vkCreateCommandPool(demo->device, &cmd_pool_info, NULL, &demo->cmd_pool);
        assert(!err);
    }

    const VkCommandBufferAllocateInfo cmd = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = demo->cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    err = vkAllocateCommandBuffers(demo->device, &cmd, &demo->cmd);
    assert(!err);
    demo_name_object(demo, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)demo->cmd, "PrepareCB");
    VkCommandBufferBeginInfo cmd_buf_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = 0,
        .pInheritanceInfo = NULL,
    };
    err = vkBeginCommandBuffer(demo->cmd, &cmd_buf_info);
    demo_push_cb_label(demo, demo->cmd, NULL, "Prepare");
    assert(!err);

    demo_prepare_depth(demo);
    demo_prepare_textures(demo);
    demo_prepare_cube_data_buffers(demo);

    demo_prepare_descriptor_layout(demo);
    demo_prepare_render_pass(demo);
    demo_prepare_pipeline(demo);

    for (uint32_t i = 0; i < demo->swapchainImageCount; i++) {
        err = vkAllocateCommandBuffers(demo->device, &cmd, &demo->swapchain_image_resources[i].cmd);
        assert(!err);
    }

    if (demo->separate_present_queue) {
        const VkCommandPoolCreateInfo present_cmd_pool_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = NULL,
            .queueFamilyIndex = demo->present_queue_family_index,
            .flags = 0,
        };
        err = vkCreateCommandPool(demo->device, &present_cmd_pool_info, NULL, &demo->present_cmd_pool);
        assert(!err);
        const VkCommandBufferAllocateInfo present_cmd_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = NULL,
            .commandPool = demo->present_cmd_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        for (uint32_t i = 0; i < demo->swapchainImageCount; i++) {
            err = vkAllocateCommandBuffers(demo->device, &present_cmd_info,
                                           &demo->swapchain_image_resources[i].graphics_to_present_cmd);
            assert(!err);
            demo_build_image_ownership_cmd(demo, i);
            demo_name_object(demo, VK_OBJECT_TYPE_COMMAND_BUFFER,
                             (uint64_t)demo->swapchain_image_resources[i].graphics_to_present_cmd, "GfxToPresent(%u)", i);
        }
    }

    demo_prepare_descriptor_pool(demo);
    demo_prepare_descriptor_set(demo);

    demo_prepare_framebuffers(demo);

    for (uint32_t i = 0; i < demo->swapchainImageCount; i++) {
        demo->current_buffer = i;
        demo_draw_build_cmd(demo, demo->swapchain_image_resources[i].cmd);
    }

    /*
     * Prepare functions above may generate pipeline commands
     * that need to be flushed before beginning the render loop.
     */
    demo_pop_cb_label(demo, demo->cmd);  // "Prepare"
    demo_flush_init_cmd(demo);
    if (demo->staging_texture.buffer) {
        demo_destroy_texture(demo, &demo->staging_texture);
    }

    demo->current_buffer = 0;
    demo->prepared = true;
    demo->first_swapchain_frame = true;
}

static void demo_cleanup(struct demo *demo) {
    uint32_t i;

    demo->prepared = false;
    vkDeviceWaitIdle(demo->device);

    // Wait for fences from present operations
    for (i = 0; i < FRAME_LAG; i++) {
        vkWaitForFences(demo->device, 1, &demo->fences[i], VK_TRUE, UINT64_MAX);
        vkDestroyFence(demo->device, demo->fences[i], NULL);
        vkDestroySemaphore(demo->device, demo->image_acquired_semaphores[i], NULL);
        vkDestroySemaphore(demo->device, demo->draw_complete_semaphores[i], NULL);
        if (demo->separate_present_queue) {
            vkDestroySemaphore(demo->device, demo->image_ownership_semaphores[i], NULL);
        }
    }

    // If the window is currently minimized, demo_resize has already done some cleanup for us.
    if (!demo->is_minimized) {
        for (i = 0; i < demo->swapchainImageCount; i++) {
            vkDestroyFramebuffer(demo->device, demo->swapchain_image_resources[i].framebuffer, NULL);
        }
        vkDestroyDescriptorPool(demo->device, demo->desc_pool, NULL);

        vkDestroyPipeline(demo->device, demo->pipeline, NULL);
        vkDestroyPipelineCache(demo->device, demo->pipelineCache, NULL);
        vkDestroyRenderPass(demo->device, demo->render_pass, NULL);
        vkDestroyPipelineLayout(demo->device, demo->pipeline_layout, NULL);
        vkDestroyDescriptorSetLayout(demo->device, demo->desc_layout, NULL);

        for (i = 0; i < DEMO_TEXTURE_COUNT; i++) {
            vkDestroyImageView(demo->device, demo->textures[i].view, NULL);
            vkDestroyImage(demo->device, demo->textures[i].image, NULL);
            vkFreeMemory(demo->device, demo->textures[i].mem, NULL);
            vkDestroySampler(demo->device, demo->textures[i].sampler, NULL);
        }
        vkDestroySwapchainKHR(demo->device, demo->swapchain, NULL);

        vkDestroyImageView(demo->device, demo->depth.view, NULL);
        vkDestroyImage(demo->device, demo->depth.image, NULL);
        vkFreeMemory(demo->device, demo->depth.mem, NULL);

        for (i = 0; i < demo->swapchainImageCount; i++) {
            vkDestroyImageView(demo->device, demo->swapchain_image_resources[i].view, NULL);
            vkFreeCommandBuffers(demo->device, demo->cmd_pool, 1, &demo->swapchain_image_resources[i].cmd);
            vkDestroyBuffer(demo->device, demo->swapchain_image_resources[i].uniform_buffer, NULL);
            vkUnmapMemory(demo->device, demo->swapchain_image_resources[i].uniform_memory);
            vkFreeMemory(demo->device, demo->swapchain_image_resources[i].uniform_memory, NULL);
        }
        free(demo->swapchain_image_resources);
        free(demo->queue_props);
        vkDestroyCommandPool(demo->device, demo->cmd_pool, NULL);

        if (demo->separate_present_queue) {
            vkDestroyCommandPool(demo->device, demo->present_cmd_pool, NULL);
        }
    }
    vkDeviceWaitIdle(demo->device);
    vkDestroyDevice(demo->device, NULL);
    if (demo->validate) {
        vkDestroyDebugUtilsMessengerEXT(demo->inst, demo->dbg_messenger, NULL);
    }
    vkDestroySurfaceKHR(demo->inst, demo->surface, NULL);

#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (demo->wsi_platform == WSI_PLATFORM_XLIB) {
        XDestroyWindow(demo->xlib_display, demo->xlib_window);
        XCloseDisplay(demo->xlib_display);
    }
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (demo->wsi_platform == WSI_PLATFORM_XCB) {
        xcb_destroy_window(demo->connection, demo->xcb_window);
        xcb_disconnect(demo->connection);
        free(demo->atom_wm_delete_window);
    }
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    if (demo->wsi_platform == WSI_PLATFORM_WAYLAND) {
        if (demo->keyboard) wl_keyboard_destroy(demo->keyboard);
        if (demo->pointer) wl_pointer_destroy(demo->pointer);
        if (demo->seat) wl_seat_destroy(demo->seat);
        xdg_toplevel_destroy(demo->xdg_toplevel);
        xdg_surface_destroy(demo->xdg_surface);
        wl_surface_destroy(demo->window);
        xdg_wm_base_destroy(demo->xdg_wm_base);
        if (demo->xdg_decoration_mgr) {
            zxdg_toplevel_decoration_v1_destroy(demo->toplevel_decoration);
            zxdg_decoration_manager_v1_destroy(demo->xdg_decoration_mgr);
        }
        wl_compositor_destroy(demo->compositor);
        wl_registry_destroy(demo->registry);
        wl_display_disconnect(demo->wayland_display);
    }
#endif
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    if (demo->wsi_platform == WSI_PLATFORM_DIRECTFB) {
        demo->event_buffer->Release(demo->event_buffer);
        demo->directfb_window->Release(demo->directfb_window);
        demo->dfb->Release(demo->dfb);
    }
#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    if (demo->wsi_platform == WSI_PLATFORM_QNX) {
        screen_destroy_event(demo->screen_event);
        screen_destroy_window(demo->screen_window);
        screen_destroy_context(demo->screen_context);
    }
#endif

    vkDestroyInstance(demo->inst, NULL);
}

static void demo_resize(struct demo *demo) {
    uint32_t i;

    // Don't react to resize until after first initialization.
    if (!demo->prepared) {
        if (demo->is_minimized) {
            demo_prepare(demo);
        }
        return;
    }
    // In order to properly resize the window, we must re-create the swapchain
    // AND redo the command buffers, etc.
    //
    // First, perform part of the demo_cleanup() function:
    demo->prepared = false;
    vkDeviceWaitIdle(demo->device);

    for (i = 0; i < demo->swapchainImageCount; i++) {
        vkDestroyFramebuffer(demo->device, demo->swapchain_image_resources[i].framebuffer, NULL);
    }
    vkDestroyDescriptorPool(demo->device, demo->desc_pool, NULL);

    vkDestroyPipeline(demo->device, demo->pipeline, NULL);
    vkDestroyPipelineCache(demo->device, demo->pipelineCache, NULL);
    vkDestroyRenderPass(demo->device, demo->render_pass, NULL);
    vkDestroyPipelineLayout(demo->device, demo->pipeline_layout, NULL);
    vkDestroyDescriptorSetLayout(demo->device, demo->desc_layout, NULL);

    for (i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        vkDestroyImageView(demo->device, demo->textures[i].view, NULL);
        vkDestroyImage(demo->device, demo->textures[i].image, NULL);
        vkFreeMemory(demo->device, demo->textures[i].mem, NULL);
        vkDestroySampler(demo->device, demo->textures[i].sampler, NULL);
    }

    vkDestroyImageView(demo->device, demo->depth.view, NULL);
    vkDestroyImage(demo->device, demo->depth.image, NULL);
    vkFreeMemory(demo->device, demo->depth.mem, NULL);

    for (i = 0; i < demo->swapchainImageCount; i++) {
        vkDestroyImageView(demo->device, demo->swapchain_image_resources[i].view, NULL);
        vkFreeCommandBuffers(demo->device, demo->cmd_pool, 1, &demo->swapchain_image_resources[i].cmd);
        vkDestroyBuffer(demo->device, demo->swapchain_image_resources[i].uniform_buffer, NULL);
        vkUnmapMemory(demo->device, demo->swapchain_image_resources[i].uniform_memory);
        vkFreeMemory(demo->device, demo->swapchain_image_resources[i].uniform_memory, NULL);
    }
    vkDestroyCommandPool(demo->device, demo->cmd_pool, NULL);
    demo->cmd_pool = VK_NULL_HANDLE;
    if (demo->separate_present_queue) {
        vkDestroyCommandPool(demo->device, demo->present_cmd_pool, NULL);
    }
    free(demo->swapchain_image_resources);

    // Second, re-perform the demo_prepare() function, which will re-create the
    // swapchain:
    demo_prepare(demo);
}

// On MS-Windows, make this a global, so it's available to WndProc()
struct demo demo;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
static void demo_run(struct demo *demo) {
    if (!demo->prepared) return;

    demo_draw(demo);
    demo->curFrame++;
    if (demo->frameCount != INT32_MAX && demo->curFrame == demo->frameCount) {
        PostQuitMessage(validation_error);
    }
}

// MS-Windows event handling function:
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
            PostQuitMessage(validation_error);
            break;
        case WM_PAINT:
            // The validation callback calls MessageBox which can generate paint
            // events - don't make more Vulkan calls if we got here from the
            // callback
            if (!in_callback) {
                demo_run(&demo);
            }
            break;
        case WM_GETMINMAXINFO:  // set window's minimum size
            ((MINMAXINFO *)lParam)->ptMinTrackSize = demo.minsize;
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_SIZE:
            // Resize the application to the new window size, except when
            // it was minimized. Vulkan doesn't support images or swapchains
            // with width=0 and height=0.
            if (wParam != SIZE_MINIMIZED) {
                demo.width = lParam & 0xffff;
                demo.height = (lParam & 0xffff0000) >> 16;
                demo_resize(&demo);
            }
            break;
        case WM_KEYDOWN:
            switch (wParam) {
                case VK_ESCAPE:
                    PostQuitMessage(validation_error);
                    break;
                case VK_LEFT:
                    demo.spin_angle -= demo.spin_increment;
                    break;
                case VK_RIGHT:
                    demo.spin_angle += demo.spin_increment;
                    break;
                case VK_SPACE:
                    demo.pause = !demo.pause;
                    break;
            }
            return 0;
        default:
            break;
    }
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

static void demo_create_window(struct demo *demo) {
    WNDCLASSEX win_class;

    // Initialize the window class structure:
    win_class.cbSize = sizeof(WNDCLASSEX);
    win_class.style = CS_HREDRAW | CS_VREDRAW;
    win_class.lpfnWndProc = WndProc;
    win_class.cbClsExtra = 0;
    win_class.cbWndExtra = 0;
    win_class.hInstance = demo->connection;  // hInstance
    win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    win_class.lpszMenuName = NULL;
    win_class.lpszClassName = demo->name;
    win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
    // Register window class:
    if (!RegisterClassEx(&win_class)) {
        // It didn't work, so try to give a useful error:
        printf("Unexpected error trying to start the application!\n");
        fflush(stdout);
        exit(1);
    }
    // Create window with the registered class:
    RECT wr = {0, 0, demo->width, demo->height};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    demo->window = CreateWindowEx(0,
                                  demo->name,            // class name
                                  demo->name,            // app name
                                  WS_OVERLAPPEDWINDOW |  // window style
                                      WS_VISIBLE | WS_SYSMENU,
                                  100, 100,            // x/y coords
                                  wr.right - wr.left,  // width
                                  wr.bottom - wr.top,  // height
                                  NULL,                // handle to parent
                                  NULL,                // handle to menu
                                  demo->connection,    // hInstance
                                  NULL);               // no extra parameters
    if (!demo->window) {
        // It didn't work, so try to give a useful error:
        printf("Cannot create a window in which to draw!\n");
        fflush(stdout);
        exit(1);
    }
    // Window client area size must be at least 1 pixel high, to prevent crash.
    demo->minsize.x = GetSystemMetrics(SM_CXMINTRACK);
    demo->minsize.y = GetSystemMetrics(SM_CYMINTRACK) + 1;
}
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
static void demo_create_xlib_window(struct demo *demo) {
    const char *display_envar = getenv("DISPLAY");
    if (display_envar == NULL || display_envar[0] == '\0') {
        printf("Environment variable DISPLAY requires a valid value.\nExiting ...\n");
        fflush(stdout);
        exit(1);
    }

    XInitThreads();
    demo->xlib_display = XOpenDisplay(NULL);
    long visualMask = VisualScreenMask;
    int numberOfVisuals;
    XVisualInfo vInfoTemplate = {};
    vInfoTemplate.screen = DefaultScreen(demo->xlib_display);
    XVisualInfo *visualInfo = XGetVisualInfo(demo->xlib_display, visualMask, &vInfoTemplate, &numberOfVisuals);

    Colormap colormap =
        XCreateColormap(demo->xlib_display, RootWindow(demo->xlib_display, vInfoTemplate.screen), visualInfo->visual, AllocNone);

    XSetWindowAttributes windowAttributes = {};
    windowAttributes.colormap = colormap;
    windowAttributes.background_pixel = 0xFFFFFFFF;
    windowAttributes.border_pixel = 0;
    windowAttributes.event_mask = KeyPressMask | KeyReleaseMask | StructureNotifyMask | ExposureMask;

    demo->xlib_window = XCreateWindow(demo->xlib_display, RootWindow(demo->xlib_display, vInfoTemplate.screen), 0, 0, demo->width,
                                      demo->height, 0, visualInfo->depth, InputOutput, visualInfo->visual,
                                      CWBackPixel | CWBorderPixel | CWEventMask | CWColormap, &windowAttributes);

    XSelectInput(demo->xlib_display, demo->xlib_window, ExposureMask | KeyPressMask);
    XMapWindow(demo->xlib_display, demo->xlib_window);
    XFlush(demo->xlib_display);
    demo->xlib_wm_delete_window = XInternAtom(demo->xlib_display, "WM_DELETE_WINDOW", False);
}
static void demo_handle_xlib_event(struct demo *demo, const XEvent *event) {
    switch (event->type) {
        case ClientMessage:
            if ((Atom)event->xclient.data.l[0] == demo->xlib_wm_delete_window) demo->quit = true;
            break;
        case KeyPress:
            switch (event->xkey.keycode) {
                case 0x9:  // Escape
                    demo->quit = true;
                    break;
                case 0x71:  // left arrow key
                    demo->spin_angle -= demo->spin_increment;
                    break;
                case 0x72:  // right arrow key
                    demo->spin_angle += demo->spin_increment;
                    break;
                case 0x41:  // space bar
                    demo->pause = !demo->pause;
                    break;
            }
            break;
        case ConfigureNotify:
            if ((demo->width != event->xconfigure.width) || (demo->height != event->xconfigure.height)) {
                demo->width = event->xconfigure.width;
                demo->height = event->xconfigure.height;
                demo_resize(demo);
            }
            break;
        default:
            break;
    }
}

static void demo_run_xlib(struct demo *demo) {
    while (!demo->quit) {
        XEvent event;

        if (demo->pause) {
            XNextEvent(demo->xlib_display, &event);
            demo_handle_xlib_event(demo, &event);
        }
        while (XPending(demo->xlib_display) > 0) {
            XNextEvent(demo->xlib_display, &event);
            demo_handle_xlib_event(demo, &event);
        }

        demo_draw(demo);
        demo->curFrame++;
        if (demo->frameCount != INT32_MAX && demo->curFrame == demo->frameCount) demo->quit = true;
    }
}
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
static void demo_handle_xcb_event(struct demo *demo, const xcb_generic_event_t *event) {
    uint8_t event_code = event->response_type & 0x7f;
    switch (event_code) {
        case XCB_EXPOSE:
            // TODO: Resize window
            break;
        case XCB_CLIENT_MESSAGE:
            if ((*(xcb_client_message_event_t *)event).data.data32[0] == (*demo->atom_wm_delete_window).atom) {
                demo->quit = true;
            }
            break;
        case XCB_KEY_RELEASE: {
            const xcb_key_release_event_t *key = (const xcb_key_release_event_t *)event;

            switch (key->detail) {
                case 0x9:  // Escape
                    demo->quit = true;
                    break;
                case 0x71:  // left arrow key
                    demo->spin_angle -= demo->spin_increment;
                    break;
                case 0x72:  // right arrow key
                    demo->spin_angle += demo->spin_increment;
                    break;
                case 0x41:  // space bar
                    demo->pause = !demo->pause;
                    break;
            }
        } break;
        case XCB_CONFIGURE_NOTIFY: {
            const xcb_configure_notify_event_t *cfg = (const xcb_configure_notify_event_t *)event;
            if ((demo->width != cfg->width) || (demo->height != cfg->height)) {
                demo->width = cfg->width;
                demo->height = cfg->height;
                demo_resize(demo);
            }
        } break;
        default:
            break;
    }
}

static void demo_run_xcb(struct demo *demo) {
    xcb_flush(demo->connection);

    while (!demo->quit) {
        xcb_generic_event_t *event;

        if (demo->pause) {
            event = xcb_wait_for_event(demo->connection);
        } else {
            event = xcb_poll_for_event(demo->connection);
        }
        while (event) {
            demo_handle_xcb_event(demo, event);
            free(event);
            event = xcb_poll_for_event(demo->connection);
        }

        demo_draw(demo);
        demo->curFrame++;
        if (demo->frameCount != INT32_MAX && demo->curFrame == demo->frameCount) demo->quit = true;
    }
}

static void demo_create_xcb_window(struct demo *demo) {
    uint32_t value_mask, value_list[32];

    demo->xcb_window = xcb_generate_id(demo->connection);

    value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    value_list[0] = demo->screen->black_pixel;
    value_list[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    xcb_create_window(demo->connection, XCB_COPY_FROM_PARENT, demo->xcb_window, demo->screen->root, 0, 0, demo->width, demo->height,
                      0, XCB_WINDOW_CLASS_INPUT_OUTPUT, demo->screen->root_visual, value_mask, value_list);

    /* Magic code that will send notification when window is destroyed */
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(demo->connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(demo->connection, cookie, 0);

    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(demo->connection, 0, 16, "WM_DELETE_WINDOW");
    demo->atom_wm_delete_window = xcb_intern_atom_reply(demo->connection, cookie2, 0);

    xcb_change_property(demo->connection, XCB_PROP_MODE_REPLACE, demo->xcb_window, (*reply).atom, 4, 32, 1,
                        &(*demo->atom_wm_delete_window).atom);
    free(reply);

    xcb_map_window(demo->connection, demo->xcb_window);

    // Force the x/y coordinates to 100,100 results are identical in consecutive
    // runs
    const uint32_t coords[] = {100, 100};
    xcb_configure_window(demo->connection, demo->xcb_window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);
}
// VK_USE_PLATFORM_XCB_KHR
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
static void demo_run(struct demo *demo) {
    while (!demo->quit) {
        // Flush any commands to the server
        wl_display_flush(demo->wayland_display);

        if (demo->pause) {
            // block and wait for input
            wl_display_dispatch(demo->wayland_display);
        } else {
            // Lock the display event queue in case the driver is doing something on another thread
            // while we wait, keep pumping events
            while (wl_display_prepare_read(demo->wayland_display) != 0) {
                wl_display_dispatch_pending(demo->wayland_display);
            }
            // Actually do the read from the socket
            wl_display_read_events(demo->wayland_display);

            // Pump events
            wl_display_dispatch_pending(demo->wayland_display);

            demo_draw(demo);
            demo->curFrame++;
            if (demo->frameCount != INT32_MAX && demo->curFrame == demo->frameCount) demo->quit = true;
        }
    }
}

static void handle_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
    struct demo *demo = (struct demo *)data;
    xdg_surface_ack_configure(xdg_surface, serial);
    if (demo->xdg_surface_has_been_configured) {
        demo_resize(demo);
    }
    demo->xdg_surface_has_been_configured = 1;
}

static const struct xdg_surface_listener xdg_surface_listener = {handle_surface_configure};

static void handle_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel UNUSED, int32_t width, int32_t height,
                                      struct wl_array *states UNUSED) {
    struct demo *demo = (struct demo *)data;
    /* zero values imply the program may choose its own size, so in that case
     * stay with the existing value (which on startup is the default) */
    if (width > 0) {
        demo->width = width;
    }
    if (height > 0) {
        demo->height = height;
    }
    /* This should be followed by a surface configure */
}

static void handle_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel UNUSED) {
    struct demo *demo = (struct demo *)data;
    demo->quit = true;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {handle_toplevel_configure, handle_toplevel_close};

static void demo_create_wayland_window(struct demo *demo) {
    if (!demo->xdg_wm_base) {
        printf("Compositor did not provide the standard protocol xdg-wm-base\n");
        fflush(stdout);
        exit(1);
    }

    demo->window = wl_compositor_create_surface(demo->compositor);
    if (!demo->window) {
        printf("Can not create wayland_surface from compositor!\n");
        fflush(stdout);
        exit(1);
    }

    demo->xdg_surface = xdg_wm_base_get_xdg_surface(demo->xdg_wm_base, demo->window);
    if (!demo->xdg_surface) {
        printf("Can not get xdg_surface from wayland_surface!\n");
        fflush(stdout);
        exit(1);
    }
    demo->xdg_toplevel = xdg_surface_get_toplevel(demo->xdg_surface);
    if (!demo->xdg_toplevel) {
        printf("Can not allocate xdg_toplevel for xdg_surface!\n");
        fflush(stdout);
        exit(1);
    }
    xdg_surface_add_listener(demo->xdg_surface, &xdg_surface_listener, demo);
    xdg_toplevel_add_listener(demo->xdg_toplevel, &xdg_toplevel_listener, demo);
    xdg_toplevel_set_title(demo->xdg_toplevel, APP_SHORT_NAME);
    if (demo->xdg_decoration_mgr) {
        // if supported, let the compositor render titlebars for us
        demo->toplevel_decoration =
            zxdg_decoration_manager_v1_get_toplevel_decoration(demo->xdg_decoration_mgr, demo->xdg_toplevel);
        zxdg_toplevel_decoration_v1_set_mode(demo->toplevel_decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }

    wl_surface_commit(demo->window);
}
#endif
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
static void demo_create_directfb_window(struct demo *demo) {
    DFBResult ret;

    ret = DirectFBInit(NULL, NULL);
    if (ret) {
        printf("DirectFBInit failed to initialize DirectFB!\n");
        fflush(stdout);
        exit(1);
    }

    ret = DirectFBCreate(&demo->dfb);
    if (ret) {
        printf("DirectFBCreate failed to create main interface of DirectFB!\n");
        fflush(stdout);
        exit(1);
    }

    DFBSurfaceDescription desc;
    desc.flags = DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT;
    desc.caps = DSCAPS_PRIMARY;
    desc.width = demo->width;
    desc.height = demo->height;
    ret = demo->dfb->CreateSurface(demo->dfb, &desc, &demo->directfb_window);
    if (ret) {
        printf("CreateSurface failed to create DirectFB surface interface!\n");
        fflush(stdout);
        exit(1);
    }

    ret = demo->dfb->CreateInputEventBuffer(demo->dfb, DICAPS_KEYS, DFB_FALSE, &demo->event_buffer);
    if (ret) {
        printf("CreateInputEventBuffer failed to create DirectFB event buffer interface!\n");
        fflush(stdout);
        exit(1);
    }
}

static void demo_handle_directfb_event(struct demo *demo, const DFBInputEvent *event) {
    if (event->type != DIET_KEYPRESS) return;
    switch (event->key_symbol) {
        case DIKS_ESCAPE:  // Escape
            demo->quit = true;
            break;
        case DIKS_CURSOR_LEFT:  // left arrow key
            demo->spin_angle -= demo->spin_increment;
            break;
        case DIKS_CURSOR_RIGHT:  // right arrow key
            demo->spin_angle += demo->spin_increment;
            break;
        case DIKS_SPACE:  // space bar
            demo->pause = !demo->pause;
            break;
        default:
            break;
    }
}

static void demo_run_directfb(struct demo *demo) {
    while (!demo->quit) {
        DFBInputEvent event;

        if (demo->pause) {
            demo->event_buffer->WaitForEvent(demo->event_buffer);
            if (!demo->event_buffer->GetEvent(demo->event_buffer, DFB_EVENT(&event))) demo_handle_directfb_event(demo, &event);
        } else {
            if (!demo->event_buffer->GetEvent(demo->event_buffer, DFB_EVENT(&event))) demo_handle_directfb_event(demo, &event);

            demo_draw(demo);
            demo->curFrame++;
            if (demo->frameCount != INT32_MAX && demo->curFrame == demo->frameCount) demo->quit = true;
        }
    }
}
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
static void demo_run(struct demo *demo) {
    if (!demo->prepared) return;

    demo_draw(demo);
    demo->curFrame++;
}
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
static void demo_run(struct demo *demo) {
    demo_draw(demo);
    demo->curFrame++;
    if (demo->frameCount != INT32_MAX && demo->curFrame == demo->frameCount) {
        demo->quit = TRUE;
    }
}
#endif
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
static VkResult demo_create_display_surface(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;
    uint32_t display_count;
    uint32_t mode_count;
    uint32_t plane_count;
    VkDisplayPropertiesKHR display_props;
    VkDisplayKHR display;
    VkDisplayModePropertiesKHR mode_props;
    VkDisplayPlanePropertiesKHR *plane_props;
    VkBool32 found_plane = VK_FALSE;
    uint32_t plane_index;
    VkExtent2D image_extent;
    VkDisplaySurfaceCreateInfoKHR create_info;

    // Get the first display
    display_count = 1;
    err = vkGetPhysicalDeviceDisplayPropertiesKHR(demo->gpu, &display_count, &display_props);
    assert(!err || (err == VK_INCOMPLETE));

    display = display_props.display;

    // Get the first mode of the display
    err = vkGetDisplayModePropertiesKHR(demo->gpu, display, &mode_count, NULL);
    assert(!err);

    if (mode_count == 0) {
        printf("Cannot find any mode for the display!\n");
        fflush(stdout);
        exit(1);
    }

    mode_count = 1;
    err = vkGetDisplayModePropertiesKHR(demo->gpu, display, &mode_count, &mode_props);
    assert(!err || (err == VK_INCOMPLETE));

    // Get the list of planes
    err = vkGetPhysicalDeviceDisplayPlanePropertiesKHR(demo->gpu, &plane_count, NULL);
    assert(!err);

    if (plane_count == 0) {
        printf("Cannot find any plane!\n");
        fflush(stdout);
        exit(1);
    }

    plane_props = malloc(sizeof(VkDisplayPlanePropertiesKHR) * plane_count);
    assert(plane_props);

    err = vkGetPhysicalDeviceDisplayPlanePropertiesKHR(demo->gpu, &plane_count, plane_props);
    assert(!err);

    // Find a plane compatible with the display
    for (plane_index = 0; plane_index < plane_count; plane_index++) {
        uint32_t supported_count;
        VkDisplayKHR *supported_displays;

        // Disqualify planes that are bound to a different display
        if ((plane_props[plane_index].currentDisplay != VK_NULL_HANDLE) && (plane_props[plane_index].currentDisplay != display)) {
            continue;
        }

        err = vkGetDisplayPlaneSupportedDisplaysKHR(demo->gpu, plane_index, &supported_count, NULL);
        assert(!err);

        if (supported_count == 0) {
            continue;
        }

        supported_displays = malloc(sizeof(VkDisplayKHR) * supported_count);
        assert(supported_displays);

        err = vkGetDisplayPlaneSupportedDisplaysKHR(demo->gpu, plane_index, &supported_count, supported_displays);
        assert(!err);

        for (uint32_t i = 0; i < supported_count; i++) {
            if (supported_displays[i] == display) {
                found_plane = VK_TRUE;
                break;
            }
        }

        free(supported_displays);

        if (found_plane) {
            break;
        }
    }

    if (!found_plane) {
        printf("Cannot find a plane compatible with the display!\n");
        fflush(stdout);
        exit(1);
    }

    VkDisplayPlaneCapabilitiesKHR planeCaps;
    vkGetDisplayPlaneCapabilitiesKHR(demo->gpu, mode_props.displayMode, plane_index, &planeCaps);
    // Find a supported alpha mode
    VkDisplayPlaneAlphaFlagBitsKHR alphaMode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR;
    VkDisplayPlaneAlphaFlagBitsKHR alphaModes[4] = {
        VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR,
        VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR,
        VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR,
        VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT_KHR,
    };
    for (uint32_t i = 0; i < sizeof(alphaModes); i++) {
        if (planeCaps.supportedAlpha & alphaModes[i]) {
            alphaMode = alphaModes[i];
            break;
        }
    }
    image_extent.width = mode_props.parameters.visibleRegion.width;
    image_extent.height = mode_props.parameters.visibleRegion.height;

    create_info.sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.displayMode = mode_props.displayMode;
    create_info.planeIndex = plane_index;
    create_info.planeStackIndex = plane_props[plane_index].currentStackIndex;
    create_info.transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    create_info.alphaMode = alphaMode;
    create_info.globalAlpha = 1.0f;
    create_info.imageExtent = image_extent;

    free(plane_props);

    return vkCreateDisplayPlaneSurfaceKHR(demo->inst, &create_info, NULL, &demo->surface);
}

static void demo_run_display(struct demo *demo) {
    while (!demo->quit) {
        demo_draw(demo);
        demo->curFrame++;

        if (demo->frameCount != INT32_MAX && demo->curFrame == demo->frameCount) {
            demo->quit = true;
        }
    }
}
#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)

#include <sys/keycodes.h>

static void demo_run(struct demo *demo) {
    int size[2] = {0, 0};
    screen_window_t win;
    int val;
    int rc;

    while (!demo->quit) {
        while (!screen_get_event(demo->screen_context, demo->screen_event, demo->pause ? ~0 : 0)) {
            rc = screen_get_event_property_iv(demo->screen_event, SCREEN_PROPERTY_TYPE, &val);
            if (rc) {
                printf("Cannot get SCREEN_PROPERTY_TYPE of the event! (%s)\n", strerror(errno));
                fflush(stdout);
                demo->quit = true;
                break;
            }
            if (val == SCREEN_EVENT_NONE) {
                break;
            }
            switch (val) {
                case SCREEN_EVENT_KEYBOARD:
                    rc = screen_get_event_property_iv(demo->screen_event, SCREEN_PROPERTY_FLAGS, &val);
                    if (rc) {
                        printf("Cannot get SCREEN_PROPERTY_FLAGS of the event! (%s)\n", strerror(errno));
                        fflush(stdout);
                        demo->quit = true;
                        break;
                    }
                    if (val & KEY_DOWN) {
                        rc = screen_get_event_property_iv(demo->screen_event, SCREEN_PROPERTY_SYM, &val);
                        if (rc) {
                            printf("Cannot get SCREEN_PROPERTY_SYM of the event! (%s)\n", strerror(errno));
                            fflush(stdout);
                            demo->quit = true;
                            break;
                        }
                        switch (val) {
                            case KEYCODE_ESCAPE:
                                demo->quit = true;
                                break;
                            case KEYCODE_SPACE:
                                demo->pause = !demo->pause;
                                break;
                            case KEYCODE_LEFT:
                                demo->spin_angle -= demo->spin_increment;
                                break;
                            case KEYCODE_RIGHT:
                                demo->spin_angle += demo->spin_increment;
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                case SCREEN_EVENT_PROPERTY:
                    rc = screen_get_event_property_pv(demo->screen_event, SCREEN_PROPERTY_WINDOW, (void **)&win);
                    if (rc) {
                        printf("Cannot get SCREEN_PROPERTY_WINDOW of the event! (%s)\n", strerror(errno));
                        fflush(stdout);
                        demo->quit = true;
                        break;
                    }
                    rc = screen_get_event_property_iv(demo->screen_event, SCREEN_PROPERTY_NAME, &val);
                    if (rc) {
                        printf("Cannot get SCREEN_PROPERTY_NAME of the event! (%s)\n", strerror(errno));
                        fflush(stdout);
                        demo->quit = true;
                        break;
                    }
                    if (win == demo->screen_window) {
                        switch (val) {
                            case SCREEN_PROPERTY_SIZE:
                                rc = screen_get_window_property_iv(win, SCREEN_PROPERTY_SIZE, size);
                                if (rc) {
                                    printf("Cannot get SCREEN_PROPERTY_SIZE of the window in the event! (%s)\n", strerror(errno));
                                    fflush(stdout);
                                    demo->quit = true;
                                    break;
                                }
                                demo->width = size[0];
                                demo->height = size[1];
                                demo_resize(demo);
                                break;
                            default:
                                /* We are not interested in any other events for now */
                                break;
                        }
                    }
                    break;
            }
        }

        if (demo->pause) {
        } else {
            demo_draw(demo);
            demo->curFrame++;
            if (demo->frameCount != INT32_MAX && demo->curFrame == demo->frameCount) {
                demo->quit = true;
            }
        }
    }
}

static void demo_create_screen_window(struct demo *demo) {
    const char *idstr = APP_SHORT_NAME;
    int size[2];
    int usage = SCREEN_USAGE_VULKAN;
    int rc;

    rc = screen_create_context(&demo->screen_context, 0);
    if (rc) {
        printf("Cannot create QNX Screen context!\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    rc = screen_create_window(&demo->screen_window, demo->screen_context);
    if (rc) {
        printf("Cannot create QNX Screen window!\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    rc = screen_create_event(&demo->screen_event);
    if (rc) {
        printf("Cannot create QNX Screen event!\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    /* Set window caption */
    screen_set_window_property_cv(demo->screen_window, SCREEN_PROPERTY_ID_STRING, strlen(idstr), idstr);

    /* Setup VULKAN usage flags */
    rc = screen_set_window_property_iv(demo->screen_window, SCREEN_PROPERTY_USAGE, &usage);
    if (rc) {
        printf("Cannot set SCREEN_USAGE_VULKAN flag!\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    /* Setup window size */
    if ((demo->width == 0) || (demo->height == 0)) {
        /* Obtain automatically set window size provided by WM */
        rc = screen_get_window_property_iv(demo->screen_window, SCREEN_PROPERTY_SIZE, size);
        if (rc) {
            printf("Cannot obtain current window size!\n");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        demo->width = size[0];
        demo->height = size[1];
    } else {
        size[0] = demo->width;
        size[1] = demo->height;
        rc = screen_set_window_property_iv(demo->screen_window, SCREEN_PROPERTY_SIZE, size);
        if (rc) {
            printf("Cannot set window size!\n");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
    }
}
#endif

/*
 * Return 1 (true) if all layer names specified in check_names
 * can be found in given layer properties.
 */
static VkBool32 demo_check_layers(uint32_t check_count, char **check_names, uint32_t layer_count, VkLayerProperties *layers) {
    for (uint32_t i = 0; i < check_count; i++) {
        VkBool32 found = 0;
        for (uint32_t j = 0; j < layer_count; j++) {
            if (!strcmp(check_names[i], layers[j].layerName)) {
                found = 1;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "Cannot find layer: %s\n", check_names[i]);
            return 0;
        }
    }
    return 1;
}
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
int find_display_gpu(int gpu_number, uint32_t gpu_count, VkPhysicalDevice *physical_devices) {
    uint32_t display_count = 0;
    VkResult U_ASSERT_ONLY result;
    int gpu_return = gpu_number;
    if (gpu_number >= 0) {
        result = vkGetPhysicalDeviceDisplayPropertiesKHR(physical_devices[gpu_number], &display_count, NULL);
        assert(!result);
    } else {
        for (uint32_t i = 0; i < gpu_count; i++) {
            result = vkGetPhysicalDeviceDisplayPropertiesKHR(physical_devices[i], &display_count, NULL);
            assert(!result);
            if (display_count) {
                gpu_return = i;
                break;
            }
        }
    }
    if (display_count > 0)
        return gpu_return;
    else
        return -1;
}
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
static void pointer_handle_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t sx,
                                 wl_fixed_t sy) {}

static void pointer_handle_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface) {}

static void pointer_handle_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy) {}

static void pointer_handle_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button,
                                  uint32_t state) {
    struct demo *demo = data;
    if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED) {
        xdg_toplevel_move(demo->xdg_toplevel, demo->seat, serial);
    }
}

static void pointer_handle_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {}

static const struct wl_pointer_listener pointer_listener = {
    pointer_handle_enter, pointer_handle_leave, pointer_handle_motion, pointer_handle_button, pointer_handle_axis,
};

static void keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size) {}

static void keyboard_handle_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface,
                                  struct wl_array *keys) {}

static void keyboard_handle_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface) {}

static void keyboard_handle_key(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key,
                                uint32_t state) {
    if (state != WL_KEYBOARD_KEY_STATE_RELEASED) return;
    struct demo *demo = data;
    switch (key) {
        case KEY_ESC:  // Escape
            demo->quit = true;
            break;
        case KEY_LEFT:  // left arrow key
            demo->spin_angle -= demo->spin_increment;
            break;
        case KEY_RIGHT:  // right arrow key
            demo->spin_angle += demo->spin_increment;
            break;
        case KEY_SPACE:  // space bar
            demo->pause = !demo->pause;
            break;
    }
}

static void keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed,
                                      uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {}

static const struct wl_keyboard_listener keyboard_listener = {
    keyboard_handle_keymap, keyboard_handle_enter, keyboard_handle_leave, keyboard_handle_key, keyboard_handle_modifiers,
};

static void seat_handle_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability caps) {
    // Subscribe to pointer events
    struct demo *demo = data;
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !demo->pointer) {
        demo->pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(demo->pointer, &pointer_listener, demo);
    } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && demo->pointer) {
        wl_pointer_destroy(demo->pointer);
        demo->pointer = NULL;
    }
    // Subscribe to keyboard events
    if (caps & WL_SEAT_CAPABILITY_KEYBOARD) {
        demo->keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(demo->keyboard, &keyboard_listener, demo);
    } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && demo->keyboard) {
        wl_keyboard_destroy(demo->keyboard);
        demo->keyboard = NULL;
    }
}

static const struct wl_seat_listener seat_listener = {
    seat_handle_capabilities,
};

static void wm_base_ping(void *data UNUSED, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener wm_base_listener = {wm_base_ping};

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t id, const char *interface,
                                   uint32_t version UNUSED) {
    struct demo *demo = data;
    // pickup wayland objects when they appear
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        uint32_t minVersion = version < 4 ? version : 4;
        demo->compositor = wl_registry_bind(registry, id, &wl_compositor_interface, minVersion);
        if (demo->VK_KHR_incremental_present_enabled && minVersion < 4) {
            fprintf(stderr, "Wayland compositor doesn't support VK_KHR_incremental_present, disabling.\n");
            demo->VK_KHR_incremental_present_enabled = false;
        }
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        demo->xdg_wm_base = wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(demo->xdg_wm_base, &wm_base_listener, NULL);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        demo->seat = wl_registry_bind(registry, id, &wl_seat_interface, 1);
        wl_seat_add_listener(demo->seat, &seat_listener, demo);
    } else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
        demo->xdg_decoration_mgr = wl_registry_bind(registry, id, &zxdg_decoration_manager_v1_interface, 1);
    }
}

static void registry_handle_global_remove(void *data UNUSED, struct wl_registry *registry UNUSED, uint32_t name UNUSED) {}

static const struct wl_registry_listener registry_listener = {registry_handle_global, registry_handle_global_remove};
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
static const char *demo_init_xcb_connection(struct demo *demo) {
    demo->xcb_library = initialize_xcb();
    if (NULL == demo->xcb_library) {
        return "Cannot load XCB dynamic library.";
    }

    const xcb_setup_t *setup;
    xcb_screen_iterator_t iter;
    int scr;

    const char *display_envar = getenv("DISPLAY");
    if (display_envar == NULL || display_envar[0] == '\0') {
        return "Environment variable DISPLAY requires a valid value.n";
    }

    demo->connection = xcb_connect(NULL, &scr);
    if (xcb_connection_has_error(demo->connection) > 0) {
        return "Cannot connect to XCB.";
    }

    setup = xcb_get_setup(demo->connection);
    iter = xcb_setup_roots_iterator(setup);
    while (scr-- > 0) xcb_screen_next(&iter);

    demo->screen = iter.data;
    return NULL;
}
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
static const char *demo_init_xlib_connection(struct demo *demo) {
    demo->xlib_library = initialize_xlib();
    if (NULL == demo->xlib_library) {
        return "Cannot load XLIB dynamic library.";
    }
    return NULL;
}
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
static const char *demo_init_wayland_connection(struct demo *demo) {
    demo->wayland_library = initialize_wayland();
    if (NULL == demo->wayland_library) {
        return "Cannot load wayland dynamic library.";
    }

    demo->wayland_display = wl_display_connect(NULL);

    if (demo->wayland_display == NULL) {
        return "Cannot connect to wayland.";
    }

    demo->registry = wl_display_get_registry(demo->wayland_display);
    wl_registry_add_listener(demo->registry, &registry_listener, demo);
    wl_display_roundtrip(demo->wayland_display);
    return NULL;
}
#endif

// Check that WSI platforms are available - only necessary when multiple WSI platforms exist, like on linux
// If the wsi_platform is AUTO, this function also sets wsi_platform to the first available WSI platform
// Otherwise, it errors out if the specified wsi_platform isn't available
static void demo_check_and_set_wsi_platform(struct demo *demo) {
#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (demo->wsi_platform == WSI_PLATFORM_XCB || demo->wsi_platform == WSI_PLATFORM_AUTO) {
        bool xcb_extension_available = false;
        for (uint32_t i = 0; i < demo->enabled_extension_count; i++) {
            if (strcmp(demo->extension_names[i], VK_KHR_XCB_SURFACE_EXTENSION_NAME) == 0) {
                xcb_extension_available = true;
                break;
            }
        }
        if (xcb_extension_available) {
            const char *error_msg = demo_init_xcb_connection(demo);
            if (error_msg != NULL) {
                if (demo->wsi_platform == WSI_PLATFORM_XCB) {
                    fprintf(stderr, "%s\nExiting ...\n", error_msg);
                    fflush(stdout);
                    exit(1);
                }
            } else {
                demo->wsi_platform = WSI_PLATFORM_XCB;
                return;
            }
        }
    }
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (demo->wsi_platform == WSI_PLATFORM_XLIB || demo->wsi_platform == WSI_PLATFORM_AUTO) {
        bool xlib_extension_available = false;
        for (uint32_t i = 0; i < demo->enabled_extension_count; i++) {
            if (strcmp(demo->extension_names[i], VK_KHR_XLIB_SURFACE_EXTENSION_NAME) == 0) {
                xlib_extension_available = true;
                break;
            }
        }
        if (xlib_extension_available) {
            const char *error_msg = demo_init_xlib_connection(demo);
            if (error_msg != NULL) {
                if (demo->wsi_platform == WSI_PLATFORM_XLIB) {
                    fprintf(stderr, "%s\nExiting ...\n", error_msg);
                    fflush(stdout);
                    exit(1);
                }
            } else {
                demo->wsi_platform = WSI_PLATFORM_XLIB;
                return;
            }
        }
    }
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    if (demo->wsi_platform == WSI_PLATFORM_WAYLAND || demo->wsi_platform == WSI_PLATFORM_AUTO) {
        bool wayland_extension_available = false;
        for (uint32_t i = 0; i < demo->enabled_extension_count; i++) {
            if (strcmp(demo->extension_names[i], VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME) == 0) {
                wayland_extension_available = true;
                break;
            }
        }
        if (wayland_extension_available) {
            const char *error_msg = demo_init_wayland_connection(demo);
            if (error_msg != NULL) {
                if (demo->wsi_platform == WSI_PLATFORM_WAYLAND) {
                    fprintf(stderr, "%s\nExiting ...\n", error_msg);
                    fflush(stdout);
                    exit(1);
                }
            } else {
                demo->wsi_platform = WSI_PLATFORM_WAYLAND;
                return;
            }
        }
    }
#endif
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    if (demo->wsi_platform == WSI_PLATFORM_DIRECTFB || demo->wsi_platform == WSI_PLATFORM_AUTO) {
        bool direftfb_extension_available = false;
        for (uint32_t i = 0; i < demo->enabled_extension_count; i++) {
            if (strcmp(demo->extension_names[i], VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME) == 0) {
                direftfb_extension_available = true;
                break;
            }
        }
        if (direftfb_extension_available) {
            // Because DirectFB is still linked in, we can assume that it works if we got here
            demo->wsi_platform = WSI_PLATFORM_DIRECTFB;
            return;
        }
    }
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (demo->wsi_platform == WSI_PLATFORM_WIN32 || demo->wsi_platform == WSI_PLATFORM_AUTO) {
        bool win32_extension_available = false;
        for (uint32_t i = 0; i < demo->enabled_extension_count; i++) {
            if (strcmp(demo->extension_names[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0) {
                win32_extension_available = true;
                break;
            }
        }
        if (win32_extension_available) {
            demo->wsi_platform = WSI_PLATFORM_WIN32;
            return;
        }
    }
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (demo->wsi_platform == WSI_PLATFORM_METAL || demo->wsi_platform == WSI_PLATFORM_AUTO) {
        bool metal_extension_available = false;
        for (uint32_t i = 0; i < demo->enabled_extension_count; i++) {
            if (strcmp(demo->extension_names[i], VK_EXT_METAL_SURFACE_EXTENSION_NAME) == 0) {
                metal_extension_available = true;
                break;
            }
        }
        if (metal_extension_available) {
            demo->wsi_platform = WSI_PLATFORM_METAL;
            return;
        }
    }
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (demo->wsi_platform == WSI_PLATFORM_ANDROID || demo->wsi_platform == WSI_PLATFORM_AUTO) {
        bool android_extension_available = false;
        for (uint32_t i = 0; i < demo->enabled_extension_count; i++) {
            if (strcmp(demo->extension_names[i], VK_KHR_ANDROID_SURFACE_EXTENSION_NAME) == 0) {
                android_extension_available = true;
                break;
            }
        }
        if (android_extension_available) {
            demo->wsi_platform = WSI_PLATFORM_ANDROID;
            return;
        }
    }
#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    if (demo->wsi_platform == WSI_PLATFORM_QNX || demo->wsi_platform == WSI_PLATFORM_AUTO) {
        bool qnx_extension_available = false;
        for (uint32_t i = 0; i < demo->enabled_extension_count; i++) {
            if (strcmp(demo->extension_names[i], VK_QNX_SCREEN_SURFACE_EXTENSION_NAME) == 0) {
                qnx_extension_available = true;
                break;
            }
        }
        if (qnx_extension_available) {
            demo->wsi_platform = WSI_PLATFORM_ANDROID;
            return;
        }
    }
#endif
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
    if (demo->wsi_platform == WSI_PLATFORM_DISPLAY || demo->wsi_platform == WSI_PLATFORM_AUTO) {
        bool display_extension_available = false;
        for (uint32_t i = 0; i < demo->enabled_extension_count; i++) {
            if (strcmp(demo->extension_names[i], VK_KHR_DISPLAY_EXTENSION_NAME) == 0) {
                display_extension_available = true;
                break;
            }
        }
        if (display_extension_available) {
            // Because DISPLAY doesn't require additional libraries, we can assume that it works if we got here
            demo->wsi_platform = WSI_PLATFORM_DISPLAY;
            return;
        }
    }
#endif
}

static void demo_init_vk(struct demo *demo) {
    VkResult err;
    uint32_t instance_extension_count = 0;
    uint32_t instance_layer_count = 0;
    char *instance_validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
    demo->enabled_extension_count = 0;
    demo->enabled_layer_count = 0;
    demo->is_minimized = false;
    demo->cmd_pool = VK_NULL_HANDLE;

    err = volkInitialize();
    if (err != VK_SUCCESS) {
        ERR_EXIT(
            "Unable to find the Vulkan runtime on the system.\n\n"
            "This likely indicates that no Vulkan capable drivers are installed.",
            "Installation Failure");
    }
    // Look for validation layers
    VkBool32 validation_found = 0;
    if (demo->validate) {
        err = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
        assert(!err);

        if (instance_layer_count > 0) {
            VkLayerProperties *instance_layers = malloc(sizeof(VkLayerProperties) * instance_layer_count);
            err = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers);
            assert(!err);

            validation_found = demo_check_layers(ARRAY_SIZE(instance_validation_layers), instance_validation_layers,
                                                 instance_layer_count, instance_layers);
            if (validation_found) {
                demo->enabled_layer_count = ARRAY_SIZE(instance_validation_layers);
                demo->enabled_layers[0] = "VK_LAYER_KHRONOS_validation";
            }
            free(instance_layers);
        }

        if (!validation_found) {
            ERR_EXIT(
                "vkEnumerateInstanceLayerProperties failed to find required validation layer.\n\n"
                "Please look at the Getting Started guide for additional information.\n",
                "vkCreateInstance Failure");
        }
    }

    /* Look for instance extensions */
    VkBool32 surfaceExtFound = false;
    VkBool32 platformSurfaceExtFound = false;
    bool portabilityEnumerationActive = false;
    memset(demo->extension_names, 0, sizeof(demo->extension_names));

    err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
    assert(!err);

    if (instance_extension_count > 0) {
        VkExtensionProperties *instance_extensions = malloc(sizeof(VkExtensionProperties) * instance_extension_count);
        err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, instance_extensions);
        assert(!err);
        for (uint32_t i = 0; i < instance_extension_count; i++) {
            if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                surfaceExtFound = true;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
            }
#if defined(VK_USE_PLATFORM_WIN32_KHR)
            if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) &&
                (demo->wsi_platform == WSI_PLATFORM_AUTO || demo->wsi_platform == WSI_PLATFORM_WIN32)) {
                platformSurfaceExtFound = true;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
            }
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
            if (!strcmp(VK_KHR_XLIB_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) &&
                (demo->wsi_platform == WSI_PLATFORM_AUTO || demo->wsi_platform == WSI_PLATFORM_XLIB)) {
                platformSurfaceExtFound = true;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
            }
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
            if (!strcmp(VK_KHR_XCB_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) &&
                (demo->wsi_platform == WSI_PLATFORM_AUTO || demo->wsi_platform == WSI_PLATFORM_XCB)) {
                platformSurfaceExtFound = true;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
            }
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
            if (!strcmp(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) &&
                (demo->wsi_platform == WSI_PLATFORM_AUTO || demo->wsi_platform == WSI_PLATFORM_WAYLAND)) {
                platformSurfaceExtFound = true;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
            }
#endif
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
            if (!strcmp(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) &&
                (demo->wsi_platform == WSI_PLATFORM_AUTO || demo->wsi_platform == WSI_PLATFORM_DIRECTFB)) {
                platformSurfaceExtFound = true;
                demo->extension_names[demo->enabled_extension_count++] = VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME;
            }
#endif
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
            if (!strcmp(VK_KHR_DISPLAY_EXTENSION_NAME, instance_extensions[i].extensionName) &&
                (demo->wsi_platform == WSI_PLATFORM_AUTO || demo->wsi_platform == WSI_PLATFORM_DISPLAY)) {
                platformSurfaceExtFound = true;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_DISPLAY_EXTENSION_NAME;
            }
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
            if (!strcmp(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) &&
                (demo->wsi_platform == WSI_PLATFORM_AUTO || demo->wsi_platform == WSI_PLATFORM_ANDROID)) {
                platformSurfaceExtFound = true;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
            }
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
            if (!strcmp(VK_EXT_METAL_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) &&
                (demo->wsi_platform == WSI_PLATFORM_AUTO || demo->wsi_platform == WSI_PLATFORM_METAL)) {
                platformSurfaceExtFound = true;
                demo->extension_names[demo->enabled_extension_count++] = VK_EXT_METAL_SURFACE_EXTENSION_NAME;
            }
#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
            if (!strcmp(VK_QNX_SCREEN_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) &&
                (demo->wsi_platform == WSI_PLATFORM_AUTO || demo->wsi_platform == WSI_PLATFORM_QNX)) {
                platformSurfaceExtFound = true;
                demo->extension_names[demo->enabled_extension_count++] = VK_QNX_SCREEN_SURFACE_EXTENSION_NAME;
            }
#endif
            if (!strcmp(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
            }
            if (!strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                if (demo->validate) {
                    demo->extension_names[demo->enabled_extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
                }
            }
            // We want cube to be able to enumerate drivers that support the portability_subset extension, so we have to enable
            // the portability enumeration extension.
            if (!strcmp(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                portabilityEnumerationActive = true;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
            }
            assert(demo->enabled_extension_count < 64);
        }

        free(instance_extensions);
    }

    if (!surfaceExtFound) {
        ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_SURFACE_EXTENSION_NAME
                 " extension.\n\n"
                 "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                 "Please look at the Getting Started guide for additional information.\n",
                 "vkCreateInstance Failure");
    }
    if (!platformSurfaceExtFound) {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        if (demo->wsi_platform == WSI_PLATFORM_WIN32) {
            ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_WIN32_SURFACE_EXTENSION_NAME
                     " extension.\n\n"
                     "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                     "Please look at the Getting Started guide for additional information.\n",
                     "vkCreateInstance Failure");
        }
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
        if (demo->wsi_platform == WSI_PLATFORM_METAL) {
            ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_EXT_METAL_SURFACE_EXTENSION_NAME
                     " extension.\n\n"
                     "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                     "Please look at the Getting Started guide for additional information.\n",
                     "vkCreateInstance Failure");
        }
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
        if (demo->wsi_platform == WSI_PLATFORM_XCB) {
            ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_XCB_SURFACE_EXTENSION_NAME
                     " extension.\n\n"
                     "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                     "Please look at the Getting Started guide for additional information.\n",
                     "vkCreateInstance Failure");
        }
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
        if (demo->wsi_platform == WSI_PLATFORM_WAYLAND) {
            ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
                     " extension.\n\n"
                     "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                     "Please look at the Getting Started guide for additional information.\n",
                     "vkCreateInstance Failure");
        }
#endif
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
        if (demo->wsi_platform == WSI_PLATFORM_DISPLAY) {
            ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_DISPLAY_EXTENSION_NAME
                     " extension.\n\n"
                     "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                     "Please look at the Getting Started guide for additional information.\n",
                     "vkCreateInstance Failure");
        }
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        if (demo->wsi_platform == WSI_PLATFORM_ANDROID) {
            ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
                     " extension.\n\n"
                     "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                     "Please look at the Getting Started guide for additional information.\n",
                     "vkCreateInstance Failure");
        }
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        if (demo->wsi_platform == WSI_PLATFORM_XLIB) {
            ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_XLIB_SURFACE_EXTENSION_NAME
                     " extension.\n\n"
                     "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                     "Please look at the Getting Started guide for additional information.\n",
                     "vkCreateInstance Failure");
        }
#endif
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
        if (demo->wsi_platform == WSI_PLATFORM_DIRECTFB) {
            ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME
                     " extension.\n\n"
                     "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                     "Please look at the Getting Started guide for additional information.\n",
                     "vkCreateInstance Failure");
        }
#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
        if (demo->wsi_platform == WSI_PLATFORM_WIN32) {
            ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_QNX_SCREEN_SURFACE_EXTENSION_NAME
                     " extension.\n\n"
                     "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                     "Please look at the Getting Started guide for additional information.\n",
                     "vkCreateInstance Failure");
        }
#endif
        ERR_EXIT(
            "vkEnumerateInstanceExtensionProperties failed to find any supported WSI surface extension.\n\n"
            "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
            "Please look at the Getting Started guide for additional information.\n",
            "vkCreateInstance Failure");
    }

    bool auto_wsi_platform = demo->wsi_platform == WSI_PLATFORM_AUTO;

    demo_check_and_set_wsi_platform(demo);

    // Print a message to indicate the automatically set WSI platform
    if (auto_wsi_platform && demo->wsi_platform != WSI_PLATFORM_AUTO) {
        fprintf(stderr, "Selected WSI platform: %s\n", wsi_to_string(demo->wsi_platform));
    }

    const VkApplicationInfo app = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = APP_SHORT_NAME,
        .applicationVersion = 0,
        .pEngineName = APP_SHORT_NAME,
        .engineVersion = 0,
        .apiVersion = VK_API_VERSION_1_0,
    };
    VkInstanceCreateInfo inst_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .flags = (portabilityEnumerationActive ? VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR : 0),
        .pApplicationInfo = &app,
        .enabledLayerCount = demo->enabled_layer_count,
        .ppEnabledLayerNames = (const char *const *)instance_validation_layers,
        .enabledExtensionCount = demo->enabled_extension_count,
        .ppEnabledExtensionNames = (const char *const *)demo->extension_names,
    };

    /*
     * This is info for a temp callback to use during CreateInstance.
     * After the instance is created, we use the instance-based
     * function to register the final callback.
     */
    VkDebugUtilsMessengerCreateInfoEXT dbg_messenger_create_info;
    if (demo->validate) {
        // VK_EXT_debug_utils style
        dbg_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        dbg_messenger_create_info.pNext = NULL;
        dbg_messenger_create_info.flags = 0;
        dbg_messenger_create_info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        dbg_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        dbg_messenger_create_info.pfnUserCallback = debug_messenger_callback;
        dbg_messenger_create_info.pUserData = demo;
        inst_info.pNext = &dbg_messenger_create_info;
    }

    err = vkCreateInstance(&inst_info, NULL, &demo->inst);
    if (err == VK_ERROR_INCOMPATIBLE_DRIVER) {
        ERR_EXIT(
            "Cannot find a compatible Vulkan installable client driver (ICD).\n\n"
            "Please look at the Getting Started guide for additional information.\n",
            "vkCreateInstance Failure");
    } else if (err == VK_ERROR_EXTENSION_NOT_PRESENT) {
        ERR_EXIT(
            "Cannot find a specified extension library.\n"
            "Make sure your layers path is set appropriately.\n",
            "vkCreateInstance Failure");
    } else if (err) {
        ERR_EXIT(
            "vkCreateInstance failed.\n\n"
            "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
            "Please look at the Getting Started guide for additional information.\n",
            "vkCreateInstance Failure");
    }

    volkLoadInstance(demo->inst);
}

static void demo_select_physical_device(struct demo *demo) {
    VkResult err;
    /* Make initial call to query gpu_count, then second call for gpu info */
    uint32_t gpu_count = 0;
    err = vkEnumeratePhysicalDevices(demo->inst, &gpu_count, NULL);
    assert(!err);

    if (gpu_count <= 0) {
        ERR_EXIT(
            "vkEnumeratePhysicalDevices reported zero accessible devices.\n\n"
            "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
            "Please look at the Getting Started guide for additional information.\n",
            "vkEnumeratePhysicalDevices Failure");
    }

    VkPhysicalDevice *physical_devices = malloc(sizeof(VkPhysicalDevice) * gpu_count);
    err = vkEnumeratePhysicalDevices(demo->inst, &gpu_count, physical_devices);
    assert(!err);
    if (demo->invalid_gpu_selection || (demo->gpu_number >= 0 && !((uint32_t)demo->gpu_number < gpu_count))) {
        fprintf(stderr, "GPU %d specified is not present, GPU count = %u\n", demo->gpu_number, gpu_count);
        ERR_EXIT("Specified GPU number is not present", "User Error");
    }

    if (demo->wsi_platform == WSI_PLATFORM_DISPLAY) {
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
        demo->gpu_number = find_display_gpu(demo->gpu_number, gpu_count, physical_devices);
        if (demo->gpu_number < 0) {
            printf("Cannot find any display!\n");
            fflush(stdout);
            exit(1);
        }
#else
        printf("WSI selection was set to DISPLAY but vkcube was not compiled with support for the DISPLAY platform, exiting \n");
        fflush(stdout);
        exit(1);
#endif
    } else {
        /* Try to auto select most suitable device */
        if (demo->gpu_number == -1) {
            VkPhysicalDeviceProperties physicalDeviceProperties;
            int prev_priority = 0;
            for (uint32_t i = 0; i < gpu_count; i++) {
                vkGetPhysicalDeviceProperties(physical_devices[i], &physicalDeviceProperties);
                assert(physicalDeviceProperties.deviceType <= VK_PHYSICAL_DEVICE_TYPE_CPU);

                // Continue next gpu if this gpu does not support the surface.
                VkBool32 supported = VK_FALSE;
                VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physical_devices[i], 0, demo->surface, &supported);
                if (result != VK_SUCCESS || !supported) continue;

                int priority = 0;
                switch (physicalDeviceProperties.deviceType) {
                    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                        priority = 5;
                        break;
                    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                        priority = 4;
                        break;
                    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                        priority = 3;
                        break;
                    case VK_PHYSICAL_DEVICE_TYPE_CPU:
                        priority = 2;
                        break;
                    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                        priority = 1;
                        break;
                    default:
                        priority = -1;
                        break;
                }

                if (priority > prev_priority) {
                    demo->gpu_number = i;
                    prev_priority = priority;
                }
            }
        }
    }

    assert(demo->gpu_number >= 0);
    demo->gpu = physical_devices[demo->gpu_number];
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(demo->gpu, &physicalDeviceProperties);
        fprintf(stderr, "Selected GPU %d: %s, type: %s\n", demo->gpu_number, physicalDeviceProperties.deviceName,
                to_string(physicalDeviceProperties.deviceType));
    }
    free(physical_devices);

    /* Look for device extensions */
    uint32_t device_extension_count = 0;
    VkBool32 swapchainExtFound = 0;
    demo->enabled_extension_count = 0;
    memset(demo->extension_names, 0, sizeof(demo->extension_names));

    err = vkEnumerateDeviceExtensionProperties(demo->gpu, NULL, &device_extension_count, NULL);
    assert(!err);

    if (device_extension_count > 0) {
        VkExtensionProperties *device_extensions = malloc(sizeof(VkExtensionProperties) * device_extension_count);
        err = vkEnumerateDeviceExtensionProperties(demo->gpu, NULL, &device_extension_count, device_extensions);
        assert(!err);

        for (uint32_t i = 0; i < device_extension_count; i++) {
            if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, device_extensions[i].extensionName)) {
                swapchainExtFound = 1;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            }
            if (!strcmp("VK_KHR_portability_subset", device_extensions[i].extensionName)) {
                demo->extension_names[demo->enabled_extension_count++] = "VK_KHR_portability_subset";
            }
            assert(demo->enabled_extension_count < 64);
        }

        if (demo->VK_KHR_incremental_present_enabled) {
            // Even though the user "enabled" the extension via the command
            // line, we must make sure that it's enumerated for use with the
            // device.  Therefore, disable it here, and re-enable it again if
            // enumerated.
            demo->VK_KHR_incremental_present_enabled = false;
            for (uint32_t i = 0; i < device_extension_count; i++) {
                if (!strcmp(VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME, device_extensions[i].extensionName)) {
                    demo->extension_names[demo->enabled_extension_count++] = VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME;
                    demo->VK_KHR_incremental_present_enabled = true;
                    DbgMsg("VK_KHR_incremental_present extension enabled\n");
                }
                assert(demo->enabled_extension_count < 64);
            }
            if (!demo->VK_KHR_incremental_present_enabled) {
                DbgMsg("VK_KHR_incremental_present extension NOT AVAILABLE\n");
            }
        }

        if (demo->VK_GOOGLE_display_timing_enabled) {
            // Even though the user "enabled" the extension via the command
            // line, we must make sure that it's enumerated for use with the
            // device.  Therefore, disable it here, and re-enable it again if
            // enumerated.
            demo->VK_GOOGLE_display_timing_enabled = false;
            for (uint32_t i = 0; i < device_extension_count; i++) {
                if (!strcmp(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME, device_extensions[i].extensionName)) {
                    demo->extension_names[demo->enabled_extension_count++] = VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME;
                    demo->VK_GOOGLE_display_timing_enabled = true;
                    DbgMsg("VK_GOOGLE_display_timing extension enabled\n");
                }
                assert(demo->enabled_extension_count < 64);
            }
            if (!demo->VK_GOOGLE_display_timing_enabled) {
                DbgMsg("VK_GOOGLE_display_timing extension NOT AVAILABLE\n");
            }
        }

        free(device_extensions);
    }

    if (!swapchainExtFound) {
        ERR_EXIT("vkEnumerateDeviceExtensionProperties failed to find the " VK_KHR_SWAPCHAIN_EXTENSION_NAME
                 " extension.\n\nDo you have a compatible Vulkan installable client driver (ICD) installed?\n"
                 "Please look at the Getting Started guide for additional information.\n",
                 "vkCreateInstance Failure");
    }

    if (demo->validate) {
        /*
         * This is info for a temp callback to use during CreateInstance.
         * After the instance is created, we use the instance-based
         * function to register the final callback.
         */
        VkDebugUtilsMessengerCreateInfoEXT dbg_messenger_create_info;
        // VK_EXT_debug_utils style
        dbg_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        dbg_messenger_create_info.pNext = NULL;
        dbg_messenger_create_info.flags = 0;
        dbg_messenger_create_info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        dbg_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        dbg_messenger_create_info.pfnUserCallback = debug_messenger_callback;
        dbg_messenger_create_info.pUserData = demo;

        err = vkCreateDebugUtilsMessengerEXT(demo->inst, &dbg_messenger_create_info, NULL, &demo->dbg_messenger);
        switch (err) {
            case VK_SUCCESS:
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                ERR_EXIT("CreateDebugUtilsMessengerEXT: out of host memory\n", "CreateDebugUtilsMessengerEXT Failure");
                break;
            default:
                ERR_EXIT("CreateDebugUtilsMessengerEXT: unknown failure\n", "CreateDebugUtilsMessengerEXT Failure");
                break;
        }
    }
    vkGetPhysicalDeviceProperties(demo->gpu, &demo->gpu_props);

    /* Call with NULL data to get count */
    vkGetPhysicalDeviceQueueFamilyProperties(demo->gpu, &demo->queue_family_count, NULL);
    assert(demo->queue_family_count >= 1);

    demo->queue_props = (VkQueueFamilyProperties *)malloc(demo->queue_family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(demo->gpu, &demo->queue_family_count, demo->queue_props);

    // Query fine-grained feature support for this device.
    //  If app has specific feature requirements it should check supported
    //  features based on this query
    VkPhysicalDeviceFeatures physDevFeatures;
    vkGetPhysicalDeviceFeatures(demo->gpu, &physDevFeatures);
}

static void demo_create_device(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;
    float queue_priorities[1] = {0.0};
    VkDeviceQueueCreateInfo queues[2];
    queues[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queues[0].pNext = NULL;
    queues[0].queueFamilyIndex = demo->graphics_queue_family_index;
    queues[0].queueCount = 1;
    queues[0].pQueuePriorities = queue_priorities;
    queues[0].flags = 0;

    VkDeviceCreateInfo device = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = NULL,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = queues,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = demo->enabled_extension_count,
        .ppEnabledExtensionNames = (const char *const *)demo->extension_names,
        .pEnabledFeatures = NULL,  // If specific features are required, pass them in here
    };
    if (demo->separate_present_queue) {
        queues[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queues[1].pNext = NULL;
        queues[1].queueFamilyIndex = demo->present_queue_family_index;
        queues[1].queueCount = 1;
        queues[1].pQueuePriorities = queue_priorities;
        queues[1].flags = 0;
        device.queueCreateInfoCount = 2;
    }
    err = vkCreateDevice(demo->gpu, &device, NULL, &demo->device);
    assert(!err);

    volkLoadDevice(demo->device);
}

static void demo_create_surface(struct demo *demo) {
    VkResult U_ASSERT_ONLY err = VK_SUCCESS;

// Create a WSI surface for the window:
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (demo->wsi_platform == WSI_PLATFORM_WIN32) {
        VkWin32SurfaceCreateInfoKHR win32_createInfo;
        win32_createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        win32_createInfo.pNext = NULL;
        win32_createInfo.flags = 0;
        win32_createInfo.hinstance = demo->connection;
        win32_createInfo.hwnd = demo->window;

        err = vkCreateWin32SurfaceKHR(demo->inst, &win32_createInfo, NULL, &demo->surface);
    }
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    if (demo->wsi_platform == WSI_PLATFORM_WAYLAND) {
        VkWaylandSurfaceCreateInfoKHR wayland_createInfo;
        wayland_createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
        wayland_createInfo.pNext = NULL;
        wayland_createInfo.flags = 0;
        wayland_createInfo.display = demo->wayland_display;
        wayland_createInfo.surface = demo->window;

        err = vkCreateWaylandSurfaceKHR(demo->inst, &wayland_createInfo, NULL, &demo->surface);
    }
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (demo->wsi_platform == WSI_PLATFORM_ANDROID) {
        VkAndroidSurfaceCreateInfoKHR android_createInfo;
        android_createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        android_createInfo.pNext = NULL;
        android_createInfo.flags = 0;
        android_createInfo.window = (struct ANativeWindow *)(demo->window);

        err = vkCreateAndroidSurfaceKHR(demo->inst, &android_createInfo, NULL, &demo->surface);
    }
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (demo->wsi_platform == WSI_PLATFORM_XLIB) {
        VkXlibSurfaceCreateInfoKHR xlib_createInfo;
        xlib_createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        xlib_createInfo.pNext = NULL;
        xlib_createInfo.flags = 0;
        xlib_createInfo.dpy = demo->xlib_display;
        xlib_createInfo.window = demo->xlib_window;

        err = vkCreateXlibSurfaceKHR(demo->inst, &xlib_createInfo, NULL, &demo->surface);
    }
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (demo->wsi_platform == WSI_PLATFORM_XCB) {
        VkXcbSurfaceCreateInfoKHR xcb_createInfo;
        xcb_createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        xcb_createInfo.pNext = NULL;
        xcb_createInfo.flags = 0;
        xcb_createInfo.connection = demo->connection;
        xcb_createInfo.window = demo->xcb_window;

        err = vkCreateXcbSurfaceKHR(demo->inst, &xcb_createInfo, NULL, &demo->surface);
    }
#endif
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    if (demo->wsi_platform == WSI_PLATFORM_DIRECTFB) {
        VkDirectFBSurfaceCreateInfoEXT directfb_createInfo;
        directfb_createInfo.sType = VK_STRUCTURE_TYPE_DIRECTFB_SURFACE_CREATE_INFO_EXT;
        directfb_createInfo.pNext = NULL;
        directfb_createInfo.flags = 0;
        directfb_createInfo.dfb = demo->dfb;
        directfb_createInfo.surface = demo->directfb_window;

        err = vkCreateDirectFBSurfaceEXT(demo->inst, &directfb_createInfo, NULL, &demo->surface);
    }

#endif
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
    if (demo->wsi_platform == WSI_PLATFORM_DISPLAY) {
        err = demo_create_display_surface(demo);
    }
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (demo->wsi_platform == WSI_PLATFORM_METAL) {
        VkMetalSurfaceCreateInfoEXT metal_createInfo;
        metal_createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
        metal_createInfo.pNext = NULL;
        metal_createInfo.flags = 0;
        metal_createInfo.pLayer = demo->caMetalLayer;

        err = vkCreateMetalSurfaceEXT(demo->inst, &metal_createInfo, NULL, &demo->surface);
    }
#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    if (demo->wsi_platform == WSI_PLATFORM_QNX) {
        VkScreenSurfaceCreateInfoQNX qnx_createInfo;
        qnx_createInfo.sType = VK_STRUCTURE_TYPE_SCREEN_SURFACE_CREATE_INFO_QNX;
        qnx_createInfo.pNext = NULL;
        qnx_createInfo.flags = 0;
        qnx_createInfo.context = demo->screen_context;
        qnx_createInfo.window = demo->screen_window;

        err = vkCreateScreenSurfaceQNX(demo->inst, &qnx_createInfo, NULL, &demo->surface);
    }
#endif
    assert(!err);
}

static VkSurfaceFormatKHR pick_surface_format(const VkSurfaceFormatKHR *surfaceFormats, uint32_t count) {
    // Prefer non-SRGB formats...
    for (uint32_t i = 0; i < count; i++) {
        const VkFormat format = surfaceFormats[i].format;

        if (format == VK_FORMAT_R8G8B8A8_UNORM || format == VK_FORMAT_B8G8R8A8_UNORM ||
            format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 || format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 ||
            format == VK_FORMAT_A1R5G5B5_UNORM_PACK16 || format == VK_FORMAT_R5G6B5_UNORM_PACK16 ||
            format == VK_FORMAT_R16G16B16A16_SFLOAT) {
            return surfaceFormats[i];
        }
    }

    printf("Can't find our preferred formats... Falling back to first exposed format. Rendering may be incorrect.\n");

    assert(count >= 1);
    return surfaceFormats[0];
}

static void demo_init_vk_swapchain(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;

    // Iterate over each queue to learn whether it supports presenting:
    VkBool32 *supportsPresent = (VkBool32 *)malloc(demo->queue_family_count * sizeof(VkBool32));
    for (uint32_t i = 0; i < demo->queue_family_count; i++) {
        vkGetPhysicalDeviceSurfaceSupportKHR(demo->gpu, i, demo->surface, &supportsPresent[i]);
    }

    // Search for a graphics and a present queue in the array of queue
    // families, try to find one that supports both
    uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
    uint32_t presentQueueFamilyIndex = UINT32_MAX;
    for (uint32_t i = 0; i < demo->queue_family_count; i++) {
        if ((demo->queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            if (graphicsQueueFamilyIndex == UINT32_MAX) {
                graphicsQueueFamilyIndex = i;
            }

            if (supportsPresent[i] == VK_TRUE) {
                graphicsQueueFamilyIndex = i;
                presentQueueFamilyIndex = i;
                break;
            }
        }
    }

    if (presentQueueFamilyIndex == UINT32_MAX) {
        // If didn't find a queue that supports both graphics and present, then
        // find a separate present queue.
        for (uint32_t i = 0; i < demo->queue_family_count; ++i) {
            if (supportsPresent[i] == VK_TRUE) {
                presentQueueFamilyIndex = i;
                break;
            }
        }
    }

    // Generate error if could not find both a graphics and a present queue
    if (graphicsQueueFamilyIndex == UINT32_MAX || presentQueueFamilyIndex == UINT32_MAX) {
        ERR_EXIT("Could not find both graphics and present queues\n", "Swapchain Initialization Failure");
    }

    demo->graphics_queue_family_index = graphicsQueueFamilyIndex;
    demo->present_queue_family_index = presentQueueFamilyIndex;
    demo->separate_present_queue = (demo->graphics_queue_family_index != demo->present_queue_family_index);
    free(supportsPresent);

    demo_create_device(demo);

    vkGetDeviceQueue(demo->device, demo->graphics_queue_family_index, 0, &demo->graphics_queue);

    if (!demo->separate_present_queue) {
        demo->present_queue = demo->graphics_queue;
    } else {
        vkGetDeviceQueue(demo->device, demo->present_queue_family_index, 0, &demo->present_queue);
    }

    // Get the list of VkFormat's that are supported:
    uint32_t formatCount;
    err = vkGetPhysicalDeviceSurfaceFormatsKHR(demo->gpu, demo->surface, &formatCount, NULL);
    assert(!err);
    VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    err = vkGetPhysicalDeviceSurfaceFormatsKHR(demo->gpu, demo->surface, &formatCount, surfFormats);
    assert(!err);
    VkSurfaceFormatKHR surfaceFormat = pick_surface_format(surfFormats, formatCount);
    demo->format = surfaceFormat.format;
    demo->color_space = surfaceFormat.colorSpace;
    free(surfFormats);

    demo->quit = false;
    demo->curFrame = 0;

    // Create semaphores to synchronize acquiring presentable buffers before
    // rendering and waiting for drawing to be complete before presenting
    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
    };

    // Create fences that we can use to throttle if we get too far
    // ahead of the image presents
    VkFenceCreateInfo fence_ci = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext = NULL, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    for (uint32_t i = 0; i < FRAME_LAG; i++) {
        err = vkCreateFence(demo->device, &fence_ci, NULL, &demo->fences[i]);
        assert(!err);

        err = vkCreateSemaphore(demo->device, &semaphoreCreateInfo, NULL, &demo->image_acquired_semaphores[i]);
        assert(!err);
        demo_name_object(demo, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)demo->image_acquired_semaphores[i], "AcquireSem(%u)", i);

        err = vkCreateSemaphore(demo->device, &semaphoreCreateInfo, NULL, &demo->draw_complete_semaphores[i]);
        assert(!err);
        demo_name_object(demo, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)demo->draw_complete_semaphores[i], "DrawCompleteSem(%u)", i);

        if (demo->separate_present_queue) {
            err = vkCreateSemaphore(demo->device, &semaphoreCreateInfo, NULL, &demo->image_ownership_semaphores[i]);
            assert(!err);
            demo_name_object(demo, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)demo->image_ownership_semaphores[i], "ImageOwnerSem(%u)", i);
        }
    }
    demo->frame_index = 0;
    demo->first_swapchain_frame = true;

    // Get Memory information and properties
    vkGetPhysicalDeviceMemoryProperties(demo->gpu, &demo->memory_properties);
}

static void demo_init(struct demo *demo, int argc, char **argv) {
    vec3 eye = {0.0f, 3.0f, 5.0f};
    vec3 origin = {0, 0, 0};
    vec3 up = {0.0f, 1.0f, 0.0};

    memset(demo, 0, sizeof(*demo));
    demo->presentMode = VK_PRESENT_MODE_FIFO_KHR;
    demo->frameCount = INT32_MAX;
    /* Autodetect suitable / best GPU by default */
    demo->gpu_number = -1;
    demo->width = 500;
    demo->height = 500;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use_staging") == 0) {
            demo->use_staging_buffer = true;
            continue;
        }
        if ((strcmp(argv[i], "--present_mode") == 0) && (i < argc - 1)) {
            demo->presentMode = atoi(argv[i + 1]);
            i++;
            continue;
        }
        if (strcmp(argv[i], "--break") == 0) {
            demo->use_break = true;
            continue;
        }
        if (strcmp(argv[i], "--validate") == 0) {
            demo->validate = true;
            continue;
        }
        if (strcmp(argv[i], "--xlib") == 0) {
            fprintf(stderr, "--xlib is deprecated and no longer does anything\n");
            continue;
        }
        if (strcmp(argv[i], "--c") == 0 && demo->frameCount == INT32_MAX && i < argc - 1 &&
            sscanf(argv[i + 1], "%d", &demo->frameCount) == 1 && demo->frameCount >= 0) {
            i++;
            continue;
        }
        if (strcmp(argv[i], "--width") == 0) {
            if (i < argc - 1 && sscanf(argv[i + 1], "%d", &demo->width) == 1) {
                if (demo->width > 0) {
                    i++;
                    continue;
                } else {
                    ERR_EXIT("The --width parameter must be greater than 0", "User Error");
                }
            }
            ERR_EXIT("The --width parameter must be followed by a number", "User Error");
        }
        if (strcmp(argv[i], "--height") == 0) {
            if (i < argc - 1 && sscanf(argv[i + 1], "%d", &demo->height) == 1) {
                if (demo->height > 0) {
                    i++;
                    continue;
                } else {
                    ERR_EXIT("The --height parameter must be greater than 0", "User Error");
                }
            }
            ERR_EXIT("The --height parameter must be followed by a number", "User Error");
        }
        if (strcmp(argv[i], "--suppress_popups") == 0) {
            demo->suppress_popups = true;
            continue;
        }
        if (strcmp(argv[i], "--display_timing") == 0) {
            demo->VK_GOOGLE_display_timing_enabled = true;
            continue;
        }
        if (strcmp(argv[i], "--incremental_present") == 0) {
            demo->VK_KHR_incremental_present_enabled = true;
            continue;
        }
        if ((strcmp(argv[i], "--gpu_number") == 0) && (i < argc - 1)) {
            demo->gpu_number = atoi(argv[i + 1]);
            if (demo->gpu_number < 0) demo->invalid_gpu_selection = true;
            i++;
            continue;
        }
        if (strcmp(argv[i], "--force_errors") == 0) {
            demo->force_errors = true;
            continue;
        }
        if ((strcmp(argv[i], "--wsi") == 0) && (i < argc - 1)) {
            size_t argc_len = strlen(argv[i + 1]);
            for (size_t argc_i = 0; argc_i < argc_len; argc_i++) {
                argv[i + 1][argc_i] = tolower(argv[i + 1][argc_i]);
            }
            WSI_PLATFORM selection = wsi_from_string(argv[i + 1]);
            if (selection == WSI_PLATFORM_INVALID) {
                printf(
                    "The --wsi parameter %s is not a supported WSI platform. The list of available platforms is available from "
                    "--help\n",
                    (const char *)&(argv[i + 1][0]));
                fflush(stdout);
                exit(1);
            }
            demo->wsi_platform = selection;
            i++;
            continue;
        }
#if defined(ANDROID)
        ERR_EXIT("Usage: vkcube [--validate]\n", "Usage");
#else

        // Making the help for --wsi nice requires a little extra work since the list depends on what is available at
        // compile time
        size_t max_str_len = 100;
        char *available_wsi_platforms = (char *)malloc(max_str_len);
        memset(available_wsi_platforms, 0, max_str_len);
#if defined(VK_USE_PLATFORM_XCB_KHR)
        strncat(available_wsi_platforms, "xcb", max_str_len);
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        if (strlen(available_wsi_platforms) > 0) {
            strncat(available_wsi_platforms, "|", max_str_len);
        }
        strncat(available_wsi_platforms, "xlib", max_str_len);
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
        if (strlen(available_wsi_platforms) > 0) {
            strncat(available_wsi_platforms, "|", max_str_len);
        }
        strncat(available_wsi_platforms, "wayland", max_str_len);
#endif
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
        if (strlen(available_wsi_platforms) > 0) {
            strncat(available_wsi_platforms, "|", max_str_len);
        }
        strncat(available_wsi_platforms, "directfb", max_str_len);
#endif
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
        if (strlen(available_wsi_platforms) > 0) {
            strncat(available_wsi_platforms, "|", max_str_len);
        }
        strncat(available_wsi_platforms, "display", max_str_len);
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        if (strlen(available_wsi_platforms) > 0) {
            strncat(available_wsi_platforms, "|", max_str_len);
        }
        strncat(available_wsi_platforms, "win32", max_str_len);
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        if (strlen(available_wsi_platforms) > 0) {
            strncat(available_wsi_platforms, "|", max_str_len);
        }
        strncat(available_wsi_platforms, "android", max_str_len);
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
        if (strlen(available_wsi_platforms) > 0) {
            strncat(available_wsi_platforms, "|", max_str_len);
        }
        strncat(available_wsi_platforms, "metal", max_str_len);
#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
        if (strlen(available_wsi_platforms) > 0) {
            strncat(available_wsi_platforms, "|", max_str_len);
        }
        strncat(available_wsi_platforms, "qnx", max_str_len);
#endif

        char *message =
            "Usage:\n  %s\t[--use_staging] [--validate]\n"
            "\t[--break] [--c <framecount>] [--suppress_popups]\n"
            "\t[--incremental_present] [--display_timing]\n"
            "\t[--gpu_number <index of physical device>]\n"
            "\t[--present_mode <present mode enum>]\n"
            "\t[--width <width>] [--height <height>]\n"
            "\t[--force_errors]\n"
            "\t[--wsi <%s>]\n"
            "\t<present_mode_enum>\n"
            "\t\tVK_PRESENT_MODE_IMMEDIATE_KHR = %d\n"
            "\t\tVK_PRESENT_MODE_MAILBOX_KHR = %d\n"
            "\t\tVK_PRESENT_MODE_FIFO_KHR = %d\n"
            "\t\tVK_PRESENT_MODE_FIFO_RELAXED_KHR = %d\n";
        int length = snprintf(NULL, 0, message, APP_SHORT_NAME, available_wsi_platforms, VK_PRESENT_MODE_IMMEDIATE_KHR,
                              VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR);
        char *usage = (char *)malloc(length + 1);
        if (!usage) {
            exit(1);
        }
        snprintf(usage, length + 1, message, APP_SHORT_NAME, available_wsi_platforms, VK_PRESENT_MODE_IMMEDIATE_KHR,
                 VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR);
#if defined(_WIN32)
        if (!demo->suppress_popups) MessageBox(NULL, usage, "Usage Error", MB_OK);
#else
        fprintf(stderr, "%s", usage);
        fflush(stderr);
#endif
        free(usage);
        exit(1);
#endif
    }

    demo_init_vk(demo);

    demo->spin_angle = 4.0f;
    demo->spin_increment = 0.2f;
    demo->pause = false;

    mat4x4_perspective(demo->projection_matrix, (float)degreesToRadians(45.0f), 1.0f, 0.1f, 100.0f);
    mat4x4_look_at(demo->view_matrix, eye, origin, up);
    mat4x4_identity(demo->model_matrix);

    demo->projection_matrix[1][1] *= -1;  // Flip projection matrix from GL to Vulkan orientation.
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
// Include header required for parsing the command line options.
#include <shellapi.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    MSG msg;    // message
    bool done;  // flag saying when app is complete
    int argc;
    char **argv;

    // Ensure wParam is initialized.
    msg.wParam = 0;

    // Use the CommandLine functions to get the command line arguments.
    // Unfortunately, Microsoft outputs
    // this information as wide characters for Unicode, and we simply want the
    // Ascii version to be compatible
    // with the non-Windows side.  So, we have to convert the information to
    // Ascii character strings.
    LPWSTR *commandLineArgs = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (NULL == commandLineArgs) {
        argc = 0;
    }

    if (argc > 0) {
        argv = (char **)malloc(sizeof(char *) * argc);
        if (argv == NULL) {
            argc = 0;
        } else {
            for (int iii = 0; iii < argc; iii++) {
                size_t wideCharLen = wcslen(commandLineArgs[iii]);
                size_t numConverted = 0;

                argv[iii] = (char *)malloc(sizeof(char) * (wideCharLen + 1));
                if (argv[iii] != NULL) {
                    wcstombs_s(&numConverted, argv[iii], wideCharLen + 1, commandLineArgs[iii], wideCharLen + 1);
                }
            }
        }
    } else {
        argv = NULL;
    }

    demo_init(&demo, argc, argv);

    // Free up the items we had to allocate for the command line arguments.
    if (argc > 0 && argv != NULL) {
        for (int iii = 0; iii < argc; iii++) {
            if (argv[iii] != NULL) {
                free(argv[iii]);
            }
        }
        free(argv);
    }

    demo.connection = hInstance;
    strncpy(demo.name, "Vulkan Cube", APP_NAME_STR_LEN);
    demo_create_window(&demo);
    demo_create_surface(&demo);
    demo_select_physical_device(&demo);
    demo_init_vk_swapchain(&demo);

    demo_prepare(&demo);

    done = false;  // initialize loop condition variable

    // main message loop
    while (!done) {
        if (demo.pause) {
            const BOOL succ = WaitMessage();

            if (!succ) {
                struct demo *tmp = &demo;
                struct demo *demo = tmp;
                ERR_EXIT("WaitMessage() failed on paused demo", "event loop error");
            }
        }
        PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        if (msg.message == WM_QUIT)  // check for a quit message
        {
            done = true;  // if found, quit app
        } else {
            /* Translate and dispatch to event queue*/
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        RedrawWindow(demo.window, NULL, NULL, RDW_INTERNALPAINT);
    }

    demo_cleanup(&demo);

    return (int)msg.wParam;
}

#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
static void demo_main(struct demo *demo, void *caMetalLayer, int argc, const char *argv[]) {
    demo_init(demo, argc, (char **)argv);
    demo->caMetalLayer = caMetalLayer;
    demo_create_surface(demo);
    demo_select_physical_device(demo);
    demo_init_vk_swapchain(demo);
    demo_prepare(demo);
    demo->spin_angle = 0.4f;
}

#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#include <android/log.h>
#include <android_native_app_glue.h>
#include "android_util.h"

static bool initialized = false;
static bool active = false;
struct demo demo;

static int32_t processInput(struct android_app *app, AInputEvent *event) { return 0; }

static void processCommand(struct android_app *app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW: {
            if (app->window) {
                // We're getting a new window.  If the app is starting up, we
                // need to initialize.  If the app has already been
                // initialized, that means that we lost our previous window,
                // which means that we have a lot of work to do.  At a minimum,
                // we need to destroy the swapchain and surface associated with
                // the old window, and create a new surface and swapchain.
                // However, since there are a lot of other objects/state that
                // is tied to the swapchain, it's easiest to simply cleanup and
                // start over (i.e. use a brute-force approach of re-starting
                // the app)
                if (demo.prepared) {
                    demo_cleanup(&demo);
                }

                // Parse Intents into argc, argv
                // Use the following key to send arguments, i.e.
                // --es args "--validate"
                const char key[] = "args";
                char *appTag = (char *)APP_SHORT_NAME;
                int argc = 0;
                char **argv = get_args(app, key, appTag, &argc);

                __android_log_print(ANDROID_LOG_INFO, appTag, "argc = %i", argc);
                for (int i = 0; i < argc; i++) __android_log_print(ANDROID_LOG_INFO, appTag, "argv[%i] = %s", i, argv[i]);

                demo_init(&demo, argc, argv);

                // Free the argv malloc'd by get_args
                for (int i = 0; i < argc; i++) free(argv[i]);

                demo.window = (void *)app->window;
                demo_create_surface(&demo);
                demo_select_physical_device(&demo);
                demo_init_vk_swapchain(&demo);
                demo_prepare(&demo);
                initialized = true;
            }
            break;
        }
        case APP_CMD_GAINED_FOCUS: {
            active = true;
            break;
        }
        case APP_CMD_LOST_FOCUS: {
            active = false;
            break;
        }
    }
}

void android_main(struct android_app *app) {
    demo.prepared = false;

    app->onAppCmd = processCommand;
    app->onInputEvent = processInput;

    while (1) {
        int events;
        struct android_poll_source *source;
        while (ALooper_pollOnce(active ? 0 : -1, NULL, &events, (void **)&source) >= 0) {
            if (source) {
                source->process(app, source);
            }

            if (app->destroyRequested != 0) {
                demo_cleanup(&demo);
                return;
            }
        }
        if (initialized && active) {
            demo_run(&demo);
        }
    }
}
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__QNX__) || defined(__GNU__)
int main(int argc, char **argv) {
    struct demo demo;

    demo_init(&demo, argc, argv);
    switch (demo.wsi_platform) {
        default:
        case (WSI_PLATFORM_AUTO):
            fprintf(stderr,
                    "WSI platform should have already been set, indicating a bug. Please set a WSI platform manually with "
                    "--wsi\n");
            exit(1);
            break;
#if defined(VK_USE_PLATFORM_XCB_KHR)
        case (WSI_PLATFORM_XCB):
            demo_create_xcb_window(&demo);
            break;
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        case (WSI_PLATFORM_XLIB):
            demo_create_xlib_window(&demo);
            break;
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
        case (WSI_PLATFORM_WAYLAND):
            demo_create_wayland_window(&demo);
            break;
#endif
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
        case (WSI_PLATFORM_DIRECTFB):
            demo_create_directfb_window(&demo);
            break;
#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
        case (WSI_PLATFORM_QNX):
            demo_create_screen_window(&demo);
            break;
#endif
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
        case (WSI_PLATFORM_DISPLAY):
            // nothing to do here
            break;
#endif
    }
    demo_create_surface(&demo);
    demo_select_physical_device(&demo);

    demo_init_vk_swapchain(&demo);

    demo_prepare(&demo);

    switch (demo.wsi_platform) {
        default:
        case (WSI_PLATFORM_AUTO):
            fprintf(stderr,
                    "WSI platform should have already been set, indicating a bug. Please set a WSI platform manually with "
                    "--wsi\n");
            exit(1);
            break;
#if defined(VK_USE_PLATFORM_XCB_KHR)
        case (WSI_PLATFORM_XCB):
            demo_run_xcb(&demo);
            break;
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        case (WSI_PLATFORM_XLIB):
            demo_run_xlib(&demo);
            break;
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
        case (WSI_PLATFORM_WAYLAND):
            demo_run(&demo);
            break;
#endif
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
        case (WSI_PLATFORM_DIRECTFB):
            demo_run_directfb(&demo);
            break;
#endif
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
        case (WSI_PLATFORM_DISPLAY):
            demo_run_display(&demo);
            break;
#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
        case (WSI_PLATFORM_QNX):
            demo_run(&demo);
            break;
#endif
    }

    demo_cleanup(&demo);

    return validation_error;
}
#endif
