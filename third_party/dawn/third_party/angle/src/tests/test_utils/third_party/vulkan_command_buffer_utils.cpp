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

#include "vulkan_command_buffer_utils.h"

#include <assert.h>
#include <string.h>
#include <cstdlib>
#include <iterator>

#include "SPIRV/GlslangToSpv.h"

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
#    include <linux/input.h>
#endif

using namespace std;

/*
 * TODO: function description here
 */
VkResult init_global_extension_properties(layer_properties &layer_props)
{
    VkExtensionProperties *instance_extensions;
    uint32_t instance_extension_count;
    VkResult res;
    char *layer_name = NULL;

    layer_name = layer_props.properties.layerName;

    do
    {
        res = vkEnumerateInstanceExtensionProperties(layer_name, &instance_extension_count, NULL);
        if (res)
            return res;

        if (instance_extension_count == 0)
        {
            return VK_SUCCESS;
        }

        layer_props.instance_extensions.resize(instance_extension_count);
        instance_extensions = layer_props.instance_extensions.data();
        res = vkEnumerateInstanceExtensionProperties(layer_name, &instance_extension_count,
                                                     instance_extensions);
    } while (res == VK_INCOMPLETE);

    return res;
}

/*
 * TODO: function description here
 */
VkResult init_global_layer_properties(struct sample_info &info)
{
    uint32_t instance_layer_count;
    VkLayerProperties *vk_props = NULL;
    VkResult res;

    /*
     * It's possible, though very rare, that the number of
     * instance layers could change. For example, installing something
     * could include new layers that the loader would pick up
     * between the initial query for the count and the
     * request for VkLayerProperties. The loader indicates that
     * by returning a VK_INCOMPLETE status and will update the
     * the count parameter.
     * The count parameter will be updated with the number of
     * entries loaded into the data pointer - in case the number
     * of layers went down or is smaller than the size given.
     */
    do
    {
        res = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
        if (res)
            return res;

        if (instance_layer_count == 0)
        {
            return VK_SUCCESS;
        }

        vk_props = (VkLayerProperties *)realloc(vk_props,
                                                instance_layer_count * sizeof(VkLayerProperties));

        res = vkEnumerateInstanceLayerProperties(&instance_layer_count, vk_props);
    } while (res == VK_INCOMPLETE);

    /*
     * Now gather the extension list for each instance layer.
     */
    for (uint32_t i = 0; i < instance_layer_count; i++)
    {
        layer_properties layer_props;
        layer_props.properties = vk_props[i];
        res                    = init_global_extension_properties(layer_props);
        if (res)
            return res;
        info.instance_layer_properties.push_back(layer_props);
    }
    free(vk_props);

    return res;
}

VkResult init_device_extension_properties(struct sample_info &info, layer_properties &layer_props)
{
    VkExtensionProperties *device_extensions;
    uint32_t device_extension_count;
    VkResult res;
    char *layer_name = NULL;

    layer_name = layer_props.properties.layerName;

    do
    {
        res = vkEnumerateDeviceExtensionProperties(info.gpus[0], layer_name,
                                                   &device_extension_count, NULL);
        if (res)
            return res;

        if (device_extension_count == 0)
        {
            return VK_SUCCESS;
        }

        layer_props.device_extensions.resize(device_extension_count);
        device_extensions = layer_props.device_extensions.data();
        res               = vkEnumerateDeviceExtensionProperties(info.gpus[0], layer_name,
                                                                 &device_extension_count, device_extensions);
    } while (res == VK_INCOMPLETE);

    return res;
}

/*
 * Return 1 (true) if all layer names specified in check_names
 * can be found in given layer properties.
 */
VkBool32 demo_check_layers(const std::vector<layer_properties> &layer_props,
                           const std::vector<const char *> &layer_names)
{
    uint32_t check_count = layer_names.size();
    uint32_t layer_count = layer_props.size();
    for (uint32_t i = 0; i < check_count; i++)
    {
        VkBool32 found = 0;
        for (uint32_t j = 0; j < layer_count; j++)
        {
            if (!strcmp(layer_names[i], layer_props[j].properties.layerName))
            {
                found = 1;
            }
        }
        if (!found)
        {
            std::cout << "Cannot find layer: " << layer_names[i] << std::endl;
            return 0;
        }
    }
    return 1;
}

void init_instance_extension_names(struct sample_info &info)
{
    info.instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef __ANDROID__
    info.instance_extension_names.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_WIN32)
    info.instance_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    info.instance_extension_names.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#else
    info.instance_extension_names.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
#ifndef __ANDROID__
    info.instance_layer_names.push_back("VK_LAYER_LUNARG_standard_validation");
    if (!demo_check_layers(info.instance_layer_properties, info.instance_layer_names))
    {
        // If standard validation is not present, search instead for the
        // individual layers that make it up, in the correct order.
        //

        info.instance_layer_names.clear();
        info.instance_layer_names.push_back("VK_LAYER_GOOGLE_threading");
        info.instance_layer_names.push_back("VK_LAYER_LUNARG_parameter_validation");
        info.instance_layer_names.push_back("VK_LAYER_LUNARG_object_tracker");
        info.instance_layer_names.push_back("VK_LAYER_LUNARG_core_validation");
        info.instance_layer_names.push_back("VK_LAYER_LUNARG_image");
        info.instance_layer_names.push_back("VK_LAYER_LUNARG_swapchain");
        info.instance_layer_names.push_back("VK_LAYER_GOOGLE_unique_objects");

        if (!demo_check_layers(info.instance_layer_properties, info.instance_layer_names))
        {
            exit(1);
        }
    }

    // Enable debug callback extension
    info.instance_extension_names.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
}

VkResult init_instance(struct sample_info &info, char const *const app_short_name)
{
    VkResult res = VK_SUCCESS;
#if ANGLE_SHARED_LIBVULKAN
    res = volkInitialize();
    ASSERT(res == VK_SUCCESS);
#endif  // ANGLE_SHARED_LIBVULKAN
    VkApplicationInfo app_info  = {};
    app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext              = NULL;
    app_info.pApplicationName   = app_short_name;
    app_info.applicationVersion = 1;
    app_info.pEngineName        = app_short_name;
    app_info.engineVersion      = 1;
    app_info.apiVersion         = VK_API_VERSION_1_0;

    VkInstanceCreateInfo inst_info = {};
    inst_info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pNext                = NULL;
    inst_info.flags                = 0;
    inst_info.pApplicationInfo     = &app_info;
    inst_info.enabledLayerCount    = info.instance_layer_names.size();
    inst_info.ppEnabledLayerNames =
        info.instance_layer_names.size() ? info.instance_layer_names.data() : NULL;
    inst_info.enabledExtensionCount   = info.instance_extension_names.size();
    inst_info.ppEnabledExtensionNames = info.instance_extension_names.data();

    res = vkCreateInstance(&inst_info, NULL, &info.inst);
    ASSERT(res == VK_SUCCESS);
#if ANGLE_SHARED_LIBVULKAN
    volkLoadInstance(info.inst);
#endif  // ANGLE_SHARED_LIBVULKAN

    return res;
}

