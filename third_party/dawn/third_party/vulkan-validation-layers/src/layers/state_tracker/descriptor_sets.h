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

#pragma once

#include "state_tracker/state_object.h"
#include "utils/hash_util.h"
#include "state_tracker/shader_stage_state.h"
#include "generated/vk_object_types.h"
#include "generated/error_location_helper.h"
#include <vulkan/utility/vk_safe_struct.hpp>
#include <map>
#include <set>
#include <vector>

class CoreChecks;
struct DeviceExtensions;

namespace vvl {
class Sampler;
class DescriptorSet;
class Device;
class CommandBuffer;
class ImageView;
class Buffer;
class BufferView;
class Pipeline;
class AccelerationStructureNV;
class AccelerationStructureKHR;
struct AllocateDescriptorSetsData;

// "bindless" does not have a concrete definition, but we use it as means to know:
// "is GPU-AV going to have to validate this or not"
// (see docs/gpu_av_descriptor_indexing.md for more details)
static inline bool IsBindless(VkDescriptorBindingFlags flags) {
    return (flags & (VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT)) != 0;
}

class DescriptorPool : public StateObject {
  public:
    DescriptorPool(Device &dev, const VkDescriptorPool handle, const VkDescriptorPoolCreateInfo *pCreateInfo);
    ~DescriptorPool() { Destroy(); }

    VkDescriptorPool VkHandle() const { return handle_.Cast<VkDescriptorPool>(); };

    void Allocate(const VkDescriptorSetAllocateInfo *alloc_info, const VkDescriptorSet *descriptor_sets,
                  const vvl::AllocateDescriptorSetsData &ds_data);
    void Free(uint32_t count, const VkDescriptorSet *descriptor_sets);
    void Reset();
    void Destroy() override;

    const VulkanTypedHandle *InUse() const override;
    uint32_t GetAvailableCount(uint32_t type) const {
        auto guard = ReadLock();
        auto iter = available_counts_.find(type);
        return iter != available_counts_.end() ? iter->second : 0;
    }

    // The type map is only created once so can guarantee this will find if type was used
    // Unlike GetAvailableCount, this won't give a false positive that it just ran out of an available count
    bool IsAvailableType(uint32_t type) const {
        auto guard = ReadLock();
        return available_counts_.find(type) != available_counts_.end();
    }

    uint32_t GetAvailableSets() const {
        auto guard = ReadLock();
        return available_sets_;
    }

    const vku::safe_VkDescriptorPoolCreateInfo safe_create_info;
    const VkDescriptorPoolCreateInfo &create_info;

    const uint32_t maxSets;  // Max descriptor sets allowed in this pool
    using TypeCountMap = vvl::unordered_map<uint32_t, uint32_t>;
    const TypeCountMap maxDescriptorTypeCount;  // Max # of descriptors of each type in this pool

  protected:
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }
    uint32_t available_sets_;        // Available descriptor sets in this pool
    TypeCountMap available_counts_;  // Available # of descriptors of each type in this pool
    vvl::unordered_map<VkDescriptorSet, vvl::DescriptorSet *> sets_;  // Collection of all sets in this pool
    Device &dev_data_;
    mutable std::shared_mutex lock_;
};

class DescriptorUpdateTemplate : public StateObject {
  public:
    const vku::safe_VkDescriptorUpdateTemplateCreateInfo safe_create_info;
    const VkDescriptorUpdateTemplateCreateInfo &create_info;

    DescriptorUpdateTemplate(VkDescriptorUpdateTemplate handle, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo)
        : StateObject(handle, kVulkanObjectTypeDescriptorUpdateTemplate),
          safe_create_info(pCreateInfo),
          create_info(*safe_create_info.ptr()) {}

    VkDescriptorUpdateTemplate VkHandle() const { return handle_.Cast<VkDescriptorUpdateTemplate>(); };
};

// Utility structs/classes/types
// Index range for global indices below, end is exclusive, i.e. [start,end)
struct IndexRange {
    IndexRange(uint32_t start_in, uint32_t end_in) : start(start_in), end(end_in) {}
    IndexRange() = default;
    uint32_t start;
    uint32_t end;
};

/*
 * DescriptorSetLayoutDef/DescriptorSetLayout classes
 *
 * Overview - These two classes encapsulate the Vulkan VkDescriptorSetLayout data (layout).
 *   A layout consists of some number of bindings, each of which has a binding#, a
 *   type, descriptor count, stage flags, and pImmutableSamplers.

 *   The DescriptorSetLayoutDef represents a canonicalization of the input data and contains
 *   neither per handle or per device state.  It is possible for different handles on
 *   different devices to share a common def.  This is used and useful for quick compatibiltiy
 *   validation.  The DescriptorSetLayout refers to a DescriptorSetLayoutDef and contains
 *   all per handle state.
 *
 * Index vs Binding - A layout is created with an array of VkDescriptorSetLayoutBinding
 *  where each array index will have a corresponding binding# that is defined in that struct.
 *  The binding#, then, is decoupled from VkDescriptorSetLayoutBinding index, which allows
 *  bindings to be defined out-of-order. This DescriptorSetLayout class, however, stores
 *  the bindings internally in-order. This is useful for operations which may "roll over"
 *  from a single binding to the next consecutive binding.
 *
 *  Note that although the bindings are stored in-order, there still may be "gaps" in the
 *  binding#. For example, if the binding creation order is 8, 7, 10, 3, 4, then the
 *  internal binding array will have five entries stored in binding order 3, 4, 7, 8, 10.
 *  To process all of the bindings in a layout you can iterate from 0 to GetBindingCount()
 *  and use the Get*FromIndex() functions for each index. To just process a single binding,
 *  use the Get*FromBinding() functions.
 *
 * Global Index - The binding vector index has as many indices as there are bindings.
 *  This class also has the concept of a Global Index. For the global index functions,
 *  there are as many global indices as there are descriptors in the layout.
 *  For the global index, consider all of the bindings to be a flat array where
 *  descriptor 0 of of the lowest binding# is index 0 and each descriptor in the layout
 *  increments from there. So if the lowest binding# in this example had descriptorCount of
 *  10, then the GlobalStartIndex of the 2nd lowest binding# will be 10 where 0-9 are the
 *  global indices for the lowest binding#.
 */
class DescriptorSetLayoutDef {
  public:
    // Constructors and destructor
    DescriptorSetLayoutDef(const VkDescriptorSetLayoutCreateInfo *p_create_info);
    size_t hash() const;

