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
#pragma once
#include <string>

struct Location;

namespace vvl {
class Pipeline;

enum class CopyError {
    TexelBlockSize_07975,
    MultiPlaneCompatible_07976,
    TransferGranularity_07747,
    BufferOffset_07737,
    BufferOffset_07978,
    MemoryOverlap_00173,
    ImageExtentWidthZero_06659,
    ImageExtentHeightZero_06660,
    ImageExtentDepthZero_06661,
    ImageExtentRowLength_09101,
    ImageExtentImageHeight_09102,
    AspectMaskSingleBit_09103,
    ExceedBufferBounds_00171,

    ImageOffest_07971,
    ImageOffest_07972,
    Image1D_07979,
    Image1D_07980,
    Image3D_07983,
    TexelBlockExtentWidth_07274,
    TexelBlockExtentHeight_07275,
    TexelBlockExtentDepth_07276,
    TexelBlockExtentWidth_00207,
    TexelBlockExtentHeight_00208,
    TexelBlockExtentDepth_00209,
    MultiPlaneAspectMask_07981,
    ImageOffest_09104,
    AspectMask_09105,
    bufferRowLength_09106,
    bufferImageHeight_09107,
    bufferRowLength_09108,

    SrcImage1D_00146,
    DstImage1D_00152,
    SrcImage1D_01785,
    DstImage1D_01786,
    SrcOffset_01728,
    SrcOffset_01729,
    SrcOffset_01730,
    DstOffset_01732,
    DstOffset_01733,
    DstOffset_01734,
    SrcImageContiguous_07966,
    DstImageContiguous_07966,
    SrcImageSubsampled_07969,
    DstImageSubsampled_07969,
    SrcSubresourceLayerCount_07968,
    DstSubresourceLayerCount_07968,
    SrcOffset_07278,
    SrcOffset_07279,
    SrcOffset_07280,
    DstOffset_07281,
    DstOffset_07282,
    DstOffset_07283,
    SrcSubresource_00142,
    DstSubresource_00143,
    SrcOffset_00144,
    SrcOffset_00145,
    SrcOffset_00147,
    DstOffset_00150,
    DstOffset_00151,
    DstOffset_00153,
    SrcImage3D_04443,
    DstImage3D_04444,
};

// Does not contain Host Image Copy
const std::string &GetCopyBufferImageDeviceVUID(const Location &loc, CopyError error);
// contains Host Image Copy
const std::string &GetCopyBufferImageVUID(const Location &loc, CopyError error);
const std::string &GetCopyImageVUID(const Location &loc, CopyError error);
const std::string &GetImageMipLevelVUID(const Location &loc);
const std::string &GetImageArrayLayerRangeVUID(const Location &loc);
const std::string &GetImageImageLayoutVUID(const Location &loc);

enum class SubresourceRangeError {
    BaseMip_01486,
    MipCount_01724,
    BaseLayer_01488,
    LayerCount_01725,
};
const std::string &GetSubresourceRangeVUID(const Location &loc, SubresourceRangeError error);

enum class PipelineInterfaceVariableError {
    ShaderStage_07988,
    Mutable_07990,
    DescriptorCount_07991,
    Inline_10391,
};
const char *GetPipelineInterfaceVariableVUID(const vvl::Pipeline &pipeline, PipelineInterfaceVariableError error);

}  // namespace vvl