/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
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

#include "state_tracker/descriptor_sets.h"
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include "state_tracker/image_state.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/ray_tracing_state.h"
#include "state_tracker/sampler_state.h"
#include "state_tracker/shader_module.h"

static vvl::DescriptorPool::TypeCountMap GetMaxTypeCounts(const VkDescriptorPoolCreateInfo *create_info) {
    vvl::DescriptorPool::TypeCountMap counts;
    // Collect maximums per descriptor type.
    for (uint32_t i = 0; i < create_info->poolSizeCount; ++i) {
        const auto &pool_size = create_info->pPoolSizes[i];
        uint32_t type = static_cast<uint32_t>(pool_size.type);
        // Same descriptor types can appear several times
        counts[type] += pool_size.descriptorCount;
    }
    return counts;
}

vvl::DescriptorPool::DescriptorPool(vvl::Device &dev, const VkDescriptorPool handle, const VkDescriptorPoolCreateInfo *pCreateInfo)
    : StateObject(handle, kVulkanObjectTypeDescriptorPool),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      maxSets(pCreateInfo->maxSets),
      maxDescriptorTypeCount(GetMaxTypeCounts(pCreateInfo)),
      available_sets_(pCreateInfo->maxSets),
      available_counts_(maxDescriptorTypeCount),
      dev_data_(dev) {}

void vvl::DescriptorPool::Allocate(const VkDescriptorSetAllocateInfo *alloc_info, const VkDescriptorSet *descriptor_sets,
                                   const vvl::AllocateDescriptorSetsData &ds_data) {
    auto guard = WriteLock();
    // Account for sets and individual descriptors allocated from pool
    available_sets_ -= alloc_info->descriptorSetCount;
    for (auto it = ds_data.required_descriptors_by_type.begin(); it != ds_data.required_descriptors_by_type.end(); ++it) {
        available_counts_[it->first] -= ds_data.required_descriptors_by_type.at(it->first);
    }

    const auto *variable_count_info = vku::FindStructInPNextChain<VkDescriptorSetVariableDescriptorCountAllocateInfo>(alloc_info->pNext);
    const bool variable_count_valid =
        variable_count_info && variable_count_info->descriptorSetCount == alloc_info->descriptorSetCount;

    // Create tracking object for each descriptor set; insert into global map and the pool's set.
    for (uint32_t i = 0; i < alloc_info->descriptorSetCount; i++) {
        uint32_t variable_count = variable_count_valid ? variable_count_info->pDescriptorCounts[i] : 0;

        auto new_ds = dev_data_.CreateDescriptorSet(descriptor_sets[i], this, ds_data.layout_nodes[i], variable_count);

        sets_.emplace(descriptor_sets[i], new_ds.get());
        dev_data_.Add(std::move(new_ds));
    }
}

void vvl::DescriptorPool::Free(uint32_t count, const VkDescriptorSet *descriptor_sets) {
    auto guard = WriteLock();
    // Update available descriptor sets in pool
    available_sets_ += count;

    // For each freed descriptor add its resources back into the pool as available and remove from pool and device data
    for (uint32_t i = 0; i < count; ++i) {
        if (descriptor_sets[i] != VK_NULL_HANDLE) {
            auto iter = sets_.find(descriptor_sets[i]);
            ASSERT_AND_CONTINUE(iter != sets_.end());
            auto *set_state = iter->second;
            const auto &layout = set_state->Layout();
            uint32_t type_index = 0, descriptor_count = 0;
            for (uint32_t j = 0; j < layout.GetBindingCount(); ++j) {
                type_index = static_cast<uint32_t>(layout.GetTypeFromIndex(j));
                descriptor_count = layout.GetDescriptorCountFromIndex(j);
                available_counts_[type_index] += descriptor_count;
            }
            dev_data_.Destroy<vvl::DescriptorSet>(iter->first);
            sets_.erase(iter);
        }
    }
}

void vvl::DescriptorPool::Reset() {
    auto guard = WriteLock();
    // For every set off of this pool, clear it, remove from setMap, and free vvl::DescriptorSet
    for (auto entry : sets_) {
        dev_data_.Destroy<vvl::DescriptorSet>(entry.first);
    }
    sets_.clear();
    // Reset available count for each type and available sets for this pool
    available_counts_ = maxDescriptorTypeCount;
    available_sets_ = maxSets;
}

const VulkanTypedHandle *vvl::DescriptorPool::InUse() const {
    auto guard = ReadLock();
    for (const auto &entry : sets_) {
        const auto *ds = entry.second;
        if (ds) {
            return ds->InUse();
        }
    }
    return nullptr;
}

void vvl::DescriptorPool::Destroy() {
    Reset();
    StateObject::Destroy();
}

// ExtendedBinding collects a VkDescriptorSetLayoutBinding and any extended
// state that comes from a different array/structure so they can stay together
// while being sorted by binding number.
struct ExtendedBinding {
    ExtendedBinding(const VkDescriptorSetLayoutBinding *l, VkDescriptorBindingFlags f) : layout_binding(l), binding_flags(f) {}

    const VkDescriptorSetLayoutBinding *layout_binding;
    VkDescriptorBindingFlags binding_flags;
};

struct BindingNumCmp {
    bool operator()(const ExtendedBinding &a, const ExtendedBinding &b) const {
        return a.layout_binding->binding < b.layout_binding->binding;
    }
};

vvl::DescriptorClass vvl::DescriptorTypeToClass(VkDescriptorType type) {
    switch (type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            return DescriptorClass::PlainSampler;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            return DescriptorClass::ImageSampler;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
            return DescriptorClass::Image;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            return DescriptorClass::TexelBuffer;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            return DescriptorClass::GeneralBuffer;
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
            return DescriptorClass::InlineUniform;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
            return DescriptorClass::AccelerationStructure;
        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
            return DescriptorClass::Mutable;
        case VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV:
            // TODO
            break;
        case VK_DESCRIPTOR_TYPE_MAX_ENUM:
            break;
    }
    return DescriptorClass::Invalid;
}

using DescriptorSet = vvl::DescriptorSet;
using DescriptorSetLayout = vvl::DescriptorSetLayout;
using DescriptorSetLayoutDef = vvl::DescriptorSetLayoutDef;
using DescriptorSetLayoutId = vvl::DescriptorSetLayoutId;

// Canonical dictionary of DescriptorSetLayoutDef (without any handle/device specific information)
vvl::DescriptorSetLayoutDict descriptor_set_layout_dict;

DescriptorSetLayoutId GetCanonicalId(const VkDescriptorSetLayoutCreateInfo *p_create_info) {
    return descriptor_set_layout_dict.LookUp(DescriptorSetLayoutDef(p_create_info));
}

