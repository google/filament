/*
 * Copyright (c) 2022 The Khronos Group Inc.
 * Copyright (c) 2022 Valve Corporation
 * Copyright (c) 2022 LunarG, Inc.
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
 * Author: Mark Young <marky@lunarg.com>
 * Author: Lenny Komow <lenny@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 */

#include "unknown_function_handling.h"

// If the assembly code necessary for unknown functions isn't supported, then replace all of the functions with stubs.
// This way, if an application queries for an unknown function, they receive NULL and can act accordingly.
// Previously, there was a fallback path written in C. However, it depended on the compiler optimizing the functions
// in such a way as to not disturb the callstack. This reliance on implementation defined behavior is unsustainable and was only
// known to work with GCC.
#if !defined(UNKNOWN_FUNCTIONS_SUPPORTED)

void loader_init_dispatch_dev_ext(struct loader_instance *inst, struct loader_device *dev) {
    (void)inst;
    (void)dev;
}
void *loader_dev_ext_gpa_tramp(struct loader_instance *inst, const char *funcName) {
    (void)inst;
    (void)funcName;
    return NULL;
}
void *loader_dev_ext_gpa_term(struct loader_instance *inst, const char *funcName) {
    (void)inst;
    (void)funcName;
    return NULL;
}

void *loader_phys_dev_ext_gpa_tramp(struct loader_instance *inst, const char *funcName) {
    (void)inst;
    (void)funcName;
    return NULL;
}
void *loader_phys_dev_ext_gpa_term(struct loader_instance *inst, const char *funcName) {
    (void)inst;
    (void)funcName;
    return NULL;
}

void loader_free_dev_ext_table(struct loader_instance *inst) { (void)inst; }
void loader_free_phys_dev_ext_table(struct loader_instance *inst) { (void)inst; }

#else

#include "allocation.h"
#include "log.h"

// Forward declarations
void *loader_get_dev_ext_trampoline(uint32_t index);
void *loader_get_phys_dev_ext_tramp(uint32_t index);
void *loader_get_phys_dev_ext_termin(uint32_t index);

// Device function handling

// Initialize device_ext dispatch table entry as follows:
// If dev == NULL find all logical devices created within this instance and
//  init the entry (given by idx) in the ext dispatch table.
// If dev != NULL only initialize the entry in the given dev's dispatch table.
// The initialization value is gotten by calling down the device chain with
// GDPA.
// If GDPA returns NULL then don't initialize the dispatch table entry.
void loader_init_dispatch_dev_ext_entry(struct loader_instance *inst, struct loader_device *dev, uint32_t idx, const char *funcName)

{
    void *gdpa_value;
    if (dev != NULL) {
        gdpa_value = dev->loader_dispatch.core_dispatch.GetDeviceProcAddr(dev->chain_device, funcName);
        if (gdpa_value != NULL) dev->loader_dispatch.ext_dispatch[idx] = (PFN_vkDevExt)gdpa_value;
    } else {
        for (struct loader_icd_term *icd_term = inst->icd_terms; icd_term != NULL; icd_term = icd_term->next) {
            struct loader_device *ldev = icd_term->logical_device_list;
            while (ldev) {
                gdpa_value = ldev->loader_dispatch.core_dispatch.GetDeviceProcAddr(ldev->chain_device, funcName);
                if (gdpa_value != NULL) ldev->loader_dispatch.ext_dispatch[idx] = (PFN_vkDevExt)gdpa_value;
                ldev = ldev->next;
            }
        }
    }
}

// Find all dev extension in the function names array  and initialize the dispatch table
// for dev  for each of those extension entrypoints found in function names array.
void loader_init_dispatch_dev_ext(struct loader_instance *inst, struct loader_device *dev) {
    for (uint32_t i = 0; i < MAX_NUM_UNKNOWN_EXTS; i++) {
        if (inst->dev_ext_disp_functions[i] != NULL)
            loader_init_dispatch_dev_ext_entry(inst, dev, i, inst->dev_ext_disp_functions[i]);
    }
}

bool loader_check_icds_for_dev_ext_address(struct loader_instance *inst, const char *funcName) {
    struct loader_icd_term *icd_term;
    icd_term = inst->icd_terms;
    while (NULL != icd_term) {
        if (icd_term->scanned_icd->GetInstanceProcAddr(icd_term->instance, funcName))
            // this icd supports funcName
            return true;
        icd_term = icd_term->next;
    }

    return false;
}

