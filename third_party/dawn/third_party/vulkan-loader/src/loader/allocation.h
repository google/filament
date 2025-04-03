/*
 *
 * Copyright (c) 2014-2021 The Khronos Group Inc.
 * Copyright (c) 2014-2021 Valve Corporation
 * Copyright (c) 2014-2021 LunarG, Inc.
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
 */

#pragma once

#include "loader_common.h"

// The loader always aligns memory allocations to the largest unit size which is the size of a uint64_t
#define loader_aligned_size(x) ((((x) + sizeof(uint64_t) - 1) / sizeof(uint64_t)) * sizeof(uint64_t))

void *loader_instance_heap_alloc(const struct loader_instance *instance, size_t size, VkSystemAllocationScope allocation_scope);
void *loader_instance_heap_calloc(const struct loader_instance *instance, size_t size, VkSystemAllocationScope allocation_scope);
void loader_instance_heap_free(const struct loader_instance *instance, void *pMemory);
void *loader_instance_heap_realloc(const struct loader_instance *instance, void *pMemory, size_t orig_size, size_t size,
                                   VkSystemAllocationScope allocation_scope);

void *loader_device_heap_alloc(const struct loader_device *device, size_t size, VkSystemAllocationScope allocationScope);
void *loader_device_heap_calloc(const struct loader_device *device, size_t size, VkSystemAllocationScope allocationScope);
void loader_device_heap_free(const struct loader_device *device, void *pMemory);
void *loader_device_heap_realloc(const struct loader_device *device, void *pMemory, size_t orig_size, size_t size,
                                 VkSystemAllocationScope alloc_scope);

// Wrappers around various memory functions. The loader will use the VkAllocationCallbacks functions if pAllocator is not NULL,
// otherwise use the system functions
void *loader_alloc(const VkAllocationCallbacks *pAllocator, size_t size, VkSystemAllocationScope allocation_scope);
void *loader_calloc(const VkAllocationCallbacks *pAllocator, size_t size, VkSystemAllocationScope allocation_scope);
void loader_free(const VkAllocationCallbacks *pAllocator, void *pMemory);
void *loader_realloc(const VkAllocationCallbacks *pAllocator, void *pMemory, size_t orig_size, size_t size,
                     VkSystemAllocationScope allocation_scope);

// helper allocation functions that will use pAllocator and if it is NULL, fallback to the allocator of instance
void *loader_alloc_with_instance_fallback(const VkAllocationCallbacks *pAllocator, const struct loader_instance *instance,
                                          size_t size, VkSystemAllocationScope allocation_scope);
void *loader_calloc_with_instance_fallback(const VkAllocationCallbacks *pAllocator, const struct loader_instance *instance,
                                           size_t size, VkSystemAllocationScope allocation_scope);
void loader_free_with_instance_fallback(const VkAllocationCallbacks *pAllocator, const struct loader_instance *instance,
                                        void *pMemory);
void *loader_realloc_with_instance_fallback(const VkAllocationCallbacks *pAllocator, const struct loader_instance *instance,
                                            void *pMemory, size_t orig_size, size_t size, VkSystemAllocationScope allocation_scope);