std::string DescriptorSetLayoutDef::DescribeDifference(uint32_t index, const DescriptorSetLayoutDef &other) const {
    std::ostringstream ss;
    ss << "Set " << index << " ";
    auto lhs_binding_flags = GetBindingFlags();
    auto rhs_binding_flags = other.GetBindingFlags();
    const auto &lhs_bindings = GetBindings();
    const auto &rhs_bindings = other.GetBindings();

    if (GetCreateFlags() != other.GetCreateFlags()) {
        ss << "VkDescriptorSetLayoutCreateFlags " << string_VkDescriptorSetLayoutCreateFlags(GetCreateFlags()) << " doesn't match "
           << string_VkDescriptorSetLayoutCreateFlags(other.GetCreateFlags());
    } else if (lhs_binding_flags.size() != rhs_binding_flags.size()) {
        ss << "VkDescriptorSetLayoutBindingFlagsCreateInfo::bindingCount " << lhs_binding_flags.size() << " doesn't match "
           << rhs_binding_flags.size();
    } else if (lhs_binding_flags != rhs_binding_flags) {
        ss << "VkDescriptorSetLayoutBindingFlagsCreateInfo::pBindingFlags (";
        for (auto flag : lhs_binding_flags) {
            ss << string_VkDescriptorBindingFlags(flag) << " ";
        }
        ss << ") doesn't match (";
        for (auto flag : rhs_binding_flags) {
            ss << string_VkDescriptorBindingFlags(flag) << " ";
        }
        ss << ")";
    } else if (lhs_bindings.size() != rhs_bindings.size()) {
        ss << "binding count " << lhs_bindings.size() << " doesn't match " << rhs_bindings.size();
    } else {
        for (uint32_t i = 0; i < lhs_bindings.size(); i++) {
            const auto &l = lhs_bindings[i];
            const auto &r = rhs_bindings[i];
            if (l.binding != r.binding) {
                ss << "VkDescriptorSetLayoutBinding::binding " << l.binding << " doesn't match " << r.binding;
                break;
            } else if (l.descriptorType != r.descriptorType) {
                ss << "binding " << i << " descriptorType " << string_VkDescriptorType(l.descriptorType) << " doesn't match "
                   << string_VkDescriptorType(r.descriptorType);
                break;
            } else if (l.descriptorCount != r.descriptorCount) {
                ss << "binding " << i << " descriptorCount " << l.descriptorCount << " doesn't match " << r.descriptorCount;
                break;
            } else if (l.stageFlags != r.stageFlags) {
                ss << "binding " << i << " stageFlags " << string_VkShaderStageFlags(l.stageFlags) << " doesn't match "
                   << string_VkShaderStageFlags(r.stageFlags);
                break;
            } else if (l.pImmutableSamplers != r.pImmutableSamplers) {
                ss << "binding " << i << " pImmutableSamplers " << l.pImmutableSamplers << " doesn't match "
                   << r.pImmutableSamplers;
                break;
            } else if (l.pImmutableSamplers) {
                for (uint32_t s = 0; s < l.descriptorCount; s++) {
                    if (l.pImmutableSamplers[s] != r.pImmutableSamplers[s]) {
                        ss << "binding " << i << " pImmutableSamplers[" << s << "] " << l.pImmutableSamplers[s] << " doesn't match "
                           << r.pImmutableSamplers[s];
                        break;
                    }
                }
            } else if (GetMutableTypes(i) != other.GetMutableTypes(i)) {
                // These have been sorted already so can direct compare
                ss << "Mutable types doesn't match at binding " << i << "\n[" << PrintMutableTypes(i) << "]\ndoesn't match"
                   << "\n[" << other.PrintMutableTypes(i) << "]";
            }
        }
    }
    ss << '\n';
    return ss.str();
}

// Construct DescriptorSetLayout instance from given create info
// Proactively reserve and resize as possible, as the reallocation was visible in profiling
vvl::DescriptorSetLayoutDef::DescriptorSetLayoutDef(const VkDescriptorSetLayoutCreateInfo *p_create_info)
    : flags_(p_create_info->flags),
      binding_count_(0),
      descriptor_count_(0),
      non_inline_descriptor_count_(0),
      dynamic_descriptor_count_(0) {
    const auto *flags_create_info = vku::FindStructInPNextChain<VkDescriptorSetLayoutBindingFlagsCreateInfo>(p_create_info->pNext);

    binding_type_stats_ = {0, 0};
    std::set<ExtendedBinding, BindingNumCmp> sorted_bindings;
    const uint32_t input_bindings_count = p_create_info->bindingCount;
    // Sort the input bindings in binding number order, eliminating duplicates
    for (uint32_t i = 0; i < input_bindings_count; i++) {
        VkDescriptorBindingFlags flags = 0;
        if (flags_create_info && flags_create_info->bindingCount == p_create_info->bindingCount) {
            flags = flags_create_info->pBindingFlags[i];
        }
        sorted_bindings.emplace(p_create_info->pBindings + i, flags);
    }

    const auto *mutable_descriptor_type_create_info = vku::FindStructInPNextChain<VkMutableDescriptorTypeCreateInfoEXT>(p_create_info->pNext);
    if (mutable_descriptor_type_create_info) {
        mutable_types_.resize(mutable_descriptor_type_create_info->mutableDescriptorTypeListCount);
        for (uint32_t i = 0; i < mutable_descriptor_type_create_info->mutableDescriptorTypeListCount; ++i) {
            const auto &list = mutable_descriptor_type_create_info->pMutableDescriptorTypeLists[i];
            mutable_types_[i].reserve(list.descriptorTypeCount);
            for (uint32_t j = 0; j < list.descriptorTypeCount; ++j) {
                mutable_types_[i].push_back(list.pDescriptorTypes[j]);
            }
            std::sort(mutable_types_[i].begin(), mutable_types_[i].end());
        }
    }

    // Store the create info in the sorted order from above
    uint32_t index = 0;
    binding_count_ = static_cast<uint32_t>(sorted_bindings.size());
    bindings_.reserve(binding_count_);
    binding_flags_.reserve(binding_count_);
    binding_to_index_map_.reserve(binding_count_);
    for (const auto &input_binding : sorted_bindings) {
        // Add to binding and map, s.t. it is robust to invalid duplication of binding_num
        const auto binding_num = input_binding.layout_binding->binding;
        binding_to_index_map_[binding_num] = index++;
        bindings_.emplace_back(input_binding.layout_binding);
        auto &binding_info = bindings_.back();
        binding_flags_.emplace_back(input_binding.binding_flags);

        descriptor_count_ += binding_info.descriptorCount;
        if (binding_info.descriptorCount > 0) {
            non_empty_bindings_.insert(binding_num);
        }

        non_inline_descriptor_count_ +=
            (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) ? 1 : binding_info.descriptorCount;

        if (IsDynamicDescriptor(binding_info.descriptorType)) {
            dynamic_descriptor_count_ += binding_info.descriptorCount;
        }

        // Get stats depending on descriptor type for caching later
        if (IsBufferDescriptor(binding_info.descriptorType)) {
            if (IsDynamicDescriptor(binding_info.descriptorType)) {
                binding_type_stats_.dynamic_buffer_count++;
            } else {
                binding_type_stats_.non_dynamic_buffer_count++;
            }
        }
    }
    assert(bindings_.size() == binding_count_);
    assert(binding_flags_.size() == binding_count_);
    uint32_t global_index = 0;
    global_index_range_.reserve(binding_count_);
    // Vector order is finalized so build vectors of descriptors and dynamic offsets by binding index
    for (uint32_t i = 0; i < binding_count_; ++i) {
        auto final_index = global_index + bindings_[i].descriptorCount;
        global_index_range_.emplace_back(global_index, final_index);
        global_index = final_index;
    }
}

size_t vvl::DescriptorSetLayoutDef::hash() const {
    hash_util::HashCombiner hc;
    hc << flags_;
    hc.Combine(bindings_);
    hc.Combine(binding_flags_);
    return hc.Value();
}
//

// Return valid index or "end" i.e. binding_count_;
// The asserts in "Get" are reduced to the set where no valid answer(like null or 0) could be given
// Common code for all binding lookups.
uint32_t vvl::DescriptorSetLayoutDef::GetIndexFromBinding(uint32_t binding) const {
    const auto &bi_itr = binding_to_index_map_.find(binding);
    if (bi_itr != binding_to_index_map_.cend()) return bi_itr->second;
    return GetBindingCount();
}
VkDescriptorSetLayoutBinding const *vvl::DescriptorSetLayoutDef::GetDescriptorSetLayoutBindingPtrFromIndex(
    const uint32_t index) const {
    if (index >= bindings_.size()) return nullptr;
    return bindings_[index].ptr();
}
// Return descriptorCount for given index, 0 if index is unavailable
uint32_t vvl::DescriptorSetLayoutDef::GetDescriptorCountFromIndex(const uint32_t index) const {
    if (index >= bindings_.size()) return 0;
    return bindings_[index].descriptorCount;
}
// For the given index, return descriptorType
VkDescriptorType vvl::DescriptorSetLayoutDef::GetTypeFromIndex(const uint32_t index) const {
    assert(index < bindings_.size());
    if (index < bindings_.size()) return bindings_[index].descriptorType;
    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}