// Look in the layers list of device extensions, which contain names of entry points. If funcName is present, return true
// If not, call down the first layer's vkGetInstanceProcAddr to determine if any layers support the function
bool loader_check_layer_list_for_dev_ext_address(struct loader_instance *inst, const char *funcName) {
    // Iterate over the layers.
    for (uint32_t layer = 0; layer < inst->expanded_activated_layer_list.count; ++layer) {
        // Iterate over the extensions.
        const struct loader_device_extension_list *const extensions =
            &(inst->expanded_activated_layer_list.list[layer]->device_extension_list);
        for (uint32_t extension = 0; extension < extensions->count; ++extension) {
            // Iterate over the entry points.
            const struct loader_dev_ext_props *const property = &(extensions->list[extension]);
            for (uint32_t entry = 0; entry < property->entrypoints.count; ++entry) {
                if (strcmp(property->entrypoints.list[entry], funcName) == 0) {
                    return true;
                }
            }
        }
    }
    // If the function pointer doesn't appear in the layer manifest for intercepted device functions, look down the
    // vkGetInstanceProcAddr chain
    if (inst->expanded_activated_layer_list.count > 0) {
        const struct loader_layer_functions *const functions = &(inst->expanded_activated_layer_list.list[0]->functions);
        if (NULL != functions->get_instance_proc_addr) {
            return NULL != functions->get_instance_proc_addr((VkInstance)inst->instance, funcName);
        }
    }

    return false;
}

void loader_free_dev_ext_table(struct loader_instance *inst) {
    for (uint32_t i = 0; i < inst->dev_ext_disp_function_count; i++) {
        loader_instance_heap_free(inst, inst->dev_ext_disp_functions[i]);
    }
    memset(inst->dev_ext_disp_functions, 0, sizeof(inst->dev_ext_disp_functions));
}

/*
 * This function returns generic trampoline code address for unknown entry points.
 * Presumably, these unknown entry points (as given by funcName) are device extension
 * entrypoints.
 * A function name array is used to keep a list of unknown entry points and their
 * mapping to the device extension dispatch table.
 * \returns
 * For a given entry point string (funcName), if an existing mapping is found the
 * trampoline address for that mapping is returned.
 * Otherwise, this unknown entry point has not been seen yet.
 * Next check if an ICD supports it, and if is_tramp is true, check if any layer
 * supports it by calling down the chain.
 * If so then a new entry in the function name array is added and that trampoline
 * address for the new entry is returned.
 * NULL is returned if the function name array is full or if no discovered layer or
 * ICD returns a non-NULL GetProcAddr for it.
 */
void *loader_dev_ext_gpa_impl(struct loader_instance *inst, const char *funcName, bool is_tramp) {
    // Linearly look through already added functions to make sure we haven't seen it before
    // if we have, return the function at the index found
    for (uint32_t i = 0; i < inst->dev_ext_disp_function_count; i++) {
        if (inst->dev_ext_disp_functions[i] && !strcmp(inst->dev_ext_disp_functions[i], funcName))
            return loader_get_dev_ext_trampoline(i);
    }

    // Check if funcName is supported in either ICDs or a layer library
    if (!loader_check_icds_for_dev_ext_address(inst, funcName)) {
        if (!is_tramp || !loader_check_layer_list_for_dev_ext_address(inst, funcName)) {
            // if support found in layers continue on
            return NULL;
        }
    }
    if (inst->dev_ext_disp_function_count >= MAX_NUM_UNKNOWN_EXTS) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0, "loader_dev_ext_gpa: Exhausted the unknown device function array!");
        return NULL;
    }

    // add found function to dev_ext_disp_functions;
    size_t funcName_len = strlen(funcName) + 1;
    inst->dev_ext_disp_functions[inst->dev_ext_disp_function_count] =
        (char *)loader_instance_heap_alloc(inst, funcName_len, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    if (NULL == inst->dev_ext_disp_functions[inst->dev_ext_disp_function_count]) {
        // failed to allocate memory, return NULL
        return NULL;
    }
    loader_strncpy(inst->dev_ext_disp_functions[inst->dev_ext_disp_function_count], funcName_len, funcName, funcName_len);
    // init any dev dispatch table entries as needed
    loader_init_dispatch_dev_ext_entry(inst, NULL, inst->dev_ext_disp_function_count, funcName);
    void *out_function = loader_get_dev_ext_trampoline(inst->dev_ext_disp_function_count);
    inst->dev_ext_disp_function_count++;
    return out_function;
}