void init_device_extension_names(struct sample_info &info)
{
    info.device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

VkResult init_enumerate_device(struct sample_info &info, uint32_t gpu_count)
{
    VkResult res = vkEnumeratePhysicalDevices(info.inst, &gpu_count, NULL);
    ASSERT(gpu_count);
    info.gpus.resize(gpu_count);

    res = vkEnumeratePhysicalDevices(info.inst, &gpu_count, info.gpus.data());
    ASSERT(!res);

    vkGetPhysicalDeviceQueueFamilyProperties(info.gpus[0], &info.queue_family_count, NULL);
    ASSERT(info.queue_family_count >= 1);

    info.queue_props.resize(info.queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(info.gpus[0], &info.queue_family_count,
                                             info.queue_props.data());
    ASSERT(info.queue_family_count >= 1);

    /* This is as good a place as any to do this */
    vkGetPhysicalDeviceMemoryProperties(info.gpus[0], &info.memory_properties);
    vkGetPhysicalDeviceProperties(info.gpus[0], &info.gpu_props);
    /* query device extensions for enabled layers */
    for (auto &layer_props : info.instance_layer_properties)
    {
        init_device_extension_properties(info, layer_props);
    }

    return res;
}

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)

static void handle_ping(void *data, wl_shell_surface *shell_surface, uint32_t serial)
{
    wl_shell_surface_pong(shell_surface, serial);
}

static void handle_configure(void *data,
                             wl_shell_surface *shell_surface,
                             uint32_t edges,
                             int32_t width,
                             int32_t height)
{}

static void handle_popup_done(void *data, wl_shell_surface *shell_surface) {}

static const wl_shell_surface_listener shell_surface_listener = {handle_ping, handle_configure,
                                                                 handle_popup_done};

static void registry_handle_global(void *data,
                                   wl_registry *registry,
                                   uint32_t id,
                                   const char *interface,
                                   uint32_t version)
{
    sample_info *info = (sample_info *)data;
    // pickup wayland objects when they appear
    if (strcmp(interface, "wl_compositor") == 0)
    {
        info->compositor =
            (wl_compositor *)wl_registry_bind(registry, id, &wl_compositor_interface, 1);
    }
    else if (strcmp(interface, "wl_shell") == 0)
    {
        info->shell = (wl_shell *)wl_registry_bind(registry, id, &wl_shell_interface, 1);
    }
}

static void registry_handle_global_remove(void *data, wl_registry *registry, uint32_t name) {}

static const wl_registry_listener registry_listener = {registry_handle_global,
                                                       registry_handle_global_remove};

#endif

void init_connection(struct sample_info &info)
{
#if defined(VK_USE_PLATFORM_XCB_KHR)
    const xcb_setup_t *setup;
    xcb_screen_iterator_t iter;
    int scr;

    info.connection = xcb_connect(NULL, &scr);
    if (info.connection == NULL || xcb_connection_has_error(info.connection))
    {
        std::cout << "Unable to make an XCB connection\n";
        exit(-1);
    }

    setup = xcb_get_setup(info.connection);
    iter  = xcb_setup_roots_iterator(setup);
    while (scr-- > 0)
        xcb_screen_next(&iter);

    info.screen = iter.data;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    info.display = wl_display_connect(nullptr);

    if (info.display == nullptr)
    {
        printf(
            "Cannot find a compatible Vulkan installable client driver "
            "(ICD).\nExiting ...\n");
        fflush(stdout);
        exit(1);
    }

    info.registry = wl_display_get_registry(info.display);
    wl_registry_add_listener(info.registry, &registry_listener, &info);
    wl_display_dispatch(info.display);
#endif
}
#ifdef _WIN32
static void run(struct sample_info *info)
{ /* Placeholder for samples that want to show dynamic content */
}

// MS-Windows event handling function:
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    struct sample_info *info =
        reinterpret_cast<struct sample_info *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
            break;
        case WM_PAINT:
            run(info);
            return 0;
        default:
            break;
    }
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

void init_window(struct sample_info &info)
{
    WNDCLASSEXA win_class;
    ASSERT(info.width > 0);
    ASSERT(info.height > 0);

    info.connection = GetModuleHandle(NULL);
    sprintf(info.name, "Sample");

    // Initialize the window class structure:
    win_class.cbSize        = sizeof(WNDCLASSEX);
    win_class.style         = CS_HREDRAW | CS_VREDRAW;
    win_class.lpfnWndProc   = WndProc;
    win_class.cbClsExtra    = 0;
    win_class.cbWndExtra    = 0;
    win_class.hInstance     = info.connection;  // hInstance
    win_class.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    win_class.hCursor       = LoadCursor(NULL, IDC_ARROW);
    win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    win_class.lpszMenuName  = NULL;
    win_class.lpszClassName = info.name;
    win_class.hIconSm       = LoadIcon(NULL, IDI_WINLOGO);
    // Register window class:
    if (!RegisterClassExA(&win_class))
    {
        // It didn't work, so try to give a useful error:
        printf("Unexpected error trying to start the application!\n");
        fflush(stdout);
        exit(1);
    }
    // Create window with the registered class:
    RECT wr = {0, 0, info.width, info.height};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    info.window = CreateWindowExA(0,
                                  info.name,             // class name
                                  info.name,             // app name
                                  WS_OVERLAPPEDWINDOW |  // window style
                                      WS_VISIBLE | WS_SYSMENU,
                                  100, 100,            // x/y coords
                                  wr.right - wr.left,  // width
                                  wr.bottom - wr.top,  // height
                                  NULL,                // handle to parent
                                  NULL,                // handle to menu
                                  info.connection,     // hInstance
                                  NULL);               // no extra parameters
    if (!info.window)
    {
        // It didn't work, so try to give a useful error:
        printf("Cannot create a window in which to draw!\n");
        fflush(stdout);
        exit(1);
    }
    SetWindowLongPtr(info.window, GWLP_USERDATA, (LONG_PTR)&info);
}

void destroy_window(struct sample_info &info)
{
    vkDestroySurfaceKHR(info.inst, info.surface, NULL);
    DestroyWindow(info.window);
    UnregisterClassA(info.name, GetModuleHandle(NULL));
}

#elif defined(__ANDROID__)
// Android implementation.
void init_window(struct sample_info &info) {}

void destroy_window(struct sample_info &info)
{
    vkDestroySurfaceKHR(info.inst, info.surface, NULL);
}

#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)

void init_window(struct sample_info &info)
{
    ASSERT(info.width > 0);
    ASSERT(info.height > 0);

    info.window = wl_compositor_create_surface(info.compositor);
    if (!info.window)
    {
        printf("Can not create wayland_surface from compositor!\n");
        fflush(stdout);
        exit(1);
    }

    info.shell_surface = wl_shell_get_shell_surface(info.shell, info.window);
    if (!info.shell_surface)
    {
        printf("Can not get shell_surface from wayland_surface!\n");
        fflush(stdout);
        exit(1);
    }

    wl_shell_surface_add_listener(info.shell_surface, &shell_surface_listener, &info);
    wl_shell_surface_set_toplevel(info.shell_surface);
}

void destroy_window(struct sample_info &info)
{
    wl_shell_surface_destroy(info.shell_surface);
    wl_surface_destroy(info.window);
    wl_shell_destroy(info.shell);
    wl_compositor_destroy(info.compositor);
    wl_registry_destroy(info.registry);
    wl_display_disconnect(info.display);
}

#else

