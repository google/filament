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

#include "gpuav/descriptor_validation/gpuav_descriptor_set.h"

#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "state_tracker/descriptor_sets.h"

using vvl::DescriptorClass;

namespace gpuav {

// Returns the number of bytes to hold 32 bit aligned array of bits.
static uint32_t BitBufferSize(uint32_t num_bits) {
    static constexpr uint32_t kBitsPerWord = 32;
    return (((num_bits + (kBitsPerWord - 1)) & ~(kBitsPerWord - 1)) / kBitsPerWord) * sizeof(uint32_t);
}

DescriptorSet::DescriptorSet(const VkDescriptorSet handle, vvl::DescriptorPool *pool,
                             const std::shared_ptr<vvl::DescriptorSetLayout const> &layout, uint32_t variable_count,
                             vvl::Device *state_data)
    : vvl::DescriptorSet(handle, pool, layout, variable_count, state_data),
      post_process_buffer_(*static_cast<Validator *>(state_data)),
      input_buffer_(*static_cast<Validator *>(state_data)) {
    BuildBindingLayouts();
}

DescriptorSet::~DescriptorSet() {
    post_process_buffer_.Destroy();
    input_buffer_.Destroy();
}

void DescriptorSet::BuildBindingLayouts() {
    const uint32_t binding_count = (GetBindingCount() > 0) ? GetLayout()->GetMaxBinding() + 1 : 0;

    binding_layouts_.resize(binding_count);
    uint32_t start = 0;
    for (const BindingPtr &binding : bindings_) {
        if (binding->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
            binding_layouts_[binding->binding] = {start, 1};
            start++;
        } else {
            binding_layouts_[binding->binding] = {start, binding->count};
            start += binding->count;
        }
    }
}

static glsl::DescriptorState GetInData(const vvl::BufferDescriptor &desc) {
    auto buffer_state = static_cast<const Buffer *>(desc.GetBufferState());
    if (!buffer_state) {
        return glsl::DescriptorState(DescriptorClass::GeneralBuffer, glsl::kNullDescriptor, vvl::kU32Max);
    }
    return glsl::DescriptorState(DescriptorClass::GeneralBuffer, buffer_state->id, static_cast<uint32_t>(desc.GetEffectiveRange()));
}

static glsl::DescriptorState GetInData(const vvl::TexelDescriptor &desc) {
    auto buffer_view_state = static_cast<const BufferView *>(desc.GetBufferViewState());
    if (!buffer_view_state) {
        return glsl::DescriptorState(DescriptorClass::TexelBuffer, glsl::kNullDescriptor, vvl::kU32Max);
    }
    auto view_size = buffer_view_state->Size();
    uint32_t res_size = static_cast<uint32_t>(view_size / GetTexelBufferFormatSize(buffer_view_state->create_info.format));
    return glsl::DescriptorState(DescriptorClass::TexelBuffer, buffer_view_state->id, res_size);
}

static glsl::DescriptorState GetInData(const vvl::ImageDescriptor &desc) {
    auto image_state = static_cast<const ImageView *>(desc.GetImageViewState());
    return glsl::DescriptorState(DescriptorClass::Image, image_state ? image_state->id : glsl::kNullDescriptor);
}

static glsl::DescriptorState GetInData(const vvl::SamplerDescriptor &desc) {
    auto sampler_state = static_cast<const Sampler *>(desc.GetSamplerState());
    return glsl::DescriptorState(DescriptorClass::PlainSampler, sampler_state->id);
}

static glsl::DescriptorState GetInData(const vvl::ImageSamplerDescriptor &desc) {
    auto image_state = static_cast<const ImageView *>(desc.GetImageViewState());
    auto sampler_state = static_cast<const Sampler *>(desc.GetSamplerState());
    return glsl::DescriptorState(DescriptorClass::ImageSampler, image_state ? image_state->id : glsl::kNullDescriptor,
                                 sampler_state ? sampler_state->id : 0);
}

static glsl::DescriptorState GetInData(const vvl::AccelerationStructureDescriptor &ac) {
    uint32_t id;
    if (ac.IsKHR()) {
        auto ac_state = static_cast<const AccelerationStructureKHR *>(ac.GetAccelerationStructureStateKHR());
        id = ac_state ? ac_state->id : glsl::kNullDescriptor;
    } else {
        auto ac_state = static_cast<const AccelerationStructureNV *>(ac.GetAccelerationStructureStateNV());
        id = ac_state ? ac_state->id : glsl::kNullDescriptor;
    }
    return glsl::DescriptorState(DescriptorClass::AccelerationStructure, id);
}

static glsl::DescriptorState GetInData(const vvl::MutableDescriptor &desc) {
    auto desc_class = desc.ActiveClass();
    switch (desc_class) {
        case DescriptorClass::GeneralBuffer: {
            auto buffer_state = std::static_pointer_cast<const Buffer>(desc.GetSharedBufferState());
            if (!buffer_state) {
                return glsl::DescriptorState(desc_class, glsl::kNullDescriptor, vvl::kU32Max);
            }
            return glsl::DescriptorState(desc_class, buffer_state->id, static_cast<uint32_t>(buffer_state->create_info.size));
        }
        case DescriptorClass::TexelBuffer: {
            auto buffer_view_state = std::static_pointer_cast<const BufferView>(desc.GetSharedBufferViewState());
            if (!buffer_view_state) {
                return glsl::DescriptorState(desc_class, glsl::kNullDescriptor, vvl::kU32Max);
            }
            auto view_size = buffer_view_state->Size();
            uint32_t res_size = static_cast<uint32_t>(view_size / GetTexelBufferFormatSize(buffer_view_state->create_info.format));
            return glsl::DescriptorState(desc_class, buffer_view_state->id, res_size);
        }
        case DescriptorClass::PlainSampler: {
            auto sampler_state = std::static_pointer_cast<const Sampler>(desc.GetSharedSamplerState());
            return glsl::DescriptorState(desc_class, sampler_state->id);
        }
        case DescriptorClass::ImageSampler: {
            auto image_state = std::static_pointer_cast<const ImageView>(desc.GetSharedImageViewState());
            auto sampler_state = std::static_pointer_cast<const Sampler>(desc.GetSharedSamplerState());
            // image can be null in some cases, but the sampler can't
            return glsl::DescriptorState(desc_class, image_state ? image_state->id : glsl::kNullDescriptor,
                                         sampler_state ? sampler_state->id : 0);
        }
        case DescriptorClass::Image: {
            auto image_state = std::static_pointer_cast<const ImageView>(desc.GetSharedImageViewState());
            return glsl::DescriptorState(desc_class, image_state ? image_state->id : glsl::kNullDescriptor);
        }
        case DescriptorClass::AccelerationStructure: {
            uint32_t id;
            if (desc.IsAccelerationStructureKHR()) {
                auto ac_state = static_cast<const AccelerationStructureKHR *>(desc.GetAccelerationStructureStateKHR());
                id = ac_state ? ac_state->id : glsl::kNullDescriptor;
            } else {
                auto ac_state = static_cast<const AccelerationStructureNV *>(desc.GetAccelerationStructureStateNV());
                id = ac_state ? ac_state->id : glsl::kNullDescriptor;
            }
            return glsl::DescriptorState(desc_class, id);
        }
        case DescriptorClass::InlineUniform:
        case DescriptorClass::Mutable:
        case DescriptorClass::Invalid:
            assert(false);
            break;
    }
    // If unsupported descriptor, act as if it is null and skip
    return glsl::DescriptorState(desc_class, glsl::kNullDescriptor, vvl::kU32Max);
}

template <typename Binding>
void FillBindingInData(const Binding &binding, glsl::DescriptorState *data, uint32_t &index) {
    for (uint32_t di = 0; di < binding.count; di++) {
        if (!binding.updated[di]) {
            data[index++] = glsl::DescriptorState();
        } else {
            data[index++] = GetInData(binding.descriptors[di]);
        }
    }
}

// Inline Uniforms are currently treated as a single descriptor. Writes to any offsets cause the whole range to be valid.
template <>
void FillBindingInData(const vvl::InlineUniformBinding &binding, glsl::DescriptorState *data, uint32_t &index) {
    // While not techincally a "null descriptor" we want to skip it as if it is one
    data[index++] = glsl::DescriptorState(DescriptorClass::InlineUniform, glsl::kNullDescriptor, vvl::kU32Max);
}

VkDeviceAddress DescriptorSet::GetTypeAddress(Validator &gpuav, const Location &loc) {
    auto guard = Lock();
    const uint32_t current_version = current_version_.load();

    // Will be empty on first time getting the state
    if (input_buffer_.Address() != 0) {
        if (last_used_version_ == current_version) {
            return input_buffer_.Address();  // nothing has changed
        } else {
            // will replace (descriptor array size might have change, so need to resize buffer)
            input_buffer_.Destroy();
        }
    }

    last_used_version_ = current_version;

    if (GetNonInlineDescriptorCount() == 0) {
        // no descriptors case, return a dummy state object
        return input_buffer_.Address();
    }

    VkBufferCreateInfo buffer_info = vku::InitStruct<VkBufferCreateInfo>();
    buffer_info.size = GetNonInlineDescriptorCount() * sizeof(glsl::DescriptorState);
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    // The descriptor state buffer can be very large (4mb+ in some games). Allocating it as HOST_CACHED
    // and manually flushing it at the end of the state updates is faster than using HOST_COHERENT.
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    const bool success = input_buffer_.Create(loc, &buffer_info, &alloc_info);
    if (!success) {
        return 0;
    }

    auto data = (glsl::DescriptorState *)input_buffer_.MapMemory(loc);

    uint32_t index = 0;
    for (uint32_t i = 0; i < bindings_.size(); i++) {
        const auto &binding = *bindings_[i];
        switch (binding.descriptor_class) {
            case DescriptorClass::InlineUniform:
                FillBindingInData(static_cast<const vvl::InlineUniformBinding &>(binding), data, index);
                break;
            case DescriptorClass::GeneralBuffer:
                FillBindingInData(static_cast<const vvl::BufferBinding &>(binding), data, index);
                break;
            case DescriptorClass::TexelBuffer:
                FillBindingInData(static_cast<const vvl::TexelBinding &>(binding), data, index);
                break;
            case DescriptorClass::Mutable:
                FillBindingInData(static_cast<const vvl::MutableBinding &>(binding), data, index);
                break;
            case DescriptorClass::PlainSampler:
                FillBindingInData(static_cast<const vvl::SamplerBinding &>(binding), data, index);
                break;
            case DescriptorClass::ImageSampler:
                FillBindingInData(static_cast<const vvl::ImageSamplerBinding &>(binding), data, index);
                break;
            case DescriptorClass::Image:
                FillBindingInData(static_cast<const vvl::ImageBinding &>(binding), data, index);
                break;
            case DescriptorClass::AccelerationStructure:
                FillBindingInData(static_cast<const vvl::AccelerationStructureBinding &>(binding), data, index);
                break;
            case DescriptorClass::Invalid:
                gpuav.InternalError(gpuav.device, loc, "Unknown DescriptorClass");
        }
    }

    // Flush the descriptor state buffer before unmapping so that the new state is visible to the GPU
    input_buffer_.FlushAllocation(loc);
    input_buffer_.UnmapMemory();

    return input_buffer_.Address();
}

// There are times we need a dummy address because the app is legally using something that doesn't require a post process buffer
bool DescriptorSet::CanPostProcess() const {
    // When no descriptors (only inline, zero bindingCount, etc)
    if (GetNonInlineDescriptorCount() == 0) {
        return false;
    }
    return true;
}

VkDeviceAddress DescriptorSet::GetPostProcessBuffer(Validator &gpuav, const Location &loc) {
    auto guard = Lock();
    // Each set only needs to create its post process buffer once. It is based on total descriptor count, and even with things like
    // VARIABLE_DESCRIPTOR_COUNT_BIT, the size will only get smaller afterwards.
    if (post_process_buffer_.Address() != 0) {
        return post_process_buffer_.Address();
    }

    if (!CanPostProcess()) {
        return post_process_buffer_.Address();
    }

    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = GetNonInlineDescriptorCount() * sizeof(glsl::PostProcessDescriptorIndexSlot);
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    const bool success = post_process_buffer_.Create(loc, &buffer_info, &alloc_info);
    if (!success) {
        return 0;
    }

    void *data = post_process_buffer_.MapMemory(loc);
    memset(data, 0, static_cast<size_t>(buffer_info.size));

    post_process_buffer_.UnmapMemory();

    return post_process_buffer_.Address();
}

// cross checks the two buffers (our layout with the output from the GPU-AV run) and builds a map of which indexes in which binding
// where accessed
std::vector<DescriptorAccess> DescriptorSet::GetDescriptorAccesses(const Location &loc, uint32_t shader_set) const {
    std::vector<DescriptorAccess> descriptor_accesses;
    if (post_process_buffer_.IsDestroyed()) {
        return descriptor_accesses;
    }

    auto slot_ptr = (glsl::PostProcessDescriptorIndexSlot *)post_process_buffer_.MapMemory(loc);
    post_process_buffer_.InvalidateAllocation(loc);

    for (uint32_t binding = 0; binding < binding_layouts_.size(); binding++) {
        const gpuav::spirv::BindingLayout &binding_layout = binding_layouts_[binding];
        for (uint32_t descriptor_i = 0; descriptor_i < binding_layout.count; descriptor_i++) {
            const glsl::PostProcessDescriptorIndexSlot slot = slot_ptr[binding_layout.start + descriptor_i];
            if (slot.descriptor_set & glsl::kDescriptorSetAccessedMask) {
                if ((slot.descriptor_set & glsl::kDescriptorSetSelectionMask) == shader_set) {
                    descriptor_accesses.emplace_back(DescriptorAccess{binding, descriptor_i, slot.variable_id});
                }
            }
        }
    }

    post_process_buffer_.UnmapMemory();
    return descriptor_accesses;
}

void DescriptorSet::PerformPushDescriptorsUpdate(uint32_t write_count, const VkWriteDescriptorSet *write_descs) {
    vvl::DescriptorSet::PerformPushDescriptorsUpdate(write_count, write_descs);
    current_version_++;
}

void DescriptorSet::PerformWriteUpdate(const VkWriteDescriptorSet &write_desc) {
    vvl::DescriptorSet::PerformWriteUpdate(write_desc);
    current_version_++;
}

void DescriptorSet::PerformCopyUpdate(const VkCopyDescriptorSet &copy_desc, const vvl::DescriptorSet &src_set) {
    vvl::DescriptorSet::PerformCopyUpdate(copy_desc, src_set);
    current_version_++;
}

DescriptorHeap::DescriptorHeap(Validator &gpuav, uint32_t max_descriptors, const Location &loc)
    : max_descriptors_(max_descriptors), buffer_(gpuav) {
    // If max_descriptors_ is 0, GPU-AV aborted during vkCreateDevice(). We still need to
    // support calls into this class as no-ops if this happens.
    if (max_descriptors_ == 0) {
        return;
    }

    VkBufferCreateInfo buffer_info = vku::InitStruct<VkBufferCreateInfo>();
    buffer_info.size = BitBufferSize(max_descriptors_ + 1);  // add extra entry since 0 is the invalid id.
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    const bool success = buffer_.Create(loc, &buffer_info, &alloc_info);
    if (!success) {
        return;
    }

    gpu_heap_state_ = (uint32_t *)buffer_.MapMemory(loc);
    memset(gpu_heap_state_, 0, static_cast<size_t>(buffer_info.size));
}

DescriptorHeap::~DescriptorHeap() {
    if (max_descriptors_ > 0) {
        buffer_.UnmapMemory();
        buffer_.Destroy();
        gpu_heap_state_ = nullptr;
    }
}

DescriptorId DescriptorHeap::NextId(const VulkanTypedHandle &handle) {
    if (max_descriptors_ == 0) {
        return 0;
    }
    DescriptorId result;

    // NOTE: valid ids are in the range [1, max_descriptors_] (inclusive)
    // 0 is the invalid id.
    auto guard = Lock();
    if (alloc_map_.size() >= max_descriptors_) {
        return 0;
    }
    do {
        result = next_id_++;
        if (next_id_ > max_descriptors_) {
            next_id_ = 1;
        }
    } while (alloc_map_.count(result) > 0);
    alloc_map_[result] = handle;
    gpu_heap_state_[result / 32] |= 1u << (result & 31);
    return result;
}

void DescriptorHeap::DeleteId(DescriptorId id) {
    if (max_descriptors_ > 0) {
        auto guard = Lock();
        // Note: We don't mess with next_id_ here because ids should be signed in LRU order.
        gpu_heap_state_[id / 32] &= ~(1u << (id & 31));
        alloc_map_.erase(id);
    }
}

}  // namespace gpuav