void *loader_dev_ext_gpa_tramp(struct loader_instance *inst, const char *funcName) {
    return loader_dev_ext_gpa_impl(inst, funcName, true);
}

void *loader_dev_ext_gpa_term(struct loader_instance *inst, const char *funcName) {
    return loader_dev_ext_gpa_impl(inst, funcName, false);
}

// Physical Device function handling

bool loader_check_icds_for_phys_dev_ext_address(struct loader_instance *inst, const char *funcName) {
    struct loader_icd_term *icd_term;
    icd_term = inst->icd_terms;
    while (NULL != icd_term) {
        if (icd_term->scanned_icd->interface_version >= MIN_PHYS_DEV_EXTENSION_ICD_INTERFACE_VERSION &&
            icd_term->scanned_icd->GetPhysicalDeviceProcAddr &&
            icd_term->scanned_icd->GetPhysicalDeviceProcAddr(icd_term->instance, funcName))
            // this icd supports funcName
            return true;
        icd_term = icd_term->next;
    }

    return false;
}

bool loader_check_layer_list_for_phys_dev_ext_address(struct loader_instance *inst, const char *funcName) {
    for (uint32_t layer = 0; layer < inst->expanded_activated_layer_list.count; layer++) {
        struct loader_layer_properties *layer_prop_list = inst->expanded_activated_layer_list.list[layer];
        // Find the first layer in the call chain which supports vk_layerGetPhysicalDeviceProcAddr
        // and call that, returning whether it found a valid pointer for this function name.
        // We return if the topmost layer supports GPDPA since the layer should call down the chain for us.
        if (layer_prop_list->interface_version > 1) {
            const struct loader_layer_functions *const functions = &(layer_prop_list->functions);
            if (NULL != functions->get_physical_device_proc_addr) {
                return NULL != functions->get_physical_device_proc_addr((VkInstance)inst->instance, funcName);
            }
        }
    }
    return false;
}

void loader_free_phys_dev_ext_table(struct loader_instance *inst) {
    for (uint32_t i = 0; i < MAX_NUM_UNKNOWN_EXTS; i++) {
        loader_instance_heap_free(inst, inst->phys_dev_ext_disp_functions[i]);
    }
    memset(inst->phys_dev_ext_disp_functions, 0, sizeof(inst->phys_dev_ext_disp_functions));
}