    uint32_t GetTotalDescriptorCount() const { return descriptor_count_; };
    uint32_t GetNonInlineDescriptorCount() const { return non_inline_descriptor_count_; };
    uint32_t GetDynamicDescriptorCount() const { return dynamic_descriptor_count_; };
    VkDescriptorSetLayoutCreateFlags GetCreateFlags() const { return flags_; }
    // For a given binding, return the number of descriptors in that binding and all successive bindings
    uint32_t GetBindingCount() const { return binding_count_; };
    // Return true if given binding is present in this layout
    bool HasBinding(const uint32_t binding) const { return binding_to_index_map_.count(binding) > 0; };
    // Return true if binding 1 beyond given exists and has same type, stageFlags & immutable sampler use
    uint32_t GetIndexFromBinding(uint32_t binding) const;
    // Various Get functions that can either be passed a binding#, which will
    //  be automatically translated into the appropriate index, or the index# can be passed in directly
    uint32_t GetMaxBinding() const {
        assert(!bindings_.empty());
        return bindings_.empty() ? 0 : bindings_[bindings_.size() - 1].binding;
    }
    VkDescriptorSetLayoutBinding const *GetDescriptorSetLayoutBindingPtrFromIndex(const uint32_t) const;
    VkDescriptorSetLayoutBinding const *GetDescriptorSetLayoutBindingPtrFromBinding(uint32_t binding) const {
        return GetDescriptorSetLayoutBindingPtrFromIndex(GetIndexFromBinding(binding));
    }
    const std::vector<vku::safe_VkDescriptorSetLayoutBinding> &GetBindings() const { return bindings_; }
    const VkDescriptorSetLayoutBinding *GetBindingInfoFromIndex(const uint32_t index) const { return bindings_[index].ptr(); }
    const VkDescriptorSetLayoutBinding *GetBindingInfoFromBinding(const uint32_t binding) const {
        return GetBindingInfoFromIndex(GetIndexFromBinding(binding));
    }
    const std::vector<VkDescriptorBindingFlags> &GetBindingFlags() const { return binding_flags_; }
    uint32_t GetDescriptorCountFromIndex(const uint32_t) const;
    uint32_t GetDescriptorCountFromBinding(const uint32_t binding) const {
        return GetDescriptorCountFromIndex(GetIndexFromBinding(binding));
    }
    VkDescriptorType GetTypeFromIndex(const uint32_t) const;
    VkDescriptorType GetTypeFromBinding(const uint32_t binding) const { return GetTypeFromIndex(GetIndexFromBinding(binding)); }

    VkDescriptorBindingFlags GetDescriptorBindingFlagsFromIndex(const uint32_t) const;
    VkDescriptorBindingFlags GetDescriptorBindingFlagsFromBinding(const uint32_t binding) const {
        return GetDescriptorBindingFlagsFromIndex(GetIndexFromBinding(binding));
    }
    VkSampler const *GetImmutableSamplerPtrFromIndex(const uint32_t) const;
    bool IsTypeMutable(const VkDescriptorType type, uint32_t binding) const;
    const std::vector<VkDescriptorType> &GetMutableTypes(uint32_t binding) const;
    std::string PrintMutableTypes(uint32_t binding) const;
    // For a particular binding, get the global index range
    //  This call should be guarded by a call to "HasBinding(binding)" to verify that the given binding exists
    const IndexRange &GetGlobalIndexRangeFromBinding(const uint32_t) const;
    const vvl::IndexRange &GetGlobalIndexRangeFromIndex(uint32_t index) const;

    // Helper function to get the next valid binding for a descriptor
    uint32_t GetNextValidBinding(const uint32_t) const;
    bool IsPushDescriptor() const { return GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT; };

    struct BindingTypeStats {
        uint32_t dynamic_buffer_count;
        uint32_t non_dynamic_buffer_count;
    };
    const BindingTypeStats &GetBindingTypeStats() const { return binding_type_stats_; }

    std::string DescribeDifference(uint32_t index, const DescriptorSetLayoutDef &other) const;

  private:
    // Only the first three data members are used for hash and equality checks, the other members are derived from them, and are
    // used to speed up the various lookups/queries/validations
    VkDescriptorSetLayoutCreateFlags flags_;
    std::vector<vku::safe_VkDescriptorSetLayoutBinding> bindings_;
    std::vector<VkDescriptorBindingFlags> binding_flags_;
    // List of mutable types for each binding: [binding][mutable type]
    std::vector<std::vector<VkDescriptorType>> mutable_types_;

    // Convenience data structures for rapid lookup of various descriptor set layout properties
    std::set<uint32_t> non_empty_bindings_;  // Containing non-emtpy bindings in numerical order
    vvl::unordered_map<uint32_t, uint32_t> binding_to_index_map_;
    // The following map allows an non-iterative lookup of a binding from a global index...
    std::vector<IndexRange> global_index_range_;  // range is exclusive of .end

    uint32_t binding_count_;     // # of bindings in this layout
    // total # descriptors in this layout (used to check if two layouts are the same or not)
    uint32_t descriptor_count_;
    // only counts INLINE_UNIFORM_BLOCK descriptors as one.
    // When using Inline Uniform Block, each descriptor is the number of bytes, which can skew descriptor_count_
    uint32_t non_inline_descriptor_count_;
    uint32_t dynamic_descriptor_count_;
    BindingTypeStats binding_type_stats_;
};

// Canonical dictionary of DSL definitions -- independent of device or handle
using DescriptorSetLayoutDict = hash_util::Dictionary<DescriptorSetLayoutDef, hash_util::HasHashMember<DescriptorSetLayoutDef>>;
using DescriptorSetLayoutId = DescriptorSetLayoutDict::Id;

// Compare is in header and static because hash_util KeyValueEqual need the symbol at compile time
static inline bool operator==(const DescriptorSetLayoutDef &lhs, const DescriptorSetLayoutDef &rhs) {
    // trivial types
    if ((lhs.GetCreateFlags() != rhs.GetCreateFlags()) || (lhs.GetBindingFlags() != rhs.GetBindingFlags())) {
        return false;
    }
    // vectors of vku::safe_VkDescriptorSetLayoutBinding structures
    const auto &lhs_bindings = lhs.GetBindings();
    const auto &rhs_bindings = rhs.GetBindings();
    if (lhs_bindings.size() != rhs_bindings.size()) {
        return false;
    }
    for (uint32_t i = 0; i < lhs_bindings.size(); i++) {
        const auto &l = lhs_bindings[i];
        const auto &r = rhs_bindings[i];
        // For things where we are comparing with the bound pipeline, the binding will always be right, but when comparing two
        // arbitrary layouts (ex. templates, Device Generated Commands, etc) the bindings might be different
        if (l.binding != r.binding) {
            return false;
        }
        if (l.descriptorType != r.descriptorType || l.descriptorCount != r.descriptorCount || l.stageFlags != r.stageFlags) {
            return false;
        }
        if (l.pImmutableSamplers != r.pImmutableSamplers) {
            return false;
        }
        if (l.pImmutableSamplers) {
            for (uint32_t s = 0; s < l.descriptorCount; s++) {
                if (l.pImmutableSamplers[s] != r.pImmutableSamplers[s]) {
                    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8497
                    // This just checks pointers, but two different VkSampler handles could be created with same createInfo.
                    // Since this is rare enough, mark as "not the same" and check later when checking for compatibility.
                    return false;
                }
            }
        }
        // These have been sorted already so can direct compare
        if (lhs.GetMutableTypes(i) != rhs.GetMutableTypes(i)) {
            return false;
        }
    }
    return true;
}

