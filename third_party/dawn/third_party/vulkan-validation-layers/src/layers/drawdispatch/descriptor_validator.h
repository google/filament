/* Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
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
#include <vulkan/vulkan.h>
#include "error_message/error_location.h"

namespace spirv {
struct ResourceInterfaceVariable;
}  // namespace spirv

namespace vvl {
struct DrawDispatchVuid;
class DescriptorBinding;
class Device;
class BufferDescriptor;
class ImageDescriptor;
class ImageSamplerDescriptor;
class TexelDescriptor;
class AccelerationStructureDescriptor;
class SamplerDescriptor;
class CommandBuffer;
class Sampler;
class DescriptorSet;

class DescriptorValidator {
 public:
   DescriptorValidator(Device& dev, vvl::CommandBuffer& cb_state, vvl::DescriptorSet& descriptor_set, uint32_t set_index,
                       VkFramebuffer framebuffer, const Location& loc);

   // Used with normal validation where we know which descriptors are accessed.
   bool ValidateBindingStatic(const spirv::ResourceInterfaceVariable& binding_info, const vvl::DescriptorBinding& binding) const;
   // Used with GPU-AV when we need to run the GPU to know which descriptors are accessed.
   // The main reason we can't combine is one function needs to be const and the other is non-const.
   bool ValidateBindingDynamic(const spirv::ResourceInterfaceVariable& binding_info, DescriptorBinding& binding,
                               const uint32_t index);

 private:
   template <typename T>
   bool ValidateDescriptorsStatic(const spirv::ResourceInterfaceVariable& binding_info, const T& binding) const;

   template <typename T>
   bool ValidateDescriptorsDynamic(const spirv::ResourceInterfaceVariable& binding_info, const T& binding, const uint32_t index);

   bool ValidateDescriptor(const spirv::ResourceInterfaceVariable& binding_info, const uint32_t index,
                           VkDescriptorType descriptor_type, const vvl::BufferDescriptor& descriptor) const;
   bool ValidateDescriptor(const spirv::ResourceInterfaceVariable& binding_info, const uint32_t index,
                           VkDescriptorType descriptor_type, const vvl::ImageDescriptor& descriptor) const;
   bool ValidateDescriptor(const spirv::ResourceInterfaceVariable& binding_info, const uint32_t index,
                           VkDescriptorType descriptor_type, const vvl::ImageSamplerDescriptor& descriptor) const;
   bool ValidateDescriptor(const spirv::ResourceInterfaceVariable& binding_info, const uint32_t index,
                           VkDescriptorType descriptor_type, const vvl::TexelDescriptor& descriptor) const;
   bool ValidateDescriptor(const spirv::ResourceInterfaceVariable& binding_info, const uint32_t index,
                           VkDescriptorType descriptor_type, const vvl::AccelerationStructureDescriptor& descriptor) const;
   bool ValidateDescriptor(const spirv::ResourceInterfaceVariable& binding_info, const uint32_t index,
                           VkDescriptorType descriptor_type, const vvl::SamplerDescriptor& descriptor) const;

   // helper for the common parts of ImageSamplerDescriptor and SamplerDescriptor validation
   bool ValidateSamplerDescriptor(const spirv::ResourceInterfaceVariable& binding_info, uint32_t index, VkSampler sampler,
                                  bool is_immutable, const vvl::Sampler* sampler_state) const;

   std::string DescribeDescriptor(const spirv::ResourceInterfaceVariable& binding_info, uint32_t index) const;

   vvl::Device& dev_state;
   vvl::CommandBuffer& cb_state;
   vvl::DescriptorSet& descriptor_set;
   const uint32_t set_index;
   const VkFramebuffer framebuffer;
   const Location& loc;
   const DrawDispatchVuid& vuids;

};
}  // namespace vvl
