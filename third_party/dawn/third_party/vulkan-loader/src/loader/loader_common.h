/*
 *
 * Copyright (c) 2014-2022 The Khronos Group Inc.
 * Copyright (c) 2014-2022 Valve Corporation
 * Copyright (c) 2014-2022 LunarG, Inc.
 * Copyright (C) 2015 Google Inc.
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
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chia-I Wu <olv@lunarg.com>
 * Author: Mark Lobodzinski <mark@LunarG.com>
 * Author: Lenny Komow <lenny@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 *
 */

#pragma once

#include "vulkan/vk_platform.h"
#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vk_icd.h>

#include "vk_loader_platform.h"
#include "vk_loader_layer.h"
#include "vk_layer_dispatch_table.h"
#include "vk_loader_extensions.h"

#include "settings.h"

typedef enum VkStringErrorFlagBits {
    VK_STRING_ERROR_NONE = 0x00000000,
    VK_STRING_ERROR_LENGTH = 0x00000001,
    VK_STRING_ERROR_BAD_DATA = 0x00000002,
    VK_STRING_ERROR_NULL_PTR = 0x00000004,
} VkStringErrorFlagBits;
typedef VkFlags VkStringErrorFlags;

static const int MaxLoaderStringLength = 256;           // 0xFF;
static const unsigned char UTF8_ONE_BYTE_CODE = 192;    // 0xC0;
static const unsigned char UTF8_ONE_BYTE_MASK = 224;    // 0xE0;
static const unsigned char UTF8_TWO_BYTE_CODE = 224;    // 0xE0;
static const unsigned char UTF8_TWO_BYTE_MASK = 240;    // 0xF0;
static const unsigned char UTF8_THREE_BYTE_CODE = 240;  // 0xF0;
static const unsigned char UTF8_THREE_BYTE_MASK = 248;  // 0xF8;
static const unsigned char UTF8_DATA_BYTE_CODE = 128;   // 0x80;
static const unsigned char UTF8_DATA_BYTE_MASK = 192;   // 0xC0;

// form of all dynamic lists/arrays
// only the list element should be changed
struct loader_generic_list {
    size_t capacity;
    uint32_t count;
    void *list;
};

struct loader_string_list {
    uint32_t allocated_count;
    uint32_t count;
    char **list;
};

struct loader_extension_list {
    size_t capacity;
    uint32_t count;
    VkExtensionProperties *list;
};

struct loader_dev_ext_props {
    VkExtensionProperties props;
    struct loader_string_list entrypoints;
};

struct loader_device_extension_list {
    size_t capacity;
    uint32_t count;
    struct loader_dev_ext_props *list;
};

struct loader_used_object_status {
    VkBool32 status;
    VkAllocationCallbacks allocation_callbacks;
};

struct loader_used_object_list {
    size_t capacity;
    uint32_t padding;  // count variable isn't used
    struct loader_used_object_status *list;
};

struct loader_surface_allocation {
    VkSurfaceKHR surface;
    VkAllocationCallbacks allocation_callbacks;
};

struct loader_surface_list {
    size_t capacity;
    uint32_t padding;  // count variable isn't used
    VkSurfaceKHR *list;
};

struct loader_debug_utils_messenger_list {
    size_t capacity;
    uint32_t padding;  // count variable isn't used
    VkDebugUtilsMessengerEXT *list;
};

struct loader_debug_report_callback_list {
    size_t capacity;
    uint32_t padding;  // count variable isn't used
    VkDebugReportCallbackEXT *list;
};

struct loader_name_value {
    char *name;
    char *value;
};

struct loader_layer_functions {
    char *str_gipa;
    char *str_gdpa;
    char *str_negotiate_interface;
    PFN_vkNegotiateLoaderLayerInterfaceVersion negotiate_layer_interface;
    PFN_vkGetInstanceProcAddr get_instance_proc_addr;
    PFN_vkGetDeviceProcAddr get_device_proc_addr;
    PFN_GetPhysicalDeviceProcAddr get_physical_device_proc_addr;
};

