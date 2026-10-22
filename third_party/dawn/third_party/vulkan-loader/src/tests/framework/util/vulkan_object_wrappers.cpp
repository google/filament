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

#include "vulkan_object_wrappers.h"

#include "platform_wsi.h"

VkExtensionProperties Extension::get() const noexcept {
    VkExtensionProperties props{};
    extensionName.copy(props.extensionName, VK_MAX_EXTENSION_NAME_SIZE);
    props.specVersion = specVersion;
    return props;
}

InstanceCreateInfo::InstanceCreateInfo() {
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
}

VkInstanceCreateInfo* InstanceCreateInfo::get() noexcept {
    if (fill_in_application_info) {
        application_info.pApplicationName = app_name.c_str();
        application_info.pEngineName = engine_name.c_str();
        application_info.applicationVersion = app_version;
        application_info.engineVersion = engine_version;
        application_info.apiVersion = api_version;
        instance_info.pApplicationInfo = &application_info;
    }
    instance_info.flags = flags;
    instance_info.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
    instance_info.ppEnabledLayerNames = enabled_layers.data();
    instance_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
    instance_info.ppEnabledExtensionNames = enabled_extensions.data();
    return &instance_info;
}
InstanceCreateInfo& InstanceCreateInfo::set_api_version(uint32_t major, uint32_t minor, uint32_t patch) {
    this->api_version = VK_MAKE_API_VERSION(0, major, minor, patch);
    return *this;
}
InstanceCreateInfo& InstanceCreateInfo::setup_WSI(const char* api_selection) {
    add_extensions({"VK_KHR_surface", get_platform_wsi_extension(api_selection)});
    return *this;
}

DeviceQueueCreateInfo::DeviceQueueCreateInfo() { queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; }
DeviceQueueCreateInfo::DeviceQueueCreateInfo(const VkDeviceQueueCreateInfo* create_info) {
    queue_create_info = *create_info;
    for (uint32_t i = 0; i < create_info->queueCount; i++) {
        priorities.push_back(create_info->pQueuePriorities[i]);
    }
}

VkDeviceQueueCreateInfo DeviceQueueCreateInfo::get() noexcept {
    queue_create_info.pQueuePriorities = priorities.data();
    queue_create_info.queueCount = 1;
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    return queue_create_info;
}

DeviceCreateInfo::DeviceCreateInfo(const VkDeviceCreateInfo* create_info) {
    dev = *create_info;
    for (uint32_t i = 0; i < create_info->enabledExtensionCount; i++) {
        enabled_extensions.push_back(create_info->ppEnabledExtensionNames[i]);
    }
    for (uint32_t i = 0; i < create_info->enabledLayerCount; i++) {
        enabled_layers.push_back(create_info->ppEnabledLayerNames[i]);
    }
    for (uint32_t i = 0; i < create_info->queueCreateInfoCount; i++) {
        device_queue_infos.push_back(create_info->pQueueCreateInfos[i]);
    }
}

VkDeviceCreateInfo* DeviceCreateInfo::get() noexcept {
    dev.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
    dev.ppEnabledLayerNames = enabled_layers.data();
    dev.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
    dev.ppEnabledExtensionNames = enabled_extensions.data();
    uint32_t index = 0;
    for (auto& queue : queue_info_details) {
        queue.queue_create_info.queueFamilyIndex = index++;
        queue.queue_create_info.queueCount = 1;
        device_queue_infos.push_back(queue.get());
    }

    dev.queueCreateInfoCount = static_cast<uint32_t>(device_queue_infos.size());
    dev.pQueueCreateInfos = device_queue_infos.data();
    return &dev;
}