void init_window(struct sample_info &info)
{
    ASSERT(info.width > 0);
    ASSERT(info.height > 0);

    uint32_t value_mask, value_list[32];

    info.window = xcb_generate_id(info.connection);

    value_mask    = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    value_list[0] = info.screen->black_pixel;
    value_list[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE;

    xcb_create_window(info.connection, XCB_COPY_FROM_PARENT, info.window, info.screen->root, 0, 0,
                      info.width, info.height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      info.screen->root_visual, value_mask, value_list);

    /* Magic code that will send notification when window is destroyed */
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(info.connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t *reply  = xcb_intern_atom_reply(info.connection, cookie, 0);

    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(info.connection, 0, 16, "WM_DELETE_WINDOW");
    info.atom_wm_delete_window       = xcb_intern_atom_reply(info.connection, cookie2, 0);

    xcb_change_property(info.connection, XCB_PROP_MODE_REPLACE, info.window, (*reply).atom, 4, 32,
                        1, &(*info.atom_wm_delete_window).atom);
    free(reply);

    xcb_map_window(info.connection, info.window);

    // Force the x/y coordinates to 100,100 results are identical in consecutive
    // runs
    const uint32_t coords[] = {100, 100};
    xcb_configure_window(info.connection, info.window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
                         coords);
    xcb_flush(info.connection);

    xcb_generic_event_t *e;
    while ((e = xcb_wait_for_event(info.connection)))
    {
        if ((e->response_type & ~0x80) == XCB_EXPOSE)
            break;
    }
}

void destroy_window(struct sample_info &info)
{
    vkDestroySurfaceKHR(info.inst, info.surface, NULL);
    xcb_destroy_window(info.connection, info.window);
    xcb_disconnect(info.connection);
}

#endif  // _WIN32

void init_window_size(struct sample_info &info, int32_t default_width, int32_t default_height)
{
#ifdef __ANDROID__
    info.mOSWindow = OSWindow::New();
    ASSERT(info.mOSWindow != nullptr);
    info.mOSWindow->initialize("VulkanTest", default_width, default_height);
#endif
    info.width  = default_width;
    info.height = default_height;
}

void init_swapchain_extension(struct sample_info &info)
{
    /* DEPENDS on init_connection() and init_window() */

    VkResult res;

// Construct the surface description:
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType                       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext                       = NULL;
    createInfo.hinstance                   = info.connection;
    createInfo.hwnd                        = info.window;
    res = vkCreateWin32SurfaceKHR(info.inst, &createInfo, NULL, &info.surface);
#elif defined(__ANDROID__)
    GET_INSTANCE_PROC_ADDR(info.inst, CreateAndroidSurfaceKHR);
    VkAndroidSurfaceCreateInfoKHR createInfo;
    createInfo.sType  = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext  = nullptr;
    createInfo.flags  = 0;
    createInfo.window = info.mOSWindow->getNativeWindow();
    res = info.fpCreateAndroidSurfaceKHR(info.inst, &createInfo, nullptr, &info.surface);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    VkWaylandSurfaceCreateInfoKHR createInfo = {};
    createInfo.sType                         = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext                         = NULL;
    createInfo.display                       = info.display;
    createInfo.surface                       = info.window;
    res = vkCreateWaylandSurfaceKHR(info.inst, &createInfo, NULL, &info.surface);
#else
    VkXcbSurfaceCreateInfoKHR createInfo = {};
    createInfo.sType                     = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext                     = NULL;
    createInfo.connection                = info.connection;
    createInfo.window                    = info.window;
    res = vkCreateXcbSurfaceKHR(info.inst, &createInfo, NULL, &info.surface);
#endif  // __ANDROID__  && _WIN32
    ASSERT(res == VK_SUCCESS);

    // Iterate over each queue to learn whether it supports presenting:
    VkBool32 *pSupportsPresent = (VkBool32 *)malloc(info.queue_family_count * sizeof(VkBool32));
    for (uint32_t i = 0; i < info.queue_family_count; i++)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(info.gpus[0], i, info.surface, &pSupportsPresent[i]);
    }

    // Search for a graphics and a present queue in the array of queue
    // families, try to find one that supports both
    info.graphics_queue_family_index = UINT32_MAX;
    info.present_queue_family_index  = UINT32_MAX;
    for (uint32_t i = 0; i < info.queue_family_count; ++i)
    {
        if ((info.queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            if (info.graphics_queue_family_index == UINT32_MAX)
                info.graphics_queue_family_index = i;

            if (pSupportsPresent[i] == VK_TRUE)
            {
                info.graphics_queue_family_index = i;
                info.present_queue_family_index  = i;
                break;
            }
        }
    }

    if (info.present_queue_family_index == UINT32_MAX)
    {
        // If didn't find a queue that supports both graphics and present, then
        // find a separate present queue.
        for (size_t i = 0; i < info.queue_family_count; ++i)
            if (pSupportsPresent[i] == VK_TRUE)
            {
                info.present_queue_family_index = i;
                break;
            }
    }
    free(pSupportsPresent);

    // Generate error if could not find queues that support graphics
    // and present
    if (info.graphics_queue_family_index == UINT32_MAX ||
        info.present_queue_family_index == UINT32_MAX)
    {
        std::cout << "Could not find a queues for both graphics and present";
        exit(-1);
    }

    // Get the list of VkFormats that are supported:
    uint32_t formatCount;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(info.gpus[0], info.surface, &formatCount, NULL);
    ASSERT(res == VK_SUCCESS);
    VkSurfaceFormatKHR *surfFormats =
        (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    res =
        vkGetPhysicalDeviceSurfaceFormatsKHR(info.gpus[0], info.surface, &formatCount, surfFormats);
    ASSERT(res == VK_SUCCESS);
    // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
    // the surface has no preferred format.  Otherwise, at least one
    // supported format will be returned.
    if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        info.format = VK_FORMAT_B8G8R8A8_UNORM;
    }
    else
    {
        ASSERT(formatCount >= 1);
        info.format = surfFormats[0].format;
    }
    free(surfFormats);
}

VkResult init_device(struct sample_info &info)
{
    VkResult res;
    VkDeviceQueueCreateInfo queue_info = {};

    float queue_priorities[1]   = {0.0};
    queue_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext            = NULL;
    queue_info.queueCount       = 1;
    queue_info.pQueuePriorities = queue_priorities;
    queue_info.queueFamilyIndex = info.graphics_queue_family_index;

    VkDeviceCreateInfo device_info    = {};
    device_info.sType                 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.pNext                 = NULL;
    device_info.queueCreateInfoCount  = 1;
    device_info.pQueueCreateInfos     = &queue_info;
    device_info.enabledExtensionCount = info.device_extension_names.size();
    device_info.ppEnabledExtensionNames =
        device_info.enabledExtensionCount ? info.device_extension_names.data() : NULL;
    device_info.pEnabledFeatures = NULL;

    res = vkCreateDevice(info.gpus[0], &device_info, NULL, &info.device);
    ASSERT(res == VK_SUCCESS);
#if ANGLE_SHARED_LIBVULKAN
    volkLoadDevice(info.device);
#endif  // ANGLE_SHARED_LIBVULKAN

    return res;
}

void init_command_pool(struct sample_info &info, VkCommandPoolCreateFlags cmd_pool_create_flags)
{
    /* DEPENDS on init_swapchain_extension() */
    VkResult res;

    VkCommandPoolCreateInfo cmd_pool_info = {};
    cmd_pool_info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_info.pNext                   = NULL;
    cmd_pool_info.queueFamilyIndex        = info.graphics_queue_family_index;
    cmd_pool_info.flags                   = cmd_pool_create_flags;

    res = vkCreateCommandPool(info.device, &cmd_pool_info, NULL, &info.cmd_pool);
    ASSERT(res == VK_SUCCESS);
}

void init_command_buffer(struct sample_info &info)
{
    /* DEPENDS on init_swapchain_extension() and init_command_pool() */
    VkResult res;

    VkCommandBufferAllocateInfo cmd = {};
    cmd.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd.pNext                       = NULL;
    cmd.commandPool                 = info.cmd_pool;
    cmd.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount          = 1;

    res = vkAllocateCommandBuffers(info.device, &cmd, &info.cmd);
    ASSERT(res == VK_SUCCESS);
}

void init_command_buffer_array(struct sample_info &info, int numBuffers)
{
    /* DEPENDS on init_swapchain_extension() and init_command_pool() */
    VkResult res;
    info.cmds.resize(numBuffers);
    ASSERT(info.cmds.data() != NULL);

    VkCommandBufferAllocateInfo cmd = {};
    cmd.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd.pNext                       = NULL;
    cmd.commandPool                 = info.cmd_pool;
    cmd.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount          = numBuffers;

    res = vkAllocateCommandBuffers(info.device, &cmd, info.cmds.data());
    ASSERT(res == VK_SUCCESS);
}

void init_command_buffer2_array(struct sample_info &info, int numBuffers)
{
    /* DEPENDS on init_swapchain_extension() and init_command_pool() */
    VkResult res;
    info.cmd2s.resize(numBuffers);
    VkCommandBufferAllocateInfo cmd = {};
    cmd.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd.pNext                       = NULL;
    cmd.commandPool                 = info.cmd_pool;
    cmd.level                       = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    cmd.commandBufferCount          = numBuffers;

    res = vkAllocateCommandBuffers(info.device, &cmd, info.cmd2s.data());
    ASSERT(res == VK_SUCCESS);
}

void init_device_queue(struct sample_info &info)
{
    /* DEPENDS on init_swapchain_extension() */

    vkGetDeviceQueue(info.device, info.graphics_queue_family_index, 0, &info.graphics_queue);
    if (info.graphics_queue_family_index == info.present_queue_family_index)
    {
        info.present_queue = info.graphics_queue;
    }
    else
    {
        vkGetDeviceQueue(info.device, info.present_queue_family_index, 0, &info.present_queue);
    }
}

void init_swap_chain(struct sample_info &info, VkImageUsageFlags usageFlags)
{
    /* DEPENDS on info.cmd and info.queue initialized */

    VkResult res;
    VkSurfaceCapabilitiesKHR surfCapabilities;

    res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(info.gpus[0], info.surface, &surfCapabilities);
    ASSERT(res == VK_SUCCESS);

    uint32_t presentModeCount;
    res = vkGetPhysicalDeviceSurfacePresentModesKHR(info.gpus[0], info.surface, &presentModeCount,
                                                    NULL);
    ASSERT(res == VK_SUCCESS);
    VkPresentModeKHR *presentModes =
        (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
    ASSERT(presentModes);
    res = vkGetPhysicalDeviceSurfacePresentModesKHR(info.gpus[0], info.surface, &presentModeCount,
                                                    presentModes);
    ASSERT(res == VK_SUCCESS);

    VkExtent2D swapchainExtent;
    // width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
    if (surfCapabilities.currentExtent.width == 0xFFFFFFFF)
    {
        // If the surface size is undefined, the size is set to
        // the size of the images requested.
        swapchainExtent.width  = info.width;
        swapchainExtent.height = info.height;
        if (swapchainExtent.width < surfCapabilities.minImageExtent.width)
        {
            swapchainExtent.width = surfCapabilities.minImageExtent.width;
        }
        else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width)
        {
            swapchainExtent.width = surfCapabilities.maxImageExtent.width;
        }

        if (swapchainExtent.height < surfCapabilities.minImageExtent.height)
        {
            swapchainExtent.height = surfCapabilities.minImageExtent.height;
        }
        else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height)
        {
            swapchainExtent.height = surfCapabilities.maxImageExtent.height;
        }
    }
    else
    {
        // If the surface size is defined, the swap chain size must match
        swapchainExtent = surfCapabilities.currentExtent;
    }

    // The FIFO present mode is guaranteed by the spec to be supported
    // Also note that current Android driver only supports FIFO
    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    for (uint32_t presentModeIndex = 0; presentModeIndex < presentModeCount; ++presentModeIndex)
    {
        if (presentModes[presentModeIndex] == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            break;
        }
    }

    // Determine the number of VkImage's to use in the swap chain.
    // We need to acquire only 1 presentable image at at time.
    // Asking for minImageCount images ensures that we can acquire
    // 1 presentable image as long as we present it before attempting
    // to acquire another.
    uint32_t desiredNumberOfSwapChainImages = surfCapabilities.minImageCount;

    VkSurfaceTransformFlagBitsKHR preTransform;
    if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        preTransform = surfCapabilities.currentTransform;
    }

    // Find a supported composite alpha mode - one of these is guaranteed to be set
    VkCompositeAlphaFlagBitsKHR compositeAlpha         = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (uint32_t i = 0; i < sizeof(compositeAlphaFlags); i++)
    {
        if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i])
        {
            compositeAlpha = compositeAlphaFlags[i];
            break;
        }
    }

    VkSwapchainCreateInfoKHR swapchain_ci = {};
    swapchain_ci.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_ci.pNext                    = NULL;
    swapchain_ci.surface                  = info.surface;
    swapchain_ci.minImageCount            = desiredNumberOfSwapChainImages;
    swapchain_ci.imageFormat              = info.format;
    swapchain_ci.imageExtent.width        = swapchainExtent.width;
    swapchain_ci.imageExtent.height       = swapchainExtent.height;
    swapchain_ci.preTransform             = preTransform;
    swapchain_ci.compositeAlpha           = compositeAlpha;
    swapchain_ci.imageArrayLayers         = 1;
    swapchain_ci.presentMode              = swapchainPresentMode;
    swapchain_ci.oldSwapchain             = VK_NULL_HANDLE;