// This structure is used to store the json file version in a more manageable way.
typedef struct {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
} loader_api_version;

// Enumeration used to clearly identify reason for library load failure.
enum loader_layer_library_status {
    LOADER_LAYER_LIB_NOT_LOADED = 0,

    LOADER_LAYER_LIB_SUCCESS_LOADED = 1,

    LOADER_LAYER_LIB_ERROR_WRONG_BIT_TYPE = 20,
    LOADER_LAYER_LIB_ERROR_FAILED_TO_LOAD = 21,
    LOADER_LAYER_LIB_ERROR_OUT_OF_MEMORY = 22,
};

enum layer_type_flags {
    VK_LAYER_TYPE_FLAG_INSTANCE_LAYER = 0x1,  // If not set, indicates Device layer
    VK_LAYER_TYPE_FLAG_EXPLICIT_LAYER = 0x2,  // If not set, indicates Implicit layer
    VK_LAYER_TYPE_FLAG_META_LAYER = 0x4,      // If not set, indicates standard layer
};

enum loader_layer_enabled_by_what {
    ENABLED_BY_WHAT_UNSET,  // default value indicates this field hasn't been filled in
    ENABLED_BY_WHAT_LOADER_SETTINGS_FILE,
    ENABLED_BY_WHAT_IMPLICIT_LAYER,
    ENABLED_BY_WHAT_VK_INSTANCE_LAYERS,
    ENABLED_BY_WHAT_VK_LOADER_LAYERS_ENABLE,
    ENABLED_BY_WHAT_IN_APPLICATION_API,
    ENABLED_BY_WHAT_META_LAYER,
};

struct loader_layer_properties {
    VkLayerProperties info;
    enum layer_type_flags type_flags;
    enum loader_settings_layer_control settings_control_value;

    uint32_t interface_version;  // PFN_vkNegotiateLoaderLayerInterfaceVersion
    char *manifest_file_name;
    char *lib_name;
    enum loader_layer_library_status lib_status;
    enum loader_layer_enabled_by_what enabled_by_what;
    loader_platform_dl_handle lib_handle;
    struct loader_layer_functions functions;
    struct loader_extension_list instance_extension_list;
    struct loader_device_extension_list device_extension_list;
    struct loader_name_value disable_env_var;
    struct loader_name_value enable_env_var;
    struct loader_string_list component_layer_names;
    struct {
        char *enumerate_instance_extension_properties;
        char *enumerate_instance_layer_properties;
        char *enumerate_instance_version;
    } pre_instance_functions;
    struct loader_string_list override_paths;
    bool is_override;
    bool keep;
    struct loader_string_list blacklist_layer_names;
    struct loader_string_list app_key_paths;
};

// Stores a list of loader_layer_properties
struct loader_layer_list {
    size_t capacity;
    uint32_t count;
    struct loader_layer_properties *list;
};

// Stores a list of pointers to loader_layer_properties
// Used for app_activated_layer_list and expanded_activated_layer_list
struct loader_pointer_layer_list {
    size_t capacity;
    uint32_t count;
    struct loader_layer_properties **list;
};

typedef VkResult(VKAPI_PTR *PFN_vkDevExt)(VkDevice device);

struct loader_dev_dispatch_table {
    VkLayerDispatchTable core_dispatch;
    PFN_vkDevExt ext_dispatch[MAX_NUM_UNKNOWN_EXTS];
    struct loader_device_terminator_dispatch extension_terminator_dispatch;
};

// per CreateDevice structure
struct loader_device {
    struct loader_dev_dispatch_table loader_dispatch;
    VkDevice chain_device;  // device object from the dispatch chain
    VkDevice icd_device;    // device object from the icd
    struct loader_physical_device_term *phys_dev_term;

    VkAllocationCallbacks alloc_callbacks;

    // List of activated device extensions that layers support (but not necessarily the driver which have functions that require
    // trampolines to work correctly. EX - vkDebugMarkerSetObjectNameEXT can name wrapped handles like instance, physical device,
    // or surface
    struct {
        bool ext_debug_marker_enabled;
        bool ext_debug_utils_enabled;
    } layer_extensions;

