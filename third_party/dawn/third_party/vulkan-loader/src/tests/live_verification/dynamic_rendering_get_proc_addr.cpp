/*
 * Copyright (c) 2022 The Khronos Group Inc.
 * Copyright (c) 2022 Valve Corporation
 * Copyright (c) 2022 LunarG, Inc.
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
 * Author: Charles Giessen <charles@lunarg.com>
 */

#include "test_environment.h"

/*
 * Tests to see if drivers return vkCmdBeginRenderingKHR as an unknown device or physical device extension
 * If it is returned as a physical device extension, this test should crash. Otherwise it should work just fine.
 */

int main() {
    VulkanFunctions vk_funcs{};

    InstWrapper inst(vk_funcs);
    inst.CheckCreate();
    auto phys_devs = inst.GetPhysDevs();

    for (const auto& phys_dev : phys_devs) {
        uint32_t count = 0;
        VkResult res = vk_funcs.vkEnumerateDeviceExtensionProperties(phys_dev, nullptr, &count, nullptr);
        if (res != VK_SUCCESS) {
            std::cout << "Failed to query device extension count\n";
        }
        std::vector<VkExtensionProperties> extensions{count};
        res = vk_funcs.vkEnumerateDeviceExtensionProperties(phys_dev, nullptr, &count, extensions.data());
        if (res != VK_SUCCESS) {
            std::cout << "Failed to query device extensions\n";
        }

        bool has_dynamic_rendering = false;
        for (const auto& ext : extensions) {
            if (string_eq("VK_KHR_dynamic_rendering", ext.extensionName)) {
                has_dynamic_rendering = true;
                break;
            }
        }
        if (has_dynamic_rendering) {
            DeviceWrapper dev(inst);
            dev.create_info.add_extension("VK_KHR_dynamic_rendering");
            dev.CheckCreate(phys_dev);

            DeviceFunctions funcs{vk_funcs, dev};
            VkCommandPool command_pool;
            VkCommandPoolCreateInfo pool_create_info{};
            pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            funcs.vkCreateCommandPool(dev, &pool_create_info, nullptr, &command_pool);
            VkCommandBuffer command_buffer;
            VkCommandBufferAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc_info.commandBufferCount = 1;
            alloc_info.commandPool = command_pool;
            funcs.vkAllocateCommandBuffers(dev, &alloc_info, &command_buffer);
            PFN_vkBeginCommandBuffer vkBeginCommandBuffer =
                reinterpret_cast<PFN_vkBeginCommandBuffer>(vk_funcs.vkGetInstanceProcAddr(inst.inst, "vkBeginCommandBuffer"));
            VkCommandBufferBeginInfo begin_info{};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            res = vkBeginCommandBuffer(command_buffer, &begin_info);
            if (res != VK_SUCCESS) {
                std::cout << "Failed to begin command buffer\n";
            }

            // call the dynamic rendering function -- should not go into the physical device function trampoline.
            PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR =
                reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(vk_funcs.vkGetInstanceProcAddr(inst.inst, "vkCmdBeginRenderingKHR"));
            VkRenderingInfoKHR rendering_info{};
            rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
            vkCmdBeginRenderingKHR(command_buffer, &rendering_info);
        }
    }
}
