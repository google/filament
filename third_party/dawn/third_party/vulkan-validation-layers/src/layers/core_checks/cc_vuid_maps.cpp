/* Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
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
#include "cc_vuid_maps.h"
#include "error_message/error_location.h"
#include "utils/vk_layer_utils.h"
#include "state_tracker/pipeline_state.h"
#include <map>

namespace vvl {

const std::string &GetCopyBufferImageDeviceVUID(const Location &loc, CopyError error) {
    static const std::map<CopyError, std::array<Entry, 4>> errors{
        {CopyError::TexelBlockSize_07975,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07975"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07975"},
             {Key(Func::vkCmdCopyBufferToImage2), "VUID-VkCopyBufferToImageInfo2-dstImage-07975"},
             {Key(Func::vkCmdCopyImageToBuffer2), "VUID-VkCopyImageToBufferInfo2-srcImage-07975"},
         }}},
        {CopyError::MultiPlaneCompatible_07976,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07976"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07976"},
             {Key(Func::vkCmdCopyBufferToImage2), "VUID-VkCopyBufferToImageInfo2-dstImage-07976"},
             {Key(Func::vkCmdCopyImageToBuffer2), "VUID-VkCopyImageToBufferInfo2-srcImage-07976"},
         }}},
        {CopyError::TransferGranularity_07747,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-imageOffset-07738"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-imageOffset-07747"},
             {Key(Func::vkCmdCopyBufferToImage2), "VUID-vkCmdCopyBufferToImage2-imageOffset-07738"},
             {Key(Func::vkCmdCopyImageToBuffer2), "VUID-vkCmdCopyImageToBuffer2-imageOffset-07747"},
         }}},
        {CopyError::BufferOffset_07737,
         {{
             // was split up in 1.3.236 spec (internal MR 5371)
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-commandBuffer-07737"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-commandBuffer-07746"},
             {Key(Func::vkCmdCopyBufferToImage2), "VUID-vkCmdCopyBufferToImage2-commandBuffer-07737"},
             {Key(Func::vkCmdCopyImageToBuffer2), "VUID-vkCmdCopyImageToBuffer2-commandBuffer-07746"},
         }}},
        {CopyError::BufferOffset_07978,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07978"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07978"},
             {Key(Func::vkCmdCopyBufferToImage2), "VUID-VkCopyBufferToImageInfo2-dstImage-07978"},
             {Key(Func::vkCmdCopyImageToBuffer2), "VUID-VkCopyImageToBufferInfo2-srcImage-07978"},
         }}},
        {CopyError::MemoryOverlap_00173,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-pRegions-00173"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-pRegions-00184"},
             {Key(Func::vkCmdCopyBufferToImage2), "VUID-VkCopyBufferToImageInfo2-pRegions-00173"},
             {Key(Func::vkCmdCopyImageToBuffer2), "VUID-VkCopyImageToBufferInfo2-pRegions-00184"},
         }}},
        {CopyError::ImageExtentWidthZero_06659,
         {{
             {Key(Struct::VkBufferImageCopy), "VUID-VkBufferImageCopy-imageExtent-06659"},
             {Key(Struct::VkBufferImageCopy2), "VUID-VkBufferImageCopy2-imageExtent-06659"},
             {Key(Struct::VkMemoryToImageCopy), "VUID-VkMemoryToImageCopy-imageExtent-06659"},
             {Key(Struct::VkImageToMemoryCopy), "VUID-VkImageToMemoryCopy-imageExtent-06659"},
         }}},
        {CopyError::ImageExtentHeightZero_06660,
         {{
             {Key(Struct::VkBufferImageCopy), "VUID-VkBufferImageCopy-imageExtent-06660"},
             {Key(Struct::VkBufferImageCopy2), "VUID-VkBufferImageCopy2-imageExtent-06660"},
             {Key(Struct::VkMemoryToImageCopy), "VUID-VkMemoryToImageCopy-imageExtent-06660"},
             {Key(Struct::VkImageToMemoryCopy), "VUID-VkImageToMemoryCopy-imageExtent-06660"},
         }}},
        {CopyError::ImageExtentDepthZero_06661,
         {{
             {Key(Struct::VkBufferImageCopy), "VUID-VkBufferImageCopy-imageExtent-06661"},
             {Key(Struct::VkBufferImageCopy2), "VUID-VkBufferImageCopy2-imageExtent-06661"},
             {Key(Struct::VkMemoryToImageCopy), "VUID-VkMemoryToImageCopy-imageExtent-06661"},
             {Key(Struct::VkImageToMemoryCopy), "VUID-VkImageToMemoryCopy-imageExtent-06661"},
         }}},
        {CopyError::ImageExtentRowLength_09101,
         {{
             {Key(Struct::VkBufferImageCopy), "VUID-VkBufferImageCopy-bufferRowLength-09101"},
             {Key(Struct::VkBufferImageCopy2), "VUID-VkBufferImageCopy2-bufferRowLength-09101"},
             {Key(Struct::VkMemoryToImageCopy), "VUID-VkMemoryToImageCopy-memoryRowLength-09101"},
             {Key(Struct::VkImageToMemoryCopy), "VUID-VkImageToMemoryCopy-memoryRowLength-09101"},
         }}},
        {CopyError::ImageExtentImageHeight_09102,
         {{
             {Key(Struct::VkBufferImageCopy), "VUID-VkBufferImageCopy-bufferImageHeight-09102"},
             {Key(Struct::VkBufferImageCopy2), "VUID-VkBufferImageCopy2-bufferImageHeight-09102"},
             {Key(Struct::VkMemoryToImageCopy), "VUID-VkMemoryToImageCopy-memoryImageHeight-09102"},
             {Key(Struct::VkImageToMemoryCopy), "VUID-VkImageToMemoryCopy-memoryImageHeight-09102"},
         }}},
        {CopyError::AspectMaskSingleBit_09103,
         {{
             {Key(Struct::VkBufferImageCopy), "VUID-VkBufferImageCopy-aspectMask-09103"},
             {Key(Struct::VkBufferImageCopy2), "VUID-VkBufferImageCopy2-aspectMask-09103"},
             {Key(Struct::VkMemoryToImageCopy), "VUID-VkMemoryToImageCopy-aspectMask-09103"},
             {Key(Struct::VkImageToMemoryCopy), "VUID-VkImageToMemoryCopy-aspectMask-09103"},
         }}},
        {CopyError::ExceedBufferBounds_00171,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-pRegions-00171"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-pRegions-00183"},
             {Key(Func::vkCmdCopyBufferToImage2), "VUID-VkCopyBufferToImageInfo2-pRegions-00171"},
             {Key(Func::vkCmdCopyImageToBuffer2), "VUID-VkCopyImageToBufferInfo2-pRegions-00183"},
         }}},
    };

    // It is error prone to have every call set the struct
    // Since there are a known mapping, easier to do here when we are about to print an error message
    Struct s = loc.structure;
    Func f = loc.function;
    if (IsValueIn(loc.function, {Func::vkCmdCopyImageToBuffer, Func::vkCmdCopyBufferToImage})) {
        s = Struct::VkBufferImageCopy;
    } else if (IsValueIn(loc.function, {Func::vkCmdCopyBufferToImage2, Func::vkCmdCopyBufferToImage2KHR,
                                        Func::vkCmdCopyImageToBuffer2, Func::vkCmdCopyImageToBuffer2KHR})) {
        s = Struct::VkBufferImageCopy2;
    } else if (loc.function == Func::vkCopyImageToMemory || loc.function == Func::vkCopyImageToMemoryEXT) {
        s = Struct::VkImageToMemoryCopy;
    } else if (loc.function == Func::vkCopyMemoryToImage || loc.function == Func::vkCopyMemoryToImageEXT) {
        s = Struct::VkMemoryToImageCopy;
    }
    const Location updated_loc(f, s, loc.field, loc.index);

    const auto &result = FindVUID(error, updated_loc, errors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-copy-buffer");
        return unhandled;
    }
    return result;
}

const std::string &GetCopyBufferImageVUID(const Location &loc, CopyError error) {
    static const std::map<CopyError, std::array<Entry, 6>> errors{
        {CopyError::ImageOffest_07971,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-imageSubresource-07971"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-imageSubresource-07971"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-pRegions-06223"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-imageOffset-00197"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-imageSubresource-07971"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-imageSubresource-07971"},
         }}},
        {CopyError::ImageOffest_07972,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-imageSubresource-07972"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-imageSubresource-07972"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-pRegions-06224"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-imageOffset-00198"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-imageSubresource-07972"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-imageSubresource-07972"},
         }}},
        {CopyError::Image1D_07979,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07979"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07979"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07979"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07979"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-dstImage-07979"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-srcImage-07979"},
         }}},
        {CopyError::Image1D_07980,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07980"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07980"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07980"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07980"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-dstImage-07980"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-srcImage-07980"},
         }}},
        {CopyError::Image3D_07983,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07983"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07983"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07983"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07983"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-dstImage-07983"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-srcImage-07983"},
         }}},
        {CopyError::TexelBlockExtentWidth_07274,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07274"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07274"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07274"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07274"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-dstImage-07274"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-srcImage-07274"},
         }}},
        {CopyError::TexelBlockExtentHeight_07275,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07275"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07275"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07275"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07275"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-dstImage-07275"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-srcImage-07275"},
         }}},
        {CopyError::TexelBlockExtentDepth_07276,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07276"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07276"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07276"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07276"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-dstImage-07276"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-srcImage-07276"},
         }}},
        {CopyError::TexelBlockExtentWidth_00207,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-00207"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-00207"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-00207"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-00207"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-dstImage-00207"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-srcImage-00207"},
         }}},
        {CopyError::TexelBlockExtentHeight_00208,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-00208"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-00208"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-00208"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-00208"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-dstImage-00208"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-srcImage-00208"},
         }}},
        {CopyError::TexelBlockExtentDepth_00209,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-00209"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-00209"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-00209"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-00209"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-dstImage-00209"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-srcImage-00209"},
         }}},
        {CopyError::MultiPlaneAspectMask_07981,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07981"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07981"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07981"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07981"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-dstImage-07981"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-srcImage-07981"},
         }}},
        {CopyError::ImageOffest_09104,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-imageOffset-09104"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-imageOffset-09104"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-imageOffset-09104"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-imageOffset-09104"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-imageOffset-09104"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-imageOffset-09104"},
         }}},
        {CopyError::AspectMask_09105,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-imageSubresource-09105"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-imageSubresource-09105"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-imageSubresource-09105"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-imageSubresource-09105"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-imageSubresource-09105"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-imageSubresource-09105"},
         }}},
        {CopyError::bufferRowLength_09106,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-bufferRowLength-09106"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-bufferRowLength-09106"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-bufferRowLength-09106"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-bufferRowLength-09106"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-memoryRowLength-09106"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-memoryRowLength-09106"},
         }}},
        {CopyError::bufferImageHeight_09107,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-bufferImageHeight-09107"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-bufferImageHeight-09107"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-bufferImageHeight-09107"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-bufferImageHeight-09107"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-memoryImageHeight-09107"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-memoryImageHeight-09107"},
         }}},
        {CopyError::bufferRowLength_09108,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-bufferRowLength-09108"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-bufferRowLength-09108"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-bufferRowLength-09108"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-bufferRowLength-09108"},
             {Key(Struct::VkCopyMemoryToImageInfo), "VUID-VkCopyMemoryToImageInfo-memoryRowLength-09108"},
             {Key(Struct::VkCopyImageToMemoryInfo), "VUID-VkCopyImageToMemoryInfo-memoryRowLength-09108"},
         }}},
    };

    // It is error prone to have every call set the struct
    // Since there are a known mapping, easier to do here when we are about to print an error message
    Struct s = loc.structure;
    Func f = loc.function;
    if (IsValueIn(loc.function, {Func::vkCmdCopyImageToBuffer2, Func::vkCmdCopyImageToBuffer2KHR})) {
        s = Struct::VkCopyImageToBufferInfo2;
    } else if (IsValueIn(loc.function, {Func::vkCmdCopyBufferToImage2, Func::vkCmdCopyBufferToImage2KHR})) {
        s = Struct::VkCopyBufferToImageInfo2;
    } else if (loc.function == Func::vkCopyImageToMemory || loc.function == Func::vkCopyImageToMemoryEXT) {
        s = Struct::VkCopyImageToMemoryInfo;
    } else if (loc.function == Func::vkCopyMemoryToImage || loc.function == Func::vkCopyMemoryToImageEXT) {
        s = Struct::VkCopyMemoryToImageInfo;
    }
    const Location updated_loc(f, s, loc.field, loc.index);

    const auto &result = FindVUID(error, updated_loc, errors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-copy-buffer-image");
        return unhandled;
    }
    return result;
}

const std::string &GetCopyImageVUID(const Location &loc, CopyError error) {
    static const std::map<CopyError, std::array<Entry, 3>> errors{
        {CopyError::SrcImage1D_00146,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcImage-00146"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcImage-00146"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-srcImage-07979"},
         }}},
        {CopyError::DstImage1D_00152,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstImage-00152"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstImage-00152"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-dstImage-07979"},
         }}},
        {CopyError::SrcImage1D_01785,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcImage-01785"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcImage-01785"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-srcImage-07980"},
         }}},
        {CopyError::DstImage1D_01786,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstImage-01786"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstImage-01786"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-dstImage-07980"},
         }}},
        {CopyError::SrcOffset_01728,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcImage-01728"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcImage-01728"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-srcImage-00207"},
         }}},
        {CopyError::SrcOffset_01729,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcImage-01729"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcImage-01729"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-srcImage-00208"},
         }}},
        {CopyError::SrcOffset_01730,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcImage-01730"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcImage-01730"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-srcImage-00209"},
         }}},
        {CopyError::DstOffset_01732,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstImage-01732"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstImage-01732"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-dstImage-00207"},
         }}},
        {CopyError::DstOffset_01733,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstImage-01733"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstImage-01733"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-dstImage-00208"},
         }}},
        {CopyError::DstOffset_01734,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstImage-01734"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstImage-01734"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-dstImage-00209"},
         }}},
        {CopyError::SrcImageContiguous_07966,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcImage-07966"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcImage-07966"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-srcImage-07966"},
         }}},
        {CopyError::DstImageContiguous_07966,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstImage-07966"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstImage-07966"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-dstImage-07966"},
         }}},
        {CopyError::SrcImageSubsampled_07969,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcImage-07969"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcImage-07969"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-srcImage-07969"},
         }}},
        {CopyError::DstImageSubsampled_07969,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstImage-07969"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstImage-07969"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-dstImage-07969"},
         }}},
        {CopyError::SrcOffset_07278,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-pRegions-07278"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-pRegions-07278"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-srcImage-07274"},
         }}},
        {CopyError::SrcOffset_07279,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-pRegions-07279"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-pRegions-07279"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-srcImage-07275"},
         }}},
        {CopyError::SrcOffset_07280,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-pRegions-07280"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-pRegions-07280"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-srcImage-07276"},
         }}},
        {CopyError::DstOffset_07281,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-pRegions-07281"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-pRegions-07281"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-dstImage-07274"},
         }}},
        {CopyError::DstOffset_07282,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-pRegions-07282"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-pRegions-07282"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-dstImage-07275"},
         }}},
        {CopyError::DstOffset_07283,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-pRegions-07283"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-pRegions-07283"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-dstImage-07276"},
         }}},
        {CopyError::SrcSubresource_00142,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-aspectMask-00142"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-aspectMask-00142"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-srcSubresource-09105"},
         }}},
        {CopyError::DstSubresource_00143,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-aspectMask-00143"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-aspectMask-00143"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-dstSubresource-09105"},
         }}},
        {CopyError::SrcOffset_00144,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcOffset-00144"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcOffset-00144"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-srcSubresource-07971"},
         }}},
        {CopyError::SrcOffset_00145,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcOffset-00145"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcOffset-00145"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-srcSubresource-07972"},
         }}},
        {CopyError::SrcOffset_00147,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcOffset-00147"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcOffset-00147"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-srcOffset-09104"},
         }}},
        {CopyError::DstOffset_00150,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstOffset-00150"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstOffset-00150"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-dstSubresource-07971"},
         }}},
        {CopyError::DstOffset_00151,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstOffset-00151"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstOffset-00151"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-dstSubresource-07972"},
         }}},
        {CopyError::DstOffset_00153,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstOffset-00153"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstOffset-00153"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-dstOffset-09104"},
         }}},
        {CopyError::SrcImage3D_04443,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcImage-04443"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcImage-04443"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-srcImage-07983"},
         }}},
        {CopyError::DstImage3D_04444,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstImage-04444"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstImage-04444"},
             {Key(Func::vkCopyImageToImage), "VUID-VkCopyImageToImageInfo-dstImage-07983"},
         }}},
    };

    const auto &result = FindVUID(error, loc, errors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-copy-buffer");
        return unhandled;
    }
    return result;
}

const std::string &GetImageMipLevelVUID(const Location &loc) {
    static const std::array<Entry, 20> errors{{
        {Key(Func::vkCmdCopyImage, Field::srcSubresource), "VUID-vkCmdCopyImage-srcSubresource-07967"},
        {Key(Func::vkCmdCopyImage, Field::dstSubresource), "VUID-vkCmdCopyImage-dstSubresource-07967"},
        {Key(Func::vkCmdCopyImage2, Field::srcSubresource), "VUID-VkCopyImageInfo2-srcSubresource-07967"},
        {Key(Func::vkCmdCopyImage2, Field::dstSubresource), "VUID-VkCopyImageInfo2-dstSubresource-07967"},
        {Key(Func::vkCopyImageToImage, Field::srcSubresource), "VUID-VkCopyImageToImageInfo-srcSubresource-07967"},
        {Key(Func::vkCopyImageToImage, Field::dstSubresource), "VUID-VkCopyImageToImageInfo-dstSubresource-07967"},
        {Key(Func::vkCmdBlitImage, Field::srcSubresource), "VUID-vkCmdBlitImage-srcSubresource-01705"},
        {Key(Func::vkCmdBlitImage, Field::dstSubresource), "VUID-vkCmdBlitImage-dstSubresource-01706"},
        {Key(Func::vkCmdBlitImage2, Field::srcSubresource), "VUID-VkBlitImageInfo2-srcSubresource-01705"},
        {Key(Func::vkCmdBlitImage2, Field::dstSubresource), "VUID-VkBlitImageInfo2-dstSubresource-01706"},
        {Key(Func::vkCmdResolveImage, Field::srcSubresource), "VUID-vkCmdResolveImage-srcSubresource-01709"},
        {Key(Func::vkCmdResolveImage, Field::dstSubresource), "VUID-vkCmdResolveImage-dstSubresource-01710"},
        {Key(Func::vkCmdResolveImage2, Field::srcSubresource), "VUID-VkResolveImageInfo2-srcSubresource-01709"},
        {Key(Func::vkCmdResolveImage2, Field::dstSubresource), "VUID-VkResolveImageInfo2-dstSubresource-01710"},
        {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-imageSubresource-07967"},
        {Key(Func::vkCmdCopyImageToBuffer2), "VUID-VkCopyImageToBufferInfo2-imageSubresource-07967"},
        {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-imageSubresource-07967"},
        {Key(Func::vkCmdCopyBufferToImage2), "VUID-VkCopyBufferToImageInfo2-imageSubresource-07967"},
        {Key(Func::vkCopyImageToMemory), "VUID-VkCopyImageToMemoryInfo-imageSubresource-07967"},
        {Key(Func::vkCopyMemoryToImage), "VUID-VkCopyMemoryToImageInfo-imageSubresource-07967"},
    }};

    const auto &result = FindVUID(loc, errors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-mip-level");
        return unhandled;
    }
    return result;
}

const std::string &GetImageArrayLayerRangeVUID(const Location &loc) {
    static const std::array<Entry, 20> errors{{
        {Key(Func::vkCmdCopyImage, Field::srcSubresource), "VUID-vkCmdCopyImage-srcSubresource-07968"},
        {Key(Func::vkCmdCopyImage, Field::dstSubresource), "VUID-vkCmdCopyImage-dstSubresource-07968"},
        {Key(Func::vkCmdCopyImage2, Field::srcSubresource), "VUID-VkCopyImageInfo2-srcSubresource-07968"},
        {Key(Func::vkCmdCopyImage2, Field::dstSubresource), "VUID-VkCopyImageInfo2-dstSubresource-07968"},
        {Key(Func::vkCopyImageToImage, Field::srcSubresource), "VUID-VkCopyImageToImageInfo-srcSubresource-07968"},
        {Key(Func::vkCopyImageToImage, Field::dstSubresource), "VUID-VkCopyImageToImageInfo-dstSubresource-07968"},
        {Key(Func::vkCmdBlitImage, Field::srcSubresource), "VUID-vkCmdBlitImage-srcSubresource-01707"},
        {Key(Func::vkCmdBlitImage, Field::dstSubresource), "VUID-vkCmdBlitImage-dstSubresource-01708"},
        {Key(Func::vkCmdBlitImage2, Field::srcSubresource), "VUID-VkBlitImageInfo2-srcSubresource-01707"},
        {Key(Func::vkCmdBlitImage2, Field::dstSubresource), "VUID-VkBlitImageInfo2-dstSubresource-01708"},
        {Key(Func::vkCmdResolveImage, Field::srcSubresource), "VUID-vkCmdResolveImage-srcSubresource-01711"},
        {Key(Func::vkCmdResolveImage, Field::dstSubresource), "VUID-vkCmdResolveImage-dstSubresource-01712"},
        {Key(Func::vkCmdResolveImage2, Field::srcSubresource), "VUID-VkResolveImageInfo2-srcSubresource-01711"},
        {Key(Func::vkCmdResolveImage2, Field::dstSubresource), "VUID-VkResolveImageInfo2-dstSubresource-01712"},
        {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-imageSubresource-07968"},
        {Key(Func::vkCmdCopyImageToBuffer2), "VUID-VkCopyImageToBufferInfo2-imageSubresource-07968"},
        {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-imageSubresource-07968"},
        {Key(Func::vkCmdCopyBufferToImage2), "VUID-VkCopyBufferToImageInfo2-imageSubresource-07968"},
        {Key(Func::vkCopyImageToMemory), "VUID-VkCopyImageToMemoryInfo-imageSubresource-07968"},
        {Key(Func::vkCopyMemoryToImage), "VUID-VkCopyMemoryToImageInfo-imageSubresource-07968"},
    }};

    const auto &result = FindVUID(loc, errors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-array-layer-range");
        return unhandled;
    }
    return result;
}

const std::string &GetImageImageLayoutVUID(const Location &loc) {
    static const std::array<Entry, 5> errors{{
        {Key(Func::vkTransitionImageLayout), "VUID-VkHostImageLayoutTransitionInfo-oldLayout-09229"},
        {Key(Func::vkCopyImageToMemory, Field::srcImageLayout), "VUID-VkCopyImageToMemoryInfo-srcImageLayout-09064"},
        {Key(Func::vkCopyMemoryToImage, Field::dstImageLayout), "VUID-VkCopyMemoryToImageInfo-dstImageLayout-09059"},
        {Key(Func::vkCopyImageToImage, Field::srcImageLayout), "VUID-VkCopyImageToImageInfo-srcImageLayout-09070"},
        {Key(Func::vkCopyImageToImage, Field::dstImageLayout), "VUID-VkCopyImageToImageInfo-dstImageLayout-09071"},
    }};

    const auto &result = FindVUID(loc, errors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-image-layout");
        return unhandled;
    }
    return result;
}

const std::string &GetSubresourceRangeVUID(const Location &loc, SubresourceRangeError error) {
    static const std::map<SubresourceRangeError, std::array<Entry, 6>> errors{
        {SubresourceRangeError::BaseMip_01486,
         {{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-subresourceRange-01486"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-subresourceRange-01486"},
             {Key(Func::vkTransitionImageLayout), "VUID-VkHostImageLayoutTransitionInfo-subresourceRange-01486"},
             {Key(Func::vkCmdClearColorImage), "VUID-vkCmdClearColorImage-baseMipLevel-01470"},
             {Key(Func::vkCmdClearDepthStencilImage), "VUID-vkCmdClearDepthStencilImage-baseMipLevel-01474"},
             {Key(Func::vkCreateImageView), "VUID-VkImageViewCreateInfo-subresourceRange-01478"},
         }}},
        {SubresourceRangeError::MipCount_01724,
         {{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-subresourceRange-01724"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-subresourceRange-01724"},
             {Key(Func::vkTransitionImageLayout), "VUID-VkHostImageLayoutTransitionInfo-subresourceRange-01724"},
             {Key(Func::vkCmdClearColorImage), "VUID-vkCmdClearColorImage-pRanges-01692"},
             {Key(Func::vkCmdClearDepthStencilImage), "VUID-vkCmdClearDepthStencilImage-pRanges-01694"},
             {Key(Func::vkCreateImageView), "VUID-VkImageViewCreateInfo-subresourceRange-01718"},
         }}},
        {SubresourceRangeError::BaseLayer_01488,
         {{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-subresourceRange-01488"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-subresourceRange-01488"},
             {Key(Func::vkTransitionImageLayout), "VUID-VkHostImageLayoutTransitionInfo-subresourceRange-01488"},
             {Key(Func::vkCmdClearColorImage), "VUID-vkCmdClearColorImage-baseArrayLayer-01472"},
             {Key(Func::vkCmdClearDepthStencilImage), "VUID-vkCmdClearDepthStencilImage-baseArrayLayer-01476"},
             {Key(Func::vkCreateImageView), "VUID-VkImageViewCreateInfo-image-06724"},
         }}},
        {SubresourceRangeError::LayerCount_01725,
         {{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-subresourceRange-01725"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-subresourceRange-01725"},
             {Key(Func::vkTransitionImageLayout), "VUID-VkHostImageLayoutTransitionInfo-subresourceRange-01725"},
             {Key(Func::vkCmdClearColorImage), "VUID-vkCmdClearColorImage-pRanges-01693"},
             {Key(Func::vkCmdClearDepthStencilImage), "VUID-vkCmdClearDepthStencilImage-pRanges-01695"},
             {Key(Func::vkCreateImageView), "VUID-VkImageViewCreateInfo-subresourceRange-06725"},
         }}},
    };

    const auto &result = FindVUID(error, loc, errors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-subresource-range");
        return unhandled;
    }
    return result;
}

const char *GetPipelineInterfaceVariableVUID(const vvl::Pipeline &pipeline, PipelineInterfaceVariableError error) {
    VkStructureType sType = pipeline.GetCreateInfoSType();
    switch (error) {
        case PipelineInterfaceVariableError::ShaderStage_07988:
            return sType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO  ? "VUID-VkGraphicsPipelineCreateInfo-layout-07988"
                   : sType == VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO ? "VUID-VkComputePipelineCreateInfo-layout-07988"
                   : sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR
                       ? "VUID-VkRayTracingPipelineCreateInfoKHR-layout-07988"
                       : "VUID-VkRayTracingPipelineCreateInfoNV-layout-07988";
        case PipelineInterfaceVariableError::Mutable_07990:
            return sType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO  ? "VUID-VkGraphicsPipelineCreateInfo-layout-07990"
                   : sType == VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO ? "VUID-VkComputePipelineCreateInfo-layout-07990"
                   : sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR
                       ? "VUID-VkRayTracingPipelineCreateInfoKHR-layout-07990"
                       : "VUID-VkRayTracingPipelineCreateInfoNV-layout-07990";
        case PipelineInterfaceVariableError::DescriptorCount_07991:
            return sType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO  ? "VUID-VkGraphicsPipelineCreateInfo-layout-07991"
                   : sType == VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO ? "VUID-VkComputePipelineCreateInfo-layout-07991"
                   : sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR
                       ? "VUID-VkRayTracingPipelineCreateInfoKHR-layout-07991"
                       : "VUID-VkRayTracingPipelineCreateInfoNV-layout-07991";
        case PipelineInterfaceVariableError::Inline_10391:
            return sType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO  ? "VUID-VkGraphicsPipelineCreateInfo-None-10391"
                   : sType == VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO ? "VUID-VkComputePipelineCreateInfo-None-10391"
                   : sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR
                       ? "VUID-VkRayTracingPipelineCreateInfoKHR-None-10391"
                       : "VUID-VkRayTracingPipelineCreateInfoNV-None-10391";
    }
    return "UNASSIGNED-CoreChecks-unhandled-pipeline-interface-variable";
}

}  // namespace vvl