    // List of activated device extensions that have terminators implemented in the loader
    struct {
        bool khr_swapchain_enabled;
        bool khr_display_swapchain_enabled;
        bool khr_device_group_enabled;
        bool ext_debug_marker_enabled;
        bool ext_debug_utils_enabled;
        bool ext_full_screen_exclusive_enabled;
        bool version_1_1_enabled;
        bool version_1_2_enabled;
        bool version_1_3_enabled;
    } driver_extensions;

    struct loader_device *next;

    // Makes vkGetDeviceProcAddr check if core functions are supported by the current app_api_version.
    // Only set to true if VK_KHR_maintenance5 is enabled.
    bool should_ignore_device_commands_from_newer_version;
};

// Per ICD information

// Per ICD structure
struct loader_icd_term {
    // pointers to find other structs
    const struct loader_scanned_icd *scanned_icd;
    const struct loader_instance *this_instance;
    struct loader_device *logical_device_list;
    VkInstance instance;  // instance object from the icd
    struct loader_icd_term_dispatch dispatch;

    struct loader_icd_term *next;

    PFN_PhysDevExt phys_dev_ext[MAX_NUM_UNKNOWN_EXTS];
    bool supports_get_dev_prop_2;
    bool supports_ext_surface_maintenance_1;

    uint32_t physical_device_count;

    struct loader_surface_list surface_list;
    struct loader_debug_utils_messenger_list debug_utils_messenger_list;
    struct loader_debug_report_callback_list debug_report_callback_list;
};

// Per ICD library structure
struct loader_icd_tramp_list {
    size_t capacity;
    uint32_t count;
    struct loader_scanned_icd *scanned_list;
};

struct loader_instance_dispatch_table {
    VkLayerInstanceDispatchTable layer_inst_disp;  // must be first entry in structure

    // Physical device functions unknown to the loader
    PFN_PhysDevExt phys_dev_ext[MAX_NUM_UNKNOWN_EXTS];
};

// Unique magic number identifier for the loader.
#define LOADER_MAGIC_NUMBER 0x10ADED010110ADEDUL

// Per instance structure
struct loader_instance {
    struct loader_instance_dispatch_table *disp;  // must be first entry in structure
    uint64_t magic;                               // Should be LOADER_MAGIC_NUMBER

    // Store all the terminators for instance functions in case a layer queries one *after* vkCreateInstance
    VkLayerInstanceDispatchTable terminator_dispatch;

    // Vulkan API version the app is intending to use.
    loader_api_version app_api_version;

    // We need to manually track physical devices over time.  If the user
    // re-queries the information, we don't want to delete old data or
    // create new data unless necessary.
    uint32_t total_gpu_count;
    uint32_t phys_dev_count_term;
    struct loader_physical_device_term **phys_devs_term;
    uint32_t phys_dev_count_tramp;
    struct loader_physical_device_tramp **phys_devs_tramp;

    // We also need to manually track physical device groups, but we don't need
    // loader specific structures since we have that content in the physical
    // device stored internal to the public structures.
    uint32_t phys_dev_group_count_term;
    struct VkPhysicalDeviceGroupProperties **phys_dev_groups_term;

    struct loader_instance *next;

    uint32_t icd_terms_count;
    struct loader_icd_term *icd_terms;
    struct loader_icd_tramp_list icd_tramp_list;

    // Must store the strings inside loader_instance directly - since the asm code will offset into
    // loader_instance to get the function name
    uint32_t dev_ext_disp_function_count;
    char *dev_ext_disp_functions[MAX_NUM_UNKNOWN_EXTS];
    uint32_t phys_dev_ext_disp_function_count;
    char *phys_dev_ext_disp_functions[MAX_NUM_UNKNOWN_EXTS];

    struct loader_msg_callback_map_entry *icd_msg_callback_map;

    struct loader_string_list enabled_layer_names;

    struct loader_layer_list instance_layer_list;
    bool override_layer_present;

