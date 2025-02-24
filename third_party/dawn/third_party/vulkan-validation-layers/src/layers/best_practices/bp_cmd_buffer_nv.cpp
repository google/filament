/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "best_practices/best_practices_validation.h"
#include "best_practices/bp_state.h"

static std::array<uint32_t, 4> GetRawClearColor(VkFormat format, const VkClearColorValue& clear_value) {
    std::array<uint32_t, 4> raw_color{};
    std::copy_n(clear_value.uint32, raw_color.size(), raw_color.data());

    // Zero out unused components to avoid polluting the cache with garbage
    if (!vkuFormatHasRed(format)) raw_color[0] = 0;
    if (!vkuFormatHasGreen(format)) raw_color[1] = 0;
    if (!vkuFormatHasBlue(format)) raw_color[2] = 0;
    if (!vkuFormatHasAlpha(format)) raw_color[3] = 0;

    return raw_color;
}

static bool IsClearColorZeroOrOne(VkFormat format, const std::array<uint32_t, 4> clear_color) {
    static_assert(sizeof(float) == sizeof(uint32_t), "Mismatching float <-> uint32 sizes");
    const float one = 1.0f;
    const float zero = 0.0f;
    uint32_t raw_one{};
    uint32_t raw_zero{};
    memcpy(&raw_one, &one, sizeof(one));
    memcpy(&raw_zero, &zero, sizeof(zero));

    const bool is_one =
        (!vkuFormatHasRed(format) || (clear_color[0] == raw_one)) && (!vkuFormatHasGreen(format) || (clear_color[1] == raw_one)) &&
        (!vkuFormatHasBlue(format) || (clear_color[2] == raw_one)) && (!vkuFormatHasAlpha(format) || (clear_color[3] == raw_one));
    const bool is_zero = (!vkuFormatHasRed(format) || (clear_color[0] == raw_zero)) &&
                         (!vkuFormatHasGreen(format) || (clear_color[1] == raw_zero)) &&
                         (!vkuFormatHasBlue(format) || (clear_color[2] == raw_zero)) &&
                         (!vkuFormatHasAlpha(format) || (clear_color[3] == raw_zero));
    return is_one || is_zero;
}

void BestPractices::RecordSetDepthTestState(bp_state::CommandBuffer& cb_state, VkCompareOp new_depth_compare_op,
                                            bool new_depth_test_enable) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    if (cb_state.nv.depth_compare_op != new_depth_compare_op) {
        switch (new_depth_compare_op) {
            case VK_COMPARE_OP_LESS:
            case VK_COMPARE_OP_LESS_OR_EQUAL:
                cb_state.nv.zcull_direction = ZcullDirection::Less;
                break;
            case VK_COMPARE_OP_GREATER:
            case VK_COMPARE_OP_GREATER_OR_EQUAL:
                cb_state.nv.zcull_direction = ZcullDirection::Greater;
                break;
            default:
                // The other ops carry over the previous state.
                break;
        }
    }
    cb_state.nv.depth_compare_op = new_depth_compare_op;
    cb_state.nv.depth_test_enable = new_depth_test_enable;
}

void BestPractices::RecordBindZcullScope(bp_state::CommandBuffer& cb_state, VkImage depth_attachment,
                                         const VkImageSubresourceRange& subresource_range) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    if (depth_attachment == VK_NULL_HANDLE) {
        cb_state.nv.zcull_scope = {};
        return;
    }

    assert((subresource_range.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0U);

    auto image_state = Get<vvl::Image>(depth_attachment);
    ASSERT_AND_RETURN(image_state);

    const uint32_t mip_levels = image_state->create_info.mipLevels;
    const uint32_t array_layers = image_state->create_info.arrayLayers;

    auto& tree = cb_state.nv.zcull_per_image[depth_attachment];
    if (tree.states.empty()) {
        tree.mip_levels = mip_levels;
        tree.array_layers = array_layers;
        tree.states.resize(array_layers * mip_levels);
    }

    cb_state.nv.zcull_scope.image = depth_attachment;
    cb_state.nv.zcull_scope.range = subresource_range;
    cb_state.nv.zcull_scope.tree = &tree;
}

