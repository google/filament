// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#include "vulkan.h"

// THIS FILE SHOULD BE DELETED IF VK_EXT_provoking_vertex IS EVER ADDED TO THE VULKAN HEADERS
#ifdef VK_EXT_provoking_vertex
#error "VK_EXT_provoking_vertex is already defined in the Vulkan headers, you can delete this file"
#endif

static constexpr VkStructureType VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT = static_cast<VkStructureType>(1000254000);
static constexpr VkStructureType VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT = static_cast<VkStructureType>(1000254001);
static constexpr VkStructureType VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_PROPERTIES_EXT = static_cast<VkStructureType>(1000254002);

#define VK_EXT_provoking_vertex 1
#define VK_EXT_PROVOKING_VERTEX_SPEC_VERSION 1
#define VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME "VK_EXT_provoking_vertex"

typedef enum VkProvokingVertexModeEXT {
    VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT = 0,
    VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT = 1,
    VK_PROVOKING_VERTEX_MODE_BEGIN_RANGE_EXT = VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT,
    VK_PROVOKING_VERTEX_MODE_END_RANGE_EXT = VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT,
    VK_PROVOKING_VERTEX_MODE_RANGE_SIZE_EXT = (VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT - VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT + 1),
    VK_PROVOKING_VERTEX_MODE_MAX_ENUM_EXT = 0x7FFFFFFF
} VkProvokingVertexModeEXT;

typedef struct VkPhysicalDeviceProvokingVertexFeaturesEXT {
    VkStructureType    sType;
    void*              pNext;
    VkBool32           provokingVertexLast;
} VkPhysicalDeviceProvokingVertexFeaturesEXT;

typedef struct VkPhysicalDeviceProvokingVertexPropertiesEXT {
    VkStructureType    sType;
    void*              pNext;
    VkBool32           provokingVertexModePerPipeline;
} VkPhysicalDeviceProvokingVertexPropertiesEXT;

typedef struct VkPipelineRasterizationProvokingVertexStateCreateInfoEXT {
    VkStructureType             sType;
    const void*                 pNext;
    VkProvokingVertexModeEXT    provokingVertexMode;
} VkPipelineRasterizationProvokingVertexStateCreateInfoEXT;

