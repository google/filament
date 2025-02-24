/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
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

#include <vulkan/vulkan_core.h>

// These aim to follow VkDebugReportFlagBitsEXT but were created prior
// Could replace with VkDebugReportFlagBitsEXT, but would be a LOT of churn and these
// names are less verbose and desired.
enum LogMessageTypeBits {
    kInformationBit = 0x00000001,
    kWarningBit = 0x00000002,
    kPerformanceWarningBit = 0x00000004,
    kErrorBit = 0x00000008,
    kVerboseBit = 0x00000010,
};
using LogMessageTypeFlags = VkFlags;