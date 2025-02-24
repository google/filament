/* Copyright (c) 2019-2024 The Khronos Group Inc.
 * Copyright (c) 2019-2024 Valve Corporation
 * Copyright (c) 2019-2024 LunarG, Inc.
 * Copyright (C) 2019-2024 Google Inc.
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
 *
 * John Zulauf <jzulauf@lunarg.com>
 *
 */
#include <cassert>
#include <vulkan/utility/vk_format_utils.h>

#include "subresource_adapter.h"
#include <cmath>
#include "state_tracker/image_state.h"
#include "generated/dispatch_functions.h"

namespace subresource_adapter {
Subresource::Subresource(const RangeEncoder& encoder, const VkImageSubresource& subres)
    : VkImageSubresource({0, subres.mipLevel, subres.arrayLayer}), aspect_index() {
    aspect_index = encoder.LowerBoundFromMask(subres.aspectMask);
    aspectMask = encoder.AspectBit(aspect_index);
}

IndexType RangeEncoder::Encode1AspectArrayOnly(const Subresource& pos) const { return pos.arrayLayer; }
IndexType RangeEncoder::Encode1AspectMipArray(const Subresource& pos) const { return pos.arrayLayer + pos.mipLevel * mip_size_; }
IndexType RangeEncoder::Encode1AspectMipOnly(const Subresource& pos) const { return pos.mipLevel; }

IndexType RangeEncoder::EncodeAspectArrayOnly(const Subresource& pos) const {
    return pos.arrayLayer + aspect_base_[pos.aspect_index];
}
IndexType RangeEncoder::EncodeAspectMipArray(const Subresource& pos) const {
    return pos.arrayLayer + pos.mipLevel * mip_size_ + aspect_base_[pos.aspect_index];
}
IndexType RangeEncoder::EncodeAspectMipOnly(const Subresource& pos) const { return pos.mipLevel + aspect_base_[pos.aspect_index]; }

uint32_t RangeEncoder::LowerBoundImpl1(VkImageAspectFlags aspect_mask) const {
    assert(aspect_mask & aspect_bits_[0]);
    return 0;
}
uint32_t RangeEncoder::LowerBoundWithStartImpl1(VkImageAspectFlags aspect_mask, uint32_t start) const {
    assert(start == 0);
    if (aspect_mask & aspect_bits_[0]) {
        return 0;
    }
    return limits_.aspect_index;
}

uint32_t RangeEncoder::LowerBoundImpl2(VkImageAspectFlags aspect_mask) const {
    if (aspect_mask & aspect_bits_[0]) {
        return 0;
    }
    assert(aspect_mask & aspect_bits_[1]);
    return 1;
}
uint32_t RangeEncoder::LowerBoundWithStartImpl2(VkImageAspectFlags aspect_mask, uint32_t start) const {
    switch (start) {
        case 0:
            if (aspect_mask & aspect_bits_[0]) {
                return 0;
            }
            [[fallthrough]];
        case 1:
            if (aspect_mask & aspect_bits_[1]) {
                return 1;
            }
            break;
        default:
            break;
    }
    return limits_.aspect_index;
}

uint32_t RangeEncoder::LowerBoundImpl3(VkImageAspectFlags aspect_mask) const {
    if (aspect_mask & aspect_bits_[0]) {
        return 0;
    } else if (aspect_mask & aspect_bits_[1]) {
        return 1;
    } else {
        assert(aspect_mask & aspect_bits_[2]);
        return 2;
    }
}

uint32_t RangeEncoder::LowerBoundWithStartImpl3(VkImageAspectFlags aspect_mask, uint32_t start) const {
    switch (start) {
        case 0:
            if (aspect_mask & aspect_bits_[0]) {
                return 0;
            }
            [[fallthrough]];
        case 1:
            if ((aspect_mask & aspect_bits_[1])) {
                return 1;
            }
            [[fallthrough]];
        case 2:
            if ((aspect_mask & aspect_bits_[2])) {
                return 2;
            }
            break;
        default:
            break;
    }
    return limits_.aspect_index;
}

void RangeEncoder::PopulateFunctionPointers() {
    // Select the encode/decode specialists
    if (limits_.aspect_index == 1) {
        // One aspect use simplified encode/decode math
        if (limits_.arrayLayer == 1) {  // Same as mip_size_ == 1
            encode_function_ = &RangeEncoder::Encode1AspectMipOnly;
            decode_function_ = &RangeEncoder::DecodeAspectMipOnly<1>;
        } else if (limits_.mipLevel == 1) {
            encode_function_ = &RangeEncoder::Encode1AspectArrayOnly;
            decode_function_ = &RangeEncoder::DecodeAspectArrayOnly<1>;
        } else {
            encode_function_ = &RangeEncoder::Encode1AspectMipArray;
            decode_function_ = &RangeEncoder::DecodeAspectMipArray<1>;
        }
        lower_bound_function_ = &RangeEncoder::LowerBoundImpl1;
        lower_bound_with_start_function_ = &RangeEncoder::LowerBoundWithStartImpl1;
    } else if (limits_.aspect_index == 2) {
        // Two aspect use simplified encode/decode math
        if (limits_.arrayLayer == 1) {  // Same as mip_size_ == 1
            encode_function_ = &RangeEncoder::EncodeAspectMipOnly;
            decode_function_ = &RangeEncoder::DecodeAspectMipOnly<2>;
        } else if (limits_.mipLevel == 1) {
            encode_function_ = &RangeEncoder::EncodeAspectArrayOnly;
            decode_function_ = &RangeEncoder::DecodeAspectArrayOnly<2>;
        } else {
            encode_function_ = &RangeEncoder::EncodeAspectMipArray;
            decode_function_ = &RangeEncoder::DecodeAspectMipArray<2>;
        }
        lower_bound_function_ = &RangeEncoder::LowerBoundImpl2;
        lower_bound_with_start_function_ = &RangeEncoder::LowerBoundWithStartImpl2;
    } else {
        encode_function_ = &RangeEncoder::EncodeAspectMipArray;
        decode_function_ = &RangeEncoder::DecodeAspectMipArray<3>;
        lower_bound_function_ = &RangeEncoder::LowerBoundImpl3;
        lower_bound_with_start_function_ = &RangeEncoder::LowerBoundWithStartImpl3;
    }

    // Initialize the offset array
    aspect_base_[0] = 0;
    for (uint32_t i = 1; i < limits_.aspect_index; ++i) {
        aspect_base_[i] = aspect_base_[i - 1] + aspect_size_;
    }
}

RangeEncoder::RangeEncoder(const VkImageSubresourceRange& full_range, const AspectParameters* param)
    : limits_(param->AspectMask(), full_range.levelCount, full_range.layerCount, param->AspectCount()),
      full_range_(full_range),
      mip_size_(full_range.layerCount),
      aspect_size_(mip_size_ * full_range.levelCount),
      aspect_bits_(param->AspectBits()),
      encode_function_(nullptr),
      decode_function_(nullptr) {
    // Only valid to create an encoder for a *whole* image (i.e. base must be zero, and the specified limits_.selected_aspects
    // *must* be equal to the traits aspect mask. (Encoder range assumes zero bases)
    assert(full_range.aspectMask == limits_.aspectMask);
    assert(full_range.baseArrayLayer == 0);
    assert(full_range.baseMipLevel == 0);
    // TODO: should be some static assert
    assert(param->AspectCount() <= kMaxSupportedAspect);
    PopulateFunctionPointers();
}

#ifndef NDEBUG
static bool IsValid(const RangeEncoder& encoder, const VkImageSubresourceRange& bounds) {
    const auto& limits = encoder.Limits();
    return (((bounds.aspectMask & limits.aspectMask) == bounds.aspectMask) &&
            (bounds.baseMipLevel + bounds.levelCount <= limits.mipLevel) &&
            (bounds.baseArrayLayer + bounds.layerCount <= limits.arrayLayer));
}
#endif

// Create an iterator like "generator" that for each increment produces the next index range matching the
// next contiguous (in index space) section of the VkImageSubresourceRange
// Ranges will always span the layerCount layers, and if the layerCount is the full range of the image (as known by
// the encoder) will span the levelCount mip levels as weill.
RangeGenerator::RangeGenerator(const RangeEncoder& encoder, const VkImageSubresourceRange& subres_range)
    : encoder_(&encoder), isr_pos_(encoder, subres_range), pos_(), aspect_base_() {
    assert((((isr_pos_.Limits()).aspectMask & (encoder.Limits()).aspectMask) == (isr_pos_.Limits()).aspectMask));
    assert((isr_pos_.Limits()).baseMipLevel + (isr_pos_.Limits()).levelCount <= (encoder.Limits()).mipLevel);
    assert((isr_pos_.Limits()).baseArrayLayer + (isr_pos_.Limits()).layerCount <= (encoder.Limits()).arrayLayer);

    // To see if we have a full range special case, need to compare the subres_range against the *encoders* limits
    const auto& limits = encoder.Limits();
    if ((subres_range.baseArrayLayer == 0 && subres_range.layerCount == limits.arrayLayer)) {
        if ((subres_range.baseMipLevel == 0) && (subres_range.levelCount == limits.mipLevel)) {
            if (subres_range.aspectMask == limits.aspectMask) {
                // Full range
                pos_.begin = 0;
                pos_.end = encoder.AspectSize() * limits.aspect_index;
                aspect_count_ = 1;  // Flag this to never advance aspects.
            } else {
                // All mips all layers but not all aspect
                pos_.begin = encoder.AspectBase(isr_pos_.aspect_index);
                pos_.end = pos_.begin + encoder.AspectSize();
                aspect_count_ = limits.aspect_index;
            }
        } else {
            // All array layers, but not all levels
            pos_.begin = encoder.AspectBase(isr_pos_.aspect_index) + subres_range.baseMipLevel * encoder.MipSize();
            pos_.end = pos_.begin + subres_range.levelCount * encoder.MipSize();
            aspect_count_ = limits.aspect_index;
        }

        // Full set of array layers at a time, thus we can span across all selected mip levels
        mip_count_ = 1;  // we don't ever advance across mips, as we do all of then in one range
    } else {
        // Each range covers all included array_layers for each selected mip_level for each given selected aspect
        // so we'll use the general purpose encode and smallest range size
        pos_.begin = encoder.Encode(isr_pos_);
        pos_.end = pos_.begin + subres_range.layerCount;

        // we do have to traverse across mips, though (other than Encode abover), we don't have to know which one we are on.
        mip_count_ = subres_range.levelCount;
        aspect_count_ = limits.aspect_index;
    }

    // To get to the next aspect range we offset from the last base
    aspect_base_ = pos_;
    mip_index_ = 0;
    aspect_index_ = isr_pos_.aspect_index;
}

RangeGenerator& RangeGenerator::operator++() {
    mip_index_++;
    // NOTE: If all selected mip levels are done at once, mip_count_ is set to one, not the number of selected mip_levels
    if (mip_index_ >= mip_count_) {
        const auto last_aspect_index = aspect_index_;
        // Seek the next value aspect (if any)
        aspect_index_ = encoder_->LowerBoundFromMask(isr_pos_.Limits().aspectMask, aspect_index_ + 1);
        if (aspect_index_ < aspect_count_) {
            // Force isr_pos to the beginning of this found aspect
            isr_pos_.SeekAspect(aspect_index_);
            // SubresourceGenerator should never be at tombstones we we aren't
            assert(isr_pos_.aspectMask != 0);

            // Offset by the distance between the last start of aspect and *this* start of aspect
            aspect_base_ += (encoder_->AspectBase(isr_pos_.aspect_index) - encoder_->AspectBase(last_aspect_index));
            pos_ = aspect_base_;
            mip_index_ = 0;
        } else {
            // Tombstone both index range and subresource positions to "At end" convention
            pos_ = {0, 0};
            isr_pos_.aspectMask = 0;
        }
    } else {
        // Note: for the layerCount < full_range.layerCount case, because the generated ranges per mip_level are discontinuous
        // we have to do each individual array of ranges
        pos_ += encoder_->MipSize();
        isr_pos_.SeekMip(isr_pos_.Limits().baseMipLevel + mip_index_);
    }
    return *this;
}

ImageRangeEncoder::ImageRangeEncoder(const vvl::Image& image)
    : ImageRangeEncoder(image, AspectParameters::Get(image.full_range.aspectMask)) {}

ImageRangeEncoder::ImageRangeEncoder(const vvl::Image& image, const AspectParameters* param)
    : RangeEncoder(image.full_range, param), total_size_(0U) {
    if (image.create_info.extent.depth > 1) {
        limits_.arrayLayer = image.create_info.extent.depth;
    }

    linear_image_ = false;
    if (image.create_info.tiling == VK_IMAGE_TILING_LINEAR) {
        linear_image_ = true;

        // WORKAROUND for profile and mock_icd not containing valid VkSubresourceLayout yet. Treat it as optimal image.
        const VkImageAspectFlags first_aspect = AspectBit(0);  // AspectBit returns aspects by index
        VkImageSubresource mock_test_subres = {first_aspect, 0, 0};
        VkSubresourceLayout mock_test_layout = {};
        DispatchGetImageSubresourceLayout(image.store_device_as_workaround, image.VkHandle(), &mock_test_subres, &mock_test_layout);
        if (mock_test_layout.size == 0) {  // Happens only for mock icd
            linear_image_ = false;
        }
    }

    // WORKAROUND for not being able to handle general linear images without resulting in non-monotonically increasing ranges
    // Need to clean this up to correctly detect aliasing conflicts between linear image(s) and buffers
    // Issues:
    //     * Lower resolution MIP levels are sometimes hidden in unused space of image rows smaller than minimum stride
    //     * Mutliplane YUV formats may interleave UV rows
    //     * Ranges treat row size as synonymous with row stride, causing ranges to include interleaved content when
    //       checking for hazards or updating state.
    //
    // Needs a rework on how linear range generation is done to ensure correct sizing and monotonicity, before detection of
    // aliased resources can be done correctly.
    linear_image_ = false;

    is_compressed_ = vkuFormatIsCompressed(image.create_info.format);
    texel_block_extent_ = vkuFormatTexelBlockExtent(image.create_info.format);

    is_3_d_ = image.create_info.imageType == VK_IMAGE_TYPE_3D;
    y_interleave_ = false;

    VkSubresourceLayout layout = {};
    VkImageSubresourceLayers subres_layers = {limits_.aspectMask, 0, 0, limits_.arrayLayer};

    for (uint32_t aspect_index = 0; aspect_index < limits_.aspect_index; ++aspect_index) {
        VkImageSubresource subres = {};
        subres.aspectMask = static_cast<VkImageAspectFlags>(AspectBit(aspect_index));
        subres_layers.aspectMask = subres.aspectMask;
        texel_sizes_.push_back(
            vkuFormatTexelSizeWithAspect(image.create_info.format, static_cast<VkImageAspectFlagBits>(subres.aspectMask)));
        IndexType aspect_size = 0;
        for (uint32_t mip_index = 0; mip_index < limits_.mipLevel; ++mip_index) {
            subres_layers.mipLevel = mip_index;
            subres.mipLevel = mip_index;
            auto subres_extent = image.GetEffectiveSubresourceExtent(subres_layers);

            if (linear_image_) {
                DispatchGetImageSubresourceLayout(image.store_device_as_workaround, image.VkHandle(), &subres, &layout);
                if (is_3_d_) {
                    if ((layout.depthPitch == 0) && (subres_extent.depth == 1)) {
                        layout.depthPitch = layout.size;  // Certain implmentations don't supply pitches when size is 1
                    }
                    y_interleave_ = y_interleave_ || (layout.rowPitch > layout.depthPitch);
                } else {
                    if ((layout.arrayPitch == 0) && (limits_.arrayLayer == 1)) {
                        layout.arrayPitch = layout.size;  // Certain implmentations don't supply pitches when size is 1
                    }
                    y_interleave_ = y_interleave_ || (layout.rowPitch > layout.arrayPitch);
                }
            } else {
                layout.offset += layout.size;

                const double row_pitch = subres_extent.width * texel_sizes_[aspect_index];
                // TODO: layout.rowPitch is still computed incorrectly for ASTC_10x10,
                // but it is less trivial fix comparing to arrayPitch, so will be fixed
                // later. There is no known rowPitch bugs, which somehow justifies why
                // rowPitch fix is postponed, and arrayPitch, which affected Angle, was fixed.
                layout.rowPitch = static_cast<VkDeviceSize>(row_pitch);

                layout.arrayPitch = static_cast<VkDeviceSize>(row_pitch * subres_extent.height);
                layout.depthPitch = layout.arrayPitch;
                if (is_3_d_) {
                    layout.size = layout.depthPitch * subres_extent.depth;
                } else {
                    // 2D arrays are not affected by MIP level extent reductions.
                    layout.size = layout.arrayPitch * limits_.arrayLayer;
                }
            }
            subres_info_.emplace_back(layout, subres_extent, texel_block_extent_, texel_sizes_[aspect_index]);
            aspect_size += layout.size;
            total_size_ += layout.size;
        }
        aspect_sizes_.emplace_back(aspect_size);
        aspect_extent_divisors_.emplace_back(
            vkuFindMultiplaneExtentDivisors(image.create_info.format, static_cast<VkImageAspectFlagBits>(subres.aspectMask)));
    }
}

IndexType ImageRangeEncoder::Encode2D(const VkSubresourceLayout& layout, uint32_t layer, uint32_t aspect_index,
                                      const VkOffset3D& offset) const {
    assert(offset.z == 0U);
    // The address offset of the beginning of offset.x's block is:
    //      block_offset_in_bytes = block_offset_x * block_height * texel_sizes_[apsect]
    //      where block_offset_x = floor(offset.x / block_width) * block_width
    //
    // The multiplication by block_height above ensures that we skip the entire block,
    // including block data from other rows, which might sound unintuitive. But it makes
    // sense because the block layout in memory is linear, and to jump to another block,
    // even if we move only in the x direction, we must skip the entire block.
    //
    // Since offset.x must be a multiple of block_width, we can simplify the formula by canceling out block_width.
    //      block_offset_in_bytes = floor(offset.x * block_height * texel_sizes_[apsect])
    double xSize = static_cast<double>(texel_block_extent_.height * texel_sizes_[aspect_index]);
    return layout.offset + layer * layout.arrayPitch + offset.y * layout.rowPitch +
           (offset.x ? static_cast<IndexType>(floor(offset.x * xSize)) : 0U);
}

IndexType ImageRangeEncoder::Encode3D(const VkSubresourceLayout& layout, uint32_t aspect_index, const VkOffset3D& offset) const {
    // See comment in Encode2D.
    double xSize = static_cast<double>(texel_block_extent_.height * texel_sizes_[aspect_index]);
    return layout.offset + offset.z * layout.depthPitch + offset.y * layout.rowPitch +
           (offset.x ? static_cast<IndexType>(floor(offset.x * xSize)) : 0U);
}

void ImageRangeEncoder::Decode(const VkImageSubresource& subres, const IndexType& encode, uint32_t& out_layer,
                               VkOffset3D& out_offset) const {
    uint32_t subres_index = GetSubresourceIndex(LowerBoundFromMask(subres.aspectMask), subres.mipLevel);
    const auto& subres_layout = GetSubresourceInfo(subres_index).layout;
    IndexType decode = encode - subres_layout.offset;
    out_layer = static_cast<uint32_t>(decode / subres_layout.arrayPitch);
    decode -= (out_layer * subres_layout.arrayPitch);
    out_offset.z = static_cast<int32_t>(decode / subres_layout.depthPitch);
    decode -= (out_offset.z * subres_layout.depthPitch);
    out_offset.y = static_cast<int32_t>(decode / subres_layout.rowPitch);
    decode -= (out_offset.y * subres_layout.rowPitch);
    out_offset.x = static_cast<int32_t>(static_cast<double>(decode) / texel_sizes_[LowerBoundFromMask(subres.aspectMask)]);
}


inline VkImageSubresourceRange GetRemaining(const VkImageSubresourceRange& full_range, VkImageSubresourceRange subres_range) {
    if (subres_range.levelCount == VK_REMAINING_MIP_LEVELS) {
        subres_range.levelCount = full_range.levelCount - subres_range.baseMipLevel;
    }
    if (subres_range.layerCount == VK_REMAINING_ARRAY_LAYERS) {
        subres_range.layerCount = full_range.layerCount - subres_range.baseArrayLayer;
    }
    return subres_range;
}
inline bool CoversAllLayers(const VkImageSubresourceRange& full_range, VkImageSubresourceRange subres_range) {
    return (subres_range.baseArrayLayer == 0) && (subres_range.layerCount == full_range.layerCount);
}
static bool SubresourceRangeIsEmpty(const VkImageSubresourceRange& range) {
    return (0 == range.aspectMask) || (0 == range.levelCount) || (0 == range.layerCount);
}
static bool ExtentIsEmpty(const VkExtent3D& extent) { return (0 == extent.width) || (0 == extent.height) || (0 == extent.width); }

VkOffset3D ImageRangeGenerator::GetOffset(uint32_t aspect_index) const {
    // Return the effective offset taking into account the multiplane extent divisor
    auto offset = offset_;
    auto divisor = encoder_->GetAspectExtentDivisors(aspect_index);
    offset.x /= divisor.width;
    offset.y /= divisor.height;
    return offset;
}

VkExtent3D ImageRangeGenerator::GetExtent(uint32_t aspect_index) const {
    // Return the effective extent taking into account the multiplane extent divisor
    // Note that we also have to look at the offset, because it affects the rounding
    auto offset = GetOffset(aspect_index);
    auto extent = extent_;
    auto divisor = encoder_->GetAspectExtentDivisors(aspect_index);
    extent.width = ((offset_.x + extent.width + divisor.width - 1) / divisor.width) - offset.x;
    extent.height = ((offset_.y + extent.height + divisor.height - 1) / divisor.height) - offset.y;
    return extent;
}

void ImageRangeGenerator::SetInitialPosFullOffset(uint32_t layer, uint32_t aspect_index) {
    const auto offset = GetOffset(aspect_index);
    const auto extent = GetExtent(aspect_index);
    const bool is_3D = encoder_->Is3D();
    const auto& subres_layout = subres_info_->layout;
    const IndexType encode_base = is_3D ? encoder_->Encode3D(subres_layout, aspect_index, offset)
                                        : encoder_->Encode2D(subres_layout, layer, aspect_index, offset);
    const IndexType base = base_address_ + encode_base;
    // To deal with compressed formats the span must cover the y-extent of lines (something we resmember in the y_step)
    const IndexType span = static_cast<IndexType>(floor(encoder_->TexelSize(aspect_index) * (extent.width * incr_state_.y_step)));

    const uint32_t z_count = is_3D ? extent.depth : subres_range_.layerCount;
    const IndexType z_pitch = is_3D ? subres_info_->z_step_pitch : subres_layout.arrayPitch;
    incr_state_.Set(extent.height, z_count, base, span, subres_info_->y_step_pitch, z_pitch);
}

void ImageRangeGenerator::SetInitialPosFullWidth(uint32_t layer, uint32_t aspect_index) {
    assert(!encoder_->IsInterleaveY() && (offset_.x == 0));
    const auto offset = GetOffset(aspect_index);
    const auto extent = GetExtent(aspect_index);
    const bool is_3D = encoder_->Is3D();
    const auto& subres_layout = subres_info_->layout;
    const IndexType encode_base = is_3D ? encoder_->Encode3D(subres_layout, aspect_index, offset)
                                        : encoder_->Encode2D(subres_layout, layer, aspect_index, offset);
    const IndexType base = base_address_ + encode_base;
    // Height must be in multiples of y_step (the texel dimension)... validated elsewhere
    const IndexType span = subres_layout.rowPitch * extent.height;

    const uint32_t z_count = is_3D ? extent.depth : subres_range_.layerCount;
    const IndexType z_pitch = is_3D ? subres_info_->z_step_pitch : subres_layout.arrayPitch;
    incr_state_.Set(1U, z_count, base, span, subres_info_->y_step_pitch, z_pitch);
}

void ImageRangeGenerator::SetInitialPosFullHeight(uint32_t layer, uint32_t aspect_index) {
    assert(!encoder_->Is3D() && (offset_.x == 0) && (offset_.y == 0));
    const auto& subres_layout = subres_info_->layout;
    const IndexType base = base_address_ + subres_layout.offset + subres_range_.baseArrayLayer * subres_layout.arrayPitch;
    const IndexType span = subres_info_->layer_span;
    const IndexType z_step = subres_layout.arrayPitch;

    incr_state_.Set(1, subres_range_.layerCount, base, span, span, z_step);
}

void ImageRangeGenerator::SetInitialPosSomeDepth(uint32_t layer, uint32_t aspect_index) {
    assert(encoder_->Is3D() && (offset_.x == 0) && (offset_.y == 0) && (layer == 0));
    const auto offset = GetOffset(aspect_index);
    const auto extent = GetExtent(aspect_index);
    const auto& subres_layout = subres_info_->layout;
    const IndexType encode_base = encoder_->Encode3D(subres_layout, aspect_index, offset);
    const IndexType base = base_address_ + encode_base;
    // Height must be in multiples of z_step (the texel dimension)... validated elsewhere
    const IndexType span = subres_layout.depthPitch * extent.depth;

    incr_state_.Set(1, 1, base, span, span, subres_layout.size);
}

void ImageRangeGenerator::SetInitialPosFullDepth(uint32_t layer, uint32_t aspect_index) {
    assert(encoder_->Is3D() && (offset_.x == 0) && (offset_.y == 0) && (offset_.z == 0) && (layer == 0));
    const auto& subres_layout = subres_info_->layout;
    const IndexType base = base_address_ + subres_layout.offset;
    const IndexType span = subres_layout.depthPitch * extent_.depth;

    incr_state_.Set(1, 1, base, span, span, span);
}

void ImageRangeGenerator::SetInitialPosSomeLayers(uint32_t layer, uint32_t aspect_index) {
    assert(!encoder_->Is3D() && (offset_.x == 0) && (offset_.y == 0) && (offset_.z == 0));
    const auto& subres_layout = subres_info_->layout;
    const IndexType base = base_address_ + subres_layout.offset + layer * subres_layout.arrayPitch;
    const IndexType span = subres_layout.arrayPitch * subres_range_.layerCount;
    const IndexType z_step = subres_layout.arrayPitch * encoder_->Limits().arrayLayer;
    incr_state_.Set(1, 1, base, span, span, z_step);
}

void ImageRangeGenerator::SetInitialPosAllLayers(uint32_t layer, uint32_t aspect_index) {
    assert(!encoder_->Is3D() && (offset_.x == 0) && (offset_.y == 0) && (offset_.z == 0) &&
           (layer == 0));
    const auto& subres_layout = subres_info_->layout;
    const IndexType base = base_address_ + subres_layout.offset;
    const IndexType span = subres_layout.arrayPitch * subres_range_.layerCount;
    incr_state_.Set(1, 1, base, span, span, span);
}

void ImageRangeGenerator::SetInitialPosOneAspect(uint32_t layer, uint32_t aspect_index) {
    assert(!encoder_->IsLinearImage());  // Requires the major minor of "idealized/non-linear" images
    const auto& subres_layout = subres_info_->layout;
    const IndexType base = base_address_ + subres_layout.offset;
    IndexType span = 0;
    if (subres_range_.levelCount == encoder_->Limits().mipLevel) {
        span = encoder_->GetAspectSize(aspect_index);
    } else {
        // Add up the mip sizes...
        // Assumes subres_info is pointing to index(baseMipLevel, aspect_index)
        // Assumes mip major order...
        for (uint32_t level = 0; level < subres_range_.levelCount; level++) {
            span += subres_info_[level].layout.size;
        }
    }
    incr_mip_ = subres_range_.levelCount;
    incr_state_.Set(1, 1, base, span, span, span);
}

void ImageRangeGenerator::SetInitialPosAllSubres(uint32_t layer, uint32_t aspect_index) {
    assert(!encoder_->IsLinearImage());
    const IndexType base = base_address_;
    const IndexType span = encoder_->TotalSize();

    // Just one range... everything, ++ will short circuit to "end"
    single_full_size_range_ = true;
    // We don't need to set up the rest of the incrementer, just the starting position
    incr_state_.y_base = {base, base + span};
}

bool ImageRangeGenerator::Convert2DCompatibleTo3D() {
    if (encoder_->Is3D() && is_depth_sliced_) {
        // This only valid for 2D compatible 3D images
        // Touch up the extent and the subres to make this look like a depth extent
        offset_.z = subres_range_.baseArrayLayer;
        subres_range_.baseArrayLayer = 0;
        extent_.depth = subres_range_.layerCount;
        subres_range_.layerCount = 1;
        return true;
    }
    return false;
}
ImageRangeGenerator::ImageRangeGenerator(const ImageRangeEncoder& encoder, const VkImageSubresourceRange& subres_range,
                                         VkDeviceSize base_address, bool is_depth_sliced)
    : encoder_(&encoder),
      subres_range_(GetRemaining(encoder.FullRange(), subres_range)),
      offset_(),
      extent_(),
      base_address_(base_address),
      is_depth_sliced_(is_depth_sliced) {
#ifndef NDEBUG
    assert(IsValid(*encoder_, subres_range_));
#endif
    if (SubresourceRangeIsEmpty(subres_range)) {
        // Not robust to empty ranges, so for to "at end" condition.
        pos_ = {0, 0};
        return;
    }

    SetUpSubresInfo();
    extent_ = subres_info_->extent;
    const bool converted = Convert2DCompatibleTo3D();
    SetUpIncrementerDefaults();
    if (converted && (extent_.depth != subres_info_->extent.depth)) {
        SetUpIncrementer(true, true, false);
    } else {
        SetUpSubresIncrementer();
    }
    SetInitialPos(subres_range_.baseArrayLayer, aspect_index_);
    pos_ = incr_state_.y_base;
}

ImageRangeGenerator::ImageRangeGenerator(const ImageRangeEncoder& encoder, const VkImageSubresourceRange& subres_range,
                                         const VkOffset3D& offset, const VkExtent3D& extent, VkDeviceSize base_address,
                                         bool is_depth_sliced)
    : encoder_(&encoder),
      subres_range_(GetRemaining(encoder.FullRange(), subres_range)),
      offset_(offset),
      extent_(extent),
      base_address_(base_address),
      is_depth_sliced_(is_depth_sliced) {
#ifndef NDEBUG
    assert(IsValid(*encoder_, subres_range_));
#endif

    assert(subres_range_.levelCount == 1);
    if (SubresourceRangeIsEmpty(subres_range)) {
        // Empty range forces empty position -- no operations other than deref for empty check are valid
        pos_ = {0, 0};
        return;
    }

    // When passing in an offset and extent, *must* only specify *one* mip level
    SetUpSubresInfo();
    Convert2DCompatibleTo3D();

    const VkExtent3D& subres_extent = subres_info_->extent;
    if (ExtentIsEmpty(extent_) || ((extent_.width + offset_.x) > subres_extent.width) ||
        ((extent_.height + offset_.y) > subres_extent.height) || ((extent_.depth + offset_.z) > subres_extent.depth)) {
        // Empty range forces empty position -- no operations other than deref for empty check are valid
        pos_ = {0, 0};
        return;
    }

    const bool all_width = (offset.x == 0) && (extent_.width == subres_extent.width);
    const bool all_height = (offset.y == 0) && (extent_.height == subres_extent.height);
    const bool all_depth = !encoder_->Is3D() || ((offset.z == 0) && (extent_.depth == subres_extent.depth));

    SetUpIncrementerDefaults();
    SetUpIncrementer(all_width, all_height, all_depth);
    SetInitialPos(subres_range_.baseArrayLayer, aspect_index_);
    pos_ = incr_state_.y_base;
}

void ImageRangeGenerator::SetUpSubresInfo() {
    mip_index_ = 0;
    aspect_index_ = encoder_->LowerBoundFromMask(subres_range_.aspectMask);
    subres_index_ = encoder_->GetSubresourceIndex(aspect_index_, subres_range_.baseMipLevel);
    subres_info_ = &encoder_->GetSubresourceInfo(subres_index_);
}

void ImageRangeGenerator::SetUpIncrementerDefaults() {
    // These are safe defaults that most SetInitialPos* will use.  Those that need to change them, do.
    incr_state_.y_step = encoder_->TexelBlockExtent().height;
    incr_state_.layer_z_step = encoder_->Is3D() ? encoder_->TexelBlockExtent().depth : 1U;
    incr_mip_ = 1;
    single_full_size_range_ = false;
}

// Assumes full extent in width/height/depth(if present)
void ImageRangeGenerator::SetUpSubresIncrementer() {
    const auto& full_range = encoder_->FullRange();
    const bool linear_image = encoder_->IsLinearImage();
    const bool is_3d = encoder_->Is3D();
    const bool layers_interleave = linear_image && (subres_info_->layout.arrayPitch > subres_info_->layout.size);
    if (layers_interleave) {
        // The implementation can interleave arrays, aspects, and mips arbitrarily
        if (encoder_->Is3D()) {
            set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosFullDepth;
        } else {
            set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosFullHeight;
        }
    } else if (is_3d || CoversAllLayers(full_range, subres_range_)) {
        if (!linear_image) {
            // Linear images are defined by the implementation and so we can't assume the ordering we use here
            const bool all_mips = (subres_range_.baseMipLevel == 0) && (subres_range_.levelCount == full_range.levelCount);
            const bool all_aspects = subres_range_.aspectMask == full_range.aspectMask;
            if (all_aspects && all_mips) {
                set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosAllSubres;
            } else {
                set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosOneAspect;
            }
        } else if (is_3d) {
            // 3D implies CoversAllLayers
            set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosFullDepth;
        } else {
            set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosAllLayers;
        }
    } else {
        set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosSomeLayers;
    }
}

void ImageRangeGenerator::SetUpIncrementer(bool all_width, bool all_height, bool all_depth) {
    if (!all_width || encoder_->IsInterleaveY()) {
        // Dimensional majority is not guaranteed for Linear images except in X
        // For tiled images we can use "idealized" addresses
        set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosFullOffset;
    } else if (!all_height) {
        set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosFullWidth;
    } else if (encoder_->Is3D() && !all_depth) {
        set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosSomeDepth;
    } else {
        SetUpSubresIncrementer();
    }
}

ImageRangeGenerator& ImageRangeGenerator::operator++() {
    // Short circuit
    if (single_full_size_range_) {
        // Advance directly to end
        pos_ = {0, 0};
        return *this;
    }

    incr_state_.y_index += incr_state_.y_step;
    if (incr_state_.y_index < incr_state_.y_count) {
        incr_state_.y_base += incr_state_.incr_y;
        pos_ = incr_state_.y_base;
    } else {
        incr_state_.layer_z_index += incr_state_.layer_z_step;
        if (incr_state_.layer_z_index < incr_state_.layer_z_count) {
            incr_state_.layer_z_base += incr_state_.incr_layer_z;
            incr_state_.y_base = incr_state_.layer_z_base;
            pos_ = incr_state_.y_base;
        } else {
            // For aspects and mips we need to move to a new subresource layer info
            mip_index_ += incr_mip_;
            if (mip_index_ < subres_range_.levelCount) {
                // NOTE: This means that ImageRangeGenerator is relying on the major/minor ordering of mip and aspect in the
                subres_index_ += incr_mip_;
                extent_ = subres_info_->extent;  // Overwrites input extent, but > 1 MIP isn't valid with input extent
            } else {
                const auto next_aspect_index = encoder_->LowerBoundFromMask(subres_range_.aspectMask, aspect_index_ + 1);
                if (next_aspect_index < encoder_->Limits().aspect_index) {
                    //       SubresourceLayout info in ImageRangeEncoder... it's a cheat, but it was a hotspot.
                    aspect_index_ = next_aspect_index;
                    mip_index_ = 0;
                    subres_index_ = encoder_->GetSubresourceIndex(aspect_index_, subres_range_.baseMipLevel);
                } else {
                    // At End
                    pos_ = {0, 0};
                    return *this;
                }
            }

            subres_info_ = &encoder_->GetSubresourceInfo(subres_index_);
            SetInitialPos(subres_range_.baseArrayLayer, aspect_index_);
            pos_ = incr_state_.y_base;
        }
    }

    return *this;
}

template <typename AspectTraits>
class AspectParametersImpl : public AspectParameters {
  public:
    VkImageAspectFlags AspectMask() const override { return AspectTraits::kAspectMask; }
    uint32_t AspectCount() const override { return AspectTraits::kAspectCount; };
    const VkImageAspectFlagBits* AspectBits() const override { return AspectTraits::AspectBits().data(); }
};

struct ColorAspectTraits {
    static constexpr uint32_t kAspectCount = 1;
    static constexpr VkImageAspectFlags kAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    static const std::array<VkImageAspectFlagBits, kAspectCount>& AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> k_aspect_bits{{VK_IMAGE_ASPECT_COLOR_BIT}};
        return k_aspect_bits;
    }
};