class DescriptorSetLayout : public StateObject {
  public:
    // Constructors and destructor
    DescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo *pCreateInfo, const VkDescriptorSetLayout handle);
    virtual ~DescriptorSetLayout() { Destroy(); }

    bool HasBinding(const uint32_t binding) const { return layout_id_->HasBinding(binding); }
    // Return true if this layout is compatible with passed in layout from a pipelineLayout,
    //   else return false and update error_msg with description of incompatibility
    // Return true if this layout is compatible with passed in layout
    bool IsCompatible(DescriptorSetLayout const *rh_ds_layout) const;
    // Straightforward Get functions
    VkDescriptorSetLayout VkHandle() const { return handle_.Cast<VkDescriptorSetLayout>(); };
    const DescriptorSetLayoutDef *GetLayoutDef() const { return layout_id_.get(); }
    DescriptorSetLayoutId GetLayoutId() const { return layout_id_; }
    uint32_t GetTotalDescriptorCount() const { return layout_id_->GetTotalDescriptorCount(); };
    uint32_t GetNonInlineDescriptorCount() const { return layout_id_->GetNonInlineDescriptorCount(); };
    uint32_t GetDynamicDescriptorCount() const { return layout_id_->GetDynamicDescriptorCount(); };
    uint32_t GetBindingCount() const { return layout_id_->GetBindingCount(); };
    VkDescriptorSetLayoutCreateFlags GetCreateFlags() const { return layout_id_->GetCreateFlags(); }
    uint32_t GetIndexFromBinding(uint32_t binding) const { return layout_id_->GetIndexFromBinding(binding); }
    // Various Get functions that can either be passed a binding#, which will
    //  be automatically translated into the appropriate index, or the index# can be passed in directly
    uint32_t GetMaxBinding() const { return layout_id_->GetMaxBinding(); }
    VkDescriptorSetLayoutBinding const *GetDescriptorSetLayoutBindingPtrFromIndex(const uint32_t index) const {
        return layout_id_->GetDescriptorSetLayoutBindingPtrFromIndex(index);
    }
    VkDescriptorSetLayoutBinding const *GetDescriptorSetLayoutBindingPtrFromBinding(uint32_t binding) const {
        return layout_id_->GetDescriptorSetLayoutBindingPtrFromBinding(binding);
    }
    const std::vector<vku::safe_VkDescriptorSetLayoutBinding> &GetBindings() const { return layout_id_->GetBindings(); }
    uint32_t GetDescriptorCountFromIndex(const uint32_t index) const { return layout_id_->GetDescriptorCountFromIndex(index); }
    uint32_t GetDescriptorCountFromBinding(const uint32_t binding) const {
        return layout_id_->GetDescriptorCountFromBinding(binding);
    }
    VkDescriptorType GetTypeFromIndex(const uint32_t index) const { return layout_id_->GetTypeFromIndex(index); }
    VkDescriptorType GetTypeFromBinding(const uint32_t binding) const { return layout_id_->GetTypeFromBinding(binding); }

    VkDescriptorBindingFlags GetDescriptorBindingFlagsFromIndex(const uint32_t index) const {
        return layout_id_->GetDescriptorBindingFlagsFromIndex(index);
    }
    VkDescriptorBindingFlags GetDescriptorBindingFlagsFromBinding(const uint32_t binding) const {
        return layout_id_->GetDescriptorBindingFlagsFromBinding(binding);
    }
    VkSampler const *GetImmutableSamplerPtrFromIndex(const uint32_t index) const {
        return layout_id_->GetImmutableSamplerPtrFromIndex(index);
    }
    bool IsTypeMutable(const VkDescriptorType type, uint32_t binding) const { return layout_id_->IsTypeMutable(type, binding); }
    const std::vector<VkDescriptorType> &GetMutableTypes(uint32_t binding) const { return layout_id_->GetMutableTypes(binding); }
    std::string PrintMutableTypes(uint32_t binding) const { return layout_id_->PrintMutableTypes(binding); }
    // For a particular binding, get the global index range
    //  This call should be guarded by a call to "HasBinding(binding)" to verify that the given binding exists
    const IndexRange &GetGlobalIndexRangeFromBinding(const uint32_t binding) const {
        return layout_id_->GetGlobalIndexRangeFromBinding(binding);
    }
    const IndexRange &GetGlobalIndexRangeFromIndex(uint32_t index) const { return layout_id_->GetGlobalIndexRangeFromIndex(index); }

    // Helper function to get the next valid binding for a descriptor
    uint32_t GetNextValidBinding(const uint32_t binding) const { return layout_id_->GetNextValidBinding(binding); }
    bool IsPushDescriptor() const { return layout_id_->IsPushDescriptor(); }
    bool IsVariableDescriptorCountFromIndex(uint32_t index) const {
        return !!(GetDescriptorBindingFlagsFromIndex(index) & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT);
    }
    bool IsVariableDescriptorCount(uint32_t binding) const {
        return IsVariableDescriptorCountFromIndex(GetIndexFromBinding(binding));
    }
    void SetLayoutSizeInBytes(const VkDeviceSize *layout_size_in_bytes_);
    const VkDeviceSize *GetLayoutSizeInBytes() const;

    using BindingTypeStats = DescriptorSetLayoutDef::BindingTypeStats;
    const BindingTypeStats &GetBindingTypeStats() const { return layout_id_->GetBindingTypeStats(); }

  private:
    DescriptorSetLayoutId layout_id_;
    std::unique_ptr<VkDeviceSize> layout_size_in_bytes;
};

