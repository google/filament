/*
 * Copyright (c) 2019-2021 The Khronos Group Inc.
 * Copyright (c) 2019-2021 Valve Corporation
 * Copyright (c) 2019-2021 LunarG, Inc.
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
 */

#include "allocation.h"

#include <stdlib.h>

// A debug option to disable allocators at compile time to investigate future issues.
#define DEBUG_DISABLE_APP_ALLOCATORS 0

void *loader_alloc(const VkAllocationCallbacks *pAllocator, size_t size, VkSystemAllocationScope allocation_scope) {
    void *pMemory = NULL;
#if (DEBUG_DISABLE_APP_ALLOCATORS == 1)
    {
#else
    if (pAllocator && pAllocator->pfnAllocation) {
        // These are internal structures, so it's best to align everything to
        // the largest unit size which is the size of a uint64_t.
        pMemory = pAllocator->pfnAllocation(pAllocator->pUserData, size, sizeof(uint64_t), allocation_scope);
    } else {
#endif
        pMemory = malloc(size);
    }

    return pMemory;
}

void *loader_calloc(const VkAllocationCallbacks *pAllocator, size_t size, VkSystemAllocationScope allocation_scope) {
    void *pMemory = NULL;
#if (DEBUG_DISABLE_APP_ALLOCATORS == 1)
    {
#else
    if (pAllocator && pAllocator->pfnAllocation) {
        // These are internal structures, so it's best to align everything to
        // the largest unit size which is the size of a uint64_t.
        pMemory = pAllocator->pfnAllocation(pAllocator->pUserData, size, sizeof(uint64_t), allocation_scope);
        if (pMemory) {
            memset(pMemory, 0, size);
        }
    } else {
#endif
        pMemory = calloc(1, size);
    }

    return pMemory;
}

void loader_free(const VkAllocationCallbacks *pAllocator, void *pMemory) {
    if (pMemory != NULL) {
#if (DEBUG_DISABLE_APP_ALLOCATORS == 1)
        {
#else
        if (pAllocator && pAllocator->pfnFree) {
            pAllocator->pfnFree(pAllocator->pUserData, pMemory);
        } else {
#endif
            free(pMemory);
        }
    }
}

void *loader_realloc(const VkAllocationCallbacks *pAllocator, void *pMemory, size_t orig_size, size_t size,
                     VkSystemAllocationScope allocation_scope) {
    void *pNewMem = NULL;
    if (pMemory == NULL || orig_size == 0) {
        pNewMem = loader_alloc(pAllocator, size, allocation_scope);
    } else if (size == 0) {
        loader_free(pAllocator, pMemory);
#if (DEBUG_DISABLE_APP_ALLOCATORS == 1)
#else
    } else if (pAllocator && pAllocator->pfnReallocation) {
        // These are internal structures, so it's best to align everything to
        // the largest unit size which is the size of a uint64_t.
        pNewMem = pAllocator->pfnReallocation(pAllocator->pUserData, pMemory, size, sizeof(uint64_t), allocation_scope);
#endif
    } else {
        pNewMem = realloc(pMemory, size);
        // Clear out the newly allocated memory
        if (size > orig_size) {
            memset((uint8_t *)pNewMem + orig_size, 0, size - orig_size);
        }
    }
    return pNewMem;
}

void *loader_instance_heap_alloc(const struct loader_instance *inst, size_t size, VkSystemAllocationScope allocation_scope) {
    return loader_alloc(inst ? &inst->alloc_callbacks : NULL, size, allocation_scope);
}

void *loader_instance_heap_calloc(const struct loader_instance *inst, size_t size, VkSystemAllocationScope allocation_scope) {
    return loader_calloc(inst ? &inst->alloc_callbacks : NULL, size, allocation_scope);
}

void loader_instance_heap_free(const struct loader_instance *inst, void *pMemory) {
    loader_free(inst ? &inst->alloc_callbacks : NULL, pMemory);
}
void *loader_instance_heap_realloc(const struct loader_instance *inst, void *pMemory, size_t orig_size, size_t size,
                                   VkSystemAllocationScope allocation_scope) {
    return loader_realloc(inst ? &inst->alloc_callbacks : NULL, pMemory, orig_size, size, allocation_scope);
}

void *loader_device_heap_alloc(const struct loader_device *dev, size_t size, VkSystemAllocationScope allocation_scope) {
    return loader_alloc(dev ? &dev->alloc_callbacks : NULL, size, allocation_scope);
}

void *loader_device_heap_calloc(const struct loader_device *dev, size_t size, VkSystemAllocationScope allocation_scope) {
    return loader_calloc(dev ? &dev->alloc_callbacks : NULL, size, allocation_scope);
}

void loader_device_heap_free(const struct loader_device *dev, void *pMemory) {
    loader_free(dev ? &dev->alloc_callbacks : NULL, pMemory);
}
void *loader_device_heap_realloc(const struct loader_device *dev, void *pMemory, size_t orig_size, size_t size,
                                 VkSystemAllocationScope allocation_scope) {
    return loader_realloc(dev ? &dev->alloc_callbacks : NULL, pMemory, orig_size, size, allocation_scope);
}

void *loader_alloc_with_instance_fallback(const VkAllocationCallbacks *pAllocator, const struct loader_instance *inst, size_t size,
                                          VkSystemAllocationScope allocation_scope) {
    return loader_alloc(NULL != pAllocator ? pAllocator : &inst->alloc_callbacks, size, allocation_scope);
}

void *loader_calloc_with_instance_fallback(const VkAllocationCallbacks *pAllocator, const struct loader_instance *instance,
                                           size_t size, VkSystemAllocationScope allocation_scope) {
    return loader_calloc(NULL != pAllocator ? pAllocator : &instance->alloc_callbacks, size, allocation_scope);
}

void loader_free_with_instance_fallback(const VkAllocationCallbacks *pAllocator, const struct loader_instance *instance,
                                        void *pMemory) {
    loader_free(NULL != pAllocator ? pAllocator : &instance->alloc_callbacks, pMemory);
}

void *loader_realloc_with_instance_fallback(const VkAllocationCallbacks *pAllocator, const struct loader_instance *instance,
                                            void *pMemory, size_t orig_size, size_t size,
                                            VkSystemAllocationScope allocation_scope) {
    return loader_realloc(NULL != pAllocator ? pAllocator : &instance->alloc_callbacks, pMemory, orig_size, size, allocation_scope);
}
