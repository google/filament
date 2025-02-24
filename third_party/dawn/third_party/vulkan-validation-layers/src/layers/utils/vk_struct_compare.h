/* Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
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
 */
#pragma once

// These compare are normally used for VUs that say "must be identical to the <struct>".
//
// Using memcmp will fail sometimes as the alignment/padding in the struct can have undefined values,
// so because we can't ensure the memory was zero-intialized, we need to just do a normal compare on each member.
//
// While this could be generated, each pNext added needs to be added with caution as some structs don't make sense.

#include "vulkan/vulkan_core.h"

bool ComparePipelineMultisampleStateCreateInfo(const VkPipelineMultisampleStateCreateInfo &a,
                                               const VkPipelineMultisampleStateCreateInfo &b);

bool CompareDescriptorSetLayoutBinding(const VkDescriptorSetLayoutBinding &a, const VkDescriptorSetLayoutBinding &b);

bool ComparePipelineColorBlendAttachmentState(const VkPipelineColorBlendAttachmentState &a,
                                              const VkPipelineColorBlendAttachmentState &b);

bool ComparePipelineFragmentShadingRateStateCreateInfo(const VkPipelineFragmentShadingRateStateCreateInfoKHR &a,
                                                       const VkPipelineFragmentShadingRateStateCreateInfoKHR &b);

bool CompareSamplerCreateInfo(const VkSamplerCreateInfo &a, const VkSamplerCreateInfo &b);