// Slightly broader than type, each c++ "class" will has a corresponding "DescriptorClass"
enum class DescriptorClass {
    PlainSampler,           // SAMPLER
    ImageSampler,           // COMBINED_IMAGE_SAMPLER
    Image,                  // SAMPLED_IMAGE/STORAGE_IMAGE/INPUT_ATTACHMENT
    TexelBuffer,            // UNIFORM_TEXEL_BUFFER/VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
    GeneralBuffer,          // UNIFORM_BUFFER/VK_DESCRIPTOR_TYPE_STORAGE_BUFFER (and dynamic version)
    InlineUniform,          // INLINE_UNIFORM_BLOCK
    AccelerationStructure,  // ACCELERATION_STRUCTURE
    Mutable,                // MUTABLE
    Invalid
};

DescriptorClass DescriptorTypeToClass(VkDescriptorType type);

class DescriptorSet;

// Descriptor is an abstract base class from which many separate descriptor types are derived.
// This allows the WriteUpdate() and CopyUpdate() operations to be specialized per descriptor type, but all descriptors in a set can
// be accessed via the common Descriptor.
class Descriptor {
  public:
    static bool SupportsNotifyInvalidate() { return false; }
    static bool IsNotifyInvalidateType(VulkanObjectType) { return false; }
    virtual void InvalidateNode(const std::shared_ptr<StateObject> &, bool) {}  // Most descriptor types will not call

    Descriptor() {}
    virtual ~Descriptor() {}
    virtual void WriteUpdate(DescriptorSet &set_state, const Device &dev_data, const VkWriteDescriptorSet &, const uint32_t,
                             bool is_bindless) = 0;
    virtual void CopyUpdate(DescriptorSet &set_state, const Device &dev_data, const Descriptor &, bool is_bindless,
                            VkDescriptorType type) = 0;
    virtual DescriptorClass GetClass() const = 0;
    // Special fast-path check for SamplerDescriptors that are immutable
    virtual bool IsImmutableSampler() const { return false; };
    virtual bool AddParent(StateObject *state_object) { return false; }
    virtual void RemoveParent(StateObject *state_object) {}

    virtual void UpdateDrawState(vvl::CommandBuffer &cb_state) {}

    // return true if resources used by this descriptor are destroyed or otherwise missing
    virtual bool Invalid() const { return false; }
};

// All Dynamic descriptor types
inline bool IsDynamicDescriptor(VkDescriptorType type) {
    return ((type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) || (type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC));
}

inline bool IsBufferDescriptor(VkDescriptorType type) {
    return ((type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) || (type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) ||
            (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) || (type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER));
}

class SamplerDescriptor : public Descriptor {
  public:
    SamplerDescriptor() = default;
    DescriptorClass GetClass() const override { return DescriptorClass::PlainSampler; }
    void WriteUpdate(DescriptorSet &set_state, const Device &dev_data, const VkWriteDescriptorSet &, const uint32_t,
                     bool is_bindless) override;
    void CopyUpdate(DescriptorSet &set_state, const Device &dev_data, const Descriptor &, bool is_bindless,
                    VkDescriptorType type) override;
    virtual bool IsImmutableSampler() const override { return immutable_; };
    VkSampler GetSampler() const;

    void SetImmutableSampler(std::shared_ptr<vvl::Sampler> &&state);
    const vvl::Sampler *GetSamplerState() const { return sampler_state_.get(); }
    vvl::Sampler *GetSamplerState() { return sampler_state_.get(); }
    std::shared_ptr<vvl::Sampler> GetSharedSamplerState() const { return sampler_state_; }

    bool AddParent(StateObject *state_object) override;
    void RemoveParent(StateObject *state_object) override;
    bool Invalid() const override;

  private:
    bool immutable_{false};
    std::shared_ptr<vvl::Sampler> sampler_state_;
};

class ImageDescriptor : public Descriptor {
  public:
    static bool SupportsNotifyInvalidate() { return true; }
    static bool IsNotifyInvalidateType(const VulkanObjectType node_type) {
        return node_type == VulkanObjectType::kVulkanObjectTypeImageView;
    }
    ImageDescriptor() = default;
    DescriptorClass GetClass() const override { return DescriptorClass::Image; }
    void WriteUpdate(DescriptorSet &set_state, const Device &dev_data, const VkWriteDescriptorSet &, const uint32_t,
                     bool is_bindless) override;
    void CopyUpdate(DescriptorSet &set_state, const Device &dev_data, const Descriptor &, bool is_bindless,
                    VkDescriptorType type) override;
    void UpdateDrawState(vvl::CommandBuffer &cb_state) override;
    VkImageView GetImageView() const;
    const vvl::ImageView *GetImageViewState() const { return image_view_state_.get(); }
    vvl::ImageView *GetImageViewState() { return image_view_state_.get(); }
    std::shared_ptr<vvl::ImageView> GetSharedImageViewState() const { return image_view_state_; }
    VkImageLayout GetImageLayout() const { return image_layout_; }

    bool AddParent(StateObject *state_object) override;
    void RemoveParent(StateObject *state_object) override;
    void InvalidateNode(const std::shared_ptr<StateObject> &invalid_node, bool unlink) override;
    bool Invalid() const override;

  protected:
    bool ComputeInvalid() const;
    void UpdateKnownValidView(bool is_bindless);

    std::shared_ptr<vvl::ImageView> image_view_state_;
    VkImageLayout image_layout_{VK_IMAGE_LAYOUT_UNDEFINED};
    bool known_valid_view_ = false;
};

class ImageSamplerDescriptor : public ImageDescriptor {
  public:
    ImageSamplerDescriptor() = default;
    DescriptorClass GetClass() const override { return DescriptorClass::ImageSampler; }
    void WriteUpdate(DescriptorSet &set_state, const Device &dev_data, const VkWriteDescriptorSet &, const uint32_t,
                     bool is_bindless) override;
    void CopyUpdate(DescriptorSet &set_state, const Device &dev_data, const Descriptor &, bool is_bindless,
                    VkDescriptorType type) override;
    virtual bool IsImmutableSampler() const override { return immutable_; };
    VkSampler GetSampler() const;
    void SetImmutableSampler(std::shared_ptr<vvl::Sampler> &&state);
    const vvl::Sampler *GetSamplerState() const { return sampler_state_.get(); }
    vvl::Sampler *GetSamplerState() { return sampler_state_.get(); }
    std::shared_ptr<vvl::Sampler> GetSharedSamplerState() const { return sampler_state_; }

    bool AddParent(StateObject *state_object) override;
    void RemoveParent(StateObject *state_object) override;
    bool Invalid() const override;

  private:
    std::shared_ptr<vvl::Sampler> sampler_state_;
    bool immutable_{false};
};

