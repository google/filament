/* Copyright (c) 2018-2024 The Khronos Group Inc.
 * Copyright (c) 2018-2024 Valve Corporation
 * Copyright (c) 2018-2024 LunarG, Inc.
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

// This file helps maps the layout of resources that we find in the instrumented shaders

#pragma once

#include <vector>

#include "gpuav/descriptor_validation/gpuav_descriptor_set.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"

namespace gpuav {

// "binding" here refers to "binding in the command buffer" and not the "binding in a descriptor set"
struct DescriptorCommandBinding {
    // This is where we hold the list of BDA address for a given bound descriptor snapshot.
    // The size of the SSBO doesn't change on an UpdateAfterBind so we can allocate it once and update its internals later
    vko::Buffer descritpor_state_ssbo_buffer;  // type DescriptorStateSSBO
    vko::Buffer post_process_ssbo_buffer;      // type PostProcessSSBO

    // Note: The index here is from vkCmdBindDescriptorSets::firstSet
    // for each "set" in vkCmdBindDescriptorSets::descriptorSetCount
    std::vector<std::shared_ptr<DescriptorSet>> bound_descriptor_sets;

    DescriptorCommandBinding(Validator &gpuav) : descritpor_state_ssbo_buffer(gpuav), post_process_ssbo_buffer(gpuav) {}
};

// This holds inforamtion for a given action command (draw/dispatch/trace rays)
// It needs the DescriptorCommandBinding, but to save memory, will just reference the last instance used at the time this is created
struct ActionCommandSnapshot {
    // This is a reference to the last DescriptorCommandBinding at a given action command time.
    // We use an int here because the list for DescriptorCommandBinding is a vector and reference/pointer will change on us.
    const uint32_t descriptor_command_binding_index;

    // This is information from the pipeline/shaderObject we want to save
    std::vector<const BindingVariableMap *> binding_req_maps;

    ActionCommandSnapshot(const uint32_t index) : descriptor_command_binding_index(index) {}
};

// These match the Structures found in the instrumentation GLSL logic
namespace glsl {

// Every descriptor set has various BDA pointers to data from the CPU
// Shared among all Descriptor Indexing GPU-AV checks (so we only have to create a single buffer)
struct DescriptorStateSSBO {
    // Used to know if descriptors are initialized or not
    VkDeviceAddress initialized_status;
    // The type information will change with UpdateAfterBind so will need to update this before submitting the to the queue
    VkDeviceAddress descriptor_set_types[kDebugInputBindlessMaxDescSets];
};

// Outputs
struct PostProcessSSBO {
    VkDeviceAddress descriptor_index_post_process_buffers[kDebugInputBindlessMaxDescSets];
};

// Represented as a uvec2 in the shader
// TODO - Currently duplicated as needed by various parts
// see interface.h for details
struct BindingLayout {
    uint32_t start;
    uint32_t count;
};

// Represented as a uvec2 in the shader
// For each descriptor index we have a "slot" to mark what happend on the GPU.
struct PostProcessDescriptorIndexSlot {
    // Since most devices can only support 32 descriptor sets (and we have checks for this assumption already), we try to compress
    // other access info into this 32-bits GLSL doesn't have bitfields to divide this
    uint32_t descriptor_set;
    // OpVariable ID of descriptor accessed.
    // This is required to distinguish between 2 aliased descriptors
    uint32_t variable_id;
};

// Represented as a uvec2 in the shader
struct DescriptorState {
    DescriptorState() : id(0), extra_data(0) {}
    DescriptorState(vvl::DescriptorClass dc, uint32_t id_, uint32_t extra_data_ = 1)
        : id(ClassToShaderBits(dc) | id_), extra_data(extra_data_) {}
    uint32_t id;
    uint32_t extra_data;

    static uint32_t ClassToShaderBits(vvl::DescriptorClass dc) {
        switch (dc) {
            case vvl::DescriptorClass::PlainSampler:
                return (kSamplerDesc << kDescBitShift);
            case vvl::DescriptorClass::ImageSampler:
                return (kImageSamplerDesc << kDescBitShift);
            case vvl::DescriptorClass::Image:
                return (kImageDesc << kDescBitShift);
            case vvl::DescriptorClass::TexelBuffer:
                return (kTexelDesc << kDescBitShift);
            case vvl::DescriptorClass::GeneralBuffer:
                return (kBufferDesc << kDescBitShift);
            case vvl::DescriptorClass::InlineUniform:
                return (kInlineUniformDesc << kDescBitShift);
            case vvl::DescriptorClass::AccelerationStructure:
                return (kAccelDesc << kDescBitShift);
            case vvl::DescriptorClass::Mutable:
            case vvl::DescriptorClass::Invalid:
                assert(false);
                break;
        }
        return 0;
    }
};

}  // namespace glsl

}  // namespace gpuav
