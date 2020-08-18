//
// Copyright (c) 2017-2020 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

extern const int RESULT_EXCEPTION;
extern const int RESULT_ERROR_COMMAND_LINE;
extern const int RESULT_ERROR_SOURCE_FILE;
extern const int RESULT_ERROR_FORMAT;
extern const int RESULT_ERROR_VULKAN;

enum CMD_LINE_OPT
{
    CMD_LINE_OPT_VERBOSITY,
    CMD_LINE_OPT_ITERATIONS,
    CMD_LINE_OPT_LINES,
    CMD_LINE_OPT_PHYSICAL_DEVICE,
    CMD_LINE_OPT_USER_DATA,
    CMD_LINE_OPT_VK_KHR_DEDICATED_ALLOCATION,
    CMD_LINE_OPT_VK_EXT_MEMORY_BUDGET,
    CMD_LINE_OPT_VK_LAYER_KHRONOS_VALIDATION,
    CMD_LINE_OPT_MEM_STATS,
    CMD_LINE_OPT_DUMP_STATS_AFTER_LINE,
    CMD_LINE_OPT_DEFRAGMENT_AFTER_LINE,
    CMD_LINE_OPT_DEFRAGMENTATION_FLAGS,
    CMD_LINE_OPT_DUMP_DETAILED_STATS_AFTER_LINE,
};

enum class VERBOSITY
{
    MINIMUM = 0,
    DEFAULT,
    MAXIMUM,
    COUNT,
};

enum class VULKAN_EXTENSION_REQUEST
{
    DISABLED,
    ENABLED,
    DEFAULT
};

enum class OBJECT_TYPE { BUFFER, IMAGE };

enum class VMA_FUNCTION
{
    CreatePool,
    DestroyPool,
    SetAllocationUserData,
    CreateBuffer,
    DestroyBuffer,
    CreateImage,
    DestroyImage,
    FreeMemory,
    FreeMemoryPages,
    CreateLostAllocation,
    AllocateMemory,
    AllocateMemoryPages,
    AllocateMemoryForBuffer,
    AllocateMemoryForImage,
    MapMemory,
    UnmapMemory,
    FlushAllocation,
    InvalidateAllocation,
    TouchAllocation,
    GetAllocationInfo,
    MakePoolAllocationsLost,
    ResizeAllocation,
    DefragmentationBegin,
    DefragmentationEnd,
    SetPoolName,
    Count
};
extern const char* VMA_FUNCTION_NAMES[];

extern const char* VMA_POOL_CREATE_FLAG_NAMES[];
extern const uint32_t VMA_POOL_CREATE_FLAG_VALUES[];
extern const size_t VMA_POOL_CREATE_FLAG_COUNT;

extern const char* VK_BUFFER_CREATE_FLAG_NAMES[];
extern const uint32_t VK_BUFFER_CREATE_FLAG_VALUES[];
extern const size_t VK_BUFFER_CREATE_FLAG_COUNT;

extern const char* VK_BUFFER_USAGE_FLAG_NAMES[];
extern const uint32_t VK_BUFFER_USAGE_FLAG_VALUES[];
extern const size_t VK_BUFFER_USAGE_FLAG_COUNT;

extern const char* VK_SHARING_MODE_NAMES[];
extern const size_t VK_SHARING_MODE_COUNT;

extern const char* VK_IMAGE_CREATE_FLAG_NAMES[];
extern const uint32_t VK_IMAGE_CREATE_FLAG_VALUES[];
extern const size_t VK_IMAGE_CREATE_FLAG_COUNT;

extern const char* VK_IMAGE_TYPE_NAMES[];
extern const size_t VK_IMAGE_TYPE_COUNT;

extern const char* VK_FORMAT_NAMES[];
extern const uint32_t VK_FORMAT_VALUES[];
extern const size_t VK_FORMAT_COUNT;

extern const char* VK_SAMPLE_COUNT_NAMES[];
extern const uint32_t VK_SAMPLE_COUNT_VALUES[];
extern const size_t VK_SAMPLE_COUNT_COUNT;

extern const char* VK_IMAGE_TILING_NAMES[];
extern const size_t VK_IMAGE_TILING_COUNT;

extern const char* VK_IMAGE_USAGE_FLAG_NAMES[];
extern const uint32_t VK_IMAGE_USAGE_FLAG_VALUES[];
extern const size_t VK_IMAGE_USAGE_FLAG_COUNT;

extern const char* VK_IMAGE_TILING_NAMES[];
extern const size_t VK_IMAGE_TILING_COUNT;

extern const char* VK_IMAGE_LAYOUT_NAMES[];
extern const uint32_t VK_IMAGE_LAYOUT_VALUES[];
extern const size_t VK_IMAGE_LAYOUT_COUNT;

extern const char* VMA_ALLOCATION_CREATE_FLAG_NAMES[];
extern const uint32_t VMA_ALLOCATION_CREATE_FLAG_VALUES[];
extern const size_t VMA_ALLOCATION_CREATE_FLAG_COUNT;

extern const char* VMA_MEMORY_USAGE_NAMES[];
extern const size_t VMA_MEMORY_USAGE_COUNT;

extern const char* VK_MEMORY_PROPERTY_FLAG_NAMES[];
extern const uint32_t VK_MEMORY_PROPERTY_FLAG_VALUES[];
extern const size_t VK_MEMORY_PROPERTY_FLAG_COUNT;
