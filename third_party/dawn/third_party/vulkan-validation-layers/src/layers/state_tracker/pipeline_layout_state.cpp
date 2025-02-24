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

#include "state_tracker/pipeline_layout_state.h"

#include "error_message/error_strings.h"
#include "state_tracker/state_tracker.h"
#include "state_tracker/descriptor_sets.h"

// Dictionary of canonical form of the pipeline set layout of descriptor set layouts
static PipelineLayoutSetLayoutsDict pipeline_layout_set_layouts_dict;

// Dictionary of canonical form of the "compatible for set" records
static PipelineLayoutCompatDict pipeline_layout_compat_dict;

static PushConstantRangesDict push_constant_ranges_dict;

size_t PipelineLayoutCompatDef::hash() const {
    hash_util::HashCombiner hc;
    // The set number is integral to the CompatDef's distinctiveness
    hc << set << push_constant_ranges.get();
    const auto &descriptor_set_layouts = *set_layouts_id.get();
    for (uint32_t i = 0; i <= set; i++) {
        hc << descriptor_set_layouts[i].get();
    }
    return hc.Value();
}

bool PipelineLayoutCompatDef::operator==(const PipelineLayoutCompatDef &other) const {
    if ((set != other.set) || (push_constant_ranges != other.push_constant_ranges)) {
        return false;
    }

    if (set_layouts_id == other.set_layouts_id) {
        // if it's the same set_layouts_id, then *any* subset will match
        return true;
    }

    // They aren't exactly the same PipelineLayoutSetLayouts, so we need to check if the required subsets match
    const auto &descriptor_set_layouts = *set_layouts_id.get();
    assert(set < descriptor_set_layouts.size());
    const auto &other_ds_layouts = *other.set_layouts_id.get();
    assert(set < other_ds_layouts.size());
    for (uint32_t i = 0; i <= set; i++) {
        if (descriptor_set_layouts[i] != other_ds_layouts[i]) {
            return false;
        }
    }
    return true;
}

std::string PipelineLayoutCompatDef::DescribeDifference(const PipelineLayoutCompatDef &other) const {
    std::ostringstream ss;
    if (set != other.set) {
        ss << "The set " << set << " is different from the non-compatible pipeline layout (" << other.set << ")\n";
    } else if (push_constant_ranges != other.push_constant_ranges) {
        ss << "Pipeline layout pipeline bound with last call to vkCmdBindDescriptorSets has following push constant ranges:\n";
        if (push_constant_ranges->empty()) {
            ss << "Empty\n";
        } else {
            for (const auto [pcr_i, pcr] : vvl::enumerate(push_constant_ranges->data(), push_constant_ranges->size())) {
                ss << "VkPushConstantRange[ " << pcr_i << " ]: " << string_VkPushConstantRange(pcr) << '\n';
            }
        }
        ss << "But pipeline layout of last bound pipeline or last bound shaders has following push constant ranges:\n";
        if (push_constant_ranges->empty()) {
            ss << "Empty\n";
        } else {
            for (const auto [pcr_i, pcr] : vvl::enumerate(other.push_constant_ranges->data(), other.push_constant_ranges->size())) {
                ss << "VkPushConstantRange[ " << pcr_i << " ]: " << string_VkPushConstantRange(pcr) << '\n';
            }
        }
    } else {
        const auto &descriptor_set_layouts = *set_layouts_id.get();
        const auto &other_ds_layouts = *other.set_layouts_id.get();
        for (uint32_t i = 0; i <= set; i++) {
            if (descriptor_set_layouts[i] != other_ds_layouts[i]) {
                if (!descriptor_set_layouts[i] || !other_ds_layouts[i]) {
                    ss << "Set layout " << i << " contains a null set which is considered non-compatible\n";
                    break;
                }
                return descriptor_set_layouts[i]->DescribeDifference(i, *other_ds_layouts[i]);
            }
        }
    }
    return ss.str();
}

static PipelineLayoutCompatId GetCanonicalId(const uint32_t set_index, const PushConstantRangesId &pcr_id,
                                             const PipelineLayoutSetLayoutsId &set_layouts_id) {
    return pipeline_layout_compat_dict.LookUp(PipelineLayoutCompatDef(set_index, pcr_id, set_layouts_id));
}

// For repeatable sorting, not very useful for "memory in range" search
struct PushConstantRangeCompare {
    bool operator()(const VkPushConstantRange *lhs, const VkPushConstantRange *rhs) const {
        if (lhs->offset == rhs->offset) {
            if (lhs->size == rhs->size) {
                // The comparison is arbitrary, but avoids false aliasing by comparing all fields.
                return lhs->stageFlags < rhs->stageFlags;
            }
            // If the offsets are the same then sorting by the end of range is useful for validation
            return lhs->size < rhs->size;
        }
        return lhs->offset < rhs->offset;
    }
};