// Return binding flags for given index, 0 if index is unavailable
VkDescriptorBindingFlags vvl::DescriptorSetLayoutDef::GetDescriptorBindingFlagsFromIndex(const uint32_t index) const {
    if (index >= binding_flags_.size()) return 0;
    return binding_flags_[index];
}

const vvl::IndexRange &vvl::DescriptorSetLayoutDef::GetGlobalIndexRangeFromIndex(uint32_t index) const {
    const static IndexRange k_invalid_range = {0xFFFFFFFF, 0xFFFFFFFF};
    if (index >= binding_flags_.size()) return k_invalid_range;
    return global_index_range_[index];
}

// For the given binding, return the global index range (half open)
// As start and end are often needed in pairs, get both with a single lookup.
const vvl::IndexRange &vvl::DescriptorSetLayoutDef::GetGlobalIndexRangeFromBinding(
    const uint32_t binding) const {
    uint32_t index = GetIndexFromBinding(binding);
    return GetGlobalIndexRangeFromIndex(index);
}

// Move to next valid binding having a non-zero binding count
uint32_t vvl::DescriptorSetLayoutDef::GetNextValidBinding(const uint32_t binding) const {
    auto it = non_empty_bindings_.upper_bound(binding);
    if (it != non_empty_bindings_.cend()) return *it;
    return GetMaxBinding() + 1;
}
// For given index, return ptr to ImmutableSampler array
VkSampler const *vvl::DescriptorSetLayoutDef::GetImmutableSamplerPtrFromIndex(const uint32_t index) const {
    if (index < bindings_.size()) {
        return bindings_[index].pImmutableSamplers;
    }
    return nullptr;
}

bool vvl::DescriptorSetLayoutDef::IsTypeMutable(const VkDescriptorType type, uint32_t binding) const {
    if (binding < mutable_types_.size()) {
        if (mutable_types_[binding].size() > 0) {
            for (const auto mutable_type : mutable_types_[binding]) {
                if (type == mutable_type) {
                    return true;
                }
            }
            return false;
        }
    }
    // If mutableDescriptorTypeListCount is zero or if VkMutableDescriptorTypeCreateInfoEXT structure is not included in the pNext
    // chain, the VkMutableDescriptorTypeListEXT for each element is considered to be zero or NULL for each member.
    return false;
}

std::string vvl::DescriptorSetLayoutDef::PrintMutableTypes(uint32_t binding) const {
    if (binding >= mutable_types_.size()) {
        return "no Mutable Type list at this binding";
    }
    std::ostringstream ss;
    const auto mutable_types = mutable_types_[binding];
    if (mutable_types.empty()) {
        ss << "pMutableDescriptorTypeLists is empty";
    } else {
        for (uint32_t i = 0; i < mutable_types.size(); i++) {
            ss << string_VkDescriptorType(mutable_types[i]);
            if (i + 1 != mutable_types.size()) {
                ss << ", ";
            }
        }
    }
    return ss.str();
}

const std::vector<VkDescriptorType> &vvl::DescriptorSetLayoutDef::GetMutableTypes(uint32_t binding) const {
    if (binding >= mutable_types_.size()) {
        static const std::vector<VkDescriptorType> empty = {};
        return empty;
    }
    return mutable_types_[binding];
}

void vvl::DescriptorSetLayout::SetLayoutSizeInBytes(const VkDeviceSize *layout_size_in_bytes_) {
    if (layout_size_in_bytes_) {
        layout_size_in_bytes = std::make_unique<VkDeviceSize>(*layout_size_in_bytes_);
    } else {
        layout_size_in_bytes.reset();
    }
}

const VkDeviceSize *vvl::DescriptorSetLayout::GetLayoutSizeInBytes() const { return layout_size_in_bytes.get(); }

// If our layout is compatible with rh_ds_layout, return true.
bool vvl::DescriptorSetLayout::IsCompatible(DescriptorSetLayout const *rh_ds_layout) const {
    return (this == rh_ds_layout) || (GetLayoutDef() == rh_ds_layout->GetLayoutDef());
}

// The DescriptorSetLayout stores the per handle data for a descriptor set layout, and references the common defintion for the
// handle invariant portion
vvl::DescriptorSetLayout::DescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                              const VkDescriptorSetLayout handle)
    : StateObject(handle, kVulkanObjectTypeDescriptorSetLayout), layout_id_(GetCanonicalId(pCreateInfo)) {}

void vvl::AllocateDescriptorSetsData::Init(uint32_t count) { layout_nodes.resize(count); }

vvl::DescriptorSet::DescriptorSet(const VkDescriptorSet handle, vvl::DescriptorPool *pool_state,
                                  const std::shared_ptr<DescriptorSetLayout const> &layout, uint32_t variable_count,
                                  vvl::Device *state_data)
    : StateObject(handle, kVulkanObjectTypeDescriptorSet),
      some_update_(false),
      pool_state_(pool_state),
      layout_(layout),
      state_data_(state_data),
      variable_count_(variable_count),
      change_count_(0) {
    // Foreach binding, create default descriptors of given type
    auto binding_count = layout_->GetBindingCount();
    bindings_.reserve(binding_count);
    bindings_store_.resize(binding_count);
    auto free_binding = bindings_store_.data();
    for (uint32_t i = 0; i < binding_count; ++i) {
        auto create_info = layout_->GetDescriptorSetLayoutBindingPtrFromIndex(i);
        ASSERT_AND_CONTINUE(create_info);

        uint32_t descriptor_count = create_info->descriptorCount;
        auto flags = layout_->GetDescriptorBindingFlagsFromIndex(i);
        if (flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
            descriptor_count = variable_count;
        }
        auto type = layout_->GetTypeFromIndex(i);
        auto descriptor_class = DescriptorTypeToClass(type);
        switch (descriptor_class) {
            case DescriptorClass::PlainSampler: {
                auto binding = MakeBinding<SamplerBinding>(free_binding++, *create_info, descriptor_count, flags);
                if (auto immutable_sampler_handles = layout_->GetImmutableSamplerPtrFromIndex(i)) {
                    for (uint32_t di = 0; di < descriptor_count; ++di) {
                        auto sampler = state_data->GetConstCastShared<vvl::Sampler>(immutable_sampler_handles[di]);
                        if (sampler) {
                            some_update_ = true;  // Immutable samplers are updated at creation
                            binding->updated[di] = true;
                            binding->descriptors[di].SetImmutableSampler(std::move(sampler));
                        }
                    }
                }
                bindings_.push_back(std::move(binding));
                break;
            }
            case DescriptorClass::ImageSampler: {
                auto binding = MakeBinding<ImageSamplerBinding>(free_binding++, *create_info, descriptor_count, flags);
                if (auto immutable_sampler_handles = layout_->GetImmutableSamplerPtrFromIndex(i)) {
                    for (uint32_t di = 0; di < descriptor_count; ++di) {
                        auto sampler = state_data->GetConstCastShared<vvl::Sampler>(immutable_sampler_handles[di]);
                        if (sampler) {
                            some_update_ = true;  // Immutable samplers are updated at creation
                            binding->updated[di] = true;
                            binding->descriptors[di].SetImmutableSampler(std::move(sampler));
                        }
                    }
                }
                bindings_.push_back(std::move(binding));
                break;
            }
            // ImageDescriptors
            case DescriptorClass::Image: {
                bindings_.push_back(MakeBinding<ImageBinding>(free_binding++, *create_info, descriptor_count, flags));
                break;
            }
            case DescriptorClass::TexelBuffer: {
                bindings_.push_back(MakeBinding<TexelBinding>(free_binding++, *create_info, descriptor_count, flags));
                break;
            }
            case DescriptorClass::GeneralBuffer: {
                auto binding = MakeBinding<BufferBinding>(free_binding++, *create_info, descriptor_count, flags);
                if (IsDynamicDescriptor(type)) {
                    for (uint32_t di = 0; di < descriptor_count; ++di) {
                        dynamic_offset_idx_to_descriptor_list_.emplace_back(i, di);
                    }
                }
                bindings_.push_back(std::move(binding));
                break;
            }
            case DescriptorClass::InlineUniform: {
                bindings_.push_back(MakeBinding<InlineUniformBinding>(free_binding++, *create_info, descriptor_count, flags));
                break;
            }
            case DescriptorClass::AccelerationStructure: {
                bindings_.push_back(
                    MakeBinding<AccelerationStructureBinding>(free_binding++, *create_info, descriptor_count, flags));
                break;
            }
            case DescriptorClass::Mutable: {
                bindings_.push_back(MakeBinding<MutableBinding>(free_binding++, *create_info, descriptor_count, flags));
                break;
            }
            case DescriptorClass::Invalid:
                assert(false);  // Bad descriptor type specified
                break;
        }
    }
}

