/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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

#include <vector>
#include <memory>
#include "state_tracker/state_object.h"
#include "utils/hash_util.h"
#include "utils/hash_vk_types.h"

// Fwd declarations -- including descriptor_set.h creates an ugly include loop
namespace vvl {
class Device;
class DescriptorSetLayout;
class DescriptorSetLayoutDef;
}  // namespace vvl

// Canonical dictionary for the pipeline layout's layout of descriptorsetlayouts
using DescriptorSetLayoutDef = vvl::DescriptorSetLayoutDef;
using DescriptorSetLayoutId = std::shared_ptr<const DescriptorSetLayoutDef>;
using PipelineLayoutSetLayoutsDef = std::vector<DescriptorSetLayoutId>;
using PipelineLayoutSetLayoutsDict =
    hash_util::Dictionary<PipelineLayoutSetLayoutsDef, hash_util::IsOrderedContainer<PipelineLayoutSetLayoutsDef>>;
using PipelineLayoutSetLayoutsId = PipelineLayoutSetLayoutsDict::Id;

// Canonical dictionary for PushConstantRanges
using PushConstantRangesDict = hash_util::Dictionary<PushConstantRanges>;
using PushConstantRangesId = PushConstantRangesDict::Id;

// Defines/stores a compatibility defintion for set N
// The "layout layout" must store at least set+1 entries, but only the first set+1 are considered for hash and equality testing
// Note: the "cannonical" data are referenced by Id, not including handle or device specific state
// Note: hash and equality only consider layout_id entries [0, set] for determining uniqueness
struct PipelineLayoutCompatDef {
    uint32_t set;
    PushConstantRangesId push_constant_ranges;
    PipelineLayoutSetLayoutsId set_layouts_id;
    PipelineLayoutCompatDef(const uint32_t set_index, const PushConstantRangesId pcr_id, const PipelineLayoutSetLayoutsId sl_id)
        : set(set_index), push_constant_ranges(pcr_id), set_layouts_id(sl_id) {}
    size_t hash() const;

    bool operator==(const PipelineLayoutCompatDef &other) const;
    std::string DescribeDifference(const PipelineLayoutCompatDef &other) const;
};

// Canonical dictionary for PipelineLayoutCompat records
using PipelineLayoutCompatDict = hash_util::Dictionary<PipelineLayoutCompatDef, hash_util::HasHashMember<PipelineLayoutCompatDef>>;
using PipelineLayoutCompatId = PipelineLayoutCompatDict::Id;

PushConstantRangesId GetCanonicalId(uint32_t pushConstantRangeCount, const VkPushConstantRange *pPushConstantRanges);

namespace vvl {

// Store layouts and pushconstants for PipelineLayout
class PipelineLayout : public StateObject {
  public:
    using SetLayoutVector = std::vector<std::shared_ptr<vvl::DescriptorSetLayout const>>;
    const SetLayoutVector set_layouts;
    // canonical form IDs for the "compatible for set" contents
    const PushConstantRangesId push_constant_ranges_layout;
    // table of "compatible for set N" cannonical forms for trivial accept validation
    const std::vector<PipelineLayoutCompatId> set_compat_ids;
    VkPipelineLayoutCreateFlags create_flags;

    PipelineLayout(Device &dev_data, VkPipelineLayout handle, const VkPipelineLayoutCreateInfo *pCreateInfo);
    // Merge 2 or more non-overlapping layouts
    PipelineLayout(const vvl::span<const PipelineLayout *const> &layouts);
    template <typename Container>
    PipelineLayout(const Container &layouts) : PipelineLayout(vvl::span<const PipelineLayout *const>{layouts}) {}

    VkPipelineLayout VkHandle() const { return handle_.Cast<VkPipelineLayout>(); }

    VkPipelineLayoutCreateFlags CreateFlags() const { return create_flags; }
};

}  // namespace vvl

std::vector<PipelineLayoutCompatId> GetCompatForSet(
    const std::vector<std::shared_ptr<vvl::DescriptorSetLayout const>> &set_layouts,
    const PushConstantRangesId &push_constant_ranges);