class TexelDescriptor : public Descriptor {
  public:
    TexelDescriptor() = default;
    DescriptorClass GetClass() const override { return DescriptorClass::TexelBuffer; }
    void WriteUpdate(DescriptorSet &set_state, const Device &dev_data, const VkWriteDescriptorSet &, const uint32_t,
                     bool is_bindless) override;
    void CopyUpdate(DescriptorSet &set_state, const Device &dev_data, const Descriptor &, bool is_bindless,
                    VkDescriptorType type) override;
    VkBufferView GetBufferView() const;
    const vvl::BufferView *GetBufferViewState() const { return buffer_view_state_.get(); }
    vvl::BufferView *GetBufferViewState() { return buffer_view_state_.get(); }
    std::shared_ptr<vvl::BufferView> GetSharedBufferViewState() const { return buffer_view_state_; }

    bool AddParent(StateObject *state_object) override;
    void RemoveParent(StateObject *state_object) override;
    bool Invalid() const override;

  private:
    std::shared_ptr<vvl::BufferView> buffer_view_state_;
};

class BufferDescriptor : public Descriptor {
  public:
    BufferDescriptor() = default;
    DescriptorClass GetClass() const override { return DescriptorClass::GeneralBuffer; }
    void WriteUpdate(DescriptorSet &set_state, const Device &dev_data, const VkWriteDescriptorSet &, const uint32_t,
                     bool is_bindless) override;
    void CopyUpdate(DescriptorSet &set_state, const Device &dev_data, const Descriptor &, bool is_bindless,
                    VkDescriptorType type) override;
    VkBuffer GetBuffer() const;
    const vvl::Buffer *GetBufferState() const { return buffer_state_.get(); }
    vvl::Buffer *GetBufferState() { return buffer_state_.get(); }
    std::shared_ptr<vvl::Buffer> GetSharedBufferState() const { return buffer_state_; }
    VkDeviceSize GetOffset() const { return offset_; }
    VkDeviceSize GetRange() const { return range_; }
    VkDeviceSize GetEffectiveRange() const;

    bool AddParent(StateObject *state_object) override;
    void RemoveParent(StateObject *state_object) override;
    bool Invalid() const override;

  private:
    VkDeviceSize offset_{0};
    VkDeviceSize range_{0};
    std::shared_ptr<vvl::Buffer> buffer_state_;
};

class InlineUniformDescriptor : public Descriptor {
  public:
    InlineUniformDescriptor() = default;
    DescriptorClass GetClass() const override { return DescriptorClass::InlineUniform; }
    void WriteUpdate(DescriptorSet &set_state, const Device &dev_data, const VkWriteDescriptorSet &, const uint32_t,
                     bool is_bindless) override {}
    void CopyUpdate(DescriptorSet &set_state, const Device &dev_data, const Descriptor &, bool is_bindless,
                    VkDescriptorType type) override {}
};

class AccelerationStructureDescriptor : public Descriptor {
  public:
    AccelerationStructureDescriptor() = default;
    DescriptorClass GetClass() const override { return DescriptorClass::AccelerationStructure; }
    void WriteUpdate(DescriptorSet &set_state, const Device &dev_data, const VkWriteDescriptorSet &, const uint32_t,
                     bool is_bindless) override;
    VkAccelerationStructureKHR GetAccelerationStructure() const { return acc_; }
    const vvl::AccelerationStructureKHR *GetAccelerationStructureStateKHR() const { return acc_state_.get(); }
    vvl::AccelerationStructureKHR *GetAccelerationStructureStateKHR() { return acc_state_.get(); }
    VkAccelerationStructureNV GetAccelerationStructureNV() const { return acc_nv_; }
    const vvl::AccelerationStructureNV *GetAccelerationStructureStateNV() const { return acc_state_nv_.get(); }
    vvl::AccelerationStructureNV *GetAccelerationStructureStateNV() { return acc_state_nv_.get(); }
    void CopyUpdate(DescriptorSet &set_state, const Device &dev_data, const Descriptor &, bool is_bindless,
                    VkDescriptorType type) override;
    bool IsKHR() const { return is_khr_; }

    bool AddParent(StateObject *state_object) override;
    void RemoveParent(StateObject *state_object) override;
    bool Invalid() const override;

  private:
    bool is_khr_{false};
    VkAccelerationStructureKHR acc_{VK_NULL_HANDLE};
    std::shared_ptr<vvl::AccelerationStructureKHR> acc_state_;
    VkAccelerationStructureNV acc_nv_{VK_NULL_HANDLE};
    std::shared_ptr<vvl::AccelerationStructureNV> acc_state_nv_;
};

class MutableDescriptor : public Descriptor {
  public:
    MutableDescriptor();
    DescriptorClass GetClass() const override { return DescriptorClass::Mutable; }
    void WriteUpdate(DescriptorSet &set_state, const Device &dev_data, const VkWriteDescriptorSet &, const uint32_t,
                     bool is_bindless) override;
    void CopyUpdate(DescriptorSet &set_state, const Device &dev_data, const Descriptor &, bool is_bindless,
                    VkDescriptorType type) override;

    void SetDescriptorType(VkDescriptorType type, VkDeviceSize buffer_size);
    VkDeviceSize GetBufferSize() const { return buffer_size_; }

    std::shared_ptr<vvl::Sampler> GetSharedSamplerState() const { return sampler_state_; }
    std::shared_ptr<vvl::ImageView> GetSharedImageViewState() const { return image_view_state_; }
    VkImageLayout GetImageLayout() const { return image_layout_; }
    std::shared_ptr<vvl::Buffer> GetSharedBufferState() const { return buffer_state_; }
    VkDeviceSize GetOffset() const { return offset_; }
    VkDeviceSize GetRange() const { return range_; }
    VkDeviceSize GetEffectiveRange() const;
    std::shared_ptr<vvl::BufferView> GetSharedBufferViewState() const { return buffer_view_state_; }
    VkAccelerationStructureKHR GetAccelerationStructureKHR() const { return acc_; }
    const vvl::AccelerationStructureKHR *GetAccelerationStructureStateKHR() const { return acc_state_.get(); }
    vvl::AccelerationStructureKHR *GetAccelerationStructureStateKHR() { return acc_state_.get(); }
    VkAccelerationStructureNV GetAccelerationStructureNV() const { return acc_nv_; }
    const vvl::AccelerationStructureNV *GetAccelerationStructureStateNV() const { return acc_state_nv_.get(); }
    vvl::AccelerationStructureNV *GetAccelerationStructureStateNV() { return acc_state_nv_.get(); }
    // Returns true if there is a stored KHR acceleration structure and false if there is a stored NV acceleration structure.
    // Asserts that there is only one of the two.
    bool IsAccelerationStructureKHR() const {
        auto acc_khr = GetAccelerationStructureKHR();
        assert((acc_khr != VK_NULL_HANDLE) ^ (GetAccelerationStructureNV() != VK_NULL_HANDLE));
        return acc_khr != VK_NULL_HANDLE;
    }

