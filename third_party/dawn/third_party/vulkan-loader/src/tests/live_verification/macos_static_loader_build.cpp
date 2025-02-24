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

#include <thread>
#include <chrono>
#include <vector>
#include <atomic>

#include "vulkan/vulkan.h"

static std::atomic_bool is_running;

void run_vk_code(int which_func) {
    while (!is_running) {
        // busy wait so all threads have a chance to spawn
    }
    while (is_running) {
        switch (which_func) {
            default:
            case 0: {
                VkInstance inst;
                VkInstanceCreateInfo info{};
                vkCreateInstance(&info, nullptr, &inst);
                break;
            }
            case 1: {
                uint32_t count = 0;
                vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
                std::vector<VkExtensionProperties> props(count, VkExtensionProperties{});
                vkEnumerateInstanceExtensionProperties(nullptr, &count, props.data());
                break;
            }
            case 2: {
                uint32_t count = 0;
                vkEnumerateInstanceLayerProperties(&count, nullptr);
                std::vector<VkLayerProperties> layers(count, VkLayerProperties{});
                vkEnumerateInstanceLayerProperties(&count, layers.data());

                break;
            }
            case 3: {
                uint32_t version = 0;
                vkEnumerateInstanceVersion(&version);
                break;
            }
        }
    }
}

int main() {
    uint32_t thread_count = 4;
    is_running = false;
    std::vector<std::thread> threads;
    for (uint32_t i = 0; i < thread_count; i++) {
        threads.emplace_back(run_vk_code, i % 4);
    }
    is_running = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    is_running = false;
    for (auto& t : threads) {
        t.join();
    }
    return 0;
}