void vvl::DescriptorSet::LinkChildNodes() {
    // Connect child node(s), which cannot safely be done in the constructor.
    for (auto &binding : bindings_) {
        binding->AddParent(this);
    }
}

void vvl::DescriptorSet::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    BaseClass::NotifyInvalidate(invalid_nodes, unlink);
    for (auto &binding : bindings_) {
        binding->NotifyInvalidate(invalid_nodes, unlink);
    }
}

uint32_t vvl::DescriptorSet::GetDynamicOffsetIndexFromBinding(uint32_t dynamic_binding) const {
    const uint32_t index = layout_->GetIndexFromBinding(dynamic_binding);
    if (index == bindings_.size()) {  // binding not found
        return vvl::kU32Max;
    }
    assert(IsDynamicDescriptor(bindings_[index]->type));
    uint32_t dynamic_offset_index = 0;
    for (uint32_t i = 0; i < index; i++) {
        if (IsDynamicDescriptor(bindings_[i]->type)) {
            dynamic_offset_index += bindings_[i]->count;
        }
    }
    return dynamic_offset_index;
}

void vvl::DescriptorSet::Destroy() {
    for (auto &binding : bindings_) {
        binding->RemoveParent(this);
    }
    StateObject::Destroy();
}
// Loop through the write updates to do for a push descriptor set, ignoring dstSet
void vvl::DescriptorSet::PerformPushDescriptorsUpdate(uint32_t write_count, const VkWriteDescriptorSet *write_descs) {
    assert(IsPushDescriptor());
    for (uint32_t i = 0; i < write_count; i++) {
        PerformWriteUpdate(write_descs[i]);
    }

    push_descriptor_set_writes.clear();
    push_descriptor_set_writes.reserve(static_cast<std::size_t>(write_count));
    for (uint32_t i = 0; i < write_count; i++) {
        push_descriptor_set_writes.push_back(vku::safe_VkWriteDescriptorSet(&write_descs[i]));
    }
}

// Perform write update in given update struct
void vvl::DescriptorSet::PerformWriteUpdate(const VkWriteDescriptorSet &update) {
    // Perform update on a per-binding basis as consecutive updates roll over to next binding
    auto descriptors_remaining = update.descriptorCount;
    auto iter = FindDescriptor(update.dstBinding, update.dstArrayElement);
    ASSERT_AND_RETURN(!iter.AtEnd());
    auto &orig_binding = iter.CurrentBinding();

    // Verify next consecutive binding matches type, stage flags & immutable sampler use and if AtEnd
    for (uint32_t i = 0; i < descriptors_remaining; ++i, ++iter) {
        if (iter.AtEnd() || !orig_binding.IsConsistent(iter.CurrentBinding())) {
            break;
        }
        iter->WriteUpdate(*this, *state_data_, update, i, IsBindless(iter.CurrentBinding().binding_flags));
        iter.updated(true);
    }
    if (update.descriptorCount) {
        some_update_ = true;
        ++change_count_;
    }

    if (!IsPushDescriptor() && !(orig_binding.binding_flags & (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
                                                               VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT))) {
        Invalidate(false);
    }
}
// Perform Copy update
void vvl::DescriptorSet::PerformCopyUpdate(const VkCopyDescriptorSet &update, const DescriptorSet &src_set) {
    auto src_iter = src_set.FindDescriptor(update.srcBinding, update.srcArrayElement);
    auto dst_iter = FindDescriptor(update.dstBinding, update.dstArrayElement);
    // Update parameters all look good so perform update
    for (uint32_t i = 0; i < update.descriptorCount; ++i, ++src_iter, ++dst_iter) {
        auto &src = *src_iter;
        auto &dst = *dst_iter;
        if (src_iter.updated()) {
            auto type = src_iter.CurrentBinding().type;
            if (type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                const auto &mutable_src = static_cast<const MutableDescriptor &>(src);
                type = mutable_src.ActiveType();
            }
            dst.CopyUpdate(*this, *state_data_, src, IsBindless(src_iter.CurrentBinding().binding_flags), type);
            some_update_ = true;
            ++change_count_;
            dst_iter.updated(true);
        } else {
            dst_iter.updated(false);
        }
    }

    if (!(layout_->GetDescriptorBindingFlagsFromBinding(update.dstBinding) &
          (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT))) {
        Invalidate(false);
    }
}

// Update the drawing state for the affected descriptors.
// Set cb_state to this set and this set to cb_state.
// Add the bindings of the descriptor
// Set the layout based on the current descriptor layout (will mask subsequent layer mismatch errors)
// TODO: Modify the UpdateDrawState virtural functions to *only* set initial layout and not change layouts
// Prereq: This should be called for a set that has been confirmed to be active for the given cb_state, meaning it's going
//   to be used in a draw by the given cb_state
void vvl::DescriptorSet::UpdateDrawStates(vvl::Device *device_data, vvl::CommandBuffer &cb_state,
                                          const BindingVariableMap &binding_req_map) {
    // Descriptor UpdateDrawState only call image layout validation callbacks. If it is disabled, skip the entire loop.
    if (device_data->disabled[image_layout_validation]) {
        return;
    }

    // For the active slots, use set# to look up descriptorSet from boundDescriptorSets, and bind all of that descriptor set's
    // resources
    for (const auto &binding_req_pair : binding_req_map) {
        auto *binding = GetBinding(binding_req_pair.first);
        ASSERT_AND_CONTINUE(binding);

        // core validation doesn't handle descriptor indexing, that is only done by GPU-AV
        if (ValidateBindingOnGPU(*binding, binding_req_pair.second.variable->is_runtime_descriptor_array)) {
            continue;
        }
        switch (binding->descriptor_class) {
            case DescriptorClass::Image: {
                auto *image_binding = static_cast<ImageBinding *>(binding);
                for (uint32_t i = 0; i < image_binding->count; ++i) {
                    image_binding->descriptors[i].UpdateDrawState(cb_state);
                }
                break;
            }
            case DescriptorClass::ImageSampler: {
                auto *image_binding = static_cast<ImageSamplerBinding *>(binding);
                for (uint32_t i = 0; i < image_binding->count; ++i) {
                    image_binding->descriptors[i].UpdateDrawState(cb_state);
                }
                break;
            }
            case DescriptorClass::Mutable: {
                auto *mutable_binding = static_cast<MutableBinding *>(binding);
                for (uint32_t i = 0; i < mutable_binding->count; ++i) {
                    mutable_binding->descriptors[i].UpdateDrawState(cb_state);
                }
                break;
            }
            default:
                break;
        }
    }
}

