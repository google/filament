// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BENCHMARKS_UTIL_HPP_
#define BENCHMARKS_UTIL_HPP_

#include "VulkanHeaders.hpp"
#include "glslang/Public/ShaderLang.h"

#include <vector>

namespace Util {

uint32_t getMemoryTypeIndex(vk::PhysicalDevice physicalDevice, uint32_t typeBits, vk::MemoryPropertyFlags properties = {});

vk::CommandBuffer beginSingleTimeCommands(vk::Device device, vk::CommandPool commandPool);

void endSingleTimeCommands(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, vk::CommandBuffer commandBuffer);

void transitionImageLayout(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

void copyBufferToImage(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

std::vector<uint32_t> compileGLSLtoSPIRV(const char *glslSource, EShLanguage glslLanguage);

}  // namespace Util

#endif  // BENCHMARKS_UTIL_HPP_
