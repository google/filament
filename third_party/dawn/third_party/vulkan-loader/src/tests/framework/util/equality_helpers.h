/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and/or associated documentation files (the "Materials"), to
 * deal in the Materials without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Materials, and to permit persons to whom the Materials are
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE
 * USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 */

#pragma once

#include <string.h>

#include <algorithm>
#include <vector>

#include <vulkan/vulkan.h>

inline bool string_eq(const char* a, const char* b) noexcept { return a && b && strcmp(a, b) == 0; }
inline bool string_eq(const char* a, const char* b, size_t len) noexcept { return a && b && strncmp(a, b, len) == 0; }

bool operator==(const VkExtent3D& a, const VkExtent3D& b);
bool operator!=(const VkExtent3D& a, const VkExtent3D& b);

bool operator==(const VkQueueFamilyProperties& a, const VkQueueFamilyProperties& b);
bool operator!=(const VkQueueFamilyProperties& a, const VkQueueFamilyProperties& b);

bool operator==(const VkQueueFamilyProperties& a, const VkQueueFamilyProperties2& b);
bool operator!=(const VkQueueFamilyProperties& a, const VkQueueFamilyProperties2& b);

bool operator==(const VkLayerProperties& a, const VkLayerProperties& b);
bool operator!=(const VkLayerProperties& a, const VkLayerProperties& b);

bool operator==(const VkExtensionProperties& a, const VkExtensionProperties& b);
bool operator!=(const VkExtensionProperties& a, const VkExtensionProperties& b);

bool operator==(const VkPhysicalDeviceFeatures& feats1, const VkPhysicalDeviceFeatures2& feats2);

bool operator==(const VkPhysicalDeviceMemoryProperties& props1, const VkPhysicalDeviceMemoryProperties2& props2);
bool operator==(const VkSparseImageFormatProperties& props1, const VkSparseImageFormatProperties& props2);
bool operator==(const VkSparseImageFormatProperties& props1, const VkSparseImageFormatProperties2& props2);
bool operator==(const VkExternalMemoryProperties& props1, const VkExternalMemoryProperties& props2);
bool operator==(const VkExternalSemaphoreProperties& props1, const VkExternalSemaphoreProperties& props2);
bool operator==(const VkExternalFenceProperties& props1, const VkExternalFenceProperties& props2);
bool operator==(const VkSurfaceCapabilitiesKHR& props1, const VkSurfaceCapabilitiesKHR& props2);
bool operator==(const VkSurfacePresentScalingCapabilitiesEXT& caps1, const VkSurfacePresentScalingCapabilitiesEXT& caps2);
bool operator==(const VkSurfaceFormatKHR& format1, const VkSurfaceFormatKHR& format2);
bool operator==(const VkSurfaceFormatKHR& format1, const VkSurfaceFormat2KHR& format2);
bool operator==(const VkDisplayPropertiesKHR& props1, const VkDisplayPropertiesKHR& props2);
bool operator==(const VkDisplayPropertiesKHR& props1, const VkDisplayProperties2KHR& props2);
bool operator==(const VkDisplayModePropertiesKHR& disp1, const VkDisplayModePropertiesKHR& disp2);

bool operator==(const VkDisplayModePropertiesKHR& disp1, const VkDisplayModeProperties2KHR& disp2);
bool operator==(const VkDisplayPlaneCapabilitiesKHR& caps1, const VkDisplayPlaneCapabilitiesKHR& caps2);

bool operator==(const VkDisplayPlaneCapabilitiesKHR& caps1, const VkDisplayPlaneCapabilities2KHR& caps2);
bool operator==(const VkDisplayPlanePropertiesKHR& props1, const VkDisplayPlanePropertiesKHR& props2);
bool operator==(const VkDisplayPlanePropertiesKHR& props1, const VkDisplayPlaneProperties2KHR& props2);
bool operator==(const VkExtent2D& ext1, const VkExtent2D& ext2);

// Allow comparison of vectors of different types as long as their elements are comparable (just has to make sure to only apply when
// T != U)
template <typename T, typename U, typename = std::enable_if_t<!std::is_same_v<T, U>>>
bool operator==(const std::vector<T>& a, const std::vector<U>& b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](const auto& left, const auto& right) { return left == right; });
}