void BestPractices::RecordUnbindZcullScope(bp_state::CommandBuffer& cb_state) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    RecordBindZcullScope(cb_state, VK_NULL_HANDLE, VkImageSubresourceRange{});
}

void BestPractices::RecordResetScopeZcullDirection(bp_state::CommandBuffer& cb_state) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    auto& scope = cb_state.nv.zcull_scope;
    RecordResetZcullDirection(cb_state, scope.image, scope.range);
}

template <typename Func>
static void ForEachSubresource(const vvl::Image& image, const VkImageSubresourceRange& range, Func&& func) {
    const uint32_t layer_count =
        (range.layerCount == VK_REMAINING_ARRAY_LAYERS) ? (image.full_range.layerCount - range.baseArrayLayer) : range.layerCount;
    const uint32_t level_count =
        (range.levelCount == VK_REMAINING_MIP_LEVELS) ? (image.full_range.levelCount - range.baseMipLevel) : range.levelCount;

    for (uint32_t i = 0; i < layer_count; ++i) {
        const uint32_t layer = range.baseArrayLayer + i;
        for (uint32_t j = 0; j < level_count; ++j) {
            const uint32_t level = range.baseMipLevel + j;
            func(layer, level);
        }
    }
}

void BestPractices::RecordResetZcullDirection(bp_state::CommandBuffer& cb_state, VkImage depth_image,
                                              const VkImageSubresourceRange& subresource_range) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    RecordSetZcullDirection(cb_state, depth_image, subresource_range, ZcullDirection::Unknown);

    const auto image_it = cb_state.nv.zcull_per_image.find(depth_image);
    if (image_it == cb_state.nv.zcull_per_image.end()) {
        return;
    }
    auto& tree = image_it->second;

    auto image = Get<vvl::Image>(depth_image);
    ASSERT_AND_RETURN(image);

    ForEachSubresource(*image, subresource_range, [&tree](uint32_t layer, uint32_t level) {
        auto& subresource = tree.GetState(layer, level);
        subresource.num_less_draws = 0;
        subresource.num_greater_draws = 0;
    });
}

void BestPractices::RecordSetScopeZcullDirection(bp_state::CommandBuffer& cb_state, ZcullDirection mode) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    auto& scope = cb_state.nv.zcull_scope;
    RecordSetZcullDirection(cb_state, scope.image, scope.range, mode);
}

void BestPractices::RecordSetZcullDirection(bp_state::CommandBuffer& cb_state, VkImage depth_image,
                                            const VkImageSubresourceRange& subresource_range, ZcullDirection mode) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    const auto image_it = cb_state.nv.zcull_per_image.find(depth_image);
    if (image_it == cb_state.nv.zcull_per_image.end()) {
        return;
    }
    auto& tree = image_it->second;

    auto image = Get<vvl::Image>(depth_image);
    ASSERT_AND_RETURN(image);

    ForEachSubresource(*image, subresource_range, [&tree, &cb_state](uint32_t layer, uint32_t level) {
        tree.GetState(layer, level).direction = cb_state.nv.zcull_direction;
    });
}

void BestPractices::RecordZcullDraw(bp_state::CommandBuffer& cb_state) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    // Add one draw to each subresource depending on the current Z-cull direction
    auto& scope = cb_state.nv.zcull_scope;

    auto image = Get<vvl::Image>(scope.image);
    if (!image) return;

    ForEachSubresource(*image, scope.range, [&scope](uint32_t layer, uint32_t level) {
        auto& subresource = scope.tree->GetState(layer, level);

        switch (subresource.direction) {
            case ZcullDirection::Unknown:
                // Unreachable
                assert(false);
                break;
            case ZcullDirection::Less:
                ++subresource.num_less_draws;
                break;
            case ZcullDirection::Greater:
                ++subresource.num_greater_draws;
                break;
        }
    });
}