// This is used to decide if we should validate the Descirptors on the CPU or GPU-AV
bool vvl::DescriptorSet::ValidateBindingOnGPU(const DescriptorBinding &binding, bool is_runtime_descriptor_array) const {
    // Some applications (notably Doom Eternal) might have large non-bindless descriptors attached (basically doing Descriptor
    // Indexing without the extension). Trying to loop through these on the CPU will bring FPS down by over 50% so we make use of
    // the post processing to detect which descriptors were actually accessed
    static constexpr uint32_t max_descriptor_on_cpu = 1024;
    return GetNonInlineDescriptorCount() > max_descriptor_on_cpu || IsBindless(binding.binding_flags) ||
           is_runtime_descriptor_array;
}

// Helper template to change shared pointer members of a Descriptor, while
// correctly managing links to the parent DescriptorSet.
// src and dst are shared pointers.
template <typename T>
static void ReplaceStatePtr(DescriptorSet &set_state, T &dst, const T &src, bool is_bindless) {
    if (dst && !is_bindless) {
        dst->RemoveParent(&set_state);
    }
    dst = src;
    // For descriptor bindings with UPDATE_AFTER_BIND or PARTIALLY_BOUND only set the object as a child, but not the descriptor as a
    // parent, so that destroying the object wont invalidate the descriptor
    if (dst && !is_bindless) {
        dst->AddParent(&set_state);
    }
}

void vvl::SamplerDescriptor::WriteUpdate(DescriptorSet &set_state, const vvl::Device &dev_data, const VkWriteDescriptorSet &update,
                                         const uint32_t index, bool is_bindless) {
    if (!immutable_ && update.pImageInfo) {
        ReplaceStatePtr(set_state, sampler_state_, dev_data.GetConstCastShared<vvl::Sampler>(update.pImageInfo[index].sampler),
                        is_bindless);
    }
}

void vvl::SamplerDescriptor::CopyUpdate(DescriptorSet &set_state, const vvl::Device &dev_data, const Descriptor &src,
                                        bool is_bindless, VkDescriptorType) {
    if (src.GetClass() == DescriptorClass::Mutable) {
        auto &sampler_src = static_cast<const MutableDescriptor &>(src);
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_, sampler_src.GetSharedSamplerState(), is_bindless);
        }
        return;
    }
    auto &sampler_src = static_cast<const SamplerDescriptor &>(src);
    if (!immutable_) {
        ReplaceStatePtr(set_state, sampler_state_, sampler_src.sampler_state_, is_bindless);
    }
}

VkSampler vvl::SamplerDescriptor::GetSampler() const { return sampler_state_ ? sampler_state_->VkHandle() : VK_NULL_HANDLE; }

void vvl::SamplerDescriptor::SetImmutableSampler(std::shared_ptr<vvl::Sampler> &&state) {
    sampler_state_ = std::move(state);
    immutable_ = true;
}

bool vvl::SamplerDescriptor::AddParent(StateObject *state_object) {
    bool result = false;
    if (sampler_state_) {
        result = sampler_state_->AddParent(state_object);
    }
    return result;
}
void vvl::SamplerDescriptor::RemoveParent(StateObject *state_object) {
    if (sampler_state_) {
        sampler_state_->RemoveParent(state_object);
    }
}
bool vvl::SamplerDescriptor::Invalid() const { return !sampler_state_ || sampler_state_->Invalid(); }

void vvl::ImageSamplerDescriptor::WriteUpdate(DescriptorSet &set_state, const vvl::Device &dev_data,
                                              const VkWriteDescriptorSet &update, const uint32_t index, bool is_bindless) {
    if (!update.pImageInfo) return;
    const auto &image_info = update.pImageInfo[index];
    if (!immutable_) {
        ReplaceStatePtr(set_state, sampler_state_, dev_data.GetConstCastShared<vvl::Sampler>(image_info.sampler), is_bindless);
    }
    image_layout_ = image_info.imageLayout;
    ReplaceStatePtr(set_state, image_view_state_, dev_data.GetConstCastShared<vvl::ImageView>(image_info.imageView), is_bindless);
    UpdateKnownValidView(is_bindless);
}

void vvl::ImageSamplerDescriptor::CopyUpdate(DescriptorSet &set_state, const vvl::Device &dev_data, const Descriptor &src,
                                             bool is_bindless, VkDescriptorType src_type) {
    if (src.GetClass() == DescriptorClass::Mutable) {
        auto &image_src = static_cast<const MutableDescriptor &>(src);
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_, image_src.GetSharedSamplerState(), is_bindless);
        }
        ImageDescriptor::CopyUpdate(set_state, dev_data, src, is_bindless, src_type);
        return;
    }
    auto &image_src = static_cast<const ImageSamplerDescriptor &>(src);
    if (!immutable_) {
        ReplaceStatePtr(set_state, sampler_state_, image_src.sampler_state_, is_bindless);
    }
    ImageDescriptor::CopyUpdate(set_state, dev_data, src, is_bindless, src_type);
}

VkSampler vvl::ImageSamplerDescriptor::GetSampler() const { return sampler_state_ ? sampler_state_->VkHandle() : VK_NULL_HANDLE; }

void vvl::ImageSamplerDescriptor::SetImmutableSampler(std::shared_ptr<vvl::Sampler> &&state) {
    sampler_state_ = std::move(state);
    immutable_ = true;
}

bool vvl::ImageSamplerDescriptor::AddParent(StateObject *state_object) {
    bool result = ImageDescriptor::AddParent(state_object);
    if (sampler_state_) {
        result |= sampler_state_->AddParent(state_object);
    }
    return result;
}
void vvl::ImageSamplerDescriptor::RemoveParent(StateObject *state_object) {
    ImageDescriptor::RemoveParent(state_object);
    if (sampler_state_) {
        sampler_state_->RemoveParent(state_object);
    }
}

bool vvl::ImageSamplerDescriptor::Invalid() const {
    return ImageDescriptor::Invalid() || !sampler_state_ || sampler_state_->Invalid();
}

void vvl::ImageDescriptor::WriteUpdate(DescriptorSet &set_state, const vvl::Device &dev_data, const VkWriteDescriptorSet &update,
                                       const uint32_t index, bool is_bindless) {
    if (!update.pImageInfo) return;
    const auto &image_info = update.pImageInfo[index];
    image_layout_ = image_info.imageLayout;
    ReplaceStatePtr(set_state, image_view_state_, dev_data.GetConstCastShared<vvl::ImageView>(image_info.imageView), is_bindless);
    UpdateKnownValidView(is_bindless);
}

void vvl::ImageDescriptor::CopyUpdate(DescriptorSet &set_state, const vvl::Device &dev_data, const Descriptor &src,
                                      bool is_bindless, VkDescriptorType src_type) {
    if (src.GetClass() == DescriptorClass::Mutable) {
        auto &image_src = static_cast<const MutableDescriptor &>(src);

        image_layout_ = image_src.GetImageLayout();
        ReplaceStatePtr(set_state, image_view_state_, image_src.GetSharedImageViewState(), is_bindless);
        UpdateKnownValidView(is_bindless);
        return;
    }
    auto &image_src = static_cast<const ImageDescriptor &>(src);

    image_layout_ = image_src.image_layout_;
    ReplaceStatePtr(set_state, image_view_state_, image_src.image_view_state_, is_bindless);
    UpdateKnownValidView(is_bindless);
}