struct DepthAspectTraits {
    static constexpr uint32_t kAspectCount = 1;
    static constexpr VkImageAspectFlags kAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    static const std::array<VkImageAspectFlagBits, kAspectCount>& AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> k_aspect_bits{{VK_IMAGE_ASPECT_DEPTH_BIT}};
        return k_aspect_bits;
    }
};

struct StencilAspectTraits {
    static constexpr uint32_t kAspectCount = 1;
    static constexpr VkImageAspectFlags kAspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    static const std::array<VkImageAspectFlagBits, kAspectCount>& AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> k_aspect_bits{{VK_IMAGE_ASPECT_STENCIL_BIT}};
        return k_aspect_bits;
    }
};

struct DepthStencilAspectTraits {
    // VK_IMAGE_ASPECT_DEPTH_BIT = 0x00000002,  >> 1 -> 1 -1 -> 0
    // VK_IMAGE_ASPECT_STENCIL_BIT = 0x00000004, >> 1 -> 2 -1 = 1
    static constexpr uint32_t kAspectCount = 2;
    static constexpr VkImageAspectFlags kAspectMask = (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    static const std::array<VkImageAspectFlagBits, kAspectCount>& AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> k_aspect_bits{
            {VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_ASPECT_STENCIL_BIT}};
        return k_aspect_bits;
    }
};