bool BestPractices::ValidateZcullScope(const bp_state::CommandBuffer& cb_state, const Location& loc) const {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    bool skip = false;

    if (cb_state.nv.depth_test_enable) {
        auto& scope = cb_state.nv.zcull_scope;
        skip |= ValidateZcull(cb_state, scope.image, scope.range, loc);
    }

    return skip;
}

bool BestPractices::ValidateZcull(const bp_state::CommandBuffer& cb_state, VkImage image,
                                  const VkImageSubresourceRange& subresource_range, const Location& loc) const {
    bool skip = false;

    const char* good_mode = nullptr;
    const char* bad_mode = nullptr;
    bool is_balanced = false;

    const auto image_it = cb_state.nv.zcull_per_image.find(image);
    if (image_it == cb_state.nv.zcull_per_image.end()) {
        return skip;
    }
    const auto& tree = image_it->second;

    auto image_state = Get<vvl::Image>(image);
    ASSERT_AND_RETURN_SKIP(image_state);

    ForEachSubresource(*image_state, subresource_range, [&](uint32_t layer, uint32_t level) {
        if (is_balanced) {
            return;
        }
        const auto& resource = tree.GetState(layer, level);
        const uint64_t num_draws = resource.num_less_draws + resource.num_greater_draws;

        if (num_draws == 0) {
            return;
        }
        const uint64_t less_ratio = (resource.num_less_draws * 100) / num_draws;
        const uint64_t greater_ratio = (resource.num_greater_draws * 100) / num_draws;

        if ((less_ratio > kZcullDirectionBalanceRatioNVIDIA) && (greater_ratio > kZcullDirectionBalanceRatioNVIDIA)) {
            is_balanced = true;

            if (greater_ratio > less_ratio) {
                good_mode = "GREATER";
                bad_mode = "LESS";
            } else {
                good_mode = "LESS";
                bad_mode = "GREATER";
            }
        }
    });

    if (is_balanced) {
        skip |= LogPerformanceWarning(
            "BestPractices-NVIDIA-Zcull-LessGreaterRatio", cb_state.Handle(), loc,
            "%s Depth attachment %s is primarily rendered with depth compare op %s, but some draws use %s. "
            "Z-cull is disabled for the least used direction, which harms depth testing performance. "
            "The Z-cull direction can be reset by clearing the depth attachment, transitioning from VK_IMAGE_LAYOUT_UNDEFINED, "
            "using VK_ATTACHMENT_LOAD_OP_DONT_CARE, or using VK_ATTACHMENT_STORE_OP_DONT_CARE.",
            VendorSpecificTag(kBPVendorNVIDIA), FormatHandle(cb_state.nv.zcull_scope.image).c_str(), good_mode, bad_mode);
    }

    return skip;
}

static constexpr std::array<VkFormat, 12> kCustomClearColorCompressedFormatsNVIDIA = {
    VK_FORMAT_R8G8B8A8_UNORM,           VK_FORMAT_B8G8R8A8_UNORM,           VK_FORMAT_A8B8G8R8_UNORM_PACK32,
    VK_FORMAT_A2R10G10B10_UNORM_PACK32, VK_FORMAT_A2B10G10R10_UNORM_PACK32, VK_FORMAT_R16G16B16A16_UNORM,
    VK_FORMAT_R16G16B16A16_SNORM,       VK_FORMAT_R16G16B16A16_UINT,        VK_FORMAT_R16G16B16A16_SINT,
    VK_FORMAT_R16G16B16A16_SFLOAT,      VK_FORMAT_R32G32B32A32_SFLOAT,      VK_FORMAT_B10G11R11_UFLOAT_PACK32,
};

void BestPractices::RecordClearColor(VkFormat format, const VkClearColorValue& clear_value) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    const std::array<uint32_t, 4> raw_color = GetRawClearColor(format, clear_value);
    if (IsClearColorZeroOrOne(format, raw_color)) {
        // These colors are always compressed
        return;
    }

    const auto it =
        std::find(kCustomClearColorCompressedFormatsNVIDIA.begin(), kCustomClearColorCompressedFormatsNVIDIA.end(), format);
    if (it == kCustomClearColorCompressedFormatsNVIDIA.end()) {
        // The format cannot be compressed with a custom color
        return;
    }

    // Record custom clear color
    WriteLockGuard guard{clear_colors_lock_};
    if (clear_colors_.size() < kMaxRecommendedNumberOfClearColorsNVIDIA) {
        clear_colors_.insert(raw_color);
    }
}