void vvl::ImageDescriptor::UpdateDrawState(vvl::CommandBuffer &cb_state) {
    // Add binding for image
    auto iv_state = GetImageViewState();
    if (iv_state) {
        cb_state.SetImageViewInitialLayout(*iv_state, image_layout_);
    }
}

VkImageView vvl::ImageDescriptor::GetImageView() const {
    return image_view_state_ ? image_view_state_->VkHandle() : VK_NULL_HANDLE;
}

bool vvl::ImageDescriptor::AddParent(StateObject *state_object) {
    bool result = false;
    if (image_view_state_) {
        result = image_view_state_->AddParent(state_object);
    }
    return result;
}
void vvl::ImageDescriptor::RemoveParent(StateObject *state_object) {
    if (image_view_state_) {
        image_view_state_->RemoveParent(state_object);
    }
}
void vvl::ImageDescriptor::InvalidateNode(const std::shared_ptr<StateObject> &invalid_node, bool unlink) {
    if (invalid_node == image_view_state_) {
        known_valid_view_ = false;
        if (unlink) {
            image_view_state_.reset();
        }
    }
}

bool vvl::ImageDescriptor::Invalid() const { return !known_valid_view_ && ComputeInvalid(); }
bool vvl::ImageDescriptor::ComputeInvalid() const { return !image_view_state_ || image_view_state_->Invalid(); }
void vvl::ImageDescriptor::UpdateKnownValidView(bool is_bindless) { known_valid_view_ = !is_bindless && !ComputeInvalid(); }

void vvl::BufferDescriptor::WriteUpdate(DescriptorSet &set_state, const vvl::Device &dev_data, const VkWriteDescriptorSet &update,
                                        const uint32_t index, bool is_bindless) {
    const auto &buffer_info = update.pBufferInfo[index];
    offset_ = buffer_info.offset;
    range_ = buffer_info.range;
    auto buffer_state = dev_data.GetConstCastShared<vvl::Buffer>(buffer_info.buffer);
    ReplaceStatePtr(set_state, buffer_state_, buffer_state, is_bindless);
}

void vvl::BufferDescriptor::CopyUpdate(DescriptorSet &set_state, const vvl::Device &dev_data, const Descriptor &src,
                                       bool is_bindless, VkDescriptorType src_type) {
    if (src.GetClass() == DescriptorClass::Mutable) {
        const auto &buff_desc = static_cast<const MutableDescriptor &>(src);
        offset_ = buff_desc.GetOffset();
        range_ = buff_desc.GetRange();
        ReplaceStatePtr(set_state, buffer_state_, buff_desc.GetSharedBufferState(), is_bindless);
        return;
    }
    const auto &buff_desc = static_cast<const BufferDescriptor &>(src);
    offset_ = buff_desc.offset_;
    range_ = buff_desc.range_;
    ReplaceStatePtr(set_state, buffer_state_, buff_desc.buffer_state_, is_bindless);
}

VkBuffer vvl::BufferDescriptor::GetBuffer() const { return buffer_state_ ? buffer_state_->VkHandle() : VK_NULL_HANDLE; }

bool vvl::BufferDescriptor::AddParent(StateObject *state_object) {
    bool result = false;
    if (buffer_state_) {
        result = buffer_state_->AddParent(state_object);
    }
    return result;
}
void vvl::BufferDescriptor::RemoveParent(StateObject *state_object) {
    if (buffer_state_) {
        buffer_state_->RemoveParent(state_object);
    }
}
bool vvl::BufferDescriptor::Invalid() const { return !buffer_state_ || buffer_state_->Invalid(); }

VkDeviceSize vvl::BufferDescriptor::GetEffectiveRange() const {
    // The buffer can be null if using nullDescriptors, if that is the case, the size/range will not be accessed
    if (range_ == VK_WHOLE_SIZE && buffer_state_) {
        // When range is VK_WHOLE_SIZE the effective range is calculated at vkUpdateDescriptorSets is by taking the size of buffer
        // minus the offset.
        return buffer_state_->create_info.size - offset_;
    } else {
        return range_;
    }
}

void vvl::TexelDescriptor::WriteUpdate(DescriptorSet &set_state, const vvl::Device &dev_data, const VkWriteDescriptorSet &update,
                                       const uint32_t index, bool is_bindless) {
    auto buffer_view = dev_data.GetConstCastShared<vvl::BufferView>(update.pTexelBufferView[index]);
    ReplaceStatePtr(set_state, buffer_view_state_, buffer_view, is_bindless);
}

void vvl::TexelDescriptor::CopyUpdate(DescriptorSet &set_state, const vvl::Device &dev_data, const Descriptor &src,
                                      bool is_bindless, VkDescriptorType src_type) {
    if (src.GetClass() == DescriptorClass::Mutable) {
        ReplaceStatePtr(set_state, buffer_view_state_, static_cast<const MutableDescriptor &>(src).GetSharedBufferViewState(),
                        is_bindless);
        return;
    }
    ReplaceStatePtr(set_state, buffer_view_state_, static_cast<const TexelDescriptor &>(src).buffer_view_state_, is_bindless);
}

VkBufferView vvl::TexelDescriptor::GetBufferView() const {
    return buffer_view_state_ ? buffer_view_state_->VkHandle() : VK_NULL_HANDLE;
}

bool vvl::TexelDescriptor::AddParent(StateObject *state_object) {
    bool result = false;
    if (buffer_view_state_) {
        result = buffer_view_state_->AddParent(state_object);
    }
    return result;
}
void vvl::TexelDescriptor::RemoveParent(StateObject *state_object) {
    if (buffer_view_state_) {
        buffer_view_state_->RemoveParent(state_object);
    }
}

bool vvl::TexelDescriptor::Invalid() const { return !buffer_view_state_ || buffer_view_state_->Invalid(); }

void vvl::AccelerationStructureDescriptor::WriteUpdate(DescriptorSet &set_state, const vvl::Device &dev_data,
                                                       const VkWriteDescriptorSet &update, const uint32_t index, bool is_bindless) {
    const auto *acc_info = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureKHR>(update.pNext);
    const auto *acc_info_nv = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureNV>(update.pNext);
    assert(acc_info || acc_info_nv);
    is_khr_ = (acc_info != NULL);
    if (is_khr_) {
        acc_ = acc_info->pAccelerationStructures[index];
        ReplaceStatePtr(set_state, acc_state_, dev_data.GetConstCastShared<vvl::AccelerationStructureKHR>(acc_), is_bindless);
    } else {
        acc_nv_ = acc_info_nv->pAccelerationStructures[index];
        ReplaceStatePtr(set_state, acc_state_nv_, dev_data.GetConstCastShared<vvl::AccelerationStructureNV>(acc_nv_), is_bindless);
    }
}

void vvl::AccelerationStructureDescriptor::CopyUpdate(DescriptorSet &set_state, const vvl::Device &dev_data, const Descriptor &src,
                                                      bool is_bindless, VkDescriptorType src_type) {
    if (src.GetClass() == DescriptorClass::Mutable) {
        auto &acc_desc = static_cast<const MutableDescriptor &>(src);
        is_khr_ = acc_desc.IsAccelerationStructureKHR();
        if (is_khr_) {
            acc_ = acc_desc.GetAccelerationStructureKHR();
            ReplaceStatePtr(set_state, acc_state_, dev_data.GetConstCastShared<vvl::AccelerationStructureKHR>(acc_), is_bindless);
        } else {
            acc_nv_ = acc_desc.GetAccelerationStructureNV();
            ReplaceStatePtr(set_state, acc_state_nv_, dev_data.GetConstCastShared<vvl::AccelerationStructureNV>(acc_nv_),
                            is_bindless);
        }
        return;
    }
    auto acc_desc = static_cast<const AccelerationStructureDescriptor &>(src);
    is_khr_ = acc_desc.is_khr_;
    if (is_khr_) {
        acc_ = acc_desc.acc_;
        ReplaceStatePtr(set_state, acc_state_, dev_data.GetConstCastShared<vvl::AccelerationStructureKHR>(acc_), is_bindless);
    } else {
        acc_nv_ = acc_desc.acc_nv_;
        ReplaceStatePtr(set_state, acc_state_nv_, dev_data.GetConstCastShared<vvl::AccelerationStructureNV>(acc_nv_), is_bindless);
    }
}

