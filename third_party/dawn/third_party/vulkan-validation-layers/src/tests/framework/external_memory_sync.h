/*
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include <generated/vk_function_pointers.h>
class ErrorMonitor;

void IgnoreHandleTypeError(ErrorMonitor *monitor);

VkExternalMemoryHandleTypeFlags GetCompatibleHandleTypes(VkPhysicalDevice gpu, const VkBufferCreateInfo &buffer_create_info,
                                                         VkExternalMemoryHandleTypeFlagBits handle_type);
VkExternalMemoryHandleTypeFlags GetCompatibleHandleTypes(VkPhysicalDevice gpu, const VkImageCreateInfo &image_create_info,
                                                         VkExternalMemoryHandleTypeFlagBits handle_type);
VkExternalFenceHandleTypeFlags GetCompatibleHandleTypes(VkPhysicalDevice gpu, VkExternalFenceHandleTypeFlagBits handle_type);
VkExternalSemaphoreHandleTypeFlags GetCompatibleHandleTypes(VkPhysicalDevice gpu,
                                                            VkExternalSemaphoreHandleTypeFlagBits handle_type);

VkExternalFenceHandleTypeFlags FindSupportedExternalFenceHandleTypes(VkPhysicalDevice gpu,
                                                                     VkExternalFenceFeatureFlags requested_features);
VkExternalSemaphoreHandleTypeFlags FindSupportedExternalSemaphoreHandleTypes(VkPhysicalDevice gpu,
                                                                             VkExternalSemaphoreFeatureFlags requested_features);

VkExternalMemoryHandleTypeFlags FindSupportedExternalMemoryHandleTypes(VkPhysicalDevice gpu,
                                                                       const VkBufferCreateInfo &buffer_create_info,
                                                                       VkExternalMemoryFeatureFlags requested_features);
VkExternalMemoryHandleTypeFlags FindSupportedExternalMemoryHandleTypes(VkPhysicalDevice gpu,
                                                                       const VkImageCreateInfo &image_create_info,
                                                                       VkExternalMemoryFeatureFlags requested_features);
VkExternalMemoryHandleTypeFlagsNV FindSupportedExternalMemoryHandleTypesNV(VkPhysicalDevice gpu,
                                                                           const VkImageCreateInfo &image_create_info,
                                                                           VkExternalMemoryFeatureFlagsNV requested_features);

bool HandleTypeNeedsDedicatedAllocation(VkPhysicalDevice gpu, const VkBufferCreateInfo &buffer_create_info,
                                        VkExternalMemoryHandleTypeFlagBits handle_type);
bool HandleTypeNeedsDedicatedAllocation(VkPhysicalDevice gpu, const VkImageCreateInfo &image_create_info,
                                        VkExternalMemoryHandleTypeFlagBits handle_type);

bool SemaphoreExportImportSupported(VkPhysicalDevice gpu, VkExternalSemaphoreHandleTypeFlagBits handle_type,
                                    void *p_next = nullptr);
