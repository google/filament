/*
 * Copyright (c) 2021 The Khronos Group Inc.
 * Copyright (c) 2021 Valve Corporation
 * Copyright (c) 2021 LunarG, Inc.
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

#include <thread>

void create_destroy_instance_loop_with_function_queries(FrameworkEnvironment* env, uint32_t num_loops_create_destroy_instance,
                                                        uint32_t num_loops_try_get_instance_proc_addr,
                                                        uint32_t num_loops_try_get_device_proc_addr) {
    for (uint32_t i = 0; i < num_loops_create_destroy_instance; i++) {
        InstWrapper inst{env->vulkan_functions};
        inst.CheckCreate();
        PFN_vkEnumeratePhysicalDevices enum_pd = nullptr;
        for (uint32_t j = 0; j < num_loops_try_get_instance_proc_addr; j++) {
            enum_pd = inst.load("vkEnumeratePhysicalDevices");
            ASSERT_NE(enum_pd, nullptr);
        }
        VkPhysicalDevice phys_dev = inst.GetPhysDev();

        DeviceWrapper dev{inst};
        dev.CheckCreate(phys_dev);
        for (uint32_t j = 0; j < num_loops_try_get_device_proc_addr; j++) {
            PFN_vkCmdBindPipeline p = dev.load("vkCmdBindPipeline");
            p(VK_NULL_HANDLE, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, VK_NULL_HANDLE);
        }
    }
}

void create_destroy_device_loop(FrameworkEnvironment* env, uint32_t num_loops_create_destroy_device,
                                uint32_t num_loops_try_get_proc_addr) {
    InstWrapper inst{env->vulkan_functions};
    inst.CheckCreate();
    for (uint32_t i = 0; i < num_loops_create_destroy_device; i++) {
        DeviceWrapper dev{inst};
        dev.CheckCreate(inst.GetPhysDev());

        for (uint32_t j = 0; j < num_loops_try_get_proc_addr; j++) {
            PFN_vkCmdBindPipeline p = dev.load("vkCmdBindPipeline");
            PFN_vkCmdBindDescriptorSets d = dev.load("vkCmdBindDescriptorSets");
            PFN_vkCmdBindVertexBuffers vb = dev.load("vkCmdBindVertexBuffers");
            PFN_vkCmdBindIndexBuffer ib = dev.load("vkCmdBindIndexBuffer");
            PFN_vkCmdDraw c = dev.load("vkCmdDraw");
            p(VK_NULL_HANDLE, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, VK_NULL_HANDLE);
            d(VK_NULL_HANDLE, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, VK_NULL_HANDLE, 0, 0, nullptr, 0, nullptr);
            vb(VK_NULL_HANDLE, 0, 0, nullptr, nullptr);
            ib(VK_NULL_HANDLE, 0, 0, VkIndexType::VK_INDEX_TYPE_UINT16);
            c(VK_NULL_HANDLE, 0, 0, 0, 0);
        }
    }
}
VKAPI_ATTR void VKAPI_CALL test_vkCmdBindPipeline(VkCommandBuffer, VkPipelineBindPoint, VkPipeline) {}
VKAPI_ATTR void VKAPI_CALL test_vkCmdBindDescriptorSets(VkCommandBuffer, VkPipelineBindPoint, VkPipelineLayout, uint32_t, uint32_t,
                                                        const VkDescriptorSet*, uint32_t, const uint32_t*) {}
VKAPI_ATTR void VKAPI_CALL test_vkCmdBindVertexBuffers(VkCommandBuffer, uint32_t, uint32_t, const VkBuffer*, const VkDeviceSize*) {}
VKAPI_ATTR void VKAPI_CALL test_vkCmdBindIndexBuffer(VkCommandBuffer, uint32_t, uint32_t, const VkBuffer*, const VkDeviceSize*) {}
VKAPI_ATTR void VKAPI_CALL test_vkCmdDraw(VkCommandBuffer, uint32_t, uint32_t, uint32_t, uint32_t) {}
TEST(Threading, InstanceCreateDestroyLoop) {
    const auto processor_count = std::thread::hardware_concurrency();

    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("")};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    uint32_t num_loops_create_destroy_instance = 500;
    uint32_t num_loops_try_get_instance_proc_addr = 5;
    uint32_t num_loops_try_get_device_proc_addr = 100;

    driver.physical_devices.emplace_back("physical_device_0")
        .known_device_functions.push_back({"vkCmdBindPipeline", to_vkVoidFunction(test_vkCmdBindPipeline)});

    std::vector<std::thread> instance_creation_threads;
    std::vector<std::thread> function_query_threads;
    for (uint32_t i = 0; i < processor_count; i++) {
        instance_creation_threads.emplace_back(create_destroy_instance_loop_with_function_queries, &env,
                                               num_loops_create_destroy_instance, num_loops_try_get_instance_proc_addr,
                                               num_loops_try_get_device_proc_addr);
    }
    for (uint32_t i = 0; i < processor_count; i++) {
        instance_creation_threads[i].join();
    }
}

TEST(Threading, DeviceCreateDestroyLoop) {
    const auto processor_count = std::thread::hardware_concurrency();

    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("")};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));

    uint32_t num_loops_create_destroy_device = 1000;
    uint32_t num_loops_try_get_device_proc_addr = 5;

    driver.physical_devices.emplace_back("physical_device_0").known_device_functions = {
        {"vkCmdBindPipeline", to_vkVoidFunction(test_vkCmdBindPipeline)},
        {"vkCmdBindDescriptorSets", to_vkVoidFunction(test_vkCmdBindDescriptorSets)},
        {"vkCmdBindVertexBuffers", to_vkVoidFunction(test_vkCmdBindVertexBuffers)},
        {"vkCmdBindIndexBuffer", to_vkVoidFunction(test_vkCmdBindIndexBuffer)},
        {"vkCmdDraw", to_vkVoidFunction(test_vkCmdDraw)}};

    std::vector<std::thread> device_creation_threads;

    for (uint32_t i = 0; i < processor_count; i++) {
        device_creation_threads.emplace_back(create_destroy_device_loop, &env, num_loops_create_destroy_device,
                                             num_loops_try_get_device_proc_addr);
    }
    for (uint32_t i = 0; i < processor_count; i++) {
        device_creation_threads[i].join();
    }
}