bool vvl::AccelerationStructureDescriptor::AddParent(StateObject *state_object) {
    bool result = false;
    if (acc_state_) {
        result |= acc_state_->AddParent(state_object);
    }
    if (acc_state_nv_) {
        result |= acc_state_nv_->AddParent(state_object);
    }
    return result;
}
void vvl::AccelerationStructureDescriptor::RemoveParent(StateObject *state_object) {
    if (acc_state_) {
        acc_state_->RemoveParent(state_object);
    }
    if (acc_state_nv_) {
        acc_state_nv_->RemoveParent(state_object);
    }
}
bool vvl::AccelerationStructureDescriptor::Invalid() const {
    if (is_khr_) {
        return !acc_state_ || acc_state_->Invalid();
    } else {
        return !acc_state_nv_ || acc_state_nv_->Invalid();
    }
}

vvl::MutableDescriptor::MutableDescriptor()
    : Descriptor(),
      buffer_size_(0),
      active_descriptor_type_(VK_DESCRIPTOR_TYPE_MUTABLE_EXT),
      immutable_(false),
      image_layout_(VK_IMAGE_LAYOUT_UNDEFINED),
      offset_(0),
      range_(0),
      is_khr_(false),
      acc_(VK_NULL_HANDLE) {}

void vvl::MutableDescriptor::WriteUpdate(DescriptorSet &set_state, const vvl::Device &dev_data, const VkWriteDescriptorSet &update,
                                         const uint32_t index, bool is_bindless) {
    VkDeviceSize buffer_size = 0;
    switch (DescriptorTypeToClass(update.descriptorType)) {
        case DescriptorClass::PlainSampler:
            if (!immutable_ && update.pImageInfo) {
                ReplaceStatePtr(set_state, sampler_state_,
                                dev_data.GetConstCastShared<vvl::Sampler>(update.pImageInfo[index].sampler), is_bindless);
            }
            break;
        case DescriptorClass::ImageSampler: {
            if (update.pImageInfo) {
                const auto &image_info = update.pImageInfo[index];
                if (!immutable_) {
                    ReplaceStatePtr(set_state, sampler_state_, dev_data.GetConstCastShared<vvl::Sampler>(image_info.sampler),
                                    is_bindless);
                }
                image_layout_ = image_info.imageLayout;
                ReplaceStatePtr(set_state, image_view_state_, dev_data.GetConstCastShared<vvl::ImageView>(image_info.imageView),
                                is_bindless);
            }
            break;
        }
        case DescriptorClass::Image: {
            if (update.pImageInfo) {
                const auto &image_info = update.pImageInfo[index];
                image_layout_ = image_info.imageLayout;
                ReplaceStatePtr(set_state, image_view_state_, dev_data.GetConstCastShared<vvl::ImageView>(image_info.imageView),
                                is_bindless);
            }
            break;
        }
        case DescriptorClass::GeneralBuffer: {
            if (update.pBufferInfo) {
                const auto &buffer_info = update.pBufferInfo[index];
                offset_ = buffer_info.offset;
                range_ = buffer_info.range;
                // can be null if using nullDescriptors
                const auto buffer_state = dev_data.GetConstCastShared<vvl::Buffer>(update.pBufferInfo->buffer);
                if (buffer_state) {
                    buffer_size = buffer_state->create_info.size;
                }
                ReplaceStatePtr(set_state, buffer_state_, buffer_state, is_bindless);
            }
            break;
        }
        case DescriptorClass::TexelBuffer: {
            if (update.pTexelBufferView) {
                // can be null if using nullDescriptors
                const auto buffer_view = dev_data.GetConstCastShared<vvl::BufferView>(update.pTexelBufferView[index]);
                if (buffer_view) {
                    buffer_size = buffer_view->buffer_state->create_info.size;
                }
                ReplaceStatePtr(set_state, buffer_view_state_, buffer_view, is_bindless);
            }
            break;
        }
        case DescriptorClass::AccelerationStructure: {
            const auto *acc_info = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureKHR>(update.pNext);
            const auto *acc_info_nv = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureNV>(update.pNext);
            assert(acc_info || acc_info_nv);
            is_khr_ = (acc_info != NULL);
            if (is_khr_) {
                acc_ = acc_info->pAccelerationStructures[index];
                ReplaceStatePtr(set_state, acc_state_, dev_data.GetConstCastShared<vvl::AccelerationStructureKHR>(acc_),
                                is_bindless);
            } else {
                acc_nv_ = acc_info_nv->pAccelerationStructures[index];
                ReplaceStatePtr(set_state, acc_state_nv_, dev_data.GetConstCastShared<vvl::AccelerationStructureNV>(acc_nv_),
                                is_bindless);
            }
            break;
        }
        case DescriptorClass::InlineUniform:
        case DescriptorClass::Mutable:
        case DescriptorClass::Invalid:
            break;
    }
    SetDescriptorType(update.descriptorType, buffer_size);
}