    void UpdateDrawState(vvl::CommandBuffer &cb_state) override;

    bool AddParent(StateObject *state_object) override;
    void RemoveParent(StateObject *state_object) override;

    bool IsKHR() const { return is_khr_; }
    bool Invalid() const override;

    VkDescriptorType ActiveType() const { return active_descriptor_type_; }
    DescriptorClass ActiveClass() const { return DescriptorTypeToClass(active_descriptor_type_); }

  private:
    VkDeviceSize buffer_size_{0};
    VkDescriptorType active_descriptor_type_{VK_DESCRIPTOR_TYPE_MUTABLE_EXT};

    // Sampler and ImageSampler Descriptor
    bool immutable_{false};
    std::shared_ptr<vvl::Sampler> sampler_state_;
    // Image Descriptor
    std::shared_ptr<vvl::ImageView> image_view_state_;
    VkImageLayout image_layout_{VK_IMAGE_LAYOUT_UNDEFINED};
    // Texel Descriptor
    std::shared_ptr<vvl::BufferView> buffer_view_state_;
    // Buffer Descriptor
    VkDeviceSize offset_{0};
    VkDeviceSize range_{0};
    std::shared_ptr<vvl::Buffer> buffer_state_;
    // Acceleration Structure Descriptor
    bool is_khr_{false};
    VkAccelerationStructureKHR acc_{VK_NULL_HANDLE};
    std::shared_ptr<vvl::AccelerationStructureKHR> acc_state_;
    VkAccelerationStructureNV acc_nv_{VK_NULL_HANDLE};
    std::shared_ptr<vvl::AccelerationStructureNV> acc_state_nv_;
};

// Structs to contain common elements that need to be shared between Validate* and Perform* calls below
struct AllocateDescriptorSetsData {
    std::map<uint32_t, uint32_t> required_descriptors_by_type;
    std::vector<std::shared_ptr<DescriptorSetLayout const>> layout_nodes;
    void Init(uint32_t);
    AllocateDescriptorSetsData(){};
};
// "Perform" does the update with the assumption that ValidateUpdateDescriptorSets() has passed for the given update
void PerformUpdateDescriptorSets(Device &, uint32_t, const VkWriteDescriptorSet *, uint32_t, const VkCopyDescriptorSet *);

class DescriptorBinding {
  public:
    using NodeList = StateObject::NodeList;
    DescriptorBinding(const VkDescriptorSetLayoutBinding &create_info, uint32_t count_, VkDescriptorBindingFlags binding_flags_)
        : binding(create_info.binding),
          type(create_info.descriptorType),
          descriptor_class(DescriptorTypeToClass(type)),
          stage_flags(create_info.stageFlags),
          binding_flags(binding_flags_),
          count(count_),
          has_immutable_samplers(create_info.pImmutableSamplers != nullptr),
          updated(count_, false) {}
    virtual ~DescriptorBinding() {}

    virtual void AddParent(DescriptorSet *ds) = 0;
    virtual void RemoveParent(DescriptorSet *ds) = 0;
    virtual void NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) = 0;

    virtual const Descriptor *GetDescriptor(const uint32_t index) const = 0;
    virtual Descriptor *GetDescriptor(const uint32_t index) = 0;

    bool IsVariableCount() const { return (binding_flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) != 0; }

    bool IsConsistent(const DescriptorBinding &other) const {
        // A write update can overlap over following binding but bindings with descriptorCount == 0 must be skipped.
        // Therefore we consider "consistent" a binding that should be skipped
        if (other.count == 0) {
            return true;
        }
        return type == other.type && stage_flags == other.stage_flags && binding_flags == other.binding_flags &&
               has_immutable_samplers == other.has_immutable_samplers;
    }

    const uint32_t binding;
    const VkDescriptorType type;
    const DescriptorClass descriptor_class;
    const VkShaderStageFlags stage_flags;
    const VkDescriptorBindingFlags binding_flags;
    const uint32_t count;
    const bool has_immutable_samplers;
    small_vector<bool, 1, uint32_t> updated;
};

template <typename T>
class DescriptorBindingImpl : public DescriptorBinding {
  public:
    DescriptorBindingImpl(const VkDescriptorSetLayoutBinding &create_info, uint32_t count_, VkDescriptorBindingFlags binding_flags_)
        : DescriptorBinding(create_info, count_, binding_flags_), descriptors(count_) {}

    const Descriptor *GetDescriptor(const uint32_t index) const override { return index < count ? &descriptors[index] : nullptr; }

    Descriptor *GetDescriptor(const uint32_t index) override { return index < count ? &descriptors[index] : nullptr; }

    template <typename Fn>
    void ForAllUpdated(Fn &&op) {
        auto size = updated.size();
        for (uint32_t i = 0; i < size; i++) {
            if (updated[i] != 0) {
                op(descriptors[i]);
            }
        }
    }

    void AddParent(DescriptorSet *ds) override {
        auto add_parent = [ds](T &descriptor) { descriptor.AddParent(ds); };
        ForAllUpdated(add_parent);
    }

    void RemoveParent(DescriptorSet *ds) override {
        auto remove_parent = [ds](T &descriptor) { descriptor.RemoveParent(ds); };
        ForAllUpdated(remove_parent);
    }

    void NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) override {
        if (!T::SupportsNotifyInvalidate()) return;

        for (const auto &node : invalid_nodes) {
            if (T::IsNotifyInvalidateType(node->Type())) {
                auto notify_invalidate = [&node, unlink](T &descriptor) { descriptor.InvalidateNode(node, unlink); };
                ForAllUpdated(notify_invalidate);
            }
        }
    }

    small_vector<T, 1, uint32_t> descriptors;
};

using SamplerBinding = DescriptorBindingImpl<SamplerDescriptor>;
using ImageBinding = DescriptorBindingImpl<ImageDescriptor>;
using ImageSamplerBinding = DescriptorBindingImpl<ImageSamplerDescriptor>;
using TexelBinding = DescriptorBindingImpl<TexelDescriptor>;
using BufferBinding = DescriptorBindingImpl<BufferDescriptor>;
using InlineUniformBinding = DescriptorBindingImpl<InlineUniformDescriptor>;
using AccelerationStructureBinding = DescriptorBindingImpl<AccelerationStructureDescriptor>;
using MutableBinding = DescriptorBindingImpl<MutableDescriptor>;