// This function returns a generic trampoline or terminator function
// address for any unknown physical device extension commands.  An array
// is used to keep a list of unknown entry points and their
// mapping to the physical device extension dispatch table (struct
// loader_phys_dev_ext_dispatch_table).
// For a given entry point string (funcName), if an existing mapping is
// found, then the address for that mapping is returned. The is_tramp
// parameter is used to decide whether to return a trampoline or terminator
// If it has not been seen before check if a layer or and ICD supports it.
// If so then a new entry in the function name array is added.
// Null is returned if discovered layer or ICD returns a non-NULL GetProcAddr for it
// or if the function name table is full.
void *loader_phys_dev_ext_gpa_impl(struct loader_instance *inst, const char *funcName, bool is_tramp) {
    assert(NULL != inst);

    // We should always check to see if any ICD supports it.
    if (!loader_check_icds_for_phys_dev_ext_address(inst, funcName)) {
        // If we're not checking layers, or we are and it's not in a layer, just
        // return
        if (!is_tramp || !loader_check_layer_list_for_phys_dev_ext_address(inst, funcName)) {
            return NULL;
        }
    }

    bool has_found = false;
    uint32_t new_function_index = 0;
    // Linearly look through already added functions to make sure we haven't seen it before
    // if we have, return the function at the index found
    for (uint32_t i = 0; i < inst->phys_dev_ext_disp_function_count; i++) {
        if (inst->phys_dev_ext_disp_functions[i] && !strcmp(inst->phys_dev_ext_disp_functions[i], funcName)) {
            has_found = true;
            new_function_index = i;
            break;
        }
    }

    // A never before seen function name, store it in the array
    if (!has_found) {
        if (inst->phys_dev_ext_disp_function_count >= MAX_NUM_UNKNOWN_EXTS) {
            loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0,
                       "loader_dev_ext_gpa: Exhausted the unknown physical device function array!");
            return NULL;
        }

        loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0,
                   "loader_phys_dev_ext_gpa: Adding unknown physical function %s to internal store at index %u", funcName,
                   inst->phys_dev_ext_disp_function_count);

        // add found function to phys_dev_ext_disp_functions;
        size_t funcName_len = strlen(funcName) + 1;
        inst->phys_dev_ext_disp_functions[inst->phys_dev_ext_disp_function_count] =
            (char *)loader_instance_heap_alloc(inst, funcName_len, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
        if (NULL == inst->phys_dev_ext_disp_functions[inst->phys_dev_ext_disp_function_count]) {
            // failed to allocate memory, return NULL
            return NULL;
        }
        loader_strncpy(inst->phys_dev_ext_disp_functions[inst->phys_dev_ext_disp_function_count], funcName_len, funcName,
                       funcName_len);

        new_function_index = inst->phys_dev_ext_disp_function_count;
        // increment the count so that the subsequent logic includes the newly added entry point when searching for functions
        inst->phys_dev_ext_disp_function_count++;
    }

    // Setup the ICD function pointers
    struct loader_icd_term *icd_term = inst->icd_terms;
    while (NULL != icd_term) {
        if (MIN_PHYS_DEV_EXTENSION_ICD_INTERFACE_VERSION <= icd_term->scanned_icd->interface_version &&
            NULL != icd_term->scanned_icd->GetPhysicalDeviceProcAddr) {
            icd_term->phys_dev_ext[new_function_index] =
                (PFN_PhysDevExt)icd_term->scanned_icd->GetPhysicalDeviceProcAddr(icd_term->instance, funcName);
            if (NULL != icd_term->phys_dev_ext[new_function_index]) {
                // Make sure we set the instance dispatch to point to the loader's terminator now since we can at least handle
                // it in one ICD.
                inst->disp->phys_dev_ext[new_function_index] = loader_get_phys_dev_ext_termin(new_function_index);

                loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0, "loader_phys_dev_ext_gpa: Driver %s returned ptr %p for %s",
                           icd_term->scanned_icd->lib_name, inst->disp->phys_dev_ext[new_function_index], funcName);
            }
        } else {
            icd_term->phys_dev_ext[new_function_index] = NULL;
        }

        icd_term = icd_term->next;
    }

    // Now if this is being run in the trampoline, search for the first layer attached and query using it to get the first entry
    // point. Only set the instance dispatch table to it if it isn't NULL.
    if (is_tramp) {
        for (uint32_t i = 0; i < inst->expanded_activated_layer_list.count; i++) {
            struct loader_layer_properties *layer_prop = inst->expanded_activated_layer_list.list[i];
            if (layer_prop->interface_version > 1 && NULL != layer_prop->functions.get_physical_device_proc_addr) {
                void *layer_ret_function =
                    (PFN_PhysDevExt)layer_prop->functions.get_physical_device_proc_addr(inst->instance, funcName);
                if (NULL != layer_ret_function) {
                    inst->disp->phys_dev_ext[new_function_index] = layer_ret_function;
                    loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0, "loader_phys_dev_ext_gpa: Layer %s returned ptr %p for %s",
                               layer_prop->info.layerName, inst->disp->phys_dev_ext[new_function_index], funcName);
                    break;
                }
            }
        }
    }

    if (is_tramp) {
        return loader_get_phys_dev_ext_tramp(new_function_index);
    } else {
        return loader_get_phys_dev_ext_termin(new_function_index);
    }
}
// Main interface functions, makes it clear whether it is getting a terminator or trampoline
void *loader_phys_dev_ext_gpa_tramp(struct loader_instance *inst, const char *funcName) {
    return loader_phys_dev_ext_gpa_impl(inst, funcName, true);
}
void *loader_phys_dev_ext_gpa_term(struct loader_instance *inst, const char *funcName) {
    return loader_phys_dev_ext_gpa_impl(inst, funcName, false);
}

#endif