    // List of activated layers.
    //  app_      is the version based on exactly what the application asked for.
    //            This is what must be returned to the application on Enumerate calls.
    //  expanded_ is the version based on expanding meta-layers into their
    //            individual component layers.  This is what is used internally.
    struct loader_pointer_layer_list app_activated_layer_list;
    struct loader_pointer_layer_list expanded_activated_layer_list;

    VkInstance instance;  // layers/ICD instance returned to trampoline

    struct loader_extension_list ext_list;  // icds and loaders extensions
    struct loader_instance_extension_enables enabled_known_extensions;

    // Indicates which indices in the array are in-use and which are free to be reused
    struct loader_used_object_list surfaces_list;
    struct loader_used_object_list debug_utils_messengers_list;
    struct loader_used_object_list debug_report_callbacks_list;

    // Stores debug callbacks - used in the log.
    VkLayerDbgFunctionNode *current_dbg_function_head;        // Current head
    VkLayerDbgFunctionNode *instance_only_dbg_function_head;  // Only used for instance create/destroy

    VkAllocationCallbacks alloc_callbacks;

    // Set to true after vkCreateInstance has returned - necessary for loader_gpa_instance_terminator()
    bool instance_finished_creation;

    loader_settings settings;

    bool portability_enumeration_enabled;
    bool portability_enumeration_flag_bit_set;
    bool portability_enumeration_extension_enabled;

    bool wsi_surface_enabled;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    bool wsi_win32_surface_enabled;
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    bool wsi_wayland_surface_enabled;
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    bool wsi_xcb_surface_enabled;
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    bool wsi_xlib_surface_enabled;
#endif
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    bool wsi_directfb_surface_enabled;
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    bool wsi_android_surface_enabled;
#endif
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    bool wsi_macos_surface_enabled;
#endif
#if defined(VK_USE_PLATFORM_IOS_MVK)
    bool wsi_ios_surface_enabled;
#endif
#if defined(VK_USE_PLATFORM_GGP)
    bool wsi_ggp_surface_enabled;
#endif
    bool wsi_headless_surface_enabled;
#if defined(VK_USE_PLATFORM_METAL_EXT)
    bool wsi_metal_surface_enabled;
#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
    bool wsi_imagepipe_surface_enabled;
#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    bool wsi_screen_surface_enabled;
#endif
#if defined(VK_USE_PLATFORM_VI_NN)
    bool wsi_vi_surface_enabled;
#endif
    bool wsi_display_enabled;
    bool wsi_display_props2_enabled;
    bool create_terminator_invalid_extension;
    bool supports_get_dev_prop_2;
};

// VkPhysicalDevice requires special treatment by loader.  Firstly, terminator
// code must be able to get the struct loader_icd_term to call into the proper
// driver  (multiple ICD/gpu case). This can be accomplished by wrapping the
// created VkPhysicalDevice in loader terminate_EnumeratePhysicalDevices().
// Secondly, the loader must be able to handle wrapped by layer VkPhysicalDevice
// in trampoline code.  This implies, that the loader trampoline code must also
// wrap the VkPhysicalDevice object in trampoline code.  Thus, loader has to
// wrap the VkPhysicalDevice created object twice. In trampoline code it can't
// rely on the terminator object wrapping since a layer may also wrap. Since
// trampoline code wraps the VkPhysicalDevice this means all loader trampoline
// code that passes a VkPhysicalDevice should unwrap it.

// Unique identifier for physical devices
#define PHYS_TRAMP_MAGIC_NUMBER 0x10ADED020210ADEDUL

// Per enumerated PhysicalDevice structure, used to wrap in trampoline code and
// also same structure used to wrap in terminator code
struct loader_physical_device_tramp {
    struct loader_instance_dispatch_table *disp;  // must be first entry in structure
    struct loader_instance *this_instance;
    uint64_t magic;             // Should be PHYS_TRAMP_MAGIC_NUMBER
    VkPhysicalDevice phys_dev;  // object from layers/loader terminator
};