struct Multiplane2AspectTraits {
    // VK_IMAGE_ASPECT_PLANE_0_BIT = 0x00000010, >> 4 - 1 -> 0
    // VK_IMAGE_ASPECT_PLANE_1_BIT = 0x00000020, >> 4 - 1 -> 1
    static constexpr uint32_t kAspectCount = 2;
    static constexpr VkImageAspectFlags kAspectMask = (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT);
    static const std::array<VkImageAspectFlagBits, kAspectCount>& AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> k_aspect_bits{
            {VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT}};
        return k_aspect_bits;
    }
};

struct Multiplane3AspectTraits {
    // VK_IMAGE_ASPECT_PLANE_0_BIT = 0x00000010, >> 4 - 1 -> 0
    // VK_IMAGE_ASPECT_PLANE_1_BIT = 0x00000020, >> 4 - 1 -> 1
    // VK_IMAGE_ASPECT_PLANE_2_BIT = 0x00000040, >> 4 - 1 -> 3
    static constexpr uint32_t kAspectCount = 3;
    static constexpr VkImageAspectFlags kAspectMask =
        (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT);
    static const std::array<VkImageAspectFlagBits, kAspectCount>& AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> k_aspect_bits{
            {VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT, VK_IMAGE_ASPECT_PLANE_2_BIT}};
        return k_aspect_bits;
    }
};