bool BestPractices::ValidateClearColor(VkCommandBuffer commandBuffer, VkFormat format, const VkClearColorValue& clear_value,
                                       const Location& loc) const {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    bool skip = false;

    const std::array<uint32_t, 4> raw_color = GetRawClearColor(format, clear_value);
    if (IsClearColorZeroOrOne(format, raw_color)) {
        return skip;
    }

    const auto it =
        std::find(kCustomClearColorCompressedFormatsNVIDIA.begin(), kCustomClearColorCompressedFormatsNVIDIA.end(), format);
    if (it == kCustomClearColorCompressedFormatsNVIDIA.end()) {
        // The format is not compressible
        std::string format_list;
        for (VkFormat compressed_format : kCustomClearColorCompressedFormatsNVIDIA) {
            if (compressed_format == kCustomClearColorCompressedFormatsNVIDIA.back()) {
                format_list += "or ";
            }
            format_list += string_VkFormat(compressed_format);
            if (compressed_format != kCustomClearColorCompressedFormatsNVIDIA.back()) {
                format_list += ", ";
            }
        }

        skip |= LogPerformanceWarning("BestPractices-NVIDIA-ClearColor-NotCompressed", commandBuffer, loc,
                                      "%s Clearing image with format %s without a 1.0f or 0.0f clear color. "
                                      "The clear will not get compressed in the GPU, harming performance. "
                                      "This can be fixed using a clear color of VkClearColorValue{0.0f, 0.0f, 0.0f, 0.0f}, or "
                                      "VkClearColorValue{1.0f, 1.0f, 1.0f, 1.0f}. Alternatively, use %s.",
                                      VendorSpecificTag(kBPVendorNVIDIA), string_VkFormat(format), format_list.c_str());
    } else {
        // The format is compressible
        bool registered = false;
        {
            ReadLockGuard guard{clear_colors_lock_};
            registered = clear_colors_.find(raw_color) != clear_colors_.end();

            if (!registered) {
                // If it's not in the list, it might be new. Check if there's still space for new entries.
                registered = clear_colors_.size() < kMaxRecommendedNumberOfClearColorsNVIDIA;
            }
        }
        if (!registered) {
            std::string clear_color_str;

            if (vkuFormatIsUINT(format)) {
                clear_color_str = std::to_string(clear_value.uint32[0]) + ", " + std::to_string(clear_value.uint32[1]) + ", " +
                                  std::to_string(clear_value.uint32[2]) + ", " + std::to_string(clear_value.uint32[3]);
            } else if (vkuFormatIsSINT(format)) {
                clear_color_str = std::to_string(clear_value.int32[0]) + ", " + std::to_string(clear_value.int32[1]) + ", " +
                                  std::to_string(clear_value.int32[2]) + ", " + std::to_string(clear_value.int32[3]);
            } else {
                clear_color_str = std::to_string(clear_value.float32[0]) + ", " + std::to_string(clear_value.float32[1]) + ", " +
                                  std::to_string(clear_value.float32[2]) + ", " + std::to_string(clear_value.float32[3]);
            }

            skip |= LogPerformanceWarning(
                "BestPractices-NVIDIA-ClearColor-NotCompressed", commandBuffer, loc,
                "%s Clearing image with unregistered VkClearColorValue{%s}. "
                "This clear will not get compressed in the GPU, harming performance. "
                "The clear color is not registered because too many unique colors have been used. "
                "Select a discrete set of clear colors and stick to those. "
                "VkClearColorValue{0, 0, 0, 0} and VkClearColorValue{1.0f, 1.0f, 1.0f, 1.0f} are always registered.",
                VendorSpecificTag(kBPVendorNVIDIA), clear_color_str.c_str());
        }
    }

    return skip;
}
