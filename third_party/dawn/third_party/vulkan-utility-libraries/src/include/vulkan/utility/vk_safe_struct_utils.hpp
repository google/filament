/***************************************************************************
 *
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#pragma once
#include <vulkan/vulkan.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <vector>

namespace vku {

// State that elements in a pNext chain may need to be aware of
struct PNextCopyState {
    // Custom initialization function. Returns true if the structure passed to init was initialized, false otherwise
    std::function<bool(VkBaseOutStructure* /* safe_sruct */, const VkBaseOutStructure* /* in_struct */)> init;
};

void* SafePnextCopy(const void* pNext, PNextCopyState* copy_state = {});
void FreePnextChain(const void* pNext);
char* SafeStringCopy(const char* in_string);

template <typename Base, typename T>
bool AddToPnext(Base& base, const T& data) {
    assert(base.ptr());  // All safe struct have a ptr() method. Prevent use with non-safe structs.
    auto** prev = reinterpret_cast<VkBaseOutStructure**>(const_cast<void**>(&base.pNext));
    auto* current = *prev;
    while (current) {
        if (data.sType == current->sType) {
            return false;
        }
        prev = reinterpret_cast<VkBaseOutStructure**>(&current->pNext);
        current = *prev;
    }
    *prev = reinterpret_cast<VkBaseOutStructure*>(SafePnextCopy(&data));
    return true;
}

template <typename Base>
bool RemoveFromPnext(Base& base, VkStructureType t) {
    assert(base.ptr());  // All safe struct have a ptr() method. Prevent use with non-safe structs.
    auto** prev = reinterpret_cast<VkBaseOutStructure**>(const_cast<void**>(&base.pNext));
    auto* current = *prev;
    while (current) {
        if (t == current->sType) {
            *prev = current->pNext;
            current->pNext = nullptr;
            FreePnextChain(current);
            return true;
        }
        prev = reinterpret_cast<VkBaseOutStructure**>(&current->pNext);
        current = *prev;
    }
    return false;
}

template <typename CreateInfo>
uint32_t FindExtension(CreateInfo& ci, const char* extension_name) {
    assert(ci.ptr());  // All safe struct have a ptr() method. Prevent use with non-safe structs.
    for (uint32_t i = 0; i < ci.enabledExtensionCount; i++) {
        if (strcmp(ci.ppEnabledExtensionNames[i], extension_name) == 0) {
            return i;
        }
    }
    return ci.enabledExtensionCount;
}

template <typename CreateInfo>
bool AddExtension(CreateInfo& ci, const char* extension_name) {
    assert(ci.ptr());  // All safe struct have a ptr() method. Prevent use with non-safe structs.
    uint32_t pos = FindExtension(ci, extension_name);
    if (pos < ci.enabledExtensionCount) {
        // already present
        return false;
    }
    char** exts = new char*[ci.enabledExtensionCount + 1];
    if (ci.ppEnabledExtensionNames) {
        memcpy(exts, ci.ppEnabledExtensionNames, sizeof(char*) * ci.enabledExtensionCount);
    }
    exts[ci.enabledExtensionCount] = SafeStringCopy(extension_name);
    delete[] ci.ppEnabledExtensionNames;
    ci.ppEnabledExtensionNames = exts;
    ci.enabledExtensionCount++;
    return true;
}

template <typename CreateInfo>
bool RemoveExtension(CreateInfo& ci, const char* extension_name) {
    assert(ci.ptr());  // All safe struct have a ptr() method. Prevent use with non-safe structs.
    uint32_t pos = FindExtension(ci, extension_name);
    if (pos >= ci.enabledExtensionCount) {
        // not present
        return false;
    }
    if (ci.enabledExtensionCount == 1) {
        delete[] ci.ppEnabledExtensionNames[0];
        delete[] ci.ppEnabledExtensionNames;
        ci.ppEnabledExtensionNames = nullptr;
        ci.enabledExtensionCount = 0;
        return true;
    }
    uint32_t out_pos = 0;
    char** exts = new char*[ci.enabledExtensionCount - 1];
    for (uint32_t i = 0; i < ci.enabledExtensionCount; i++) {
        if (i == pos) {
            delete[] ci.ppEnabledExtensionNames[i];
        } else {
            exts[out_pos++] = const_cast<char*>(ci.ppEnabledExtensionNames[i]);
        }
    }
    delete[] ci.ppEnabledExtensionNames;
    ci.ppEnabledExtensionNames = exts;
    ci.enabledExtensionCount--;
    return true;
}

}  // namespace vku