// Create the encoder parameter suitable to the full range aspect mask (*must* be canonical)
const AspectParameters* AspectParameters::Get(VkImageAspectFlags aspect_mask) {
    // We need a persitent instance of each specialist containing only a VTABLE each
    static const AspectParametersImpl<ColorAspectTraits> k_color_param;
    static const AspectParametersImpl<DepthAspectTraits> k_depth_param;
    static const AspectParametersImpl<StencilAspectTraits> k_stencil_param;
    static const AspectParametersImpl<DepthStencilAspectTraits> k_depth_stencil_param;
    static const AspectParametersImpl<Multiplane2AspectTraits> k_mutliplane2_param;
    static const AspectParametersImpl<Multiplane3AspectTraits> k_mutliplane3_param;

    const AspectParameters* param = nullptr;
    switch (aspect_mask) {
        case ColorAspectTraits::kAspectMask:
            param = &k_color_param;
            break;
        case DepthAspectTraits::kAspectMask:
            param = &k_depth_param;
            break;
        case StencilAspectTraits::kAspectMask:
            param = &k_stencil_param;
            break;
        case DepthStencilAspectTraits::kAspectMask:
            param = &k_depth_stencil_param;
            break;
        case Multiplane2AspectTraits::kAspectMask:
            param = &k_mutliplane2_param;
            break;
        case Multiplane3AspectTraits::kAspectMask:
            param = &k_mutliplane3_param;
            break;
        default:
            assert(false);
    }
    return param;
}

inline ImageRangeEncoder::SubresInfo::SubresInfo(const VkSubresourceLayout& layout_, const VkExtent3D& extent_,
                                                 const VkExtent3D& texel_extent, double texel_size)
    : layout(layout_),
      extent(extent_),
      y_step_pitch(layout.rowPitch * texel_extent.height),
      z_step_pitch(layout.depthPitch * texel_extent.depth),
      layer_span(layout.rowPitch * extent_.height) {}

ImageRangeEncoder::SubresInfo::SubresInfo(const SubresInfo&rhs)
    : layout(rhs.layout),
      extent(rhs.extent),
      y_step_pitch(rhs.y_step_pitch),
      z_step_pitch(rhs.z_step_pitch),
      layer_span(rhs.layer_span) {}

void ImageRangeGenerator::IncrementerState::Set(uint32_t y_count_, uint32_t layer_z_count_, IndexType base, IndexType span,
                                                IndexType y_step, IndexType z_step) {
    y_count = y_count_;
    layer_z_count = layer_z_count_;
    y_index = 0;
    layer_z_index = 0;
    y_base.begin = base;
    y_base.end = base + span;
    layer_z_base = y_base;
    incr_y = y_step;
    incr_layer_z = z_step;
}

}  // namespace subresource_adapter
