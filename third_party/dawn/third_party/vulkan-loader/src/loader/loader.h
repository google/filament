/*
 *
 * Copyright (c) 2014-2023 The Khronos Group Inc.
 * Copyright (c) 2014-2023 Valve Corporation
 * Copyright (c) 2014-2023 LunarG, Inc.
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

#include "loader_common.h"
#include "cJSON.h"

// Declare the once_init variable
LOADER_PLATFORM_THREAD_ONCE_EXTERN_DEFINITION(once_init)

static inline VkPhysicalDevice loader_unwrap_physical_device(VkPhysicalDevice physicalDevice) {
    struct loader_physical_device_tramp *phys_dev = (struct loader_physical_device_tramp *)physicalDevice;
    if (PHYS_TRAMP_MAGIC_NUMBER != phys_dev->magic) {
        return VK_NULL_HANDLE;
    }
    return phys_dev->phys_dev;
}

static inline void loader_set_dispatch(void *obj, const void *data) { *((const void **)obj) = data; }

static inline VkLayerDispatchTable *loader_get_dispatch(const void *obj) {
    if (VK_NULL_HANDLE == obj) {
        return NULL;
    }
    VkLayerDispatchTable *disp = *((VkLayerDispatchTable **)obj);
    if (VK_NULL_HANDLE == disp || DEVICE_DISP_TABLE_MAGIC_NUMBER != disp->magic) {
        return NULL;
    }
    return disp;
}

static inline struct loader_dev_dispatch_table *loader_get_dev_dispatch(const void *obj) {
    return *((struct loader_dev_dispatch_table **)obj);
}

static inline VkLayerInstanceDispatchTable *loader_get_instance_layer_dispatch(const void *obj) {
    return *((VkLayerInstanceDispatchTable **)obj);
}

static inline struct loader_instance_dispatch_table *loader_get_instance_dispatch(const void *obj) {
    return *((struct loader_instance_dispatch_table **)obj);
}

static inline void loader_init_dispatch(void *obj, const void *data) {
#if defined(DEBUG)
    assert(valid_loader_magic_value(obj) &&
           "Incompatible ICD, first dword must be initialized to "
           "ICD_LOADER_MAGIC. See loader/README.md for details.");
#endif

    loader_set_dispatch(obj, data);
}

// Global variables used across files
extern struct loader_struct loader;
extern loader_platform_thread_mutex loader_lock;
extern loader_platform_thread_mutex loader_preload_icd_lock;
extern loader_platform_thread_mutex loader_global_instance_list_lock;

bool compare_vk_extension_properties(const VkExtensionProperties *op1, const VkExtensionProperties *op2);

VkResult loader_validate_layers(const struct loader_instance *inst, const uint32_t layer_count,
                                const char *const *ppEnabledLayerNames, const struct loader_layer_list *list);

VkResult loader_validate_instance_extensions(struct loader_instance *inst, const struct loader_extension_list *icd_exts,
                                             const struct loader_layer_list *instance_layer,
                                             const struct loader_envvar_all_filters *layer_filters,
                                             const VkInstanceCreateInfo *pCreateInfo);

#if defined(_WIN32)
BOOL __stdcall loader_initialize(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *Context);
#else
void loader_initialize(void);
#endif
void loader_release(void);
void loader_preload_icds(void);
void loader_unload_preloaded_icds(void);
VkResult loader_init_library_list(struct loader_layer_list *instance_layers, loader_platform_dl_handle **libs);

// Allocate a new string able to hold source_str and place it in dest_str
VkResult loader_copy_to_new_str(const struct loader_instance *inst, const char *source_str, char **dest_str);

// Allocate a loader_string_list with enough space for allocated_count strings inside of it
VkResult create_string_list(const struct loader_instance *inst, uint32_t allocated_count, struct loader_string_list *string_list);
// Resize if there isn't enough space, then add the string str to the end of the loader_string_list
// This function takes ownership of the str passed in - but only when it succeeds
VkResult append_str_to_string_list(const struct loader_instance *inst, struct loader_string_list *string_list, char *str);
// Resize if there isn't enough space, then copy the string str to a new string the end of the loader_string_list
// This function does not take ownership of the string, it merely copies it.
// This function appends a null terminator to the string automatically
// The str_len parameter does not include the null terminator
VkResult copy_str_to_string_list(const struct loader_instance *inst, struct loader_string_list *string_list, const char *str,
                                 size_t str_len);

// Free any string inside of loader_string_list and then free the list itself
void free_string_list(const struct loader_instance *inst, struct loader_string_list *string_list);

VkResult loader_init_generic_list(const struct loader_instance *inst, struct loader_generic_list *list_info, size_t element_size);
VkResult loader_resize_generic_list(const struct loader_instance *inst, struct loader_generic_list *list_info);
VkResult loader_get_next_available_entry(const struct loader_instance *inst, struct loader_used_object_list *list_info,
                                         uint32_t *free_index, const VkAllocationCallbacks *pAllocator);
void loader_release_object_from_list(struct loader_used_object_list *list_info, uint32_t index_to_free);
bool has_vk_extension_property_array(const VkExtensionProperties *vk_ext_prop, const uint32_t count,
                                     const VkExtensionProperties *ext_array);
bool has_vk_extension_property(const VkExtensionProperties *vk_ext_prop, const struct loader_extension_list *ext_list);
// This function takes ownership of layer_property in the case that allocation fails
VkResult loader_append_layer_property(const struct loader_instance *inst, struct loader_layer_list *layer_list,
                                      struct loader_layer_properties *layer_property);
VkResult loader_add_layer_properties(const struct loader_instance *inst, struct loader_layer_list *layer_instance_list, cJSON *json,
                                     bool is_implicit, char *filename);
bool loader_find_layer_name_in_list(const char *name, const struct loader_pointer_layer_list *layer_list);
VkResult loader_add_layer_properties_to_list(const struct loader_instance *inst, struct loader_pointer_layer_list *list,
                                             struct loader_layer_properties *props);
void loader_free_layer_properties(const struct loader_instance *inst, struct loader_layer_properties *layer_properties);
bool loader_implicit_layer_is_enabled(const struct loader_instance *inst, const struct loader_envvar_all_filters *filters,
                                      const struct loader_layer_properties *prop);
VkResult loader_add_meta_layer(const struct loader_instance *inst, const struct loader_envvar_all_filters *filters,
                               struct loader_layer_properties *prop, struct loader_pointer_layer_list *target_list,
                               struct loader_pointer_layer_list *expanded_target_list, const struct loader_layer_list *source_list,
                               bool *out_found_all_component_layers);
VkResult loader_add_to_ext_list(const struct loader_instance *inst, struct loader_extension_list *ext_list,
                                uint32_t prop_list_count, const VkExtensionProperties *props);
VkResult loader_add_device_extensions(const struct loader_instance *inst,
                                      PFN_vkEnumerateDeviceExtensionProperties fpEnumerateDeviceExtensionProperties,
                                      VkPhysicalDevice physical_device, const char *lib_name,
                                      struct loader_extension_list *ext_list);
VkResult loader_init_generic_list(const struct loader_instance *inst, struct loader_generic_list *list_info, size_t element_size);
void loader_destroy_generic_list(const struct loader_instance *inst, struct loader_generic_list *list);
void loader_destroy_pointer_layer_list(const struct loader_instance *inst, struct loader_pointer_layer_list *layer_list);
TEST_FUNCTION_EXPORT void loader_delete_layer_list_and_properties(const struct loader_instance *inst,
                                                                  struct loader_layer_list *layer_list);
void loader_remove_layer_in_list(const struct loader_instance *inst, struct loader_layer_list *layer_list,
                                 uint32_t layer_to_remove);
VkResult loader_init_scanned_icd_list(const struct loader_instance *inst, struct loader_icd_tramp_list *icd_tramp_list);
void loader_clear_scanned_icd_list(const struct loader_instance *inst, struct loader_icd_tramp_list *icd_tramp_list);
VkResult loader_icd_scan(const struct loader_instance *inst, struct loader_icd_tramp_list *icd_tramp_list,
                         const VkInstanceCreateInfo *pCreateInfo, bool *skipped_portability_drivers);
void loader_icd_close_objects(struct loader_instance *ptr_inst, struct loader_icd_term *icd_term);
void loader_icd_destroy(struct loader_instance *ptr_inst, struct loader_icd_term *icd_term,
                        const VkAllocationCallbacks *pAllocator);
VkResult loader_scan_for_layers(struct loader_instance *inst, struct loader_layer_list *instance_layers,
                                const struct loader_envvar_all_filters *layer_filters);
VkResult loader_scan_for_implicit_layers(struct loader_instance *inst, struct loader_layer_list *instance_layers,
                                         const struct loader_envvar_all_filters *layer_filters);
VkResult loader_get_icd_loader_instance_extensions(const struct loader_instance *inst, struct loader_icd_tramp_list *icd_tramp_list,
                                                   struct loader_extension_list *inst_exts);
struct loader_icd_term *loader_get_icd_and_device(const void *device, struct loader_device **found_dev);
struct loader_instance *loader_get_instance(const VkInstance instance);
loader_platform_dl_handle loader_open_layer_file(const struct loader_instance *inst, struct loader_layer_properties *prop);
struct loader_device *loader_create_logical_device(const struct loader_instance *inst, const VkAllocationCallbacks *pAllocator);
void loader_add_logical_device(struct loader_icd_term *icd_term, struct loader_device *found_dev);
void loader_remove_logical_device(struct loader_icd_term *icd_term, struct loader_device *found_dev,
                                  const VkAllocationCallbacks *pAllocator);
// NOTE: Outside of loader, this entry-point is only provided for error
// cleanup.
void loader_destroy_logical_device(struct loader_device *dev, const VkAllocationCallbacks *pAllocator);

VkResult loader_enable_instance_layers(struct loader_instance *inst, const VkInstanceCreateInfo *pCreateInfo,
                                       const struct loader_layer_list *instance_layers,
                                       const struct loader_envvar_all_filters *layer_filters);

VkResult loader_create_instance_chain(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                      struct loader_instance *inst, VkInstance *created_instance);

void loader_activate_instance_layer_extensions(struct loader_instance *inst, VkInstance created_inst);

VKAPI_ATTR VkResult VKAPI_CALL loader_layer_create_device(VkInstance instance, VkPhysicalDevice physicalDevice,
                                                          const VkDeviceCreateInfo *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                                          PFN_vkGetInstanceProcAddr layerGIPA, PFN_vkGetDeviceProcAddr *nextGDPA);
VKAPI_ATTR void VKAPI_CALL loader_layer_destroy_device(VkDevice device, const VkAllocationCallbacks *pAllocator,
                                                       PFN_vkDestroyDevice destroyFunction);

VkResult loader_create_device_chain(const VkPhysicalDevice pd, const VkDeviceCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, const struct loader_instance *inst,
                                    struct loader_device *dev, PFN_vkGetInstanceProcAddr callingLayer,
                                    PFN_vkGetDeviceProcAddr *layerNextGDPA);

VkResult loader_validate_device_extensions(struct loader_instance *this_instance,
                                           const struct loader_pointer_layer_list *activated_device_layers,
                                           const struct loader_extension_list *icd_exts, const VkDeviceCreateInfo *pCreateInfo);

VkResult setup_loader_tramp_phys_devs(struct loader_instance *inst, uint32_t phys_dev_count, VkPhysicalDevice *phys_devs);
VkResult setup_loader_tramp_phys_dev_groups(struct loader_instance *inst, uint32_t group_count,
                                            VkPhysicalDeviceGroupProperties *groups);
void unload_drivers_without_physical_devices(struct loader_instance *inst);

VkStringErrorFlags vk_string_validate(const int max_length, const char *char_array);
char *loader_get_next_path(char *path);
VkResult add_data_files(const struct loader_instance *inst, char *search_path, struct loader_string_list *out_files,
                        bool use_first_found_manifest);

loader_api_version loader_make_version(uint32_t version);
loader_api_version loader_combine_version(uint32_t major, uint32_t minor, uint32_t patch);

// Helper macros for determining if a version is valid or not
bool loader_check_version_meets_required(loader_api_version required, loader_api_version version);

// Convenience macros for common versions
#if !defined(LOADER_VERSION_1_0_0)
#define LOADER_VERSION_1_0_0 loader_combine_version(1, 0, 0)
#endif

#if !defined(LOADER_VERSION_1_1_0)
#define LOADER_VERSION_1_1_0 loader_combine_version(1, 1, 0)
#endif