// Helper class to encapsulate the descriptor update template decoding logic
struct DecodedTemplateUpdate {
    std::vector<VkWriteDescriptorSet> desc_writes;
    std::vector<VkWriteDescriptorSetInlineUniformBlock> inline_infos;
    std::vector<VkWriteDescriptorSetAccelerationStructureKHR> inline_infos_khr;
    std::vector<VkWriteDescriptorSetAccelerationStructureNV> inline_infos_nv;
    DecodedTemplateUpdate(const Device &device_data, VkDescriptorSet descriptorSet, const DescriptorUpdateTemplate *template_state,
                          const void *pData, VkDescriptorSetLayout push_layout = VK_NULL_HANDLE);
};

/*
 * DescriptorSet class
 *
 * Overview - This class encapsulates the Vulkan VkDescriptorSet data (set).
 *   A set has an underlying layout which defines the bindings in the set and the
 *   types and numbers of descriptors in each descriptor slot. Most of the layout
 *   interfaces are exposed through identically-named functions in the set class.
 *   Please refer to the DescriptorSetLayout comment above for a description of
 *   index, binding, and global index.
 *
 * At construction a vector of Descriptor* is created with types corresponding to the
 *   layout. The primary operation performed on the descriptors is to update them
 *   via write or copy updates, and validate that the update contents are correct.
 *   In order to validate update contents, the DescriptorSet stores a bunch of ptrs
 *   to data maps where various Vulkan objects can be looked up. The management of
 *   those maps is performed externally. The set class relies on their contents to
 *   be correct at the time of update.
 */
class DescriptorSet : public StateObject {
  public:
    using BaseClass = StateObject;
    // Given that we are providing placement new allocation for bindings, the deleter needs to *only* call the destructor
    struct BindingDeleter {
        void operator()(DescriptorBinding *binding) { binding->~DescriptorBinding(); }
    };
    using BindingPtr = std::unique_ptr<DescriptorBinding, BindingDeleter>;
    using BindingVector = std::vector<BindingPtr>;
    using BindingIterator = BindingVector::iterator;
    using ConstBindingIterator = BindingVector::const_iterator;

