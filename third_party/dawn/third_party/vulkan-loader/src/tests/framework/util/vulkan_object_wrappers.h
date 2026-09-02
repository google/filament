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

#include <string>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "builder_defines.h"

struct Extension {
    BUILDER_VALUE(std::string, extensionName)
    BUILDER_VALUE_WITH_DEFAULT(uint32_t, specVersion, VK_API_VERSION_1_0)

    Extension(const char* name, uint32_t specVersion = VK_API_VERSION_1_0) noexcept
        : extensionName(name), specVersion(specVersion) {}
    Extension(std::string extensionName, uint32_t specVersion = VK_API_VERSION_1_0) noexcept
        : extensionName(extensionName), specVersion(specVersion) {}

    VkExtensionProperties get() const noexcept;
};

struct MockQueueFamilyProperties {
    BUILDER_VALUE(VkQueueFamilyProperties, properties)
    BUILDER_VALUE(bool, support_present)

    VkQueueFamilyProperties get() const noexcept { return properties; }
};

struct InstanceCreateInfo {
    BUILDER_VALUE(VkInstanceCreateInfo, instance_info)
    BUILDER_VALUE(VkApplicationInfo, application_info)
    BUILDER_VALUE(std::string, app_name)
    BUILDER_VALUE(std::string, engine_name)
    BUILDER_VALUE(uint32_t, flags)
    BUILDER_VALUE(uint32_t, app_version)
    BUILDER_VALUE(uint32_t, engine_version)
    BUILDER_VALUE_WITH_DEFAULT(uint32_t, api_version, VK_API_VERSION_1_0)
    BUILDER_VECTOR(const char*, enabled_layers, layer)
    BUILDER_VECTOR(const char*, enabled_extensions, extension)
    // tell the get() function to not provide `application_info`
    BUILDER_VALUE_WITH_DEFAULT(bool, fill_in_application_info, true)

    InstanceCreateInfo();

    VkInstanceCreateInfo* get() noexcept;

    InstanceCreateInfo& set_api_version(uint32_t major, uint32_t minor, uint32_t patch);

    InstanceCreateInfo& setup_WSI(const char* api_selection = nullptr);
};

struct DeviceQueueCreateInfo {
    DeviceQueueCreateInfo();
    DeviceQueueCreateInfo(const VkDeviceQueueCreateInfo* create_info);

    BUILDER_VALUE(VkDeviceQueueCreateInfo, queue_create_info)
    BUILDER_VECTOR(float, priorities, priority)

    VkDeviceQueueCreateInfo get() noexcept;
};

struct DeviceCreateInfo {
    DeviceCreateInfo() = default;
    DeviceCreateInfo(const VkDeviceCreateInfo* create_info);

    BUILDER_VALUE(VkDeviceCreateInfo, dev)
    BUILDER_VECTOR(const char*, enabled_extensions, extension)
    BUILDER_VECTOR(const char*, enabled_layers, layer)
    BUILDER_VECTOR(DeviceQueueCreateInfo, queue_info_details, device_queue)

    VkDeviceCreateInfo* get() noexcept;

   private:
    std::vector<VkDeviceQueueCreateInfo> device_queue_infos;
};