void vvl::MutableDescriptor::CopyUpdate(DescriptorSet &set_state, const vvl::Device &dev_data, const Descriptor &src,
                                        bool is_bindless, VkDescriptorType src_type) {
    VkDeviceSize buffer_size = 0;
    switch (src.GetClass()) {
        case DescriptorClass::PlainSampler: {
            auto &sampler_src = static_cast<const SamplerDescriptor &>(src);
            if (!immutable_) {
                ReplaceStatePtr(set_state, sampler_state_, sampler_src.GetSharedSamplerState(), is_bindless);
            }
            break;
        }
        case DescriptorClass::ImageSampler: {
            auto &image_src = static_cast<const ImageSamplerDescriptor &>(src);
            if (!immutable_) {
                ReplaceStatePtr(set_state, sampler_state_, image_src.GetSharedSamplerState(), is_bindless);
            }

            image_layout_ = image_src.GetImageLayout();
            ReplaceStatePtr(set_state, image_view_state_, image_src.GetSharedImageViewState(), is_bindless);
            break;
        }
        case DescriptorClass::Image: {
            auto &image_src = static_cast<const ImageDescriptor &>(src);

            image_layout_ = image_src.GetImageLayout();
            ReplaceStatePtr(set_state, image_view_state_, image_src.GetSharedImageViewState(), is_bindless);
            break;
        }
        case DescriptorClass::TexelBuffer: {
            ReplaceStatePtr(set_state, buffer_view_state_, static_cast<const TexelDescriptor &>(src).GetSharedBufferViewState(),
                            is_bindless);
            buffer_size = buffer_view_state_ ? buffer_view_state_->Size() : vvl::kU32Max;
            break;
        }
        case DescriptorClass::GeneralBuffer: {
            const auto buff_desc = static_cast<const BufferDescriptor &>(src);
            offset_ = buff_desc.GetOffset();
            range_ = buff_desc.GetRange();
            ReplaceStatePtr(set_state, buffer_state_, buff_desc.GetSharedBufferState(), is_bindless);
            buffer_size = range_;
            break;
        }
        case DescriptorClass::AccelerationStructure: {
            auto &acc_desc = static_cast<const AccelerationStructureDescriptor &>(src);
            if (is_khr_) {
                acc_ = acc_desc.GetAccelerationStructure();
                ReplaceStatePtr(set_state, acc_state_, dev_data.GetConstCastShared<vvl::AccelerationStructureKHR>(acc_),
                                is_bindless);
            } else {
                acc_nv_ = acc_desc.GetAccelerationStructureNV();
                ReplaceStatePtr(set_state, acc_state_nv_, dev_data.GetConstCastShared<vvl::AccelerationStructureNV>(acc_nv_),
                                is_bindless);
            }
            break;
        }
        case DescriptorClass::Mutable: {
            const auto &mutable_src = static_cast<const MutableDescriptor &>(src);
            auto active_class = DescriptorTypeToClass(mutable_src.ActiveType());
            switch (active_class) {
                case DescriptorClass::PlainSampler: {
                    if (!immutable_) {
                        ReplaceStatePtr(set_state, sampler_state_, mutable_src.GetSharedSamplerState(), is_bindless);
                    }
                } break;
                case DescriptorClass::ImageSampler: {
                    if (!immutable_) {
                        ReplaceStatePtr(set_state, sampler_state_, mutable_src.GetSharedSamplerState(), is_bindless);
                    }

                    image_layout_ = mutable_src.GetImageLayout();
                    ReplaceStatePtr(set_state, image_view_state_, mutable_src.GetSharedImageViewState(), is_bindless);
                } break;
                case DescriptorClass::Image: {
                    image_layout_ = mutable_src.GetImageLayout();
                    ReplaceStatePtr(set_state, image_view_state_, mutable_src.GetSharedImageViewState(), is_bindless);
                } break;
                case DescriptorClass::GeneralBuffer: {
                    offset_ = mutable_src.GetOffset();
                    range_ = mutable_src.GetRange();
                    ReplaceStatePtr(set_state, buffer_state_, mutable_src.GetSharedBufferState(), is_bindless);
                } break;
                case DescriptorClass::TexelBuffer: {
                    ReplaceStatePtr(set_state, buffer_view_state_, mutable_src.GetSharedBufferViewState(), is_bindless);
                } break;
                case DescriptorClass::AccelerationStructure: {
                    if (mutable_src.IsKHR()) {
                        acc_ = mutable_src.GetAccelerationStructureKHR();
                        ReplaceStatePtr(set_state, acc_state_, dev_data.GetConstCastShared<vvl::AccelerationStructureKHR>(acc_),
                                        is_bindless);
                    } else {
                        acc_nv_ = mutable_src.GetAccelerationStructureNV();
                        ReplaceStatePtr(set_state, acc_state_nv_,
                                        dev_data.GetConstCastShared<vvl::AccelerationStructureNV>(acc_nv_), is_bindless);
                    }

                } break;
                case DescriptorClass::InlineUniform:
                case DescriptorClass::Mutable:
                case DescriptorClass::Invalid:
                    break;
            }
            buffer_size = mutable_src.GetBufferSize();
            break;
        }
        case vvl::DescriptorClass::InlineUniform:
        case vvl::DescriptorClass::Invalid:
            break;
    }
    SetDescriptorType(src_type, buffer_size);
}

void vvl::MutableDescriptor::SetDescriptorType(VkDescriptorType type, VkDeviceSize buffer_size) {
    active_descriptor_type_ = type;
    buffer_size_ = buffer_size;
}

VkDeviceSize vvl::MutableDescriptor::GetEffectiveRange() const {
    // The buffer can be null if using nullDescriptors, if that is the case, the size/range will not be accessed
    if (range_ == VK_WHOLE_SIZE && buffer_state_) {
        // When range is VK_WHOLE_SIZE the effective range is calculated at vkUpdateDescriptorSets is by taking the size of buffer
        // minus the offset.
        return buffer_state_->create_info.size - offset_;
    } else {
        return range_;
    }
}

void vvl::MutableDescriptor::UpdateDrawState(vvl::CommandBuffer &cb_state) {
    const vvl::DescriptorClass active_class = ActiveClass();
    if (active_class == DescriptorClass::Image || active_class == DescriptorClass::ImageSampler) {
        if (image_view_state_) {
            cb_state.SetImageViewInitialLayout(*image_view_state_, image_layout_);
        }
    }
}

bool vvl::MutableDescriptor::AddParent(StateObject *state_object) {
    bool result = false;
    const vvl::DescriptorClass active_class = ActiveClass();
    switch (active_class) {
        case DescriptorClass::PlainSampler:
            if (sampler_state_) {
                result |= sampler_state_->AddParent(state_object);
            }
            break;
        case DescriptorClass::ImageSampler:
            if (sampler_state_) {
                result |= sampler_state_->AddParent(state_object);
            }
            if (image_view_state_) {
                result = image_view_state_->AddParent(state_object);
            }
            break;
        case DescriptorClass::TexelBuffer:
            if (buffer_view_state_) {
                result = buffer_view_state_->AddParent(state_object);
            }
            break;
        case DescriptorClass::Image:
            if (image_view_state_) {
                result = image_view_state_->AddParent(state_object);
            }
            break;
        case DescriptorClass::GeneralBuffer:
            if (buffer_state_) {
                result = buffer_state_->AddParent(state_object);
            }
            break;
        case DescriptorClass::AccelerationStructure:
            if (acc_state_) {
                result |= acc_state_->AddParent(state_object);
            }
            if (acc_state_nv_) {
                result |= acc_state_nv_->AddParent(state_object);
            }
            break;
        case DescriptorClass::InlineUniform:
        case DescriptorClass::Mutable:
        case DescriptorClass::Invalid:
            break;
    }
    return result;
}
void vvl::MutableDescriptor::RemoveParent(StateObject *state_object) {
    if (sampler_state_) {
        sampler_state_->RemoveParent(state_object);
    }
    if (image_view_state_) {
        image_view_state_->RemoveParent(state_object);
    }
    if (buffer_view_state_) {
        buffer_view_state_->RemoveParent(state_object);
    }
    if (buffer_state_) {
        buffer_state_->RemoveParent(state_object);
    }
    if (acc_state_) {
        acc_state_->RemoveParent(state_object);
    }
    if (acc_state_nv_) {
        acc_state_nv_->RemoveParent(state_object);
    }
}

bool vvl::MutableDescriptor::Invalid() const {
    switch (ActiveClass()) {
        case DescriptorClass::PlainSampler:
            return !sampler_state_ || sampler_state_->Destroyed();

        case DescriptorClass::ImageSampler:
            return !sampler_state_ || sampler_state_->Invalid() || !image_view_state_ || image_view_state_->Invalid();

        case DescriptorClass::TexelBuffer:
            return !buffer_view_state_ || buffer_view_state_->Invalid();

        case DescriptorClass::Image:
            return !image_view_state_ || image_view_state_->Invalid();

        case DescriptorClass::GeneralBuffer:
            return !buffer_state_ || buffer_state_->Invalid();

        case DescriptorClass::AccelerationStructure:
            if (is_khr_) {
                return !acc_state_ || acc_state_->Invalid();
            } else {
                return !acc_state_nv_ || acc_state_nv_->Invalid();
            }
        case DescriptorClass::InlineUniform:
        case DescriptorClass::Mutable:
        case DescriptorClass::Invalid:
            break;
    }
    return false;
}

std::string vvl::DslErrorSource::PrintMessage(const Logger &error_logger) const {
    std::stringstream msg;
    msg << " (The VkDescriptorSetLayout was used to ";
    if (pipeline_layout_handle_ == VK_NULL_HANDLE) {
        msg << "allocate " << error_logger.FormatHandle(ds_handle_);
    } else {
        msg << "create " << error_logger.FormatHandle(pipeline_layout_handle_) << " at pSetLayouts[" << set_ << "]";
    }
    msg << ")";
    return msg.str();
}