    DescriptorSet(const VkDescriptorSet handle, vvl::DescriptorPool *, const std::shared_ptr<DescriptorSetLayout const> &,
                  uint32_t variable_count, Device *state_data);
    void LinkChildNodes() override;
    void NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) override;
    ~DescriptorSet() { Destroy(); }

    // A number of common Get* functions that return data based on layout from which this set was created
    uint32_t GetTotalDescriptorCount() const { return layout_->GetTotalDescriptorCount(); };
    uint32_t GetNonInlineDescriptorCount() const { return layout_->GetNonInlineDescriptorCount(); };
    uint32_t GetDynamicDescriptorCount() const { return layout_->GetDynamicDescriptorCount(); };
    uint32_t GetBindingCount() const { return layout_->GetBindingCount(); };
    uint32_t GetDescriptorCountFromBinding(const uint32_t binding) const {
        return layout_->GetDescriptorCountFromBinding(binding);
    };
    // Return true if given binding is present in this set
    bool HasBinding(const uint32_t binding) const { return layout_->HasBinding(binding); };

    // Perform a push update whose contents were just validated using ValidatePushDescriptorsUpdate
    virtual void PerformPushDescriptorsUpdate(uint32_t write_count, const VkWriteDescriptorSet *write_descs);
    // Perform a WriteUpdate whose contents were just validated using ValidateWriteUpdate
    virtual void PerformWriteUpdate(const VkWriteDescriptorSet &);
    // Perform a CopyUpdate whose contents were just validated using ValidateCopyUpdate
    virtual void PerformCopyUpdate(const VkCopyDescriptorSet &, const DescriptorSet &src_set);

    const std::shared_ptr<DescriptorSetLayout const> &GetLayout() const { return layout_; };
    VkDescriptorSet VkHandle() const { return handle_.Cast<VkDescriptorSet>(); };
    // Bind given cmd_buffer to this descriptor set and
    // update CB image layout map with image/imagesampler descriptor image layouts
    void UpdateDrawStates(Device *, vvl::CommandBuffer &cb_state, const BindingVariableMap &);

    // For a particular binding, get the global index
    const IndexRange GetGlobalIndexRangeFromBinding(const uint32_t binding, bool actual_length = false) const {
        if (actual_length && binding == layout_->GetMaxBinding() && GetBinding(binding)->IsVariableCount()) {
            IndexRange range = layout_->GetGlobalIndexRangeFromBinding(binding);
            auto diff = GetDescriptorCountFromBinding(binding) - GetVariableDescriptorCount();
            range.end -= diff;
            return range;
        }
        return layout_->GetGlobalIndexRangeFromBinding(binding);
    };
    bool IsPushDescriptor() const { return layout_->IsPushDescriptor(); }
    bool IsUpdateAfterBind() const {
        return (layout_->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) != 0;
    }
    uint32_t GetVariableDescriptorCount() const { return variable_count_; }
    vvl::DescriptorPool *GetPoolState() const { return pool_state_; }

    // These are overriding STL so need lower case names
    ConstBindingIterator begin() const { return bindings_.begin(); }
    ConstBindingIterator end() const { return bindings_.end(); }
    ConstBindingIterator FindBinding(uint32_t binding) const {
        auto index = layout_->GetIndexFromBinding(binding);
        return (index < bindings_.size()) ? bindings_.begin() + index : bindings_.end();
    }

    BindingIterator begin() { return bindings_.begin(); }
    BindingIterator end() { return bindings_.end(); }
    BindingIterator FindBinding(uint32_t binding) {
        auto index = layout_->GetIndexFromBinding(binding);
        return (index < bindings_.size()) ? bindings_.begin() + index : bindings_.end();
    }

    const DescriptorBinding *GetBinding(uint32_t binding) const {
        auto index = layout_->GetIndexFromBinding(binding);
        return index < bindings_.size() ? bindings_[index].get() : nullptr;
    }

    DescriptorBinding *GetBinding(uint32_t binding) {
        auto index = layout_->GetIndexFromBinding(binding);
        return index < bindings_.size() ? bindings_[index].get() : nullptr;
    }

    const Descriptor *GetDescriptorFromBinding(const uint32_t binding, const uint32_t index = 0) const {
        const auto *binding_data = GetBinding(binding);
        return binding_data ? binding_data->GetDescriptor(index) : nullptr;
    }

    Descriptor *GetDescriptorFromBinding(const uint32_t binding, const uint32_t index = 0) {
        auto *binding_data = GetBinding(binding);
        return binding_data ? binding_data->GetDescriptor(index) : nullptr;
    }

    // For a given dynamic offset array, return the corresponding index into the list of descriptors in set
    const Descriptor *GetDescriptorFromDynamicOffsetIndex(const uint32_t index) const {
        auto pos = dynamic_offset_idx_to_descriptor_list_.at(index);
        return bindings_[pos.first]->GetDescriptor(pos.second);
    }

    // Returns index in the dynamic offset array (specified by
    // vkCmdBindDescriptorSets) for the given dynamic descriptor binding.
    // The caller has to ensure that binding has dynamic descriptor type.
    uint32_t GetDynamicOffsetIndexFromBinding(uint32_t dynamic_binding) const;

    uint64_t GetChangeCount() const { return change_count_; }

    const std::vector<vku::safe_VkWriteDescriptorSet> &GetWrites() const { return push_descriptor_set_writes; }

    void Destroy() override;

    const DescriptorSetLayout &Layout() const { return *layout_; }

    template <typename Iter>
    class DescriptorIterator {
      public:
        DescriptorIterator() = delete;
        DescriptorIterator(const DescriptorIterator &other) = default;
        DescriptorIterator &operator=(const DescriptorIterator &rhs) = default;

        DescriptorIterator(DescriptorSet &descriptor_set, uint32_t binding, uint32_t index = 0)
            : iter_(descriptor_set.FindBinding(binding)), end_(descriptor_set.end()), index_(index) {
            if (!AtEnd()) {
                assert(index_ < (*iter_)->count);
            }
        }

        DescriptorIterator(const DescriptorSet &descriptor_set, uint32_t binding, uint32_t index = 0)
            : iter_(descriptor_set.FindBinding(binding)), end_(descriptor_set.end()), index_(index) {
            if (!AtEnd()) {
                assert(index_ < (*iter_)->count);
            }
        }

        bool AtEnd() const { return iter_ == end_; }

        bool operator==(const DescriptorIterator &rhs) { return (iter_ == rhs.iter_) && (index_ == rhs.index_); }

        DescriptorIterator &operator++() {
            if (!AtEnd()) {
                index_++;
                if (index_ >= (*iter_)->count) {
                    index_ = 0;
                    do {
                        ++iter_;
                    } while (!AtEnd() && (*iter_)->count == 0);
                }
            }
            return *this;
        }

        const DescriptorBinding &CurrentBinding() const {
            assert(iter_ != end_);
            return **iter_;
        }
        DescriptorBinding &CurrentBinding() {
            assert(iter_ != end_);
            return **iter_;
        }
        uint32_t CurrentIndex() const {
            return index_;
        }

        const Descriptor *operator->() const {
            assert(iter_ != end_);
            assert(index_ < (*iter_)->count);
            return (*iter_)->GetDescriptor(index_);
        }
        const Descriptor &operator*() const { return *(this->operator->()); }

        Descriptor *operator->() {
            assert(iter_ != end_);
            assert(index_ < (*iter_)->count);
            return (*iter_)->GetDescriptor(index_);
        }
        Descriptor &operator*() { return *(this->operator->()); }

        bool updated() const { return CurrentBinding().updated[index_] != 0; }

        void updated(bool val) { CurrentBinding().updated[index_] = static_cast<uint32_t>(val); }

      private:
        Iter iter_;
        Iter end_;
        uint32_t index_;
    };
    DescriptorIterator<BindingIterator> FindDescriptor(uint32_t binding, uint32_t index) {
        return DescriptorIterator<BindingIterator>(*this, binding, index);
    }

    DescriptorIterator<ConstBindingIterator> FindDescriptor(uint32_t binding, uint32_t index) const {
        return DescriptorIterator<ConstBindingIterator>(*this, binding, index);
    }

    bool ValidateBindingOnGPU(const DescriptorBinding &binding, bool is_runtime_descriptor_array) const;

  protected:
    union AnyBinding {
        SamplerBinding sampler;
        ImageSamplerBinding image_sampler;
        ImageBinding image;
        TexelBinding texel;
        BufferBinding buffer;
        InlineUniformBinding inline_uniform;
        AccelerationStructureBinding accelerator_structure;
        MutableBinding mutable_binding;
        ~AnyBinding() = delete;
    };

    struct alignas(alignof(AnyBinding)) BindingBackingStore {
        uint8_t data[sizeof(AnyBinding)];
    };

    template <typename T>
    std::unique_ptr<T, BindingDeleter> MakeBinding(BindingBackingStore *location, const VkDescriptorSetLayoutBinding &create_info,
                                                   uint32_t descriptor_count, VkDescriptorBindingFlags flags) {
        return std::unique_ptr<T, BindingDeleter>(new (location->data) T(create_info, descriptor_count, flags));
    }

    std::atomic<bool> some_update_;  // has any part of the set ever been updated?
    vvl::DescriptorPool *pool_state_;
    const std::shared_ptr<DescriptorSetLayout const> layout_;
    // NOTE: the the backing store for the bindings must be declared *before* it so it will be destructed *after* it
    // "Destructors for nonstatic member objects are called in the reverse order in which they appear in the class declaration."
    std::vector<BindingBackingStore> bindings_store_;
    std::vector<BindingPtr> bindings_;
    Device *state_data_;
    uint32_t variable_count_;
    std::atomic<uint64_t> change_count_;

    // For a given dynamic offset index in the set, map to associated index of the descriptors in the set
    std::vector<std::pair<uint32_t, uint32_t>> dynamic_offset_idx_to_descriptor_list_;

    // If this descriptor set is a push descriptor set, the descriptor
    // set writes that were last pushed.
    std::vector<vku::safe_VkWriteDescriptorSet> push_descriptor_set_writes;
};

// When updating a descriptor the VkDescriptorSetLayout can be sourced from 2 spots
// 1. The VkDescriptorSet found in VkWriteDescriptorSet::dstSet (normal way)
// 2. The VkPipelineLayout provided in vkCmdPushDescriptorSet
//
// This object is created to allow both code paths to share same logic, but still provide error messages that fit for each incoming
// call
struct DslErrorSource {
    const Location &ds_loc_;
    const VkDescriptorSet ds_handle_ = VK_NULL_HANDLE;
    const VkPipelineLayout pipeline_layout_handle_ = VK_NULL_HANDLE;
    const uint32_t set_ = 0;  // used for vkCmdPushDescriptorSet to pick set

    DslErrorSource(const Location &ds_loc, VkDescriptorSet ds_handle) : ds_loc_(ds_loc), ds_handle_(ds_handle) {}
    DslErrorSource(const Location &ds_loc, VkPipelineLayout pipeline_layout_handle, uint32_t set)
        : ds_loc_(ds_loc), pipeline_layout_handle_(pipeline_layout_handle), set_(set) {}

    std::string PrintMessage(const Logger &error_logger) const;
};

}  // namespace vvl