PushConstantRangesId GetCanonicalId(uint32_t pushConstantRangeCount, const VkPushConstantRange *pPushConstantRanges) {
    if (!pPushConstantRanges) {
        // Hand back the empty entry (creating as needed)...
        return push_constant_ranges_dict.LookUp(PushConstantRanges());
    }

    // Sort the input ranges to ensure equivalent ranges map to the same id
    std::set<const VkPushConstantRange *, PushConstantRangeCompare> sorted;
    for (uint32_t i = 0; i < pushConstantRangeCount; i++) {
        sorted.insert(pPushConstantRanges + i);
    }

    PushConstantRanges ranges;
    ranges.reserve(sorted.size());
    for (const auto *range : sorted) {
        ranges.emplace_back(*range);
    }
    return push_constant_ranges_dict.LookUp(std::move(ranges));
}

static PushConstantRangesId GetPushConstantRangesFromLayouts(const vvl::span<const vvl::PipelineLayout *const> &layouts) {
    PushConstantRangesId ret{};
    for (const auto *layout : layouts) {
        if (layout && layout->push_constant_ranges_layout) {
            ret = layout->push_constant_ranges_layout;

            if (ret->size() > 0) {
                return ret;
            }
        }
    }
    return ret;
}

std::vector<PipelineLayoutCompatId> GetCompatForSet(const std::vector<std::shared_ptr<vvl::DescriptorSetLayout const>> &set_layouts,
                                                    const PushConstantRangesId &push_constant_ranges) {
    PipelineLayoutSetLayoutsDef set_layout_ids(set_layouts.size());
    for (size_t i = 0; i < set_layouts.size(); i++) {
        if (set_layouts[i]) {
            set_layout_ids[i] = set_layouts[i]->GetLayoutId();
        }
    }
    auto set_layouts_id = pipeline_layout_set_layouts_dict.LookUp(set_layout_ids);

    std::vector<PipelineLayoutCompatId> set_compat_ids;
    set_compat_ids.reserve(set_layouts.size());

    for (uint32_t i = 0; i < set_layouts.size(); i++) {
        set_compat_ids.emplace_back(GetCanonicalId(i, push_constant_ranges, set_layouts_id));
    }
    return set_compat_ids;
}

VkPipelineLayoutCreateFlags GetCreateFlags(const vvl::span<const vvl::PipelineLayout *const> &layouts) {
    VkPipelineLayoutCreateFlags flags = 0;
    for (const auto &layout : layouts) {
        if (layout) {
            flags |= layout->CreateFlags();
        }
    }
    return flags;
}

namespace vvl {

static PipelineLayout::SetLayoutVector GetSetLayouts(Device &dev_data, const VkPipelineLayoutCreateInfo *pCreateInfo) {
    PipelineLayout::SetLayoutVector set_layouts(pCreateInfo->setLayoutCount);

    for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
        set_layouts[i] = dev_data.Get<vvl::DescriptorSetLayout>(pCreateInfo->pSetLayouts[i]);
    }
    return set_layouts;
}

static PipelineLayout::SetLayoutVector GetSetLayouts(const vvl::span<const PipelineLayout *const> &layouts) {
    PipelineLayout::SetLayoutVector set_layouts;
    size_t num_layouts = 0;
    for (const auto &layout : layouts) {
        if (layout && (layout->set_layouts.size() > num_layouts)) {
            num_layouts = layout->set_layouts.size();
        }
    }

    if (!num_layouts) {
        return {};
    }

    set_layouts.reserve(num_layouts);
    for (size_t i = 0; i < num_layouts; ++i) {
        const PipelineLayout *used_layout = nullptr;
        for (const auto *layout : layouts) {
            if (layout) {
                if (layout->set_layouts.size() > i) {
                    // This _could_ be the layout we're looking for
                    used_layout = layout;

                    if (layout->set_layouts[i]) {
                        // This is the layout we're looking for. Any subsequent ones that match must be identical to this one.
                        break;
                    }
                }
            }
        }
        if (used_layout) {
            set_layouts.emplace_back(used_layout->set_layouts[i]);
        }
    }
    return set_layouts;
}

PipelineLayout::PipelineLayout(Device &dev_data, VkPipelineLayout handle, const VkPipelineLayoutCreateInfo *pCreateInfo)
    : StateObject(handle, kVulkanObjectTypePipelineLayout),
      set_layouts(GetSetLayouts(dev_data, pCreateInfo)),
      push_constant_ranges_layout(GetCanonicalId(pCreateInfo->pushConstantRangeCount, pCreateInfo->pPushConstantRanges)),
      set_compat_ids(GetCompatForSet(set_layouts, push_constant_ranges_layout)),
      create_flags(pCreateInfo->flags) {}

PipelineLayout::PipelineLayout(const vvl::span<const PipelineLayout *const> &layouts)
    : StateObject(static_cast<VkPipelineLayout>(VK_NULL_HANDLE), kVulkanObjectTypePipelineLayout),
      set_layouts(GetSetLayouts(layouts)),
      push_constant_ranges_layout(GetPushConstantRangesFromLayouts(layouts)),  // TODO is this correct?
      set_compat_ids(GetCompatForSet(set_layouts, push_constant_ranges_layout)),
      create_flags(GetCreateFlags(layouts)) {}

}  // namespace vvl