#ifndef __ANDROID__
    swapchain_ci.clipped = true;
#else
    swapchain_ci.clipped = false;
#endif
    swapchain_ci.imageColorSpace       = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    swapchain_ci.imageUsage            = usageFlags;
    swapchain_ci.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.queueFamilyIndexCount = 0;
    swapchain_ci.pQueueFamilyIndices   = NULL;
    uint32_t queueFamilyIndices[2]     = {(uint32_t)info.graphics_queue_family_index,
                                          (uint32_t)info.present_queue_family_index};
    if (info.graphics_queue_family_index != info.present_queue_family_index)
    {
        // If the graphics and present queues are from different queue families,
        // we either have to explicitly transfer ownership of images between the
        // queues, or we have to create the swapchain with imageSharingMode
        // as VK_SHARING_MODE_CONCURRENT
        swapchain_ci.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        swapchain_ci.queueFamilyIndexCount = 2;
        swapchain_ci.pQueueFamilyIndices   = queueFamilyIndices;
    }

    res = vkCreateSwapchainKHR(info.device, &swapchain_ci, NULL, &info.swap_chain);
    ASSERT(res == VK_SUCCESS);

    res = vkGetSwapchainImagesKHR(info.device, info.swap_chain, &info.swapchainImageCount, NULL);
    ASSERT(res == VK_SUCCESS);

    VkImage *swapchainImages = (VkImage *)malloc(info.swapchainImageCount * sizeof(VkImage));
    ASSERT(swapchainImages);
    res = vkGetSwapchainImagesKHR(info.device, info.swap_chain, &info.swapchainImageCount,
                                  swapchainImages);
    ASSERT(res == VK_SUCCESS);

    for (uint32_t i = 0; i < info.swapchainImageCount; i++)
    {
        swap_chain_buffer sc_buffer;

        VkImageViewCreateInfo color_image_view           = {};
        color_image_view.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        color_image_view.pNext                           = NULL;
        color_image_view.format                          = info.format;
        color_image_view.components.r                    = VK_COMPONENT_SWIZZLE_R;
        color_image_view.components.g                    = VK_COMPONENT_SWIZZLE_G;
        color_image_view.components.b                    = VK_COMPONENT_SWIZZLE_B;
        color_image_view.components.a                    = VK_COMPONENT_SWIZZLE_A;
        color_image_view.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        color_image_view.subresourceRange.baseMipLevel   = 0;
        color_image_view.subresourceRange.levelCount     = 1;
        color_image_view.subresourceRange.baseArrayLayer = 0;
        color_image_view.subresourceRange.layerCount     = 1;
        color_image_view.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        color_image_view.flags                           = 0;

        sc_buffer.image = swapchainImages[i];

        color_image_view.image = sc_buffer.image;

        res = vkCreateImageView(info.device, &color_image_view, NULL, &sc_buffer.view);
        info.buffers.push_back(sc_buffer);
        ASSERT(res == VK_SUCCESS);
    }
    free(swapchainImages);
    info.current_buffer = 0;

    if (NULL != presentModes)
    {
        free(presentModes);
    }
}