// Per enumerated PhysicalDevice structure, used to wrap in terminator code
struct loader_physical_device_term {
    struct loader_instance_dispatch_table *disp;  // must be first entry in structure
    struct loader_icd_term *this_icd_term;
    VkPhysicalDevice phys_dev;  // object from ICD
};

#if defined(LOADER_ENABLE_LINUX_SORT)
// Structure for storing the relevant device information for selecting a device.
// NOTE: Needs to be defined here so we can store this content in the term structure
//       for quicker sorting.
struct LinuxSortedDeviceInfo {
    // Associated Vulkan Physical Device
    VkPhysicalDevice physical_device;
    bool default_device;

    // Loader specific items about the driver providing support for this physical device
    struct loader_icd_term *icd_term;

    // Some generic device properties
    VkPhysicalDeviceType device_type;
    char device_name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
    uint32_t vendor_id;
    uint32_t device_id;

    // PCI information on this device
    bool has_pci_bus_info;
    uint32_t pci_domain;
    uint32_t pci_bus;
    uint32_t pci_device;
    uint32_t pci_function;
};
#endif  // LOADER_ENABLE_LINUX_SORT

// Per enumerated PhysicalDeviceGroup structure, used to wrap in terminator code
struct loader_physical_device_group_term {
    struct loader_icd_term *this_icd_term;
    VkPhysicalDeviceGroupProperties group_props;
#if defined(LOADER_ENABLE_LINUX_SORT)
    struct LinuxSortedDeviceInfo internal_device_info[VK_MAX_DEVICE_GROUP_SIZE];
#endif  // LOADER_ENABLE_LINUX_SORT
};

struct loader_struct {
    struct loader_instance *instances;
};

struct loader_scanned_icd {
    char *lib_name;
    loader_platform_dl_handle handle;
    uint32_t api_version;
    uint32_t interface_version;
    PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
    PFN_GetPhysicalDeviceProcAddr GetPhysicalDeviceProcAddr;
    PFN_vkCreateInstance CreateInstance;
    PFN_vkEnumerateInstanceExtensionProperties EnumerateInstanceExtensionProperties;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vk_icdEnumerateAdapterPhysicalDevices EnumerateAdapterPhysicalDevices;
#endif
};

enum loader_data_files_type {
    LOADER_DATA_FILE_MANIFEST_DRIVER = 0,
    LOADER_DATA_FILE_MANIFEST_EXPLICIT_LAYER,
    LOADER_DATA_FILE_MANIFEST_IMPLICIT_LAYER,
    LOADER_DATA_FILE_NUM_TYPES  // Not a real field, used for possible loop terminator
};

struct loader_icd_physical_devices {
    uint32_t device_count;
    VkPhysicalDevice *physical_devices;
    struct loader_icd_term *icd_term;
#if defined(WIN32)
    LUID windows_adapter_luid;
#endif
};

struct loader_msg_callback_map_entry {
    VkDebugReportCallbackEXT icd_obj;
    VkDebugReportCallbackEXT loader_obj;
};

typedef enum loader_filter_string_type {
    FILTER_STRING_FULLNAME = 0,
    FILTER_STRING_SUBSTRING,
    FILTER_STRING_PREFIX,
    FILTER_STRING_SUFFIX,
    FILTER_STRING_SPECIAL,
} loader_filter_string_type;

struct loader_envvar_filter_value {
    char value[VK_MAX_EXTENSION_NAME_SIZE];
    size_t length;
    loader_filter_string_type type;
};

#define MAX_ADDITIONAL_FILTERS 16
struct loader_envvar_filter {
    uint32_t count;
    struct loader_envvar_filter_value filters[MAX_ADDITIONAL_FILTERS];
};
struct loader_envvar_disable_layers_filter {
    struct loader_envvar_filter additional_filters;
    bool disable_all;
    bool disable_all_implicit;
    bool disable_all_explicit;
};

struct loader_envvar_all_filters {
    struct loader_envvar_filter enable_filter;
    struct loader_envvar_disable_layers_filter disable_filter;
    struct loader_envvar_filter allow_filter;
};