bool memory_type_from_properties(struct sample_info &info,
                                 uint32_t typeBits,
                                 VkFlags requirements_mask,
                                 uint32_t *typeIndex)
{
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < info.memory_properties.memoryTypeCount; i++)
    {
        if ((typeBits & 1) == 1)
        {
            // Type is available, does it match user properties?
            if ((info.memory_properties.memoryTypes[i].propertyFlags & requirements_mask) ==
                requirements_mask)
            {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    // No memory types matched, return failure
    return false;
}

void init_depth_buffer(struct sample_info &info)
{
    VkResult res;
    bool pass;
    VkImageCreateInfo image_info = {};

/* allow custom depth formats */
#ifdef __ANDROID__
    // Depth format needs to be VK_FORMAT_D24_UNORM_S8_UINT on Android.
    info.depth.format = VK_FORMAT_D24_UNORM_S8_UINT;
#else
    if (info.depth.format == VK_FORMAT_UNDEFINED)
        info.depth.format = VK_FORMAT_D16_UNORM;
#endif

    const VkFormat depth_format = info.depth.format;
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(info.gpus[0], depth_format, &props);
    if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        image_info.tiling = VK_IMAGE_TILING_LINEAR;
    }
    else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    }
    else
    {
        /* Try other depth formats? */
        std::cout << "depth_format " << depth_format << " Unsupported.\n";
        exit(-1);
    }

    image_info.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext                 = NULL;
    image_info.imageType             = VK_IMAGE_TYPE_2D;
    image_info.format                = depth_format;
    image_info.extent.width          = info.width;
    image_info.extent.height         = info.height;
    image_info.extent.depth          = 1;
    image_info.mipLevels             = 1;
    image_info.arrayLayers           = 1;
    image_info.samples               = NUM_SAMPLES;
    image_info.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.queueFamilyIndexCount = 0;
    image_info.pQueueFamilyIndices   = NULL;
    image_info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    image_info.usage                 = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_info.flags                 = 0;

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext                = NULL;
    mem_alloc.allocationSize       = 0;
    mem_alloc.memoryTypeIndex      = 0;

    VkImageViewCreateInfo view_info           = {};
    view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext                           = NULL;
    view_info.image                           = VK_NULL_HANDLE;
    view_info.format                          = depth_format;
    view_info.components.r                    = VK_COMPONENT_SWIZZLE_R;
    view_info.components.g                    = VK_COMPONENT_SWIZZLE_G;
    view_info.components.b                    = VK_COMPONENT_SWIZZLE_B;
    view_info.components.a                    = VK_COMPONENT_SWIZZLE_A;
    view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
    view_info.subresourceRange.baseMipLevel   = 0;
    view_info.subresourceRange.levelCount     = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount     = 1;
    view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    view_info.flags                           = 0;

    if (depth_format == VK_FORMAT_D16_UNORM_S8_UINT ||
        depth_format == VK_FORMAT_D24_UNORM_S8_UINT || depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT)
    {
        view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkMemoryRequirements mem_reqs;

    /* Create image */
    res = vkCreateImage(info.device, &image_info, NULL, &info.depth.image);
    ASSERT(res == VK_SUCCESS);

    vkGetImageMemoryRequirements(info.device, info.depth.image, &mem_reqs);

    mem_alloc.allocationSize = mem_reqs.size;
    /* Use the memory properties to determine the type of memory required */
    pass = memory_type_from_properties(info, mem_reqs.memoryTypeBits,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                       &mem_alloc.memoryTypeIndex);
    ASSERT(pass);

    /* Allocate memory */
    res = vkAllocateMemory(info.device, &mem_alloc, NULL, &info.depth.mem);
    ASSERT(res == VK_SUCCESS);

    /* Bind memory */
    res = vkBindImageMemory(info.device, info.depth.image, info.depth.mem, 0);
    ASSERT(res == VK_SUCCESS);

    /* Create image view */
    view_info.image = info.depth.image;
    res             = vkCreateImageView(info.device, &view_info, NULL, &info.depth.view);
    ASSERT(res == VK_SUCCESS);
}

void init_uniform_buffer(struct sample_info &info)
{
    VkResult res;
    bool pass;

    info.MVP = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f,  0.5f, 1.0f};

    /* VULKAN_KEY_START */
    VkBufferCreateInfo buf_info    = {};
    buf_info.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.pNext                 = NULL;
    buf_info.usage                 = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buf_info.size                  = sizeof(float) * 16;  // info.MVP.data() size
    buf_info.queueFamilyIndexCount = 0;
    buf_info.pQueueFamilyIndices   = NULL;
    buf_info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.flags                 = 0;
    res = vkCreateBuffer(info.device, &buf_info, NULL, &info.uniform_data.buf);
    ASSERT(res == VK_SUCCESS);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(info.device, info.uniform_data.buf, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext                = NULL;
    alloc_info.memoryTypeIndex      = 0;

    alloc_info.allocationSize = mem_reqs.size;
    pass                      = memory_type_from_properties(
        info, mem_reqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &alloc_info.memoryTypeIndex);
    ASSERT(pass && "No mappable, coherent memory");

    res = vkAllocateMemory(info.device, &alloc_info, NULL, &(info.uniform_data.mem));
    ASSERT(res == VK_SUCCESS);

    uint8_t *pData;
    res = vkMapMemory(info.device, info.uniform_data.mem, 0, mem_reqs.size, 0, (void **)&pData);
    ASSERT(res == VK_SUCCESS);

    memcpy(pData, info.MVP.data(), sizeof(float) * 16);  // info.MVP.data() size

    vkUnmapMemory(info.device, info.uniform_data.mem);

    res = vkBindBufferMemory(info.device, info.uniform_data.buf, info.uniform_data.mem, 0);
    ASSERT(res == VK_SUCCESS);

    info.uniform_data.buffer_info.buffer = info.uniform_data.buf;
    info.uniform_data.buffer_info.offset = 0;
    info.uniform_data.buffer_info.range  = sizeof(float) * 16;  // info.MVP.data() size
}

void init_descriptor_and_pipeline_layouts(struct sample_info &info,
                                          bool use_texture,
                                          VkDescriptorSetLayoutCreateFlags descSetLayoutCreateFlags)
{
    VkDescriptorSetLayoutBinding layout_bindings[2];
    layout_bindings[0].binding            = 0;
    layout_bindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_bindings[0].descriptorCount    = 1;
    layout_bindings[0].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
    layout_bindings[0].pImmutableSamplers = NULL;

    if (use_texture)
    {
        layout_bindings[1].binding            = 1;
        layout_bindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layout_bindings[1].descriptorCount    = 1;
        layout_bindings[1].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
        layout_bindings[1].pImmutableSamplers = NULL;
    }

    /* Next take layout bindings and use them to create a descriptor set layout
     */
    VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
    descriptor_layout.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout.pNext        = NULL;
    descriptor_layout.flags        = descSetLayoutCreateFlags;
    descriptor_layout.bindingCount = use_texture ? 2 : 1;
    descriptor_layout.pBindings    = layout_bindings;

    VkResult res;

    info.desc_layout.resize(NUM_DESCRIPTOR_SETS);
    res =
        vkCreateDescriptorSetLayout(info.device, &descriptor_layout, NULL, info.desc_layout.data());
    ASSERT(res == VK_SUCCESS);

    /* Now use the descriptor layout to create a pipeline layout */
    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = NULL;
    pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pPipelineLayoutCreateInfo.pPushConstantRanges    = NULL;
    pPipelineLayoutCreateInfo.setLayoutCount         = NUM_DESCRIPTOR_SETS;
    pPipelineLayoutCreateInfo.pSetLayouts            = info.desc_layout.data();

    res = vkCreatePipelineLayout(info.device, &pPipelineLayoutCreateInfo, NULL,
                                 &info.pipeline_layout);
    ASSERT(res == VK_SUCCESS);
}

void init_renderpass(struct sample_info &info,
                     bool include_depth,
                     bool clear,
                     VkImageLayout finalLayout)
{
    /* DEPENDS on init_swap_chain() and init_depth_buffer() */

    VkResult res;
    /* Need attachments for render target and depth buffer */
    VkAttachmentDescription attachments[2];
    attachments[0].format        = info.format;
    attachments[0].samples       = NUM_SAMPLES;
    attachments[0].loadOp        = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[0].storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout    = finalLayout;
    attachments[0].flags          = 0;

    if (include_depth)
    {
        attachments[1].format  = info.depth.format;
        attachments[1].samples = NUM_SAMPLES;
        attachments[1].loadOp  = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments[1].flags          = 0;
    }

    VkAttachmentReference color_reference = {};
    color_reference.attachment            = 0;
    color_reference.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_reference = {};
    depth_reference.attachment            = 1;
    depth_reference.layout                = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass    = {};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags                   = 0;
    subpass.inputAttachmentCount    = 0;
    subpass.pInputAttachments       = NULL;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &color_reference;
    subpass.pResolveAttachments     = NULL;
    subpass.pDepthStencilAttachment = include_depth ? &depth_reference : NULL;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments    = NULL;

    VkRenderPassCreateInfo rp_info = {};
    rp_info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.pNext                  = NULL;
    rp_info.attachmentCount        = include_depth ? 2 : 1;
    rp_info.pAttachments           = attachments;
    rp_info.subpassCount           = 1;
    rp_info.pSubpasses             = &subpass;
    rp_info.dependencyCount        = 0;
    rp_info.pDependencies          = NULL;

    res = vkCreateRenderPass(info.device, &rp_info, NULL, &info.render_pass);
    ASSERT(res == VK_SUCCESS);
}

void init_framebuffers(struct sample_info &info, bool include_depth)
{
    /* DEPENDS on init_depth_buffer(), init_renderpass() and
     * init_swapchain_extension() */

    VkResult res;
    VkImageView attachments[2];
    attachments[1] = info.depth.view;

    VkFramebufferCreateInfo fb_info = {};
    fb_info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_info.pNext                   = NULL;
    fb_info.renderPass              = info.render_pass;
    fb_info.attachmentCount         = include_depth ? 2 : 1;
    fb_info.pAttachments            = attachments;
    fb_info.width                   = info.width;
    fb_info.height                  = info.height;
    fb_info.layers                  = 1;

    uint32_t i;

    info.framebuffers = (VkFramebuffer *)malloc(info.swapchainImageCount * sizeof(VkFramebuffer));

    for (i = 0; i < info.swapchainImageCount; i++)
    {
        attachments[0] = info.buffers[i].view;
        res            = vkCreateFramebuffer(info.device, &fb_info, NULL, &info.framebuffers[i]);
        ASSERT(res == VK_SUCCESS);
    }
}

void init_vertex_buffer(struct sample_info &info,
                        const void *vertexData,
                        uint32_t dataSize,
                        uint32_t dataStride,
                        bool use_texture)
{
    VkResult res;
    bool pass;

    VkBufferCreateInfo buf_info    = {};
    buf_info.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.pNext                 = NULL;
    buf_info.usage                 = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buf_info.size                  = dataSize;
    buf_info.queueFamilyIndexCount = 0;
    buf_info.pQueueFamilyIndices   = NULL;
    buf_info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.flags                 = 0;
    res = vkCreateBuffer(info.device, &buf_info, NULL, &info.vertex_buffer.buf);
    ASSERT(res == VK_SUCCESS);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(info.device, info.vertex_buffer.buf, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext                = NULL;
    alloc_info.memoryTypeIndex      = 0;

    alloc_info.allocationSize = mem_reqs.size;
    pass                      = memory_type_from_properties(
        info, mem_reqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &alloc_info.memoryTypeIndex);
    ASSERT(pass && "No mappable, coherent memory");

    res = vkAllocateMemory(info.device, &alloc_info, NULL, &(info.vertex_buffer.mem));
    ASSERT(res == VK_SUCCESS);
    info.vertex_buffer.buffer_info.range  = mem_reqs.size;
    info.vertex_buffer.buffer_info.offset = 0;

    uint8_t *pData;
    res = vkMapMemory(info.device, info.vertex_buffer.mem, 0, mem_reqs.size, 0, (void **)&pData);
    ASSERT(res == VK_SUCCESS);

    memcpy(pData, vertexData, dataSize);

    vkUnmapMemory(info.device, info.vertex_buffer.mem);

    res = vkBindBufferMemory(info.device, info.vertex_buffer.buf, info.vertex_buffer.mem, 0);
    ASSERT(res == VK_SUCCESS);

    info.vi_binding.binding   = 0;
    info.vi_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    info.vi_binding.stride    = dataStride;

    info.vi_attribs[0].binding  = 0;
    info.vi_attribs[0].location = 0;
    info.vi_attribs[0].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
    info.vi_attribs[0].offset   = 0;
    info.vi_attribs[1].binding  = 0;
    info.vi_attribs[1].location = 1;
    info.vi_attribs[1].format =
        use_texture ? VK_FORMAT_R32G32_SFLOAT : VK_FORMAT_R32G32B32A32_SFLOAT;
    info.vi_attribs[1].offset = 16;
}

void init_descriptor_pool(struct sample_info &info, bool use_texture)
{
    /* DEPENDS on init_uniform_buffer() and
     * init_descriptor_and_pipeline_layouts() */

    VkResult res;
    VkDescriptorPoolSize type_count[2];
    type_count[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    type_count[0].descriptorCount = 1;
    if (use_texture)
    {
        type_count[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        type_count[1].descriptorCount = 1;
    }

    VkDescriptorPoolCreateInfo descriptor_pool = {};
    descriptor_pool.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool.pNext                      = NULL;
    descriptor_pool.maxSets                    = 1;
    descriptor_pool.poolSizeCount              = use_texture ? 2 : 1;
    descriptor_pool.pPoolSizes                 = type_count;

    res = vkCreateDescriptorPool(info.device, &descriptor_pool, NULL, &info.desc_pool);
    ASSERT(res == VK_SUCCESS);
}

void init_descriptor_set(struct sample_info &info)
{
    /* DEPENDS on init_descriptor_pool() */

    VkResult res;

    VkDescriptorSetAllocateInfo alloc_info[1];
    alloc_info[0].sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info[0].pNext              = NULL;
    alloc_info[0].descriptorPool     = info.desc_pool;
    alloc_info[0].descriptorSetCount = NUM_DESCRIPTOR_SETS;
    alloc_info[0].pSetLayouts        = info.desc_layout.data();

    info.desc_set.resize(NUM_DESCRIPTOR_SETS);
    res = vkAllocateDescriptorSets(info.device, alloc_info, info.desc_set.data());
    ASSERT(res == VK_SUCCESS);

    VkWriteDescriptorSet writes[2];

    writes[0]                 = {};
    writes[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].pNext           = NULL;
    writes[0].dstSet          = info.desc_set[0];
    writes[0].descriptorCount = 1;
    writes[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].pBufferInfo     = &info.uniform_data.buffer_info;
    writes[0].dstArrayElement = 0;
    writes[0].dstBinding      = 0;

    vkUpdateDescriptorSets(info.device, 1, writes, 0, NULL);
}

bool GLSLtoSPV(const VkShaderStageFlagBits shader_type,
               const char *pshader,
               std::vector<unsigned int> &spirv)
{
    EShLanguage stage = FindLanguage(shader_type);
    glslang::TShader shader(stage);
    glslang::TProgram program;
    const char *shaderStrings[1];
    TBuiltInResource Resources;
    init_resources(Resources);

    // Enable SPIR-V and Vulkan rules when parsing GLSL
    EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

    shaderStrings[0] = pshader;
    shader.setStrings(shaderStrings, 1);

    if (!shader.parse(&Resources, 100, false, messages))
    {
        puts(shader.getInfoLog());
        puts(shader.getInfoDebugLog());
        return false;  // something didn't work
    }

    program.addShader(&shader);

    //
    // Program-level processing...
    //

    if (!program.link(messages))
    {
        puts(shader.getInfoLog());
        puts(shader.getInfoDebugLog());
        fflush(stdout);
        return false;
    }

    glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);
    return true;
}

void init_shaders(struct sample_info &info, const char *vertShaderText, const char *fragShaderText)
{
    VkResult res;
    bool retVal;

    // If no shaders were submitted, just return
    if (!(vertShaderText || fragShaderText))
        return;

    glslang::InitializeProcess();
    VkShaderModuleCreateInfo moduleCreateInfo;

    if (vertShaderText)
    {
        std::vector<unsigned int> vtx_spv;
        info.shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.shaderStages[0].pNext = NULL;
        info.shaderStages[0].pSpecializationInfo = NULL;
        info.shaderStages[0].flags               = 0;
        info.shaderStages[0].stage               = VK_SHADER_STAGE_VERTEX_BIT;
        info.shaderStages[0].pName               = "main";

        retVal = GLSLtoSPV(VK_SHADER_STAGE_VERTEX_BIT, vertShaderText, vtx_spv);
        ASSERT(retVal);

        moduleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.pNext    = NULL;
        moduleCreateInfo.flags    = 0;
        moduleCreateInfo.codeSize = vtx_spv.size() * sizeof(unsigned int);
        moduleCreateInfo.pCode    = vtx_spv.data();
        res                       = vkCreateShaderModule(info.device, &moduleCreateInfo, NULL,
                                                         &info.shaderStages[0].module);
        ASSERT(res == VK_SUCCESS);
    }

    if (fragShaderText)
    {
        std::vector<unsigned int> frag_spv;
        info.shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.shaderStages[1].pNext = NULL;
        info.shaderStages[1].pSpecializationInfo = NULL;
        info.shaderStages[1].flags               = 0;
        info.shaderStages[1].stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
        info.shaderStages[1].pName               = "main";

        retVal = GLSLtoSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderText, frag_spv);
        ASSERT(retVal);

        moduleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.pNext    = NULL;
        moduleCreateInfo.flags    = 0;
        moduleCreateInfo.codeSize = frag_spv.size() * sizeof(unsigned int);
        moduleCreateInfo.pCode    = frag_spv.data();
        res                       = vkCreateShaderModule(info.device, &moduleCreateInfo, NULL,
                                                         &info.shaderStages[1].module);
        ASSERT(res == VK_SUCCESS);
    }

    glslang::FinalizeProcess();
}

void init_pipeline_cache(struct sample_info &info)
{
    VkResult res;

    VkPipelineCacheCreateInfo pipelineCache;
    pipelineCache.sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipelineCache.pNext           = NULL;
    pipelineCache.initialDataSize = 0;
    pipelineCache.pInitialData    = NULL;
    pipelineCache.flags           = 0;
    res = vkCreatePipelineCache(info.device, &pipelineCache, NULL, &info.pipelineCache);
    ASSERT(res == VK_SUCCESS);
}

void init_pipeline(struct sample_info &info, VkBool32 include_depth, VkBool32 include_vi)
{
    VkResult res;

    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext             = NULL;
    dynamicState.pDynamicStates    = NULL;
    dynamicState.dynamicStateCount = 0;

    VkPipelineVertexInputStateCreateInfo vi;
    memset(&vi, 0, sizeof(vi));
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    if (include_vi)
    {
        vi.pNext                           = NULL;
        vi.flags                           = 0;
        vi.vertexBindingDescriptionCount   = 1;
        vi.pVertexBindingDescriptions      = &info.vi_binding;
        vi.vertexAttributeDescriptionCount = 2;
        vi.pVertexAttributeDescriptions    = info.vi_attribs;
    }
    VkPipelineInputAssemblyStateCreateInfo ia;
    ia.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.pNext                  = NULL;
    ia.flags                  = 0;
    ia.primitiveRestartEnable = VK_FALSE;
    ia.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rs;
    rs.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.pNext                   = NULL;
    rs.flags                   = 0;
    rs.polygonMode             = VK_POLYGON_MODE_FILL;
    rs.cullMode                = VK_CULL_MODE_BACK_BIT;
    rs.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rs.depthClampEnable        = VK_FALSE;
    rs.rasterizerDiscardEnable = VK_FALSE;
    rs.depthBiasEnable         = VK_FALSE;
    rs.depthBiasConstantFactor = 0;
    rs.depthBiasClamp          = 0;
    rs.depthBiasSlopeFactor    = 0;
    rs.lineWidth               = 1.0f;

    VkPipelineColorBlendStateCreateInfo cb;
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.flags = 0;
    cb.pNext = NULL;
    VkPipelineColorBlendAttachmentState att_state[1];
    att_state[0].colorWriteMask      = 0xf;
    att_state[0].blendEnable         = VK_FALSE;
    att_state[0].alphaBlendOp        = VK_BLEND_OP_ADD;
    att_state[0].colorBlendOp        = VK_BLEND_OP_ADD;
    att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    cb.attachmentCount               = 1;
    cb.pAttachments                  = att_state;
    cb.logicOpEnable                 = VK_FALSE;
    cb.logicOp                       = VK_LOGIC_OP_NO_OP;
    cb.blendConstants[0]             = 1.0f;
    cb.blendConstants[1]             = 1.0f;
    cb.blendConstants[2]             = 1.0f;
    cb.blendConstants[3]             = 1.0f;

    VkPipelineViewportStateCreateInfo vp = {};
    vp.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.pNext                             = NULL;
    vp.flags                             = 0;
#ifndef __ANDROID__
    vp.viewportCount = NUM_VIEWPORTS;
    dynamicState.dynamicStateCount++;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    vp.scissorCount = NUM_SCISSORS;
    dynamicState.dynamicStateCount++;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
    vp.pScissors  = NULL;
    vp.pViewports = NULL;
#else
    // Temporary disabling dynamic viewport on Android because some of drivers doesn't
    // support the feature.
    VkViewport viewports;
    viewports.minDepth = 0.0f;
    viewports.maxDepth = 1.0f;
    viewports.x        = 0;
    viewports.y        = 0;
    viewports.width    = info.width;
    viewports.height   = info.height;
    VkRect2D scissor;
    scissor.extent.width  = info.width;
    scissor.extent.height = info.height;
    scissor.offset.x      = 0;
    scissor.offset.y      = 0;
    vp.viewportCount      = NUM_VIEWPORTS;
    vp.scissorCount       = NUM_SCISSORS;
    vp.pScissors          = &scissor;
    vp.pViewports         = &viewports;
#endif
    VkPipelineDepthStencilStateCreateInfo ds;
    ds.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.pNext                 = NULL;
    ds.flags                 = 0;
    ds.depthTestEnable       = include_depth;
    ds.depthWriteEnable      = include_depth;
    ds.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
    ds.depthBoundsTestEnable = VK_FALSE;
    ds.stencilTestEnable     = VK_FALSE;
    ds.back.failOp           = VK_STENCIL_OP_KEEP;
    ds.back.passOp           = VK_STENCIL_OP_KEEP;
    ds.back.compareOp        = VK_COMPARE_OP_ALWAYS;
    ds.back.compareMask      = 0;
    ds.back.reference        = 0;
    ds.back.depthFailOp      = VK_STENCIL_OP_KEEP;
    ds.back.writeMask        = 0;
    ds.minDepthBounds        = 0;
    ds.maxDepthBounds        = 0;
    ds.stencilTestEnable     = VK_FALSE;
    ds.front                 = ds.back;

    VkPipelineMultisampleStateCreateInfo ms;
    ms.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.pNext                 = NULL;
    ms.flags                 = 0;
    ms.pSampleMask           = NULL;
    ms.rasterizationSamples  = NUM_SAMPLES;
    ms.sampleShadingEnable   = VK_FALSE;
    ms.alphaToCoverageEnable = VK_FALSE;
    ms.alphaToOneEnable      = VK_FALSE;
    ms.minSampleShading      = 0.0;

    VkGraphicsPipelineCreateInfo pipeline;
    pipeline.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline.pNext               = NULL;
    pipeline.layout              = info.pipeline_layout;
    pipeline.basePipelineHandle  = VK_NULL_HANDLE;
    pipeline.basePipelineIndex   = 0;
    pipeline.flags               = 0;
    pipeline.pVertexInputState   = &vi;
    pipeline.pInputAssemblyState = &ia;
    pipeline.pRasterizationState = &rs;
    pipeline.pColorBlendState    = &cb;
    pipeline.pTessellationState  = NULL;
    pipeline.pMultisampleState   = &ms;
    pipeline.pDynamicState       = &dynamicState;
    pipeline.pViewportState      = &vp;
    pipeline.pDepthStencilState  = &ds;
    pipeline.pStages             = info.shaderStages;
    pipeline.stageCount          = 2;
    pipeline.renderPass          = info.render_pass;
    pipeline.subpass             = 0;

    if (dynamicStateEnables.size() > 0)
    {
        dynamicState.pDynamicStates    = dynamicStateEnables.data();
        dynamicState.dynamicStateCount = dynamicStateEnables.size();
    }

    res = vkCreateGraphicsPipelines(info.device, info.pipelineCache, 1, &pipeline, NULL,
                                    &info.pipeline);
    ASSERT(res == VK_SUCCESS);
}

void init_viewports(struct sample_info &info)
{
#ifdef __ANDROID__
// Disable dynamic viewport on Android. Some drive has an issue with the dynamic viewport
// feature.
#else
    info.viewport.height   = (float)info.height;
    info.viewport.width    = (float)info.width;
    info.viewport.minDepth = (float)0.0f;
    info.viewport.maxDepth = (float)1.0f;
    info.viewport.x        = 0;
    info.viewport.y        = 0;
    vkCmdSetViewport(info.cmd, 0, NUM_VIEWPORTS, &info.viewport);
#endif
}

void init_viewports_array(struct sample_info &info, int index)
{
#ifdef __ANDROID__
// Disable dynamic viewport on Android. Some drive has an issue with the dynamic viewport
// feature.
#else
    info.viewport.height   = (float)info.height;
    info.viewport.width    = (float)info.width;
    info.viewport.minDepth = (float)0.0f;
    info.viewport.maxDepth = (float)1.0f;
    info.viewport.x        = 0;
    info.viewport.y        = 0;
    vkCmdSetViewport(info.cmds[index], 0, NUM_VIEWPORTS, &info.viewport);
#endif
}

void init_viewports2_array(struct sample_info &info, int index)
{
#ifdef __ANDROID__
// Disable dynamic viewport on Android. Some drive has an issue with the dynamic viewport
// feature.
#else
    info.viewport.height   = (float)info.height;
    info.viewport.width    = (float)info.width;
    info.viewport.minDepth = (float)0.0f;
    info.viewport.maxDepth = (float)1.0f;
    info.viewport.x        = 0;
    info.viewport.y        = 0;
    vkCmdSetViewport(info.cmd2s[index], 0, NUM_VIEWPORTS, &info.viewport);
#endif
}

void init_scissors(struct sample_info &info)
{
#ifdef __ANDROID__
// Disable dynamic viewport on Android. Some drive has an issue with the dynamic scissors
// feature.
#else
    info.scissor.extent.width  = info.width;
    info.scissor.extent.height = info.height;
    info.scissor.offset.x      = 0;
    info.scissor.offset.y      = 0;
    vkCmdSetScissor(info.cmd, 0, NUM_SCISSORS, &info.scissor);
#endif
}

void init_scissors_array(struct sample_info &info, int index)
{
#ifdef __ANDROID__
// Disable dynamic viewport on Android. Some drive has an issue with the dynamic scissors
// feature.
#else
    info.scissor.extent.width  = info.width;
    info.scissor.extent.height = info.height;
    info.scissor.offset.x      = 0;
    info.scissor.offset.y      = 0;
    vkCmdSetScissor(info.cmds[index], 0, NUM_SCISSORS, &info.scissor);
#endif
}

void init_scissors2_array(struct sample_info &info, int index)
{
#ifdef __ANDROID__
// Disable dynamic viewport on Android. Some drive has an issue with the dynamic scissors
// feature.
#else
    info.scissor.extent.width  = info.width;
    info.scissor.extent.height = info.height;
    info.scissor.offset.x      = 0;
    info.scissor.offset.y      = 0;
    vkCmdSetScissor(info.cmd2s[index], 0, NUM_SCISSORS, &info.scissor);
#endif
}

void destroy_pipeline(struct sample_info &info)
{
    vkDestroyPipeline(info.device, info.pipeline, NULL);
}

void destroy_pipeline_cache(struct sample_info &info)
{
    vkDestroyPipelineCache(info.device, info.pipelineCache, NULL);
}

void destroy_uniform_buffer(struct sample_info &info)
{
    vkDestroyBuffer(info.device, info.uniform_data.buf, NULL);
    vkFreeMemory(info.device, info.uniform_data.mem, NULL);
}

void destroy_descriptor_and_pipeline_layouts(struct sample_info &info)
{
    for (int i = 0; i < NUM_DESCRIPTOR_SETS; i++)
        vkDestroyDescriptorSetLayout(info.device, info.desc_layout[i], NULL);
    vkDestroyPipelineLayout(info.device, info.pipeline_layout, NULL);
}

void destroy_descriptor_pool(struct sample_info &info)
{
    vkDestroyDescriptorPool(info.device, info.desc_pool, NULL);
}

void destroy_shaders(struct sample_info &info)
{
    vkDestroyShaderModule(info.device, info.shaderStages[0].module, NULL);
    vkDestroyShaderModule(info.device, info.shaderStages[1].module, NULL);
}

void destroy_command_buffer(struct sample_info &info)
{
    VkCommandBuffer cmd_bufs[1] = {info.cmd};
    vkFreeCommandBuffers(info.device, info.cmd_pool, 1, cmd_bufs);
}

void destroy_command_buffer_array(struct sample_info &info, int numBuffers)
{
    vkFreeCommandBuffers(info.device, info.cmd_pool, numBuffers, info.cmds.data());
}

void reset_command_buffer2_array(struct sample_info &info,
                                 VkCommandBufferResetFlags cmd_buffer_reset_flags)
{
    for (auto cb : info.cmd2s)
    {
        vkResetCommandBuffer(cb, cmd_buffer_reset_flags);
    }
}

void destroy_command_buffer2_array(struct sample_info &info, int numBuffers)
{
    vkFreeCommandBuffers(info.device, info.cmd_pool, numBuffers, info.cmd2s.data());
}

void reset_command_pool(struct sample_info &info, VkCommandPoolResetFlags cmd_pool_reset_flags)
{
    vkResetCommandPool(info.device, info.cmd_pool, cmd_pool_reset_flags);
}

void destroy_command_pool(struct sample_info &info)
{
    vkDestroyCommandPool(info.device, info.cmd_pool, NULL);
}

void destroy_depth_buffer(struct sample_info &info)
{
    vkDestroyImageView(info.device, info.depth.view, NULL);
    vkDestroyImage(info.device, info.depth.image, NULL);
    vkFreeMemory(info.device, info.depth.mem, NULL);
}

void destroy_vertex_buffer(struct sample_info &info)
{
    vkDestroyBuffer(info.device, info.vertex_buffer.buf, NULL);
    vkFreeMemory(info.device, info.vertex_buffer.mem, NULL);
}

void destroy_swap_chain(struct sample_info &info)
{
    for (uint32_t i = 0; i < info.swapchainImageCount; i++)
    {
        vkDestroyImageView(info.device, info.buffers[i].view, NULL);
    }
    vkDestroySwapchainKHR(info.device, info.swap_chain, NULL);
}

void destroy_framebuffers(struct sample_info &info)
{
    for (uint32_t i = 0; i < info.swapchainImageCount; i++)
    {
        vkDestroyFramebuffer(info.device, info.framebuffers[i], NULL);
    }
    free(info.framebuffers);
}

void destroy_renderpass(struct sample_info &info)
{
    vkDestroyRenderPass(info.device, info.render_pass, NULL);
}

void destroy_device(struct sample_info &info)
{
    vkDeviceWaitIdle(info.device);
    vkDestroyDevice(info.device, NULL);
}

void destroy_instance(struct sample_info &info)
{
    vkDestroyInstance(info.inst, NULL);
}
